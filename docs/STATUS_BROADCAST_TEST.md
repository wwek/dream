# StatusBroadcast 测试指南

## 📋 概述

StatusBroadcast 通过 Unix Domain Socket 实时广播 DRM 接收器状态信息。
- **更新频率**: 500ms
- **数据格式**: JSON
- **Socket 路径**: `/tmp/dream_status_{PID}.sock`
- **多客户端**: 支持多个客户端同时连接

---

## 🚀 快速测试（3步）

### 步骤1: 启动 dream 播放测试文件

```bash
# 在终端1中运行
./dream -f "test_data/drm-bbc XHE-AAC drm iq.rec"
```

启动后会显示类似信息：
```
StatusBroadcast: Started on /tmp/dream_status_12345.sock
```

### 步骤2: 使用 Python 测试客户端

```bash
# 在终端2中运行（自动查找socket）
python3 test_status_broadcast.py

# 或者手动指定socket路径
python3 test_status_broadcast.py /tmp/dream_status_12345.sock
```

### 步骤3: 观察实时状态输出

你会看到每500ms更新一次的DRM状态：

```
================================================================================
⏰ 19:30:45.123
================================================================================

📡 SIGNAL QUALITY:
   SNR: 16.1 dB
   Doppler: 0.22 Hz
   Delay: 5.79 ms
   Sample Rate Offset: 0.0 Hz

🔵 STATUS LIGHTS:
   IO:        RX_OK
   Time Sync: RX_OK
   Freq Sync: RX_OK
   FAC:       RX_OK
   SDC:       RX_OK
   MSC:       RX_OK

📻 DRM MODE:
   Robustness: Mode B
   Spectrum: 3
   Bandwidth: 10.0 kHz
   Interleaver: Long

🎧 SERVICES:
   [0] CNR-1
       Audio Codec: xHE-AAC
       Bitrate: 12.0 kbps
       Language: Chinese (Mandarin)
```

---

## 🔧 手动测试方法

### 方法1: 使用 netcat (nc)

```bash
# 查找 socket 路径
ls -la /tmp/dream_status_*.sock

# 连接并查看原始 JSON
nc -U /tmp/dream_status_12345.sock
```

### 方法2: 使用 socat

```bash
# 安装 socat (如果没有)
brew install socat  # macOS

# 连接 socket
socat - UNIX-CONNECT:/tmp/dream_status_12345.sock
```

### 方法3: 使用 Python one-liner

```bash
# 查看原始 JSON 数据
python3 -c "
import socket
s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
s.connect('/tmp/dream_status_12345.sock')
while True:
    print(s.recv(4096).decode())
"
```

---

## 📊 JSON 数据格式示例

```json
{
  "timestamp": "2025-10-29T19:30:45.123",
  "snr": 16.1,
  "doppler": 0.22,
  "delay": 5.79,
  "sample_rate_offset": 0.0,
  "io_status": 3,
  "time_sync": 3,
  "freq_sync": 3,
  "fac_status": 3,
  "sdc_status": 3,
  "msc_status": 3,
  "robustness_mode": "B",
  "spectrum_occupancy": 3,
  "bandwidth": 10.0,
  "interleaver_depth": "Long",
  "services": [
    {
      "id": 1,
      "label": "CNR-1",
      "audio_codec": "xHE-AAC",
      "bitrate": 12.0,
      "language": "Chinese (Mandarin)",
      "country_code": "CHN"
    }
  ],
  "text_message": "测试文本消息"
}
```

### 状态值说明

| 值 | 状态 | 说明 |
|----|------|------|
| 0 | NOT_PRESENT | 未检测到信号 |
| 1 | CRC_ERROR | CRC校验错误 |
| 2 | DATA_ERROR | 数据错误 |
| 3 | RX_OK | 接收正常 |

---

## 🧪 测试场景

### 场景1: 正常播放测试

**目的**: 验证状态广播基本功能

```bash
# 终端1: 启动 dream
./dream -f "test_data/drm-bbc XHE-AAC drm iq.rec"

# 终端2: 运行测试客户端
python3 test_status_broadcast.py

# 预期结果:
# - 连接成功
# - 每500ms更新一次
# - MSC状态应该是 RX_OK (3)
# - 能看到服务信息和音频编解码器
```

### 场景2: 多客户端测试

**目的**: 验证多个客户端可以同时连接

```bash
# 终端1: dream
./dream -f "test_data/drm-bbc XHE-AAC drm iq.rec"

# 终端2: 客户端1
python3 test_status_broadcast.py

# 终端3: 客户端2
python3 test_status_broadcast.py

# 终端4: 客户端3 (使用 nc)
nc -U /tmp/dream_status_*.sock

# 预期结果:
# - 所有客户端都能接收到相同的数据
# - 断开一个客户端不影响其他客户端
```

### 场景3: 信号变化测试

**目的**: 观察状态变化时的广播行为

```bash
# 播放测试文件，观察状态变化
# 特别关注 MSC 状态的变化

# 预期行为:
# - 信号良好时: msc_status = 3 (RX_OK)
# - 解码失败时: msc_status = 1 或 2
# - 信号恢复后应该快速回到 3
```

### 场景4: 长时间运行测试

**目的**: 验证内存泄漏和稳定性

```bash
# 运行测试客户端并记录日志
python3 test_status_broadcast.py > status_log.txt 2>&1 &

# 让 dream 运行一段时间 (例如 30 分钟)
# 然后检查:
# - 是否有内存泄漏 (ps aux | grep dream)
# - Socket 连接是否稳定
# - 数据格式是否一致
```

---

## 🔍 故障排查

### 问题1: 找不到 socket 文件

```bash
# 检查 dream 是否在运行
ps aux | grep dream

# 检查 /tmp 目录权限
ls -la /tmp/dream_status_*.sock

# 检查 dream 输出是否有错误信息
./dream -f "test_data/drm-bbc XHE-AAC drm iq.rec" 2>&1 | grep -i status
```

### 问题2: 连接被拒绝

```bash
# 确认 socket 存在且有正确权限
ls -la /tmp/dream_status_*.sock

# 应该显示类似:
# srwxr-xr-x  1 user  wheel  0 Oct 29 19:30 /tmp/dream_status_12345.sock

# 检查 socket 是否可写
file /tmp/dream_status_*.sock
```

### 问题3: 收到损坏的数据

```bash
# 查看原始数据流
nc -U /tmp/dream_status_*.sock | hexdump -C

# 检查是否有完整的 JSON
nc -U /tmp/dream_status_*.sock | python3 -m json.tool
```

### 问题4: Windows 平台

StatusBroadcast **不支持 Windows**（Unix Domain Socket 仅限 Unix-like 系统）

如果在 Windows 上测试，会看到：
```
StatusBroadcast: Not implemented on Windows
```

**替代方案**: 在 WSL (Windows Subsystem for Linux) 中运行

---

## 📝 测试清单

- [ ] dream 启动成功，显示 socket 路径
- [ ] 能在 /tmp 找到 socket 文件
- [ ] Python 测试客户端能成功连接
- [ ] 每500ms收到一次更新
- [ ] JSON 格式正确（能被解析）
- [ ] 所有状态字段都存在
- [ ] 多客户端能同时连接
- [ ] 断开客户端不影响服务器
- [ ] 长时间运行稳定（无内存泄漏）
- [ ] MSC 状态能正确反映解码状态

---

## 🎯 验证之前的修复

使用 StatusBroadcast 可以实时验证之前的修复效果：

### 验证1: FAC 阈值提高（30帧）

**观察**: `fac_status` 字段

**预期**:
- 信号波动时 `fac_status` 可能短暂变为 1 或 2
- 但应该在 30 帧内恢复，而不是触发系统重启
- MSC 应该保持稳定

### 验证2: Reverb 状态重置

**观察**: `msc_status` 字段

**预期**:
- 系统重启后（如果发生），MSC 应该能快速恢复
- 不应该出现长时间卡在 CRC_ERROR 状态

### 验证3: 解码失败静音

**观察**: `msc_status` 字段 + 音频输出

**预期**:
- `msc_status = 1 或 2` 时，音频应该静音
- 不应该有爆音/沙沙声
- 状态恢复到 3 时，音频应该平滑恢复

---

## 💡 高级用法

### 记录状态到文件

```bash
# 记录原始 JSON
nc -U /tmp/dream_status_*.sock > drm_status_log.json

# 记录格式化输出
python3 test_status_broadcast.py > drm_status_formatted.txt
```

### 提取特定字段

```bash
# 只显示 MSC 状态
nc -U /tmp/dream_status_*.sock | \
  python3 -c "
import sys, json
for line in sys.stdin:
    try:
        data = json.loads(line)
        print(f'{data[\"timestamp\"]}: MSC={data[\"msc_status\"]}')
    except: pass
"
```

### 监控信号质量

```bash
# 实时监控 SNR 和 MSC
nc -U /tmp/dream_status_*.sock | \
  python3 -c "
import sys, json
for line in sys.stdin:
    try:
        d = json.loads(line)
        print(f'SNR: {d[\"snr\"]:5.1f} dB | MSC: {d[\"msc_status\"]} | Doppler: {d[\"doppler\"]:5.2f} Hz')
    except: pass
"
```

---

## 🎓 总结

StatusBroadcast 提供了一个强大的调试和监控工具：
- ✅ 实时查看 DRM 接收器内部状态
- ✅ 验证修复效果（FAC阈值、Reverb、解码静音）
- ✅ 支持多客户端同时监控
- ✅ JSON 格式便于解析和记录

**开始测试**:
```bash
# 一键启动
./dream -f "test_data/drm-bbc XHE-AAC drm iq.rec"

# 另一个终端
python3 test_status_broadcast.py
```
