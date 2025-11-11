#!/usr/bin/env python3
"""
Debug script to verify API key reading and formatting
"""

import os
from dotenv import load_dotenv

# Load environment variables
load_dotenv()

# Get API key
api_key = os.getenv("KIMI_API_KEY")
print("API Key Debug Information")
print("=" * 50)
print(f"Raw API key: '{api_key}'")
print(f"Key length: {len(api_key) if api_key else 0}")
print(f"Key starts with 'sk-': {api_key.startswith('sk-') if api_key else False}")
print(f"Key contains whitespace: {any(c.isspace() for c in api_key) if api_key else False}")
print(f"Key has leading/trailing spaces: '{api_key}' != '{api_key.strip()}'" if api_key else "No key")

if api_key:
    stripped_key = api_key.strip()
    print(f"\nStripped key: '{stripped_key}'")
    print(f"Stripped key length: {len(stripped_key)}")
    
    # Test the header format
    auth_header = f"Bearer {stripped_key}"
    print(f"\nAuthorization header: '{auth_header[:20]}...{auth_header[-20:]}'")
    print(f"Header length: {len(auth_header)}")