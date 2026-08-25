import unreal


ROOT = "/Game/_AnimationLibrary/GASP"
NAMES = [
    "M_Neutral_Stand_Turn_045_L",
    "M_Neutral_Stand_Turn_045_R",
    "M_Neutral_Stand_Turn_090_L",
    "M_Neutral_Stand_Turn_090_R",
    "M_Neutral_Stand_Turn_135_L",
    "M_Neutral_Stand_Turn_135_R",
    "M_Neutral_Stand_Turn_180_L",
    "M_Neutral_Stand_Turn_180_R",
]


for name in NAMES:
    sequence = unreal.load_asset(f"{ROOT}/{name}")
    if not isinstance(sequence, unreal.AnimSequence):
        raise RuntimeError(f"Missing turn animation: {name}")

    sequence.set_editor_property("enable_root_motion", False)
    sequence.set_editor_property("force_root_lock", True)
    unreal.EditorAssetLibrary.save_loaded_asset(sequence)
    unreal.log(
        f"TURN_TIMING asset={name} length={sequence.get_play_length():.4f}"
    )

