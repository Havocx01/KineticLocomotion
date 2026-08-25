import json
import os
import traceback

import unreal


PROJECT_DIR = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
REPORT_PATH = os.path.join(PROJECT_DIR, "Saved", "GASPJumpLandingAnimationSetupReport.json")
ANIMATION_ROOT = "/Game/_AnimationLibrary/GASP"

ANIMATIONS = {
    "jump_start_standing_animation": "M_Neutral_Jump_F_Start_Stand_Rfoot",
    "jump_start_moving_animation": "M_Neutral_Jump_F_Start_Run_Rfoot",
    "fall_loop_animation": "M_Neutral_Jump_Loop_Fall",
    "light_landing_standing_animation": "M_Neutral_Jump_F_Land_Stand_Light_Rfoot",
    "light_landing_moving_animation": "M_Neutral_Jump_F_Land_Run_Light_Rfoot",
    "heavy_landing_standing_animation": "M_Neutral_Jump_F_Land_Stand_Heavy_Rfoot",
    "heavy_landing_moving_animation": "M_Neutral_Jump_F_Land_Run_Heavy_Rfoot",
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

        # Movement remains capsule-driven. Locking extracted root translation avoids
        # double movement and makes impact recovery deterministic.
        for setting_name, value in (("enable_root_motion", False), ("force_root_lock", True)):
            try:
                animation.set_editor_property(setting_name, value)
            except Exception as error:
                unreal.log_warning(
                    f"[GASPJumpLandingSetup] Could not set {setting_name} on {asset_name}: {error}"
                )
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

    animation_cdo.set_editor_property("light_landing_contact_time", 0.52)
    animation_cdo.set_editor_property("heavy_landing_contact_time", 1.02)
    animation_cdo.set_editor_property("light_landing_visible_duration", 0.30)
    animation_cdo.set_editor_property("medium_landing_visible_duration", 0.46)
    animation_cdo.set_editor_property("heavy_landing_visible_duration", 0.62)

    unreal.BlueprintEditorLibrary.compile_blueprint(animation_blueprint)
    unreal.EditorAssetLibrary.save_loaded_asset(animation_blueprint)


def main():
    animations = load_animations()
    configure_animation_blueprint(animations)
    unreal.EditorAssetLibrary.save_directory(ANIMATION_ROOT, only_if_is_dirty=False, recursive=True)
    save_report(
        "success",
        {
            "animation_set": "Game Animation Sample - Neutral Jump and Landing",
            "animations": {name: asset.get_path_name() for name, asset in animations.items()},
        },
    )
    unreal.log("[GASPJumpLandingSetup] SUCCESS: jump, fall, and landing set configured")


try:
    main()
except Exception as exception:
    save_report("failed", {"error": str(exception), "traceback": traceback.format_exc()})
    unreal.log_error("[GASPJumpLandingSetup] FAILED: " + str(exception))
    unreal.log_error(traceback.format_exc())
    raise
