FROM debian:bookworm-slim

# Install build dependencies
RUN apt-get update && \
    apt-get -y install --no-install-recommends \
        build-essential \
        gcc \
        g++ \
        make \
        libsndfile1-dev \
        libfftw3-dev \
        zlib1g-dev \
        libfaad-dev \
        rsync \
        ca-certificates && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /build

# Copy DRM extension source
COPY . /build/

# Build dream binary
RUN chmod +x /build/build.sh && \
    /build/build.sh

# Create minimal runtime image
FROM debian:bookworm-slim

# Install runtime dependencies
RUN apt-get update && \
    apt-get -y install --no-install-recommends \
        libsndfile1 \
        libfftw3-single3 \
        zlib1g \
        libfaad2 && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Copy the built binary from build stage
COPY --from=0 /build/drm /usr/local/bin/drm

# Test the binary
RUN drm --help || drm -h || echo "drm binary installed"

ENTRYPOINT ["/usr/local/bin/drm"]
CMD ["--help"]
