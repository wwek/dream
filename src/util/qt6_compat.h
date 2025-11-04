#ifndef QT6_COMPAT_H
#define QT6_COMPAT_H

// Qt6 compatibility shims
#include <QAudioDevice>
#include <QMediaDevices>
#include <QAudioFormat>
#include <QAudio>

// QAudioDeviceInfo compatibility - create a wrapper class
class QAudioDeviceInfo {
public:
    QAudioDeviceInfo(const QAudioDevice &device) : m_device(device) {}

    QString deviceName() const { return m_device.description(); }
    QString description() const { return m_device.description(); }

    QAudioFormat nearestFormat(const QAudioFormat &format) const {
        // Qt6 doesn't have nearestFormat - return the requested format
        Q_UNUSED(format);
        return format;
    }

    QAudioDevice nativeDevice() const { return m_device; }

private:
    QAudioDevice m_device;
};

// Implicit conversion operator for QAudioDevice to QAudioDeviceInfo
inline QAudioDeviceInfo toAudioDeviceInfo(const QAudioDevice &device) {
    return QAudioDeviceInfo(device);
}

// Enum value compatibility - use QAudioDevice::Mode
#define AudioInput QAudioDevice::Mode::Input
#define AudioOutput QAudioDevice::Mode::Output

// Device enumeration functions
inline QList<QAudioDeviceInfo> QAudioDeviceInfo_availableDevices(QAudioDevice::Mode mode) {
    if (mode == QAudioDevice::Mode::Input) {
        QList<QAudioDeviceInfo> devices;
        for (const QAudioDevice &dev : QMediaDevices::audioInputs()) {
            devices.append(QAudioDeviceInfo(dev));
        }
        return devices;
    } else {
        QList<QAudioDeviceInfo> devices;
        for (const QAudioDevice &dev : QMediaDevices::audioOutputs()) {
            devices.append(QAudioDeviceInfo(dev));
        }
        return devices;
    }
}

// Default device functions
inline QAudioDeviceInfo QAudioDeviceInfo_defaultInputDevice() {
    return QAudioDeviceInfo(QMediaDevices::defaultAudioInput());
}

inline QAudioDeviceInfo QAudioDeviceInfo_defaultOutputDevice() {
    return QAudioDeviceInfo(QMediaDevices::defaultAudioOutput());
}

// Add SDK version check override for macOS
#ifdef Q_OS_MAC
#define QT_MACOS_SDK_VERSION_IGNORE CONFIG+=sdk_no_version_check
#endif

#endif // QT6_COMPAT_H
