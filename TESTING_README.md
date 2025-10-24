# Dream 2.2.x Docker 转码测试套件

本目录包含了用于测试 Dream 2.2.x 在 Docker 环境中 DRM 转码功能的完整测试套件。

## 文件说明

### 测试脚本
- **`run_tests.sh`** - 主测试运行脚本
- **`test_file_mode.sh`** - 文件I/O模式转码测试
- **`test_pipe_mode.sh`** - 管道模式转码测试

### 测试数据
- **`test_data/2019-01-21_19-44-38_05.rec`** - DRM测试文件 (424KB)

### 输出目录
- **`output/`** - 所有测试结果文件保存目录

## 快速开始

### 1. 环境验证
```bash
cd /path/to/dream.x
./run_tests.sh verify
```

### 2. 运行所有测试
```bash
./run_tests.sh
```

### 3. 单独运行测试

#### 文件模式测试
```bash
./run_tests.sh file
# 或者
./test_file_mode.sh
```

#### 管道模式测试
```bash
./run_tests.sh pipe
# 或者
./test_pipe_mode.sh
```

### 4. 查看帮助
```bash
./run_tests.sh help
```

## 测试功能

### 文件模式测试 (`test_file_mode.sh`)

**测试内容**：
- ✅ DRM文件输入处理
- ✅ 文件I/O模式 (`-f` 参数)
- ✅ WAV文件输出 (`-w` 参数)
- ✅ FDK-AAC库初始化
- ✅ USAC位流检测
- ✅ 音频转码验证

**输出文件**：
- `output/file_test_YYYYMMDD_HHMMSS.wav` - 解码后的音频文件
- `output/file_test_report_YYYYMMDD_HHMMSS.txt` - 测试报告
- `/tmp/dream_file_test_YYYYMMDD_HHMMSS.log` - 详细日志

### 管道模式测试 (`test_pipe_mode.sh`)

**测试内容**：
- ✅ 管道数据输入处理
- ✅ 标准输入/输出模式
- ✅ 文件I/O + 管道组合
- ✅ OpenWebRX兼容性测试
- ✅ 实时性能测试

**输出文件**：
- `output/pipe_test_YYYYMMDD_HHMMSS.wav` - 管道模式解码结果
- `output/openwebrx_test_YYYYMMDD_HHMMSS.wav` - OpenWebRX兼容测试
- `output/perf_test_YYYYMMDD_HHMMSS.wav` - 性能测试结果
- `output/pipe_test_report_YYYYMMDD_HHMMSS.txt` - 综合测试报告
- `/tmp/dream_pipe_test_YYYYMMDD_HHMMSS.log` - 详细日志

## Docker环境要求

### 必需组件
1. **Docker** - 容器运行环境
2. **Docker镜像** - `dream-drm:stdin-stdout-fix` 或最新构建版本
3. **测试数据** - `test_data/2019-01-21_19-44-38_05.rec`

### 推荐配置
- **内存**: 最少 512MB，推荐 1GB+
- **磁盘**: 最少 100MB 可用空间
- **权限**: 脚本执行权限 (`chmod +x *.sh`)

## 测试参数

### DRM解码配置
- **通道模式**: `-c 6` (I/Q input positive, 0 Hz IF)
- **信号采样率**: `--sigsrate 48000`
- **音频采样率**: `--audsrate 48000`
- **静音模式**: `-m 1` (避免音频系统错误)
- **文件I/O**: `-f /dev/null` (禁用声卡)
- **WAV输出**: `-w /path/to/output.wav`

### OpenWebRX兼容命令
```bash
["dream", "-c", "6", "--sigsrate", "48000", "--audsrate", "48000",
 "-f", "/dev/null", "-O", "-"]
```

## 故障排除

### 常见问题

#### 1. Docker镜像不存在
```bash
# 解决方案：构建镜像
docker build -t dream-drm:stdin-stdout-fix .
```

#### 2. 脚本权限不足
```bash
# 解决方案：添加执行权限
chmod +x *.sh
```

#### 3. 输出文件未生成
- **检查**：输入文件是否存在
- **检查**：Docker容器是否正常启动
- **检查**：磁盘空间是否足够
- **查看**：日志文件中的详细错误信息

#### 4. 音频系统错误 (正常现象)
在Docker环境中，以下错误是正常的：
```
pa_c_sync failed, error -1
pa_init pa_context_connect failed
CSoundOutPulse::Init_HW pa_init failed
CSoundOutPulse::Write(): write_HW error
```

**解决方案**：这些错误不影响转码功能，因为使用文件I/O模式。

### 日志分析

#### 查看详细日志
```bash
# 文件模式日志
cat /tmp/dream_file_test_*.log

# 管道模式日志
cat /tmp/dream_pipe_test_*.log
```

#### 关键日志信息
- ✅ `Got FAAD2 library` - FAAD库加载成功
- ✅ `NOT conforming USAC bit stream!` - 检测到USAC流
- ✅ `init: iResOutBlockSize=3840` - 初始化成功
- ⚠️ `write_HW error` - 音频系统错误（可忽略）

## 预期结果

### 成功指标
- **输入处理**: 424KB DRM文件 → 8MB+ WAV文件
- **音频格式**: 16-bit立体声WAV，48kHz采样率
- **处理时间**: 通常 30-60秒，取决于文件大小
- **文件I/O**: 完全绕过音频硬件，适合Docker环境

### 集成测试结果

### 与OpenWebRX集成
测试脚本生成的命令格式可直接用于OpenWebRX集成：

```python
# 在OpenWebRX配置中的推荐调用
["dream", "-c", "6", "--sigsrate", "48000", "--audsrate", "48000",
 "-m", "1", "-f", "/dev/null", "-w", "/path/to/output.wav"]
```

### 性能基准
- **单次转码**: 424KB → 8.5MB (~78秒音频)
- **实时处理**: 支持持续数据流处理
- **资源使用**: 最小内存占用，无音频硬件依赖
- **并发能力**: Docker容器隔离，可运行多实例

## 技术细节

### Docker容器配置
- **基础镜像**: Debian Bullseye Slim
- **依赖库**: FDK-AAC v2.0.1, FFTW, Speex
- **二进制**: `/usr/local/bin/dream`
- **工作目录**: `/data`

### 测试文件格式
- **格式**: DRM自定义格式 (.rec)
- **大小**: 424KB
- **内容**: DRM编码的音频数据和元数据
- **识别**: 包含DRM头部标识符 ("DMDI", "afpf"等)

---

**注意**: 这些测试专为Docker环境优化，使用文件I/O模式完全绕过音频系统限制，确保在各种容器化环境中稳定运行。