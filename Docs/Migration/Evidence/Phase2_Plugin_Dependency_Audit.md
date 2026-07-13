# Phase 2 project-owned plugin dependency audit

Status: `PHASE2_INVENTORIED / PENDING_PORT`

Captured: 2026-07-13, America/Bogota.

## Scope and provenance

- Source inspected read-only: `D:\Projects UE5\LustAsDeadlySin`.
- Source Git HEAD at capture: `8808ee6493b6c8e73b58fca30d9d5f1c1bca77b4`.
- Target receiving documentation: `D:\Projects UE5\NoShellForWinter`.
- Target Git HEAD before these documentation edits: `5ca531956aa230658e97e671311f9b2429fae22e`.
- Method: direct read of the 11 `.uplugin` descriptors, every module `Build.cs`, source/header trees, plugin `Content`/`Config` roots, and the UE 5.7/UE 5.8 ACF public headers referenced by project code.
- Mutation status: no source or plugin file was written; no plugin was copied to the target; no Unreal asset was modified or resaved.
- Validation status: static inventory only. Build/load/PIE/cook/package claims for these 11 plugins remain `PENDING_PORT`.

## Descriptor receipts

| Plugin | Descriptor SHA-256 |
|---|---|
| EFCharacterCreation | `ACB295524A7AE5535231F58102F115115656B0E1269ACD5F51AB0B8D5278FC2F` |
| EFCharacterCreationDazBridge | `D4103A96BF4FF119F56A5232F3DAABBF8692EC67A0B89FBF3670A988BE989002` |
| EFClothingMorph | `953DED3D1089E96C6FF9B8F78031E003890405A488E81A0593E80DD3C41C0602` |
| EFProcedural | `9B533B8BD2C5B161740E633DB2EB7EB726257813866410C3D77CD08B83F341D7` |
| EFLevelFlow | `EA946BCC94587A7DEA16AD2DF19D2A868DF5A8333498DEC87A9DFAC8FF7AD89F` |
| EFCharacterCreationACFUBridge | `C8B1D5C397F04815A26725431B4AA88BFA042C5F57BA38D3CA0D8C56F26EDD26` |
| EFProjectSystems | `5FD4BBC14C9EB52D716FBC60B7F6DBD1F1BDC64C21E891D85A7240A3F4F27234` |
| EFBlink | `A5B4AC3FE902FF12791DA60766C2D5F83A5A3E40852E5E2B0B507C29554DF4D2` |
| DirtyPawnRuntime | `8CA7D4AEC8CFAE851EA599E209B5FCB45B3930AF6D9E0BE31F4C8613EF3F464B` |
| ACFTrainingSystem | `868C9064E416D45E5EB8872CC1D0E38F68D9775154ACF84D282AE4D7C43F3D23` |
| CodeWidgetDesignerBridge | `23C65F1EDD60890A149AA5587FCF17CD66C31DA0DFB0B3EDB652D8775BCB336F` |

Each receipt hashes `D:\Projects UE5\LustAsDeadlySin\Plugins\<Plugin>\<Plugin>.uplugin` from the live working tree.

## Exact module and dependency evidence

### EFCharacterCreation

- `Source/EFCharacterCreationRuntime/EFCharacterCreationRuntime.Build.cs`: public `Core`, `CoreUObject`, `Engine`, `AssetRegistry`, `DeveloperSettings`, `InputCore`, `Slate`, `SlateCore`, `UMG`.
- `Source/EFCharacterCreationEditor/EFCharacterCreationEditor.Build.cs`: public `Core`, `CoreUObject`, `Engine`, `EFCharacterCreationRuntime`.
- Plugin assets: `Content/UI/WBP_EFCharacterCreationRoot.uasset`, `Content/UI/WBP_EFMorphSlider.uasset`.
- Config: `Config/DefaultGame.ini`; contains `bAutoEnterTestingMap=True`, `TestingMapName=TestingMap`, and old `/Game/DazToUnreal/AmalaforGenesis9/...` body mesh configuration.
- Public owners: `AEFCharacterCreationBootstrapActor`, `UEFCharacterCreationSettings`, `UEFCharacterCreationSubsystem`, `UEFCharacterCustomizationComponent`, `UEFCharacterCustomizationSaveGame`, `UEFMorphPhysicsConstraintComponent`, `UEFCharacterCreationRootWidget`, `UEFMorphSliderWidget`.

### EFCharacterCreationDazBridge

- `Source/EFCharacterCreationDazBridgeEditor/EFCharacterCreationDazBridgeEditor.Build.cs`: public `EFCharacterCreationRuntime`, `Core`, `CoreUObject`, `DazToUnreal`, `Engine`, `Projects`; private `AnimGraph`, `AssetRegistry`, `BlueprintGraph`, `ControlRig`, `ControlRigDeveloper`, `Json`, `UnrealEd`.
- High-risk implementation: `Source/EFCharacterCreationDazBridgeEditor/Private/EFCharacterCreationDazBridgeEditor.cpp`; creates/compiles AnimBlueprint graphs, assigns post-process AnimBlueprints, and can execute `CreateAutoJCMControlRig.py`.

### EFClothingMorph

- `Source/EFClothingMorphRuntime/EFClothingMorphRuntime.Build.cs`: public `Core`, `CoreUObject`, `Engine`, `GameplayTags`, `EFCharacterCreationRuntime`.
- Contract owner: `Source/EFClothingMorphRuntime/Public/EFClothingMorphComponent.h`, `UEFClothingMorphComponent`.
- Reflection implementation: `Source/EFClothingMorphRuntime/Private/EFClothingMorphComponent.cpp`; expects `/Script/InventorySystem.ACFEquipmentComponent`, `/Script/InventorySystem.ACFArmorSlotComponent`, `OnEquippedArmorChanged`, `GetModularMeshes`, `ArmorSlot`, and `skinnedArmor`.

### EFProcedural

- `EFProceduralRuntime.Build.cs`: public `Core`, `CoreUObject`, `Engine`, `DeveloperSettings`.
- `EFProceduralACFURuntime.Build.cs`: public basics + `EFProceduralRuntime`; its implementation uses class/path hints and no direct ACF header.
- `EFProceduralPCGRuntime.Build.cs`: public basics + `EFProceduralRuntime`; private `AIModule`, `GameplayTasks`, `NavigationSystem`, `PCG`, `EFProceduralACFURuntime`.
- `EFProceduralEditor.Build.cs`: public basics + `EFProceduralRuntime`; private `UnrealEd`.
- `Source/EFProceduralPCGRuntime/Private/EFProceduralPCGSubsystem.cpp` calls UE 5.8-present `UPCGComponent::GenerateLocal`, `NotifyPropertiesChangedFromBlueprint`, and the generated/cancelled/cleaned delegates.

### EFLevelFlow

- `Source/EFLevelFlowRuntime/EFLevelFlowRuntime.Build.cs`: public `Core`, `CoreUObject`, `Engine`, `DeveloperSettings`, `UMG`; private `AIModule`, `AIFramework`, `Slate`, `SlateCore`, `EFProceduralRuntime`, `EFCharacterCreationRuntime`.
- `Source/EFLevelFlowRuntime/Private/EFLevelFlowSubsystem.cpp` uses `AACFAIController::GetThreatManager/GetTarget/SetTarget/ResetToDefaultState` and `UACFThreatManagerComponent::GetActorWithHigherThreat/IsThreatening/RemoveThreatening`; all declarations exist in target ACFU 4.3.5.

### EFCharacterCreationACFUBridge

- `Source/EFCharacterCreationACFURuntime/EFCharacterCreationACFURuntime.Build.cs`: public `EFCharacterCreationRuntime`, `Core`, `CoreUObject`, `Engine`, `GameplayAbilities`.
- `Source/EFCharacterCreationACFURuntime/Private/EFCharacterCreationACFUBridge.cpp` reflects `SetCanMove` on an ACF movement component and calls GAS `CancelAllAbilities`.

### EFProjectSystems

- `EFProjectSystemsCore.Build.cs`: `Core`, `CoreUObject`, `Engine`, `DeveloperSettings`, `InputCore`.
- `EFProjectSystemsUI.Build.cs`: `Core`, `CoreUObject`, `Engine`, `UMG`, `DeveloperSettings`, `EFProjectSystemsCore`.
- `EFProjectSystemsEditor.Build.cs`: public `Core`, `CoreUObject`, `Engine`, `GameplayTags`, `AscentCombatFramework`, `EFLevelFlowRuntime`, `EFProjectSystemsCore`, `EFProjectSystemsGameplay`, `EFProceduralEditor`; private `UnrealEd`, `Json`, `JsonUtilities`, `BlueprintGraph`, `AnimGraph`, `AnimationBlueprintLibrary`.
- `EFProjectSystemsGameplay.Build.cs`: public `Core`, `CoreUObject`, `Engine`, `ImageWrapper`, `InputCore`, `UMG`, `Slate`, `SlateCore`, `CodeWidgetDesignerBridge`, `GameplayAbilities`, `GameplayTags`, `GameplayTasks`, `DeveloperSettings`, `AssetRegistry`, `DirtyPawnRuntime`, `AIModule`, `NavigationSystem`, `Niagara`, `RenderCore`, `RHI`, `SkinnedDecalComponent`, `EnhancedInput`, `PhysicsCore`, `EFProjectSystemsCore`, `EFCharacterCreationRuntime`, `EFLevelFlowRuntime`, `EFProceduralRuntime`, `AdvancedRPGSystem`, `ACFTrainingSystem`, `AscentCoreInterfaces`, `AscentCombatFramework`, `AscentTeams`, `AscentGASRuntime`, `AscentTargetingSystem`, `CinematicCameraManager`, `AIFramework`, `InventorySystem`, `StatusEffectSystem`, `AscentDialogueSystem`, `ActionsSystem`, `CharacterController`, `CollisionsManager`, `AscentSaveSystem`; private `ApplicationCore`, `Json`, plus editor-only `UnrealEd`, `BlueprintGraph`.
- Layout risk: all 144 headers sit directly under module directories/subdirectories rather than conventional `Public/Private`, and `PublicIncludePaths` exposes module roots. `EFProjectSystemsEditor` also adds the Core and Gameplay module directories directly.
- Public feature roots: `Camera`, `CharacterBackground`, `Characters`, `Combat`, `Debug`, `Defeat`, `Dialogue`, `DirtyPawn`, `DungeonCurse`, `Effects`, `Intimacy`, `Lockpicking`, `Locomotion`, `RuntimePerformance`, `SinfulAscension`, `Survival`, `TattooShop`, `Training`, `UI`.

### EFBlink

- `Source/EFBlinkRuntime/EFBlinkRuntime.Build.cs`: `Core`, `CoreUObject`, `Engine`, `DeveloperSettings`.
- Owners: `UEFBlinkMorphComponent`, `UEFBlinkPlayerSubsystem`, `UEFBlinkSettings`, `FEFBlinkMorphTarget`.

### DirtyPawnRuntime

- `DirtyPawnRuntime.Build.cs`: `Core`, `CoreUObject`, `Engine`, `InputCore`, `bUseUnity=false`.
- `DirtyPawnRuntimeEditor.Build.cs`: public `Core`, `CoreUObject`, `Engine`, `UnrealEd`; private `DirtyPawnRuntime`, `Kismet`, `KismetCompiler`, `bUseUnity=false`.
- Owners: `UDirtyPawnComponent`, `UDirtyPawnWorldSubsystem`, `ADirtyPawnVolumeBase` and its water/mud/bleach/fade/blood/smear/dirt/burn/sand/snow/hurt/interior variants.

### ACFTrainingSystem

- `Source/ACFTrainingSystem/ACFTrainingSystem.Build.cs`: public `Core`, `CoreUObject`, `Engine`, `GameplayAbilities`, `GameplayTags`, `GameplayTasks`, `DeveloperSettings`, `UMG`, `AdvancedRPGSystem`, `AscentGASRuntime`; private `Slate`, `SlateCore`, `bUseUnity=false`.
- Owners: `UACFTrainingComponent`, `UACFTrainingMinigameBase`, `UACFTrainingSettings`, `FACFTrainingDefinition` and progress/reward/requirement types.
- `Source/ACFTrainingSystem/Private/ACFTrainingComponent.cpp` uses ARS methods that remain declared in ACFU 4.3.5: `GetCurrentValueForStatitstic`, `GetCurrentAttributeValue`, `AddAttributeSetModifier`, `RemoveAttributeSetModifier`, `OnAttributeSetModified`.

### CodeWidgetDesignerBridge

- Runtime `Build.cs`: `Core`, `CoreUObject`, `Engine`, `UMG`.
- Editor `Build.cs`: public basics + `UMG`, `CodeWidgetDesignerBridge`; private `ApplicationCore`, `AssetRegistry`, `AssetTools`, `BlueprintGraph`, `Kismet`, `KismetCompiler`, `Slate`, `SlateCore`, `UMGEditor`, `UnrealEd`.
- Public contract: `Source/CodeWidgetDesignerBridge/Public/CodeWidgetDesignerTreeProvider.h`, `ICodeWidgetDesignerTreeProvider` and `FCodeWidgetDesigner*` specs/reports.
- Editor API: `Source/CodeWidgetDesignerBridgeEditor/Public/CodeWidgetToWBPBridgeLibrary.h`, `UCodeWidgetToWBPBridgeLibrary`; commandlets remain private and unverified on UE 5.8.

## Confirmed ACFU delta

Project-owned call site:

```text
D:\Projects UE5\LustAsDeadlySin\Plugins\EFProjectSystems\Source\EFProjectSystemsGameplay\SinfulAscension\ProjectSinfulAscensionComponent.cpp:2590
EffectsDispatcher->PlayReplicatedActionEffect(SoundEffect, OwnerCharacter);
```

Vendor declarations:

```text
ACFU 4.2.3 ACMEffectsDispatcherComponent.h:20
void PlayReplicatedActionEffect(const FActionEffect& effect, ACharacter* instigator);

ACFU 4.3.5 ACMEffectsDispatcherComponent.h:20
void PlayReplicatedActionEffect(const FActionEffect& effect, ACharacter* instigator, const FComponentFX& outComps);
```

Required future project-owned action: default-construct/prepare `FComponentFX` and pass it as the third argument. Status remains `PENDING_PORT`; this evidence does not authorize modifying ACFU.

## Port gate

Recommended acyclic order:

```text
CodeWidgetDesignerBridge + DirtyPawnRuntime + EFBlink
-> EFCharacterCreation
-> EFCharacterCreationDazBridge + EFClothingMorph + EFCharacterCreationACFUBridge
-> ACFTrainingSystem
-> EFProcedural
-> EFLevelFlow
-> EFProjectSystems
```

For each node: staged source-only import, descriptor enablement, UHT, cold Development Editor build, Game build, plugin/module load, Blueprint compile, focused PIE, visual QA where applicable, cook, package, and protected invariant re-hash. Until those artifacts exist, the only valid state is `PHASE2_INVENTORIED / PENDING_PORT`.
