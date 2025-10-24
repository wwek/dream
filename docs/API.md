# DRM Status API 参考

Dream DRM 解码器状态 Unix Domain Socket API。

## 快速开始

### 启动 Dream
```bash
./dream --mode receive --input-file input.iq --status-socket /tmp/drm_status.sock
```

### 读取状态
```bash
# 原始 JSON
socat - UNIX-CONNECT:/tmp/drm_status.sock

# 格式化输出
socat - UNIX-CONNECT:/tmp/drm_status.sock | jq .
```

## JSON 格式

### 完整示例
```json
{
  "timestamp": 1761276899,
  "status": {"io":0, "time":0, "frame":0, "fac":0, "sdc":0, "msc":0},
  "signal": {"if_level_db":-34.7, "snr_db":10.6},
  "mode": {"robustness":0, "bandwidth":3, "bandwidth_khz":10.0, "interleaver":0},
  "coding": {"sdc_qam":0, "msc_qam":1, "protection_a":0, "protection_b":0},
  "services": {"audio":1, "data":1},
  "service_list": [
    {
      "id":"3E9",
      "label":"CNR-1",
      "is_audio":true,
      "audio_coding":3,
      "bitrate_kbps":11.60,
      "text":"Now playing..."
    },
    {
      "id":"5F",
      "label":"CNR-1",
      "is_audio":false,
      "bitrate_kbps":2.00
    }
  ]
}
```

**示例解读**:
- **音频服务** (id: 3E9): CNR-1 节目，使用 xHE-AAC (编码 3) @ 11.60 kbps
- **数据服务** (id: 5F): CNR-1 数据服务 @ 2.00 kbps (可能是 EPG 或其他数据)

## 字段说明

### status - 状态指示
值含义: `-1`=未同步, `0`=正常, `1`=CRC错误, `2`=数据错误

| 字段 | 说明 |
|------|------|
| io | I/O 状态 |
| time | 时间同步 |
| frame | 帧同步 |
| fac | FAC 通道 |
| sdc | SDC 通道 |
| msc | MSC 通道 |

### signal - 信号质量

| 字段 | 说明 | 单位 |
|------|------|------|
| if_level_db | IF 信号电平 | dB |
| snr_db | 信噪比 | dB |

### mode - DRM 模式参数 *（仅有信号时）*

| 字段 | 说明 | 值范围 |
|------|------|--------|
| robustness | 鲁棒性模式 | 0=A, 1=B, 2=C, 3=D |
| bandwidth | 带宽索引 | 0-5 |
| bandwidth_khz | 实际带宽 | 4.5/5/9/10/18/20 kHz |
| interleaver | 交织器模式 | 0=Long, 1=Short |

### coding - 调制和保护 *（仅有信号时）*

| 字段 | 说明 | 值含义 |
|------|------|--------|
| sdc_qam | SDC QAM 模式 | 0=4-QAM, 1=16-QAM |
| msc_qam | MSC QAM 模式 | 0=4-QAM, 1=16-QAM, 2=64-QAM |
| protection_a | 保护等级 A | 0-3 (0=最强, 3=最弱) |
| protection_b | 保护等级 B | 0-3 |

### services - 服务统计 *（仅有信号时）*

| 字段 | 说明 |
|------|------|
| audio | 音频服务数量 |
| data | 数据服务数量 |

### service_list - 服务详情数组 *（仅有信号时）*

| 字段 | 类型 | 说明 | 可选 |
|------|------|------|------|
| id | string | 服务 ID (十六进制，如 "3EA") | 必需 |
| label | string | 服务名称（广播电台设置的节目名） | 必需 |
| is_audio | boolean | 音频/数据标志 | 必需 |
| audio_coding | int | 音频编码类型 | 音频服务 |
| bitrate_kbps | float | 比特率 | 必需 |
| text | string | 文本消息（正在播放的内容） | 可选 |

**服务 ID 和名称**:
- **id**: DRM 服务唯一标识符（Service ID），由广播机构分配，十六进制格式
  - 示例: `"3EA"`, `"5F"`, `"1A2B"`
- **label**: 服务标签（Service Label），由广播机构在 FAC/SDC 中传输的节目名称
  - 示例: `"CNR-1"` (中央人民广播电台第一套节目)
  - 示例: `"CNR-1 EEP Audio"` (带传输模式描述)
  - 最大长度: 16 字节（支持 UTF-8 编码）
- **text**: 文本消息（Text Message），可选的动态文本信息
  - 示例: `"Now playing: Classical Music"`
  - 用于显示当前播放内容、歌曲信息等
  - 最大长度: 128 字节（8 段，每段 16 字节）

**音频编码类型**:
- `0` = AAC (MPEG-4 AAC)
- `1` = OPUS (DRM+ 音频编码)
- `2` = RESERVED (保留)
- `3` = xHE-AAC (Extended HE-AAC / USAC，DRM+ 增强音频)

**注意**: xHE-AAC (值 3) 是 DRM+ 标准的主要音频编码格式，提供比传统 AAC 更好的音质和更低的比特率。需要 FDK-AAC 2.0+ 库支持。

## Python 示例

```python
import socket, json

sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
sock.connect('/tmp/drm_status.sock')

data = sock.recv(4096).decode('utf-8')
status = json.loads(data)

print(f"SNR: {status['signal']['snr_db']} dB")
print(f"Mode: {['A','B','C','D'][status['mode']['robustness']]}")

for svc in status['service_list']:
    print(f"{svc['id']}: {svc['label']} @ {svc['bitrate_kbps']} kbps")
```

## 技术细节

### 更新频率
- 500ms 间隔（每秒 2 次）
- 支持多客户端
- 非阻塞 I/O

### Socket 文件
- **默认路径**: `/tmp/dream_status.sock`
- **自定义路径**: `--status-socket /path/to/socket`
- **自动清理**: 启动时清理过期文件，退出时删除

### 数据大小
- 无信号: ~150 字节
- 有信号 (1 服务): ~400-500 字节
- 有信号 (4 服务): ~800-1000 字节
- 带宽消耗: 0.8-2 KB/秒

## 实现的功能

✅ 状态指示灯 (IO, Time, Frame, FAC, SDC, MSC)
✅ 信号质量 (IF Level, SNR)
✅ DRM 模式 (Robustness, Bandwidth, Interleaver)
✅ 调制方案 (SDC/MSC QAM, Protection)
✅ 服务信息 (ID, Label, Codec, Bitrate, Text)

## 无法实现的功能

❌ EPG (需要 EPG 解码器模块)
❌ Journaline (需要专门解码器)
❌ MOT Slideshow (需要 MOT 解码器)

## 故障排除

### Socket 文件不存在
```bash
ls -la /tmp/*.sock
ps aux | grep dream
```

### 连接被拒绝
- 确保使用 `--mode receive` (非 transmit)
- 检查编译时已启用 `USE_CONSOLEIO`

### 没有数据
- 确认 Dream 正在处理输入数据
- 查看 Dream 的标准错误输出

## 版本历史

- **Phase 1 (MVP)**: 基础状态、信号质量、模式信息
- **Phase 2 (Complete)**: 完整 DRM 参数、服务详情、文本消息 ✅ 当前版本
- **Phase 3 (Future)**: 数据服务解码 (需要额外模块)
