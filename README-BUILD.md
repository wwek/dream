# Dream DRM Receiver 2.2.x - 构建说明

这是一个**独立的 DRM 解码器二进制程序**,用于接收和解码 Digital Radio Mondiale (DRM) 广播信号。

## 特性

### ✅ 已应用的 xHE-AAC 解码修复

本版本包含了来自 [SourceForge 论坛讨论](https://sourceforge.net/p/drm/discussion/general/thread/01c6e64c3b/) 的所有关键修复:

1. **帧边界计算修复** (`src/MSC/xheaacsuperframe.cpp`)
   - 正确处理 xHE-AAC 超帧格式
   - 修复了原始代码错误的2字节头部偏移

2. **缓冲区溢出保护** (`src/sourcedecoders/AudioSourceDecoder.cpp:1378`)
   - 使用 `OUTPUT_BUFFER_OVERHEAD_MARGIN` 防止可变帧长导致的溢出
   - 由 clang AddressSanitizer 检测发现的问题

3. **除零保护** (`src/sourcedecoders/AudioSourceDecoder.cpp:638, 1387`)
   - 防止 `iAudioSampleRate == 0` 时的崩溃
   - 在初始化和解码过程中都有保护

4. **USAC 编译支持** (`src/sourcedecoders/fdk_aac_codec.cpp:89`)
   - 正确启用 FDK-AAC 的 xHE-AAC (USAC) 支持

## 系统要求

### 必需依赖

#### Debian/Ubuntu
```bash
sudo apt-get install \
    qt5-qmake qtbase5-dev \
    build-essential \
    libfdk-aac-dev \
    libspeexdsp-dev \
    libspeex-dev \
    libfftw3-dev \
    libpulse-dev \
    libsamplerate0-dev \
    libfaad-dev \
    libopus-dev \
    libhamlib-dev
```

#### macOS
```bash
brew install \
    qt@5 \
    fdk-aac \
    speex \
    speexdsp \
    fftw \
    libsamplerate \
    pulseaudio \
    faad2 \
    opus \
    hamlib
```

### 关键依赖说明

- **libfdk-aac-dev**: FDK-AAC v2 编解码器,支持 xHE-AAC (USAC)
- **libspeex-dev / libspeexdsp-dev**: Speex 重采样器
- **libfftw3-dev**: 快速傅里叶变换库
- **libpulse-dev**: PulseAudio 音频输出
- **qt5-qmake**: 即使是 console 模式也需要 qmake 构建系统

## 构建方法

### 方法 1: 使用 Docker 构建 (推荐)

**优点**:
- ✅ 无需安装依赖,环境隔离
- ✅ 可重现的构建环境
- ✅ 自动验证所有 xHE-AAC 修复
- ✅ 支持多平台 (x86_64, ARM64)

```bash
# 进入 Dream 目录
cd extensions/dream.x

# 构建 Docker 镜像
docker build -t dream-drm:2.2 .

# 运行测试验证二进制
docker run --rm dream-drm:2.2 /usr/local/bin/dream --help

# 提取编译好的二进制到当前目录
docker run --rm -v "$(pwd):/output" dream-drm:2.2 \
    sh -c "cp /usr/local/bin/dream /output/dream"

# 验证提取的二进制
./dream --help
```

**常用 Docker 命令**:

```bash
# 查看镜像信息
docker images dream-drm:2.2

# 进入容器交互式调试
docker run --rm -it dream-drm:2.2 /bin/bash

# 检查二进制链接的库
docker run --rm dream-drm:2.2 ldd /usr/local/bin/dream

# 清理 Docker 镜像
docker rmi dream-drm:2.2

# 重新构建 (不使用缓存)
docker build --no-cache -t dream-drm:2.2 .
```

### 方法 2: 使用本地构建脚本

```bash
cd extensions/dream.x

# 运行构建脚本 (会自动检查依赖和验证修复)
./build.sh

# 或指定自定义安装路径
INSTALL_PREFIX=/opt/dream ./build.sh
```

### 方法 3: 手动构建

```bash
cd extensions/dream.x

# 清理之前的构建
make distclean 2>/dev/null || true

# 配置 (console 模式 - 无 GUI, 启用 FDK-AAC)
qmake CONFIG+=console CONFIG+=fdk-aac dream.pro

# 编译
make -j$(nproc)

# 安装
sudo make install

# 重命名为 dream (避免与其他版本冲突)
sudo mv /usr/bin/dream /usr/local/bin/dream
```

## 配置选项

### CONFIG+=console (必需)

**为什么使用 console 模式?**

```qmake
console {
    QT -= core gui          # 不依赖 Qt GUI 库
    UI_MESSAGE = console mode
    VERSION_MESSAGE = No Qt
    SOURCES += src/main.cpp # 使用命令行入口点
    unix:!cross_compile {
        HEADERS += src/linux/ConsoleIO.h
        SOURCES += src/linux/ConsoleIO.cpp
        LIBS += -lpthread   # 线程支持
    }
}
```

**优点:**
- ✅ 适合服务器/无头系统运行
- ✅ 更小的二进制文件
- ✅ 更少的依赖
- ✅ 可通过管道接收 SDR 数据流
- ✅ 适合集成到 OpenWebRX

## 使用方法

### 基本用法

```bash
# 查看帮助
dream --help

# 从文件解码
dream -i recording.iq -o audio.wav


# 使用特定声音设备
dream -i input.iq --sound-device pulse
```

### 常见 DRM 频率

| 电台 | 频率 | 位置 | 模式 |
|------|------|------|------|
| Voice of Nigeria | 13675 kHz | 非洲 | DRM30 |
| All India Radio | 621 kHz | 印度 | DRM30 |
| Radio Romania | 153 kHz | 欧洲 | DRM30 |
| Deutsche Welle | 多个短波频率 | 全球 | DRM30 |

### 集成到 OpenWebRX

Dream 2.2.x 作为**独立的解码器进程**,可以通过以下方式集成:

```python
# 在 OpenWebRX 中启动 Dream 解码器
import subprocess

dream_process = subprocess.Popen(
    ['dream', '-i', '-', '-o', '/dev/null'],
    stdin=subprocess.PIPE,
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE
)

# 将 SDR 数据流传送给 Dream
dream_process.stdin.write(iq_data)
```

## 验证构建

### 检查 xHE-AAC 支持

```bash
# 运行 Dream 并检查启动信息
dream 2>&1 | grep -i "fdk\|xhe\|usac"

# 检查链接的库
ldd /usr/local/bin/dream | grep fdk-aac
# 或在 macOS:
otool -L /usr/local/bin/dream | grep fdk-aac
```

预期输出应包含:
```
libfdk-aac.so.2 => /usr/lib/x86_64-linux-gnu/libfdk-aac.so.2
```

### 测试解码

使用 KiwiSDR 提供的测试文件:

```bash
# 下载测试文件 (如果有)
# 或使用您自己录制的 DRM 信号

dream -i test.iq -o test.wav

# 检查输出音频
ffprobe test.wav
```

## 故障排除

### 问题: "no codec found" 错误

**原因**: FDK-AAC 库未正确链接或版本不支持 USAC

**解决**:
```bash
# 检查 FDK-AAC 版本
pkg-config --modversion fdk-aac
# 应该是 >= 2.0.0

# 重新安装 FDK-AAC v2
sudo apt-get install --reinstall libfdk-aac-dev libfdk-aac2
```

### 问题: "speex/speex_resampler.h: No such file"

**原因**: Speex 开发库未安装

**解决**:
```bash
sudo apt-get install libspeex-dev libspeexdsp-dev
```

### 问题: qmake 配置失败

**原因**: Qt5 未正确安装或环境变量未设置

**解决**:
```bash
# Ubuntu/Debian
sudo apt-get install qt5-qmake qtbase5-dev

# macOS (可能需要设置 PATH)
brew install qt@5
export PATH="/usr/local/opt/qt@5/bin:$PATH"
```

### 问题: 缓冲区溢出或崩溃

**验证**: 确认已应用所有 xHE-AAC 修复

```bash
# 检查关键修复
grep "Prevent division by zero for xHE-AAC" \
    src/sourcedecoders/AudioSourceDecoder.cpp

grep "OUTPUT_BUFFER_OVERHEAD_MARGIN" \
    src/sourcedecoders/AudioSourceDecoder.cpp

grep "frameBorderCount == 0" \
    src/MSC/xheaacsuperframe.cpp
```

## 开发信息

### 源代码结构

```
extensions/dream.x/
├── dream.pro                      # QMake 项目文件
├── src/
│   ├── main.cpp                   # Console 模式入口
│   ├── MSC/
│   │   └── xheaacsuperframe.cpp  # ✅ xHE-AAC 超帧解析 (已修复)
│   └── sourcedecoders/
│       ├── AudioSourceDecoder.cpp # ✅ 音频解码器 (已修复)
│       └── fdk_aac_codec.cpp      # ✅ FDK-AAC 接口 (已修复)
└── README-BUILD.md                # 本文件
```

### 编译选项

在 `dream.pro` 中定义的重要选项:

```qmake
# xHE-AAC 支持
fdk-aac {
    DEFINES += HAVE_LIBFDK_AAC HAVE_USAC
    LIBS += -lfdk-aac
    HEADERS += src/sourcedecoders/fdk_aac_codec.h
    SOURCES += src/sourcedecoders/fdk_aac_codec.cpp
}

# Console 模式
console {
    QT -= core gui
    SOURCES += src/main.cpp
}
```

## 参考资料

- [SourceForge 论坛 - xHE-AAC 修复讨论](https://sourceforge.net/p/drm/discussion/general/thread/01c6e64c3b/)
- [KiwiSDR Dream 移植](https://github.com/jks-prv/Beagle_SDR_GPS/tree/master/extensions/DRM/dream)
- [Dream DRM 官方网站](https://sourceforge.net/projects/drm/)
- [Digital Radio Mondiale 规范](https://www.drm.org/)

## 许可证

Dream DRM Receiver 采用 GPL v2 许可证。

## 致谢

感谢以下贡献者的 xHE-AAC 修复工作:
- John (KiwiSDR) - 原始补丁和测试
- Tarmo Tanilsoo - Linux 移植验证
- Rafael Diniz - 集成和测试
- Julian Cable - Dream 维护者
