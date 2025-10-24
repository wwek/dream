#include "cpos.h"
#include <QDebug>
#include <QNmeaPositionInfoSource>
#include <QFile>

CPos::CPos(gps_data_t* data, QGeoPositionInfoSource* src):gps(data)
{
    QGeoPositionInfoSource *source;
    if(src) {
        source = src;
    }
    else {
        source = QGeoPositionInfoSource::createDefaultSource(this);
    }
    if (source) {
        connect(source, SIGNAL(positionUpdated(QGeoPositionInfo)),
                this, SLOT(positionUpdated(QGeoPositionInfo)));
        source->startUpdates();
        //qDebug() << "Position Updates Requested:";
    }
    else {
        //qDebug() << "Position Updates Unavailable:";
    }
}

void CPos::positionUpdated(const QGeoPositionInfo &info)
{
    //qDebug() << "Position updated:" << info;
    if(info.isValid()) {
        gps->set=STATUS_SET; // don't set TIME_SET as it comes from the local clock, not the NMEA stream
        gps->status = STATUS_FIX;
        //gps->fix.time = double(info.timestamp().currentMSecsSinceEpoch())/1000.0;
        QGeoCoordinate xy = info.coordinate();
        if(xy.type()>=QGeoCoordinate::Coordinate2D) {
            gps->set |= LATLON_SET;
            qDebug("position set in parameters");
            gps->fix.latitude = xy.latitude();
            gps->fix.longitude = xy.longitude();
        }
        if(xy.type()==QGeoCoordinate::Coordinate3D) {
            gps->set |= ALTITUDE_SET;
            qDebug("altitude set in parameters");
            gps->fix.altitude = xy.altitude();
        }
    }
    else {
        //qDebug("position invalid");
    }
}
