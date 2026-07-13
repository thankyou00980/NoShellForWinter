# QA matrix

Status: `PHASE_4_CORE_MODERN_UI_PROCEDURAL_MAP_AND_DIRTYPAWN_IN_PROGRESS`

Allowed terminal results: `PASS`, `PASS_EXPECTED_DELTA`, `FAIL`, `BLOCKED_EXTERNAL`.

| Area | Build/test | PIE/runtime | Visual | Cook/package | Result | Evidence | Commit |
|---|---|---|---|---|---|---|---|
| Baseline target-owned | PASS | PASS | PASS | PASS minimal cook | PASS | `Phase1_Baseline_PIE.json`, compile/cook logs | PENDING_PHASE1_COMMIT |
| ACFU vendor Blueprint | BLOCKED_EXTERNAL | PASS probes | N/A | PASS minimal cook | BLOCKED_EXTERNAL | `ACF_PickAction_BP`; immutable plugin hash PASS | PENDING_PHASE1_COMMIT |
| MCP transport | PASS | PASS | N/A | N/A | PASS | baseline MCP controller log | PENDING_PHASE1_COMMIT |
| UBG licensed tools | BLOCKED_EXTERNAL | N/A | N/A | N/A | BLOCKED_EXTERNAL | plugin requests license activation | PENDING_PHASE1_COMMIT |
| Project-owned plugins/modules | PASS: all required and auxiliary C++ modules compile in applicable UE 5.8 Editor/Game targets | PENDING per subsystem | N/A | PENDING | PENDING | `03_Plugin_Compatibility_Matrix.md`; per-plugin Editor/Game build evidence | `fd46235`-`9ffccde` |
| EFProjectSystems core config/content | PASS: UE 5.8 Editor/Game builds; 9/9 clean automation | PENDING | PENDING | PENDING | PENDING | `Phase4_EFProjectCoreContent_ConfigAutomation.json`; exact 31-package batch, generated preview Texture2D, raw preview, 9/11 soft-package probe, source/protected gates PASS | `671feda` |
| Modern UI exact batch | PASS: 127/127 UE 5.7 AssetTools; 66/66 WBP compile; UE 5.8 load/compile/resave; Editor/Game builds; 80/80 automation | PENDING | PENDING | PENDING cook-manifest/package | PENDING | `Phase4_ModernUI_ConfigBuild.json`; 0 external `/Game` deps, 0 redirectors, source/protected gates PASS | `4a66c74` |
| EFProcedural exact contracts batch | PASS: 20/20 UE 5.7 AssetTools; UE 5.8 load/compile/resave; 6/6 Blueprint compile; Editor/Game builds; source/protected gates PASS | PENDING StartPoint discovery/spawn and dungeon runtime | PENDING | PENDING cook-manifest/package | PENDING | `Phase4_ProceduralContracts_ContentBuild.json`; 19 Calysto contracts + canonical `BP_StartPoint`; 0 external `/Game` deps, 0 redirectors | `259bf42` |
| DungeonGeneration exact map batch | PASS: 1/1 UE 5.7 AssetTools; UE 5.7 read-only map load; UE 5.8 load/save/reload; Editor/Game builds; source/protected gates PASS | PENDING map PIE, PCG/navigation, StartPoint, and level flow | PENDING | PENDING cook-manifest/package | PENDING | `Phase4_DungeonGeneration_ContentBuild.json`; exact `World`, 0 `/Game` deps, 0 sidecars/external packages | `95fcd1b` |
| DirtyPawn exact material closure | PASS: 15/15 UE 5.7 AssetTools; UE 5.8 load/material compile/resave/reload; zero material/shader errors; Editor/Game builds; source/protected gates PASS | PASS focused binding contract: 6/6 `SucceededWithWarnings`, six `Result={Success}`, six `Ready ... bindings=6`, wrapper missing 0, lifecycle errors 0 | PENDING wet/mud/blood/smear/snow/sand, tattoo compatibility, and blood alpha | PENDING cook-manifest/package/packaged runtime | PENDING | `Phase4_DirtyPawnAssets_ContentRuntime.json`; runtime scope is wrapper resolution/binding only; morph-physics warnings remain Phase 7 | `ec10c8e` |
| Player invariants | PASS hash | PASS Female ownership | PASS baseline | PASS minimal cook | PASS | PIE and protected-invariant evidence | PENDING_PHASE1_COMMIT |
| Character creation | PENDING | PENDING | PENDING | PENDING | PENDING | PENDING | PENDING |
| Morphs/blink/clothing | PENDING | PENDING | PENDING | PENDING | PENDING | PENDING | PENDING |
| SXP/Willpower | PENDING | PENDING | PENDING | PENDING | PENDING | PENDING | PENDING |
| Needs/Status/consumables | PENDING | PENDING | PENDING | PENDING | PENDING | PENDING | PENDING |
| Chronicle | PENDING | PENDING | PENDING | PENDING | PENDING | PENDING | PENDING |
| Input/locomotion/actions | PENDING | PENDING | PENDING | PENDING | PENDING | PENDING | PENDING |
| Combat/enemies/defeat | PENDING | PENDING | PENDING | PENDING | PENDING | PENDING | PENDING |
| Procedural/full flow | PENDING | PENDING | PENDING | PENDING | PENDING | PENDING | PENDING |
| Save migration | PENDING | PENDING | N/A | PENDING | PENDING | PENDING | PENDING |
