# Phase 5 — Enemy/Companion Male/Female and Intimacy migration

Date: 2026-07-16
Branch: `migration/ue58-lasd-parity`
Source: `D:\Projects UE5\LustAsDeadlySin` (read-only)
Target: `D:\Projects UE5\NoShellForWinter`

## Result

The project-owned enemy and Intimacy migration is implemented for 22 current UE 5.8 character classes:

- 16 hostile classes: eight Male and eight Female.
- 6 companions: three Male and three Female.
- Hostile-only systems are excluded from companions.
- Identity, selection, Social Card, Chronicle, history and Intimacy apply to all 22.
- Male-only genital/arousal morph configuration contains only the eight hostile Male classes.
- Female receives shared body/skin variation and no male morph configuration.

The public interaction id remains `Actions.Together.0001Scene`.

## Implemented runtime contracts

- `UEFProjectEnemySettings` contains explicit Male/Female identity registries and a hostile-only runtime registry.
- `UProjectCharacterIdentitySubsystem` applies one exclusive ASC gender tag and creates/updates `UProjectIntimacyPartnerComponent` for existing and spawned actors.
- `UProjectEnemyVisualVariationSubsystem` separates the shared 16-hostile list from the eight-Male genital/arousal list.
- Intimacy profiles repair legacy gender from the live partner component/class registry and retain relationship/history progression.
- `T -> Y` can open the contextual Project Emote menu for a selected Intimacy partner during ACF combat.
- `T -> -` starts `Actions.Together.0001Scene` through the quick-start contract.
- `Y` cancellation restores movement/look input, clears both combat shields, closes the session and restores the selected target.

## Character assets and naming

All final character packages use the organized paths below:

- `/Game/_Game/Characters/Male/*BPMale` — 11 packages.
- `/Game/_Game/Characters/Female/*BPFemale` — 11 packages.

The 11 Female object paths end in `BPFemale`; no final Female object path contains `BPMale1` or a numeric suffix. A binary text scan can still see old generated function-symbol strings embedded in renamed packages; those are not package/object references.

Two pre-existing, untracked Male root redirector files remain locally under `/Game/_Game/Characters` after Editor delete, commandlet delete and `ResavePackages -FixupRedirects` all reported success without removing the physical files. They have no referencers and are deliberately excluded from Git. Status: `PENDING_EDITOR_REDIRECTOR_CLEANUP`.

## Intimacy assets

- `/Game/_Game/Animations/Intimacy/Scenes/BP_IntimacyScene_0001`
- `/Game/_Game/Animations/Intimacy/Female/AS_DoggyClassic_1_Female`
- `/Game/_Game/Animations/Intimacy/Male/AS_DoggyClassic_1_Male_Corrected`

The scene uses the current `/Game/DazToUnreal/Female/Female` and `/Game/DazToUnreal/Male/Male` meshes. The menu Data Asset and native fallbacks use only the new scene path.

## Validation

| Gate | Result | Evidence |
|---|---|---|
| 22 character Blueprints + scene compile | PASS | Live UE 5.8 MCP compile: 23 compiled, zero failures |
| Final names/old paths | PASS | `BPMale1` asset search empty; three legacy `ExportedAnimations/Together` asset paths absent |
| Enemy registry automation | PASS | `Saved/Migration/Phase5/Tests/EnemiesFinal/index.json`: 2 succeeded, 0 failed |
| Intimacy automation | PASS | `Saved/Migration/Phase5/Tests/IntimacyFinal/index.json`: 14 succeeded, 0 failed |
| Cold Editor build after input fixes | PASS | UBT `NoShellForWinterEditor Win64 Development`, 48.10 s |
| `T -> Y` contextual menu | PASS runtime; PARTIAL visual | Runtime menu/widget states pass and `PARTNER` is visible in `02_ActionsCategory.png`; first desktop capture is rejected because Steam covered it, and accepted captures contain a small external reimport prompt |
| `T -> -`, active session and `Y` cancel | PASS | `Saved/Migration/Phase5/Runtime/EnemyIntimacyQuickStartPIE58.json` |
| Build/cook/stage/pak/archive | PASS | `Saved/Migration/Phase5/Package/EnemyIntimacyWin64FinalProtected`; UAT `BUILD SUCCESSFUL` in 238.14 s |
| Packaged content inventory | PASS | UnrealPak: 22 organized character assets and all 3 Intimacy assets present |
| Packaged interactive smoke | PENDING | Headless launch stayed alive but did not honor automated quit; no interactive packaged-session claim is made |
| Animation retarget/pose parity | PENDING | Explicitly permitted exception; asset load, references and package are blocking and pass |
| Male erection visual | PENDING_ASSET_DATA | Runtime routing is Male-only, but current Male mesh reports missing `DK_Flacid04` and `DK_Erection` morph targets, so the driver correctly skips them |

The focused PIE result proves:

- quick start returned true;
- session and runtime action became active;
- runtime action id was `Actions.Together.0001Scene`;
- the active partner was preserved during the scene;
- player and partner shields were applied during the scene;
- after `Y`, session/action were inactive, target was restored, shields were absent, and move/look input were enabled.

## Source and protected invariants

Source read-only verification is `PASS` in `Saved/Migration/Phase5/Hashes/EnemyIntimacy_SourceReadOnly_Final.json`; source HEAD/status and recorded modified-file hashes are unchanged.

Source assets after migration:

| Source file | SHA-256 |
|---|---|
| `ExportedAnimations/Together/0001Scene.uasset` | `C10760DE321CD6AF7B31F2A4A5963C540E894D649B4C034DD22445C3D3532AA0` |
| `ExportedAnimations/M_SexAnimations/AS_DoggyClassic_1_Male_Corrected.uasset` | `57B14FE30D263D9D22FBF097CFDA608AC7101F348BBB53175FFC4194D429DA24` |
| `ExportedAnimations/SexAnimations/AS_DoggyClassic_1_Female.uasset` | `13A895CC893FFC284F0F29DC715F8410623FD49D9724A2EDB3D9F86280A5C5C7` |
| `_Game/Images/Intimacy/Preview_IntimacyImage.png` | `849439546DDE6F5D9C60E83DAC2341371ECAB435FC520A6C3A77F3711377CEA6` |

Protected comparison against the original Phase 0 baseline still reports the 69 differences already recorded on 2026-07-15. Comparing the final state against `ContentOrganization_Post_ProtectedInvariants.json` gives:

- mismatch count before/after: 69 / 69;
- mismatch delta: 0;
- authoritative-asset delta: 0;
- task delta result: `PASS`.

ACFU (5,043 files) and the DazToUnreal plugin (213 files) match Phase 0 exactly. Final authoritative hashes are:

| Asset | Final SHA-256 | Task result |
|---|---|---|
| Player | `8B7E5EF8B831F4F13339FF6E06935E8D0DEAE1E7841F310A01D2D2AB6175EBAB` | unchanged from task start |
| Female | `B3BC3B6AEDF79C025F826305EFB219F827E3BE487DF7F2F7A4DC31FF17377BAB` | unchanged |
| Multiple | `350B862EC2CC547BC1820F17559880C25ACCCC063A669906E8FE6F1F8C17F780` | unchanged |
| Male | `6E4D5C11DFE71CC8FFB638950FAF897A96C580C06A2386D0D33CDAAE2C584217` | restored exactly to task-start LFS object after an Editor resave changed package metadata |
| Frederick | — | `PENDING_RUNTIME_IDENTITY`; no unambiguous package path exists in current evidence |

The rejected post-Editor Male package was preserved only under `Saved/Migration/Phase5/Hashes/ProtectedRecovery`; it is not staged.
