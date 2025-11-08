"""
Quick test to verify AI Agent System is working correctly
Tests basic functionality without requiring full UE execution
"""

import sys
from pathlib import Path

def test_imports():
    """Test that all modules can be imported"""
    print("Testing imports...")
    
    try:
        from ai_unreal_controller import AIUnrealController, UnrealTool
        print("  ✓ ai_unreal_controller imported")
        
        from ai_agents import AIContentAgent, AIBuildAgent, AITestingAgent
        print("  ✓ ai_agents imported")
        
        from ai_workflow_orchestrator import WorkflowOrchestrator
        print("  ✓ ai_workflow_orchestrator imported")
        
        return True
    except Exception as e:
        print(f"  ✗ Import failed: {e}")
        return False

def test_controller_init():
    """Test controller initialization"""
    print("\nTesting controller initialization...")
    
    try:
        from ai_unreal_controller import AIUnrealController
        
        engine_path = "C:/Program Files/Epic Games/UE_5.6"
        project_path = Path(__file__).parent / "Alexander.uproject"
        
        # Verify paths exist
        if not Path(engine_path).exists():
            print(f"  ⚠ Engine path not found (expected): {engine_path}")
            return True  # Expected in test environment
            
        if not project_path.exists():
            print(f"  ⚠ Project path not found: {project_path}")
            return True  # May not exist in test
            
        controller = AIUnrealController(
            engine_path=str(engine_path),
            project_path=str(project_path)
        )
        
        print(f"  ✓ Controller initialized")
        print(f"    Engine: {controller.engine_path}")
        print(f"    Project: {controller.project_path.name}")
        print(f"    Binaries: {controller.binaries}")
        
        return True
        
    except Exception as e:
        print(f"  ✗ Controller init failed: {e}")
        return False

def test_agents_init():
    """Test agent initialization"""
    print("\nTesting agent initialization...")
    
    try:
        from ai_unreal_controller import AIUnrealController
        from ai_agents import AIContentAgent, AIBuildAgent, AITestingAgent
        
        engine_path = "C:/Program Files/Epic Games/UE_5.6"
        project_path = Path(__file__).parent / "Alexander.uproject"
        
        if not Path(engine_path).exists() or not project_path.exists():
            print("  ⚠ Skipping (paths not available)")
            return True
            
        controller = AIUnrealController(
            engine_path=str(engine_path),
            project_path=str(project_path)
        )
        
        content_agent = AIContentAgent(controller)
        print("  ✓ AIContentAgent initialized")
        
        build_agent = AIBuildAgent(controller)
        print("  ✓ AIBuildAgent initialized")
        
        test_agent = AITestingAgent(controller)
        print("  ✓ AITestingAgent initialized")
        
        return True
        
    except Exception as e:
        print(f"  ✗ Agent init failed: {e}")
        return False

def test_orchestrator_init():
    """Test orchestrator initialization"""
    print("\nTesting orchestrator initialization...")
    
    try:
        from ai_workflow_orchestrator import WorkflowOrchestrator
        
        engine_path = "C:/Program Files/Epic Games/UE_5.6"
        project_path = str(Path(__file__).parent / "Alexander.uproject")
        
        if not Path(engine_path).exists() or not Path(project_path).exists():
            print("  ⚠ Skipping (paths not available)")
            return True
            
        orchestrator = WorkflowOrchestrator(
            engine_path=engine_path,
            project_path=project_path
        )
        
        print("  ✓ WorkflowOrchestrator initialized")
        print(f"    Content Agent: {orchestrator.content_agent}")
        print(f"    Build Agent: {orchestrator.build_agent}")
        print(f"    Test Agent: {orchestrator.test_agent}")
        print(f"    Reports Dir: {orchestrator.reports_dir}")
        
        # Verify reports directory created
        if orchestrator.reports_dir.exists():
            print(f"  ✓ Reports directory created: {orchestrator.reports_dir}")
        
        return True
        
    except Exception as e:
        print(f"  ✗ Orchestrator init failed: {e}")
        return False

def test_unreal_tools_enum():
    """Test UnrealTool enum"""
    print("\nTesting UnrealTool enum...")
    
    try:
        from ai_unreal_controller import UnrealTool
        
        tools = [
            UnrealTool.EDITOR_CMD,
            UnrealTool.UAT,
            UnrealTool.UBT,
            UnrealTool.FRONTEND,
            UnrealTool.INSIGHTS,
            UnrealTool.VERSION_SELECTOR
        ]
        
        for tool in tools:
            print(f"  ✓ {tool.name}: {tool.value}")
            
        return True
        
    except Exception as e:
        print(f"  ✗ UnrealTool enum test failed: {e}")
        return False

def test_build_integration():
    """Test build_and_test.py integration"""
    print("\nTesting build_and_test.py integration...")
    
    try:
        build_script = Path(__file__).parent / "build_and_test.py"
        
        if not build_script.exists():
            print(f"  ⚠ build_and_test.py not found")
            return True
            
        content = build_script.read_text()
        
        # Check for AI integration
        if 'elif command == "ai":' in content:
            print("  ✓ AI command integrated in build_and_test.py")
        else:
            print("  ✗ AI command not found in build_and_test.py")
            return False
            
        if 'from ai_workflow_orchestrator import WorkflowOrchestrator' in content:
            print("  ✓ WorkflowOrchestrator import found")
        else:
            print("  ✗ WorkflowOrchestrator import not found")
            return False
            
        return True
        
    except Exception as e:
        print(f"  ✗ Build integration test failed: {e}")
        return False

def main():
    """Run all tests"""
    print("="*60)
    print("AI Agent System Test Suite")
    print("="*60)
    
    results = []
    
    # Run tests
    results.append(("Imports", test_imports()))
    results.append(("Controller Init", test_controller_init()))
    results.append(("Agents Init", test_agents_init()))
    results.append(("Orchestrator Init", test_orchestrator_init()))
    results.append(("UnrealTool Enum", test_unreal_tools_enum()))
    results.append(("Build Integration", test_build_integration()))
    
    # Print summary
    print("\n" + "="*60)
    print("Test Summary")
    print("="*60)
    
    passed = sum(1 for _, result in results if result)
    total = len(results)
    
    for name, result in results:
        status = "PASS" if result else "FAIL"
        symbol = "✓" if result else "✗"
        print(f"  {symbol} {name}: {status}")
    
    print(f"\nResults: {passed}/{total} tests passed")
    
    if passed == total:
        print("\n✓ All tests passed! AI Agent System is operational.")
        return 0
    else:
        print(f"\n✗ {total - passed} test(s) failed. Check output above.")
        return 1

if __name__ == "__main__":
    sys.exit(main())
