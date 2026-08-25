import json
import math
import os
import traceback

import unreal


LEVEL_PATH = "/Game/ThirdPerson/Lvl_ThirdPerson"
LAB_TAG = "TerrainMovementLab"
REPORT_PATH = os.path.join(
    unreal.Paths.project_saved_dir(), "TerrainMovementLabSetupReport.json"
)


def save_report(status, **details):
    os.makedirs(os.path.dirname(REPORT_PATH), exist_ok=True)
    with open(REPORT_PATH, "w", encoding="utf-8") as handle:
        json.dump({"status": status, **details}, handle, indent=2)


def main():
    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if not level_subsystem.load_level(LEVEL_PATH):
        raise RuntimeError(f"Could not load {LEVEL_PATH}")

    old_lab_actors = [
        actor
        for actor in actor_subsystem.get_all_level_actors()
        if LAB_TAG in {str(tag) for tag in actor.get_editor_property("tags")}
    ]
    if old_lab_actors:
        actor_subsystem.destroy_actors(old_lab_actors)

    ramp_mesh = unreal.load_asset("/Game/LevelPrototyping/Meshes/SM_Ramp")
    cube_mesh = unreal.load_asset("/Engine/BasicShapes/Cube")
    material = unreal.load_asset(
        "/Game/LevelPrototyping/Materials/MI_PrototypeGrid_Gray"
    )
    if not ramp_mesh or not cube_mesh or not material:
        raise RuntimeError("A terrain lab mesh or material could not be loaded")

    created = []

    def spawn(mesh, label, location, scale):
        actor = actor_subsystem.spawn_actor_from_class(
            unreal.StaticMeshActor, unreal.Vector(*location), unreal.Rotator()
        )
        if not actor:
            raise RuntimeError(f"Could not spawn {label}")
        actor.set_actor_label(label)
        actor.set_actor_scale3d(unreal.Vector(*scale))
        actor.set_editor_property("tags", [unreal.Name(LAB_TAG)])
        component = actor.get_component_by_class(unreal.StaticMeshComponent)
        if not component:
            raise RuntimeError(f"{label} has no StaticMeshComponent")
        component.set_static_mesh(mesh)
        component.set_material(0, material)
        created.append(label)
        return actor

    ramp_length = 800.0
    ramp_width = 280.0
    ramp_start_x = 400.0
    for angle, lane_y in ((12.0, -900.0), (25.0, 0.0), (40.0, 900.0)):
        height = math.tan(math.radians(angle)) * ramp_length
        spawn(
            ramp_mesh,
            f"TerrainLab_Ramp_{int(angle)}deg",
            (ramp_start_x, lane_y - ramp_width * 0.5, 0.0),
            (ramp_length / 100.0, ramp_width / 100.0, height / 100.0),
        )
        spawn(
            cube_mesh,
            f"TerrainLab_Ramp_{int(angle)}deg_Top",
            (ramp_start_x + ramp_length + 140.0, lane_y, height + 10.0),
            (2.8, ramp_width / 100.0, 0.2),
        )

    # A deliberately non-walkable control surface confirms the 46-degree limit.
    control_angle = 48.0
    control_y = 2400.0
    control_height = math.tan(math.radians(control_angle)) * 500.0
    spawn(
        ramp_mesh,
        "TerrainLab_Ramp_48deg_NonWalkable",
        (ramp_start_x, control_y - ramp_width * 0.5, 0.0),
        (5.0, ramp_width / 100.0, control_height / 100.0),
    )

    def spawn_staircase(prefix, start_x, lane_y, risers, tread_depth):
        stair_width = 280.0
        total_height = 0.0
        for index, rise in enumerate(risers):
            total_height += rise
            spawn(
                cube_mesh,
                f"TerrainLab_{prefix}_{index + 1:02d}",
                (
                    start_x + index * tread_depth + tread_depth * 0.5,
                    lane_y,
                    total_height * 0.5,
                ),
                (
                    tread_depth / 100.0,
                    stair_width / 100.0,
                    total_height / 100.0,
                ),
            )
        spawn(
            cube_mesh,
            f"TerrainLab_{prefix}_Top",
            (
                start_x + len(risers) * tread_depth + 140.0,
                lane_y,
                total_height + 10.0,
            ),
            (2.8, stair_width / 100.0, 0.2),
        )

    # Coverage includes building-code stairs, shallow outdoor stairs, demanding
    # 30 cm gameplay steps, and an irregular damaged staircase.
    spawn_staircase("StairStandard", 400.0, -1700.0, [18.0] * 10, 32.0)
    spawn_staircase("StairShallow", 400.0, -2400.0, [10.0] * 8, 50.0)
    spawn_staircase("StairSteep", 1600.0, -1700.0, [30.0] * 8, 45.0)
    spawn_staircase(
        "StairIrregular",
        1600.0,
        1700.0,
        [12.0, 18.0, 15.0, 22.0, 14.0, 20.0, 17.0, 24.0],
        40.0,
    )

    # Small height changes exercise floor continuity without becoming actual stairs.
    tile_heights = (2.0, 5.0, 3.0, 8.0, 4.0, 10.0, 6.0, 2.0, 7.0, 4.0, 9.0, 3.0)
    tile_start_x = 300.0
    tile_y = 1700.0
    tile_length = 100.0
    tile_width = 260.0
    for index, height in enumerate(tile_heights):
        spawn(
            cube_mesh,
            f"TerrainLab_UnevenTile_{index + 1:02d}",
            (tile_start_x + index * tile_length, tile_y, height * 0.5),
            (tile_length / 100.0, tile_width / 100.0, height / 100.0),
        )

    if not level_subsystem.save_current_level():
        raise RuntimeError("Could not save the terrain movement lab level")
    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    save_report("success", level=LEVEL_PATH, created_actors=created)
    unreal.log(f"[TerrainMovementLab] SUCCESS: created {len(created)} test actors")


try:
    main()
except Exception as exception:
    save_report("failed", error=str(exception), traceback=traceback.format_exc())
    unreal.log_error(f"[TerrainMovementLab] FAILED: {exception}")
    unreal.log_error(traceback.format_exc())
    raise
