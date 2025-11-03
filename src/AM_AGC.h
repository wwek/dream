/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001-2025
 *
 * Author(s):
 *	wwek
 *
 * Description:
 *	Automatic Gain Control (AGC) with Gain Smoothing
 *
 *  This module provides:
 *  - CGainSmoother: Gain transition smoothing to prevent audio clicks
 *  - CAGC: Traditional AGC with attack/decay control
 *  - CAGCAutomatic: Automatic AGC with adaptive parameters
 *
 ******************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
\******************************************************************************/

#ifndef AGC_H__INCLUDED_
#define AGC_H__INCLUDED_

#include "GlobalDefinitions.h"
#include "util/Vector.h"
#include "Parameter.h"

/* AGC Parameters *********************************************************** */
/* Set value for desired amplitude for AM signal, controlled by the AGC. Maximum
   value is the range for short variables (16 bit) -> 32768 */
#define DES_AV_AMPL_AM_SIGNAL           ((CReal) 8000.0)

/* Lower bound for estimated average amplitude. That is needed, since we
   divide by this estimate so it must not be zero */
#define LOWER_BOUND_AMP_LEVEL           ((CReal) 10.0)

/* Amplitude correction factor for demodulation */
#define AM_AMPL_CORR_FACTOR             ((CReal) 5.0)

/* Maximum gain change per sample to prevent extreme jumps */
#define MAX_GAIN_CHANGE_PER_SAMPLE      ((CReal) 0.5)


/* Gain Smoother ************************************************************ */
/* Smooths gain transitions to prevent audio clicks and pops */
class CGainSmoother
{
public:
    enum ESmoothMode {
        SM_FAST   = 0,  /* 0.7  - Quick response for fast AGC */
        SM_MEDIUM = 1,  /* 0.9  - Balanced for medium AGC */
        SM_SLOW   = 2   /* 0.95 - Smooth for slow AGC */
    };

    CGainSmoother();

    void Init(ESmoothMode eMode = SM_MEDIUM);
    CReal Process(CReal rTargetGain);
    void Reset();

    CReal GetCurrentGain() const { return rCurrentGain; }
    void SetMode(ESmoothMode eMode);

protected:
    ESmoothMode eMode;
    CReal       rCurrentGain;
    CReal       rSmoothFactor;

    CReal GetSmoothFactor() const;
};


/* Traditional AGC ********************************************************** */
/* Automatic gain control with configurable attack/decay times */
class CAGC
{
public:
    CAGC();

    void Init(int iSampleRate, int iBlockSize);
    void Process(CRealVector& vecrIn);

    void SetType(const EAmAgcType eNewType);
    EAmAgcType GetType() const { return eType; }

protected:
    int           iSampleRate;
    int           iBlockSize;
    EAmAgcType    eType;
    CReal         rAttack, rDecay;
    CReal         rAvAmplEst;

    CGainSmoother Smoother;  /* Gain smoothing to prevent clicks */
};


/* Automatic AGC ************************************************************ */
/* Fully automatic AGC with adaptive parameters */
class CAGCAutomatic
{
public:
    CAGCAutomatic();

    void Init(int iSampleRate, int iBlockSize);
    void Process(CRealVector& vecrIn);

    /* Get current AGC state for debugging/monitoring */
    CReal GetCurrentGain() const { return Smoother.GetCurrentGain(); }
    CReal GetAverageAmplitude() const { return rAvAmplEst; }

protected:
    /* Core parameters */
    int           iSampleRate;
    int           iBlockSize;
    CReal         rAvAmplEst;

    /* Adaptive AGC parameters */
    CReal         rAttack, rDecay;           /* Dynamic attack/decay */
    CReal         rSignalVariation;          /* Signal variation metric */
    CReal         rPeakValue;                /* Peak amplitude tracking */
    CReal         rRmsValue;                 /* RMS amplitude tracking */

    /* History buffers for signal analysis */
    CRealVector   vecrAmplitudeHistory;      /* Amplitude history window */
    int           iHistoryIndex;             /* Current position in history */
    int           iHistorySize;              /* Size of history window */

    /* Smoothing and adaptation */
    CGainSmoother Smoother;                  /* Gain smoothing */

    /* Internal methods */
    void AnalyzeSignalCharacteristics(const CRealVector& vecrIn);
    void AdaptAGCParameters();
    CGainSmoother::ESmoothMode DetermineSmoothMode() const;
};

#endif // AGC_H__INCLUDED_
