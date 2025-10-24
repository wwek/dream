#ifndef STREAMWIDGET_H
#define STREAMWIDGET_H

#include "receivercontroller.h"
#include <../Parameter.h>
#include <QWidget>

namespace Ui {
class StreamWidget;
}

class QTreeWidgetItem;

class StreamWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StreamWidget(ReceiverController*, QWidget *parent = 0);
    ~StreamWidget();

private:
    struct Stream {
        bool audio;
        double bitrate;
        int bits; // TODO - UEP

    };

    Ui::StreamWidget *ui;
    Stream stream[4];
    QTreeWidgetItem* msc;
    ChannelConfiguration cc;
    CService service[4];

private slots:
    void setService(int, CService);
    void setChannel(ChannelConfiguration);
};

#endif // STREAMWIDGET_H
