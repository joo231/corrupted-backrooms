import os
import traceback

import unreal

ROOT = r"C:\Users\dio\projects\CorruptedBackrooms\RawArt"
LOG = r"C:\Users\dio\projects\CorruptedBackrooms\Tools\hub_save_log.txt"


def log(msg):
    unreal.log(msg)
    with open(LOG, "a", encoding="utf-8") as f:
        f.write(msg + "\n")


def import_file(src, dest):
    if not os.path.isfile(src):
        log("Missing " + src)
        return []
    task = unreal.AssetImportTask()
    task.filename = src
    task.destination_path = dest
    task.replace_existing = True
    task.automated = True
    task.save = True
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    imported = []
    try:
        imported = list(task.imported_object_paths)
    except Exception:
        pass
    log("Imported " + src + " -> " + str(imported))
    return imported


def set_complex_collision(path):
    mesh = unreal.load_asset(path)
    if not mesh:
        log("missing mesh " + path)
        return
    mat = unreal.load_asset("/Game/Art/Materials/M_RomanMarble")
    if mat:
        try:
            mesh.set_material(0, mat)
        except Exception as ex:
            log("set mat " + path + ": " + str(ex))
    try:
        body = mesh.get_editor_property("body_setup")
        if body:
            body.set_editor_property(
                "collision_trace_flag",
                unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE,
            )
            log("complex collision " + path)
    except Exception as ex:
        log("collision flag " + path + ": " + str(ex))
    try:
        unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    except Exception:
        pass
    bounds = mesh.get_bounds()
    log("KIT " + path + " extent=" + str(bounds.box_extent))


def main():
    open(LOG, "w", encoding="utf-8").write("start stairs\n")
    try:
        kit = os.path.join(ROOT, "Kit")
        import_file(os.path.join(kit, "SM_RomanStairChunk.obj"), "/Game/Art/Kit")
        import_file(os.path.join(kit, "SM_RomanStairUp.obj"), "/Game/Art/Kit")
        set_complex_collision("/Game/Art/Kit/SM_RomanStairChunk")
        set_complex_collision("/Game/Art/Kit/SM_RomanStairUp")
        world = unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
        log("world=" + str(world))
        count = unreal.CBWorldStatics.rebuild_hub(world)
        log("cpp rebuild actors=" + str(count))
        saved = unreal.EditorLoadingAndSavingUtils.save_map(world, "/Game/Maps/Hub")
        log("saved=" + str(saved))
        unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    except Exception as ex:
        log("FATAL " + str(ex))
        log(traceback.format_exc())


if __name__ == "__main__":
    main()
