#include "qcplevelmeter.h"
#include <QFont>

QCPLevelMeter::QCPLevelMeter(QWidget* parent):QCustomPlot(parent)
{
    QFont font;
    if(parent)
        font = parent->font();
    font.setPointSizeF(7.0);
    QPen pen;
    pen.setWidthF(1.2);
    pen.setColor(Qt::red);
    setBackground(Qt::black);
    yAxis->setPadding(0);

    axisRect(0)->setAutoMargins(QCP::msLeft);
    axisRect(0)->setMargins(QMargins(0,7,3,5));

    QVector<QString> labels;
    labels << "-50" << "0";
    QVector<double> ticks;
    ticks << 0 << 50;
    yAxis->setRange(0, 50);
    //yAxis->setAutoTicks(false);
    //yAxis->setAutoSubTicks(false);
    //yAxis->setAutoTickLabels(false);
    //yAxis->setTickVector(ticks);
    yAxis->ticker()->setTickCount(2);
    //yAxis->setTickVectorLabels(labels);
    yAxis->setBasePen(pen);
    yAxis->setTickLengthIn(0);
    yAxis->setTickLengthOut(6);
    yAxis->setTickPen(pen);
    yAxis->setTickLabelPadding(1);
    yAxis->setTickLabelColor(Qt::red);
    yAxis->setTickLabelFont(font);
    //yAxis->ticker()->setSubTickCount(4);
    yAxis->setSubTickLengthIn(0);
    yAxis->setSubTickLengthOut(3);
    yAxis->setSubTickPen(pen);
    yAxis->setOffset(3);
    yAxis->grid()->setVisible(false);

    xAxis->setRange(0, 0.5);
    xAxis->setVisible(false);

    QCPBars *bars = new QCPBars(xAxis, yAxis);
    //addPlottable(bars);
    bars->setName("dB");
    bars->setPen(pen);
    bars->setWidth(0.5);
    bars->setBrush(Qt::green);//QColor(0, 255, 0, 100));
    QVector<double> k,v;
    v << 20.0;
    k << 0;
    bars->setData(k, v);
}

void QCPLevelMeter::setLevel(double d)
{
    QVector<double> v, k;
    d += 50.0;
    v << d;
    k << 0;
    QCPAbstractPlottable* bars = plottable(0);
    qobject_cast<QCPBars*>(bars)->setData(k, v);
    replot();
}
