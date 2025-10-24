#include "engineeringtabwidget.h"
#include "channelwidget.h"
#include "receivercontroller.h"
#include <QLabel>

EngineeringTabWidget::EngineeringTabWidget(ReceiverController* controller, QWidget *parent) :
    QTabWidget(parent)
{
    int iPlotStyle = 0;// TODO set from menu
    ChannelWidget* pCh = new ChannelWidget(controller);
    pCh->setPlotStyle(iPlotStyle);
    controller->setControls(); // new controls so fill their values from the receiver controller
    //connect(parent, SIGNAL(plotStyleChanged(int)), pCh, SLOT(setPlotStyle(int)));
    addTab(pCh, "Channel");
    addTab(new QLabel("Streams"), "Streams");
    addTab(new QLabel("AFS"), "AFS");
    addTab(new QLabel("GPS"), "GPS");
}
