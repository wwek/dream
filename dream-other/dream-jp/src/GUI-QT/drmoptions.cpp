#include "drmoptions.h"
#include "ui_drmoptions.h"
#include <QButtonGroup>

DRMOptions::DRMOptions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DRMOptions),rsci_mode(false)
{
    ui->setupUi(this);
    /* Init slider control */
    ui->SliderNoOfIterations->setRange(0, 4);
    ui->SliderNoOfIterations->setValue(0);
    ui->TextNumOfIterations->setText(tr("MLC: Number of Iterations: ") + QString().setNum(0));

    QButtonGroup* ti = new QButtonGroup(this);
    ti->addButton(ui->RadioButtonTiLinear, CChannelEstimation::TLINEAR);
    ti->addButton(ui->RadioButtonTiWiener, CChannelEstimation::TWIENER);
    connect(ti, SIGNAL(buttonClicked(int)), this, SIGNAL(timeIntChanged(int)));
    QButtonGroup* fr = new QButtonGroup(this);
    fr->addButton(ui->RadioButtonFreqLinear, CChannelEstimation::FLINEAR);
    fr->addButton(ui->RadioButtonFreqDFT, CChannelEstimation::FDFTFILTER);
    fr->addButton(ui->RadioButtonFreqWiener, CChannelEstimation::FWIENER);
    connect(fr, SIGNAL(buttonClicked(int)), this, SIGNAL(freqIntChanged(int)));
    QButtonGroup* ts = new QButtonGroup(this);
    ts->addButton(ui->RadioButtonTiSyncEnergy, CTimeSyncTrack::TSENERGY);
    ts->addButton(ui->RadioButtonTiSyncFirstPeak, CTimeSyncTrack::TSFIRSTPEAK);
    connect(ts, SIGNAL(buttonClicked(int)), this, SIGNAL(timeSyncChanged(int)));
    /* Check boxes */
    connect(ui->CheckBoxFlipSpec, SIGNAL(clicked(bool)), this, SIGNAL(flipSpectrum(bool)));
    connect(ui->CheckBoxRecFilter, SIGNAL(clicked(bool)), this, SIGNAL(recFilter(bool)));
    connect(ui->CheckBoxModiMetric, SIGNAL(clicked(bool)), this, SIGNAL(modiMetric(bool)));
}

DRMOptions::~DRMOptions()
{
    delete ui;
}

void
DRMOptions::setRSCIModeEnabled(bool enabled)
{
    rsci_mode = enabled;
    ui->SliderNoOfIterations->setEnabled(!enabled);

    ui->ButtonGroupChanEstFreqInt->setEnabled(!enabled);
    ui->ButtonGroupChanEstTimeInt->setEnabled(!enabled);
    ui->ButtonGroupTimeSyncTrack->setEnabled(!enabled);
    ui->CheckBoxFlipSpec->setEnabled(!enabled);
    ui->GroupBoxInterfRej->setEnabled(!enabled);
}

/* Slider for MLC number of iterations */
void
DRMOptions::setNumIterations(int iNumIt)
{
    if (ui->SliderNoOfIterations->value() != iNumIt)
    {
        /* Update slider and label */
        ui->SliderNoOfIterations->setValue(iNumIt);
        ui->TextNumOfIterations->setText(tr("MLC: Number of Iterations: ") +
                                         QString().setNum(iNumIt));
    }
}

void DRMOptions::on_SliderNoOfIterations_valueChanged(int value)
{
    /* Show the new value in the label control */
    ui->TextNumOfIterations->setText(tr("MLC: Number of Iterations: ") +
                                     QString().setNum(value));
}

void DRMOptions::setTimeInt(CChannelEstimation::ETypeIntTime state)
{
    switch (state)
    {
    case CChannelEstimation::TLINEAR:
        if (!ui->RadioButtonTiLinear->isChecked())
            ui->RadioButtonTiLinear->setChecked(true);
        break;

    case CChannelEstimation::TWIENER:
        if (!ui->RadioButtonTiWiener->isChecked())
            ui->RadioButtonTiWiener->setChecked(true);
        break;
    }
}

void DRMOptions::setFreqInt(CChannelEstimation::ETypeIntFreq state)
{
    switch (state)
    {
    case CChannelEstimation::FLINEAR:
        if (!ui->RadioButtonFreqLinear->isChecked())
            ui->RadioButtonFreqLinear->setChecked(true);
        break;

    case CChannelEstimation::FDFTFILTER:
        if (!ui->RadioButtonFreqDFT->isChecked())
            ui->RadioButtonFreqDFT->setChecked(true);
        break;

    case CChannelEstimation::FWIENER:
        if (!ui->RadioButtonFreqWiener->isChecked())
            ui->RadioButtonFreqWiener->setChecked(true);
        break;
    }

}

void DRMOptions::setTiSyncTrac(CTimeSyncTrack::ETypeTiSyncTrac state)
{
    switch (state)
    {
    case CTimeSyncTrack::TSFIRSTPEAK:
        if (!ui->RadioButtonTiSyncFirstPeak->isChecked())
            ui->RadioButtonTiSyncFirstPeak->setChecked(true);
        break;

    case CTimeSyncTrack::TSENERGY:
        if (!ui->RadioButtonTiSyncEnergy->isChecked())
            ui->RadioButtonTiSyncEnergy->setChecked(true);
        break;
    }

}

void DRMOptions::setRecFilterEnabled(bool b)
{
    ui->CheckBoxRecFilter->setChecked(b);
}

void DRMOptions::setIntConsEnabled(bool b)
{
    ui->CheckBoxModiMetric->setChecked(b);
}

void DRMOptions::setFlipSpectrumEnabled(bool b)
{
    ui->CheckBoxFlipSpec->setChecked(b);
}

void DRMOptions::AddWhatsThisHelp()
{
    /* Flip Input Spectrum */
    ui->CheckBoxFlipSpec->setWhatsThis(
        tr("<b>Flip Input Spectrum:</b> Checking this box "
           "will flip or invert the input spectrum. This is necessary if the "
           "mixer in the front-end uses the lower side band."));

    /* Wiener */
    const QString strWienerChanEst =
        tr("<b>Channel Estimation Settings:</b> With these "
           "settings, the channel estimation method in time and frequency "
           "direction can be selected. The default values use the most powerful "
           "algorithms. For more detailed information about the estimation "
           "algorithms there are a lot of papers and books available.<br>"
           "<b>Wiener:</b> Wiener interpolation method "
           "uses estimation of the statistics of the channel to design an optimal "
           "filter for noise reduction.");

    ui->RadioButtonFreqWiener->setWhatsThis(strWienerChanEst);
    ui->RadioButtonTiWiener->setWhatsThis(strWienerChanEst);

    /* Linear */
    const QString strLinearChanEst =
        tr("<b>Channel Estimation Settings:</b> With these "
           "settings, the channel estimation method in time and frequency "
           "direction can be selected. The default values use the most powerful "
           "algorithms. For more detailed information about the estimation "
           "algorithms there are a lot of papers and books available.<br>"
           "<b>Linear:</b> Simple linear interpolation "
           "method to get the channel estimate. The real and imaginary parts "
           "of the estimated channel at the pilot positions are linearly "
           "interpolated. This algorithm causes the lowest CPU load but "
           "performs much worse than the Wiener interpolation at low SNRs.");

    ui->RadioButtonFreqLinear->setWhatsThis(strLinearChanEst);
    ui->RadioButtonTiLinear->setWhatsThis(strLinearChanEst);

    /* DFT Zero Pad */
    ui->RadioButtonFreqDFT->setWhatsThis(
        tr("<b>Channel Estimation Settings:</b> With these "
           "settings, the channel estimation method in time and frequency "
           "direction can be selected. The default values use the most powerful "
           "algorithms. For more detailed information about the estimation "
           "algorithms there are a lot of papers and books available.<br>"
           "<b>DFT Zero Pad:</b> Channel estimation method "
           "for the frequency direction using Discrete Fourier Transformation "
           "(DFT) to transform the channel estimation at the pilot positions to "
           "the time domain. There, a zero padding is applied to get a higher "
           "resolution in the frequency domain -> estimates at the data cells. "
           "This algorithm is very speed efficient but has problems at the edges "
           "of the OFDM spectrum due to the leakage effect."));

    /* Guard Energy */
    ui->RadioButtonTiSyncEnergy->setWhatsThis(
        tr("<b>Guard Energy:</b> Time synchronization "
           "tracking algorithm utilizes the estimation of the impulse response. "
           "This method tries to maximize the energy in the guard-interval to set "
           "the correct timing."));

    /* First Peak */
    ui->RadioButtonTiSyncFirstPeak->setWhatsThis(
        tr("<b>First Peak:</b> This algorithms searches for "
           "the first peak in the estimated impulse response and moves this peak "
           "to the beginning of the guard-interval (timing tracking algorithm)."));

    /* Interferer Rejection */
    const QString strInterfRej =
        tr("<b>Interferer Rejection:</b> There are two "
           "algorithms available to reject interferers:<ul>"
           "<li><b>Bandpass Filter (BP-Filter):</b>"
           " The bandpass filter is designed to have the same bandwidth as "
           "the DRM signal. If, e.g., a strong signal is close to the border "
           "of the actual DRM signal, under some conditions this signal will "
           "produce interference in the useful bandwidth of the DRM signal "
           "although it is not on the same frequency as the DRM signal. "
           "The reason for that behaviour lies in the way the OFDM "
           "demodulation is done. Since OFDM demodulation is a block-wise "
           "operation, a windowing has to be applied (which is rectangular "
           "in case of OFDM). As a result, the spectrum of a signal is "
           "convoluted with a Sinc function in the frequency domain. If a "
           "sinusoidal signal close to the border of the DRM signal is "
           "considered, its spectrum will not be a distinct peak but a "
           "shifted Sinc function. So its spectrum is broadened caused by "
           "the windowing. Thus, it will spread in the DRM spectrum and "
           "act as an in-band interferer.<br>"
           "There is a special case if the sinusoidal signal is in a "
           "distance of a multiple of the carrier spacing of the DRM signal. "
           "Since the Sinc function has zeros at certain positions it happens "
           "that in this case the zeros are exactly at the sub-carrier "
           "frequencies of the DRM signal. In this case, no interference takes "
           "place. If the sinusoidal signal is in a distance of a multiple of "
           "the carrier spacing plus half of the carrier spacing away from the "
           "DRM signal, the interference reaches its maximum.<br>"
           "As a result, if only one DRM signal is present in the 20 kHz "
           "bandwidth, bandpass filtering has no effect. Also,  if the "
           "interferer is far away from the DRM signal, filtering will not "
           "give much improvement since the squared magnitude of the spectrum "
           "of the Sinc function is approx -15 dB down at 1 1/2 carrier "
           "spacing (approx 70 Hz with DRM mode B) and goes down to approx "
           "-30 dB at 10 times the carrier spacing plus 1 / 2 of the carrier "
           "spacing (approx 525 Hz with DRM mode B). The bandpass filter must "
           "have very sharp edges otherwise the gain in performance will be "
           "very small.</li>"
           "<li><b>Modified Metrics:</b> Based on the "
           "information from the SNR versus sub-carrier estimation, the metrics "
           "for the Viterbi decoder can be modified so that sub-carriers with "
           "high noise are attenuated and do not contribute too much to the "
           "decoding result. That can improve reception under bad conditions but "
           "may worsen the reception in situations where a lot of fading happens "
           "and no interferer are present since the SNR estimation may be "
           "not correct.</li></ul>");

    ui->GroupBoxInterfRej->setWhatsThis(strInterfRej);
    ui->CheckBoxRecFilter->setWhatsThis(strInterfRej);
    ui->CheckBoxModiMetric->setWhatsThis(strInterfRej);
}
