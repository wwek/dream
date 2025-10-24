# Dream DRM OpenWebRX 集成文档索引

Dream DRM 2.2.x 完整集成到 OpenWebRX 的技术文档。

## 📚 文档列表

### 用户文档

1. **PATCH_INSTALL.md** - 补丁安装指南（⭐ 从这里开始）
   - 传统 `patch` 命令格式
   - 适用于 Linux 系统
   - 包含完整的安装步骤和故障排除

2. **FRONTEND_INTEGRATION.md** - 前端集成详细指南
   - HTML 面板容器配置
   - JavaScript 加载方式
   - WebSocket 消息处理机制
   - 调试和测试方法

### 技术文档

3. **API.md** - Unix Domain Socket API 参考
   - JSON 消息格式规范
   - 字段说明和示例
   - Python 客户端示例

4. **INTEGRATION.md** - OpenWebRX 集成架构
   - 多用户支持设计
   - MetaProvider 接口实现
   - 状态传输机制

5. **OPENWEBRX_INTEGRATION_COMPLETE.md** - 完整集成文档
   - 已完成的工作清单
   - 待完成的步骤
   - 调试和故障排除
   - 性能和资源使用

6. **KIWISDR_ANALYSIS.md** - KiwiSDR 实现分析
   - KiwiSDR DRM 实现研究
   - UI 设计参考
   - 最佳实践

## 🚀 快速开始

### 最简步骤（5 分钟）

```bash
# 1. 应用补丁
cd /path/to/openwebrx
patch -p1 < extensions/dream-qgx-2.2/docs/openwebrx-drm-core.patch

# 2. 复制新文件
cp extensions/dream-qgx-2.2/owrx/drm.py owrx/drm.py
cp extensions/dream-qgx-2.2/htdocs/lib/DrmPanel.js htdocs/lib/DrmPanel.js

# 3. 编辑 index.html 添加面板（详见 PATCH_INSTALL.md）
# 4. 重启 OpenWebRX
python3 openwebrx.py
```

### 详细步骤

请按顺序阅读：

1. **PATCH_INSTALL.md** - 后端和核心组件安装
2. **FRONTEND_INTEGRATION.md** - 前端界面集成
3. **OPENWEBRX_INTEGRATION_COMPLETE.md** - 验证和测试

## 📦 补丁文件

### openwebrx-drm-core.patch

核心后端补丁（使用 `patch -p1` 应用）：

- **csdr/module/drm.py** - 添加 UUID-based socket 支持
- **csdr/chain/drm.py** - 实现 MetaProvider 接口

### 新增文件

需要手动复制的文件：

- **owrx/drm.py** - DRM 状态监控线程（103 行）
- **htdocs/lib/DrmPanel.js** - 前端状态面板（252 行，包含内联 CSS）

### 前端修改

需要手动编辑的文件：

- **htdocs/index.html** - 添加面板容器和 JavaScript 引用

## 🔑 核心特性

### ✅ 已实现

- **多用户支持** - UUID-based socket 隔离，每个用户独立 Dream 实例
- **实时状态显示** - 信号质量、模式信息、服务列表
- **自动化集成** - MetaProvider 接口，自动注册和消息处理
- **xHE-AAC 支持** - Dream 2.2.x with FDK-AAC 2.0.2
- **Docker 构建** - 完整的 Docker 镜像构建支持

### ⚠️ 待完成

- HTML 模板集成（需手动修改 `index.html`）
- 模式切换显示控制（可选）
- 前端编译配置（可选，可直接引入 JS）

## 🛠️ 技术架构

### 数据流

```
Dream 进程 (--status-socket)
    ↓ Unix Domain Socket
DrmStatusMonitor 线程 (owrx/drm.py)
    ↓ 回调机制
Drm Chain (csdr/chain/drm.py)
    ↓ MetaProvider 接口
MetaWriter (pickle 序列化)
    ↓ WebSocket
前端 openwebrx.js
    ↓ MetaPanel 系统
DrmPanel.update() (htdocs/lib/DrmPanel.js)
    ↓ DOM 渲染
用户界面
```

### 多用户隔离

每个用户连接：
- 独立的 Dream 进程
- 唯一的 Socket 路径：`/tmp/dream_status_{uuid}.sock`
- 独立的 DrmStatusMonitor 线程
- 自动清理机制

## 📋 文件清单

### 修改的文件（3 个）
- `csdr/module/drm.py` - 添加 18 行（UUID socket）
- `csdr/chain/drm.py` - 添加 37 行（MetaProvider 实现）
- `owrx/controllers/assets.py` - 添加 1 行（DrmPanel.js 到编译列表）

### 新增的文件（2 个）
- `owrx/drm.py` - 103 行（状态监控器）
- `htdocs/lib/DrmPanel.js` - 252 行（前端面板，包含 CSS）

### 需手动修改（1 个）
- `htdocs/index.html` - 添加 3 行（面板容器）

### Docker 相关（2 个）
- `docker/Dockerfiles/Dockerfile-base` - 添加 Dream 构建步骤
- `docker/scripts/install-dependencies.sh` - 注释旧 Dream，添加依赖

## 🔗 相关链接

- **OpenWebRX**: https://github.com/jketterl/openwebrx
- **Dream DRM**: https://sourceforge.net/projects/drm/
- **FDK-AAC**: https://github.com/mstorsjo/fdk-aac
- **KiwiSDR**: http://kiwisdr.com/

## 📝 版本信息

- **Dream 版本**: 2.2.x (with xHE-AAC support)
- **FDK-AAC 版本**: 2.0.2
- **OpenWebRX 兼容**: 最新版本 (2024+)
- **文档创建**: 2025-10-24

## 💡 贡献

欢迎提交问题和改进建议！

## 📄 许可

本集成遵循 OpenWebRX 和 Dream DRM 的原始许可证。
