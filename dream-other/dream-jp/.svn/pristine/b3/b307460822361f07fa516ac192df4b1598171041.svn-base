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

#ifndef CDRMPLOTQWT_H
#define CDRMPLOTQWT_H

#include "DRMPlot.h"
#include <QTimer>
#include <QDialog>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <../enumerations.h>

/* Define the plot font size */
#define PLOT_FONT_SIZE 8

/* Classes ********************************************************************/
#if QWT_VERSION >= 0x060000
typedef QPointF QwtDoublePoint; // just for compilation
#endif

class WaterfallWidget;
class CDRMReceiver;
class QTreeWidget;
class QResizeEvent;
class QwtPlotPicker;
class QwtLegend;
class ReceiverController;

class CDRMPlotQwt : public QObject, public PlotInterface
{
    Q_OBJECT

public:

    CDRMPlotQwt(QWidget*);
    ~CDRMPlotQwt();

    QwtPlot         *plot;

    QWidget* widget() const {
        return plot;
    }

protected:
    void applyColors(QColor MainGridColorPlot, QColor BckgrdColorPlot);
    void replot();
    void clearPlots();
    void setupBasicPlot(const char* titleText,
                        const char* xText, const char* yText, const char* legendText,
                        double left, double right, double bottom, double top, QColor pc, QColor bc);
    void add2ndGraph(const char* axisText, const char* legendText, double bottom, double top, QColor pc);
    void addxMarker(QColor color, double initialPos);
    void addBwMarker(QColor c);
    void addyMarker(QColor color, double initialPos);
    void setupConstPlot(const char* text);
    void addConstellation(const char* legendText, int n);
    void setupWaterfall(double sr);
    void setData(int n, CVector<_COMPLEX>& veccData);
    void setData(int n, CVector<_REAL>& vecrData, CVector<_REAL>& vecrScale, const QString& axisLabel="");
    void setxMarker(int n, _REAL r);
    void setBwMarker(int n, _REAL c, _REAL b);
    void setyMarker(int n, _REAL r);
    void updateWaterfall(CVector<_REAL>& vecrData, CVector<_REAL>& vecrScale);
    void setQAMGrid(double div, int step, int substep);
    void setWhatsThis(const QString& s) {
        plot->setWhatsThis(s);
    }
    void setAutoScalePolicy(Plot::EAxis axis, Plot::EPolicy policy, double limit);

    void AddWhatsThisHelpChar(const ECharType NCharType);
    QVector<double> getBounds(QwtPlot::Axis axis);

    Plot::EPolicy   policy[4];
    double          limit[4];
    ECharType       InitCharType;
    ECodScheme      eLastSDCCodingScheme;
    ECodScheme      eLastMSCCodingScheme;
    bool        bLastAudioDecoder;

    QwtText         leftTitle, rightTitle, bottomTitle;

    QwtPlotCurve    main1curve, main2curve;
    QwtPlotCurve    curve[4];
    QwtPlotCurve    yMarker;
    QwtPlotCurve    hcurvegrid, vcurvegrid;
    QwtPlotGrid     grid;
    QwtPlotPicker   *picker;
    QwtLegend       *legend;
    int             nCurves;

    /* Waterfall spectrum stuff */
    WaterfallWidget* waterfallWidget;

private slots:
    void OnSelected(const QPointF &pos);

signals:
    void plotClicked(double);
};

#endif // CDRMPLOTQWT_H
