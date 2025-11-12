"""
Kimi API Client - Secure wrapper for Moonshot AI API integration
Handles authentication, rate limiting, error handling, cost monitoring, and token tracking
"""

import os
import time
import logging
from typing import Dict, List, Optional, Any, Union
from dataclasses import dataclass, field
from enum import Enum
import requests
from dotenv import load_dotenv
import json
from datetime import datetime, timedelta

# Load environment variables
load_dotenv()

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


class KimiModel(Enum):
    """Available Kimi AI models"""
    MOONSHOT_V1_8K = "moonshot-v1-8k"
    MOONSHOT_V1_32K = "moonshot-v1-32k"
    MOONSHOT_V1_128K = "moonshot-v1-128k"


@dataclass
class TokenUsage:
    """Track token usage for cost monitoring"""
    prompt_tokens: int = 0
    completion_tokens: int = 0
    total_tokens: int = 0
    estimated_cost: float = 0.0
    timestamp: datetime = field(default_factory=datetime.now)

    def to_dict(self) -> Dict[str, Any]:
        return {
            "prompt_tokens": self.prompt_tokens,
            "completion_tokens": self.completion_tokens,
            "total_tokens": self.total_tokens,
            "estimated_cost": self.estimated_cost,
            "timestamp": self.timestamp.isoformat()
        }


@dataclass
class APIResponse:
    """Structured API response with metadata"""
    success: bool
    content: Optional[str] = None
    error_message: Optional[str] = None
    token_usage: Optional[TokenUsage] = None
    response_time: float = 0.0
    status_code: Optional[int] = None


class KimiAPIClient:
    """
    Secure Kimi API client with authentication, rate limiting, and monitoring
    """

    # Pricing per 1M tokens (in USD)
    PRICING = {
        KimiModel.MOONSHOT_V1_8K.value: {"input": 12.0, "output": 12.0},
        KimiModel.MOONSHOT_V1_32K.value: {"input": 24.0, "output": 24.0},
        KimiModel.MOONSHOT_V1_128K.value: {"input": 60.0, "output": 60.0}
    }

    # Rate limiting (requests per minute)
    RATE_LIMIT = 100
    RATE_LIMIT_WINDOW = 60  # seconds

    def __init__(self, api_key: Optional[str] = None, base_url: Optional[str] = None):
        """
        Initialize Kimi API client

        Args:
            api_key: Kimi API key (defaults to KIMI_API_KEY env var)
            base_url: API base URL (defaults to KIMI_API_BASE_URL env var)
        """
        self.api_key = api_key or os.getenv("KIMI_API_KEY")
        self.base_url = base_url or os.getenv("KIMI_API_BASE_URL", "https://api.moonshot.cn/v1")
        
        if not self.api_key:
            raise ValueError("Kimi API key not provided. Set KIMI_API_KEY environment variable or pass api_key parameter.")

        self.model = os.getenv("KIMI_MODEL", KimiModel.MOONSHOT_V1_8K.value)
        self.max_tokens = int(os.getenv("KIMI_MAX_TOKENS", "8000"))
        self.temperature = float(os.getenv("KIMI_TEMPERATURE", "0.7"))
        self.max_retries = int(os.getenv("MAX_RETRIES", "3"))
        self.request_timeout = int(os.getenv("REQUEST_TIMEOUT", "30"))

        # Rate limiting
        self.request_times: List[datetime] = []
        
        # Usage tracking
        self.usage_history: List[TokenUsage] = []
        self.total_cost: float = 0.0
        
        # Session for connection pooling
        self.session = requests.Session()
        self.session.headers.update({
            "Authorization": f"Bearer {self.api_key}",
            "Content-Type": "application/json"
        })

        logger.info(f"Kimi API client initialized with model: {self.model}")

    def _check_rate_limit(self) -> bool:
        """Check if we're within rate limits"""
        now = datetime.now()
        self.request_times = [t for t in self.request_times if now - t < timedelta(seconds=self.RATE_LIMIT_WINDOW)]
        
        if len(self.request_times) >= self.RATE_LIMIT:
            logger.warning(f"Rate limit reached: {len(self.request_times)} requests in {self.RATE_LIMIT_WINDOW}s")
            return False
        
        return True

    def _wait_for_rate_limit(self):
        """Wait until rate limit resets"""
        while not self._check_rate_limit():
            time.sleep(1)

    def _calculate_cost(self, model: str, prompt_tokens: int, completion_tokens: int) -> float:
        """Calculate API call cost"""
        pricing = self.PRICING.get(model, self.PRICING[KimiModel.MOONSHOT_V1_8K.value])
        input_cost = (prompt_tokens / 1_000_000) * pricing["input"]
        output_cost = (completion_tokens / 1_000_000) * pricing["output"]
        return input_cost + output_cost

    def _track_usage(self, usage_data: Dict[str, int], model: str):
        """Track token usage and cost"""
        prompt_tokens = usage_data.get("prompt_tokens", 0)
        completion_tokens = usage_data.get("completion_tokens", 0)
        total_tokens = usage_data.get("total_tokens", 0)
        
        cost = self._calculate_cost(model, prompt_tokens, completion_tokens)
        self.total_cost += cost
        
        token_usage = TokenUsage(
            prompt_tokens=prompt_tokens,
            completion_tokens=completion_tokens,
            total_tokens=total_tokens,
            estimated_cost=cost
        )
        
        self.usage_history.append(token_usage)
        logger.info(f"Token usage: {total_tokens} tokens, Cost: ${cost:.4f}")
        
        return token_usage

    def _make_request(self, endpoint: str, data: Dict[str, Any]) -> APIResponse:
        """Make API request with error handling and retries"""
        url = f"{self.base_url}{endpoint}"
        start_time = time.time()
        
        # Wait for rate limit if needed
        self._wait_for_rate_limit()
        
        for attempt in range(self.max_retries):
            try:
                self.request_times.append(datetime.now())
                
                response = self.session.post(
                    url,
                    json=data,
                    timeout=self.request_timeout
                )
                
                response_time = time.time() - start_time
                
                if response.status_code == 200:
                    response_data = response.json()
                    
                    # Extract usage data
                    usage_data = response_data.get("usage", {})
                    token_usage = self._track_usage(usage_data, data.get("model", self.model))
                    
                    content = response_data.get("choices", [{}])[0].get("message", {}).get("content")
                    
                    return APIResponse(
                        success=True,
                        content=content,
                        token_usage=token_usage,
                        response_time=response_time,
                        status_code=200
                    )
                else:
                    error_msg = f"API Error {response.status_code}: {response.text}"
                    logger.error(error_msg)
                    
                    if response.status_code == 429:  # Rate limited
                        time.sleep(2 ** attempt)  # Exponential backoff
                        continue
                    elif response.status_code >= 500:  # Server error
                        if attempt < self.max_retries - 1:
                            time.sleep(2 ** attempt)
                            continue
                    
                    return APIResponse(
                        success=False,
                        error_message=error_msg,
                        response_time=response_time,
                        status_code=response.status_code
                    )
                    
            except requests.exceptions.Timeout:
                logger.error(f"Request timeout (attempt {attempt + 1}/{self.max_retries})")
                if attempt < self.max_retries - 1:
                    time.sleep(2 ** attempt)
                    continue
                return APIResponse(
                    success=False,
                    error_message="Request timeout",
                    response_time=time.time() - start_time
                )
                
            except Exception as e:
                logger.error(f"Request error (attempt {attempt + 1}/{self.max_retries}): {e}")
                if attempt < self.max_retries - 1:
                    time.sleep(2 ** attempt)
                    continue
                return APIResponse(
                    success=False,
                    error_message=str(e),
                    response_time=time.time() - start_time
                )
        
        return APIResponse(
            success=False,
            error_message="Max retries exceeded",
            response_time=time.time() - start_time
        )

    def chat_completion(
        self,
        messages: List[Dict[str, str]],
        model: Optional[str] = None,
        max_tokens: Optional[int] = None,
        temperature: Optional[float] = None,
        stream: bool = False
    ) -> APIResponse:
        """
        Send chat completion request to Kimi API

        Args:
            messages: List of conversation messages [{"role": "user", "content": "..."}]
            model: Model to use (defaults to initialized model)
            max_tokens: Maximum tokens to generate
            temperature: Sampling temperature (0.0 to 1.0)
            stream: Whether to stream response (not implemented yet)

        Returns:
            APIResponse with generated content and metadata
        """
        if not messages:
            return APIResponse(success=False, error_message="Messages cannot be empty")

        data = {
            "model": model or self.model,
            "messages": messages,
            "max_tokens": max_tokens or self.max_tokens,
            "temperature": temperature or self.temperature,
            "stream": stream
        }

        return self._make_request("/chat/completions", data)

    def analyze_code(
        self,
        code: str,
        analysis_type: str = "general",
        context: Optional[str] = None
    ) -> APIResponse:
        """
        Analyze code with Kimi AI

        Args:
            code: Code to analyze
            analysis_type: Type of analysis (general, security, performance, bugs)
            context: Additional context for analysis

        Returns:
            APIResponse with analysis results
        """
        prompt = f"""Please analyze the following code for {analysis_type} issues:

```python
{code}
```

Context: {context or "No additional context provided"}

Please provide:
1. Summary of findings
2. Specific issues identified
3. Recommendations for improvements
4. Security concerns (if any)
5. Performance considerations (if any)
"""

        messages = [
            {"role": "system", "content": "You are an expert code analyst. Provide detailed, actionable feedback on code quality, security, and performance."},
            {"role": "user", "content": prompt}
        ]

        return self.chat_completion(messages)

    def generate_task_plan(
        self,
        goal: str,
        context: Optional[str] = None,
        constraints: Optional[List[str]] = None
    ) -> APIResponse:
        """
        Generate a detailed task plan using Kimi AI

        Args:
            goal: Development goal to achieve
            context: Additional context about the project
            constraints: List of constraints or requirements

        Returns:
            APIResponse with generated task plan
        """
        constraints_str = "\n".join(f"- {c}" for c in constraints) if constraints else "No specific constraints"
        
        prompt = f"""Generate a detailed task plan for the following development goal:

GOAL: {goal}

CONTEXT: {context or "No additional context provided"}

CONSTRAINTS:
{constraints_str}

Please provide:
1. Task breakdown with clear steps
2. Dependencies between tasks
3. Estimated complexity for each task
4. Potential risks and mitigation strategies
5. Success criteria for each task
6. Recommended order of execution

Format the response as a structured plan that can be parsed programmatically.
"""

        messages = [
            {"role": "system", "content": "You are an expert software architect and project planner. Create detailed, actionable task plans for development projects."},
            {"role": "user", "content": prompt}
        ]

        return self.chat_completion(messages)

    def get_usage_report(self) -> Dict[str, Any]:
        """Get comprehensive usage report"""
        if not self.usage_history:
            return {"message": "No usage data available"}

        total_tokens = sum(u.total_tokens for u in self.usage_history)
        total_cost = sum(u.estimated_cost for u in self.usage_history)
        avg_tokens_per_request = total_tokens / len(self.usage_history) if self.usage_history else 0
        
        return {
            "total_requests": len(self.usage_history),
            "total_tokens": total_tokens,
            "total_cost": total_cost,
            "average_tokens_per_request": round(avg_tokens_per_request, 2),
            "recent_usage": [u.to_dict() for u in self.usage_history[-10:]],  # Last 10 requests
            "cost_tier": self._get_cost_tier(total_cost)
        }

    def _get_cost_tier(self, cost: float) -> str:
        """Get cost tier for alerting"""
        if cost < 1.0:
            return "LOW"
        elif cost < 10.0:
            return "MEDIUM"
        elif cost < 50.0:
            return "HIGH"
        else:
            return "CRITICAL"

    def reset_usage_tracking(self):
        """Reset usage tracking (useful for testing)"""
        self.usage_history.clear()
        self.total_cost = 0.0
        self.request_times.clear()
        logger.info("Usage tracking reset")

    def close(self):
        """Close the client and cleanup resources"""
        self.session.close()
        logger.info("Kimi API client closed")


# ============================================================================
# TESTING AND USAGE EXAMPLES
# ============================================================================

def test_kimi_client():
    """Test the Kimi API client"""
    try:
        # Initialize client
        client = KimiAPIClient()
        
        # Test simple chat
        print("Testing simple chat...")
        messages = [
            {"role": "user", "content": "Hello! Can you help me with software development tasks?"}
        ]
        
        response = client.chat_completion(messages)
        
        if response.success:
            print(f"✓ Chat successful: {response.content[:100]}...")
            print(f"  Tokens used: {response.token_usage.total_tokens}")
            print(f"  Cost: ${response.token_usage.estimated_cost:.4f}")
            print(f"  Response time: {response.response_time:.2f}s")
        else:
            print(f"✗ Chat failed: {response.error_message}")
            return False
        
        # Test code analysis
        print("\nTesting code analysis...")
        sample_code = """
def calculate_total(items):
    total = 0
    for item in items:
        total += item
    return total
"""
        
        response = client.analyze_code(sample_code, analysis_type="performance")
        
        if response.success:
            print("✓ Code analysis successful")
            print(f"  Analysis: {response.content[:150]}...")
        else:
            print(f"✗ Code analysis failed: {response.error_message}")
        
        # Get usage report
        print("\nUsage Report:")
        report = client.get_usage_report()
        print(json.dumps(report, indent=2))
        
        client.close()
        return True
        
    except Exception as e:
        print(f"✗ Client initialization failed: {e}")
        return False


if __name__ == "__main__":
    print("Kimi API Client Test")
    print("=" * 50)
    
    success = test_kimi_client()
    
    print("\n" + "=" * 50)
    if success:
        print("✓ All tests passed!")
    else:
        print("✗ Some tests failed")
        exit(1)