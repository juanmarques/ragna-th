# Ragna-TH Changelog

## 2026-03-08 — Audit Fixes & TODO Implementation

Two rounds of fixes based on a comprehensive skill/casting system audit and a design/TODO sweep across the codebase.

---

### Round 1: Bug Fixes (commit `d90a8ca2`)

Three real bugs identified by the audit and fixed:

#### 1. Hiding SP Drain Timer Safety
**File:** `Skills/Abilities/ROAbility_Hiding.cpp`

The SP drain timer used a raw `BindUObject` callback, which could crash if the ability was garbage-collected while the timer was still running. Replaced with a `TWeakObjectPtr` lambda that checks validity before calling `OnSPDrainTick()`.

#### 2. Stone Curse Phase Reset on Refresh
**File:** `Combat/ROStatusEffectComponent.cpp`

Re-applying Stone Curse to an already-stoned target would reset the duration timer but leave `bStoneCurseHardened` and `StonePhaseTimer` in their old state. This could cause a target to skip Phase 1 ("soft stone") entirely and jump straight to Phase 2 (full petrification). Now resets both fields when Stone is refreshed.

#### 3. Target Validation for Single-Target Skills
**Files:** `Skills/ROGameplayAbility.h`, `Skills/ROGameplayAbility.cpp`, plus 5 ability files

Added a `bRequiresTarget` property to the base ability class. When set, `CanActivateAbility` checks that the player controller has a valid `SelectedTarget`. Applied to: Fire Bolt, Cold Bolt, Lightning Bolt, Bash, Double Strafe.

---

### Round 2: Design & TODO Fixes (commit `d445a882`)

Addressed four categories of design issues across 17 files (+338/−367 lines).

#### A. Duplicate Elemental Table Eliminated
**Files:** `Data/RODamageFormulas.cpp`, `Data/RODamageFormulas.h`

`RODamageFormulas` and `ROElementalSystem` both contained identical 10×10×4 elemental modifier tables (~270 lines each). Removed the copy from `RODamageFormulas`; `GetElementalModifier()` now delegates to `UROElementalSystem::GetElementalModifier()` as the single source of truth.

#### B. Monster Property Replication
**Files:** `Monsters/ROMonsterBase.h`, `Monsters/ROMonsterBase.cpp`

`InitializeFromData()` sets all monster stats, but `GetLifetimeReplicatedProps` only replicated a subset. Added `Replicated` specifier and `DOREPLIFETIME` for 11 missing properties:
- `ATK`, `ATKMin`, `ATKMax`, `MATK`
- `DEF`, `MDEF`, `HIT`, `FLEE`
- `ElementLevel`, `Size`, `Race`

#### C. MagnumBreak Line-of-Sight
**File:** `Skills/Abilities/ROAbility_MagnumBreak.cpp`

The AoE overlap query hit targets through walls. Added a `LineTraceSingleByChannel` visibility check per target — enemies behind geometry are now skipped.

#### D. TODO Stub Implementations (20+ TODOs resolved)

| System | File(s) | What was implemented |
|--------|---------|---------------------|
| **Chat — name resolution** | `Social/ROChatSubsystem.cpp/.h` | `GetPlayerName()` and `ResolvePlayerName()` iterate `GameState->PlayerArray`, cast to `AROPlayerState`, match by character name (case-insensitive) |
| **Chat — local radius** | `Social/ROChatSubsystem.cpp` | `RouteLocalMessage()` finds sender pawn location, iterates all players, delivers to those within `LocalChatRadius` (500 units) |
| **Chat — global broadcast** | `Social/ROChatSubsystem.cpp` | `RouteGlobalMessage()` iterates `PlayerArray` and calls `DeliverMessageToPlayer()` for each |
| **Chat — whisper names** | `Social/ROChatSubsystem.cpp` | `ParseAndSendMessage()` resolves quoted/unquoted whisper target names via `ResolvePlayerName()` instead of raw `Atoi` |
| **WoE — start/end announce** | `World/ROWoEManager.cpp` | `StartWoE()` and `EndWoE()` broadcast system chat messages via `UROChatSubsystem` |
| **WoE — castle capture announce** | `World/ROWoEManager.cpp` | `OnEmperiumDestroyed()` broadcasts "[CastleName] has been captured by Guild X!" via system chat |
| **Boss kill broadcast** | `Monsters/ROMonsterSpawnManager.cpp` | MVP/boss death broadcasts "[MVP] MonsterName defeated by KillerName!" to all players via system chat |
| **Death EXP penalty** | `Core/ROGameModeBase.cpp` | `ApplyDeathPenalty()` uses `UROLevelingComponent` to deduct `DeathExpPenaltyPercent` of the level's required EXP from `CurrentBaseExp` |
| **Portal level check** | `World/ROPortalActor.cpp` | Checks `LevelingComponent->BaseLevel >= RequiredBaseLevel` before allowing teleport |
| **Zone flag application** | `World/ROMapZone.cpp` | `ApplyZoneRules()` sets `bPvPEnabled`, `bTeleportBlocked`, `bInTown`, `bInGuildZone`, `GuildZoneOwnerID` on `AROCharacterBase` |
| **Zone flag removal** | `World/ROMapZone.cpp` | `RemoveZoneRules()` resets all zone flags to safe defaults |
| **Save point persistence** | `World/ROSpawnPoint.cpp` | `SetPlayerSpawnPoint()` writes `SavedSpawnMapID` and `SavedSpawnLocation` to `AROCharacterBase` |
| **Character zone properties** | `Character/ROCharacterBase.h/.cpp` | Added `bPvPEnabled`, `bTeleportBlocked`, `bInTown`, `bInGuildZone`, `GuildZoneOwnerID`, `SavedSpawnMapID`, `SavedSpawnLocation` with defaults |
| **Emperium → WoE wiring** | `World/ROEmperiumActor.cpp` | `HandleDestruction()` now notifies `UROWoEManager::OnEmperiumDestroyed()` for castle ownership transfer |
| **Vending shop position** | `Social/ROVendingSystem.cpp` | Shop creation retrieves player pawn location for the shop's world position |

---

### Remaining TODOs (not implemented — require external systems)

These TODOs remain in the codebase because they depend on systems or assets that don't exist yet:

| TODO | Reason deferred |
|------|----------------|
| Weather system VFX | Requires particle systems and post-processing materials |
| GameMode `DefaultPawnClass` / `HUDClass` | Placeholder comments; set when character and HUD classes are ready |
| Server-side anticheat DB logging | Requires production database infrastructure |
| Network subsystem multi-server transfer | Architecture-level feature for cross-server travel |
| Guild emblem system | Requires custom bitmap rendering pipeline |
| Guild data persistence | Requires save/load backend |
| WoE kick non-guild / respawn Emperium | Handled via `OnCastleOwnerChanged` delegate — bind in Blueprint for map-specific logic |

---

*All changes are on branch `claude/fix-loop-bugs-axznr`.*
