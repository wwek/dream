/******************************************************************************\
 * Copyright (c) 2001-2014
 *
 * Author(s):
 *  Julian Cable, David Flamand
 *
 * Description:
 *  QT Sound Interface
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

#include "qtaudio.h"
#include <QThread>
#include <QAudioInput>
#include <QAudioOutput>
#include <QAudioDeviceInfo>
#include <QAudioFormat>

/* some system might need this for proper sample rate listing */
#if defined(__linux__) && !defined(__ANDROID__)
# define QTAUDIO_SAMPLERATE_HACK
#endif

#define INPUT_BUFFER_LEN_IN_SEC 2
#define OUTPUT_BUFFER_LEN_IN_SEC 2


// TODO do the conversion if the device support mono only

// On linux with pulseaudio, sample rate can range from 1Hz to 192kHz,
//  but supportedSampleRates() return only standard sample rate (8000 11025 22050 44100 48000).
// TODO isSampleRateSupported() hack is very ugly


/**********************************/
/* QT Audio Common Implementation */

CSoundCommonQT::CSoundCommonQT(bool bInput) :
    bInput(bInput), bDevChanged(false),
    iSampleRate(0), iBufferSize(0), bBlocking(false),
    iFrameSize(0), pIODevice(NULL)
{
}

CSoundCommonQT::~CSoundCommonQT()
{
}

void CSoundCommonQT::sleep(int len) const
{
    /* 1000000 = number of usec. in sec., 2 = channels */
    qint64 timeToSleep = (qint64)len * (1000000 / iFrameSize) / iSampleRate;
    /* timeToSleep is the worst case time, zero is the best case time,
       so let divide this time by 2 to have a mean between worst and best case time,
       pros: latency and jitter on input is improved, and output will have less chance to underflow
       cons: function call increased, increased overhead */
    timeToSleep /= 2;
    if (timeToSleep)
        QThread::usleep((unsigned long)timeToSleep);
}

bool CSoundCommonQT::isSampleRateSupported(const QAudioDeviceInfo &di, int samplerate) const
{
    samplerate = abs(samplerate);
#ifdef QTAUDIO_SAMPLERATE_HACK
    (void)di;
    return samplerate >= 1 && samplerate <= 192000;
#else
    return di.supportedSampleRates().contains(samplerate);
#endif
}

bool CSoundCommonQT::isDeviceGood(const QAudioDeviceInfo &di, const int *desiredsamplerates) const
{
    bool bSampleRateOk = false;
    for (const int* dsr=desiredsamplerates; *dsr; dsr++)
    {
        int samplerate = abs(*dsr);
        if (isSampleRateSupported(di, samplerate))
        {
            bSampleRateOk = true;
            break;
        }
    }
    return
        bSampleRateOk &&
        di.supportedChannelCounts().contains(2) && // TODO
        di.supportedSampleSizes().contains(16) &&
        di.supportedSampleTypes().contains(QAudioFormat::SignedInt) &&
        di.supportedByteOrders().contains(QAudioFormat::LittleEndian) &&
        di.supportedCodecs().contains("audio/pcm");
}

void CSoundCommonQT::setSamplerate(deviceprop& dp, const QAudioDeviceInfo &di, const int *desiredsamplerates) const
{
    dp.samplerates.clear();
    for (const int* dsr=desiredsamplerates; *dsr; dsr++)
    {
        int samplerate = abs(*dsr);
        dp.samplerates[samplerate] = isSampleRateSupported(di, samplerate);
    }
}

QAudioDeviceInfo CSoundCommonQT::getDevice()
{
    QAudioDeviceInfo di;
    if (sDev == DEFAULT_DEVICE_NAME)
        di = bInput ? QAudioDeviceInfo::defaultInputDevice() : QAudioDeviceInfo::defaultOutputDevice();
    else
    {
        QAudio::Mode mode = bInput ? QAudio::AudioInput : QAudio::AudioOutput;
        foreach(const QAudioDeviceInfo& tdi, QAudioDeviceInfo::availableDevices(mode))
        {
            string name(tdi.deviceName().toLocal8Bit().constData());
//            qDebug("CSoundCommonQT::getDevice() %i name=%s", bInput, name.c_str());
            if (name == sDev)
            {
                di = tdi;
                break;
            }
        }
    }
    dumpAudioDeviceInfo(di, "CSoundCommonQT::getDevice(): di");
    dumpAudioFormat(di.preferredFormat(), "CSoundCommonQT::getDevice(): di.preferredFormat()");
    return di;
}

QAudioFormat CSoundCommonQT::getFormat()
{
    QAudioFormat format;
    format.setSampleRate(iSampleRate);
    format.setSampleSize(16);
    format.setSampleType(QAudioFormat::SignedInt);
    format.setChannelCount(2); // TODO
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setCodec("audio/pcm");
    return format;
}

void CSoundCommonQT::dumpAudioDeviceInfo(QAudioDeviceInfo di, const char* text)
{
    string codec;
    for (int i=0; i<di.supportedCodecs().size(); ++i)
        codec += string(di.supportedCodecs().at(i).toLocal8Bit().constData()) + " ";
    string channel;
    QList<int> channellist = di.supportedChannelCounts();
    for (QList<int>::iterator i= channellist.begin(); i != channellist.end(); ++i)
        channel += string(QString::number(*i).toLocal8Bit().constData()) + " ";
    string samplesize;
    QList<int> sizelist = di.supportedSampleSizes();
    for (QList<int>::iterator i= sizelist.begin(); i != sizelist.end(); ++i)
        samplesize += string(QString::number(*i).toLocal8Bit().constData()) + " ";
    string samplerate;
    QList<int> ratelist = di.supportedSampleRates();
    for (QList<int>::iterator i= ratelist.begin(); i != ratelist.end(); ++i)
        samplerate += string(QString::number(*i).toLocal8Bit().constData()) + " ";
    string byteorder;
    QList<QAudioFormat::Endian> orderlist = di.supportedByteOrders();
    for (QList<QAudioFormat::Endian>::iterator i= orderlist.begin(); i != orderlist.end(); ++i)
        switch (*i) {
        case QAudioFormat::BigEndian:
            byteorder += "BigEndian ";
            break;
        case QAudioFormat::LittleEndian:
            byteorder += "LittleEndian ";
            break;
        }
    string sampletype;
    QList<QAudioFormat::SampleType> typelist = di.supportedSampleTypes();
    for (QList<QAudioFormat::SampleType>::iterator i= typelist.begin(); i != typelist.end(); ++i)
        switch (*i) {
        case QAudioFormat::Unknown:
            sampletype += "Unknown ";
            break;
        case QAudioFormat::SignedInt:
            sampletype += "SignedInt ";
            break;
        case QAudioFormat::UnSignedInt:
            sampletype += "UnSignedInt ";
            break;
        case QAudioFormat::Float:
            sampletype += "Float ";
            break;
        }
    qDebug(
        "%s\n"
        "  CSoundCommonQT::dumpAudioDeviceInfo():\n"
        "    input/output = %s\n"
        "    name = %s\n"
        "    codec = %s\n"
        "    channel = %s\n"
        "    sample size = %s\n"
        "    sample rate = %s\n"
        "    byte order = %s\n"
        "    sample type = %s"
        , text
        , bInput ? "input" : "output"
        , string(di.deviceName().toLocal8Bit().constData()).c_str()
        , codec.c_str()
        , channel.c_str()
        , samplesize.c_str()
        , samplerate.c_str()
        , byteorder.c_str()
        , sampletype.c_str()
    );
}

void CSoundCommonQT::dumpAudioFormat(QAudioFormat format, const char* text)
{
    string byteorder;
    switch (format.byteOrder()) {
    case QAudioFormat::BigEndian:
        byteorder += "BigEndian ";
        break;
    case QAudioFormat::LittleEndian:
        byteorder += "LittleEndian ";
        break;
    }
    string sampletype;
    switch (format.sampleType()) {
    case QAudioFormat::Unknown:
        sampletype += "Unknown ";
        break;
    case QAudioFormat::SignedInt:
        sampletype += "SignedInt ";
        break;
    case QAudioFormat::UnSignedInt:
        sampletype += "UnSignedInt ";
        break;
    case QAudioFormat::Float:
        sampletype += "Float ";
        break;
    }
    qDebug(
        "%s\n"
        "  CSoundCommonQT::dumpAudioFormat():\n"
        "    codec = %s\n"
        "    channel = %i\n"
        "    sample size = %i\n"
        "    sample rate = %i\n"
        "    byte order = %s\n"
        "    sample type = %s"
        , text
        , string(format.codec().toLocal8Bit().constData()).c_str()
        , format.channelCount()
        , format.sampleSize()
        , format.sampleRate()
        , byteorder.c_str()
        , sampletype.c_str()
    );
}

void CSoundCommonQT::Enumerate(vector<deviceprop>& devs, const int *desiredsamplerates)
{
    devs.clear();
    deviceprop dp;

    /* Default device */
    const QAudioDeviceInfo& di(bInput ? QAudioDeviceInfo::defaultInputDevice() : QAudioDeviceInfo::defaultOutputDevice());
    if (isDeviceGood(di, desiredsamplerates))
    {
        dp.name = DEFAULT_DEVICE_NAME;
        setSamplerate(dp, di, desiredsamplerates);
        devs.push_back(dp);
    }

    foreach(const QAudioDeviceInfo& di, QAudioDeviceInfo::availableDevices(bInput ? QAudio::AudioInput : QAudio::AudioOutput))
    {
        if (isDeviceGood(di, desiredsamplerates))
        {
            string name(di.deviceName().toLocal8Bit().constData());
//            qDebug("CSoundCommonQT::Enumerate() %i name=%s", bInput, name.c_str());
            dp.name = name;
            setSamplerate(dp, di, desiredsamplerates);
            devs.push_back(dp);
        }
    }
}

string CSoundCommonQT::GetDev()
{
    return sDev;
}

void CSoundCommonQT::SetDev(string sNewDev)
{
//    qDebug("CSoundCommonQT::SetDev() name=%s", sNewDev.c_str());
    if (sDev != sNewDev)
    {
        sDev = sNewDev;
        bDevChanged = true;
    }
}


/******************************/
/* QT Audio In Implementation */

CSoundInQT::CSoundInQT() :
    CSoundCommonQT(true), pAudioInput(NULL)
{
}

CSoundInQT::~CSoundInQT()
{
    Close();
}

bool CSoundInQT::Init(int iNewSampleRate, int iNewBufferSize, bool bNewBlocking)
{
//    qDebug("CSoundInQT::Init() %i %i %i", iNewSampleRate, iNewBufferSize, bNewBlocking);
    bool bChanged = false;
    iBufferSize = iNewBufferSize;
    bBlocking = bNewBlocking;

    if (iSampleRate != iNewSampleRate)
    {
        iSampleRate = iNewSampleRate;
        Close();
        bChanged = true;
    }

    if (bDevChanged || !pAudioInput)
    {
        bDevChanged = false;
        pIODevice = NULL;
        QAudioDeviceInfo di = getDevice();
        QAudioFormat format = getFormat();
        pAudioInput = new QAudioInput(di, format);
        /* make sure we have the requested format */
        if (pAudioInput->format() == format)
        {
            iFrameSize = pAudioInput->format().channelCount() * pAudioInput->format().sampleSize() / 8;
            pAudioInput->setBufferSize(iSampleRate * iFrameSize * INPUT_BUFFER_LEN_IN_SEC);
            QIODevice* pIODevice = pAudioInput->start();
            if (pAudioInput->error() == QAudio::NoError)
                this->pIODevice = pIODevice;
            else
                qDebug("CSoundInQT::Init() Can't open audio input (error = %i)", (int)pAudioInput->error());
        }
        else
            qDebug("CSoundInQT::Init() Can't open audio input (bad format)");
    }
    return bChanged;
}

bool CSoundInQT::Read(CVector<short>& vecsSoundBuffer)
{
    bool bError = true;
    if (bDevChanged)
        Init(iSampleRate, iBufferSize, bBlocking);
    if (pIODevice)
    {
        int len = (int)(sizeof(short) * vecsSoundBuffer.Size());
        for (int pos=0;;) {
            int ret = (int)pIODevice->read((char*)&vecsSoundBuffer[pos], len);
            if (ret < 0)
                break;
            pos += ret/sizeof(short);
            len -= ret;
            if (len <= 0)
                break;
            sleep(len);
        }
        bError = len != 0;
    }
    return bError;
}

void CSoundInQT::Close()
{
//    qDebug("CSoundInQT::Close()");
    if (pAudioInput)
    {
        pAudioInput->stop();
        delete pAudioInput;
        pAudioInput = NULL;
        pIODevice = NULL;
    }

}


/*******************************/
/* QT Audio Out Implementation */

CSoundOutQT::CSoundOutQT() :
    CSoundCommonQT(false), pAudioOutput(NULL)
{
}

CSoundOutQT::~CSoundOutQT()
{
    Close();
}

bool CSoundOutQT::Init(int iNewSampleRate, int iNewBufferSize, bool bNewBlocking)
{
//    qDebug("CSoundOutQT::Init() %i %i %i", iNewSampleRate, iNewBufferSize, bNewBlocking);
    bool bChanged = false;
    iBufferSize = iNewBufferSize;
    bBlocking = bNewBlocking;

    if (iSampleRate != iNewSampleRate)
    {
        iSampleRate = iNewSampleRate;
        Close();
        bChanged = true;
    }

    if (bDevChanged || !pAudioOutput)
    {
        bDevChanged = false;
        pIODevice = NULL;
        QAudioDeviceInfo di = getDevice();
        QAudioFormat format = getFormat();
        pAudioOutput = new QAudioOutput(di, format);
        /* make sure we have the requested format */
        if (pAudioOutput->format() == format)
        {
            iFrameSize = pAudioOutput->format().channelCount() * pAudioOutput->format().sampleSize() / 8;
            pAudioOutput->setBufferSize(iSampleRate * iFrameSize * OUTPUT_BUFFER_LEN_IN_SEC);
            QIODevice* pIODevice = pAudioOutput->start();
            if (pAudioOutput->error() == QAudio::NoError)
                this->pIODevice = pIODevice;
            else
                qDebug("CSoundInQT::Init() Can't open audio output (error = %i)", (int)pAudioOutput->error());
        }
        else
            qDebug("CSoundInQT::Init() Can't open audio output (bad format)");
    }
    return bChanged;
}

bool CSoundOutQT::Write(CVector<short>& vecsSoundBuffer)
{
    bool bError = true;
    if (bDevChanged)
        Init(iSampleRate, iBufferSize, bBlocking);
    if (pIODevice)
    {
        int len = (int)(sizeof(short) * vecsSoundBuffer.Size());
        for (int pos=0;;) {
            int ret = (int)pIODevice->write((char*)&vecsSoundBuffer[pos], len);
            if (ret < 0)
                break;
            pos += ret/sizeof(short);
            len -= ret;
            if (len <= 0)
                break;
            sleep(len);
        }
        bError = len != 0;
    }
    return bError;
}

void CSoundOutQT::Close()
{
//    qDebug("CSoundOutQT::Close()");
    if (pAudioOutput)
    {
        pAudioOutput->stop();
        delete pAudioOutput;
        pAudioOutput = NULL;
        pIODevice = NULL;
    }
}

