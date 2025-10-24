#ifndef ENGINEERINGTABWIDGET_H
#define ENGINEERINGTABWIDGET_H

#include <QTabWidget>

class ReceiverController;

class EngineeringTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit EngineeringTabWidget(ReceiverController*, QWidget *parent = 0);

signals:

public slots:

};

#endif // ENGINEERINGTABWIDGET_H
