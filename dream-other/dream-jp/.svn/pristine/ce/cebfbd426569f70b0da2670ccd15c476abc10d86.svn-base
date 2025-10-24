#include "dreamtabwidget.h"
#include <QLabel>
#include <QVariant>
#include <QTabBar>
#include "../Parameter.h"
#include <../util-QT/Util.h>
#include "journalineviewer.h"
#ifdef QT_WEBKIT_LIB
# include "bwsviewerwidget.h"
#endif
#include "slideshowwidget.h"
#include "audiodetailwidget.h"
#include "EPGDlg.h"
#include "channelwidget.h"
#include "streamwidget.h"
#include "gpswidget.h"
#include "afswidget.h"
#include "amwidget.h"
#include <../datadecoding/DataDecoder.h>
#include "receivercontroller.h"
#include "stationswidget.h"

#define CHANNEL_POS 128
#define STREAM_POS 129
#define AFS_POS 130
#define GPS_POS 131
#define AM_POS 132
#define AMSS_POS 133
#define MAX_ENGINEERING_POS 192
#define STATIONS_POS 193
#define LIVE_STATIONS_POS 194
#define AM_POS 132

DreamTabWidget::DreamTabWidget(ReceiverController* rc, QWidget *parent) :
    QTabWidget(parent),controller(rc),
    stations(new StationsWidget(rc)),
    eng(false)
{
    add(stations, "Stations", STATIONS_POS);
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(on_currentChanged(int)));
}

void DreamTabWidget::onServiceChanged(int short_id, const CService& service)
{
    // label can be updated independently of other service parameters
    QString l;
    if(service.strLabel=="")
        l = QString("%1").arg(service.iServiceID,4,16,QChar('0'));
    else
        l = QString::fromUtf8(service.strLabel.c_str());

    // ? is count() always the same as tabBar->count() ???????????
    QTabBar* tb = tabBar();
    // if this is an audio service, create it or update the label
    if(service.AudioParam.iStreamID!=STREAM_ID_NOT_USED)
    {
        int audioIndex = -1;
        for(int i=0; i<tb->count(); i++)
        {
            if(tb->tabData(i).toInt()==short_id)
                audioIndex = i;
        }
        if(audioIndex==-1)
        {
            AudioDetailWidget* pApp = new AudioDetailWidget(controller);
            pApp->setEngineering(eng);
            add(pApp, l, short_id);
        }
        else
        {
            setTabText(audioIndex, l);
        }
    }
    if(service.DataParam.iStreamID!=STREAM_ID_NOT_USED)
    {
        // if this is an audio service wih a data component, add the app name to the label
        if(service.eAudDataFlag==CService::SF_AUDIO)
        {
            l = l + " " + GetDataTypeString(service);
        }
        int dataIndex = -1;
        for(int i=0; i<tb->count(); i++)
        {
            if((tb->tabData(i).toInt())==(4+short_id))
                dataIndex = i;
        }
        // if there is a data component, create it or update the label
        if(dataIndex==-1)
        {
            add(makeDataApp(short_id, service), l, 4+short_id);
        }
        else
        {
            setTabText(dataIndex, l);
        }
    }
}

void DreamTabWidget::removeServices()
{
    for(int i=tabBar()->count()-1; i>=0; --i)
    {
        int n = tabBar()->tabData(i).toInt();
        if(n<CHANNEL_POS)
        {
            removeTab(i);
        }
    }
}

void DreamTabWidget::on_currentChanged(int index)
{
    int short_id = tabBar()->tabData(index).toInt();
    if(short_id>=CHANNEL_POS)
        return; // it is not a service tab
    if(short_id<4)
        emit audioServiceSelected(short_id);
    else
        emit dataServiceSelected(short_id-4);
}

void DreamTabWidget::add(QWidget* w, const QString& l, int ordering)
{
    int before = tabBar()->count();
    for(int i=0; i<tabBar()->count(); i++)
    {
        if(tabBar()->tabData(i).toInt()>ordering)
        {
            before=i;
            break;
        }
    }
    int index = insertTab(before, w, l);
    tabBar()->setTabData(index, ordering);
}

QWidget* DreamTabWidget::makeDataApp(int short_id, const CService& service) const
{
    QWidget* pApp = NULL;
    if (service.DataParam.ePacketModInd == CDataParam::PM_PACKET_MODE)
        pApp = makePacketApp(short_id, service);
    if(pApp==NULL)
    {
        pApp = new QLabel(QString("short id %1 stream %2 packet id %3")
                          .arg(short_id)
                          .arg(service.DataParam.iStreamID)
                          .arg(service.DataParam.iPacketID));
    }
    return pApp;
}

QWidget* DreamTabWidget::makePacketApp(int short_id, const CService& service) const
{
    QWidget *pApp = NULL;
    if (service.DataParam.eAppDomain == CDataParam::AD_DAB_SPEC_APP)
    {
        switch (service.DataParam.iUserAppIdent)
        {
        case DAB_AT_MOTSLIDESHOW:
        {
            SlideShowWidget* p = new SlideShowWidget();
            p->setServiceInformation(short_id, service);
            connect(controller, SIGNAL(dataStatusChanged(int, ETypeRxStatus)), p, SLOT(setStatus(int, ETypeRxStatus)));
            pApp = p;
        }
        break;

        case DAB_AT_BROADCASTWEBSITE:
        {
#ifdef QT_WEBKIT_LIB
            BWSViewerWidget* p = new BWSViewerWidget();
            p->setDecoder(service.DataParam.pDecoder);
            p->setServiceInformation(short_id, service);
            connect(controller, SIGNAL(dataStatusChanged(int, ETypeRxStatus)), p, SLOT(setStatus(int, ETypeRxStatus)));
            pApp = p;
#endif
        }
        break;

        case DAB_AT_EPG:
            break;

        case DAB_AT_JOURNALINE:
        {
            JournalineViewer* p = new JournalineViewer(short_id);
            p->setDecoder(service.DataParam.pDecoder);
            p->setServiceInformation(service, service.iServiceID);
            connect(controller, SIGNAL(dataStatusChanged(int, ETypeRxStatus)), p, SLOT(setStatus(int, ETypeRxStatus)));
            pApp = p;
        }
        break;

        default:
            ;
        }
    }
    else if (service.DataParam.eAppDomain == CDataParam::AD_DRM_SPEC_APP)
    {
        switch (service.DataParam.iUserAppIdent)
        {
        case DRM_AT_GINGA:
            /* TODO: make GingaViewerWidget */
            break;

        default:
            ;
        }
    }

    return pApp;
}

void DreamTabWidget::setText(int short_id, const QString& text)
{
    QString t(text);
    Linkify(t);
    QTabBar* tb = tabBar();
    for(int i=0; i<tb->count(); i++)
    {
        int n = tb->tabData(i).toInt();
        if(short_id==n)
        {
            AudioDetailWidget* adw = dynamic_cast<AudioDetailWidget*>(widget(i));
            if(adw)
                adw->setTextMessage(t);
        }
    }
}

void DreamTabWidget::on_engineeringMode(bool b)
{
    if(eng==b)
        return;
    eng = b;
    if(eng)
    {
        int iPlotStyle = 0;// TODO set from menu
        ChannelWidget* pCh = new ChannelWidget(controller);
        pCh->setPlotStyle(iPlotStyle);
        //connect(parent, SIGNAL(plotStyleChanged(int)), pCh, SLOT(setPlotStyle(int)));
        add(pCh, "Channel", CHANNEL_POS);
        add(new StreamWidget(controller), "Streams", STREAM_POS);
        add(new AFSWidget(controller), "AFS", AFS_POS);
        add(new GPSWidget(controller), "GPS", GPS_POS);
        add(new AMWidget(controller), "AM", AM_POS);
        controller->setControls(); // new controls so fill their values from the receiver controller
    }
    else
    {
        for(int i=tabBar()->count()-1; i>=0; --i)
        {
            int n = tabBar()->tabData(i).toInt();
            if((CHANNEL_POS<=n) && (n<=MAX_ENGINEERING_POS))
            {
                removeTab(i);
            }
        }
    }
    QList<AudioDetailWidget*> list = findChildren<AudioDetailWidget*>();
    for (int i = 0; i < list.size(); ++i) {
        list[i]->setEngineering(eng);
    }
}
