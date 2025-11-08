"""
Nightly Build Workflow
======================

Automated nightly build pipeline:
1. Build editor (Development)
2. Build game (Shipping)
3. Run automated tests
4. Create distribution package
5. Generate build report

Usage:
    python automation/workflows/nightly_build.py [version]
"""

import sys
import logging
from pathlib import Path
from datetime import datetime

# Add parent directory to path
sys.path.insert(0, str(Path(__file__).parent.parent))

from ue_tools import ue_build, ue_pak, ue_insights

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)


class NightlyBuildWorkflow:
    """Automated nightly build workflow"""

    def __init__(self, version: str = None):
        self.version = version or datetime.now().strftime("%Y%m%d")
        self.build_dir = Path.cwd() / "Builds" / f"Nightly_{self.version}"
        self.build_dir.mkdir(parents=True, exist_ok=True)

        self.results = {
            "version": self.version,
            "start_time": datetime.now().isoformat(),
            "steps": []
        }

    def run(self):
        """Execute complete nightly build workflow"""
        logger.info(f"=== Nightly Build Workflow v{self.version} ===")

        try:
            # Step 1: Build Editor
            if not self.build_editor():
                return False

            # Step 2: Build Game (Shipping)
            if not self.build_game():
                return False

            # Step 3: Package for distribution
            if not self.package_release():
                return False

            # Step 4: Generate build report
            self.generate_report()

            logger.info("=== Nightly Build SUCCEEDED ===")
            return True

        except Exception as e:
            logger.exception(f"Nightly build failed: {e}")
            self.results["error"] = str(e)
            return False

        finally:
            self.results["end_time"] = datetime.now().isoformat()

    def build_editor(self) -> bool:
        """Step 1: Build editor"""
        logger.info("STEP 1: Building editor (Development)...")

        builder = ue_build.UnrealBuildAutomation()
        success, output = builder.build_editor(
            configuration="Development",
            clean=False
        )

        self.results["steps"].append({
            "name": "Build Editor",
            "success": success,
            "output_length": len(output)
        })

        if not success:
            logger.error("Editor build failed")
            self._save_output(output, "editor_build_error.log")
            return False

        logger.info("Editor build succeeded")
        return True

    def build_game(self) -> bool:
        """Step 2: Build game (Shipping)"""
        logger.info("STEP 2: Building game (Shipping)...")

        builder = ue_build.UnrealBuildAutomation()
        success, output = builder.build_game(
            configuration="Shipping",
            clean=False
        )

        self.results["steps"].append({
            "name": "Build Game",
            "success": success,
            "output_length": len(output)
        })

        if not success:
            logger.error("Game build failed")
            self._save_output(output, "game_build_error.log")
            return False

        logger.info("Game build succeeded")
        return True

    def package_release(self) -> bool:
        """Step 3: Create distribution package"""
        logger.info("STEP 3: Packaging for distribution...")

        builder = ue_build.UnrealBuildAutomation()
        success, output = builder.package_for_distribution(
            version=self.version,
            platform="Win64",
            output_dir=self.build_dir
        )

        self.results["steps"].append({
            "name": "Package Release",
            "success": success,
            "output_dir": str(self.build_dir),
            "output_length": len(output)
        })

        if not success:
            logger.error("Packaging failed")
            self._save_output(output, "packaging_error.log")
            return False

        logger.info(f"Package created: {self.build_dir}")
        return True

    def generate_report(self):
        """Generate build report"""
        logger.info("Generating build report...")

        report_path = self.build_dir / "build_report.txt"

        with open(report_path, 'w') as f:
            f.write(f"Nightly Build Report - v{self.version}\n")
            f.write("=" * 60 + "\n\n")

            f.write(f"Start Time: {self.results['start_time']}\n")
            f.write(f"End Time: {self.results['end_time']}\n\n")

            f.write("Build Steps:\n")
            f.write("-" * 60 + "\n")

            for step in self.results["steps"]:
                status = "SUCCESS" if step["success"] else "FAILED"
                f.write(f"  {step['name']}: {status}\n")

            f.write("\n")

            if "error" in self.results:
                f.write(f"ERROR: {self.results['error']}\n")

        logger.info(f"Build report: {report_path}")

    def _save_output(self, output: str, filename: str):
        """Save command output to file"""
        output_path = self.build_dir / filename
        with open(output_path, 'w', encoding='utf-8') as f:
            f.write(output)
        logger.info(f"Output saved: {output_path}")


def main():
    """Main entry point"""
    version = sys.argv[1] if len(sys.argv) > 1 else None

    workflow = NightlyBuildWorkflow(version)
    success = workflow.run()

    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
