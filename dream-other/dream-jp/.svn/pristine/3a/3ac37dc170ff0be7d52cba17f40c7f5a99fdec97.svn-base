#ifndef GPSWIDGET_H
#define GPSWIDGET_H

#include <QWidget>

namespace Ui {
class GPSWidget;
}

class ReceiverController;

class GPSWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GPSWidget(ReceiverController*, QWidget *parent = 0);
    ~GPSWidget();
    void connectController(ReceiverController*);
public slots:
    void setPosition(double,double);
    void setSpeed(double);
    void setTrack(double);
    void setAltitude(double);
    void setGPSDatTime(double);
signals:
    void position(double,double);

private:
    Ui::GPSWidget *ui;
};

#endif // GPSWIDGET_H
