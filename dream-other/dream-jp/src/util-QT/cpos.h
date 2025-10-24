#ifndef CPOS_H
#define CPOS_H

#include <QGeoPositionInfoSource>
#include <QGeoPositionInfo>
#include "../Parameter.h"

class CPos : public QObject
{
    Q_OBJECT
public:
    CPos(gps_data_t* data, QGeoPositionInfoSource* src=NULL);
    ~CPos() {}
public slots:
    void positionUpdated(const QGeoPositionInfo &info);
private:
    gps_data_t *gps;
};

#endif // CPOS_H
