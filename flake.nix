rec {
  outputs = {
    self,
    nixpkgs,
    ...
  }: let
    system = "x86_64-linux";
    pkgs = import nixpkgs {inherit system;};
    pname = "obolc";

    buildInputs = with pkgs;
    with pkgs.qt6; [
      full
      qtbase
      qttools
      qtdeclarative
      qtwayland
      wayland
      wayland-protocols
      kdePackages.layer-shell-qt
      stdenv.cc.cc.lib
      llvmPackages.libcxx
    ];

    nativeBuildInputs = with pkgs; [
      cmake
      clang
      pkg-config
      qt6.wrapQtAppsHook
      patchelf
    ];

    devTools = with pkgs; [
      clang-tools
      ccls
      rr
      inotify-tools
      gdb
      gf
      valgrind
      include-what-you-use
      rr
      cppcheck
    ];
  in {
    devShells.${system}.default = pkgs.mkShell {
      packages = nativeBuildInputs ++ buildInputs ++ devTools;
      shellHook = ''
        export LD_LIBRARY_PATH=${pkgs.lib.makeLibraryPath buildInputs}:$LD_LIBRARY_PATH
        export INCLUDE_PATH=${pkgs.lib.makeIncludePath buildInputs}:$INCLUDE_PATH
        echo "🚀 obolc devshell ready!"
      '';
    };

    packages.${system}.default = pkgs.stdenv.mkDerivation {
      inherit pname buildInputs nativeBuildInputs;
      version = "0.1";
      src = ./.;

      buildPhase = ''
        LD_LIBRARY_PATH=${pkgs.lib.makeLibraryPath buildInputs}:$LD_LIBRARY_PATH
        INCLUDE_PATH=${pkgs.lib.makeIncludePath buildInputs}:$INCLUDE_PATH
        cmake --build .
      '';

      installPhase = ''
        mkdir -p $out/bin
        cp ${pname} $out/bin/${pname}
      '';

      postBuild = ''
        patchelf --set-rpath ${pkgs.lib.makeLibraryPath buildInputs} $out/bin/${pname}
      '';

      meta = with pkgs.lib; {
        inherit description;
        platforms = platforms.linux;
      };
    };

    apps.${system}.default = {
      type = "app";
      program = "${self.packages.${system}.default}/bin/${pname}";
    };

    defaultPackage.${system} = self.packages.${system}.default;
    defaultApp.${system} = self.apps.${system}.default;
  };

  description = "obolc - Qt6 system panel";

  nixConfig = {
    experimental-features = ["nix-command" "flakes" "pipe-operators"];
    accept-flake-config = true;
    show-trace = true;

    extra-substituters = [
      "https://nix-community.cachix.org"
      "https://charon.cachix.org"
    ];
    extra-trusted-public-keys = [
      "nix-community.cachix.org-1:mB9FSh9qf2dCimDSUo8Zy7bkq5CX+/rkCWyvRCYg3Fs="
      "charon.cachix.org-1:epdetEs1ll8oi8DT8OG2jEA4whj3FDbqgPFvapEPbY8="
    ];
  };

  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
}
