#ifndef JOURNALINEVIEWER_H
#define JOURNALINEVIEWER_H

#include <QWidget>
#include <QTimer>
#include <QTextDocument>
#include <../Parameter.h>
#include <../datadecoding/Journaline.h>

namespace Ui {
class JournalineViewer;
}

class CDataDecoder;

class JournalineViewer : public QWidget
{
    Q_OBJECT

public:
    explicit JournalineViewer(int, QWidget *parent = 0);
    ~JournalineViewer();

private:
    Ui::JournalineViewer *ui;
    QTextDocument   document;
    int             short_id;

private slots:
    void on_buttonClearCache_clicked();
public slots:
    void setSavePath(const QString&);
    void setStatus(int, ETypeRxStatus);
    void setDecoder(CDataDecoder* dec);
    void setServiceInformation(const CService&, uint32_t);
};

#endif // JOURNALINEVIEWER_H
