# DRM Extension for OpenWebRX

Enhanced DRM receiver implementation based on KiwiSDR, providing advanced features:
- AAC audio codec support (via FDK-AAC)
- Real-time metadata extraction
- Enhanced signal quality monitoring
- Journaline and slideshow support
- Backward compatible with original dream binary

## Installation

The main installation script `../install-dream-enhanced.sh` will automatically build and install this extension using the KiwiSDR DRM implementation.

## Features

- Backward compatible with original dream binary
- Enhanced audio quality with AAC support
- Real-time metadata via Unix socket
- Improved signal processing algorithms from KiwiSDR
- Journaline text support
- Slideshow image support
- FDK-AAC audio codec integration

## Files Structure

```
extensions/DRM/
├── dream/                   # KiwiSDR DRM implementation
│   ├── DRM_main.cpp
│   ├── DRMReceiver.cpp
│   ├── DRMReceiver.h
│   ├── DRM.cpp
│   ├── DRM.h
│   └── [other DRM source files]
├── FDK-AAC/               # AAC audio codec implementation
├── Makefile              # Build configuration
├── install-dream-enhanced.sh # Installation script (parent directory)
└── README.md               # This file
```

## Building

```bash
# From project root:
./extensions/DRM/install-dream-enhanced.sh
```

This will compile the KiwiSDR DRM implementation and install it as `dream-enhanced` with a compatibility symlink as `dream`.