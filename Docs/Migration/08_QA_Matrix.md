# QA matrix

Status: `PHASE_1_COMPLETE`

Allowed terminal results: `PASS`, `PASS_EXPECTED_DELTA`, `FAIL`, `BLOCKED_EXTERNAL`.

| Area | Build/test | PIE/runtime | Visual | Cook/package | Result | Evidence | Commit |
|---|---|---|---|---|---|---|---|
| Baseline target-owned | PASS | PASS | PASS | PASS minimal cook | PASS | `Phase1_Baseline_PIE.json`, compile/cook logs | PENDING_PHASE1_COMMIT |
| ACFU vendor Blueprint | BLOCKED_EXTERNAL | PASS probes | N/A | PASS minimal cook | BLOCKED_EXTERNAL | `ACF_PickAction_BP`; immutable plugin hash PASS | PENDING_PHASE1_COMMIT |
| MCP transport | PASS | PASS | N/A | N/A | PASS | baseline MCP controller log | PENDING_PHASE1_COMMIT |
| UBG licensed tools | BLOCKED_EXTERNAL | N/A | N/A | N/A | BLOCKED_EXTERNAL | plugin requests license activation | PENDING_PHASE1_COMMIT |
| Plugins | PENDING | PENDING | N/A | PENDING | PENDING | PENDING | PENDING |
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
