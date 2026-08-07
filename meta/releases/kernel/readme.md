# KaworuOS

I recommend downloading the `full-release` as it already contains an efi
firmware for qemu which you will need in order to run the kernel under qemu.

And ofcourse will also need qemu (aarch64) installed in order to emulate
the kernel.

Or you can manually download efi firmware for qemu from the releases page of:
https://github.com/osdev0/edk2-ovmf-stable-bins

Then set `EFI_FIRMWARE` in `run` script, pointing to the firmware, and start
the script.
