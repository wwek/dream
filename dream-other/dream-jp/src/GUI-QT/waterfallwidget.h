#ifndef WATERFALLWIDGET_H
#define WATERFALLWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <vector>
#include <../GlobalDefinitions.h>

class QPaintEvent;
class QResizeEvent;

class WaterFallPlot
{
public:
    WaterFallPlot();
    void    updatePlot(const std::vector<_REAL>& vec, _REAL min, _REAL max);
    void    resize(QSize);
    QPixmap pixmap;
protected:
    QImage  image;
};

class WaterfallWidget : public QWidget
{
    Q_OBJECT
public:
    explicit WaterfallWidget(QWidget *parent = NULL);

public slots:
    void     updatePlot(const std::vector<_REAL>& vec, _REAL min, _REAL max);

protected:
    void    paintEvent(QPaintEvent *);
    void    resizeEvent(QResizeEvent *);
    bool    resizeGlitch;
    WaterFallPlot plot;
};

#endif // WATERFALLWIDGET_H
