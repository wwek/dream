# Dream DRM Receiver 2.2.x

Standalone DRM (Digital Radio Mondiale) decoder with xHE-AAC audio decoding support.

## Quick Start

### Build with Docker (Recommended)

```bash
# Build with docker (Multi-architecture)
## arm64
docker buildx build --platform linux/arm64 -t dream-drm:2.2 .
docker run --rm -v "$(pwd)/output:/output" dream-drm:2.2 \
    sh -c "cp /usr/local/bin/dream /output/dream-arm64"

## armv7
docker buildx build --platform linux/arm/v7 -t dream-drm:2.2 .
docker run --rm -v "$(pwd)/output:/output" dream-drm:2.2 \
    sh -c "cp /usr/local/bin/dream /output/dream-armv7"

## amd64
docker buildx build --platform linux/amd64 -t dream-drm:2.2 .
docker run --rm -v "$(pwd)/output:/output" dream-drm:2.2 \
    sh -c "cp /usr/local/bin/dream /output/dream-amd64"

# Test
./dream --help
```

### Build with Local Script

```bash
# Build Dream (console mode, no GUI)
./build.sh

# Run
dream --help

```

## Features

✅ **xHE-AAC Decoding Support**
- Uses FDK-AAC v2 codec
- Supports USAC (Unified Speech and Audio Coding)
- All SourceForge forum fix patches applied

✅ **Local Socket Status Monitoring**
View status in terminal:
```
socat UNIX-CONNECT:/tmp/dream_status.sock -
```


✅ **Console Mode**
- No Qt GUI dependency
- Suitable for server/headless systems
- Can receive SDR data streams via pipe
- Standalone binary

✅ **Fixed Issues**
- Frame boundary calculation error (`xheaacsuperframe.cpp`)
- Buffer overflow (`AudioSourceDecoder.cpp`)
- Division by zero crash (`AudioSourceDecoder.cpp`)
- USAC compilation support (`fdk_aac_codec.cpp`)

## Documentation

- **[SourceForge Forum](https://sourceforge.net/p/drm/discussion/general/thread/01c6e64c3b/)** - xHE-AAC fixes discussion

## License

GPL v2 - See LICENSE file for details

## Acknowledgments

xHE-AAC fix contributors:
- John (KiwiSDR) - Original patches
- Tarmo Tanilsoo - Linux validation
- Rafael Diniz - Integration testing
- Julian Cable - Dream maintenance
