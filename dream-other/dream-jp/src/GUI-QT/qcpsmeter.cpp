#include "qcpsmeter.h"

QCPSMeter::QCPSMeter(QWidget* parent):QCustomPlot(parent)
{
    xAxis2->setRange(S_METER_THERMO_MIN, S_METER_THERMO_MAX);
    //xAxis2->setTickStep(10.0);
    xAxis2->ticker()->setTickCount(5);
    xAxis->setVisible(false);
    xAxis2->setVisible(true);
    yAxis->setVisible(false);
    yAxis2->setVisible(false);

    QCPBars *bars = new QCPBars(yAxis, xAxis2);
    //addPlottable(bars);
    bars->setName("dB");
    bars->setWidth(6);
    bars->setPen(Qt::NoPen);
    bars->setBrush(Qt::green);
    setMinimumWidth(200);
    QVector<double> v, k;
    v << S_METER_THERMO_MAX;
    k << 0;
    bars->setData(k, v);

    setBackground(QBrush(QColor(240,240,240)));
}

void QCPSMeter::setLevel(double d)
{
    QVector<double> v, k;
    d += 50.0;
    v << d;
    k << 0;
    QCPAbstractPlottable* bars = plottable(0);
    qobject_cast<QCPBars*>(bars)->setData(k, v);
    replot();
}
