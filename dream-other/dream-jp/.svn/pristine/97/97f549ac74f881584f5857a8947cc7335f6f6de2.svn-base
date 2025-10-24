#ifndef RECEIVERCONTROLLER_H
#define RECEIVERCONTROLLER_H

#include <QDateTime>
#include <QObject>
#include <../DrmReceiver.h>
#include <../util/Settings.h>

struct Reception {
    double snr, mer, wmer;
    double sigmaEstimate;
    double minDelay;
    double sampleOffset;
    int sampleRate;
    double dcOffset;
    double rdop;
};

struct ChannelConfiguration {
    int robm;
    int mode;
    int interl;
    int sdcConst, mscConst;
    CMSCProtLev protLev;
    int nData, nAudio;
};

class ReceiverController : public QObject, public CController
{
    Q_OBJECT
public:
    explicit ReceiverController(CDRMReceiver*, CSettings&, QObject *parent = 0);
    // TODO - remove this:
    CDRMReceiver* getReceiver() {
        return receiver;
    }
    int getMode() const;

signals:
    void serviceChanged(int, const CService&);
    void newDataService(int);
    void dataStatusChanged(int, ETypeRxStatus);
    void frequencyChanged(int);
    void mode(int);
    void position(double,double);
    void AFS(const CAltFreqSign&);
    void setAFS(bool);
    void serviceInformation(const CServiceInformation&);
    void textMessageChanged(int, QString);
    void MSCChanged(ETypeRxStatus);
    void SDCChanged(ETypeRxStatus);
    void FACChanged(ETypeRxStatus);
    void FSyncChanged(ETypeRxStatus);
    void TSyncChanged(ETypeRxStatus);
    void InputStatusChanged(ETypeRxStatus);
    void OutputStatusChanged(ETypeRxStatus);
    void InputSignalLevelChanged(double);
    void signalLost();
    void channelReceptionChanged(Reception&);
    void channelConfigurationChanged(ChannelConfiguration&);
    void timeIntChanged(int);
    void freqIntChanged(int);
    void tiSyncTracTypeChanged(int);
    void numMSCMLCIterationsChanged(int);
    void flippedSpectrumChanged(bool);
    void recFilterChanged(bool);
    void intConsChanged(bool);
    void amFilterBandwidthChanged(int);
    void DRMTimeChanged(QDateTime);
    void dataAvailable();

public slots:
    void setControls();
    void selectDataService(int);
    void selectAudioService(int);
    void triggerNewAcquisition();
    void setMode(int);
    void setRobustnesMode(ERobMode eRobMode);
    void setFrequency(int);
    void muteAudio(bool);
    void setSaveAudio(const string&);
    void setTimeInt(int);
    void setFreqInt(int);
    void setTiSyncTracType(int);
    void setNumMSCMLCIterations(int);
    void setFlippedSpectrum(bool);
    void setReverbEffect(bool);
    void setRecFilter(bool);
    void setIntCons(bool);
    void setAnalogModulation(int i);
    void setAnalogAGC(int i);
    void setAnalogNoiseReduction(int i);
    void setAMFilterBW(int value);
    void setEnableAutoFreqAcq(bool b);
    void setEnablePLL(bool b);
    void setAMDemodAcq(double dVal);
    void setNoiRedLevel(int value);

private:
    CDRMReceiver* receiver;
    int           iCurrentFrequency;
    int           currentAMfilterBW;
    ERecMode      currentMode;
    CSettings&    settings;
    QDateTime     drmTime;

private slots:
    void on_new_data();
    void updateDRM(CParameter&);
};

#endif // RECEIVERCONTROLLER_H
