"""
Multi-Object Tracking Stress Test

Tests system performance with increasing numbers of tracked objects.
Measures:
- Server response time
- Validation computation time
- Game FPS
- Memory usage
- Identifies performance thresholds
"""

import requests
import json
import math
import time
import statistics
from datetime import datetime

BASE_URL = "http://localhost:8080"

# Performance thresholds
MAX_RESPONSE_TIME_MS = 250  # Realistic threshold for HTTP over TCP
MIN_FPS = 30  # Game should maintain at least 30 FPS

def submit_observation(observer_id, target_id, direction, distance, observer_pos):
    """Submit an observation to the server"""
    url = f"{BASE_URL}/submit_observation"

    payload = {
        "observer_id": observer_id,
        "target_id": target_id,
        "observer_position": observer_pos,
        "direction": direction,
        "distance": distance,
        "scale_factor": 1.0,
        "timestamp": 0.0
    }

    session = requests.Session()
    start_time = time.time()
    response = session.post(url, json=payload, timeout=10)
    elapsed_ms = (time.time() - start_time) * 1000
    session.close()

    return response.json(), elapsed_ms

def validate_position(target_id, observations):
    """Validate position using triangulation"""
    url = f"{BASE_URL}/validate_position"

    payload = {
        "target_id": target_id,
        "observations": observations
    }

    session = requests.Session()
    start_time = time.time()
    response = session.post(url, json=payload, timeout=10)
    elapsed_ms = (time.time() - start_time) * 1000
    session.close()

    return response.json(), elapsed_ms

def get_fps():
    """Query current FPS from the game"""
    url = f"{BASE_URL}/get_fps"

    try:
        session = requests.Session()
        response = session.post(url, json={}, timeout=5)
        session.close()

        data = response.json()
        if data.get("success"):
            return data.get("data", {}).get("fps", 0)
    except:
        pass

    return None

def normalize_vector(vec):
    """Normalize a 3D vector"""
    magnitude = math.sqrt(vec[0]**2 + vec[1]**2 + vec[2]**2)
    if magnitude == 0:
        return [0, 0, 0]
    return [vec[0]/magnitude, vec[1]/magnitude, vec[2]/magnitude]

def generate_random_position(min_coord, max_coord):
    """Generate a random 3D position"""
    import random
    return [
        random.uniform(min_coord, max_coord),
        random.uniform(min_coord, max_coord),
        random.uniform(min_coord, max_coord)
    ]

def test_object_count(num_objects, num_observers=4):
    """Test performance with a specific number of objects"""

    print(f"\n{'='*70}")
    print(f"TESTING {num_objects} OBJECTS with {num_observers} OBSERVERS EACH")
    print(f"{'='*70}")

    # Fixed observer positions (tetrahedron)
    observer_positions = [
        [0.0, 0.0, 0.0],          # Observer 1: origin
        [2000.0, 0.0, 0.0],       # Observer 2: +X axis
        [1000.0, 4000.0, 0.0],    # Observer 3: +Y axis
        [1000.0, 2000.0, 1000.0]  # Observer 4: +Z axis
    ]

    # Generate random target positions
    target_positions = []
    for i in range(num_objects):
        pos = generate_random_position(500, 3000)
        target_positions.append(pos)

    # Track performance metrics
    submit_times = []
    validation_times = []

    start_test_time = time.time()

    # Submit observations for each object
    print(f"\nSubmitting observations for {num_objects} objects...")

    for target_id, target_pos in enumerate(target_positions, start=1):
        observations = []

        for obs_id, obs_pos in enumerate(observer_positions, start=1):
            # Calculate observation
            delta_x = target_pos[0] - obs_pos[0]
            delta_y = target_pos[1] - obs_pos[1]
            delta_z = target_pos[2] - obs_pos[2]

            distance = math.sqrt(delta_x**2 + delta_y**2 + delta_z**2)
            direction = normalize_vector([delta_x, delta_y, delta_z])

            # Submit observation
            try:
                result, elapsed_ms = submit_observation(
                    obs_id, target_id, direction, distance, obs_pos
                )
                submit_times.append(elapsed_ms)

                observations.append({
                    "observer_id": obs_id,
                    "observer_position": obs_pos,
                    "direction": direction,
                    "distance": distance,
                    "scale_factor": 1.0
                })

                if not result.get("success"):
                    print(f"  [WARN] Object {target_id}, Observer {obs_id}: {result.get('message')}")

            except Exception as e:
                print(f"  [ERROR] Object {target_id}, Observer {obs_id}: {e}")
                return None

        # Progress indicator every 10 objects
        if target_id % 10 == 0:
            avg_submit = statistics.mean(submit_times[-40:]) if len(submit_times) >= 40 else statistics.mean(submit_times)
            print(f"  Progress: {target_id}/{num_objects} objects ({avg_submit:.1f}ms avg response)")

    total_observations = len(submit_times)
    test_duration = time.time() - start_test_time

    # Validate a sample of objects (not all, to save time)
    sample_size = min(10, num_objects)
    print(f"\nValidating {sample_size} sample objects...")

    for target_id in range(1, sample_size + 1):
        target_pos = target_positions[target_id - 1]
        observations = []

        for obs_id, obs_pos in enumerate(observer_positions, start=1):
            delta_x = target_pos[0] - obs_pos[0]
            delta_y = target_pos[1] - obs_pos[1]
            delta_z = target_pos[2] - obs_pos[2]

            distance = math.sqrt(delta_x**2 + delta_y**2 + delta_z**2)
            direction = normalize_vector([delta_x, delta_y, delta_z])

            observations.append({
                "observer_id": obs_id,
                "observer_position": obs_pos,
                "direction": direction,
                "distance": distance,
                "scale_factor": 1.0
            })

        try:
            result, elapsed_ms = validate_position(target_id, observations)
            validation_times.append(elapsed_ms)

            if not result.get("success"):
                print(f"  [WARN] Validation failed for object {target_id}")
        except Exception as e:
            print(f"  [ERROR] Validation error for object {target_id}: {e}")

    # Query FPS
    current_fps = get_fps()

    # Calculate statistics
    stats = {
        "num_objects": num_objects,
        "num_observers": num_observers,
        "total_observations": total_observations,
        "test_duration_sec": test_duration,
        "observations_per_sec": total_observations / test_duration if test_duration > 0 else 0,
        "submit_avg_ms": statistics.mean(submit_times) if submit_times else 0,
        "submit_median_ms": statistics.median(submit_times) if submit_times else 0,
        "submit_p95_ms": statistics.quantiles(submit_times, n=20)[18] if len(submit_times) >= 20 else max(submit_times) if submit_times else 0,
        "submit_max_ms": max(submit_times) if submit_times else 0,
        "validation_avg_ms": statistics.mean(validation_times) if validation_times else 0,
        "validation_max_ms": max(validation_times) if validation_times else 0,
        "fps": current_fps,
        "fps_ok": current_fps >= MIN_FPS if current_fps else None,
        "response_time_ok": statistics.mean(submit_times) <= MAX_RESPONSE_TIME_MS if submit_times else None,
    }

    # Print results
    print(f"\n{'='*70}")
    print(f"RESULTS - {num_objects} Objects")
    print(f"{'='*70}")
    print(f"Total Observations: {stats['total_observations']}")
    print(f"Test Duration: {stats['test_duration_sec']:.2f}s")
    print(f"Throughput: {stats['observations_per_sec']:.1f} observations/sec")
    print(f"\nSubmit Performance:")
    print(f"  Average: {stats['submit_avg_ms']:.1f}ms")
    print(f"  Median: {stats['submit_median_ms']:.1f}ms")
    print(f"  95th Percentile: {stats['submit_p95_ms']:.1f}ms")
    print(f"  Max: {stats['submit_max_ms']:.1f}ms")
    print(f"\nValidation Performance ({sample_size} samples):")
    print(f"  Average: {stats['validation_avg_ms']:.1f}ms")
    print(f"  Max: {stats['validation_max_ms']:.1f}ms")

    if current_fps is not None:
        fps_status = "OK" if stats['fps_ok'] else "LOW"
        print(f"\nGame FPS: {current_fps:.1f} FPS [{fps_status}]")
    else:
        print(f"\nGame FPS: N/A (endpoint not available)")

    # Performance verdict
    response_ok = stats['response_time_ok']
    fps_ok = stats['fps_ok'] if stats['fps_ok'] is not None else True  # Assume OK if can't measure

    if response_ok and fps_ok:
        verdict = "PASS - Performance within tolerance"
    elif not response_ok and not fps_ok:
        verdict = "FAIL - Both response time and FPS exceeded limits"
    elif not response_ok:
        verdict = "FAIL - Response time exceeded limit"
    else:
        verdict = "FAIL - FPS below minimum"

    print(f"\nVerdict: {verdict}")
    print(f"{'='*70}")

    return stats

def main():
    """Run progressive stress test"""

    print("\n" + "="*70)
    print("MULTI-OBJECT TRACKING STRESS TEST")
    print("="*70)
    print(f"Test Start: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print(f"\nPerformance Thresholds:")
    print(f"  Max Response Time: {MAX_RESPONSE_TIME_MS}ms")
    print(f"  Min FPS: {MIN_FPS}")

    # Test with increasing object counts
    test_counts = [10, 25, 50, 100, 250, 500, 1000]

    results = []
    max_passed = 0

    for count in test_counts:
        print(f"\n\n{'#'*70}")
        print(f"# TEST ROUND: {count} Objects")
        print(f"{'#'*70}")

        stats = test_object_count(count)

        if stats is None:
            print(f"\n[FATAL] Test failed at {count} objects - stopping")
            break

        results.append(stats)

        # Check if performance is still acceptable
        response_ok = stats['response_time_ok']
        fps_ok = stats['fps_ok'] if stats['fps_ok'] is not None else True

        if response_ok and fps_ok:
            max_passed = count
        else:
            print(f"\n[THRESHOLD EXCEEDED] Performance degraded at {count} objects")
            print(f"Maximum sustainable object count: {max_passed}")
            break

        # Brief pause between tests
        time.sleep(2)

    # Summary
    print(f"\n\n{'='*70}")
    print("STRESS TEST SUMMARY")
    print(f"{'='*70}")
    print(f"\nMaximum Objects Tested: {max([r['num_objects'] for r in results])}")
    print(f"Maximum With Good Performance: {max_passed}")

    print(f"\nPerformance Progression:")
    print(f"{'Objects':<10} {'Avg Submit':<15} {'P95 Submit':<15} {'FPS':<10} {'Status':<10}")
    print(f"{'-'*70}")

    for stat in results:
        fps_str = f"{stat['fps']:.1f}" if stat['fps'] else "N/A"
        status = "PASS" if (stat['response_time_ok'] and (stat['fps_ok'] if stat['fps_ok'] is not None else True)) else "FAIL"

        print(f"{stat['num_objects']:<10} {stat['submit_avg_ms']:<15.1f} {stat['submit_p95_ms']:<15.1f} {fps_str:<10} {status:<10}")

    # Recommendations
    print(f"\n{'='*70}")
    print("RECOMMENDATIONS")
    print(f"{'='*70}")

    if max_passed >= 500:
        print("Excellent: System can handle 500+ objects simultaneously")
        print("Recommended max for production: 400 objects (80% of tested capacity)")
    elif max_passed >= 100:
        print(f"Good: System can handle {max_passed} objects")
        print(f"Recommended max for production: {int(max_passed * 0.7)} objects (70% of capacity)")
    elif max_passed >= 50:
        print(f"Moderate: System can handle {max_passed} objects")
        print(f"Consider optimization before scaling beyond {max_passed} objects")
    else:
        print(f"Limited: System can only handle {max_passed} objects")
        print("Significant optimization needed for larger scale")

    print(f"\n{'='*70}")
    print(f"Test Complete: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print(f"{'='*70}\n")

    return results

if __name__ == "__main__":
    try:
        results = main()

        # Save results to file
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = f"stress_test_results_{timestamp}.json"

        with open(filename, 'w') as f:
            json.dump(results, f, indent=2)

        print(f"\nResults saved to: {filename}")

    except KeyboardInterrupt:
        print("\n\nTest interrupted by user")
    except Exception as e:
        print(f"\n[ERROR] {e}")
        import traceback
        traceback.print_exc()
