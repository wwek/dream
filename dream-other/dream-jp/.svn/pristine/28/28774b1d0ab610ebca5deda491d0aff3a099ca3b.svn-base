#ifndef SERVICESELECTOR_H
#define SERVICESELECTOR_H

#include <QWidget>
#include <QButtonGroup>
#include <QPushButton>
#include <QLabel>
#include <vector>

namespace Ui {
class ServiceSelector;
}

class CService;

class ServiceSelector : public QWidget
{
    Q_OBJECT

public:
    explicit ServiceSelector(QWidget *parent = 0);
    ~ServiceSelector();
    void check(int i);
    void disableAll();
public slots:
    void onServiceChanged(int, const CService&);
private:
    Ui::ServiceSelector* ui;
    QButtonGroup*        pAudioButtonGroup;
    QButtonGroup*        pDataButtonGroup;
    std::vector<QLabel*> serviceLabels;
    std::vector<QPushButton*> buttons;
signals:
    void audioServiceSelected(int);
    void dataServiceSelected(int);
};

#endif // SERVICESELECTOR_H
