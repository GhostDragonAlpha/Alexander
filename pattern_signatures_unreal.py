"""
Unreal Engine-specific pattern signatures for the Fractal Debugging Framework.
Defines bug patterns unique to UE4/UE5 C++ development with their indicators,
resonance points, and intervention strategies.
"""

# Pattern signature database for Unreal Engine-specific bugs
SIGNATURES = {
    'null_cascade_ue': {
        'name': 'Null Pointer Cascade (UE)',
        'description': 'Chain reaction of null pointer dereferences caused by TWeakObjectPtr safety issues, missing includes, or uninitialized UPROPERTY members',
        'indicators': [
            'TWeakObjectPtr operator bool() deprecation',
            'missing include directive',
            'uninitialized UPROPERTY',
            'incomplete forward declaration',
            'dangling pointer access',
            'null reference in delegate',
            'stale object reference',
            'subsystem initialization order issue'
        ],
        'scales': {
            'micro': ['variable', 'line', 'expression'],
            'meso': ['function', 'method', 'class member'],
            'macro': ['component', 'subsystem', 'module'],
            'meta': ['initialization order', 'dependency graph', 'module coupling']
        },
        'resonance_points': [
            'pointer_safety',
            'include_management',
            'initialization_order',
            'forward_declaration',
            'object_lifecycle'
        ],
        'intervention_templates': {
            'pointer_safety': {
                'description': 'Convert TWeakObjectPtr operator bool() to IsValid() checks',
                'pattern': r'if\s*\(\s*(\w+)\s*\)',
                'replacement': r'if (\1.IsValid())',
                'cascade_prediction': {
                    'file_impact': 0.7,
                    'build_impact': 0.8,
                    'runtime_impact': 0.9
                }
            },
            'include_management': {
                'description': 'Add missing include directives for forward-declared types',
                'pattern': r'//\s*TODO:\s*Add include for (\w+)',
                'replacement': r'#include "\1.h"',
                'cascade_prediction': {
                    'file_impact': 0.9,
                    'build_impact': 0.6,
                    'runtime_impact': 0.4
                }
            },
            'initialization_order': {
                'description': 'Initialize UPROPERTY members in constructor',
                'pattern': r'(\w+)\s*\(\s*\)\s*:\s*Super\(\)',
                'replacement': r'\1() : Super()\n{\n    // Initialize default values\n}',
                'cascade_prediction': {
                    'file_impact': 0.5,
                    'build_impact': 0.7,
                    'runtime_impact': 0.8
                }
            }
        },
        'severity': 'critical',
        'frequency_weight': 0.85
    },
    
    'state_desync_ue': {
        'name': 'State Desynchronization (UE)',
        'description': 'State inconsistencies across subsystems due to initialization order, replication timing, or event propagation issues',
        'indicators': [
            'subsystem initialization order error',
            'replication mismatch',
            'event ordering violation',
            'stale cached state',
            'inconsistent actor state',
            'tick group dependency issue',
            'asynchronous state update race',
            'BeginPlay order violation'
        ],
        'scales': {
            'micro': ['variable state', 'cache invalidation'],
            'meso': ['function call order', 'event propagation'],
            'macro': ['subsystem lifecycle', 'actor component coordination'],
            'meta': ['game instance flow', 'level transition state']
        },
        'resonance_points': [
            'subsystem_order',
            'replication_timing',
            'event_propagation',
            'cache_management',
            'tick_dependency'
        ],
        'intervention_templates': {
            'subsystem_order': {
                'description': 'Enforce subsystem initialization dependencies',
                'pattern': r'void Initialize\(FSubsystemCollectionBase& Collection\)',
                'replacement': r'void Initialize(FSubsystemCollectionBase& Collection)\n{\n    // Ensure dependencies are initialized first\n    Collection.InitializeDependency<UDependentSystem>();\n    Super::Initialize(Collection);\n}',
                'cascade_prediction': {
                    'file_impact': 0.6,
                    'build_impact': 0.7,
                    'runtime_impact': 0.9
                }
            },
            'replication_timing': {
                'description': 'Add replication readiness checks',
                'pattern': r'if\s*\(\s*HasAuthority\(\)\s*\)',
                'replacement': r'if (HasAuthority() && GetNetMode() != NM_Client)',
                'cascade_prediction': {
                    'file_impact': 0.4,
                    'build_impact': 0.5,
                    'runtime_impact': 0.8
                }
            },
            'event_propagation': {
                'description': 'Ensure proper event ordering with delegates',
                'pattern': r'(\w+)\.Broadcast\(',
                'replacement': r'if (\1.IsBound()) \1.Broadcast(',
                'cascade_prediction': {
                    'file_impact': 0.5,
                    'build_impact': 0.6,
                    'runtime_impact': 0.7
                }
            }
        },
        'severity': 'high',
        'frequency_weight': 0.70
    },
    
    'coupling_cascade_ue': {
        'name': 'Coupling Cascade (UE)',
        'description': 'Excessive coupling between systems causing circular dependencies, compilation bottlenecks, and architectural fragility',
        'indicators': [
            'circular dependency',
            'excessive includes in header',
            'friend class declaration',
            'tight coupling metric > 0.7',
            'cross-module boundary violation',
            'shared mutable global state',
            'component interdependency web',
            'build order cycle'
        ],
        'scales': {
            'micro': ['include directive', 'friend declaration'],
            'meso': ['class coupling', 'interface violation'],
            'macro': ['module dependency', 'build order'],
            'meta': ['architecture layer violation', 'circular reference graph']
        },
        'resonance_points': [
            'include_management',
            'interface_abstraction',
            'dependency_injection',
            'module_boundaries',
            'event_driven_architecture'
        ],
        'intervention_templates': {
            'include_management': {
                'description': 'Replace includes with forward declarations in headers',
                'pattern': r'#include "(\w+)\.h"',
                'replacement': r'class \1;\n// #include "\1.h" // Moved to cpp',
                'cascade_prediction': {
                    'file_impact': 0.8,
                    'build_impact': 0.9,
                    'runtime_impact': 0.3
                }
            },
            'interface_abstraction': {
                'description': 'Create interface classes to break coupling',
                'pattern': r'class (\w+)\s*:\s*public\s+(\w+)',
                'replacement': r'class I\1Interface\n{\npublic:\n    virtual ~I\1Interface() = default;\n    // Interface methods\n};\n\nclass \1 : public \2, public I\1Interface',
                'cascade_prediction': {
                    'file_impact': 0.7,
                    'build_impact': 0.8,
                    'runtime_impact': 0.6
                }
            },
            'dependency_injection': {
                'description': 'Replace direct dependencies with injected references',
                'pattern': r'(\w+)\* (\w+) = NewObject<(\w+)>\(\);',
                'replacement': r'TWeakObjectPtr<\1> \2;\n// Injected via Initialize()',
                'cascade_prediction': {
                    'file_impact': 0.6,
                    'build_impact': 0.7,
                    'runtime_impact': 0.8
                }
            }
        },
        'severity': 'medium',
        'frequency_weight': 0.60
    },
    
    'api_drift_ue': {
        'name': 'API Drift (UE)',
        'description': 'Usage of deprecated APIs, signature mismatches, and version compatibility issues',
        'indicators': [
            'deprecated function call',
            'signature mismatch',
            'version compatibility warning',
            'obsolete macro usage',
            'deprecated enum value',
            'removed function reference',
            'changed behavior warning',
            'migration required tag'
        ],
        'scales': {
            'micro': ['function call', 'parameter type'],
            'meso': ['method signature', 'class interface'],
            'macro': ['module API', 'subsystem interface'],
            'meta': ['engine version compatibility', 'plugin API']
        },
        'resonance_points': [
            'signature_updates',
            'macro_replacement',
            'enum_standardization',
            'version_gating',
            'migration_automation'
        ],
        'intervention_templates': {
            'signature_updates': {
                'description': 'Update deprecated function signatures',
                'pattern': r'ApplicationContextMask',
                'replacement': r'EditorContext',
                'cascade_prediction': {
                    'file_impact': 0.9,
                    'build_impact': 0.8,
                    'runtime_impact': 0.5
                }
            },
            'macro_replacement': {
                'description': 'Replace deprecated macros with modern equivalents',
                'pattern': r'UFUNCTION\(BlueprintCallable\)',
                'replacement': r'UFUNCTION(BlueprintCallable, Category="API")',
                'cascade_prediction': {
                    'file_impact': 0.6,
                    'build_impact': 0.7,
                    'runtime_impact': 0.4
                }
            },
            'enum_standardization': {
                'description': 'Standardize enum usage across codebase',
                'pattern': r'enum\s+(\w+)\s*{',
                'replacement': r'UENUM(BlueprintType)\nenum class E\1 : uint8 {',
                'cascade_prediction': {
                    'file_impact': 0.8,
                    'build_impact': 0.6,
                    'runtime_impact': 0.7
                }
            }
        },
        'severity': 'medium',
        'frequency_weight': 0.75
    }
}

# Pattern matching regex patterns for code analysis
PATTERN_REGEX = {
    'null_cascade_ue': {
        'TWeakObjectPtr_bool': r'if\s*\(\s*(\w+)\s*\)\s*{?\s*(\w+)->',
        'missing_include': r'//\s*TODO:\s*Add include|class\s+\w+;\s*//\s*Forward declaration',
        'uninitialized_member': r'(\w+)\s*\(\s*\)\s*:\s*Super\(\)\s*{?\s*}',
        'null_reference': r'(\w+)\s*->\s*(\w+)\s*\(\s*\)',
        'stale_delegate': r'(\w+)\.Broadcast\('
    },
    'state_desync_ue': {
        'subsystem_order': r'void Initialize\(FSubsystemCollectionBase& Collection\)',
        'replication_timing': r'if\s*\(\s*HasAuthority\(\)\s*\)',
        'event_order': r'BeginPlay\(\)',
        'tick_dependency': r'Tick\(\s*float DeltaTime\s*\)'
    },
    'coupling_cascade_ue': {
        'circular_include': r'#include "(\w+)\.h"',
        'friend_declaration': r'friend class (\w+);',
        'tight_coupling': r'(\w+)\* (\w+) = NewObject<(\w+)>\(\);'
    },
    'api_drift_ue': {
        'deprecated_mask': r'ApplicationContextMask',
        'signature_mismatch': r'ValidatePositive\([^)]+\)',
        'deprecated_enum': r'enum\s+(\w+)\s*{',
        'obsolete_macro': r'UFUNCTION\(\s*BlueprintCallable\s*\)'
    }
}

# Historical pattern frequency tracking
PATTERN_HISTORY = {
    'null_cascade_ue': {
        'detection_count': 0,
        'fix_success_rate': 0.0,
        'average_cascade_score': 0.0,
        'affected_files': set()
    },
    'state_desync_ue': {
        'detection_count': 0,
        'fix_success_rate': 0.0,
        'average_cascade_score': 0.0,
        'affected_files': set()
    },
    'coupling_cascade_ue': {
        'detection_count': 0,
        'fix_success_rate': 0.0,
        'average_cascade_score': 0.0,
        'affected_files': set()
    },
    'api_drift_ue': {
        'detection_count': 0,
        'fix_success_rate': 0.0,
        'average_cascade_score': 0.0,
        'affected_files': set()
    }
}

def get_pattern_signature(pattern_type):
    """
    Get the signature definition for a specific pattern type.
    
    Args:
        pattern_type (str): The pattern type identifier
        
    Returns:
        dict: Pattern signature with all metadata
    """
    return SIGNATURES.get(pattern_type, None)

def get_all_signatures():
    """
    Get all pattern signatures.
    
    Returns:
        dict: All pattern signatures
    """
    return SIGNATURES.copy()

def update_pattern_history(pattern_type, success, cascade_score, affected_files):
    """
    Update historical tracking for a pattern type.
    
    Args:
        pattern_type (str): The pattern type identifier
        success (bool): Whether the intervention was successful
        cascade_score (float): The measured cascade effect score
        affected_files (list): List of affected file paths
    """
    if pattern_type in PATTERN_HISTORY:
        history = PATTERN_HISTORY[pattern_type]
        history['detection_count'] += 1
        
        # Update success rate
        total_fixes = history['detection_count']
        prev_success_rate = history['fix_success_rate']
        if success:
            history['fix_success_rate'] = ((prev_success_rate * (total_fixes - 1)) + 1.0) / total_fixes
        else:
            history['fix_success_rate'] = (prev_success_rate * (total_fixes - 1)) / total_fixes
        
        # Update average cascade score
        prev_avg = history['average_cascade_score']
        history['average_cascade_score'] = ((prev_avg * (total_fixes - 1)) + cascade_score) / total_fixes
        
        # Add affected files
        history['affected_files'].update(affected_files)

def get_pattern_statistics():
    """
    Get statistics for all patterns.
    
    Returns:
        dict: Pattern statistics with frequencies and success rates
    """
    stats = {}
    for pattern_type, history in PATTERN_HISTORY.items():
        signature = SIGNATURES[pattern_type]
        stats[pattern_type] = {
            'name': signature['name'],
            'detection_count': history['detection_count'],
            'fix_success_rate': history['fix_success_rate'],
            'average_cascade_score': history['average_cascade_score'],
            'affected_file_count': len(history['affected_files']),
            'severity': signature['severity'],
            'frequency_weight': signature['frequency_weight']
        }
    return stats

def predict_pattern_priority(pattern_type):
    """
    Calculate dynamic priority for a pattern based on historical data.
    
    Args:
        pattern_type (str): The pattern type identifier
        
    Returns:
        float: Priority score (higher = more important)
    """
    signature = SIGNATURES.get(pattern_type)
    history = PATTERN_HISTORY.get(pattern_type)
    
    if not signature or not history:
        return 0.0
    
    # Base priority from severity
    severity_weights = {'critical': 1.0, 'high': 0.8, 'medium': 0.6, 'low': 0.4}
    base_priority = severity_weights.get(signature['severity'], 0.5)
    
    # Adjust by frequency weight
    freq_adjustment = signature['frequency_weight']
    
    # Adjust by historical success rate (lower success = higher priority)
    success_penalty = 1.0 - history['fix_success_rate'] if history['detection_count'] > 0 else 0.5
    
    # Adjust by average cascade score (higher cascade = higher priority)
    cascade_boost = history['average_cascade_score'] if history['detection_count'] > 0 else 0.5
    
    # Calculate final priority
    priority = (base_priority * freq_adjustment * 0.4 + 
                success_penalty * 0.3 + 
                cascade_boost * 0.3)
    
    return priority

def get_top_patterns(limit=3):
    """
    Get top priority patterns based on historical data and severity.
    
    Args:
        limit (int): Maximum number of patterns to return
        
    Returns:
        list: Top priority pattern types
    """
    priorities = [(pt, predict_pattern_priority(pt)) for pt in SIGNATURES.keys()]
    priorities.sort(key=lambda x: x[1], reverse=True)
    return [pt for pt, _ in priorities[:limit]]