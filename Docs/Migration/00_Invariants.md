# Migration invariants

## Project identity

- Source: `D:\Projects UE5\LustAsDeadlySin` (strictly read-only).
- Target: `D:\Projects UE5\NoShellForWinter` (writable).
- Target engine association observed in `NoShellForWinter.uproject`: `5.8`.
- Source engine association observed in `ACFSample.uproject`: `5.7`.

## Authority order

1. Unreal Engine 5.8 APIs.
2. Target ACFU 4.3.5 and its current configuration.
3. Target DazToUnreal and current Daz assets.
4. Target Player, Frederick, Multiple, Female, and Male assets.
5. Source project-owned behavior and content.
6. Updated SXP/Willpower specification.
7. Project-owned reconstruction when a clean port is impossible.

## Prohibited operations

- No source writes or UE 5.8 resaves.
- No bulk replacement of `Content`, `Config`, `Plugins`, `/Game/FullSample`, or `Player`.
- No replacement or direct edits of ACFU, DazToUnreal, Marketplace plugins, or Engine plugins.
- No success claim based on compilation alone.

## Verified recovery point

- Snapshot: `D:\Projects UE5\NoShellForWinter_Baseline_PreMigration_20260713`.
- Included: full non-generated target state, including `Content`, `Config`, `Source`, and project descriptors.
- Excluded as regenerable: `Binaries`, `Intermediate`, `Saved`, `DerivedDataCache`, and `.vs`.
- Files verified by SHA-256: `5916`.
- Verified bytes: `5.781 GB`.
- Hash mismatches: `0`.
- Manifest: `D:\Projects UE5\NoShellForWinter_Baseline_PreMigration_20260713\baseline_sha256_manifest.csv`.

## Verified source fingerprint

- Source Git HEAD: `8808ee6493b6c8e73b58fca30d9d5f1c1bca77b4` on `main`, upstream `origin/main`, ahead 3 / behind 0.
- The source working tree is intentionally dirty in eight tracked files, including `Content/FullSample/Player.uasset`; those live files are authoritative for behavior and must not be replaced by HEAD versions.
- LFS entries: `10,952`, all hydrated.
- LFS manifest SHA-256: `50FF7F3DF5DD50A8E687BD312BDE1172BCBAF89D6F0588697E5B0A13B1757015`.
- Source Git/LFS report: `Saved/Migration/Reports/Phase0_Source_Git_State_Before.json`.
- Source LFS manifest: `Saved/Migration/Reports/Phase0_Source_LFS_Manifest_Before.txt`.
- Post-audit verification: `PASS`; HEAD, porcelain status, eight live-file hashes, LFS entry count, and LFS manifest hash all remained identical.
- Verification evidence: `Docs/Migration/Evidence/Phase0_Source_ReadOnly_Verification.json`.

## Protected target manifests

- ACFU 4.3.5: 5,043 files / 6,170,659,421 bytes; manifest SHA-256 `69F46CACC120E44AC3B1729342E059CCD24D77D01534CB4E0F36C4A8A26D87F9`.
- DazToUnreal 5.8.0.491: 213 files / 172,602,860 bytes; manifest SHA-256 `523200EBEED3B1284445C0570029CE746C10D4E5039B41AAD769544166E2B491`.
- Target Daz assets: 189 files / 980,685,486 bytes; manifest SHA-256 `A0F3176C7AFF078DC3D8669D93D1A65ACA6D520AF6938F593500967835414FAA`.
- Player/Female/Multiple/Male individual SHA-256 hashes are recorded in `Docs/Migration/Evidence/Phase0_Target_Invariant_Hashes.json`.

## Phase-gate rule

Advance automatically only after the current phase has evidence for its gate. A failed gate is corrected and repeated. An external blocker is documented while independent work continues.
