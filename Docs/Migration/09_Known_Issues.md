# Known issues

## MIG-0001 — Target has no Git repository

- Severity: P3 process/traceability risk.
- Evidence: `git rev-parse --show-toplevel` failed at the target root.
- Mitigation completed: full external snapshot with zero SHA-256 mismatches.
- Remaining decision: choose a space-safe version-control/LFS strategy after source sizing and LFS audit.

## MIG-0002 — Forensic inventory incomplete

- Severity: gate condition.
- Status: `IN_PROGRESS`.
- Impact: no source plugin or content migration may begin until all project-owned roots are classified.

## MIG-0003 — Target Blueprint baseline is red

- Severity: P1 baseline defect.
- Evidence: `CompileAllBlueprints` exited 83 with 34 assets and 61 unique compiler messages.
- Dominant cause: template assets import `/Script/TP_ThirdPerson` while the live target module is `/Script/NoShellForWinter`.
- Other owners: project FullSample content and two ACFU plugin assets.
- Policy: repair project-owned redirects/assets; do not edit ACFU or other Marketplace plugin content.
- Log: `Saved/Migration/Logs/Phase1_CompileAllBlueprints_20260713.log`.

## MIG-0004 — Source checkout contains authoritative uncommitted work

- Severity: P0 data-preservation risk if ignored.
- Evidence: eight tracked modifications, including `Content/FullSample/Player.uasset` and live Intimacy/Emote code/tools.
- Mitigation: SHA-256 hashes and full LFS OID manifest captured before migration.
- Policy: source remains read-only; compare against the live working tree, not only HEAD.

## MIG-0005 — Additional project-owned plugins were omitted from the initial required list

- Severity: P1 omission risk.
- Found: `EFBlink`, `DirtyPawnRuntime`, `ACFTrainingSystem`, and `CodeWidgetDesignerBridge`.
- Action: include in dependency/usage audit and migrate or explicitly classify obsolete with evidence.

## MIG-0006 — Target Daz/Player baseline warnings

- Severity: P1/P2 pending runtime confirmation.
- Pre-existing evidence includes `Male` skeleton mismatch, `Multiple_PhysicsAsset` body-count ensure, invalid Daz texture folder package names, missing ACF/GAS imports, and lost parent-function metadata in `Player`.
- Action: preserve target assets, capture live ownership/manifests, and repair through target-owned composition/configuration only.
