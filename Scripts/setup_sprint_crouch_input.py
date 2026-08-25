import json
import os
import traceback

import unreal


INPUT_ACTION_ROOT = "/Game/Input/Actions"
DEFAULT_IMC_PATH = "/Game/Input/IMC_Default"
CHARACTER_BP_PATH = "/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"

# Mirrors the native fallback bindings in AmaskedCharacter::SetupPlayerInputComponent,
# so moving to Enhanced Input preserves the exact same physical keys.
ACTIONS = {
    "IA_Sprint": ("LeftShift", "Gamepad_LeftThumbstick"),
    "IA_Crouch": ("C", "LeftControl", "Gamepad_FaceButton_Right"),
}

REPORT_PATH = os.path.join(
    unreal.Paths.project_saved_dir(), "SprintCrouchInputSetupReport.json"
)


def save_report(status, **details):
    os.makedirs(os.path.dirname(REPORT_PATH), exist_ok=True)
    with open(REPORT_PATH, "w", encoding="utf-8") as handle:
        json.dump({"status": status, **details}, handle, indent=2)


def get_or_create_action(name):
    path = f"{INPUT_ACTION_ROOT}/{name}"
    existing = unreal.load_asset(path)
    if isinstance(existing, unreal.InputAction):
        return existing

    factory = unreal.InputActionFactory()
    action = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name=name,
        package_path=INPUT_ACTION_ROOT,
        asset_class=unreal.InputAction,
        factory=factory,
    )
    if not action:
        raise RuntimeError(f"Could not create {path}")
    return action


def add_mapping(imc, action, key_name):
    key = unreal.Key(key_name)
    try:
        # Preferred: the dedicated mapping API appends a fresh entry itself.
        imc.map_key(action, key)
        return
    except AttributeError:
        # Fallback for bindings without map_key: edit the Mappings array directly.
        mapping = unreal.EnhancedActionKeyMapping()
        mapping.set_editor_property("action", action)
        mapping.set_editor_property("key", key)
        mappings = list(imc.get_editor_property("mappings"))
        mappings.append(mapping)
        imc.set_editor_property("mappings", mappings)


def main():
    jump_action = unreal.load_asset(f"{INPUT_ACTION_ROOT}/IA_Jump")
    if not isinstance(jump_action, unreal.InputAction):
        raise RuntimeError("IA_Jump is missing; it is the digital value-type template")

    # A fresh UInputAction already defaults to Bool, but copy the value type from
    # the jump action so sprint/crouch are guaranteed digital regardless of engine
    # defaults (and no enum literal has to be spelled out here).
    digital_value_type = jump_action.get_editor_property("value_type")

    actions = {}
    for name in ACTIONS:
        action = get_or_create_action(name)
        action.set_editor_property("value_type", digital_value_type)
        unreal.EditorAssetLibrary.save_loaded_asset(action)
        actions[name] = action

    imc = unreal.load_asset(DEFAULT_IMC_PATH)
    if not imc:
        raise RuntimeError(f"Could not load {DEFAULT_IMC_PATH}")

    # Best-effort idempotency: clear stale bindings before re-adding so a rerun
    # never accumulates duplicate key mappings.
    for action in actions.values():
        try:
            imc.unmap_all_keys_from_action(action)
        except AttributeError:
            pass

    for name, key_names in ACTIONS.items():
        for key_name in key_names:
            add_mapping(imc, actions[name], key_name)
    unreal.EditorAssetLibrary.save_loaded_asset(imc)

    character_bp = unreal.load_asset(CHARACTER_BP_PATH)
    if not character_bp:
        raise RuntimeError(f"Could not load {CHARACTER_BP_PATH}")
    character_cdo = unreal.get_default_object(character_bp.generated_class())
    character_cdo.set_editor_property("sprint_action", actions["IA_Sprint"])
    character_cdo.set_editor_property("crouch_action", actions["IA_Crouch"])
    unreal.BlueprintEditorLibrary.compile_blueprint(character_bp)
    unreal.EditorAssetLibrary.save_loaded_asset(character_bp)

    save_report(
        "success",
        actions={name: actions[name].get_path_name() for name in actions},
        mappings=ACTIONS,
        mapping_context=imc.get_path_name(),
        character=character_bp.get_path_name(),
    )
    unreal.log("[SprintCrouchInput] SUCCESS: sprint/crouch moved to Enhanced Input")


try:
    main()
except Exception as exception:
    save_report("failed", error=str(exception), traceback=traceback.format_exc())
    unreal.log_error("[SprintCrouchInput] FAILED: " + str(exception))
    unreal.log_error(traceback.format_exc())
    raise
