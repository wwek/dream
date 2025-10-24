#ifndef OPENRSCIDIALOG_H
#define OPENRSCIDIALOG_H

#include <QDialog>

namespace Ui {
class OpenRSCIDialog;
}

class OpenRSCIDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OpenRSCIDialog(QWidget *parent = 0);
    ~OpenRSCIDialog();
    QString getFile() {
        return file;
    }
    void setPath(const QString& p) {
        path = p;
    }
public slots:
    void accept();

private:
    Ui::OpenRSCIDialog *ui;
    QString file;
    QString path;
private slots:
    void OnChooseFile();
};

#endif // OPENRSCIDIALOG_H
