# OpenWebRX DRM 前端集成指南

详细说明如何将 DRM 面板集成到 OpenWebRX 前端。

## 📦 前端组件说明

### DrmPanel.js 工作原理

DrmPanel.js 已经包含了自动注册机制（第 246-251 行）：

```javascript
// 注册到 MetaPanel 系统（如果存在）
if (typeof MetaPanel !== 'undefined') {
    MetaPanel.types = MetaPanel.types || {};
    MetaPanel.types.drm = function(el) {
        return new DrmPanel(el);
    };
}
```

这意味着 DrmPanel 会自动注册到 MetaPanel 系统，无需手动配置！

## 🔌 集成步骤

### 步骤 1: 添加 HTML 面板容器

编辑 `htdocs/index.html`，在数字模式面板区域添加 DRM 面板：

```html
<!-- 在其他 meta panel 附近添加 -->
<div id="openwebrx-panel-metadata-drm" class="openwebrx-meta-panel openwebrx-panel" style="display: none;">
    <div class="openwebrx-panel-inner"></div>
</div>
```

**位置参考**：查找类似的面板（如 `openwebrx-panel-metadata-dmr`），在其附近添加。

### 步骤 2: 加载 DrmPanel.js

**重要**：DrmPanel.js 需要被加载到浏览器中。OpenWebRX 的加载方式取决于版本：

**方法 A: 检查是否有编译系统**

查找 `htdocs/compiled/receiver.js` 文件：
- **如果存在**：需要将 DrmPanel.js 添加到编译配置
- **如果不存在**：使用方法 B 直接引入

**方法 B: 直接加载**（推荐，最简单）

在 `htdocs/index.html` 的 `<head>` 部分，`compiled/receiver.js` 之后添加：

```html
<script src="compiled/receiver.js"></script>
<script src="lib/MetaPanel.js"></script>  <!-- 如果未包含 -->
<script src="lib/DrmPanel.js"></script>   <!-- 添加这行 -->
```

**注意**：DrmPanel.js 必须在 MetaPanel.js 之后加载！

### 步骤 3: WebSocket 消息处理（已自动处理）

**好消息**：OpenWebRX 已有通用的 metadata 处理机制！

在 `htdocs/openwebrx.js` 第 837-840 行：

```javascript
case "metadata":
    $('.openwebrx-meta-panel').metaPanel().each(function(){
        this.update(json['value']);
    });
    break;
```

这意味着：
1. 所有 class 包含 `openwebrx-meta-panel` 的元素会被自动处理
2. 当后端发送 `type: "metadata"` 的消息时，会调用对应面板的 `update()` 方法
3. **DRM 面板会自动接收和更新状态数据**

## 📊 数据流

### 后端 → 前端数据流

```
Dream 进程
   ↓ (写入 Unix Socket)
DrmStatusMonitor 线程
   ↓ (解析 JSON)
Drm Chain (_on_drm_status)
   ↓ (pickle 序列化)
MetaWriter
   ↓ (WebSocket)
前端 openwebrx.js (case "metadata")
   ↓ (调用 metaPanel().update())
DrmPanel.update(status)
   ↓ (渲染 HTML)
用户界面
```

### 消息格式

**后端发送**（在 csdr/chain/drm.py 中）：
```python
msg = {"type": "drm_status", "value": status}
self.metawriter.write(pickle.dumps(msg))
```

**注意**：后端发送的是 `"drm_status"`，但 OpenWebRX 的 WebSocket 处理会将其转换为 `"metadata"` 消息！

### 前端接收（自动处理）

```javascript
// openwebrx.js 自动处理
case "metadata":
    $('.openwebrx-meta-panel').metaPanel().each(function(){
        this.update(json['value']);  // 调用 DrmPanel.update()
    });
    break;
```

## 🎨 面板显示控制

### 显示/隐藏逻辑

DRM 面板应该在以下情况显示：
- 用户选择 DRM 解调模式时

参考其他数字模式的实现（如 DMR），在模式切换代码中添加：

```javascript
// 在模式切换函数中
if (mode === 'drm') {
    $('#openwebrx-panel-metadata-drm').show();
} else {
    $('#openwebrx-panel-metadata-drm').hide();
}
```

## 🔧 测试和调试

### 1. 检查 DrmPanel 是否已注册

打开浏览器控制台：

```javascript
// 检查 MetaPanel.types 是否包含 drm
console.log(MetaPanel.types.drm);
// 应该输出：function(el) { return new DrmPanel(el); }
```

### 2. 检查面板是否初始化

```javascript
// 检查面板元素
$('#openwebrx-panel-metadata-drm').data('metapanel');
// 应该输出：DrmPanel 实例
```

### 3. 手动测试面板

```javascript
// 获取面板实例
var drmPanel = $('#openwebrx-panel-metadata-drm').metaPanel()[0];

// 手动更新数据
var testData = {
    "timestamp": 1761276899,
    "status": {"io": 0, "time": 0, "frame": 0, "fac": 0, "sdc": 0, "msc": 0},
    "signal": {"if_level_db": -34.7, "snr_db": 10.6},
    "mode": {"robustness": 0, "bandwidth": 3, "bandwidth_khz": 10.0, "interleaver": 0},
    "coding": {"sdc_qam": 0, "msc_qam": 1, "protection_a": 0, "protection_b": 0},
    "services": {"audio": 1, "data": 1},
    "service_list": [
        {
            "id": "3E9",
            "label": "CNR-1",
            "is_audio": true,
            "audio_coding": 3,
            "bitrate_kbps": 11.60,
            "text": "Now playing..."
        }
    ]
};

drmPanel.update(testData);
```

### 4. 监控 WebSocket 消息

```javascript
// 拦截 WebSocket 消息（调试用）
var originalOnMessage = ws.onmessage;
ws.onmessage = function(evt) {
    var msg = JSON.parse(evt.data);
    if (msg.type === 'drm_status' || msg.type === 'metadata') {
        console.log('DRM/Metadata message:', msg);
    }
    originalOnMessage.call(this, evt);
};
```

## ⚠️ 常见问题

### Q1: DrmPanel.js 加载了但面板不显示

**原因**：面板容器的 `display: none` 样式

**解决**：
1. 检查 CSS 是否隐藏了面板
2. 确保模式切换时正确显示面板

```javascript
// 在 DRM 模式激活时
$('#openwebrx-panel-metadata-drm').show();
```

### Q2: 面板显示但没有数据

**原因**：WebSocket 消息未正确发送或处理

**解决**：
1. 检查后端日志是否有 "DRM chain initialized" 消息
2. 检查 socket 文件是否创建：`ls -la /tmp/dream_status_*.sock`
3. 查看浏览器控制台是否有 JavaScript 错误

### Q3: 数据更新延迟

**原因**：Dream 进程状态更新间隔

**解决**：Dream 每秒更新一次状态（1000ms 间隔），这是正常的

### Q4: 多用户冲突

**原因**：Socket 路径冲突或共享

**解决**：检查 csdr/module/drm.py 是否正确使用 UUID：

```python
self.instance_id = str(uuid.uuid4())[:8]
self.socket_path = f"/tmp/dream_status_{self.instance_id}.sock"
```

## 📝 完整集成检查清单

- [ ] ✅ 应用后端补丁（openwebrx-drm-core.patch）
- [ ] ✅ 复制 owrx/drm.py
- [ ] ✅ 复制 htdocs/lib/DrmPanel.js
- [ ] ⚠️ 在 index.html 添加面板容器
- [ ] ⚠️ 加载 DrmPanel.js（编译或直接引入）
- [ ] ⚠️ 添加模式切换时的显示/隐藏逻辑
- [ ] ⚠️ 测试单用户 DRM 解调
- [ ] ⚠️ 测试多用户并发
- [ ] ⚠️ 验证面板自动更新

## 🎯 最小化集成方案

如果只想快速测试，最小步骤：

### 1. 应用补丁
```bash
patch -p1 < extensions/dream-qgx-2.2/docs/openwebrx-drm-core.patch
cp extensions/dream-qgx-2.2/owrx/drm.py owrx/drm.py
cp extensions/dream-qgx-2.2/htdocs/lib/DrmPanel.js htdocs/lib/DrmPanel.js
```

### 2. 修改 index.html（一次性）

在 `htdocs/index.html` 中：

**A. 添加面板容器**（查找类似 `openwebrx-panel-metadata-dmr` 的位置）：
```html
<div id="openwebrx-panel-metadata-drm" class="openwebrx-meta-panel openwebrx-panel">
    <div class="openwebrx-panel-inner"></div>
</div>
```

**B. 加载脚本**（在 `<head>` 部分或编译配置中）：
```html
<script src="lib/DrmPanel.js"></script>
```

### 3. 重启 OpenWebRX

```bash
python3 openwebrx.py
```

### 4. 测试

1. 打开浏览器访问 OpenWebRX
2. 选择 DRM 模式
3. 调整频率到 DRM 广播（如 CNR-1: 6040 kHz）
4. 检查浏览器控制台是否有错误
5. 查看 DRM 面板是否显示状态

## 🔗 相关文件

- **MetaPanel.js** - 元数据面板基类和注册系统
- **openwebrx.js** - WebSocket 消息处理和面板更新（第 837-840 行）
- **index.html** - 主页面模板（需要添加面板容器）
- **DrmPanel.js** - DRM 状态显示面板（已包含自动注册）

## 📚 参考

- OpenWebRX 其他数字模式面板实现：
  - `DmrMetaPanel` - DMR 面板（第 93-119 行）
  - `M17MetaPanel` - M17 面板（第 324-360 行）
  - `DabMetaPanel` - DAB 面板（第 553-626 行）

所有这些面板都使用相同的 MetaPanel 系统，DRM 面板完全兼容！
