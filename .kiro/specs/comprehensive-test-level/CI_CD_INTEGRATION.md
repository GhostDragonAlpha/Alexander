# Comprehensive Test Level - CI/CD Integration Guide

## Overview

This guide provides instructions for integrating the Comprehensive Test Level with GitHub Actions and other CI/CD systems. Automated test execution ensures code quality before merge and tracks performance regressions.

## GitHub Actions Workflow

### Setup: Create Workflow File

Create `.github/workflows/test-comprehensive-level.yml`:

```yaml
name: Comprehensive Test Level

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main, develop ]
  schedule:
    # Run daily at 2 AM UTC
    - cron: '0 2 * * *'

jobs:
  comprehensive-tests:
    runs-on: windows-latest

    strategy:
      matrix:
        unreal-version: ['5.6']

    steps:
      - name: Checkout Code
        uses: actions/checkout@v3
        with:
          fetch-depth: 0  # Full history for regression detection

      - name: Setup Python
        uses: actions/setup-python@v4
        with:
          python-version: '3.11'
          cache: 'pip'

      - name: Install Python Dependencies
        run: |
          pip install --upgrade pip
          pip install -r requirements-test.txt

      - name: Build Project
        run: |
          & "C:\Program Files\Epic Games\UE_${{ matrix.unreal-version }}\Engine\Binaries\Win64\UnrealBuildTool.exe" `
            -ProjectFiles `
            -Project="${{ github.workspace }}\Alexander.uproject" `
            -Game `
            -Engine `
            -VS2022

          dotnet build `
            -c Development `
            "${{ github.workspace }}\Alexander.sln"

      - name: Run Comprehensive Tests
        run: |
          python run_comprehensive_test_level.py `
            --report-format all `
            --output-dir ./test-results `
            --timeout 1800 `
            --check-regression

        continue-on-error: true

      - name: Parse Test Results
        id: parse-results
        run: |
          python scripts/parse_test_results.py `
            --input ./test-results/test_report.json `
            --output test-summary.txt

      - name: Generate Test Report Summary
        run: |
          python scripts/generate_test_summary.py `
            --results ./test-results/test_report.json `
            --baseline ./test-baselines/baseline.json `
            --output SUMMARY.md

      - name: Upload Test Results
        uses: actions/upload-artifact@v3
        if: always()
        with:
          name: test-results
          path: test-results/
          retention-days: 30

      - name: Upload Test Report
        uses: actions/upload-artifact@v3
        if: always()
        with:
          name: test-report
          path: |
            test-results/test_report.html
            SUMMARY.md

      - name: Check Test Results
        id: check-results
        run: |
          python scripts/check_test_status.py `
            --results ./test-results/test_report.json `
            --fail-on-regression

      - name: Comment PR with Results
        if: github.event_name == 'pull_request'
        uses: actions/github-script@v6
        with:
          script: |
            const fs = require('fs');
            const summary = fs.readFileSync('SUMMARY.md', 'utf8');
            github.rest.issues.createComment({
              issue_number: context.issue.number,
              owner: context.repo.owner,
              repo: context.repo.repo,
              body: summary
            });

      - name: Notify Slack on Failure
        if: failure()
        uses: slackapi/slack-github-action@v1
        with:
          webhook-url: ${{ secrets.SLACK_WEBHOOK }}
          payload: |
            {
              "text": "Comprehensive Test Level Failed",
              "blocks": [
                {
                  "type": "section",
                  "text": {
                    "type": "mrkdwn",
                    "text": "Test run failed in ${{ github.repository }}\n${{ github.server_url }}/${{ github.repository }}/actions/runs/${{ github.run_id }}"
                  }
                }
              ]
            }

  performance-regression-check:
    needs: comprehensive-tests
    runs-on: windows-latest

    steps:
      - name: Checkout Code
        uses: actions/checkout@v3

      - name: Setup Python
        uses: actions/setup-python@v4
        with:
          python-version: '3.11'

      - name: Download Test Results
        uses: actions/download-artifact@v3
        with:
          name: test-results
          path: test-results/

      - name: Analyze Performance
        run: |
          python scripts/analyze_performance.py `
            --current ./test-results/test_report.json `
            --baseline ./test-baselines/baseline.json `
            --regression-threshold 15 `
            --output performance-analysis.json

      - name: Generate Performance Report
        if: always()
        run: |
          python scripts/generate_performance_report.py `
            --analysis ./performance-analysis.json `
            --output PERFORMANCE.md

      - name: Comment PR with Performance Data
        if: github.event_name == 'pull_request'
        uses: actions/github-script@v6
        with:
          script: |
            const fs = require('fs');
            const perfReport = fs.readFileSync('PERFORMANCE.md', 'utf8');
            github.rest.issues.createComment({
              issue_number: context.issue.number,
              owner: context.repo.owner,
              repo: context.repo.repo,
              body: perfReport
            });

  test-report-dashboard:
    needs: comprehensive-tests
    runs-on: windows-latest
    if: github.event_name == 'push' && github.ref == 'refs/heads/main'

    steps:
      - name: Download Test Results
        uses: actions/download-artifact@v3
        with:
          name: test-results
          path: test-results/

      - name: Update Dashboard
        run: |
          python scripts/update_test_dashboard.py `
            --results ./test-results/test_report.json `
            --dashboard-path ./docs/test-dashboard.html

      - name: Push Updates
        run: |
          git config --global user.name "CI Bot"
          git config --global user.email "ci@alexander.local"
          git add docs/test-dashboard.html
          git commit -m "Update test dashboard - ${{ github.run_number }}"
          git push
```

### Helper Scripts

Create `scripts/parse_test_results.py`:

```python
#!/usr/bin/env python3
"""Parse test results and generate summary."""

import json
import argparse
from pathlib import Path
from datetime import datetime

def parse_results(results_file: Path) -> dict:
    """Parse test results JSON file."""
    with open(results_file, 'r') as f:
        return json.load(f)

def generate_summary(results: dict) -> str:
    """Generate text summary."""
    summary = []
    summary.append("# Test Execution Summary")
    summary.append(f"Generated: {datetime.now().isoformat()}")
    summary.append("")

    summary.append("## Overview")
    summary.append(f"Total Stations: {results['total_stations']}")
    summary.append(f"Passed: {results['passed_stations']}")
    summary.append(f"Failed: {results['failed_stations']}")
    summary.append(f"Total Tests: {results['total_tests']}")
    summary.append(f"Pass Rate: {(results['passed_tests'] / results['total_tests'] * 100):.1f}%")
    summary.append("")

    if results['station_results']:
        summary.append("## Station Results")
        for station in results['station_results']:
            status = "PASS" if station['status'] == 'PASS' else 'FAIL'
            summary.append(f"- **{station['station_name']}** [{status}] "
                         f"({station['passed_tests']}/{station['total_tests']})")

    if results['errors']:
        summary.append("")
        summary.append("## Errors")
        for error in results['errors']:
            summary.append(f"- {error}")

    return "\n".join(summary)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--input', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()

    results = parse_results(args.input)
    summary = generate_summary(results)

    with open(args.output, 'w') as f:
        f.write(summary)

    print(f"Summary written to {args.output}")

if __name__ == "__main__":
    main()
```

Create `scripts/check_test_status.py`:

```python
#!/usr/bin/env python3
"""Check test results and exit with appropriate code."""

import json
import argparse
import sys
from pathlib import Path

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--results', required=True, type=Path)
    parser.add_argument('--fail-on-regression', action='store_true')
    args = parser.parse_args()

    with open(args.results, 'r') as f:
        results = json.load(f)

    # Exit codes:
    # 0 = All pass
    # 1 = Some failures
    # 2 = Performance regression

    exit_code = 0

    if results['failed_tests'] > 0 or results['failed_stations'] > 0:
        print(f"FAILED: {results['failed_tests']} test(s) failed")
        exit_code = 1

    if args.fail_on_regression and results.get('performance_regression_detected'):
        print("REGRESSION: Performance regression detected")
        exit_code = 2

    if exit_code == 0:
        print(f"SUCCESS: All {results['total_tests']} tests passed")

    sys.exit(exit_code)

if __name__ == "__main__":
    main()
```

### Requirements File

Create `requirements-test.txt`:

```
pytest==7.4.0
requests==2.31.0
```

## Gitlab CI Integration

### Create `.gitlab-ci.yml`:

```yaml
stages:
  - build
  - test
  - report
  - deploy

variables:
  UE_VERSION: "5.6"
  ARTIFACT_RETENTION: "30 days"

comprehensive_tests:
  stage: test
  image: windows-server-core:ltsc2022

  script:
    - powershell -Command "$ProgressPreference='SilentlyContinue'; [System.Net.ServicePointManager]::SecurityProtocol = 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))"
    - choco install -y python
    - python -m pip install --upgrade pip
    - python -m pip install -r requirements-test.txt
    - python run_comprehensive_test_level.py --report-format all --output-dir test-results --timeout 1800 --check-regression

  artifacts:
    paths:
      - test-results/
    reports:
      junit: test-results/test_report.xml
    expire_in: $ARTIFACT_RETENTION

  retry:
    max: 1
    when: runner_system_failure

test_analysis:
  stage: report
  image: python:3.11

  dependencies:
    - comprehensive_tests

  script:
    - pip install -r requirements-test.txt
    - python scripts/check_test_status.py --results test-results/test_report.json --fail-on-regression
    - python scripts/analyze_performance.py --current test-results/test_report.json --baseline test-baselines/baseline.json

  allow_failure: true

publish_results:
  stage: report
  image: alpine:latest

  dependencies:
    - comprehensive_tests

  script:
    - mkdir -p public
    - cp -r test-results/* public/
    - ls -la public/

  artifacts:
    paths:
      - public
    expire_in: $ARTIFACT_RETENTION

  only:
    - main
    - develop
```

## Local Pre-Commit Hook

Create `.git/hooks/pre-commit`:

```bash
#!/bin/bash

# Run comprehensive tests before committing
echo "Running comprehensive tests..."

python run_comprehensive_test_level.py \
    --timeout 300 \
    --report-format json \
    --output-dir ./tmp-test-results

RESULT=$?

# Check results
if [ $RESULT -ne 0 ]; then
    echo "Tests failed! Commit aborted."
    echo "Run tests locally with: python run_comprehensive_test_level.py"
    exit 1
fi

echo "All tests passed. Proceeding with commit."
exit 0
```

Make it executable:

```bash
chmod +x .git/hooks/pre-commit
```

## Jenkins Pipeline

### Create `Jenkinsfile`:

```groovy
pipeline {
    agent {
        node {
            label 'windows'
        }
    }

    options {
        buildDiscarder(logRotator(numToKeepStr: '30', daysToKeepStr: '7'))
        timeout(time: 2, unit: 'HOURS')
        timestamps()
    }

    triggers {
        pollSCM('H H * * *')  // Daily
        githubPush()
    }

    environment {
        PYTHONUNBUFFERED = '1'
        TEST_RESULTS = "${WORKSPACE}/test-results"
    }

    stages {
        stage('Prepare') {
            steps {
                cleanWs()
                checkout scm

                script {
                    sh '''
                        python --version
                        pip install --upgrade pip
                        pip install -r requirements-test.txt
                    '''
                }
            }
        }

        stage('Tests') {
            steps {
                script {
                    def status = sh(
                        script: '''
                            python run_comprehensive_test_level.py \
                                --report-format all \
                                --output-dir "${TEST_RESULTS}" \
                                --timeout 1800 \
                                --check-regression
                        ''',
                        returnStatus: true
                    )

                    if (status != 0) {
                        echo "Test execution had issues (exit code: ${status})"
                    }
                }
            }
        }

        stage('Parse Results') {
            steps {
                script {
                    sh '''
                        python scripts/parse_test_results.py \
                            --input "${TEST_RESULTS}/test_report.json" \
                            --output test-summary.txt

                        cat test-summary.txt
                    '''
                }
            }
        }

        stage('Archive') {
            steps {
                archiveArtifacts artifacts: 'test-results/**', allowEmptyArchive: true
                junit testResults: 'test-results/test_report.xml', allowEmptyResults: true

                publishHTML([
                    reportDir: 'test-results',
                    reportFiles: 'test_report.html',
                    reportName: 'Test Report'
                ])
            }
        }
    }

    post {
        always {
            cleanWs(deleteDirs: true, patterns: [[pattern: 'test-results', type: 'INCLUDE']])
        }

        failure {
            emailext(
                subject: "Test Pipeline Failed: ${env.JOB_NAME} #${env.BUILD_NUMBER}",
                body: "Check console output at ${env.BUILD_URL}",
                to: "${env.CHANGE_AUTHOR_EMAIL}"
            )
        }

        unstable {
            slackSend(
                color: 'warning',
                message: "Unstable: ${env.JOB_NAME} #${env.BUILD_NUMBER}"
            )
        }
    }
}
```

## Azure Pipelines

### Create `azure-pipelines.yml`:

```yaml
trigger:
  - main
  - develop

pr:
  - main
  - develop

schedules:
  - cron: "0 2 * * *"
    displayName: Daily Test Run
    branches:
      include:
        - main
        - develop

pool:
  vmImage: 'windows-2022'

variables:
  pythonVersion: '3.11'
  unreal_version: '5.6'

jobs:
  - job: ComprehensiveTests
    displayName: 'Run Comprehensive Tests'

    steps:
      - task: UsePythonVersion@0
        inputs:
          versionSpec: '$(pythonVersion)'
        displayName: 'Use Python $(pythonVersion)'

      - task: PipAuthenticate@1
        inputs:
          artifactFeeds: 'internal'

      - script: |
          python -m pip install --upgrade pip
          pip install -r requirements-test.txt
        displayName: 'Install Dependencies'

      - script: |
          python run_comprehensive_test_level.py `
            --report-format all `
            --output-dir $(Build.ArtifactStagingDirectory)/test-results `
            --timeout 1800 `
            --check-regression
        displayName: 'Run Comprehensive Tests'
        continueOnError: true

      - script: |
          python scripts/check_test_status.py `
            --results $(Build.ArtifactStagingDirectory)/test-results/test_report.json
        displayName: 'Check Test Results'

      - task: PublishBuildArtifacts@1
        inputs:
          pathToPublish: '$(Build.ArtifactStagingDirectory)/test-results'
          artifactName: 'test-results'
          publishLocation: 'Container'
        condition: always()

      - task: PublishTestResults@2
        inputs:
          testResultsFormat: 'JUnit'
          testResultsFiles: '**/test_report.xml'
          failTaskOnFailedTests: true
        condition: always()

      - task: PublishPipelineArtifact@1
        inputs:
          targetPath: '$(Build.ArtifactStagingDirectory)/test-results'
          artifact: 'TestReports'
        condition: always()
```

## Performance Baseline Management

### Initialize Baseline

```bash
# Run tests once and save as baseline
python run_comprehensive_test_level.py --output-dir ./test-baselines

# Move results to baseline file
mv test-results/test_report.json test-baselines/baseline.json
```

### Update Baseline

```bash
# After performance-improving changes
python scripts/update_baseline.py \
    --current ./test-results/test_report.json \
    --baseline ./test-baselines/baseline.json \
    --reason "Optimization: Improved shader compilation"
```

### Detect Regressions

```bash
# Automatic regression detection in test runner
python run_comprehensive_test_level.py --check-regression
```

## Slack Integration

### Setup Slack Webhook

1. Go to Slack App Directory
2. Search for "Incoming Webhooks"
3. Create new webhook for #ci-testing channel
4. Copy webhook URL

### Add to GitHub Secrets

```
Name: SLACK_WEBHOOK
Value: https://hooks.slack.com/services/YOUR/WEBHOOK/URL
```

### Custom Slack Messages

Create `scripts/slack_notification.py`:

```python
#!/usr/bin/env python3
"""Send test results to Slack."""

import json
import requests
import argparse
from pathlib import Path

def format_slack_message(results: dict, webhook_url: str) -> None:
    """Send formatted test results to Slack."""

    pass_rate = (results['passed_tests'] / results['total_tests'] * 100) \
        if results['total_tests'] > 0 else 0

    color = 'good' if pass_rate == 100 else 'warning' if pass_rate >= 90 else 'danger'

    message = {
        "color": color,
        "title": "Comprehensive Test Results",
        "text": f"Pass Rate: {pass_rate:.1f}%",
        "fields": [
            {
                "title": "Total Tests",
                "value": str(results['total_tests']),
                "short": True
            },
            {
                "title": "Passed",
                "value": str(results['passed_tests']),
                "short": True
            },
            {
                "title": "Failed",
                "value": str(results['failed_tests']),
                "short": True
            },
            {
                "title": "Duration",
                "value": f"{results['total_duration']:.2f}s",
                "short": True
            }
        ]
    }

    requests.post(webhook_url, json={"attachments": [message]})

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--results', required=True, type=Path)
    parser.add_argument('--webhook', required=True)
    args = parser.parse_args()

    with open(args.results, 'r') as f:
        results = json.load(f)

    format_slack_message(results, args.webhook)

if __name__ == "__main__":
    main()
```

## Test Result Dashboard

### HTML Dashboard Template

Create `docs/test-dashboard.html`:

```html
<!DOCTYPE html>
<html>
<head>
    <title>Test Dashboard</title>
    <style>
        body { font-family: Arial; margin: 20px; background: #f5f5f5; }
        .header { background: #667eea; color: white; padding: 20px; border-radius: 8px; }
        .metric { display: inline-block; margin: 10px; background: white; padding: 15px;
                  border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        .metric-value { font-size: 2em; font-weight: bold; color: #667eea; }
        .pass { color: #28a745; }
        .fail { color: #dc3545; }
        .trend { margin-top: 20px; }
        .chart { width: 100%; height: 300px; }
    </style>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
</head>
<body>
    <div class="header">
        <h1>Comprehensive Test Level Dashboard</h1>
        <p>Real-time test execution metrics</p>
    </div>

    <div id="metrics"></div>
    <div class="trend">
        <h2>Pass Rate Trend (Last 30 Days)</h2>
        <canvas id="trendChart" class="chart"></canvas>
    </div>

    <script>
        // Load test data from latest run
        // Update metrics and chart
        // Auto-refresh every 5 minutes
    </script>
</body>
</html>
```

## Troubleshooting CI/CD

### Tests Timeout in CI

**Solution**: Increase timeout in workflow:

```yaml
- name: Run Comprehensive Tests
  run: python run_comprehensive_test_level.py --timeout 3600  # 1 hour
```

### Python Not Found

**Solution**: Use proper path or install first:

```yaml
- name: Setup Python
  uses: actions/setup-python@v4
  with:
    python-version: '3.11'
```

### Unreal Editor License Issue

**Solution**: Ensure build environment has valid Unreal Engine installation:

```yaml
- name: Setup Unreal Engine
  run: |
    # Download and setup Unreal Engine
    # Or use pre-built image with UE installed
```

### Performance Regression False Positives

**Solution**: Adjust regression threshold or exclude specific metrics:

```python
# In check for regressions, use percentage tolerance
REGRESSION_THRESHOLD = 0.15  # 15% tolerance
```

## Best Practices

1. **Run tests on every commit** - Catch regressions early
2. **Maintain baseline metrics** - Update after intentional optimizations
3. **Archive test results** - Keep historical data for trend analysis
4. **Monitor trends** - Watch for gradual performance degradation
5. **Notify on failures** - Use Slack/email alerts for failures
6. **Review performance regularly** - Weekly performance reports
7. **Version control test scripts** - Track test runner changes

## Next Steps

1. Choose your CI/CD platform (GitHub Actions recommended)
2. Create workflow/pipeline file
3. Add secrets for notifications
4. Run first test and validate
5. Set up result dashboard
6. Configure notifications
