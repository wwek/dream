# DRM Status Patch

## 版本信息
- **补丁版本**: 1.0
- **创建日期**: 2025-10-26
- **适用版本**: OpenWebRX+ (带 DRM 支持的版本)

## 补丁内容
- 添加 DRM 状态信息到 WebSocket 消息中


## 安装方法

### 方法 1: 自动安装 (推荐)
```bash
# 1. 解压补丁
tar -xzf drm_status_patch.tar.gz
cd drm_status_patch

# 2. 运行安装脚本
./install.sh

# 或使用 -y 参数自动确认（无交互模式）
./install.sh -y

# 脚本会自动:
# - 检测 OpenWebRX 路径
# - 创建备份
# - 应用补丁
# - 验证安装
# - 重启服务

# 查看帮助信息
./install.sh -h
```

### 方法 2: 手动安装
手动替换以下文件：
- `csdr/chain/drm.py`
- `csdr/module/drm.py`
- `owrx/drm.py`

## 验证补丁

### 检查文件变更
```bash
# 检查是否包含修复
grep -n "threading.Lock" csdr/chain/drm.py        # 应该找到
grep -n "drm_module.stop" csdr/chain/drm.py       # 应该找到
grep -n "os.unlink" csdr/module/drm.py             # 应该找到
grep -n "buffer = b\"\"" owrx/drm.py               # 应该找到
```

### 运行时验证
1. 启动 OpenWebRX 并切换到 DRM 模式
2. 打开浏览器开发者工具查看 WebSocket 消息
3. 应该能看到 `{"type": "drm_status", "value": {...}}` 消息
4. 停止 DRM 后检查 `/tmp/` 目录，不应有残留 `dream_status_*.sock` 文件

## 卸载/回滚方法

如果出现问题，可以快速回滚：
```bash
cd drm_status_patch
./uninstall.sh

# 或使用 -y 参数自动确认（无交互模式，保留备份）
./uninstall.sh -y

# 脚本会自动:
# - 查找备份
# - 恢复原文件
# - 验证回滚
# - 重启服务

# 查看帮助信息
./uninstall.sh -h
```

## 技术细节

### 修改行数统计
- `csdr/chain/drm.py`: +5 行 (添加 import threading 和锁逻辑)
- `csdr/module/drm.py`: +14 行 (添加 socket 清理)
- `owrx/drm.py`: +4 行 (改进缓冲和异常处理)

### 依赖要求
- Python 3.6+
- threading 模块 (标准库)
- os 模块 (标准库)

### 兼容性
- ✅ 向后兼容：不破坏现有 API
- ✅ 多用户安全：每个实例独立 socket
- ✅ 线程安全：完全的并发保护

## 常见问题

**Q: 补丁是否影响性能？**
A: 否。锁的开销可忽略（<1μs），socket 清理只在启动/停止时执行。

**Q: 需要重新编译吗？**
A: 不需要。这是纯 Python 代码修改。

**Q: 是否会影响其他解调模式？**
A: 不会。修改仅影响 DRM 模式。

**Q: 可以用在其他 OpenWebRX 分支吗？**
A: 可以，但需要确认文件路径和代码结构一致。

## 支持

如有问题，请提供：
1. OpenWebRX 版本
2. Python 版本
3. 错误日志 (journalctl -u openwebrx -n 100)
4. 是否成功应用补丁

## 许可证

本补丁遵循 OpenWebRX 项目原有许可证。
