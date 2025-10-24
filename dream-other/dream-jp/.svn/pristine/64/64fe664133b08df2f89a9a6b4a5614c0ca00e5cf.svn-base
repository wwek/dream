#ifndef AMSPECTRUMPLOT_H
#define AMSPECTRUMPLOT_H

#include <qcustomplot.h>
#include "../util/Vector.h"
#include "waterfallwidget.h"

class AMSpectrumPlot : public QCustomPlot
{
    Q_OBJECT
public:
    explicit AMSpectrumPlot(QWidget *parent = 0);

signals:

public slots:
    void updateBWMarker(double centerFreq, double bandwidth);
    void updateDCCarrier(double rDCFreq);
    void updateSpectrum(CVector<_REAL>& vecrData, CVector<_REAL>& vecrScale);
    void setWaterfallMode(bool b);
    void setMarkersVisible(bool b);
protected:
    void resizeEvent(QResizeEvent *);
private:
    QCPItemLine*    dc;
    QCPBars*        bw;
    WaterFallPlot*  wfplot;
    QCPItemPixmap*  wfitem;
    bool            isWf;
    bool            markersVisible;
};

#endif // AMSPECTRUMPLOT_H
