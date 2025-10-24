// Stub DRM implementation for standalone build
#include "DRM.h"

// Global DRM instance (stub)
static drm_t drm_stub_instance = {
    .use_LPF = 0,
    .dbgUs = 0,
    .p_i = {0},  // Initialize array
    .dummy = 0
};

drm_t *_DRM_drm_p_global = &drm_stub_instance;
