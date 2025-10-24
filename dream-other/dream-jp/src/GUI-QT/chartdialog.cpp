#include "chartdialog.h"
#include "ui_chartdialog.h"

ChartDialog::ChartDialog(QWidget *parent) :
    QDialog(parent), plot(new CDRMPlot(parent)),
    ui(new Ui::ChartDialog)
{
    ui->setupUi(this);
    ui->gridLayout->addWidget(plot->widget());
}

ChartDialog::~ChartDialog()
{
    delete ui;
}
