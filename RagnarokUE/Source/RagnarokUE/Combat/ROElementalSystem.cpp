// Copyright Ragna-TH Project. All Rights Reserved.

#include "ROElementalSystem.h"

/**
 * Full Ragnarok Online Elemental Modifier Table.
 *
 * Indices: [AttackElement][DefenseElement][DefLevel]
 *   AttackElement order: Neutral=0, Water=1, Earth=2, Fire=3, Wind=4, Poison=5, Holy=6, Shadow=7, Ghost=8, Undead=9
 *   DefenseElement order: same as above
 *   DefLevel: 0=Lv1, 1=Lv2, 2=Lv3, 3=Lv4
 *
 * Values represent the damage multiplier (percentage / 100).
 * Based on the official iRO elemental table.
 */
const float UROElementalSystem::ElementalTable[10][10][4] =
{
	// Attack: Neutral (0)
	{
		// vs Neutral Lv1-4
		{ 1.00f, 1.00f, 1.00f, 1.00f },
		// vs Water Lv1-4
		{ 1.00f, 1.00f, 1.00f, 1.00f },
		// vs Earth Lv1-4
		{ 1.00f, 1.00f, 1.00f, 1.00f },
		// vs Fire Lv1-4
		{ 1.00f, 1.00f, 1.00f, 1.00f },
		// vs Wind Lv1-4
		{ 1.00f, 1.00f, 1.00f, 1.00f },
		// vs Poison Lv1-4
		{ 1.00f, 1.00f, 1.00f, 1.00f },
		// vs Holy Lv1-4
		{ 1.00f, 1.00f, 1.00f, 1.00f },
		// vs Shadow Lv1-4
		{ 1.00f, 1.00f, 1.00f, 1.00f },
		// vs Ghost Lv1-4
		{ 0.25f, 0.00f, 0.00f, 0.00f },
		// vs Undead Lv1-4
		{ 1.00f, 1.00f, 1.00f, 1.00f },
	},
	// Attack: Water (1)
	{
		// vs Neutral
		{ 1.00f, 1.00f, 1.00f, 1.00f },
		// vs Water
		{ 0.25f, 0.00f, -0.25f, -0.50f },
		// vs Earth
		{ 1.00f, 1.00f, 1.00f, 1.00f },
		// vs Fire
		{ 1.50f, 1.75f, 2.00f, 2.00f },
		// vs Wind
		{ 0.75f, 0.50f, 0.25f, 0.00f },
		// vs Poison
		{ 1.00f, 0.75f, 0.50f, 0.25f },
		// vs Holy
		{ 0.75f, 0.50f, 0.25f, 0.00f },
		// vs Shadow
		{ 1.00f, 0.75f, 0.50f, 0.25f },
		// vs Ghost
		{ 1.00f, 0.75f, 0.50f, 0.25f },
		// vs Undead
		{ 1.00f, 1.00f, 1.25f, 1.50f },
	},
	// Attack: Earth (2)
	{
		// vs Neutral
		{ 1.00f, 1.00f, 1.00f, 1.00f },
		// vs Water
		{ 1.00f, 1.00f, 1.00f, 1.00f },
		// vs Earth
		{ 0.25f, 0.00f, -0.25f, -0.50f },
		// vs Fire
		{ 0.75f, 0.50f, 0.25f, 0.00f },
		// vs Wind
		{ 1.50f, 1.75f, 2.00f, 2.00f },
		// vs Poison
		{ 1.00f, 0.75f, 0.50f, 0.25f },
		// vs Holy
		{ 0.75f, 0.50f, 0.25f, 0.00f },
		// vs Shadow
		{ 1.00f, 0.75f, 0.50f, 0.25f },
		// vs Ghost
		{ 1.00f, 0.75f, 0.50f, 0.25f },
		// vs Undead
		{ 1.00f, 1.00f, 1.00f, 1.00f },
	},
	// Attack: Fire (3)
	{
		// vs Neutral
		{ 1.00f, 1.00f, 1.00f, 1.00f },
		// vs Water
		{ 0.50f, 0.25f, 0.00f, -0.25f },
		// vs Earth
		{ 1.50f, 1.75f, 2.00f, 2.00f },
		// vs Fire
		{ 0.25f, 0.00f, -0.25f, -0.50f },
		// vs Wind
		{ 1.00f, 1.00f, 1.00f, 1.00f },
		// vs Poison
		{ 1.00f, 0.75f, 0.50f, 0.25f },
		// vs Holy
		{ 0.75f, 0.50f, 0.25f, 0.00f },
		// vs Shadow
		{ 1.00f, 0.75f, 0.50f, 0.25f },
		// vs Ghost
		{ 1.00f, 0.75f, 0.50f, 0.25f },
		// vs Undead
		{ 1.25f, 1.50f, 1.75f, 2.00f },
	},
	// Attack: Wind (4)
	{
		// vs Neutral
		{ 1.00f, 1.00f, 1.00f, 1.00f },
		// vs Water
		{ 1.75f, 2.00f, 2.00f, 2.00f },
		// vs Earth
		{ 0.50f, 0.25f, 0.00f, -0.25f },
		// vs Fire
		{ 1.00f, 1.00f, 1.00f, 1.00f },
		// vs Wind
		{ 0.25f, 0.00f, -0.25f, -0.50f },
		// vs Poison
		{ 1.00f, 0.75f, 0.50f, 0.25f },
		// vs Holy
		{ 0.75f, 0.50f, 0.25f, 0.00f },
		// vs Shadow
		{ 1.00f, 0.75f, 0.50f, 0.25f },
		// vs Ghost
		{ 1.00f, 0.75f, 0.50f, 0.25f },
		// vs Undead
		{ 1.00f, 1.00f, 1.25f, 1.50f },
	},
	// Attack: Poison (5)
	{
		// vs Neutral
		{ 1.00f, 1.00f, 1.00f, 1.00f },
		// vs Water
		{ 1.00f, 1.00f, 1.00f, 1.00f },
		// vs Earth
		{ 1.00f, 1.00f, 1.00f, 1.00f },
		// vs Fire
		{ 1.00f, 1.00f, 1.00f, 1.00f },
		// vs Wind
		{ 1.00f, 1.00f, 1.00f, 1.00f },
		// vs Poison
		{ 0.00f, 0.00f, 0.00f, -0.25f },
		// vs Holy
		{ 0.50f, 0.25f, 0.00f, 0.00f },
		// vs Shadow
		{ 0.50f, 0.25f, 0.00f, -0.25f },
		// vs Ghost
		{ 1.00f, 0.75f, 0.50f, 0.25f },
		// vs Undead
		{ -0.25f, -0.50f, -0.75f, -1.00f },
	},
	// Attack: Holy (6)
	{
		// vs Neutral
		{ 1.00f, 1.00f, 1.00f, 1.00f },
		// vs Water
		{ 0.75f, 0.50f, 0.25f, 0.00f },
		// vs Earth
		{ 0.75f, 0.50f, 0.25f, 0.00f },
		// vs Fire
		{ 0.75f, 0.50f, 0.25f, 0.00f },
		// vs Wind
		{ 0.75f, 0.50f, 0.25f, 0.00f },
		// vs Poison
		{ 1.00f, 0.75f, 0.50f, 0.25f },
		// vs Holy
		{ 0.00f, -0.25f, -0.50f, -0.75f },
		// vs Shadow
		{ 1.25f, 1.50f, 1.75f, 2.00f },
		// vs Ghost
		{ 1.00f, 0.75f, 0.50f, 0.25f },
		// vs Undead
		{ 1.50f, 1.75f, 2.00f, 2.00f },
	},
	// Attack: Shadow (7)
	{
		// vs Neutral
		{ 1.00f, 1.00f, 1.00f, 1.00f },
		// vs Water
		{ 1.00f, 0.75f, 0.50f, 0.25f },
		// vs Earth
		{ 1.00f, 0.75f, 0.50f, 0.25f },
		// vs Fire
		{ 1.00f, 0.75f, 0.50f, 0.25f },
		// vs Wind
		{ 1.00f, 0.75f, 0.50f, 0.25f },
		// vs Poison
		{ 0.50f, 0.25f, 0.00f, -0.25f },
		// vs Holy
		{ 1.25f, 1.50f, 1.75f, 2.00f },
		// vs Shadow
		{ 0.00f, -0.25f, -0.50f, -0.75f },
		// vs Ghost
		{ 1.00f, 0.75f, 0.50f, 0.25f },
		// vs Undead
		{ 0.00f, 0.00f, 0.00f, -0.25f },
	},
	// Attack: Ghost (8)
	{
		// vs Neutral
		{ 0.25f, 0.00f, 0.00f, 0.00f },
		// vs Water
		{ 1.00f, 0.75f, 0.50f, 0.25f },
		// vs Earth
		{ 1.00f, 0.75f, 0.50f, 0.25f },
		// vs Fire
		{ 1.00f, 0.75f, 0.50f, 0.25f },
		// vs Wind
		{ 1.00f, 0.75f, 0.50f, 0.25f },
		// vs Poison
		{ 0.75f, 0.50f, 0.25f, 0.00f },
		// vs Holy
		{ 0.75f, 0.50f, 0.25f, 0.00f },
		// vs Shadow
		{ 0.75f, 0.50f, 0.25f, 0.00f },
		// vs Ghost
		{ 1.25f, 1.50f, 1.75f, 2.00f },
		// vs Undead
		{ 1.00f, 1.00f, 1.00f, 1.00f },
	},
	// Attack: Undead (9)
	{
		// vs Neutral
		{ 1.00f, 1.00f, 1.00f, 1.00f },
		// vs Water
		{ 1.00f, 0.75f, 0.50f, 0.25f },
		// vs Earth
		{ 1.00f, 0.75f, 0.50f, 0.25f },
		// vs Fire
		{ 1.00f, 1.00f, 1.00f, 1.00f },
		// vs Wind
		{ 1.00f, 0.75f, 0.50f, 0.25f },
		// vs Poison
		{ 0.50f, 0.00f, -0.25f, -0.50f },
		// vs Holy
		{ 1.00f, 1.25f, 1.50f, 1.75f },
		// vs Shadow
		{ 0.00f, 0.00f, -0.25f, -0.50f },
		// vs Ghost
		{ 1.00f, 0.75f, 0.50f, 0.25f },
		// vs Undead
		{ 0.00f, 0.00f, -0.25f, -0.50f },
	},
};

float UROElementalSystem::GetElementalModifier(EROElement AttackElement, EROElement DefenseElement, EROElementLevel DefLevel)
{
	const int32 AtkIdx = static_cast<int32>(AttackElement);
	const int32 DefIdx = static_cast<int32>(DefenseElement);
	const int32 LvlIdx = static_cast<int32>(DefLevel);

	// Bounds check
	if (AtkIdx < 0 || AtkIdx >= 10 || DefIdx < 0 || DefIdx >= 10 || LvlIdx < 0 || LvlIdx >= 4)
	{
		return 1.0f; // Default to neutral modifier
	}

	return ElementalTable[AtkIdx][DefIdx][LvlIdx];
}
