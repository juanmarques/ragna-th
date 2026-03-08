# Ragna-TH Implementation Roadmap

Tracking all missing systems, features, and formula corrections identified in the rAthena/Hercules comparison audit. Items are checked off as they are implemented.

## Phase 1: Combat Formula Fixes
> Critical formula corrections that produce numerically wrong results

- [x] **Monster ATK variance per attack** — Roll between ATKMin/ATKMax per attack via `GetAttackDamage()`
- [ ] **Weapon ATK min/max rolling** — Player attacks should roll between weapon ATK min and max; crits use max
- [x] **Weapon-type size penalty table** — `GetWeaponSizeModifier()` with full 19-weapon x 3-size table
- [x] **ASPD per (job, weapon-type)** — `GetBaseASPDForJob()` now takes `EROWeaponType` with offset table
- [x] **Soft DEF randomization** — `rnd() % softDEF` variance applied per hit in `CalculatePhysicalDamage()`
- [ ] **Hard DEF / Soft DEF separation in GAS execution** — Split TotalDEF into separate hard and soft DEF in RODamageExecution
- [x] **Card-based damage modifiers (cardfix)** — `CardFixModifier` parameter added to physical and magical damage formulas
- [ ] **Refine bonus tables per weapon level** — Weapon refine: Lv1 +2, Lv2 +3, Lv3 +5, Lv4 +7 ATK per refine (already in WeaponData)
- [ ] **Per-level HP/SP tables** — Replace simple job modifiers with per-level HP tables matching rAthena

## Phase 2: Status Effect System
> Missing mechanics that RO players would immediately notice

- [x] **Stone Curse two-phase** — Phase 1 "soft stone" (3s, can move, can't attack), Phase 2 full petrification (1% HP drain/5s)
- [x] **Freeze element change** — Frozen targets become Water Lv1 element via `GetElementOverride()`, Stone Phase 2 becomes Earth Lv1
- [x] **VIT/LUK/INT-based resistance** — `ApplyStatusEffectWithResist()` with per-effect stat scaling
- [x] **OPT1 mutual exclusion** — Stone/Freeze/Stun/Sleep are mutually exclusive via `IsOPT1Effect()`
- [x] **Curse behavior** — Movement speed reduction via tag (handled by movement component)
- [x] **Confusion behavior** — Movement input randomization via tag (handled by movement component)
- [x] **Bleeding: block SP regen** — Bleeding blocks HP/SP regen via Status.Bleeding tag
- [x] **Status refresh behavior** — Reapplying resets timer to new full duration

## Phase 3: Inventory & Equipment
> Equipment system gaps

- [x] **50%/90% weight thresholds** — `IsOverweight50()` and `IsOverweight90()` added
- [x] **MaxWeight from STR** — `UpdateMaxWeight(STR)` uses `ROConstants::CalculateMaxWeight()`
- [x] **Two-handed weapon / shield mutual exclusion** — `ValidateEquipSlot()` blocks 2H+shield combos
- [x] **Equipment job class restrictions** — `AllowedJobs` array added to `UROItemBase`
- [x] **Card slot initialization on equipment creation** — CardSlots properly sized from weapon/armor data
- [x] **Item trade/drop restriction flags** — `bNoTrade`, `bNoDrop`, `bNoStore`, `bNoSell` added to items
- [x] **Item use cooldown tracking** — Shared consumable cooldown via `ItemCooldowns` map
- [ ] **Refine bonus per weapon level** — Already exists in `UROWeaponData::GetRefineBonusPerLevel()`

## Phase 4: Guild System
> Major missing guild features

- [x] **Guild EXP accumulation** — `ContributeGuildExp()` with member contribution tracking
- [x] **Guild leveling** — EXP-to-next-level table, `ProcessGuildLevelUp()`, skill points on level-up
- [x] **Guild skill allocation** — `AllocateGuildSkillPoint()` for master to spend skill points
- [x] **20 configurable position slots** — `FROGuildPosition` with title, invite/expel/storage permissions
- [x] **Guild alliance/hostility** — `RequestAlliance()`, `DeclareHostility()`, `RemoveRelation()` (max 3 each)
- [ ] **Guild emblem system** — Custom bitmap emblems displayed above characters
- [x] **Guild notice/announcement** — `SetGuildNotice()` / `GetGuildNotice()`
- [x] **Guild master transfer** — `TransferGuildMaster()` with rank swapping
- [x] **Pending invite cleanup** — `CleanupExpiredInvites()` with 30-second timeout
- [ ] **Guild data persistence** — Save/load guild data to persistent storage

## Phase 5: Monster System
> Spawning and combat improvements

- [x] **ATK variance per attack** — `GetAttackDamage()` rolls between ATKMin/ATKMax each attack
- [x] **Minimum respawn delay** — Enforced 5-second minimum in `OnMonsterDied()`
- [ ] **Day/night spawn support** — Monsters that only spawn during day or night
- [x] **Boss/MVP death announcement** — Log broadcast on boss kill (TODO: client multicast)
- [x] **Monster skill conditions** — `CheckSkillConditions()` checks HP%, range, cooldown, use chance
- [ ] **Per-spawn cell randomization** — Randomize initial center, subsequent respawns use randomized center

## Phase 6: Additional Systems (Future)
> Important but lower-priority features

- [ ] **Kafra (account) storage** — Separate from character inventory
- [ ] **Cart inventory** — Merchant-class separate cart inventory
- [ ] **Multi-hit attack support** — Double Attack, Triple Attack, auto-attack double-hit from cards
- [ ] **War of Emperium** — Castle ownership, Emperium defense, guardian stones, guild dungeon access
- [ ] **Guild storage** — Shared storage accessible by authorized members

---

*Last updated: 2026-03-08*
*Based on comparison with rAthena, Hercules, and eAthena implementations*
