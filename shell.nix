/*

Some developers do not want a pinned package set by default.
If you want to use the pinned nix-shell or a more sophisticated set of arguments:

    $ nix-shell default.nix --arg shell true

*/

{ pkgs ? import <nixpkgs> {}
, doCheck ? true
}:
import ./default.nix {
  inherit pkgs doCheck;
  shell = true;
}
