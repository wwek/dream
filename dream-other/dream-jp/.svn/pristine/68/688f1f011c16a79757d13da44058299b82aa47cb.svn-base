#ifndef CHANNELWIDGET_H
#define CHANNELWIDGET_H

#include <QWidget>
#include <QTreeWidgetItem>
#include <../enumerations.h>

class CDRMPlot;
class ReceiverController;
struct Reception;
struct ChannelConfiguration;

namespace Ui {
class ChannelWidget;
}

class ChannelWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChannelWidget(ReceiverController*, QWidget *parent = nullptr);
    ~ChannelWidget();
    void connectController(ReceiverController*);

private:
    Ui::ChannelWidget *ui;
    CDRMPlot *pMainPlot;
    int sr;
public slots:
    void setLEDFAC(ETypeRxStatus);
    void setLEDSDC(ETypeRxStatus);
    void setLEDFrameSync(ETypeRxStatus status);
    void setLEDTimeSync(ETypeRxStatus status);
    void setLEDIOInterface(ETypeRxStatus status);
    void setSNR(double rSNR);
    void setMER(double rMER, double rWMERMSC);
    void setDelay_Doppler(double rSigmaEstimate, double rMinDelay);
    void setSampleFrequencyOffset(double rCurSamROffs, double rSampleRate);
    void setFrequencyOffset(double);
    void setChannel(ERobMode, ESpecOcc, ESymIntMod, ECodScheme, ECodScheme);
    void setCodeRate(int,int);
    void setPlotStyle(int);
    void on_channelConfigurationChanged(ChannelConfiguration&);
    void on_channelReceptionChanged(Reception&);
    void setNumIterations(int);
    void setTimeInt(int);
    void setFreqInt(int);
    void setTiSyncTrac(int);
    void setRecFilterEnabled(bool);
    void setIntConsEnabled(bool);
    void setFlipSpectrumEnabled(bool);
    void on_new_data();

private slots:
    void on_chartSelector_currentItemChanged(QTreeWidgetItem *);
    void on_showOptions_toggled(bool);
    void showEvent(QShowEvent*);
    void hideEvent(QHideEvent*);

signals:
};

#endif // ChannelWidget_H
