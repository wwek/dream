#!/bin/bash

# Docker build script for DRM extension
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
IMAGE_NAME="openwebrx-drm"
IMAGE_TAG="${1:-latest}"

echo "Building Docker image: ${IMAGE_NAME}:${IMAGE_TAG}"
echo "Build context: $SCRIPT_DIR"

cd "$SCRIPT_DIR"

docker build \
    -t "${IMAGE_NAME}:${IMAGE_TAG}" \
    -f Dockerfile \
    .

echo ""
echo "Build complete!"
echo "Image: ${IMAGE_NAME}:${IMAGE_TAG}"
echo ""
echo "To run the dream binary:"
echo "  docker run --rm ${IMAGE_NAME}:${IMAGE_TAG}"
echo ""
echo "To test with input file:"
echo "  docker run --rm -v /path/to/audio:/data ${IMAGE_NAME}:${IMAGE_TAG} -c /data/input.wav"
