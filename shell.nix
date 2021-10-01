{ pkgs ? null
, doCheck ? true
}:
import ./default.nix ({
  inherit doCheck;
  shell = true;
} // (if pkgs != null then { inherit pkgs; } else {}))
