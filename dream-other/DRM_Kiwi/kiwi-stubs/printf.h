// Stub printf.h for standalone DRM build
#pragma once

#include <stdio.h>
#include <stdlib.h>

// Simple panic function for standalone build
#define panic(s) { fprintf(stderr, "PANIC: %s\n", s); exit(1); }

// Stub exit function (maps to standard exit)
#define kiwi_exit(code) exit(code)
