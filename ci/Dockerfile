FROM nixos/nix

COPY default.nix /project/

RUN cd /project && nix-shell --run ':'
