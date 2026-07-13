# Configuration merge

Status: `PHASE4_CORE_CONTENT_SELECTIVE_PASS`

This document began as the Phase 2 three-way decision set. Phase 3 contains the controlled additive plugin/config merges, and Phase 4 now adds the exact EFProjectSystems core-content batch recorded below. The Phase 4 PASS is limited to the enumerated assets, effective settings, Blueprint compile/resave, native automation, and Editor/Game builds; PIE input behavior, visual QA, cook, and packaged runtime remain pending.

Evidence: [Phase2_Config_ThreeWay_Audit.md](Evidence/Phase2_Config_ThreeWay_Audit.md)

Phase 4 evidence: [Phase4_EFProjectCoreContent_ConfigAutomation.json](Evidence/Phase4_EFProjectCoreContent_ConfigAutomation.json)

## Authorities

- Source gameplay intent: `D:\Projects UE5\LustAsDeadlySin`, read-only, Git HEAD `8808ee6493b6c8e73b58fca30d9d5f1c1bca77b4`.
- Writable UE 5.8 target: `D:\Projects UE5\NoShellForWinter`.
- ACF authority: `D:\Unreal Engine 5\Library\UE_5.8\Engine\Plugins\Marketplace\ACFUAsce5ab7c1439afbV5\Config`.
- Daz authority: the complete current target section `[/Script/DazToUnreal.DazToUnrealSettings]`.
- Project-owned additions are distinguished from the original import with source history `1048d76..HEAD` and then compared against ACFU 4.3.5.

## Three-way decision matrix

| Target file | Section/key | Source UE 5.7 | Target UE 5.8 / ACFU 4.3.5 | Decision | Gate |
|---|---|---|---|---|---|
| `DefaultEngine.ini` | `[CoreRedirects]` ACF base | Two ACF package redirects plus older ACF redirect files | Current ACF base plus four validated `TP_ThirdPerson -> NoShellForWinter` redirects | `KEEP_TARGET` | Re-hash target ACFU and protected assets |
| `DefaultEngine.ini` | Project module/class redirects | 47 project-owned Calysto, ACFUltimateSample, Project, and EFEspabilar redirects | Absent | `DEFER_MERGE` additively; never replace target redirects | Required project modules load and redirect destinations resolve |
| `DefaultEngine.ini` | 419 lines in source `Plugins/ACFUltimate/Config/ACFUPlugin.ini` | Unsectioned historical package redirect candidates | Absent | `REJECT_BULK`; promote individually only from dependency evidence | Old reference exists, new package exists, load test passes |
| `DefaultEngine.ini` | `GameDefaultMap`, transition, server mode, GameInstance | Same effective ACF sample routes as target | Baseline runtime passes | `KEEP_TARGET` during migration | PIE and cook remain green |
| `DefaultEngine.ini` | `GlobalDefaultGameMode` | `/Game/FullSample/Integrations/ACF_GASUltimateGameMode_BP...` | Current ACFU mode route | `DEFER_MERGE` through a project-owned compatible asset or adapter | Source mode migrated, compiles against ACFU 4.3.5, PIE passes |
| `DefaultEngine.ini` | `EditorStartupMap` | `/Game/_Game/Hub/HUB.HUB` | `/Game/FullSample/L_UltOpenWorld.L_UltOpenWorld` | `DEFER_MERGE` | HUB and dependencies migrated, load, PIE and cook pass |
| `DefaultEngine.ini` | Game user settings class | Old project `/Game/Integrations/.../ACF_GameSettings_BP` | `/AscentCombatFramework/UITools/Blueprints/AUT_GameSettings_BP...` | `KEEP_TARGET` | Current class loads |
| `DefaultEngine.ini` | Windows RHI | DX11 | DX12 SM6 | `KEEP_TARGET_PENDING_COMPARISON`; do not force DX11 | DX12 visual/performance gate, optional DX11 comparison |
| `DefaultEngine.ini` | Project render policy | 2048 MB pool, pool-to-VRAM, culling on, motion blur off, VSM off | Only virtual textures explicitly set | `DEFER_MERGE` exact source-owned deltas | UE 5.8 CVar validation plus visual/performance QA |
| `DefaultEngine.ini` | UI settings | 1920x1080 design scale, shortest-side DPI, focus never | Absent | `DEFER_MERGE` | UMG layout and packaged HUD QA |
| `DefaultEngine.ini` | `ConsoleVariables` | `fx.Niagara.ForceLastTickGroup=1` | Absent | `DEFER_MERGE` | Niagara behavior and performance smoke |
| `DefaultEngine.ini` | Android file server token | Source-local generated token | Different target-local generated token | `REJECT_SOURCE`; do not copy secrets/tokens | Rotate or remove target token before release if versioned |
| `DefaultGame.ini` | Current Daz settings | Old two-key override, including `ZeroRootRotationOnImport=False` | Full current Daz settings, including `ZeroRootRotationOnImport=True` and Multiple skeleton mapping | `KEEP_TARGET` complete section | Daz and protected mesh hashes unchanged |
| `DefaultGame.ini` | ACFU CommonUI/CommonInput | Present in source baseline and ACFU 4.3.5 | Missing because target file currently contains only Daz settings | `DEFER_MERGE_FROM_ACFU_4_3_5` | Config parse and baseline UI/input smoke |
| `DefaultGame.ini` | ACFU always-cook roots | `/AscentCombatFramework`, `/Game/FullSample` | Missing from project file; present in ACFU 4.3.5 template | `DEFER_MERGE_FROM_ACFU_4_3_5` | Cook manifest confirms roots |
| `DefaultGame.ini` | `/Game/_Game/Widgets` always-cook root | Project-owned addition | Absent | `DEFER_MERGE` | Root migrated and cook reference audit passes |
| `DefaultGame.ini` | `/NNEDenoiser` always-cook root | Legacy source entry | Absent | `REJECT_UNLESS_REFERENCED` | NNEDenoiser enabled and an asset dependency proves need |
| `DefaultGame.ini` | EFProjectSystems, EFProcedural, EFCharacterCreation, GameplayAbilities sections | Project-owned settings added after initial import | Absent | `DEFER_MERGE` section-by-section | Owning plugin loads; every configured asset path resolves |
| `DefaultGame.ini` | Temporary gameplay tuning | Pain `0.25`, sensation hold `30`, dance lust `0.10` with `TEMP_TEST` comments | Absent | `REJECT_TEMP_VALUES`; release candidates are `0.01`, `10`, `0.05` | Owning native tests and gameplay QA |
| `DefaultGame.ini` | Packaging legacy keys | Includes Blueprint nativization-era keys | Absent | `REJECT_UE5_LEGACY`; do not copy file wholesale | Validate desired packaging through UE 5.8 settings/CDO |
| `DefaultGameplayTags.ini` | ACF tag tables and ACF tags | Older ACF table paths and fewer tables | Exact current ACFU 4.3.5 file | `KEEP_TARGET` | ACF tag resolution smoke |
| `DefaultGameplayTags.ini` | `Project.Intimacy.*`, `Project.Gender.*` | 58 project-owned tag lines | Absent | `DEFER_MERGE` additively | EFProjectSystems loads; all 58 tags resolve |
| `DefaultInput.ini` | Enhanced Input settings | Older UE 5.7 layout/default mapping entry | Exact current ACFU 4.3.5 file | `KEEP_TARGET`; do not copy source file wholesale | ACF actions and input modes pass PIE |
| `DefaultInput.ini` | `ConsoleKeys=Equals` | Present | Present in ACFU/target | `DEFER_REMOVE_EQUALS` because it conflicts with top-row Plus | Plus interaction works; console still accessible on approved key |
| `DefaultGameUserSettings.ini` | Scalability defaults | Project low-cost profile | Exact ACFU high profile | `DEFER_MERGE_SOURCE_PROFILE` | UE 5.8 CVar validation and visual/performance QA |
| `DefaultScalability.ini` | Source quality overrides | Project-owned file added after initial import | Missing | `DEFER_ADD_FILE` | Same gate as GameUserSettings |
| `DefaultPlugins.ini` | ACF developer settings | Old ACF modules/paths | Exact current ACFU 4.3.5 file | `KEEP_TARGET` | ACF developer settings and vendor assets load |
| `DefaultAscentCombatFramework.ini`, `BaseAscentCombatFramework.ini` | ACF redirects | Old, much smaller 5.7-era copies | Current large ACFU 4.3.5 plugin copies | `REJECT_SOURCE` | No missing-module regression in Blueprint/load tests |
| `DefaultEditor.ini` | Preview profiles | UE 5.7 serialized post-process schema | UE 5.8 schema with new fields/version | `KEEP_TARGET`; never copy old profile rows | Editor asset preview smoke |
| `DefaultEditor.ini`, `DefaultEditorPerProjectUserSettings.ini` | `ThirdPersonCPP` editor paths | Not applicable | Stale target-only paths | `DEFER_CLEANUP` | Confirm no editor workflow depends on them |
| `NoShellForWinter.uproject` | Engine/module identity | Engine 5.7, module `ACFSample` | Engine 5.8, module `NoShellForWinter` | `KEEP_TARGET` | Project generation and cold build |
| `NoShellForWinter.uproject` | ACFU and Daz entries | Old ACF URL/version and project-local old Daz | Current ACFU and current Daz | `KEEP_TARGET` | Plugin/version/hash invariants |
| `NoShellForWinter.uproject` | Project-owned plugins | Source explicitly enables the aggregate and several support plugins; required plugins also appear through dependencies | Absent | `DEFER_MERGE` explicit required entries after plugin migration | Descriptor validation, project generation, build |
| `NoShellForWinter.uproject` | Optional engine feature plugins | PCG interops, ScriptableTools, DeformerGraph, MLDeformer, Volumetrics, NiagaraFluids, WaterAdvanced | Only a subset enabled | `DEFER_BY_CONTENT_DEPENDENCY` | Phase 2 manifest proves references and UE 5.8 plugin exists |

## Exact target preservations

At the Phase 2 baseline, `DefaultInput.ini`, `DefaultGameplayTags.ini`, `DefaultPlugins.ini`, and `DefaultGameUserSettings.ini` were byte-identical to ACFU 4.3.5. Phase 3 now has two intentional project-owned divergences:

- `DefaultInput.ini` preserves the UE 5.8 Enhanced Input base and `+ConsoleKeys=Tab`, while applying `-ConsoleKeys=Equals` and removing `+ConsoleKeys=Equals`.
- `DefaultGameplayTags.ini` preserves every current ACFU tag/table row and adds exactly 58 unique `Project.*` tags.

`DefaultPlugins.ini` and `DefaultGameUserSettings.ini` remain target-authoritative.

Preserve the full current Daz section in `Config/DefaultGame.ini`; do not merge individual source Daz values.

Preserve these `.uproject` identities and entries:

- `EngineAssociation: 5.8`
- module `NoShellForWinter` with `Engine`, `AIModule`, and `UMG`
- current `AscentCombatFramework` and `DazToUnreal`
- `ModelingToolsEditorMode`, `StateTree`, `GameplayStateTree`
- `OneClickMaterials`, `BpGeneratorUltimate`, `PCGExtendedToolkit`, `SkinnedDecalComponent`

## Project configuration ledger

Merge only owner sections, never the surrounding UE 5.7 file. Current disposition:

- `APPLIED_STRUCTURAL_PASS`: `EFProjectInputSettings`, `EFProjectEnemySettings`, `ProjectEnemyVisualVariationSettings`, `ProjectEnemyLevelSettings`, `ProjectActivityFeedSettings`, and `ProjectSinfulAscensionSettings`.
- `APPLIED_EARLIER`: `EFCharacterCreationSettings`.
- `APPLIED_CONTENT_STRUCTURAL_PASS`: `EFProjectSurvivalSettings`, `ProjectDefeatFlowSettings`, and `ProjectCharacterBackgroundSettings` for the exact Phase 4 batch.
- `PENDING_CONTENT`: `EFProjectWorldFlowSettings`, `ProjectRuntimePerformanceSettings`, and `EFProceduralSettings`.
- `PENDING_GAS_CUE_VALIDATION`: `AbilitySystemGlobals`.

Configured source-only dependencies still include HUB, DungeonGeneration, DoorToLevel, and `_Game/Widgets`. The exact Phase 4 batch now supplies the survival data, food registry, defeat UI and its self-contained visual dependencies, character-background data/UI, and `_Game/Images/preview.png`; this does not authorize migration of the remaining roots.

Phase 3 has applied only the four EFProcedural class redirects after its Runtime and PCG modules passed Editor/Game builds. The `EFProceduralSettings` section remains deferred: its three configured dungeon asset paths are absent from target Content, and the isolated UE 5.7 registry closure is evidence for inspection only, not an authorized migration allowlist.

The two EFLevelFlow class redirects were then applied after its destination module passed Editor/Game builds. EFLevelFlow has no explicit source settings section to merge; its native CDO defaults and ACFU loading widget passed a read-only UE 5.8 probe. Serialized redirect resolution and the DungeonGeneration runtime remain pending until the gated map migration exists.

### EFProjectSystems selective merge

Applied after all four EFProjectSystems modules passed UE 5.8 Editor and Game builds:

- EFProjectSystems is explicitly enabled in `NoShellForWinter.uproject`.
- Five project-owned Core Redirect lines were added. Destination modules/classes load; serialized legacy-reference resolution remains `PENDING_ALLOWLISTED_ASSET_LOAD`.
- Exactly 58 unique `Project.Intimacy.*` and `Project.Gender.*` tags were added.
- Six owner sections were merged: `EFProjectInputSettings`, `EFProjectEnemySettings`, `ProjectEnemyVisualVariationSettings`, `ProjectEnemyLevelSettings`, `ProjectActivityFeedSettings`, and `ProjectSinfulAscensionSettings`.
- Legacy `MeleeMale`, `RangedMale`, `MageMale`, and `DummyMale` paths were not retained. Settings use target-authoritative `ACFMeleeEnemyBP`, `ACFRangedEnemyBP`, and `ACFMageEnemyBP`.
- Release values were retained: sensation hold `10.0`, dance lust `0.05`, and native defeat pain `0.01`. Temporary `30.0`, `0.10`, and `0.25` values were rejected.
- CommonUI/CommonInput settings and the `/AscentCombatFramework`, `/Game/FullSample`, and `/Game/ExportedAnimations` always-cook roots were restored. Their config presence is structurally verified; cook-manifest validation remains `PENDING_COOK`.
- `Equals` was released from console ownership while `Tab` remains available.

The UE 5.8 read-only structural probe still passes all 46 checks. After the exact Phase 4 migration it resolves 9 of its 11 soft-package contracts; only `/Game/Procedural/DoorToLevel` and `/Game/Procedural/Maps/DungeonGeneration` remain absent. The raw `Content/_Game/Images/preview.png` contract is now present and hash-verified. This probe is structural evidence only and does not substitute for map load, PIE, visual, cook, or packaged validation.

### Phase 4 EFProjectSystems core-content batch

The controlled Phase 4 action migrated exactly 31 UE packages: 19 core packages covering survival statuses, the food consumable registry, character-background tables/UI, defeat UI, and status icons, plus 12 self-contained font/texture dependencies used by the defeat widget. UE 5.7 AssetTools performed the source-side exact migration in a detached read-only harness; UE 5.8 then loaded, compiled where applicable, resaved, and validated the target packages.

Phase 4 also:

- imported the hash-verified raw `_Game/Images/preview.png` sidecar and generated `/Game/_Game/Images/T_ProjectCharacterBackgroundPreview` as the packaged Texture2D counterpart;
- applied the effective survival, defeat, and character-background settings, retaining the release defeat pain value `0.01`;
- passed the focused EFProjectSystems automation gate with 9 clean successes and no warnings, failures, or unrun tests;
- passed UE 5.8 Editor and Game builds after the content/config and project-owned runtime preload updates; and
- revalidated the source read-only gate and the protected ACFU, DazToUnreal, Player, Female, Frederick, Multiple, and Male invariants.

These results are not a full gameplay-system PASS. DoorToLevel and DungeonGeneration remain absent, and the complete input contract, PIE behavior, visual presentation, cook manifest, package, and packaged runtime are still `PENDING`.

## Applied ACFU 4.3.5 repair to `DefaultGame.ini`

The controlled merge restored these ACFU 4.3.5 values while retaining the complete Daz section:

```ini
[/Script/CommonUI.CommonUISettings]
CommonButtonAcceptKeyHandling=TriggerClick

[/Script/CommonInput.CommonInputSettings]
InputData=/CommonUI/GenericInputData.GenericInputData_C

[/Script/UnrealEd.ProjectPackagingSettings]
+DirectoriesToAlwaysCook=(Path="/AscentCombatFramework")
+DirectoriesToAlwaysCook=(Path="/Game/FullSample")
+DirectoriesToAlwaysCook=(Path="/Game/ExportedAnimations")
```

The project-owned `/Game/_Game/Widgets` root is a separate gated addition. Do not restore `/NNEDenoiser` without a proven dependency.

## Gameplay tags

Exactly 58 `Project.*` lines from the source were added while preserving all current ACFU 4.3.5 table routes, including:

- `/AscentCombatFramework/Configuration/NavigationMarkerTags`
- `/AscentCombatFramework/Configuration/GameplayCueTags`
- `/AscentCombatFramework/Configuration/TurnAbilityTags`
- `/AscentCombatFramework/TurnCombat/Configuration/ACFJ_AbilityTag`
- `/AscentCombatFramework/Editor/DesignerMode/ACFCreatorTags`
- `/AscentCombatFramework/Configuration/LadderClimbingTags`

Also preserve the current `GameplayCue.ACF.PickUp` tag. Do not reintroduce the old source table substitutions under `/Game/FullSample`.

## Input contract

The project settings source of truth is:

| Contract | Source binding mechanism | Required result |
|---|---|---|
| O | `ToggleGameplayFreeCameraKey=O` | Free camera |
| Period | Direct `EKeys::Period` in EFCharacterCreation | Character creator |
| L | `ToggleGameplayDebugMenuKey=L` | Debug menu |
| Comma | `ToggleNeedsHudKey=Comma` | Full Needs & Status HUD |
| N | `ToggleWalkKey=N` | Custom walk |
| C | `ToggleCrawlKey=C` | Crawl |
| Y | `ToggleInteractionMenuKey=Y` | Actions/emotes/interactions |
| J | `ToggleActivityFeedKey=J` | Chronicle |
| H | Direct conditional `EKeys::H` in survival subsystem | Conditional status debug |
| Minus | Direct `Hyphen` and `Subtract` defaults in intimacy settings | Preserve both keyboard variants |
| T / Plus | Not represented in textual config; owned by input assets/Blueprint contracts | `PENDING` asset inspection and PIE |

`Equals` has been removed from console ownership without replacing the UE 5.8 Enhanced Input section. The structural probe validates CDO/config only; the complete Period, H, O, N, C, Y, J, L, Comma, Down, T, Plus, Hyphen, and Subtract behavior remains `PENDING_PIE`.

## Core redirect disposition

Promote the 47 project-owned redirect candidates from source `Config/DefaultEngine.ini` only in validated subsets after their destination modules exist. The applied EFProcedural, EFLevelFlow, and EFProjectSystems subsets are recorded above; the remaining candidates cover:

- CalystoCharacterCreation to EFCharacterCreation
- CalystoLevelFlow to EFLevelFlow
- CalystoProceduralDungeon to EFProcedural
- selected ACFUltimateSample character-creation and project symbols to EF modules
- EFEspabilarRuntime to EFBlinkRuntime

The 419 unsectioned redirect candidates in source `Plugins/ACFUltimate/Config/ACFUPlugin.ini` are not an authorized config payload. Audit results:

- 319 have missing old package and existing new package in source.
- 79 have existing old package but missing new package.
- 17 have neither side.
- 1 has both sides and is ambiguous.
- 2 use NiagaraFluids rather than `/Game`.
- Only the old Ultimate Player to current `/Game/FullSample/Player.Player` destination already exists in target.

Never place these 419 lines in the marketplace plugin. Any selected redirect belongs in project-owned `[CoreRedirects]` and needs a reference/load test.

## `.uproject` project-owned additions

These project-owned plugins have now been explicitly enabled after their descriptors were migrated and validated by their recorded build gates:

- EFCharacterCreation
- EFCharacterCreationDazBridge
- EFClothingMorph
- EFProcedural
- EFLevelFlow
- EFCharacterCreationACFUBridge
- EFProjectSystems
- EFBlink
- ACFTrainingSystem
- DirtyPawnRuntime
- CodeWidgetDesignerBridge

PCG interops, ScriptableTools, DeformerGraph, MLDeformerFramework, Volumetrics, NiagaraFluids, and WaterAdvanced all exist in UE 5.8, but enable each only when the content manifest or a required plugin descriptor proves the dependency.

## Reject list

- Source ACFU plugin and its old Marketplace URL.
- Source DazToUnreal plugin/config and `ZeroRootRotationOnImport=False`.
- Whole-file copies of `DefaultInput.ini`, `DefaultGameplayTags.ini`, `DefaultPlugins.ini`, `DefaultEngine.ini`, or `DefaultGame.ini`.
- Source `BaseAscentCombatFramework.ini` and `DefaultAscentCombatFramework.ini`.
- UE 5.7 `AdvancedPreviewScene.SharedProfiles` serialized rows.
- Blueprint-nativization packaging keys.
- Source Android file server token.
- Temporary gameplay tuning values called out above.
- All 419 historical package redirects as a bulk operation.

## Execution and validation order

1. Migrate and build required project-owned plugins without changing config ownership.
2. Migrate the referenced source-only assets through the Phase 2 content manifest.
3. Add project redirects whose destination modules now load.
4. Restore the missing ACFU 4.3.5 `DefaultGame.ini` sections while preserving Daz.
5. Merge project-owned `DefaultGame.ini` sections and 58 tags.
6. Resolve the Equals/Plus conflict, then validate the entire input contract in PIE.
7. Apply scalability/render policy only after UE 5.8 CVar validation.
8. Switch HUB/GameMode routes only after load, Blueprint compile, PIE, visual, cook, and packaged gates.
9. Inspect effective output under `Saved/Config/WindowsEditor`, then re-hash ACFU, Daz, Player, Female, Frederick, Multiple, and Male.

Rows explicitly marked `APPLIED_STRUCTURAL_PASS` have build, native-load, and config evidence only. `APPLIED_CONTENT_STRUCTURAL_PASS` additionally covers content resolution and Blueprint compile/resave only for the exact 31-package Phase 4 batch. PIE input, visual QA, cook, package, and packaged runtime remain execution gates.
