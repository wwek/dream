#ifndef COREAUDIO_MACOS_H
#define COREAUDIO_MACOS_H

#include <AudioToolbox/AudioQueue.h>
#include <CoreAudio/CoreAudio.h>
#include <QObject>
#include <QByteArray>
#include <QTimer>
#include <QThread>
#include <QDebug>

class CoreAudioInput : public QObject
{
    Q_OBJECT
public:
    explicit CoreAudioInput(QObject *parent = nullptr);
    ~CoreAudioInput();

    bool initialize(int sampleRate = 48000, int channels = 2);
    bool start(const QString &deviceName = QString());
    void stop();
    qint64 read(char *data, qint64 maxSize);
    bool isOpen() const { return isRunning; }
    qint64 bytesAvailable() const { return dataBuffer.size(); }

signals:
    void dataAvailable(const QByteArray &data);
    void errorOccurred(const QString &error);

private slots:
    void checkDataAvailable();
    void appendData(const QByteArray &newData);

private:
    AudioQueueRef audioQueue;
    AudioStreamBasicDescription audioFormat;
    bool isRunning;
    QByteArray dataBuffer;
    QTimer *dataTimer;
    static const int BUFFER_SIZE = 4096;
    static const int BUFFER_COUNT = 3;

    static void audioInputCallback(void *inUserData,
                                AudioQueueRef inAQ,
                                AudioQueueBufferRef inBuffer,
                                const AudioTimeStamp *inStartTime,
                                UInt32 inNumberPacketDescriptions,
                                const AudioStreamPacketDescription *inPacketDescs);

    void logError(const QString &message, OSStatus status);
};

#endif // COREAUDIO_MACOS_H