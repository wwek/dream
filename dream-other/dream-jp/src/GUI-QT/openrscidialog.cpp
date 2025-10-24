#include "openrscidialog.h"
#include "ui_openrscidialog.h"
#include <QFileDialog>

OpenRSCIDialog::OpenRSCIDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OpenRSCIDialog)
{
    ui->setupUi(this);
    connect(ui->pushButtonFile, SIGNAL(clicked()), this, SLOT(OnChooseFile()));
}

OpenRSCIDialog::~OpenRSCIDialog()
{
    delete ui;
}

void OpenRSCIDialog::accept()
{
    bool ok;
    int n = ui->lineEditPort->text().toInt(&ok);
    if(ok) {
        if(path == "") {
            file = ui->lineEditAddress->text().append(QString(":%1").arg(n));
            if(ui->radioButtonTCP->isChecked())
                file = QString("t").append(file);
        }
        else {
            file = path.append(QString("#%1").arg(n));
        }
    }
    else {
        file = path;
    }
    QDialog::accept();
}

void OpenRSCIDialog::OnChooseFile()
{
    QString rawtypes = "*.rsA *.rsB *.rsC *.rsD *.rsQ *.rsM";
    QString supportedtypes = rawtypes;
#ifdef HAVE_LIBPCAP
    supportedtypes.append(" *.pcap");
#endif
    QString filter = tr("Supported Files (%1);;").arg(supportedtypes);
#ifdef HAVE_LIBPCAP
    filter.append(tr(" Capture Files (*.pcap);;"));
#endif
    filter.append(tr(" Raw Files (%1);;").arg(rawtypes));
    filter.append(" All Files (*)");
    path = QFileDialog::getOpenFileName(this, tr("Open File"), file, filter);
}
