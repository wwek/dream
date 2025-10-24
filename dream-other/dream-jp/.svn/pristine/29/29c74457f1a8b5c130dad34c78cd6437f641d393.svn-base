#include "drmdisplay.h"
#include "ui_drmdisplay.h"
#include "../util-QT/Util.h"
#include "DialogUtil.h"

DRMDisplay::DRMDisplay(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DRMDisplay)
{
    ui->setupUi(this);
    /* Init progress bar for input signal level */
    inputLevel = LevelMeter::createLevelMeter();
    ui->levelMeterLayout->addWidget(inputLevel->widget());
    /* Update times for color LEDs */
    ui->CLED_FAC->SetUpdateTime(1500);
    ui->CLED_SDC->SetUpdateTime(1500);
    ui->CLED_MSC->SetUpdateTime(600);
    AddWhatsThisHelp();
}

DRMDisplay::~DRMDisplay()
{
    delete ui;
}

void DRMDisplay::setBars(int bars)
{
    ui->onebar->setAutoFillBackground(bars>0);
    ui->twobars->setAutoFillBackground(bars>1);
    ui->threebars->setAutoFillBackground(bars>2);
    ui->fourbars->setAutoFillBackground(bars>3);
    ui->fivebars->setAutoFillBackground(bars>4);
}

void DRMDisplay::setLevel(double level)
{
    /* Input level meter */
    inputLevel->setLevel(level);
}

void DRMDisplay::showReceptionStatus(ETypeRxStatus fac, ETypeRxStatus sdc, ETypeRxStatus msc)
{
    SetStatus(ui->CLED_FAC, fac);
    SetStatus(ui->CLED_SDC, sdc);
    SetStatus(ui->CLED_MSC, msc);
}

void DRMDisplay::showTextMessage(const QString& textMessage)
{
    /* Activate text window */
    ui->TextTextMessage->setEnabled(true);

    QString formattedMessage = "";
    for (int i = 0; i < (int)textMessage.length(); i++)
    {
        switch (textMessage.at(i).unicode())
        {
        case 0x0A:
        /* Code 0x0A may be inserted to indicate a preferred
           line break */
        case 0x1F:
            /* Code 0x1F (hex) may be inserted to indicate a
               preferred word break. This code may be used to
                   display long words comprehensibly */
            formattedMessage += "<br>";
            break;

        case 0x0B:
            /* End of a headline */
            formattedMessage = "<b><u>"
                               + formattedMessage
                               + "</u></b></center><br><center>";
            break;

        case '<':
            formattedMessage += "&lt;";
            break;

        case '>':
            formattedMessage += "&gt;";
            break;

        default:
            formattedMessage += textMessage[int(i)];
        }
    }
    Linkify(formattedMessage);
    formattedMessage = "<center>" + formattedMessage + "</center>";
    ui->TextTextMessage->setText(formattedMessage);

}

void DRMDisplay::showServiceInfo(const CService& service)
{
    /* Service label (UTF-8 encoded string -> convert) */
    QString ServiceLabel(QString::fromUtf8(service.strLabel.c_str()));
    ui->LabelServiceLabel->setText(ServiceLabel);

    /* Service ID (plot number in hexadecimal format) */
    const long iServiceID = (long) service.iServiceID;

    if (iServiceID != 0)
    {
        ui->LabelServiceID->setText(QString("ID:%1").arg(iServiceID,4,16).toUpper());
    }
    else
        ui->LabelServiceID->setText("");

    /* Codec label */
    ui->LabelCodec->setText(GetCodecString(service));

    /* Type (Mono / Stereo) label */
    ui->LabelStereoMono->setText(GetTypeString(service));

    /* Language and program type labels (only for audio service) */
    if (service.eAudDataFlag == CService::SF_AUDIO)
    {
        /* SDC Language */
        const string strLangCode = service.strLanguageCode;

        if ((!strLangCode.empty()) && (strLangCode != "---"))
        {
            ui->LabelLanguage->
            setText(QString(GetISOLanguageName(strLangCode).c_str()));
        }
        else
        {
            /* FAC Language */
            const int iLanguageID = service.iLanguage;

            if ((iLanguageID > 0) &&
                    (iLanguageID < LEN_TABLE_LANGUAGE_CODE))
            {
                ui->LabelLanguage->setText(
                    strTableLanguageCode[iLanguageID].c_str());
            }
            else
                ui->LabelLanguage->setText("");
        }

        /* Program type */
        const int iProgrammTypeID = service.iServiceDescr;

        if ((iProgrammTypeID > 0) &&
                (iProgrammTypeID < LEN_TABLE_PROG_TYPE_CODE))
        {
            ui->LabelProgrType->setText(
                strTableProgTypCod[iProgrammTypeID].c_str());
        }
        else
            ui->LabelProgrType->setText("");
    }

    /* Country code */
    const string strCntryCode = service.strCountryCode;

    if ((!strCntryCode.empty()) && (strCntryCode != "--"))
    {
        ui->LabelCountryCode->
        setText(QString(GetISOCountryName(strCntryCode).c_str()));
    }
    else
        ui->LabelCountryCode->setText("");
}

/* Bit-rate */
void
DRMDisplay::setBitRate(_REAL rBitRate, _REAL rPartABLenRat)
{
    QString strBitrate = QString().setNum(rBitRate, 'f', 2) + tr(" kbps");

    /* Equal or unequal error protection */
    if (rPartABLenRat != (_REAL) 0.0)
    {
        /* Print out the percentage of part A length to total length */
        strBitrate += " UEP (" +
                      QString().setNum(rPartABLenRat * 100, 'f', 1) + " %)";
    }
    else
    {
        /* If part A is zero, equal error protection (EEP) is used */
        strBitrate += " EEP";
    }
    ui->LabelBitrate->setText(strBitrate);

}


void
DRMDisplay::clearDisplay(const QString& serviceLabel)
{
    ui->LabelServiceLabel->setText(serviceLabel);
    ui->LabelBitrate->setText("");
    ui->LabelCodec->setText("");
    ui->LabelStereoMono->setText("");
    ui->LabelProgrType->setText("");
    ui->LabelLanguage->setText("");
    ui->LabelCountryCode->setText("");
    ui->LabelServiceID->setText("");
}

void DRMDisplay::SetDisplayColor(const QColor& newColor)
{
    /* Collect pointers to the desired controls in a vector */
    vector<QWidget*> vecpWidgets;
    vecpWidgets.push_back(ui->TextTextMessage);
    vecpWidgets.push_back(ui->LabelBitrate);
    vecpWidgets.push_back(ui->LabelCodec);
    vecpWidgets.push_back(ui->LabelStereoMono);
    vecpWidgets.push_back(ui->FrameAudioDataParams);
    vecpWidgets.push_back(ui->LabelProgrType);
    vecpWidgets.push_back(ui->LabelLanguage);
    vecpWidgets.push_back(ui->LabelCountryCode);
    vecpWidgets.push_back(ui->LabelServiceID);
    vecpWidgets.push_back(ui->TextLabelInputLevel);
    //vecpWidgets.push_back(ui->ProgrInputLevel);
    vecpWidgets.push_back(ui->CLED_FAC);
    vecpWidgets.push_back(ui->CLED_SDC);
    vecpWidgets.push_back(ui->CLED_MSC);
    //vecpWidgets.push_back(ui->FrameMainDisplay);

    for (size_t i = 0; i < vecpWidgets.size(); i++)
    {
        /* Request old palette */
        QPalette CurPal(vecpWidgets[i]->palette());

        /* Change colors */
        if (vecpWidgets[i] != ui->TextTextMessage)
        {
            CurPal.setColor(QPalette::Active, QPalette::Text, newColor);
            CurPal.setColor(QPalette::Active, QPalette::Foreground, newColor);
            CurPal.setColor(QPalette::Inactive, QPalette::Text, newColor);
            CurPal.setColor(QPalette::Inactive, QPalette::Foreground, newColor);
        }
        CurPal.setColor(QPalette::Active, QPalette::Button, newColor);
        CurPal.setColor(QPalette::Active, QPalette::Light, newColor);
        CurPal.setColor(QPalette::Active, QPalette::Dark, newColor);

        CurPal.setColor(QPalette::Inactive, QPalette::Button, newColor);
        CurPal.setColor(QPalette::Inactive, QPalette::Light, newColor);
        CurPal.setColor(QPalette::Inactive, QPalette::Dark, newColor);

        /* Special treatment for text message window */
        if (vecpWidgets[i] == ui->TextTextMessage)
        {
            /* We need to specify special color for disabled */
            CurPal.setColor(QPalette::Disabled, QPalette::Light, Qt::black);
            CurPal.setColor(QPalette::Disabled, QPalette::Dark, Qt::black);
        }

        /* Set new palette */
        vecpWidgets[i]->setPalette(CurPal);
    }
}



void DRMDisplay::AddWhatsThisHelp()
{
    /*
        This text was taken from the only documentation of Dream software
    */
    /* Text Message */
    QString strTextMessage =
        tr("<b>Text Message:</b> On the top right the text "
           "message label is shown. This label only appears when an actual text "
           "message is transmitted. If the current service does not transmit a "
           "text message, the label will be disabled.");

    /* Input Level */
    const QString strInputLevel =
        tr("<b>Input Level:</b> The input level meter shows "
           "the relative input signal peak level in dB. If the level is too high, "
           "the meter turns from green to red. The red region should be avoided "
           "since overload causes distortions which degrade the reception "
           "performance. Too low levels should be avoided too, since in this case "
           "the Signal-to-Noise Ratio (SNR) degrades.");


    /* Status LEDs */
    const QString strStatusLEDS =
        tr("<b>Status LEDs:</b> The three status LEDs show "
           "the current CRC status of the three logical channels of a DRM stream. "
           "These LEDs are the same as the top LEDs on the Evaluation Dialog.");


    /* Station Label and Info Display */
    const QString strStationLabelOther =
        tr("<b>Station Label and Info Display:</b> In the "
           "big label with the black background the station label and some other "
           "information about the current selected service is displayed. The "
           "magenta text on the top shows the bit-rate of the current selected "
           "service (The abbreviations EEP and "
           "UEP stand for Equal Error Protection and Unequal Error Protection. "
           "UEP is a feature of DRM for a graceful degradation of the decoded "
           "audio signal in case of a bad reception situation. UEP means that "
           "some parts of the audio is higher protected and some parts are lower "
           "protected (the ratio of higher protected part length to total length "
           "is shown in the brackets)), the audio compression format "
           "(e.g. AAC), if SBR is used and what audio mode is used (Mono, Stereo, "
           "P-Stereo -> low-complexity or parametric stereo). In case SBR is "
           "used, the actual sample rate is twice the sample rate of the core AAC "
           "decoder. The next two types of information are the language and the "
           "program type of the service (e.g. German / News).<br>The big "
           "turquoise text in the middle is the station label. This label may "
           "appear later than the magenta text since this information is "
           "transmitted in a different logical channel of a DRM stream. On the "
           "right, the ID number connected with this service is shown.");

    ui->TextTextMessage->setWhatsThis(strTextMessage);
    ui->TextLabelInputLevel->setWhatsThis(strInputLevel);
    inputLevel->widget()->setWhatsThis(strInputLevel);
    ui->CLED_MSC->setWhatsThis(strStatusLEDS);
    ui->CLED_SDC->setWhatsThis(strStatusLEDS);
    ui->CLED_FAC->setWhatsThis(strStatusLEDS);
    ui->LabelBitrate->setWhatsThis(strStationLabelOther);
    ui->LabelCodec->setWhatsThis(strStationLabelOther);
    ui->LabelStereoMono->setWhatsThis(strStationLabelOther);
    ui->LabelServiceLabel->setWhatsThis(strStationLabelOther);
    ui->LabelProgrType->setWhatsThis(strStationLabelOther);
    ui->LabelServiceID->setWhatsThis(strStationLabelOther);
    ui->LabelLanguage->setWhatsThis(strStationLabelOther);
    ui->LabelCountryCode->setWhatsThis(strStationLabelOther);
    ui->FrameAudioDataParams->setWhatsThis(strStationLabelOther);

    const QString strBars = tr("from 1 to 5 bars indicates WMER in the range 8 to 24 dB");
    ui->onebar->setWhatsThis(strBars);
    ui->twobars->setWhatsThis(strBars);
    ui->threebars->setWhatsThis(strBars);
    ui->fourbars->setWhatsThis(strBars);
    ui->fivebars->setWhatsThis(strBars);
}
