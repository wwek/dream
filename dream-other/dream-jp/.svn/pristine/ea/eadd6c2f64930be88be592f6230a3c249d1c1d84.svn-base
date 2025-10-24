#include "cdrmplotqcp.h"
#include <qcustomplot.h>
#include "../matlib/MatlibStdToolbox.h"


CDRMPlotQCP::CDRMPlotQCP(QWidget* parent):QObject(parent),plot(new QCustomPlot(parent)),wfplot(NULL),wfitem(NULL),
    title(NULL),hlines(),vlines()
{
    this->plot = plot;
    title = new QCPTextElement(plot);
    QFont f = title->font();
    f.setPointSize(10);
    title->setFont(f);

    QCPLayoutGrid *ll = new QCPLayoutGrid;

    //ll->addElement(0, 0, plot->legend);
    //ll->setMargins(QMargins(20,30,20,60));

    //plot->plotLayout()->addElement(0, 1, ll);
    // TODO position and size legend better
    //plot->plotLayout()->setColumnStretchFactor(0, 4); // plot shall have 80% of width
    //plot->plotLayout()->setColumnStretchFactor(1, 1); // legend only 20% (4:1 = 80:20)
    plot->plotLayout()->insertRow(0); // inserts an empty row above the default axis rect
    plot->plotLayout()->addElement(0, 0, title);
    connect(plot, SIGNAL(plottableClick(QCPAbstractPlottable*,int,QMouseEvent*)),
            this, SLOT(on_plotClick(QCPAbstractPlottable*,int,QMouseEvent*)));
}

CDRMPlotQCP::~CDRMPlotQCP()
{
}

void CDRMPlotQCP::clearPlots()
{
    plot->clearGraphs();
    plot->clearItems();
    plot->clearPlottables();
    //for(int i=0; i<vlines.size(); i++)
    //    delete vlines[i];
    vlines.clear();
    //for(int i=0; i<hlines.size(); i++)
    //    delete hlines[i];
    hlines.clear();
}

void CDRMPlotQCP::replot()
{
    plot->replot();
}

void CDRMPlotQCP::applyColors(QColor MainGridColorPlot, QColor BckgrdColorPlot)
{
    /*
     * set background to BckgrdColorPlot
     * set gridlines to MainGridColorPlot
     * re-initialise plotting system
     */
    plot->axisRect()->setBackground(BckgrdColorPlot);
}

void CDRMPlotQCP::setupBasicPlot(const char* titleText, const char* xText, const char* yText, const char* legendText,
                                 double left, double right, double bottom, double top, QColor pc, QColor bc)
{
    title->setText(QObject::tr(titleText));
    /* Fixed scale */
    plot->xAxis->setLabel(QObject::tr(xText));
    plot->xAxis->setRange(left, right);

    plot->xAxis->setVisible(true);
    plot->xAxis2->setVisible(false);
    plot->yAxis->setVisible(true);
    plot->yAxis2->setVisible(false);

    plot->yAxis->setRange(bottom, top);
    plot->yAxis->setLabel(QObject::tr(yText));

    plot->addGraph();
    plot->graph(0)->setPen(QPen(pc, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    plot->graph(0)->setName(QObject::tr(legendText));
    plot->graph(0)->addToLegend();

    plot->legend->setVisible(false);
    for(int i=0; i<4; i++) {
        policy[i] = Plot::fixed;
    }
}

void CDRMPlotQCP::add2ndGraph(const char* axisText, const char* legendText, double bottom, double top, QColor pc)
{
    plot->yAxis2->setVisible(true);
    plot->yAxis2->setLabel(QObject::tr(axisText));
    plot->yAxis2->setRange(bottom, top);

    plot->addGraph();

    plot->graph(1)->setPen(QPen(pc, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    plot->graph(1)->setValueAxis(plot->yAxis2);
    plot->graph(1)->setName(QObject::tr(legendText));
    plot->graph(1)->addToLegend();

    plot->legend->setVisible(true);

}

void CDRMPlotQCP::addxMarker(QColor color, double initialPos)
{
    QCPItemLine* l = new QCPItemLine(plot);
    l->setPen(QPen(color, 1, Qt::DashLine));
    //plot->addItem(l);
    l->start->setCoords(initialPos, plot->yAxis->range().lower);
    l->end->setCoords(initialPos, plot->yAxis->range().upper);
    vlines << l;
}

void CDRMPlotQCP::addBwMarker(QColor c)
{
    QCPBars* bw = new QCPBars(plot->xAxis, plot->yAxis);
    bw->setBrush(QBrush(c));
    bw->setPen(QPen(Qt::NoPen));
    //plot->addPlottable(bw);
    if(plot->addLayer("bw")) {
        bw->setLayer("bw");
    }
    bars << bw;
}

void CDRMPlotQCP::addyMarker(QColor color, double initialPos)
{
    QCPItemLine* l = new QCPItemLine(plot);
    l->setPen(QPen(color, 1, Qt::DashLine));
    //plot->addItem(l);
    l->start->setCoords(initialPos, plot->xAxis->range().lower);
    l->end->setCoords(initialPos, plot->xAxis->range().upper);
    hlines << l;
}

void CDRMPlotQCP::setupWaterfall(double sr)
{
    title->setText(QObject::tr("Waterfall Spectrum"));
    wfitem = new QCPItemPixmap(plot);
    wfplot = new WaterFallPlot();
    wfplot->resize(plot->axisRect()->size());
    //plot->addItem(wfitem);
    plot->legend->setVisible(false);
    plot->xAxis->setLabel(QObject::tr("Frequency [kHz]"));
    plot->xAxis->setRange(0.0, sr);
    plot->yAxis->setVisible(false);
}

void CDRMPlotQCP::setQAMGrid(double div, int step, int substep)
{
    int i;
    double pos;
    QVector <double> ticks;
    QVector <double> subticks;

    for (i=0; i<=step*2; i++)
    {
        pos = -div + div / step * i;
        /* Keep 2 digit after the point */
        pos = Round(pos * 100.0) / 100.0;
        ticks << pos;
    }

    substep *= step;
    for (i=0; i<=substep*2; i++)
    {
        pos = -div + div / substep * i;
        /* Keep 2 digit after the point */
        pos = Round(pos * 100.0) / 100.0;
        subticks << pos;
    }
    plot->xAxis->ticker()->setTickCount(step);
    plot->xAxis->setRange(ticks[0], ticks[ticks.size()-1]);
    plot->yAxis->setRange(ticks[0], ticks[ticks.size()-1]);
    plot->yAxis->ticker()->setTickCount(step);
}

void CDRMPlotQCP::setupConstPlot(const char* text)
{
    plot->yAxis2->setVisible(false);

    //plot->xAxis->ticker()->setAutoTicks(false);
    plot->xAxis->setVisible(true);;
    plot->xAxis->setLabel(QObject::tr("Real"));

    //plot->yAxis->ticker()->setAutoTicks(false);
    plot->yAxis->setVisible(true);
    plot->yAxis->setLabel(QObject::tr("Imaginary"));
    plot->legend->setVisible(false);
    title->setText(QObject::tr(text));
}

void CDRMPlotQCP::addConstellation(const char *legendText, int c)
{
    QCPGraph* g = plot->addGraph();
    g->setName(QObject::tr(legendText));
    switch(c) {
    case 0:
        g->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssPlus, Qt::black, 6));
        break;
    case 1:
        g->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDiamond, Qt::red, 6));
        break;
    case 2:
        g->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross, Qt::blue, 6));
        break;
    default:
        ;
    }
    g->setLineStyle(QCPGraph::lsNone);
    if(plot->graphCount()>1)
        plot->legend->setVisible(true);
}

void CDRMPlotQCP::updateWaterfall(CVector<_REAL>& vecrData, CVector<_REAL>& vecrScale)
{
    if(wfplot->pixmap.size()!=plot->axisRect()->size())
        wfplot->resize(plot->axisRect()->size());
    wfplot->updatePlot(vecrData, plot->yAxis->range().lower, plot->yAxis->range().upper);
    wfitem->setPixmap(wfplot->pixmap);
}

void CDRMPlotQCP::setData(int n, CVector<_COMPLEX>& veccData)
{
    QVector<double> k,v;
    for(int i=0; i<veccData.Size(); i++) {
        double re = veccData[i].real();
        double im = veccData[i].imag();
        k << re;
        v << im;
    }
    plot->graph(n)->setData(k, v);
}

void CDRMPlotQCP::setData(int n, CVector<_REAL>& vecrData, CVector<_REAL>& vecrScale, const QString &axisLabel)
{
    QVector<double> k,v;
    for(int i=0; i<vecrData.Size(); i++) {
        k << vecrScale[i];
        v << vecrData[i];
    }
    plot->graph(n)->setData(k, v);
    if(n==0)
    {
        if(policy[Plot::left]==Plot::min)
        {
            /* Get maximum and minimum values */
            _REAL MaxFreq = -numeric_limits<_REAL>::max();
            _REAL MinFreq = numeric_limits<_REAL>::max();
            for (int i = 0; i < vecrScale.Size(); i++)
            {
                if (vecrData[i] > MaxFreq)
                    MaxFreq = vecrData[i];

                if (vecrData[i] < MinFreq)
                    MinFreq = vecrData[i];
            }
            /* Apply scale to plot */
            plot->yAxis->setRange(floor(MinFreq / limit[Plot::left]), ceil(MaxFreq / limit[Plot::left]));
        }
        if(policy[Plot::left]==Plot::fit)
        {
            const int iMaxDisToMax = limit[Plot::left]; /* dB */

            /* Get maximum value */
            _REAL MaxSNR = -numeric_limits<_REAL>::max();

            for (int i = 0; i < vecrScale.Size(); i++)
            {
                if (vecrData[i] > MaxSNR)
                    MaxSNR = vecrData[i];
            }

            /* Quantize scale to a multiple of "iMaxDisToMax" */
            double dMaxYScaleSNR = (ceil(MaxSNR / iMaxDisToMax) * iMaxDisToMax);

            const int iMinValueSNRYScale = 15; /* dB */
            /* Bound at the minimum allowed value */
            if (dMaxYScaleSNR < (double) iMinValueSNRYScale)
                dMaxYScaleSNR = (double) iMinValueSNRYScale;

            const _REAL rRatioAudSNR = 1.5;
            const double dMaxYScaleAudio = dMaxYScaleSNR * rRatioAudSNR;

            /* Apply scale to plot */
            plot->yAxis->setRange(0.0, dMaxYScaleSNR);
        }
        if(policy[Plot::left]==Plot::enlarge)
        {
            const int iSize = vecrScale.Size();
            /* Get maximum value */
            _REAL rMaxSNR = -numeric_limits<_REAL>::max();
            for (int i = 0; i < iSize; i++)
            {
                if (vecrData[i] > rMaxSNR)
                    rMaxSNR = vecrData[i];
            }

            double dMaxScaleYAxis = MAX_VAL_SNR_SPEC_Y_AXIS_DB;

            if (rMaxSNR > dMaxScaleYAxis)
            {
                const double rEnlareStep = (double) 10.0; /* dB */
                dMaxScaleYAxis = ceil(rMaxSNR / rEnlareStep) * rEnlareStep;
            }

            /* Set scale */
            plot->yAxis->setRange(MIN_VAL_SNR_SPEC_Y_AXIS_DB, dMaxScaleYAxis);
        }
        if(policy[Plot::bottom]==Plot::first)
        {
            plot->xAxis->setRange(vecrScale[0], 0.0);
        }
        if(policy[Plot::bottom]==Plot::last)
        {
            plot->xAxis->setRange(0.0, vecrScale.Size());
        }
    }
    if(n==1)
    {
        if(policy[Plot::right]==Plot::min)
        {
            _REAL MaxSam = -numeric_limits<_REAL>::max();
            _REAL MinSam = numeric_limits<_REAL>::max();
            for (int i = 0; i < vecrScale.Size(); i++)
            {
                if (vecrData[i] > MaxSam)
                    MaxSam = vecrData[i];

                if (vecrData[i] < MinSam)
                    MinSam = vecrData[i];
            }
            plot->yAxis2->setRange(floor(MinSam / limit[Plot::right]), ceil(MaxSam / limit[Plot::right]));
        }
        if(policy[Plot::right]==Plot::fit)
        {
            const int iMaxDisToMax = limit[Plot::right];

            /* Get maximum value */
            _REAL Max = -numeric_limits<_REAL>::max();

            for (int i = 0; i < vecrScale.Size(); i++)
            {
                if (vecrData[i] > Max)
                    Max = vecrData[i];
            }

            /* Quantize scale to a multiple of "iMaxDisToMax" */
            double dMaxYScale = (ceil(Max / iMaxDisToMax) * iMaxDisToMax);

            double MinValueYScale = 15; /* dB */
            /* Bound at the minimum allowed value */
            if (dMaxYScale < MinValueYScale)
                dMaxYScale = MinValueYScale;

            plot->yAxis2->setRange(0.0, dMaxYScale);
        }
    }
    //plot->graph(n)->rescaleKeyAxis(true, true);
    //plot->graph(n)->rescaleValueAxis(true);
    if(axisLabel != "") {
        if(n==0)
            plot->yAxis->setLabel(axisLabel);
        else
            plot->yAxis2->setLabel(axisLabel);
    }
}

void CDRMPlotQCP::setxMarker(int n, _REAL r)
{
    vlines[n]->start->setCoords(r, plot->yAxis->range().lower);
    vlines[n]->end->setCoords(r, plot->yAxis->range().upper);
}

void CDRMPlotQCP::setBwMarker(int n, _REAL c, _REAL w)
{
    QVector<double> x, y;
    x << c;
    y << plot->yAxis->range().lower; // full height
    bars[n]->setData(x, y);
    bars[n]->setWidth(w);
}

void CDRMPlotQCP::setyMarker(int n, _REAL r)
{
    hlines[n]->start->setCoords(r, plot->xAxis->range().lower);
    hlines[n]->end->setCoords(r, plot->xAxis->range().upper);
}

void CDRMPlotQCP::setAutoScalePolicy(Plot::EAxis axis, Plot::EPolicy policy, double limit)
{
    this->policy[axis] = policy;
    this->limit[axis] = limit;
}

void CDRMPlotQCP::on_plotClick(QCPAbstractPlottable *, int, QMouseEvent *event)
{
    double d = plot->xAxis->pixelToCoord(event->x());
    emit plotClicked(d);
}
