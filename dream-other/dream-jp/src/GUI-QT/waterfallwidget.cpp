#include "waterfallwidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <../matlib/MatlibStdToolbox.h>
#include <algorithm>

/* Init some constants */
static const int maxHue = 359; /* Range of "Hue" is 0-359 */
static const int maxSat = 255; /* Range of saturation is 0-255 */

static QColor fromReal(_REAL val)
{
    int hue = maxHue - 60 - int(Round(val * maxHue));

    /* Prevent out-of-range */
    if(hue>maxHue) hue = maxHue;
    if (hue < 0) hue = 0;

    /* Also change saturation to get dark colors when low level */
    int sat = int((1 - double(hue) / maxHue) * maxSat);
    /* Prevent out-of-range */
    if (sat > maxSat) sat = maxSat;
    if (sat < 0) sat = 0;

    return QColor::fromHsv(hue, sat, sat);
}
#if 0
SimpleWaterfallWidget::SimpleWaterfallWidget(QWidget *parent) :
    QWidget(parent),Canvas()
{
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setCursor(Qt::CrossCursor);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void SimpleWaterfallWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    // let the painter scale the data to the window
    painter.setWindow(Canvas.rect());
    painter.drawPixmap(0, 0, Canvas);
}

void SimpleWaterfallWidget::resizeEvent(QResizeEvent *e)
{
    if(e->size().height() != e->oldSize().height())
    {
        // increase/decrease the vertical history
        QPixmap tmp = Canvas;

        Canvas = QPixmap(Canvas.width(), e->size().height());
        Canvas.fill(QColor::fromRgb(0, 0, 0));

        QRect r(QPoint(0,0), tmp.size());

        QPainter p(&Canvas);
        p.drawPixmap(r, tmp, r);
    }
}

void SimpleWaterfallWidget::updatePlot(const vector<_REAL>& vec, _REAL min, _REAL max)
{
    if(int(vec.size()) != Canvas.width())
    {
        // size the canvas to the data
        Canvas = QPixmap(vec.size(), height());

        // always use a black background
        Canvas.fill(QColor::fromRgb(0, 0, 0));
    }

    /* Scroll Canvas */
    Canvas.scroll(0, 1, Canvas.rect());

    QPainter painter;
    painter.begin(&Canvas);

    /* Actual waterfall data */
    for (size_t i = 0; i <  vec.size(); i++)
    {
        /* Translate dB-values to colors */
        _REAL norm = (vec[i] - min) / (max - min);

        /* Generate pixel */
        painter.setPen(fromReal(norm));
        painter.drawPoint(i, 0);
    }

    painter.end();
    update();
}
#endif

WaterFallPlot::WaterFallPlot()
{

}

void WaterFallPlot::updatePlot(const std::vector<_REAL>& vec, _REAL min, _REAL max)
{
    int w = image.width();
    if (w == 0)
        return;

    QPainter painter;
    if(!painter.begin(&pixmap)) // David had problems on Linux without this
        return;

    /* Scroll Canvas */
    pixmap.scroll(0, 1, pixmap.rect());

    // cast from u_char (8bits wide) to QRgb (probably 24bits wide)
    QRgb* scanLine = ((QRgb*)image.scanLine(0));

    // scaling factor between data and image
    _REAL ratio = _REAL(vec.size())/_REAL(w);
    if(ratio<1.0)
    {
        ratio = 1.0; // TODO handle stretching
    }
    vector<_REAL>::const_iterator vi = vec.begin();
    size_t l=0; // index of left edge of bin
    for(int i=0; i<w; i++)
    {
        // calculate right edge of current bin - not all bins are same width for all possible ratios
        size_t r = size_t((i+1)*ratio); // index of right edge (ie left edge of following bin
        vector<_REAL>::const_iterator biggest = max_element(vi+l, vi+r);
        // reduce dB value by 6 dB to avoid FSD and normalise
        _REAL norm = ((*biggest) - 6.0 - min) / (max - min);
        // Translate normalised value to a color and generate pixel
        scanLine[i] = fromReal(norm).rgb();
        l = r;
    }
    painter.drawImage(0, 0, image);
    painter.end();
}

void WaterFallPlot::resize(QSize size)
{
    QPixmap tmp = pixmap;
    pixmap = QPixmap(size);
    image = QImage(size.width(), 1, QImage::Format_RGB32);

    // first time ever - initialise to black
    if(tmp.size().width()==0)
    {
        tmp = QPixmap(size);
        // always use a black background
        tmp.fill(QColor::fromRgb(0, 0, 0));
    }
    // vertical stretch - fill below old with black
    if(pixmap.height()>tmp.height())
        pixmap.fill(QColor::fromRgb(0, 0, 0));
    // copy old data, scaling to fit horizontally but making space below
    // TODO - the width scaling is slightly off.
    QPainter p(&pixmap);
    QPoint origin(0,0);
    QSize top(pixmap.width(), tmp.height());
    p.drawPixmap(QRect(origin, top), tmp);

}

WaterfallWidget::WaterfallWidget(QWidget *parent) :
    QWidget(parent), resizeGlitch(false)
{
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setCursor(Qt::CrossCursor);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void WaterfallWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, plot.pixmap);
}

void WaterfallWidget::resizeEvent(QResizeEvent *e)
{
    // prevent resize glitch when the widget is created,
    // the fist resizeEvent() is bogus
    if(!resizeGlitch)
    {
        resizeGlitch = true;
        return;
    }

    plot.resize(e->size());
}

void WaterfallWidget::updatePlot(const vector<_REAL>& vec, _REAL min, _REAL max)
{
    plot.updatePlot(vec, min, max);
    update();
}
