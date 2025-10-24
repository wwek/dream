#ifndef AUDIODETAILWIDGET_H
#define AUDIODETAILWIDGET_H

#include <QWidget>
#include "../Parameter.h"

namespace Ui {
class AudioDetailWidget;
}

class CDRMPlot;
class ReceiverController;

class AudioDetailWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AudioDetailWidget(ReceiverController*, QWidget * = 0);
    ~AudioDetailWidget();

signals:
    void listen(int);
public slots:
    void setPlotStyle(int);
    void setRxStatus(int, ETypeRxStatus);
    void setTextMessage(const QString&);
    void updateDisplay(int, CService);
    void setEngineering(bool);
    void on_new_data();
private:
    Ui::AudioDetailWidget *ui;
    int short_id;
    bool engineeringMode;
    CDRMPlot *pMainPlot;
    ReceiverController* controller;
    int iPlotStyle;
    void updateEngineeringModeDisplay(int, const CService&);
    void updateUserModeDisplay(int, const CService&);
    void addItem(const QString&, const QString&);
private slots:
    void on_buttonListen_clicked();
    void on_mute_stateChanged(int);
    void on_reverb_stateChanged(int);
    void on_save_stateChanged(int);
};

#endif // AUDIODETAILWIDGET_H
