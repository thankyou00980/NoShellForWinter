# Known issues

## MIG-0001 — No remote Git rollback

- Severity: P3 process/traceability risk.
- Status: MITIGATED.
- Completed: external snapshot, local Git LFS baseline commit `d3fab0b`, baseline tag, migration branch.
- Remaining limitation: no Git remote is configured, so rollback is local and same-volume.

## MIG-0002 — Forensic inventory incomplete

- Severity: gate condition.
- Status: IN_PROGRESS.
- Completed: package union and source/target counts, source code symbol inventory, plugin/module/map inventory, and exact Phase 4 classification/evidence for the 31 migrated core-content packages, one target-generated texture, the 127-package Modern UI batch, and the 20-package EFProcedural contracts batch.
- Remaining: asset class/dependency enrichment and final action classification for every other manifest row.

## MIG-0003 — Target-owned Blueprint baseline was red

- Severity: P1 baseline defect.
- Status: RESOLVED.
- Initial evidence: exit 83, 34 assets, 61 unique messages, 269 `/Script/TP_ThirdPerson` occurrences.
- Repair: narrow Core Redirects plus Editor AssetTools retirement of a zero-referencer obsolete dialogue WBP.
- Final evidence: zero target-owned Blueprint failures and zero `TP_ThirdPerson` occurrences.

## MIG-0004 — Source checkout contains authoritative uncommitted work

- Severity: P0 data-preservation risk if ignored.
- Status: PROTECTED.
- Evidence: eight tracked modifications, including source `Player.uasset` and live Intimacy/Emote work.
- Gate: HEAD, status, all eight SHA-256 values, and the 10,952-entry LFS manifest still match after phase 1.

## MIG-0005 — Additional project-owned plugins were omitted initially

- Severity: P1 omission risk.
- Status: RESOLVED_PHASE_3.
- Found: `EFBlink`, `DirtyPawnRuntime`, `ACFTrainingSystem`, and `CodeWidgetDesignerBridge` in addition to the seven required plugins.
- Resolution: all four were included in the dependency/usage audit, migrated as project-owned plugins, explicitly enabled, and covered by the Phase 3 descriptor/build gates.

## MIG-0006 — Target Daz/Player baseline warnings

- Severity: P1/P2 pending subsystem validation.
- Status: OPEN.
- Pre-existing warnings include Male skeleton mismatch, `Multiple_PhysicsAsset` body-count ensure, invalid Daz texture-folder package names, and lost parent-function metadata in Player.
- Current evidence: authoritative assets are byte-identical; visible Female ownership and baseline PIE pass.
- Action: preserve target assets and repair only through target-owned composition/configuration after morph and animation audits.

## MIG-0007 — ACFU 4.3.5 ships one stale Blueprint

- Severity: `BLOCKED_EXTERNAL` vendor defect.
- Status: OPEN_EXTERNAL.
- Asset: `/AscentCombatFramework/Blueprints/Abilities/ACF_PickAction_BP`.
- Error: stale `Get Inventory Component` return pin and removed `GetInventoryComponent` function.
- Scope: one immutable Marketplace asset, two unique compiler messages.
- Regression checks: ACFU 5,043-file hash manifest PASS; asset/GAS runtime probes PASS; visible PIE PASS; minimal cook PASS.
- Policy: do not edit ACFU. Recheck after a vendor update; use a project-owned adapter only if gameplay QA proves the action is required and broken.

## MIG-0008 — UBG MCP actions require license activation

- Severity: P3 tooling limitation.
- Status: `BLOCKED_EXTERNAL`.
- MCP transport/authentication: PASS.
- UECP action response: license verification required in plugin settings.
- Mitigation: built-in Unreal Python and project-owned editor tooling; zero runtime dependency on UBG.

## MIG-0009 — Baseline quest smoke error

- Severity: P2 pending ownership classification.
- Status: OPEN.
- PIE log contains one `LogTemp: Error: Can't Start the quest` line on the ACFU Test map.
- No fatal, ensure, crash, or PIE lifecycle failure occurred.
- Action: classify against the migrated quest/story flow before final closeout; do not treat this sample-map condition as migrated quest behavior.

## MIG-0010 — EFProjectSystems core content is not yet a runtime/package PASS

- Severity: migration gate condition.
- Status: IN_PROGRESS.
- Completed: exact migration and UE 5.8 validation of 31 packages, generation of the packaged character-background preview texture, preservation of the raw preview, effective core settings validation, 9/9 clean focused automation, Editor/Game builds, source read-only verification, and protected invariant re-hash.
- Current resolution: the structural soft-package probe resolves 9/11 contracts; only `/Game/Procedural/DoorToLevel` and `/Game/Procedural/Maps/DungeonGeneration` remain absent.
- Remaining: migrate or replace those procedural contracts through an approved exact batch/adapter, then pass PIE, complete input behavior, visual QA, cook, package, and packaged-runtime validation. The current Phase 4 evidence must not be promoted to a full subsystem or migration PASS before those gates complete.

## MIG-0011 — Modern UI content is not yet a runtime/package PASS

- Severity: migration gate condition.
- Status: IN_PROGRESS.
- Completed: exact migration of 127 packages (12,370,672 bytes) under the five approved `/Game/_Game/Widgets` roots: Chronicle 23, InnerState 24, Status 17, Attributes 27, and SinfulAscensionAltar 36.
- Static validation: 66 Widget Blueprints, 54 Texture2D assets, and 7 FontFace assets; UE 5.7 loaded/compiled all 66 Widget Blueprints and resolved 52 native parents; UE 5.7 AssetTools and UE 5.8 load/compile/resave passed with zero external `/Game` dependencies and zero redirectors.
- Config/build gates: the `/Game/_Game/Widgets` always-cook root is applied; UE 5.8 Editor/Game builds, 71/71 native plus 9/9 content-focused automation tests, source read-only verification, and protected invariant re-hash passed.
- Commit: `4a66c74`.
- Remaining: exercise the widgets in PIE, complete visual QA, verify the root and assets in the cook manifest, then pass package and packaged-runtime validation. Static migration evidence alone is not a full Modern UI PASS.

## MIG-0012 — EFProcedural contracts are statically migrated, not runtime-complete

- Severity: migration gate condition.
- Status: IN_PROGRESS.
- Completed: exact migration of 20 packages—19 Calysto data contracts plus canonical `/Game/Calysto/Dungeon/Blueprint/Utility/BP_StartPoint`—through UE 5.7 AssetTools, followed by UE 5.8 load/compile/resave. All 6 Blueprints compile; Editor/Game builds, source read-only verification, and protected invariant re-hash pass. Evidence: `Phase4_ProceduralContracts_ContentBuild.json`.
- Commit: `259bf42`.
- Process note: the first post-migration binary-hash gate stopped after the log had recorded 20/20 successful AssetTools migrations because AssetTools reserialized `BP_StartPoint`; byte identity with the staged package was therefore not a valid success condition. The resume gate verified the exact already-created 20-package delta and completed successfully. No Blueprint compile, load, dependency, or functional defect was inferred from this serialization delta.
- Configuration guard: do not point `EFProceduralSettings` at `/Game/Calysto/Dungeon/Blueprint/BP_MassiveDungeon` yet.
- Remaining: migrate or replace `BP_MassiveDungeon`, `/Game/Procedural/Maps/DungeonGeneration`, and `/Game/Procedural/DoorToLevel`; then pass StartPoint discovery/spawn in PIE, full dungeon generation/cleanup, visual QA, cook/cooked-manifest, package, and packaged-runtime validation.
