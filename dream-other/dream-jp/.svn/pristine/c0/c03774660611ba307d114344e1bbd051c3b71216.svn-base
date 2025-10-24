#include <QNmeaPositionInfoSource>
#include <QThread>
#include <QTemporaryFile>
#include <QtTest/QtTest>
#include <QObject>
#include "../../src/util-QT/cpos.h"

class TestCPos: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase()
    {
        qDebug("called before everything else");
        QTemporaryFile* dev = new QTemporaryFile();
        //if (!dev->open(QIODevice::ReadWrite | QIODevice::Text))
        if (!dev->open())
            return;
        QTextStream out(dev);
        out << "$GPRMC,134730.361,A,5540.3220,N,01231.2858,E,1.06,86.57,041112,,,A*55\n";
	out.flush();
	out.seek(0);
        source = new QNmeaPositionInfoSource(QNmeaPositionInfoSource::SimulationMode, this);
        if (source) {
            source->setDevice(dev);
            source->setUserEquivalentRangeError(5.1);
        }
    }
    void cleanupTestCase()
    {
        qDebug("called after tests");
    }
    void testConnection()
    {
        gps_data_t data;
        data.status=STATUS_NO_FIX;
        CPos uut(&data, source);
        QThread::sleep(1);
        QVERIFY(data.status==STATUS_FIX);
        qDebug(QString("lat %1 lng %2").arg(data.fix.latitude).arg(data.fix.longitude).toLatin1());
        QVERIFY(data.fix.longitude>12.5 && data.fix.longitude < 12.6);
        QVERIFY(data.fix.latitude>55.5 && data.fix.latitude < 55.7);
    }
private:
    QNmeaPositionInfoSource *source;
};

QTEST_MAIN(TestCPos)
#include "testcpos.moc"
