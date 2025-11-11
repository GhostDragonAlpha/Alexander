# Autonomous Playtesting System - Configuration Guide

**Version**: 1.0.0  
**Last Updated**: 2025-11-11

## Table of Contents
1. [Configuration Overview](#configuration-overview)
2. [Main Configuration File](#main-configuration-file)
3. [Desktop Launcher Configuration](#desktop-launcher-configuration)
4. [Screenshot Capture Configuration](#screenshot-capture-configuration)
5. [Test Scenarios Configuration](#test-scenarios-configuration)
6. [Performance Monitoring Configuration](#performance-monitoring-configuration)
7. [HTTP API Server Configuration](#http-api-server-configuration)
8. [Logging Configuration](#logging-configuration)
9. [Data Management Configuration](#data-management-configuration)
10. [Analysis Server Configuration](#analysis-server-configuration)
11. [Example Configurations](#example-configurations)
12. [Configuration Best Practices](#configuration-best-practices)

## Configuration Overview

The Autonomous Playtesting System uses JSON-based configuration files to control all aspects of its behavior. The main configuration file is `playtesting_config.json`, with additional specialized configuration files for specific components.

### Configuration Files Structure

```
Project Root/
├── playtesting_config.json          # Main configuration file
├── visual_analysis_config.json      # Analysis server settings
└── automation_config/
    └── screenshot_config.json       # Screenshot capture settings
```

### Configuration Loading Order

1. Default configuration values are loaded
2. Main configuration file (`playtesting_config.json`) is loaded
3. Component-specific configurations are loaded
4. Environment variables override file settings
5. Command-line arguments override everything

## Main Configuration File

### File: `playtesting_config.json`

The main configuration file controls all major system components and behaviors.

```json
{
  "desktop_launcher": { ... },
  "screenshot_capture": { ... },
  "test_scenarios": { ... },
  "game_state_extractor": { ... },
  "performance_monitoring": { ... },
  "http_api_server": { ... },
  "logging": { ... },
  "integration": { ... },
  "data_management": { ... }
}
```

### Complete Example

```json
{
  "desktop_launcher": {
    "default_resolution": "1280x720",
    "default_quality": "medium",
    "default_port": 8080,
    "max_restarts": 3,
    "timeout_seconds": 0,
    "enable_console": true,
    "enable_debug": true,
    "process_check_interval": 10,
    "http_api_timeout": 180
  },
  "screenshot_capture": {
    "capture_interval": 3.0,
    "jpeg_quality": 85,
    "max_screenshots": 1000,
    "auto_cleanup": true,
    "enable_compression": true,
    "metadata_embedding": true,
    "enable_ai_transmission": true,
    "server_endpoint": "/receive_screenshot",
    "resize_to": null,
    "capture_region": null
  },
  "test_scenarios": {
    "default_timeout": 300,
    "max_attempts": 3,
    "required_screenshots": 5,
    "scenario_configs": {
      "first_mission": {
        "timeout": 180,
        "success_criteria": {
          "mission_completed": true,
          "min_distance_traveled": 1000.0,
          "max_time": 120
        },
        "failure_criteria": {
          "ship_destroyed": true,
          "timeout": true,
          "stuck_duration": 30
        }
      }
    }
  },
  "game_state_extractor": {
    "state_update_interval": 0.1,
    "nearby_actor_detection_radius": 5000.0,
    "track_performance": true,
    "extract_player_state": true,
    "extract_world_state": true,
    "extract_performance_metrics": true,
    "extract_inventory_state": true,
    "extract_mission_state": true,
    "extract_nearby_actors": true,
    "extract_ui_state": true
  },
  "performance_monitoring": {
    "track_fps": true,
    "track_memory": true,
    "track_hardware": true,
    "fps_warning_threshold": 30,
    "memory_warning_threshold_mb": 4096,
    "log_performance_interval": 60
  },
  "http_api_server": {
    "listen_port": 8080,
    "max_requests_per_second": 60,
    "enable_verbose_logging": true,
    "enable_rate_limiting": true,
    "cors_enabled": true,
    "allowed_origins": ["*"]
  },
  "logging": {
    "log_level": "INFO",
    "log_to_file": true,
    "log_to_console": true,
    "max_log_file_size_mb": 100,
    "max_log_files": 10,
    "log_directory": "automation_config/logs"
  },
  "integration": {
    "game_mode": "/Script/Alexander.AutomationGameMode",
    "default_map": "VRTemplateMap",
    "unreal_editor_path": "C:\\Program Files\\Epic Games\\UE_5.6\\Engine\\Binaries\\Win64\\UnrealEditor.exe",
    "project_file": "C:\\Users\\allen\\Desktop\\Alexander\\Alexander\\Alexander.uproject"
  },
  "data_management": {
    "data_retention_days": 30,
    "compress_old_data": true,
    "cleanup_interval_hours": 24
  }
}
```

## Desktop Launcher Configuration

Controls game launching and process monitoring.

### Configuration Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `default_resolution` | string | "1280x720" | Game window resolution (WIDTHxHEIGHT) |
| `default_quality` | string | "medium" | Graphics quality preset |
| `default_port` | integer | 8080 | HTTP API port |
| `max_restarts` | integer | 3 | Maximum automatic restarts on crash |
| `timeout_seconds` | integer | 0 | Session timeout (0 = no timeout) |
| `enable_console` | boolean | true | Show game console window |
| `enable_debug` | boolean | true | Enable debug logging |
| `process_check_interval` | integer | 10 | Process health check interval (seconds) |
| `http_api_timeout` | integer | 180 | HTTP API readiness timeout (seconds) |

### Quality Settings

The `default_quality` setting affects multiple graphics settings:

- **low**: Reduced shadows, textures, effects (50% screen percentage)
- **medium**: Balanced settings (75% screen percentage)
- **high**: High quality settings (100% screen percentage)
- **epic**: Maximum quality settings (100% screen percentage)

### Example Configurations

**Fast Testing Configuration**:
```json
{
  "desktop_launcher": {
    "default_resolution": "1024x768",
    "default_quality": "low",
    "enable_console": false,
    "enable_debug": false
  }
}
```

**High Quality Testing Configuration**:
```json
{
  "desktop_launcher": {
    "default_resolution": "1920x1080",
    "default_quality": "high",
    "enable_console": true,
    "enable_debug": true
  }
}
```

**Overnight Stress Test Configuration**:
```json
{
  "desktop_launcher": {
    "default_resolution": "1280x720",
    "default_quality": "medium",
    "timeout_seconds": 28800,
    "max_restarts": 5
  }
}
```

## Screenshot Capture Configuration

Controls screenshot frequency, quality, and storage.

### Configuration Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `capture_interval` | float | 3.0 | Seconds between screenshots |
| `jpeg_quality` | integer | 85 | JPEG quality (1-100) |
| `max_screenshots` | integer | 1000 | Maximum screenshots to keep |
| `auto_cleanup` | boolean | true | Automatically delete old screenshots |
| `enable_compression` | boolean | true | Compress images before storage |
| `metadata_embedding` | boolean | true | Embed metadata in storage |
| `enable_ai_transmission` | boolean | true | Send screenshots to AI analysis |
| `server_endpoint` | string | "/screenshot" | API endpoint for screenshot submission |
| `resize_to` | array | null | Resize to [width, height] or null |
| `capture_region` | array | null | Capture region [x, y, width, height] or null |

### Performance Impact

**Capture Interval**:
- **1-2 seconds**: High detail, high storage usage, performance impact
- **3-5 seconds**: Balanced detail and performance (recommended)
- **10+ seconds**: Lower detail, minimal performance impact

**JPEG Quality**:
- **95-100**: Excellent quality, large file size
- **85-94**: Good quality, moderate file size (recommended)
- **70-84**: Acceptable quality, smaller file size
- **50-69**: Lower quality, significant compression

### Example Configurations

**High Frequency Capture**:
```json
{
  "screenshot_capture": {
    "capture_interval": 1.0,
    "jpeg_quality": 95,
    "max_screenshots": 5000,
    "enable_compression": false
  }
}
```

**Low Frequency Capture**:
```json
{
  "screenshot_capture": {
    "capture_interval": 10.0,
    "jpeg_quality": 75,
    "max_screenshots": 100,
    "auto_cleanup": true
  }
}
```

**Region-Specific Capture**:
```json
{
  "screenshot_capture": {
    "capture_interval": 3.0,
    "capture_region": [100, 100, 800, 600],
    "resize_to": [400, 300]
  }
}
```

## Test Scenarios Configuration

Defines test scenarios, success criteria, and failure conditions.

### Configuration Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `default_timeout` | integer | 300 | Default test timeout (seconds) |
| `max_attempts` | integer | 3 | Maximum retry attempts |
| `required_screenshots` | integer | 5 | Screenshots needed for validation |
| `scenario_configs` | object | {} | Individual scenario configurations |

### Scenario Configuration Structure

```json
{
  "scenario_name": {
    "timeout": 180,
    "randomize": false,
    "success_criteria": {
      "criterion_name": value
    },
    "failure_criteria": {
      "criterion_name": true
    }
  }
}
```

### Success Criteria Options

- `mission_completed`: boolean
- `min_distance_traveled`: float (units)
- `max_time`: float (seconds)
- `trade_completed`: boolean
- `credits_earned`: integer
- `items_traded`: integer
- `enemies_defeated`: integer
- `player_survived`: boolean
- `min_distance_explored`: float (units)
- `discoveries_made`: integer
- `waypoints_visited`: integer

### Failure Criteria Options

- `ship_destroyed`: boolean
- `timeout`: boolean
- `stuck_duration`: float (seconds)
- `insufficient_credits`: boolean
- `trade_rejected`: boolean
- `player_destroyed`: boolean
- `ammo_depleted`: boolean
- `fuel_depleted`: boolean

### Example Configurations

**First Mission Scenario**:
```json
{
  "test_scenarios": {
    "scenario_configs": {
      "first_mission": {
        "timeout": 180,
        "success_criteria": {
          "mission_completed": true,
          "min_distance_traveled": 1000.0,
          "max_time": 120
        },
        "failure_criteria": {
          "ship_destroyed": true,
          "timeout": true,
          "stuck_duration": 30
        }
      }
    }
  }
}
```

**Combat Scenario**:
```json
{
  "test_scenarios": {
    "scenario_configs": {
      "combat_scenario": {
        "timeout": 300,
        "randomize": true,
        "success_criteria": {
          "enemies_defeated": 3,
          "player_survived": true,
          "combat_duration": 60
        },
        "failure_criteria": {
          "player_destroyed": true,
          "ammo_depleted": true,
          "timeout": true
        }
      }
    }
  }
}
```

**Exploration Scenario**:
```json
{
  "test_scenarios": {
    "scenario_configs": {
      "free_exploration": {
        "timeout": 600,
        "randomize": true,
        "success_criteria": {
          "min_distance_explored": 5000.0,
          "discoveries_made": 3,
          "waypoints_visited": 5
        },
        "failure_criteria": {
          "ship_destroyed": true,
          "fuel_depleted": true,
          "timeout": true
        }
      }
    }
  }
}
```

## Performance Monitoring Configuration

Controls performance tracking and alerting.

### Configuration Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `track_fps` | boolean | true | Track frames per second |
| `track_memory` | boolean | true | Track memory usage |
| `track_hardware` | boolean | true | Track CPU/GPU usage |
| `fps_warning_threshold` | integer | 30 | FPS warning threshold |
| `memory_warning_threshold_mb` | integer | 4096 | Memory warning threshold (MB) |
| `log_performance_interval` | integer | 60 | Performance logging interval (seconds) |

### Threshold Guidelines

**FPS Thresholds**:
- **VR Games**: 90 FPS target, 45 FPS warning
- **Desktop Games**: 60 FPS target, 30 FPS warning
- **Mobile Games**: 30 FPS target, 20 FPS warning

**Memory Thresholds**:
- **8GB System**: 4096 MB warning (50% of total)
- **16GB System**: 8192 MB warning (50% of total)
- **32GB System**: 12288 MB warning (38% of total)

### Example Configurations

**Aggressive Monitoring**:
```json
{
  "performance_monitoring": {
    "track_fps": true,
    "track_memory": true,
    "track_hardware": true,
    "fps_warning_threshold": 45,
    "memory_warning_threshold_mb": 3072,
    "log_performance_interval": 30
  }
}
```

**Minimal Monitoring**:
```json
{
  "performance_monitoring": {
    "track_fps": true,
    "track_memory": false,
    "track_hardware": false,
    "fps_warning_threshold": 30,
    "log_performance_interval": 300
  }
}
```

## HTTP API Server Configuration

Configures the game HTTP API server.

### Configuration Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `listen_port` | integer | 8080 | HTTP API port |
| `max_requests_per_second` | integer | 60 | Rate limit |
| `enable_verbose_logging` | boolean | true | Enable detailed logging |
| `enable_rate_limiting` | boolean | true | Enable rate limiting |
| `cors_enabled` | boolean | true | Enable CORS |
| `allowed_origins` | array | ["*"] | Allowed CORS origins |

### Security Considerations

**Development**:
```json
{
  "http_api_server": {
    "cors_enabled": true,
    "allowed_origins": ["*"],
    "enable_verbose_logging": true
  }
}
```

**Production**:
```json
{
  "http_api_server": {
    "cors_enabled": true,
    "allowed_origins": ["http://localhost:8082"],
    "enable_verbose_logging": false,
    "max_requests_per_second": 30
  }
}
```

## Logging Configuration

Controls system logging behavior.

### Configuration Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `log_level` | string | "INFO" | Logging level |
| `log_to_file` | boolean | true | Write logs to file |
| `log_to_console` | boolean | true | Write logs to console |
| `max_log_file_size_mb` | integer | 100 | Maximum log file size (MB) |
| `max_log_files` | integer | 10 | Maximum number of log files |
| `log_directory` | string | "automation_config/logs" | Log directory path |

### Log Levels

- **DEBUG**: Detailed debugging information
- **INFO**: General information (recommended)
- **WARNING**: Warning messages
- **ERROR**: Error messages only
- **CRITICAL**: Critical errors only

### Example Configurations

**Development Logging**:
```json
{
  "logging": {
    "log_level": "DEBUG",
    "log_to_file": true,
    "log_to_console": true,
    "max_log_file_size_mb": 50,
    "max_log_files": 20
  }
}
```

**Production Logging**:
```json
{
  "logging": {
    "log_level": "INFO",
    "log_to_file": true,
    "log_to_console": false,
    "max_log_file_size_mb": 100,
    "max_log_files": 5
  }
}
```

## Data Management Configuration

Controls data retention and storage.

### Configuration Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `data_retention_days` | integer | 30 | Days to retain data |
| `compress_old_data` | boolean | true | Compress old data |
| `cleanup_interval_hours` | integer | 24 | Cleanup interval (hours) |

### Storage Planning

**Daily Data Generation**:
- Screenshots: ~20 MB per hour (at 3s interval, 85% quality)
- Performance data: ~5 MB per hour
- Logs: ~1 MB per hour
- **Total**: ~26 MB per hour, ~624 MB per day

**Monthly Storage Requirements**:
- With 30-day retention: ~18 GB
- With compression: ~12 GB (33% reduction)

### Example Configurations

**Short-term Testing**:
```json
{
  "data_management": {
    "data_retention_days": 7,
    "compress_old_data": false,
    "cleanup_interval_hours": 12
  }
}
```

**Long-term Testing**:
```json
{
  "data_management": {
    "data_retention_days": 90,
    "compress_old_data": true,
    "cleanup_interval_hours": 24
  }
}
```

## Analysis Server Configuration

Controls visual analysis and AI processing.

### File: `visual_analysis_config.json`

```json
{
  "analysis_server": {
    "host": "localhost",
    "port": 8081,
    "max_file_size": 10485760,
    "timeout": 2.0
  },
  "visual_analysis": {
    "enable_ui_detection": true,
    "enable_artifact_detection": true,
    "confidence_threshold": 0.7
  },
  "text_recognition": {
    "enable_ocr": true,
    "languages": ["eng"],
    "confidence_threshold": 0.6
  },
  "object_detection": {
    "enable_detection": true,
    "model_path": "models/object_detector.tflite",
    "confidence_threshold": 0.5
  },
  "issue_classification": {
    "enable_classification": true,
    "severity_threshold": "medium",
    "min_frequency_for_pattern": 3
  }
}
```

### Analysis Server Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `host` | string | "localhost" | Server host |
| `port` | integer | 8081 | Server port |
| `max_file_size` | integer | 10485760 | Maximum file size (bytes) |
| `timeout` | float | 2.0 | Request timeout (seconds) |

### Visual Analysis Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `enable_ui_detection` | boolean | true | Enable UI element detection |
| `enable_artifact_detection` | boolean | true | Enable visual artifact detection |
| `confidence_threshold` | float | 0.7 | Detection confidence threshold |

### Text Recognition Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `enable_ocr` | boolean | true | Enable OCR text recognition |
| `languages` | array | ["eng"] | OCR languages |
| `confidence_threshold` | float | 0.6 | Text recognition confidence |

### Object Detection Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `enable_detection` | boolean | true | Enable object detection |
| `model_path` | string | "models/object_detector.tflite" | Model file path |
| `confidence_threshold` | float | 0.5 | Detection confidence threshold |

### Issue Classification Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `enable_classification` | boolean | true | Enable AI issue classification |
| `severity_threshold` | string | "medium" | Minimum severity to report |
| `min_frequency_for_pattern` | integer | 3 | Minimum occurrences for pattern detection |

## Example Configurations

### Development Configuration

**Fast iteration, detailed logging**:

```json
{
  "desktop_launcher": {
    "default_resolution": "1024x768",
    "default_quality": "low",
    "enable_console": true,
    "enable_debug": true
  },
  "screenshot_capture": {
    "capture_interval": 5.0,
    "jpeg_quality": 75,
    "max_screenshots": 100
  },
  "logging": {
    "log_level": "DEBUG",
    "log_to_console": true,
    "max_log_file_size_mb": 10
  },
  "performance_monitoring": {
    "fps_warning_threshold": 30,
    "log_performance_interval": 30
  }
}
```

### Production Configuration

**Comprehensive testing, long-term storage**:

```json
{
  "desktop_launcher": {
    "default_resolution": "1920x1080",
    "default_quality": "high",
    "max_restarts": 5,
    "timeout_seconds": 0
  },
  "screenshot_capture": {
    "capture_interval": 3.0,
    "jpeg_quality": 85,
    "max_screenshots": 10000,
    "enable_ai_transmission": true
  },
  "test_scenarios": {
    "default_timeout": 600,
    "max_attempts": 3,
    "scenario_configs": {
      "comprehensive_test": {
        "timeout": 1800,
        "randomize": true
      }
    }
  },
  "performance_monitoring": {
    "track_hardware": true,
    "fps_warning_threshold": 45,
    "memory_warning_threshold_mb": 6144
  },
  "data_management": {
    "data_retention_days": 90,
    "compress_old_data": true
  },
  "logging": {
    "log_level": "INFO",
    "log_to_console": false,
    "max_log_file_size_mb": 100,
    "max_log_files": 10
  }
}
```

### Overnight Testing Configuration

**Unattended operation, automatic recovery**:

```json
{
  "desktop_launcher": {
    "default_resolution": "1280x720",
    "default_quality": "medium",
    "max_restarts": 10,
    "timeout_seconds": 28800,
    "enable_console": false,
    "enable_debug": false
  },
  "screenshot_capture": {
    "capture_interval": 10.0,
    "jpeg_quality": 80,
    "max_screenshots": 5000,
    "auto_cleanup": true
  },
  "test_scenarios": {
    "default_timeout": 1800,
    "max_attempts": 5,
    "scenario_configs": {
      "overnight_stress_test": {
        "timeout": 7200,
        "randomize": true,
        "success_criteria": {
          "min_distance_explored": 10000,
          "discoveries_made": 10
        }
      }
    }
  },
  "performance_monitoring": {
    "fps_warning_threshold": 30,
    "memory_warning_threshold_mb": 4096,
    "log_performance_interval": 300
  },
  "data_management": {
    "data_retention_days": 30,
    "cleanup_interval_hours": 6
  },
  "logging": {
    "log_level": "WARNING",
    "log_to_console": false
  }
}
```

### Performance Optimization Configuration

**Minimal overhead, focused testing**:

```json
{
  "desktop_launcher": {
    "default_resolution": "800x600",
    "default_quality": "low",
    "enable_console": false,
    "enable_debug": false
  },
  "screenshot_capture": {
    "capture_interval": 15.0,
    "jpeg_quality": 70,
    "max_screenshots": 100,
    "enable_ai_transmission": false,
    "resize_to": [400, 300]
  },
  "test_scenarios": {
    "default_timeout": 120,
    "max_attempts": 2
  },
  "performance_monitoring": {
    "track_hardware": false,
    "log_performance_interval": 120
  },
  "logging": {
    "log_level": "ERROR",
    "log_to_file": false
  }
}
```

## Configuration Best Practices

### 1. Start Simple
Begin with default configuration and adjust based on needs:
```json
{
  "desktop_launcher": {
    "default_resolution": "1280x720",
    "default_quality": "medium"
  },
  "screenshot_capture": {
    "capture_interval": 3.0,
    "jpeg_quality": 85
  }
}
```

### 2. Document Changes
Keep a configuration changelog:
```markdown
# Configuration Changes
- 2024-01-15: Increased capture_interval from 3s to 5s (disk space)
- 2024-01-20: Lowered jpeg_quality from 85 to 75 (performance)
- 2024-01-25: Added custom combat scenario
```

### 3. Use Environment-Specific Configs
Create separate configs for different environments:
- `playtesting_config_dev.json`
- `playtesting_config_prod.json`
- `playtesting_config_overnight.json`

### 4. Monitor Configuration Impact
Track how changes affect:
- Disk usage
- System performance
- Test effectiveness
- Issue detection rate

### 5. Regular Review
Review and update configuration:
- Monthly for active projects
- After major system changes
- When adding new test scenarios
- Based on storage capacity changes

### 6. Backup Configurations
Keep backups of working configurations:
```bash
# Version control configurations
git add playtesting_config.json
git commit -m "Update: Lowered capture interval for detailed analysis"

# Keep historical configs
cp playtesting_config.json configs/config_2024_01_15.json
```

### 7. Test Configuration Changes
Validate configuration changes:
```bash
# Test new configuration
python continuous_playtesting.py --config new_config.json --iterations 5

# Monitor resource usage
# Check log files for errors
# Verify screenshot quality
# Confirm issue detection still works
```

---

**Next Steps**: See [PLAYTESTING_INTEGRATION.md](PLAYTESTING_INTEGRATION.md) for CI/CD integration examples.