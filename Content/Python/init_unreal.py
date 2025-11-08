"""
AUTONOMOUS DEVELOPMENT SYSTEM WITH VALIDATION
Automatically runs validation on Unreal startup
"""
import unreal

print("=" * 80)
print("AUTONOMOUS DEVELOPMENT SYSTEM WITH VALIDATION")
print("=" * 80)

# Run autonomous validation
try:
    exec(open('Content/Python/autonomous_startup.py').read())
    print("AUTONOMOUS VALIDATION COMPLETED")
except Exception as e:
    print(f"AUTONOMOUS VALIDATION ERROR: {e}")

print("=" * 80)
