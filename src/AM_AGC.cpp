/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001-2025
 *
 * Author(s):
 *	wwek
 *
 * Description:
 *	Implementation of Automatic Gain Control (AGC) with Gain Smoothing
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

#include "AM_AGC.h"
#include "matlib/MatlibSigProToolbox.h"


/******************************************************************************\
* Gain Smoother                                                                *
\******************************************************************************/
CGainSmoother::CGainSmoother() :
    eMode(SM_MEDIUM),
    rCurrentGain((CReal) 1.0),
    rSmoothFactor((CReal) 0.9)
{
}

void CGainSmoother::Init(ESmoothMode eNewMode)
{
    eMode = eNewMode;
    rCurrentGain = (CReal) 1.0;
    rSmoothFactor = GetSmoothFactor();
}

void CGainSmoother::Reset()
{
    rCurrentGain = (CReal) 1.0;
}

void CGainSmoother::SetMode(ESmoothMode eNewMode)
{
    eMode = eNewMode;
    rSmoothFactor = GetSmoothFactor();
}

CReal CGainSmoother::GetSmoothFactor() const
{
    switch (eMode)
    {
    case SM_FAST:
        return (CReal) 0.7;   /* Quick response for fast AGC */
    case SM_MEDIUM:
        return (CReal) 0.9;   /* Balanced for medium AGC */
    case SM_SLOW:
        return (CReal) 0.95;  /* Smooth for slow AGC */
    default:
        return (CReal) 0.9;
    }
}

CReal CGainSmoother::Process(CReal rTargetGain)
{
    /* Limit gain change to prevent extreme jumps */
    CReal rGainDelta = rTargetGain - rCurrentGain;

    if (rGainDelta > MAX_GAIN_CHANGE_PER_SAMPLE)
        rGainDelta = MAX_GAIN_CHANGE_PER_SAMPLE;
    else if (rGainDelta < -MAX_GAIN_CHANGE_PER_SAMPLE)
        rGainDelta = -MAX_GAIN_CHANGE_PER_SAMPLE;

    /* Apply smoothed gain change */
    rCurrentGain += rGainDelta * ((CReal) 1.0 - rSmoothFactor);

    return rCurrentGain;
}


/******************************************************************************\
* Traditional AGC                                                              *
\******************************************************************************/
CAGC::CAGC() :
    iSampleRate(0),
    iBlockSize(0),
    eType(AT_MEDIUM),
    rAttack((CReal) 0.0),
    rDecay((CReal) 0.0),
    rAvAmplEst(DES_AV_AMPL_AM_SIGNAL / AM_AMPL_CORR_FACTOR)
{
}

void CAGC::Init(int iNewSampleRate, int iNewBlockSize)
{
    /* Set internal parameters */
    iSampleRate = iNewSampleRate;
    iBlockSize = iNewBlockSize;

    /* Init filters based on AGC type */
    SetType(eType);

    /* Init average amplitude estimation with desired amplitude */
    rAvAmplEst = DES_AV_AMPL_AM_SIGNAL / AM_AMPL_CORR_FACTOR;

    /* Init gain smoother based on AGC type */
    CGainSmoother::ESmoothMode eSmoothMode;
    switch (eType)
    {
    case AT_FAST:
        eSmoothMode = CGainSmoother::SM_FAST;
        break;
    case AT_SLOW:
        eSmoothMode = CGainSmoother::SM_SLOW;
        break;
    case AT_MEDIUM:
    default:
        eSmoothMode = CGainSmoother::SM_MEDIUM;
        break;
    }
    Smoother.Init(eSmoothMode);
}

void CAGC::SetType(const EAmAgcType eNewType)
{
    /* Set internal parameter */
    eType = eNewType;

    /*          Slow     Medium   Fast    */
    /* Attack: 0.025 s, 0.015 s, 0.005 s  */
    /* Decay : 4.000 s, 2.000 s, 0.200 s  */
    switch (eType)
    {
    case AT_SLOW:
        rAttack = IIR1Lam(0.025, iSampleRate);
        rDecay = IIR1Lam(4.000, iSampleRate);
        Smoother.SetMode(CGainSmoother::SM_SLOW);
        break;

    case AT_MEDIUM:
        rAttack = IIR1Lam(0.015, iSampleRate);
        rDecay = IIR1Lam(2.000, iSampleRate);
        Smoother.SetMode(CGainSmoother::SM_MEDIUM);
        break;

    case AT_FAST:
        rAttack = IIR1Lam(0.005, iSampleRate);
        rDecay = IIR1Lam(0.200, iSampleRate);
        Smoother.SetMode(CGainSmoother::SM_FAST);
        break;

    case AT_NO_AGC:
        break;

    case AT_AUTOMATIC:
        /* Automatic AGC is handled by CAGCAutomatic class */
        break;
    }
}

void CAGC::Process(CRealVector& vecrIn)
{
    if (eType == AT_NO_AGC)
    {
        /* No modification of the signal (except of an amplitude
           correction factor) */
        vecrIn *= AM_AMPL_CORR_FACTOR;
    }
    else
    {
        for (int i = 0; i < iBlockSize; i++)
        {
            /* Two sided one-pole recursion for average amplitude
               estimation */
            IIR1TwoSided(rAvAmplEst, Abs(vecrIn[i]), rAttack, rDecay);

            /* Lower bound for estimated average amplitude */
            if (rAvAmplEst < LOWER_BOUND_AMP_LEVEL)
                rAvAmplEst = LOWER_BOUND_AMP_LEVEL;

            /* Calculate target gain */
            CReal rTargetGain = DES_AV_AMPL_AM_SIGNAL / rAvAmplEst;

            /* Apply smoothed gain to signal */
            vecrIn[i] *= Smoother.Process(rTargetGain);
        }
    }
}


/******************************************************************************\
* Automatic AGC                                                                *
\******************************************************************************/
CAGCAutomatic::CAGCAutomatic() :
    iSampleRate(0),
    iBlockSize(0),
    rAvAmplEst(DES_AV_AMPL_AM_SIGNAL / AM_AMPL_CORR_FACTOR),
    rAttack((CReal) 0.0),
    rDecay((CReal) 0.0),
    rSignalVariation((CReal) 0.0),
    rPeakValue((CReal) 0.0),
    rRmsValue((CReal) 0.0),
    iHistoryIndex(0),
    iHistorySize(0)
{
}

void CAGCAutomatic::Init(int iNewSampleRate, int iNewBlockSize)
{
    /* Set internal parameters */
    iSampleRate = iNewSampleRate;
    iBlockSize = iNewBlockSize;

    /* Init average amplitude estimation */
    rAvAmplEst = DES_AV_AMPL_AM_SIGNAL / AM_AMPL_CORR_FACTOR;

    /* Init adaptive parameters with medium defaults */
    rAttack = IIR1Lam(0.015, iSampleRate);  /* Medium attack */
    rDecay = IIR1Lam(2.000, iSampleRate);   /* Medium decay */

    /* Init signal analysis parameters */
    rSignalVariation = (CReal) 0.0;
    rPeakValue = (CReal) 0.0;
    rRmsValue = (CReal) 0.0;

    /* Init history buffer (0.5 second window for signal analysis) */
    /* Add boundary check to prevent division by zero */
    if (iBlockSize > 0)
        iHistorySize = (int) (0.5 * iSampleRate / iBlockSize);
    else
        iHistorySize = 5;  /* Default to minimum if block size is invalid */
    if (iHistorySize < 5) iHistorySize = 5;  /* Minimum 5 blocks */
    vecrAmplitudeHistory.Init(iHistorySize, (CReal) 0.0);
    iHistoryIndex = 0;

    /* Init gain smoother with medium mode as default */
    Smoother.Init(CGainSmoother::SM_MEDIUM);
}

void CAGCAutomatic::AnalyzeSignalCharacteristics(const CRealVector& vecrIn)
{
    /* Calculate block statistics */
    CReal rBlockPeak = (CReal) 0.0;
    CReal rBlockSumSquares = (CReal) 0.0;

    for (int i = 0; i < iBlockSize; i++)
    {
        CReal rAbsVal = Abs(vecrIn[i]);

        /* Track peak */
        if (rAbsVal > rBlockPeak)
            rBlockPeak = rAbsVal;

        /* Accumulate for RMS calculation */
        rBlockSumSquares += rAbsVal * rAbsVal;
    }

    /* Calculate block RMS */
    CReal rBlockRms = Sqrt(rBlockSumSquares / iBlockSize);

    /* Update amplitude history (circular buffer) */
    vecrAmplitudeHistory[iHistoryIndex] = rBlockRms;
    iHistoryIndex = (iHistoryIndex + 1) % iHistorySize;

    /* Calculate signal variation over history window */
    CReal rMean = (CReal) 0.0;
    CReal rVariance = (CReal) 0.0;

    for (int i = 0; i < iHistorySize; i++)
        rMean += vecrAmplitudeHistory[i];
    rMean /= iHistorySize;

    for (int i = 0; i < iHistorySize; i++)
    {
        CReal rDiff = vecrAmplitudeHistory[i] - rMean;
        rVariance += rDiff * rDiff;
    }
    rVariance /= iHistorySize;

    /* Update signal variation metric (normalized by mean to get CV) */
    if (rMean > LOWER_BOUND_AMP_LEVEL)
        rSignalVariation = Sqrt(rVariance) / rMean;
    else
        rSignalVariation = (CReal) 0.0;

    /* Update peak and RMS with smoothing */
    const CReal rSmoothFactor = (CReal) 0.8;
    rPeakValue = rPeakValue * rSmoothFactor + rBlockPeak * ((CReal) 1.0 - rSmoothFactor);
    rRmsValue = rRmsValue * rSmoothFactor + rBlockRms * ((CReal) 1.0 - rSmoothFactor);
}

void CAGCAutomatic::AdaptAGCParameters()
{
    /* Adaptive strategy based on signal variation:
     * - High variation (>0.3): Fast response needed
     * - Medium variation (0.1-0.3): Balanced response
     * - Low variation (<0.1): Smooth response
     */

    if (rSignalVariation > (CReal) 0.3)
    {
        /* Fast AGC for highly variable signals */
        rAttack = IIR1Lam(0.005, iSampleRate);  /* Fast attack: 5ms */
        rDecay = IIR1Lam(0.200, iSampleRate);   /* Fast decay: 200ms */
    }
    else if (rSignalVariation > (CReal) 0.1)
    {
        /* Medium AGC for moderately variable signals */
        rAttack = IIR1Lam(0.015, iSampleRate);  /* Medium attack: 15ms */
        rDecay = IIR1Lam(2.000, iSampleRate);   /* Medium decay: 2s */
    }
    else
    {
        /* Slow AGC for stable signals */
        rAttack = IIR1Lam(0.025, iSampleRate);  /* Slow attack: 25ms */
        rDecay = IIR1Lam(4.000, iSampleRate);   /* Slow decay: 4s */
    }

    /* Adapt smoothing mode based on signal characteristics */
    CGainSmoother::ESmoothMode eNewMode = DetermineSmoothMode();
    Smoother.SetMode(eNewMode);
}

CGainSmoother::ESmoothMode CAGCAutomatic::DetermineSmoothMode() const
{
    /* Determine smoothing mode based on signal variation and crest factor */
    CReal rCrestFactor = (CReal) 1.0;
    if (rRmsValue > LOWER_BOUND_AMP_LEVEL)
        rCrestFactor = rPeakValue / rRmsValue;

    /* High crest factor (>3) indicates impulsive signals → need fast smoothing
     * High variation (>0.3) indicates rapid changes → need fast smoothing
     * Otherwise use smooth mode for audio quality */

    if (rCrestFactor > (CReal) 3.0 || rSignalVariation > (CReal) 0.3)
        return CGainSmoother::SM_FAST;
    else if (rCrestFactor > (CReal) 2.0 || rSignalVariation > (CReal) 0.1)
        return CGainSmoother::SM_MEDIUM;
    else
        return CGainSmoother::SM_SLOW;
}

void CAGCAutomatic::Process(CRealVector& vecrIn)
{
    /* Step 1: Analyze signal characteristics every block */
    AnalyzeSignalCharacteristics(vecrIn);

    /* Step 2: Adapt AGC parameters based on analysis */
    AdaptAGCParameters();

    /* Step 3: Apply adaptive AGC similar to traditional AGC but with dynamic parameters */
    for (int i = 0; i < iBlockSize; i++)
    {
        /* Two-sided one-pole recursion for average amplitude estimation
         * using dynamically adapted attack/decay parameters */
        IIR1TwoSided(rAvAmplEst, Abs(vecrIn[i]), rAttack, rDecay);

        /* Lower bound for estimated average amplitude */
        if (rAvAmplEst < LOWER_BOUND_AMP_LEVEL)
            rAvAmplEst = LOWER_BOUND_AMP_LEVEL;

        /* Calculate target gain */
        CReal rTargetGain = DES_AV_AMPL_AM_SIGNAL / rAvAmplEst;

        /* Apply smoothed gain with adaptive smoothing mode */
        vecrIn[i] *= Smoother.Process(rTargetGain);
    }
}
