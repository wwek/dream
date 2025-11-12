/******************************************************************************\
 * Anti-aliasing FIR filter for audio resampling
 * Adapted from KiwiSDR CuteSDR/fir.h by Moe Wheatley
 *
 * This class implements a FIR filter using a double flat coefficient
 * array to eliminate testing for buffer wrap around.
 *
 * Filter coefficients are generated using Kaiser-Bessel windowed sinc algorithm
 *
 ******************************************************************************
 *
 * Original Copyright 2010 Moe Wheatley (Simplified BSD License)
 * Dream DRM integration Copyright (c) 2025
 *
\******************************************************************************/

#ifndef FIR_H
#define FIR_H

#include "../GlobalDefinitions.h"
#include <cmath>

#define MAX_NUMCOEF 97
#define K_2PI (2.0 * 3.14159265358979323846)
#define K_PI (3.14159265358979323846)

////////////
// Class for FIR Filters
////////////
class CFir
{
public:
    CFir();

    int m_NumTaps;

    // Initialize Low Pass Filter
    // NumTaps: if non-zero, forces filter design to be this number of taps (0 = auto)
    // Scale: linear amplitude scale factor
    // Astop: Stopband Attenuation in dB (e.g., 20dB)
    // Fpass: Lowpass passband frequency in Hz
    // Fstop: Lowpass stopband frequency in Hz
    // Fsamprate: Sample Rate in Hz
    int InitLPFilter(int NumTaps, _REAL Scale, _REAL Astop, _REAL Fpass, _REAL Fstop, _REAL Fsamprate);

    // Process filter: InBuf -> OutBuf (can be same buffer for in-place filtering)
    void ProcessFilter(int InLength, _REAL* InBuf, _REAL* OutBuf);

private:
    _REAL Izero(_REAL x);  // Modified Bessel function of the first kind, order 0
    _REAL m_SampleRate;
    int m_State;
    _REAL m_Coef[MAX_NUMCOEF*2];
    _REAL m_rZBuf[MAX_NUMCOEF];
};

#endif // FIR_H
