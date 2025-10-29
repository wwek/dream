# Dream DRM Receiver - macOS ARM Deployment Guide

## Overview

This guide explains how to build and package a distributable version of Dream DRM Receiver for macOS ARM (Apple Silicon).

## Files

- **`deploy-arm.sh`**: ARM deployment script (recommended)
- **`deploy.sh`**: Original script for Intel + MacPorts (outdated)

## Prerequisites

### 1. Install Dependencies

Ensure all dependencies are installed via Homebrew:

```bash
brew install qt@5 fftw hamlib libpcap qwt-qt5 fdk-aac speexdsp
```

### 2. Build Application

Ensure `dream.app` has been successfully built:

```bash
/opt/homebrew/opt/qt@5/bin/qmake dream.pro CONFIG+=fdk-aac
make -j8
```

## Usage

### Run Deployment Script

```bash
./macx/deploy-arm.sh
```

### Script Operations

1. ✅ **Check Dependencies**: Verify Qt5 and dream.app exist
2. 📦 **Create Copy**: Create app copy in `dream/` directory
3. 📚 **Bundle Libraries**: Copy all dependency dylibs to `dream.app/Contents/Frameworks/`
4. 🔗 **Fix Paths**: Use `install_name_tool` to change library paths to relative paths
5. 🔧 **Package Qt**: Run `macdeployqt` to bundle Qt frameworks
6. 💿 **Create DMG**: Generate `dream-arm64.dmg` disk image

### Output Files

After successful execution:

- **`dream/dream.app`**: Standalone app bundle (with all dependencies)
- **`dream-arm64.dmg`**: macOS installer disk image (distributable)

## Testing

### Test Local App Bundle

```bash
open dream/dream.app
```

### Test DMG Image

```bash
open dream-arm64.dmg
# Drag dream.app to /Applications
# Double-click to launch
```

## Comparison with Original deploy.sh

| Feature | deploy.sh (Original) | deploy-arm.sh (New) |
|---------|---------------------|---------------------|
| Architecture | Intel (x86_64) | ARM (arm64) |
| Package Manager | MacPorts (`/opt/local`) | Homebrew (`/opt/homebrew`) |
| Dependencies | Missing fftw, speexdsp, fdk-aac | Includes all new deps |
| Qwt Support | ❌ Missing | ✅ Full support |
| Error Handling | Basic | Enhanced (set -e, checks) |
| Output DMG | `dream.dmg` | `dream-arm64.dmg` |

## Dependency Libraries

Script automatically copies the following dynamic libraries:

### Core Dependencies
- `libfftw3.3.dylib` - Fast Fourier Transform
- `libspeexdsp.1.dylib` - Speex DSP audio resampling
- `libfdk-aac.2.dylib` - FDK AAC codec
- `libhamlib.4.dylib` - Ham radio control
- `libpcap.A.dylib` - Network packet capture

### Qwt Framework
- `qwt.framework` - Qt Widgets for Technical Applications

### Qt Frameworks (via macdeployqt)
- `QtCore.framework`
- `QtGui.framework`
- `QtWidgets.framework`
- `QtNetwork.framework`
- `QtXml.framework`
- And more...

## Troubleshooting

### Issue 1: Qt5 Not Found

**Error**: `❌ Qt5 not found at /opt/homebrew/opt/qt@5`

**Solution**:
```bash
brew install qt@5
```

### Issue 2: dream.app Does Not Exist

**Error**: `❌ dream.app not found`

**Solution**: Build the app first
```bash
/opt/homebrew/opt/qt@5/bin/qmake dream.pro CONFIG+=fdk-aac
make -j8
```

### Issue 3: Library Files Missing

**Warning**: `⚠️ Warning: libxxx.dylib not found`

**Solution**: Install missing library
```bash
brew install <package-name>
```

### Issue 4: macdeployqt Failed

**Error**: `⚠️ Warning: macdeployqt not found`

**Solution**: Ensure Qt5 is properly installed and `$PATH` includes Qt bin directory

## Technical Details

### Dynamic Library Path Fixing

Script uses `install_name_tool` to convert absolute paths to relative paths:

**Before** (absolute path):
```
/opt/homebrew/lib/libfftw3.3.dylib
```

**After** (relative path):
```
@executable_path/../Frameworks/libfftw3.3.dylib
```

This ensures the app can run on any Mac without requiring Homebrew dependencies.

### Bundle Structure

Deployed app structure:

```
dream.app/
├── Contents/
│   ├── MacOS/
│   │   └── dream              # Main executable
│   ├── Frameworks/            # Bundled dylibs and frameworks
│   │   ├── libfftw3.3.dylib
│   │   ├── libspeexdsp.1.dylib
│   │   ├── libfdk-aac.2.dylib
│   │   ├── libhamlib.4.dylib
│   │   ├── libpcap.A.dylib
│   │   ├── qwt.framework/
│   │   ├── QtCore.framework/
│   │   ├── QtGui.framework/
│   │   └── ...
│   ├── Resources/             # Resource files
│   └── Info.plist             # App metadata
```

## Distribution Recommendations

### Local Distribution
Share `dream-arm64.dmg` file directly

### Online Distribution
1. Upload DMG to file hosting service
2. Provide download link with SHA256 checksum

### Code Signing (Optional)
For macOS Gatekeeper compliance:

```bash
codesign --deep --force --verify --verbose \
  --sign "Developer ID Application: Your Name" \
  dream/dream.app

# Notarization (requires Apple Developer account)
xcrun notarytool submit dream-arm64.dmg \
  --apple-id "your@email.com" \
  --password "app-specific-password" \
  --team-id "TEAM_ID"
```

## Additional Notes

### Why Create a New Script?

Original `deploy.sh` has these issues:
1. ❌ Hardcoded Intel Homebrew paths (`/usr/local`)
2. ❌ Hardcoded MacPorts paths (`/opt/local`)
3. ❌ Missing support for new dependencies (fftw, speexdsp, fdk-aac, qwt)
4. ❌ No ARM architecture support

### Compatibility

- ✅ **macOS Version**: macOS 11.0+ (Big Sur and later)
- ✅ **Architecture**: ARM64 (Apple Silicon)
- ⚠️ **Intel Mac**: Requires recompilation or Rosetta 2

## References

- [macdeployqt Documentation](https://doc.qt.io/qt-5/macos-deployment.html)
- [Homebrew on Apple Silicon](https://docs.brew.sh/Installation)
- [install_name_tool Manual](https://www.manpagez.com/man/1/install_name_tool/)
