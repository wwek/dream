#!/bin/bash
#
# macOS ARM (Apple Silicon) deployment script for Dream DRM Receiver
# Updated for Homebrew on ARM64 architecture
#
# Usage: ./macx/deploy-arm.sh
#

set -e  # Exit on error

echo "🚀 Starting Dream DRM deployment for macOS ARM..."

# Detect Qt5 installation
QT5_PREFIX="/opt/homebrew/opt/qt@5"
if [ ! -d "$QT5_PREFIX" ]; then
    echo "❌ Qt5 not found at $QT5_PREFIX"
    echo "   Install with: brew install qt@5"
    exit 1
fi

# Homebrew library prefix for ARM
HOMEBREW_PREFIX="/opt/homebrew"

# Check if dream.app exists
if [ ! -d "dream.app" ]; then
    echo "❌ dream.app not found. Build the app first with:"
    echo "   qmake dream.pro CONFIG+=fdk-aac"
    echo "   make"
    exit 1
fi

# Create deployment directory
echo "📦 Creating deployment directory..."
mkdir -p dream
rm -rf dream/dream.app
cp -r dream.app dream
cd dream

# Create Frameworks directory
echo "📚 Creating Frameworks directory..."
mkdir -p dream.app/Contents/Frameworks

# Copy all required dynamic libraries
echo "📥 Copying dynamic libraries..."

# Core dependencies
LIBS=(
    # Audio/Signal processing
    "libfftw3.3.dylib"
    "libspeexdsp.1.dylib"
    "libfdk-aac.2.dylib"

    # Ham radio support
    "libhamlib.4.dylib"

    # Network
    "libpcap.A.dylib"

    # Audio I/O (if using portaudio)
    # "libportaudio.2.dylib"

    # Audio file support (if using libsndfile)
    # "libsndfile.1.dylib"
    # "libFLAC.12.dylib"
    # "libvorbis.0.dylib"
    # "libvorbisenc.2.dylib"
    # "libogg.0.dylib"
)

for lib in "${LIBS[@]}"; do
    if [ -f "$HOMEBREW_PREFIX/lib/$lib" ]; then
        echo "  ✓ Copying $lib"
        cp "$HOMEBREW_PREFIX/lib/$lib" dream.app/Contents/Frameworks/
    else
        echo "  ⚠️  Warning: $lib not found at $HOMEBREW_PREFIX/lib/"
    fi
done

# Copy Qwt framework
echo "📥 Copying Qwt framework..."
QWT_FRAMEWORK="$HOMEBREW_PREFIX/opt/qwt-qt5/lib/qwt.framework"
if [ -d "$QWT_FRAMEWORK" ]; then
    cp -R "$QWT_FRAMEWORK" dream.app/Contents/Frameworks/
    echo "  ✓ Qwt framework copied"
else
    echo "  ⚠️  Warning: Qwt framework not found at $QWT_FRAMEWORK"
fi

# Run macdeployqt to bundle Qt frameworks
echo "🔧 Running macdeployqt..."
if [ -f "$QT5_PREFIX/bin/macdeployqt" ]; then
    "$QT5_PREFIX/bin/macdeployqt" dream.app
    echo "  ✓ Qt frameworks bundled"
else
    echo "  ⚠️  Warning: macdeployqt not found"
fi

# Fix library paths using install_name_tool
echo "🔗 Fixing library paths..."

DREAM_BINARY="dream.app/Contents/MacOS/dream"

# Fix main executable library paths
for lib in "${LIBS[@]}"; do
    lib_name="${lib%.*}"  # Remove extension
    if [ -f "dream.app/Contents/Frameworks/$lib" ]; then
        install_name_tool -change \
            "$HOMEBREW_PREFIX/lib/$lib" \
            "@executable_path/../Frameworks/$lib" \
            "$DREAM_BINARY" 2>/dev/null || true
        echo "  ✓ Fixed $lib path in executable"
    fi
done

# Fix Qwt framework path
install_name_tool -change \
    "$QWT_FRAMEWORK/Versions/6/qwt" \
    "@executable_path/../Frameworks/qwt.framework/Versions/6/qwt" \
    "$DREAM_BINARY" 2>/dev/null || true
echo "  ✓ Fixed Qwt framework path"

# Fix inter-library dependencies (if any)
# Example: if libsndfile depends on libogg, libvorbis, etc.
# Uncomment and adjust as needed:
#
# install_name_tool -change \
#     "$HOMEBREW_PREFIX/lib/libogg.0.dylib" \
#     "@executable_path/../Frameworks/libogg.0.dylib" \
#     dream.app/Contents/Frameworks/libsndfile.1.dylib 2>/dev/null || true

echo "✅ Deployment preparation complete!"

# Create DMG
cd ..
echo "💿 Creating DMG installer..."
rm -f dream-arm64.dmg
hdiutil create dream-arm64.dmg -srcfolder dream/ -ov -format UDZO

echo ""
echo "🎉 Deployment complete!"
echo ""
echo "📦 Outputs:"
echo "   - App bundle: dream/dream.app"
echo "   - DMG installer: dream-arm64.dmg"
echo ""
echo "🧪 To test the app bundle:"
echo "   open dream/dream.app"
echo ""
echo "📤 To distribute:"
echo "   Share dream-arm64.dmg"
echo ""
