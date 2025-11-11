# Autonomous Playtesting System - User Manual

**Version**: 1.0.0  
**Last Updated**: 2025-11-11

## Table of Contents
1. [Getting Started](#getting-started)
2. [System Overview](#system-overview)
3. [Starting the System](#starting-the-system)
4. [Understanding the Dashboard](#understanding-the-dashboard)
5. [Test Scenarios](#test-scenarios)
6. [Interpreting Results](#interpreting-results)
7. [Performance Monitoring](#performance-monitoring)
8. [Issue Management](#issue-management)
9. [Manual vs Automated Testing](#manual-vs-automated-testing)
10. [Best Practices](#best-practices)
11. [Advanced Features](#advanced-features)

## Getting Started

### What is the Autonomous Playtesting System?

The Autonomous Playtesting System is a comprehensive automated testing framework that continuously tests your Unreal Engine game without human intervention. It captures screenshots, monitors performance, detects issues, and provides detailed reports - all while you focus on development.

### Key Benefits
- **24/7 Testing**: Continuous testing even when you're away
- **Visual Verification**: Automatically detects visual bugs and UI issues
- **Performance Monitoring**: Tracks FPS, memory, and frame times
- **Intelligent Issue Detection**: AI-powered bug detection and classification
- **Comprehensive Reporting**: Detailed dashboards and reports
- **GitHub Integration**: Automatic issue ticket creation

### Before You Begin

Ensure you have:
- Completed the installation from [PLAYTESTING_SETUP_GUIDE.md](PLAYTESTING_SETUP_GUIDE.md)
- Verified all components are working
- Created your configuration files
- Set up environment variables

## System Overview

### System Components

The system consists of several integrated components:

1. **ContinuousPlaytestingOrchestrator** - Main system coordinator
2. **GameLauncher** - Launches and monitors the game
3. **ScreenshotCapture** - Captures and analyzes screenshots
4. **AnalysisServer** - Visual verification and issue detection
5. **PerformanceCollector** - Performance metrics gathering
6. **IssueReporter** - Issue detection and reporting
7. **DataManager** - Data storage and management
8. **PlaytestingDashboard** - Web-based monitoring interface

### System Ports
- **Port 8080**: Game HTTP API
- **Port 8081**: Analysis API Server
- **Port 8082**: Playtesting Dashboard

### Data Flow
1. Game launches with AutomationGameMode
2. System captures screenshots every 3 seconds
3. Performance metrics collected every 5 seconds
4. Analysis server processes screenshots for issues
5. Issue reporter classifies and tracks problems
6. Dashboard displays real-time results
7. Data manager handles storage and cleanup

## Starting the System

### Method 1: Quick Start (Recommended for Beginners)

Run the main orchestrator:

```bash
python continuous_playtesting.py
```

This will:
- Launch the game automatically
- Start all components
- Run continuous testing
- Generate reports

**Options**:
```bash
# Limit to specific number of iterations
python continuous_playtesting.py --iterations 10

# Use custom configuration
python continuous_playtesting.py --config my_config.json

# Manual mode (launch game only, no automated testing)
python continuous_playtesting.py --manual
```

### Method 2: Component-by-Component Start (Advanced)

For more control, start components individually:

1. **Start Analysis Server** (Terminal 1):
   ```bash
   python analysis_server.py
   ```

2. **Start Dashboard** (Terminal 2):
   ```bash
   python playtesting_dashboard.py
   ```

3. **Run Continuous Testing** (Terminal 3):
   ```bash
   python continuous_playtesting.py
   ```

### Method 3: Using Batch Scripts

Use provided batch files for convenience:

```bash
# Launch automated testing
LAUNCH_AUTOMATED.bat

# Launch with specific configuration
launch_autonomous_testing.bat

# Launch continuous playtesting
launch_continuous_playtesting.bat
```

### Monitoring Startup

Watch for these indicators of successful startup:

1. **Game Window Opens**: Game launches in 1280x720 window
2. **HTTP API Ready**: "Game ready!" message appears
3. **Screenshot Capture**: "Screenshot captured" messages
4. **Dashboard Accessible**: Can open http://localhost:8082
5. **No Error Messages**: Clean log output

### Startup Troubleshooting

If the system fails to start:

1. **Check Ports**: Ensure ports 8080-8082 are available
2. **Verify Game**: Try launching game manually first
3. **Check Logs**: Review `automation_config/logs/` for errors
4. **Test Components**: Run individual components to isolate issues

## Understanding the Dashboard

### Accessing the Dashboard

Open your web browser and navigate to:
```
http://localhost:8082
```

### Dashboard Sections

#### 1. Header Section
- **Title**: "Alexander Comprehensive Test Dashboard"
- **Timestamp**: When the dashboard was generated
- **Auto-refresh**: Refresh interval (if enabled)
- **Status Banner**: Overall system status

**Status Indicators**:
- 🟢 **Green**: All systems nominal (95%+ pass rate)
- 🟡 **Yellow**: Some tests failed, review recommended (80-95% pass rate)
- 🔴 **Red**: Critical failures, immediate review required (<80% pass rate)

#### 2. Metrics Grid

**Key Performance Indicators**:

| Metric | Description | Good | Warning | Bad |
|--------|-------------|------|---------|-----|
| **Overall Pass Rate** | Percentage of tests passing | ≥95% | 80-95% | <80% |
| **Total Tests** | Number of tests executed | - | - | - |
| **Passed** | Successfully completed tests | High | Medium | Low |
| **Failed** | Tests that failed | 0 | 1-3 | >3 |
| **Average FPS** | Game performance | ≥90 | 60-90 | <60 |
| **Memory Usage** | RAM consumption | <3GB | 3-4GB | >4GB |

**Color Coding**:
- 🟢 **Green**: Good/Passing
- 🟡 **Yellow**: Warning/Needs Attention
- 🔴 **Red**: Critical/Failed

#### 3. Performance Trends Charts

**Pass Rate Trend**:
- Line chart showing pass rate over time
- Look for consistent high percentages
- Sudden drops indicate problems

**FPS Trend**:
- Line chart of average FPS
- Should be stable and above target (90+ FPS for VR)
- Spikes or drops need investigation

**Memory Usage Trend**:
- Line chart of memory consumption
- Should be stable, not continuously growing
- Growth indicates memory leaks

#### 4. Test Station Breakdown

**Station Information**:
- **Station Name**: Test scenario name
- **Pass Count**: Number of passed tests (green)
- **Fail Count**: Number of failed tests (red)
- **Total**: Total tests for this station

**Failed Stations**:
- Highlighted with red border
- Expand to see individual test failures
- Click for detailed error information

#### 5. Human Review Queue

**Items Requiring Attention**:
- Failed tests that need manual investigation
- Visual anomalies detected
- Performance regressions
- Gameplay logic issues

**Review Items Include**:
- Station and test name
- Error message
- Severity level
- Screenshot (if available)

### Dashboard Features

#### Auto-Refresh
Enable auto-refresh to see real-time updates:
```bash
python playtesting_dashboard.py --auto-refresh 30
# Refreshes every 30 seconds
```

#### Mobile-Friendly
Dashboard is responsive and works on mobile devices:
- Collapsible sections
- Touch-friendly interface
- Optimized for small screens

#### Export Data
Export dashboard data for further analysis:
- JSON format for programmatic access
- CSV format for spreadsheet analysis
- Screenshot export for documentation

## Test Scenarios

### Built-in Test Scenarios

The system includes several pre-configured test scenarios:

#### 1. First Mission Test
**Purpose**: Test the initial player experience
**Duration**: 3 minutes
**Success Criteria**:
- Mission completes successfully
- Player travels minimum distance
- Completes within time limit

**Failure Conditions**:
- Ship destroyed
- Timeout
- Player stuck for 30+ seconds

#### 2. Trading System Test
**Purpose**: Validate trading mechanics
**Duration**: 4 minutes
**Success Criteria**:
- Trade completed
- Credits earned
- Items traded

**Failure Conditions**:
- Insufficient credits
- Trade rejected
- Timeout

#### 3. Combat Scenario Test
**Purpose**: Test combat systems
**Duration**: 5 minutes
**Success Criteria**:
- Enemies defeated (3)
- Player survives
- Combat duration reasonable

**Failure Conditions**:
- Player destroyed
- Ammo depleted
- Timeout

#### 4. Free Exploration Test
**Purpose**: Test exploration mechanics
**Duration**: 10 minutes
**Success Criteria**:
- Minimum distance explored (5000 units)
- Discoveries made (3)
- Waypoints visited (5)

**Failure Conditions**:
- Ship destroyed
- Fuel depleted
- Timeout

### Custom Test Scenarios

Create custom scenarios in `playtesting_config.json`:

```json
{
  "test_scenarios": {
    "scenario_configs": {
      "my_custom_scenario": {
        "timeout": 300,
        "randomize": true,
        "success_criteria": {
          "custom_metric": true,
          "min_value": 100
        },
        "failure_criteria": {
          "timeout": true,
          "critical_error": true
        }
      }
    }
  }
}
```

### Running Specific Scenarios

```bash
# Run specific scenario
python continuous_playtesting.py --scenario first_mission

# Run multiple scenarios
python continuous_playtesting.py --scenario first_mission,combat_scenario
```

## Interpreting Results

### Test Result Statuses

**Passed** 🟢:
- Test completed successfully
- All success criteria met
- No critical issues detected

**Failed** 🔴:
- Test did not complete
- Success criteria not met
- Critical issues detected

**Error** 🟠:
- Test execution error
- Exception or crash occurred
- Needs investigation

**Timeout** ⏱️:
- Test exceeded time limit
- May indicate performance issues
- Could be stuck or frozen

### Performance Metrics

#### FPS (Frames Per Second)
- **Target**: 90+ FPS for VR, 60+ FPS for desktop
- **Good**: Stable, consistent frame rate
- **Warning**: Drops below target occasionally
- **Critical**: Consistently below 30 FPS

#### Memory Usage
- **Target**: < 4GB total usage
- **Good**: Stable, not growing over time
- **Warning**: Approaching 4GB limit
- **Critical**: Exceeds 4GB or growing continuously

#### Frame Time
- **Target**: < 16.6ms for 60 FPS, < 11.1ms for 90 FPS
- **Good**: Consistent, low variance
- **Warning**: Occasional spikes
- **Critical**: Frequent spikes above 33ms

### Issue Severity Levels

**Critical** 🔴:
- Game crashes or freezes
- Unplayable performance
- Data loss or corruption
- **Action**: Immediate investigation required

**High** 🟠:
- Significant performance issues
- Major functionality broken
- Frequent visual artifacts
- **Action**: Fix in next development cycle

**Medium** 🟡:
- Minor performance degradation
- Occasional visual glitches
- Non-critical functionality issues
- **Action**: Add to backlog

**Low** 🔵:
- Cosmetic issues
- Minor UI inconsistencies
- Rare edge cases
- **Action**: Fix when convenient

## Performance Monitoring

### Real-Time Monitoring

The system continuously monitors:

1. **FPS Tracking**:
   - Current FPS
   - Average FPS
   - Minimum/Maximum FPS
   - FPS stability

2. **Memory Tracking**:
   - Current memory usage
   - Memory growth rate
   - Peak memory usage
   - Memory leak detection

3. **Frame Time Analysis**:
   - Average frame time
   - Frame time variance
   - Spike detection
   - Bottleneck identification

4. **Hardware Metrics**:
   - CPU usage
   - GPU usage
   - Disk I/O
   - Network usage

### Performance Baselines

**Establishing Baselines**:
- First iteration establishes performance baseline
- Subsequent iterations compared against baseline
- Deviations trigger alerts
- Automatic regression detection

**Baseline Metrics**:
- Average FPS: [established value]
- Memory usage: [established value]
- Frame time: [established value]
- Draw calls: [established value]

### Performance Alerts

**Automatic Alerts**:
- FPS drops below threshold (30 FPS)
- Memory exceeds threshold (4GB)
- Frame time spikes detected
- Performance regression identified

**Alert Actions**:
- Log warning messages
- Create issue reports
- Update dashboard status
- Optionally pause testing

## Issue Management

### Issue Detection

The system automatically detects:

**Performance Issues**:
- Low FPS
- High memory usage
- Frame time spikes
- Resource leaks

**Visual Issues**:
- UI artifacts
- Text rendering problems
- Object detection failures
- Screen tearing

**Gameplay Issues**:
- Player stuck detection
- Mission progression blocked
- Logic errors
- State inconsistencies

### Issue Classification

**AI-Powered Classification**:
- Visual analysis of screenshots
- Performance data correlation
- Pattern recognition
- Severity assessment

**Classification Categories**:
- Performance
- Visual
- Gameplay
- System

### Issue Tracking

**Database Storage**:
- SQLite database for issue tracking
- Unique issue IDs
- Frequency counting
- Timeline tracking

**Issue Lifecycle**:
1. **Detected**: Issue identified
2. **Classified**: Severity and type determined
3. **Tracked**: Added to database
4. **Reported**: Dashboard and logs updated
5. **Resolved**: Fixed and verified
6. **Closed**: Removed from active issues

### GitHub Integration

**Automatic Ticket Creation**:
- Critical and high-severity issues
- Frequent occurrences (3+ times)
- Issues older than 1 hour
- Structured ticket format

**Ticket Information**:
- Issue description
- Severity level
- Frequency count
- Game state data
- Performance metrics
- Recommended fixes
- Screenshots (if available)

## Manual vs Automated Testing

### Automated Testing Mode

**When to Use**:
- Continuous integration
- Overnight testing
- Regression testing
- Performance monitoring
- Large-scale test scenarios

**Benefits**:
- Unattended operation
- Consistent test execution
- Comprehensive coverage
- Historical data collection
- Automatic issue detection

**Configuration**:
```bash
# Run automated mode
python continuous_playtesting.py

# With iteration limit
python continuous_playtesting.py --iterations 100
```

### Manual Testing Mode

**When to Use**:
- Debugging specific issues
- Testing new features
- Exploratory testing
- Quick validation
- Interactive testing

**Benefits**:
- Human observation
- Flexible test scenarios
- Immediate feedback
- Creative exploration
- Subjective assessment

**Configuration**:
```bash
# Run manual mode
python continuous_playtesting.py --manual

# Or launch game only
python launch_game_desktop.py
```

### Hybrid Approach

**Best of Both Worlds**:
- Use automated for regression testing
- Use manual for new feature validation
- Combine automated data with human insight
- Use automated monitoring during manual testing

## Best Practices

### System Configuration

**Optimal Settings**:
- Capture interval: 3-5 seconds (balance between detail and performance)
- Screenshot quality: 85% (good quality, reasonable file size)
- Max screenshots: 1000 (adjust based on disk space)
- Retention period: 30 days (adjust based on storage)

**Performance Optimization**:
- Use medium quality settings for game
- 1280x720 resolution for faster processing
- Disable unnecessary visual effects
- Close other applications during testing

### Test Scenario Design

**Effective Scenarios**:
- Keep scenarios focused and specific
- Set clear success/failure criteria
- Use appropriate timeouts
- Include randomization for variety
- Test edge cases and error conditions

**Scenario Duration**:
- Short scenarios: 2-5 minutes (quick tests)
- Medium scenarios: 5-15 minutes (feature tests)
- Long scenarios: 15+ minutes (stress tests)

### Data Management

**Storage Management**:
- Monitor disk space regularly
- Adjust retention policies as needed
- Archive important test sessions
- Export critical findings
- Clean up old data periodically

**Backup Strategy**:
- Backup critical findings daily
- Keep important test sessions
- Export data before cleanup
- Document significant issues

### Monitoring and Maintenance

**Daily Tasks**:
- Check dashboard for new issues
- Review performance trends
- Verify system is running
- Check disk space usage

**Weekly Tasks**:
- Review issue reports
- Analyze performance regressions
- Clean up old screenshots
- Archive completed sessions

**Monthly Tasks**:
- Comprehensive system review
- Performance baseline updates
- Configuration optimization
- Storage cleanup and archiving

## Advanced Features

### Custom Analysis Modules

**Creating Custom Analyzers**:
```python
from visual_analyzer import VisualAnalyzer

class CustomAnalyzer(VisualAnalyzer):
    def analyze_screenshot(self, image_path):
        # Custom analysis logic
        results = super().analyze_screenshot(image_path)
        # Add custom detection
        results['custom_metric'] = self.detect_custom_issues(image_path)
        return results
```

### Integration with CI/CD

**GitHub Actions Integration**:
```yaml
name: Playtesting
on: [push, pull_request]
jobs:
  test:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v2
      - name: Run Playtesting
        run: |
          python continuous_playtesting.py --iterations 50
      - name: Upload Results
        uses: actions/upload-artifact@v2
        with:
          name: test-results
          path: playtesting_data/
```

### Custom Issue Classification

**Extending Issue Detection**:
```python
from issue_classifier import IssueClassifier

class CustomIssueClassifier(IssueClassifier):
    def classify_issues(self, analysis_results):
        issues = super().classify_issues(analysis_results)
        # Add custom classification
        custom_issues = self.detect_custom_issues(analysis_results)
        issues.extend(custom_issues)
        return issues
```

### Performance Optimization

**Tuning for Large Projects**:
- Increase capture interval (5-10 seconds)
- Reduce screenshot quality (70-80%)
- Limit concurrent scenarios
- Increase cleanup frequency
- Use lower game quality settings

### Multi-Client Testing

**Testing with Multiple Game Instances**:
```bash
# Launch multiple clients
python launch_multi_client_consensus_test.py --clients 3

# Each client runs on different port
# Client 1: Port 8080
# Client 2: Port 8081
# Client 3: Port 8082
```

---

**Next Steps**: See [PLAYTESTING_API_REFERENCE.md](PLAYTESTING_API_REFERENCE.md) for detailed API documentation.