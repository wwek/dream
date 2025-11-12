/**
 * DRM Schedule Plugin for OpenWebRX - KiwiSDR Aligned Version
 *
 * 版本: 1.1.0 (KiwiSDR-aligned)
 * 功能: 显示全球DRM短波广播时间表，完全对齐KiwiSDR显示
 */

// 设置插件版本
Plugins.drm_schedule = {
    _version: 1.1
};

// 插件初始化函数
Plugins.drm_schedule.init = function() {
    console.log('[DRM Schedule] Plugin initializing (KiwiSDR-aligned)...');

    // 检查依赖
    if (typeof $ === 'undefined') {
        console.error('[DRM Schedule] jQuery is required');
        return false;
    }

    // 等待OpenWebRX初始化完成
    $(document).on('event:owrx_initialized', function() {
        console.log('[DRM Schedule] OpenWebRX initialized, loading schedule...');
        DRM_Schedule.initialize();
    });

    // 如果已经初始化,直接加载
    if (typeof demodulatorPanel !== 'undefined') {
        DRM_Schedule.initialize();
    }

    return true;
};

/**
 * DRM Schedule Core Class - KiwiSDR Aligned
 */
var DRM_Schedule = {
    // 配置 (KiwiSDR数据源)
    config: {
        // 远程数据源 (KiwiSDR官方数据)
        remote_url: 'https://drm.kiwisdr.com/drm/drmrx.cjson',
        backup_url: 'https://drm.kiwisdr.com/drm/stations2.cjson',

        // 本地备份数据
        local_backup: 'static/plugins/receiver/drm_schedule/data/stations.json',

        // 缓存时间 (小时)
        cache_hours: 24,

        // 刷新间隔 (毫秒)
        refresh_interval: 60000, // 1分钟

        // 面板尺寸 (匹配KiwiSDR)
        panel_width: 675,
        panel_height: 300
    },

    // 状态
    stations: null,          // 电台数据
    displayMode: 'BY_SVC',   // 显示模式
    isPanelVisible: false,   // 面板是否可见
    refreshTimer: null,      // 刷新定时器
    currentSource: 'remote', // 当前数据源

    // 显示模式常量 (匹配KiwiSDR)
    DISPLAY_MODES: {
        BY_SVC: 'BY_SVC',
        BY_TIME: 'BY_TIME',
        BY_FREQ: 'BY_FREQ'
    },

    // 电台类型常量 (匹配KiwiSDR)
    STATION_TYPES: {
        SINGLE: 0,
        MULTI: 1,
        REGION: 2,
        SERVICE: 3
    },

    // ========== 初始化 ==========
    initialize: function() {
        console.log('[DRM Schedule] Initializing (KiwiSDR-aligned)...');

        this.createUI();
        this.loadStations();
        this.bindEvents();
        this.startAutoRefresh();

        console.log('[DRM Schedule] Initialized successfully (KiwiSDR-aligned)');
    },

    // ========== UI 创建 ==========
    createUI: function() {
        var self = this;

        // 创建面板HTML (匹配KiwiSDR结构)
        var panelHtml = `
            <div id="id-drm-panel-1-by-svc" class="cl-drm-sched" style="display:none;">
                <div id="id-drm-tscale"></div>
                <div id="id-drm-panel-by-svc" class="w3-scroll-y w3-absolute" style="width:100%; height:100%;">
                    <div class="drm-loading-msg">&nbsp;loading data from kiwisdr.com ...</div>
                </div>
            </div>

            <div class="drm-schedule-controls">
                <button class="drm-btn" data-mode="BY_SVC" onclick="DRM_Schedule.setDisplayMode('BY_SVC')">By Service</button>
                <button class="drm-btn" data-mode="BY_TIME" onclick="DRM_Schedule.setDisplayMode('BY_TIME')">By Time</button>
                <button class="drm-btn" data-mode="BY_FREQ" onclick="DRM_Schedule.setDisplayMode('BY_FREQ')">By Frequency</button>
            </div>
        `;

        // 插入到页面
        $('#openwebrx-panel-container-right').append(panelHtml);

        // 添加菜单项
        this.addMenuItem();

        console.log('[DRM Schedule] UI created (KiwiSDR structure)');
    },

    addMenuItem: function() {
        // 尝试在OpenWebRX菜单中添加入口
        var panelList = $('#openwebrx-panel-receiver ul, .openwebrx-panel-list');
        if (panelList.length > 0) {
            var menuItem = `
                <li>
                    <a href="#" class="drm-schedule-menu-item" onclick="DRM_Schedule.togglePanel(); return false;">
                        <i class="fa fa-calendar"></i> DRM Schedule
                    </a>
                </li>
            `;
            panelList.append(menuItem);
        }
    },

    // ========== 数据加载 ==========
    loadStations: function() {
        var self = this;

        this.showLoading();
        this.setStatus('Loading schedule data...');

        console.log('[DRM Schedule] Loading from remote:', this.config.remote_url);

        // 尝试从远程加载 (CJSON格式,可能包含注释)
        $.ajax({
            url: this.config.remote_url,
            dataType: 'text',  // 先作为文本加载
            timeout: 10000,
            cache: false,
            success: function(text) {
                console.log('[DRM Schedule] Remote data loaded, parsing CJSON...');
                try {
                    // 移除C风格注释 (// 和 /* */)
                    var cleanJson = self.stripComments(text);
                    var data = JSON.parse(cleanJson);

                    self.stations = self.parseStations(data);
                    self.onDataLoaded('remote');
                } catch(e) {
                    console.warn('[DRM Schedule] CJSON parse failed:', e);
                    self.loadBackupServer();
                }
            },
            error: function(xhr, status, error) {
                console.warn('[DRM Schedule] Remote load failed:', error);
                self.loadBackupServer();
            }
        });
    },

    loadBackupServer: function() {
        var self = this;
        console.log('[DRM Schedule] Trying backup server:', this.config.backup_url);

        $.ajax({
            url: this.config.backup_url,
            dataType: 'text',
            timeout: 10000,
            success: function(text) {
                try {
                    var cleanJson = self.stripComments(text);
                    var data = JSON.parse(cleanJson);

                    self.stations = self.parseStations(data);
                    self.onDataLoaded('backup');
                } catch(e) {
                    console.warn('[DRM Schedule] Backup server parse failed:', e);
                    self.loadLocalBackup();
                }
            },
            error: function() {
                console.warn('[DRM Schedule] Backup server failed');
                self.loadLocalBackup();
            }
        });
    },

    loadLocalBackup: function() {
        var self = this;
        console.log('[DRM Schedule] Falling back to local backup');

        $.ajax({
            url: this.config.local_backup,
            dataType: 'json',
            timeout: 5000,
            success: function(data) {
                console.log('[DRM Schedule] Local data loaded');
                self.stations = self.parseStations(data);
                self.onDataLoaded('local');
            },
            error: function() {
                console.error('[DRM Schedule] All data sources failed');
                self.showError();
            }
        });
    },

    // 移除JSON注释 (支持CJSON格式)
    stripComments: function(text) {
        // 移除单行注释 //
        text = text.replace(/\/\/.*$/gm, '');
        // 移除多行注释 /* */
        text = text.replace(/\/\*[\s\S]*?\*\//g, '');
        return text;
    },

    onDataLoaded: function(source) {
        console.log('[DRM Schedule] Data loaded from:', source);
        this.currentSource = source;

        this.hideLoading();
        this.renderSchedule();

        var statusText = source === 'remote' ?
            'Loaded from kiwisdr.com' :
            source === 'backup' ?
            'Loaded from backup server' :
            'Using default data';
        this.setStatus(statusText);
    },

    // ========== 数据解析 ==========
    parseStations: function(data) {
        var stations = [];
        var idx = 0;
        var isIndiaMW = false;

        try {
            // 格式1: KiwiSDR drmrx.cjson格式
            if (Array.isArray(data)) {
                data.forEach(function(regionObj) {
                    var prefix = '';
                    var regionName = null;

                    // 获取区域名称 (SW, MW, 或其他)
                    for (var key in regionObj) {
                        if (regionObj[key] === null) {
                            regionName = key;
                            if (regionName === 'India MW') {
                                prefix = 'India, ';
                                isIndiaMW = true;
                            }
                            break;
                        }
                    }

                    for (var serviceName in regionObj) {
                        // 跳过区域标识键和空值
                        if (!regionObj[serviceName] || serviceName === regionName) {
                            continue;
                        }

                        var serviceData = regionObj[serviceName];
                        if (!Array.isArray(serviceData)) continue;

                        // 清理服务名称中的下划线 (KiwiSDR用_表示换行)
                        var cleanName = serviceName.replace(/_/g, ' ');

                        // 提取URL (如果存在)
                        var serviceUrl = null;
                        var startIdx = 0;
                        if (serviceData.length > 0 && typeof serviceData[0] === 'string') {
                            serviceUrl = serviceData[0];
                            startIdx = 1;
                        }

                        // 处理频率/时间对 (freq, start, end, freq2, start2, end2, ...)
                        for (var i = startIdx; i < serviceData.length - 2; i += 3) {
                            var freq = serviceData[i];
                            var startTimeRaw = serviceData[i + 1];
                            var endTimeRaw = serviceData[i + 2];

                            // 跳过非数字频率
                            if (typeof freq !== 'number') continue;

                            // 统一处理时间格式（支持字符串和小数格式）
                            var startTime = this.parseKiwiTime(startTimeRaw);
                            var endTime = this.parseKiwiTime(endTimeRaw);

                            // 根据KiwiSDR逻辑：负数表示需要验证
                            var verified = (startTime < 0 || endTime < 0);

                            // 取绝对值用于时间计算
                            var absStart = Math.abs(startTime);
                            var absEnd = Math.abs(endTime);

                            // 处理跨天的情况（与KiwiSDR逻辑一致）
                            if (absEnd < absStart) {
                                // 分成两个广播段
                                stations.push({
                                    t: this.STATION_TYPES.MULTI,
                                    f: freq,
                                    s: prefix + cleanName,
                                    r: regionName,
                                    b: absStart,
                                    e: 24,
                                    br: Math.round(absStart),
                                    h: Math.round(24 - absStart),
                                    v: verified,
                                    u: serviceUrl,
                                    i: idx++,
                                    mw: (freq >= 530 && freq <= 1700)
                                });
                                stations.push({
                                    t: this.STATION_TYPES.MULTI,
                                    f: freq,
                                    s: prefix + cleanName,
                                    r: regionName,
                                    b: 0,
                                    e: absEnd,
                                    br: 0,
                                    h: Math.round(absEnd),
                                    v: verified,
                                    u: serviceUrl,
                                    i: idx++,
                                    mw: (freq >= 530 && freq <= 1700)
                                });
                            } else {
                                stations.push({
                                    t: this.STATION_TYPES.MULTI,
                                    f: freq,
                                    s: prefix + cleanName,
                                    r: regionName,
                                    b: absStart,
                                    e: absEnd,
                                    br: Math.round(absStart),
                                    h: Math.round(absEnd - absStart),
                                    v: verified,
                                    u: serviceUrl,
                                    i: idx++,
                                    mw: (freq >= 530 && freq <= 1700)
                                });
                            }
                        }
                    }

                    // 添加服务分隔符 (匹配KiwiSDR逻辑 - 除了India MW)
                    if (!isIndiaMW) {
                        stations.push({
                            t: this.STATION_TYPES.SERVICE,
                            f: 0,
                            s: prefix + cleanName,
                            r: regionName
                        });
                        idx++;
                    }
                });
            }
            // 格式2: 本地JSON格式 (备用)
            else if (data.stations && Array.isArray(data.stations)) {
                var self = this; // Fix: ensure self is defined
                data.stations.forEach(function(station) {
                    if (!station.schedule) return;

                    station.schedule.forEach(function(sched) {
                        // 转换时间格式: "0000" → 0, "1230" → 12.5
                        var startHour = parseInt(sched.start.substring(0, 2));
                        var startMin = parseInt(sched.start.substring(2, 4));
                        var startTime = startHour + startMin / 60;

                        // 计算结束时间
                        var endTime = startTime + sched.duration / 60;
                        if (endTime >= 24) endTime = endTime - 24;

                        stations.push({
                            t: self.STATION_TYPES.SINGLE,
                            f: station.freq,
                            s: station.service,
                            r: station.target || 'Unknown',
                            b: startTime,
                            e: endTime,
                            br: Math.round(startTime),
                            h: Math.round(sched.duration / 60),
                            v: false,
                            u: station.url || null,
                            i: idx++
                        });
                    });
                });
            }

            console.log('[DRM Schedule] Parsed stations:', stations.length);
        } catch(e) {
            console.error('[DRM Schedule] Parse error:', e);
        }

        return stations;
    },

    // 统一处理KiwiSDR时间格式
    parseKiwiTime: function(timeRaw) {
        if (typeof timeRaw === 'number') {
            // stations2.cjson格式：直接返回数字
            return timeRaw;
        } else if (typeof timeRaw === 'string') {
            // drmrx.cjson格式：解析字符串
            timeRaw = timeRaw.replace(/"/g, '');
            var parts = timeRaw.split(':');
            if (parts.length !== 2) return parseFloat(timeRaw) || 0;

            var hours = parseFloat(parts[0]) || 0;
            var minutes = parseFloat(parts[1]) || 0;

            // 关键：负数时减去分钟（与kiwi_hh_mm一致）
            if (hours < 0) {
                return hours - (minutes / 60);
            } else {
                return hours + (minutes / 60);
            }
        }
        return 0;
    },

    // ========== 渲染 ==========
    renderSchedule: function() {
        if (!this.stations || this.stations.length === 0) {
            this.setStatus('No schedule data available');
            return;
        }

        console.log('[DRM Schedule] Rendering in mode:', this.displayMode);

        // 高亮当前模式按钮
        $('.drm-btn[data-mode]').removeClass('active');
        $('.drm-btn[data-mode="' + this.displayMode + '"]').addClass('active');

        // 渲染时间轴 (KiwiSDR风格)
        this.renderTimeScale();

        // 根据模式渲染
        var html = '';
        switch(this.displayMode) {
            case this.DISPLAY_MODES.BY_SVC:
                html = this.renderByService();
                break;
            case this.DISPLAY_MODES.BY_TIME:
                html = this.renderByTime();
                break;
            case this.DISPLAY_MODES.BY_FREQ:
                html = this.renderByFrequency();
                break;
        }

        $('#id-drm-panel-by-svc').html(html);

        // 更新状态
        var count = this.stations.length;
        this.setStatus(count + ' broadcasts found');
    },

    renderTimeScale: function() {
        var html = '';
        var narrow = false;

        // 渲染小时标记 (匹配KiwiSDR的drm_tscale)
        for (var hour = 0; hour <= 24; hour++) {
            var pos = this.timeToPixels(hour, narrow);
            html += '<div id="id-drm-sched-tscale" style="left:' + pos + 'px;"></div>';
        }

        // 渲染当前时间线
        var now = new Date();
        var currentHour = now.getUTCHours() + now.getUTCMinutes() / 60;
        var currentPos = this.timeToPixels(currentHour, narrow);
        var currentTime = now.getUTCHours().toString().padStart(2, '0') + ':' +
                         now.getUTCMinutes().toString().padStart(2, '0');

        html += '<div id="id-drm-sched-now" style="left:' + currentPos + 'px;" data-time="' + currentTime + ' UTC"></div>';

        $('#id-drm-tscale').html(html);

        // 启动时间线更新定时器 (仅一次)
        if (!this.timelineInterval) {
            var self = this;
            this.timelineInterval = setInterval(function() {
                // 只更新时间线位置,不重新渲染整个时间轴
                var now = new Date();
                var currentHour = now.getUTCHours() + now.getUTCMinutes() / 60;
                var pos = self.timeToPixels(currentHour, false);
                $('#id-drm-sched-now').css('left', pos + 'px');
            }, 60000); // 每分钟更新一次
        }
    },

    // 将时间转换为像素位置 (匹配KiwiSDR的drm_tscale)
    timeToPixels: function(hours, narrow) {
        var Lmargin = 27, Rmargin = narrow ? 0 : 20, scrollBar = 15;
        var width = this.config.panel_width;
        return (Lmargin + hours * (width - Lmargin - Rmargin - scrollBar) / 24).toFixed(0);
    },

    // 计算时间偏移 (匹配KiwiSDR)
    calculateTimeOffset: function(narrow) {
        var Lmargin = 27, Rmargin = narrow ? 0 : 20, scrollBar = 15;
        var width = this.config.panel_width;
        return (Lmargin + 0.25 * (width - Lmargin - Rmargin - scrollBar) / 24).toFixed(0);
    },

    // 格式化时间提示 (匹配KiwiSDR)
    formatTimeTooltip: function(station) {
        var b_hh = Math.floor(station.b);
        var b_mm = Math.round(60 * (station.b - b_hh));
        var e_hh = Math.floor(station.e);
        var e_mm = Math.round(60 * (station.e - e_hh));
        var freq = station.f;

        return freq.toFixed(0) + ' kHz\n' +
            b_hh.toString().padStart(2, '0') + b_mm.toString().padStart(2, '0') + '-' +
            e_hh.toString().padStart(2, '0') + e_mm.toString().padStart(2, '0');
    },

    // ========== 按服务渲染 (匹配KiwiSDR) ==========
    renderByService: function() {
        var self = this;
        var html = '';
        var narrow = false;
        var usingDefault = this.currentSource !== 'remote';

        // 添加警告 (如果使用默认数据)
        if (usingDefault) {
            html += '<div class="w3-yellow w3-padding w3-show-inline-block">can\'t contact kiwisdr.com<br>using default data</div>';
        }

        // 计算时间偏移
        var toff = this.calculateTimeOffset(narrow);

        // 按服务名称分组 (匹配KiwiSDR逻辑)
        var grouped = {};
        this.stations.forEach(function(station) {
            if (station.t === self.STATION_TYPES.REGION) return; // 跳过区域条目

            var key = station.s + '|' + station.f;
            if (!grouped[key]) {
                grouped[key] = {
                    name: station.s,
                    frequency: station.f,
                    url: station.u,
                    region: station.r,
                    verified: station.v || false,
                    schedules: []
                };
            }
            grouped[key].schedules.push(station);
        });

        // 渲染每个服务组 (完全匹配KiwiSDR)
        var keys = Object.keys(grouped);
        keys.forEach(function(key, index) {
            var service = grouped[key];
            var timeSlotsHtml = '';
            var time_h = 30; // 标准高度

            // 处理所有时间段
            service.schedules.forEach(function(sched) {
                var b_px = self.timeToPixels(sched.b, narrow);
                var e_px = self.timeToPixels(sched.e, narrow);
                var width = Math.max((e_px - b_px + 2), 2); // 最小2px宽度

                timeSlotsHtml += '<div class="id-drm-sched-time ' +
                    (sched.v ? 'w3-light-green' : '') + '" ' +
                    'style="left:' + b_px + 'px; width:' + width + 'px; height:' + time_h + 'px;" ' +
                    'title="' + self.formatTimeTooltip(sched) + '" ' +
                    'onclick="kiwi_drm_click(' + sched.i + ');"' +
                    '></div>';
            });

            // 构建info图标 (如果有URL)
            var infoIcon = '';
            if (service.url) {
                infoIcon = '<a href="' + service.url + '" target="_blank" class="w3-valign" ' +
                          'onclick="event.stopPropagation();">' +
                          '<i class="fa fa-info-circle w3-link-darker-color cl-drm-sched-info"></i>' +
                          '</a>';
            }

            // 格式化电台名称 (匹配KiwiSDR)
            var station_name = service.name;
            station_name += '&nbsp;&nbsp;&nbsp;' + (narrow ? '<br>' : '') + service.frequency + ' kHz';

            var count = (station_name.match(/<br>/g) || []).length;
            var em = count + (narrow ? 2 : 1);

            // 构建电台条目 (完全匹配KiwiSDR)
            html += '<div class="cl-drm-sched-station cl-drm-sched-striped w3-valign">' +
                '<div style="font-size:' + em + 'em;">&nbsp;</div>' +
                infoIcon +
                timeSlotsHtml +
                '<div class="cl-drm-station-name" style="left:' + toff + 'px;">' + station_name + '</div>' +
                '</div>';

            // 添加服务分隔符 (匹配KiwiSDR)
            if (index < keys.length - 1) {
                html += '<div class="cl-drm-sched-hr-div cl-drm-sched-striped"><hr class="cl-drm-sched-hr"></div>';
            }
        });

        return html;
    },

    // ========== 其他渲染模式 ==========
    renderByTime: function() {
        // 按开始时间排序
        var sorted = this.stations.slice().sort(function(a, b) {
            return a.b - b.b;
        });

        return this.renderStationList(sorted);
    },

    renderByFrequency: function() {
        // 按频率排序
        var sorted = this.stations.slice().sort(function(a, b) {
            return a.f - b.f;
        });

        // 添加波段分隔
        return this.renderStationListWithBands(sorted);
    },

    renderStationList: function(stations) {
        var html = '';
        var utcNow = this.getUTCHours();

        stations.forEach(function(station) {
            var isActive = (utcNow >= station.b && utcNow < station.e);

            html += '<div class="drm-station-entry ' + (isActive ? 'drm-active' : '') + '">' +
                '<div class="drm-station-info">' +
                '<span class="drm-name">' + station.s + '</span>' +
                '<span class="drm-freq">' + station.f + ' kHz</span>' +
                '<span class="drm-time">' + this.formatTime(station.b) + '-' + this.formatTime(station.e) + '</span>' +
                '</div>' +
                '</div>';
        }.bind(this));

        return html;
    },

    renderStationListWithBands: function(stations) {
        var html = '';
        var lastBand = '';
        var utcNow = this.getUTCHours();

        stations.forEach(function(station) {
            var band = this.getBand(station.f);

            if (band !== lastBand) {
                html += '<div class="drm-band-separator">' + band + '</div>';
                lastBand = band;
            }

            var isActive = (utcNow >= station.b && utcNow < station.e);

            html += '<div class="drm-station-entry ' + (isActive ? 'drm-active' : '') + '">' +
                '<div class="drm-station-info">' +
                '<span class="drm-name">' + station.s + '</span>' +
                '<span class="drm-freq">' + station.f + ' kHz</span>' +
                '<span class="drm-time">' + this.formatTime(station.b) + '-' + this.formatTime(station.e) + '</span>' +
                '</div>' +
                '</div>';
        }.bind(this));

        return html;
    },

    // ========== 工具函数 ==========
    getUTCHours: function() {
        var now = new Date();
        return now.getUTCHours() + now.getUTCMinutes() / 60;
    },

    formatTime: function(hours) {
        var h = Math.floor(hours);
        var m = Math.round((hours - h) * 60);
        return h.toString().padStart(2, '0') + ':' + m.toString().padStart(2, '0');
    },

    getBand: function(freq) {
        var bands = [
            { name: 'LW', min: 140, max: 300 },
            { name: 'MW', min: 500, max: 1700 },
            { name: '120m', min: 2250, max: 2500 },
            { name: '90m', min: 3100, max: 3500 },
            { name: '75m', min: 3800, max: 4100 },
            { name: '60m', min: 4700, max: 5100 },
            { name: '49m', min: 5800, max: 6300 },
            { name: '41m', min: 7200, max: 7600 },
            { name: '31m', min: 9300, max: 10000 },
            { name: '25m', min: 11500, max: 12200 },
            { name: '22m', min: 13500, max: 14000 },
            { name: '19m', min: 15000, max: 15900 },
            { name: '16m', min: 17400, max: 18000 },
            { name: '15m', min: 18800, max: 19100 },
            { name: '13m', min: 21400, max: 22000 },
            { name: '11m', min: 25500, max: 26200 }
        ];

        for (var i = 0; i < bands.length; i++) {
            if (freq >= bands[i].min && freq <= bands[i].max) {
                return bands[i].name;
            }
        }
        return 'Other';
    },

    // ========== UI 函数 ==========
    showLoading: function() {
        $('#id-drm-panel-by-svc').html('<div class="drm-loading-msg">&nbsp;loading data from kiwisdr.com ...</div>');
    },

    hideLoading: function() {
        $('.drm-loading-msg').remove();
    },

    showError: function() {
        $('#id-drm-panel-by-svc').html('<div class="w3-yellow w3-padding w3-show-inline-block">Failed to load schedule data</div>');
    },

    setStatus: function(text) {
        console.log('[DRM Schedule] Status:', text);
    },

    setDisplayMode: function(mode) {
        this.displayMode = mode;
        this.renderSchedule();

        // 更新按钮状态
        $('.drm-btn[data-mode]').removeClass('active');
        $('.drm-btn[data-mode="' + mode + '"]').addClass('active');
    },

    togglePanel: function() {
        var panel = $('#id-drm-panel-1-by-svc');
        if (panel.is(':visible')) {
            panel.hide();
            this.isPanelVisible = false;
        } else {
            panel.show();
            this.isPanelVisible = true;
        }
    },

    bindEvents: function() {
        var self = this;

        // 模式按钮
        $('.drm-btn[data-mode]').on('click', function() {
            var mode = $(this).data('mode');
            self.setDisplayMode(mode);
        });

        // 设置初始模式
        this.setDisplayMode(this.DISPLAY_MODES.BY_SVC);
    },

    startAutoRefresh: function() {
        var self = this;

        // 清除现有定时器
        if (this.refreshTimer) {
            clearInterval(this.refreshTimer);
        }

        // 启动新定时器
        this.refreshTimer = setInterval(function() {
            console.log('[DRM Schedule] Auto-refreshing...');
            self.loadStations();
        }, this.config.refresh_interval);
    },

    // 全局点击处理
    stopPropagation: function(e) {
        if (e) e.stopPropagation();
    }
};

// 全局点击处理函数 (需要与您的系统集成)
window.kiwi_drm_click = function(index) {
    console.log('[DRM Schedule] Station clicked:', index);

    if (typeof DRM_Schedule !== 'undefined' && DRM_Schedule.stations) {
        var station = DRM_Schedule.stations[index];
        if (station) {
            console.log('Tuning to:', station.f, 'kHz');

            // 这里应该调用您的频率调谐函数
            // 例如: if (typeof tune_to === 'function') {
            //         tune_to(station.f, 'drm', undefined);
            //       }
        }
    }
};

console.log('[DRM Schedule] KiwiSDR-aligned module loaded');