#ifndef QT6_AUDIO_COMPAT_H
#define QT6_AUDIO_COMPAT_H

#include <QIODevice>
#include <QAudioSource>
#include <QAudioSink>
#include <QMediaDevices>
#include <QAudioDevice>

// Qt6 uses QAudioSource/QAudioSink instead of QAudioInput/QAudioOutput
class QCompatAudioInput : public QObject {
public:
    // Constructor for QAudioDevice (Qt6)
    QCompatAudioInput(const QAudioDevice &device, const QAudioFormat &format, QObject *parent = nullptr)
        : QObject(parent) {
        m_source = new QAudioSource(device, format, this);
    }

    // Single parameter constructor - use default format
    QCompatAudioInput(const QAudioDevice &device, QObject *parent = nullptr)
        : QObject(parent) {
        QAudioFormat format;
        // Use higher sample rates for better compatibility with macOS Sequoia
        format.setSampleRate(96000);
        format.setChannelCount(2);
        format.setSampleFormat(QAudioFormat::Int16);
        m_source = new QAudioSource(device, format, this);
    }

    // Default constructor for Qt6 - use default device
    QCompatAudioInput(QObject *parent = nullptr)
        : QObject(parent) {
        QAudioDevice defaultDevice = QMediaDevices::defaultAudioInput();
        QAudioFormat format;
        // Use higher sample rates for better compatibility with macOS Sequoia
        format.setSampleRate(96000);
        format.setChannelCount(2);
        format.setSampleFormat(QAudioFormat::Int16);
        m_source = new QAudioSource(defaultDevice, format, this);
    }

    ~QCompatAudioInput() {
        if (m_source) {
            delete m_source;
        }
    }

    void setBufferSize(int bufferSize) {
        // Qt6 buffer size is managed automatically
        Q_UNUSED(bufferSize);
    }

    void setDevice(const QAudioDevice &device) {
        Q_UNUSED(device);
    }

    QIODevice* start() {
        m_device = m_source->start();

        // Enhanced macOS compatibility: Check if device is properly initialized
        if (m_device == nullptr) {
            fprintf(stderr, "QCompatAudioInput: Warning - QAudioSource returned null device\n");
            fprintf(stderr, "QCompatAudioInput: This may indicate permission issues or device unavailability on macOS\n");
            fprintf(stderr, "QCompatAudioInput: Current state: %d, Error: %d\n",
                   m_source->state(), m_source->error());
        } else {
            fprintf(stderr, "QCompatAudioInput: Successfully started audio input\n");
            fprintf(stderr, "QCompatAudioInput: Device state: %d, Error: %d, Buffer size: %d\n",
                   m_source->state(), m_source->error(), m_source->bufferSize());
        }

        return m_device;
    }

    void stop() {
        if (m_source) {
            m_source->stop();
        }
    }

    QAudio::State state() const {
        return m_source ? m_source->state() : QAudio::StoppedState;
    }

    QAudio::Error error() const {
        return m_source ? m_source->error() : QAudio::NoError;
    }

    qint64 bytesReady() const {
        // Qt6 doesn't have bytesReady() - buffer management is automatic
        return 4096; // Return a reasonable default buffer size
    }

    int bufferSize() const {
        return m_source ? m_source->bufferSize() : 0;
    }

    int periodSize() const {
        // Qt6 doesn't have periodSize() - return a reasonable default
        return 4096;
    }

    int bytesFree() const {
        // Qt6 doesn't have bytesFree() - return a reasonable default
        return 8192;
    }

private:
    QAudioSource* m_source = nullptr;
    QIODevice* m_device = nullptr;
};

class QCompatAudioOutput : public QObject {
public:
    // Constructor for QAudioDevice (Qt6)
    QCompatAudioOutput(const QAudioDevice &device, const QAudioFormat &format, QObject *parent = nullptr)
        : QObject(parent) {
        m_sink = new QAudioSink(device, format, this);
    }

    // Single parameter constructor - use default format
    QCompatAudioOutput(const QAudioDevice &device, QObject *parent = nullptr)
        : QObject(parent) {
        QAudioFormat format;
        format.setSampleRate(48000);
        format.setChannelCount(2);
        format.setSampleFormat(QAudioFormat::Int16);
        m_sink = new QAudioSink(device, format, this);
    }

    // Default constructor for Qt6 - use default device
    QCompatAudioOutput(QObject *parent = nullptr)
        : QObject(parent) {
        QAudioDevice defaultDevice = QMediaDevices::defaultAudioOutput();
        QAudioFormat format;
        format.setSampleRate(48000);
        format.setChannelCount(2);
        format.setSampleFormat(QAudioFormat::Int16);
        m_sink = new QAudioSink(defaultDevice, format, this);
    }

    ~QCompatAudioOutput() {
        if (m_sink) {
            delete m_sink;
        }
    }

    void setBufferSize(int bufferSize) {
        Q_UNUSED(bufferSize);
    }

    void setDevice(const QAudioDevice &device) {
        Q_UNUSED(device);
    }

    QIODevice* start() {
        m_device = m_sink->start();
        return m_device;
    }

    void stop() {
        if (m_sink) {
            m_sink->stop();
        }
    }

    QAudio::State state() const {
        return m_sink ? m_sink->state() : QAudio::StoppedState;
    }

    QAudio::Error error() const {
        return m_sink ? m_sink->error() : QAudio::NoError;
    }

    int bufferSize() const {
        return m_sink ? m_sink->bufferSize() : 0;
    }

    int periodSize() const {
        // Qt6 doesn't have periodSize() - return a reasonable default
        return 4096;
    }

    int bytesFree() const {
        // Qt6 doesn't have bytesFree() - return a reasonable default
        return 8192;
    }

private:
    QAudioSink* m_sink = nullptr;
    QIODevice* m_device = nullptr;
};

// Use wrapper classes directly to avoid macro conflicts
typedef QCompatAudioInput CAudioInput;
typedef QCompatAudioOutput CAudioOutput;

#endif // QT6_AUDIO_COMPAT_H
