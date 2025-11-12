/******************************************************************************\
 * Anti-aliasing FIR filter for audio resampling
 * Adapted from KiwiSDR CuteSDR/fir.cpp by Moe Wheatley
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

#include "Fir.h"
#include <cmath>
#include <iostream>

using namespace std;

/////////////////////////////////////////////////////////////////////////////////
// Construct CFir object
/////////////////////////////////////////////////////////////////////////////////
CFir::CFir()
{
    m_NumTaps = 1;
    m_State = 0;
    m_SampleRate = 0.0;
}

/////////////////////////////////////////////////////////////////////////////////
// Process InLength InBuf[] samples and place in OutBuf[]
// Note the Coefficient array is twice the length and has a duplicated set
// in order to eliminate testing for buffer wrap in the inner loop
// REAL version - supports in-place filtering (InBuf == OutBuf)
/////////////////////////////////////////////////////////////////////////////////
void CFir::ProcessFilter(int InLength, _REAL* InBuf, _REAL* OutBuf)
{
    _REAL acc;
    _REAL* Zptr;
    const _REAL* Hptr;

    for(int i=0; i<InLength; i++)
    {
        m_rZBuf[m_State] = InBuf[i];
        Hptr = &m_Coef[m_NumTaps - m_State];
        Zptr = m_rZBuf;
        acc = (*Hptr++ * *Zptr++);    // do the 1st MAC
        for(int j=1; j<m_NumTaps; j++)
            acc += (*Hptr++ * *Zptr++);    // do the remaining MACs
        if(--m_State < 0)
            m_State += m_NumTaps;
        OutBuf[i] = acc;
    }
}

////////////////////////////////////////////////////////////////////
// Create a FIR Low Pass filter with scaled amplitude 'Scale'
// NumTaps if non-zero, forces filter design to be this number of taps
// Scale is linear amplitude scale factor.
// Astop = Stopband Attenuation in dB (ie 20dB is 20dB stopband attenuation)
// Fpass = Lowpass passband frequency in Hz
// Fstop = Lowpass stopband frequency in Hz
// Fsamprate = Sample Rate in Hz
//
//           -------------
//                        |
//                         |
//                          |
//                           |
//    Astop                   ---------------
//                    Fpass   Fstop
//
// Returns: actual number of taps used
////////////////////////////////////////////////////////////////////
int CFir::InitLPFilter(int NumTaps, _REAL Scale, _REAL Astop, _REAL Fpass, _REAL Fstop, _REAL Fsamprate)
{
    int n;
    _REAL Beta;
    m_SampleRate = Fsamprate;

    // Create normalized frequency parameters
    _REAL normFpass = Fpass/Fsamprate;
    _REAL normFstop = Fstop/Fsamprate;
    _REAL normFcut = (normFstop + normFpass)/2.0;    // low pass filter 6dB cutoff

    // Calculate Kaiser-Bessel window shape factor, Beta, from stopband attenuation
    if(Astop < 20.96)
        Beta = 0;
    else if(Astop >= 50.0)
        Beta = .1102 * (Astop - 8.71);
    else
        Beta = .5842 * pow((Astop-20.96), 0.4) + .07886 * (Astop - 20.96);

    // Now Estimate number of filter taps required based on filter specs
    m_NumTaps = int((Astop - 8.0) / (2.285*K_2PI*(normFstop - normFpass)) + 1);

    // Clamp range of filter taps
    if(m_NumTaps > MAX_NUMCOEF)
        m_NumTaps = MAX_NUMCOEF;
    if(m_NumTaps < 9)
        m_NumTaps = 9;

    if(NumTaps)    // if need to force to a number of taps
        m_NumTaps = NumTaps;

    _REAL fCenter = .5*(_REAL)(m_NumTaps-1);
    _REAL izb = Izero(Beta);        // precalculate denominator since is same for all points

    for(n=0; n < m_NumTaps; n++)
    {
        _REAL x = (_REAL)n - fCenter;
        _REAL c;
        // Create ideal Sinc() LP filter with normFcut
        if((_REAL)n == fCenter)    // deal with odd size filter singularity where sin(0)/0==1
            c = 2.0 * normFcut;
        else
            c = (_REAL)sin(K_2PI*x*normFcut)/(K_PI*x);

        // Calculate Kaiser window and multiply to get coefficient
        x = ((_REAL)n - ((_REAL)m_NumTaps-1.0)/2.0) / (((_REAL)m_NumTaps-1.0)/2.0);
        m_Coef[n] = Scale * c * Izero(Beta * sqrt(1 - (x*x))) / izb;
    }

    // Make a 2x length array for FIR flat calculation efficiency
    for(n = 0; n < m_NumTaps; n++)
        m_Coef[n+m_NumTaps] = m_Coef[n];

    // Initialize the FIR buffers and state
    for(int i=0; i<m_NumTaps; i++)
    {
        m_rZBuf[i] = 0.0;
    }
    m_State = 0;

    return m_NumTaps;
}

////////////////////////////////////////////////////////////////////
// Modified Bessel function of the first kind, order 0
// Used for Kaiser window calculation
////////////////////////////////////////////////////////////////////
_REAL CFir::Izero(_REAL x)
{
    _REAL sum, u, halfx, temp;
    int n;

    sum = u = n = 1;
    halfx = x/2.0;
    do
    {
        temp = halfx/(_REAL)n;
        n += 1;
        temp *= temp;
        u *= temp;
        sum += u;
    } while(u >= 1e-21*sum);

    return sum;
}
