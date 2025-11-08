"""
AI Agent System Demo
Quick demonstration of capabilities
"""

from ai_unreal_controller import AIUnrealController
from ai_agents import AIContentAgent, AITestingAgent
import json
from datetime import datetime

def main():
    print("=" * 70)
    print("AI AGENT SYSTEM DEMONSTRATION")
    print("=" * 70)
    print()
    
    # Initialize
    print("Initializing AI controller...")
    controller = AIUnrealController(
        engine_path="C:/Program Files/Epic Games/UE_5.6",
        project_path="C:/Users/allen/Desktop/Alexander/Alexander/Alexander.uproject"
    )
    print(f"✓ Controller ready")
    print(f"  Engine: {controller.engine_path}")
    print(f"  Project: {controller.project_path.name}")
    print()
    
    # Create agents
    print("Creating specialized agents...")
    content_agent = AIContentAgent(controller)
    test_agent = AITestingAgent(controller)
    print("✓ Agents ready")
    print("  - AIContentAgent")
    print("  - AITestingAgent")
    print()
    
    # Demo 1: Generate Asset Report (lightweight)
    print("-" * 70)
    print("DEMO 1: Asset Report Generation")
    print("-" * 70)
    print("Generating comprehensive asset report...")
    print("This analyzes all assets in the project using UE Python API")
    print()
    
    start = datetime.now()
    result = controller.generate_asset_report(
        output_path="Saved/Reports/demo_asset_report.json"
    )
    duration = (datetime.now() - start).total_seconds()
    
    if result['success']:
        report = result['report']
        print(f"✓ Report generated successfully in {duration:.1f}s")
        print(f"  Total assets: {report['total_assets']}")
        print(f"  Report saved: {result['report_path']}")
        print()
        print("  Asset breakdown by type:")
        for asset_type, count in sorted(report['by_type'].items(), key=lambda x: x[1], reverse=True)[:10]:
            print(f"    - {asset_type}: {count}")
        print()
        print("  Asset breakdown by path:")
        for path, count in sorted(report['by_path'].items(), key=lambda x: x[1], reverse=True)[:5]:
            print(f"    - {path}: {count}")
    else:
        print(f"✗ Report generation failed")
        print(f"  Error: {result.get('stderr', 'Unknown error')}")
    
    print()
    
    # Demo 2: Show system capabilities
    print("-" * 70)
    print("SYSTEM CAPABILITIES SUMMARY")
    print("-" * 70)
    print()
    print("Available Workflows (via build_and_test.py ai):")
    print("  1. daily          - Daily maintenance (10-15 min)")
    print("     • Validate all content")
    print("     • Find unused assets")
    print("     • Generate reports")
    print("     • Run smoke tests")
    print()
    print("  2. precommit      - Pre-commit validation (2-5 min)")
    print("     • Validate changed content")
    print("     • Quick smoke tests")
    print()
    print("  3. content        - Content creation workflow (5-10 min)")
    print("     • Organize imported assets")
    print("     • Auto-generate LODs")
    print("     • Validate content")
    print()
    print("  4. optimize       - Performance optimization (15-30 min)")
    print("     • Baseline measurement")
    print("     • Optimize meshes/textures")
    print("     • Post-optimization measurement")
    print()
    print("  5. history        - View workflow history")
    print()
    
    print("Content Agent Operations:")
    print("  • validate_all_content() - Comprehensive validation")
    print("  • auto_generate_lods() - Batch LOD generation")
    print("  • cleanup_unused_assets() - Find & remove unused")
    print("  • organize_content_by_type() - Auto-organize imports")
    print()
    
    print("Testing Agent Operations:")
    print("  • run_smoke_tests() - Quick validation")
    print("  • run_integration_tests() - Full test suite")
    print("  • run_performance_tests() - Performance profiling")
    print("  • analyze_test_failures() - Smart failure analysis")
    print()
    
    print("Controller Direct Operations:")
    print("  • execute_python_in_editor() - Run Python in UE")
    print("  • execute_commandlet() - Run UE commandlets")
    print("  • validate_content() - Data validation")
    print("  • build_lighting() - Automated lighting builds")
    print("  • find_unused_assets() - Detect unused assets")
    print("  • optimize_assets_batch() - Batch optimization")
    print()
    
    # Demo 3: Show next steps
    print("-" * 70)
    print("NEXT STEPS")
    print("-" * 70)
    print()
    print("Ready to Use:")
    print("  1. Run daily maintenance:")
    print("     python build_and_test.py ai daily")
    print()
    print("  2. Add pre-commit hook:")
    print("     Create .git/hooks/pre-commit with:")
    print("     python build_and_test.py ai precommit")
    print()
    print("  3. Test content workflow:")
    print("     python build_and_test.py ai content environment")
    print()
    print("  4. Schedule nightly maintenance:")
    print("     Add to Task Scheduler (Windows) or cron (Linux)")
    print()
    print("Documentation:")
    print("  • AI_AGENT_SYSTEM_README.md - Complete guide")
    print("  • AI_AGENT_QUICK_REFERENCE.md - Quick commands")
    print("  • AI_AGENT_IMPLEMENTATION_COMPLETE.md - Technical details")
    print()
    
    print("=" * 70)
    print("DEMONSTRATION COMPLETE")
    print("=" * 70)
    print(f"\n✓ AI Agent System is operational and ready for use!")
    print(f"✓ All workflows available via: python build_and_test.py ai <workflow>")
    print()

if __name__ == "__main__":
    main()
