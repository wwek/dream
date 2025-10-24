#include "serviceselector.h"
#include "ui_serviceselector.h"
#include "../Parameter.h"
#include "../util-QT/Util.h"
#include <../datadecoding/DataDecoder.h>
#include "ThemeCustomizer.h"

ServiceSelector::ServiceSelector(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ServiceSelector),
    pAudioButtonGroup(NULL),pDataButtonGroup(NULL),serviceLabels(4),buttons(4)
{
    ui->setupUi(this);
    pAudioButtonGroup = new QButtonGroup(this);
    pDataButtonGroup = new QButtonGroup(this);
    pAudioButtonGroup->setExclusive(true);
    pDataButtonGroup->setExclusive(true); // shouldn't be but that's how Dream currently written

    connect(pAudioButtonGroup, SIGNAL(buttonClicked(int)), this, SIGNAL(audioServiceSelected(int)));
    connect(pDataButtonGroup, SIGNAL(buttonClicked(int)), this, SIGNAL(dataServiceSelected(int)));

    buttons[0] = ui->PushButtonService1;
    buttons[1] = ui->PushButtonService2;
    buttons[2] = ui->PushButtonService3;
    buttons[3] = ui->PushButtonService4;

    serviceLabels[0] = ui->TextMiniService1;
    serviceLabels[1] = ui->TextMiniService2;
    serviceLabels[2] = ui->TextMiniService3;
    serviceLabels[3] = ui->TextMiniService4;

    /* Service Selectors */
    const QString strServiceSel =
        tr("<b>Service Selectors:</b> In a DRM stream up to "
           "four services can be carried. The service can be an audio service, "
           "a data service or an audio service with data. "
           "Audio services can have associated text messages, in addition to any data component. "
           "If a Multimedia data service is selected, the Multimedia Dialog will automatically show up. "
           "On the right of each service selection button a short description of the service is shown. "
           "If an audio service has associated Multimedia data, \"+ MM\" is added to this text. "
           "If such a service is selected, opening the Multimedia Dialog will allow the data to be viewed "
           "while the audio is still playing. If the data component of a service is not Multimedia, "
           "but an EPG (Electronic Programme Guide) \"+ EPG\" is added to the description. "
           "The accumulated Programme Guides for all stations can be viewed by opening the Programme Guide Dialog. "
           "The selected channel in the Programme Guide Dialog defaults to the station being received. "
           "If Alternative Frequency Signalling is available, \"+ AFS\" is added to the description. "
           "In this case the alternative frequencies can be viewed by opening the Live Schedule Dialog."
          );
    ui->PushButtonService1->setWhatsThis(strServiceSel);
    ui->PushButtonService2->setWhatsThis(strServiceSel);
    ui->PushButtonService3->setWhatsThis(strServiceSel);
    ui->PushButtonService4->setWhatsThis(strServiceSel);
    ui->TextMiniService1->setWhatsThis(strServiceSel);
    ui->TextMiniService2->setWhatsThis(strServiceSel);
    ui->TextMiniService3->setWhatsThis(strServiceSel);
    ui->TextMiniService4->setWhatsThis(strServiceSel);

    APPLY_CUSTOM_THEME_UI();
}

ServiceSelector::~ServiceSelector()
{
    delete ui;
}

void ServiceSelector::onServiceChanged(int short_id, const CService& service)
{

    pAudioButtonGroup->removeButton(buttons[short_id]);
    pDataButtonGroup->removeButton(buttons[short_id]);

    QString text;

    /* Check, if service is used */
    if (service.IsActive())
    {
        /* Do UTF-8 to string conversion with the label strings */
        QString strLabel = QString().fromUtf8(service.strLabel.c_str());

        /* Label for service selection button (service label, codec
           and Mono / Stereo information) */
        QString strCodec = GetCodecString(service);
        QString strType = GetTypeString(service);
        text = strLabel;
        if (!strCodec.isEmpty() || !strType.isEmpty())
            text += "  |   " + strCodec + " " + strType;

        /* Bit-rate (only show if greater than 0) */
        if (service.AudioParam.rBitRate > (_REAL) 0.0)
        {
            text += " (" + QString().setNum(service.AudioParam.rBitRate, 'f', 2) + " kbps)";
        }

        /* Audio service */
        if ((service.eAudDataFlag == CService::SF_AUDIO))
        {            
            pAudioButtonGroup->addButton(buttons[short_id], short_id);

            /* Report missing codec */
            if (!service.AudioParam.bCanDecode)
                text += tr(" [no codec available]");

            /* Show, if a multimedia stream is connected to this service */
            if (service.DataParam.iStreamID != STREAM_ID_NOT_USED)
            {
                // auxiliary data service on an audio service
                // don't pop up the window
                //pDataButtonGroup->addButton(buttons[short_id], short_id);

                if (service.DataParam.iUserAppIdent == DAB_AT_EPG)
                    text += tr(" + EPG"); /* EPG service */
                else
                {
                    text += tr(" + MM"); /* other multimedia service */
                }

                /* Bit-rate of connected data stream */
                text += " (" + QString().setNum(service.DataParam.rBitRate, 'f', 2) + " kbps)";
            }
        }
        /* Data service */
        else
        {
            pDataButtonGroup->addButton(buttons[short_id], short_id);

            if (service.DataParam.ePacketModInd == CDataParam::PM_PACKET_MODE)
            {
                if (service.DataParam.eAppDomain == CDataParam::AD_DAB_SPEC_APP)
                {
                    switch (service.DataParam.iUserAppIdent)
                    {
                    case DAB_AT_BROADCASTWEBSITE:
                    case DAB_AT_JOURNALINE:
                    case DAB_AT_MOTSLIDESHOW:
                        break;
                    }
                }
            }
        }
    }
    serviceLabels[short_id]->setText(text);
    buttons[short_id]->setEnabled(text != "");
}

void ServiceSelector::check(int i)
{
    //pButtonGroup->button(i)->setChecked(true);
}

void ServiceSelector::disableAll()
{
    for(size_t i=0; i<serviceLabels.size(); i++)
    {
        buttons[i]->setEnabled(false);
        buttons[i]->setChecked(false);
        serviceLabels[i]->setText("");
    }
}
