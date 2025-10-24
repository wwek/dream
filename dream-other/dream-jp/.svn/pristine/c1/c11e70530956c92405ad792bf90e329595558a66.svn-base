#ifndef QCPSMETER_H
#define QCPSMETER_H

#include <qcustomplot.h>
#include "DialogUtil.h"

class QCPSMeter : public QCustomPlot, public SMeter
{
    Q_OBJECT
public:
    explicit QCPSMeter(QWidget *parent = 0);

    void setLevel(double level);
    QWidget* widget() {
        return this;
    }
};


#endif // QCPSMETER_H
