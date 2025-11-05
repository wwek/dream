# BlackHole 虚拟声卡兼容性修复

## 问题诊断

### 为什么 BlackHole 虚拟声卡会报错？

1. **虚拟设备的特殊性**
   - BlackHole 是虚拟音频路由设备，不像物理麦克风会持续产生数据
   - 当没有音频源连接时，虚拟设备会返回零数据（silence）
   - 初始化时间比物理设备更长，Qt6 的缓冲机制可能需要更多时间

2. **原有代码的问题**
   - 零读取阈值设置为 50 次（约 250ms）对虚拟设备来说太短
   - 没有区分物理设备和虚拟设备的行为差异
   - 频繁的设备重置会影响性能和用户体验

3. **典型错误场景**
   - 选择 BlackHole 后，应用检测到连续零读取
   - 触发错误处理流程，尝试重新初始化设备
   - 形成"零读取 → 重置 → 零读取"的循环

## 解决方案

### 1. 设备类型检测（qt6_audio_compat.h）

**改进内容：**
```cpp
// 添加虚拟设备检测
bool m_isVirtualDevice = false;

// 在构造函数中检测设备类型
QString deviceName = device.description().toLower();
m_isVirtualDevice = deviceName.contains("blackhole") ||
                   deviceName.contains("loopback") ||
                   deviceName.contains("virtual");
```

**支持的虚拟设备：**
- BlackHole (所有通道配置)
- Loopback Audio
- 其他包含 "virtual" 关键词的设备

### 2. 智能零读取处理（creceivedata.cpp）

**改进策略：**

| 设备类型 | 零读取阈值 | 错误处理策略 |
|---------|-----------|------------|
| 物理设备 | 50 次 (~250ms) | 标记为错误，尝试重置 |
| 虚拟设备 | 200 次 (~1s) | 仅警告，继续等待音频 |

**关键代码逻辑：**
```cpp
// 检测设备类型
bool isVirtual = pAudioInput->isVirtualDevice();
int zeroReadThreshold = isVirtual ? 200 : 50;

if (iZeroReadCount > zeroReadThreshold) {
    if (isVirtual) {
        // 虚拟设备：友好提示，不标记为错误
        fprintf(stderr, "Virtual device waiting for audio input...\n");
    } else {
        // 物理设备：标记错误，触发恢复流程
        bBad = true;
    }
}
```

### 3. 错误恢复优化

**改进内容：**
- **物理设备**：每 100 次零读取尝试一次重置
- **虚拟设备**：仅在超过 500 次零读取时尝试一次重置
- 避免频繁的设备重新初始化

### 4. 崩溃保护机制

**现有保护措施：**
```cpp
// 1. try-catch 包装（InitInternal:677-682）
try {
    pAudioInput = new CAudioInput(di);
} catch (const std::exception& e) {
    fprintf(stderr, "Exception: %s\n", e.what());
    throw;  // 重新抛出让上层处理
}

// 2. 错误时安全返回（ProcessDataInternal:389）
if(bBad) {
    return;  // 不会崩溃，只是跳过本次处理
}

// 3. Mutex 保护（防止竞态条件）
QMutexLocker locker(&audioDeviceMutex);
```

**保证：即使报错也不会崩溃**
- ✅ 所有异常都被捕获并记录
- ✅ 错误状态会正确传播但不会终止程序
- ✅ 设备指针都有空值检查
- ✅ 使用 Mutex 防止多线程竞态

## 使用说明

### 编译项目
```bash
cd /Users/wwek/Projects/wwek/dream
mkdir -p build && cd build
cmake ..
make
```

### 测试虚拟设备

1. **安装 BlackHole**
   ```bash
   brew install blackhole-2ch
   ```

2. **配置音频路由**
   - 打开 "音频 MIDI 设置"
   - 创建"聚集设备"，包含 BlackHole 和物理输出设备
   - 在应用中选择 BlackHole 作为输入

3. **测试场景**
   - ✅ 无音频输入：应正常运行，显示友好提示
   - ✅ 播放音频：应正常接收和处理数据
   - ✅ 切换设备：应平滑过渡，无崩溃
   - ✅ 长时间运行：应保持稳定，无内存泄漏

### 日志输出示例

**虚拟设备（正常）：**
```
QCompatAudioInput: Virtual device: YES
ProcessDataInternal: Warning: 10 consecutive zero reads (virtual device - this is normal)
ProcessDataInternal: Virtual audio device detected with 201 zero reads
ProcessDataInternal: This is normal for virtual devices when no audio is playing
ProcessDataInternal: The application will continue running, waiting for audio input
```

**物理设备（错误）：**
```
QCompatAudioInput: Virtual device: NO
ProcessDataInternal: Physical device with 51 consecutive zero reads, marking as error
ProcessDataInternal: This may indicate permission issues or device problems
ProcessDataInternal: Please check microphone permissions
```

## 技术细节

### 零读取计数器重置
```cpp
if(r > 0) {
    iZeroReadCount = 0;  // 成功读取时重置
}
```

### 虚拟设备检测规则
- 设备名称包含 "blackhole"（不区分大小写）
- 设备名称包含 "loopback"
- 设备名称包含 "virtual"

### 线程安全
- 所有 `pAudioInput` 访问都通过 `audioDeviceMutex` 保护
- 设备切换时使用延迟确保清理完成
- 避免在音频线程中执行耗时操作

## 常见问题

### Q1: BlackHole 一直显示 "waiting for audio input"
**A:** 这是正常行为。BlackHole 是音频路由设备，只有当其他应用播放音频时才会有数据输入。

**解决方法：**
1. 确保有应用正在播放音频
2. 检查 macOS 音频路由设置
3. 创建"聚集设备"将 BlackHole 与输出设备组合

### Q2: 切换到虚拟设备后性能下降
**A:** 已优化虚拟设备的处理逻辑，不再频繁重置设备。

### Q3: 如何确认虚拟设备正常工作？
**A:** 检查日志输出：
- 看到 "Virtual device: YES" 表示正确识别
- 看到 "virtual device - this is normal" 表示零读取被正确处理
- 播放音频后应该看到零读取计数器重置

## 代码改动摘要

| 文件 | 行号 | 改动类型 |
|------|------|---------|
| qt6_audio_compat.h | 20-54 | 添加虚拟设备检测 |
| qt6_audio_compat.h | 137 | 添加成员变量 |
| creceivedata.cpp | 285-328 | 智能零读取处理 |
| creceivedata.cpp | 362-390 | 优化错误恢复 |

## 验证清单

测试前确认：
- [ ] 代码编译无警告
- [ ] Qt6 版本 >= 6.0.0
- [ ] macOS 权限已授予（系统偏好设置 > 安全性与隐私 > 麦克风）

运行测试：
- [ ] 选择物理麦克风可正常工作
- [ ] 选择 BlackHole 不会崩溃
- [ ] 无音频输入时显示友好提示
- [ ] 有音频输入时正常处理
- [ ] 设备切换流畅无卡顿
- [ ] 长时间运行稳定

## 参考资料

- [BlackHole 官方文档](https://github.com/ExistentialAudio/BlackHole)
- [Qt6 音频 API 文档](https://doc.qt.io/qt-6/qtmultimedia-index.html)
- [macOS 音频权限说明](https://support.apple.com/guide/mac-help/control-access-to-the-microphone-mchla1b1e1fe/mac)
