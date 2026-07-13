# Phase 2 critical content audit

## Status and scope

- Status: `AUDIT_COMPLETE_READ_ONLY`.
- Date: `2026-07-13`.
- Source: `D:\Projects UE5\LustAsDeadlySin` (`READ_ONLY`).
- Target: `D:\Projects UE5\NoShellForWinter`.
- Registry evidence: `Saved/Migration/Phase2/SourceAssetRegistry57.json`.
- Registry size: 12,184 exported asset records, all mounted below `/Game`.
- Method: source/target filesystem inventory, package-path comparison, SHA-256 comparison for overlaps, Asset Registry dependency/referencer inspection, and config/source ownership search.
- No source or target asset was loaded, resaved, copied, moved, renamed, or modified by this audit.

The registry is useful for dependency evidence, but it is not complete for the procedural stack. Physical packages absent from the JSON are explicitly classified as `PENDING_REGISTRY_OPAQUE`; absence from the JSON must never be treated as proof that an asset is unused.

## Manifest policies

| Policy | Meaning |
|---|---|
| `PROTECT_TARGET_EXACT` | Keep the current target package and invariant hash. Never replace it with the source package. |
| `KEEP_TARGET_IDENTICAL` | Source and target SHA-256 match; retain the target copy without migrating it. |
| `MIGRATE_SOURCE_PROJECT_OWNED` | Migrate the source-owned package after its owning project plugin is ported. |
| `MIGRATE_EDITOR_DEPENDENCY_CLOSURE` | Seed migration from the named anchor through Unreal Editor AssetTools; do not use raw filesystem copying. |
| `REBUILD_OR_ADAPT_TO_TARGET` | Recreate behavior through project-owned adapters, bridges, components, subclasses, or interfaces against current 5.8 assets. |
| `EXCLUDE_DEMO_OR_LEGACY` | Keep out of production migration unless an explicit runtime dependency or product requirement is demonstrated. |
| `PENDING_REGISTRY_OPAQUE` | Package exists physically but was omitted by the source registry export; inspect it in Unreal after its owner modules are available. |

Target protection rules have higher priority than dependency-closure migration. If a source closure reaches `/Game/FullSample`, `/Game/DazToUnreal`, `/Game/Characters`, or `/Game/Input`, overlapping target packages remain authoritative.

## Critical root counts

Counts below are filesystem package counts (`.uasset` plus `.umap`), not exported-object counts.

| System/root | Source packages | Source-only | Same overlap | Different overlap | Default policy |
|---|---:|---:|---:|---:|---|
| `/Game/_Game/Locations/StorySelection` | 1 | 1 | 0 | 0 | `MIGRATE_SOURCE_PROJECT_OWNED` |
| `/Game/_Game/Hub` | 1 | 1 | 0 | 0 | `MIGRATE_EDITOR_DEPENDENCY_CLOSURE` |
| Character Background data/UI | 3 | 3 | 0 | 0 | `MIGRATE_SOURCE_PROJECT_OWNED` |
| `/EFCharacterCreation/UI` plugin content | 2 | 2 | 0 | 0 | Migrate with `EFCharacterCreation` plugin |
| `/Game/_Game/Widgets/Chronicle` | 23 | 23 | 0 | 0 | `MIGRATE_SOURCE_PROJECT_OWNED` |
| `/Game/_Game/Widgets/InnerState` | 24 | 24 | 0 | 0 | `MIGRATE_SOURCE_PROJECT_OWNED` |
| `/Game/_Game/Widgets/Status` | 17 | 17 | 0 | 0 | `MIGRATE_SOURCE_PROJECT_OWNED` |
| `/Game/_Game/Data/Survival` | 1 | 1 | 0 | 0 | `MIGRATE_SOURCE_PROJECT_OWNED` |
| `/Game/_Game/FoodSystem` | 172 | 172 | 0 | 0 | `MIGRATE_EDITOR_DEPENDENCY_CLOSURE` |
| `/Game/UI/Survival` | 3 | 3 | 0 | 0 | `EXCLUDE_DEMO_OR_LEGACY` pending referencers |
| `/Game/_Game/Widgets/Attributes` | 27 | 27 | 0 | 0 | `MIGRATE_SOURCE_PROJECT_OWNED` |
| `/Game/_Game/Widgets/SinfulAscensionAltar` | 36 | 36 | 0 | 0 | `MIGRATE_SOURCE_PROJECT_OWNED` |
| `/Game/UI/SinfulAscension` | 38 | 38 | 0 | 0 | `EXCLUDE_DEMO_OR_LEGACY` pending Editor referencers |
| `/Game/UI/Defeat` | 13 | 13 | 0 | 0 | `MIGRATE_SOURCE_PROJECT_OWNED` |
| `/Game/Procedural` | 3 | 3 | 0 | 0 | `MIGRATE_EDITOR_DEPENDENCY_CLOSURE` |
| `/Game/Calysto/Dungeon` | 244 | 244 | 0 | 0 | Seeded closure only; never copy root wholesale |
| `/Game/Calysto/Shared` | 148 | 148 | 0 | 0 | Seeded closure only; never copy root wholesale |
| `/Game/__ExternalActors__/Calysto/Dungeon` | 50 | 50 | 0 | 0 | Exclude demo external actors unless proven required |
| `/Game/Fantastic_Dungeon_Pack` | 1,342 | 1,342 | 0 | 0 | Seeded closure only; never copy root wholesale |
| `/Game/_Game/Input` | 2 | 2 | 0 | 0 | TattooShop-only migration |
| `/Game/FullSample/Player` | 1 | 0 | 0 | 1 | `PROTECT_TARGET_EXACT` |
| `/Game/FullSample/Blueprints/Characters/Player` | 2 | 0 | 1 | 1 | Per-package target policy |
| `/Game/FullSample/Blueprints/Characters/Enemies` | 19 | 7 | 1 | 11 | Per-package target/adaptation policy |
| `/Game/FullSample/Assets/Characters/Enemy_Great_Spider` | 21 | 0 | 21 | 0 | `KEEP_TARGET_IDENTICAL` |
| `/Game/DazToUnreal` | 403 | 264 | 0 | 139 | Protect every overlap; source-only is opt-in only |
| `/Game/Characters` | 105 | 92 | 0 | 13 | Protect every overlap; source-only is opt-in only |

## StorySelection and character creation anchors

Source-only packages:

- `/Game/_Game/Locations/StorySelection`
- `/Game/Data/CharacterBackground/DT_ProjectBackstories`
- `/Game/Data/CharacterBackground/DT_ProjectProfessions`
- `/Game/UI/CharacterBackground/WBP_ProjectCharacterBackgroundCreationWidget`

Direct registered visual dependencies of `StorySelection`:

- `/Game/_Game/LevelDesign/Torch`
- `/Game/_Game/Textures/Stone_Wall/Stone_Wall`

Owner evidence:

- `Plugins/EFProjectSystems/Source/EFProjectSystemsGameplay/CharacterBackground/ProjectCharacterBackgroundSubsystem.*`
- `Config/DefaultGame.ini:409-418`

The map's registered dependencies do not contain a hard reference to the source Player. Migrate StorySelection and its project-owned data/UI as a coherent flow, then validate StorySelection -> physical creator -> HUB in PIE.

The physical creator's two UI packages are plugin content and are therefore absent from the `/Game` registry export:

- `/EFCharacterCreation/UI/WBP_EFCharacterCreationRoot`
- `/EFCharacterCreation/UI/WBP_EFMorphSlider`

Owner/config evidence:

- `Plugins/EFCharacterCreation/Config/DefaultGame.ini:76-77`
- `Plugins/EFCharacterCreation/Source/EFCharacterCreationRuntime`

Risk: the source plugin default at `Plugins/EFCharacterCreation/Config/DefaultGame.ini:58` names `/Game/DazToUnreal/AmalaforGenesis9/AmalaforGenesis9`. Do not import that body option blindly. Reconcile creator settings against the authoritative target Female, Multiple, Male, and current Player.

## Chronicle anchors

The complete active root `/Game/_Game/Widgets/Chronicle/**` contains 23 source-only packages. Primary anchor:

- `/Game/_Game/Widgets/Chronicle/Main/WBP_ProjectChronicleWidget`

Owner evidence:

- `Plugins/EFProjectSystems/Source/EFProjectSystemsGameplay/UI/ProjectActivityFeedWidget.*`
- `Plugins/EFProjectSystems/Source/EFProjectSystemsGameplay/UI/ProjectActivityFeedSubsystem.*`
- `ProjectPerformanceBudgetSettings.cpp` preloads Chronicle packages.

Migrate all 23 packages together after `EFProjectSystems` is ported. The set includes Main, Globals, Normal, Expanded, fonts, and textures.

## Survival, Needs, Status, and Food closures

Active project-owned anchors:

- `/Game/_Game/Widgets/InnerState/Main/WBP_ProjectInnerStateWidget`
- `/Game/_Game/Widgets/Status/Main/WBP_ProjectSurvivalStatusWidget`
- `/Game/_Game/Data/Survival/DT_ProjectSurvivalStatuses`
- `/Game/_Game/FoodSystem/Food/Data/DA_FoodConsumableRegistry`
- `Config/DefaultGame.ini:149-158`

The status table directly depends on 13 packages under `/Game/_Game/Icons`. The active widgets and data are owned by `EFProjectSystemsGameplay`.

Food closure evidence:

- Seed packages under `/Game/_Game/FoodSystem`: 172.
- Registered transitive closure: 559 packages.
- Closure inside `/Game/Food_Props_Kit`: 377 of the root's 699 packages.
  - 81 meshes.
  - 79 materials.
  - 217 textures.
- Other registered project dependencies: 10 FullSample packages (5 Audio, 4 Assets, 1 Blueprints). Overlaps in those target-authoritative roots must resolve to target, not be imported from source.

Policy: run an Unreal Editor migration seeded from `/Game/_Game/FoodSystem/**`; accept the actual dependency closure, not the entire 699-package/approximately 1.87 GB Food Props root.

`/Game/UI/Survival/**` contains three old icons while active code paths point to `_Game/Widgets`. Keep it excluded unless an Editor referencer scan demonstrates an active consumer.

## SXP / Sinful Ascension anchors

Active source-only roots:

- `/Game/_Game/Widgets/Attributes/**`: 27 packages.
- `/Game/_Game/Widgets/SinfulAscensionAltar/**`: 36 packages.

Primary anchors:

- `/Game/_Game/Widgets/Attributes/Main/WBP_ProjectSinfulAscensionWidget`
- `/Game/_Game/Widgets/SinfulAscensionAltar/Main/WBP_ProjectSinfulAscensionExchangeMenu`

Owner evidence:

- `Plugins/EFProjectSystems/Source/EFProjectSystemsGameplay/SinfulAscension/ProjectSinfulAscension*`
- `Config/DefaultGame.ini:365-405`
- Current code-widget manifests and hardcoded visual paths point to the `_Game` roots.

`/Game/UI/SinfulAscension/**` contains 38 source-only legacy duplicates. Source code/config search found no current owner path to this root. Classify it as `EXCLUDE_DEMO_OR_LEGACY`, subject to an Unreal Editor referencer scan before final exclusion.

## Defeat anchors

`/Game/UI/Defeat/**` contains 13 source-only packages. Primary anchor:

- `/Game/UI/Defeat/Struggle/WBP_ProjectKnockoutStruggleWidget`

Owner/config evidence:

- `Plugins/EFProjectSystems/Source/EFProjectSystemsGameplay/Defeat`
- `Config/DefaultGame.ini:281-302`

Migrate the complete 13-package set after `EFProjectSystems`, then validate the advanced defeat/struggle flow in PIE.

## Procedural anchors and closure policy

Core source-owned maps/assets:

- `/Game/_Game/Hub/HUB`
- `/Game/_Game/Locations/PCGLevel`
- `/Game/Procedural/Maps/DungeonGeneration`
- `/Game/Procedural/DoorToLevel`
- `/Game/Procedural/Blueprints/Altar`

Config anchors:

- `Config/DefaultEngine.ini:10`: HUB editor startup map.
- `Config/DefaultGame.ini:145-147`: DungeonGeneration and DoorToLevel.
- `Config/DefaultGame.ini:260`: dungeon combat map.
- `Config/DefaultGame.ini:333-348`: dungeon actor, start point, and enemy lookup rules.

Use these Unreal Editor migration seeds:

- `/Game/Procedural/Maps/DungeonGeneration`
- `/Game/Calysto/Dungeon/Blueprint/BP_MassiveDungeon`
- `/Game/Calysto/Dungeon/Blueprint/Utility/BP_StartPoint`
- `/Game/Procedural/DoorToLevel`
- `/Game/Procedural/Blueprints/Altar`

Known registered `Altar` closure into Fantastic Dungeon is only 16 packages: 3 meshes, 5 materials, and 8 textures. The three mesh anchors are:

- `/Game/Fantastic_Dungeon_Pack/meshes/props/fabrics/SM_PROP_altar_cloth_dungeon_02`
- `/Game/Fantastic_Dungeon_Pack/meshes/props/furniture/SM_PROP_altar_dungeon_01`
- `/Game/Fantastic_Dungeon_Pack/meshes/props/small_deco/SM_PROP_book_dungeon_07`

A registered graph walk seeded from HUB, StorySelection, DoorToLevel, Altar, BP_MassiveDungeon, and BP_StartPoint reached 1,603 packages. Most were cascading FullSample/GASP/Daz dependencies, including 972 under `FullSample/GASP`, 161 under `FullSample/Animations`, 81 under `FullSample/Assets`, 67 under `DazToUnreal/Male`, and 54 under `DazToUnreal/Female`. This is evidence for filtered target resolution, not authorization to import those source roots.

Do not copy `/Game/Calysto/Dungeon`, `/Game/Calysto/Shared`, `/Game/Fantastic_Dungeon_Pack`, or the demo ExternalActors wholesale. Exclude demo maps/external actors unless the Editor-reported dependency closure proves they are required by a product map.

## Registry-opaque packages

There are 67 physically present critical packages missing from the source registry JSON:

- `/Game/_Game/Locations/PCGLevel`: 1.
- `/Game/Procedural/Maps/DungeonGeneration`: 1.
- `/Game/Calysto/Dungeon/**`: 41.
  - Critical Blueprints: `/Game/Calysto/Dungeon/Blueprint/BP_MassiveDungeon` and `/Game/Calysto/Dungeon/Blueprint/Utility/BP_Grid`.
  - Demo map/level-instance packages: 2.
  - Demo `PCGDA_*` packages: 5.
  - `/Game/Calysto/Dungeon/PCG/Function/**`: 28.
  - Root PCG graphs: `PCG_DungeonEditor`, `PCG_Grid`, `PCG_MassiveDungeonMaster`, and `PCG_MassiveDungeonShape`.
- `/Game/Calysto/Shared/**`: 24.
  - Blueprint packages: `Blueprint/BP_PlaceOfInterest`, `Blueprint/BP_SpawnerOverride`, and root `BP_PlaceOfInterest`.
  - Shared PCG graphs: 21.

These packages are `PENDING_REGISTRY_OPAQUE`. Inspect/load them in UE 5.8 only after `EFProcedural` and required PCG dependencies compile. Record their actual dependencies before migration or exclusion.

## Map classification

Source contains 40 maps:

- 35 `SOURCE_ONLY`.
- 3 overlapping and different.
- 2 overlapping and byte-identical.

Core project maps to migrate/inspect:

- `/Game/_Game/Hub/HUB`
- `/Game/_Game/Locations/StorySelection`
- `/Game/_Game/Locations/PCGLevel`
- `/Game/Procedural/Maps/DungeonGeneration`

Different FullSample overlaps; always keep target:

- `/Game/FullSample/FullMap`
- `/Game/FullSample/L_UltOpenWorld`
- `/Game/FullSample/Integrations/SubMaps/L_Gameplay`

Identical overlaps; keep target without copying:

- `/Game/FullSample/Integrations/SubMaps/L_Routine`
- `/Game/FullSample/Integrations/UIIntegrations/Level/MenuMap`

Source-only ACF/sample candidates such as `FullSample/Integrations/UltimateMap` and `FullSample/Integrations/SubMaps/L_Landscape` require individual ownership review. Demo maps from EasyFog, Fantastic Dungeon, Food Props, TattooShop, Kawaii, RealisticBlood, Smoke, Volumetric, and similar roots remain `EXCLUDE_DEMO_OR_LEGACY` unless explicitly required.

## Input policies and unresolved keys

The required project input contract is primarily project-owned native/config behavior, not a `/Game/Input` asset migration:

- `Config/DefaultGame.ini:134-142`: N, C, Y, Comma, J, L, O.
- `EFCharacterCreationSubsystem.cpp`: Period.
- `ProjectSurvivalNeedsSubsystem.cpp`: H.
- `ProjectIntimacySettings.cpp`: Hyphen and Subtract for the Intimacy HUD.

The two `/Game/_Game/Input` packages are TattooShop-specific and source-only:

- `/Game/_Game/Input/IMC_TattooShop_LADS`
- `/Game/_Game/Input/IMC_TattooShopOpen_LADS`

They depend on six InputActions under `/Game/TattooShop/Input/**` and should migrate only with TattooShop.

`Config/DefaultInput.ini:119` names `/AscentCombatFramework/Input/ACF_DefaultMapping`; preserve the ACFU 4.3.5 version. Treat target `/Game/Input/**` as target-authoritative.

Pending:

- Exact owner and behavior of `T`: `PENDING_OWNER_TRACE`.
- Exact meaning/owner of `Plus`: `PENDING_OWNER_TRACE`.
- `Minus` has native evidence for Intimacy via Hyphen/Subtract, but still requires runtime input-contract validation.

## Player and enemy policies

Target-authoritative Player packages:

- `/Game/FullSample/Player`: `OVERLAP_DIFFERENT`, `PROTECT_TARGET_EXACT`.
- `/Game/FullSample/Blueprints/Characters/Player/ACFFullPlayerBP`: `OVERLAP_DIFFERENT`, keep target.
- `/Game/FullSample/Blueprints/Characters/Player/ACF_RiderController_BP`: `OVERLAP_SAME`, keep target.

The source Player references old Daz packages. Never migrate it. Reintroduce source behavior through project-owned components, bridges, adapters, or subclasses on the current 5.8 Player.

Enemy overlaps that differ and must retain target:

- `/Game/FullSample/Blueprints/Characters/Enemies/ACFBaseCompanionBP`
- `/Game/FullSample/Blueprints/Characters/Enemies/ACFDefenderEnemyBP`
- `/Game/FullSample/Blueprints/Characters/Enemies/ACFDummyEnemyBP`
- `/Game/FullSample/Blueprints/Characters/Enemies/ACFGunEnemyBP`
- `/Game/FullSample/Blueprints/Characters/Enemies/ACFMageEnemyBP`
- `/Game/FullSample/Blueprints/Characters/Enemies/ACFMeleeEnemyBP`
- `/Game/FullSample/Blueprints/Characters/Enemies/ACFMMEnemyBP`
- `/Game/FullSample/Blueprints/Characters/Enemies/ACFRangedCompanionBP`
- `/Game/FullSample/Blueprints/Characters/Enemies/ACFRangedEnemyBP`
- `/Game/FullSample/Blueprints/Characters/Enemies/Spider/ACFSpiderBP`
- `/Game/FullSample/Blueprints/Characters/Enemies/Spider/ACFSpiderControllerBP`

`ACFMeleeCompanionBP` is identical and remains target-owned.

Source-only enemy packages requiring migration or reconstruction against ACFU 4.3.5:

- `/Game/FullSample/Blueprints/Characters/Enemies/ACFSampleAIBP`
- `/Game/FullSample/Blueprints/Characters/Enemies/DummyMale`
- `/Game/FullSample/Blueprints/Characters/Enemies/MageMale`
- `/Game/FullSample/Blueprints/Characters/Enemies/MeleeMale`
- `/Game/FullSample/Blueprints/Characters/Enemies/RangedMale`
- `/Game/FullSample/Blueprints/Characters/Enemies/Spider/ACFFullSpiderBP`
- `/Game/FullSample/Blueprints/Characters/Enemies/Spider/ACF_Spider_AS`

The four `*Male` classes are explicitly configured at `Config/DefaultGame.ini:155-158` and `240-243`. They must resolve to the current target Male; never import the source Male.

All 21 `/Game/FullSample/Assets/Characters/Enemy_Great_Spider/**` packages are byte-identical and should remain target-owned.

## Daz, Characters, and Frederick policies

`/Game/DazToUnreal` source inventory:

- 403 source packages.
- 264 source-only.
- 139 overlaps, all byte-different.

`/Game/Characters` source inventory:

- 105 source packages.
- 92 source-only.
- 13 overlaps, all byte-different.

Policy:

- Every overlap is `PROTECT_TARGET_EXACT`.
- Female, Multiple, Male, their Skeletons, PhysicsAssets, Control Rigs, materials, and textures remain target-authoritative.
- Source-only packages are not implicitly approved. Add one only when a required project-owned anchor reaches it, no current target replacement exists, and compatibility is validated.
- Re-hash protected Player/Female/Multiple/Male invariants after the content phase.

Frederick did not resolve to an unambiguous package path by filesystem name or current evidence. Classification remains `PENDING_RUNTIME_IDENTITY`; do not invent a source-to-target mapping or change an asset based only on the display name.

## Required follow-up gates

1. Port and compile the owning project plugins before loading project-owned Blueprint assets.
2. Run Unreal Editor referencer/dependency scans for every `PENDING_REGISTRY_OPAQUE` package.
3. Generate the actual migration closure from each approved anchor in the Editor.
4. Enforce target-protection rules when a closure reaches FullSample, DazToUnreal, Characters, Input, ACFU, or Marketplace content.
5. Resolve T, Plus, and Frederick with runtime/class/path evidence.
6. Compile migrated Blueprints, run PIE and visual QA, cook core maps, and re-hash all protected invariants.
