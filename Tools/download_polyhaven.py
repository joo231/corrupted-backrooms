import json
import math
import os
import urllib.request

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
RAW = os.path.join(ROOT, "RawArt")


def api(url):
    with urllib.request.urlopen(url) as r:
        return json.loads(r.read().decode("utf-8"))


def download(url, dest):
    os.makedirs(os.path.dirname(dest), exist_ok=True)
    if os.path.isfile(dest) and os.path.getsize(dest) > 1024:
        print("skip", dest)
        return
    print("GET", url)
    urllib.request.urlretrieve(url, dest)
    print(" saved", dest, os.path.getsize(dest))


def download_fbx_model(asset_id, res="2k"):
    files = api("https://api.polyhaven.com/files/%s" % asset_id)
    fbx = files["fbx"][res]["fbx"]
    folder = os.path.join(RAW, "Models", asset_id)
    download(fbx["url"], os.path.join(folder, asset_id + ".fbx"))
    for rel, meta in fbx.get("include", {}).items():
        download(meta["url"], os.path.join(folder, rel.replace("/", os.sep)))


def download_texture(asset_id, res="2k"):
    files = api("https://api.polyhaven.com/files/%s" % asset_id)
    folder = os.path.join(RAW, "Textures", asset_id)
    mapping = {
        "Diffuse": "diff.jpg",
        "nor_gl": "nor.jpg",
        "Rough": "rough.jpg",
        "AO": "ao.jpg",
        "Displacement": "disp.jpg",
    }
    for key, filename in mapping.items():
        if key not in files:
            continue
        node = files[key].get(res, {})
        fmt = node.get("jpg") or node.get("png")
        if not fmt:
            continue
        download(fmt["url"], os.path.join(folder, asset_id + "_" + filename))


def download_hdri(asset_id, res="2k"):
    files = api("https://api.polyhaven.com/files/%s" % asset_id)
    hdr = files.get("hdri", {}).get(res, {}).get("hdr")
    if hdr:
        download(hdr["url"], os.path.join(RAW, "HDRI", asset_id + "_" + res + ".hdr"))


def main():
    models = [
        "marble_bust_01",
        "horse_statue_01",
        "lion_head",
        "antique_ceramic_vase_01",
        "ceramic_vase_02",
    ]
    for m in models:
        download_fbx_model(m)
    download_texture("marble_01")
    download_texture("marble_tiles")
    download_hdri("kloofendal_38d_partly_cloudy")
    print("DONE")


if __name__ == "__main__":
    main()
