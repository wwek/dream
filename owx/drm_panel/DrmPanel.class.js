/**
 * DrmPanel - Dream DRM 状态显示面板类
 *
 * 实时显示 DRM 解调状态、信号质量、模式信息和服务列表
 * 参考 DAB/MetaPanel 的实现方式：构造函数中创建容器，update 时只更新内容
 */

function DrmPanel(el) {
    MetaPanel.call(this, el);
    this.modes = ['DRM'];

    // 音频编码类型名称
    this.audioCodecNames = ['AAC', 'OPUS', 'RESERVED', 'xHE-AAC'];

    // 创建容器结构（一次性创建，后续只更新内容）- Dream DRM 风格布局
    var $container = $(
        '<div class="drm-container">' +
            // 第一行：状态指示灯
            '<div class="drm-status-row">' +
                '<span class="drm-indicator drm-indicator-off" data-drm-ind="io">IO</span>' +
                '<span class="drm-indicator drm-indicator-off" data-drm-ind="time">Time</span>' +
                '<span class="drm-indicator drm-indicator-off" data-drm-ind="frame">Frame</span>' +
                '<span class="drm-indicator drm-indicator-off" data-drm-ind="fac">FAC</span>' +
                '<span class="drm-indicator drm-indicator-off" data-drm-ind="sdc">SDC</span>' +
                '<span class="drm-indicator drm-indicator-off" data-drm-ind="msc">MSC</span>' +
            '</div>' +

            // 第二行：信号质量（横向显示）
            '<div class="drm-signal-row">' +
                '<span class="drm-signal-item">IF Level: <span class="drm-value" data-drm-val="if_level">--</span> dB</span>' +
                '<span class="drm-signal-item">SNR: <span class="drm-value drm-snr" data-drm-val="snr">--</span> dB</span>' +
            '</div>' +

            // 第三行：DRM 模式和信道
            '<div class="drm-mode-row">' +
                '<span class="drm-mode-group">' +
                    'DRM mode ' +
                    '<span class="drm-mode-badges">' +
                        '<span class="drm-badge" data-drm-mode="A">A</span>' +
                        '<span class="drm-badge drm-active" data-drm-mode="B">B</span>' +
                        '<span class="drm-badge" data-drm-mode="C">C</span>' +
                        '<span class="drm-badge" data-drm-mode="D">D</span>' +
                    '</span>' +
                '</span>' +
                '<span class="drm-chan-group">' +
                    'Chan ' +
                    '<span class="drm-chan-badges">' +
                        '<span class="drm-chan-badge" data-chan="4.5">4.5</span>' +
                        '<span class="drm-chan-badge" data-chan="5">5</span>' +
                        '<span class="drm-chan-badge" data-chan="9">9</span>' +
                        '<span class="drm-chan-badge drm-active" data-chan="10">10</span>' +
                        '<span class="drm-chan-badge" data-chan="18">18</span>' +
                        '<span class="drm-chan-badge" data-chan="20">20</span>' +
                    '</span> kHz' +
                '</span>' +
                '<span class="drm-ilv-group">' +
                    'ILV <span class="drm-badge" data-drm-val="interleaver_long">L</span> ' +
                    '<span class="drm-badge drm-active" data-drm-val="interleaver_short">S</span>' +
                '</span>' +
            '</div>' +

            // 第四行：调制方式和保护级别
            '<div class="drm-qam-row">' +
                '<span class="drm-qam-group">' +
                    'SDC ' +
                    '<span class="drm-badge drm-active" data-drm-val="sdc_4">4</span>' +
                    '<span class="drm-badge" data-drm-val="sdc_16">16</span>' +
                    ' QAM' +
                '</span>' +
                '<span class="drm-qam-group">' +
                    'MSC ' +
                    '<span class="drm-badge drm-active" data-drm-val="msc_16">16</span>' +
                    '<span class="drm-badge" data-drm-val="msc_64">64</span>' +
                    ' QAM' +
                '</span>' +
                '<span class="drm-protect-group">' +
                    'Protect: A=<span data-drm-val="prot_level_a">0</span> ' +
                    'B=<span data-drm-val="prot_level_b">0</span>' +
                '</span>' +
                '<span class="drm-services-stat">' +
                    'Services: A=<span data-drm-val="audio_count">0</span> ' +
                    'D=<span data-drm-val="data_count">0</span>' +
                '</span>' +
            '</div>' +

            // 服务列表标题
            '<div class="drm-services-header">Services:</div>' +

            // 服务列表
            '<div class="drm-services-list" data-drm-services></div>' +

            // 时间显示区域
            '<div class="drm-time-row">' +
                '<span class="drm-time-item">UTC: <span class="drm-value" data-drm-val="time_utc">---- -- -- --:--:--</span></span>' +
                '<span class="drm-time-item">Local: <span class="drm-value" data-drm-val="time_local">---- -- -- --:--:--</span></span>' +
            '</div>' +
        '</div>'
    );

    $(this.el).append($container);
    this.$container = $container;

    // 添加文本按钮点击事件委托
    var me = this;
    this.$container.on('click', '.drm-service-text-btn', function() {
        var index = $(this).data('service-index');
        me.toggleTextContent(index, $(this));
    });

    // 保存展开状态的 Map: index -> boolean
    this.textExpandedState = {};

    this.clear();
}

DrmPanel.prototype = new MetaPanel();

DrmPanel.prototype.isSupported = function(data) {
    return data.type === 'drm_status';
};

DrmPanel.prototype.update = function(data) {
    if (!this.isSupported(data)) return;

    var status = data.value;
    if (!status) return;

    // Debug log
    //console.log('[DRM Panel] Received status update:', status);

    this.$container.show();

    // 更新状态指示灯 (处理数值: 0=ok, -1=off, >0=err)
    if (status.status) {
        this.updateIndicator('io', this.mapStatusValue(status.status.io));
        this.updateIndicator('time', this.mapStatusValue(status.status.time));
        this.updateIndicator('frame', this.mapStatusValue(status.status.frame));
        this.updateIndicator('fac', this.mapStatusValue(status.status.fac));
        this.updateIndicator('sdc', this.mapStatusValue(status.status.sdc));
        this.updateIndicator('msc', this.mapStatusValue(status.status.msc));
    }

    // 更新信号质量 (支持两种数据格式: signal.snr_db 或直接 snr)
    var snr = (status.signal && status.signal.snr_db != null) ? status.signal.snr_db : status.snr;
    var ifLevel = (status.signal && status.signal.if_level_db != null) ? status.signal.if_level_db : status.if_level;

    this.updateValue('snr', snr != null ? snr.toFixed(1) : '--');
    this.updateValue('if_level', ifLevel != null ? ifLevel.toFixed(1) : '--');
    this.updateValue('wmer', status.wmer != null ? status.wmer.toFixed(2) : '--');
    this.updateValue('delay', status.delay != null ? status.delay.toFixed(2) : '--');
    this.updateValue('doppler', status.doppler != null ? status.doppler.toFixed(2) : '--');

    // 更新模式信息 (支持嵌套 mode.* 格式)
    var modeData = status.mode || {};
    var robustness = modeData.robustness != null ? modeData.robustness : status.robustness;
    var bandwidth = modeData.bandwidth_khz != null ? modeData.bandwidth_khz : (modeData.bandwidth || status.bandwidth);
    var interleaver = modeData.interleaver != null ? modeData.interleaver : status.interleaver;

    // DRM Mode (A/B/C/D) - 基于 robustness
    var drmModeLetters = ['A', 'B', 'C', 'D'];
    var drmMode = robustness != null && robustness < drmModeLetters.length
        ? drmModeLetters[robustness]
        : 'B';
    this.updateDrmModeBadges(drmMode);

    // 更新信道带宽高亮
    var bandwidthDisplay = ['4.5', '5', '9', '10', '18', '20'];
    var bwValue = typeof bandwidth === 'number' && bandwidth < bandwidthDisplay.length
        ? bandwidthDisplay[bandwidth]
        : bandwidth;
    this.updateChannelBadges(bwValue);

    // 更新交织器按钮
    this.updateInterleaverBadges(interleaver);

    // 更新编码模式 (SDC/MSC QAM: 0=4-QAM, 1=16-QAM, 2=64-QAM)
    var codingData = status.coding || {};
    this.updateQamBadges('sdc', codingData.sdc_qam);
    this.updateQamBadges('msc', codingData.msc_qam);

    // 更新保护级别 (Protection Level: 0-3)
    var protA = codingData.protection_a != null ? codingData.protection_a : status.protection_level_a;
    var protB = codingData.protection_b != null ? codingData.protection_b : status.protection_level_b;

    // 更新保护级别并高亮非零值
    this.updateValueWithHighlight('prot_level_a', protA != null ? protA : '--');
    this.updateValueWithHighlight('prot_level_b', protB != null ? protB : '--');

    // 更新服务列表 (优先使用 service_list，其次 services)
    var serviceList = status.service_list || status.services || [];

    // 统计音频和数据服务数量
    var audioCount = 0;
    var dataCount = 0;
    serviceList.forEach(function(service) {
        if (service.is_audio) {
            audioCount++;
        } else {
            dataCount++;
        }
    });

    // 更新服务计数并高亮非零值
    this.updateValueWithHighlight('audio_count', audioCount);
    this.updateValueWithHighlight('data_count', dataCount);

    if (serviceList.length > 0) {
        this.updateServices(serviceList);
    } else {
        this.$container.find('[data-drm-services]').empty();
    }

    // 更新时间显示 (从 timestamp 字段获取)
    if (status.timestamp) {
        this.updateTime(status.timestamp);
    }
};

DrmPanel.prototype.mapStatusValue = function(value) {
    // Dream DRM 状态值映射: 0=ok, -1=off/未激活, >0=error
    if (value === 0) return 'ok';
    if (value === -1) return 'off';
    if (value > 0) return 'err';
    return 'off';
};

DrmPanel.prototype.updateIndicator = function(name, state) {
    var $ind = this.$container.find('[data-drm-ind="' + name + '"]');
    $ind.removeClass('drm-indicator-ok drm-indicator-off drm-indicator-err');

    if (state === 'ok' || state === 'green') {
        $ind.addClass('drm-indicator-ok');
    } else if (state === 'err' || state === 'red') {
        $ind.addClass('drm-indicator-err');
    } else {
        $ind.addClass('drm-indicator-off');
    }
};

DrmPanel.prototype.updateValue = function(name, value) {
    this.$container.find('[data-drm-val="' + name + '"]').text(value);
};

// 更新数值并根据是否为0添加绿色高亮
DrmPanel.prototype.updateValueWithHighlight = function(name, value) {
    var $elem = this.$container.find('[data-drm-val="' + name + '"]');
    $elem.text(value);

    // 如果值不为0且不为'--'，添加active类（绿色高亮）
    if (value !== 0 && value !== '0' && value !== '--' && value !== null && value !== undefined) {
        $elem.addClass('drm-value-active');
    } else {
        $elem.removeClass('drm-value-active');
    }
};

DrmPanel.prototype.updateDrmModeBadges = function(mode) {
    // 更新 DRM 模式按钮高亮 (A/B/C/D)
    var $badges = this.$container.find('[data-drm-mode]');
    $badges.removeClass('drm-active');
    $badges.filter('[data-drm-mode="' + mode + '"]').addClass('drm-active');
};

DrmPanel.prototype.updateChannelBadges = function(bandwidth) {
    // 更新信道带宽按钮高亮
    var $badges = this.$container.find('.drm-chan-badge');
    $badges.removeClass('drm-active');
    $badges.filter('[data-chan="' + bandwidth + '"]').addClass('drm-active');
};

DrmPanel.prototype.updateInterleaverBadges = function(interleaver) {
    // 更新交织器按钮 (0=Short, 1=Long)
    var $short = this.$container.find('[data-drm-val="interleaver_short"]');
    var $long = this.$container.find('[data-drm-val="interleaver_long"]');

    $short.removeClass('drm-active');
    $long.removeClass('drm-active');

    if (interleaver === 1) {
        $long.addClass('drm-active');
    } else {
        $short.addClass('drm-active');
    }
};

DrmPanel.prototype.updateQamBadges = function(type, qamIndex) {
    // 更新 QAM 调制按钮 (type: 'sdc' or 'msc', qamIndex: 0=4-QAM, 1=16-QAM, 2=64-QAM)
    // SDC 只有 4 和 16, MSC 只有 16 和 64
    var $badges4 = this.$container.find('[data-drm-val="' + type + '_4"]');
    var $badges16 = this.$container.find('[data-drm-val="' + type + '_16"]');
    var $badges64 = this.$container.find('[data-drm-val="' + type + '_64"]');

    $badges4.removeClass('drm-active');
    $badges16.removeClass('drm-active');
    $badges64.removeClass('drm-active');

    if (qamIndex === 0) {
        $badges4.addClass('drm-active');
    } else if (qamIndex === 1) {
        $badges16.addClass('drm-active');
    } else if (qamIndex === 2) {
        $badges64.addClass('drm-active');
    }
};

DrmPanel.prototype.updateTime = function(timestamp) {
    // 将 Unix 时间戳转换为 UTC 和本地时间
    // timestamp 是秒级时间戳,需要转换为毫秒
    var date = new Date(timestamp * 1000);

    // UTC 时间 (YYYY-MM-DD HH:MM:SS 格式)
    var utcYear = date.getUTCFullYear();
    var utcMonth = (date.getUTCMonth() + 1).toString().padStart(2, '0');
    var utcDay = date.getUTCDate().toString().padStart(2, '0');
    var utcHours = date.getUTCHours().toString().padStart(2, '0');
    var utcMinutes = date.getUTCMinutes().toString().padStart(2, '0');
    var utcSeconds = date.getUTCSeconds().toString().padStart(2, '0');
    var utcTime = utcYear + '-' + utcMonth + '-' + utcDay + ' ' + utcHours + ':' + utcMinutes + ':' + utcSeconds;

    // 本地时间 (YYYY-MM-DD HH:MM:SS 格式)
    var localYear = date.getFullYear();
    var localMonth = (date.getMonth() + 1).toString().padStart(2, '0');
    var localDay = date.getDate().toString().padStart(2, '0');
    var localHours = date.getHours().toString().padStart(2, '0');
    var localMinutes = date.getMinutes().toString().padStart(2, '0');
    var localSeconds = date.getSeconds().toString().padStart(2, '0');
    var localTime = localYear + '-' + localMonth + '-' + localDay + ' ' + localHours + ':' + localMinutes + ':' + localSeconds;

    // 更新显示
    this.updateValue('time_utc', utcTime);
    this.updateValue('time_local', localTime);
};

DrmPanel.prototype.updateServices = function(services) {
    var $list = this.$container.find('[data-drm-services]');
    $list.empty();

    if (!services || services.length === 0) return;

    var me = this;
    services.forEach(function(service, index) {
        var $service = $('<div class="drm-service-item"></div>');

        // 服务编号
        var serviceNum = index + 1;

        // 音频编码名称
        var codecName = service.audio_coding != null
            ? me.audioCodecNames[service.audio_coding] || 'Unknown'
            : '--';

        // 比特率
        var bitrate = service.bitrate_kbps != null ? service.bitrate_kbps : service.bitrate;
        var bitrateStr = bitrate != null ? bitrate.toFixed(2) : '--';

        // 服务标签 (需要解码 Unicode 编码)
        var label = me.decodeUnicode(service.label || 'Unknown');

        // 服务 ID
        var serviceId = service.id || '--';

        // 服务类型标识 (Audio/Data)
        var typeIndicator = service.is_audio
            ? '<span class="drm-service-type-badge drm-audio-badge">A</span> '
            : '<span class="drm-service-type-badge">D</span> ';

        // 保护模式 (如果有)
        var protectionMode = service.protection_mode || 'EEP';

        // 语言信息 (如果有) - 只显示语言名称，不显示 FAC:x
        var languageInfo = '';
        if (service.language && service.language.name) {
            languageInfo = '<span class="drm-service-language">' + me.escapeHtml(service.language.name) + '</span> ';
        }

        // 国家信息 (如果有) - 显示 code 和 name
        var countryInfo = '';
        if (service.country) {
            var countryCode = service.country.code || '';
            var countryName = service.country.name || '';
            if (countryCode || countryName) {
                var countryParts = [];
                if (countryCode) countryParts.push(countryCode);
                if (countryName) countryParts.push(countryName);
                countryInfo = '<span class="drm-service-country">(' + me.escapeHtml(countryParts.join(', ')) + ')</span> ';
            }
        }

        // 节目类型信息 (如果有)
        var programTypeInfo = '';
        if (service.program_type) {
            var progParts = [];
            if (service.program_type.id != null) {
                progParts.push('PTY:' + service.program_type.id);
            }
            if (service.program_type.name) {
                progParts.push(me.escapeHtml(service.program_type.name));
            }
            if (progParts.length > 0) {
                programTypeInfo = '<span class="drm-service-progtype">' + progParts.join(' ') + '</span> ';
            }
        }

        // 组装服务行: 序号 Service ID 电台标识 语言 国家 节目类型 编码 比特率 [技术参数]
        // 新顺序: 1 [A] ID:3E9 CNR-1 Chinese (Mandarin) (kp, Korea Democratic Rep.) News xHE-AAC 11.60 kbps [EEP FAC:3 PTY:1]

        // 构建技术参数组 [EEP FAC:3 PTY:1]
        var facInfo = (service.language && service.language.fac_id != null) ? ' FAC:' + service.language.fac_id : '';
        var ptyInfo = (service.program_type && service.program_type.id != null) ? ' PTY:' + service.program_type.id : '';
        var techParamsHtml = '<span class="drm-service-protection">[' + me.escapeHtml(protectionMode) + facInfo + ptyInfo + ']</span>';

        // 文本内容按钮 (如果有 text 字段)
        var textButton = '';
        if (service.text) {
            // 检查是否之前已展开
            var isExpanded = me.textExpandedState[index];
            var btnClass = 'drm-service-text-btn drm-text-available' + (isExpanded ? ' drm-text-expanded' : '');
            var btnText = isExpanded ? 'T↓' : 'T';
            textButton = '<span class="' + btnClass + '" data-service-index="' + index + '">' + btnText + '</span>';
        }

        var serviceHtml =
            '<span class="drm-service-num">' + serviceNum + '</span> ' +
            typeIndicator +
            '<span class="drm-service-id">ID:' + me.escapeHtml(serviceId) + '</span> ' +
            '<span class="drm-service-label">' + me.escapeHtml(label) + '</span> ' +
            languageInfo +
            countryInfo +
            (service.program_type && service.program_type.name ? '<span class="drm-service-progtype">' + me.escapeHtml(service.program_type.name) + '</span> ' : '') +
            '<span class="drm-service-codec">' + me.escapeHtml(codecName) + '</span> ' +
            '<span class="drm-service-bitrate">' + bitrateStr + ' kbps</span> ' +
            techParamsHtml +
            (textButton ? ' ' + textButton : '');

        $service.html(serviceHtml);

        // 如果有文本内容,创建文本展开区域
        if (service.text) {
            var isExpanded = me.textExpandedState[index];
            var displayStyle = isExpanded ? 'display: block;' : 'display: none;';
            var $textContent = $('<div class="drm-service-text-content" data-service-index="' + index + '" style="' + displayStyle + '">' +
                '<span class="drm-text-icon">📄</span> ' +
                '<span class="drm-text-body">' + me.escapeHtml(me.decodeUnicode(service.text)) + '</span>' +
                '</div>');
            $service.append($textContent);
        }

        $list.append($service);
    });
};

DrmPanel.prototype.decodeUnicode = function(str) {
    // 解码 Unicode 编码的字符串 (例如: \u4e2d\u6587 -> 中文)
    if (!str) return '';
    try {
        // 使用 JSON.parse 解码 Unicode 转义序列
        return JSON.parse('"' + str.replace(/"/g, '\\"') + '"');
    } catch (e) {
        // 如果解码失败,返回原字符串
        return str;
    }
};

DrmPanel.prototype.escapeHtml = function(text) {
    if (!text) return '';
    var div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
};

DrmPanel.prototype.toggleTextContent = function(index, $button) {
    // 查找对应的文本内容区域
    var $textContent = this.$container.find('.drm-service-text-content[data-service-index="' + index + '"]');

    if ($textContent.length === 0) return;

    // 切换显示/隐藏
    if ($textContent.is(':visible')) {
        // 收起
        $textContent.slideUp(200);
        $button.removeClass('drm-text-expanded').text('T');
        $button.attr('title', 'Click to view text');
        // 保存状态
        this.textExpandedState[index] = false;
    } else {
        // 展开
        $textContent.slideDown(200);
        $button.addClass('drm-text-expanded').text('T↓');
        $button.attr('title', 'Click to hide text');
        // 保存状态
        this.textExpandedState[index] = true;
    }
};

DrmPanel.prototype.clear = function() {
    // 重置所有指示灯为 off
    this.$container.find('[data-drm-ind]').removeClass('drm-indicator-ok drm-indicator-err').addClass('drm-indicator-off');

    // 重置所有值为 -- (排除 badge 元素,它们有固定的文本内容)
    this.$container.find('[data-drm-val]')
        .not('[data-drm-val="service_count"]')
        .not('.drm-badge')
        .not('.drm-chan-badge')
        .text('--');
    this.$container.find('[data-drm-val="service_count"]').text('0');

    // 移除所有 badge 的激活状态
    this.$container.find('.drm-badge, .drm-chan-badge').removeClass('drm-active');

    // 清空服务列表
    this.$container.find('[data-drm-services]').empty();

    // 隐藏容器（没有信号时）
    this.$container.hide();
};
