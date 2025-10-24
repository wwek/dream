#include "amspectrumplot.h"

AMSpectrumPlot::AMSpectrumPlot(QWidget *parent) :
    QCustomPlot(parent),dc(NULL),bw(NULL),wfplot(NULL),wfitem(NULL),
    isWf(false),markersVisible(false)
{

    /* Fixed scale */
    xAxis->setLabel(tr("Frequency [kHz]"));

    yAxis->setRange(-120.0, 0.0);
    yAxis->setLabel(tr("Input PSD [dB]"));


    wfitem = new QCPItemPixmap(this);
    wfplot = new WaterFallPlot();
    bw = new QCPBars(xAxis, yAxis);
    bw->setBrush(QBrush(QColor(192, 192, 255, 125)));
    bw->setPen(QPen(Qt::NoPen));
    dc = new QCPItemLine(this);
    dc->setPen(QPen(Qt::red, 1, Qt::DashLine));

    addGraph();
    graph(0)->setPen(QPen(Qt::blue, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    /* Insert line for bandwidth marker */
    //addPlottable(bw);
    /* Insert line for DC carrier */
    //addItem(dc);
    //addItem(wfitem);

    if(addLayer("spectrum")) {
        graph(0)->setLayer("spectrum");
    }
    if(addLayer("bwdc")) {
        bw->setLayer("bwdc");
        dc->setLayer("bwdc");
        if(!markersVisible)
            layer("bwdc")->setVisible(false);
        setToolTip(tr("Click on the plot to set the demodulation frequency"));
    }
    if(addLayer("waterfall")) {
        layer("waterfall")->setVisible(false);
        wfitem->setLayer("waterfall");
    }

    /* Add title */
    plotLayout()->insertRow(0); // inserts an empty row above the default axis rect
    plotLayout()->addElement(0, 0, new QCPTextElement(this, tr("Input PSD")));
}

void AMSpectrumPlot::updateSpectrum(CVector<_REAL>& vecrData, CVector<_REAL>& vecrScale)
{
    if(isWf) {
        wfplot->updatePlot(vecrData, yAxis->range().lower, yAxis->range().upper);
        wfitem->setPixmap(wfplot->pixmap);
    }
    else {
        QVector<double> k, v;
        for(int i=0; i<vecrData.Size(); i++) {
            k << vecrScale[i];
            v << vecrData[i];
        }
        graph(0)->setData(k, v);
    }
    replot();
}

void AMSpectrumPlot::setMarkersVisible(bool b)
{
    if(!isWf)
        layer("bwdc")->setVisible(b);
    markersVisible = b;
}

void AMSpectrumPlot::setWaterfallMode(bool b)
{
    layer("waterfall")->setVisible(b);
    layer("spectrum")->setVisible(!b);
    if(markersVisible)
        layer("bwdc")->setVisible(!b);
    isWf = b;
}

void AMSpectrumPlot::updateBWMarker(double centerFreq, double bandwidth)
{
    QVector<double> x, y;
    x << centerFreq;
    y << yAxis->range().lower; // full height
    bw->setData(x, y);
    bw->setWidth(bandwidth);
    replot();
}

void AMSpectrumPlot::updateDCCarrier(double f)
{
    dc->start->setCoords(f, yAxis->range().lower);
    dc->end->setCoords(f, yAxis->range().upper);
    replot();
}

void AMSpectrumPlot::resizeEvent(QResizeEvent *e)
{
    QCustomPlot::resizeEvent(e);
    wfplot->resize(axisRect()->size());
    if(isWf)
        replot(); // will get two replots but need QCustomPlot to do its stuff first
}
