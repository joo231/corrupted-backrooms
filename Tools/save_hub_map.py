import traceback

import unreal

LOG = r"C:\Users\dio\projects\CorruptedBackrooms\Tools\hub_save_log.txt"


def log(msg):
    unreal.log(msg)
    with open(LOG, "a", encoding="utf-8") as f:
        f.write(msg + "\n")


def main():
    open(LOG, "w", encoding="utf-8").write("start\n")
    try:
        world = unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
        log("world=" + str(world))
        count = unreal.CBWorldStatics.rebuild_hub(world)
        log("cpp rebuild actors=" + str(count))
        saved = unreal.EditorLoadingAndSavingUtils.save_map(world, "/Game/Maps/Hub")
        log("saved=" + str(saved))
    except Exception as ex:
        log("FATAL " + str(ex))
        log(traceback.format_exc())


if __name__ == "__main__":
    main()
