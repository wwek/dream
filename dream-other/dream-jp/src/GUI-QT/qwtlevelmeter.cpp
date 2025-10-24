#include "qwtlevelmeter.h"

QwtLevelMeter::QwtLevelMeter(QWidget *parent) :
    QwtThermo(parent)
{
# if QWT_VERSION < 0x060100
    setRange(-50.0, 0.0);
    setOrientation(Qt::Vertical, QwtThermo::LeftScale);
# else
    setScale(-50.0, 0.0);
    setOrientation(Qt::Vertical);
    setScalePosition(QwtThermo::TrailingScale);
# endif
    setAlarmLevel(-12.5);
    QColor alarmColor(QColor(255, 0, 0));
    QColor fillColor(QColor(0, 190, 0));
# if QWT_VERSION < 0x060000
    setAlarmColor(alarmColor);
    setFillColor(fillColor);
# else
    QPalette newPalette = palette();
    newPalette.setColor(QPalette::Base, newPalette.color(QPalette::Window));
    newPalette.setColor(QPalette::ButtonText, fillColor);
    newPalette.setColor(QPalette::Highlight, alarmColor);
    setPalette(newPalette);
# endif

}
