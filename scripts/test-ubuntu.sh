#!/usr/bin/env bash
# Run the FindCHOLMOD.cmake example inside an Ubuntu container using
# Apple's `container` CLI (or any drop-in: docker / podman).
#
# Usage:
#   scripts/test-ubuntu.sh 22.04
#   scripts/test-ubuntu.sh 24.04
#
# The host's working tree is mounted at /work; libsuitesparse-dev is
# installed via apt so we exercise the exact path Ubuntu users hit.

set -euo pipefail

UBUNTU_VERSION="${1:-22.04}"
IMAGE="ubuntu:${UBUNTU_VERSION}"

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

# Pick the container runtime: prefer Apple `container`, fall back to docker/podman.
if command -v container >/dev/null 2>&1; then
    RUNTIME=container
elif command -v docker >/dev/null 2>&1; then
    RUNTIME=docker
elif command -v podman >/dev/null 2>&1; then
    RUNTIME=podman
else
    echo "No container runtime found (need 'container', 'docker', or 'podman')." >&2
    exit 1
fi

echo "==> Using runtime: ${RUNTIME}"
echo "==> Image: ${IMAGE}"
echo "==> Repo: ${REPO_ROOT}"

"${RUNTIME}" run --rm \
    --volume "${REPO_ROOT}:/work" \
    --workdir /work \
    "${IMAGE}" \
    bash -euxc '
        export DEBIAN_FRONTEND=noninteractive
        apt-get update -qq
        apt-get install -y --no-install-recommends \
            build-essential cmake ninja-build pkg-config \
            libsuitesparse-dev libblas-dev liblapack-dev \
            ca-certificates >/dev/null

        rm -rf build-ubuntu
        cmake -S example -B build-ubuntu -G Ninja
        cmake --build build-ubuntu
        ctest --test-dir build-ubuntu --output-on-failure
    '
