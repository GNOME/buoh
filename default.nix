/*

How to use?
***********

If you have Nix installed, you can get an environment with everything
needed to compile buoh by running:

    $ nix-shell

at the root of the buoh repository.

You can also compile buoh and ‘install’ it by running:

    $ nix-build

at the root of the buoh repository. The command will install
buoh to a location under Nix store and create a ‘result’ symlink
in the current directory pointing to the in-store location.

How to tweak default arguments?
*******************************

Nix supports the ‘--arg’ option (see nix-build(1)) that allows you
to override the top-level arguments.

For instance, to use your local copy of nixpkgs:

    $ nix-build --arg pkgs "import $HOME/Projects/nixpkgs {}"

Or to speed up the build by not running the test suite:

    $ nix-build --arg doCheck false

*/

{ sources ? import ./nix/sources.nix
, pkgs ?
    import sources.nixpkgs {
      overlays = [];
      config = {};
    }
, doCheck ? true
, shell ? false
  # We do not use lib.inNixShell because that would also apply
  # when in a nix-shell of some package depending on this one.
}:

let
  inherit (pkgs) lib;
in (if shell then pkgs.mkShell else pkgs.stdenv.mkDerivation) rec {
  name = "buoh";

  src =
    let
      # Do not copy to the store:
      # - build directory, since Meson will want to use it
      # - .git directory, since it would unnecessarily bloat the source
      cleanSource = path: _: !lib.elem (builtins.baseNameOf path) [ "build" ".git" ];
    in
      if shell then null else builtins.filterSource cleanSource ./.;

  # Dependencies for build platform
  nativeBuildInputs = with pkgs; [
    meson
    ninja
    pkg-config
    gettext
    python3
    xvfb-run
    libxslt
    wrapGAppsHook
  ] ++ lib.optionals shell [
    niv
  ];

  # Dependencies for host platform
  buildInputs = with pkgs; [
    glib
    gtk3
    glib-networking # For TLS
    libsoup
    libxml2
  ];

  checkInputs = with pkgs; [
    desktop-file-utils
    appstream-glib
  ];

  inherit doCheck;

  # Hardening does not work in debug mode
  hardeningDisable = lib.optionals shell [ "all" ];

  checkPhase = ''
    runHook preCheck

    export NO_AT_BRIDGE=1
    xvfb-run -s '-screen 0 800x600x24' \
      meson test --print-errorlogs

    runHook postCheck
  '';
}
