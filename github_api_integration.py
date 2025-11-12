#!/usr/bin/env python3
"""
GitHub API Integration for Repository Management
Handles repository access, branch management, and pull request creation
"""

import os
import json
import time
import logging
import base64
from typing import Dict, List, Optional, Any, Tuple
from dataclasses import dataclass, field
from enum import Enum
from datetime import datetime
import requests
from dotenv import load_dotenv

# Load environment variables
load_dotenv()

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


class GitHubOperationType(Enum):
    """Types of GitHub operations"""
    READ = "read"
    WRITE = "write"
    BRANCH = "branch"
    PULL_REQUEST = "pull_request"
    COMMIT = "commit"


class SafetyLevel(Enum):
    """Safety classification for operations"""
    SAFE = "safe"  # Read-only operations
    LOW_RISK = "low_risk"  # Local changes
    MEDIUM_RISK = "medium_risk"  # Branch operations
    HIGH_RISK = "high_risk"  # Push to remote
    CRITICAL = "critical"  # Merge to main, delete branches


@dataclass
class FileChange:
    """Represents a file change"""
    file_path: str
    content: str
    encoding: str = "utf-8"
    sha: Optional[str] = None  # Required for updates
    
    def to_dict(self) -> Dict[str, Any]:
        result = {
            "path": self.file_path,
            "content": self.content,
            "encoding": self.encoding
        }
        if self.sha:
            result["sha"] = self.sha
        return result


@dataclass
class CommitInfo:
    """Represents commit information"""
    message: str
    author: str
    email: str
    files: List[FileChange]
    parent_sha: Optional[str] = None
    
    def to_dict(self) -> Dict[str, Any]:
        return {
            "message": self.message,
            "author": self.author,
            "email": self.email,
            "files": [f.to_dict() for f in self.files],
            "parent_sha": self.parent_sha
        }


@dataclass
class PullRequestInfo:
    """Represents pull request information"""
    title: str
    body: str
    head_branch: str
    base_branch: str
    draft: bool = False
    
    def to_dict(self) -> Dict[str, Any]:
        return {
            "title": self.title,
            "body": self.body,
            "head": self.head_branch,
            "base": self.base_branch,
            "draft": self.draft
        }


class GitHubAPIClient:
    """
    GitHub API Client for repository management
    Handles authentication, rate limiting, and safe operations
    """
    
    # GitHub API configuration
    API_BASE_URL = "https://api.github.com"
    REQUEST_TIMEOUT = 30
    MAX_RETRIES = 3
    
    def __init__(self, token: Optional[str] = None, repo_url: Optional[str] = None):
        """
        Initialize GitHub API client
        
        Args:
            token: GitHub personal access token
            repo_url: Repository URL (e.g., https://github.com/user/repo)
        """
        self.token = token or os.getenv("GITHUB_TOKEN")
        self.repo_url = repo_url or os.getenv("GITHUB_REPO_URL", "https://github.com/GhostDragonAlpha/Alexander.git")
        
        if not self.token:
            raise ValueError("GitHub token not provided. Set GITHUB_TOKEN environment variable.")
        
        if not self.repo_url:
            raise ValueError("Repository URL not provided.")
        
        # Parse repository owner and name from URL
        self.repo_owner, self.repo_name = self._parse_repo_url(self.repo_url)
        
        # Session for connection pooling
        self.session = requests.Session()
        self.session.headers.update({
            "Authorization": f"token {self.token}",
            "Accept": "application/vnd.github.v3+json",
            "User-Agent": "AI-Automation-Client"
        })
        
        # Rate limiting
        self.rate_limit_remaining = None
        self.rate_limit_reset = None
        
        logger.info(f"GitHub API client initialized for {self.repo_owner}/{self.repo_name}")
    
    def _parse_repo_url(self, url: str) -> Tuple[str, str]:
        """
        Parse repository owner and name from URL
        
        Args:
            url: Repository URL
            
        Returns:
            Tuple of (owner, repo_name)
        """
        # Remove .git extension if present
        if url.endswith(".git"):
            url = url[:-4]
        
        # Parse URL
        parts = url.strip("/").split("/")
        if len(parts) >= 2:
            return parts[-2], parts[-1]
        else:
            raise ValueError(f"Invalid repository URL: {url}")
    
    def _check_rate_limit(self, response: requests.Response):
        """Update rate limit information from response"""
        self.rate_limit_remaining = int(response.headers.get("X-RateLimit-Remaining", 0))
        self.rate_limit_reset = int(response.headers.get("X-RateLimit-Reset", 0))
        
        if self.rate_limit_remaining and self.rate_limit_remaining < 10:
            logger.warning(f"Rate limit low: {self.rate_limit_remaining} requests remaining")
    
    def _make_request(self, method: str, endpoint: str, **kwargs) -> Dict[str, Any]:
        """
        Make authenticated request to GitHub API
        
        Args:
            method: HTTP method
            endpoint: API endpoint
            **kwargs: Additional request parameters
            
        Returns:
            API response data
        """
        url = f"{self.API_BASE_URL}{endpoint}"
        
        for attempt in range(self.MAX_RETRIES):
            try:
                response = self.session.request(method, url, timeout=self.REQUEST_TIMEOUT, **kwargs)
                self._check_rate_limit(response)
                
                if response.status_code in [200, 201, 204]:
                    if response.status_code == 204:
                        return {"success": True}
                    return response.json()
                elif response.status_code == 404:
                    raise Exception(f"Resource not found: {endpoint}")
                elif response.status_code == 401:
                    raise Exception("Authentication failed - invalid token")
                elif response.status_code == 403:
                    raise Exception(f"Rate limit exceeded or access denied: {response.text}")
                elif response.status_code >= 500:
                    if attempt < self.MAX_RETRIES - 1:
                        time.sleep(2 ** attempt)
                        continue
                    raise Exception(f"GitHub API error: {response.status_code} - {response.text}")
                else:
                    raise Exception(f"Unexpected status code: {response.status_code} - {response.text}")
                    
            except requests.exceptions.Timeout:
                if attempt < self.MAX_RETRIES - 1:
                    time.sleep(2 ** attempt)
                    continue
                raise Exception("Request timeout")
            except Exception as e:
                if attempt < self.MAX_RETRIES - 1:
                    time.sleep(2 ** attempt)
                    continue
                raise e
    
    def get_repository_info(self) -> Dict[str, Any]:
        """
        Get repository information
        
        Returns:
            Repository data
        """
        endpoint = f"/repos/{self.repo_owner}/{self.repo_name}"
        return self._make_request("GET", endpoint)
    
    def get_branches(self) -> List[Dict[str, Any]]:
        """
        Get all branches in repository
        
        Returns:
            List of branch data
        """
        endpoint = f"/repos/{self.repo_owner}/{self.repo_name}/branches"
        return self._make_request("GET", endpoint)
    
    def get_branch(self, branch_name: str) -> Dict[str, Any]:
        """
        Get specific branch information
        
        Args:
            branch_name: Branch name
            
        Returns:
            Branch data
        """
        endpoint = f"/repos/{self.repo_owner}/{self.repo_name}/branches/{branch_name}"
        return self._make_request("GET", endpoint)
    
    def create_branch(self, branch_name: str, from_branch: str = "main") -> Dict[str, Any]:
        """
        Create a new branch
        
        Args:
            branch_name: New branch name
            from_branch: Branch to create from
            
        Returns:
            Branch creation result
        """
        # Get reference to base branch
        endpoint = f"/repos/{self.repo_owner}/{self.repo_name}/git/refs/heads/{from_branch}"
        base_ref = self._make_request("GET", endpoint)
        
        # Create new branch
        endpoint = f"/repos/{self.repo_owner}/{self.repo_name}/git/refs"
        data = {
            "ref": f"refs/heads/{branch_name}",
            "sha": base_ref["object"]["sha"]
        }
        
        return self._make_request("POST", endpoint, json=data)
    
    def get_file_content(self, file_path: str, branch: str = "main") -> Optional[str]:
        """
        Get file content from repository
        
        Args:
            file_path: File path in repository
            branch: Branch name
            
        Returns:
            File content as string, or None if not found
        """
        endpoint = f"/repos/{self.repo_owner}/{self.repo_name}/contents/{file_path}"
        params = {"ref": branch}
        
        try:
            response = self._make_request("GET", endpoint, params=params)
            
            if isinstance(response, dict) and "content" in response:
                content = response["content"]
                encoding = response.get("encoding", "base64")
                
                if encoding == "base64":
                    return base64.b64decode(content).decode("utf-8")
                else:
                    return content
            
            return None
            
        except Exception as e:
            if "Resource not found" in str(e):
                return None
            raise
    
    def create_file(self, file_path: str, content: str, message: str, 
                    branch: str = "main") -> Dict[str, Any]:
        """
        Create a new file in repository
        
        Args:
            file_path: File path to create
            content: File content
            message: Commit message
            branch: Branch name
            
        Returns:
            File creation result
        """
        endpoint = f"/repos/{self.repo_owner}/{self.repo_name}/contents/{file_path}"
        
        data = {
            "message": message,
            "content": base64.b64encode(content.encode("utf-8")).decode("utf-8"),
            "branch": branch
        }
        
        return self._make_request("PUT", endpoint, json=data)
    
    def update_file(self, file_path: str, content: str, message: str, 
                    branch: str = "main") -> Dict[str, Any]:
        """
        Update an existing file in repository
        
        Args:
            file_path: File path to update
            content: New file content
            message: Commit message
            branch: Branch name
            
        Returns:
            File update result
        """
        # Get current file to get SHA
        endpoint = f"/repos/{self.repo_owner}/{self.repo_name}/contents/{file_path}"
        params = {"ref": branch}
        
        current_file = self._make_request("GET", endpoint, params=params)
        
        if not isinstance(current_file, dict) or "sha" not in current_file:
            raise Exception(f"File not found: {file_path}")
        
        # Update file
        data = {
            "message": message,
            "content": base64.b64encode(content.encode("utf-8")).decode("utf-8"),
            "sha": current_file["sha"],
            "branch": branch
        }
        
        return self._make_request("PUT", endpoint, json=data)
    
    def delete_file(self, file_path: str, message: str, branch: str = "main") -> Dict[str, Any]:
        """
        Delete a file from repository
        
        Args:
            file_path: File path to delete
            message: Commit message
            branch: Branch name
            
        Returns:
            File deletion result
        """
        # Get current file to get SHA
        endpoint = f"/repos/{self.repo_owner}/{self.repo_name}/contents/{file_path}"
        params = {"ref": branch}
        
        current_file = self._make_request("GET", endpoint, params=params)
        
        if not isinstance(current_file, dict) or "sha" not in current_file:
            raise Exception(f"File not found: {file_path}")
        
        # Delete file
        data = {
            "message": message,
            "sha": current_file["sha"],
            "branch": branch
        }
        
        return self._make_request("DELETE", endpoint, json=data)
    
    def create_pull_request(self, pr_info: PullRequestInfo) -> Dict[str, Any]:
        """
        Create a pull request
        
        Args:
            pr_info: Pull request information
            
        Returns:
            Pull request data
        """
        endpoint = f"/repos/{self.repo_owner}/{self.repo_name}/pulls"
        return self._make_request("POST", endpoint, json=pr_info.to_dict())
    
    def get_pull_requests(self, state: str = "open") -> List[Dict[str, Any]]:
        """
        Get pull requests
        
        Args:
            state: PR state (open, closed, all)
            
        Returns:
            List of pull requests
        """
        endpoint = f"/repos/{self.repo_owner}/{self.repo_name}/pulls"
        params = {"state": state}
        return self._make_request("GET", endpoint, params=params)
    
    def get_commit_history(self, branch: str = "main", limit: int = 10) -> List[Dict[str, Any]]:
        """
        Get commit history for a branch
        
        Args:
            branch: Branch name
            limit: Number of commits to retrieve
            
        Returns:
            List of commits
        """
        endpoint = f"/repos/{self.repo_owner}/{self.repo_name}/commits"
        params = {"sha": branch, "per_page": limit}
        return self._make_request("GET", endpoint, params=params)
    
    def check_safety_level(self, operation: GitHubOperationType, 
                          file_path: Optional[str] = None) -> SafetyLevel:
        """
        Determine safety level for an operation
        
        Args:
            operation: Type of operation
            file_path: File path (for file-specific safety)
            
        Returns:
            Safety level
        """
        if operation == GitHubOperationType.READ:
            return SafetyLevel.SAFE
        
        elif operation == GitHubOperationType.WRITE:
            if file_path:
                # Check file extensions and paths
                critical_extensions = [".uproject", ".dll", ".exe", ".bat", ".ps1"]
                critical_paths = ["Config/", "Source/", "Build/"]
                
                if any(file_path.endswith(ext) for ext in critical_extensions):
                    return SafetyLevel.HIGH_RISK
                
                if any(file_path.startswith(path) for path in critical_paths):
                    return SafetyLevel.MEDIUM_RISK
            
            return SafetyLevel.LOW_RISK
        
        elif operation == GitHubOperationType.BRANCH:
            return SafetyLevel.MEDIUM_RISK
        
        elif operation == GitHubOperationType.PULL_REQUEST:
            return SafetyLevel.MEDIUM_RISK
        
        elif operation == GitHubOperationType.COMMIT:
            return SafetyLevel.LOW_RISK
        
        return SafetyLevel.MEDIUM_RISK
    
    def requires_approval(self, safety_level: SafetyLevel) -> bool:
        """
        Check if operation requires manual approval
        
        Args:
            safety_level: Safety level of operation
            
        Returns:
            True if approval required
        """
        return safety_level in [SafetyLevel.HIGH_RISK, SafetyLevel.CRITICAL]
    
    def create_ai_development_branch(self, goal_description: str) -> Tuple[str, str]:
        """
        Create a branch for AI development work
        
        Args:
            goal_description: Description of development goal
            
        Returns:
            Tuple of (branch_name, branch_url)
        """
        # Generate branch name from goal
        import re
        safe_name = re.sub(r'[^a-zA-Z0-9]+', '-', goal_description.lower())[:50]
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        branch_name = f"ai/{safe_name}_{timestamp}"
        
        # Create branch from develop
        self.create_branch(branch_name, from_branch="develop")
        
        branch_url = f"{self.repo_url.rstrip('.git')}/tree/{branch_name}"
        
        logger.info(f"Created AI development branch: {branch_name}")
        return branch_name, branch_url
    
    def close(self):
        """Close the GitHub API client session"""
        if self.session:
            self.session.close()
            logger.info("GitHub API client session closed")


def test_github_api_integration():
    """Test the GitHub API integration"""
    print("Testing GitHub API Integration")
    print("=" * 50)
    
    try:
        # Initialize client
        client = GitHubAPIClient()
        
        # Test 1: Repository info
        print("\n1. Testing repository info...")
        try:
            repo_info = client.get_repository_info()
            print(f"+ Repository info retrieved")
            print(f"  Name: {repo_info.get('name', 'Unknown')}")
            print(f"  Description: {repo_info.get('description', 'No description')}")
            print(f"  Stars: {repo_info.get('stargazers_count', 0)}")
        except Exception as e:
            print(f"- Repository info failed: {e}")
        
        # Test 2: Branch listing
        print("\n2. Testing branch listing...")
        try:
            branches = client.get_branches()
            print(f"+ Branches retrieved: {len(branches)} branches")
            for branch in branches[:5]:  # Show first 5
                print(f"  - {branch['name']}")
            if len(branches) > 5:
                print(f"  ... and {len(branches) - 5} more")
        except Exception as e:
            print(f"- Branch listing failed: {e}")
        
        # Test 3: Safety level checking
        print("\n3. Testing safety level checking...")
        safety_read = client.check_safety_level(GitHubOperationType.READ)
        safety_write = client.check_safety_level(GitHubOperationType.WRITE, "README.md")
        safety_write_critical = client.check_safety_level(GitHubOperationType.WRITE, "Config/DefaultEngine.ini")
        
        print(f"+ Safety levels:")
        print(f"  Read operation: {safety_read.value}")
        print(f"  Write README.md: {safety_write.value}")
        print(f"  Write Config file: {safety_write_critical.value}")
        
        # Test 4: Approval requirements
        print("\n4. Testing approval requirements...")
        needs_approval_safe = client.requires_approval(safety_read)
        needs_approval_critical = client.requires_approval(safety_write_critical)
        
        print(f"+ Approval requirements:")
        print(f"  Read operation: {'Yes' if needs_approval_safe else 'No'}")
        print(f"  Config file write: {'Yes' if needs_approval_critical else 'No'}")
        
        # Cleanup
        client.close()
        
        print("\n" + "=" * 50)
        print("+ GitHub API Integration test completed successfully")
        print("+ Note: Full functionality requires valid GitHub token with repo access")
        return True
        
    except Exception as e:
        print(f"- GitHub API Integration test failed: {e}")
        print("- Note: Requires GITHUB_TOKEN environment variable with repo access")
        return False


if __name__ == "__main__":
    success = test_github_api_integration()
    exit(0 if success else 1)