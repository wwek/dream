#ifndef SCHEDULELOADER_H
#define SCHEDULELOADER_H

#include <QObject>
#include <QString>
#include <QNetworkReply>
#include <QNetworkAccessManager>

class ScheduleLoader: public QObject
{
    Q_OBJECT
public:
    ScheduleLoader();
    void fetch(QString, QString);
private:
    QString filename;
    QNetworkAccessManager *manager;

private slots:
    void OnUrlFinished(QNetworkReply*);
signals:
    void fileReady();
};

#endif // SCHEDULELOADER_H
