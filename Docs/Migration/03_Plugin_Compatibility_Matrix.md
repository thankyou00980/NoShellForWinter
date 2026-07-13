# Plugin compatibility matrix

Status: `PHASE4_ALL_PROJECT_MODULES_BUILD_PASS_CONTENT_RUNTIME_PENDING`

The Phase 2 rows preserve the read-only inventory of the live UE 5.7 source tree. Every required and auxiliary project-owned plugin listed below now compiles in its applicable UE 5.8 Editor/Game targets. Phase 4 content statuses are promoted only for exact approved batches; PIE, visual, cook, package, and packaged-runtime claims remain pending until their own gates pass.

## Authoritative external plugins

| Plugin | Descriptor | Location | Version | Runtime/Editor | Classification | Load | Build | Cook/package | Evidence |
|---|---|---|---|---|---|---|---|---|---|
| AscentCombatFramework | `AscentCombatFramework.uplugin` | Engine Marketplace | Source 4.2.3 / Target 4.3.5 | Runtime + Editor | KEEP_TARGET | PASS baseline | PASS | PENDING | target descriptor, baseline logs, protected hash manifest |
| DazToUnreal | `DazToUnreal.uplugin` | Engine | Source 5.7.0.480 / Target 5.8.0.491 | Editor | KEEP_TARGET | PASS baseline | PASS | PENDING | target descriptor, baseline logs, protected hash manifest |
| BpGeneratorUltimate | `BpGeneratorUltimate.uplugin` | Engine Marketplace | Source 1.5.5 / Target 1.7.0 | Editor tooling | KEEP_TARGET | PASS_WITH_WARNINGS | PASS_WITH_WARNINGS | PENDING | UBT/commandlet logs; UECP tool calls blocked by vendor license |
| PCGExtendedToolkit | project/Engine descriptors | Engine Marketplace | Source 0.75.8 / Target 0.76.2 | Runtime + Editor | KEEP_TARGET | PASS baseline | PASS | PENDING | descriptors + build logs |
| SkinnedDecalComponent | `SkinnedDecalComponent.uplugin` | Engine Marketplace | Source 3.0 / Target 3.0.1 | Runtime | KEEP_TARGET | PASS baseline | PASS | PENDING | descriptors + build logs |

## Project-owned plugin inventory

Every source descriptor below is rooted at `D:\Projects UE5\LustAsDeadlySin\Plugins\<Plugin>\<Plugin>.uplugin`. The original Phase 2 audit found none of these directories in the target; all listed project-owned plugins have since been migrated or rebuilt under target ownership and pass their applicable UE 5.8 compile gates.

| Plugin | Version | Modules, type, loading phase | Descriptor plugin edges | Source files | Public surface | Content/config | Classification | Phase status |
|---|---:|---|---|---:|---|---|---|---|
| EFCharacterCreation | 1.0.0 | `EFCharacterCreationRuntime` Runtime/Default; `EFCharacterCreationEditor` Editor/Default | none | 25 | 12 headers; 8 UCLASS, 9 USTRUCT, 1 UENUM | 2 WBP + recomposed project `DefaultGame.ini` section | MIGRATE_SOURCE_WITH_CONFIG_MERGE | PHASE3_EDITOR_GAME_ASSETS_CONFIG_PASS / RUNTIME_QA_PENDING |
| EFCharacterCreationDazBridge | 1.0.0 | `EFCharacterCreationDazBridgeEditor` Editor/Default | EFCharacterCreation, DazToUnreal, DeformerGraph enabled in target | 3 | 1 module header | none | REBUILD_AGAINST_TARGET | PHASE3_EDITOR_BUILD_READ_ONLY_PROBE_PASS / MUTATING_COMMANDS_BLOCKED |
| EFClothingMorph | 1.0.0 | `EFClothingMorphRuntime` Runtime/Default | EFCharacterCreation | 4 | 1 header; 1 UCLASS, 8 USTRUCT, 2 UENUM | none | MIGRATE_SOURCE | PHASE3_EDITOR_GAME_BUILD_PASS / PLAYER_INTEGRATION_QA_PENDING |
| EFProcedural | 1.0.0 | `EFProceduralRuntime`, `EFProceduralACFURuntime`, `EFProceduralPCGRuntime` Runtime/Default; `EFProceduralEditor` Editor/Default | PCG | 26 | 13 headers; 4 UCLASS, 3 UINTERFACE | exact 20-package contract batch present; dungeon actor/map/door deferred | MIGRATE_SOURCE | PHASE4_EDITOR_GAME_AND_EXACT_CONTRACT_CONTENT_PASS / PIE_VISUAL_COOK_PACKAGE_DUNGEON_RUNTIME_PENDING |
| EFLevelFlow | 1.0.0 | `EFLevelFlowRuntime` Runtime/Default | AscentCombatFramework, EFProcedural, EFCharacterCreation | 7 | 3 headers; 2 UCLASS | native defaults; source-only DungeonGeneration map deferred | REBUILD_AGAINST_TARGET | PHASE4_EDITOR_GAME_BUILD_PASS / CONTENT_RUNTIME_COOK_PENDING |
| EFCharacterCreationACFUBridge | 1.0.0 | `EFCharacterCreationACFURuntime` Runtime/Default | EFCharacterCreation, GameplayAbilities | 5 | 2 headers | none | REBUILD_AGAINST_TARGET | PHASE3_EDITOR_GAME_BUILD_PASS / RUNTIME_QA_PENDING |
| EFProjectSystems | 0.1.0 | `EFProjectSystemsCore`, `EFProjectSystemsGameplay`, `EFProjectSystemsUI` Runtime/Default; `EFProjectSystemsEditor` Editor/Default | GameplayAbilities, EnhancedInput, AscentCombatFramework, ACFTrainingSystem, EFCharacterCreation, EFLevelFlow, EFProcedural, CodeWidgetDesignerBridge, DirtyPawnRuntime, SkinnedDecalComponent | 288 | 144 headers; 217 UCLASS, 91 USTRUCT, 31 UENUM, 4 UINTERFACE | plugin Content empty; 5 Core Redirects, 58 `Project.*` tags, and 6 settings sections applied selectively; `/Game` dependencies pending | MIGRATE_SOURCE_AFTER_DEPENDENCIES | PHASE3_EDITOR_GAME_CONFIG_STRUCTURAL_NATIVE_AUTOMATION_PASS / CONTENT_BLUEPRINT_PIE_VISUAL_COOK_PACKAGE_PENDING |
| EFBlink | 0.1.0 | `EFBlinkRuntime` Runtime/Default | none | 11 | 5 headers; 3 UCLASS, 1 USTRUCT | none | MIGRATE_SOURCE | PHASE3_EDITOR_GAME_BUILD_PASS / CONFIG_AND_VISUAL_QA_PENDING |
| DirtyPawnRuntime | 1.0.0 | `DirtyPawnRuntime` Runtime/Default; `DirtyPawnRuntimeEditor` Editor/Default | none | 12 | 5 headers; 16 UCLASS, 5 USTRUCT, 3 UENUM | portability manifest only | MIGRATE_SOURCE | PHASE3_EDITOR_GAME_BUILD_PASS / CONTENT_AND_QA_PENDING |
| ACFTrainingSystem | 0.1.0 | `ACFTrainingSystem` Runtime/Default | GameplayAbilities, AscentCombatFramework | 11 | 5 headers; 3 UCLASS, 5 USTRUCT, 1 UENUM | 1 project AnimSequence migrated through AssetTools 5.7 and resaved 5.8 | REBUILD_AGAINST_TARGET | PHASE3_EDITOR_GAME_ASSET_PASS / RUNTIME_COOK_QA_PENDING |
| CodeWidgetDesignerBridge | 0.1.0 | `CodeWidgetDesignerBridge` Runtime/Default; `CodeWidgetDesignerBridgeEditor` Editor/Default | none | 22 | 2 public headers; 1 UCLASS, 5 USTRUCT, 2 UENUM, 1 UINTERFACE | none | REBUILD_AGAINST_UE58_EDITOR | PHASE3_EDITOR_GAME_BUILD_PASS / QA_PENDING |

`CanContainContent=true` is present on EFCharacterCreation, EFProjectSystems, DirtyPawnRuntime, and ACFTrainingSystem, but only EFCharacterCreation currently contains plugin assets. Project DataAssets, DataTables, Widget Blueprints, maps, input assets, and other `/Game` dependencies must therefore be handled by the content manifest; porting plugin source alone is not a subsystem-complete migration.

EFProjectSystems was imported through a 289-file descriptor-plus-Source allowlist totaling 3,491,166 bytes. Ten project-owned files differ intentionally from that receipt: the ACFU signature adaptation, UE 5.8 delegate and runtime/editor separation fixes, and remapping of absent legacy enemy classes to the three target-authoritative ACF enemy assets.

Its Phase 3 read-only UE 5.8 probe passed 46/46 checks across ten representative native classes/CDOs, effective input/survival/release settings, five redirect lines, and 58 project tags. At that point it found the three authoritative enemy packages and eight missing soft packages, plus one pending PNG outside AssetRegistry. It did not load maps or content objects, compile Blueprints, save assets, or run PIE. A separate strict native Automation gate passed 71/71 tests with no warnings; the later exact Phase 4 content batches have their own evidence and do not promote the deferred PIE gates.

The approved EFProcedural content increment contains exactly 20 packages: 19 Calysto data contracts plus the canonical `/Game/Calysto/Dungeon/Blueprint/Utility/BP_StartPoint`. UE 5.7 AssetTools migration, UE 5.8 load/compile/resave, Editor/Game builds, source read-only verification, and protected-target re-hash all pass. `/Game/Calysto/Dungeon/Blueprint/BP_MassiveDungeon`, `/Game/Procedural/Maps/DungeonGeneration`, and `/Game/Procedural/DoorToLevel` remain excluded and pending. Evidence: [Phase4_ProceduralContracts_ContentBuild.json](Evidence/Phase4_ProceduralContracts_ContentBuild.json).

## Dependency DAG

The project-owned dependency graph is acyclic:

```text
EFCharacterCreation
|- EFCharacterCreationDazBridge
|- EFClothingMorph
|- EFCharacterCreationACFUBridge
`- EFLevelFlow

EFProcedural
`- EFLevelFlow

ACFTrainingSystem ---------.
CodeWidgetDesignerBridge --+
DirtyPawnRuntime ----------+
EFCharacterCreation -------+
EFProcedural --------------+
EFLevelFlow ---------------`- EFProjectSystems

EFBlink (independent leaf)
```

Module-internal edges add no cycles: each Editor module depends on its Runtime module; `EFProceduralPCGRuntime` depends on `EFProceduralRuntime` and `EFProceduralACFURuntime`; `EFProjectSystemsEditor` depends on Core, Gameplay, EFLevelFlowRuntime, and EFProceduralEditor.

## Required port order and current state

Steps 1 through 7 are complete at the source-port and applicable Editor/Game compile level. Their subsystem runtime, visual, cook, package, and packaged-runtime gates remain independent.

1. Port and compile independent leaves: CodeWidgetDesignerBridge, DirtyPawnRuntime, EFBlink.
2. Port EFCharacterCreation; migrate its two WBP assets through Unreal and merge, rather than replace, `Config/DefaultGame.ini`.
3. Port EFCharacterCreationDazBridge, EFClothingMorph, and EFCharacterCreationACFUBridge.
4. Port ACFTrainingSystem and validate its ARS/GAS contract against ACFU 4.3.5.
5. Port EFProcedural in module order: Runtime, ACFURuntime, PCGRuntime, Editor.
6. Port EFLevelFlow after EFCharacterCreation and EFProcedural are green.
7. EFProjectSystems descriptor/source import, Editor/Game build, selective structural config probe, and strict native Automation gate are complete. Exact core, Modern UI, and procedural-contract content batches have static validation; remaining content, PIE, visual QA, cook, package, and packaged runtime remain pending.

Passing the source port does not make a subsystem complete: each wave still needs its applicable load, Blueprint, PIE, cook, package, and protected-hash evidence. No `Binaries`, `Intermediate`, or `Saved` subtree is a port input.

## Confirmed ACFU 4.3.5 signature change

This is a confirmed compile blocker, not a speculative hotspot:

- Project call: `D:\Projects UE5\LustAsDeadlySin\Plugins\EFProjectSystems\Source\EFProjectSystemsGameplay\SinfulAscension\ProjectSinfulAscensionComponent.cpp:2590`
- ACFU 4.2.3 declaration: `D:\Unreal Engine 5\Library\UE_5.7\Engine\Plugins\Marketplace\AscentCoa789c5ab7b4cV4\Source\CollisionsManager\Public\ACMEffectsDispatcherComponent.h:20`
- ACFU 4.3.5 declaration: `D:\Unreal Engine 5\Library\UE_5.8\Engine\Plugins\Marketplace\ACFUAsce5ab7c1439afbV5\Source\CollisionsManager\Public\ACMEffectsDispatcherComponent.h:20`

`UACMEffectsDispatcherComponent::PlayReplicatedActionEffect` changed from two arguments to three and now requires `const FComponentFX& outComps`. The target-owned compatibility patch constructs an `FComponentFX` and passes it as the third argument. UE 5.8 Editor and Game builds pass; runtime hit-feedback validation remains pending. ACFU 4.3.5 was not modified.

## UE 5.8 / ACFU compatibility hotspots

| Plugin | Initial hotspot/gate |
|---|---|
| EFCharacterCreation | Merge settings; reject old `AmalaforGenesis9` body option and `bAutoEnterTestingMap=True` unless explicitly retained; validate runtime WidgetTree/input/camera with authoritative Female Player. |
| EFCharacterCreationDazBridge | High-risk editor APIs around Control Rig, AnimBlueprint graph generation, post-process assignment, and `CreateAutoJCMControlRig.py`; audit-only before any repair command. |
| EFClothingMorph | Runtime reflection of InventorySystem class/property/delegate names plus morph-map and leader-pose behavior; equip/unequip probe required. |
| EFProcedural | PCG delegates, `GenerateLocal`, navigation rebuild/invokers, and spawned-pawn sanitation; dungeon PIE required. |
| EFLevelFlow | Direct AIFramework threat APIs plus input/camera/loading state restoration. |
| EFCharacterCreationACFUBridge | Reflected `ACFCharacterMovementComponent.SetCanMove` and GAS `CancelAllAbilities`; both need runtime contract probes. |
| EFBlink | Low compile risk; authoritative Female morph existence and visible blink QA remain required. |
| DirtyPawnRuntime | Dynamic material/height-mask runtime plus KismetCompiler editor module; visual wet/mud/blood/smear/snow/sand QA required. |
| ACFTrainingSystem | ARS modifier handles, ACF GAS attribute tags, GameplayEffect fallback, replication, progress/save, and minigame delegates. |
| CodeWidgetDesignerBridge | High-risk UMGEditor/KismetCompiler/WidgetBlueprint factories and commandlets; compile commandlets before using them to mutate any WBP. |
| EFProjectSystems | Highest risk: broad ACF module surface, direct inheritance/interfaces, reflection, 144 effectively public headers, and the confirmed `FComponentFX` signature change. `bUseUnity=false` should remain for the first strict build. |

Detailed evidence and exact descriptor hashes are recorded in `Docs/Migration/Evidence/Phase2_Plugin_Dependency_Audit.md`.
