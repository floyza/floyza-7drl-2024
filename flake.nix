{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-24.05";
    nixpkgs-unstable.url = "github:nixos/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };
  outputs =
    {
      self,
      nixpkgs,
      nixpkgs-unstable,
      flake-utils,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        unstable-pkgs = nixpkgs-unstable.legacyPackages.${system};
      in
      {
        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            SDL2
            boost
            unstable-pkgs.vcpkg
            gnumake
            cmake
            pkg-config

            ninja
            zlib
            utf8proc
            zstd
            lz4
            autoconf
            automake
            libtool
            libxcrypt
            util-linux
            xz
            libcap
            (python3.withPackages (p: with p; [ jinja2 ]))
            systemd
            expat
            dbus

            pulseaudio
            xorg.libXext
          ];
          LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath (
            with pkgs;
            [
              xorg.libX11
              xorg.libXext
              libGL
            ]
          );
          VCPKG_ROOT = "${unstable-pkgs.vcpkg}/share/vcpkg";
        };
      }
    );
}
