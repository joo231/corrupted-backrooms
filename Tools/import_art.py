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


def import_model_textures():
    models = os.path.join(ROOT, "Models")
    if not os.path.isdir(models):
        return
    for asset_id in os.listdir(models):
        tex_dir = os.path.join(models, asset_id, "textures")
        if not os.path.isdir(tex_dir):
            continue
        for name in os.listdir(tex_dir):
            lower = name.lower()
            if lower.endswith((".jpg", ".jpeg", ".png", ".exr", ".tga")):
                import_file(os.path.join(tex_dir, name), "/Game/Art/Textures")


def import_folder_fbx(folder, dest):
    if not os.path.isdir(folder):
        return
    for name in os.listdir(folder):
        model_dir = os.path.join(folder, name)
        if not os.path.isdir(model_dir):
            continue
        fbx = os.path.join(model_dir, name + ".fbx")
        if os.path.isfile(fbx):
            try:
                import_file(fbx, dest, fbx_options())
            except Exception as ex:
                unreal.log_warning("FBX options failed, fallback: " + str(ex))
                import_file(fbx, dest, None)


def import_pbr_textures():
    tex_root = os.path.join(ROOT, "Textures")
    if not os.path.isdir(tex_root):
        return
    for asset_id in os.listdir(tex_root):
        folder = os.path.join(tex_root, asset_id)
        if not os.path.isdir(folder):
            continue
        for name in os.listdir(folder):
            import_file(os.path.join(folder, name), "/Game/Art/Textures")


def import_kit():
    kit = os.path.join(ROOT, "Kit")
    if not os.path.isdir(kit):
        return
    for name in os.listdir(kit):
        if name.lower().endswith(".obj"):
            import_file(os.path.join(kit, name), "/Game/Art/Kit")


def load_tex(*paths):
    for path in paths:
        asset = unreal.load_asset(path)
        if asset:
            return asset
    return None


def mark_normal(tex):
    if not tex:
        return
    tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_NORMALMAP)
    tex.set_editor_property("srgb", False)


def mark_linear(tex):
    if not tex:
        return
    tex.set_editor_property("srgb", False)


def create_pbr_material(name, diff, nor=None, rough=None, metal=None):
    dest = "/Game/Art/Materials"
    existing = unreal.load_asset(dest + "/" + name)
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    if existing:
        mat = existing
    else:
        mat = asset_tools.create_asset(name, dest, unreal.Material, unreal.MaterialFactoryNew())
    if mat is None:
        unreal.log_warning("Could not create " + name)
        return None

    mel = unreal.MaterialEditingLibrary
    try:
        mel.delete_all_material_expressions(mat)
    except Exception:
        pass

    if diff:
        sample = mel.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -400, 0)
        sample.set_editor_property("texture", diff)
        mel.connect_material_property(sample, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
    if nor:
        mark_normal(nor)
        nsample = mel.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -400, 200)
        nsample.set_editor_property("texture", nor)
        nsample.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL)
        mel.connect_material_property(nsample, "RGB", unreal.MaterialProperty.MP_NORMAL)
    if rough:
        mark_linear(rough)
        rsample = mel.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -400, 400)
        rsample.set_editor_property("texture", rough)
        rsample.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR)
        mel.connect_material_property(rsample, "R", unreal.MaterialProperty.MP_ROUGHNESS)
    if metal:
        mark_linear(metal)
        msample = mel.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -400, 600)
        msample.set_editor_property("texture", metal)
        msample.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR)
        mel.connect_material_property(msample, "R", unreal.MaterialProperty.MP_METALLIC)
    spec = mel.create_material_expression(mat, unreal.MaterialExpressionConstant, -80, 520)
    spec.set_editor_property("r", 0.35 if metal else 0.42)
    mel.connect_material_property(spec, "", unreal.MaterialProperty.MP_SPECULAR)
    mel.recompile_material(mat)
    unreal.log("Material " + name)
    return mat


def create_prop_materials():
    props = [
        "marble_bust_01",
        "horse_statue_01",
        "lion_head",
        "horse_head",
        "bull_head",
        "antique_ceramic_vase_01",
        "ceramic_vase_02",
        "brass_vase_01",
    ]
    for asset_id in props:
        diff = load_tex("/Game/Art/Textures/" + asset_id + "_diff_2k")
        nor = load_tex(
            "/Game/Art/Textures/" + asset_id + "_nor_gl_2k",
            "/Game/Art/Textures/" + asset_id + "_nor_2k",
        )
        rough = load_tex(
            "/Game/Art/Textures/" + asset_id + "_rough_2k",
            "/Game/Art/Textures/" + asset_id + "_rough",
        )
        metal = load_tex("/Game/Art/Textures/" + asset_id + "_metal_2k")
        if not diff:
            unreal.log_warning("No diffuse for " + asset_id)
            continue
        create_pbr_material("M_" + asset_id, diff, nor, rough, metal)


def create_marble_material():
    diff = load_tex("/Game/Art/Textures/marble_tiles_diff", "/Game/Art/Textures/marble_01_diff")
    nor = load_tex("/Game/Art/Textures/marble_tiles_nor", "/Game/Art/Textures/marble_01_nor")
    rough = load_tex("/Game/Art/Textures/marble_tiles_rough", "/Game/Art/Textures/marble_01_rough")
    create_pbr_material("M_RomanMarble", diff, nor, rough, None)


def main():
    import_model_textures()
    import_pbr_textures()
    import_folder_fbx(os.path.join(ROOT, "Models"), "/Game/Art/Meshes")
    import_kit()
    create_marble_material()
    create_prop_materials()
    save_all()
    unreal.log("Roman art import finished")


if __name__ == "__main__":
    main()
