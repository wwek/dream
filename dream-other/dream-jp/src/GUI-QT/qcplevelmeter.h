#ifndef QCPLEVELMETER_H
#define QCPLEVELMETER_H

#include <qcustomplot.h>
#include "DialogUtil.h"

class QCPLevelMeter : public QCustomPlot, public LevelMeter
{
public:
    QCPLevelMeter(QWidget* parent=0);
public slots:
    void setLevel(double d);
    QWidget* widget() {
        return this;
    }
};

#endif // QCPLEVELMETER_H
