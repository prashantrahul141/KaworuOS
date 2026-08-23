#!/usr/bin/env python3

import sys


def parse_value(value):
    value = value.strip()
    if value == "y":
        return "true"

    if value == "n":
        return "false"

    if value.startswith('"') and value.endswith('"'):
        return value

    try:
        int(value, 0)
        return value
    except ValueError:
        pass

    return '"' + value.replace('"', '\\"') + '"'


def main():
    if len(sys.argv) != 2:
        print("usage: kconfig-to-meson.py .config")
        return -1

    config_file = sys.argv[1]
    try:
        with open(config_file, "r", encoding="utf-8") as f:
            for line in f.readlines():
                line = line.strip()
                if not line.startswith("CONFIG_") or "=" not in line:
                    continue
                name, value = line.split("=", 1)
                print(f"{name} = {parse_value(value)}")
    except FileNotFoundError:
        print(
            "Kconfig configuration file not found: "
            + config_file
            + "\nplease run `make defconfig` or `make debugconfig` first"
        )
        sys.exit(-1)

    return 0


if __name__ == "__main__":
    sys.exit(main())
