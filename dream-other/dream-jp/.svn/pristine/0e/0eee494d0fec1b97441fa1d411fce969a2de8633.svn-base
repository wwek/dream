#include "gpswidget.h"
#include "receivercontroller.h"
#include "ui_gpswidget.h"
#include <ctime>

GPSWidget::GPSWidget(ReceiverController* rc, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GPSWidget)
{
    ui->setupUi(this);
    connectController(rc);
}

GPSWidget::~GPSWidget()
{
    delete ui;
}

void GPSWidget::connectController(ReceiverController* controller)
{
    connect(controller, SIGNAL(position(double,double)), this, SLOT(setPosition(double,double)));
}

void GPSWidget::setPosition(double lat, double lng)
{
    ui->lat->setText(QString().setNum(lat));
    ui->lng->setText(QString().setNum(lng));
}

void GPSWidget::setSpeed(double speed)
{
    ui->speed->setText(QString().setNum(speed));
}

void GPSWidget::setTrack(double track)
{
    ui->heading->setText(QString("%1\260").arg(track));
}

void GPSWidget::setAltitude(double)
{

}

void GPSWidget::setGPSDatTime(double time)
{
    struct tm * p_ts;
    time_t tt = time_t(time);
    p_ts = gmtime(&tt);
    QChar fill('0');
    ui->time->setText( QString("UTC: %1/%2/%3 %4:%5:%6  ")
                       .arg(1900 + p_ts->tm_year)
                       .arg(1 + p_ts->tm_mon, 2, 10, fill)
                       .arg(p_ts->tm_mday, 2, 10, fill)
                       .arg(p_ts->tm_hour, 2, 10, fill)
                       .arg(p_ts->tm_min, 2, 10, fill)
                       .arg(p_ts->tm_sec,2, 10, fill)
                     );
}
