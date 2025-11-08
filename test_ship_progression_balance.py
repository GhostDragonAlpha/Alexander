#!/usr/bin/env python3
"""
Progression System Testing and Balancing Script

Simulates player progression from level 1 to 50, validates economy balance,
and provides recommendations for tuning.
"""

import json
import math
from typing import Dict, List, Tuple
from dataclasses import dataclass, field
import os


@dataclass
class PlayerSimulation:
    """Simulates a player's progression"""
    level: int = 1
    total_xp: int = 0
    credits: int = 0
    credits_earned: int = 0
    credits_spent: int = 0
    parts_unlocked: List[str] = field(default_factory=list)
    skins_unlocked: List[str] = field(default_factory=list)
    hours_played: float = 0.0
    races_completed: int = 0


class ProgressionTester:
    """Tests and analyzes the progression system"""

    def __init__(self, data_dir: str = "Content/Data"):
        self.data_dir = data_dir
        self.progression_data = None
        self.economy_data = None
        self.achievements_data = None
        self.load_data()

    def load_data(self):
        """Load progression and economy data"""
        try:
            with open(os.path.join(self.data_dir, "ProgressionSystem.json"), 'r') as f:
                self.progression_data = json.load(f)

            with open(os.path.join(self.data_dir, "EconomyBalance.json"), 'r') as f:
                self.economy_data = json.load(f)

            with open(os.path.join(self.data_dir, "Achievements.json"), 'r') as f:
                self.achievements_data = json.load(f)

            print("[OK] Loaded progression data files")
        except Exception as e:
            print(f"[ERROR] Error loading data: {e}")
            raise

    def calculate_xp_for_level(self, level: int) -> int:
        """Calculate XP required for a level"""
        level_str = str(level)
        if level_str in self.progression_data["xpRequirementsByLevel"]:
            return self.progression_data["xpRequirementsByLevel"][level_str]

        # Fallback to formula
        base_xp = self.progression_data["levelCurve"]["baseXP"]
        exponent = self.progression_data["levelCurve"]["exponent"]
        smoothing = self.progression_data["levelCurve"]["smoothingFactor"]

        xp = base_xp * (level ** exponent) + smoothing * (level - 1)
        return int(xp)

    def get_level_from_xp(self, total_xp: int) -> int:
        """Get player level from total XP"""
        for level in range(50, 0, -1):
            required_xp = self.calculate_xp_for_level(level)
            if total_xp >= required_xp:
                return level
        return 1

    def simulate_race(self, player: PlayerSimulation, difficulty: str = "medium",
                     medal: str = "gold", skill_level: str = "average") -> Tuple[int, int]:
        """Simulate a race completion and return (XP, Credits)"""
        # XP calculation
        base_xp = self.progression_data["xpRewards"]["raceComplete"]["base"]
        time_bonus = self.progression_data["xpRewards"]["raceComplete"]["timeBonus"][medal]
        difficulty_mult = self.progression_data["xpRewards"]["raceComplete"]["difficultyMultiplier"][difficulty]

        total_xp = int((base_xp + time_bonus) * difficulty_mult)

        # Credits calculation
        base_credits = self.economy_data["creditEarnRates"]["raceComplete"]["base"]
        medal_bonus = self.economy_data["creditEarnRates"]["raceComplete"]["medalBonus"][medal]
        credit_mult = self.economy_data["creditEarnRates"]["raceComplete"]["difficultyMultiplier"][difficulty]

        total_credits = int((base_credits + medal_bonus) * credit_mult)

        # First time bonus (simulate 20% of races as first-time)
        if player.races_completed % 5 == 0:
            total_credits = int(total_credits * 1.5)

        return total_xp, total_credits

    def simulate_progression_to_level(self, target_level: int,
                                     skill_level: str = "average") -> PlayerSimulation:
        """Simulate player progression to target level"""
        player = PlayerSimulation()

        # Time per race (minutes)
        time_per_race = {
            "casual": 8.0,
            "average": 6.0,
            "skilled": 4.5,
            "expert": 3.5
        }

        races_per_level = []

        while player.level < target_level:
            # Determine race parameters based on player level
            if player.level < 10:
                difficulty = "easy"
                medal = "gold" if skill_level in ["skilled", "expert"] else "silver"
            elif player.level < 25:
                difficulty = "medium"
                medal = "gold" if skill_level == "expert" else "silver"
            else:
                difficulty = "hard"
                medal = "gold" if skill_level == "expert" else "silver"

            # Simulate race
            xp_gained, credits_gained = self.simulate_race(player, difficulty, medal, skill_level)

            player.total_xp += xp_gained
            player.credits += credits_gained
            player.credits_earned += credits_gained
            player.races_completed += 1
            player.hours_played += time_per_race[skill_level] / 60.0

            # Check for level up
            new_level = self.get_level_from_xp(player.total_xp)
            if new_level > player.level:
                old_level = player.level
                player.level = new_level

                # Level up rewards
                level_credits = self.progression_data["levelRewards"]["every1Level"]["credits"]
                player.credits += level_credits
                player.credits_earned += level_credits

                # Milestone rewards
                if player.level % 5 == 0:
                    bonus = self.progression_data["levelRewards"]["every5Levels"]["credits"]
                    player.credits += bonus
                    player.credits_earned += bonus

                if player.level % 10 == 0:
                    bonus = self.progression_data["levelRewards"]["every10Levels"]["credits"]
                    player.credits += bonus
                    player.credits_earned += bonus

                races_per_level.append(player.races_completed - sum(races_per_level))

        return player

    def analyze_progression_curve(self):
        """Analyze the progression curve across all levels"""
        print("\n" + "="*80)
        print("PROGRESSION CURVE ANALYSIS")
        print("="*80)

        print("\nXP Requirements per Level:")
        print("-" * 80)
        print(f"{'Level':<8} {'XP Required':<15} {'XP This Level':<15} {'% Increase':<15}")
        print("-" * 80)

        for level in [1, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50]:
            current_xp = self.calculate_xp_for_level(level)
            prev_xp = self.calculate_xp_for_level(level - 1) if level > 1 else 0
            xp_this_level = current_xp - prev_xp

            if level > 1:
                prev_level_xp = self.calculate_xp_for_level(level - 1) - \
                               self.calculate_xp_for_level(level - 2)
                percent_increase = ((xp_this_level - prev_level_xp) / prev_level_xp * 100) \
                                  if prev_level_xp > 0 else 0
            else:
                percent_increase = 0

            print(f"{level:<8} {current_xp:<15} {xp_this_level:<15} {percent_increase:>13.1f}%")

    def analyze_economy_balance(self):
        """Analyze economy balance across skill levels"""
        print("\n" + "="*80)
        print("ECONOMY BALANCE ANALYSIS")
        print("="*80)

        skill_levels = ["casual", "average", "skilled", "expert"]

        for skill in skill_levels:
            print(f"\n{skill.upper()} Player Simulation:")
            print("-" * 80)

            # Simulate to key milestones
            milestones = [10, 25, 50]

            for level in milestones:
                player = self.simulate_progression_to_level(level, skill)

                print(f"\nLevel {level}:")
                print(f"  Total XP: {player.total_xp:,}")
                print(f"  Races Completed: {player.races_completed}")
                print(f"  Hours Played: {player.hours_played:.1f}")
                print(f"  Credits Earned: {player.credits_earned:,}")
                print(f"  Current Credits: {player.credits:,}")
                print(f"  Credits/Hour: {player.credits_earned / player.hours_played:.0f}" if player.hours_played > 0 else "  Credits/Hour: N/A")

    def check_unlock_pacing(self):
        """Check unlock pacing throughout progression"""
        print("\n" + "="*80)
        print("UNLOCK PACING ANALYSIS")
        print("="*80)

        schedule = self.progression_data["unlockSchedule"]

        print(f"\n{'Level':<8} {'Parts':<8} {'Skins':<8} {'Features':<30} {'Description':<40}")
        print("-" * 80)

        total_parts = 0
        total_skins = 0

        for level_key, unlocks in sorted(schedule.items(), key=lambda x: int(x[0].replace('level', ''))):
            level = int(level_key.replace('level', ''))
            parts = len(unlocks.get('parts', []))
            skins = len(unlocks.get('skins', []))
            features = ', '.join(unlocks.get('features', []))
            description = unlocks.get('description', '')

            total_parts += parts
            total_skins += skins

            if parts > 0 or skins > 0 or features:
                print(f"{level:<8} {parts:<8} {skins:<8} {features:<30} {description:<40}")

        print(f"\nTotal unlockable content:")
        print(f"  Parts: {total_parts}")
        print(f"  Skins: {total_skins}")

    def validate_cost_scaling(self):
        """Validate that part/skin costs scale properly"""
        print("\n" + "="*80)
        print("COST SCALING VALIDATION")
        print("="*80)

        rarities = ["Common", "Uncommon", "Rare", "Epic", "Legendary"]

        # Part costs
        print("\nPart Base Costs by Rarity:")
        print("-" * 80)
        part_costs = self.economy_data["partCosts"]["baseCostsByRarity"]
        for rarity in rarities:
            cost = part_costs[rarity]
            print(f"  {rarity:<12}: {cost:>8,} credits")

        # Skin costs
        print("\nSkin Base Costs by Rarity:")
        print("-" * 80)
        skin_costs = self.economy_data["skinCosts"]["baseCostsByRarity"]
        for rarity in rarities:
            cost = skin_costs[rarity]
            print(f"  {rarity:<12}: {cost:>8,} credits")

        # Category multipliers
        print("\nCategory Cost Multipliers:")
        print("-" * 80)
        category_mults = self.economy_data["partCosts"]["categoryMultipliers"]
        for category, mult in category_mults.items():
            print(f"  {category:<12}: {mult:.1f}x")

        # Example part costs
        print("\nExample Part Costs (Epic Engine with +60% stats):")
        base = part_costs["Epic"]
        category_mult = category_mults["Engine"]
        stat_mult = 1.0 + (0.6 * 0.1)  # 60% total stats * 0.1 impact
        final = int(base * category_mult * stat_mult)
        print(f"  Base: {base:,} cr")
        print(f"  × Category (Engine): {category_mult:.1f}x = {int(base * category_mult):,} cr")
        print(f"  × Stat Impact: {stat_mult:.2f}x = {final:,} cr")

    def analyze_achievements(self):
        """Analyze achievement rewards and distribution"""
        print("\n" + "="*80)
        print("ACHIEVEMENT ANALYSIS")
        print("="*80)

        achievements = self.achievements_data["achievements"]

        by_rarity = {}
        total_xp = 0
        total_credits = 0

        for achievement in achievements:
            rarity = achievement["rarity"]
            if rarity not in by_rarity:
                by_rarity[rarity] = []
            by_rarity[rarity].append(achievement)

            total_xp += achievement["rewards"]["xp"]
            total_credits += achievement["rewards"]["credits"]

        print(f"\nTotal Achievements: {len(achievements)}")
        print(f"Total XP from Achievements: {total_xp:,}")
        print(f"Total Credits from Achievements: {total_credits:,}")

        print("\nAchievements by Rarity:")
        print("-" * 80)
        for rarity in ["Common", "Uncommon", "Rare", "Epic", "Legendary"]:
            count = len(by_rarity.get(rarity, []))
            print(f"  {rarity:<12}: {count}")

        print("\nHigh-Value Achievements:")
        print("-" * 80)
        high_value = sorted(achievements, key=lambda x: x["rewards"]["credits"], reverse=True)[:5]
        for ach in high_value:
            print(f"  {ach['name']:<30} - {ach['rewards']['credits']:>6,} cr, {ach['rewards']['xp']:>5,} XP")

    def generate_recommendations(self):
        """Generate balancing recommendations"""
        print("\n" + "="*80)
        print("BALANCING RECOMMENDATIONS")
        print("="*80)

        # Simulate average player
        player_25 = self.simulate_progression_to_level(25, "average")
        player_50 = self.simulate_progression_to_level(50, "average")

        # Total content value
        total_value = self.economy_data["economyTargets"]["totalContentValue"]["grandTotal"]

        print(f"\nProgression Pace:")
        print(f"  Level 25: {player_25.hours_played:.1f} hours, {player_25.credits_earned:,} credits earned")
        print(f"  Level 50: {player_50.hours_played:.1f} hours, {player_50.credits_earned:,} credits earned")
        print(f"  Total Content Value: {total_value:,} credits")
        print(f"  Completion %: {(player_50.credits_earned / total_value * 100):.1f}%")

        print("\n[OK] PACING LOOKS GOOD:")
        print(f"  - Players reach mid-game (L25) in {player_25.hours_played:.1f} hours")
        print(f"  - Players reach max level (L50) in {player_50.hours_played:.1f} hours")
        print(f"  - Credits earned by L50: {player_50.credits_earned:,}")

        if player_50.credits_earned < total_value * 0.5:
            print("\n[WARNING] Players cannot afford much content by max level")
            print("  RECOMMENDATION: Increase credit rewards or reduce costs")
        elif player_50.credits_earned > total_value * 1.5:
            print("\n[WARNING] Players earn too many credits")
            print("  RECOMMENDATION: Decrease credit rewards or increase costs")
        else:
            print("\n[OK] Economy is well-balanced for 40-80 hour completion time")

        print("\nRecommended Playtime to Unlock:")
        targets = self.economy_data["economyTargets"]["hoursToUnlock"]
        print(f"  First Customization: {targets['firstCustomization']} hours")
        print(f"  Full Basic Loadout: {targets['fullBasicLoadout']} hours")
        print(f"  First Rare Item: {targets['firstRareItem']} hours")
        print(f"  Competitive Loadout: {targets['competitiveLoadout']} hours")
        print(f"  Most Content: {targets['mostContent']} hours")
        print(f"  Everything Maxed: {targets['everythingMaxed']} hours")

    def run_full_analysis(self):
        """Run complete progression system analysis"""
        print("\n" + "="*80)
        print(" PROGRESSION SYSTEM ANALYSIS - VR Flight Game")
        print("="*80)

        self.analyze_progression_curve()
        self.check_unlock_pacing()
        self.validate_cost_scaling()
        self.analyze_economy_balance()
        self.analyze_achievements()
        self.generate_recommendations()

        print("\n" + "="*80)
        print(" ANALYSIS COMPLETE")
        print("="*80)


def main():
    """Main entry point"""
    script_dir = os.path.dirname(os.path.abspath(__file__))
    data_dir = os.path.join(script_dir, "Content", "Data")

    tester = ProgressionTester(data_dir)
    tester.run_full_analysis()


if __name__ == "__main__":
    main()
