#!/bin/sh

# usage: build-image.sh <job-token> <ci-registry> <project-path> <image-tag>

alias jq="docker run -i stedolan/jq"
alias skopeo="docker run -i alexeiled/skopeo skopeo"

CI_JOB_TOKEN=$1
CI_REGISTRY=$2
CI_PROJECT_PATH=$3
IMAGE_TAG=$4

IMAGE="$CI_REGISTRY/$CI_PROJECT_PATH:$IMAGE_TAG"

NIX_EXPRESSION_HASH=$(sha256sum default.nix | cut -f 1 -d ' ')
IMAGE_EXPRESSION_HASH=$(skopeo inspect "docker://$IMAGE" | jq '.Labels.ExpressionHash')

echo $IMAGE
jq --version
sha256sum --version

if test "$NIX_EXPRESSION_HASH" = "$IMAGE_EXPRESSION_HASH"; then
  echo 'Image already up to date, skipping build…'
else
  echo 'Building Docker image…'
  docker login -u gitlab-ci-token -p "$CI_JOB_TOKEN" "$CI_REGISTRY"
  docker build -t "$IMAGE_TAG" . -f ci/Dockerfile --build-arg "EXPRESSION_HASH=$NIX_EXPRESSION_HASH"
  docker push "$IMAGE_TAG"
fi
