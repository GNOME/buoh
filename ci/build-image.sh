#!/bin/sh

jq --version

if test -f ~/.docker/config.json; then
  echo 'Adding “experimental” flag to an existing docker configuration'
  jq '.experimental = true' ~/.docker/config.json > ~/.docker/config.json.tmp
  mv ~/.docker/config.json.tmp ~/.docker/config.json
else
  mkdir -p ~/.docker
  echo 'Adding “experimental” flag to a new docker configuration'
  echo '{"experimental": "enabled"}' > ~/.docker/config.json
fi

if docker manifest inspect $IMAGE_TAG > /dev/null; then
  echo 'Image already exists, skipping…'
then
  docker login -u gitlab-ci-token -p $CI_JOB_TOKEN $CI_REGISTRY
  docker build -t $IMAGE_TAG . -f ci/Dockerfile
  docker push $IMAGE_TAG
fi
