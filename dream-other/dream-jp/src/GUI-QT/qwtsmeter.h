#ifndef QWTSMETER_H
#define QWTSMETER_H

#include <qwt_thermo.h>
#include "DialogUtil.h"

class QwtSMeter : public QwtThermo, public SMeter
{
    Q_OBJECT
public:
    explicit QwtSMeter(QWidget *parent = 0);

    void setLevel(double level) {
        setValue(level);
    }
    QWidget* widget() {
        return this;
    }
};

#endif // QWTSMETER_H
