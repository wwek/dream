# Dream 2.2 Docker Guide

## Quick Start

```bash
# Build (Multi-architecture)
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

## Usage

### File Mode (Recommended for Docker)

```bash
# File input → file output
docker run --rm -v "$(pwd):/data" dream-drm:2.2 \
    /usr/local/bin/dream -c 6 --sigsrate 48000 --audsrate 48000 \
    -f "/data/input.iq" -w "/data/output.wav"

# Stdin → file output
cat input.iq | docker run --rm -i -v "$(pwd):/data" dream-drm:2.2 \
    /usr/local/bin/dream -c 6 --sigsrate 48000 --audsrate 48000 \
    -I - -w "/data/output.wav"

# File input → stdout
docker run --rm -v "$(pwd):/data" dream-drm:2.2 \
    /usr/local/bin/dream -c 6 --sigsrate 48000 --audsrate 48000 \
    -f "/data/input.iq" -O - | play -t raw -r 48000 -e s -b 16 -c 1 -
```

### Testing

**File Mode:**
```bash
# File → File
docker run --rm -v "$(pwd):/data" dream-drm:2.2 \
    /usr/local/bin/dream -c 6 --sigsrate 48000 --audsrate 48000 -m 1 \
    -f "/data/test_data/input.rec" -w "/data/output/output.wav"

# Verify output
file output/output.wav
ffprobe -v quiet -show_entries format=duration -of csv=p=0 output/output.wav
```

**Stdin/Stdout:**
```bash
# Stdin → file
cat test_data/input.iq | docker run --rm -i -v "$(pwd):/data" dream-drm:2.2 \
    /usr/local/bin/dream -c 6 --sigsrate 48000 --audsrate 48000 \
    -I - -w "/data/output.wav"

# File → stdout
docker run --rm -v "$(pwd):/data" dream-drm:2.2 \
    /usr/local/bin/dream -c 6 --sigsrate 48000 --audsrate 48000 \
    -f "/data/input.iq" -O - > output.raw
```

## Common Commands

```bash
# Build without cache (Multi-architecture)
docker buildx build --no-cache --platform linux/amd64,linux/arm64 -t dream-drm:2.2 .

# Interactive shell
docker run --rm -it dream-drm:2.2 /bin/bash

# Check linked libraries
docker run --rm dream-drm:2.2 ldd /usr/local/bin/dream

# Multi-platform build
docker buildx build --platform linux/amd64,linux/arm64 -t dream-drm:2.2 .
```

## Image Info

- **Base**: Debian Bullseye Slim
- **Binary**: `/usr/local/bin/dream` (~1.4MB)
- **Libraries**: FDK-AAC v2.0.1, FFTW3, Speex, PulseAudio, Hamlib, libpcap

## Parameter Reference

### Input/Output
- `-I <device>` : Input device/file (`-` for stdin)
- `-O <device>` : Output device/file (`-` for stdout)
- `-f <file>` : Input file (disables soundcard)
- `-w <file>` : Output WAV file

### Signal Processing
- `-c <channel>` : Channel config (6 = I/Q positive, 0 Hz IF)
- `--sigsrate <rate>` : Signal sample rate (default: 48000)
- `--audsrate <rate>` : Audio sample rate (default: 48000)
- `-m <mode>` : Mute mode (1 = silent)

## Troubleshooting

### Docker Audio Errors
```
pa_c_sync failed, error -1
CSoundPulse::Init_HW pa_init failed
```
**Solution**: Use `-f` or `-w` to bypass audio system

### Build Issues
```bash
docker builder prune          # Clean cache
docker system df              # Check disk space
docker build --progress=plain # Verbose logs
```

### Binary Issues
```bash
file dream                # Check architecture
ldd dream                 # Check missing libs
```

## Advanced

### Custom Build Options

Edit `Dockerfile` qmake line:
```dockerfile
qmake CONFIG+=console CONFIG+=fdk-aac CONFIG+=debug dream.pro
```

### Development Mode

```bash
docker run --rm -it -v "$(pwd):/src" dream-drm:2.2 /bin/bash
```

## References

- [Dream DRM](https://sourceforge.net/projects/drm/)
- [OpenWebRX](https://github.com/jketterl/openwebrx)
- [Docker Best Practices](https://docs.docker.com/develop/develop-images/dockerfile_best-practices/)
