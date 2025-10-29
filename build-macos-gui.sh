#!/bin/bash

# Dream DRM Receiver - macOS Qt GUI Build Script
# For macOS ARM (Apple Silicon)

set -e

echo "=== Dream DRM Receiver - macOS GUI Build ==="
echo ""

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check Qt5
QT5_PREFIX="/opt/homebrew/opt/qt@5"
if [ ! -d "$QT5_PREFIX" ]; then
    echo -e "${RED}❌ Qt5 not found!${NC}"
    echo "Install with: brew install qt@5"
    exit 1
fi
echo -e "${GREEN}✅ Qt5 found at $QT5_PREFIX${NC}"

# Check dependencies
echo ""
echo "Checking dependencies..."

check_dep() {
    if brew list $1 &>/dev/null; then
        echo -e "${GREEN}✅ $1${NC}"
        return 0
    else
        echo -e "${YELLOW}⚠️  $1 not installed${NC}"
        return 1
    fi
}

MISSING_DEPS=()

check_dep "fftw" || MISSING_DEPS+=("fftw")
check_dep "hamlib" || MISSING_DEPS+=("hamlib")
check_dep "libpcap" || MISSING_DEPS+=("libpcap")
check_dep "opus" || MISSING_DEPS+=("opus")
check_dep "speex" || MISSING_DEPS+=("speex")
check_dep "qwt-qt5" || MISSING_DEPS+=("qwt-qt5")

# Check FDK-AAC
if [ -f "/opt/homebrew/lib/libfdk-aac.dylib" ] || [ -f "/opt/homebrew/lib/libfdk-aac.a" ]; then
    echo -e "${GREEN}✅ fdk-aac${NC}"
else
    echo -e "${YELLOW}⚠️  fdk-aac not installed${NC}"
    MISSING_DEPS+=("fdk-aac-encoder")
fi

if [ ${#MISSING_DEPS[@]} -gt 0 ]; then
    echo ""
    echo -e "${YELLOW}Missing dependencies. Install with:${NC}"
    echo "brew install ${MISSING_DEPS[@]}"
    echo ""
    read -p "Install now? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        brew install ${MISSING_DEPS[@]}
    else
        exit 1
    fi
fi

# Setup environment
echo ""
echo "Setting up build environment..."
export PATH="$QT5_PREFIX/bin:$PATH"
export PKG_CONFIG_PATH="$QT5_PREFIX/lib/pkgconfig:$PKG_CONFIG_PATH"
export LDFLAGS="-L/opt/homebrew/lib"
export CPPFLAGS="-I/opt/homebrew/include"

# Clean previous build
echo ""
echo "Cleaning previous build..."
make clean 2>/dev/null || true
rm -f Makefile

# Generate Makefile
echo ""
echo "Generating Makefile with qmake..."
$QT5_PREFIX/bin/qmake dream.pro CONFIG+=fdk-aac CONFIG+=speexdsp

if [ $? -ne 0 ]; then
    echo -e "${RED}❌ qmake failed!${NC}"
    exit 1
fi

# Build
echo ""
echo "Building Dream GUI..."
NCPU=$(sysctl -n hw.ncpu)
echo "Using $NCPU CPU cores..."

make -j$NCPU

if [ $? -ne 0 ]; then
    echo -e "${RED}❌ Build failed!${NC}"
    exit 1
fi

echo ""
echo -e "${GREEN}✅ Build successful!${NC}"
echo ""

# Check output
if [ -d "dream.app" ]; then
    echo -e "${GREEN}GUI Application: dream.app${NC}"
    echo ""
    echo "Run with: open dream.app"
elif [ -f "dream" ]; then
    echo -e "${GREEN}Binary: ./dream${NC}"
    echo ""
    echo "Run with: ./dream"
fi

echo ""
echo "=== Build Complete ==="
