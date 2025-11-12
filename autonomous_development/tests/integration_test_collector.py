"""
Integration test: Run actual build and verify collector works
"""

import sys
from pathlib import Path

# Add parent directory to path
sys.path.insert(0, str(Path(__file__).parent.parent))

from information_gatherers.compilation_error_collector import CompilationErrorCollector


def test_real_build():
    """Test with actual Unreal project build."""
    print("Integration Test: Real Build Collection")
    print("="*70)
    
    # Configuration
    ENGINE_PATH = "C:/Program Files/Epic Games/UE_5.6"
    PROJECT_PATH = "C:/Users/allen/Desktop/Alexander/Alexander"
    
    try:
        # Create collector
        print("Creating collector...")
        collector = CompilationErrorCollector(ENGINE_PATH, PROJECT_PATH)
        
        # Run build (this will take time)
        print("Running build - this may take several minutes...")
        result = collector.run_build(
            configuration="Development",
            platform="Win64",
            module="Alexander"
        )
        
        # Verify result structure
        assert "collector_type" in result
        assert "timestamp" in result
        assert "build_config" in result
        assert "execution" in result
        assert "results" in result
        assert "success" in result["results"]
        
        print("\n✅ Integration test passed!")
        print(f"Build success: {result['results']['success']}")
        print(f"Errors: {result['results']['total_errors']}")
        print(f"Warnings: {result['results']['total_warnings']}")
        
        return True
        
    except Exception as e:
        print(f"\n❌ Integration test failed: {e}")
        import traceback
        traceback.print_exc()
        return False


if __name__ == "__main__":
    success = test_real_build()
    sys.exit(0 if success else 1)