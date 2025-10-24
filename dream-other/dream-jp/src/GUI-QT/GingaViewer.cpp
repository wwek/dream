/******************************************************************************\
 * PUC-Rio / Telemidia Lab.
 * Copyright (c) 2015-2018
 *
 * 
 * Author(s):
 *   Rafael Diniz
 *
 * Description: Ginga Application Executor
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

#include "GingaViewer.h"
#include "../DrmReceiver.h"
#include "../util-QT/Util.h"
#include "../datadecoding/DataDecoder.h"
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QProcess>

GingaViewer::GingaViewer(CDRMReceiver& rec, CSettings& Settings, QWidget* parent):
    CWindow(parent, Settings, "Ginga"),
    receiver(rec), decoder(NULL), 
    receivedObjects(0), receivedSize(0), totalObjects(0),
    iCurrentDataServiceID(0)
{

    carouselDirectory = Settings.Get("Receiver", "datafilesdirectory", string(DEFAULT_DATA_FILES_DIRECTORY));
    strDataDir = QString::fromUtf8(carouselDirectory.c_str());

    connect(&Timer, SIGNAL(timeout()), this, SLOT(OnTimer()));
    
    Timer.start(GUI_CONTROL_UPDATE_TIME);

}

GingaViewer::~GingaViewer()
{
    Timer.stop();
}

void GingaViewer::OnTimer()
{
    /* Get current service parameters */
    uint32_t iServiceID;
    bool bServiceValid;
    QString strLabel;
    ETypeRxStatus eStatus;
    GetServiceParams(&iServiceID, &bServiceValid, &strLabel, &eStatus);

    /* Check for new valid data service */
    if (bServiceValid)
    {
        if (iCurrentDataServiceID != iServiceID)
        {
            // cerr << "Restart Ginga?" << endl;
        }

        iCurrentDataServiceID = iServiceID;
    }
    
    
    switch(eStatus)
    {
    case NOT_PRESENT:
        break;
        
    case CRC_ERROR:
        qDebug("MOT CRC ERROR");
        break;

    case DATA_ERROR:
        qDebug("MOT DATA ERROR");
        break;

    case RX_OK:
        // qDebug("MOT RX OK");
        break;
    }

    if (decoder == NULL)
    {
        decoder = receiver.GetDataDecoder();
        if (decoder == NULL)
            qDebug("can't get data decoder from receiver");
    }

    if (Changed())
    {

        if (receivedObjects == totalObjects)
        {
            // TODO: Embed the Ginga player GUI to Dream
            qDebug("Calling Ginga\n");

            QString path = savePath();
            QString index = QString::fromStdString(directoryIndex);
            QStringList args;

            QProcess *process = new QProcess(this);

            args << path + index;

            if ( !process->startDetached("ginga", args) )
            {
                QMessageBox::information(this, "Dream", tr("Ginga executable not found in PATH."));
                Timer.stop();
            }
            
        }
    }

    
}

void GingaViewer::eventShow(QShowEvent*)
{
    // TODO: for now we'll use Ginga's own window
    this->hide();
}

void GingaViewer::eventHide(QHideEvent*)
{

}

bool GingaViewer::Changed()
{
    bool bChanged = false;

    if (decoder != NULL)
    {
        CMOTObject obj;

        /* Poll the data decoder module for new object */
        while (decoder->GetMOTObject(obj, CDataDecoder::AT_GINGA))
        {
            /* Get the current directory */
            CMOTDirectory MOTDir;
            if (decoder->GetMOTDirectory(MOTDir, CDataDecoder::AT_GINGA))
            {
                /* ETSI TS 101 498-1 Section 5.5.1 */
                
                totalObjects = MOTDir.iNumberOfObjects;
                
                /* Checks if the DirectoryIndex has values */
                if (MOTDir.DirectoryIndex.size() > 0)
                {
                    /* Only profile 1 is defined for Ginga */
                    if(MOTDir.DirectoryIndex.find(1) != MOTDir.DirectoryIndex.end())
                        directoryIndex = MOTDir.DirectoryIndex[1].c_str();
                }
            }
            
            /* Get object name */
            QString strObjName(obj.strName.c_str());
            
            // cerr << "New object name : " << obj.strName.c_str() << endl;
            
            /* Get ContentType - Ginga don't care about the MIME type */
            QString strContentType(obj.strMimeType.c_str());
            
            SaveMOTObject(strObjName, obj);
            
            bChanged = true;

        }
    }
    return bChanged;
}

void GingaViewer::SaveMOTObject(const QString& strObjName,
                              const CMOTObject& obj)
{
    const CVector<_BYTE>& vecbRawData = obj.Body.vecData;

    QString strCurrentSavePath = savePath();

    /* Generate safe filename */
    QString strFileName = strCurrentSavePath + strObjName;

    /* First, create directory for storing the file (if not exists) */
    CreateDirectories(strFileName);

    /* Open file */
    QFile file(strFileName);
    if (file.open(QIODevice::WriteOnly))// | QIODevice::Truncate))
    {
        int i, written, size;
        size = vecbRawData.Size();

        receivedSize += size;
        receivedObjects++;
        
        /* Write data */
        for (i = 0, written = 0; size > 0 && written >= 0; i+=written, size-=written) {
            const _BYTE& bv = vecbRawData.Data().at(unsigned(i));
            written = file.write(reinterpret_cast<const char*>(&bv), size);
        }

        /* Close the file afterwards */
        file.close();

        fprintf(stderr, "Received file %s. File %u of %u. Total bytes received: %u\n",
                strFileName.toStdString().c_str(), receivedObjects, totalObjects, receivedSize);
    }
    else
    {
        QMessageBox::information(this, file.errorString(), strFileName);
    }
}

QString GingaViewer::savePath()
{
    /* Append service ID to MOT save path */
    return strDataDir+QString("/MOT/%2/").arg(iCurrentDataServiceID, 8).toUpper();
}

void GingaViewer::GetServiceParams(uint32_t* iServiceID, bool* bServiceValid, QString* strLabel, ETypeRxStatus* eStatus)
{
    CParameter& Parameters = *receiver.GetParameters();
    Parameters.Lock();
    const int iCurSelDataServ = Parameters.GetCurSelDataService();
    const CService service = Parameters.Service[iCurSelDataServ];
    if (eStatus)
        *eStatus = Parameters.DataComponentStatus[iCurSelDataServ].GetStatus();
    Parameters.Unlock();
    if (iServiceID)
        *iServiceID = service.iServiceID;
    if (bServiceValid)
        *bServiceValid = service.IsActive() && service.eAudDataFlag == CService::SF_DATA;
    /* Do UTF-8 to QString (UNICODE) conversion with the label strings */
    if (strLabel)
        *strLabel = QString().fromUtf8(service.strLabel.c_str()).trimmed();
}
