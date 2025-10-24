#ifndef DRMDETAIL_H
#define DRMDETAIL_H

#include <QWidget>
#include <../Parameter.h>
#include "MultColorLED.h"

namespace Ui {
class DRMDetail;
}

class DRMDetail : public QWidget
{
    Q_OBJECT

public:
    explicit DRMDetail(QWidget *parent = 0);
    ~DRMDetail();
    void updateDisplay(CParameter& Parameters, _REAL freqOffset, EAcqStat acqState, bool rsciMode);
    void hideMSCParams(bool);

public slots:
    void setLEDFAC(ETypeRxStatus);
    void setLEDSDC(ETypeRxStatus status);
    void setLEDMSC(ETypeRxStatus status);
    void setLEDFrameSync(ETypeRxStatus status);
    void setLEDTimeSync(ETypeRxStatus status);
    void setLEDIOInterface(ETypeRxStatus status);
    void setNumServices(int, int);
    void setSNR(double rSNR);
    void setMER(double rMER, double rWMERMSC);
    void setDelay_Doppler(double rSigmaEstimate, double rMinDelay);
    void setSampleFrequencyOffset(double rCurSamROffs, double rSampleRate);
    void setFrequencyOffset(double);
    void setChannel(ERobMode, ESpecOcc, ESymIntMod, ECodScheme, ECodScheme);
    void setCodeRate(int iPartB, int iPartA);

private:
    Ui::DRMDetail *ui;
    QString GetRobModeStr(ERobMode e);
    QString GetSpecOccStr(ESpecOcc e, ERobMode m);
    void AddWhatsThisHelp();
};

#endif // DRMDETAIL_H
