import unreal


LANDING_ASSETS = [
    "/Game/_AnimationLibrary/GASP/M_Neutral_Jump_F_Land_Stand_Light_Rfoot",
    "/Game/_AnimationLibrary/GASP/M_Neutral_Jump_F_Land_Run_Light_Rfoot",
    "/Game/_AnimationLibrary/GASP/M_Neutral_Jump_F_Land_Stand_Heavy_Rfoot",
    "/Game/_AnimationLibrary/GASP/M_Neutral_Jump_F_Land_Run_Heavy_Rfoot",
    "/Game/_AnimationLibrary/GASP/M_Neutral_Jump_F_Land_Walk_Light_Rfoot",
    "/Game/_AnimationLibrary/GASP/M_Neutral_Jump_F_Land_Walk_Heavy_Rfoot",
    "/Game/_AnimationLibrary/GASP/M_Neutral_Jump_F_Land_Sprint_Light_Rfoot",
    "/Game/_AnimationLibrary/GASP/M_Neutral_Jump_F_Land_Sprint_Heavy_Rfoot",
]


for path in LANDING_ASSETS:
    sequence = unreal.load_asset(path)
    if not sequence:
        unreal.log_warning(f"LANDING_TIMING missing={path}")
        continue

    tracks = {}
    for track_name in unreal.AnimationLibrary.get_animation_notify_track_names(sequence):
        events = unreal.AnimationLibrary.get_animation_notify_events_for_track(
            sequence, track_name
        )
        tracks[str(track_name)] = [
            round(unreal.AnimationLibrary.get_anim_notify_event_trigger_time(event), 4)
            for event in events
        ]

    markers = unreal.AnimationLibrary.get_animation_sync_markers(sequence)
    unreal.log(
        "LANDING_TIMING "
        f"asset={path} length={sequence.get_play_length():.4f} "
        f"tracks={tracks} markers={markers}"
    )
