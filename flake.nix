{
  description = "KaworuOS development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-26.05";
  };

  outputs =
    { self, nixpkgs }:
    let
      systems = [
        "x86_64-linux"
        "aarch64-linux"
        "aarch64-darwin"
      ];

      forEachSystem =
        f:
        nixpkgs.lib.genAttrs systems (
          system:
          f {
            pkgs = import nixpkgs { inherit system; };
          }
        );

    in
    {
      devShells = forEachSystem (
        { pkgs }:
        let
          ovmf = import ./nix/edk2-ovmf-stable-bins.nix { inherit pkgs; };
        in
        {
          default = pkgs.mkShell {
            packages =
              with pkgs;
              (lib.optionals stdenv.isLinux [
                (if stdenv.hostPlatform.isx86_64 then glibc_multi else glibc)
              ])
              ++ [
                llvmPackages.clang-unwrapped
                llvmPackages.lld
                llvm
                xorriso
                ovmf
                qemu
                gdb
                dtc
                gnumake
                ninja
                meson
                bear
                python313Packages.kconfiglib
              ];

            shellHook = ''
              export UEFI_FIRMWARE=${ovmf}/share/ovmf-code-aarch64.fd
              export CC=clang
              export LD=ld.lld
              export OBJDUMP=llvm-objdump
              export READELF=llvm-readelf
            '';
          };
        }
      );
    };
}
