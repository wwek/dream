// Stub DRM.h for standalone DRM build
// Minimal definitions for compatibility with dream/DRM_main.h
#pragma once

#include <stdio.h>
#include <stdlib.h>

// String conversion macro
#define STRINGIFY(x) #x
#define STRINGIFY_DEFINE(x) STRINGIFY(x)

// DRM data type enum (not used in standalone)
enum { DRM_DAT_IQ=0 } drm_dat_e;

// Stub task scheduling function (does nothing in standalone)
#define DRM_next_task(msg) ((void)0)

// Stub printf function (maps to regular printf)
#define alt_printf printf

// Stub exit function (maps to standard exit)
#define kiwi_exit(code) exit(code)

// Stub extension message sending function (does nothing in standalone)
inline int ext_send_msg(int rx_chan, bool debug, const char* msg, ...) {
    (void)rx_chan; (void)debug; (void)msg;
    return 0;
}

// DRM message type enum
enum {
    DRM_MSG_SERVICE = 0,
    DRM_MSG_SLIDESHOW = 1
};

// Stub message encoding function (does nothing in standalone)
inline int DRM_msg_encoded(int type, const char* msg, ...) {
    (void)type; (void)msg;
    return 0;
}

// Stub structures for standalone build
typedef struct {
    int use_LPF;       // LPF usage flag
    int dbgUs;         // Debug microseconds flag
    int p_i[10];       // Debug print index array
    int rx_chan;       // RX channel number
    int i_epoch;       // Epoch counter
    int i_samples;     // Sample counter
    int i_tsamples;    // Total sample counter
    int dummy;
} drm_t;

// Global DRM instance pointer (stub)
extern drm_t *_DRM_drm_p_global;

// Function to get DRM instance pointer
inline drm_t* DRM_drm_p() { return _DRM_drm_p_global; }

// Function to get DRM RX channel number
inline int DRM_rx_chan() { return _DRM_drm_p_global ? _DRM_drm_p_global->rx_chan : 0; }
