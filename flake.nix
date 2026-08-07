{
  description = "KaworuOS development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-26.05";
  };

  outputs =
    { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
      ovmf = import ./nix/edk2-ovmf-stable-bins.nix { inherit pkgs; };
    in
    {
      devShells.${system}.default = pkgs.mkShell {
        packages = with pkgs; [
          glibc_multi

          llvmPackages.clang-unwrapped
          llvmPackages.lld
          llvm

          xorriso
          ovmf

          qemu
          gdb
          dtc

          gnumake
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
    };
}
