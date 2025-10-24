#ifndef DREAMTABWIDGET_H
#define DREAMTABWIDGET_H

#include <QTabWidget>
class CService;
class ReceiverController;
class StationsWidget;

class DreamTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit DreamTabWidget(ReceiverController*, QWidget *parent = 0);
signals:
    void audioServiceSelected(int);
    void dataServiceSelected(int);

public slots:
    void onServiceChanged(int, const CService&);
    void setText(int, const QString&);
    void on_engineeringMode(bool);
    void removeServices();
private slots:
    void on_currentChanged(int);
private:
    ReceiverController* controller;
    StationsWidget*     stations;
    bool eng;

    QWidget* makeDataApp(int short_id, const CService& service) const;
    QWidget* makePacketApp(int short_id, const CService& service) const;
    void add(QWidget* w, const QString& l, int ordering);
};

#endif // DREAMTABWIDGET_H
