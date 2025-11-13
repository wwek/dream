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

    // 加载jQuery Modal库
    if (typeof $.modal === 'undefined') {
        console.log('[DRM Schedule] Loading jQuery Modal library...');

        // 加载CSS
        var modalCSS = document.createElement('link');
        modalCSS.rel = 'stylesheet';
        modalCSS.href = 'https://cdnjs.cloudflare.com/ajax/libs/jquery-modal/0.9.2/jquery.modal.min.css';
        document.head.appendChild(modalCSS);

        // 加载JS
        var modalJS = document.createElement('script');
        modalJS.src = 'https://cdnjs.cloudflare.com/ajax/libs/jquery-modal/0.9.2/jquery.modal.min.js';
        modalJS.onload = function() {
            console.log('[DRM Schedule] jQuery Modal loaded');
            DRM_Schedule.initializeWhenReady();
        };
        document.head.appendChild(modalJS);
    } else {
        console.log('[DRM Schedule] jQuery Modal already available');
        DRM_Schedule.initializeWhenReady();
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

        // 手动更新 (移除自动刷新)
        auto_refresh: false,

        // 面板尺寸 (动态获取实际宽度，此处仅作为降级默认值)
        panel_width: 450,  // 保守的默认宽度，实际使用容器实际宽度
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

    // ========== 等待依赖库加载后初始化 ==========
    initializeWhenReady: function() {
        var self = this;

        // 等待OpenWebRX初始化完成
        $(document).on('event:owrx_initialized', function() {
            console.log('[DRM Schedule] OpenWebRX initialized, loading schedule...');
            self.initialize();
        });

        // 如果已经初始化,直接加载
        if (typeof demodulatorPanel !== 'undefined') {
            this.initialize();
        }
    },

    // ========== 初始化 ==========
    initialize: function() {
        console.log('[DRM Schedule] Initializing (KiwiSDR-aligned)...');
        console.log('[DRM Schedule] Panel will be hidden by default. Click panel button to open.');

        this.createPanelButton();
        this.createUI();
        this.loadStations();
        this.bindEvents();

        console.log('[DRM Schedule] Initialized successfully (KiwiSDR-aligned)');
    },

    // ========== 创建面板按钮 (类似doppler插件) ==========
    createPanelButton: function() {
        var self = this;

        // 在模式选择器后插入DRM面板按钮行（初始隐藏）
        if ($('#drm-schedule-row').length === 0) {
            $('.openwebrx-modes').after(`
                <div id="drm-schedule-row" class="openwebrx-panel-line openwebrx-panel-flex-line" style="display: none;">
                    <div id="drm-schedule-open-btn" class="openwebrx-button openwebrx-demodulator-button" style="width: 100%;">
                        DRM Schedule
                    </div>
                </div>
            `);

            // 绑定打开面板事件
            $('#drm-schedule-open-btn').on('click', function(e) {
                e.preventDefault();
                e.stopPropagation();
                console.log('[DRM Schedule] Button clicked');
                self.showPanel();
            });

            console.log('[DRM Schedule] Panel button created and event bound');
        }

        // 监听模式变化
        this.watchModeChanges();
    },

    // 监听模式变化，只在DRM模式下显示
    watchModeChanges: function() {
        var self = this;
        var lastMode = null;

        // 检查当前模式
        var checkMode = function() {
            try {
                var currentMode = null;

                // 方法1: 检查DRM按钮是否active
                var drmButton = $('.openwebrx-demodulator-button').filter(function() {
                    return $(this).text().trim() === 'DRM';
                });

                if (drmButton.length > 0 && drmButton.hasClass('highlighted')) {
                    currentMode = 'drm';
                }

                // 方法2: 尝试通过demodulatorPanel获取
                if (!currentMode && typeof demodulatorPanel !== 'undefined' && demodulatorPanel.getDemodulator) {
                    var demod = demodulatorPanel.getDemodulator();
                    if (demod && typeof demod.get_modulation === 'function') {
                        var mode = demod.get_modulation();
                        if (mode === 'drm') {
                            currentMode = 'drm';
                        }
                    }
                }

                // 只在模式真正改变时才执行操作
                if (currentMode !== lastMode) {
                    console.log('[DRM Schedule] Mode changed from', lastMode, 'to', currentMode);
                    lastMode = currentMode;

                    if (currentMode === 'drm') {
                        $('#drm-schedule-row').show();
                        console.log('[DRM Schedule] Button shown (DRM mode active)');
                    } else {
                        $('#drm-schedule-row').hide();
                        // 如果面板打开着，关闭它
                        // 但如果正在调频，不要关闭（防止调频过程中模式暂时变为null）
                        if (self.isPanelVisible && !self.isTuning) {
                            console.log('[DRM Schedule] Closing panel (DRM mode inactive)');
                            self.hidePanel();
                        }
                    }
                }
            } catch(e) {
                console.warn('[DRM Schedule] Mode check error:', e);
            }
        };

        // 延迟初始检查，等待页面完全加载
        setTimeout(function() {
            checkMode();
            console.log('[DRM Schedule] Initial mode check completed');
        }, 1500);

        // 监听模式按钮点击
        $(document).on('click', '.openwebrx-demodulator-button', function() {
            console.log('[DRM Schedule] Mode button clicked, checking in 200ms');
            setTimeout(checkMode, 200);
        });

        // 定期检查（备用方案，降低频率）
        setInterval(checkMode, 5000);
    },

    // ========== UI 创建 (完全匹配doppler风格) ==========
    createUI: function() {
        var self = this;

        // 创建模态窗口HTML (类似doppler的satellite-modal)
        var modalHtml = `
            <div id="drm-schedule-modal" class="modal drm-schedule-modal">
                <div class="drm-schedule-modal-header">
                    DRM Schedule
                    <button class="drm-refresh-btn openwebrx-button" onclick="DRM_Schedule.manualRefresh()" title="Refresh data">
                        <span id="drm-refresh-icon">🔄</span> Refresh
                    </button>
                </div>
                <div class="drm-schedule-modal-body">
                    <div id="id-drm-tscale"></div>
                    <div id="id-drm-panel-by-svc" class="w3-scroll-y">
                        <div class="drm-loading-msg">&nbsp;loading data from kiwisdr.com ...</div>
                    </div>
                </div>
                <div class="drm-schedule-modal-footer">
                    <div class="drm-schedule-controls openwebrx-panel-line">
                        <button class="drm-btn openwebrx-button active" data-mode="BY_SVC" onclick="DRM_Schedule.setDisplayMode('BY_SVC')">By Service</button>
                        <button class="drm-btn openwebrx-button" data-mode="BY_TIME" onclick="DRM_Schedule.setDisplayMode('BY_TIME')">By Time</button>
                        <button class="drm-btn openwebrx-button" data-mode="BY_FREQ" onclick="DRM_Schedule.setDisplayMode('BY_FREQ')">By Frequency</button>
                    </div>
                    <div class="openwebrx-button" rel="modal:close" onclick="$.modal.close()">Close</div>
                </div>
            </div>
        `;

        // 将模态窗口插入到drm-schedule-row (类似doppler插入到satellite-row)
        $('#drm-schedule-row').append(modalHtml);

        // 监听BEFORE_CLOSE事件 (类似doppler的清理逻辑)
        $('#drm-schedule-modal').on($.modal.BEFORE_CLOSE, function(event, modal) {
            self.isPanelVisible = false;
            console.log('[DRM Schedule] Modal closing');
        });

        console.log('[DRM Schedule] Modal UI created (doppler-style)');
    },

    // 显示面板 (使用jQuery Modal库，完全匹配doppler)
    showPanel: function() {
        var self = this;
        console.log('[DRM Schedule] showPanel called');

        // 使用jQuery Modal显示 (与doppler完全相同的配置)
        $('#drm-schedule-modal').modal({
            escapeClose: true,
            clickClose: false,
            showClose: false
        });

        this.isPanelVisible = true;
        console.log('[DRM Schedule] Modal shown');

        // 模态窗口显示后，等待DOM渲染完成再重新渲染时间轴
        // 这样可以获取到正确的容器宽度
        setTimeout(function() {
            console.log('[DRM Schedule] Re-rendering time scale after modal shown');
            self.renderSchedule();
        }, 100);
    },

    // 隐藏面板 (使用jQuery Modal库)
    hidePanel: function() {
        $.modal.close();
        this.isPanelVisible = false;
        console.log('[DRM Schedule] Modal closed');
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
                console.log('[DRM Schedule] Remote data loaded:', text.length, 'bytes');
                try {
                    // 移除C风格注释 (// 和 /* */)
                    console.log('[DRM Schedule] Stripping comments...');
                    var cleanJson = self.stripComments(text);
                    console.log('[DRM Schedule] Clean JSON length:', cleanJson.length, 'bytes');

                    console.log('[DRM Schedule] Parsing JSON...');
                    var data = JSON.parse(cleanJson);
                    console.log('[DRM Schedule] JSON parsed successfully, entries:', Array.isArray(data) ? data.length : 'N/A');

                    self.stations = self.parseStations(data);
                    console.log('[DRM Schedule] Stations parsed:', self.stations ? self.stations.length : 0);
                    self.onDataLoaded('remote');
                } catch(e) {
                    console.error('[DRM Schedule] CJSON parse failed:', e);
                    console.error('[DRM Schedule] Error details:', e.message, e.stack);
                    self.loadBackupServer();
                }
            },
            error: function(xhr, status, error) {
                console.warn('[DRM Schedule] Remote load failed:', status, error);
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
        try {
            // 更安全的注释移除方法
            var lines = text.split('\n');
            var result = [];
            var inBlockComment = false;

            for (var i = 0; i < lines.length; i++) {
                var line = lines[i];
                var processedLine = '';
                var inString = false;
                var stringChar = '';

                for (var j = 0; j < line.length; j++) {
                    var char = line[j];
                    var nextChar = j + 1 < line.length ? line[j + 1] : '';

                    // 处理字符串
                    if ((char === '"' || char === "'") && !inBlockComment) {
                        if (!inString) {
                            inString = true;
                            stringChar = char;
                        } else if (char === stringChar && line[j - 1] !== '\\') {
                            inString = false;
                        }
                        processedLine += char;
                        continue;
                    }

                    // 在字符串内，直接添加字符
                    if (inString) {
                        processedLine += char;
                        continue;
                    }

                    // 处理块注释结束
                    if (inBlockComment) {
                        if (char === '*' && nextChar === '/') {
                            inBlockComment = false;
                            j++; // 跳过 /
                        }
                        continue;
                    }

                    // 处理块注释开始
                    if (char === '/' && nextChar === '*') {
                        inBlockComment = true;
                        j++; // 跳过 *
                        continue;
                    }

                    // 处理单行注释
                    if (char === '/' && nextChar === '/') {
                        break; // 忽略行的剩余部分
                    }

                    processedLine += char;
                }

                // 只添加非空行
                if (processedLine.trim().length > 0) {
                    result.push(processedLine);
                }
            }

            return result.join('\n');
        } catch(e) {
            console.error('[DRM Schedule] stripComments error:', e);
            // 如果解析失败，返回原始文本
            return text;
        }
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

        // 触发自定义事件，通知数据加载完成
        $(document).trigger('drm:loaded', {
            source: source,
            stations: this.stations,
            count: this.stations ? this.stations.length : 0
        });
        console.log('[DRM Schedule] Triggered drm:loaded event');
    },

    // ========== 数据解析 ==========
    parseStations: function(data) {
        var stations = [];
        var idx = 0;
        var isIndiaMW = false;
        var self = this; // 保存 this 引用

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
                            var startTime = self.parseKiwiTime(startTimeRaw);
                            var endTime = self.parseKiwiTime(endTimeRaw);

                            // 根据KiwiSDR逻辑：负数表示需要验证
                            var verified = (startTime < 0 || endTime < 0);

                            // 取绝对值用于时间计算
                            var absStart = Math.abs(startTime);
                            var absEnd = Math.abs(endTime);

                            // 处理跨天的情况（与KiwiSDR逻辑一致）
                            if (absEnd < absStart) {
                                // 分成两个广播段
                                stations.push({
                                    t: self.STATION_TYPES.MULTI,
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
                                    t: self.STATION_TYPES.MULTI,
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
                                    t: self.STATION_TYPES.MULTI,
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
                            t: self.STATION_TYPES.SERVICE,
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
                // self 已在外层定义
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
        var bgHtml = '';
        var narrow = false;

        // 渲染小时标记和背景刻度线 (KiwiSDR风格全屏背景)
        for (var hour = 0; hour <= 24; hour++) {
            var pos = this.timeToPixels(hour, narrow);

            // 时间轴上的刻度
            html += '<div class="id-drm-sched-tscale" style="left:' + pos + 'px;"></div>';

            // 时间标签（每4小时显示，格式: 0h, 4h, 8h...更紧凑）
            if (hour % 4 === 0 && hour < 24) {
                html += '<div class="drm-time-label" style="left:' + pos + 'px;">' +
                        hour + 'h</div>';
            }

            // 内容区域的背景刻度线
            bgHtml += '<div class="drm-tscale-bg" style="left:' + pos + 'px;"></div>';
        }

        // 渲染当前时间线 (使用本地时间)
        var now = new Date();
        var currentHour = now.getHours() + now.getMinutes() / 60;
        var currentPos = this.timeToPixels(currentHour, narrow);
        var currentTime = now.getHours().toString().padStart(2, '0') + ':' +
                         now.getMinutes().toString().padStart(2, '0');

        html += '<div id="id-drm-sched-now" style="position:absolute; left:' + currentPos + 'px;" data-time="' + currentTime + ' Local"></div>';

        // 更新时间轴
        $('#id-drm-tscale').html(html);

        // 在内容区域添加背景刻度线
        $('#id-drm-panel-by-svc').prepend('<div class="drm-tscale-bg-container" style="position:absolute; top:0; left:0; width:100%; height:100%; pointer-events:none; z-index:0;">' + bgHtml + '</div>');

        // 启动时间线更新定时器 (仅一次)
        if (!this.timelineInterval) {
            var self = this;
            this.timelineInterval = setInterval(function() {
                // 只更新时间线位置,不重新渲染整个时间轴 (使用本地时间)
                var now = new Date();
                var currentHour = now.getHours() + now.getMinutes() / 60;
                var pos = self.timeToPixels(currentHour, false);
                var currentTime = now.getHours().toString().padStart(2, '0') + ':' +
                                 now.getMinutes().toString().padStart(2, '0');
                $('#id-drm-sched-now').css('left', pos + 'px').attr('data-time', currentTime + ' Local');
            }, 60000); // 每分钟更新一次
        }
    },

    // 将时间转换为像素位置 (匹配KiwiSDR的drm_tscale)
    timeToPixels: function(hours, narrow) {
        var Lmargin = 27, Rmargin = narrow ? 0 : 20, scrollBar = 0;  // 不考虑滚动条宽度

        // 尝试多种方式获取实际容器宽度
        var width = $('#id-drm-tscale').width() ||
                    $('#id-drm-panel-by-svc').width() ||
                    $('.drm-schedule-modal-body').width() ||
                    this.config.panel_width;

        // 调试日志（仅在第一次调用时输出）
        if (hours === 0) {
            console.log('[DRM Schedule] timeToPixels width:', width,
                       'tscale:', $('#id-drm-tscale').width(),
                       'panel:', $('#id-drm-panel-by-svc').width(),
                       'modal:', $('.drm-schedule-modal-body').width());
        }

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

            // 处理所有时间段
            service.schedules.forEach(function(sched) {
                var b_px = self.timeToPixels(sched.b, narrow);
                var e_px = self.timeToPixels(sched.e, narrow);
                var width = Math.max((e_px - b_px + 2), 3); // 最小3px宽度

                timeSlotsHtml += '<div class="id-drm-sched-time ' +
                    (sched.v ? 'w3-light-green' : '') + '" ' +
                    'style="left:' + b_px + 'px; width:' + width + 'px;" ' +
                    'title="' + self.formatTimeTooltip(sched) + '" ' +
                    'onclick="kiwi_drm_click(' + sched.i + ');"' +
                    '></div>';
            });

            // 构建info图标 (如果有URL)
            var infoIcon = '';
            if (service.url) {
                infoIcon = '<a href="' + service.url + '" target="_blank" class="drm-info-link" ' +
                          'onclick="event.stopPropagation();">' +
                          '<i class="fa fa-info-circle cl-drm-sched-info"></i>' +
                          '</a>';
            }

            // 格式化电台名称 (匹配KiwiSDR)
            var station_name = service.name;
            station_name += '&nbsp;&nbsp;&nbsp;' + (narrow ? '<br>' : '') + service.frequency + ' kHz';

            var stationHeight = 24; // 紧凑型设计

            // 构建电台条目 (完全匹配KiwiSDR)
            html += '<div class="cl-drm-sched-station cl-drm-sched-striped" style="height:' + stationHeight + 'px;">' +
                infoIcon +
                timeSlotsHtml +
                '<div class="cl-drm-station-name">' + station_name + '</div>' +
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
        if (this.isPanelVisible) {
            this.hidePanel();
        } else {
            this.showPanel();
        }
    },

    switchMode: function(mode) {
        this.setDisplayMode(mode);
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

    // 手动刷新 (类似doppler插件的toggleRefresh)
    manualRefresh: function() {
        var refreshIcon = $('#drm-refresh-icon');

        console.log('[DRM Schedule] Manual refresh triggered');

        // 添加旋转动画
        refreshIcon.css({
            animation: 'spin 1s linear infinite'
        });

        // 重新加载数据
        this.loadStations();

        // 3秒后停止动画
        setTimeout(function() {
            refreshIcon.css({
                animation: 'none'
            });
        }, 3000);
    },

    // 全局点击处理
    stopPropagation: function(e) {
        if (e) e.stopPropagation();
    }
};

// 全局点击处理函数 (调频到选中电台)
window.kiwi_drm_click = function(index) {
    console.log('[DRM Schedule] Station clicked:', index);

    if (typeof DRM_Schedule !== 'undefined' && DRM_Schedule.stations) {
        var station = DRM_Schedule.stations[index];
        if (station) {
            var freqKHz = station.f;
            var freqHz = freqKHz * 1000; // 转换为Hz

            console.log('[DRM Schedule] Tuning to:', freqKHz, 'kHz', '(', freqHz, 'Hz)');

            // 直接调频，不显示详情弹窗
            // DRM_Schedule.showStationInfo(station);

            // OpenWebRX 调频：直接操作解调器
            try {
                // 获取 demodulatorPanel (通过 UI 对象)
                var panel = (typeof UI !== 'undefined' && UI.getDemodulatorPanel) ?
                           UI.getDemodulatorPanel() :
                           (typeof window.demodulatorPanel !== 'undefined' ? window.demodulatorPanel : null);

                if (panel && typeof center_freq !== 'undefined') {
                    console.log('[DRM Schedule] Current center_freq:', center_freq, 'Hz, target:', freqHz, 'Hz');

                    // 设置调频标志，防止模式检查关闭面板
                    DRM_Schedule.isTuning = true;

                    // 步骤1: 先设置 DRM 模式
                    panel.setMode('drm');
                    console.log('[DRM Schedule] Step 1: Set mode to DRM');

                    // 步骤2: 等待模式切换完成后设置频率
                    setTimeout(function() {
                        try {
                            // 使用 frequencychange 事件触发频率切换 (让 OpenWebRX 自动处理)
                            if (panel.tuneableFrequencyDisplay && panel.tuneableFrequencyDisplay.element) {
                                panel.tuneableFrequencyDisplay.element.trigger('frequencychange', freqHz);
                                console.log('[DRM Schedule] Step 2: Triggered frequencychange to', freqKHz, 'kHz');
                            } else {
                                console.warn('[DRM Schedule] tuneableFrequencyDisplay not available');
                            }

                            // 步骤3: 再次确保模式是 DRM
                            setTimeout(function() {
                                var currentDemod = panel.getDemodulator();
                                if (currentDemod && currentDemod.get_modulation() !== 'drm') {
                                    panel.setMode('drm');
                                    console.log('[DRM Schedule] Step 3: Re-set mode to DRM');
                                } else {
                                    console.log('[DRM Schedule] Step 3: Mode is already DRM');
                                }

                                // 调频完成，清除标志
                                setTimeout(function() {
                                    DRM_Schedule.isTuning = false;
                                    console.log('[DRM Schedule] Tuning completed');
                                }, 200);
                            }, 100);
                        } catch(err) {
                            console.error('[DRM Schedule] Frequency setting error:', err);
                            DRM_Schedule.isTuning = false;
                        }
                    }, 150);
                } else {
                    console.warn('[DRM Schedule] demodulatorPanel or center_freq not available. panel:', !!panel, 'center_freq:', center_freq);
                }
            } catch(e) {
                console.error('[DRM Schedule] Tuning error:', e);
                DRM_Schedule.isTuning = false;
            }
        }
    }
};

// 显示电台详细信息 (KiwiSDR风格信息提示)
DRM_Schedule.showStationInfo = function(station) {
    var info = '📻 ' + station.s + '\n' +
               '📡 ' + station.f + ' kHz\n' +
               '🌍 ' + station.r + '\n' +
               '⏰ ' + this.formatTime(station.b) + ' - ' + this.formatTime(station.e) + ' UTC';

    // 简单的信息提示
    if (typeof $.modal !== 'undefined') {
        // 创建临时信息弹窗
        var infoHtml = '<div style="text-align:center; padding:20px; white-space:pre-line;">' +
                       info.replace(/\n/g, '<br>') +
                       '</div>';

        // 显示3秒后自动关闭
        var $info = $('<div>' + infoHtml + '</div>').appendTo('body');
        $info.modal({
            escapeClose: true,
            clickClose: true,
            showClose: false
        });

        setTimeout(function() {
            $.modal.close();
        }, 3000);
    } else {
        // 备用方案：console输出
        console.log('[DRM Schedule] Station Info:\n' + info);
    }
};

console.log('[DRM Schedule] KiwiSDR-aligned module loaded');