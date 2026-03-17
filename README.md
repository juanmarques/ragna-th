# Ragna-TH: Ragnarok Online - Unreal Engine 5 Recreation

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A full Ragnarok Online MMORPG recreation built in Unreal Engine 5 with C++. Preserves the nostalgic gameplay (classes, stats, skills, combat, social systems) while leveraging UE5 for modern 3D graphics.

**Scope**: Prontera region as starting zone. All 6 first classes + all 12 second classes (Knight, Crusader, Wizard, Sage, Hunter, Bard/Dancer, Assassin, Rogue, Blacksmith, Alchemist, Priest, Monk). Full game systems.

**Platform**: macOS primary (Metal), cross-platform ready (Windows/Linux).

---

## Architecture

- **Language**: C++ with `UFUNCTION(BlueprintCallable)` / `UPROPERTY(BlueprintReadWrite)` exposure
- **Camera**: Low-FOV perspective camera on spring arm (isometric RO look, 45° pitch, ~30 FOV)
- **Server**: UE5 Dedicated Server with subsystems (Auth, Character, World)
- **Database**: PostgreSQL via async subsystem
- **Abilities**: Gameplay Ability System (GAS) for all skills
- **Networking**: UE5 replication, server-authoritative, custom ReplicationGraph
- **Project**: Feature-based module organization

---

## Project Structure

```
RagnarokUE/
├── RagnarokUE.uproject
├── Source/
│   ├── RagnarokUE/                    # Main game module
│   │   ├── RagnarokUE.Build.cs
│   │   ├── RagnarokUE.h / .cpp
│   │   ├── Core/                      # GameMode, GameState, PlayerController, PlayerState, GameInstance
│   │   ├── Character/                 # ROCharacterBase, StatsComponent, JobSystem, LevelingComponent
│   │   ├── Combat/                    # DamageCalculation, ElementalSystem, StatusEffects, CastingComponent
│   │   ├── Skills/                    # GAS abilities, SkillTreeComponent, ROAbilitySystemComponent
│   │   ├── Items/                     # ItemData, InventoryComponent, EquipmentComponent, CardSystem, RefinementSystem
│   │   ├── Monsters/                  # MonsterBase, AIControllers, SpawnManager, MVPManager
│   │   ├── NPC/                       # NPCBase, DialogueSystem, QuestManager, ServiceNPCs
│   │   ├── Social/                    # PartySubsystem, GuildSubsystem, ChatSubsystem, TradeSystem
│   │   ├── World/                     # MapManager, PortalSystem, ZoneStreaming, WoEManager
│   │   ├── UI/                        # HUD, StatWindow, InventoryWindow, SkillWindow, ChatWindow, Minimap
│   │   ├── Network/                   # ROReplicationGraph, ServerValidation, LoginSubsystem
│   │   └── Data/                      # Enums, Structs, DataTables, Constants
│   ├── RagnarokUEServer/              # Dedicated server module
│   │   ├── RagnarokUEServer.Build.cs
│   │   ├── AuthSubsystem/            # SHA256 auth, session tokens, ban system
│   │   ├── DatabaseSubsystem/         # PostgreSQL async wrapper
│   │   └── WorldSubsystem/           # Multi-map player tracking, broadcasts
│   ├── RagnarokUE.Target.cs           # Client target
│   ├── RagnarokUEEditor.Target.cs     # Editor target
│   └── RagnarokUEServer.Target.cs     # Dedicated server target
├── Content/                           # Asset placeholder dirs
│   ├── Characters/ Monsters/ Maps/ Items/ UI/ Effects/ Audio/ Data/
├── Config/
│   ├── DefaultEngine.ini
│   ├── DefaultGame.ini
│   ├── DefaultInput.ini
│   └── DefaultEditor.ini
└── Docs/
    └── GameDesignDocument.md
```

---

## Implementation Phases

### Phase 1: Project Skeleton (~15 files)
UE5 project structure, build targets (Client/Editor/Server), module definitions, engine/game/input config.

### Phase 2: Core Framework (~12 files)
- `ROGameInstance` - Server connections, login state, global subsystems
- `ROGameModeBase` - Server-side player spawning, game rules, death/respawn
- `ROGameStateBase` - Replicated server time, WoE status, weather
- `ROPlayerController` - Click-to-move, camera control, UI toggles, Server RPCs
- `ROPlayerState` - Replicated name, base/job level, job class, guild info

### Phase 3: Data Foundation (~8 files)
- `ROEnums.h` - 14+ enums: `EROStat` (6 stats), `EROJobClass` (40+ classes including transcendent), `EROElement` (10), `EROEquipSlot` (10), `EROStatusEffect`, `EROMonsterBehavior`, `EROItemType`, `EROWeaponType` (19 types), `EROMonsterSize`, `EROMonsterRace`, `ERODamageType`, `EROJobTier`
- `ROStructs.h` - `FROStatBlock`, `FROItemInstance`, `FROSkillInfo`, `FROMonsterSpawnInfo`, `FRODropInfo`, `FROJobClassInfo`, `FROMapInfo`
- `ROConstants.h` - Max level 99, max job level 50/70, stat point formulas, weight
- `RODamageFormulas.h/.cpp` - 14 static functions: ATK, MATK, DEF, MDEF, HIT, FLEE, ASPD, CritRate, MaxHP, MaxSP, elemental modifiers, physical/magical damage
- `ROExpTables.h/.cpp` - Full RO EXP tables for levels 1-99

### Phase 4: Character System (~14 files)
- `ROCharacterBase` - ACharacter + IAbilitySystemInterface, isometric camera, replicated HP/SP, sit/stand, death/respawn
- `ROStatsComponent` - 6 base stats + bonus, stat point allocation with cost formula, all derived stats
- `ROJobComponent` - Full job tree (Novice → 6 first → 12 second → transcendent path)
- `ROLevelingComponent` - Dual leveling (Base 1-99, Job variable), stat/skill point grants
- `ROCharacterMovement` - Click-to-move with NavMesh, AGI-based speed

### Phase 5: Combat & Skills via GAS (~18 files)
- `ROAbilitySystemComponent` - Custom ASC with job-based ability granting
- `ROAttributeSet` - 18 GAS attributes (HP, SP, ATK, MATK, DEF, MDEF, HIT, FLEE, ASPD, CritRate, etc.)
- `ROGameplayAbility` - Base ability with SP cost, variable/fixed cast times, cooldown, element
- `ROSkillTreeComponent` - Per-job skill trees with prerequisites
- **9 Abilities**: Bash, Magnum Break, Fire Bolt, Cold Bolt, Lightning Bolt, Heal, Double Strafe, Hiding, Discount
- `RODamageExecution` - Full RO damage formula via GAS execution calculation
- `RODamageGameplayEffect` / `ROHealGameplayEffect` - C++ configured GE classes
- `ROElementalSystem` - 10x10x4 elemental effectiveness table
- `ROStatusEffectComponent` - 13 status effects with durations
- `ROCastingComponent` - Variable/fixed cast time, interruption

### Phase 6: Item System (~14 files)
- `ROItemBase` / `ROWeaponData` / `ROArmorData` / `ROCardData` / `ROConsumableData` - UPrimaryDataAsset hierarchy
- `ROInventoryComponent` - Weight-based (100 slots), Zeny management, server-authoritative
- `ROEquipmentComponent` - 10 equipment slots with card sockets
- `ROCardSystem` - Card insertion/removal logic (UBlueprintFunctionLibrary)
- `RORefinementSystem` - +0 to +10 (pre-renewal), safe limits, break chance, ore requirements
- `ROItemDatabase` / `RODropTable` / `ROLootManager` - Item loading, monster drops, loot spawning

### Phase 7: Monster System (~12 files)
- `ROMonsterBase` - ACharacter with stats, threat table, drop table, EXP rewards
- `ROMonsterAIController` - Behavior tree selection per monster type
- **3 Behavior Trees**: Passive, Aggressive, Assist
- **3 BT Tasks**: RoamRandom, ChaseTarget, UseSkill
- `ROMonsterSpawnManager` - Spawn definitions, respawn timers, density control
- `ROMVPManager` - Boss tracking, announcements, tombstones
- `ROMonsterDatabase` - 12 Prontera-region monsters (Poring, Lunatic, Fabre, etc.)

### Phase 8: NPC & Quest System (~10 files)
- `RONPCBase` / `RODialogueComponent` - NPCs with branching dialogue trees
- `ROQuestManager` - Quest tracking with 7 built-in Prontera quests
- **5 Service NPCs**: Kafra Storage (600 slots), Refinement, Job Change, Shop (Discount/Overcharge), Tool Dealer

### Phase 9: Social Systems (~12 files)
- `ROPartySubsystem` - Max 12 members, Even Share (15-level range) / Individual EXP sharing
- `ROGuildSubsystem` - Max 56 members, 0-50% EXP tax, ranks, guild storage
- `ROChatSubsystem` - 6 channels (Local/Party/Guild/Whisper/Global/Trade), command parsing
- `ROTradeSystem` - Double-confirm Lock → Confirm flow
- `ROVendingSystem` - Merchant player shops
- `ROFriendSystem` - Friend list with online status

### Phase 10: World & Map System (~8 files)
- `ROMapManager` - 19 Prontera-region maps (prontera, prt_fild01-08, prt_sewb1-4, izlude, iz_dun01-05)
- `ROPortalActor` / `ROSpawnPoint` / `ROMapZone` - Map connectivity and zones
- `ROWarpPortal` - Acolyte skill portals (30s duration, 8 uses)
- `ROWeatherSystem` - Per-map weather effects
- `ROWoEManager` - War of Emperium scheduling, castle ownership, siege rules
- `ROEmperiumActor` - Destructible Emperium (normal attacks only, blocks own guild)

### Phase 11: UI System (~16 files)
16 UMG widgets: HUD, Hotbar (F1-F9), StatWindow, InventoryWindow, EquipmentWindow, SkillWindow, ChatWindow, Minimap, PartyWindow, GuildWindow, QuestLog, TradeWindow, NPCDialogue, ShopWindow, CastBar, TargetInfo

### Phase 12: Network & Server (~10 files)
- `ROReplicationGraph` - Grid spatialization, always-relevant nodes for party/guild
- `RONetworkSubsystem` - Connection management, ping tracking
- `ROServerValidation` - Movement/damage/item/cooldown anti-cheat
- `ROAuthSubsystem` - SHA256 password hashing, session tokens, ban system, login lockout
- `RODatabaseSubsystem` - PostgreSQL async, character CRUD, auto-save (5 min)
- `RODatabaseSchema` - 12 PostgreSQL tables (accounts, characters, inventory, guilds, storage, quests, friends, skills, castles, anticheat_logs)
- `ROWorldSubsystem` - Player-per-map tracking, broadcasts (map/area/all), disconnect cleanup

### Phase 13: Game Design Document
Complete GDD in `Docs/GameDesignDocument.md` covering all systems, formulas, class data, monster data.

---

## Key Formulas (Pre-Renewal RO)

### Physical ATK
```
BaseATK = STR + floor(STR/10)^2 + DEX/5 + LUK/5
FinalDamage = ATK * SkillMod * ElementMod * SizeMod - HardDEF * (1 - SoftDEF/100)
HardDEF = equipment DEF (flat reduction)
SoftDEF = VIT + floor(VIT/5)^2 (percentage reduction)
```

### Magical ATK
```
MATK_Min = INT + floor(INT/7)^2
MATK_Max = INT + floor(INT/5)^2
```

### ASPD
```
ASPD = 200 - (BaseASPD - floor((AGI * 4 + DEX) / 5) + PotionBonus + SkillBonus)
Attack Delay = (200 - ASPD) / 10 seconds
```

### Stat Point Cost
```
Cost to raise stat by 1 = floor((CurrentValue - 1) / 10) + 2
Stats 1-10 cost 2, 11-20 cost 3, ..., 91-99 cost 11
```

### Variable Cast Time
```
VCT = BaseCastTime * (1 - sqrt((DEX * 2 + INT) / 530))
```

### Refinement Safe Limits
| Weapon Level | Safe Limit | Ore |
|-------------|-----------|-----|
| Lv1 | +7 | Phracon |
| Lv2 | +6 | Emveretarcon |
| Lv3 | +5 | Oridecon |
| Lv4 | +4 | Oridecon |
| Armor | +4 | Elunium |

### Elemental Table
10 elements (Neutral/Water/Earth/Fire/Wind/Poison/Holy/Shadow/Ghost/Undead) x 4 element levels effectiveness matrix.

---

## File Count Summary

| Phase | Files | Description |
|-------|-------|-------------|
| 1 | ~15 | Project skeleton |
| 2 | ~12 | Core framework |
| 3 | ~8 | Data foundation |
| 4 | ~14 | Character system |
| 5 | ~20 | Combat & skills (GAS) |
| 6 | ~14 | Item system |
| 7 | ~12 | Monster system |
| 8 | ~10 | NPC & quests |
| 9 | ~12 | Social systems |
| 10 | ~8 | World & maps |
| 11 | ~16 | UI system |
| 12 | ~10 | Network & server |
| 13 | 1 | Game design doc |
| **Total** | **~161** | **Full project** |

---

## Verification Checklist

- [x] All `.h/.cpp` files have correct UE5 includes, UCLASS/USTRUCT/UENUM macros
- [x] Build.cs files reference correct module dependencies
- [x] All replicated properties have `GetLifetimeReplicatedProps` implementations
- [x] GAS: AbilitySystemComponent initialized, AttributeSet registered, abilities grantable
- [x] All enums used consistently across systems (EROStat::INT_STAT, not INT)
- [x] Server RPCs validate all inputs, clients never directly modify authoritative state
- [x] Client, Editor, and Server targets all reference correct modules
- [x] Damage formulas match pre-renewal RO
- [x] EXP tables verified for levels 1-99
- [x] Database schema uses PostgreSQL syntax (SERIAL, TIMESTAMP, BOOLEAN)

---

## Building

Requires Unreal Engine 5.3+.

1. Clone the repository
2. Open `RagnarokUE/RagnarokUE.uproject` in Unreal Editor
3. Build via Editor or command line:
   ```bash
   # Client
   UnrealBuildTool RagnarokUE Mac Development

   # Dedicated Server
   UnrealBuildTool RagnarokUEServer Mac Development
   ```

---

## License

Copyright Ragna-TH Project. All Rights Reserved.
