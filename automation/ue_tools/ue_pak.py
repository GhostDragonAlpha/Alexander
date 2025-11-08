"""
UnrealPak.exe Python Wrapper
============================

Provides programmatic control over .pak file operations:
- Create pak files from response files
- Extract pak file contents
- List pak contents
- Test pak integrity
- Create compressed/encrypted paks

Usage:
    from automation.ue_tools import ue_pak

    pak_tool = ue_pak.UnrealPakAutomation()

    # Create pak
    success = pak_tool.create_pak(
        "Alexander.pak",
        "FileList.txt",
        compress=True
    )

    # Test integrity
    success, output = pak_tool.test_pak("Alexander.pak")

    # Extract
    success = pak_tool.extract_pak("Alexander.pak", "C:/Extracted")
"""

import subprocess
import logging
from pathlib import Path
from typing import Tuple, Optional, List
from . import UNREAL_PAK

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


class UnrealPakAutomation:
    """Wrapper for UnrealPak.exe operations"""

    def __init__(self):
        self.pak_exe = UNREAL_PAK

        if not self.pak_exe.exists():
            raise FileNotFoundError(f"UnrealPak.exe not found: {self.pak_exe}")

    def create_pak(
        self,
        pak_name: str,
        response_file: str,
        compress: bool = True,
        encrypt: bool = False,
        output_dir: Optional[Path] = None
    ) -> Tuple[bool, str]:
        """
        Create a .pak file from response file

        Args:
            pak_name: Name of output .pak file
            response_file: Text file listing files to include
            compress: Compress files in pak
            encrypt: Encrypt pak with AES key
            output_dir: Output directory (defaults to current directory)

        Returns:
            Tuple of (success: bool, output: str)

        Response file format (one file per line):
            "C:/SourcePath/File1.uasset" "../../../Alexander/Content/File1.uasset"
            "C:/SourcePath/File2.umap" "../../../Alexander/Content/Maps/File2.umap"
        """
        logger.info(f"Creating pak: {pak_name}")

        response_path = Path(response_file)
        if not response_path.exists():
            return False, f"Response file not found: {response_file}"

        pak_path = Path(output_dir) / pak_name if output_dir else Path(pak_name)

        cmd = [str(self.pak_exe), str(pak_path), f"-create={response_path}"]

        if compress:
            cmd.append("-compress")

        if encrypt:
            cmd.append("-encrypt")

        return self._run_command(cmd)

    def extract_pak(
        self,
        pak_file: str,
        target_path: str,
        filter_pattern: Optional[str] = None
    ) -> Tuple[bool, str]:
        """
        Extract .pak file contents

        Args:
            pak_file: Path to .pak file
            target_path: Directory to extract to
            filter_pattern: Optional filter (e.g., "*.uasset")

        Returns:
            Tuple of (success: bool, output: str)
        """
        logger.info(f"Extracting pak: {pak_file} -> {target_path}")

        pak_path = Path(pak_file)
        if not pak_path.exists():
            return False, f"Pak file not found: {pak_file}"

        extract_dir = Path(target_path)
        extract_dir.mkdir(parents=True, exist_ok=True)

        cmd = [str(self.pak_exe), str(pak_path), "-extract", str(extract_dir)]

        if filter_pattern:
            cmd.append(f"-filter={filter_pattern}")

        return self._run_command(cmd)

    def test_pak_integrity(self, pak_file: str) -> Tuple[bool, str]:
        """
        Verify pak file integrity

        Args:
            pak_file: Path to .pak file

        Returns:
            Tuple of (success: bool, output: str)
        """
        logger.info(f"Testing pak integrity: {pak_file}")

        pak_path = Path(pak_file)
        if not pak_path.exists():
            return False, f"Pak file not found: {pak_file}"

        cmd = [str(self.pak_exe), str(pak_path), "-test"]
        return self._run_command(cmd)

    def list_pak_contents(self, pak_file: str) -> Tuple[bool, str]:
        """
        List files in .pak

        Args:
            pak_file: Path to .pak file

        Returns:
            Tuple of (success: bool, file_list: str)
        """
        logger.info(f"Listing pak contents: {pak_file}")

        pak_path = Path(pak_file)
        if not pak_path.exists():
            return False, f"Pak file not found: {pak_file}"

        cmd = [str(self.pak_exe), str(pak_path), "-list"]
        return self._run_command(cmd)

    def create_response_file(
        self,
        source_dir: Path,
        mount_point: str,
        output_file: str,
        extensions: Optional[List[str]] = None
    ) -> Path:
        """
        Generate response file for pak creation

        Args:
            source_dir: Directory containing files to pak
            mount_point: Virtual mount point in pak (e.g., "../../../Alexander/Content/")
            output_file: Output response file path
            extensions: File extensions to include (e.g., [".uasset", ".umap"])

        Returns:
            Path to created response file
        """
        logger.info(f"Creating response file: {output_file}")

        source_path = Path(source_dir)
        response_path = Path(output_file)

        if not source_path.exists():
            raise FileNotFoundError(f"Source directory not found: {source_dir}")

        with open(response_path, 'w') as f:
            for file_path in source_path.rglob('*'):
                if file_path.is_file():
                    # Check extension filter
                    if extensions and file_path.suffix not in extensions:
                        continue

                    # Calculate relative path from source
                    rel_path = file_path.relative_to(source_path)

                    # Build mount path
                    mount_path = f"{mount_point}{rel_path.as_posix()}"

                    # Write response line
                    f.write(f'"{file_path}" "{mount_path}"\n')

        logger.info(f"Response file created with {sum(1 for _ in open(response_path))} entries")
        return response_path

    def _run_command(self, cmd: List[str]) -> Tuple[bool, str]:
        """Execute UnrealPak command"""
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
                logger.info("Command completed successfully")
            else:
                logger.error(f"Command failed with code {result.returncode}")

            return success, result.stdout + "\n" + result.stderr

        except Exception as e:
            logger.exception(f"Command failed: {e}")
            return False, str(e)


# Convenience functions
def create_pak(pak_name: str, response_file: str, **kwargs) -> Tuple[bool, str]:
    """Quick create pak"""
    return UnrealPakAutomation().create_pak(pak_name, response_file, **kwargs)


def extract_pak(pak_file: str, target_path: str, **kwargs) -> Tuple[bool, str]:
    """Quick extract pak"""
    return UnrealPakAutomation().extract_pak(pak_file, target_path, **kwargs)


def test_pak(pak_file: str) -> Tuple[bool, str]:
    """Quick test pak"""
    return UnrealPakAutomation().test_pak_integrity(pak_file)


if __name__ == "__main__":
    # Example usage
    import sys

    if len(sys.argv) < 2:
        print("Usage: python ue_pak.py [create|extract|test|list] [args]")
        sys.exit(1)

    command = sys.argv[1]
    pak_tool = UnrealPakAutomation()

    if command == "create" and len(sys.argv) >= 4:
        pak_name = sys.argv[2]
        response_file = sys.argv[3]
        success, output = pak_tool.create_pak(pak_name, response_file)
    elif command == "extract" and len(sys.argv) >= 4:
        pak_file = sys.argv[2]
        target_path = sys.argv[3]
        success, output = pak_tool.extract_pak(pak_file, target_path)
    elif command == "test" and len(sys.argv) >= 3:
        pak_file = sys.argv[2]
        success, output = pak_tool.test_pak_integrity(pak_file)
    elif command == "list" and len(sys.argv) >= 3:
        pak_file = sys.argv[2]
        success, output = pak_tool.list_pak_contents(pak_file)
    else:
        print("Invalid command or missing arguments")
        sys.exit(1)

    print(output)
    sys.exit(0 if success else 1)
