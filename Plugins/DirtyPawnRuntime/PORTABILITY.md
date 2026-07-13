# DirtyPawnRuntime Portability

`DirtyPawnRuntime` is the portable runtime plugin for the LADS Dirty Pawn port. It intentionally has no dependency on EFProjectSystems, ACF, Sinful Ascension, Hygiene, decals, atlas pools, or SkinnedDecal.

## Runtime Folder

Copy this plugin folder first:

- `Plugins/DirtyPawnRuntime`

The plugin owns the C++ component, volumes, band model, unified wash compositor, and editor module.

## Required Project Content

The current stable LADS setup still references runtime assets under `/Game/DirtyPawnSystem`. Do not move those assets blindly. For another project, migrate the assets listed in `DirtyPawnRuntime.Portability.json` through Unreal Editor so redirectors and material function dependencies are preserved.

## Safe Integration Rules

- Keep `WashMask` away from clean `ColorIn`, DAZ base textures, and clean normals.
- Keep `BP_Water_Mud` out of unified wash.
- Keep stains visually above `Mud`, `Sand`, and `Snow`.
- Keep `Mud`, `Sand`, and `Snow` capped to three active environmental bands.
- Keep `Dirt`, `Blood`, `Burn`, and `Smear` persistent until Water/Wash.

## Optional Project Bridges

Project-specific systems should depend on `DirtyPawnRuntime`, not the reverse. LADS uses `UProjectDirtyPawnEffectsBridgeComponent` in EFProjectSystems to translate DirtyPawn coverage into Sinful Ascension/status effects.
