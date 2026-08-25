import json
import os
import traceback

import unreal


PROJECT_DIR = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
IMPORT_DIR = os.path.join(PROJECT_DIR, "Import", "Locomotion")
ROOT_PATH = "/Game/ProjectFrontier/Animation/Crouch"
SOURCE_PATH = ROOT_PATH + "/Source"
RETARGET_PATH = ROOT_PATH + "/Retargeted"
REPORT_PATH = os.path.join(PROJECT_DIR, "Saved", "CrouchAnimationSetupReport.json")

CLIPS = {
    "CrouchIdle": "Crouching Idle.fbx",
    "CrouchWalk": "Crouched Walking.fbx",
    "CrouchToStand": "Crouch To Stand.fbx",
    "CrouchToSprint": "Crouched To Sprinting.fbx",
}


def log(message):
    unreal.log("[CrouchSetup] " + str(message))


def save_report(status, details):
    os.makedirs(os.path.dirname(REPORT_PATH), exist_ok=True)
    with open(REPORT_PATH, "w", encoding="utf-8") as handle:
        json.dump({"status": status, **details}, handle, indent=2)


def assets_in(path):
    return unreal.EditorAssetLibrary.list_assets(path, recursive=True, include_folder=False)


def import_task(filename, destination_name, import_ui):
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", filename)
    task.set_editor_property("destination_path", SOURCE_PATH)
    task.set_editor_property("destination_name", destination_name)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    task.set_editor_property("options", import_ui)
    before = set(assets_in(SOURCE_PATH))
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    imported = list(task.get_editor_property("imported_object_paths"))
    after = set(assets_in(SOURCE_PATH))
    return imported or sorted(after - before)


def make_mesh_import_ui():
    ui = unreal.FbxImportUI()
    ui.set_editor_property("automated_import_should_detect_type", False)
    ui.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_SKELETAL_MESH)
    ui.set_editor_property("import_as_skeletal", True)
    ui.set_editor_property("import_mesh", True)
    ui.set_editor_property("import_animations", True)
    ui.set_editor_property("import_materials", False)
    ui.set_editor_property("import_textures", False)
    ui.set_editor_property("create_physics_asset", False)
    return ui


def make_animation_import_ui(skeleton):
    ui = unreal.FbxImportUI()
    ui.set_editor_property("automated_import_should_detect_type", False)
    ui.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_ANIMATION)
    ui.set_editor_property("import_as_skeletal", True)
    ui.set_editor_property("import_mesh", False)
    ui.set_editor_property("import_animations", True)
    ui.set_editor_property("import_materials", False)
    ui.set_editor_property("import_textures", False)
    ui.set_editor_property("skeleton", skeleton)
    return ui


def objects_of_class(paths, object_class):
    result = []
    for path in paths:
        obj = unreal.load_asset(path)
        if isinstance(obj, object_class):
            result.append(obj)
    return result


def rename_asset_object(asset, destination_path):
    source_path = asset.get_path_name()
    if source_path == destination_path or source_path.split(".")[0] == destination_path:
        return unreal.load_asset(destination_path)
    if unreal.EditorAssetLibrary.does_asset_exist(destination_path):
        return unreal.load_asset(destination_path)
    if not unreal.EditorAssetLibrary.rename_asset(source_path, destination_path):
        raise RuntimeError(f"Failed to rename {source_path} to {destination_path}")
    return unreal.load_asset(destination_path)


def create_ik_rig(asset_name, mesh):
    path = f"{ROOT_PATH}/{asset_name}"
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        unreal.EditorAssetLibrary.delete_asset(path)
    rig = unreal.IKRigDefinitionFactory.create_new_ik_rig_asset(ROOT_PATH, asset_name)
    controller = unreal.IKRigController.get_controller(rig)
    if not controller.set_skeletal_mesh(mesh):
        raise RuntimeError(f"Failed to assign {mesh.get_name()} to {asset_name}")
    controller.apply_auto_generated_retarget_definition()
    if not controller.get_retarget_chains():
        raise RuntimeError(f"Auto retarget chain generation failed for {asset_name}")
    unreal.EditorAssetLibrary.save_loaded_asset(rig)
    return rig, controller


def configure_animation(animation, loop):
    for property_name, value in (
        ("enable_root_motion", False),
        ("force_root_lock", True),
    ):
        try:
            animation.set_editor_property(property_name, value)
        except Exception as error:
            log(f"Optional property {property_name} was not applied to {animation.get_name()}: {error}")
    unreal.EditorAssetLibrary.save_loaded_asset(animation)


def assign_animations_to_blueprint(target_animations):
    anim_blueprint = unreal.load_asset("/Game/ProjectFrontier/Animation/ABP_PlayerLocomotion")
    if not anim_blueprint:
        raise RuntimeError("Could not load ABP_PlayerLocomotion")
    anim_cdo = unreal.get_default_object(anim_blueprint.generated_class())
    anim_cdo.set_editor_property("crouch_idle_animation", target_animations["CrouchIdle"])
    anim_cdo.set_editor_property("crouch_walk_animation", target_animations["CrouchWalk"])
    anim_cdo.set_editor_property("crouch_to_stand_animation", target_animations["CrouchToStand"])
    anim_cdo.set_editor_property("crouch_to_sprint_animation", target_animations["CrouchToSprint"])
    unreal.BlueprintEditorLibrary.compile_blueprint(anim_blueprint)
    unreal.EditorAssetLibrary.save_loaded_asset(anim_blueprint)


def main():
    missing_files = [name for name in CLIPS.values() if not os.path.isfile(os.path.join(IMPORT_DIR, name))]
    if missing_files:
        raise RuntimeError("Missing FBX files: " + ", ".join(missing_files))

    existing_target_animations = {
        clip_key: unreal.load_asset(f"{RETARGET_PATH}/A_RTG_{clip_key}")
        for clip_key in CLIPS
    }
    if all(existing_target_animations.values()):
        configure_animation(existing_target_animations["CrouchIdle"], True)
        configure_animation(existing_target_animations["CrouchWalk"], True)
        configure_animation(existing_target_animations["CrouchToStand"], False)
        configure_animation(existing_target_animations["CrouchToSprint"], False)
        assign_animations_to_blueprint(existing_target_animations)
        save_report("success", {
            "reused_existing_assets": True,
            "animations": {key: value.get_path_name() for key, value in existing_target_animations.items()},
        })
        log("SUCCESS: existing retargeted animations configured and assigned")
        return

    if unreal.EditorAssetLibrary.does_directory_exist(ROOT_PATH):
        unreal.EditorAssetLibrary.delete_directory(ROOT_PATH)
    unreal.EditorAssetLibrary.make_directory(SOURCE_PATH)
    unreal.EditorAssetLibrary.make_directory(RETARGET_PATH)

    idle_file = os.path.join(IMPORT_DIR, CLIPS["CrouchIdle"])
    imported = import_task(idle_file, "SK_CrouchSource", make_mesh_import_ui())
    imported.extend(assets_in(SOURCE_PATH))
    source_meshes = objects_of_class(sorted(set(imported)), unreal.SkeletalMesh)
    if not source_meshes:
        raise RuntimeError("The idle FBX did not produce a source Skeletal Mesh")
    source_mesh = rename_asset_object(source_meshes[0], SOURCE_PATH + "/SK_CrouchSource")
    source_skeleton = source_mesh.get_editor_property("skeleton")
    if not source_skeleton:
        raise RuntimeError("The imported source mesh has no Skeleton")
    source_skeleton = rename_asset_object(source_skeleton, SOURCE_PATH + "/SKEL_CrouchSource")

    source_sequences = objects_of_class(assets_in(SOURCE_PATH), unreal.AnimSequence)
    if not source_sequences:
        raise RuntimeError("The idle FBX did not produce an animation sequence")
    source_animations = {
        "CrouchIdle": rename_asset_object(source_sequences[0], SOURCE_PATH + "/A_SRC_CrouchIdle")
    }

    for clip_key in ("CrouchWalk", "CrouchToStand", "CrouchToSprint"):
        clip_path = os.path.join(IMPORT_DIR, CLIPS[clip_key])
        imported_paths = import_task(clip_path, "A_SRC_" + clip_key, make_animation_import_ui(source_skeleton))
        sequences = objects_of_class(imported_paths, unreal.AnimSequence)
        if not sequences:
            sequences = [
                asset for asset in objects_of_class(assets_in(SOURCE_PATH), unreal.AnimSequence)
                if asset.get_name() == "A_SRC_" + clip_key
            ]
        if not sequences:
            raise RuntimeError(f"The {clip_key} FBX did not produce an animation sequence")
        source_animations[clip_key] = rename_asset_object(
            sequences[0], SOURCE_PATH + "/A_SRC_" + clip_key)

    target_mesh = unreal.load_asset("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple")
    if not target_mesh:
        raise RuntimeError("Could not load SKM_Manny_Simple")

    source_ik_rig, source_controller = create_ik_rig("IK_CrouchSource", source_mesh)
    target_ik_rig, target_controller = create_ik_rig("IK_Manny_CrouchTarget", target_mesh)

    rtg_path = ROOT_PATH + "/RTG_CrouchSource_To_Manny"
    if unreal.EditorAssetLibrary.does_asset_exist(rtg_path):
        unreal.EditorAssetLibrary.delete_asset(rtg_path)
    retargeter = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name="RTG_CrouchSource_To_Manny",
        package_path=ROOT_PATH,
        asset_class=unreal.IKRetargeter,
        factory=unreal.IKRetargetFactory())
    rtg_controller = unreal.IKRetargeterController.get_controller(retargeter)
    rtg_controller.set_ik_rig(unreal.RetargetSourceOrTarget.SOURCE, source_ik_rig)
    rtg_controller.set_ik_rig(unreal.RetargetSourceOrTarget.TARGET, target_ik_rig)
    rtg_controller.set_preview_mesh(unreal.RetargetSourceOrTarget.SOURCE, source_mesh)
    rtg_controller.set_preview_mesh(unreal.RetargetSourceOrTarget.TARGET, target_mesh)
    rtg_controller.add_default_ops()
    rtg_controller.auto_map_chains(unreal.AutoMapChainType.FUZZY, True)
    try:
        rtg_controller.auto_align_all_bones(
            unreal.RetargetSourceOrTarget.TARGET,
            unreal.RetargetAutoAlignMethod.CHAIN_TO_CHAIN)
    except Exception as error:
        log(f"Target auto-alignment was unavailable; using generated retarget pose: {error}")
    unreal.EditorAssetLibrary.save_loaded_asset(retargeter)

    asset_subsystem = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
    source_asset_data = [asset_subsystem.find_asset_data(asset.get_path_name()) for asset in source_animations.values()]
    retargeted_asset_data = unreal.IKRetargetBatchOperation.duplicate_and_retarget(
        source_asset_data,
        source_mesh,
        target_mesh,
        retargeter,
        search="A_SRC_",
        replace="A_RTG_",
        prefix="",
        suffix="",
        include_referenced_assets=True)

    retargeted_objects = [asset_data.get_asset() for asset_data in (retargeted_asset_data or [])]

    target_animations = {}
    candidates = list(retargeted_objects or []) + objects_of_class(assets_in(SOURCE_PATH), unreal.AnimSequence)
    for clip_key in CLIPS:
        desired_name = "A_RTG_" + clip_key
        candidate = next((obj for obj in candidates if isinstance(obj, unreal.AnimSequence) and obj.get_name() == desired_name), None)
        if not candidate:
            raise RuntimeError(f"Retarget did not produce {desired_name}")
        target_animations[clip_key] = rename_asset_object(candidate, RETARGET_PATH + "/" + desired_name)

    configure_animation(target_animations["CrouchIdle"], True)
    configure_animation(target_animations["CrouchWalk"], True)
    configure_animation(target_animations["CrouchToStand"], False)
    configure_animation(target_animations["CrouchToSprint"], False)

    assign_animations_to_blueprint(target_animations)

    unreal.EditorAssetLibrary.save_directory(ROOT_PATH, only_if_is_dirty=False, recursive=True)
    save_report("success", {
        "source_mesh": source_mesh.get_path_name(),
        "source_skeleton": source_skeleton.get_path_name(),
        "source_chains": [str(chain.chain_name) for chain in source_controller.get_retarget_chains()],
        "target_chains": [str(chain.chain_name) for chain in target_controller.get_retarget_chains()],
        "retargeter": retargeter.get_path_name(),
        "animations": {key: value.get_path_name() for key, value in target_animations.items()},
    })
    log("SUCCESS: crouch animations imported, retargeted, configured, and assigned")


try:
    main()
except Exception as exception:
    details = {
        "error": str(exception),
        "traceback": traceback.format_exc(),
        "assets_present": list(assets_in(ROOT_PATH)) if unreal.EditorAssetLibrary.does_directory_exist(ROOT_PATH) else [],
    }
    save_report("failed", details)
    unreal.log_error("[CrouchSetup] FAILED: " + str(exception))
    unreal.log_error(details["traceback"])
    raise
