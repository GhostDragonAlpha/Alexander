#!/usr/bin/env python3
"""
Direct API test to verify authentication
"""

import os
import requests
import json
from dotenv import load_dotenv

# Load environment variables
load_dotenv()

# Get API key
api_key = os.getenv("KIMI_API_KEY")
base_url = os.getenv("KIMI_API_BASE_URL", "https://api.moonshot.cn/v1")

print("Direct API Test")
print("=" * 50)
print(f"Base URL: {base_url}")
print(f"API Key: {api_key[:20]}...{api_key[-10:]}" if api_key else "No API key found")

if not api_key:
    print("ERROR: No API key found in environment")
    exit(1)

# Test the API directly
url = f"{base_url}/chat/completions"
headers = {
    "Authorization": f"Bearer {api_key.strip()}",
    "Content-Type": "application/json"
}

data = {
    "model": "moonshot-v1-8k",
    "messages": [
        {"role": "user", "content": "Hello, test connection"}
    ],
    "max_tokens": 100,
    "temperature": 0.7
}

print(f"\nMaking request to: {url}")
print(f"Headers: {json.dumps({k: v[:30] + '...' if k == 'Authorization' else v for k, v in headers.items()}, indent=2)}")
print(f"Data: {json.dumps(data, indent=2)}")

try:
    response = requests.post(url, headers=headers, json=data, timeout=30)
    print(f"\nResponse status: {response.status_code}")
    print(f"Response headers: {dict(response.headers)}")
    
    if response.status_code == 200:
        result = response.json()
        print(f"SUCCESS: {json.dumps(result, indent=2)}")
    else:
        print(f"ERROR: {response.text}")
        
except Exception as e:
    print(f"Request failed: {e}")
    import traceback
    traceback.print_exc()