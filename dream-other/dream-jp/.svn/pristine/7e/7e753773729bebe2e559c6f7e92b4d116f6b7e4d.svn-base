/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001-2014
 *
 * Original Author(s):
 *  Volker Fischer
 *
 * Qwt 5-6 conversion Author(s):
 *  David Flamand
 *
 * Description:
 *  Custom settings of the qwt-plot, Support Qwt version 5.0.0 to 6.1.0(+)
 *
 ******************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
\******************************************************************************/

#include "cdrmplotqwt.h"
#include "receivercontroller.h"
#include "waterfallwidget.h"
#include <QTreeWidget>
#include <QResizeEvent>
#include <cmath>
#include <algorithm>
#include <QHBoxLayout>
#include <qwt_legend.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <qwt_symbol.h>

/* Define the FAC/SDC/MSC Symbol size */
# define FAC_SYMBOL_SIZE 4
# define SDC_SYMBOL_SIZE 4
# define MSC_SYMBOL_SIZE 2
# define ALL_FAC_SYMBOL_SIZE 4
# define ALL_SDC_SYMBOL_SIZE 4
# define ALL_MSC_SYMBOL_SIZE 2

/* Window margin for chart */
#define WINDOW_MARGIN 10

/* Definitions ****************************************************************/
#define GUI_CONTROL_UPDATE_WATERFALL            100 /* Milliseconds */

/* Implementation *************************************************************/
CDRMPlotQwt::CDRMPlotQwt(QWidget* parent) :
    plot(new QwtPlot(parent)),
    InitCharType(NONE_OLD),
    eLastSDCCodingScheme(ECodScheme(-1)), eLastMSCCodingScheme(ECodScheme(-1)),
    bLastAudioDecoder(false),
    leftTitle(), rightTitle(), bottomTitle(),
    main1curve(), main2curve(),
    curve(), yMarker(), hcurvegrid(), vcurvegrid(),
    grid(),picker(NULL),legend(NULL),nCurves(0),
    waterfallWidget(NULL)
{
    /* Create new plot if none is supplied */
    if (plot == NULL)
        plot = new QwtPlot(parent);

    /* Setup plot */
    plot->setAutoDelete(false);
    plot->setAutoReplot(false);
    plot->setMinimumHeight(120); // very important for getting sizeHint rational

    /* Base font */
    QFont basefont;
    basefont.setPointSize(PLOT_FONT_SIZE);
    basefont.setStyleHint(QFont::SansSerif, QFont::PreferOutline);

    /* Legend creation */
    legend = new QwtLegend();
    QFont legendfont(basefont);
    legend->setFont(legendfont);
    plot->insertLegend(legend, QwtPlot::RightLegend);

    /* Curve defaults (other curves are set by PlotDefaults) */
    curve[3].setItemAttribute(QwtPlotItem::Legend, false);
    yMarker.setItemAttribute(QwtPlotItem::Legend, false);
    vcurvegrid.setItemAttribute(QwtPlotItem::Legend, false);
    hcurvegrid.setItemAttribute(QwtPlotItem::Legend, false);
    vcurvegrid.setStyle(QwtPlotCurve::Sticks);
    hcurvegrid.setStyle(QwtPlotCurve::Sticks);
    vcurvegrid.setOrientation(Qt::Vertical);
    hcurvegrid.setOrientation(Qt::Horizontal);

    /* Grid */
    grid.enableXMin(false);
    grid.enableYMin(false);
    grid.attach(plot);

    /* Axis and title fonts */
    QFont axisfont(basefont);
    QFont titlefont(basefont);
    titlefont.setWeight(QFont::Bold);
    plot->setAxisFont(QwtPlot::xBottom, axisfont);
    plot->setAxisFont(QwtPlot::yLeft, axisfont);
    plot->setAxisFont(QwtPlot::yRight, axisfont);
    QwtText title;
    title.setFont(titlefont);
    plot->setTitle(title);

    /* Axis titles */
    bottomTitle.setFont(axisfont);
    plot->setAxisTitle(QwtPlot::xBottom, bottomTitle);
    leftTitle.setFont(axisfont);
    plot->setAxisTitle(QwtPlot::yLeft, leftTitle);
    rightTitle.setFont(axisfont);
    plot->setAxisTitle(QwtPlot::yRight, rightTitle);

    /* Global frame */
    plot->setFrameStyle(QFrame::Plain|QFrame::NoFrame);
    plot->setLineWidth(0);
    plot->setContentsMargins(
        WINDOW_MARGIN, WINDOW_MARGIN,
        WINDOW_MARGIN, WINDOW_MARGIN);

    /* Canvas */
    ((QFrame*)plot->canvas())->setLineWidth(0);
    plot->canvas()->setBackgroundRole(QPalette::Window);

    /* Picker */
    picker = new QwtPlotPicker(plot->canvas());
    picker->initMousePattern(1);
    QwtPickerClickPointMachine *machine = new QwtPickerClickPointMachine();
    picker->setStateMachine(machine);

    /* Connections */
    connect(picker, SIGNAL(selected(const QPointF&)), this, SLOT(OnSelected(const QPointF&)));
}

CDRMPlotQwt::~CDRMPlotQwt()
{
}

void CDRMPlotQwt::applyColors(QColor MainGridColorPlot, QColor BckgrdColorPlot)
{
    /* Apply colors */
    grid.setMajorPen(QPen(MainGridColorPlot, 0, Qt::DotLine));
    grid.setMinorPen(QPen(MainGridColorPlot, 0, Qt::DotLine));
    vcurvegrid.setPen(QPen(MainGridColorPlot, 1, Qt::DotLine));
    hcurvegrid.setPen(QPen(MainGridColorPlot, 1, Qt::DotLine));
    plot->setCanvasBackground(QColor(BckgrdColorPlot));

    /* Make sure that plot are being initialized again */
    InitCharType = NONE_OLD;
}

void CDRMPlotQwt::setQAMGrid(double div, int step, int substep)
{
    int i;
    _REAL pos;
    QList <double> ticks[3];

    for (i=0; i<=step*2; i++)
    {
        pos = -div + div / step * i;
        /* Keep 2 digit after the point */
        pos = Round(pos * 100.0) / 100.0;
        ticks[2].push_back((double)pos);
    }

    substep *= step;
    for (i=0; i<=substep*2; i++)
    {
        pos = -div + div / substep * i;
        /* Keep 2 digit after the point */
        pos = Round(pos * 100.0) / 100.0;
        ticks[1].push_back((double)pos);
    }

    /* Keep 2 digit after the point */
    div = (double)Round(div * 100.0) / 100.0;

    /* Set the scale */
    QwtScaleDiv scaleDiv(-div, div, ticks);
    plot->setAxisScaleDiv(QwtPlot::xBottom, scaleDiv);
    plot->setAxisScaleDiv(QwtPlot::yLeft, scaleDiv);
}

void CDRMPlotQwt::OnSelected(const QPointF &pos)
{
    const double dMaxxBottom = plot->axisScaleDiv(QwtPlot::xBottom).upperBound();

    /* Check if dMaxxBottom is valid */
    if (dMaxxBottom > 0.0)
    {
        emit plotClicked(pos.x());
    }
}

void CDRMPlotQwt::replot()
{
    plot->replot();
}

void CDRMPlotQwt::clearPlots()
{
    /* Set default value of plot items */
    for(int i=0; i<nCurves; i++)
    {
        curve[i].detach();
    }
    yMarker.detach();
    hcurvegrid.detach();
    vcurvegrid.detach();
    main1curve.detach();
    main2curve.detach();
    for(int i=0; i<nCurves; i++)
    {
        curve[i].setSamples(NULL, NULL, 0);
        curve[i].setSymbol(NULL);
        curve[i].setLegendAttribute(QwtPlotCurve::LegendShowSymbol, false);
        curve[i].setItemAttribute(QwtPlotItem::Legend, false);
    }
    yMarker.setSamples(NULL, NULL, 0);
    hcurvegrid.setSamples(NULL, NULL, 0);
    vcurvegrid.setSamples(NULL, NULL, 0);
    main1curve.setSamples(NULL, NULL, 0);
    main2curve.setSamples(NULL, NULL, 0);
    if (waterfallWidget != NULL)
    {
        delete waterfallWidget;
        waterfallWidget = NULL;
    }
    main1curve.setItemAttribute(QwtPlotItem::Legend, false);
    main2curve.setItemAttribute(QwtPlotItem::Legend, false);
    // start curve count
    nCurves=0;
}

void CDRMPlotQwt::setupBasicPlot(const char* titleText,
                                 const char* xText, const char* yText, const char* legendText,
                                 double left, double right, double bottom, double top,
                                 QColor pc, QColor bc)
{
    grid.enableX(true);
    grid.enableY(true);
    grid.setMajorPen(QPen(pc, 0, Qt::DotLine));
    grid.setMinorPen(QPen(pc, 0, Qt::DotLine));
    main1curve.setItemAttribute(QwtPlotItem::Legend, false);
    main2curve.setItemAttribute(QwtPlotItem::Legend, false);
    plot->setCanvasBackground(bc);

    plot->setTitle(titleText);
    plot->enableAxis(QwtPlot::yRight, false);
    plot->setAxisTitle(QwtPlot::xBottom, xText);
    plot->enableAxis(QwtPlot::yLeft, true);
    plot->setAxisTitle(QwtPlot::yLeft, yText);

    /* initial scale */
    plot->setAxisScale(QwtPlot::xBottom, left, right);
    plot->setAxisScale(QwtPlot::yLeft, bottom, top);

    /* Curve color */
    main1curve.setPen(QPen(pc, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    /* Add main curve */
    main1curve.setTitle(legendText);
    main1curve.attach(plot);

    for(int i=0; i<4; i++) {
        policy[i] = Plot::fixed;
    }
}

void CDRMPlotQwt::add2ndGraph(const char* axisText, const char* legendText, double bottom, double top, QColor pc)
{
    plot->enableAxis(QwtPlot::yRight);
    plot->setAxisTitle(QwtPlot::yRight, axisText);
    plot->setAxisScale(QwtPlot::yRight, bottom, top);

    /* Curve colors */
    main2curve.setPen(QPen(pc, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    /* Set legends */
    main1curve.setItemAttribute(QwtPlotItem::Legend, true);
    main2curve.setItemAttribute(QwtPlotItem::Legend, true);

    /* Add main curves */
    main2curve.setTitle(legendText);
    main2curve.setYAxis(QwtPlot::yRight);
    main2curve.attach(plot);
}

void CDRMPlotQwt::addxMarker(QColor color, double initialPos)
{
    curve[nCurves].setPen(QPen(color, 1, Qt::DashLine));
    curve[nCurves].attach(plot);
    QVector<double> yData;
    yData = getBounds(QwtPlot::yLeft);
    double x[2], y[2];
    x[0] = x[1] = initialPos;
    y[0] = yData[0];
    y[1] = yData[1];
    curve[nCurves].setSamples(x, y, 2);
    nCurves++;
}

void CDRMPlotQwt::addBwMarker(QColor c)
{
    QBrush brush(c);
    brush.setStyle(Qt::SolidPattern);
    curve[1].setBrush(brush);
    curve[1].setBaseline(MIN_VAL_INP_SPEC_Y_AXIS_DB);
    curve[1].setPen(QPen(Qt::NoPen));
    curve[1].attach(plot);

    /* Add tool tip to show the user the possibility of choosing the AM IF */
    plot->setToolTip(tr("Click on the plot to set the demodulation frequency"));
}

void CDRMPlotQwt::addyMarker(QColor color, double initialPos)
{
    yMarker.setPen(QPen(color, 1, Qt::DashLine));
    yMarker.attach(plot);
    QVector<double> xData;
    xData = getBounds(QwtPlot::xBottom);
    double x[2], y[2];
    x[0] = xData[0];
    x[1] = xData[1];
    y[0] = y[1] = initialPos;
    yMarker.setSamples(x, y, 2);
}

void CDRMPlotQwt::setupConstPlot(const char* text)
{
    plot->setTitle(text);
    plot->enableAxis(QwtPlot::yRight, false);
    plot->setAxisTitle(QwtPlot::xBottom, tr("Real"));
    plot->enableAxis(QwtPlot::yLeft, true);
    plot->setAxisTitle(QwtPlot::yLeft, tr("Imaginary"));
}

void CDRMPlotQwt::addConstellation(const char *legendText, int c)
{
    QwtSymbol* sym = new QwtSymbol();
    switch(c)
    {
    case 0:
        sym->setStyle(QwtSymbol::Ellipse);
        sym->setSize(ALL_FAC_SYMBOL_SIZE);
        sym->setPen(QPen(Qt::black));
        sym->setBrush(QBrush(Qt::black));
        break;
    case 1:
        sym->setStyle(QwtSymbol::XCross);
        sym->setSize(ALL_SDC_SYMBOL_SIZE);
        sym->setPen(QPen(Qt::red));
        sym->setBrush(QBrush(Qt::red));
        break;
    default:
        sym->setStyle(QwtSymbol::Rect);
        sym->setSize(ALL_MSC_SYMBOL_SIZE);
        sym->setPen(QPen(Qt::blue));
        sym->setBrush(QBrush(Qt::blue));
    }
    curve[nCurves].setLegendAttribute(QwtPlotCurve::LegendShowSymbol, true);
    curve[nCurves].setSymbol(sym);
    curve[nCurves].setPen(QPen(Qt::NoPen));
    curve[nCurves].setTitle(legendText);
    curve[nCurves].attach(plot);
    nCurves++;
    if(nCurves>1)
    {
        // more than one constellation - put all in legend
        for(int i=0; i<nCurves; i++)
            curve[i].setItemAttribute(QwtPlotItem::Legend, true);
    }
}

void CDRMPlotQwt::setupWaterfall(double sr)
{
    /* Init chart for waterfall input spectrum */
    plot->setTitle(tr("Waterfall Input Spectrum"));
    plot->enableAxis(QwtPlot::yRight, false);
    plot->setAxisTitle(QwtPlot::xBottom, tr("Frequency [kHz]"));
    plot->enableAxis(QwtPlot::yLeft, false);
    grid.enableX(false);
    grid.enableY(false);

    /* Create a new waterfall widget if not exists */
    if (waterfallWidget == NULL) {
        waterfallWidget = new WaterfallWidget(plot->canvas());
        waterfallWidget->show();
    }
    QLayout *layout = plot->canvas()->layout();
    if(layout != NULL)
    {
        delete layout;
    }
    layout = new QHBoxLayout;
    layout->addWidget(waterfallWidget);
    // remove top and bottom margins, set left/right to axis
    layout->setContentsMargins(4, 0, 5, 0); // assume axis size is fixed
    plot->canvas()->setLayout(layout);
    // fill the margins to match the surrounding background
    QColor c = plot->palette().color(QPalette::Window);
    plot->setCanvasBackground(c);

    /* Fixed scale DC to 50% sample rate in kHz */
    plot->setAxisScale(QwtPlot::xBottom, 0.0, sr);
}

void CDRMPlotQwt::setData(int n, CVector<_REAL>& vecrData, CVector<_REAL>& vecrScale, const QString &axisLabel)
{
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
            plot->setAxisScale(QwtPlot::yLeft, floor(MinFreq / limit[Plot::left]), ceil(MaxFreq / limit[Plot::left]));
        }
        if(policy[Plot::left]==Plot::fit)
        {
            const int iMaxDisToMax = limit[Plot::left]; /* dB */

            /* Get maximum value */
            _REAL Max = -numeric_limits<_REAL>::max();

            for (int i = 0; i < vecrScale.Size(); i++)
            {
                if (vecrData[i] > Max)
                    Max = vecrData[i];
            }

            /* Quantize scale to a multiple of "iMaxDisToMax" */
            double dMaxYScale = (ceil(Max / iMaxDisToMax) * iMaxDisToMax);

            const int iMinValueYScale = 15; /* dB */
            /* Bound at the minimum allowed value */
            if (dMaxYScale < (double) iMinValueYScale)
                dMaxYScale = (double) iMinValueYScale;

            /* Apply scale to plot */
            plot->setAxisScale(QwtPlot::yLeft, 0.0, dMaxYScale);
        }
        if(policy[Plot::left]==Plot::enlarge)
        {
            const int iSize = vecrScale.Size();
            /* Get maximum value */
            _REAL rMax = -numeric_limits<_REAL>::max();
            for (int i = 0; i < iSize; i++)
            {
                if (vecrData[i] > rMax)
                    rMax = vecrData[i];
            }

            double dMaxScaleYAxis = MAX_VAL_SNR_SPEC_Y_AXIS_DB;

            if (rMax > dMaxScaleYAxis)
            {
                const double rEnlareStep = (double) 10.0; /* dB */
                dMaxScaleYAxis = ceil(rMax / rEnlareStep) * rEnlareStep;
            }

            /* Set scale */
            plot->setAxisScale(QwtPlot::yLeft, 0.0, dMaxScaleYAxis);
        }
        if(policy[Plot::bottom]==Plot::first)
        {
            plot->setAxisScale(QwtPlot::xBottom, vecrScale[0], 0.0);
        }
        if(policy[Plot::bottom]==Plot::last)
        {
            plot->setAxisScale(QwtPlot::xBottom, 0.0, vecrScale.Size());
        }
    }
    if(n==1)
    {
        if(policy[Plot::right]==Plot::min)
        {
            _REAL Max = -numeric_limits<_REAL>::max();
            _REAL Min = numeric_limits<_REAL>::max();
            for (int i = 0; i < vecrScale.Size(); i++)
            {
                if (vecrData[i] > Max)
                    Max = vecrData[i];

                if (vecrData[i] < Min)
                    Min = vecrData[i];
            }
            plot->setAxisScale(QwtPlot::yRight, floor(Min / limit[Plot::right]), ceil(Max / limit[Plot::right]));
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

            plot->setAxisScale(QwtPlot::yRight, 0.0, dMaxYScale);
        }
    }
    const int size = vecrData.Size();
    if(n==0)
        main1curve.setSamples(size ? &vecrScale[0] : NULL, size ? &vecrData[0] : NULL, size);
    else
        main2curve.setSamples(size ? &vecrScale[0] : NULL, size ? &vecrData[0] : NULL, size);
    // set dynamic axis label
    if(axisLabel != "") {
        if(n==0)
            plot->setAxisTitle(QwtPlot::yLeft, axisLabel);
        else
            plot->setAxisTitle(QwtPlot::yRight, axisLabel);
    }
}

void CDRMPlotQwt::setData(int n, CVector<_COMPLEX>& veccData)
{
    const int size = veccData.Size();
    vector<double> r(size), im(size);
    for(int i=0; i<size; i++)
    {
        r[i] = veccData[i].real();
        im[i] = veccData[i].imag();
    }
    curve[n].setSamples(&r[0], &im[0], size);
}

void CDRMPlotQwt::setxMarker(int n, _REAL r)
{
    QVector<double> yData;
    yData = getBounds(QwtPlot::yLeft);
    double x[2], y[2];
    x[0] = x[1] = r;
    y[0] = yData[0];
    y[1] = yData[1];
    curve[n].setSamples(x, y, 2);
}

void CDRMPlotQwt::setBwMarker(int n, _REAL c, _REAL b)
{
    (void)n; // TODO support more than one marker
    QVector<double> yData;
    yData = getBounds(QwtPlot::yLeft);
    yData[0] = yData[1];
    double x[2], y[2];
    x[0] = c - b/2.0;
    x[1] = c + b/2.0;
    y[0] = y[1] = yData[1];
    curve[1].setSamples(x, y, 2);
}


void CDRMPlotQwt::setyMarker(int n, _REAL r)
{
    (void)n; // TODO support more than one marker
    QVector<double> xData, yData;
    xData = getBounds(QwtPlot::xBottom);
    yData << r << r;
    yMarker.setSamples(xData, yData);
}

void CDRMPlotQwt::updateWaterfall(CVector<_REAL>& vecrData, CVector<_REAL>& vecrScale)
{
    /* No waterfallWidget so return */
    if (waterfallWidget == nullptr)
        return;
    waterfallWidget->updatePlot(vecrData.Data(), MIN_VAL_INP_SPEC_Y_AXIS_DB, MAX_VAL_INP_SPEC_Y_AXIS_DB);
    (void)vecrScale;
}

void CDRMPlotQwt::setAutoScalePolicy(Plot::EAxis axis, Plot::EPolicy policy, double limit)
{
    this->policy[axis] = policy;
    this->limit[axis] = limit;
}

QVector<double> CDRMPlotQwt::getBounds(QwtPlot::Axis axis)
{
    QwtScaleDiv sd = plot->axisScaleDiv(axis);
    QVector<double> data;
    data << sd.lowerBound() << sd.upperBound();
    return data;
}
