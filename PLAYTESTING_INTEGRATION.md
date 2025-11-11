
# Autonomous Playtesting System - Integration Guide

**Version**: 1.0.0  
**Last Updated**: 2025-11-11

## Table of Contents
1. [Integration Overview](#integration-overview)
2. [GitHub Actions Integration](#github-actions-integration)
3. [Custom Test Scenario Development](#custom-test-scenario-development)
4. [Adding New Analysis Modules](#adding-new-analysis-modules)
5. [Extending the Visual Verification Pipeline](#extending-the-visual-verification-pipeline)
6. [Custom Issue Classification Rules](#custom-issue-classification-rules)
7. [CI/CD Pipeline Examples](#cicd-pipeline-examples)
8. [External System Integration](#external-system-integration)

## Integration Overview

The Autonomous Playtesting System is designed to integrate seamlessly with existing development workflows and CI/CD pipelines. This guide covers integration with popular tools and platforms, as well as extending the system with custom functionality.

### Integration Capabilities

- **GitHub Actions**: Automated testing on push/PR
- **Custom Test Scenarios**: Domain-specific test cases
- **Analysis Modules**: Custom visual/text/object analysis
- **Issue Classification**: Custom rules and patterns
- **External APIs**: Integration with bug trackers, notifications
- **Data Export**: Integration with analytics platforms

## GitHub Actions Integration

### Basic GitHub Actions Workflow

Create `.github/workflows/playtesting.yml`:

```yaml
name: Autonomous Playtesting

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main, develop ]
  schedule:
    # Run nightly at 2 AM UTC
    - cron: '0 2 * * *'

jobs:
  playtesting:
    runs-on: windows-latest
    
    steps:
    - name: Checkout code
      uses: actions/checkout@v3
    
    - name: Set up Python
      uses: actions/setup-python@v4
      with:
        python-version: '3.9'
    
    - name: Install dependencies
      run: |
        python -m pip install --upgrade pip
        pip install flask==2.3.2 flask-cors==4.0.0 requests==2.31.0
        pip install psutil==5.9.5 opencv-python==4.8.0.76
        pip install pillow==10.0.0 mss==9.0.1 numpy==1.24.3
    
    - name: Run playtesting
      run: |
        python continuous_playtesting.py --iterations 50 --config playtesting_config_ci.json
      timeout-minutes: 120
    
    - name: Upload test results
      uses: actions/upload-artifact@v3
      if: always()
      with:
        name: playtesting-results
        path: |
          playtesting_data/
          screenshots_visual_playtest/
          automation_config/logs/
        retention-days: 30
    
    - name: Upload dashboard
      uses: actions/upload-artifact@v3
      if: always()
      with:
        name: test-dashboard
        path: test_dashboard.html
        retention-days: 30
    
    - name: Comment PR with results
      if: github.event_name == 'pull_request'
      uses: actions/github-script@v6
      with:
        script: |
          const fs = require('fs');
          const results = JSON.parse(fs.readFileSync('playtesting_data/sessions/latest/session_summary.json', 'utf8'));
          
          const comment = `## 🚀 Playtesting Results
          
          **Session ID**: ${results.session_id}
          **Duration**: ${results.duration_minutes.toFixed(1)} minutes
          **Total Iterations**: ${results.total_iterations}
          
          ### Test Summary
          - ✅ Passed: ${results.test_results.passed}
          - ❌ Failed: ${results.test_results.failed}
          - ⚠️ Errors: ${results.test_results.error}
          - 📊 Success Rate: ${results.test_results.success_rate.toFixed(1)}%
          
          ### Performance
          - 🎮 Average FPS: ${results.performance_summary.avg_fps?.toFixed(1) || 'N/A'}
          - 💾 Memory Usage: ${results.performance_summary.avg_memory_mb?.toFixed(0) || 'N/A'} MB
          
          ### Issues
          - 🐛 Total Issues: ${results.issue_summary.total_open_issues || 0}
          
          [View Detailed Results](https://github.com/${{ github.repository }}/actions/runs/${{ github.run_id }})
          `;
          
          github.rest.issues.createComment({
            issue_number: context.issue.number,
            owner: context.repo.owner,
            repo: context.repo.repo,
            body: comment
          });
```

### Advanced GitHub Actions Workflow

```yaml
name: Comprehensive Playtesting

on:
  workflow_dispatch:
    inputs:
      test_duration:
        description: 'Test duration in minutes'
        required: true
        default: '60'
        type: choice
        options:
        - '30'
        - '60'
        - '120'
        - '240'
      test_scenarios:
        description: 'Test scenarios to run'
        required: true
        default: 'all'
        type: choice
        options:
        - 'all'
        - 'first_mission'
        - 'combat_scenario'
        - 'free_exploration'

jobs:
  setup:
    runs-on: windows-latest
    outputs:
      test_id: ${{ steps.test_setup.outputs.test_id }}
    
    steps:
    - name: Setup test environment
      id: test_setup
      run: |
        $testId = "playtest-$(Get-Date -Format 'yyyyMMdd_HHmmss')"
        echo "test_id=$testId" >> $env:GITHUB_OUTPUT
        echo "TEST_ID=$testId" >> $env:GITHUB_ENV
        
    - name: Cache dependencies
      uses: actions/cache@v3
      with:
        path: |
          ~\AppData\Local\pip\Cache
        key: ${{ runner.os }}-pip-${{ hashFiles('**/requirements.txt') }}
        restore-keys: |
          ${{ runner.os }}-pip-

  playtesting:
    needs: setup
    runs-on: windows-latest
    
    steps:
    - name: Checkout code
      uses: actions/checkout@v3
    
    - name: Set up Python
      uses: actions/setup-python@v4
      with:
        python-version: '3.9'
    
    - name: Install system dependencies
      run: |
        choco install tesseract -y
        echo "C:\Program Files\Tesseract-OCR" | Out-File -FilePath $env:GITHUB_PATH -Encoding utf8 -Append
    
    - name: Install Python dependencies
      run: |
        python -m pip install --upgrade pip
        pip install -r requirements.txt
    
    - name: Start analysis server
      run: |
        Start-Process python -ArgumentList "analysis_server.py" -NoNewWindow
        Start-Sleep -Seconds 10
    
    - name: Run playtesting
      run: |
        $duration = ${{ github.event.inputs.test_duration }}
        $iterations = [math]::Floor($duration * 60 / 30)  # 30s per iteration
        $scenarios = "${{ github.event.inputs.test_scenarios }}"
        
        if ($scenarios -eq "all") {
          python continuous_playtesting.py --iterations $iterations --config playtesting_config_ci.json
        } else {
          python continuous_playtesting.py --iterations $iterations --scenario $scenarios --config playtesting_config_ci.json
        }
      timeout-minutes: ${{ fromJSON(github.event.inputs.test_duration) + 30 }}
    
    - name: Generate reports
      if: always()
      run: |
        python generate_dashboard.py --input playtesting_data/sessions/latest --output test_dashboard.html
        python generate_performance_report.py --session latest --output performance_report.html
    
    - name: Upload results
      uses: actions/upload-artifact@v3
      if: always()
      with:
        name: playtesting-${{ needs.setup.outputs.test_id }}
        path: |
          playtesting_data/
          screenshots_visual_playtest/
          automation_config/logs/
          test_dashboard.html
          performance_report.html
        retention-days: 90
    
    - name: Upload to S3 (optional)
      if: always() && secrets.AWS_S3_BUCKET
      uses: jakejarvis/s3-sync-action@master
      with:
        args: --acl private --follow-symlinks --delete
      env:
        AWS_S3_BUCKET: ${{ secrets.AWS_S3_BUCKET }}
        AWS_ACCESS_KEY_ID: ${{ secrets.AWS_ACCESS_KEY_ID }}
        AWS_SECRET_ACCESS_KEY: ${{ secrets.AWS_SECRET_ACCESS_KEY }}
        AWS_REGION: 'us-east-1'
        SOURCE_DIR: 'playtesting_data'

  notify:
    needs: [setup, playtesting]
    runs-on: ubuntu-latest
    if: always()
    
    steps:
    - name: Send Slack notification
      if: secrets.SLACK_WEBHOOK_URL
      uses: 8398a7/action-slack@v3
      with:
        status: ${{ job.status }}
        channel: '#testing'
        webhook_url: ${{ secrets.SLACK_WEBHOOK_URL }}
        fields: repo,message,commit,author,action,eventName,ref,workflow
        mention: 'here'
        if_mention: 'failure'
```

### CI Configuration File: `playtesting_config_ci.json`

```json
{
  "desktop_launcher": {
    "default_resolution": "1280x720",
    "default_quality": "medium",
    "max_restarts": 5,
    "timeout_seconds": 0,
    "enable_console": false,
    "enable_debug": false
  },
  "screenshot_capture": {
    "capture_interval": 5.0,
    "jpeg_quality": 80,
    "max_screenshots": 500,
    "auto_cleanup": true,
    "enable_ai_transmission": true
  },
  "test_scenarios": {
    "default_timeout": 300,
    "max_attempts": 3,
    "scenario_configs": {
      "first_mission": {
        "timeout": 180,
        "success_criteria": {
          "mission_completed": true,
          "min_distance_traveled": 1000.0
        }
      },
      "combat_scenario": {
        "timeout": 240,
        "randomize": true,
        "success_criteria": {
          "enemies_defeated": 3,
          "player_survived": true
        }
      },
      "free_exploration": {
        "timeout": 600,
        "randomize": true,
        "success_criteria": {
          "min_distance_explored": 5000.0,
          "discoveries_made": 3
        }
      }
    }
  },
  "performance_monitoring": {
    "track_fps": true,
    "track_memory": true,
    "fps_warning_threshold": 30,
    "memory_warning_threshold_mb": 4096,
    "log_performance_interval": 60
  },
  "logging": {
    "log_level": "INFO",
    "log_to_file": true,
    "log_to_console": false,
    "max_log_file_size_mb": 50,
    "max_log_files": 5
  },
  "data_management": {
    "data_retention_days": 7,
    "compress_old_data": true,
    "cleanup_interval_hours": 24
  }
}
```

## Custom Test Scenario Development

### Scenario Structure

Test scenarios are defined in the configuration file and implemented in code.

### Basic Scenario Template

```python
from dataclasses import dataclass
from typing import Dict, Any, Optional
import time
import requests

@dataclass
class TestResult:
    scenario_name: str
    status: str  # "passed", "failed", "error", "timeout"
    duration: float
    message: str
    data: Optional[Dict[str, Any]] = None

class CustomTestScenario:
    def __init__(self, server_url: str = "http://localhost:8080"):
        self.server_url = server_url
        self.ship_id = None
    
    def run(self, config: Dict[str, Any]) -> TestResult:
        """
        Run the custom test scenario
        
        Args:
            config: Scenario configuration from JSON
            
        Returns:
            TestResult with test outcome
        """
        start_time = time.time()
        timeout = config.get('timeout', 300)
        
        try:
            # Setup phase
            setup_success = self._setup_scenario(config)
            if not setup_success:
                return TestResult(
                    scenario_name="custom_scenario",
                    status="error",
                    duration=time.time() - start_time,
                    message="Failed to setup scenario"
                )
            
            # Execution phase
            execution_success = self._execute_scenario(config, timeout)
            
            if execution_success:
                return TestResult(
                    scenario_name="custom_scenario",
                    status="passed",
                    duration=time.time() - start_time,
                    message="Scenario completed successfully",
                    data=self._collect_results()
                )
            else:
                return TestResult(
                    scenario_name="custom_scenario",
                    status="failed",
                    duration=time.time() - start_time,
                    message="Scenario failed to meet success criteria"
                )
                
        except TimeoutError:
            return TestResult(
                scenario_name="custom_scenario",
                status="timeout",
                duration=time.time() - start_time,
                message=f"Scenario timed out after {timeout} seconds"
            )
        except Exception as e:
            return TestResult(
                scenario_name="custom_scenario",
                status="error",
                duration=time.time() - start_time,
                message=f"Scenario error: {str(e)}"
            )
    
    def _setup_scenario(self, config: Dict[str, Any]) -> bool:
        """Setup the test scenario"""
        # Spawn ship
        spawn_response = requests.post(
            f"{self.server_url}/spawn_ship",
            json={
                "ship_class": config.get('ship_class', '/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer.BP_VRSpaceshipPlayer_C'),
                "location": config.get('start_position', {"x": 0, "y": 0, "z": 500})
            }
        ).json()
        
        if spawn_response['success']:
            self.ship_id = spawn_response['data']['ship_id']
            return True
        return False
    
    def _execute_scenario(self, config: Dict[str, Any], timeout: float) -> bool:
        """Execute the main test logic"""
        success_criteria = config.get('success_criteria', {})
        failure_criteria = config.get('failure_criteria', {})
        
        start_time = time.time()
        check_interval = config.get('check_interval', 5.0)
        
        while time.time() - start_time < timeout:
            # Get current game state
            game_state = requests.get(f"{self.server_url}/game_state").json()
            
            # Check success criteria
            if self._check_success_criteria(game_state, success_criteria):
                return True
            
            # Check failure criteria
            if self._check_failure_criteria(game_state, failure_criteria):
                return False
            
            # Execute scenario-specific logic
            self._scenario_logic(game_state, config)
            
            time.sleep(check_interval)
        
        return False
    
    def _check_success_criteria(self, game_state: Dict[str, Any], criteria: Dict[str, Any]) -> bool:
        """Check if success criteria are met"""
        for criterion, target_value in criteria.items():
            if criterion == "mission_completed":
                if game_state.get('mission', {}).get('mission_state') == 'completed':
                    return True
            elif criterion == "min_distance_traveled":
                # Calculate distance traveled
                pass
            # Add more criteria checks...
        
        return False
    
    def _check_failure_criteria(self, game_state: Dict[str, Any], criteria: Dict[str, Any]) -> bool:
        """Check if failure criteria are met"""
        for criterion, enabled in criteria.items():
            if not enabled:
                continue
            
            if criterion == "ship_destroyed":
                if game_state.get('player', {}).get('health', 100) <= 0:
                    return True
            elif criterion == "timeout":
                # Handled by main loop
                pass
            # Add more criteria checks...
        
        return False
    
    def _scenario_logic(self, game_state: Dict[str, Any], config: Dict[str, Any]):
        """Scenario-specific logic"""
        # Implement custom behavior
        # Example: Navigate to waypoints, complete objectives, etc.
        pass
    
    def _collect_results(self) -> Dict[str, Any]:
        """Collect test results data"""
        return {
            "ship_id": self.ship_id,
            "final_state": requests.get(f"{self.server_url}/game_state").json(),
            "performance": requests.get(f"{self.server_url}/performance_metrics").json()
        }
```

### Complex Scenario Example: Trading System Test

```python
class TradingSystemTest(CustomTestScenario):
    def __init__(self, server_url: str = "http://localhost:8080"):
        super().__init__(server_url)
        self.initial_credits = 0
        self.trades_completed = 0
    
    def _setup_scenario(self, config: Dict[str, Any]) -> bool:
        """Setup trading scenario"""
        # Spawn ship at trading station
        spawn_success = super()._setup_scenario(config)
        if not spawn_success:
            return False
        
        # Record initial credits
        game_state = requests.get(f"{self.server_url}/game_state").json()
        self.initial_credits = game_state.get('inventory', {}).get('credits', 0)
        
        # Navigate to trading station
        self._navigate_to_trading_station()
        
        return True
    
    def _scenario_logic(self, game_state: Dict[str, Any], config: Dict[str, Any]):
        """Execute trading logic"""
        # Check if at trading station
        if self._is_at_trading_station(game_state):
            # Attempt trade
            trade_success = self._execute_trade()
            if trade_success:
                self.trades_completed += 1
    
    def _check_success_criteria(self, game_state: Dict[str, Any], criteria: Dict[str, Any]) -> bool:
        """Check trading success criteria"""
        current_credits = game_state.get('inventory', {}).get('credits', 0)
        
        # Check credits earned
        min_credits = criteria.get('credits_earned', 0)
        if current_credits - self.initial_credits >= min_credits:
            return True
        
        # Check trades completed
        min_trades = criteria.get('trades_completed', 0)
        if self.trades_completed >= min_trades:
            return True
        
        return False
    
    def _execute_trade(self) -> bool:
        """Execute a trade transaction"""
        # This would interact with trading system UI/API
        # For now, simulate trade
        trade_response = requests.post(
            f"{self.server_url}/execute_command",
            json={"command": "trading.BuyItem", "parameters": ["fuel", "10"]}
        ).json()
        
        return trade_response.get('success', False)
    
    def _navigate_to_trading_station(self):
        """Navigate ship to trading station"""
        # Implement navigation logic
        requests.post(
            f"{self.server_url}/set_input",
            json={
                "ship_id": self.ship_id,
                "thrust": {"x": 0.5, "y": 0, "z": 0}
            }
        )
        time.sleep(10)  # Simulate travel time
    
    def _is_at_trading_station(self, game_state: Dict[str, Any]) -> bool:
        """Check if ship is at trading station"""
        # Check distance to nearest station
        nearby_actors = game_state.get('nearby_actors', [])
        for actor in nearby_actors:
            if actor.get('type') == 'trading_station':
                if actor.get('distance', 999999) < 100:  # Within 100 units
                    return True
        return False
```

## Adding New Analysis Modules

### Analysis Module Architecture

Analysis modules follow a plugin architecture for easy extension.

### Creating a Custom Visual Analyzer

```python
from typing import Dict, Any, List
import cv2
import numpy as np
from dataclasses import dataclass

@dataclass
class VisualAnalysisResult:
    ui_elements: List[Dict[str, Any]]
    visual_artifacts: List[Dict[str, Any]]
    screen_resolution: Dict[str, int]
    analysis_metadata: Dict[str, Any]

class CustomVisualAnalyzer:
    def __init__(self, config: Dict[str, Any] = None):
        self.config = config or {}
        self.confidence_threshold = self.config.get('confidence_threshold', 0.7)
    
    def analyze_screenshot(self, image_path: str) -> VisualAnalysisResult:
        """
        Analyze screenshot for UI elements and visual artifacts
        
        Args:
            image_path: Path to screenshot image
            
        Returns:
            VisualAnalysisResult with analysis results
        """
        # Load image
        image = cv2.imread(image_path)
        if image is None:
            raise ValueError(f"Could not load image: {image_path}")
        
        height, width = image.shape[:2]
        
        # Detect UI elements
        ui_elements = self._detect_ui_elements(image)
        
        # Detect visual artifacts
        visual_artifacts = self._detect_visual_artifacts(image)
        
        # Perform custom analysis
        custom_metrics = self._custom_analysis(image)
        
        return VisualAnalysisResult(
            ui_elements=ui_elements,
            visual_artifacts=visual_artifacts,
            screen_resolution={"width": width, "height": height},
            analysis_metadata={
                "custom_metrics": custom_metrics,
                "analysis_timestamp": time.time()
            }
        )
    
    def _detect_ui_elements(self, image: np.ndarray) -> List[Dict[str, Any]]:
        """Detect UI elements in the screenshot"""
        ui_elements = []
        
        # Convert to grayscale for template matching
        gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        
        # Define UI element templates (would load from files in practice)
        templates = {
            'button': self._create_button_template(),
            'text_box': self._create_textbox_template(),
            'health_bar': self._create_healthbar_template()
        }
        
        for element_type, template in templates.items():
            # Template matching
            result = cv2.matchTemplate(gray, template, cv2.TM_CCOEFF_NORMED)
            locations = np.where(result >= self.confidence_threshold)
            
            for pt in zip(*locations[::-1]):
                ui_elements.append({
                    "element_type": element_type,
                    "position": {"x": int(pt[0]), "y": int(pt[1])},
                    "size": {
                        "width": template.shape[1],
                        "height": template.shape[0]
                    },
                    "confidence": float(result[pt[1], pt[0]])
                })
        
        return ui_elements
    
    def _detect_visual_artifacts(self, image: np.ndarray) -> List[Dict[str, Any]]:
        """Detect visual artifacts and anomalies"""
        artifacts = []
        
        # Check for screen tearing
        tearing = self._detect_screen_tearing(image)
        if tearing:
            artifacts.append({
                "type": "screen_tearing",
                "severity": "medium",
                "description": "Horizontal tearing detected",
                "location": tearing
            })
        
        # Check for texture corruption
        corruption = self._detect_texture_corruption(image)
        if corruption:
            artifacts.append({
                "type": "texture_corruption",
                "severity": "high",
                "description": "Corrupted textures detected",
                "location": corruption
            })
        
        # Check for lighting artifacts
        lighting = self._detect_lighting_artifacts(image)
        if lighting:
            artifacts.append({
                "type": "lighting_artifact",
                "severity": "low",
                "description": "Unusual lighting pattern",
                "location": lighting
            })
        
        return artifacts
    
    def _custom_analysis(self, image: np.ndarray) -> Dict[str, Any]:
        """Perform custom analysis specific to your game"""
        metrics = {}
        
        # Example: Analyze color distribution
        color_hist = cv2.calcHist([image], [0, 1, 2], None, [8, 8, 8], [0, 256, 0, 256, 0, 256])
        metrics['color_distribution'] = color_hist.flatten().tolist()
        
        # Example: Detect motion blur
        gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        blur_score = cv2.Laplacian(gray, cv2.CV_64F).var()
        metrics['blur_score'] = blur_score
        metrics['is_blurry'] = blur_score < 100
        
        # Example: Check for HUD elements
        hud_region = image[0:100, 0:1280]  # Top 100 pixels
        hud_metrics = self._analyze_hud(hud_region)
        metrics['hud_analysis'] = hud_metrics
        
        return metrics
    
    def _detect_screen_tearing(self, image: np.ndarray) -> List[Dict[str, int]]:
        """Detect screen tearing artifacts"""
        # Look for horizontal discontinuities
        gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        edges = cv2.Sobel(gray, cv2.CV_64F, 0, 1, ksize=3)
        
        tearing_locations = []
        for y in range(0, image.shape[0], 10):  # Check every 10 rows
            row_variance = np.var(edges[y:y+1, :])
            if row_variance > 1000:  # High variance indicates tearing
                tearing_locations.append({"y": y})
        
        return tearing_locations
    
    def _detect_texture_corruption(self, image: np.ndarray) -> List[Dict[str, int]]:
        """Detect texture corruption"""
        # Look for uniform colored blocks (possible corruption)
        corruption_locations = []
        
        # Check for large areas of identical pixels
        h, w = image.shape[:2]
        for y in range(0, h, 50):
            for x in range(0, w, 50):
                block = image[y:y+50, x:x+50]
                if np.std(block) < 5:  # Very low variance = possible corruption
                    corruption_locations.append({"x": x, "y": y})
        
        return corruption_locations
    
    def _analyze_hud(self, hud_region: np.ndarray) -> Dict[str, Any]:
        """Analyze HUD region for completeness"""
        # Convert to HSV for color analysis
        hsv = cv2.cvtColor(hud_region, cv2.COLOR_BGR2HSV)
        
        # Look for common HUD colors (health bars, ammo counters, etc.)
        hud_colors = {
            'health_red': cv2.inRange(hsv, (0, 100, 100), (10, 255, 255)),
            'shield_blue': cv2.inRange(hsv, (100, 100, 100), (130, 255, 255)),
            'ammo_yellow': cv2.inRange(hsv, (20, 100, 100), (40, 255, 255))
        }
        
        hud_metrics = {}
        for element, mask in hud_colors.items():
            coverage = np.sum(mask > 0) / mask.size
            hud_metrics[f'{element}_coverage'] = coverage
            hud_metrics[f'{element}_present'] = coverage > 0.01
        
        return hud_metrics
    
    def _create_button_template(self) -> np.ndarray:
        """Create template for button detection"""
        # In practice, load from file
        template = np.zeros((30, 100), dtype=np.uint8)
        cv2.rectangle(template, (0, 0), (99, 29), 255, 2)
        return template
    
    def _create_textbox_template(self) -> np.ndarray:
        """Create template for textbox detection"""
        template = np.zeros((25, 150), dtype=np.uint8)
        cv2.rectangle(template, (0, 0), (149, 24), 255, 1)
        return template
    
    def _create_healthbar_template(self) -> np.ndarray:
        """Create template for healthbar detection"""
        template = np.zeros((10, 80), dtype=np.uint8)
        cv2.rectangle(template, (0, 0), (79, 9), 255, -1)
        return template
```

### Integrating Custom Analyzer

```python
# In analysis_server.py
from custom_visual_analyzer import CustomVisualAnalyzer

class AnalysisServer:
    def __init__(self, config_path: str = "visual_analysis_config.json"):
        # ... existing initialization ...
        
        # Add custom analyzer
        self.custom_analyzer = CustomVisualAnalyzer(self.config.get('custom_analysis', {}))
    
    def _perform_analysis(self, image_path: str, analysis_options: Dict[str, bool], 
                         metadata: Dict[str, Any]) -> Dict[str, Any]:
        """Perform comprehensive analysis"""
        results = {
            "analysis_id": f"analysis_{int(time.time())}",
            "analysis_type": "comprehensive",
            "timestamp": time.time(),
            "modules": {}
        }
        
        # ... existing analysis modules ...
        
        # Custom analysis
        if analysis_options.get("custom_analysis", True):
            start_time = time.time()
            custom_results = self.custom_analyzer.analyze_screenshot(image_path)
            results["modules"]["custom_analysis"] = {
                "enabled": True,
                "processing_time": time.time() - start_time,
                "results": {
                    "ui_elements_count": len(custom_results.ui_elements),
                    "visual_artifacts_count": len(custom_results.visual_artifacts),
                    "custom_metrics": custom_results.analysis_metadata.get('custom_metrics', {})
                }
            }
        
        return results
```

## Extending the Visual Verification Pipeline

### Pipeline Architecture

The visual verification pipeline processes screenshots through multiple stages:

1. **Capture**: Screenshot acquisition
2. **Preprocessing**: Image enhancement and normalization
3. **Analysis**: Multi-module analysis (UI, text, objects, custom)
4. **Comparison**: State comparison and change detection
5. **Classification**: Issue classification and severity assessment
6. **Reporting**: Results aggregation and storage

### Adding Pipeline Stages

```python
class ExtendedAnalysisPipeline:
    def __init__(self, config: Dict[str, Any]):
        self.config = config
        self.stages = [
            self.preprocessing_stage,
            self.analysis_stage,
            self.comparison_stage,
            self.classification_stage,
            self.postprocessing_stage
        ]
    
    def process_screenshot(self, image_path: str, metadata: Dict[str, Any]) -> Dict[str, Any]:
        """Process screenshot through extended pipeline"""
        context = {
            'image_path': image_path,
            'metadata': metadata,
            'results': {}
        }
        
        # Execute pipeline stages
        for stage in self.stages:
            try:
                context = stage(context)
            except Exception as e:
                print(f"Pipeline stage failed: {stage.__name__} - {e}")
                context['results'][f'{stage.__name__}_error'] = str(e)
        
        return context['results']
    
    def preprocessing_stage(self, context: Dict[str, Any]) -> Dict[str, Any]:
        """Preprocess image for better analysis"""
        image = cv2.imread(context['image_path'])
        
        # Apply denoising
        denoised = cv2.fastNlMeansDenoisingColored(image, None, 10, 10, 7, 21)
        
        # Enhance contrast
        lab = cv2.cvtColor(denoised, cv2.COLOR_BGR2LAB)
        l, a, b = cv2.split(lab)
        clahe = cv2.createCLAHE(clipLimit=3.0, tileGridSize=(8,8))
        l = clahe.apply(l)
        enhanced = cv2.merge([l, a, b])
        enhanced = cv2.cvtColor(enhanced, cv2.COLOR_LAB2BGR)
        
        # Save preprocessed image
        preprocessed_path = context['image_path'].replace('.png', '_preprocessed.png')
        cv2.imwrite(preprocessed_path, enhanced)
        
        context['preprocessed_path'] = preprocessed_path
        context['results']['preprocessing'] = {
            'denoising_applied': True,
            'contrast_enhanced': True,
            'output_path': preprocessed_path
        }
        
        return context
    
    def analysis_stage(self, context: Dict[str, Any]) -> Dict[str, Any]:
        """Run multiple analysis modules"""
        # Use preprocessed image if available
        image_path = context.get('preprocessed_path', context['image_path'])
        
        # Run analyses in parallel
        with ThreadPoolExecutor() as executor:
            future_ui = executor.submit(self._analyze_ui, image_path)
            future_text = executor.submit(self._analyze_text, image_path)
            future_objects = executor.submit(self._analyze_objects, image_path)
            future_custom = executor.submit(self._analyze_custom, image_path)
            
            context['results']['analysis'] = {
                'ui_elements': future_ui.result(),
                'text_elements': future_text.result(),
                'objects': future_objects.result(),
                'custom_metrics': future_custom.result()
            }
        
        return context
    
    def comparison_stage(self, context: Dict[str, Any]) -> Dict[str, Any]:
        """Compare with previous state or expected state"""
        current_results = context['results']['analysis']
        
        # Load previous state if available
        previous_state = self._load_previous_state()
        
        if previous_state:
            differences = self._compare_states(previous_state, current_results)
            context['results']['comparison'] = {
                'has_differences': len(differences) > 0,
                'differences': differences,
                'difference_count': len(differences)
            }
        
        # Save current state for future comparison
        self._save_state(current_results)
        
        return context
    
    def classification_stage(self, context: Dict[str, Any]) -> Dict[str, Any]:
        """Classify issues based on analysis results"""
        analysis_results = context['results']['analysis']
        comparison_results = context['results'].get('comparison', {})
        
        issues = []
        
        # Classify based on analysis
        if analysis_results.get('ui_elements'):
            ui_issues = self._classify_ui_issues(analysis_results['ui_elements'])
            issues.extend(ui_issues)
        
        if analysis_results.get('text_elements'):
            text_issues = self._classify_text_issues(analysis_results['text_elements'])
            issues.extend(text_issues)
        
        # Classify based on comparison
        if comparison_results.get('has_differences'):
            state_issues = self._classify_state_changes(comparison_results['differences'])
            issues.extend(state_issues)
        
        context['results']['classification'] = {
            'total_issues': len(issues),
            'issues': issues,
            'severity_breakdown': self._count_severities(issues)
        }
        
        return context
    
    def postprocessing_stage(self, context: Dict[str, Any]) -> Dict[str, Any]:
        """Final processing and result formatting"""
        results = context['results']
        
        # Calculate overall metrics
        total_processing_time = sum(
            stage_result.get('processing_time', 0)
            for stage_result in results.values()
            if isinstance(stage_result, dict)
        )
        
        results['overall_metrics'] = {
            'total_processing_time': total_processing_time,
            'stage_count': len(results),
            'critical_issues': results.get('classification', {}).get('severity_breakdown', {}).get('critical', 0)
        }
        
        # Add recommendations
        results['recommendations'] = self._generate_recommendations(results)
        
        return context
    
    def _analyze_ui(self, image_path: str) -> List[Dict[str, Any]]:
        """Analyze UI elements"""
        # Implementation
        return []
    
    def _analyze_text(self, image_path: str) -> List[Dict[str, Any]]:
        """Analyze text elements"""
        # Implementation
        return []
    
    def _analyze_objects(self, image_path: str) -> List[Dict[str, Any]]:
        """Analyze game objects"""
        # Implementation
        return []
    
    def _analyze_custom(self, image_path: str) -> Dict[str, Any]:
        """Custom analysis"""
        # Implementation
        return {}
    
    def _load_previous_state(self) -> Optional[Dict[str, Any]]:
        """Load previous analysis state"""
        # Implementation
        return None
    
    def _save_state(self, state: Dict[str, Any]):
        """Save current state"""
        # Implementation
        pass
    
    def _compare_states(self, previous: Dict[str, Any], current: Dict[str, Any]) -> List[Dict[str, Any]]:
        """Compare states and find differences"""
        # Implementation
        return []
    
    def _classify_ui_issues(self, ui_elements: List[Dict[str, Any]]) -> List[Dict[str, Any]]:
        """Classify UI-related issues"""
        # Implementation
        return []
    
    def _classify_text_issues(self, text_elements: List[Dict[str, Any]]) -> List[Dict[str, Any]]:
        """Classify text-related issues"""
        # Implementation
        return []
    
    def _classify_state_changes(self, differences: List[Dict[str, Any]]) -> List[Dict[str, Any]]:
        """Classify state change issues"""
        # Implementation
        return []
    
    def _count_severities(self, issues: List[Dict[str, Any]]) -> Dict[str, int]:
        """Count issues by severity"""
        severity_counts = {'critical': 0, 'high': 0, 'medium': 0, 'low': 0}
        for issue in issues:
            severity = issue.get('severity', 'medium')
            severity_counts[severity] += 1
        return severity_counts
    
    def _generate_recommendations(self, results: Dict[str, Any]) -> List[str]:
        """Generate recommendations based on results"""
        recommendations = []
        
        # Example recommendations
        if results.get('classification', {}).get('severity_breakdown', {}).get('critical', 0) > 0:
            recommendations.append("Critical issues detected - immediate investigation required")
        
        if results.get('analysis', {}).get('blur_score', 0) < 100:
            recommendations.append("Blurry screenshots detected - check capture settings")
        
        return recommendations
```

## Custom Issue Classification Rules

### Issue Classification Architecture

The issue classifier uses a rule-based system with machine learning enhancement.

### Custom Rule Definition

```python
from typing import Dict, Any, List, Optional
from dataclasses import dataclass
import re

@dataclass
class ClassificationRule:
    """Rule for classifying issues"""
    rule_id: str
    name: str
    description: str
    severity: str  # "critical", "high", "medium", "low"
    conditions: List[Dict[str, Any]]
    recommendations: List[str]
    tags: List[str]

class CustomIssueClassifier:
    def __init__(self, config: Dict[str, Any] = None):
        self.config = config or {}
        self.rules = self._load_rules()
        self.ml_model = self._load_ml_model() if self.config.get('enable_ml', False) else None
    
    def _load_rules(self) -> List[ClassificationRule]:
        """Load classification rules"""
        # Default rules
        default_rules = [
            ClassificationRule(
                rule_id="PERF_LOW_FPS",
                name="Low FPS Detection",
                description="Detects low frame rates",
                severity="high",
                conditions=[
                    {
                        "type": "performance_metric",
                        "metric": "fps",
                        "operator": "<",
                        "threshold": 30
                    }
                ],
                recommendations=[
                    "Optimize rendering pipeline",
                    "Reduce draw calls",
                    "Check for performance regressions"
                ],
                tags=["performance", "fps", "rendering"]
            ),
            ClassificationRule(
                rule_id="UI_MISSING_HUD",
                name="Missing HUD Elements",
                description="Detects missing HUD components",
                severity="medium",
                conditions=[
                    {
                        "type": "ui_element_count",
                        "element_type": "health_bar",
                        "operator": "==",
                        "threshold": 0
                    },
                    {
                        "type": "game_state",
                        "state": "in_game",
                        "operator": "==",
                        "expected": True
                    }
                ],
                recommendations=[
                    "Check HUD initialization",
                    "Verify UI blueprint setup",
                    "Test different screen resolutions"
                ],
                tags=["ui", "hud", "visual"]
            ),
            ClassificationRule(
                rule_id="GAME_PLAYER_STUCK",
                name="Player Stuck Detection",
                description="Detects if player is stuck",
                severity="high",
                conditions=[
                    {
                        "type": "player_position_variance",
                        "time_window": 30,
                        "operator": "<",
                        "threshold": 10
                    },
                    {
                        "type": "player_input_active",
                        "operator": "==",
                        "expected": True
                    }
                ],
                recommendations=[
                    "Check collision detection",
                    "Verify player movement logic",
                    "Test navigation mesh"
                ],
                tags=["gameplay", "player", "movement"]
            )
        ]
        
        # Load custom rules from config
        custom_rules_config = self.config.get('custom_rules', [])
        custom_rules = [self._parse_rule(rule_config) for rule_config in custom_rules_config]
        
        return default_rules + custom_rules
    
    def _parse_rule(self, rule_config: Dict[str, Any]) -> ClassificationRule:
        """Parse rule from configuration"""
        return ClassificationRule(
            rule_id=rule_config['rule_id'],
            name=rule_config['name'],
            description=rule_config.get('description', ''),
            severity=rule_config['severity'],
            conditions=rule_config['conditions'],
            recommendations=rule_config.get('recommendations', []),
            tags=rule_config.get('tags', [])
        )
    
    def classify_issues(self, analysis_results: Dict[str, Any]) -> List[Dict[str, Any]]:
        """
        Classify issues based on analysis results
        
        Args:
            analysis_results: Results from analysis pipeline
            
        Returns:
            List of classified issues
        """
        issues = []
        
        # Apply rule-based classification
        for rule in self.rules:
            if self._evaluate_rule(rule, analysis_results):
                issue = self._create_issue_from_rule(rule, analysis_results)
                issues.append(issue)
        
        # Apply ML classification if enabled
        if self.ml_model:
            ml_issues = self._ml_classify(analysis_results)
            issues.extend(ml_issues)
        
        # Deduplicate and merge similar issues
        issues = self._deduplicate_issues(issues)
        
        return issues
    
    def _evaluate_rule(self, rule: ClassificationRule, analysis_results: Dict[str, Any]) -> bool:
        """Evaluate if a rule matches the analysis results"""
        rule_matches = []
        
        for condition in rule.conditions:
            condition_match = self._evaluate_condition(condition, analysis_results)
            rule_matches.append(condition_match)
        
        # All conditions must match (AND logic)
        return all(rule_matches)
    
    def _evaluate_condition(self, condition: Dict[str, Any], analysis_results: Dict[str, Any]) -> bool:
        """Evaluate a single condition"""
        condition_type = condition['type']
        
        if condition_type == "performance_metric":
            return self._evaluate_performance_condition(condition, analysis_results)
        elif condition_type == "ui_element_count":
            return self._evaluate_ui_condition(condition, analysis_results)
        elif condition_type == "game_state":
            return self._evaluate_game_state_condition(condition, analysis_results)
        elif condition_type == "text_presence":
            return self._evaluate_text_condition(condition, analysis_results)
        elif condition_type == "custom_condition":
            return self._evaluate_custom_condition(condition, analysis_results)
        
        return False
    
    def _evaluate_performance_condition(self, condition: Dict[str, Any], analysis_results: Dict[str, Any]) -> bool:
        """Evaluate performance-related condition"""
        metric = condition['metric']
        operator = condition['operator']
        threshold = condition['threshold']
        
        # Get performance metrics
        perf_metrics = analysis_results.get('performance', {})
        current_value = perf_metrics.get(metric, 0)
        
        return self._compare_values(current_value, operator, threshold)
    
    def _evaluate_ui_condition(self, condition: Dict[str, Any], analysis_results: Dict[str, Any]) -> bool:
        """Evaluate UI-related condition"""
        element_type = condition['element_type']
        operator = condition['operator']
        threshold = condition['threshold']
        
        # Count UI elements
        ui_elements = analysis_results.get('ui_elements', [])
        count = sum(1 for element in ui_elements if element.get('type') == element_type)
        
        return self._compare_values(count, operator, threshold)
    
    def _evaluate_game_state_condition(self, condition: Dict[str, Any], analysis_results: Dict[str, Any]) -> bool:
        """Evaluate game state condition"""
        expected_state = condition['expected']
        current_state = analysis_results.get('game_state', {})
        
        return current_state == expected_state
    
    def _evaluate_text_condition(self, condition: Dict[str, Any], analysis_results: Dict[str, Any]) -> bool:
        """Evaluate text-related condition"""
        search_text = condition.get('text', '')
        text_elements = analysis_results.get('text_elements', [])
        
        # Check if text is present
        found_text = any(search_text.lower() in element.get('text', '').lower() 
                          for element in text_elements)
        
        operator = condition['operator']
        expected = condition.get('expected', True)
        
        if operator == "==":
            return found_text == expected
        elif operator == "!=":
            return found_text != expected
        
        return False
    
    def _evaluate_custom_condition(self, condition: Dict[str, Any], analysis_results: Dict[str, Any]) -> bool:
        """Evaluate custom condition with user-defined logic"""
        # This would call user-defined functions
        # For now, return False
        return False
    
    def _compare_values(self, current: float, operator: str, threshold: float) -> bool:
        """Compare current value with threshold using operator"""
        if operator == "<":
            return current < threshold
        elif operator == "<=":
            return current <= threshold
        elif operator == ">":
            return current > threshold
        elif operator == ">=":
            return current >= threshold
        elif operator == "==":
            return current == threshold
        elif operator == "!=":
            return current != threshold
        
        return False
    
    def _create_issue_from_rule(self, rule: ClassificationRule, analysis_results: Dict[str, Any]) -> Dict[str, Any]:
        """Create issue from matched rule"""
        return {
            "issue_id": f"{rule.rule_id}_{int(time.time())}",
            "rule_id": rule.rule_id,
            "name": rule.name,
            "description": rule.description,
            "severity": rule.severity,
            "recommendations": rule.recommendations,
            "tags": rule.tags,
            "timestamp": time.time(),
            "analysis_context": self._extract_context(analysis_results, rule)
        }
    
    def _extract_context(self, analysis_results: Dict[str, Any], rule: ClassificationRule) -> Dict[str, Any]:
        """Extract relevant context for the issue"""
        context = {}
        
        # Extract performance metrics if relevant
        if any(cond['type'] == 'performance_metric' for cond in rule.conditions):
            context['performance'] = analysis_results.get('performance', {})
        
        # Extract UI elements if relevant
        if any(cond['type'] == 'ui_element_count' for cond in rule.conditions):
            context['ui_elements'] = analysis_results.get('ui_elements', [])
        
        # Extract game state if relevant
        if any(cond['type'] == 'game_state' for cond in rule.conditions):
            context['game_state'] = analysis_results.get('game_state', {})
        
        return context
    
    def _ml_classify(self, analysis_results: Dict[str, Any]) -> List[Dict[str, Any]]:
        """Classify issues using machine learning model"""
        if not self.ml_model:
            return []
        
        # Extract features
        features = self._extract_ml_features(analysis_results)
        
        # Make prediction
        prediction = self.ml_model.predict([features])
        probabilities = self.ml_model.predict_proba([features])
        
        issues = []
        if prediction[0] == 1:  # Issue detected
            issue = {
                "issue_id": f"ML_{int(time.time())}",
                "rule_id": "ML_CLASSIFICATION",
                "name": "ML Detected Issue",
                "description": "Issue detected by machine learning model",
                "severity": self._ml_severity_from_probability(probabilities[0][1]),
                "confidence": float(probabilities[0][1]),
                "timestamp": time.time(),
                "analysis_context": {
                    "ml_features": features,
                    "ml_prediction": int(prediction[0]),
                    "ml_probabilities": probabilities[0].tolist()
                }
            }
            issues.append(issue)
        
        return issues
    
    def _extract_ml_features(self, analysis_results: Dict[str, Any]) -> List[float]:
        """Extract features for ML model"""
        features = []
        
        # Performance features
        perf = analysis_results.get('performance', {})
        features.append(perf.get('fps', 0))
        features.append(perf.get('frame_time_ms', 0))
        features.append(perf.get('memory_mb', 0))
        
        # UI features
        ui_elements = analysis_results.get('ui_elements', [])
        features.append(len(ui_elements))
        
        # Text features
        text_elements = analysis_results.get('text_elements', [])
        features.append(len(text_elements))
        features.append(sum(len(elem.get('text', '')) for elem in text_elements))
        
        return features
    
    def _ml_severity_from_probability(self, probability: float) -> str:
        """Convert ML probability to severity"""
        if probability > 0.8:
            return "critical"
        elif probability > 0.6:
            return "high"
        elif probability > 0.4:
            return "medium"
        else:
            return "low"
    
    def _deduplicate_issues(self, issues: List[Dict[str, Any]]) -> List[Dict[str, Any]]:
        """Deduplicate similar issues"""
        unique_issues = []
        seen_signatures = set()
        
        for issue in issues:
            # Create signature based on rule and key context
            signature = f"{issue['rule_id']}:{issue.get('severity', 'medium')}"
            
            if signature not in seen_signatures:
                unique_issues.append(issue)
                seen_signatures.add(signature)
        
        return unique_issues
```

### Configuration for Custom Rules

```json
{
  "issue_classification": {
    "enable_classification": true,
    "severity_threshold": "medium",
    "min_frequency_for_pattern": 3,
    "custom_rules": [
      {
        "rule_id": "CUSTOM_TRADE_ISSUE",
        "name": "Trading System Issue",
        "description": "Detects problems with trading interface",
        "severity": "high",
        "conditions": [
          {
            "type": "ui_element_count",
            "element_type": "trade_window",
            "operator": "==",
            "threshold": 0
          },
          {
            "type": "game_state",
            "state": "at_trading_station",
            "operator": "==",
            "expected": true
          }
        ],
        "recommendations": [
          "Check trading station UI setup",
          "Verify trade window blueprint",
          "Test trading interaction"
        ],
        "tags": ["trading", "ui", "gameplay"]
      },
      {
        "rule_id": "CUSTOM_QUEST_BUG",
        "name": "Quest Progression Bug",
        "description": "Detects quest progression issues",
        "severity": "critical",
        "conditions": [
          {
            "type": "text_presence",
            "text": "Quest Complete",
            "operator": "!=",
            "expected": true
          },
          {
            "type": "performance_metric",
            "metric": "mission_time",
            "operator": ">",
            "threshold": 300
          }
        ],
        "recommendations": [
          "Check quest trigger conditions",
          "Verify objective completion logic",
          "Test quest state transitions"
        ],
        "tags": ["quest", "progression", "gameplay"]
      }
    ]
  }
}
```

## CI/CD Pipeline Examples

### Jenkins Pipeline

```groovy
pipeline {
    agent { label 'windows' }
    
    parameters {
        choice(
            name: 'TEST_SCENARIO',
            choices: ['all', 'first_mission', 'combat_scenario', 'free_exploration'],
            description: 'Test scenario to run'
        )
        string(
            name: 'TEST_DURATION',
            defaultValue: '60',
            description: 'Test duration in minutes'
        )
    }
    
    environment {
        PYTHON_PATH = 'C:\\Python39'
        UNREAL_PATH = 'C:\\Program Files\\Epic Games\\UE_5.6'
        PROJECT_PATH = 'C:\\Users\\allen\\Desktop\\Alexander\\Alexander'
    }
    
    stages {
        stage('Setup') {
            steps {
                script {
                    // Clean workspace
                    cleanWs()
                    
                    // Checkout code
                    checkout scm
                    
                    // Setup Python environment
                    bat """
                        ${PYTHON_PATH}\\python.exe -m pip install --upgrade pip
                        ${PYTHON_PATH}\\python.exe -m pip install -r requirements.txt
                    """
                    
                    // Create test ID
                    env.TEST_ID = "playtest-${BUILD_NUMBER}-${new Date().format('yyyyMMdd_HHmmss')}"
                }
            }
        }
        
        stage('Start Services') {
            parallel {
                stage('Analysis Server') {
                    steps {
                        bat """
                            start /min ${PYTHON_PATH}\\python.exe analysis_server.py
                            timeout /t 10
                        """
                    }
                }
                stage('Dashboard') {
                    steps {
                        bat """
                            start /min ${PYTHON_PATH}\\python.exe playtesting_dashboard.py
                            timeout /t 10
                        """
                    }
                }
            }
        }
        
        stage('Run Playtesting') {
            steps {
                script {
                    def iterations = (params.TEST_DURATION.toInteger() * 60 / 30).toInteger()
                    
                    bat """
                        cd ${PROJECT_PATH}
                        ${PYTHON_PATH}\\python.exe continuous_playtesting.py ^
                            --iterations ${iterations} ^
                            --scenario ${params.TEST_SCENARIO} ^
                            --config playtesting_config_ci.json
                    """
                }
            }
            post {
                always {
                    // Generate reports
                    bat """
                        cd ${PROJECT_PATH}
                        ${PYTHON_PATH}\\python.exe generate_dashboard.py ^
                            --input playtesting_data/sessions/latest ^
                            --output test_dashboard.html
                        
                        ${PYTHON_PATH}\\python.exe generate_performance_report.py ^
                            --session latest ^
                            --output performance_report.html
                    """
                }
            }
        }
        
        stage('Analyze Results') {
            steps {
                script {
                    // Parse results
                    def results = readJSON file: "${PROJECT_PATH}\\playtesting_data\\sessions\\latest\\session_summary.json"
                    
                    // Check success rate
                    if (results.test_results.success_rate < 80) {
                        currentBuild.result = 'UNSTABLE'
                        echo "Low success rate: ${results.test_results.success_rate}%"
                    }
                    
                    // Check for critical issues
                    if (results.issue_summary.critical_count > 0) {
                        currentBuild.result = 'FAILURE'
                        error "Critical issues detected: ${results.issue_summary.critical_count}"
                    }
                    
                    // Archive results
                    archiveArtifacts artifacts: 'playtesting_data/**/*', fingerprint: true
                    archiveArtifacts artifacts: 'test_dashboard.html', fingerprint: true
                    archiveArtifacts artifacts: 'performance_report.html', fingerprint: true
                }
            }
        }
        
        stage('Notify') {
            steps {
                script {
                    // Send Slack notification
                    if (env.SLACK_WEBHOOK_URL) {
                        slackSend(
                            channel: '#testing',
                            color: currentBuild.result == 'SUCCESS' ? 'good' : 'danger',
                            message: """
                                Playtesting ${currentBuild.result}
                                Test ID: ${env.TEST_ID}
                                Success Rate: ${results.test_results.success_rate}%
                                Issues: ${results.issue_summary.total_open_issues}
                                Build: ${env.BUILD_URL}
                            """
                        )
                    }
                    
                    // Update GitHub status
                    if (env.GITHUB_TOKEN) {
                        def status = currentBuild.result == 'SUCCESS' ? 'success' : 'failure'
                        def description = "Playtesting: ${results.test_results.success_rate}% success rate"
                        
                        httpRequest(
                            httpMode: 'POST',
                            requestBody: """
                                {
                                    "state": "${status}",
                                    "target_url": "${env.BUILD_URL}",
                                    "description": "${description}",
                                    "context": "continuous-integration/playtesting"
                                }
                            """,
                            customHeaders: [[name: 'Authorization', value: "token ${env.GITHUB_TOKEN}"]],
                            url: "https://api.github.com/repos/${env.GITHUB_REPO}/statuses/${env.GIT_COMMIT}"
                        )
                    }
                }
            }
        }
    }
    
    post {
        always {
            // Cleanup
            bat """
                taskkill /F /IM UnrealEditor.exe 2>nul || exit 0
                taskkill /F /IM python.exe 2>nul || exit 0
            """
            
            // Clean workspace
            cleanWs()
        }
        
        failure {
            script {
                // Upload failure artifacts
                archiveArtifacts artifacts: 'automation_config/logs/**/*.log', fingerprint: true
            }
        }
    }
}
```

### GitLab CI/CD Pipeline

```yaml
stages:
  - setup
  - test
  - report
  - notify

variables:
  PYTHON_PATH: "C:\\Python39"
  UNREAL_PATH: "C:\\Program Files\\Epic Games\\UE_5.6"
  PROJECT_PATH: "C:\\Users\\allen\\Desktop\\Alexander\\Alexander"
  TEST_ID: "playtest-${CI_PIPELINE_ID}-${CI_COMMIT_SHORT_SHA}"

playtesting:
  stage: test
  tags:
    - windows
    - unreal
  
  before_script:
    - echo "Setting up environment..."
    - '& $PYTHON_PATH\python.exe -m pip install --upgrade pip'
    - '& $PYTHON_PATH\python.exe -m pip install -r requirements.txt'
    
    # Start background services
    - Start-Process -FilePath "$PYTHON_PATH\python.exe" -ArgumentList "analysis_server.py" -WindowStyle Hidden
    - Start-Process -FilePath "$PYTHON_PATH\python.exe" -ArgumentList "playtesting_dashboard.py" -WindowStyle Hidden
    - Start-Sleep -Seconds 10
  
  script:
    # Calculate iterations based on test duration
    - $iterations = [math]::Floor($env:TEST_DURATION * 60 / 30)
    - '& $PYTHON_PATH\python.exe continuous_playtesting.py --iterations $iterations --config playtesting_config_ci.json'
  
  after_script:
    # Generate reports
    - '& $PYTHON_PATH\python.exe generate_dashboard.py --input playtesting_data/sessions/latest --output test_dashboard.html'
    - '& $PYTHON_PATH\python.exe generate_performance_report.py --session latest --output performance_report.html'
    
    # Parse results
    - $results = Get-Content playtesting_data/sessions/latest/session_summary.json | ConvertFrom-Json
    - echo "TEST_SUCCESS_RATE=$($results.test_results.success_rate)" >> test_results.env
    - echo "TEST_ISSUES=$($results.issue_summary.total_open_issues)" >> test_results.env
  
  artifacts:
    when: always
    paths:
      - playtesting_data/
      - screenshots_visual_playtest/
      - automation_config/logs/
      - test_dashboard.html
      - performance_report.html
    reports:
      junit: playtesting_data/sessions/latest/test_results.xml
    expire_in: 90 days
  
  timeout: 4h

report:
  stage: report
  dependencies:
    - playtesting
  script:
    # Load test results
    - Get-Content test_results.env | ForEach-Object { Invoke-Expression "`$env:$_" }
    
    # Generate summary
    - |
      @"
      ## Playtesting Results
      
      **Success Rate**: $env:TEST_SUCCESS_RATE%
      **Issues Found**: $env:TEST_ISSUES
      
      [View Detailed Report]($env:CI_JOB_URL/artifacts/browse)
      "@ | Out-File -FilePath test_summary.md
    
    # Comment on merge request
    - |
      if ($env:CI_MERGE_REQUEST_IID) {
        $comment = "@{body=$(Get-Content test_summary.md | Out-String)}"
        Invoke-RestMethod -Uri "$env:C