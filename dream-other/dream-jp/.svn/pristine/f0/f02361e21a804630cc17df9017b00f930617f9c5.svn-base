#ifndef CDRMPLOTQCP_H
#define CDRMPLOTQCP_H

#include "DRMPlot.h"
#include <qcustomplot.h>
#include "waterfallwidget.h"

class CDRMPlotQCP : public QObject, public PlotInterface
{
    Q_OBJECT
public:
    CDRMPlotQCP(QWidget* parent=0);
    ~CDRMPlotQCP();

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

private:

    Plot::EPolicy   policy[4];
    double          limit[4];
    QCustomPlot* plot;
    WaterFallPlot* wfplot;
    QCPItemPixmap* wfitem;
    QCPTextElement* title;
    QVector<QCPItemLine*> hlines, vlines;
    QVector<QCPBars*> bars;

private slots:

    void on_plotClick(QCPAbstractPlottable*,int,QMouseEvent*);
signals:
    void plotClicked(double);
};

#endif // CDRMPLOTQCP_H
