#!/bin/bash

# Dream DRM Receiver - macOS Qt 6 GUI Build Script
# For macOS ARM (Apple Silicon)

set -e

echo "=== Dream DRM Receiver - macOS Qt 6 GUI Build ==="
echo ""

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check Qt6
QT6_PREFIX="/opt/homebrew/opt/qt@6"
if [ ! -f "$QT6_PREFIX/bin/qmake" ]; then
    echo -e "${RED}❌ Qt6 not found!${NC}"
    echo "Install with: brew install qt"
    exit 1
fi
echo -e "${GREEN}✅ Qt6 found at $QT6_PREFIX${NC}"

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

# Check Qwt - check for any 6.x version
QWT_FOUND=false
if [ -f "/opt/homebrew/lib/qwt.framework/Headers/qwt.h" ]; then
    # Check if qwt in main lib directory is version 6.x
    QWT_VERSION=$(/usr/libexec/PlistBuddy -c "Print CFBundleShortVersionString" /opt/homebrew/lib/qwt.framework/Resources/Info.plist 2>/dev/null | cut -d. -f1)
    if [ "$QWT_VERSION" = "6" ]; then
        QWT_FOUND=true
    fi
fi

# If not found in main lib, check any 6.x version in Cellar
if [ "$QWT_FOUND" = false ]; then
    for qwt_dir in /opt/homebrew/Cellar/qwt-qt5/6.*; do
        if [ -f "$qwt_dir/lib/qwt.framework/Versions/6/Headers/qwt.h" ]; then
            QWT_FOUND=true
            break
        fi
    done
fi

if [ "$QWT_FOUND" = true ]; then
    echo -e "${GREEN}✅ Qwt found (any 6.x version)${NC}"
else
    echo -e "${YELLOW}⚠️  Qwt not installed${NC}"
    MISSING_DEPS+=("qwt-qt5")
fi

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
export PATH="$QT6_PREFIX/bin:$PATH"
export PKG_CONFIG_PATH="$QT6_PREFIX/lib/pkgconfig:$PKG_CONFIG_PATH"
export LDFLAGS="-L/opt/homebrew/lib"
export CPPFLAGS="-I/opt/homebrew/include"

# Clean previous build
echo ""
echo "Cleaning previous build..."
make clean 2>/dev/null || true
rm -f Makefile

# Generate Makefile
echo ""
echo "Generating Makefile with qmake6..."
$QT6_PREFIX/bin/qmake6 dream.pro CONFIG+=fdk-aac CONFIG+=speexdsp

if [ $? -ne 0 ]; then
    echo -e "${RED}❌ qmake6 failed!${NC}"
    exit 1
fi

# Build
echo ""
echo "Building Dream GUI with Qt 6..."
NCPU=$(sysctl -n hw.ncpu)
echo "Using $NCPU CPU cores..."

make -j$NCPU

if [ $? -ne 0 ]; then
    echo -e "${RED}❌ Build failed!${NC}"
    exit 1
fi

echo ""
echo -e "${GREEN}✅ Build successful with Qt 6!${NC}"
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
echo "=== Qt 6 Build Complete ==="