// Stub string functions for standalone DRM build
#include "str.h"
#include <stdarg.h>
#include <stdio.h>

// Simple stub implementation of kiwi_snprintf_int
int _kiwi_snprintf_int(const char *buf, size_t buflen, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int result = vsnprintf((char*)buf, buflen, fmt, args);
    va_end(args);
    return result;
}
