// Stub kiwi_assert.h for standalone DRM build
#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// Simple assertion macros for standalone build
// Note: assert is already defined in assert.h

#define panic(s) { fprintf(stderr, "PANIC: %s\n", s); exit(1); }
#define check(e) assert(e)
