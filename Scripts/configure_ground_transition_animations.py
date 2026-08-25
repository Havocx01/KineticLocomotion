import unreal


ROOT = "/Game/_AnimationLibrary/GASP"
NAMES = [
    "M_Neutral_Walk_Start_F_Lfoot",
    "M_Neutral_Walk_Start_F_Rfoot",
    "M_Neutral_Walk_Stop_F_Lfoot",
    "M_Neutral_Walk_Stop_F_Rfoot",
    "M_Neutral_Run_Start_F_Lfoot",
    "M_Neutral_Run_Start_F_Rfoot",
    "M_Neutral_Run_Stop_F_Lfoot",
    "M_Neutral_Run_Stop_F_Rfoot",
    "M_Neutral_Sprint_Stop_F_Lfoot",
    "M_Neutral_Sprint_Stop_F_Rfoot",
    "M_Neutral_Sprint_Start_F_Lfoot",
    "M_Neutral_Sprint_Start_F_Rfoot",
    "M_Neutral_Walk_Reface_Start_F_L_090",
    "M_Neutral_Walk_Reface_Start_F_R_090",
    "M_Neutral_Walk_Reface_Start_F_L_180",
    "M_Neutral_Walk_Reface_Start_F_R_180",
    "M_Neutral_Run_Reface_Start_F_L_090",
    "M_Neutral_Run_Reface_Start_F_R_090",
    "M_Neutral_Run_Reface_Start_F_L_180",
    "M_Neutral_Run_Reface_Start_F_R_180",
    "M_Neutral_Sprint_Reface_Start_F_L_090",
    "M_Neutral_Sprint_Reface_Start_F_R_090",
    "M_Neutral_Sprint_Reface_Start_F_L_180",
    "M_Neutral_Sprint_Reface_Start_F_R_180",
]


for name in NAMES:
    sequence = unreal.load_asset(f"{ROOT}/{name}")
    if not isinstance(sequence, unreal.AnimSequence):
        raise RuntimeError(f"Missing ground transition animation: {name}")

    # Movement remains capsule-authoritative. Locking root translation prevents
    # starts, stops and pivots from adding a second displacement or snapping back.
    sequence.set_editor_property("enable_root_motion", False)
    sequence.set_editor_property("force_root_lock", True)
    unreal.EditorAssetLibrary.save_loaded_asset(sequence)
    unreal.log(
        f"GROUND_TRANSITION_TIMING asset={name} "
        f"length={sequence.get_play_length():.4f}"
    )
