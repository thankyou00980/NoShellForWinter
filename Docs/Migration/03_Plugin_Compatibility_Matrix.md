# Plugin compatibility matrix

Status: `PHASE2_INVENTORIED / PENDING_PORT`

This phase is a read-only inventory of the live UE 5.7 source tree. `PHASE2_INVENTORIED` means that the descriptor, module graph, `Build.cs` dependencies, public C++ surface, plugin content/config roots, and initial UE 5.8 compatibility hotspots were inspected. It does not mean that a project-owned plugin has been copied, built, loaded, cooked, packaged, or runtime-validated in the target.

## Authoritative external plugins

| Plugin | Descriptor | Location | Version | Runtime/Editor | Classification | Load | Build | Cook/package | Evidence |
|---|---|---|---|---|---|---|---|---|---|
| AscentCombatFramework | `AscentCombatFramework.uplugin` | Engine Marketplace | Source 4.2.3 / Target 4.3.5 | Runtime + Editor | KEEP_TARGET | PASS baseline | PASS | PENDING | target descriptor, baseline logs, protected hash manifest |
| DazToUnreal | `DazToUnreal.uplugin` | Engine | Source 5.7.0.480 / Target 5.8.0.491 | Editor | KEEP_TARGET | PASS baseline | PASS | PENDING | target descriptor, baseline logs, protected hash manifest |
| BpGeneratorUltimate | `BpGeneratorUltimate.uplugin` | Engine Marketplace | Source 1.5.5 / Target 1.7.0 | Editor tooling | KEEP_TARGET | PASS_WITH_WARNINGS | PASS_WITH_WARNINGS | PENDING | UBT/commandlet logs; UECP tool calls blocked by vendor license |
| PCGExtendedToolkit | project/Engine descriptors | Engine Marketplace | Source 0.75.8 / Target 0.76.2 | Runtime + Editor | KEEP_TARGET | PASS baseline | PASS | PENDING | descriptors + build logs |
| SkinnedDecalComponent | `SkinnedDecalComponent.uplugin` | Engine Marketplace | Source 3.0 / Target 3.0.1 | Runtime | KEEP_TARGET | PASS baseline | PASS | PENDING | descriptors + build logs |

## Project-owned plugin inventory

Every descriptor below is rooted at `D:\Projects UE5\LustAsDeadlySin\Plugins\<Plugin>\<Plugin>.uplugin`. None of these plugin directories exists in the target at the time of this audit.

| Plugin | Version | Modules, type, loading phase | Descriptor plugin edges | Source files | Public surface | Content/config | Classification | Phase status |
|---|---:|---|---|---:|---|---|---|---|
| EFCharacterCreation | 1.0.0 | `EFCharacterCreationRuntime` Runtime/Default; `EFCharacterCreationEditor` Editor/Default | none | 25 | 12 headers; 8 UCLASS, 9 USTRUCT, 1 UENUM | 2 WBP + `Config/DefaultGame.ini` | MIGRATE_SOURCE_WITH_CONFIG_MERGE | PHASE2_INVENTORIED / PENDING_PORT |
| EFCharacterCreationDazBridge | 1.0.0 | `EFCharacterCreationDazBridgeEditor` Editor/Default | EFCharacterCreation, DazToUnreal | 3 | 1 module header | none | REBUILD_AGAINST_TARGET | PHASE2_INVENTORIED / PENDING_PORT |
| EFClothingMorph | 1.0.0 | `EFClothingMorphRuntime` Runtime/Default | EFCharacterCreation | 4 | 1 header; 1 UCLASS, 8 USTRUCT, 2 UENUM | none | MIGRATE_SOURCE | PHASE2_INVENTORIED / PENDING_PORT |
| EFProcedural | 1.0.0 | `EFProceduralRuntime`, `EFProceduralACFURuntime`, `EFProceduralPCGRuntime` Runtime/Default; `EFProceduralEditor` Editor/Default | PCG | 26 | 13 headers; 4 UCLASS, 3 UINTERFACE | none | MIGRATE_SOURCE | PHASE2_INVENTORIED / PENDING_PORT |
| EFLevelFlow | 1.0.0 | `EFLevelFlowRuntime` Runtime/Default | AscentCombatFramework, EFProcedural, EFCharacterCreation | 7 | 3 headers; 2 UCLASS | none | REBUILD_AGAINST_TARGET | PHASE2_INVENTORIED / PENDING_PORT |
| EFCharacterCreationACFUBridge | 1.0.0 | `EFCharacterCreationACFURuntime` Runtime/Default | EFCharacterCreation, GameplayAbilities | 5 | 2 headers | none | REBUILD_AGAINST_TARGET | PHASE2_INVENTORIED / PENDING_PORT |
| EFProjectSystems | 0.1.0 | `EFProjectSystemsCore`, `EFProjectSystemsGameplay`, `EFProjectSystemsUI` Runtime/Default; `EFProjectSystemsEditor` Editor/Default | GameplayAbilities, EnhancedInput, AscentCombatFramework, ACFTrainingSystem, EFCharacterCreation, EFLevelFlow, EFProcedural, CodeWidgetDesignerBridge, DirtyPawnRuntime, SkinnedDecalComponent | 288 | 144 headers; 217 UCLASS, 91 USTRUCT, 31 UENUM, 4 UINTERFACE | none | MIGRATE_SOURCE_AFTER_DEPENDENCIES | PHASE2_INVENTORIED / PENDING_PORT |
| EFBlink | 0.1.0 | `EFBlinkRuntime` Runtime/Default | none | 11 | 5 headers; 3 UCLASS, 1 USTRUCT | none | MIGRATE_SOURCE | PHASE2_INVENTORIED / PENDING_PORT |
| DirtyPawnRuntime | 1.0.0 | `DirtyPawnRuntime` Runtime/Default; `DirtyPawnRuntimeEditor` Editor/Default | none | 12 | 5 headers; 16 UCLASS, 5 USTRUCT, 3 UENUM | none | MIGRATE_SOURCE | PHASE2_INVENTORIED / PENDING_PORT |
| ACFTrainingSystem | 0.1.0 | `ACFTrainingSystem` Runtime/Default | GameplayAbilities, AscentCombatFramework | 11 | 5 headers; 3 UCLASS, 5 USTRUCT, 1 UENUM | none | REBUILD_AGAINST_TARGET | PHASE2_INVENTORIED / PENDING_PORT |
| CodeWidgetDesignerBridge | 0.1.0 | `CodeWidgetDesignerBridge` Runtime/Default; `CodeWidgetDesignerBridgeEditor` Editor/Default | none | 22 | 2 public headers; 1 UCLASS, 5 USTRUCT, 2 UENUM, 1 UINTERFACE | none | REBUILD_AGAINST_UE58_EDITOR | PHASE3_EDITOR_GAME_BUILD_PASS / QA_PENDING |

`CanContainContent=true` is present on EFCharacterCreation, EFProjectSystems, DirtyPawnRuntime, and ACFTrainingSystem, but only EFCharacterCreation currently contains plugin assets. Project DataAssets, DataTables, Widget Blueprints, maps, input assets, and other `/Game` dependencies must therefore be handled by the content manifest; porting plugin source alone is not a subsystem-complete migration.

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

## Required port order

1. Port and compile independent leaves: CodeWidgetDesignerBridge, DirtyPawnRuntime, EFBlink.
2. Port EFCharacterCreation; migrate its two WBP assets through Unreal and merge, rather than replace, `Config/DefaultGame.ini`.
3. Port EFCharacterCreationDazBridge, EFClothingMorph, and EFCharacterCreationACFUBridge.
4. Port ACFTrainingSystem and validate its ARS/GAS contract against ACFU 4.3.5.
5. Port EFProcedural in module order: Runtime, ACFURuntime, PCGRuntime, Editor.
6. Port EFLevelFlow after EFCharacterCreation and EFProcedural are green.
7. Port EFProjectSystems last, in module order: Core, UI, Gameplay, Editor.

Each plugin/wave remains `PENDING_PORT` until its source-only staged import has its own UHT/UBT, load, Blueprint, PIE, cook, package, and protected-hash evidence. No `Binaries`, `Intermediate`, or `Saved` subtree is a port input.

## Confirmed ACFU 4.3.5 signature change

This is a confirmed compile blocker, not a speculative hotspot:

- Project call: `D:\Projects UE5\LustAsDeadlySin\Plugins\EFProjectSystems\Source\EFProjectSystemsGameplay\SinfulAscension\ProjectSinfulAscensionComponent.cpp:2590`
- ACFU 4.2.3 declaration: `D:\Unreal Engine 5\Library\UE_5.7\Engine\Plugins\Marketplace\AscentCoa789c5ab7b4cV4\Source\CollisionsManager\Public\ACMEffectsDispatcherComponent.h:20`
- ACFU 4.3.5 declaration: `D:\Unreal Engine 5\Library\UE_5.8\Engine\Plugins\Marketplace\ACFUAsce5ab7c1439afbV5\Source\CollisionsManager\Public\ACMEffectsDispatcherComponent.h:20`

`UACMEffectsDispatcherComponent::PlayReplicatedActionEffect` changed from two arguments to three and now requires `const FComponentFX& outComps`. The project-owned call must construct/pass an `FComponentFX`; ACFU remains immutable.

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
