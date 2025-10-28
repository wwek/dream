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

    // Media 内容缓存 (LRU cache, 最多5条, 1小时过期)
    this.mediaCache = {
        maxSize: 5,
        maxAge: 3600000,  // 1 hour in ms
        data: new Map(),

        set: function(key, value) {
            // LRU: 删除最旧的条目
            if (this.data.size >= this.maxSize) {
                var firstKey = this.data.keys().next().value;
                this.data.delete(firstKey);
            }
            this.data.set(key, {
                content: value,
                timestamp: Date.now()
            });
        },

        get: function(key) {
            var item = this.data.get(key);
            if (!item) return null;

            // 检查过期
            if (Date.now() - item.timestamp > this.maxAge) {
                this.data.delete(key);
                return null;
            }

            // LRU: 移到最后
            this.data.delete(key);
            this.data.set(key, item);
            return item.content;
        },

        clear: function() {
            this.data.clear();
        }
    };

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

            // 第二行：信号质量（横向显示，SNR 优先）
            '<div class="drm-signal-row">' +
                '<span class="drm-signal-item">SNR: <span class="drm-value drm-snr" data-drm-val="snr">--</span> dB</span>' +
                '<span class="drm-signal-item">IF: <span class="drm-value" data-drm-val="if_level">--</span> dB</span>' +
                '<span class="drm-signal-item">WMER/MER: <span class="drm-value" data-drm-val="wmer">--</span>/<span class="drm-value" data-drm-val="mer">--</span> dB</span>' +
                '<span class="drm-signal-item">Doppler/Delay: <span class="drm-value" data-drm-val="doppler">--</span>/<span class="drm-value" data-drm-val="delay">--</span></span>' +
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

            // Media 内容行（Program Guide, Journaline, Slideshow）- 使用方框指示灯
            '<div class="drm-media-row">' +
                '<span class="drm-label">Media:</span> ' +
                '<span class="drm-media-item drm-media-clickable" data-media-type="program_guide">' +
                    '<span class="drm-media-box drm-media-off" data-drm-media="program_guide"></span>' +
                    '<span class="drm-media-text">Program Guide</span>' +
                '</span>' +
                '<span class="drm-media-item drm-media-clickable" data-media-type="journaline">' +
                    '<span class="drm-media-box drm-media-off" data-drm-media="journaline"></span>' +
                    '<span class="drm-media-text">Journaline®</span>' +
                '</span>' +
                '<span class="drm-media-item drm-media-clickable" data-media-type="slideshow">' +
                    '<span class="drm-media-box drm-media-off" data-drm-media="slideshow"></span>' +
                    '<span class="drm-media-text">Slideshow</span>' +
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

    // 添加 Media 点击事件委托
    this.$container.on('click', '.drm-media-clickable', function() {
        var mediaType = $(this).data('media-type');
        var isAvailable = $(this).find('.drm-media-box').hasClass('drm-media-on');
        me.showMediaContent(mediaType, isAvailable);
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

    // 更新 SNR（保持绿色点亮，不分级）
    this.updateValue('snr', snr != null ? snr.toFixed(1) : '--');
    this.updateValue('if_level', ifLevel != null ? ifLevel.toFixed(1) : '--');

    // 更新扩展信号参数
    var wmer = status.signal && status.signal.wmer_db != null ? status.signal.wmer_db : null;
    var mer = status.signal && status.signal.mer_db != null ? status.signal.mer_db : null;
    var doppler = status.signal && status.signal.doppler_hz != null ? status.signal.doppler_hz : null;
    var delayMin = status.signal && status.signal.delay_min_ms != null ? status.signal.delay_min_ms : null;

    // 更新 WMER/MER（与界面一致的格式）
    this.updateValue('wmer', wmer != null ? wmer.toFixed(1) : '--');
    this.updateValue('mer', mer != null ? mer.toFixed(1) : '--');

    // 更新 Doppler/Delay（与界面一致：仅显示最小delay）
    this.updateValue('doppler', doppler != null ? doppler.toFixed(2) + ' Hz' : '--');
    this.updateValue('delay', delayMin != null ? delayMin.toFixed(2) + ' ms' : '--');

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

    // 更新时间显示 (从 received_time 字段获取 DRM 传输时间)
    // 如果 received_time 为 0 或不存在，则不显示时间
    if (status.received_time && status.received_time !== 0) {
        this.updateTime(status.received_time);
    } else {
        // 时间不可用时显示提示
        this.updateValue('time_utc', 'Service not available');
        this.updateValue('time_local', 'Service not available');
    }

    // 更新 Media 状态 (Program Guide, Journaline, Slideshow)
    if (status.media) {
        this.updateMediaIndicator('program_guide', status.media.program_guide);
        this.updateMediaIndicator('journaline', status.media.journaline);
        this.updateMediaIndicator('slideshow', status.media.slideshow);
    }

    // 处理 Media 内容 (一次性推送,客户端缓存)
    if (status.media_content) {
        this.handleMediaContent(status.media_content);
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

DrmPanel.prototype.updateMediaIndicator = function(mediaType, isAvailable) {
    var $box = this.$container.find('[data-drm-media="' + mediaType + '"]');
    if ($box.length > 0) {
        $box.removeClass('drm-media-on drm-media-off');
        if (isAvailable) {
            $box.addClass('drm-media-on');  // 绿色方框
        } else {
            $box.addClass('drm-media-off');  // 灰色方框
        }
    }
};

// 缓存单个 Media 项
DrmPanel.prototype.cacheMediaItem = function(type, content) {
    if (content) {
        var key = type + '_' + content.timestamp;
        this.mediaCache.set(key, content);

        // 构建日志信息
        var logInfo = '[DrmPanel] Cached ' + type;
        if (content.name) {
            logInfo += ': ' + content.name;
        }
        if (content.size) {
            logInfo += ' (size: ' + content.size + ' bytes)';
        }
        console.log(logInfo);
    }
};

// 处理 Media 内容 (一次性推送,客户端缓存)
DrmPanel.prototype.handleMediaContent = function(mediaContent) {
    this.cacheMediaItem('slideshow', mediaContent.slideshow);
    this.cacheMediaItem('program_guide', mediaContent.program_guide);
    this.cacheMediaItem('journaline', mediaContent.journaline);
};

// 展示 Media 内容 (KISS 方案: 简单模态框)
DrmPanel.prototype.showMediaContent = function(mediaType, isAvailable) {
    var self = this;

    // 如果没有内容,显示提示
    if (!isAvailable) {
        this.showSimpleAlert('No ' + this.getMediaTypeName(mediaType) + ' available', 'info');
        return;
    }

    // 从缓存获取最新内容 (比较内容自身的时间戳,而非缓存时间戳)
    var latestContent = null;
    var latestTimestamp = 0;
    this.mediaCache.data.forEach(function(item, key) {
        if (key.startsWith(mediaType + '_')) {
            // 使用内容自身的时间戳 (Unix 秒), 而非缓存条目的时间戳 (毫秒)
            var contentTimestamp = item.content.timestamp || 0;
            if (contentTimestamp > latestTimestamp) {
                latestTimestamp = contentTimestamp;
                latestContent = item.content;
            }
        }
    });

    if (!latestContent) {
        this.showSimpleAlert('No ' + this.getMediaTypeName(mediaType) + ' content cached yet', 'info');
        return;
    }

    // 根据类型显示内容
    switch (mediaType) {
        case 'slideshow':
            this.showSlideshowModal(latestContent);
            break;
        case 'program_guide':
            this.showProgramGuideModal(latestContent);
            break;
        case 'journaline':
            this.showJournalineModal(latestContent);
            break;
    }
};

// 获取 Media 类型名称
DrmPanel.prototype.getMediaTypeName = function(mediaType) {
    var names = {
        'slideshow': 'Slideshow',
        'program_guide': 'Program Guide',
        'journaline': 'Journaline'
    };
    return names[mediaType] || mediaType;
};

// 显示 Slideshow 模态框
DrmPanel.prototype.showSlideshowModal = function(content) {
    var timeStr = new Date(content.timestamp * 1000).toLocaleString();
    var sizeKB = (content.size / 1024).toFixed(2);

    var html = '<div class="drm-media-modal-overlay">' +
        '<div class="drm-media-modal">' +
            '<div class="drm-media-modal-header">' +
                '<h3>📷 Slideshow</h3>' +
                '<button class="drm-media-modal-close">&times;</button>' +
            '</div>' +
            '<div class="drm-media-modal-body">' +
                '<div class="drm-media-preview">' +
                    '<img src="data:' + content.mime + ';base64,' + content.data + '" alt="' + content.name + '">' +
                '</div>' +
                '<div class="drm-media-info">' +
                    '<p><strong>Name:</strong> ' + content.name + '</p>' +
                    '<p><strong>Size:</strong> ' + sizeKB + ' KB</p>' +
                    '<p><strong>Type:</strong> ' + content.mime + '</p>' +
                    '<p><strong>Time:</strong> ' + timeStr + '</p>' +
                '</div>' +
            '</div>' +
            '<div class="drm-media-modal-footer">' +
                '<button class="drm-media-download-btn" data-filename="' + content.name + '" data-mime="' + content.mime + '" data-base64="' + content.data + '">💾 Download</button>' +
                '<button class="drm-media-modal-close">Close</button>' +
            '</div>' +
        '</div>' +
    '</div>';

    this.showModal(html);
};

// 显示 Program Guide 模态框
DrmPanel.prototype.showProgramGuideModal = function(content) {
    var timeStr = new Date(content.timestamp * 1000).toLocaleString();
    var sizeKB = (content.size / 1024).toFixed(2);

    var html = '<div class="drm-media-modal-overlay">' +
        '<div class="drm-media-modal">' +
            '<div class="drm-media-modal-header">' +
                '<h3>📺 Program Guide</h3>' +
                '<button class="drm-media-modal-close">&times;</button>' +
            '</div>' +
            '<div class="drm-media-modal-body">' +
                '<div class="drm-media-info">' +
                    '<p><strong>Name:</strong> ' + content.name + '</p>' +
                    '<p><strong>Description:</strong> ' + content.description + '</p>' +
                    '<p><strong>Size:</strong> ' + sizeKB + ' KB</p>' +
                    '<p><strong>Time:</strong> ' + timeStr + '</p>' +
                '</div>' +
                '<div class="drm-media-text-content">' +
                    '<pre>' + atob(content.data) + '</pre>' +
                '</div>' +
            '</div>' +
            '<div class="drm-media-modal-footer">' +
                '<button class="drm-media-modal-close">Close</button>' +
            '</div>' +
        '</div>' +
    '</div>';

    this.showModal(html);
};

// 显示 Journaline 模态框
DrmPanel.prototype.showJournalineModal = function(content) {
    var timeStr = new Date(content.timestamp * 1000).toLocaleString();

    var html = '<div class="drm-media-modal-overlay">' +
        '<div class="drm-media-modal">' +
            '<div class="drm-media-modal-header">' +
                '<h3>📰 Journaline</h3>' +
                '<button class="drm-media-modal-close">&times;</button>' +
            '</div>' +
            '<div class="drm-media-modal-body">' +
                '<div class="drm-media-info">' +
                    '<p><strong>Time:</strong> ' + timeStr + '</p>' +
                    '<p><strong>Status:</strong> Available</p>' +
                '</div>' +
                '<div class="drm-media-text-content">' +
                    '<p>Journaline content viewer - Full implementation pending</p>' +
                '</div>' +
            '</div>' +
            '<div class="drm-media-modal-footer">' +
                '<button class="drm-media-modal-close">Close</button>' +
            '</div>' +
        '</div>' +
    '</div>';

    this.showModal(html);
};

// 下载 Media 文件
DrmPanel.prototype.downloadMediaFile = function(filename, mime, base64) {
    try {
        // 将 Base64 转换为 Blob
        var byteCharacters = atob(base64);
        var byteNumbers = new Array(byteCharacters.length);
        for (var i = 0; i < byteCharacters.length; i++) {
            byteNumbers[i] = byteCharacters.charCodeAt(i);
        }
        var byteArray = new Uint8Array(byteNumbers);
        var blob = new Blob([byteArray], { type: mime });

        // 创建下载链接
        var url = URL.createObjectURL(blob);
        var a = document.createElement('a');
        a.href = url;
        a.download = filename;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);

        console.log('[DrmPanel] Downloaded:', filename);
    } catch (e) {
        console.error('[DrmPanel] Failed to download file:', e);
        alert('Failed to download file: ' + e.message);
    }
};

// 显示模态框
DrmPanel.prototype.showModal = function(html) {
    var self = this;
    var $modal = $(html);
    $('body').append($modal);

    // 点击下载按钮
    $modal.find('.drm-media-download-btn').on('click', function() {
        var filename = $(this).data('filename');
        var mime = $(this).data('mime');
        var base64 = $(this).data('base64');
        self.downloadMediaFile(filename, mime, base64);
    });

    // 点击关闭按钮或遮罩层关闭
    $modal.find('.drm-media-modal-close').on('click', function() {
        $modal.remove();
    });
    $modal.on('click', function(e) {
        if ($(e.target).hasClass('drm-media-modal-overlay')) {
            $modal.remove();
        }
    });

    // ESC 键关闭
    $(document).on('keydown.drm-modal', function(e) {
        if (e.key === 'Escape') {
            $modal.remove();
            $(document).off('keydown.drm-modal');
        }
    });
};

// 显示简单提示
DrmPanel.prototype.showSimpleAlert = function(message, type) {
    var icon = type === 'info' ? 'ℹ️' : '⚠️';
    var html = '<div class="drm-media-modal-overlay">' +
        '<div class="drm-media-modal drm-media-modal-small">' +
            '<div class="drm-media-modal-body">' +
                '<p style="text-align:center;font-size:14px;">' + icon + ' ' + message + '</p>' +
            '</div>' +
            '<div class="drm-media-modal-footer">' +
                '<button class="drm-media-modal-close">OK</button>' +
            '</div>' +
        '</div>' +
    '</div>';

    this.showModal(html);
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

        // 保护模式 (EEP/UEP)
        var protectionMode = service.protection_mode || 'EEP';
        var protectionDisplay = protectionMode;
        if (protectionMode === 'UEP' && service.protection_percent != null) {
            protectionDisplay = protectionMode + ' (' + service.protection_percent.toFixed(1) + '%)';
        }

        // 音频模式 (Mono/Stereo/P-Stereo)
        var audioMode = service.audio_mode || '';

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
            '<span class="drm-service-bitrate">' + bitrateStr + ' kbps</span> ' +
            '<span class="drm-service-protection">' + protectionDisplay + '</span> ' +
            '<span class="drm-service-codec">' + me.escapeHtml(codecName) + '</span> ' +
            (audioMode ? '<span class="drm-service-audiomode">' + me.escapeHtml(audioMode) + '</span> ' : '') +
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
