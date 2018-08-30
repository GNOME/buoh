#!/bin/sh

# usage: build-image.sh <job-token> <image-tag>

alias jq="docker run -i stedolan/jq"
alias skopeo="docker run -i alexeiled/skopeo skopeo"

CI_JOB_TOKEN=$1
IMAGE_TAG=$2

NIX_EXPRESSION_HASH=$(sha256sum default.nix | cut -f 1 -d ' ')
IMAGE_EXPRESSION_HASH=$(skopeo inspect "docker://$IMAGE_TAG" | jq '.Labels.ExpressionHash')

echo $NIX_EXPRESSION_HASH $IMAGE_EXPRESSION_HASH

if test "$NIX_EXPRESSION_HASH" = "$IMAGE_EXPRESSION_HASH"; then
  echo 'Image already up to date, skipping build…'
else
  echo 'Building Docker image…'
  docker login -u gitlab-ci-token -p "$CI_JOB_TOKEN" "$CI_REGISTRY"
  docker build -t "$IMAGE_TAG" . -f ci/Dockerfile --build-arg "EXPRESSION_HASH=$NIX_EXPRESSION_HASH"
  docker push "$IMAGE_TAG"
fi
