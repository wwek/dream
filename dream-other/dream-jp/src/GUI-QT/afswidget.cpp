#include "afswidget.h"
#include "receivercontroller.h"

AFSWidget::AFSWidget(ReceiverController* rc, QWidget *parent) :
    QTreeWidget(parent)
{
    (void)rc; // TODO
}

void sdItem(QTreeWidgetItem* w, const CServiceDefinition& sd, const CAltFreqSign& a)
{
    (void)a; // TODO
    for(size_t i=0; i<sd.veciFrequencies.size(); i++)
    {
        int f = sd.veciFrequencies[i];
        QStringList l;
        l << QString("%1 MHz").arg(double(f)/1000.0);
        QTreeWidgetItem* item = new QTreeWidgetItem(l);
        w->addChild(item);
    }
    //sd. iRegionID;
    //sd. iScheduleID;
    //sd. iSystemID;
}

void AFSWidget::setAFS(const CAltFreqSign& a)
{
    clear();
    vector<size_t> sfn;
    for(size_t i=0; i<a.vecMultiplexes.size(); i++)
    {
        if(a.vecMultiplexes[i].bIsSyncMultplx)
        {
            sfn.push_back(i);
        }
    }
    if(sfn.size()>0)
    {
        QStringList l;
        l << "This SFN";
        QTreeWidgetItem* item = new QTreeWidgetItem(l);
        addTopLevelItem(item);
        for(size_t i=0; i<sfn.size(); i++)
        {
            sdItem(item, a.vecMultiplexes[sfn[i]], a);
        }
    }
    vector<size_t> mfn;
    for(size_t i=0; i<a.vecMultiplexes.size(); i++)
    {
        if(a.vecMultiplexes[i].bIsSyncMultplx==false)
        {
            mfn.push_back(i);
        }
    }
    if(mfn.size()>0)
    {
        QStringList l;
        l << "Related Networks";
        QTreeWidgetItem* item = new QTreeWidgetItem(l);
        addTopLevelItem(item);
        for(size_t i=0; i<sfn.size(); i++)
        {
            sdItem(item, a.vecMultiplexes[sfn[i]], a);
            //vector<int> veciServRestrict;
        }
    }
    if(a.vecOtherServices.size()>0)
    {
        QStringList l;
        l << "Other";
        QTreeWidgetItem* item = new QTreeWidgetItem(l);
        addTopLevelItem(item);
        for(vector < COtherService >::const_iterator i=a.vecOtherServices.begin(); i!=a.vecOtherServices.end(); i++)
        {
            sdItem(item, *i, a);
            //bool bSameService;
            //int iShortID;
            //uint32_t iServiceID;
        }
    }
}
