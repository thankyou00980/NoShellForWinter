"""Validate the effective target-safe EFCharacterCreation settings in UE 5.8."""

import datetime
import json
import os

import unreal


EVIDENCE_PATH = os.path.realpath(
    os.path.join(
        unreal.Paths.project_dir(),
        "Saved",
        "Migration",
        "Phase3",
        "EFCharacterCreationSettings58.json",
    )
)
EXPECTED_BODY_MESH = "/Game/DazToUnreal/Female/Female.Female"
EXPECTED_ROOT_CLASS = (
    "/EFCharacterCreation/UI/WBP_EFCharacterCreationRoot."
    "WBP_EFCharacterCreationRoot_C"
)
EXPECTED_SLIDER_CLASS = (
    "/EFCharacterCreation/UI/WBP_EFMorphSlider.WBP_EFMorphSlider_C"
)


def fail(message):
    unreal.log_error("CODEX_EFCC_CDO_FAIL: " + message)
    raise RuntimeError(message)


def object_path(value):
    return value.get_path_name() if value is not None else ""


def get_property(value, *names):
    errors = []
    for name in names:
        try:
            return value.get_editor_property(name)
        except Exception as exception:
            errors.append("{}: {}".format(name, exception))
    fail("Could not read any of {} ({})".format(names, "; ".join(errors)))


engine_version = unreal.SystemLibrary.get_engine_version()
if not engine_version.startswith("5.8."):
    fail("Expected UE 5.8 but found " + engine_version)

settings_class = unreal.load_class(
    None, "/Script/EFCharacterCreationRuntime.EFCharacterCreationSettings"
)
if settings_class is None:
    fail("Settings class is missing")

cdo = unreal.get_default_object(settings_class)
if cdo is None:
    fail("Settings CDO is missing")
if get_property(cdo, "bAutoEnterTestingMap", "auto_enter_testing_map") is not False:
    fail("bAutoEnterTestingMap must be false")
if get_property(
    cdo,
    "bAutoOpenOnCompatibleMainPawn",
    "auto_open_on_compatible_main_pawn",
) is not False:
    fail("bAutoOpenOnCompatibleMainPawn must be false")

options = list(get_property(cdo, "BodyMeshOptions", "body_mesh_options"))
if len(options) != 1:
    fail("Expected exactly one BodyMeshOptions entry, got {}".format(len(options)))
option = options[0]
display_name = str(get_property(option, "DisplayName", "display_name"))
body_mesh_path = object_path(get_property(option, "SkeletalMesh", "skeletal_mesh"))
if display_name != "Female":
    fail("Body mesh display name is not Female: " + display_name)
if body_mesh_path != EXPECTED_BODY_MESH:
    fail("Unexpected effective body mesh: " + body_mesh_path)

root_class_path = object_path(
    get_property(cdo, "RootWidgetClass", "root_widget_class")
)
slider_class_path = object_path(
    get_property(cdo, "MorphSliderWidgetClass", "morph_slider_widget_class")
)
if root_class_path != EXPECTED_ROOT_CLASS:
    fail("Unexpected root widget class: " + root_class_path)
if slider_class_path != EXPECTED_SLIDER_CLASS:
    fail("Unexpected morph slider widget class: " + slider_class_path)

payload = {
    "schema_version": 1,
    "generated_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
    "status": "EFFECTIVE_CONFIG_PASS",
    "engine_version": engine_version,
    "auto_enter_testing_map": False,
    "auto_open_on_compatible_main_pawn": False,
    "body_mesh_options": [
        {"display_name": display_name, "skeletal_mesh": body_mesh_path}
    ],
    "root_widget_class": root_class_path,
    "morph_slider_widget_class": slider_class_path,
}
os.makedirs(os.path.dirname(EVIDENCE_PATH), exist_ok=True)
with open(EVIDENCE_PATH, "w", encoding="utf-8", newline="\n") as handle:
    json.dump(payload, handle, indent=2, sort_keys=True)
    handle.write("\n")

unreal.log("CODEX_EFCC_CDO_PASS: " + EVIDENCE_PATH)
