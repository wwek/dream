# DRM Schedule Plugin for OpenWebRX

纯前端的 DRM 短波广播计划表插件，用于显示全球 DRM 数字广播时间表。

A pure frontend plugin for OpenWebRX that displays global DRM (Digital Radio Mondiale) shortwave broadcast schedules.

---

## 🎉 最新更新 / Latest Updates

**版本 / Version**: 1.1.0 (KiwiSDR-aligned, Fixed)
**更新日期 / Update Date**: 2025-11-13
**状态 / Status**: ✅ **修复完成 / Fixed**

### ✅ 已修复的问题 / Fixed Issues
1. **加载问题** - 插件现在可以正确加载和显示
2. **容器问题** - 创建独立容器,不依赖特定 DOM 元素
3. **时间轴渲染** - 时间刻度和当前时间线正确显示
4. **电台条目布局** - 所有元素(时间块、名称、图标)正确定位
5. **CSS 样式** - 完全对齐 KiwiSDR 的显示效果

### 📚 相关文档 / Documentation
- [demo.html](drm_schedule.md:demo.html) - 独立测试页面

---

## Features / 功能特性

- **三种显示模式** / Three View Modes:
  - 按服务分组 (By Service) - 按电台名称分组显示
  - 按时间排序 (By Time) - 按播出时间排序显示
  - 按频率排序 (By Frequency) - 按频率从低到高排序

- **实时状态** / Real-time Status:
  - 自动检测当前正在播出的节目（绿色高亮）
  - 每分钟自动刷新时间表状态

- **数据源** / Data Sources:
  - 优先从远程加载: `https://drm.kiwisdr.com/drm/stations2.cjson`
  - 本地备份: `data/stations.json`

- **点击调谐** / Click to Tune:
  - 点击任意电台即可自动调谐到对应频率

## Installation / 安装

### 1. 复制插件文件 / Copy Plugin Files

将整个 `owx/drm_schedule` 目录复制到 OpenWebRX 的插件目录:

Copy the entire `owx/drm_schedule` directory to OpenWebRX's plugin directory:

```bash
# 假设 OpenWebRX 安装在 /opt/openwebrx
# Assuming OpenWebRX is installed at /opt/openwebrx
cp -r owx/drm_schedule /opt/openwebrx/htdocs/static/plugins/receiver/
```

### 2. 加载插件 / Load Plugin

在 OpenWebRX 的插件配置中添加:

Add to OpenWebRX's plugin configuration:

```javascript
// 在 htdocs/plugins.js 或相应配置文件中
Plugins.load('receiver', 'drm_schedule');
```

或者在浏览器控制台手动加载（用于测试）:

Or load manually in browser console (for testing):

```javascript
Plugins.load('receiver', 'drm_schedule');
```

### 3. 刷新页面 / Refresh Page

刷新 OpenWebRX 页面，插件将自动初始化。

Refresh the OpenWebRX page, and the plugin will auto-initialize.

## Usage / 使用方法

### 打开计划表 / Open Schedule Panel

插件加载后会自动创建悬浮按钮（右下角），点击即可打开计划表面板。

A floating button will appear in the bottom-right corner after plugin loads. Click to open the schedule panel.

或者通过代码打开:

Or open via code:

```javascript
DRM_Schedule.togglePanel();
```

### 切换显示模式 / Switch View Modes

在面板顶部有三个按钮:

Three buttons at the top of the panel:

- **By Service** - 按服务分组显示
- **By Time** - 按时间排序显示
- **By Frequency** - 按频率排序显示

### 调谐到电台 / Tune to Station

点击任意时间块即可自动调谐到该频率。

Click any time block to automatically tune to that frequency.

## File Structure / 文件结构

```
owx/drm_schedule/
├── README.md              # 本文档 / This documentation
├── drm_schedule.js        # 主插件代码 / Main plugin code
├── drm_schedule.css       # 样式文件 / Stylesheet
└── data/
    └── stations.json      # 本地备份数据 / Local backup data
```

## Data Format / 数据格式

### stations.json 格式说明

```json
{
  "stations": [
    {
      "freq": 6180,                    // 频率 (kHz)
      "service": "China Radio International",  // 电台名称
      "target": "East Asia",           // 目标区域
      "system": "DRM30",               // DRM 系统类型
      "schedule": [
        {
          "days": "1111111",           // 周一到周日 (1=播出, 0=停播)
          "start": "0000",             // UTC 开始时间 (HHMM)
          "duration": 120              // 持续时长 (分钟)
        }
      ]
    }
  ],
  "metadata": {
    "version": "1.0",
    "last_updated": "2025-01-12T00:00:00Z",
    "source": "Example DRM Schedule Data"
  }
}
```

### Days Format / 日期格式

`days` 字段是 7 位字符串，代表周一到周日:

The `days` field is a 7-character string representing Mon-Sun:

- `1111111` - 每天播出 / Every day
- `1111110` - 周一到周六 / Monday to Saturday
- `0000001` - 仅周日 / Sunday only
- `0111110` - 周二到周六 / Tuesday to Saturday

### Time Format / 时间格式

所有时间均为 **UTC 时间**，格式为 `HHMM` (24小时制):

All times are in **UTC**, format `HHMM` (24-hour):

- `0000` - UTC 00:00 (午夜)
- `1200` - UTC 12:00 (中午)
- `2330` - UTC 23:30

## Configuration / 配置

可以在 `drm_schedule.js` 中修改配置:

You can modify configuration in `drm_schedule.js`:

```javascript
config: {
    // 远程数据源 URL
    remote_url: 'https://drm.kiwisdr.com/drm/stations2.cjson',

    // 本地备份文件路径
    local_backup: 'static/plugins/receiver/drm_schedule/data/stations.json',

    // 自动刷新间隔 (毫秒)
    refresh_interval: 60000,  // 60秒

    // 显示悬浮按钮
    show_float_button: true
}
```

## API Reference / API 参考

### 主要方法 / Main Methods

```javascript
// 初始化插件
DRM_Schedule.initialize();

// 切换面板显示/隐藏
DRM_Schedule.togglePanel();

// 加载电台数据
DRM_Schedule.loadStations();

// 切换显示模式
DRM_Schedule.switchView('service');  // 'service', 'time', 'frequency'

// 调谐到指定频率
DRM_Schedule.tuneToStation(6180);  // kHz
```

### 事件 / Events

```javascript
// 监听调谐事件
$(document).on('drm:tune', function(event, freqKhz) {
    console.log('Tuning to:', freqKhz, 'kHz');
});

// 监听数据加载完成
$(document).on('drm:loaded', function(event, data) {
    console.log('Stations loaded:', data.stations.length);
});
```

## Troubleshooting / 故障排除

### 插件未加载 / Plugin Not Loading

1. 检查文件路径是否正确
2. 检查浏览器控制台是否有错误信息
3. 确认 jQuery 已加载

### 数据无法加载 / Data Not Loading

1. 检查远程 URL 是否可访问: `https://drm.kiwisdr.com/drm/stations2.cjson`
2. 检查本地备份文件是否存在: `data/stations.json`
3. 查看浏览器控制台网络请求
4. **注意**: 远程数据是CJSON格式(带注释的JSON)，插件会自动移除注释

### Stations count: 0 问题

**症状**: 日志显示 `[DRM Schedule] Stations count: 0`

**原因**: 数据格式不匹配

**解决**:
- 本地JSON使用 `{stations: [...]}` 格式 (带schedule数组)
- KiwiSDR使用 `[{region, service: [freq, start, end, ...]}]` 格式
- 插件已支持两种格式自动识别

### CJSON解析错误 / CJSON Parse Error

**症状**: `SyntaxError: Unexpected token '/'`

**原因**: KiwiSDR数据包含JSON注释 (`//` 和 `/* */`)

**解决**: 插件已自动移除注释，无需手动处理

### 调谐功能不工作 / Tuning Not Working

1. 确认 OpenWebRX 的调谐 API 可用
2. 检查浏览器控制台是否有调谐错误
3. 尝试手动触发: `DRM_Schedule.tuneToStation(6180)`

### 测试数据格式 / Test Data Format

打开 `test_data_format.html` 查看数据解析测试结果。

## Technical Details / 技术细节

### 架构设计 / Architecture

- **纯前端实现**: 不需要修改 OpenWebRX 后端
- **插件化设计**: 使用 OpenWebRX 的插件系统
- **数据驱动**: 从 JSON 加载数据，易于更新

### 渲染算法 / Rendering Algorithm

时间轴渲染基于 KiwiSDR 的算法:

Timeline rendering based on KiwiSDR's algorithm:

```javascript
// UTC 小时转换为像素位置
function timeToPixel(utcHours) {
    var width = containerWidth;
    var margin = 30;
    return margin + (utcHours * (width - margin - 15) / 24);
}
```

### 活动状态检测 / Active Status Detection

```javascript
function isActive(schedule) {
    var now = new Date();
    var utcHours = now.getUTCHours();
    var utcMinutes = now.getUTCMinutes();
    var currentMinutes = utcHours * 60 + utcMinutes;
    var dayOfWeek = (now.getUTCDay() + 6) % 7; // 0=Monday

    // 检查是否在播出时间范围内
    return schedule.days[dayOfWeek] === '1' &&
           currentMinutes >= startMinutes &&
           currentMinutes < endMinutes;
}
```

## License / 许可证

与 Dream DRM 项目保持一致的开源许可证。

Same open source license as the Dream DRM project.

## Credits / 致谢

- 基于 KiwiSDR DRM 扩展的计划表实现
- 数据来源: [drm.kiwisdr.com](http://drm.kiwisdr.com)
- Dream DRM 项目: [sourceforge.net/projects/drm](https://sourceforge.net/projects/drm/)

## 本项目参考插件
https://0xaf.github.io/openwebrxplus-plugins/receiver/doppler
https://0xaf.github.io/openwebrxplus-plugins/receiver/doppler/doppler.js