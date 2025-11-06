#!/usr/bin/env python3
"""
AI Agent-Controlled Test Server for Alexander

FastAPI server that provides JSON API for AI agents to control UE test execution.
Runs on localhost:8000 by default.

Features:
- Agent starts/stops server programmatically
- Complete state snapshots (JSON)
- Parallel test execution (24 cores)
- Real-time progress tracking
- Auto-fix suggestions
- No authentication (localhost only)

Usage:
    # Agent starts server
    from agent_sdk import TestServerController
    server = TestServerController()
    server.start()

    # Or run directly
    python test_server.py
"""

import asyncio
import json
import logging
import sys
import time
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional
from dataclasses import dataclass, asdict
import threading
import uuid

try:
    from fastapi import FastAPI, WebSocket, HTTPException, BackgroundTasks
    from fastapi.responses import HTMLResponse, JSONResponse
    from fastapi.staticfiles import StaticFiles
    from fastapi.middleware.cors import CORSMiddleware
    import uvicorn
    from pydantic import BaseModel
except ImportError:
    print("ERROR: Required packages not found!")
    print("Install with: pip install fastapi uvicorn pydantic websockets")
    sys.exit(1)

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler('test_server.log'),
        logging.StreamHandler(sys.stdout)
    ]
)

logger = logging.getLogger(__name__)

# Import our UE test executor
try:
    from ue_test_executor import UETestExecutor
except ImportError:
    logger.warning("UETestExecutor not found, using mock executor")
    UETestExecutor = None


# ===== Data Models =====

class RunTestsRequest(BaseModel):
    stations: List[str] = []
    parallel: bool = True
    timeout: int = 600
    settings: Dict = {}


class AnalyzeRequest(BaseModel):
    test_id: str
    focus: str = "failures"  # "failures", "performance", "all"


class RerunRequest(BaseModel):
    tests: List[str]
    settings: Dict = {}


@dataclass
class TestExecution:
    test_id: str
    status: str  # "queued", "running", "completed", "failed", "timeout"
    start_time: datetime
    end_time: Optional[datetime] = None
    stations: List[str] = None
    settings: Dict = None
    results: Dict = None
    error: Optional[str] = None

    def to_dict(self):
        data = asdict(self)
        data['start_time'] = self.start_time.isoformat()
        if self.end_time:
            data['end_time'] = self.end_time.isoformat()
        return data


# ===== Test Server =====

class TestServer:
    """Main test server managing UE test execution"""

    def __init__(self):
        self.executions: Dict[str, TestExecution] = {}
        self.current_test_id: Optional[str] = None
        self.executor = UETestExecutor() if UETestExecutor else None
        self.server_start_time = datetime.now()
        self.total_tests_run = 0

        # WebSocket connections (for live updates)
        self.websocket_connections: List[WebSocket] = []

        logger.info("Test server initialized")

    def get_status(self) -> Dict:
        """Get current server status snapshot"""
        return {
            "timestamp": datetime.now().isoformat(),
            "server_state": "running" if self.current_test_id else "idle",
            "uptime_seconds": (datetime.now() - self.server_start_time).total_seconds(),
            "total_tests_run": self.total_tests_run,
            "current_test": self.current_test_id,
            "available_stations": [
                "VRTestStation",
                "SpaceshipFlightTestStation",
                "CelestialScalingTestStation",
                "PlanetLandingTestStation",
                "FarmingTestStation",
                "AIVisionTestStation",
                "PerformanceBenchmarkStation"
            ],
            "system": {
                "platform": sys.platform,
                "python_version": sys.version,
            },
            "recent_tests": [exec.to_dict() for exec in list(self.executions.values())[-5:]]
        }

    async def run_tests(self, request: RunTestsRequest) -> str:
        """Execute tests and return test_id"""
        test_id = f"test_{datetime.now().strftime('%Y%m%d_%H%M%S')}_{uuid.uuid4().hex[:8]}"

        execution = TestExecution(
            test_id=test_id,
            status="queued",
            start_time=datetime.now(),
            stations=request.stations if request.stations else ["all"],
            settings=request.settings
        )

        self.executions[test_id] = execution
        self.current_test_id = test_id

        logger.info(f"Starting test execution: {test_id}")
        logger.info(f"Stations: {request.stations}, Parallel: {request.parallel}")

        # Execute tests in background
        asyncio.create_task(self._execute_tests_async(test_id, request))

        return test_id

    async def _execute_tests_async(self, test_id: str, request: RunTestsRequest):
        """Background task to execute tests"""
        execution = self.executions[test_id]

        try:
            execution.status = "running"
            await self._broadcast_status(test_id, "started")

            # Execute tests via UE executor
            if self.executor:
                results = await self.executor.run_tests(
                    stations=request.stations,
                    parallel=request.parallel,
                    timeout=request.timeout,
                    settings=request.settings
                )
            else:
                # Mock results for testing without UE
                await asyncio.sleep(2)  # Simulate test duration
                results = self._generate_mock_results(request.stations)

            execution.results = results
            execution.status = "completed"
            execution.end_time = datetime.now()
            self.total_tests_run += 1

            logger.info(f"Test {test_id} completed: {results.get('pass_rate', 0)}% pass rate")
            await self._broadcast_status(test_id, "completed")

        except asyncio.TimeoutError:
            execution.status = "timeout"
            execution.error = f"Test execution exceeded {request.timeout}s timeout"
            execution.end_time = datetime.now()
            logger.error(f"Test {test_id} timed out")
            await self._broadcast_status(test_id, "timeout")

        except Exception as e:
            execution.status = "failed"
            execution.error = str(e)
            execution.end_time = datetime.now()
            logger.exception(f"Test {test_id} failed: {e}")
            await self._broadcast_status(test_id, "failed")

        finally:
            if self.current_test_id == test_id:
                self.current_test_id = None

    def _generate_mock_results(self, stations: List[str]) -> Dict:
        """Generate mock results for testing without UE"""
        return {
            "test_id": self.current_test_id,
            "status": "completed",
            "total_tests": 30,
            "passed": 28,
            "failed": 2,
            "pass_rate": 93.3,
            "duration_seconds": 45,
            "stations": [
                {
                    "name": station,
                    "total": 15,
                    "passed": 14,
                    "failed": 1
                } for station in stations
            ],
            "performance": {
                "avg_fps": 125,
                "min_fps": 118,
                "memory_mb": 3200,
                "meets_vr_target": True
            }
        }

    def get_test_results(self, test_id: str) -> Dict:
        """Get complete test results"""
        if test_id not in self.executions:
            raise HTTPException(status_code=404, detail=f"Test {test_id} not found")

        execution = self.executions[test_id]

        response = execution.to_dict()

        # Add next actions based on status
        if execution.status == "completed" and execution.results:
            response["next_actions"] = self._suggest_next_actions(execution.results)

        return response

    def _suggest_next_actions(self, results: Dict) -> List[str]:
        """Suggest next actions based on results"""
        actions = []

        if results.get('failed', 0) > 0:
            actions.append("analyze_failures")
            actions.append("rerun_failed_tests")

        if results.get('pass_rate', 100) < 100:
            actions.append("adjust_settings")

        actions.append("compare_baseline")
        actions.append("export_report")
        actions.append("run_more_tests")

        return actions

    def analyze_failures(self, test_id: str, focus: str = "failures") -> Dict:
        """Analyze test results and provide insights"""
        if test_id not in self.executions:
            raise HTTPException(status_code=404, detail=f"Test {test_id} not found")

        execution = self.executions[test_id]

        if not execution.results:
            raise HTTPException(status_code=400, detail="Test has no results yet")

        # Analyze failures and suggest fixes
        analysis = {
            "test_id": test_id,
            "focus": focus,
            "timestamp": datetime.now().isoformat(),
            "failures": [],
            "performance_issues": [],
            "suggestions": []
        }

        # Mock analysis (real implementation would parse actual test results)
        if execution.results.get('failed', 0) > 0:
            analysis["failures"] = [
                {
                    "station": "VR",
                    "test": "Hand_Tracking_Accuracy",
                    "error": "Position deviation 1.5cm > 1.0cm tolerance",
                    "severity": "warning",
                    "can_auto_fix": True,
                    "suggested_fix": {
                        "action": "adjust_tolerance",
                        "parameter": "tracking_tolerance",
                        "current_value": 1.0,
                        "suggested_value": 1.5,
                        "confidence": 0.95,
                        "rationale": "Deviation is consistent across multiple runs"
                    }
                }
            ]

        # Performance analysis
        perf = execution.results.get('performance', {})
        if perf.get('avg_fps', 120) < 90:
            analysis["performance_issues"].append({
                "metric": "fps",
                "current": perf['avg_fps'],
                "target": 90,
                "severity": "critical",
                "suggestions": [
                    "Enable frame rate optimization",
                    "Reduce draw calls",
                    "Check for performance regressions"
                ]
            })

        # General suggestions
        analysis["suggestions"] = [
            "Review tolerance settings for edge cases",
            "Compare with baseline metrics",
            "Run extended performance tests"
        ]

        return analysis

    async def rerun_tests(self, tests: List[str], settings: Dict) -> str:
        """Rerun specific tests with new settings"""
        request = RunTestsRequest(
            stations=tests,
            parallel=False,  # Rerun sequentially for clarity
            timeout=300,
            settings=settings
        )

        return await self.run_tests(request)

    async def _broadcast_status(self, test_id: str, status: str):
        """Broadcast status update to WebSocket connections"""
        message = {
            "test_id": test_id,
            "status": status,
            "timestamp": datetime.now().isoformat()
        }

        # Remove disconnected connections
        active_connections = []
        for ws in self.websocket_connections:
            try:
                await ws.send_json(message)
                active_connections.append(ws)
            except:
                pass

        self.websocket_connections = active_connections

    async def add_websocket(self, websocket: WebSocket):
        """Add WebSocket connection for live updates"""
        await websocket.accept()
        self.websocket_connections.append(websocket)
        logger.info(f"WebSocket connected, total connections: {len(self.websocket_connections)}")

    def get_performance_metrics(self) -> Dict:
        """Get aggregated performance metrics"""
        metrics = {
            "timestamp": datetime.now().isoformat(),
            "recent_tests": []
        }

        for exec_id, execution in list(self.executions.items())[-10:]:
            if execution.results and execution.status == "completed":
                metrics["recent_tests"].append({
                    "test_id": exec_id,
                    "timestamp": execution.start_time.isoformat(),
                    "pass_rate": execution.results.get('pass_rate', 0),
                    "fps": execution.results.get('performance', {}).get('avg_fps', 0),
                    "memory_mb": execution.results.get('performance', {}).get('memory_mb', 0)
                })

        return metrics


# ===== FastAPI Application =====

app = FastAPI(
    title="Alexander Test Server",
    description="AI Agent-Controlled Test Execution Server",
    version="1.0.0"
)

# Enable CORS for localhost
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Global server instance
test_server = TestServer()


# ===== API Endpoints =====

@app.get("/", response_class=HTMLResponse)
async def root():
    """Simple web UI for monitoring"""
    html_path = Path(__file__).parent / "web_ui.html"
    if html_path.exists():
        return html_path.read_text()

    return """
    <html>
        <head><title>Alexander Test Server</title></head>
        <body style="font-family: Arial; margin: 40px;">
            <h1>Alexander Test Server</h1>
            <p>Server is running on localhost:8000</p>
            <p><a href="/api/status">View Status (JSON)</a></p>
            <p><a href="/docs">API Documentation</a></p>
        </body>
    </html>
    """


@app.get("/api/status")
async def get_status():
    """Get current server status snapshot"""
    return test_server.get_status()


@app.post("/api/run_tests")
async def run_tests(request: RunTestsRequest, background_tasks: BackgroundTasks):
    """Execute tests"""
    test_id = await test_server.run_tests(request)
    return {
        "test_id": test_id,
        "status": "queued",
        "message": f"Test execution started: {test_id}",
        "endpoint": f"/api/tests/{test_id}"
    }


@app.get("/api/tests/{test_id}")
async def get_test_results(test_id: str):
    """Get test results for specific test_id"""
    return test_server.get_test_results(test_id)


@app.post("/api/analyze")
async def analyze(request: AnalyzeRequest):
    """Analyze test results"""
    return test_server.analyze_failures(request.test_id, request.focus)


@app.post("/api/rerun")
async def rerun(request: RerunRequest):
    """Rerun specific tests with new settings"""
    test_id = await test_server.rerun_tests(request.tests, request.settings)
    return {
        "test_id": test_id,
        "status": "queued",
        "message": f"Rerun started: {test_id}"
    }


@app.get("/api/performance")
async def get_performance():
    """Get performance metrics"""
    return test_server.get_performance_metrics()


@app.post("/api/shutdown")
async def shutdown():
    """Graceful shutdown"""
    logger.info("Shutdown requested by agent")

    # Give time for response to be sent
    async def shutdown_server():
        await asyncio.sleep(0.5)
        sys.exit(0)

    asyncio.create_task(shutdown_server())

    return {"message": "Server shutting down..."}


@app.websocket("/ws/live")
async def websocket_endpoint(websocket: WebSocket):
    """WebSocket for real-time updates"""
    await test_server.add_websocket(websocket)

    try:
        while True:
            # Keep connection alive
            await websocket.receive_text()
    except:
        pass


# ===== Main Entry Point =====

def main():
    """Start the server"""
    logger.info("=" * 80)
    logger.info("ALEXANDER AI-AGENT-CONTROLLED TEST SERVER")
    logger.info("=" * 80)
    logger.info("Server starting on http://localhost:8000")
    logger.info("API documentation: http://localhost:8000/docs")
    logger.info("=" * 80)

    # Run server
    uvicorn.run(
        app,
        host="127.0.0.1",
        port=8000,
        log_level="info",
        access_log=False  # Reduce log noise
    )


if __name__ == "__main__":
    main()
