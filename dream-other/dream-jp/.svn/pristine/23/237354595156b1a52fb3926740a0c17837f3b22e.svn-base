/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001-2014
 *
 * Author(s):
 *  Volker Fischer, Andrew Murphy, Julian Cable
 *
 * Description:
 *
 *
 ******************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
\******************************************************************************/
#include "amwidget.h"
#include "ui_amwidget.h"
#include "receivercontroller.h"
#include "DRMPlot.h"
#include <QInputDialog>

AMWidget::AMWidget(ReceiverController* rc, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AMWidget),
    controller(rc)
{
    ui->setupUi(this);

    MainPlot = new CDRMPlot();
    ui->plotLayout->addWidget(MainPlot->widget());

    //connect(static_cast<CDRMPlotQwt*>(MainPlot), SIGNAL(xAxisValSet(double)), this, SLOT(OnChartxAxisValSet(double)));
    // TODO signal from plot

    /* Set help text for the controls */
    AddWhatsThisHelp();

    ui->sliderBandwidth->setTickPosition(QSlider::TicksBothSides);

    /* Init main plot */
    bool waterfall = false; // TODO getSetting("waterfall", false);
    ui->checkBoxWaterFall->setChecked(waterfall);
    if(MainPlot)
    {
        //TODOMainPlot->SetPlotStyle(getSetting("plotstyle", 0, true));
        MainPlot->SetupChart(waterfall?INP_SPEC_WATERF:INPUT_SIG_PSD_ANALOG, controller->getReceiver()->GetParameters()->GetSigSampleRate());
    }

    /* Init bandwidth slider */
    ui->sliderBandwidth->setRange(0, controller->getReceiver()->GetParameters()->GetSigSampleRate() / 2); // TODO

    // TODO from settings

    ui->buttonGroupDemodulation->setId(ui->radioButtonDemAM, CAMDemodulation::DT_AM);
    ui->buttonGroupDemodulation->setId(ui->radioButtonDemLSB, CAMDemodulation::DT_LSB);
    ui->buttonGroupDemodulation->setId(ui->radioButtonDemUSB, CAMDemodulation::DT_USB);
    ui->buttonGroupDemodulation->setId(ui->radioButtonDemCW, CAMDemodulation::DT_CW);
    ui->buttonGroupDemodulation->setId(ui->radioButtonDemFM, CAMDemodulation::DT_FM);
    ui->buttonGroupAGC->setId(ui->radioButtonAGCSlow, CAGC::AT_SLOW);
    ui->buttonGroupAGC->setId(ui->radioButtonAGCMed, CAGC::AT_MEDIUM);
    ui->buttonGroupAGC->setId(ui->radioButtonAGCFast, CAGC::AT_FAST);
    ui->buttonGroupNoiseReduction->setId(ui->radioButtonNoiRedOff, CAMDemodulation::NR_OFF);
    ui->buttonGroupNoiseReduction->setId(ui->radioButtonNoiRedLow, CAMDemodulation::NR_LOW);
    ui->buttonGroupNoiseReduction->setId(ui->radioButtonNoiRedMed, CAMDemodulation::NR_MEDIUM);
    ui->buttonGroupNoiseReduction->setId(ui->radioButtonNoiRedHigh, CAMDemodulation::NR_HIGH);
    ui->buttonGroupNoiseReduction->setId(ui->radioButtonNoiRedSpeex, CAMDemodulation::NR_SPEEX);

#ifdef HAVE_SPEEX
    ui->spinBoxNoiRedLevel->setValue(controller->getReceiver()->GetAMDemod()->GetNoiRedLevel());
#else
    ui->radioButtonNoiRedSpeex->hide();
    ui->spinBoxNoiRedLevel->hide();
#endif

    /* Button groups */
    connect(ui->buttonGroupDemodulation, SIGNAL(buttonClicked(int)),
            this, SLOT(OnRadioDemodulation(int)));
    connect(ui->buttonGroupAGC, SIGNAL(buttonClicked(int)),
            this, SLOT(OnRadioAGC(int)));
    connect(ui->buttonGroupNoiseReduction, SIGNAL(buttonClicked(int)),
            this, SLOT(OnRadioNoiRed(int)));

    connectController(controller);
}

AMWidget::~AMWidget()
{
    delete ui;
}

void AMWidget::on_new_data()
{
    if(MainPlot)
        MainPlot->update(controller);
}

void AMWidget::connectController(ReceiverController* controller)
{
    connect(this, SIGNAL(AGC(int)), controller, SLOT(setAnalogAGC(int)));
    connect(this, SIGNAL(modulation(int)), controller, SLOT(setAnalogModulation(int)));
    connect(this, SIGNAL(noiseReductionType(int)), controller, SLOT(setAnalogNoiseReduction(int)));
    connect(this, SIGNAL(noiseReductionLevel(int)), controller, SLOT(setNoiRedLevel(int)));
    connect(this, SIGNAL(AMFilterBW(int)), controller, SLOT(setAMFilterBW(int)));
    connect(this, SIGNAL(enableAutoFreqAcq(bool)), controller, SLOT(setEnableAutoFreqAcq(bool)));
    connect(this, SIGNAL(enablePLL(bool)), controller, SLOT(setEnablePLL(bool)));
    connect(this, SIGNAL(AMDemodAcq(double)), controller, SLOT(setAMDemodAcq(double)));

    connect(controller, SIGNAL(amFilterBandwidthChanged(int)), this, SLOT(on_amFilterBandwidthChanged(int)));
    connect(controller, SIGNAL(dataAvailable()), this, SLOT(on_new_data()));
}

void AMWidget::showEvent(QShowEvent*)
{
}

void AMWidget::hideEvent(QHideEvent*)
{
    bool waterfall = ui->checkBoxWaterFall->isChecked();
    (void)waterfall;
    // TODO putSetting("waterfall", waterfall);
}

void AMWidget::UpdatePlotStyle(int iPlotstyle)
{
    /* Update main plot window */
    if(MainPlot)
        MainPlot->SetPlotStyle(iPlotstyle);
}

void AMWidget::OnSampleRateChanged()
{
    // TODO
}

//controller->getReceiver()->GetAMDemod()->SetDemodType();
void AMWidget::OnRadioDemodulation(int iID)
{
    emit modulation(iID);
}

//controller->getReceiver()->GetAMDemod()->SetAGCType(CAGC::AT_NO_AGC);
void AMWidget::OnRadioAGC(int iID)
{
    emit AGC(iID);
}

//controller->getReceiver()->GetAMDemod()->SetNoiRedType(CAMDemodulation::NR_OFF);
void AMWidget::OnRadioNoiRed(int iID)
{
    emit noiseReductionType(iID);
#ifdef HAVE_SPEEX
    /* Set speex spinbox enable state */
    ui->spinBoxNoiRedLevel->setEnabled(ui->radioButtonNoiRedSpeex->isChecked());
#endif
}

//controller->getReceiver()->SetAMFilterBW(value);
void AMWidget::on_sliderBandwidth_valueChanged(int value)
{
    /* Set new filter in processing module */
    emit AMFilterBW(value);
    on_amFilterBandwidthChanged(value); // because widget shows in DRM mode in single window mode
}

void AMWidget::on_amFilterBandwidthChanged(int value)
{
    ui->textLabelBandWidth->setText(tr("%1 Hz").arg(value));
}

void AMWidget::on_checkBoxAutoFreqAcq_stateChanged(int i)
{
    emit enableAutoFreqAcq(i==Qt::Checked);
}

void AMWidget::on_checkBoxPLL_stateChanged(int i)
{
    emit enablePLL(i==Qt::Checked);
}

void AMWidget::OnChartxAxisValSet(double dVal)
{
    /* Perform range check */
    if (dVal < 0.0)
        dVal = 0.0;
    else if (dVal > 1.0)
        dVal = 1.0;

    /* Set new frequency in receiver module */
    emit AMDemodAcq(dVal);
}

void AMWidget::on_checkBoxWaterFall_stateChanged(int)
{
    /* Toggle between normal spectrum plot and waterfall spectrum plot */
    if (MainPlot && ui->checkBoxWaterFall->isChecked())
        MainPlot->SetupChart(INP_SPEC_WATERF, controller->getReceiver()->GetParameters()->GetSigSampleRate());
    else
        MainPlot->SetupChart(INPUT_SIG_PSD_ANALOG, controller->getReceiver()->GetParameters()->GetSigSampleRate());
}

/* Manual carrier frequency input box */
void AMWidget::on_buttonFreqOffset_clicked()
{
    bool ok = false;
    const double prev_freq =
        controller->getReceiver()->GetReceiveData()->ConvertFrequency(
            controller->getReceiver()->GetAMDemod()->GetCurMixFreqOffs());
    const double new_freq = QInputDialog::getDouble(this, this->windowTitle(),
                            ui->labelFreqOffset->text(), prev_freq, -1e6, 1e6, 2, &ok);
    if (ok)
    {
        const _REAL conv_freq =
            controller->getReceiver()->GetReceiveData()->ConvertFrequency(new_freq, true);
        const double dVal = conv_freq /
                            (controller->getReceiver()->GetParameters()->GetSigSampleRate() / 2);
        OnChartxAxisValSet(dVal);
    }
}

void AMWidget::on_spinBoxNoiRedLevel_valueChanged(int value)
{
    emit noiseReductionLevel(value);
}

void AMWidget::AddWhatsThisHelp()
{
    /* Noise Reduction */
    const QString strNoiseReduction =
        tr("<b>Noise Reduction:</b> The noise suppression is a frequency "
           "domain optimal filter design based algorithm. The noise PSD is "
           "estimated utilizing a minimum statistic. A problem of this type of "
           "algorithm is that it produces the so called \"musical tones\". The "
           "noise becomes coloured and sounds a bit strange. At the same time, "
           "the useful signal (which might be speech or music) is also "
           "distorted by the algorithm. By selecting the level of noise "
           "reduction, a compromise between distortion of the useful signal "
           "and actual noise reduction can be made.");

    /* Automatic Gain Control */
    const QString strAGC =
        tr("<b>AGC (Automatic Gain Control):</b> Input signals can have a "
           "large variation in power due to channel impairments. To compensate "
           "for that, an automatic gain control can be applied. The AGC has "
           "four settings: Off, Slow, Medium and Fast.");

    /* Filter Bandwidth */
    const QString strFilterBW =
        tr("<b>Filter Bandwidth:</b> A band-pass filter is applied before "
           "the actual demodulation process. With this filter, adjacent signals "
           "are attenuated. The bandwidth of this filter can be chosen in steps "
           "of 1 Hz by using the slider bar. Clicking on the right or left side "
           "of the slider leveler will increase/decrease the bandwidth by 1 kHz. "
           "<br>The current filter bandwidth is indicated in the spectrum plot "
           "by a selection bar.");

    /* Demodulation type */
    const QString strDemodType =
        tr("<b>Demodulation Type:</b> The following analog "
           "demodulation types are available:<ul>"
           "<li><b>AM:</b> This analog demodulation type is used in most of "
           "the hardware radios. The envelope of the complex base-band signal "
           "is used followed by a high-pass filter to remove the DC offset. "
           "Additionally, a low pass filter with the same bandwidth as the "
           "pass-band filter is applied to reduce the noise caused by "
           "non-linear distortions.</li>"
           "<li><b>LSB / USB:</b> These are single-side-band (SSB) demodulation "
           "types. Only one side of the spectrum is evaluated, the upper side "
           "band is used in USB and the lower side band with LSB. It is "
           "important for SSB demodulation that the DC frequency of the analog "
           "signal is known to get satisfactory results. The DC frequency is "
           "automatically estimated by starting a new acquisition or by "
           "clicking on the plot.</li>"
           "<li><b>CW:</b> This demodulation type can be used to receive "
           "CW signals. Only a narrow frequency band in a fixed distance "
           "to the mixing frequency is used. By clicking on the spectrum "
           "plot, the center position of the band pass filter can be set.</li>"
           "<li><b>FM:</b> This is a narrow band frequency demodulation.</li>"
           "</ul>");

    /* Carrier Frequency */
    QString strTextFreqOffset =
        tr("<b>Carrier Frequency:</b> The (estimated) carrier frequency of the "
           "analog signal is shown. (The estimation of this parameter can be done "
           "by the Autom Frequency Acquisition which uses the estimated PSD of "
           "the input signal and applies a maximum search.)");

    /* Phase lock loop */
    const QString strPLL =
        tr("<b>PLL:</b> The Phase-Lock-Loop (PLL) tracks the carrier of the "
           "modulated received signal. The resulting phase offset between the "
           "reference oscillator and the received carrier is displayed in "
           "a dial control. If the pointer is almost steady, the PLL is locked. "
           "If the pointer of the dial control turns quickly, the PLL is "
           "out of lock. To get the PLL locked, the frequency offset to "
           "the true carrier frequency must not exceed a few Hz.");

    /* Auto frequency acquisition */
    const QString strAutoFreqAcqu =
        tr("<b>Auto Frequency Acquisition:</b> Clicking on the "
           "input spectrum plot changes the mixing frequency for demodulation. "
           "If the Auto Frequency Acquisition is enabled, the largest peak "
           "near the curser is selected.");

    ui->radioButtonNoiRedOff->setWhatsThis(strNoiseReduction);
    ui->radioButtonNoiRedLow->setWhatsThis(strNoiseReduction);
    ui->radioButtonNoiRedMed->setWhatsThis(strNoiseReduction);
    ui->radioButtonNoiRedHigh->setWhatsThis(strNoiseReduction);
    ui->radioButtonAGCOff->setWhatsThis(strAGC);
    ui->radioButtonAGCSlow->setWhatsThis(strAGC);
    ui->radioButtonAGCMed->setWhatsThis(strAGC);
    ui->radioButtonAGCFast->setWhatsThis(strAGC);
    ui->textLabelBandWidth->setWhatsThis(strFilterBW);
    ui->sliderBandwidth->setWhatsThis(strFilterBW);
    ui->radioButtonDemAM->setWhatsThis(strDemodType);
    ui->radioButtonDemLSB->setWhatsThis(strDemodType);
    ui->radioButtonDemUSB->setWhatsThis(strDemodType);
    ui->radioButtonDemCW->setWhatsThis(strDemodType);
    ui->radioButtonDemFM->setWhatsThis(strDemodType);
    ui->GroupBoxAutoFreqAcq->setWhatsThis(strAutoFreqAcqu);
    ui->checkBoxAutoFreqAcq->setWhatsThis(strAutoFreqAcqu);
    ui->GroupBoxPLL->setWhatsThis(strPLL);
    ui->checkBoxPLL->setWhatsThis(strPLL);
    ui->labelFreqOffset->setWhatsThis(strTextFreqOffset);
    ui->buttonFreqOffset->setWhatsThis(strTextFreqOffset);
    ui->groupBoxNoiseReduction->setWhatsThis(strNoiseReduction);
    ui->groupBoxAGC->setWhatsThis(strAGC);
    ui->groupBoxDemodulation->setWhatsThis(strDemodType);
    ui->groupBoxBW->setWhatsThis(strFilterBW);
}
