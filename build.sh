#!/bin/bash
set -e

echo "========================================"
echo "Building Dream DRM Receiver 2.2.x"
echo "with xHE-AAC support"
echo "========================================"

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# Install dependencies for Ubuntu/Debian
echo "Installing dependencies..."
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    qt5-qmake qtbase5-dev \
    libfftw3-dev \
    libspeexdsp-dev \
    libspeex-dev \
    libpulse-dev \
    libsamplerate0-dev \
    libfaad-dev \
    libopus-dev \
    libhamlib-dev \
    libpcap-dev \
    libpcap0.8 \
    wget

# Build and install FDK-AAC 2.0.2
echo ""
echo "Building FDK-AAC 2.0.2..."
echo "=========================="
#https://github.com/rafael2k/fdk-aac

cd /tmp
wget https://downloads.sourceforge.net/project/opencore-amr/fdk-aac/fdk-aac-2.0.2.tar.gz
tar -zxvf fdk-aac-2.0.2.tar.gz
cd fdk-aac-2.0.2

./configure
make -j$(nproc)
sudo make install
sudo ldconfig

cd "$SCRIPT_DIR"

# Clean previous build
echo ""
echo "Cleaning previous build..."
make distclean 2>/dev/null || true
rm -rf obj/ moc/ ui/ qrc/ 2>/dev/null || true

# Build Dream
echo ""
echo "Building Dream..."
qmake CONFIG+=console CONFIG+=fdk-aac dream.pro
make -j$(nproc)

# Copy binary
cp dream dream-2.

echo ""
echo "✓ Build Complete!"
echo "Binary: $SCRIPT_DIR/dream"
echo ""
echo "Usage:"
echo "  ./dream --help"
echo "  ./dream -i input.iq -o output.wav"