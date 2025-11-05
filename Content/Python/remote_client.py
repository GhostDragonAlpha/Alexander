"""
===============================================================================
REMOTE EXECUTION CLIENT
===============================================================================
Send Python commands to running Unreal Editor without restarting.

This uses Unreal's Python Remote Execution feature (configured at 239.0.0.1:6766)
to execute Python code in the running editor instance.

Usage:
    # From command line:
    python remote_client.py "print('Hello from Unreal!')"

    # Execute a file:
    python remote_client.py --file "path/to/script.py"

    # From another Python script:
    from remote_client import execute_remote, execute_file_remote
    result = execute_remote("unreal.log('Test')")

Advantages:
    - No restart needed (saves 30-60 seconds per iteration)
    - Instant feedback
    - Captures output/errors
    - Perfect for rapid iteration and debugging

When to use restart instead:
    - Changing workflow_config.json
    - Modifying init_unreal.py startup sequence
    - Installing new plugins
    - Major project setting changes
===============================================================================
"""

import sys
import os
from upyrc import upyre

# Configuration for Alexander project
DEFAULT_CONFIG = {
    "multicast_group": ("239.0.0.1", 6766),  # Python Remote Execution endpoint
    "multicast_bind_address": "0.0.0.0"
}

def create_config():
    """Create remote execution configuration"""
    return upyre.RemoteExecutionConfig(
        multicast_group=DEFAULT_CONFIG["multicast_group"],
        multicast_bind_address=DEFAULT_CONFIG["multicast_bind_address"]
    )

def execute_remote(command, verbose=True, raise_on_error=False):
    """
    Execute Python code in running Unreal Editor.

    Args:
        command (str): Python code to execute
        verbose (bool): Print output and status
        raise_on_error (bool): Raise exception on execution failure

    Returns:
        dict: Result with keys: success, output, result, command

    Raises:
        ConnectionError: If can't connect to Unreal
        RuntimeError: If execution fails and raise_on_error=True
    """
    config = create_config()

    try:
        with upyre.PythonRemoteConnection(config) as conn:
            result = conn.execute_python_command(
                command,
                exec_type=upyre.ExecTypes.EXECUTE_STATEMENT,
                raise_exc=False  # We'll handle errors ourselves
            )

            # Convert PythonCommandResult object to dict for easier handling
            result_dict = {
                'success': result.success,
                'output': result.output if hasattr(result, 'output') else '',
                'result': result.result if hasattr(result, 'result') else '',
                'command': command
            }

            if verbose:
                print("=" * 70)
                print("  REMOTE EXECUTION RESULT")
                print("=" * 70)
                print(f"Success: {result_dict['success']}")
                if result_dict['output']:
                    # Handle encoding errors for Windows console
                    try:
                        print(f"\nOutput:\n{result_dict['output']}")
                    except UnicodeEncodeError:
                        # Fallback: encode with error handling
                        output_safe = str(result_dict['output']).encode('ascii', errors='replace').decode('ascii')
                        print(f"\nOutput:\n{output_safe}")
                if result_dict['result']:
                    try:
                        print(f"\nResult: {result_dict['result']}")
                    except UnicodeEncodeError:
                        result_safe = str(result_dict['result']).encode('ascii', errors='replace').decode('ascii')
                        print(f"\nResult: {result_safe}")
                print("=" * 70)

            if not result_dict['success'] and raise_on_error:
                raise RuntimeError(f"Execution failed: {result_dict.get('result', 'Unknown error')}")

            return result_dict

    except Exception as e:
        error_msg = f"Failed to connect to Unreal Editor: {e}"
        if verbose:
            print("=" * 70)
            print("  CONNECTION FAILED")
            print("=" * 70)
            print(error_msg)
            print("\nTroubleshooting:")
            print("  1. Is Unreal Editor running?")
            print("  2. Is Python Remote Execution enabled in Project Settings?")
            print("  3. Check endpoint: 239.0.0.1:6766")
            print("  4. Try restarting Unreal Editor")
            print("=" * 70)

        if raise_on_error:
            raise ConnectionError(error_msg) from e

        return {
            'success': False,
            'output': '',
            'result': str(e),
            'command': command
        }

def execute_file_remote(file_path, verbose=True, raise_on_error=False):
    """
    Execute a Python file in running Unreal Editor.

    Args:
        file_path (str): Path to Python file to execute
        verbose (bool): Print output and status
        raise_on_error (bool): Raise exception on execution failure

    Returns:
        dict: Result from execution

    Raises:
        FileNotFoundError: If file doesn't exist
        ConnectionError: If can't connect to Unreal
        RuntimeError: If execution fails and raise_on_error=True
    """
    if not os.path.exists(file_path):
        raise FileNotFoundError(f"File not found: {file_path}")

    with open(file_path, 'r', encoding='utf-8') as f:
        code = f.read()

    if verbose:
        print(f"Executing file: {file_path}")

    # Wrap in exec() to allow multi-statement scripts
    exec_command = f"exec({repr(code)})"

    return execute_remote(exec_command, verbose=verbose, raise_on_error=raise_on_error)

def test_connection(verbose=True):
    """
    Test if we can connect to Unreal Editor.

    Returns:
        bool: True if connection successful
    """
    result = execute_remote(
        "import unreal; print('Remote execution working!')",
        verbose=verbose,
        raise_on_error=False
    )
    return result['success']

def main():
    """Command-line interface"""
    import argparse

    parser = argparse.ArgumentParser(
        description='Execute Python code in running Unreal Editor',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Execute code:
  python remote_client.py "print('Hello')"

  # Execute file:
  python remote_client.py --file clear_level.py

  # Test connection:
  python remote_client.py --test

  # Silent mode:
  python remote_client.py --quiet "unreal.log('test')"
        """
    )

    parser.add_argument(
        'command',
        nargs='?',
        help='Python code to execute (or use --file)'
    )
    parser.add_argument(
        '--file', '-f',
        help='Execute Python file instead of command'
    )
    parser.add_argument(
        '--test', '-t',
        action='store_true',
        help='Test connection to Unreal Editor'
    )
    parser.add_argument(
        '--quiet', '-q',
        action='store_true',
        help='Suppress output (only show errors)'
    )
    parser.add_argument(
        '--raise-on-error',
        action='store_true',
        help='Exit with error code if execution fails'
    )

    args = parser.parse_args()

    # Test mode
    if args.test:
        success = test_connection(verbose=not args.quiet)
        return 0 if success else 1

    # File execution
    if args.file:
        try:
            result = execute_file_remote(
                args.file,
                verbose=not args.quiet,
                raise_on_error=args.raise_on_error
            )
            return 0 if result['success'] else 1
        except Exception as e:
            print(f"Error: {e}", file=sys.stderr)
            return 1

    # Command execution
    if args.command:
        result = execute_remote(
            args.command,
            verbose=not args.quiet,
            raise_on_error=args.raise_on_error
        )
        return 0 if result['success'] else 1

    # No command or file
    parser.print_help()
    return 1

if __name__ == '__main__':
    sys.exit(main())
