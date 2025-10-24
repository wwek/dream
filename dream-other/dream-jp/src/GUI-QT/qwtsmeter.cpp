#include "qwtsmeter.h"

QwtSMeter::QwtSMeter(QWidget *parent) :
    QwtThermo(parent)
{
# if QWT_VERSION < 0x060100
    setRange(S_METER_THERMO_MIN, S_METER_THERMO_MAX);
    setScale(S_METER_THERMO_MIN, S_METER_THERMO_MAX, 10.0);
    setScalePosition(QwtThermo::TopScale);
    setOrientation(Qt::Horizontal, QwtThermo::TopScale);
# else
    setScale(S_METER_THERMO_MIN, S_METER_THERMO_MAX);
    setScaleStepSize(10.0);
    setScalePosition(QwtThermo::TrailingScale);
    setOrientation(Qt::Horizontal);
# endif
    setAlarmLevel(S_METER_THERMO_ALARM);
    setAlarmLevel(-12.5);
    setAlarmEnabled(true);
    setValue(S_METER_THERMO_MIN);
# if QWT_VERSION < 0x060000
    setAlarmColor(QColor(255, 0, 0));
    setFillColor(QColor(0, 190, 0));
# else
    QPalette newPalette;
    if(parent)
        newPalette = parent->palette();
    newPalette.setColor(QPalette::Base, newPalette.color(QPalette::Window));
    newPalette.setColor(QPalette::ButtonText, QColor(0, 190, 0));
    newPalette.setColor(QPalette::Highlight,  QColor(255, 0, 0));
    setPalette(newPalette);
# endif
}
