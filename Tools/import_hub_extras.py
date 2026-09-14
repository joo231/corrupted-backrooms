import os
import unreal

ROOT = r"C:\Users\dio\projects\CorruptedBackrooms\RawArt"


def save_all():
    try:
        unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
        unreal.log("Saved dirty packages")
    except Exception as ex:
        unreal.log_warning("save dirty: " + str(ex))


def import_file(src, dest, options=None):
    if not os.path.isfile(src):
        unreal.log_warning("Missing " + src)
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
    unreal.log("Imported " + src + " -> " + dest + " " + str(imported))
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


def main():
    models = [
        "train_locomotive",
        "train_carriage",
        "train_tail",
        "train_track",
    ]
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
            unreal.log_warning("FBX options failed, fallback: " + str(ex))
            import_file(fbx, "/Game/Art/Meshes", None)

    kit_names = ["SM_RomanRoof.obj", "SM_RomanPediment.obj", "SM_RomanStairChunk.obj"]
    for name in kit_names:
        import_file(os.path.join(ROOT, "Kit", name), "/Game/Art/Kit")

    save_all()
    unreal.log("hub extras import finished")


if __name__ == "__main__":
    main()
