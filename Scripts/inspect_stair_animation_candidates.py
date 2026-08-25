import unreal


CANDIDATES = [
    "/Game/_AnimationLibrary/GASP/M_Neutral_Walk_Loop_F",
    "/Game/_AnimationLibrary/GASP/M_Neutral_Walk_Start_F_Lfoot",
    "/Game/_AnimationLibrary/GASP/M_Neutral_Walk_Start_F_Rfoot",
    "/Game/Characters/Mannequins/Anims/Unarmed/Walk/MF_Unarmed_Walk_Fwd",
    "/Game/Characters/Mannequins/Anims/Unarmed/Walk/MF_Unarmed_Walk_Fwd_Left",
    "/Game/Characters/Mannequins/Anims/Unarmed/Walk/MF_Unarmed_Walk_Fwd_Right",
]


def describe(path):
    sequence = unreal.load_asset(path)
    if not sequence:
        unreal.log_warning(f"STAIR_CANDIDATE missing {path}")
        return

    skeleton = sequence.get_editor_property("skeleton")
    sync_markers = unreal.AnimationLibrary.get_animation_sync_markers(sequence)
    notify_events = unreal.AnimationLibrary.get_animation_notify_events(sequence)
    notify_times = [
        round(unreal.AnimationLibrary.get_anim_notify_event_trigger_time(event), 4)
        for event in notify_events
    ]
    notify_tracks = {}
    for track_name in unreal.AnimationLibrary.get_animation_notify_track_names(sequence):
        track_events = unreal.AnimationLibrary.get_animation_notify_events_for_track(
            sequence, track_name
        )
        notify_tracks[str(track_name)] = [
            round(unreal.AnimationLibrary.get_anim_notify_event_trigger_time(event), 4)
            for event in track_events
        ]
    unreal.log(
        "STAIR_CANDIDATE "
        f"{path} length={sequence.get_play_length():.4f} "
        f"rate_scale={sequence.get_editor_property('rate_scale'):.3f} "
        f"root_motion={sequence.get_editor_property('enable_root_motion')} "
        f"skeleton={skeleton.get_path_name() if skeleton else 'None'} "
        f"sync_markers={sync_markers} notify_times={notify_times} notify_tracks={notify_tracks}"
    )


for candidate in CANDIDATES:
    describe(candidate)

unreal.log(
    "STAIR_ANIMATION_LIBRARY_API "
    + str(
        [
            name
            for name in dir(unreal.AnimationLibrary)
            if any(token in name.lower() for token in ("notify", "sync", "marker", "curve"))
        ]
    )
)

anim_blueprint = unreal.load_asset("/Game/ProjectFrontier/Animation/ABP_PlayerLocomotion")
if anim_blueprint:
    anim_cdo = unreal.get_default_object(anim_blueprint.generated_class())
    interesting = [
        name
        for name in dir(anim_cdo)
        if any(token in name.lower() for token in ("rate", "stride", "speed", "walk", "locomotion"))
    ]
    unreal.log("STAIR_ABP_PROPERTIES " + str(interesting))
    unreal.log(
        "STAIR_RUNTIME_ANIMATION "
        f"stair_play_rate={anim_cdo.get_editor_property('stair_locomotion_play_rate')} "
        "mode=continuous_base_locomotion"
    )

character_blueprint = unreal.load_asset(
    "/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"
)
if character_blueprint:
    character_cdo = unreal.get_default_object(character_blueprint.generated_class())
    unreal.log(
        "STAIR_RUNTIME_MOVEMENT "
        f"up={character_cdo.get_editor_property('ascending_stair_max_speed')} "
        f"down={character_cdo.get_editor_property('descending_stair_max_speed')} "
        f"up_cadence={character_cdo.get_editor_property('ascending_stair_footfall_cadence')} "
        f"down_cadence={character_cdo.get_editor_property('descending_stair_footfall_cadence')}"
    )
