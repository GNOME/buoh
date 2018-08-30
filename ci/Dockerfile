FROM nixos/nix
ARG EXPRESSION_HASH=unknown
LABEL ExpressionHash=$EXPRESSION_HASH
COPY default.nix /project/
RUN cd /project && nix-shell --run ':'
