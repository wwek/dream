#!/bin/bash

# Enhanced DRM build script for KiwiSDR implementation
# This script builds the dream binary using a standalone Makefile
set -euxo pipefail

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "Building Enhanced DRM Receiver from $SCRIPT_DIR..."

# Check dependencies
echo "Checking dependencies..."
for dep in make gcc g++; do
    if ! command -v $dep &> /dev/null; then
        echo "Error: $dep is not installed"
        exit 1
    fi
done

# Check if dream source directory exists
if [ ! -d "$SCRIPT_DIR/dream" ]; then
    echo "Error: dream subdirectory not found in $SCRIPT_DIR"
    exit 1
fi

# Use standalone Makefile for independent build
cd "$SCRIPT_DIR"

if [ -f "Makefile.standalone" ]; then
    echo "Building with standalone Makefile..."
    make -f Makefile.standalone clean || true
    make -f Makefile.standalone -j4

    # Install the binary (renamed from dream to drm)
    if [ -f "drm" ]; then
        echo "Installing drm to /usr/local/bin..."
        make -f Makefile.standalone install
        echo "Build completed successfully!"
        echo "drm is now available at /usr/local/bin/drm"
    else
        echo "Error: drm binary not found after build"
        ls -la
        exit 1
    fi
else
    echo "Error: Makefile.standalone not found in $SCRIPT_DIR"
    exit 1
fi