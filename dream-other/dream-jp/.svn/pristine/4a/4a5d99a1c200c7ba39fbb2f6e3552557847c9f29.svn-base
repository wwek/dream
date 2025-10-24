#ifndef QWTLEVELMETER_H
#define QWTLEVELMETER_H

#include <qwt_thermo.h>
#include "DialogUtil.h"

class QwtLevelMeter : public QwtThermo, public LevelMeter
{
    Q_OBJECT
public:
    explicit QwtLevelMeter(QWidget *parent = 0);

    void setLevel(double level) {
        setValue(level);
    }
    QWidget* widget() {
        return this;
    }
};

#endif // QWTLEVELMETER_H
