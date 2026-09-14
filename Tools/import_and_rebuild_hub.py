import os
import traceback

import unreal

ROOT = r"C:\Users\dio\projects\CorruptedBackrooms\RawArt"
LOG = r"C:\Users\dio\projects\CorruptedBackrooms\Tools\hub_save_log.txt"


def log(msg):
    unreal.log(msg)
    with open(LOG, "a", encoding="utf-8") as f:
        f.write(msg + "\n")


def save_all():
    try:
        unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
        log("Saved dirty packages")
    except Exception as ex:
        log("save dirty: " + str(ex))


def import_file(src, dest, options=None):
    if not os.path.isfile(src):
        log("Missing " + src)
        return []
    task = unreal.AssetImportTask()
    task.filename = src
    task.destination_path = dest
    task.replace_existing = True
    task.automated = True
    task.save = True
    if options is not None:
        task.options = options
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    imported = []
    try:
        imported = list(task.imported_object_paths)
    except Exception:
        pass
    log("Imported " + src + " -> " + dest + " " + str(imported))
    return imported


def fbx_options():
    options = unreal.FbxImportUI()
    options.import_mesh = True
    options.import_as_skeletal = False
    options.import_materials = True
    options.import_textures = True
    options.import_animations = False
    options.mesh_type_to_import = unreal.FBXImportType.FBXIT_STATIC_MESH
    sm = options.static_mesh_import_data
    sm.combine_meshes = True
    sm.auto_generate_collision = True
    sm.convert_scene = True
    sm.force_front_x_axis = False
    sm.import_uniform_scale = 1.0
    return options


def import_extras():
    models = ["train_locomotive", "train_carriage", "train_tail", "train_track"]
    for asset_id in models:
        tex_dir = os.path.join(ROOT, "Models", asset_id, "textures")
        if os.path.isdir(tex_dir):
            for name in os.listdir(tex_dir):
                lower = name.lower()
                if lower.endswith((".jpg", ".jpeg", ".png", ".exr", ".tga")):
                    import_file(os.path.join(tex_dir, name), "/Game/Art/Textures")
        fbx = os.path.join(ROOT, "Models", asset_id, asset_id + ".fbx")
        try:
            import_file(fbx, "/Game/Art/Meshes", fbx_options())
        except Exception as ex:
            log("FBX options failed, fallback: " + str(ex))
            import_file(fbx, "/Game/Art/Meshes", None)

    for name in ("SM_RomanRoof.obj", "SM_RomanPediment.obj", "SM_RomanStairChunk.obj"):
        import_file(os.path.join(ROOT, "Kit", name), "/Game/Art/Kit")


def apply_marble():
    mat = unreal.load_asset("/Game/Art/Materials/M_RomanMarble")
    if not mat:
        log("no marble material")
        return
    for name in (
        "SM_RomanRoof",
        "SM_RomanPediment",
        "SM_RomanStairChunk",
    ):
        mesh = unreal.load_asset("/Game/Art/Kit/" + name)
        if not mesh:
            log("missing kit " + name)
            continue
        try:
            unreal.EditorStaticMeshLibrary.set_static_mesh_material(
                mesh, 0, "/Game/Art/Materials/M_RomanMarble.M_RomanMarble"
            )
        except Exception as ex:
            log("set mat " + name + ": " + str(ex))
            try:
                mesh.set_material(0, mat)
            except Exception:
                pass
        try:
            unreal.EditorAssetLibrary.save_loaded_asset(mesh)
        except Exception:
            pass
        bounds = mesh.get_bounds()
        log("KIT " + name + " extent=" + str(bounds.box_extent))


def log_mesh(path):
    mesh = unreal.load_asset(path)
    if not mesh:
        log("missing mesh " + path)
        return
    bounds = mesh.get_bounds()
    log("PROP " + path + " extent=" + str(bounds.box_extent) + " origin=" + str(bounds.origin))


def rebuild_hub():
    world = unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
    log("world=" + str(world))
    count = unreal.CBWorldStatics.rebuild_hub(world)
    log("cpp rebuild actors=" + str(count))
    saved = unreal.EditorLoadingAndSavingUtils.save_map(world, "/Game/Maps/Hub")
    log("saved=" + str(saved))


def main():
    open(LOG, "w", encoding="utf-8").write("start\n")
    try:
        import_extras()
        apply_marble()
        for p in (
            "/Game/Art/Meshes/train_locomotive",
            "/Game/Art/Meshes/train_carriage",
            "/Game/Art/Meshes/train_tail",
            "/Game/Art/Meshes/train_track",
            "/Game/Art/Kit/SM_RomanRoof",
            "/Game/Art/Kit/SM_RomanStairChunk",
        ):
            log_mesh(p)
        save_all()
        rebuild_hub()
    except Exception as ex:
        log("FATAL " + str(ex))
        log(traceback.format_exc())


if __name__ == "__main__":
    main()
