#ifndef DRMDISPLAY_H
#define DRMDISPLAY_H

#include <QWidget>
#include <../Parameter.h>
#include <MultColorLED.h>

class LevelMeter;

namespace Ui {
class DRMDisplay;
}

class DRMDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit DRMDisplay(QWidget *parent = 0);
    ~DRMDisplay();
    void setBars(int);
    void showReceptionStatus(ETypeRxStatus fac, ETypeRxStatus sdc, ETypeRxStatus msc);
    void showTextMessage(const QString&);
    void showServiceInfo(const CService&);
    void setBitRate(_REAL rBitRate, _REAL rPartABLenRat);
    void clearDisplay(const QString& serviceLabel);
    void SetDisplayColor(const QColor&);
public slots:
    void setLevel(double);

private:
    Ui::DRMDisplay *ui;
    LevelMeter* inputLevel;
    void AddWhatsThisHelp();
};

#endif // DRMDISPLAY_H
