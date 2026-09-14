import unreal


def fix_normal(path):
    tex = unreal.load_asset(path)
    if not tex:
        unreal.log_warning("missing " + path)
        return
    tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_NORMALMAP)
    tex.set_editor_property("srgb", False)
    try:
        unreal.EditorAssetLibrary.save_loaded_asset(tex)
    except Exception:
        pass
    unreal.log("normalmap " + path)


def apply_marble_to_kit():
    mat = unreal.load_asset("/Game/Art/Materials/M_RomanMarble")
    if not mat:
        unreal.log_warning("no marble material")
        return
    names = [
        "SM_RomanFloor",
        "SM_RomanColumn",
        "SM_RomanEntablature",
        "SM_RomanPedestal",
        "SM_RomanBaluster",
        "SM_RomanStep",
        "SM_RomanRoof",
        "SM_RomanPediment",
        "SM_RomanStairChunk",
    ]
    for name in names:
        mesh = unreal.load_asset("/Game/Art/Kit/" + name)
        if not mesh:
            continue
        try:
            unreal.EditorStaticMeshLibrary.set_static_mesh_material(mesh, 0, "/Game/Art/Materials/M_RomanMarble.M_RomanMarble")
        except Exception as ex:
            unreal.log_warning("set mat " + name + ": " + str(ex))
            try:
                mesh.set_material(0, mat)
            except Exception:
                pass
        try:
            unreal.EditorAssetLibrary.save_loaded_asset(mesh)
        except Exception:
            pass
        bounds = mesh.get_bounds()
        unreal.log("KIT " + name + " extent=" + str(bounds.box_extent))


def log_prop(path):
    mesh = unreal.load_asset(path)
    if not mesh:
        unreal.log_warning("missing mesh " + path)
        return
    bounds = mesh.get_bounds()
    unreal.log("PROP " + path + " extent=" + str(bounds.box_extent) + " origin=" + str(bounds.origin))


def main():
    fix_normal("/Game/Art/Textures/marble_tiles_nor")
    fix_normal("/Game/Art/Textures/marble_01_nor")
    apply_marble_to_kit()
    for p in [
        "/Game/Art/Meshes/marble_bust_01",
        "/Game/Art/Meshes/horse_statue_01",
        "/Game/Art/Meshes/lion_head",
        "/Game/Art/Meshes/horse_head",
        "/Game/Art/Meshes/bull_head",
        "/Game/Art/Meshes/brass_vase_01",
        "/Game/Art/Meshes/train_locomotive",
        "/Game/Art/Meshes/train_carriage",
        "/Game/Art/Meshes/train_tail",
        "/Game/Art/Meshes/train_track",
        "/Game/Art/Kit/SM_RomanRoof",
        "/Game/Art/Kit/SM_RomanStairChunk",
        "/Game/Art/Kit/SM_RomanFloor",
        "/Game/Art/Kit/SM_RomanColumn",
    ]:
        log_prop(p)
    unreal.log("art fix finished")


if __name__ == "__main__":
    main()
