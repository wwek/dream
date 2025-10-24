#include "audiodetailwidget.h"
#include "ui_audiodetailwidget.h"
#include <../util-QT/Util.h>
#include <../tables/TableFAC.h>
#include <../DrmReceiver.h>
#include "receivercontroller.h"
#include "DRMPlot.h"
#include <QFileDialog>
#include <QDebug>

AudioDetailWidget::AudioDetailWidget(ReceiverController* rc, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AudioDetailWidget),short_id(-1),
    engineeringMode(false),pMainPlot(NULL),controller(rc),iPlotStyle(0)
{
    ui->setupUi(this);
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(rc, SIGNAL(serviceChanged(int,CService)), this, SLOT(updateDisplay(int,CService)));
}

AudioDetailWidget::~AudioDetailWidget()
{
    delete ui;
}

void AudioDetailWidget::addItem(const QString& key, const QString& val)
{
    ui->treeWidget->addTopLevelItem((new QTreeWidgetItem(QStringList() << key << val)));
}

void AudioDetailWidget::on_new_data()
{
    /*
    if(sender())
    {
        ReceiverController* rc = qobject_cast<ReceiverController*>(sender());
    }*/
    if(pMainPlot)
        pMainPlot->update(controller);
}

void AudioDetailWidget::setEngineering(bool eng)
{
    engineeringMode = eng;
    if(eng)
    {
        if(pMainPlot==NULL)
        {
            pMainPlot = new CDRMPlot();
            ui->plotLayout->addWidget(pMainPlot->widget());
            pMainPlot->SetPlotStyle(iPlotStyle);
            pMainPlot->SetupChart(AUDIO_SPECTRUM, 0);
            connect(controller, SIGNAL(dataAvailable()), this, SLOT(on_new_data()));
        }
    }
    else
    {
        disconnect(controller, SIGNAL(dataAvailable()), this, SLOT(on_new_data()));
    }
    update();
}

void AudioDetailWidget::setTextMessage(const QString& s)
{
    ui->textMessage->setText(s);
}

void AudioDetailWidget::setRxStatus(int sid, ETypeRxStatus s)
{
    if(sid == short_id)
    {
        (void)s; // TODO
    }
}

void AudioDetailWidget::updateDisplay(int id, CService s)
{
    short_id = id;
    if(engineeringMode)
        updateEngineeringModeDisplay(id, s);
    else
        updateUserModeDisplay(id,s);
}

void AudioDetailWidget::updateUserModeDisplay(int, const CService&)
{
    ui->stackedWidget->setCurrentIndex(0);
}

void AudioDetailWidget::updateEngineeringModeDisplay(int id, const CService& s)
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->treeWidget->clear();
    addItem( tr("Codec"),  GetCodecString(s));
    addItem( tr("Mode"),  GetTypeString(s));
    addItem( tr("Decodable"),  s.AudioParam.bCanDecode?tr("Yes"):tr("No"));
    addItem( tr("Text Messages"),  s.AudioParam.bTextflag?tr("Yes"):tr("No"));
    addItem( tr("Language Code"),  s.strLanguageCode.c_str());
    addItem( tr("Language"),  GetISOLanguageName(s.strLanguageCode).c_str());
    addItem( tr("Country"),  GetISOCountryName(s.strCountryCode).c_str());
    addItem( tr("Service ID"), QString("%1").arg(s.iServiceID, 6, 16));
    addItem( tr("Conditional Access"),  s.AudioParam.bCA?tr("Yes"):tr("No"));
    addItem( tr("Stream ID"),  QString("%1").arg(s.AudioParam.iStreamID));
    addItem( tr("Short ID"),  QString("%1").arg(id));

    ui->buttonListen->setEnabled(s.AudioParam.bCanDecode);
}

void AudioDetailWidget::on_buttonListen_clicked()
{
    emit listen(short_id);
}

void AudioDetailWidget::on_mute_stateChanged(int state)
{
    /* Set parameter in working thread module */
    controller->getReceiver()->GetWriteData()->MuteAudio(state == Qt::Checked);
}

void AudioDetailWidget::on_reverb_stateChanged(int state)
{
    /* Set parameter in working thread module */
    controller->getReceiver()->GetAudSorceDec()->SetReverbEffect(state == Qt::Checked);
}

void AudioDetailWidget::on_save_stateChanged(int state)
{
    /*
        This code is copied in AnalogDemDlg.cpp. If you do changes here, you should
        apply the changes in the other file, too
    */
    if (state == Qt::Checked)
    {
        /* Show "save file" dialog */
        QString strFileName =
            QFileDialog::getSaveFileName(this, "*.wav", tr("DreamOut.wav"));

        /* Check if user not hit the cancel button */
        if (!strFileName.isEmpty())
        {
            string s = strFileName.toUtf8().constData();
            controller->getReceiver()->GetWriteData()->StartWriteWaveFile(s);
        }
        else
        {
            /* User hit the cancel button, uncheck the button */
            ui->save->setChecked(false);
        }
    }
    else
        controller->getReceiver()->GetWriteData()->StopWriteWaveFile();
}

void AudioDetailWidget::setPlotStyle(int n)
{
    /* Save the new style */
    iPlotStyle = n;
    /* Update main plot window */
    if(pMainPlot)
        pMainPlot->SetPlotStyle(iPlotStyle);
}
