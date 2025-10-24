#ifndef SLIDESHOWWIDGET_H
#define SLIDESHOWWIDGET_H

#include <../Parameter.h>
#include <QWidget>

namespace Ui {
class SlideShowWidget;
}

class CMOTDABDec;

class SlideShowWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SlideShowWidget(QWidget *parent = 0);
    ~SlideShowWidget();
public slots:
    void setSavePath(const QString&);
    void setStatus(int, ETypeRxStatus);
    void setServiceInformation(int, CService);

private:

    struct Img {
        QPixmap pic;
        QString name;
    };

    Ui::SlideShowWidget *ui;
    QString                 strCurrentSavePath;
    vector<Img>             pics;
    int                     iCurImagePos;
    bool                    bClearMOTCache;
    CMOTDABDec*             motdec;
    int                     short_id;

    void                    SetImage(int);
    void                    UpdateButtons();

private slots:
    void on_ButtonStepBack_clicked();
    void on_ButtonStepForward_clicked();
    void on_ButtonJumpBegin_clicked();
    void on_ButtonJumpEnd_clicked();
    void OnSave();
    void OnSaveAll();
    void OnClearAll();
};

#endif // SLIDESHOWWIDGET_H
