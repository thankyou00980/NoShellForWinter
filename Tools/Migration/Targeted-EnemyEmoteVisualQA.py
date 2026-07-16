"""Run the established Y-menu visual sequence after reproducing the T-selected enemy contract."""

import os
import runpy

import unreal


SOURCE_VISUAL_SCRIPT = os.environ.get(
    "CODEX_TARGETED_EMOTE_BASE_SCRIPT",
    r"D:\Projects UE5\LustAsDeadlySin\Tools\EmoteMenu\test_project_emote_visual_runtime.py",
)


namespace = runpy.run_path(SOURCE_VISUAL_SCRIPT, run_name="__codex_targeted_enemy_emote_base__")
original_execute_action = namespace["execute_action"]
selection_applied = False


def select_enemy_target():
    world = unreal.EditorLevelLibrary.get_game_world()
    player = unreal.GameplayStatics.get_player_pawn(world, 0) if world else None
    enemy_class = unreal.load_class(
        None,
        "/Game/_Game/Characters/Male/ACFRangedEnemyBPMale.ACFRangedEnemyBPMale_C",
    )
    enemies = unreal.GameplayStatics.get_all_actors_of_class(world, enemy_class) if world and enemy_class else []
    enemy = enemies[0] if enemies else None
    if not player or not enemy:
        raise RuntimeError("Targeted Y-menu QA could not resolve the player and Male enemy.")

    targeting_component = None
    for component in player.get_components_by_class(unreal.ActorComponent):
        if "ProjectTargetingFixComponent" in component.get_class().get_name():
            targeting_component = component
            break
    if not targeting_component:
        raise RuntimeError("Targeted Y-menu QA could not resolve ProjectTargetingFixComponent.")
    if not targeting_component.debug_set_current_target_actor(enemy):
        raise RuntimeError("Targeted Y-menu QA could not reproduce T-selected enemy state.")

    unreal.log(
        "[CodexProjectEmoteVisual] targeted_enemy_selected=True enemy=" + enemy.get_name()
    )


def targeted_execute_action(subsystem, action_name):
    global selection_applied
    if action_name == "toggle_menu" and not selection_applied:
        select_enemy_target()
        selection_applied = True
    return original_execute_action(subsystem, action_name)


# runpy may return a shallow namespace copy; patch the live globals retained by
# the registered tick callback rather than only the returned dictionary.
original_execute_action.__globals__["execute_action"] = targeted_execute_action
