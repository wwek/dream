# Dream DRM Receiver - Multi-architecture Qt6 Build
# Use Ubuntu 22.04 for reliable Qt6 ARM64 support
FROM --platform=$TARGETPLATFORM ubuntu:22.04

LABEL maintainer="OpenWebRX Dream DRM Receiver"
LABEL description="Dream DRM Receiver 2.2.x build environment with Qt6 and xHE-AAC support"

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive \
    INSTALL_PREFIX=/usr/local \
    TZ=UTC

# Set timezone
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Enable restricted, universe, and multiverse repositories for Ubuntu 22.04
RUN sed -i 's/# deb/deb/g' /etc/apt/sources.list

# Architecture detection
ARG TARGETPLATFORM
ARG BUILDPLATFORM
RUN echo "Building on $BUILDPLATFORM for $TARGETPLATFORM" && \
    echo "Architecture: $(uname -m)" && \
    echo "Debian Architecture: $(dpkg --print-architecture)"

# Install build dependencies and runtime dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    # Build tools
    build-essential \
    cmake \
    make \
    gcc \
    g++ \
    pkg-config \
    git \
    wget \
    ca-certificates \
    libc-bin \
    \
    # Qt6 (minimal for console mode)
    qt6-qmake \
    qtbase6-dev \
    qt6-base-dev-tools \
    qt6-multimedia-dev \
    \
    # Audio libraries (FDK-AAC from non-free)
    libfdk-aac-dev \
    libfdk-aac2 \
    libfaad-dev \
    libopus-dev \
    \
    # Signal processing
    libfftw3-dev \
    libspeex-dev \
    libspeexdsp-dev \
    libsamplerate0-dev \
    \
    # Audio output
    libpulse-dev \
    pulseaudio \
    \
    # Hardware support
    libhamlib-dev \
    libusb-1.0-0-dev \
    \
    # Network capture
    libpcap-dev \
    \
    # Utilities
    file \
    && rm -rf /var/lib/apt/lists/*

# Verify FDK-AAC version (must be >= 2.0.0 for USAC/xHE-AAC support)
RUN pkg-config --modversion fdk-aac && \
    pkg-config --atleast-version=2.0.0 fdk-aac || \
    (echo "ERROR: FDK-AAC version must be >= 2.0.0 for xHE-AAC support" && exit 1)

# Create working directory
WORKDIR /build

# Copy Dream source code
COPY . /build/

# Build Dream in console mode
RUN echo "Building Dream 2.2.x with Qt6..." && \
    pkg-config --list-all | grep -E "fdk-aac|fftw|opus|speex" && \
    qmake CONFIG+=console CONFIG+=fdk-aac CONFIG+=speexdsp dream.pro && \
    make -j$(nproc) && \
    echo "✓ Build successful"

# Verify binary
RUN echo "Verifying binary..." && \
    test -f dream && \
    file dream && \
    ldd dream | grep -E "fdk-aac|speex|fftw|pulse" && \
    echo "✓ All required libraries linked successfully"

# Install to system
RUN make install && \
    mkdir -p /output && \
    cp dream /output/dream && \
    if [ -f /usr/bin/dream ]; then \
        cp /usr/bin/dream /usr/local/bin/dream; \
    elif [ -f /usr/local/bin/dream ]; then \
        echo "✓ Binary already at /usr/local/bin/dream"; \
    else \
        echo "ERROR: dream binary not found after install" && exit 1; \
    fi && \
    cp /usr/local/bin/dream /output/dream-installed

# Display build information
RUN echo "========================================" && \
    echo "Dream DRM Receiver 2.2.x Build Complete" && \
    echo "========================================" && \
    echo "Binary: /usr/local/bin/dream" && \
    echo "Size: $(ls -lh /usr/local/bin/dream | awk '{print $5}')" && \
    echo "" && \
    echo "Linked libraries:" && \
    ldd /usr/local/bin/dream | grep -E "fdk-aac|speex|fftw|pulse" && \
    echo "" && \
    echo "Version info:" && \
    /usr/local/bin/dream --version || echo "(Version flag not supported)" && \
    echo "========================================"

# Set working directory for runtime
WORKDIR /data

# Default command shows help
CMD ["/usr/local/bin/dream", "--help"]
