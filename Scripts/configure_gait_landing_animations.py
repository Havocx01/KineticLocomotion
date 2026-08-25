import unreal


ROOT = "/Game/_AnimationLibrary/GASP"
NAMES = [
    "M_Neutral_Jump_F_Land_Walk_Light_Rfoot",
    "M_Neutral_Jump_F_Land_Walk_Heavy_Rfoot",
    "M_Neutral_Jump_F_Land_Sprint_Light_Rfoot",
    "M_Neutral_Jump_F_Land_Sprint_Heavy_Rfoot",
]


for name in NAMES:
    sequence = unreal.load_asset(f"{ROOT}/{name}")
    if not isinstance(sequence, unreal.AnimSequence):
        raise RuntimeError(f"Missing gait landing animation: {name}")

    sequence.set_editor_property("enable_root_motion", False)
    sequence.set_editor_property("force_root_lock", True)
    unreal.EditorAssetLibrary.save_loaded_asset(sequence)
    unreal.log(f"GAIT_LANDING asset={name} length={sequence.get_play_length():.4f}")
