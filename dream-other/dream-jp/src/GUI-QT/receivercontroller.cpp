#include "receivercontroller.h"
#include <QDebug>
#include <QTime>

ReceiverController::ReceiverController(CDRMReceiver* p, CSettings& s, QObject *parent) :
    QObject(parent),
    receiver(p),
    iCurrentFrequency(-1),
    currentMode(RM_NONE),
    settings(s),
    drmTime()
{
    setObjectName("controller");
    connect(this, SIGNAL(dataAvailable()), this, SLOT(on_new_data()), Qt::QueuedConnection);
}

void ReceiverController::setControls()
{
    // initialise from settings here so multiple panels are synced
    emit numMSCMLCIterationsChanged(settings.Get("Receiver", "mlciter", 1));
    emit timeIntChanged(settings.Get("Receiver", "timeint", 1));
    emit freqIntChanged(settings.Get("Receiver", "freqint", 1));
    emit tiSyncTracTypeChanged(settings.Get("Receiver", "timesync", 1));

    emit recFilterChanged(settings.Get("Receiver", "filter", false));
    emit intConsChanged(settings.Get("Receiver", "modemetric", false));
    emit flippedSpectrumChanged(settings.Get("Receiver", "flipspectrum", false));
}

void ReceiverController::on_new_data()
{
    //qDebug() << QTime::currentTime().toString("hh:mm:ss.zzz");

    ERecMode newMode = receiver->GetReceiverMode();
    if(newMode!=currentMode)
    {
        currentMode = newMode;
        emit mode(currentMode);
    }

    CParameter& Parameters = *receiver->GetParameters();
    Parameters.Lock();

    /* Input level meter */
    double sigstr = Parameters.GetIFSignalLevel();
    gps_data_t gps_data = Parameters.gps_data;
    if(gps_data.status&LATLON_SET)
        emit position(gps_data.fix.latitude, gps_data.fix.longitude);

    Parameters.Unlock();

    if(currentMode == RM_DRM)
    {
        Parameters.Lock();
        /* LEDs */
        ETypeRxStatus msc,sdc,fac,fsync,tsync,inp,outp;
        fac = Parameters.ReceiveStatus.FAC.GetStatus();
        sdc = Parameters.ReceiveStatus.SDC.GetStatus();
        int iShortID = Parameters.GetCurSelAudioService();
        if(Parameters.Service[iShortID].eAudDataFlag == CService::SF_AUDIO)
            msc = Parameters.AudioComponentStatus[iShortID].GetStatus();
        else
            msc = Parameters.DataComponentStatus[iShortID].GetStatus();
        fsync = Parameters.ReceiveStatus.FSync.GetStatus();
        tsync = Parameters.ReceiveStatus.TSync.GetStatus();
        inp = Parameters.ReceiveStatus.InterfaceI.GetStatus();
        outp = Parameters.ReceiveStatus.InterfaceO.GetStatus();

        /* detect if AFS mux information is available */
        bool bAFS = (Parameters.AltFreqSign.vecMultiplexes.size() > 0)
                    || (Parameters.AltFreqSign.vecOtherServices.size() > 0);

        CServiceInformation si = Parameters.ServiceInformation;

        Reception reception;

        reception.snr = Parameters.GetSNR();
        reception.mer = Parameters.rMER;
        reception.wmer = Parameters.rWMERMSC;
        reception.sigmaEstimate = Parameters.rSigmaEstimate;
        reception.minDelay = Parameters.rMinDelay;
        reception.sampleOffset = Parameters.rResampleOffset;
        reception.sampleRate = Parameters.GetSigSampleRate();
        reception.dcOffset = receiver->GetReceiveData()->ConvertFrequency(Parameters.GetDCFrequency());
        reception.rdop = Parameters.rRdop;

        ChannelConfiguration channel;

        channel.robm = Parameters.GetWaveMode();
        channel.mode = Parameters.GetSpectrumOccup();
        channel.interl = Parameters.eSymbolInterlMode;
        channel.sdcConst = Parameters.eSDCCodingScheme;
        channel.mscConst = Parameters.eMSCCodingScheme;
        channel.protLev = Parameters.MSCPrLe;
        channel.nAudio = Parameters.iNumAudioService;
        channel.nData = Parameters.iNumDataService;

        Parameters.Unlock();


        emit InputSignalLevelChanged(sigstr);
        emit FACChanged(fac);
        emit SDCChanged(sdc);
        emit MSCChanged(msc);
        emit FSyncChanged(fsync);
        emit TSyncChanged(tsync);
        emit InputStatusChanged(inp);
        emit OutputStatusChanged(outp);

        emit setAFS(bAFS);
        // NB following is not efficient - TODO - have a better idea
        if(bAFS)
            emit AFS(Parameters.AltFreqSign);
        emit serviceInformation(si);

        /* Check if receiver does receive a signal */
        if(receiver->GetAcquiState() == AS_WITH_SIGNAL)
        {
            updateDRM(Parameters);

            emit channelReceptionChanged(reception);
            emit channelConfigurationChanged(channel);

            int iFreq = receiver->GetFrequency();
            if(iFreq != iCurrentFrequency)
            {
                emit frequencyChanged(iFreq);
                iCurrentFrequency = iFreq;
            }
        }
        else
        {
            emit signalLost();
        }
    }

    if(currentMode == RM_AM)
    {
        Parameters.Lock();
        // TODO fetch
        Parameters.Unlock();
        // TODO emit
        int newAMfilterBW = receiver->GetAMDemod()->GetFilterBW();
        if(newAMfilterBW != currentAMfilterBW)
        {
            currentAMfilterBW = newAMfilterBW;
            emit amFilterBandwidthChanged(currentAMfilterBW);
        }
    }

    if(currentMode == RM_FM)
    {
        Parameters.Lock();
        // TODO fetch
        Parameters.Unlock();
        // TODO emit
    }


    /* Time, date ####################*/
    QDate d(Parameters.iYear, Parameters.iMonth, Parameters.iDay);
    QTime t(Parameters.iUTCHour, Parameters.iUTCMin);
    QDateTime dt(d, t, Qt::UTC);
    if(dt != drmTime)
    {
        drmTime = dt;
        emit DRMTimeChanged(dt);
    }
}



void ReceiverController::updateDRM(CParameter& Parameters)
{
    for(int i=0; i < MAX_NUM_SERVICES; i++)
    {
        Parameters.Lock();
        CService& service = Parameters.Service[i];
        service.AudioParam.rBitRate = Parameters.GetBitRateKbps(i, false);
        service.AudioParam.bCanDecode = receiver->GetAudSorceDec()->CanDecode(service.AudioParam.eAudioCoding);
        service.DataParam.rBitRate = Parameters.GetBitRateKbps(i, true);
        service.DataParam.pDecoder = receiver->GetDataDecoder();
        ETypeRxStatus e = Parameters.DataComponentStatus[i].GetStatus();
        Parameters.Unlock();

        emit serviceChanged(i, service); // TODO emit only when changed

        /* If we have text messages */
        QString textMessage = "";
        if (service.AudioParam.bTextflag)
        {
            // Text message of current selected audio service (UTF-8 decoding)
            textMessage = QString::fromUtf8(service.AudioParam.strTextMessage.c_str());
        }
        emit textMessageChanged(i, textMessage);

        if(service.DataParam.iStreamID != STREAM_ID_NOT_USED)
            emit dataStatusChanged(i, e);
    }
}

void ReceiverController::setFrequency(int f)
{
    receiver->SetFrequency(f);
}

void ReceiverController::triggerNewAcquisition()
{
    receiver->RequestNewAcquisition();
}

void ReceiverController::setMode(int newMode)
{
    receiver->SetReceiverMode(ERecMode(newMode));
}

void ReceiverController::setRobustnesMode(ERobMode eRobMode)
{
    receiver->GetParameters()->SetWaveMode(eRobMode);
}

int ReceiverController::getMode() const
{
    return currentMode;
}

void ReceiverController::selectAudioService(int shortId)
{
    CParameter& Parameters = *receiver->GetParameters();
    Parameters.Lock();
    Parameters.SetCurSelAudioService(shortId);
    Parameters.Unlock();
}

void ReceiverController::selectDataService(int shortId)
{
    CParameter& Parameters = *receiver->GetParameters();
    Parameters.Lock();
    Parameters.SetCurSelDataService(shortId);
    Parameters.Unlock();
    emit newDataService(shortId);
}

void ReceiverController::setSaveAudio(const string& s)
{
    if(s!="")
        receiver->GetWriteData()->StartWriteWaveFile(s);
    else
        receiver->GetWriteData()->StopWriteWaveFile();
}

void ReceiverController::muteAudio(bool b)
{
    receiver->GetWriteData()->MuteAudio(b);
}

void ReceiverController::setTimeInt(int v)
{
    CChannelEstimation::ETypeIntTime e = CChannelEstimation::ETypeIntTime(v);
    if (receiver->GetTimeInt() != e)
        receiver->SetTimeInt(e);
}

void ReceiverController::setFreqInt(int v)
{
    CChannelEstimation::ETypeIntFreq e = CChannelEstimation::ETypeIntFreq(v);
    if (receiver->GetFreqInt() != e)
        receiver->SetFreqInt(e);
}

void ReceiverController::setTiSyncTracType(int v)
{
    CTimeSyncTrack::ETypeTiSyncTrac e = CTimeSyncTrack::ETypeTiSyncTrac(v);
    if (receiver->GetTiSyncTracType() != e)
        receiver->SetTiSyncTracType(e);
}

void ReceiverController::setNumMSCMLCIterations(int value)
{
    receiver->GetMSCMLC()->SetNumIterations(value);
}

void ReceiverController::setFlippedSpectrum(bool b)
{
    receiver->GetReceiveData()->SetFlippedSpectrum(b);
}

void ReceiverController::setReverbEffect(bool b)
{
    receiver->GetAudSorceDec()->SetReverbEffect(b);
}

void ReceiverController::setRecFilter(bool b)
{
    /* Set parameter in working thread module */
    receiver->GetFreqSyncAcq()->SetRecFilter(b);

    /* If filter status is changed, a new aquisition is necessary */
    receiver->RequestNewAcquisition();
}

void ReceiverController::setIntCons(bool b)
{
    /* Set parameter in working thread module */
    receiver->SetIntCons(b);
}

// AM

void ReceiverController::setAnalogModulation(int i)
{
    receiver->GetAMDemod()->SetDemodType(CAMDemodulation::EDemodType(i));
}

void ReceiverController::setAnalogAGC(int i)
{
    receiver->GetAMDemod()->SetAGCType(CAGC::EType(i));
}

void ReceiverController::setAnalogNoiseReduction(int i)
{
    receiver->GetAMDemod()->SetNoiRedType(CAMDemodulation::ENoiRedType(i));
}

void ReceiverController::setAMFilterBW(int value)
{
    receiver->SetAMFilterBW(value);
}

void ReceiverController::setEnableAutoFreqAcq(bool b)
{
    /* Set parameter in working thread module */
    receiver->GetAMDemod()->EnableAutoFreqAcq(b);
}

void ReceiverController::setEnablePLL(bool b)
{
    /* Set parameter in working thread module */
    receiver->GetAMDemod()->EnablePLL(b);
}

void ReceiverController::setAMDemodAcq(double dVal)
{
    receiver->SetAMDemodAcq(dVal);
}

void ReceiverController::setNoiRedLevel(int value)
{
    receiver->GetAMDemod()->SetNoiRedLevel(value);
}
