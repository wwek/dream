// Stub timer functions for standalone DRM build
#include "timer.h"
#include <unistd.h>

// Simple CPacer stub implementation with multiple constructor overloads
CPacer::CPacer(u64_t time_us) {
    (void)time_us;  // Unused in stub
}

CPacer::CPacer(unsigned long time_us) {
    (void)time_us;  // Unused in stub
}

CPacer::~CPacer() {
}

void CPacer::wait() {
    // Simple 1ms sleep as placeholder
    usleep(1000);
}
