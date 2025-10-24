# Dream DRM Receiver 2.2.x

独立的 DRM (Digital Radio Mondiale) 解码器,支持 xHE-AAC 音频解码。

## 快速开始

### 使用 Docker 构建 (推荐)

```bash
# 构建 Docker 镜像
docker build -t dream-drm:2.2 .

# 提取编译好的二进制
docker run --rm -v "$(pwd):/output" dream-drm:2.2 \
    sh -c "cp /usr/local/bin/dream /output/dream"

# 运行
./dream --help

```

### 使用本地构建脚本

```bash
# 构建 Dream (console模式,无GUI)
./build.sh

# 运行
dream --help

```

## 特性

✅ **xHE-AAC 解码支持**
- 使用 FDK-AAC v2 编解码器
- 支持 USAC (Unified Speech and Audio Coding)
- 应用了所有 SourceForge 论坛修复补丁

✅ **Console 模式**
- 无 Qt GUI 依赖
- 适合服务器/无头系统
- 可通过管道接收 SDR 数据流
- 独立二进制文件

✅ **已修复的问题**
- 帧边界计算错误 (`xheaacsuperframe.cpp`)
- 缓冲区溢出 (`AudioSourceDecoder.cpp`)
- 除零崩溃 (`AudioSourceDecoder.cpp`)
- USAC 编译支持 (`fdk_aac_codec.cpp`)

## 系统要求

### Debian/Ubuntu
```bash
sudo apt-get install \
    qt5-qmake qtbase5-dev build-essential \
    libfdk-aac-dev libspeexdsp-dev libspeex-dev \
    libfftw3-dev libpulse-dev libsamplerate0-dev \
    libfaad-dev libopus-dev libhamlib-dev
```

### macOS
```bash
brew install qt@5 fdk-aac speex speexdsp fftw \
    libsamplerate pulseaudio faad2 opus hamlib

# 添加 Qt5 到 PATH
export PATH="/usr/local/opt/qt@5/bin:$PATH"
```

## 使用方法

### 基本命令

```bash
# 从文件解码
dream -i recording.iq -o audio.wav


# 使用特定音频设备
dream -i input.iq --sound-device pulse
```

## 文档

- **[README-BUILD.md](README-BUILD.md)** - 完整的构建和使用文档
- **[SourceForge 论坛](https://sourceforge.net/p/drm/discussion/general/thread/01c6e64c3b/)** - xHE-AAC 修复讨论

## 集成到 OpenWebRX

Dream 2.2.x 作为独立解码器进程,可通过管道集成:

```python
import subprocess

dream = subprocess.Popen(
    ['dream', '-i', '-', '-o', '/dev/null'],
    stdin=subprocess.PIPE,
    stdout=subprocess.PIPE
)

# 传送 SDR 数据
dream.stdin.write(iq_data)
```

## 技术细节

**xHE-AAC (Extended High Efficiency AAC)**
- 也称为 USAC (Unified Speech and Audio Coding)
- DRM30/DRM+ 标准使用的现代音频编码
- 提供更高的压缩效率和音质

**Console 模式**
- `CONFIG+=console` 编译选项
- 不依赖 Qt GUI 库
- 使用 `src/main.cpp` 作为入口点
- 支持管道输入/输出

## 许可证

GPL v2 - 详见 LICENSE 文件

## 致谢

xHE-AAC 修复贡献者:
- John (KiwiSDR) - 原始补丁
- Tarmo Tanilsoo - Linux 验证
- Rafael Diniz - 集成测试
- Julian Cable - Dream 维护
