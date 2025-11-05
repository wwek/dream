# DRM Panel Plugin

![v1.4](https://img.shields.io/badge/v-1.4-blue) ![AGPL-3.0](https://img.shields.io/badge/license-AGPL--3.0-green)

Dream DRM 风格的状态显示面板插件。

---

## 快速开始

编辑 `htdocs/plugins/receiver/init.js`，添加:
```javascript
Plugins.load('drm_panel');
```

刷新浏览器 (`Ctrl+F5`)，切换到 DRM 模式。

**覆盖官方面板**：如果 OpenWebRX+ 官方已实现 DRM 面板，本插件会自动检测并覆盖官方实现，使用增强版的 Dream DRM 风格面板。

---

## 布局预览

```
[🟢IO] [🟢Time] [🟢Frame] [🟢FAC] [🟢SDC] [🟢MSC]
IF Level: -22.1 dB          SNR: 9.0 dB
DRM mode [🟢B]  Chan [4.5][5][9][🟢10][18][20] kHz  ILV [L][🟢S]
SDC [🟢4][16] QAM    MSC [🟢16][64] QAM
Protect: A=0 B=0     Services: A=🟢1 D=0
Services:
1  [🟢A]  ID:3E9  CNR-1  Chinese (Mandarin)  News  xHE-AAC 11.64 kbps  [EEP FAC:3 PTY:1]  [T↓]
   └─ 📄 上海艺术节上演精彩剧目。赛。
```

---

## 功能

- 6 个状态指示灯（绿/红/灰）
- 信号质量（SNR, IF Level）
- DRM 模式徽章（A/B/C/D）
- 信道带宽选择器（4.5-20 kHz）
- 交织器（Long/Short）
- 调制方式（4/16/64-QAM）
- 服务统计（音频/数据）
- **服务文本内容显示** ([T] 按钮展开/收起)
- 完全响应式

---

## 数据映射

### 真实ws收到的数据
```

{"type": "metadata", "value": {"type": "drm_status", "value": {"timestamp": 1761533471, "status": {"io": 0, "time": 0, "frame": 0, "fac": 0, "sdc": 0, "msc": 0}, "signal": {"if_level_db": -13.5, "snr_db": 12.8}, "mode": {"robustness": 0, "bandwidth": 3, "bandwidth_khz": 10.0, "interleaver": 0}, "coding": {"sdc_qam": 0, "msc_qam": 1, "protection_a": 0, "protection_b": 0}, "services": {"audio": 1, "data": 0}, "service_list": [{"id": "1", "label": "CNR-1", "is_audio": true, "audio_coding": 3, "bitrate_kbps": 12.0}]}}}


{"type": "metadata", "value": {"type": "drm_status", "value": {"timestamp": 1761535865, "status": {"io": 0, "time": 0, "frame": 0, "fac": 0, "sdc": 0, "msc": 0}, "signal": {"if_level_db": -30.0, "snr_db": 13.9}, "mode": {"robustness": 0, "bandwidth": 3, "bandwidth_khz": 10.0, "interleaver": 0}, "coding": {"sdc_qam": 0, "msc_qam": 1, "protection_a": 0, "protection_b": 0}, "services": {"audio": 1, "data": 0}, "service_list": [{"id": "3E9", "label": "CNR-1", "is_audio": true, "audio_coding": 3, "bitrate_kbps": 11.6, "language": {"fac_id": 3, "name": "Chinese (Mandarin)"}, "program_type": {"id": 1, "name": "News"}}]}}}

{"type": "metadata", "value": {"type": "drm_status", "value": {"timestamp": 1761560992, "status": {"io": 0, "time": 0, "frame": 0, "fac": 0, "sdc": 0, "msc": 0}, "signal": {"if_level_db": -29.6, "snr_db": 16.3}, "mode": {"robustness": 0, "bandwidth": 2, "bandwidth_khz": 9.0, "interleaver": 0}, "coding": {"sdc_qam": 1, "msc_qam": 2, "protection_a": 0, "protection_b": 0}, "services": {"audio": 1, "data": 1}, "service_list": [{"id": "111111", "label": "DRM Service A", "is_audio": true, "audio_coding": 3, "bitrate_kbps": 10.08, "text": "\u4e0a\u6d77\u827a\u672f\u8282\u4e0a\u6f14\u7cbe\u5f69\u5267\u76ee\u3002\u8d5b\u3002"}, {"id": "222222", "label": "DRM Service B", "is_audio": false, "bitrate_kbps": 9.6}]}}}


{"type": "metadata", "value": {"type": "drm_status", "value": {"timestamp": 1761640323, "status": {"io": 0, "time": 0, "frame": 0, "fac": 0, "sdc": 0, "msc": 0}, "signal": {"if_level_db": -22.9, "snr_db": 14.0, "wmer_db": 13.8, "doppler_hz": 0.6, "delay_min_ms": 3.9, "delay_max_ms": 6.56}, "mode": {"robustness": 1, "bandwidth": 3, "bandwidth_khz": 10.0, "interleaver": 0}, "coding": {"sdc_qam": 0, "msc_qam": 1, "protection_a": 0, "protection_b": 0}, "services": {"audio": 1, "data": 0}, "service_list": [{"id": "1", "label": "CNR-1", "is_audio": true, "audio_coding": 3, "bitrate_kbps": 11.64, "language": {"fac_id": 3, "name": "Chinese (Mandarin)"}, "program_type": {"id": 1, "name": "News"}}]}}}



---
owx官方元数据
{"type": "metadata", "value": {"timestamp": 1762315276, "drm_time": {"valid": true, "year": 2025, "month": 11, "day": 5, "hour": 12, "min": 4, "timestamp": 1762344240, "has_local_offset": true, "offset_min": 0}, "status": {"io": 0, "time": 0, "frame": 0, "fac": 0, "sdc": 0, "msc": 1}, "signal": {"if_level_db": -29.1, "snr_db": 10.1, "wmer_db": 10.3, "mer_db": 5.2, "doppler_hz": 0.74, "delay_min_ms": 1.44, "delay_max_ms": 1.95}, "frequency": {"dc_offset_hz": 5000.1, "sample_offset_hz": -0.69, "sample_offset_ppm": -14}, "mode": "DRM", "coding": {"sdc_qam": 0, "msc_qam": 1, "protection_a": 0, "protection_b": 0}, "services": {"audio": 1, "data": 0}, "service_list": [{"id": "1", "label": "CNR-1", "is_audio": true, "audio_coding": 3, "bitrate_kbps": 11.64, "audio_mode": "Mono", "protection_mode": "EEP", "language": {"fac_id": 3, "name": "Chinese (Mandarin)"}, "program_type": {"id": 1, "name": "News"}}], "media": {"program_guide": false, "journaline": false, "slideshow": false}, "drm_mode": {"robustness": 1, "bandwidth": 3, "bandwidth_khz": 10.0, "interleaver": 0}}}

{"type":"metadata","value":{"timestamp":1762318307,"drm_time":{"valid":true,"year":2025,"month":11,"day":5,"hour":12,"min":54,"timestamp":1762347240,"has_local_offset":true,"offset_min":0},"status":{"io":0,"time":0,"frame":0,"fac":0,"sdc":0,"msc":0},"signal":{"if_level_db":-31,"snr_db":10.8,"wmer_db":11.6,"mer_db":10.8,"doppler_hz":1.55,"delay_min_ms":1.23,"delay_max_ms":2.67},"frequency":{"dc_offset_hz":5001.93,"sample_offset_hz":-1.34,"sample_offset_ppm":-28},"mode":"DRM","coding":{"sdc_qam":0,"msc_qam":1,"protection_a":0,"protection_b":0},"services":{"audio":1,"data":0},"service_list":[{"id":"1","label":"CNR-1","is_audio":true,"audio_coding":3,"bitrate_kbps":11.64,"audio_mode":"Mono","protection_mode":"EEP","language":{"fac_id":3,"name":"Chinese (Mandarin)"},"program_type":{"id":1,"name":"News"}}],"media":{"program_guide":false,"journaline":false,"slideshow":false},"drm_mode":{"robustness":1,"bandwidth":3,"bandwidth_khz":10,"interleaver":0}}}
```

### 状态值
- `0` → 🟢 绿色（正常）
- `-1` → ⚪ 灰色（未激活）
- `>0` → 🔴 红色（错误）

### 模式映射
- **Robustness**: `0=A, 1=B, 2=C, 3=D`
- **Bandwidth**: `0=4.5, 1=5, 2=9, 3=10, 4=18, 5=20 kHz`
- **Interleaver**: `0=Short, 1=Long`
- **QAM**: `0=4-QAM, 1=16-QAM, 2=64-QAM`
- **Audio**: `0=AAC, 1=OPUS, 2=RESERVED, 3=xHE-AAC`

---

## 测试

打开 `test.html` 查看演示效果。

---

## Changelog

**v1.4.1** (2025-11-05)
- 🔄 **增强数据格式兼容性**
  - 支持 owx 官方新元数据结构（无 `type: "drm_status"` 嵌套层级）
  - 统一处理嵌套结构，自动适配新老数据格式
  - 优先使用新的 `drm_mode` 字段，兼容老的 `mode` 字段
  - 新增 `frequency` 对象支持（DC 偏移、采样偏移等参数）
  - 信号参数字段统一从 `value.signal` 访问，移除不必要的兜底逻辑
  - 状态字段统一从 `value.status` 访问
  - 服务列表字段统一使用 `value.service_list`
  - 统计字段使用 `value.services.audio` 和 `value.services.data`
  - 保护级别字段：统一使用 `coding.protection_a` 和 `coding.protection_b`
- 📱 **面板显示优化**
  - 检测并覆盖官方 `id="openwebrx-panel-metadata-drm"` 面板
  - 即使没有数据也显示插件面板（显示默认状态）
  - 移除过早返回逻辑，确保面板始终可见
  - **修复面板隐藏问题**：移除 `clear()` 中的 `hide()` 调用
  - 在构造函数中确保面板初始显示状态
  - **修复外层容器隐藏问题**：移除注入HTML时的 `style="display: none;"`
  - 覆盖官方面板时强制显示：移除隐藏类，强制设置 `display: block !important`
  - 强制添加CSS类：`openwebrx-panel openwebrx-meta-panel`
  - **修复145-190行注入逻辑**：有官方面板时清空后不创建，没官方面板时创建，现在统一为：有官方面板时清空+设置样式属性，没官方面板时创建+设置
  - **关键修复 - metaPanel() 不调用构造函数**：添加双重保障机制：如果 metaPanel() 成功但HTML为空，自动手动调用构造函数；确保HTML注入成功
  - **关键修复 - HTML注入到错误容器**：修复构造函数中使用 `this.el` 而未定义的错误，改用传入的 `el` 参数；确保HTML正确注入到面板容器
  - **增强调试日志**：构造函数ID验证、HTML注入验证、HTML长度验证、update()参数验证、字段解析验证、DOM选择器验证（find元素数量）

**v1.4** (2025-11-03)
- 🔄 **官方面板覆盖功能**
  - 自动检测 OpenWebRX+ 官方 DRM 面板实现
  - 如果官方面板存在，清空并覆盖为插件实现
  - 如果不存在，保持原有注入逻辑
  - 销毁旧的面板实例，避免冲突
  - 使用构造函数注册，完全兼容官方 `MetaPanel.types.drm`
  - 数据格式兼容：同时支持 `status.mode` 和官方 `status.drm_mode`

**v1.3.1** (2025-10-27)
- 🐛 **修复移动端布局问题**
  - 修复信号质量行 (IF Level, SNR) 在移动端不必要的换行
  - 修复时间行 (UTC, Local) 在移动端不必要的换行
  - 添加 `flex-wrap: nowrap` 确保横向布局在有空间时保持单行显示
  - 优化移动端 gap 间距为 10px,保持紧凑但可读

**v1.3** (2025-10-27)
- 📄 **新增服务文本内容显示功能**
  - 在服务列表中添加 `[T]` 按钮,用于显示服务的 `text` 字段内容
  - 点击 `[T]` 按钮可展开/收起文本内容
  - 支持悬停提示 (hover title)
  - 文本内容使用缩进布局,保持视觉层次清晰
  - 使用 📄 图标标识文本内容
  - 支持 Unicode 文本解码,正确显示中文等多语言内容
  - 响应式设计,适配移动端显示
  - 平滑动画效果 (slideUp/slideDown)

**v1.2** (2025-10-27)
- 🎨 **Services 行 UI/UX 全面优化**
  - 重新设计信息顺序：`序号 [类型] Service-ID 电台 语言 节目类型 编码 码率 [技术参数]`
  - 建立清晰的视觉层级系统（5级字体大小：13px/12px/11px/10px）
  - 优化配色方案（方案A：语义化分层配色）
    - 电台标识：`#FFFF50` 亮黄色（第一视觉焦点）
    - 节目类型：`#FFD700` 金黄色（第二视觉焦点）
    - 音频技术：`#4CAF50` 成功绿（编码+码率）
    - 语言信息：`#FFFF50` 亮黄色 + 85% 透明度（辅助信息）
    - 技术参数：`#9E9E9E` 中性灰 + 75% 透明度（最低层级）
  - 添加服务类型徽章系统
    - `[A]` 绿色徽章 = Audio 音频服务
    - `[D]` 红色徽章 = Data 数据服务
  - 去除冗余的 "Audio" 文字标识
  - 技术参数用方括号包裹：`[EEP FAC:3 PTY:1]`
  - 去除 FAC 重复显示（只在技术参数组显示）
  - 优化信息分组间距（8px 基础间距 + 4px/8px 分组间距）
  - 添加微交互：hover 效果（背景变亮 + 右移 2px）
- 🟢 **动态数值高亮功能**
  - `Protect: A=/B=` 非零值显示为绿色
  - `Services: A=/D=` 非零值显示为绿色
  - 动态切换：0=灰色，>0=绿色高亮
- 📝 **代码优化**
  - 新增 `updateValueWithHighlight()` 方法
  - 优化注释：Service ID 明确标注（非国家代码）
  - 字体层级系统：weight 400/500/600/700
  - 间距系统：gap 8px + margin 4px/8px

**v1.1** (2025-10-27)
- 修复交织器(ILV)显示问题,正确映射 L/S 标签
- 修复 QAM badge 在 clear() 时被覆盖为 `--` 的问题
- 添加 Unicode 解码支持,正确显示中文服务标签
- 优化 badge 尺寸,减小横向宽度 15-20%
- 调整元素间距,整体布局更紧凑
- SDC 仅显示 4/16-QAM, MSC 仅显示 16/64-QAM

**v1.0** (2025-10-26)
- Dream DRM 风格横向布局
- 徽章系统（16+ 可视化状态）
- 服务统计（音频/数据）
- 信息密度提升 60%

**v1.0** (2024-10-26)
- 首次发布

---

**License**: [AGPL-3.0](https://www.gnu.org/licenses/agpl-3.0.html)
