import json
import os
import traceback

import unreal


PROJECT_DIR = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
REPORT_PATH = os.path.join(PROJECT_DIR, "Saved", "GASPCrouchAnimationSetupReport.json")
ANIMATION_ROOT = "/Game/_AnimationLibrary/GASP"

ANIMATIONS = {
    "crouch_idle_animation": "M_Neutral_Crouch_Idle_Loop",
    "crouch_walk_animation": "M_Neutral_Crouch_Loop_F",
    "crouch_walk_forward_right_animation": "M_Neutral_Crouch_Loop_FR",
    "crouch_walk_right_animation": "M_Neutral_Crouch_Loop_RR",
    "crouch_walk_backward_right_animation": "M_Neutral_Crouch_Loop_BR",
    "crouch_walk_backward_animation": "M_Neutral_Crouch_Loop_B",
    "crouch_walk_backward_left_animation": "M_Neutral_Crouch_Loop_BL",
    "crouch_walk_left_animation": "M_Neutral_Crouch_Loop_LL",
    "crouch_walk_forward_left_animation": "M_Neutral_Crouch_Loop_FL",
    "stand_to_crouch_animation": "M_Neutral_Transition_Stand_to_Crouch",
    "crouch_to_stand_animation": "M_Neutral_Transition_Crouch_to_Stand",
}


def save_report(status, details):
    os.makedirs(os.path.dirname(REPORT_PATH), exist_ok=True)
    with open(REPORT_PATH, "w", encoding="utf-8") as handle:
        json.dump({"status": status, **details}, handle, indent=2)


def load_animations():
    loaded = {}
    for property_name, asset_name in ANIMATIONS.items():
        path = f"{ANIMATION_ROOT}/{asset_name}"
        animation = unreal.load_asset(path)
        if not isinstance(animation, unreal.AnimSequence):
            raise RuntimeError(f"Missing or invalid AnimSequence: {path}")

        for setting_name, value in (("enable_root_motion", False), ("force_root_lock", True)):
            try:
                animation.set_editor_property(setting_name, value)
            except Exception as error:
                unreal.log_warning(f"[GASPCrouchSetup] Could not set {setting_name} on {asset_name}: {error}")
        unreal.EditorAssetLibrary.save_loaded_asset(animation)
        loaded[property_name] = animation
    return loaded


def configure_animation_blueprint(animations):
    animation_blueprint = unreal.load_asset("/Game/ProjectFrontier/Animation/ABP_PlayerLocomotion")
    if not animation_blueprint:
        raise RuntimeError("Could not load ABP_PlayerLocomotion")

    animation_cdo = unreal.get_default_object(animation_blueprint.generated_class())
    for property_name, animation in animations.items():
        animation_cdo.set_editor_property(property_name, animation)

    # The dedicated neutral exit is a better sprint handoff than the old Mixamo clip.
    animation_cdo.set_editor_property("crouch_to_sprint_animation", animations["crouch_to_stand_animation"])
    animation_cdo.set_editor_property("legacy_crouch_transition_animation", None)
    unreal.BlueprintEditorLibrary.compile_blueprint(animation_blueprint)
    unreal.EditorAssetLibrary.save_loaded_asset(animation_blueprint)


def configure_character_blueprint():
    character_blueprint = unreal.load_asset("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter")
    if not character_blueprint:
        raise RuntimeError("Could not load BP_ThirdPersonCharacter")
    character_cdo = unreal.get_default_object(character_blueprint.generated_class())
    character_cdo.set_editor_property("crouch_visual_mesh_height_offset", 0.0)
    unreal.BlueprintEditorLibrary.compile_blueprint(character_blueprint)
    unreal.EditorAssetLibrary.save_loaded_asset(character_blueprint)


def main():
    animations = load_animations()
    configure_animation_blueprint(animations)
    configure_character_blueprint()
    unreal.EditorAssetLibrary.save_directory(ANIMATION_ROOT, only_if_is_dirty=False, recursive=True)
    save_report("success", {
        "animation_set": "Game Animation Sample - Neutral Crouch",
        "animations": {name: asset.get_path_name() for name, asset in animations.items()},
    })
    unreal.log("[GASPCrouchSetup] SUCCESS: neutral eight-direction crouch set configured")


try:
    main()
except Exception as exception:
    save_report("failed", {"error": str(exception), "traceback": traceback.format_exc()})
    unreal.log_error("[GASPCrouchSetup] FAILED: " + str(exception))
    unreal.log_error(traceback.format_exc())
    raise
