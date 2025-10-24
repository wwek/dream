#include "stationswidget.h"
#include <ui_stationswidget.h>
#include "../util/Settings.h"
#include "../util-QT/Util.h" /* TODO for ColumnParamToStr and ColumnParamFromStr */
#include "receivercontroller.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDateTime>
#include <QMessageBox>
#include <QFileInfo>
#include <cmath>


StationsWidget::StationsWidget(ReceiverController* rc, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StationsWidget),
    schedule(),scheduleLoader(),
    greenCube(), redCube(":/icons/redCube.png"),
    orangeCube(":/icons/orangeCube.png"), pinkCube(":/icons/pinkCube.png")
{
    ui->setupUi(this);
    for(int i=0; i<ui->comboBoxFilterTime->count(); i++)
    {
        QString l = ui->comboBoxFilterTime->itemText(i);
        if(l == tr("Any"))
            ui->comboBoxFilterTime->setItemData(i, -1);
        if(l == tr("Active"))
            ui->comboBoxFilterTime->setItemData(i, 0);
        if(l.contains("(5"))
            ui->comboBoxFilterTime->setItemData(i, NUM_SECONDS_PREV_5MIN);
        if(l.contains("(15"))
            ui->comboBoxFilterTime->setItemData(i, NUM_SECONDS_PREV_15MIN);
        if(l.contains("(30"))
            ui->comboBoxFilterTime->setItemData(i, NUM_SECONDS_PREV_30MIN);
    }

    QStringList headers;
    headers
            << QString() /* icon, enable sorting by online/offline */
            << tr("Station Name")
            << tr("Time [UTC]")
            << tr("Frequency [kHz]")
            << tr("Power [kW]")
            << tr("Target")
            << tr("Country")
            << tr("Site")
            << tr("Language")
            << tr("Days");

    ui->stations->setHeaderLabels(headers);
    ui->stations->headerItem()->setTextAlignment(2, Qt::AlignRight | Qt::AlignVCenter);
    ui->stations->headerItem()->setTextAlignment(3, Qt::AlignRight | Qt::AlignVCenter);
    ui->stations->headerItem()->setTextAlignment(4, Qt::AlignRight | Qt::AlignVCenter);

    connect(this, SIGNAL(frequencyChanged(int)), rc, SLOT(setFrequency(int)));
    //connect(rc, SIGNAL(mode(int)), this, SLOT(OnSwitchMode(int)));
    connect(rc, SIGNAL(frequencyChanged(int)), this, SLOT(SetFrequency(int)));

    QTreeWidgetItem* drm = new QTreeWidgetItem(QStringList("DRM"));
    QTreeWidgetItem* am = new QTreeWidgetItem(QStringList("AM"));
    ui->stations->addTopLevelItem(drm);
    ui->stations->addTopLevelItem(am);
    QStringList x;
    x << "" << "-";
    QTreeWidgetItem* drm_manual = new QTreeWidgetItem(x);
    drm->addChild(drm_manual);
    QTreeWidgetItem* am_manual = new QTreeWidgetItem(x);
    am->addChild(am_manual);
    drm_manual->setFlags(drm_manual->flags() | Qt::ItemIsEditable);
    am_manual->setFlags(am_manual->flags() | Qt::ItemIsEditable);
}

StationsWidget::~StationsWidget()
{
    delete ui;
}

void StationsWidget::SetFrequency(int f)
{
    (void)f; // TODO
//    ui->frequency->setValue(f);
}

void StationsWidget::on_comboBoxFilterTarget_activated(const QString& s)
{
    schedule.setTargetFilter(s);
    updateTransmissionStatus();
}

void StationsWidget::on_comboBoxFilterCountry_activated(const QString& s)
{
    schedule.setCountryFilter(s);
    updateTransmissionStatus();
}

void StationsWidget::on_comboBoxFilterLanguage_activated(const QString& s)
{
    schedule.setLanguageFilter(s);
    updateTransmissionStatus();
}

void StationsWidget::on_comboBoxFilterTime_activated(int n)
{
    int secs = ui->comboBoxFilterTime->itemData(n).toInt();
    schedule.SetSecondsPreview(secs);
    updateTransmissionStatus();
}

void StationsWidget::loadSchedule()
{
    schedule.LoadSchedule(DRMSCHEDULE_INI_FILE_NAME);
}

void StationsWidget::OnTimer()
{
    /* Get current UTC time */
    time_t ltime;
    time(&ltime);

    /* reload schedule on minute boundaries */
    if (ltime % 60 == 0)
        updateTransmissionStatus();
}

void StationsWidget::OnUpdate()
{
    if (!isVisible())
        return;

    if (schedule.GetNumberOfStations()==0)
    {
        if(QFile::exists(DRMSCHEDULE_INI_FILE_NAME))
        {
            loadSchedule();
        }
        else
        {
            QMessageBox::information(this, "Dream", tr("The schedule file "
                                     " could not be found or contains no data.\n"
                                     "No stations can be displayed.\n"
                                     "Try to download this file by using the 'Update' menu."),
                                     QMessageBox::Ok);
        }
    }
    loadScheduleView();
    updateTransmissionStatus();
}

void StationsWidget::loadSettings(const CSettings& settings)
{
    bool showAll = settings.Get("Stations", "showall", false);
    int iPrevSecs = settings.Get("Stations", "preview", NUM_SECONDS_PREV_5MIN);
    schedule.SetSecondsPreview(iPrevSecs);
    if(showAll)
        ui->comboBoxFilterTime->setCurrentIndex(0);
    else
        ui->comboBoxFilterTime->setCurrentIndex(ui->comboBoxFilterTime->findData(iPrevSecs));

    ui->stations->sortItems(iSortColumn, sortOrder);

    schedule.SetSecondsPreview(settings.Get("Stations","preview", NUM_SECONDS_PREV_5MIN));
    iSortColumn = settings.Get("Stations","sortcolumn", 0);
    sortOrder = Qt::SortOrder(settings.Get("Stations","sortascending", 0));
    strColumnParam = settings.Get("Stations","columnparam", string()).c_str();

    drm_url = settings.Get("Stations","DRM URL", string(DRM_SCHEDULE_URL)).c_str();
    schedule.setTargetFilter(settings.Get("Stations","targetfilter", string()).c_str());
    schedule.setLanguageFilter(settings.Get("Stations","languagefilter", string()).c_str());
    schedule.setCountryFilter(settings.Get("Stations","countryfilter", string()).c_str());
}

void StationsWidget::saveSettings(CSettings& settings)
{
    settings.Put("Stations","targetfilter", schedule.getTargetFilter().toStdString());
    settings.Put("Stations","countryfilter", schedule.getCountryFilter().toStdString());
    settings.Put("Stations","languagefilter", schedule.getLanguageFilter().toStdString());

    /* Store preview settings */
    settings.Put("Stations","preview", schedule.GetSecondsPreview());

    settings.Put("Stations","sortcolumn", iSortColumn);
    settings.Get("Stations","sortascending", sortOrder==Qt::AscendingOrder);
    settings.Put("Stations","columnparam", strColumnParam.toStdString());
}

void StationsWidget::loadScheduleView(QTreeWidgetItem* parent)
{
    if(parent==NULL)
        ui->stations->clear();
    for (int i = 0; i < schedule.GetNumberOfStations(); i++)
    {
        const CStationsItem& station = schedule.GetItem(i);

        /* Get power of the station. We have to do a special treatment
        * here, because we want to avoid having a "0" in the list when
        * a "?" was in the schedule-ini-file */
        const _REAL rPower = station.rPower;

        QString strPower;
        if (rPower == (_REAL) 0.0)
            strPower = "?";
        else
            strPower.setNum(rPower);

        QString strTimes = QString().sprintf("%04d-%04d", station.StartTime(), station.StopTime());

        /* Generate new list station with all necessary column entries */
        QTreeWidgetItem* item = new CaseInsensitiveTreeWidgetItem();
        item->setText(1, station.strName);
        item->setText(2, strTimes /* time */);
        item->setText(3, QString().setNum(station.iFreq) /* freq. */);
        item->setText(4, strPower            /* power */);
        item->setText(5, station.strTarget   /* target */);
        item->setText(6, station.strCountry  /* country */);
        item->setText(7, station.strSite     /* site */);
        item->setText(8, station.strLanguage /* language */);
        item->setText(9, station.strDaysShow);
        item->setData(1, Qt::UserRole, i);
        item->setTextAlignment(2, Qt::AlignRight | Qt::AlignVCenter);
        item->setTextAlignment(3, Qt::AlignRight | Qt::AlignVCenter);
        item->setTextAlignment(4, Qt::AlignRight | Qt::AlignVCenter);
        if(parent)
            parent->addChild(item);
        else
            ui->stations->addTopLevelItem(item);
    }
    ui->comboBoxFilterTarget->clear();
    ui->comboBoxFilterCountry->clear();
    ui->comboBoxFilterLanguage->clear();
    ui->comboBoxFilterTarget->addItems(schedule.ListTargets);
    ui->comboBoxFilterCountry->addItems(schedule.ListCountries);
    ui->comboBoxFilterLanguage->addItems(schedule.ListLanguages);

    ui->comboBoxFilterTarget->setCurrentIndex(ui->comboBoxFilterTarget->findText(schedule.getTargetFilter()));
    ui->comboBoxFilterCountry->setCurrentIndex(ui->comboBoxFilterCountry->findText(schedule.getCountryFilter()));
    ui->comboBoxFilterLanguage->setCurrentIndex(ui->comboBoxFilterLanguage->findText(schedule.getLanguageFilter()));
}

void StationsWidget::updateTransmissionStatus()
{
    return; // until rewritten for heirarchy of items.
    Timer.stop();
    mutex.lock();
    bool bShowAll = showAll();
    ui->stations->setSortingEnabled(false);
    for (int i = 0; i < ui->stations->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = ui->stations->topLevelItem(i);
        int scheduleItem = item->data(1, Qt::UserRole).toInt();

        Station::EState iState = schedule.GetState(scheduleItem);

        switch (iState)
        {
        case Station::IS_ACTIVE:
            item->setData(0, Qt::UserRole, 1);
            item->setIcon(0, greenCube);
            break;
        case Station::IS_PREVIEW:
            item->setData(0, Qt::UserRole, 2);
            item->setIcon(0, orangeCube);
            break;
        case Station::IS_SOON_INACTIVE:
            item->setData(0, Qt::UserRole, 0);
            item->setIcon(0, pinkCube);
            break;
        case Station::IS_INACTIVE:
            item->setData(0, Qt::UserRole, 3);
            item->setIcon(0, redCube);
            break;
        default:
            item->setData(0, Qt::UserRole, 4);
            item->setIcon(0, redCube);
            break;
        }
        if(schedule.CheckFilter(scheduleItem) && (bShowAll || (iState != Station::IS_INACTIVE)))
        {
            item->setHidden(false);
        }
        else
        {
            item->setHidden(true);
        }
    }
    ui->stations->setSortingEnabled(true);
    ui->stations->sortItems(iSortColumn, sortOrder);
    ui->stations->setFocus();
    mutex.unlock();
    Timer.start();
}

void StationsWidget::on_stations_itemSelectionChanged()
{
    QList<QTreeWidgetItem *> items =  ui->stations->selectedItems();

    if(items.size()==1)
    {
        QTreeWidgetItem* w = items.first();
        if(w->parent()==NULL)
        {
            QString col1 = w->text(0);
            if(col1 == "DRM")
            {
                schedule.LoadSchedule(DRMSCHEDULE_INI_FILE_NAME);
                loadScheduleView(w);
                updateTransmissionStatus();
            }
            if(col1 == "AM")
            {
                schedule.LoadSchedule(AMSCHEDULE_CSV_FILE_NAME);
                loadScheduleView(w);
                updateTransmissionStatus();
            }
        }
        else
        {
            int iFreq = QString(items.first()->text(3)).toInt();
            emit frequencyChanged(iFreq);
        }
    }
}

void StationsWidget::AddWhatsThisHelp()
{
    /* Stations List */
    QString strList =
        tr("<b>Stations List:</b> In the stations list "
           "view all DRM stations which are stored in the schedule.ini file "
           "are shown. It is possible to show only active stations by changing a "
           "setting in the 'view' menu. The color of the cube on the left of a "
           "menu item shows the current status of the DRM transmission. A green "
           "box shows that the transmission takes place right now, a "
           //           "yellow cube shows that this is a test transmission and with a "
           "red cube it is shown that the transmission is offline, "
           "a pink cube shown that the transmission soon will be offline.<br>"
           "If the stations preview is active an orange box shows the stations "
           "that will be active.<br>"
           "The list can be sorted by clicking on the headline of the "
           "column.<br>By clicking on a menu item, a remote front-end can "
           "be automatically switched to the current frequency and the "
           "Dream software is reset to a new acquisition (to speed up the "
           "synchronization process). Also, the log-file frequency edit "
           "is automatically updated.");

    ui->stations->setWhatsThis(strList);
}

bool StationsWidget::showAll()
{
    return ui->comboBoxFilterTime->currentIndex()==0;
}
