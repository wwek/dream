#include "streamwidget.h"
#include "ui_streamwidget.h"
#include <../Parameter.h>

StreamWidget::StreamWidget(ReceiverController* rc, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StreamWidget),
    stream(),
    msc(NULL),
    cc(),
    service()
{
    ui->setupUi(this);
    QStringList headers;
    headers
            << tr("Stream")
            << tr("Type")
            << tr("bit/s");
    ui->treeWidget->setHeaderLabels(headers);
    QStringList l;
    l.clear();
    l << "FAC" << tr("Fast Access Channel") << "160";
    ui->treeWidget->addTopLevelItem(new QTreeWidgetItem(l));
    l.clear();
    l << "SDC" << tr("Service Description Channel") << "?";
    ui->treeWidget->addTopLevelItem(new QTreeWidgetItem(l));
    l.clear();
    l << "MSC" << tr("Main Service Channel") << "?";
    ui->treeWidget->addTopLevelItem(msc = new QTreeWidgetItem(l));
    connect(rc, SIGNAL(serviceChanged(int,CService)), this, SLOT(setService(int, CService)));
    connect(rc, SIGNAL(channelConfigurationChanged(ChannelConfiguration)), this, SLOT(setChannel(ChannelConfiguration)));
    //CParameter* p = rc->getReceiver()->GetParameters();
}

StreamWidget::~StreamWidget()
{
    delete ui;
}

void StreamWidget::setService(int n, CService s)
{
    service[n] = s;
    int sid = s.AudioParam.iStreamID;
    if(sid!=STREAM_ID_NOT_USED)
    {
        stream[sid].audio=true;
        QString id = QString("%1").arg(sid);
        QList<QTreeWidgetItem*> w = ui->treeWidget->findItems(id, Qt::MatchFixedString|Qt::MatchRecursive);
        if(w.count()==0)
        {
            QStringList l;
            l << id;
            if(s.AudioParam.bTextflag)
                l << tr("audio+text");
            else
                l << tr("audio");
            l << QString("%1").arg(1000.0*s.AudioParam.rBitRate);
            msc->addChild(new QTreeWidgetItem(l));
        }
    }
    sid = s.DataParam.iStreamID;
    if(sid!=STREAM_ID_NOT_USED)
    {
        stream[sid].audio=false;
        QString id = QString("%1").arg(sid);
        QList<QTreeWidgetItem*> w = ui->treeWidget->findItems(id, Qt::MatchFixedString|Qt::MatchRecursive);
        QTreeWidgetItem* str;
        if(w.count()==0)
        {
            QStringList l;
            l << id;
            if(s.DataParam.ePacketModInd)
            {
                l << QString(tr("%1 byte packets")).arg(s.DataParam.iPacketLen);
            }
            else
            {
                l << tr("data stream");
            }
            l << "?";
            msc->addChild(str=new QTreeWidgetItem(l));
        }
        else
        {
            str = w.first();
        }
        if(s.DataParam.ePacketModInd)
        {
            QString p = QString("%1/%2").arg(sid).arg(s.DataParam.iPacketID);
            QList<QTreeWidgetItem*> wp = ui->treeWidget->findItems(p, Qt::MatchFixedString|Qt::MatchRecursive);
            if(wp.count()==0)
            {
                QStringList l;
                l << p << tr("packet mode app") << "?";
                str->addChild(new QTreeWidgetItem(l));
            }
        }
    }
}

void StreamWidget::setChannel(ChannelConfiguration c)
{
    cc = c;
}
