#include "coreaudio_macos.h"
#include <CoreAudio/CoreAudio.h>
#include <AudioToolbox/AudioQueue.h>

CoreAudioInput::CoreAudioInput(QObject *parent)
    : QObject(parent), audioQueue(nullptr), isRunning(false)
{
    dataTimer = new QTimer();
    dataTimer->moveToThread(this->thread());
    connect(dataTimer, &QTimer::timeout, this, &CoreAudioInput::checkDataAvailable);
    dataTimer->moveToThread(QThread::currentThread());

    // 设置音频格式 (16-bit PCM, 立体声, 48kHz)
    audioFormat.mSampleRate = 48000.0;
    audioFormat.mFormatID = kAudioFormatLinearPCM;
    audioFormat.mFormatFlags = kAudioFormatFlagIsSignedInteger |
                              kAudioFormatFlagIsPacked |
                              kAudioFormatFlagsNativeEndian;
    audioFormat.mChannelsPerFrame = 2;
    audioFormat.mBitsPerChannel = 16;
    audioFormat.mBytesPerFrame = (audioFormat.mBitsPerChannel / 8) * audioFormat.mChannelsPerFrame;
    audioFormat.mFramesPerPacket = 1;
    audioFormat.mBytesPerPacket = audioFormat.mBytesPerFrame;
}

CoreAudioInput::~CoreAudioInput()
{
    // Ensure timer is stopped in correct thread
    if (dataTimer) {
        if (QThread::currentThread() == dataTimer->thread()) {
            dataTimer->stop();
            dataTimer->deleteLater();
        } else {
            QMetaObject::invokeMethod(dataTimer, "stop", Qt::QueuedConnection);
            QMetaObject::invokeMethod(dataTimer, "deleteLater", Qt::QueuedConnection);
        }
        dataTimer = nullptr;
    }
    stop();
}

bool CoreAudioInput::initialize(int sampleRate, int channels)
{
    if (isRunning) {
        stop();
    }

    // 更新音频格式
    audioFormat.mSampleRate = sampleRate;
    audioFormat.mChannelsPerFrame = channels;
    audioFormat.mBytesPerFrame = (audioFormat.mBitsPerChannel / 8) * audioFormat.mChannelsPerFrame;
    audioFormat.mBytesPerPacket = audioFormat.mBytesPerFrame;

    qDebug() << "CoreAudio: Initialized with format:"
             << sampleRate << "Hz," << channels << "channels";

    return true;
}

bool CoreAudioInput::start(const QString &deviceName)
{
    if (isRunning) {
        qDebug() << "CoreAudio: Already running";
        return true;
    }

    qDebug() << "CoreAudio: Starting audio input for device:" << deviceName;

    // 创建音频队列
    OSStatus status = AudioQueueNewInput(&audioFormat, audioInputCallback,
                                        this, nullptr, nullptr, 0, &audioQueue);
    if (status != noErr) {
        logError("AudioQueueNewInput failed", status);
        return false;
    }

    // 设置音频输入设备 (默认输入设备)
    AudioObjectID defaultInputDevice = kAudioObjectUnknown;
    UInt32 propertySize = sizeof(AudioObjectID);

    // 获取默认输入设备ID
    AudioObjectPropertyAddress propertyAddress = {
        kAudioHardwarePropertyDefaultInputDevice,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
    };

    status = AudioObjectGetPropertyData(kAudioObjectSystemObject,
                                       &propertyAddress,
                                       0,
                                       nullptr,
                                       &propertySize,
                                       &defaultInputDevice);

    if (status == noErr && defaultInputDevice != kAudioObjectUnknown) {
        // 设置音频队列的当前设备 - 注意：kAudioQueueProperty_CurrentDevice 在 iOS/macOS 上不可用
        // 暂时跳过设备设置，使用默认设备
        qDebug() << "CoreAudio: Using default input device, device ID:" << defaultInputDevice;

        // 在 macOS 上，AudioQueueNewInput 会自动使用默认输入设备
        // 如需特定设备，需要使用 AudioUnit API
    } else {
        qWarning() << "CoreAudio: Failed to get default input device:" << status;
    }

    // 创建并分配缓冲区
    for (int i = 0; i < BUFFER_COUNT; ++i) {
        AudioQueueBufferRef buffer;
        status = AudioQueueAllocateBuffer(audioQueue, BUFFER_SIZE, &buffer);
        if (status != noErr) {
            logError(QString("AudioQueueAllocateBuffer %1 failed").arg(i), status);
            continue;
        }

        status = AudioQueueEnqueueBuffer(audioQueue, buffer, 0, nullptr);
        if (status != noErr) {
            logError(QString("AudioQueueEnqueueBuffer %1 failed").arg(i), status);
        }
    }

    // 启动音频队列
    status = AudioQueueStart(audioQueue, nullptr);
    if (status != noErr) {
        logError("AudioQueueStart failed", status);
        AudioQueueDispose(audioQueue, true);
        audioQueue = nullptr;
        return false;
    }

    isRunning = true;
    dataBuffer.clear();
    dataTimer->start(10); // 每10ms检查一次数据

    qDebug() << "CoreAudio: Audio input started successfully";

    // 验证音频队列状态
    UInt32 isRunning = 0;
    UInt32 dataSize = sizeof(isRunning);
    status = AudioQueueGetProperty(audioQueue, kAudioQueueProperty_IsRunning,
                                 &dataSize, &isRunning);
    if (status == noErr) {
        qDebug() << "CoreAudio: Queue is running check:" << isRunning;
    } else {
        qWarning() << "CoreAudio: Failed to check queue running status:" << status;
    }

    return true;
}

void CoreAudioInput::stop()
{
    if (!isRunning) {
        return;
    }

    qDebug() << "CoreAudio: Stopping audio input";

    // Stop timer safely
    if (dataTimer && QThread::currentThread() == dataTimer->thread()) {
        dataTimer->stop();
    } else if (dataTimer) {
        QMetaObject::invokeMethod(dataTimer, "stop", Qt::QueuedConnection);
    }

    isRunning = false;

    if (audioQueue) {
        AudioQueueStop(audioQueue, true);
        AudioQueueDispose(audioQueue, true);
        audioQueue = nullptr;
    }

    dataBuffer.clear();
    qDebug() << "CoreAudio: Audio input stopped";
}

qint64 CoreAudioInput::read(char *data, qint64 maxSize)
{
    if (!isRunning || dataBuffer.isEmpty()) {
        return 0;
    }

    qint64 bytesToRead = qMin(maxSize, (qint64)dataBuffer.size());
    memcpy(data, dataBuffer.constData(), bytesToRead);
    dataBuffer.remove(0, bytesToRead);

    return bytesToRead;
}

void CoreAudioInput::audioInputCallback(void *inUserData,
                                      AudioQueueRef inAQ,
                                      AudioQueueBufferRef inBuffer,
                                      const AudioTimeStamp *inStartTime,
                                      UInt32 inNumberPacketDescriptions,
                                      const AudioStreamPacketDescription *inPacketDescs)
{
    CoreAudioInput *self = static_cast<CoreAudioInput*>(inUserData);

    if (!self || !self->isRunning) {
        static int callbackWarningCount = 0;
        if (callbackWarningCount < 5) {
            qWarning() << "CoreAudio: Callback called but self is null or not running";
            callbackWarningCount++;
        }
        return;
    }

    // 将音频数据添加到缓冲区
    if (inBuffer->mAudioDataByteSize > 0) {
        static int dataCount = 0;
        dataCount++;
        if (dataCount <= 10 || dataCount % 1000 == 0) {
            qDebug() << "CoreAudio: Callback received" << inBuffer->mAudioDataByteSize << "bytes (count:" << dataCount << ")";
        }

        QByteArray newData((char*)inBuffer->mAudioData, inBuffer->mAudioDataByteSize);

        // 线程安全地添加数据 - 始终使用信号槽确保线程安全
        QMetaObject::invokeMethod(self, "appendData", Qt::QueuedConnection,
                              Q_ARG(QByteArray, newData));
    } else {
        // 记录无数据情况
        static int noDataCount = 0;
        noDataCount++;
        if (noDataCount <= 5 || noDataCount % 1000 == 0) {
            qWarning() << "CoreAudio: Callback called but no audio data available (count:" << noDataCount << ")";
        }
    }

    // 重新入队缓冲区
    OSStatus status = AudioQueueEnqueueBuffer(inAQ, inBuffer, 0, nullptr);
    if (status != noErr) {
        qWarning() << "CoreAudio: Failed to re-enqueue buffer:" << status;
    }
}

void CoreAudioInput::checkDataAvailable()
{
    if (!dataBuffer.isEmpty()) {
        emit dataAvailable(dataBuffer);
    }
}

void CoreAudioInput::appendData(const QByteArray &newData)
{
    // Thread-safe data addition
    dataBuffer.append(newData);

    // 限制缓冲区大小以防止内存溢出
    const int MAX_BUFFER_SIZE = 32768;  // 减小缓冲区大小
    if (dataBuffer.size() > MAX_BUFFER_SIZE) {
        dataBuffer = dataBuffer.right(MAX_BUFFER_SIZE);
    }

    emit dataAvailable(newData);
}

void CoreAudioInput::logError(const QString &message, OSStatus status)
{
    QString errorStr;
    switch (status) {
        case -43: // kAudio_FileNotFoundError
            errorStr = "Audio file not found";
            break;
        case -44: // kAudio_FilePermissionsError
            errorStr = "Permissions error";
            break;
        case -45: // kAudio_FileBadFilePath
            errorStr = "Bad file path";
            break;
        case -50: // kAudio_FileInvalidFile
            errorStr = "Invalid file";
            break;
        case -51: // kAudio_FileFormatUnsupported
            errorStr = "Unsupported format";
            break;
        default:
            errorStr = QString("CoreAudio error code: %1").arg(status);
            break;
    }

    qCritical() << "CoreAudio Error:" << message << "-" << errorStr;
    emit errorOccurred(QString("%1 - %2").arg(message, errorStr));
}