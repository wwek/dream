#ifndef AMWIDGET_H
#define AMWIDGET_H

#include <QWidget>

namespace Ui {
class AMWidget;
}

class ReceiverController;
class QShowEvent;
class QHideEvent;
class CDRMPlot;

class AMWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AMWidget(ReceiverController*, QWidget *parent = 0);
    ~AMWidget();
    void connectController(ReceiverController*);

private:
    Ui::AMWidget *ui;
    CDRMPlot* MainPlot;
    ReceiverController* controller;
    void AddWhatsThisHelp();
    void showEvent(QShowEvent* pEvent);
    void hideEvent(QHideEvent* pEvent);

public slots:
    void UpdatePlotStyle(int);
    void OnSampleRateChanged();
    void OnRadioDemodulation(int iID);
    void OnRadioAGC(int iID);
    void OnChartxAxisValSet(double dVal);
    void on_sliderBandwidth_valueChanged(int value);
    void OnRadioNoiRed(int iID);
    void on_checkBoxWaterFall_stateChanged(int);
    void on_checkBoxAutoFreqAcq_stateChanged(int);
    void on_checkBoxPLL_stateChanged(int);
    void on_buttonFreqOffset_clicked();
    void on_spinBoxNoiRedLevel_valueChanged(int value);
    void on_amFilterBandwidthChanged(int value);
    void on_new_data();

signals:
    void modulation(int);
    void noiseReductionType(int);
    void noiseReductionLevel(int);
    void AGC(int);
    void AMFilterBW(int);
    void enableAutoFreqAcq(bool);
    void enablePLL(bool);
    void AMDemodAcq(double);
};

#endif // AMWIDGET_H
