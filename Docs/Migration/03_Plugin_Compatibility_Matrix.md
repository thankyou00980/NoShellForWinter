# Plugin compatibility matrix

Status: `IN_PROGRESS`

| Plugin | Descriptor | Location | Version | Runtime/Editor | Source | Target | Classification | Load | Build | Cook/package | Evidence |
|---|---|---|---|---|---|---|---|---|---|---|---|
| AscentCombatFramework | `AscentCombatFramework.uplugin` | Engine Marketplace | Source 4.2.3 / Target 4.3.5 | Runtime + Editor | Yes | Yes | KEEP_TARGET | PASS baseline | PASS | PENDING | descriptor + build logs |
| DazToUnreal | `DazToUnreal.uplugin` | Engine | Source 5.7.0.480 / Target 5.8.0.491 | Editor | Yes | Yes | KEEP_TARGET | PASS baseline | PASS | PENDING | descriptor + build logs |
| BpGeneratorUltimate | `BpGeneratorUltimate.uplugin` | Engine Marketplace | Source 1.5.5 / Target 1.7.0 | Editor tooling | Yes | Yes | KEEP_TARGET | PASS_WITH_WARNINGS | PASS_WITH_WARNINGS | PENDING | UBT/commandlet logs |
| PCGExtendedToolkit | project/Engine descriptors | Engine Marketplace | Source 0.75.8 / Target 0.76.2 | Runtime + Editor | Yes | Yes | KEEP_TARGET | PASS baseline | PASS | PENDING | descriptors + build logs |
| SkinnedDecalComponent | `SkinnedDecalComponent.uplugin` | Engine Marketplace | Source 3.0 / Target 3.0.1 | Runtime | Yes | Yes | KEEP_TARGET | PASS baseline | PASS | PENDING | descriptors + build logs |
| EFCharacterCreation | source project | Project-owned | PENDING port version | Runtime + Editor | Yes | No | MIGRATE_SOURCE | PENDING | PENDING | PENDING | source receipt |
| EFCharacterCreationDazBridge | source project | Project-owned | PENDING port version | Runtime | Yes | No | REBUILD_AGAINST_TARGET | PENDING | PENDING | PENDING | source receipt |
| EFClothingMorph | source project | Project-owned | PENDING port version | Runtime | Yes | No | MIGRATE_SOURCE | PENDING | PENDING | PENDING | source receipt |
| EFProcedural | source project | Project-owned | PENDING port version | Runtime + Editor | Yes | No | MIGRATE_SOURCE | PENDING | PENDING | PENDING | source receipt |
| EFLevelFlow | source project | Project-owned | PENDING port version | Runtime | Yes | No | MIGRATE_SOURCE | PENDING | PENDING | PENDING | source receipt |
| EFCharacterCreationACFUBridge | source project | Project-owned | PENDING port version | Runtime | Yes | No | REBUILD_AGAINST_TARGET | PENDING | PENDING | PENDING | source receipt |
| EFProjectSystems | source project | Project-owned | PENDING port version | Runtime + Editor | Yes | No | MIGRATE_SOURCE | PENDING | PENDING | PENDING | 288 source files |
| EFBlink | source project | Project-owned | PENDING | Runtime | Yes | No | MIGRATE_SOURCE | PENDING | PENDING | PENDING | source descriptor |
| DirtyPawnRuntime | source project | Project-owned | PENDING | Runtime | Yes | No | MIGRATE_SOURCE | PENDING | PENDING | PENDING | source descriptor |
| ACFTrainingSystem | source project | Project-owned | PENDING | Runtime | Yes | No | MIGRATE_SOURCE | PENDING | PENDING | PENDING | source descriptor |
| CodeWidgetDesignerBridge | source project | Project-owned | PENDING | Runtime/Editor PENDING | Yes | No | AUDIT_USAGE | PENDING | PENDING | PENDING | source descriptor |

Unverified plugins remain `PENDING`; descriptor names will not be inferred from friendly names.
