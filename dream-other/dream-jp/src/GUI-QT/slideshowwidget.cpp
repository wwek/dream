/******************************************************************************\
 * British Broadcasting Corporation
 * Copyright (c) 2001-2014
 *
 * Author(s):
 *   Julian Cable, David Flamand
 *
 * Description: SlideShow Viewer
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

#include "slideshowwidget.h"
#include "ui_slideshowwidget.h"
#include "../util-QT/Util.h"
#include <../datadecoding/DataDecoder.h>
#include "../datadecoding/DABMOT.h"
#include <QFileDialog>

SlideShowWidget::SlideShowWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SlideShowWidget),
    pics(), iCurImagePos(-1),
    bClearMOTCache(false),
    motdec(NULL)
{
    ui->setupUi(this);
    ui->LEDStatus->SetUpdateTime(1000);
}

SlideShowWidget::~SlideShowWidget()
{
    delete ui;
}

void SlideShowWidget::setSavePath(const QString& s)
{
    strCurrentSavePath = s;
}

void SlideShowWidget::setServiceInformation(int s, CService service)
{
    short_id = s;
    if(service.DataParam.pDecoder)
        motdec = service.DataParam.pDecoder->getApplication(service.DataParam.iPacketID);
    QString strLabel = QString().fromUtf8(service.strLabel.c_str()).trimmed();
    QString strTitle("MOT Slide Show");
    QString strServiceID;

    if (service.iServiceID != 0)
    {
        if (strLabel != "")
            strLabel += " - ";

        /* Service ID (plot number in hexadecimal format) */
        strServiceID = "ID:" +
                       QString().setNum(service.iServiceID, 16).toUpper();
    }

    /* Add the description on the title of the dialog */
    if (strLabel != "" || strServiceID != "")
        strTitle += " [" + strLabel + strServiceID + "]";
    setWindowTitle(strTitle);
}

void SlideShowWidget::setStatus(int i, ETypeRxStatus eStatus)
{
    if(short_id == i)
        SetStatus(ui->LEDStatus, eStatus);

    if (motdec == NULL)
    {
        return;
    }
    if (bClearMOTCache)
    {
        bClearMOTCache = false;
        /* Remove all object from cache */
        while (motdec->NewObjectAvailable())
        {
            CMOTObject  NewObj;
            motdec->GetNextObject(NewObj);
        }
    }
    else
    {
        /* Poll the data decoder module for one new picture */
        if(motdec->NewObjectAvailable())
        {
            CMOTObject  NewObj;
            motdec->GetNextObject(NewObj);

            /* Store received picture */
            int iCurNumPict = pics.size();
            CVector<_BYTE>& imagedata = NewObj.Body.vecData;

            /* Load picture in QT format */
            QPixmap pic;
            if (pic.loadFromData(&imagedata[0], unsigned(imagedata.Size())))
            {
                /* Set new picture in source factory */
                Img p;
                p.pic = pic;
                p.name = QString::fromUtf8(NewObj.strName.c_str());
                pics.push_back(p);
            }

            /* If the last received picture was selected, automatically show
               new picture */
            if (iCurImagePos == iCurNumPict - 1)
                SetImage(iCurNumPict);
            else
                UpdateButtons();
        }
    }
}

void SlideShowWidget::on_ButtonStepBack_clicked()
{
    SetImage(iCurImagePos-1);
}

void SlideShowWidget::on_ButtonStepForward_clicked()
{
    SetImage(iCurImagePos+1);
}

void SlideShowWidget::on_ButtonJumpBegin_clicked()
{
    SetImage(0);
}

void SlideShowWidget::on_ButtonJumpEnd_clicked()
{
    SetImage(pics.size()-1);
}

void SlideShowWidget::OnSave()
{
    /* Create directory for storing the file (if not exists) */
    CreateDirectories(strCurrentSavePath);

    QString strFilename = strCurrentSavePath + VerifyFilename(pics[iCurImagePos].name);
    strFilename = QFileDialog::getSaveFileName(this,
                  tr("Save File"), strFilename, tr("Images (*.png *.jpg)"));

    /* Check if user not hit the cancel button */
    if (!strFilename.isEmpty())
    {
        pics[iCurImagePos].pic.save(strFilename);

        strCurrentSavePath = QFileInfo(strFilename).path() + PATH_SEPARATOR;
    }
}

void SlideShowWidget::OnSaveAll()
{
    /* Create directory for storing the files (if not exists) */
    CreateDirectories(strCurrentSavePath);

    QString strDirectory = QFileDialog::getExistingDirectory(this,
                           tr("Open Directory"), strCurrentSavePath);

    /* Check if user not hit the cancel button */
    if (!strDirectory.isEmpty())
    {
        strCurrentSavePath = strDirectory + PATH_SEPARATOR;

        for(size_t i=0; i<pics.size(); i++)
            pics[i].pic.save(strCurrentSavePath + VerifyFilename(pics[i].name));
    }
}

void SlideShowWidget::OnClearAll()
{
    pics.clear();
    iCurImagePos = -1;
    UpdateButtons();
    ui->LabelTitle->setText("");
    bClearMOTCache = true;
}

void SlideShowWidget::SetImage(int pos)
{
    if(pics.size()==0)
        return;
    if(pos<0)
        pos = 0;
    if(pos>int(pics.size()-1))
        pos = pics.size()-1;
    iCurImagePos = pos;
    ui->Image->setPixmap(pics[pos].pic);
    QString imagename = pics[pos].name;
    ui->Image->setToolTip(imagename);
    imagename =  "<b>" + imagename + "</b>";
    Linkify(imagename);
    ui->LabelTitle->setText(imagename);
    UpdateButtons();
}

void SlideShowWidget::UpdateButtons()
{

    if (iCurImagePos <= 0)
    {
        /* We are already at the beginning */
        ui->ButtonStepBack->setEnabled(false);
        ui->ButtonJumpBegin->setEnabled(false);
    }
    else
    {
        ui->ButtonStepBack->setEnabled(true);
        ui->ButtonJumpBegin->setEnabled(true);
    }

    if (iCurImagePos == int(pics.size()-1))
    {
        /* We are already at the end */
        ui->ButtonStepForward->setEnabled(false);
        ui->ButtonJumpEnd->setEnabled(false);
    }
    else
    {
        ui->ButtonStepForward->setEnabled(true);
        ui->ButtonJumpEnd->setEnabled(true);
    }

    QString strTotImages = QString().setNum(pics.size());
    QString strNumImage = QString().setNum(iCurImagePos + 1);

    QString strSep("");

    for (int i = 0; i < (strTotImages.length() - strNumImage.length()); i++)
        strSep += " ";

    ui->LabelCurPicNum->setText(strSep + strNumImage + PATH_SEPARATOR + strTotImages);

    /* If no picture was received, show the following text */
    if (iCurImagePos < 0)
    {
        /* Init text browser window */
        ui->Image->setText("<center>" + tr("MOT Slideshow Viewer") + "</center>");
        ui->Image->setToolTip("");
    }
}
