#!/usr/bin/env python3
"""
regenerates .config and config.h using kconfiglib and reconfigures meson if needed

"""

import filecmp
import os
import shutil
import subprocess
import sys
from pathlib import Path

import kconfiglib


USAGE = """
usage: configure.py <src> <build> <cross-file> <defconfig|menuconfig> [defconfig-file]
"""


def main():
    if len(sys.argv) < 5:
        print(USAGE)
        return 1

    src = Path(sys.argv[1])
    build = Path(sys.argv[2])
    cross = sys.argv[3]
    mode = sys.argv[4]
    cfg = sys.argv[5] if len(sys.argv) > 5 else None

    os.chdir(src)
    build.mkdir(parents=True, exist_ok=True)

    kconf = kconfiglib.Kconfig("Kconfig", suppress_traceback=True)

    if mode == "menuconfig":
        import menuconfig

        menuconfig.menuconfig(kconf)
    else:
        kconf.load_config(cfg)
        kconf.write_config()

    kconf.write_autoconf("config.h")

    prev = build / ".prev.config.h"
    config_h = Path("config.h")

    if not prev.exists() or not filecmp.cmp(config_h, prev, shallow=False):
        shutil.copy2(config_h, prev)
        setup = subprocess.run(
            ["meson", "setup", str(build), "--reconfigure", "--cross-file", cross],
            check=True,
        )
        if setup.returncode != 0:
            subprocess.run(
                ["meson", "setup", str(build), "--cross-file", cross], check=True
            )
    else:
        shutil.copy2(prev, config_h)

    return 0


if __name__ == "__main__":
    sys.exit(main())
