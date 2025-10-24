// Stub shmem.h for standalone DRM build
#pragma once

// Minimal shared memory stub definitions for standalone build
// DRM shared memory will be disabled in standalone mode

typedef struct {
    int dummy;
} drm_shmem_t;
