# Multi-architecture support
FROM --platform=$BUILDPLATFORM debian:bullseye-slim

LABEL maintainer="OpenWebRX Dream DRM Receiver"
LABEL description="Dream DRM Receiver 2.2.x build environment with xHE-AAC support"

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive \
    INSTALL_PREFIX=/usr/local

# Enable non-free repository for FDK-AAC
RUN echo "deb http://deb.debian.org/debian bullseye main contrib non-free" > /etc/apt/sources.list && \
    echo "deb http://deb.debian.org/debian-security bullseye-security main contrib non-free" >> /etc/apt/sources.list && \
    echo "deb http://deb.debian.org/debian bullseye-updates main contrib non-free" >> /etc/apt/sources.list

# Install build dependencies
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
    # Qt5 (minimal for console mode)
    qt5-qmake \
    qtbase5-dev \
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
RUN echo "Building Dream 2.2.x..." && \
    echo "Available packages:" && \
    pkg-config --list-all | grep -i "fdk\|fftw" && \
    qmake CONFIG+=console CONFIG+=fdk-aac dream.pro && \
    echo "Checking Makefile for FDK-AAC and FFTW..." && \
    grep -E "fdk-aac|fftw" Makefile | head -5 && \
    make -j$(nproc) && \
    echo "✓ Build successful"

# Verify binary
RUN echo "Verifying binary..." && \
    test -f dream && \
    file dream && \
    ldd dream | grep fdk-aac && \
    echo "✓ FDK-AAC linked successfully"

# Install to system (dream 默认安装到 /usr/bin/dream)
RUN make install && \
    echo "Binary installed by make install"

# Move to /usr/local/bin and create output artifacts
RUN mkdir -p /output && \
    cp dream /output/dream && \
    if [ -f /usr/bin/dream ]; then \
        cp /usr/bin/dream /usr/local/bin/dream && \
        echo "✓ Moved to /usr/local/bin/dream"; \
    elif [ -f /usr/local/bin/dream ]; then \
        echo "✓ Already at /usr/local/bin/dream"; \
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
    /usr/local/bin/dream --version || echo "Version flag not supported" && \
    echo "========================================" && \
    echo ""

# Set working directory for runtime
WORKDIR /data

# Default command shows help
CMD ["/usr/local/bin/dream", "--help"]
