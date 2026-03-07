# Ragnarok UE - Game Design Document

## 1. Overview

**Title**: Ragnarok UE
**Genre**: MMORPG (Massively Multiplayer Online Role-Playing Game)
**Engine**: Unreal Engine 5.3
**Platform**: macOS (Metal), Windows (DX12), Linux (Vulkan)
**Perspective**: Isometric/Top-down 3D (low-FOV perspective camera)

A faithful recreation of Ragnarok Online using Unreal Engine 5 for modern 3D graphics while preserving the classic gameplay systems, job classes, combat mechanics, and social features that defined the original.

---

## 2. Core Pillars

1. **Nostalgic Gameplay**: Preserve the original RO stat system, job classes, skill trees, and combat feel
2. **Modern Visuals**: Leverage UE5 for enhanced 3D graphics, lighting, and effects
3. **Community**: Full social systems (party, guild, WoE, trade, vending, chat)
4. **Depth**: Complex interconnected systems (stats affect skills affect combat affect builds)
5. **Accessibility**: Runs on macOS, Windows, Linux with reasonable hardware

---

## 3. Character System

### 3.1 Stats
Six primary stats, each starting at 1, max 99:

| Stat | Primary Effect | Secondary Effects |
|------|---------------|-------------------|
| **STR** | Physical ATK | Weight Capacity (+30 per point) |
| **AGI** | Attack Speed (ASPD), Flee | - |
| **VIT** | Max HP, HP Recovery | Status Effect Resistance (Stun, Poison) |
| **INT** | Magic ATK (MATK), Max SP | SP Recovery, Variable Cast Time reduction |
| **DEX** | Hit Rate, Ranged ATK | Variable Cast Time reduction, Min ATK stability |
| **LUK** | Critical Rate, Perfect Dodge | Small bonuses to ATK, MATK, Flee |

**Stat Point Cost**: `floor((CurrentValue - 1) / 10) + 2`
- Stat 1→2: 2 points, Stat 10→11: 3 points, Stat 50→51: 7 points, Stat 98→99: 12 points

**Stat Points Per Level**: `floor((Level - 1) / 5) + 3` per base level up

### 3.2 Formulas

**Physical ATK**:
```
BaseATK = STR + floor(STR/10)^2 + floor(DEX/5) + floor(LUK/5)
WeaponATK = weapon's ATK value
TotalATK = BaseATK + WeaponATK + RefineBonus
```

**Magic ATK**:
```
MATK_Max = INT + floor(INT/5)^2
MATK_Min = INT + floor(INT/7)^2
```

**HIT Rate**: `175 + BaseLevel + DEX + floor(LUK/3)`
**FLEE Rate**: `100 + BaseLevel + AGI + floor(LUK/5)`
**Critical Rate**: `LUK * 0.3 + 1` (doubled with Katar weapons)
**Perfect Dodge**: `floor(LUK/10) + 1`

**Max HP** (varies by job, approximate for Swordsman-type):
```
Base: 35 + BaseLevel * 5 + floor(BaseLevel^2 / 50)
VIT Bonus: MaxHP_Base * VIT/100
```

**Max SP**:
```
Base: 10 + BaseLevel * 2
INT Bonus: MaxSP_Base * INT/100
```

**ASPD**: `200 - (BaseASPD - floor((AGI*4 + DEX) / 5))`
- Attack delay = `(200 - ASPD) / 50` seconds

### 3.3 Leveling
Dual-level system:
- **Base Level** (1-99): Grants stat points, affects HP/SP, required for job change
- **Job Level** (1-50 for 1st/2nd class, 1-70 for transcendent): Grants skill points

EXP Table (selected values):
| Level | Base EXP Required | Cumulative |
|-------|-------------------|------------|
| 1→2 | 9 | 9 |
| 10→11 | 1,000 | ~3,800 |
| 20→21 | 4,200 | ~28,000 |
| 30→31 | 11,800 | ~100,000 |
| 40→41 | 28,000 | ~340,000 |
| 50→51 | 60,000 | ~1,000,000 |
| 60→61 | 117,500 | ~2,700,000 |
| 70→71 | 250,000 | ~7,000,000 |
| 80→81 | 600,000 | ~20,000,000 |
| 90→91 | 2,000,000 | ~70,000,000 |
| 98→99 | 10,000,000 | ~100,000,000 |

---

## 4. Job Class System

### 4.1 Class Tree

```
                        Novice (Base)
                           |
    ┌──────────┬──────────┬┴──────────┬──────────┬──────────┐
    │          │          │           │          │          │
Swordsman  Magician   Archer     Thief    Merchant  Acolyte
  │    │     │    │    │    │     │    │    │    │    │    │
Knight Cru. Wiz. Sage Hunt. B/D  Ass. Rog. BS  Alch Priest Monk
  │    │     │    │    │    │     │    │    │    │    │    │
  L.K. Pal. H.W. Prof Snip M/G  A.X. Stk. WS  Cre. H.P. Champ
```

### 4.2 Job Change Requirements

| From | To | Requirements |
|------|----|-------------|
| Novice | 1st Class | Job Level 10+, Basic Skill Lv9 |
| 1st Class | 2nd Class | Job Level 40+ (50 recommended), Base Level 40+ |
| 2nd Class | Transcendent | Base Level 99, Job Level 50, rebirth quest |
| Trans 2nd | Same (Trans ver.) | Follow normal leveling path as High Novice → High 1st → Trans 2nd |

### 4.3 Class Details

**Swordsman** → Knight / Crusader
- Role: Melee tank/DPS
- Key Skills: Bash, Magnum Break, Provoke, Endure, HP Recovery
- Knight skills: Bowling Bash, Pierce, Brandish Spear, Two-Hand Quicken
- Crusader skills: Holy Cross, Grand Cross, Shield Boomerang, Devotion

**Magician** → Wizard / Sage
- Role: Magic DPS / Support
- Key Skills: Fire/Cold/Lightning Bolt, Napalm Beat, Soul Strike, Safety Wall
- Wizard skills: Storm Gust, Lord of Vermilion, Meteor Storm, Jupitel Thunder
- Sage skills: Dispel, Land Protector, Spell Breaker, Auto Spell

**Archer** → Hunter / Bard (Male) / Dancer (Female)
- Role: Ranged DPS / Support
- Key Skills: Double Strafe, Arrow Shower, Improve Concentration, Owl's Eye
- Hunter skills: Blitz Beat, Falcon Assault, Claymore Trap, Ankle Snare
- Bard/Dancer skills: Songs/Dances that buff party, Amp, Lullaby

**Thief** → Assassin / Rogue
- Role: Melee DPS / Stealth
- Key Skills: Hiding, Steal, Double Attack, Envenom, Detoxify
- Assassin skills: Sonic Blow, Grimtooth, Enchant Poison, Katar Mastery
- Rogue skills: Backstab, Raid, Intimidate, Strip Skills, Plagiarism

**Merchant** → Blacksmith / Alchemist
- Role: Crafting / Support / DPS
- Key Skills: Discount, Overcharge, Mammonite, Vending, Cart Revolution
- Blacksmith skills: Weapon Forge, Weapon Refine, Power Thrust, Adrenaline Rush
- Alchemist skills: Potion Creation, Acid Terror, Homunculus, Bio Cannibalize

**Acolyte** → Priest / Monk
- Role: Healer/Support / Melee DPS
- Key Skills: Heal, Blessing, Increase Agility, Ruwach, Angelus
- Priest skills: Resurrection, Sanctuary, Magnus Exorcismus, Turn Undead, Kyrie Eleison
- Monk skills: Occult Impaction, Finger Offensive (Throw Spirit Sphere), Asura Strike, Combo skills

---

## 5. Combat System

### 5.1 Physical Damage
```
1. Hit Check: (Source HIT - Target FLEE) + 80 >= random(1,100)
   - Miss if check fails (unless critical)
2. Critical Check: CritRate > random(1,100)
   - Critical: +40% damage, ignores FLEE (not Perfect Dodge)
3. Damage = (BaseATK + WeaponATK * SizeModifier) * SkillModifier
4. Apply Element Modifier (from elemental table)
5. Subtract DEF: FinalDamage = Damage * (100 - SoftDEF%) / 100 - HardDEF
6. Minimum damage = 1
```

### 5.2 Magical Damage
```
1. MATK = random(MATK_Min, MATK_Max)
2. Damage = MATK * SkillModifier
3. Apply Element Modifier
4. Subtract MDEF: FinalDamage = Damage - MDEF
5. Minimum damage = 1
```

### 5.3 Size Modifiers (Weapon Type vs Monster Size)
| Weapon | Small | Medium | Large |
|--------|-------|--------|-------|
| Dagger | 100% | 75% | 50% |
| Sword | 75% | 100% | 75% |
| Two-Hand Sword | 75% | 75% | 100% |
| Spear | 75% | 75% | 100% |
| Axe | 50% | 75% | 100% |
| Mace | 75% | 100% | 100% |
| Rod | 100% | 100% | 100% |
| Bow | 100% | 100% | 75% |
| Katar | 75% | 100% | 75% |
| Knuckle | 100% | 75% | 50% |
| Book | 100% | 100% | 50% |

### 5.4 Elemental Table
Attacking Element vs Defending Element (Level 1):

|  | Neutral | Water | Earth | Fire | Wind | Poison | Holy | Shadow | Ghost | Undead |
|--|---------|-------|-------|------|------|--------|------|--------|-------|--------|
| **Neutral** | 100% | 100% | 100% | 100% | 100% | 100% | 100% | 100% | 25% | 100% |
| **Water** | 100% | 25% | 100% | 150% | 90% | 100% | 75% | 100% | 100% | 100% |
| **Earth** | 100% | 100% | 25% | 90% | 150% | 100% | 75% | 100% | 100% | 100% |
| **Fire** | 100% | 90% | 150% | 25% | 100% | 100% | 75% | 100% | 100% | 125% |
| **Wind** | 100% | 150% | 90% | 100% | 25% | 100% | 75% | 100% | 100% | 100% |
| **Poison** | 100% | 100% | 100% | 100% | 100% | 0% | 100% | 50% | 100% | -25% |
| **Holy** | 100% | 100% | 100% | 100% | 100% | 100% | 0% | 125% | 100% | 150% |
| **Shadow** | 100% | 100% | 100% | 100% | 100% | 50% | 125% | 0% | 100% | -25% |
| **Ghost** | 25% | 100% | 100% | 100% | 100% | 100% | 100% | 100% | 125% | 100% |
| **Undead** | 100% | 100% | 100% | 100% | 100% | -25% | 150% | -25% | 100% | 0% |

(Higher defense element levels shift these values further)

### 5.5 Cast Time
- **Variable Cast Time (VCT)**: Reduced by DEX and INT
  - Reduction formula: `VCT * (1 - sqrt((DEX*2 + INT) / 530))`
  - At 150 DEX + INT, VCT ≈ 0 (instant cast)
- **Fixed Cast Time (FCT)**: Cannot be reduced by stats (only special equipment)
- Total Cast Time = VCT + FCT
- Interrupted by: taking damage (unless Phen card equipped), moving

### 5.6 Status Effects

| Effect | Duration | Impact |
|--------|----------|--------|
| Stun | 2-5 sec | Cannot move, attack, or use skills |
| Freeze | 3-10 sec | Cannot act, becomes Water element, shatter on hit |
| Stone | 5-20 sec | Gradual petrification, cannot act when full, loses HP |
| Poison | 10-30 sec | Loses HP over time, reduced recovery |
| Blind | 10-30 sec | Reduced HIT and FLEE |
| Silence | 10-30 sec | Cannot use skills |
| Sleep | 10-30 sec | Cannot act, wakes on damage |
| Curse | 10-30 sec | LUK = 0, reduced movement speed |
| Bleeding | 10-30 sec | Loses HP, cannot regenerate, leaves trail |
| Confusion | 10-30 sec | Movement direction is randomized |

---

## 6. Item System

### 6.1 Equipment Slots
- Head Top (hat, helmet)
- Head Mid (glasses, mask)
- Head Low (mouth piece)
- Armor (body)
- Weapon (right hand)
- Shield (left hand)
- Garment (cape, mantle)
- Footgear (shoes, boots)
- Accessory Left (ring, clip)
- Accessory Right (ring, clip)

### 6.2 Refinement System

| Refine Level | Weapon Lv1 | Weapon Lv2 | Weapon Lv3 | Weapon Lv4 | Armor |
|-------------|-----------|-----------|-----------|-----------|-------|
| +1 to Safe | 100% | 100% | 100% | 100% | 100% |
| Safe limit | +7 | +6 | +5 | +4 | +4 |
| Safe+1 | 60% | 60% | 60% | 60% | 60% |
| Safe+2 | 40% | 40% | 40% | 40% | 40% |
| Safe+3 | 20% | 20% | 20% | 20% | 20% |
| Beyond | Decreasing | Decreasing | Decreasing | Decreasing | Decreasing |

**Refine Bonus Per Level**:
- Weapon Lv1: +2 ATK per refine
- Weapon Lv2: +3 ATK per refine
- Weapon Lv3: +5 ATK per refine
- Weapon Lv4: +7 ATK per refine
- Armor: +1 DEF per refine (but 0.66 actual reduction)

**Failure**: Equipment is destroyed (broken).
**Required Ores**: Phracon (Lv1), Emveretarcon (Lv2), Oridecon (Lv3-4), Elunium (Armor)

### 6.3 Card System
- Cards drop from monsters (very low rate, typically 0.01-0.02%)
- Insert into equipment with available card slots (0-4 slots)
- Cards provide bonuses: stats, elemental properties, race bonuses, special effects
- Card names form compound equipment names (e.g., "Poring Carded Sword" → "Lucky Sword")
- Card removal requires special NPC services

---

## 7. Monster System

### 7.1 AI Behaviors
- **Passive**: Ignores players, fights back only when attacked
- **Aggressive**: Attacks any player in range on sight
- **Assist**: Passive until ally of same type is attacked, then joins fight
- **Cast Sensor**: Immediately targets players casting spells nearby
- **Detector**: Can see and attack hidden/cloaked players
- **Looter**: Picks up items on the ground

### 7.2 Prontera Region Monsters

| Monster | HP | ATK | DEF | Element | Size | Race | Behavior | Base EXP | Job EXP |
|---------|-----|-----|-----|---------|------|------|----------|----------|---------|
| Poring | 50 | 7-10 | 0 | Neutral 1 | Small | Plant | Passive | 2 | 1 |
| Lunatic | 60 | 9-12 | 0 | Neutral 3 | Small | Brute | Passive | 6 | 2 |
| Fabre | 63 | 8-11 | 0 | Neutral 1 | Small | Insect | Passive | 4 | 2 |
| Pupa | 427 | 1 | 0 | Earth 1 | Small | Insect | Passive | 4 | 4 |
| Drops | 55 | 10-13 | 0 | Fire 1 | Medium | Plant | Passive | 4 | 2 |
| Poporing | 182 | 24-29 | 0 | Poison 1 | Medium | Plant | Passive | 81 | 27 |
| Willow | 80 | 11-14 | 0 | Fire 1 | Medium | Plant | Passive | 11 | 5 |
| Condor | 92 | 13-16 | 0 | Wind 1 | Medium | Brute | Passive | 6 | 3 |
| Roda Frog | 97 | 12-15 | 0 | Water 1 | Medium | Fish | Passive | 15 | 6 |
| Rocker | 198 | 24-31 | 5 | Earth 1 | Medium | Insect | Passive | 20 | 15 |
| Thief Bug | 50 | 10-13 | 5 | Neutral 1 | Small | Insect | Aggressive | 15 | 8 |
| Steel Chonchon | 413 | 40-53 | 10 | Wind 1 | Small | Insect | Aggressive | 77 | 30 |

### 7.3 MVP Bosses
- Special powerful monsters with unique drops
- Server-wide spawn/kill announcements
- Respawn timer: 1-2 hours with random variance
- MVP reward system: tombstone at death location
- MVP cards: extremely rare (0.01%), powerful effects

---

## 8. Social Systems

### 8.1 Party System
- Max 12 members
- EXP sharing modes:
  - **Each Take**: Only the attacker gets EXP
  - **Even Share**: EXP split equally (must be within 15 base levels)
- Party members see each other's HP/SP bars
- Party chat channel

### 8.2 Guild System
- Created with Emperium item
- Base 16 members, expandable to 56
- Ranks: Guild Master, Sub Master, Member
- EXP tax: 0-50% of base EXP contributed to guild
- Guild skills (e.g., Guild Extension for more members)
- Guild storage, guild emblem

### 8.3 War of Emperium (WoE)
- Scheduled guild-vs-guild castle sieges
- Destroy the Emperium (normal attacks only, no skills) to capture castle
- Castle ownership grants access to treasure rooms
- Special rules: no MVP card effects, restricted skills
- Typically scheduled Wednesday and Saturday evenings

### 8.4 Trade & Vending
- Direct player-to-player trade with double-confirm system
- Merchant class vending: set up shop with custom prices
- Chat-based buying/selling channels

---

## 9. World Design

### 9.1 Prontera Region (Starting Zone)
- **Prontera City**: Capital, all services (Kafra, shops, guild, job change NPCs)
- **Prontera Fields 01-08**: Increasing difficulty, various monsters
- **Prontera Culvert (Sewers)**: 4-floor dungeon, Thief Bug family
- **Izlude**: Satellite town, Swordsman guild
- **Byalan Island**: 5-floor underwater dungeon

### 9.2 Map Connectivity
```
prt_fild01 ←→ Prontera ←→ prt_fild02
     ↕              ↕           ↕
prt_fild03     prt_fild04  prt_fild05
     ↕              ↕           ↕
prt_fild06     prt_fild07  prt_fild08
                    ↕
                 Izlude
                    ↕
              Byalan Island
```

---

## 10. UI Layout

Classic RO-style interface:
- **Bottom**: Hotbar (F1-F9 × 4 rows), basic info (HP/SP bars, level)
- **Top-right**: Minimap
- **Toggle Windows**: Stats (A), Inventory (I), Equipment (E), Skills (S), Quest (Q), Party (Z), Guild (G)
- **Chat**: Bottom-left, tabbed channels
- **Target Info**: Top-center when targeting
- **Cast Bar**: Above character during casting

---

## 11. Technical Architecture

### 11.1 Client
- UE5 C++ with Blueprint-exposed functions
- Isometric camera (low-FOV perspective, spring arm)
- Click-to-move with pathfinding
- Gameplay Ability System for all skills
- UMG widgets for UI

### 11.2 Server
- UE5 Dedicated Server
- Server-authoritative gameplay (all validation server-side)
- Custom Replication Graph for MMO-scale
- Subsystems: Auth, Database (PostgreSQL), World Management

### 11.3 Data Persistence
- PostgreSQL database
- Async save/load operations
- Auto-save every 5 minutes
- Immediate save on logout
- Tables: accounts, characters, inventory, guilds, storage, quests, friends
