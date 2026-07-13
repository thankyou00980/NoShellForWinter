# NoShellForWinter baseline

Status: `IN_PROGRESS`

## Recovery baseline

| Check | Result | Evidence |
|---|---|---|
| Target is a Git worktree | FAIL_PREEXISTING | `git rev-parse` reported no repository |
| External restorable snapshot | PASS | `D:\Projects UE5\NoShellForWinter_Baseline_PreMigration_20260713` |
| Snapshot SHA-256 verification | PASS | 5,916/5,916 files matched |
| No Unreal processes during capture | PASS | Process query returned no Unreal Editor or ShaderCompileWorker process |
| Target EngineAssociation | PASS | `NoShellForWinter.uproject` reports `5.8` |
| UE installations | PASS | UE 5.8.0 CL 55116800 and UE 5.7.4 CL 51494982 present |
| Generate project files | PASS | `Saved/Migration/Logs/Phase1_GenerateProjectFiles_Retry_20260713.log` |
| Development Editor Win64 | PASS | `Saved/Migration/Logs/Phase1_DevelopmentEditor_Build_20260713.log` |
| Development Game Win64 | PASS | `Saved/Migration/Logs/Phase1_DevelopmentGame_Build_20260713.log` |
| Compile All Blueprints | FAIL_PREEXISTING | Exit 83; 34 assets / 61 unique compiler messages in `Phase1_CompileAllBlueprints_20260713.log` |
| Source unchanged after forensic audit | PASS | `Docs/Migration/Evidence/Phase0_Source_ReadOnly_Verification.json` |
| ACFU/Daz immutable manifests | PASS | `Docs/Migration/Evidence/Phase0_Target_Invariant_Hashes.json` |

## Pending baseline gates

- Immutable hash manifests for target ACFU and DazToUnreal.
- Player/Female/Frederick/Multiple/Male asset manifest.
- Editor startup and pre-existing warning/error inventory.
- Basic PIE, locomotion, combat, and target baseline.
- Baseline screenshots.
- Minimal cook.

No migration subsystem is considered started while these checks remain pending.

## Verified target plugin versions

- ACFU descriptor: `D:\Unreal Engine 5\Library\UE_5.8\Engine\Plugins\Marketplace\ACFUAsce5ab7c1439afbV5\AscentCombatFramework.uplugin`; version `4.3.5`, Engine `5.8.0`.
- DazToUnreal descriptor: `D:\Unreal Engine 5\Library\UE_5.8\Engine\Plugins\DazToUnreal\DazToUnreal.uplugin`; version `5.8.0.491`.
- No project-local EF plugin exists yet.

## Pre-existing Blueprint failures

The global commandlet compiled C++ successfully but returned 83. It found 34 assets with compiler errors. Most template assets still import `/Script/TP_ThirdPerson` although the target module is `/Script/NoShellForWinter`; this is a target baseline migration defect, not an EF port regression. Additional failures include ACFU plugin content, `ACFMageEnemyBP`, and `SampleDialogueButton_WBP`. The complete log remains authoritative.
