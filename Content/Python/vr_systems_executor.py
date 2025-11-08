
import unreal
import json
from datetime import datetime

def execute_all_vr_systems():
    """Execute all deployed VR systems"""
    
    print("=" * 80)
    print("EXECUTING VR SPACE FARMING SIMULATOR SYSTEMS")
    print("=" * 80)
    
    execution_results = {
        'timestamp': datetime.now().isoformat(),
        'execution_type': 'vr_systems_deployment',
        'systems_executed': [],
        'execution_log': [],
        'status': 'running'
    }
    
    vr_systems = [
        ('VR Interaction System', 'Content/Python/vr_interaction_system.py'),
        ('Space Farming Mechanics', 'Content/Python/space_farming_mechanics.py'),
        ('6DOF Flight System', 'Content/Python/6dof_flight_system.py')
    ]
    
    for system_name, script_path in vr_systems:
        try:
            print(f"\nExecuting: {system_name}")
            print("-" * 50)
            
            # Execute the VR system script
            exec(open(script_path).read())
            
            execution_results['systems_executed'].append(system_name)
            execution_results['execution_log'].append(f"✅ {system_name}: Successfully executed")
            print(f"✅ {system_name} executed successfully!")
            
        except Exception as e:
            error_msg = f"❌ {system_name}: Error - {str(e)}"
            execution_results['execution_log'].append(error_msg)
            print(f"❌ Error executing {system_name}: {e}")
    
    # Final status
    successful_systems = len(execution_results['systems_executed'])
    total_systems = len(vr_systems)
    
    if successful_systems == total_systems:
        execution_results['status'] = 'complete_success'
        print(f"\n🎉 ALL VR SYSTEMS EXECUTED SUCCESSFULLY! ({successful_systems}/{total_systems})")
    elif successful_systems > 0:
        execution_results['status'] = 'partial_success'
        print(f"\n⚠️ PARTIAL SUCCESS: {successful_systems}/{total_systems} systems executed")
    else:
        execution_results['status'] = 'failed'
        print(f"\n❌ EXECUTION FAILED: No systems executed successfully")
    
    print("=" * 80)
    print("VR SPACE FARMING SIMULATOR READY FOR TESTING!")
    print("=" * 80)
    
    # Save execution results
    results_file = "C:/Users/allen/Desktop/Alexander/vr_execution_results.json"
    try:
        with open(results_file, 'w') as f:
            json.dump(execution_results, f, indent=2)
        print(f"Execution results saved: {results_file}")
    except Exception as e:
        print(f"Could not save execution results: {e}")
    
    # Mark execution completion
    completion_file = "C:/Users/allen/Desktop/Alexander/vr_execution_complete.txt"
    try:
        with open(completion_file, 'w') as f:
            f.write(f"VR execution completed at {datetime.now().isoformat()}\n")
            f.write(f"Status: {execution_results['status']}\n")
            f.write(f"Systems: {successful_systems}/{total_systems} executed\n")
            for log_entry in execution_results['execution_log']:
                f.write(f"{log_entry}\n")
        print(f"Completion marker saved: {completion_file}")
    except Exception as e:
        print(f"Could not save completion marker: {e}")
    
    return execution_results

# Execute all VR systems
execute_all_vr_systems()
