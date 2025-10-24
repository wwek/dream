#include "scheduleloader.h"
#include <QMessageBox>
#include <QNetworkRequest>
#include <QFile>
#include <QUrl>

ScheduleLoader::ScheduleLoader():QObject(),filename(),manager(new QNetworkAccessManager(this))
{
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(OnUrlFinished(QNetworkReply*)));
}

void ScheduleLoader::fetch(QString url, QString file)
{
    filename = file;
    if (QMessageBox::information(NULL, tr("Dream Schedule Update"),
                                 tr("Dream tries to download the newest schedule\n"
                                    "Your computer must be connected to the internet.\n\n"
                                    "The current file will be overwritten.\n"
                                    "Do you want to continue?"),
                                 QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes )
    {
        /* Try to download the current schedule. Copy the file to the
        current working directory (which is "QDir().absFilePath(NULL)") */
        manager->get(QNetworkRequest(QUrl(url)));
    }
}

void ScheduleLoader::OnUrlFinished(QNetworkReply* reply)
{
    if(reply->error()==QNetworkReply::NoError)
    {
        QFile f(filename);
        if(f.open(QIODevice::WriteOnly)) {
            f.write(reply->readAll());
            f.close();
            /* Notify the user that update was successful */
            QMessageBox::information(NULL, "Dream", tr("Update successful."), QMessageBox::Ok);
            emit fileReady();
        } else {
            QMessageBox::information(NULL, "Dream", tr("Can't save new schedule"), QMessageBox::Ok);
        }
    }
    else
    {
        QMessageBox::information(NULL, "Dream",
                                 tr("Update failed. The following things may have caused the "
                                    "failure:\n"
                                    "\t- the internet connection was not set up properly\n"
                                    "\t- the server is currently not available\n"
                                    "\t- the file 'schedule.ini' could not be written"),
                                 QMessageBox::Ok);
    }
}
