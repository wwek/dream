# OpenWebRX 集成指南

集成 Dream DRM 解码器状态显示到 OpenWebRX。

## 当前 DRM 集成架构

OpenWebRX 已经集成了 Dream 解码器：

```python
# csdr/module/drm.py
class DrmModule(ExecModule):
    def __init__(self):
        super().__init__(
            Format.COMPLEX_SHORT,
            Format.SHORT,
            ["dream", "-c", "6", "--sigsrate", "48000", "--audsrate", "48000", "-I", "-", "-O", "-"]
        )

# csdr/chain/drm.py
class Drm(BaseDemodulatorChain, FixedIfSampleRateChain, FixedAudioRateChain):
    def __init__(self):
        workers = [
            Convert(Format.COMPLEX_FLOAT, Format.COMPLEX_SHORT),
            DrmModule(),  # Dream 进程通过 stdin/stdout
            Downmix(Format.SHORT),
        ]
```

**关键点**：
- Dream 通过 stdin 接收 IQ 数据，通过 stdout 输出音频
- Dream 进程已经在运行，我们只需添加状态监控
- 不需要单独启动 Dream 进程

## 新增状态监控架构（多用户支持）

```
OpenWebRX Process (多用户)
  ├─ User 1 DRM Chain
  │   ├─ DrmModule → dream --status-socket /tmp/drm_user1.sock
  │   └─ DrmStatusMonitor → 连接 /tmp/drm_user1.sock
  │
  ├─ User 2 DRM Chain
  │   ├─ DrmModule → dream --status-socket /tmp/drm_user2.sock
  │   └─ DrmStatusMonitor → 连接 /tmp/drm_user2.sock
  │
  └─ Web Clients
      ├─ User 1 WebSocket → 接收 User 1 状态
      └─ User 2 WebSocket → 接收 User 2 状态
```

**关键点**：
- 每个用户/连接有独立的 DRM chain 实例
- 每个 Dream 进程使用唯一的 socket 路径
- 每个监控器连接到对应的 socket

## 实现步骤

### 步骤 1：修改 DrmModule 支持多实例

**文件**: `csdr/module/drm.py`

```python
from pycsdr.modules import ExecModule
from pycsdr.types import Format
import uuid


class DrmModule(ExecModule):
    def __init__(self):
        # 为每个实例生成唯一的 socket 路径
        self.instance_id = str(uuid.uuid4())[:8]
        self.socket_path = f"/tmp/dream_status_{self.instance_id}.sock"

        super().__init__(
            Format.COMPLEX_SHORT,
            Format.SHORT,
            [
                "dream", "-c", "6",
                "--sigsrate", "48000",
                "--audsrate", "48000",
                "-I", "-", "-O", "-",
                "--status-socket", self.socket_path
            ]
        )

    def getSocketPath(self):
        """返回此实例的 socket 路径"""
        return self.socket_path
```

### 步骤 2：创建状态监控器

**文件**: `owrx/drm.py`
```python
import socket
import json
import threading
import time
import logging

logger = logging.getLogger(__name__)

class DrmMonitor(threading.Thread):
    """监控 Dream 状态并发布到 WebSocket 客户端"""

    def __init__(self, socket_path="/tmp/dream_status.sock"):
        super().__init__(daemon=True, name="DrmMonitor")
        self.socket_path = socket_path
        self.running = False
        self.callbacks = []

    def add_callback(self, callback):
        """添加状态更新回调函数"""
        self.callbacks.append(callback)

    def run(self):
        """监控线程主循环"""
        self.running = True

        while self.running:
            try:
                # 连接到 Dream socket
                sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
                sock.settimeout(5.0)
                sock.connect(self.socket_path)
                logger.info(f"Connected to DRM status socket: {self.socket_path}")

                # 接收数据循环
                buffer = ""
                while self.running:
                    try:
                        data = sock.recv(4096).decode('utf-8')
                        if not data:
                            logger.warning("DRM socket closed")
                            break

                        buffer += data
                        # 处理完整的 JSON 行
                        while '\n' in buffer:
                            line, buffer = buffer.split('\n', 1)
                            if line:
                                self._process_status(line)

                    except socket.timeout:
                        continue
                    except Exception as e:
                        logger.error(f"Error reading DRM status: {e}")
                        break

            except FileNotFoundError:
                logger.debug(f"DRM socket not found: {self.socket_path}")
                time.sleep(2)
            except Exception as e:
                logger.error(f"DRM monitor error: {e}")
                time.sleep(2)
            finally:
                if sock:
                    sock.close()

    def _process_status(self, json_str):
        """处理 DRM 状态 JSON"""
        try:
            status = json.loads(json_str)
            for callback in self.callbacks:
                callback(status)
        except json.JSONDecodeError as e:
            logger.error(f"Invalid DRM status JSON: {e}")

    def stop(self):
        """停止监控线程"""
        self.running = False
```

### 步骤 3：在 DRM chain 中启动监控器

**文件**: `csdr/chain/drm.py`

```python
from csdr.chain.demodulator import BaseDemodulatorChain, FixedIfSampleRateChain, FixedAudioRateChain
from pycsdr.modules import Convert, Downmix
from pycsdr.types import Format
from csdr.module.drm import DrmModule
from owrx.drm import DrmStatusMonitor


class Drm(BaseDemodulatorChain, FixedIfSampleRateChain, FixedAudioRateChain):
    def __init__(self):
        # 创建 DRM 模块实例
        self.drm_module = DrmModule()

        workers = [
            Convert(Format.COMPLEX_FLOAT, Format.COMPLEX_SHORT),
            self.drm_module,  # 每个实例有唯一的 socket
            Downmix(Format.SHORT),
        ]
        super().__init__(workers)

        # 启动状态监控器，连接到此实例的 socket
        self.monitor = DrmStatusMonitor(self.drm_module.getSocketPath())
        self.monitor.start()

    def stop(self):
        if self.monitor:
            self.monitor.stop()
        super().stop()

    def supportsSquelch(self) -> bool:
        return False

    def getFixedIfSampleRate(self) -> int:
        return 48000

    def getFixedAudioRate(self) -> int:
        return 48000
```

### 步骤 4：连接到 WebSocket

需要在 DRM chain 或 DSP 层面将状态发送到 WebSocket 客户端。参考其他数字模式的实现方式。

**关键点**：
- 每个用户的 DRM chain 独立运行
- 每个 chain 有自己的 socket 和监控器
- 状态只发送给对应的 WebSocket 客户端

## JavaScript 实现

### htdocs/lib/DrmPanel.js
```javascript
class DrmPanel {
    constructor(elementId) {
        this.element = document.getElementById(elementId);
        this.status = null;
    }

    onMessage(message) {
        if (message.type === 'drm_status') {
            this.update(message.value);
        }
    }

    update(status) {
        this.status = status;
        this.render();
    }

    render() {
        if (!this.status) return;

        const s = this.status;
        let html = '';

        // 状态指示灯
        html += '<div class="drm-indicators">';
        html += this.indicator(s.status.io, 'IO');
        html += this.indicator(s.status.time, 'Time');
        html += this.indicator(s.status.frame, 'Frame');
        html += this.indicator(s.status.fac, 'FAC');
        html += this.indicator(s.status.sdc, 'SDC');
        html += this.indicator(s.status.msc, 'MSC');
        html += '</div>';

        // 信号质量
        html += '<div class="drm-signal">';
        html += `<div>IF: ${s.signal.if_level_db.toFixed(1)} dB</div>`;
        html += `<div>SNR: ${s.signal.snr_db.toFixed(1)} dB</div>`;
        html += '</div>';

        // 模式信息（仅在有信号时）
        if (s.mode) {
            const modeStr = ['A', 'B', 'C', 'D'][s.mode.robustness];
            const interleaverStr = ['Long', 'Short'][s.mode.interleaver];
            html += '<div class="drm-mode">';
            html += `<div>Mode: ${modeStr}</div>`;
            html += `<div>BW: ${s.mode.bandwidth_khz} kHz</div>`;
            html += `<div>ILV: ${interleaverStr}</div>`;
            html += '</div>';
        }

        // 调制信息
        if (s.coding) {
            const qamMap = ['4-QAM', '16-QAM', '64-QAM'];
            html += '<div class="drm-coding">';
            html += `<div>SDC: ${qamMap[s.coding.sdc_qam]}</div>`;
            html += `<div>MSC: ${qamMap[s.coding.msc_qam]}</div>`;
            html += `<div>Prot: ${s.coding.protection_a}/${s.coding.protection_b}</div>`;
            html += '</div>';
        }

        // 服务列表
        if (s.service_list && s.service_list.length > 0) {
            html += '<div class="drm-services">';
            s.service_list.forEach(svc => {
                const codec = svc.is_audio ?
                    ['AAC', 'CELP', 'HVXC', 'xHE-AAC'][svc.audio_coding] : 'Data';
                html += '<div class="drm-service">';
                html += `<strong>${svc.label}</strong> (${svc.id})`;
                html += `<br>${codec} @ ${svc.bitrate_kbps.toFixed(2)} kbps`;
                if (svc.text) html += `<br>${svc.text}`;
                html += '</div>';
            });
            html += '</div>';
        }

        this.element.innerHTML = html;
    }

    indicator(value, label) {
        const cls = value === 0 ? 'ok' : value === -1 ? 'off' : 'err';
        return `<span class="indicator ${cls}">${label}</span>`;
    }
}

// 初始化
const drmPanel = new DrmPanel('drm-panel');

// 在 WebSocket 消息处理中添加
ws.addEventListener('message', (event) => {
    const message = JSON.parse(event.data);
    drmPanel.onMessage(message);
});
```

## CSS 样式

### htdocs/css/drm.css
```css
.drm-indicators {
    display: flex;
    gap: 5px;
    margin-bottom: 10px;
    flex-wrap: wrap;
}

.indicator {
    padding: 3px 8px;
    border-radius: 3px;
    font-size: 11px;
    font-weight: bold;
}

.indicator.ok {
    background: #4CAF50;
    color: white;
}

.indicator.off {
    background: #9E9E9E;
    color: white;
}

.indicator.err {
    background: #F44336;
    color: white;
}

.drm-signal,
.drm-mode,
.drm-coding {
    margin-bottom: 8px;
    font-size: 12px;
}

.drm-signal div,
.drm-mode div,
.drm-coding div {
    padding: 2px 0;
}

.drm-services {
    margin-top: 10px;
}

.drm-service {
    padding: 5px;
    margin-bottom: 5px;
    background: #f5f5f5;
    border-radius: 3px;
    font-size: 11px;
}

.drm-service strong {
    color: #333;
}
```

## HTML 集成

在 `index.html` 中添加：

```html
<div id="drm-panel" class="panel">
    <h3>DRM Status</h3>
    <div id="drm-content"></div>
</div>

<script src="lib/DrmPanel.js"></script>
<link rel="stylesheet" href="css/drm.css">
```

## 集成检查清单

### 后端
- [ ] 修改 `csdr/module/drm.py` - 支持多实例（唯一 socket 路径）
- [ ] 创建 `owrx/drm.py` - DrmStatusMonitor 类
- [ ] 修改 `csdr/chain/drm.py` - 集成监控器
- [ ] 连接到 WebSocket 系统 - 将状态发送到客户端

### 前端
- [ ] 创建 `htdocs/lib/DrmPanel.js` - 面板组件
- [ ] 创建 `htdocs/css/drm.css` - 样式
- [ ] 在 `index.html` 中添加 DRM 面板
- [ ] 连接 WebSocket 消息处理

### 测试
- [ ] 单客户端接收状态
- [ ] **多客户端同时使用 DRM** - 验证每个用户独立 socket
- [ ] Dream 进程重启处理
- [ ] Socket 连接失败处理
- [ ] Socket 文件清理（进程退出时）

## 关键实现要点

### 1. 多用户架构
- **OpenWebRX 是多用户系统** - 多个用户可同时使用 DRM
- 每个用户有独立的 DRM chain 实例
- 每个 Dream 进程必须使用唯一的 socket 路径
- 使用 UUID 生成唯一标识符

### 2. Socket 路径管理
- **格式**: `/tmp/dream_status_{instance_id}.sock`
- **生成**: 每个 `DrmModule` 实例创建时生成唯一 ID
- **清理**: Dream 进程退出时自动删除 socket 文件
- **传递**: `DrmModule` 通过 `getSocketPath()` 暴露路径

### 3. 状态监控独立
- 每个 DRM chain 有独立的 `DrmStatusMonitor`
- 监控器连接到对应实例的 socket
- 不影响音频处理链
- 自动重连和错误处理

### 4. WebSocket 集成
- 每个监控器只发送到对应用户的 WebSocket
- 不同用户的状态完全隔离
- 参考其他数字模式（APRS, WSJT）的实现
- 使用统一的消息格式

## 参考实现

- **API 文档**: `API.md`
- **Dream 源码**: `src/util/StatusBroadcast.cpp`
- **类似实现**: `owrx/aprs/__init__.py`, `owrx/wsjt.py`
- **Chain 实现**: `csdr/chain/digiham.py`, `csdr/chain/m17.py`
