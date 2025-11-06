#!/usr/bin/env python3
"""
Visual Regression Testing for Alexander Comprehensive Test Level

Uses perceptual hashing to detect visual differences between test runs
Only flags significant changes for human review (5-10% of screenshots)

Features:
- Perceptual hash comparison (resistant to minor pixel differences)
- Configurable sensitivity thresholds
- Side-by-side diff generation for flagged images
- Automated pass/fail without human review
- HTML report with visual diffs

Requirements:
    pip install Pillow imagehash

Usage:
    python visual_regression_test.py --reference ReferenceScreenshots --test TestResults/screenshots
    python visual_regression_test.py --reference ReferenceScreenshots --test TestResults/screenshots --threshold 10
"""

import argparse
import json
import sys
from pathlib import Path
from typing import Dict, List, Tuple
from dataclasses import dataclass, asdict
import hashlib

try:
    import imagehash
    from PIL import Image, ImageDraw, ImageFont, ImageChops
except ImportError:
    print("ERROR: Required libraries not found!")
    print("Install with: pip install Pillow imagehash")
    sys.exit(1)


@dataclass
class ImageComparisonResult:
    """Result of comparing two images"""
    image_name: str
    status: str  # 'pass', 'fail', 'new', 'missing'
    hash_difference: int
    requires_human_review: bool
    reference_path: str = ""
    test_path: str = ""
    diff_path: str = ""
    notes: str = ""


class VisualRegressionTester:
    """Automated visual regression testing with intelligent flagging"""

    def __init__(self, reference_dir: Path, test_dir: Path, threshold: int = 5):
        self.reference_dir = Path(reference_dir)
        self.test_dir = Path(test_dir)
        self.threshold = threshold
        self.results: List[ImageComparisonResult] = []
        self.diff_dir = Path("VisualDiffs")
        self.diff_dir.mkdir(exist_ok=True)

    def compare_all_screenshots(self):
        """Compare all test screenshots against references"""
        print(f"Comparing screenshots...")
        print(f"  Reference: {self.reference_dir}")
        print(f"  Test: {self.test_dir}")
        print(f"  Threshold: {self.threshold}")
        print()

        if not self.test_dir.exists():
            print(f"ERROR: Test directory not found: {self.test_dir}")
            return

        test_images = list(self.test_dir.glob("*.png"))
        print(f"Found {len(test_images)} test images")

        for test_img_path in test_images:
            result = self.compare_screenshot(test_img_path)
            self.results.append(result)

            # Print progress
            status_symbol = {
                'pass': '✓',
                'fail': '✗',
                'new': '+',
                'missing': '?'
            }.get(result.status, '?')

            print(f"  {status_symbol} {result.image_name}: {result.status.upper()}", end="")
            if result.status in ['pass', 'fail']:
                print(f" (diff: {result.hash_difference})", end="")
            if result.requires_human_review:
                print(" [REVIEW NEEDED]", end="")
            print()

    def compare_screenshot(self, test_img_path: Path) -> ImageComparisonResult:
        """Compare single screenshot"""
        image_name = test_img_path.name
        ref_img_path = self.reference_dir / image_name

        # Check if reference exists
        if not ref_img_path.exists():
            return ImageComparisonResult(
                image_name=image_name,
                status='new',
                hash_difference=0,
                requires_human_review=True,
                test_path=str(test_img_path),
                notes="New screenshot - no reference available"
            )

        try:
            # Load images
            ref_img = Image.open(ref_img_path)
            test_img = Image.open(test_img_path)

            # Check dimensions
            if ref_img.size != test_img.size:
                return ImageComparisonResult(
                    image_name=image_name,
                    status='fail',
                    hash_difference=999,  # Max difference for size mismatch
                    requires_human_review=True,
                    reference_path=str(ref_img_path),
                    test_path=str(test_img_path),
                    notes=f"Size mismatch: {ref_img.size} vs {test_img.size}"
                )

            # Compute perceptual hashes
            hash_ref = imagehash.phash(ref_img, hash_size=16)  # 16x16 = 256-bit hash
            hash_test = imagehash.phash(test_img, hash_size=16)

            # Calculate Hamming distance
            difference = hash_ref - hash_test

            if difference <= self.threshold:
                # Images are perceptually similar - PASS
                return ImageComparisonResult(
                    image_name=image_name,
                    status='pass',
                    hash_difference=difference,
                    requires_human_review=False,
                    reference_path=str(ref_img_path),
                    test_path=str(test_img_path),
                    notes="Perceptually similar"
                )
            else:
                # Significant difference - generate diff and flag for review
                diff_path = self.generate_diff_image(ref_img, test_img, image_name)

                return ImageComparisonResult(
                    image_name=image_name,
                    status='fail',
                    hash_difference=difference,
                    requires_human_review=True,
                    reference_path=str(ref_img_path),
                    test_path=str(test_img_path),
                    diff_path=str(diff_path),
                    notes=f"Visual difference detected (hash diff: {difference})"
                )

        except Exception as e:
            return ImageComparisonResult(
                image_name=image_name,
                status='fail',
                hash_difference=999,
                requires_human_review=True,
                reference_path=str(ref_img_path),
                test_path=str(test_img_path),
                notes=f"Error: {str(e)}"
            )

    def generate_diff_image(self, ref_img: Image.Image, test_img: Image.Image, image_name: str) -> Path:
        """Generate side-by-side comparison with difference highlight"""
        # Create side-by-side comparison
        width, height = ref_img.size
        comparison = Image.new('RGB', (width * 3, height + 50))

        # Paste images
        comparison.paste(ref_img, (0, 50))
        comparison.paste(test_img, (width, 50))

        # Generate difference image
        diff = ImageChops.difference(ref_img.convert('RGB'), test_img.convert('RGB'))

        # Enhance difference for visibility
        diff = diff.point(lambda p: p * 10)  # Amplify differences
        comparison.paste(diff, (width * 2, 50))

        # Add labels
        draw = ImageDraw.Draw(comparison)
        try:
            font = ImageFont.truetype("arial.ttf", 20)
        except:
            font = ImageFont.load_default()

        draw.text((10, 10), "Reference", fill='white', font=font)
        draw.text((width + 10, 10), "Test", fill='white', font=font)
        draw.text((width * 2 + 10, 10), "Difference (10x)", fill='white', font=font)

        # Save comparison
        diff_path = self.diff_dir / f"diff_{image_name}"
        comparison.save(diff_path)

        return diff_path

    def get_human_review_items(self) -> List[ImageComparisonResult]:
        """Return only items requiring human review"""
        return [r for r in self.results if r.requires_human_review]

    def generate_statistics(self) -> Dict:
        """Generate test statistics"""
        total = len(self.results)
        if total == 0:
            return {
                "total_images": 0,
                "passed": 0,
                "failed": 0,
                "new": 0,
                "pass_rate": 0.0,
                "human_review_needed": 0,
                "human_review_percentage": 0.0
            }

        passed = sum(1 for r in self.results if r.status == 'pass')
        failed = sum(1 for r in self.results if r.status == 'fail')
        new = sum(1 for r in self.results if r.status == 'new')
        review_needed = len(self.get_human_review_items())

        return {
            "total_images": total,
            "passed": passed,
            "failed": failed,
            "new": new,
            "pass_rate": (passed / total) * 100,
            "human_review_needed": review_needed,
            "human_review_percentage": (review_needed / total) * 100
        }

    def generate_json_report(self, output_path: Path):
        """Generate JSON report"""
        report = {
            "timestamp": str(Path().resolve()),
            "reference_dir": str(self.reference_dir),
            "test_dir": str(self.test_dir),
            "threshold": self.threshold,
            "statistics": self.generate_statistics(),
            "results": [asdict(r) for r in self.results]
        }

        with open(output_path, 'w') as f:
            json.dump(report, f, indent=2)

        print(f"\nJSON report saved: {output_path}")

    def generate_html_report(self, output_path: Path):
        """Generate HTML report with visual diffs"""
        stats = self.generate_statistics()
        review_items = self.get_human_review_items()

        html = f"""<!DOCTYPE html>
<html>
<head>
    <title>Visual Regression Test Report</title>
    <style>
        body {{ font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }}
        h1 {{ color: #333; }}
        .stats {{ background: white; padding: 20px; border-radius: 8px; margin: 20px 0; }}
        .stat {{ display: inline-block; margin: 0 20px; }}
        .stat-value {{ font-size: 32px; font-weight: bold; }}
        .stat-label {{ color: #666; font-size: 14px; }}
        .pass {{ color: #4caf50; }}
        .fail {{ color: #f44336; }}
        .new {{ color: #ff9800; }}
        .results {{ background: white; padding: 20px; border-radius: 8px; }}
        .result-item {{ margin: 15px 0; padding: 15px; border: 1px solid #ddd; border-radius: 4px; }}
        .result-item.review {{ background: #fff3cd; border-color: #ffc107; }}
        .diff-image {{ max-width: 100%; margin: 10px 0; cursor: pointer; }}
        .diff-image:hover {{ opacity: 0.8; }}
    </style>
</head>
<body>
    <h1>Visual Regression Test Report</h1>

    <div class="stats">
        <div class="stat">
            <div class="stat-value">{stats['total_images']}</div>
            <div class="stat-label">Total Images</div>
        </div>
        <div class="stat">
            <div class="stat-value pass">{stats['passed']}</div>
            <div class="stat-label">Passed</div>
        </div>
        <div class="stat">
            <div class="stat-value fail">{stats['failed']}</div>
            <div class="stat-label">Failed</div>
        </div>
        <div class="stat">
            <div class="stat-value new">{stats['new']}</div>
            <div class="stat-label">New</div>
        </div>
        <div class="stat">
            <div class="stat-value">{stats['pass_rate']:.1f}%</div>
            <div class="stat-label">Pass Rate</div>
        </div>
        <div class="stat">
            <div class="stat-value">{stats['human_review_needed']}</div>
            <div class="stat-label">Human Review Needed</div>
        </div>
        <div class="stat">
            <div class="stat-value">{stats['human_review_percentage']:.1f}%</div>
            <div class="stat-label">Review Percentage</div>
        </div>
    </div>

    <div class="results">
        <h2>Items Requiring Human Review ({len(review_items)})</h2>
"""

        for result in review_items:
            status_class = result.status
            html += f"""
        <div class="result-item review">
            <h3 class="{status_class}">{result.image_name} - {result.status.upper()}</h3>
            <p><strong>Hash Difference:</strong> {result.hash_difference} (threshold: {self.threshold})</p>
            <p><strong>Notes:</strong> {result.notes}</p>
"""
            if result.diff_path:
                html += f"""
            <img src="{result.diff_path}" class="diff-image" alt="Visual diff"
                 onclick="window.open(this.src, '_blank')"
                 title="Click to open full size">
"""
            html += """
        </div>
"""

        # Add all results summary
        html += """
        <h2>All Results</h2>
"""
        for result in self.results:
            review_class = "review" if result.requires_human_review else ""
            status_class = result.status
            html += f"""
        <div class="result-item {review_class}">
            <strong class="{status_class}">{result.image_name}</strong>:
            {result.status.upper()}
            (diff: {result.hash_difference})
            {' [REVIEW NEEDED]' if result.requires_human_review else ''}
        </div>
"""

        html += """
    </div>
</body>
</html>
"""

        with open(output_path, 'w') as f:
            f.write(html)

        print(f"HTML report saved: {output_path}")

    def print_summary(self):
        """Print summary to console"""
        stats = self.generate_statistics()

        print("\n" + "=" * 80)
        print("VISUAL REGRESSION TEST SUMMARY")
        print("=" * 80)
        print(f"Total Images: {stats['total_images']}")
        print(f"Passed: {stats['passed']} ({stats['pass_rate']:.1f}%)")
        print(f"Failed: {stats['failed']}")
        print(f"New: {stats['new']}")
        print(f"\nHuman Review Needed: {stats['human_review_needed']} ({stats['human_review_percentage']:.1f}%)")
        print("=" * 80)

        # Return exit code
        if stats['failed'] > 0 and stats['human_review_needed'] > stats['new']:
            return 1  # Failures beyond just new images
        return 0


def main():
    parser = argparse.ArgumentParser(
        description='Visual regression testing for Unreal Engine screenshots'
    )
    parser.add_argument('--reference', required=True, help='Reference screenshots directory')
    parser.add_argument('--test', required=True, help='Test screenshots directory')
    parser.add_argument('--threshold', type=int, default=5,
                       help='Perceptual hash difference threshold (default: 5)')
    parser.add_argument('--output', default='visual_regression_report',
                       help='Output report filename (without extension)')

    args = parser.parse_args()

    # Create tester
    tester = VisualRegressionTester(
        reference_dir=Path(args.reference),
        test_dir=Path(args.test),
        threshold=args.threshold
    )

    # Run comparison
    tester.compare_all_screenshots()

    # Generate reports
    tester.generate_json_report(Path(f"{args.output}.json"))
    tester.generate_html_report(Path(f"{args.output}.html"))

    # Print summary and exit
    exit_code = tester.print_summary()
    sys.exit(exit_code)


if __name__ == "__main__":
    main()
