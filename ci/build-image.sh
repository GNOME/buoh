#!/bin/sh

# docker manifest is an experimental feature
echo 'Adding “experimental” flag to Docker configuration'
# We assume the config does not exist because we lack
# programs that would allow us to update the file safely (jq).
mkdir -p ~/.docker
echo '{"experimental": "enabled"}' > ~/.docker/config.json

# Checking if the image exists; it will need to be manually
# removed from registry to be re-built.
if docker manifest inspect "$IMAGE_TAG" > /dev/null; then
  echo 'Image already exists, skipping…'
else
  echo 'Building Docker image…'
  docker login -u gitlab-ci-token -p "$CI_JOB_TOKEN" "$CI_REGISTRY"
  docker build -t "$IMAGE_TAG" . -f ci/Dockerfile
  docker push "$IMAGE_TAG"
fi
