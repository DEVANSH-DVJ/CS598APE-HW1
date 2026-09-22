#!/bin/bash
# Opens a shell in the container with this repository at /host.
# The image is built from docker/Dockerfile on first use (the Docker Hub image is arm64-only).
docker image inspect 598ape-hw1 > /dev/null 2>&1 || docker build -t 598ape-hw1 docker || exit 1
docker run -it --rm --user "$(id -u):$(id -g)" --security-opt seccomp=unconfined -v "$(pwd):/host" -w /host 598ape-hw1 /bin/bash
