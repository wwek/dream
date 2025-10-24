/******************************************************************************\
 * PUC-Rio / Lab. Telemidia 
 * Copyright (c) 2015
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

#ifndef _GINGAVIEWER_H
#define _GINGAVIEWER_H

#include "DialogUtil.h"
#include "CWindow.h"
#include <QMutex>

class CMOTObject;
class CDRMReceiver;
class CDataDecoder;

class GingaViewer : public CWindow
{
    Q_OBJECT

public:
    GingaViewer(CDRMReceiver&, CSettings&, QWidget* parent = 0);
    virtual ~GingaViewer();

protected:
    QTimer          Timer;
    CDRMReceiver&   receiver;
    CDataDecoder*   decoder;
    
    string          carouselDirectory;
    string          directoryIndex;
    uint32_t        receivedObjects;
    uint32_t        receivedSize;
    uint32_t        totalObjects;
    uint32_t        iCurrentDataServiceID;    
    QString         strDataDir;
    

    bool Changed();
    void SaveMOTObject(const QString& strObjName, const CMOTObject& obj);
    bool RemoveDir(const QString &dirName, int level = 0);
    QString savePath();
    void GetServiceParams(uint32_t* iServiceID=NULL, bool* bServiceValid=NULL, QString* strLabel=NULL, ETypeRxStatus* eStatus=NULL);
    virtual void eventShow(QShowEvent*);
    virtual void eventHide(QHideEvent*);

public slots:
    void OnTimer();
};

#endif

