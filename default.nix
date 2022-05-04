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

  gitignore_nix = import sources."gitignore.nix" {
    inherit lib;
  };
  inherit (gitignore_nix) gitignoreFilter;

in (if shell then pkgs.mkShell else pkgs.stdenv.mkDerivation) rec {
  name = "buoh";

  src =
    let
      sourcePath = ./.;
    in
    if shell then null else builtins.filterSource (gitignoreFilter sourcePath) sourcePath;

  # Dependencies for build platform
  nativeBuildInputs = with pkgs; [
    meson
    ninja
    pkg-config
    gettext
    gi-docgen
    gobject-introspection
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
    gobject-introspection # for libgirepository
    libsoup
    libxml2
  ];

  checkInputs = with pkgs; [
    desktop-file-utils
    appstream-glib
  ];

  mesonFlags = [
    "-Dintrospection=enabled"
    "-Dapi_docs=enabled"
  ];

  inherit doCheck;

  preBuild = ''
    # Generating introspection needed for building docs runs BuohApplication,
    # which tries to create config file in XDG_CONFIG_DIR,
    # so we need to point HOME to an existing directory.
    export HOME="$TMPDIR"
  '';

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
