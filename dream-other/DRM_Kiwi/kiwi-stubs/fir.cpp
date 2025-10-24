// Stub fir.cpp for standalone DRM build
// Minimal CFir class implementation - pass-through filters only
#include "fir.h"
#include <cstring>

CFir::CFir() : m_NumTaps(0), m_SampleRate(0), m_State(0) {
    memset(m_Coef, 0, sizeof(m_Coef));
    memset(m_ICoef, 0, sizeof(m_ICoef));
    memset(m_QCoef, 0, sizeof(m_QCoef));
    memset(m_rZBuf, 0, sizeof(m_rZBuf));
    memset(m_cZBuf, 0, sizeof(m_cZBuf));
}

void CFir::InitConstFir(int NumTaps, const TYPEREAL* pCoef, TYPEREAL Fsamprate) {
    m_NumTaps = NumTaps;
    m_SampleRate = Fsamprate;
    // Stub - don't actually initialize coefficients
}

void CFir::InitConstFir(int NumTaps, const TYPEREAL* pICoef, const TYPEREAL* pQCoef, TYPEREAL Fsamprate) {
    m_NumTaps = NumTaps;
    m_SampleRate = Fsamprate;
    // Stub - don't actually initialize coefficients
}

int CFir::InitLPFilter(int NumTaps, TYPEREAL Scale, TYPEREAL Astop, TYPEREAL Fpass, TYPEREAL Fstop, TYPEREAL Fsamprate, bool dump) {
    m_NumTaps = NumTaps;
    m_SampleRate = Fsamprate;
    // Stub - don't actually design filter
    return NumTaps;
}

int CFir::InitHPFilter(int NumTaps, TYPEREAL Scale, TYPEREAL Astop, TYPEREAL Fpass, TYPEREAL Fstop, TYPEREAL Fsamprate) {
    m_NumTaps = NumTaps;
    m_SampleRate = Fsamprate;
    // Stub - don't actually design filter
    return NumTaps;
}

void CFir::GenerateHBFilter(TYPEREAL FreqOffset) {
    // Stub - don't generate halfband filter
}

// Pass-through filter implementations - just copy input to output
void CFir::ProcessFilter(int InLength, TYPEREAL* InBuf, TYPEREAL* OutBuf) {
    // Simple pass-through
    if (InBuf != OutBuf) {
        memcpy(OutBuf, InBuf, InLength * sizeof(TYPEREAL));
    }
}

void CFir::ProcessFilter(int InLength, TYPEREAL* InBuf, TYPECPX* OutBuf) {
    // Convert real to complex (zero imaginary part)
    for (int i = 0; i < InLength; i++) {
        OutBuf[i].re = InBuf[i];
        OutBuf[i].im = 0;
    }
}

void CFir::ProcessFilter(int InLength, TYPECPX* InBuf, TYPECPX* OutBuf) {
    // Simple pass-through for complex data
    if (InBuf != OutBuf) {
        memcpy(OutBuf, InBuf, InLength * sizeof(TYPECPX));
    }
}

void CFir::ProcessFilter(int InLength, TYPEREAL* InBuf, TYPEMONO16* OutBuf) {
    // Convert float to 16-bit integer
    for (int i = 0; i < InLength; i++) {
        TYPEREAL sample = InBuf[i];
        if (sample > 1.0) sample = 1.0;
        if (sample < -1.0) sample = -1.0;
        OutBuf[i] = (TYPEMONO16)(sample * 32767.0);
    }
}

void CFir::ProcessFilter(int InLength, TYPEMONO16* InBuf, TYPEMONO16* OutBuf) {
    // Simple pass-through for 16-bit data
    if (InBuf != OutBuf) {
        memcpy(OutBuf, InBuf, InLength * sizeof(TYPEMONO16));
    }
}

// Double precision overload for dream compatibility
void CFir::ProcessFilter(int InLength, double* InBuf, double* OutBuf) {
    // Simple pass-through for double precision data
    if (InBuf != OutBuf) {
        memcpy(OutBuf, InBuf, InLength * sizeof(double));
    }
}

TYPEREAL CFir::Izero(TYPEREAL x) {
    // Stub - not needed for pass-through
    return 1.0;
}
