/******************************************************************************\
 * BBC and Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001-2019
 *
 * Author(s):
 * Volker Fischer, Julian Cable
 *
 * Description:
 *
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

#include "creceivedata.h"
#include "Parameter.h"
#include "util/qt6_compat.h"
#include "util/qt6_audio_compat.h"
#ifdef QT_MULTIMEDIA_LIB
# include <QSet>
# include <QThread>
# include <QTimer>
# include <QEventLoop>
# include <QCoreApplication>
# include <QMetaObject>
# include <QMediaDevices>
# include <QAudioDevice>
# include <QAudioInput>
#else
# include "sound/sound.h"
#endif
#include "sound/audiofilein.h"
#include "util/FileTyper.h"
#include "IQInputFilter.h"
#include "UpsampleFilter.h"
#include "matlib/MatlibSigProToolbox.h"

using namespace std;

const static int SineTable[] = { 0, 1, 0, -1, 0 };
const static _REAL PSDWindowGain = 0.39638; /* power gain of the Hamming window */

//inline _REAL sample2real(_SAMPLE s) { return _REAL(s)/32768.0; }
inline _REAL sample2real(_SAMPLE s) {
    return _REAL(s);
}

CReceiveData::CReceiveData() :
#ifdef QT_MULTIMEDIA_LIB
    pAudioInput(nullptr),
    pIODevice(nullptr),
    bDeviceChanged(false),
    iZeroReadCount(0),
  #endif
    pSound(nullptr),
    vecrInpData(INPUT_DATA_VECTOR_SIZE, 0.0),
    bFippedSpectrum(false), eInChanSelection(CS_MIX_CHAN), iPhase(0),spectrumAnalyser()
{
}

void CReceiveData::Stop()
{
#ifdef QT_MULTIMEDIA_LIB
    QMutexLocker locker(&audioDeviceMutex);  // Lock during device cleanup

    if(pIODevice!=nullptr) {
        pIODevice->close();
        // Give Qt time to process the close operation to prevent race conditions
        QThread::msleep(10);
        pIODevice = nullptr;
    }
    if(pAudioInput != nullptr) {
        // Qt 6: QAudioInput doesn't have stop() method
        // Add delay before deletion to prevent race conditions
        QThread::msleep(20);
        delete pAudioInput;
        pAudioInput = nullptr;
    }
#endif
    if(pSound!=nullptr) {
        pSound->Close();
        pSound = nullptr;
    }
}

CReceiveData::~CReceiveData()
{
#ifdef QT_MULTIMEDIA_LIB
    /* Ensure QAudioInput is cleaned up properly */
    if (pAudioInput != nullptr) {
        fprintf(stderr, "~CReceiveData: Cleaning up QAudioInput\n");
        /* Don't call stop() to avoid double-stop issues */
        delete pAudioInput;
        pAudioInput = nullptr;
    }
    if (pIODevice != nullptr) {
        fprintf(stderr, "~CReceiveData: Cleaning up QIODevice\n");
        delete pIODevice;
        pIODevice = nullptr;
    }
#endif
    if(pSound!=nullptr) {
        pSound->Close();
        delete pSound;
        pSound = nullptr;
    }
}

void CReceiveData::Enumerate(vector<string>& names, vector<string>& descriptions, string& defaultInput)
{
#ifdef QT_MULTIMEDIA_LIB
    QSet<QString> s;
    QString def;

    // Qt 6 implementation
    def = QMediaDevices::defaultAudioInput().description();
    defaultInput = def.toStdString();

    foreach(const QAudioDevice& di, QMediaDevices::audioInputs())
    {
        s.insert(di.description());
    }

    names.clear(); descriptions.clear();
    foreach(const QString n, s) {
        names.push_back(n.toStdString());
        if(n == def) {
            descriptions.push_back("default");
        }
        else {
            descriptions.push_back("");
        }
    }
#else
    if(pSound==nullptr) pSound = new CSoundIn;
    pSound->Enumerate(names, descriptions, defaultInput);
#endif
}

void
CReceiveData::SetSoundInterface(string device)
{
    soundDevice = device;
    bDeviceChanged = true;  // Mark that device needs to be initialized

#ifdef QT_MULTIMEDIA_LIB
    QMutexLocker locker(&audioDeviceMutex);  // Lock for thread-safe device switch

    /* Safely clean up existing audio input - double cleanup for safety */
    if(pAudioInput != nullptr) {
        // Qt 6: QAudioInput doesn't have stop() method
        // Just delete the object - cleanup happens automatically
        // Add delay to prevent race conditions with audio processing thread
        QThread::msleep(30);
        delete pAudioInput;
        pAudioInput = nullptr;
    }

    if(pIODevice != nullptr) {
        pIODevice->close();
        // Give Qt time to process the close operation
        QThread::msleep(10);
        pIODevice = nullptr;
    }
#endif

    if(pSound != nullptr) {
        pSound->Close();
        delete pSound;
        pSound = nullptr;
    }

    if(FileTyper::resolve(device) != FileTyper::unrecognised) {
        CAudioFileIn* pAudioFileIn = new CAudioFileIn();
        pAudioFileIn->SetFileName(device);
        int sr = pAudioFileIn->GetSampleRate();
        if(iSampleRate!=sr) {
            // TODO
            cerr << "file sample rate is " << sr << endl;
            iSampleRate = sr;
        }
        pSound = pAudioFileIn;
    }
    else {
        /* For audio input devices: device will be initialized in InitInternal
         * to ensure proper thread context and avoid race conditions
         */
    }
}


void CReceiveData::ProcessDataInternal(CParameter& Parameters)
{
    int i;

    /* OPH: update free-running symbol counter */
    Parameters.Lock();

    iFreeSymbolCounter++;
    if (iFreeSymbolCounter >= Parameters.CellMappingTable.iNumSymPerFrame * 2) /* x2 because iOutputBlockSize=iSymbolBlockSize/2 */
    {
        iFreeSymbolCounter = 0;
        /* calculate the PSD once per frame for the RSI output */
        if (Parameters.bMeasurePSD) {
            emitRSCIData(Parameters);
        }
    }
    Parameters.Unlock();


    /* Get data from sound interface. The read function must be a
       blocking function! */

#ifdef QT_MULTIMEDIA_LIB
    bool bBad = false;
    QIODevice* pDeviceToRead = nullptr;

    // Safely acquire device pointer with mutex protection
    {
        QMutexLocker locker(&audioDeviceMutex);
        pDeviceToRead = pIODevice;
    }

    if(pDeviceToRead)
    {
#if 0

        QTimer timer;
        timer.setSingleShot(true);
        QEventLoop loop;
        QObject::connect(pDeviceToRead,  SIGNAL(readyRead()), &loop, SLOT(quit()) );
        QObject::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
        timer.start(1000);
        loop.exec();

        if(timer.isActive()) {
            //qDebug("data from sound card");
            qint64 n = 2*vecsSoundBuffer.Size();
            qint64 r = pDeviceToRead->read(reinterpret_cast<char*>(&vecsSoundBuffer[0]), n);
            if(r!=n) {
                cerr << "short read" << endl;
            }
        }
        else {
            qDebug("timeout");
        }

#else
        /* Check if device is actually open and valid before reading */
        if (pDeviceToRead && !pDeviceToRead->isOpen()) {
            fprintf(stderr, "ProcessDataInternal: QIODevice is not open, attempting to restart audio\n");
            bDeviceChanged = true;
            bBad = true;
        }
        else if (pDeviceToRead && pDeviceToRead->isOpen()) {
            /* Qt Multimedia data reading - add device validation */
            qint64 n = 2*vecsSoundBuffer.Size();
            char *p = reinterpret_cast<char*>(&vecsSoundBuffer[0]);
            do {
                qint64 r = pDeviceToRead->read(p, n);
                if(r>0) {
                    p += r;
                    n -= r;
                    iZeroReadCount = 0;  // Reset counter on successful read
                }
                else if(r == 0) {
                    /* No data available yet - wait and retry in next call */
                    iZeroReadCount++;
                    // Only mark as error after 5 consecutive zero reads across calls (~25ms)
                    // This allows temporary buffer gaps in virtual sound cards while detecting real errors
                    if (iZeroReadCount > 5) {
                        fprintf(stderr, "ProcessDataInternal: Consecutive zero reads detected (%d), marking as error\n", iZeroReadCount);
                        bBad = true;
                    }
                    break;  // Exit loop and retry in next ProcessDataInternal call
                }
                else {
                    /* Read error - check if device was switched */
                    fprintf(stderr, "ProcessDataInternal: Qt Read error occurred\n");
                    QThread::msleep(100);
                    bBad = true;
                    break; // Exit read loop on error to prevent crashes
                }
            } while (n>0);
        }
        else {
            /* Device is null or invalid */
            bBad = true;
        }
#endif
    }
    else if (pSound != nullptr) { // for audio files
        bBad = pSound->Read(vecsSoundBuffer);
    }
    else {
        bBad = true;
    }
#else
    bool bBad = true;
    if (pSound != nullptr)
    {
        bBad = pSound->Read(vecsSoundBuffer);
    }
#endif

    Parameters.Lock();
    Parameters.ReceiveStatus.InterfaceI.SetStatus(bBad ? CRC_ERROR : RX_OK); /* Red light */
    Parameters.Unlock();

    if(bBad)
        return;

    /* Upscale if ratio greater than one */
    if (iUpscaleRatio > 1)
    {
        /* The actual upscaling, currently only 2X is supported */
        InterpFIR_2X(2, &vecsSoundBuffer[0], vecf_ZL, vecf_YL, vecf_B);
        InterpFIR_2X(2, &vecsSoundBuffer[1], vecf_ZR, vecf_YR, vecf_B);

        /* Write data to output buffer. Do not set the switch command inside
           the for-loop for efficiency reasons */
        switch (eInChanSelection)
        {
        case CS_LEFT_CHAN:
            for (i = 0; i < iOutputBlockSize; i++)
                (*pvecOutputData)[i] = _REAL(vecf_YL[unsigned(i)]);
            break;

        case CS_RIGHT_CHAN:
            for (i = 0; i < iOutputBlockSize; i++)
                (*pvecOutputData)[i] = _REAL(vecf_YR[unsigned(i)]);
            break;

        case CS_MIX_CHAN:
            for (i = 0; i < iOutputBlockSize; i++)
            {
                /* Mix left and right channel together */
                (*pvecOutputData)[i] = _REAL(vecf_YL[unsigned(i)] + vecf_YR[unsigned(i)]) / 2.0;
            }
            break;

        case CS_SUB_CHAN:
            for (i = 0; i < iOutputBlockSize; i++)
            {
                /* Subtract right channel from left */
                (*pvecOutputData)[i] = _REAL(vecf_YL[unsigned(i)] - vecf_YR[unsigned(i)]) / 2.0;
            }
            break;

        /* I / Q input */
        case CS_IQ_POS:
            for (i = 0; i < iOutputBlockSize; i++)
            {
                (*pvecOutputData)[i] = HilbertFilt(_REAL(vecf_YL[unsigned(i)]), _REAL(vecf_YR[unsigned(i)]));
            }
            break;

        case CS_IQ_NEG:
            for (i = 0; i < iOutputBlockSize; i++)
            {
                (*pvecOutputData)[i] = HilbertFilt(_REAL(vecf_YR[unsigned(i)]), _REAL(vecf_YL[unsigned(i)]));
            }
            break;

        case CS_IQ_POS_ZERO:
            for (i = 0; i < iOutputBlockSize; i++)
            {
                /* Shift signal to vitual intermediate frequency before applying
                   the Hilbert filtering */
                _COMPLEX cCurSig = _COMPLEX(_REAL(vecf_YL[unsigned(i)]), _REAL(vecf_YR[unsigned(i)]));

                cCurSig *= cCurExp;

                /* Rotate exp-pointer on step further by complex multiplication
                   with precalculated rotation vector cExpStep */
                cCurExp *= cExpStep;

                (*pvecOutputData)[i] =
                    HilbertFilt(cCurSig.real(), cCurSig.imag());
            }
            break;

        case CS_IQ_NEG_ZERO:
            for (i = 0; i < iOutputBlockSize; i++)
            {
                /* Shift signal to vitual intermediate frequency before applying
                   the Hilbert filtering */
                _COMPLEX cCurSig = _COMPLEX(_REAL(vecf_YR[unsigned(i)]), _REAL(vecf_YL[unsigned(i)]));

                cCurSig *= cCurExp;

                /* Rotate exp-pointer on step further by complex multiplication
                   with precalculated rotation vector cExpStep */
                cCurExp *= cExpStep;

                (*pvecOutputData)[i] =
                    HilbertFilt(cCurSig.real(), cCurSig.imag());
            }
            break;

        case CS_IQ_POS_SPLIT:
            for (i = 0; i < iOutputBlockSize; i += 4)
            {
                (*pvecOutputData)[i + 0] =  _REAL(vecf_YL[unsigned(i + 0)]);
                (*pvecOutputData)[i + 1] = _REAL(-vecf_YR[unsigned(i + 1)]);
                (*pvecOutputData)[i + 2] = _REAL(-vecf_YL[unsigned(i + 2)]);
                (*pvecOutputData)[i + 3] =  _REAL(vecf_YR[unsigned(i + 3)]);
            }
            break;

        case CS_IQ_NEG_SPLIT:
            for (i = 0; i < iOutputBlockSize; i += 4)
            {
                (*pvecOutputData)[i + 0] =  _REAL(vecf_YR[unsigned(i + 0)]);
                (*pvecOutputData)[i + 1] = _REAL(-vecf_YL[unsigned(i + 1)]);
                (*pvecOutputData)[i + 2] = _REAL(-vecf_YR[unsigned(i + 2)]);
                (*pvecOutputData)[i + 3] =  _REAL(vecf_YL[unsigned(i + 3)]);
            }
            break;
        }
    }

    /* Upscale ratio equal to one */
    else {
        /* Write data to output buffer. Do not set the switch command inside
           the for-loop for efficiency reasons */
        switch (eInChanSelection)
        {
        case CS_LEFT_CHAN:
            for (i = 0; i < iOutputBlockSize; i++)
                (*pvecOutputData)[i] = sample2real(vecsSoundBuffer[2 * i]);
            break;

        case CS_RIGHT_CHAN:
            for (i = 0; i < iOutputBlockSize; i++)
                (*pvecOutputData)[i] = sample2real(vecsSoundBuffer[2 * i + 1]);
            break;

        case CS_MIX_CHAN:
            for (i = 0; i < iOutputBlockSize; i++)
            {
                /* Mix left and right channel together */
                const _REAL rLeftChan = sample2real(vecsSoundBuffer[2 * i]);
                const _REAL rRightChan = sample2real(vecsSoundBuffer[2 * i + 1]);
                (*pvecOutputData)[i] = (rLeftChan + rRightChan) / 2;
            }
            break;

        case CS_SUB_CHAN:
            for (i = 0; i < iOutputBlockSize; i++)
            {
                /* Subtract right channel from left */
                const _REAL rLeftChan = sample2real(vecsSoundBuffer[2 * i]);
                const _REAL rRightChan = sample2real(vecsSoundBuffer[2 * i + 1]);
                (*pvecOutputData)[i] = (rLeftChan - rRightChan) / 2;
            }
            break;

        /* I / Q input */
        case CS_IQ_POS:
            for (i = 0; i < iOutputBlockSize; i++)
            {
                (*pvecOutputData)[i] =
                    HilbertFilt(sample2real(vecsSoundBuffer[2 * i]),
                                sample2real(vecsSoundBuffer[2 * i + 1]));
            }
            break;

        case CS_IQ_NEG:
            for (i = 0; i < iOutputBlockSize; i++)
            {
                (*pvecOutputData)[i] =
                    HilbertFilt(sample2real(vecsSoundBuffer[2 * i + 1]),
                                sample2real(vecsSoundBuffer[2 * i]));
            }
            break;

        case CS_IQ_POS_ZERO:
            for (i = 0; i < iOutputBlockSize; i++)
            {
                /* Shift signal to vitual intermediate frequency before applying
                   the Hilbert filtering */
                _COMPLEX cCurSig = _COMPLEX(sample2real(vecsSoundBuffer[2 * i]),
                                            sample2real(vecsSoundBuffer[2 * i + 1]));

                cCurSig *= cCurExp;

                /* Rotate exp-pointer on step further by complex multiplication
                   with precalculated rotation vector cExpStep */
                cCurExp *= cExpStep;

                (*pvecOutputData)[i] =
                    HilbertFilt(cCurSig.real(), cCurSig.imag());
            }
            break;

        case CS_IQ_NEG_ZERO:
            for (i = 0; i < iOutputBlockSize; i++)
            {
                /* Shift signal to vitual intermediate frequency before applying
                   the Hilbert filtering */
                _COMPLEX cCurSig = _COMPLEX(sample2real(vecsSoundBuffer[2 * i + 1]),
                                            sample2real(vecsSoundBuffer[2 * i]));

                cCurSig *= cCurExp;

                /* Rotate exp-pointer on step further by complex multiplication
                   with precalculated rotation vector cExpStep */
                cCurExp *= cExpStep;

                (*pvecOutputData)[i] =
                    HilbertFilt(cCurSig.real(), cCurSig.imag());
            }
            break;

        case CS_IQ_POS_SPLIT: /* Require twice the bandwidth */
            for (i = 0; i < iOutputBlockSize; i++)
            {
                iPhase = (iPhase + 1) & 3;
                _REAL rValue = vecsSoundBuffer[2 * i]     * /*COS*/SineTable[iPhase + 1] -
                               vecsSoundBuffer[2 * i + 1] * /*SIN*/SineTable[iPhase];
                (*pvecOutputData)[i] = rValue;
            }
            break;

        case CS_IQ_NEG_SPLIT: /* Require twice the bandwidth */
            for (i = 0; i < iOutputBlockSize; i++)
            {
                iPhase = (iPhase + 1) & 3;
                _REAL rValue = vecsSoundBuffer[2 * i + 1] * /*COS*/SineTable[iPhase + 1] -
                               vecsSoundBuffer[2 * i]     * /*SIN*/SineTable[iPhase];
                (*pvecOutputData)[i] = rValue;
            }
            break;
        }
    }

    /* Flip spectrum if necessary ------------------------------------------- */
    if (bFippedSpectrum)
    {
        /* Since iOutputBlockSize is always even we can do some opt. here */
        for (i = 0; i < iOutputBlockSize; i+=2)
        {
            /* We flip the spectrum by using the mirror spectrum at the negative
               frequencys. If we shift by half of the sample frequency, we can
               do the shift without the need of a Hilbert transformation */
            (*pvecOutputData)[i] = -(*pvecOutputData)[i];
        }
    }

    /* Copy data in buffer for spectrum calculation */
    mutexInpData.Lock();
    vecrInpData.AddEnd((*pvecOutputData), iOutputBlockSize);
    mutexInpData.Unlock();

    /* Update level meter */
    SignalLevelMeter.Update((*pvecOutputData));
    Parameters.Lock();
    Parameters.SetIFSignalLevel(SignalLevelMeter.Level());
    Parameters.Unlock();

}

void CReceiveData::InitInternal(CParameter& Parameters)
{
    /* Init sound interface. Set it to one symbol. The sound card interface
           has to taken care about the buffering data of a whole MSC block.
           Use stereo input (* 2) */

#ifdef QT_MULTIMEDIA_LIB
    if (pSound == nullptr && pAudioInput == nullptr)
    {
        /* Don't return - allow initialization to continue for non-audio modes */
    }
#else
    if (pSound == nullptr)
        return;
#endif

    Parameters.Lock();
    /* We define iOutputBlockSize as half the iSymbolBlockSize because
       if a positive frequency offset is present in drm signal,
       after some time a buffer overflow occur in the output buffer of
       InputResample.ProcessData() */
    /* Define output block-size */
    iOutputBlockSize = Parameters.CellMappingTable.iSymbolBlockSize / 2;
    iMaxOutputBlockSize = iOutputBlockSize * 2;
    /* Get signal sample rate */
    iSampleRate = Parameters.GetSigSampleRate();
    iUpscaleRatio = Parameters.GetSigUpscaleRatio();
    Parameters.Unlock();

    const int iOutputBlockAlignment = iOutputBlockSize & 3;
    if (iOutputBlockAlignment) {
        fprintf(stderr, "CReceiveData::InitInternal(): iOutputBlockAlignment = %i\n", iOutputBlockAlignment);
    }

    try {
        /* Wrap entire InitInternal in try-catch to prevent crashes */
        fprintf(stderr, "InitInternal: Starting initialization...\n");

        bool bChanged = false;
        int wantedBufferSize = iOutputBlockSize * 2 / iUpscaleRatio; // samples

        if(pSound) { // must be sound file
            bChanged = (pSound==nullptr)?true:pSound->Init(iSampleRate / iUpscaleRatio, wantedBufferSize, true);
        }
        else if (bDeviceChanged || pAudioInput == nullptr) {
            /* Create QAudioInput */
            fprintf(stderr, "InitInternal: Creating QAudioInput for device '%s'\n", soundDevice.c_str());

            /* Clean up old audio input first - use mutex to avoid race condition */
            {
                QMutexLocker locker(&audioDeviceMutex);
                if (pAudioInput != nullptr) {
                    // Qt 6: QAudioInput doesn't have stop() method
                    // Just delete the object - cleanup happens automatically
                    delete pAudioInput;
                    pAudioInput = nullptr;
                }
                if (pIODevice != nullptr) {
                    pIODevice->close();
                    pIODevice = nullptr;
                }
            }

            /* Create audio input in current thread (worker thread) */
            bool bFound = false;

            // Qt 6 implementation
            // In Qt 6, QAudioInput no longer takes a QAudioFormat parameter
            // Devices use their preferred format automatically
            foreach(const QAudioDevice& di, QMediaDevices::audioInputs()) {
                if(soundDevice == di.description().toStdString()) {
                    fprintf(stderr, "InitInternal: Found device match, creating QAudioInput...\n");
                    fprintf(stderr, "InitInternal: Using device preferred format\n");
                    /* Create new audio input with mutex protection - Qt 6 doesn't take format parameter */
                    {
                        QMutexLocker locker(&audioDeviceMutex);
                        pAudioInput = new CAudioInput(di);
                    }
                    fprintf(stderr, "InitInternal: QAudioInput created successfully\n");
                    bFound = true;
                    break;
                }
            }
            if (!bFound) {
                fprintf(stderr, "InitInternal: couldn't find device '%s', using default device\n", soundDevice.c_str());
                /* Try to use default device if specified device not found */
                QAudioDevice defaultInfo = QMediaDevices::defaultAudioInput();
                fprintf(stderr, "InitInternal: Using default device with preferred format\n");
                {
                    QMutexLocker locker(&audioDeviceMutex);
                    pAudioInput = new CAudioInput(defaultInfo);
                }
                fprintf(stderr, "InitInternal: Created QAudioInput with default device\n");
                bFound = true;
            }
            bDeviceChanged = false;
            bChanged = true;
        }

        if(pAudioInput) {
            /* Use mutex to protect all pAudioInput access - avoid race condition */
            {
                QMutexLocker locker(&audioDeviceMutex);

# if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                // Qt 6: setBufferSize() method doesn't exist
                // Buffer size is managed automatically by Qt 6
                fprintf(stderr, "InitInternal: Qt 6 buffer size is managed automatically\n");

                // Qt 6: QAudioInput API is simplified
                // In Qt 6, just check if we need to start the audio input
                bool needRestart = (pIODevice == nullptr);

                if(needRestart) {
                    fprintf(stderr, "InitInternal: Starting QAudioInput for device '%s'...\n", soundDevice.c_str());

                    /* In Qt 6, audio input needs to be recreated if there's an issue */
                    if(pIODevice != nullptr) {
                        fprintf(stderr, "InitInternal: Cleaning up existing QIODevice before restart\n");
                        pIODevice->close();
                        pIODevice = nullptr;
                    }

                    /* In Qt 6, QAudioInput API is different - use start() method to get QIODevice */
                    pIODevice = pAudioInput->start();

                    /* In Qt 6, we check if the QIODevice is valid instead of checking QAudioInput state */
                    if(pIODevice != nullptr && pIODevice->isOpen()) {
                        fprintf(stderr, "InitInternal: QAudioInput started successfully\n");
                        fprintf(stderr, "InitInternal: QIODevice created, open: %s, readable: %s\n",
                               pIODevice->isOpen() ? "yes" : "no",
                               pIODevice->isReadable() ? "yes" : "no");
                    } else {
                        fprintf(stderr, "InitInternal: QAudioInput failed to start\n");

                        /* Try to recover by recreating the QAudioInput */
                        fprintf(stderr, "InitInternal: Attempting to recover...\n");
                        delete pAudioInput;
                        foreach(const QAudioDevice& di, QMediaDevices::audioInputs()) {
                            if(soundDevice == di.description().toStdString()) {
                                pAudioInput = new CAudioInput(di);
                                break;
                            }
                        }
                        if(pAudioInput == nullptr) {
                            QAudioDevice defaultInfo = QMediaDevices::defaultAudioInput();
                            pAudioInput = new CAudioInput(defaultInfo);
                        }
                        pIODevice = pAudioInput->start();

                        if(pIODevice != nullptr && pIODevice->isOpen()) {
                            fprintf(stderr, "InitInternal: Recovery successful, QAudioInput started\n");
                        } else {
                            fprintf(stderr, "InitInternal: Recovery failed\n");
                            pIODevice = nullptr;
                        }
                    }
                } else {
                    fprintf(stderr, "InitInternal: QAudioInput already started\n");
                }

// Qt 6: bufferSize() method doesn't exist
                cerr << "sound card buffer size requested " << 2*wantedBufferSize << " (Qt 6 manages automatically)" << endl;
            } // End QMutexLocker scope
        } // End pAudioInput check
#endif

        /* Clear input data buffer on change samplerate change */
        if (bChanged)
            ClearInputData();

        /* Init 2X upscaler if enabled */
        if (iUpscaleRatio > 1)
        {
            const int taps = (NUM_TAPS_UPSAMPLE_FILT + 3) & ~3;
            vecf_B.resize(taps, 0.0f);
            for (unsigned i = 0; i < NUM_TAPS_UPSAMPLE_FILT; i++)
                vecf_B[i] = float(dUpsampleFilt[i] * iUpscaleRatio);
            if (bChanged)
            {
                vecf_ZL.resize(0);
                vecf_ZR.resize(0);
            }
            vecf_ZL.resize(unsigned(iOutputBlockSize + taps) / 2, 0.0f);
            vecf_ZR.resize(unsigned(iOutputBlockSize + taps) / 2, 0.0f);
            vecf_YL.resize(unsigned(iOutputBlockSize));
            vecf_YR.resize(unsigned(iOutputBlockSize));
        }
        else
        {
            vecf_B.resize(0);
            vecf_YL.resize(0);
            vecf_YR.resize(0);
            vecf_ZL.resize(0);
            vecf_ZR.resize(0);
        }

        /* Init buffer size for taking stereo input */
        vecsSoundBuffer.Init(iOutputBlockSize * 2 / iUpscaleRatio);

        /* Init signal meter */
        SignalLevelMeter.Init(0);

        /* Inits for I / Q input, only if it is not already
           to keep the history intact */
        if (vecrReHist.Size() != NUM_TAPS_IQ_INPUT_FILT || bChanged)
        {
            vecrReHist.Init(NUM_TAPS_IQ_INPUT_FILT, 0.0);
            vecrImHist.Init(NUM_TAPS_IQ_INPUT_FILT, 0.0);
        }

        /* Start with phase null (can be arbitrarily chosen) */
        cCurExp = 1.0;

        /* Set rotation vector to mix signal from zero frequency to virtual
           intermediate frequency */
        const _REAL rNormCurFreqOffsetIQ = 2.0 * crPi * VIRTUAL_INTERMED_FREQ / _REAL(iSampleRate);

        cExpStep = _COMPLEX(cos(rNormCurFreqOffsetIQ), sin(rNormCurFreqOffsetIQ));


        /* OPH: init free-running symbol counter */
        iFreeSymbolCounter = 0;

        fprintf(stderr, "InitInternal: Initialization completed successfully\n");
    }
    catch (CGenErr GenErr)
    {
        fprintf(stderr, "InitInternal: Caught CGenErr: %s\n", GenErr.strError.c_str());
        pSound = nullptr;
    }
    catch (string strError)
    {
        fprintf(stderr, "InitInternal: Caught string exception: %s\n", strError.c_str());
        pSound = nullptr;
    }
    catch (const std::exception& e)
    {
        fprintf(stderr, "InitInternal: Caught std::exception: %s\n", e.what());
        /* Don't exit, just log and continue */
    }
    catch (...)
    {
        fprintf(stderr, "InitInternal: Caught unknown exception\n");
        /* Don't exit, just log and continue */
    }
}

_REAL CReceiveData::HilbertFilt(const _REAL rRe, const _REAL rIm)
{
    /*
        Hilbert filter for I / Q input data. This code is based on code written
        by Cesco (HB9TLK)
    */

    /* Move old data */
    for (int i = 0; i < NUM_TAPS_IQ_INPUT_FILT - 1; i++)
    {
        vecrReHist[i] = vecrReHist[i + 1];
        vecrImHist[i] = vecrImHist[i + 1];
    }

    vecrReHist[NUM_TAPS_IQ_INPUT_FILT - 1] = rRe;
    vecrImHist[NUM_TAPS_IQ_INPUT_FILT - 1] = rIm;

    /* Filter */
    _REAL rSum = 0.0;
    for (unsigned i = 1; i < NUM_TAPS_IQ_INPUT_FILT; i += 2)
        rSum += _REAL(fHilFiltIQ[i]) * vecrImHist[int(i)];

    return (rSum + vecrReHist[IQ_INP_HIL_FILT_DELAY]) / 2;
}

void CReceiveData::InterpFIR_2X(const int channels, _SAMPLE* X, vector<float>& Z, vector<float>& Y, vector<float>& B)
{
    /*
        2X interpolating filter. When combined with CS_IQ_POS_SPLIT or CS_IQ_NEG_SPLIT
        input data mode, convert I/Q input to full bandwidth, code by David Flamand
    */
    int i, j;
    const int B_len = int(B.size());
    const int Z_len = int(Z.size());
    const int Y_len = int(Y.size());
    const int Y_len_2 = Y_len / 2;
    float *B_beg_ptr = &B[0];
    float *Z_beg_ptr = &Z[0];
    float *Y_ptr = &Y[0];
    float *B_end_ptr, *B_ptr, *Z_ptr;
    float y0, y1, y2, y3;

    /* Check for size and alignment requirement */
    if ((B_len & 3) || (Z_len != (B_len/2 + Y_len_2)) || (Y_len & 1))
        return;

    /* Copy the old history at the end */
    for (i = B_len/2-1; i >= 0; i--)
        Z_beg_ptr[Y_len_2 + i] = Z_beg_ptr[i];

    /* Copy the new sample at the beginning of the history */
    for (i = 0, j = 0; i < Y_len_2; i++, j+=channels)
        Z_beg_ptr[Y_len_2 - i - 1] = X[j];

    /* The actual lowpass filtering using FIR */
    for (i = Y_len_2-1; i >= 0; i--)
    {
        B_end_ptr  = B_beg_ptr + B_len;
        B_ptr      = B_beg_ptr;
        Z_ptr      = Z_beg_ptr + i;
        y0 = y1 = y2 = y3 = 0.0f;
        while (B_ptr != B_end_ptr)
        {
            y0 = y0 + B_ptr[0] * Z_ptr[0];
            y1 = y1 + B_ptr[1] * Z_ptr[0];
            y2 = y2 + B_ptr[2] * Z_ptr[1];
            y3 = y3 + B_ptr[3] * Z_ptr[1];
            B_ptr += 4;
            Z_ptr += 2;
        }
        *Y_ptr++ = y0 + y2;
        *Y_ptr++ = y1 + y3;
    }
}

/*
    Convert Real to I/Q frequency when bInvert is false
    Convert I/Q to Real frequency when bInvert is true
*/
_REAL CReceiveData::ConvertFrequency(_REAL rFrequency, bool bInvert) const
{
    const int iInvert = bInvert ? -1 : 1;

    if (eInChanSelection == CS_IQ_POS_SPLIT ||
            eInChanSelection == CS_IQ_NEG_SPLIT)
        rFrequency -= iSampleRate / 4 * iInvert;
    else if (eInChanSelection == CS_IQ_POS_ZERO ||
             eInChanSelection == CS_IQ_NEG_ZERO)
        rFrequency -= VIRTUAL_INTERMED_FREQ * iInvert;

    return rFrequency;
}

void CReceiveData::GetInputSpec(CVector<_REAL>& vecrData, CVector<_REAL>& vecrScale)
{
    spectrumAnalyser.setNegativeFrequency(eInChanSelection == CS_IQ_POS_SPLIT || eInChanSelection == CS_IQ_NEG_SPLIT);
    spectrumAnalyser.setOffsetFrequency((eInChanSelection == CS_IQ_POS_ZERO) || (eInChanSelection == CS_IQ_NEG_ZERO));
    mutexInpData.Lock();
    spectrumAnalyser.CalculateSpectrum(vecrInpData, NUM_SMPLS_4_INPUT_SPECTRUM);
    mutexInpData.Unlock();
    /* The calibration factor of 18.49 was determined experimentaly,
       give 0 dB for a full scale sine wave input (0 dBFS) */

    const _REAL rNormData = pow(_REAL(_MAXSHORT) * _REAL(NUM_SMPLS_4_INPUT_SPECTRUM),2) / 18.49;
    spectrumAnalyser.PSD2LogPSD(rNormData, vecrData, vecrScale);
}

void CReceiveData::GetInputPSD(CVector<_REAL>& vecrData, CVector<_REAL>& vecrScale,
                 const int iLenPSDAvEachBlock,
                 const int iNumAvBlocksPSD,
                 const int iPSDOverlap)
{
    spectrumAnalyser.setNegativeFrequency(eInChanSelection == CS_IQ_POS_SPLIT || eInChanSelection == CS_IQ_NEG_SPLIT);
    spectrumAnalyser.setOffsetFrequency((eInChanSelection == CS_IQ_POS_ZERO) || (eInChanSelection == CS_IQ_NEG_ZERO));
    mutexInpData.Lock();
    spectrumAnalyser.CalculateLinearPSD(vecrInpData, iLenPSDAvEachBlock, iNumAvBlocksPSD, iPSDOverlap);
    mutexInpData.Unlock();

    const _REAL rNormData =  pow(_REAL(_MAXSHORT) * _REAL(iLenPSDAvEachBlock), 2) * _REAL(iNumAvBlocksPSD) * PSDWindowGain;

    spectrumAnalyser.PSD2LogPSD(rNormData, vecrData, vecrScale);
}

void CReceiveData::emitRSCIData(CParameter& Parameters)
{
    /* Init the constants for scale and normalization */
    spectrumAnalyser.setNegativeFrequency(eInChanSelection == CS_IQ_POS_SPLIT || eInChanSelection == CS_IQ_NEG_SPLIT);
    spectrumAnalyser.setOffsetFrequency((eInChanSelection == CS_IQ_POS_ZERO) || (eInChanSelection == CS_IQ_NEG_ZERO));
    mutexInpData.Lock();
    spectrumAnalyser.CalculateLinearPSD(vecrInpData, LEN_PSD_AV_EACH_BLOCK_RSI, NUM_AV_BLOCKS_PSD_RSI, PSD_OVERLAP_RSI);
    mutexInpData.Unlock();


    const _REAL rNormData =  pow(_REAL(_MAXSHORT) * _REAL(LEN_PSD_AV_EACH_BLOCK_RSI), 2) * _REAL(NUM_AV_BLOCKS_PSD_RSI) * PSDWindowGain;

    CVector<_REAL>		vecrData;
    CVector<_REAL>		vecrScale;
    spectrumAnalyser.PSD2LogPSD(rNormData, vecrData, vecrScale);

    /* Data required for rpsd tag */
    /* extract the values from -8kHz to +8kHz/18kHz relative to 12kHz, i.e. 4kHz to 20kHz */
    /*const int startBin = 4000.0 * LEN_PSD_AV_EACH_BLOCK_RSI /iSampleRate;
    const int endBin = 20000.0 * LEN_PSD_AV_EACH_BLOCK_RSI /iSampleRate;*/
    /* The above calculation doesn't round in the way FhG expect. Probably better to specify directly */

    /* For 20k mode, we need -8/+18, which is more than the Nyquist rate of 24kHz. */
    /* Assume nominal freq = 7kHz (i.e. 2k to 22k) and pad with zeroes (roughly 1kHz each side) */

    int iStartBin = 22;
    int iEndBin = 106;
    int iVecSize = iEndBin - iStartBin + 1; //85

    //_REAL rIFCentreFrequency = Parameters.FrontEndParameters.rIFCentreFreq;

    ESpecOcc eSpecOcc = Parameters.GetSpectrumOccup();
    if (eSpecOcc == SO_4 || eSpecOcc == SO_5)
    {
        iStartBin = 0;
        iEndBin = 127;
        iVecSize = 139;
    }
    /* Line up the the middle of the vector with the quarter-Nyquist bin of FFT */
    int iStartIndex = iStartBin - (LEN_PSD_AV_EACH_BLOCK_RSI/4) + (iVecSize-1)/2;

    /* Fill with zeros to start with */
    Parameters.vecrPSD.Init(iVecSize, 0.0);

    for (int i=iStartIndex, j=iStartBin; j<=iEndBin; i++,j++)
        Parameters.vecrPSD[i] = vecrData[j];

    spectrumAnalyser.CalculateSigStrengthCorrection(Parameters, vecrData);

    spectrumAnalyser.CalculatePSDInterferenceTag(Parameters, vecrData);
}

