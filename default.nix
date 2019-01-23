# How to use?

# If you have Nix installed, you can get in an environment with everything
# needed to compile buoh by running:
# $ nix-shell
# at the root of the buoh repository.

# How to tweak default arguments?

# nix-shell supports the --arg option (see Nix doc) that allows you for
# instance to do this:
# $ nix-shell --arg doCheck false

# You can also compile buoh and "install" it by running:
# $ rm -rf build # (only needed if you have left-over compilation files)
# $ nix-build
# at the root of the buoh repository.
# nix-build also supports the --arg option, so you will be able to do:
# $ nix-build --arg doCheck false
# if you want to speed up things by not running the test-suite.
# Once the build is finished, you will find, in the current directory,
# a symlink to where buoh was installed.

{ pkgs ?
    (import (fetchTarball {
      url = "https://github.com/NixOS/nixpkgs/archive/1b3affcbaa8e31af0bfe1be280f91ac3e384d96f.tar.gz";
      sha256 = "0b7spivfa1fd07ch4plakp3j16pq9rjsg2ird3kz2sg86cwhhkcv";
    }) {})
, doCheck ? true
, shell ? false
  # We don't use lib.inNixShell because that would also apply
  # when in a nix-shell of some package depending on this one.
}:

with pkgs;
with stdenv.lib;

stdenv.mkDerivation rec {

  name = "buoh";

  nativeBuildInputs = [
    meson ninja pkgconfig gettext python3 xvfb_run libxslt wrapGAppsHook
  ];

  buildInputs = [
    glib gtk3 libsoup libxml2
  ];

  checkInputs = [
    desktop-file-utils
  ];

  src =
    if shell then null
    else
      with builtins; filterSource
        (path: _: !elem (baseNameOf path) [ ".git" "result" ]) ./.;

  checkPhase = ''
    export NO_AT_BRIDGE=1
    xvfb-run -s '-screen 0 800x600x24' \
      meson test --print-errorlogs
  '';

  # Hardening does not work in debug mode
  hardeningDisable = optionals shell [ "all" ];

  inherit doCheck;
}
