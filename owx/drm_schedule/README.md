# DRM Schedule Plugin for OpenWebRX

全球DRM短波广播时间表插件

轻量级 | 主题自适应 | 移动端友好

---

## 这是什么？

一个显示全球DRM数字短波广播时间表的插件，可以：
- 查看当前正在播出的节目（绿色显示）
- 点击节目直接调谐到对应频率
- 按电台/时间/频率三种方式查看

## 安装

```bash
# 1. 解压并复制到插件目录
cp -r drm_schedule /usr/lib/python3/dist-packages/htdocs/plugins/receiver

# 2. 加载插件（编辑 init.js）
Plugins.load('drm_schedule');

# 3. 刷新OpenWebRX页面
```

## 怎么用

### 打开时间表
- 切换到DRM模式
- 点击下方 DRM Schedule 按钮

### 切换查看方式
面板底部三个按钮：
- **By Service** - 按电台分组（相同电台合并显示）
- **By Time** - 按播出时间排序
- **By Frequency** - 按频率从低到高排序

### 调谐到电台
点击任意绿色或粉色时间块 → 自动调谐到该频率并切换到DRM模式

### 快捷操作
- **ESC键** - 关闭面板
- **刷新按钮** - 重新加载最新数据

## 常见问题

**插件没反应？**
1. F12打开控制台查看错误
3. 检查文件路径是否正确

**看不到数据？**
- 数据来自 `https://drm.kiwisdr.com`
- 如果远程加载失败，会使用本地备份

**点击调谐没反应？**
- 确认当前模式支持调谐(打开owx的频率模式  settings > general > Allow users to change center frequency ☑️)

**移动端显示不正常？**
- 面板支持横向滚动查看完整24小时
- 可以用手指滑动查看

## 数据源

- 主数据源: [KiwiSDR DRM](http://drm.kiwisdr.com)
- 本地备份: `data/drmrx.cjson`

## 参考

- 参考插件: [doppler](https://0xaf.github.io/openwebrxplus-plugins/receiver/doppler)