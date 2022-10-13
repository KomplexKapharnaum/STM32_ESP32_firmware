# https://docs.platformio.org/en/latest/plus/debug-tools/custom.html?highlight=j-link#j-link-as-debugger-and-uploader

from os import makedirs
from os.path import isdir, join
Import('env')

def _jlink_cmd_script(env, source):
    build_dir = env.subst("$BUILD_DIR")
    if not isdir(build_dir):
        makedirs(build_dir)
    script_path = join(build_dir, "upload.jlink")
#     commands = ["h", "loadbin %s, 0x0" % source, "r", "q"]
    commands = ["h", "loadbin %s/firmware.bin, 0x0" % build_dir, "r", "q"]
    with open(script_path, "w") as fp:
        fp.write("\n".join(commands))
    return script_path

env.Replace(
    __jlink_cmd_script=_jlink_cmd_script,
    # UPLOADER="C:/Program Files (x86)/SEGGER/JLink_V640/JLink.exe",
    UPLOADERFLAGS=[
        "-device", "STM32F070F6",
        "-speed", "4000",
        "-if", "swd",
        "-autoconnect", "1"
    ],
    UPLOADCMD='"$UPLOADER" $UPLOADERFLAGS -CommanderScript ${__jlink_cmd_script(__env__, SOURCE)}'
)