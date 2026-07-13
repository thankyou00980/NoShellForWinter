# Phase 2 configuration three-way audit evidence

Status: `PHASE2_INVENTORIED/PENDING_PLUGIN_AND_ASSET_GATES`

Generated UTC: `2026-07-13T09:54:19.0632616Z`

## Scope and safety

This was a read-only comparison of source UE 5.7, target UE 5.8, installed ACFU 4.3.5, and project/plugin config files. The audit did not edit `Config`, `.uproject`, the source project, marketplace plugins, or Unreal assets. Documentation under the writable target is the only output.

Source Git HEAD during inspection: `8808ee6493b6c8e73b58fca30d9d5f1c1bca77b4`.

## Inspected authorities

- `D:\Projects UE5\LustAsDeadlySin\Config`
- `D:\Projects UE5\LustAsDeadlySin\ACFSample.uproject`
- `D:\Projects UE5\LustAsDeadlySin\Plugins\ACFUltimate\Config\ACFUPlugin.ini`
- `D:\Projects UE5\LustAsDeadlySin\Plugins\EFCharacterCreation\Config\DefaultGame.ini`
- `D:\Projects UE5\NoShellForWinter\Config`
- `D:\Projects UE5\NoShellForWinter\NoShellForWinter.uproject`
- `D:\Unreal Engine 5\Library\UE_5.8\Engine\Plugins\Marketplace\ACFUAsce5ab7c1439afbV5\Config`
- `D:\Unreal Engine 5\Library\UE_5.8\Engine\Plugins\DazToUnreal`

## SHA-256 evidence

| Input | SHA-256 |
|---|---|
| Source `DefaultEngine.ini` | `9ACB39F3562F1F17D8E41FD2360156FB5B3AE9FB131B1209D39C7EE7BBD1249F` |
| Source `DefaultGame.ini` | `9ECF2A49D26B4BA67BB68BFBBF99ACFF1BE7E724B8F49B981089983B1FA56C3B` |
| Source `DefaultInput.ini` | `A4AC7C28DA616E2D607152CEAFF3331E4291EAA41C1AE002CC182923A2754A1F` |
| Source `DefaultGameplayTags.ini` | `637A4275ECF17BE699F8F0935A052C594C65FE3C078033E0ACB131DE3BD89C60` |
| Source `DefaultPlugins.ini` | `843748F0E4529CC89E57306E4F2A2B0F0EDB754AA97F58211986FF6F008E8675` |
| Source `DefaultGameUserSettings.ini` | `0C4AFDB112045DFC33919652AFBCA8D5BC552440C3F52B26E0493631CA9E426C` |
| Source `DefaultScalability.ini` | `E18536DC09579BD7E749688F0F4E875C1F2E4DF9F74DC56B3FAF23920D909474` |
| Source `ACFSample.uproject` | `B62C86CE180C30C97A4EA910C947C5A8679D227FA29EE69693BF26E6BDB35CCE` |
| Source `ACFUPlugin.ini` | `3114BE1FEC8E23D1F065D6F79325E137D9983F0AADA67D0411C5BD1694421694` |
| Source EFCharacterCreation `DefaultGame.ini` | `A379C5AFEC1522284568A97BD23ED58B6720593A3B98CD296E2B245C4E7A0275` |
| Target `DefaultEngine.ini` | `6C8EE9FCD80FF8759AC178C3311AB6C74A2AFECE4BB89E828606689C1F870D2F` |
| Target `DefaultGame.ini` | `C662247C3A265200A54ED702536091A8D01772C5C821355ACCFA78C1A7B7203D` |
| Target `DefaultInput.ini` | `C9AC04CB9CEAC69DB053D8D2CB8FAF1CBC2F4D92A25EFB753B28D6CE944998E0` |
| Target `DefaultGameplayTags.ini` | `E5A6CA87CAACF04D27D3023D4A228E436F67362C525B770D71261C9BCD39E63A` |
| Target `DefaultPlugins.ini` | `C6318A8C49A828A6876A888EA5BEE1B489A6E95EFE738404091D8D42EF40615C` |
| Target `DefaultGameUserSettings.ini` | `BFD048A4B15ACCA84DCD5DC2496653E783D4AC7B4C2ABFF29E612041FEF2E866` |
| Target `NoShellForWinter.uproject` | `A6F7AC8B81F174DA5F58B7DB23643824BE60AA7C373BA82645ECD36D369BB36D` |
| ACFU 4.3.5 `DefaultEngine.ini` | `DAD91F77ECC0DB76518222FEE4B231F4EB9A257BBAB95B49C54015690967EF5F` |
| ACFU 4.3.5 `DefaultGame.ini` | `96F4FE8533D7306DA903BD1AA8F6AC9B1B48434ABDA3DD3F77F9741FFB86AA25` |
| ACFU 4.3.5 `DefaultInput.ini` | `C9AC04CB9CEAC69DB053D8D2CB8FAF1CBC2F4D92A25EFB753B28D6CE944998E0` |
| ACFU 4.3.5 `DefaultGameplayTags.ini` | `E5A6CA87CAACF04D27D3023D4A228E436F67362C525B770D71261C9BCD39E63A` |
| ACFU 4.3.5 `DefaultPlugins.ini` | `C6318A8C49A828A6876A888EA5BEE1B489A6E95EFE738404091D8D42EF40615C` |
| ACFU 4.3.5 `DefaultGameUserSettings.ini` | `BFD048A4B15ACCA84DCD5DC2496653E783D4AC7B4C2ABFF29E612041FEF2E866` |

## Exact ACFU equivalence result

Target and ACFU 4.3.5 hashes match for:

- `DefaultInput.ini`
- `DefaultGameplayTags.ini`
- `DefaultPlugins.ini`
- `DefaultGameUserSettings.ini`

Target `DefaultEngine.ini` differs only by project additions observed in the diff: four `TP_ThirdPerson -> NoShellForWinter` redirects plus HTTP and Android file-server settings.

Target `DefaultGame.ini` is not an ACFU merge. It contains the current Daz section and omits ACFU 4.3.5 CommonUI, CommonInput, and always-cook rows. This is a concrete deferred repair, not permission to overwrite the file.

## Source history classification

The following source files were compared with `git diff 1048d76..HEAD` to distinguish later project work from original ACF sample content:

- `Config/DefaultEngine.ini`
- `Config/DefaultGame.ini`
- `Config/DefaultGameplayTags.ini`
- `Config/DefaultGameUserSettings.ini`
- `Config/DefaultScalability.ini`
- `ACFSample.uproject`

Confirmed post-import project deltas:

- 47 project-owned Core Redirect rows.
- 58 `Project.*` gameplay tags.
- 14 project/system settings sections in `DefaultGame.ini`.
- `/Game/_Game/Widgets` as an additional cook root.
- 2048 MB texture pool, VRAM limiting, instance culling, motion blur disabled, VSM disabled.
- Low-cost scalability policy and a new `DefaultScalability.ini`.
- Explicit plugin additions for project systems, training, blink, DirtyPawn, widget bridge, tattoos, rendering/water features.

No worktree delta was found in the audited source config or `.uproject` files during this pass.

## Redirect evidence

Source `Config/DefaultEngine.ini` has 49 relevant redirect lines in total: two base ACF package redirects and 47 project-owned redirects. The latter cover Calysto-to-EF, ACFUltimateSample-to-EF, project symbol extraction, and EFEspabilar-to-EFBlink.

Source `Plugins/ACFUltimate/Config/ACFUPlugin.ini` contains 419 unsectioned `+PackageRedirects` lines and no section header:

| Classification against source filesystem | Count |
|---|---:|
| Old package missing, new package exists | 319 |
| Old package exists, new package missing | 79 |
| Both packages missing | 17 |
| Both packages exist | 1 |
| Non-`/Game` NiagaraFluids entries | 2 |

Only one candidate has an already existing target destination:

```text
/Game/FullSample/Integrations/Ultimate/Player.Player
  -> /Game/FullSample/Player.Player
```

This evidence rejects bulk promotion. It does not approve even that Player redirect until a target reference/load test requests it.

## Tags evidence

The source/current ACF diff shows the source would remove or replace new ACFU 4.3.5 tables, including current navigation, gameplay-cue, turn-ability, creator, and ladder-climbing tables. Therefore the target file is the merge base.

The only approved source candidates are the 58 post-import `Project.Intimacy.*` and `Project.Gender.*` rows. They remain pending EFProjectSystems load and tag-resolution tests.

## Input evidence

`DefaultInput.ini` source-to-current-ACFU diff shows the source would remove UE 5.8 Enhanced Input settings such as input-settings save slot, input-mode filtering, and default input mode. It must not replace target.

Textual ownership was traced as follows:

- O, L, Comma, N, C, Y, and J are project config keys in `EFProjectInputSettings`.
- Period is a direct EFCharacterCreation C++ binding.
- H is a conditional survival C++ binding.
- Minus is represented by direct `Hyphen` and `Subtract` defaults in intimacy settings.
- T and Plus were not found in textual config and remain asset/Blueprint plus PIE gates.
- `ConsoleKeys=Equals` exists in both source and target/ACFU config and conflicts with top-row Plus.

No input result is marked PASS from static inspection.

## Map and class route evidence

Source-only configured assets at audit time:

- `/Game/_Game/Hub/HUB.HUB`
- `/Game/FullSample/Integrations/ACF_GASUltimateGameMode_BP.ACF_GASUltimateGameMode_BP_C`
- `/Game/Procedural/Maps/DungeonGeneration.DungeonGeneration`
- `/Game/Procedural/DoorToLevel.DoorToLevel_C`
- `/Game/UI/Defeat/Struggle/WBP_ProjectKnockoutStruggleWidget...`
- `/Game/Data/CharacterBackground/DT_ProjectBackstories...`
- `/Game/Data/CharacterBackground/DT_ProjectProfessions...`
- `/Game/UI/CharacterBackground/WBP_ProjectCharacterBackgroundCreationWidget...`
- `Content/_Game/Images/preview.png`

The current target map/GameMode/GameInstance baseline has already produced a valid PIE baseline elsewhere in Phase 1 evidence. Therefore route changes are deferred until those source-only packages and their dependencies are migrated and validated.

## ACFU and Daz rejects

Rejected source payloads:

- Source ACFU binaries/config and old ACF Marketplace entry.
- Source `BaseAscentCombatFramework.ini` and `DefaultAscentCombatFramework.ini`.
- Source `DefaultPlugins.ini` as a whole; it contains old ACF modules and routes.
- Source Daz plugin/config and `ZeroRootRotationOnImport=False`.
- Source UE 5.7 preview-profile serialization.
- Generated source Android file-server token.

The target Daz section, ACFU 4.3.5 settings, Player/Female/Frederick/Multiple/Male invariants, and UE 5.8 module identity remain authoritative.

## `.uproject` evidence

The source is Engine 5.7/module `ACFSample`; the target is Engine 5.8/module `NoShellForWinter`. Identity replacement is rejected.

Required deferred project-owned plugins:

- EFCharacterCreation
- EFCharacterCreationDazBridge
- EFClothingMorph
- EFProcedural
- EFLevelFlow
- EFCharacterCreationACFUBridge
- EFProjectSystems

Source support plugins additionally identified:

- EFBlink
- ACFTrainingSystem
- DirtyPawnRuntime
- CodeWidgetDesignerBridge

PCG interops, ScriptableTools, DeformerGraph, MLDeformerFramework, Volumetrics, NiagaraFluids, and WaterAdvanced descriptors were found in the UE 5.8 engine. Availability alone does not approve enablement; each remains dependent on the content/plugin graph.

## Pending gates

1. Required project-owned plugins exist, descriptors validate, project files generate, and cold build passes.
2. Every configured source-only asset is migrated or deliberately remapped.
3. Configured classes and soft object paths load under UE 5.8.
4. Project redirects load only after their destination modules exist.
5. All 58 project tags resolve while current ACF tags remain intact.
6. O, Period, L, Comma, N, C, Y, J, H, T, Plus, and Minus pass PIE; Equals no longer steals Plus.
7. Render/scalability CVars are accepted by UE 5.8 and visual/performance QA passes.
8. HUB/GameMode route changes pass Blueprint compile, PIE, visual QA, cook, and packaged validation.
9. Effective merged config is inspected under `Saved/Config/WindowsEditor`.
10. Protected ACFU, Daz, Player, Female, Frederick, Multiple, and Male hashes are reverified.

Conclusion: inventory complete; no config merge has been executed or passed.
