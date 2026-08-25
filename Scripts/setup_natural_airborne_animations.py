import json
import os
import traceback

import unreal


PROJECT_DIR = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
REPORT_PATH = os.path.join(PROJECT_DIR, "Saved", "NaturalAirborneAnimationSetupReport.json")

ANIMATIONS = {
    "routine_jump_loop_animation": "/Game/Characters/Mannequins/Anims/Unarmed/Jump/MM_Jump",
    "fall_loop_animation": "/Game/_AnimationLibrary/ALS/ALS_N_FallLoop",
}


def save_report(status, details):
    os.makedirs(os.path.dirname(REPORT_PATH), exist_ok=True)
    with open(REPORT_PATH, "w", encoding="utf-8") as handle:
        json.dump({"status": status, **details}, handle, indent=2)


def load_animations():
    loaded = {}
    for property_name, path in ANIMATIONS.items():
        animation = unreal.load_asset(path)
        if not isinstance(animation, unreal.AnimSequence):
            raise RuntimeError(f"Missing or invalid AnimSequence: {path}")

        for setting_name, value in (("enable_root_motion", False), ("force_root_lock", True)):
            try:
                animation.set_editor_property(setting_name, value)
            except Exception as error:
                unreal.log_warning(
                    f"[NaturalAirborneSetup] Could not set {setting_name} on {path}: {error}"
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

    unreal.BlueprintEditorLibrary.compile_blueprint(animation_blueprint)
    unreal.EditorAssetLibrary.save_loaded_asset(animation_blueprint)


def main():
    animations = load_animations()
    configure_animation_blueprint(animations)
    save_report(
        "success",
        {"animations": {name: asset.get_path_name() for name, asset in animations.items()}},
    )
    unreal.log("[NaturalAirborneSetup] SUCCESS: restrained jump arc and dedicated fall loop configured")


try:
    main()
except Exception as exception:
    save_report("failed", {"error": str(exception), "traceback": traceback.format_exc()})
    unreal.log_error("[NaturalAirborneSetup] FAILED: " + str(exception))
    unreal.log_error(traceback.format_exc())
    raise
