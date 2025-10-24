#ifndef BWSVIEWER_H
#define BWSVIEWER_H

#include <QWidget>
#include <QTimer>
#include <../Parameter.h>
#include "../util/Utilities.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

namespace Ui {
class BWSViewerWidget;
}

class CDataDecoder;
class CMOTObject;
class QShowEvent;
class QHideEvent;


class CWebsiteObject
{
public:
    CWebsiteObject(unsigned int id, const QString& strContentType, CVector<_BYTE>& vecbData)
        : id(id), strContentType(strContentType),
          data(QByteArray((const char*)&vecbData[0], vecbData.Size())) {}
    virtual ~CWebsiteObject() {}
    unsigned int    id;
    QString         strContentType;
    QByteArray      data;
};


class CWebsiteCache : public QObject
{
    Q_OBJECT

public:
    CWebsiteCache() : id_counter(0), total_size(0) {}
    virtual ~CWebsiteCache() {}
    void GetObjectCountAndSize(unsigned int& count, unsigned int& size);
    void ClearAll();
    void AddObject(QString strObjName, QString strContentType, CVector<_BYTE>& vecbData);
    int GetObjectContentType(const QString& strObjName, QString& strContentType);
    int GetObjectSize(const QString& strObjName, const unsigned int id);
    int CopyObjectData(const QString& strObjName, const unsigned int id, char *buffer, int maxsize, int offset);
    bool SetDirectoryIndex(const QString strNewDirectoryIndex);
    QString GetDirectoryIndex();

protected:
    CWebsiteObject* FindObject(const QString& strObjName);
    std::map<QString,CWebsiteObject> objects;
    QMutex          mutex;
    QString         strDirectoryIndex;
    unsigned int    id_counter;
    unsigned int    total_size;

signals:
    void ObjectAdded(QString strObjName);
};


class CNetworkReplyCache : public QNetworkReply
{
    Q_OBJECT

public:
    CNetworkReplyCache(QNetworkAccessManager::Operation op,
                       const QNetworkRequest& req, CWebsiteCache& cache, CCounter& waitobjs);
    virtual ~CNetworkReplyCache();
    void abort() {
        id = 0;
    }
    qint64 bytesAvailable() const;
    bool isSequential() const {
        return true;
    }

protected:
    qint64 readData(char * data, qint64 maxSize);
    void customEvent(QEvent* event);
    CWebsiteCache&      cache;
    CCounter&           waitobjs;
    QString             path;
    int                 readOffset;
    bool                emitted;
    unsigned int        id;

public slots:
    void CheckObject(QString strObjName);
};


class CNetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT

public:
    CNetworkAccessManager(QObject* parent, CWebsiteCache& cache,
                          CCounter& waitobjs, const bool& bAllowExternalContent, const QString& strCacheHost)
        : QNetworkAccessManager(parent), cache(cache), waitobjs(waitobjs),
          bAllowExternalContent(bAllowExternalContent), strCacheHost(strCacheHost) {}
    virtual ~CNetworkAccessManager() {}

protected:
    QNetworkReply *createRequest(Operation op, const QNetworkRequest & req, QIODevice *);
    CWebsiteCache&  cache;
    CCounter&       waitobjs;
    const bool&     bAllowExternalContent;
    const QString&  strCacheHost;
};

class BWSViewerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BWSViewerWidget(QWidget* parent = nullptr);
    ~BWSViewerWidget();
public slots:
    void setDecoder(CDataDecoder*);
    void setServiceInformation(int, CService);
    void setStatus(int, ETypeRxStatus);
    void setSavePath(const QString&);
private:
    Ui::BWSViewerWidget* ui;
    int             short_id;
    CDataDecoder*   decoder;
    CService        service;
    CNetworkAccessManager nam;
    CWebsiteCache   cache;
    bool            bHomeSet;
    bool            bPageLoading;
    bool            bSaveFileToDisk;
    bool            bRestrictedProfile;
    bool            bAllowExternalContent;
    bool            bClearCacheOnNewService;
    bool            bDirectoryIndexChanged;
    unsigned int    iLastAwaitingOjects;
    CCounter        waitobjs;
    const QString   strCacheHost;
    bool            bLastServiceValid;
    uint32_t        iLastServiceID, iCurrentDataServiceID, iLastValidServiceID;
    QString         strCurrentSavePath;
    QTimer          Timer;
    ETypeRxStatus   status;

    bool Changed();
    void SaveMOTObject(const QString& strObjName, const CMOTObject& obj);
    bool RemoveDir(const QString &dirName, int level = 0);
    void UpdateButtons();
    void UpdateStatus();
    void UpdateWindowTitle(const uint32_t iServiceID, const bool bServiceValid, QString strLabel);
    QString ObjectStr(unsigned int count);
    void SetupSavePath(QString& strSavePath);
    void GetServiceParams(uint32_t* iServiceID, bool* bServiceValid, QString* strLabel, ETypeRxStatus* eStatus=nullptr);
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);

private slots:
    void OnClearCache();
    void OnHome();
    void OnStopRefresh();
    void OnBack();
    void OnForward();
    void OnSetProfile(bool);
    void OnSaveFileToDisk(bool);
    void OnAllowExternalContent(bool);
    void OnClearCacheOnNewService(bool);
    void OnWebViewLoadStarted();
    void OnWebViewLoadFinished(bool ok);
    void OnWebViewTitleChanged(const QString& title);
    void Update();
    void OnTimer();
};

#endif // BWSVIEWER_H
