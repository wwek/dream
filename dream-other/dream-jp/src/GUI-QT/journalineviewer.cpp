#include "journalineviewer.h"
#include "ui_journalineviewer.h"
#include <../util-QT/Util.h>
#include <QFontDialog>

JournalineViewer::JournalineViewer(int s, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::JournalineViewer),
    document(),short_id(s)
{
    ui->setupUi(this);
    ui->textBrowser->setDocument(&document);
    ui->textBrowser->setSource(QUrl("0"));

    /* Update time for color LED */
    ui->LEDStatus->SetUpdateTime(1000);

    /* Connect controls */
    connect(ui->textBrowser, SIGNAL(backwardAvailable(bool)), ui->buttonStepBack, SLOT(setEnabled(bool)));
    on_buttonClearCache_clicked();
}

JournalineViewer::~JournalineViewer()
{
    delete ui;
}

void JournalineViewer::on_buttonClearCache_clicked()
{
    //actionClear_All->setEnabled(false);
    ui->buttonStepBack->setEnabled(false);
    ui->textBrowser->clear();
    ui->textBrowser->clearHistory();
    // TODO - clear JL object cache ?
}

void JournalineViewer::setSavePath(const QString&)
{
    //    strCurrentSavePath = s;
}

void JournalineViewer::setDecoder(CDataDecoder* dec)
{
    ui->textBrowser->setDecoder(dec);
}

void JournalineViewer::setServiceInformation(const CService& service, uint32_t iAudioServiceID)
{
    /* Add the service description into the dialog caption */
    QString strTitle = tr("Journaline");

    if (service.IsActive())
    {
        /* Do UTF-8 to QString (UNICODE) conversion with the label strings */
        QString strLabel = QString().fromUtf8(service.strLabel.c_str()).trimmed();

        /* Service ID (plot number in hexadecimal format) */
        QString strServiceID = "";

        /* show the ID only if differ from the audio service */
        if ((service.iServiceID != 0) && (service.iServiceID != iAudioServiceID))
        {
            if (strLabel != "")
                strLabel += " ";

            strServiceID = "- ID:" +
                           QString().setNum(long(service.iServiceID), 16).toUpper();
        }

        /* add the description on the title of the dialog */
        if (strLabel != "" || strServiceID != "")
            strTitle += " [" + strLabel + strServiceID + "]";
    }
    setWindowTitle(strTitle);
}

void JournalineViewer::setStatus(int s, ETypeRxStatus status)
{
    if(s==short_id)
    {
        SetStatus(ui->LEDStatus, status);

        if(ui->textBrowser->changed())
        {
            ui->textBrowser->reload();
        }
    }
}
