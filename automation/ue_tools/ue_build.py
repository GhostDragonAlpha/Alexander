"""
UnrealBuildTool and Unreal Automation Tool (UAT) Python Wrapper
================================================================

Provides programmatic control over:
- C++ compilation (UnrealBuildTool)
- Content cooking (UAT Cook command)
- Project packaging (UAT BuildCookRun command)
- Automated builds and deployment

Usage:
    from automation.ue_tools import ue_build

    builder = ue_build.UnrealBuildAutomation()

    # Build editor
    success, output = builder.build_editor()

    # Build game (shipping)
    success, output = builder.build_game(configuration="Shipping")

    # Full package
    success, output = builder.build_cook_package(
        platform="Win64",
        configuration="Shipping",
        archive_dir="C:/Builds/Alexander_v1.0"
    )
"""

import subprocess
import logging
from pathlib import Path
from typing import Tuple, Optional, List
from . import BUILD_BAT, RUN_UAT_BAT, PROJECT_FILE

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


class UnrealBuildAutomation:
    """Wrapper for UnrealBuildTool and UAT automation"""

    def __init__(self, project_path: Optional[Path] = None):
        """
        Initialize build automation

        Args:
            project_path: Path to .uproject file (defaults to PROJECT_FILE)
        """
        self.project_path = Path(project_path) if project_path else PROJECT_FILE
        self.build_bat = BUILD_BAT
        self.uat_bat = RUN_UAT_BAT

        if not self.project_path.exists():
            raise FileNotFoundError(f"Project file not found: {self.project_path}")
        if not self.build_bat.exists():
            raise FileNotFoundError(f"Build.bat not found: {self.build_bat}")
        if not self.uat_bat.exists():
            raise FileNotFoundError(f"RunUAT.bat not found: {self.uat_bat}")

    def build_editor(
        self,
        platform: str = "Win64",
        configuration: str = "Development",
        clean: bool = False,
        verbose: bool = False
    ) -> Tuple[bool, str]:
        """
        Build the editor (AlexanderEditor target)

        Args:
            platform: Target platform (Win64, Linux, Mac)
            configuration: Build configuration (Debug, Development, Shipping)
            clean: Clean before building
            verbose: Enable verbose output

        Returns:
            Tuple of (success: bool, output: str)
        """
        logger.info(f"Building editor: {platform} {configuration}")

        cmd = [
            str(self.build_bat),
            "AlexanderEditor",
            platform,
            configuration,
            f"-Project={self.project_path}",
            "-WaitMutex"
        ]

        if clean:
            cmd.append("-Clean")
        if verbose:
            cmd.append("-Verbose")

        return self._run_command(cmd)

    def build_game(
        self,
        platform: str = "Win64",
        configuration: str = "Shipping",
        clean: bool = False
    ) -> Tuple[bool, str]:
        """
        Build the standalone game (Alexander target)

        Args:
            platform: Target platform
            configuration: Build configuration
            clean: Clean before building

        Returns:
            Tuple of (success: bool, output: str)
        """
        logger.info(f"Building game: {platform} {configuration}")

        cmd = [
            str(self.build_bat),
            "Alexander",
            platform,
            configuration,
            f"-Project={self.project_path}",
            "-WaitMutex"
        ]

        if clean:
            cmd.append("-Clean")

        return self._run_command(cmd)

    def cook_content(
        self,
        platform: str = "Win64",
        maps: Optional[List[str]] = None,
        iterate: bool = False
    ) -> Tuple[bool, str]:
        """
        Cook content for target platform

        Args:
            platform: Target platform
            maps: Specific maps to cook (None = all maps)
            iterate: Use iterative cooking (faster for iterations)

        Returns:
            Tuple of (success: bool, output: str)
        """
        logger.info(f"Cooking content for {platform}")

        cmd = [
            str(self.uat_bat),
            "Cook",
            f"-project={self.project_path}",
            f"-platform={platform}",
            "-unattended",
            "-utf8output"
        ]

        if maps:
            cmd.append(f"-map={'+'.join(maps)}")

        if iterate:
            cmd.append("-iterate")

        return self._run_command(cmd)

    def build_cook_package(
        self,
        platform: str = "Win64",
        configuration: str = "Shipping",
        archive_dir: Optional[Path] = None,
        pak: bool = True,
        compress: bool = True,
        clean: bool = False,
        skip_cook: bool = False
    ) -> Tuple[bool, str]:
        """
        Complete build, cook, and package pipeline

        Args:
            platform: Target platform
            configuration: Build configuration
            archive_dir: Directory to archive build (None = no archive)
            pak: Create .pak files
            compress: Compress pak files
            clean: Clean before building
            skip_cook: Skip cooking step (use existing cooked content)

        Returns:
            Tuple of (success: bool, output: str)
        """
        logger.info(f"Build-Cook-Package: {platform} {configuration}")

        cmd = [
            str(self.uat_bat),
            "BuildCookRun",
            f"-project={self.project_path}",
            f"-platform={platform}",
            f"-configuration={configuration}",
            "-build",
            "-stage",
            "-package",
            "-unattended",
            "-utf8output"
        ]

        if not skip_cook:
            cmd.append("-cook")

        if pak:
            cmd.append("-pak")

        if compress:
            cmd.append("-compressed")

        if clean:
            cmd.append("-clean")

        if archive_dir:
            archive_path = Path(archive_dir)
            archive_path.mkdir(parents=True, exist_ok=True)
            cmd.extend(["-archive", f"-archivedirectory={archive_path}"])

        return self._run_command(cmd)

    def package_for_distribution(
        self,
        version: str,
        platform: str = "Win64",
        output_dir: Optional[Path] = None
    ) -> Tuple[bool, str]:
        """
        Convenience method: Full distribution package with version

        Args:
            version: Version string (e.g., "1.0.0")
            platform: Target platform
            output_dir: Output directory (defaults to PROJECT_ROOT/Builds/v{version})

        Returns:
            Tuple of (success: bool, output: str)
        """
        if output_dir is None:
            output_dir = self.project_path.parent / "Builds" / f"v{version}"

        logger.info(f"Creating distribution package v{version}")

        return self.build_cook_package(
            platform=platform,
            configuration="Shipping",
            archive_dir=output_dir,
            pak=True,
            compress=True,
            clean=True
        )

    def _run_command(self, cmd: List[str]) -> Tuple[bool, str]:
        """
        Execute build command and capture output

        Args:
            cmd: Command list

        Returns:
            Tuple of (success: bool, output: str)
        """
        logger.debug(f"Executing: {' '.join(cmd)}")

        try:
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                encoding='utf-8',
                errors='replace'
            )

            success = result.returncode == 0

            if success:
                logger.info("Build completed successfully")
            else:
                logger.error(f"Build failed with code {result.returncode}")

            return success, result.stdout + "\n" + result.stderr

        except Exception as e:
            logger.exception(f"Build command failed: {e}")
            return False, str(e)


# Convenience functions for quick access
def build_editor(**kwargs) -> Tuple[bool, str]:
    """Quick build editor"""
    return UnrealBuildAutomation().build_editor(**kwargs)


def build_game(**kwargs) -> Tuple[bool, str]:
    """Quick build game"""
    return UnrealBuildAutomation().build_game(**kwargs)


def package_release(version: str, **kwargs) -> Tuple[bool, str]:
    """Quick package for distribution"""
    return UnrealBuildAutomation().package_for_distribution(version, **kwargs)


if __name__ == "__main__":
    # Example usage
    import sys

    if len(sys.argv) < 2:
        print("Usage: python ue_build.py [editor|game|package] [args]")
        sys.exit(1)

    command = sys.argv[1]
    builder = UnrealBuildAutomation()

    if command == "editor":
        success, output = builder.build_editor()
    elif command == "game":
        success, output = builder.build_game()
    elif command == "package":
        version = sys.argv[2] if len(sys.argv) > 2 else "dev"
        success, output = builder.package_for_distribution(version)
    else:
        print(f"Unknown command: {command}")
        sys.exit(1)

    print(output)
    sys.exit(0 if success else 1)
