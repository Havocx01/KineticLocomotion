import json
import os
import traceback

import unreal


CHARACTER_BLUEPRINT = "/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"
ANIMATION_BLUEPRINT = "/Game/ProjectFrontier/Animation/ABP_PlayerLocomotion"
FOOT_IK_CONTROL_RIG = "/Game/Characters/Mannequins/Rigs/CR_Mannequin_FootIK"


def set_property(target, property_name, value, changes):
    old_value = target.get_editor_property(property_name)
    target.set_editor_property(property_name, value)
    changes[property_name] = {"before": old_value, "after": value}


def main():
    report = {
        "success": False,
        "character_blueprint": CHARACTER_BLUEPRINT,
        "animation_blueprint": ANIMATION_BLUEPRINT,
        "foot_ik_control_rig": FOOT_IK_CONTROL_RIG,
        "changes": {},
    }

    try:
        character_bp = unreal.load_asset(CHARACTER_BLUEPRINT)
        anim_bp = unreal.load_asset(ANIMATION_BLUEPRINT)
        foot_ik_rig = unreal.load_asset(FOOT_IK_CONTROL_RIG)
        if not character_bp or not anim_bp or not foot_ik_rig:
            raise RuntimeError("Could not load the character, animation Blueprint, and foot IK rig.")

        character_cdo = unreal.get_default_object(character_bp.generated_class())
        anim_cdo = unreal.get_default_object(anim_bp.generated_class())

        character_changes = {}
        set_property(character_cdo, "minimum_detected_step_height", 4.0, character_changes)
        set_property(character_cdo, "ascending_stair_max_speed", 155.0, character_changes)
        set_property(character_cdo, "descending_stair_max_speed", 175.0, character_changes)
        set_property(character_cdo, "ascending_stair_sprint_max_speed", 290.0, character_changes)
        set_property(character_cdo, "descending_stair_sprint_max_speed", 320.0, character_changes)
        set_property(character_cdo, "minimum_adaptive_stair_speed", 90.0, character_changes)
        set_property(character_cdo, "default_stair_tread_depth", 38.0, character_changes)
        set_property(character_cdo, "ascending_stair_footfall_cadence", 4.25, character_changes)
        set_property(character_cdo, "descending_stair_footfall_cadence", 4.60, character_changes)
        set_property(character_cdo, "ascending_stair_sprint_footfall_cadence", 7.40, character_changes)
        set_property(character_cdo, "descending_stair_sprint_footfall_cadence", 8.10, character_changes)
        set_property(character_cdo, "minimum_adaptive_stair_sprint_speed", 220.0, character_changes)
        set_property(character_cdo, "stair_look_ahead_distance", 80.0, character_changes)
        set_property(character_cdo, "stair_acceleration_scale", 0.68, character_changes)
        set_property(character_cdo, "stair_braking_scale", 1.40, character_changes)
        set_property(character_cdo, "stair_sprint_acceleration_scale", 0.58, character_changes)
        set_property(character_cdo, "stair_sprint_braking_scale", 1.25, character_changes)
        set_property(character_cdo, "stair_response_hold_duration", 0.42, character_changes)
        set_property(character_cdo, "stair_response_interp_speed", 12.0, character_changes)
        set_property(character_cdo, "stair_response_exit_interp_speed", 4.0, character_changes)
        set_property(character_cdo, "stair_visual_smooth_time", 0.14, character_changes)
        set_property(character_cdo, "stair_visual_descend_smooth_time", 0.12, character_changes)
        set_property(character_cdo, "stair_visual_catchup_time", 0.14, character_changes)
        set_property(character_cdo, "max_stair_visual_offset", 40.0, character_changes)
        set_property(character_cdo, "stair_ground_perch_radius", 20.0, character_changes)
        set_property(character_cdo, "stair_ground_perch_additional_height", 16.0, character_changes)
        set_property(character_cdo, "stair_camera_lag_speed", 6.5, character_changes)
        set_property(character_cdo, "stair_camera_lag_max_distance", 50.0, character_changes)

        rig_changes = {}
        # Per-frame anim instance drives traces. Default stays on for flat ground.

        animation_changes = {}
        set_property(anim_cdo, "stair_locomotion_play_rate", 1.08, animation_changes)

        unreal.BlueprintEditorLibrary.compile_blueprint(character_bp)
        unreal.BlueprintEditorLibrary.compile_blueprint(anim_bp)
        unreal.BlueprintEditorLibrary.compile_blueprint(foot_ik_rig)
        unreal.EditorAssetLibrary.save_loaded_asset(character_bp, only_if_is_dirty=False)
        unreal.EditorAssetLibrary.save_loaded_asset(anim_bp, only_if_is_dirty=False)
        unreal.EditorAssetLibrary.save_loaded_asset(foot_ik_rig, only_if_is_dirty=False)

        report["changes"]["character"] = character_changes
        report["changes"]["foot_ik_control_rig"] = rig_changes
        animation_changes["control_rig_pin"] = "Driven every update by MaskedCharacterAnimInstance"
        report["changes"]["animation"] = animation_changes
        report["success"] = True
    except Exception as exc:
        report["error"] = str(exc)
        report["traceback"] = traceback.format_exc()

    report_path = os.path.join(
        unreal.Paths.project_saved_dir(), "StairFootPlacementSetupReport.json"
    )
    with open(report_path, "w", encoding="utf-8") as output_file:
        json.dump(report, output_file, indent=2, default=str)

    if not report["success"]:
        raise RuntimeError(report.get("error", "Stair foot placement setup failed."))


main()
