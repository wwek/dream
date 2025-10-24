#ifndef STATIONSWIDGET_H
#define STATIONSWIDGET_H

#include <QWidget>
#include <QTreeWidgetItem>
#include <QMutex>
#include <QTimer>
#include "Schedule.h"
#include "../Parameter.h"
#include "../util-QT/scheduleloader.h"

namespace Ui {
class StationsWidget;
}

class CSettings;
class ReceiverController;
class QNetworkAccessManager;
class QNetworkReply;

/* Classes ********************************************************************/
class CaseInsensitiveTreeWidgetItem : public QTreeWidgetItem
{
public:
    CaseInsensitiveTreeWidgetItem(QTreeWidget* parent=0) : QTreeWidgetItem(parent)
    {
    }

    bool operator< ( const QTreeWidgetItem & rhs) const
    {
        // bug 29 - sort frequency and power numerically
        int col = treeWidget()->sortColumn();
        if (col == 0) // online/offline
            return data(0, Qt::UserRole).toInt() < rhs.data(0, Qt::UserRole).toInt();
        else if (col == 3) // integer frequency
            return text( col ).toInt() < rhs.text( col ).toInt();
        else if (col == 4) // real power
            return text( col ).toDouble() < rhs.text( col ).toDouble();
        else
            return text( col ).toLower() < rhs.text( col ).toLower();
    }
};

class StationsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StationsWidget(ReceiverController*, QWidget *parent = 0);
    ~StationsWidget();

signals:
    void frequencyChanged(int);
    void modeChanged(int);

public slots:
    void SetFrequency(int);
private:
    Ui::StationsWidget* ui;
    CSchedule   schedule;
    ScheduleLoader scheduleLoader;
    QIcon       greenCube,redCube,orangeCube,pinkCube;
    QMutex      mutex;
    QTimer      Timer;
    Qt::SortOrder        sortOrder;
    int         iSortColumn;
    QString     strColumnParam, drm_url;

    void        updateTransmissionStatus();
    void        AddWhatsThisHelp();
    bool        showAll();
    void        loadSchedule();
    void        loadScheduleView(QTreeWidgetItem *parent=0);
    void        loadSettings(const CSettings &settings);
    void        saveSettings(CSettings &settings);

private slots:
    void on_comboBoxFilterTarget_activated(const QString&);
    void on_comboBoxFilterCountry_activated(const QString&);
    void on_comboBoxFilterLanguage_activated(const QString&);
    void on_comboBoxFilterTime_activated(int);
    void OnTimer();
    void OnUpdate();
    void on_stations_itemSelectionChanged();
};

#endif // STATIONSWIDGET_H
