/******************************************************************************\
 * Copyright (c) 2001-2014
 *
 * Author(s):
 *  David Flamand
 *
 * Description:
 *  Intended to provide a way to customize appearance of widget
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

#include "ThemeCustomizer.h"

#ifdef USE_THEMECUSTOMIZER

#include <QPalette>
#include <QFont>
#include <QFrame>
#include <QDesktopWidget>
#include "ui_DRMMainWindow.h"
#include "ui_serviceselector.h"
#include "EvaluationDlg.h"
#include "AnalogDemDlg.h"
#include "DialogUtil.h"
#include "StationsDlg.h"
#include "EPGDlg.h"
#include "GeneralSettingsDlg.h"
#include "MultSettingsDlg.h"
#ifdef HAVE_QTWEBKIT
# include "BWSViewer.h"
#endif
#include "JLViewer.h"
#include "LiveScheduleDlg.h"
#include "SlideShowViewer.h"
#ifdef HAVE_LIBHAMLIB
# include "RigDlg.h"
#endif


#define WINDOW_BORDER_MARGIN 0

//
// TODO list:
//  light theme, only dark theme is currently inplemented.
//  theme selection menu.
//  scaling font with screen size.
//

static void SetFontSize(QWidget *widget, int fontSize)
{
    QFont font(widget->font());
    font.setPointSize(fontSize);
    widget->setFont(font);
}

static void ResizeWidget(QWidget *widget)
{
    QWidget* parent = widget->parentWidget();
    if (parent)
        widget->setGeometry(parent->geometry());
}

static QPalette BaseSetup(QWidget *widget)
{
    QPalette palette(widget->palette());
    palette.setColor(QPalette::Base,       QColor("#212421"));
    palette.setColor(QPalette::Text,       QColor("#FFFFFF"));
    palette.setColor(QPalette::Window,     QColor("#000000"));
    palette.setColor(QPalette::WindowText, QColor("#FFFFFF"));
    palette.setColor(QPalette::Button,     QColor("#212421"));
    palette.setColor(QPalette::ButtonText, QColor("#FFFFFF"));
    palette.setColor(QPalette::Light,      QColor("#616361"));
    palette.setColor(QPalette::Dark,       QColor("#616361"));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#616361"));
    widget->setPalette(palette);
    SetFontSize(widget, 13);
    return palette;
}

static void QFrameSetup(QFrame *widget)
{
    widget->setFrameShape(QFrame::Panel);
    widget->setFrameShadow(QFrame::Sunken);
}

void ApplyCustomTheme(QWidget *widget, void* pUi)
{
    QString name(widget->objectName());
    if (name == "DRMMainWindow")
    {
        Ui::DRMMainWindow* ui = (Ui::DRMMainWindow*)pUi;
        QPalette prevPalette(ui->TextTextMessage->palette());
        QPalette basePalette(BaseSetup(widget));
        ui->centralwidget->layout()->setMargin(WINDOW_BORDER_MARGIN);
        ui->FrameMainDisplay->layout()->setMargin(0);
        ui->FrameMainDisplay->setFrameShape(QFrame::NoFrame);
        ui->FrameMainDisplay->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        ui->TextTextMessage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
        QPalette palette(ui->TextTextMessage->palette());
        palette.setColor(QPalette::Active,   QPalette::Light, prevPalette.color(QPalette::Active,   QPalette::Light));
        palette.setColor(QPalette::Active,   QPalette::Dark,  prevPalette.color(QPalette::Active,   QPalette::Dark));
        palette.setColor(QPalette::Inactive, QPalette::Light, prevPalette.color(QPalette::Inactive, QPalette::Light));
        palette.setColor(QPalette::Inactive, QPalette::Dark,  prevPalette.color(QPalette::Inactive, QPalette::Dark));
        palette.setColor(QPalette::Disabled, QPalette::Light, basePalette.color(QPalette::Disabled, QPalette::Light));
        palette.setColor(QPalette::Disabled, QPalette::Dark,  basePalette.color(QPalette::Disabled, QPalette::Dark));
        ui->TextTextMessage->setPalette(palette);
        ui->onebar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        ui->onebar->setMinimumSize(0, 6);
        ui->onebar->setMaximumSize(16777215, 6);
        ui->twobars->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        ui->twobars->setMinimumSize(0, 6);
        ui->twobars->setMaximumSize(16777215, 12);
        ui->threebars->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        ui->threebars->setMinimumSize(0, 6);
        ui->threebars->setMaximumSize(16777215, 18);
        ui->fourbars->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        ui->fourbars->setMinimumSize(0, 6);
        ui->fourbars->setMaximumSize(16777215, 24);
        ui->fivebars->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        ui->fivebars->setMinimumSize(0, 6);
        ui->fivebars->setMaximumSize(16777215, 30);
        SetFontSize(ui->TextLabelInputLevel, 12);
        SetFontSize(ui->ProgrInputLevel, 15);
        SetFontSize(ui->LabelBitrate, 15);
        SetFontSize(ui->LabelCodec, 15);
        SetFontSize(ui->LabelStereoMono, 15);
        SetFontSize(ui->LabelLanguage, 15);
        SetFontSize(ui->LabelCountryCode, 15);
        SetFontSize(ui->LabelProgrType, 15);
        SetFontSize(ui->LabelServiceID, 15);
        SetFontSize(ui->labelAFS, 13);
        SetFontSize(ui->LabelServiceLabel, 22);
        SetFontSize(ui->lineEditFrequency, 24);
        SetFontSize(ui->TextTextMessage, 16);
    }
    else if (name == "ServiceSelector")
    {
        Ui::ServiceSelector* ui = (Ui::ServiceSelector*)pUi;
        QPalette palette(widget->palette());
        QFrameSetup(ui->TextMiniService1);
        QFrameSetup(ui->TextMiniService2);
        QFrameSetup(ui->TextMiniService3);
        QFrameSetup(ui->TextMiniService4);
        widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        SetFontSize(widget, 18);
    }
    else if (name == "SystemEvaluationWindow")
    {
        systemevalDlg* ui = (systemevalDlg*)widget;
        BaseSetup(widget);
        ui->centralwidget->layout()->setMargin(WINDOW_BORDER_MARGIN);
        QFrameSetup(ui->FrameParamStatusLEDs);
        QFrameSetup(ui->FrameParamStatusLEDs_2);
        QFrameSetup(ui->FrameFACParams);
        /*      palette.setColor(QPalette::Window, "#FFFFFF");
                palette.setColor(QPalette::Base, "#FFFFFF");
                palette.setColor(QPalette::Button, "#FFFFFF");
                palette.setColor(QPalette::AlternateBase, "#FFFFFF");
                palette.setColor(QPalette::Mid, "#FFFFFF");
                palette.setColor(QPalette::Shadow, "#FFFFFF");
                palette.setColor(QPalette::Midlight, "#FFFFFF");
                palette.setColor(QPalette::Highlight, "#FFFFFF");
                palette.setColor(QPalette::Light, "#FFFFFF");
                palette.setColor(QPalette::Dark, "#FFFFFF");
                palette.setColor(QPalette::NoRole, "#FFFFFF");
                ui->ButtonGroupChanEstFreqInt->setPalette(palette);
                ui->RadioButtonFreqWiener->setPalette(palette);
        */      QPalette palette(ui->chartSelector->palette());
        palette.setColor(QPalette::Base, palette.color(QPalette::Window));
        ui->chartSelector->setPalette(palette);
        QFrameSetup(ui->chartSelector);
        ui->buttonOk->setDefault(false);
        SetFontSize(ui->buttonOk, 13);
        SetFontSize(widget, 7);
    }
    else if (name == "AMMainWindow")
    {
        AnalogDemDlg* ui = (AnalogDemDlg*)widget;
        BaseSetup(widget);
        ui->centralwidget->layout()->setMargin(WINDOW_BORDER_MARGIN);
        QFrameSetup(ui->frame);
        QFrameSetup(ui->frame_2);
        ui->ButtonDRM->setDefault(false);
    }
    else if (name == "CAMSSDlgBase")
    {
        CAMSSDlg* ui = (CAMSSDlg*)widget;
        BaseSetup(widget);
        ui->centralwidget->layout()->setMargin(WINDOW_BORDER_MARGIN);
        QFrameSetup(ui->FrameMainDisplay);
        QColor color(ui->TextAMSSInfo->palette().color(QPalette::WindowText));
        QPalette palette(ui->FrameMainDisplay->palette());
        palette.setColor(QPalette::Light, color);
        palette.setColor(QPalette::Dark, color);
        ui->FrameMainDisplay->setPalette(palette);
        ui->FrameMainDisplay->setLineWidth(2);
        ui->buttonOk->setDefault(false);
        SetFontSize(ui->TextAMSSTimeDate, 12);
        SetFontSize(ui->TextAMSSServiceLabel, 22);
        SetFontSize(ui->TextAMSSLanguage, 15);
        SetFontSize(ui->TextAMSSCountryCode, 15);
        SetFontSize(ui->TextAMSSAMCarrierMode, 15);
        SetFontSize(ui->TextAMSSInfo, 15);
        SetFontSize(ui->TextAMSSServiceID, 15);
    }
    else if (name == "CAboutDlgBase")
    {
        CAboutDlg* ui = (CAboutDlg*)widget;
        BaseSetup(widget);
        widget->layout()->setMargin(WINDOW_BORDER_MARGIN);
        ResizeWidget(widget);
        ui->setSizeGripEnabled(false);
        ui->buttonOk->setDefault(false);
    }
    else if (name == "StationsDlgbase")
    {
        StationsDlg* ui = (StationsDlg*)widget;
        BaseSetup(widget);
        ui->centralwidget->layout()->setMargin(WINDOW_BORDER_MARGIN);
        QFrameSetup(ui->TextLabelUTCTime);
        ui->buttonOk->setDefault(false);
    }
    else if (name == "CEPGDlgbase")
    {
        EPGDlg* ui = (EPGDlg*)widget;
        BaseSetup(widget);
        ui->centralwidget->layout()->setMargin(WINDOW_BORDER_MARGIN);
        QFrameSetup(ui->frame);
        ui->buttonOk->setDefault(false);
    }
    else if (name == "CGeneralSettingsDlgBase")
    {
        GeneralSettingsDlg* ui = (GeneralSettingsDlg*)widget;
        BaseSetup(widget);
        widget->layout()->setMargin(WINDOW_BORDER_MARGIN);
        ResizeWidget(widget);
        ui->setSizeGripEnabled(false);
        ui->buttonOk->setDefault(false);
    }
    else if (name == "CMultSettingsDlgBase")
    {
        MultSettingsDlg* ui = (MultSettingsDlg*)widget;
        BaseSetup(widget);
        widget->layout()->setMargin(WINDOW_BORDER_MARGIN);
        ResizeWidget(widget);
        ui->setSizeGripEnabled(false);
        ui->buttonOk->setDefault(false);
    }
#ifdef HAVE_QTWEBKIT
    else if (name == "BWSViewer")
    {
        BWSViewer* ui = (BWSViewer*)widget;
        ui->centralwidget->layout()->setMargin(WINDOW_BORDER_MARGIN);
        QPalette palette(BaseSetup(widget));
        palette.setColor(QPalette::Base, QColor("#FFFFFF"));
        palette.setColor(QPalette::Text, QColor("#000000"));
        ui->webView->setPalette(palette);
        QFrameSetup(ui->frame);
    }
#endif
    else if (name == "JLViewer")
    {
        JLViewer* ui = (JLViewer*)widget;
        ui->centralwidget->layout()->setMargin(WINDOW_BORDER_MARGIN);
        QPalette palette(BaseSetup(widget));
        palette.setColor(QPalette::Base, QColor("#FFFFFF"));
        palette.setColor(QPalette::Text, QColor("#000000"));
        ui->textBrowser->setPalette(palette);
        QFrameSetup(ui->textBrowser);
    }
    else if (name == "LiveScheduleWindow")
    {
        LiveScheduleDlg* ui = (LiveScheduleDlg*)widget;
        ui->centralwidget->layout()->setMargin(WINDOW_BORDER_MARGIN);
        BaseSetup(widget);
        QFrameSetup(ui->TextLabelUTCTime);
        QFrameSetup(ui->labelFrequency);
    }
    else if (name == "SlideShowViewer")
    {
        SlideShowViewer* ui = (SlideShowViewer*)widget;
        ui->centralwidget->layout()->setMargin(WINDOW_BORDER_MARGIN);
        BaseSetup(widget);
        QFrameSetup(ui->frame);
    }
#ifdef HAVE_LIBHAMLIB
    else if (name == "RigDlg")
    {
        RigDlg* ui = (RigDlg*)widget;
        widget->layout()->setMargin(WINDOW_BORDER_MARGIN);
        BaseSetup(widget);
        ResizeWidget(widget);
        ui->setSizeGripEnabled(false);
    }
#endif
// TODO CodecParams TransmDlgBase
}

#endif
