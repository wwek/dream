/**
 * DRM Panel Plugin for OpenWebRX+
 *
 * @description Dream DRM 状态显示面板插件
 * @description 实时显示 DRM 解调状态、信号质量、模式信息和服务列表
 * @version 1.0.0
 * @author OpenWebRX+ Community
 * @license AGPL-3.0
 *
 * Features:
 * - Real-time DRM signal monitoring
 * - Status indicators (IO, Time, Frame, FAC, SDC, MSC)
 * - Signal quality metrics (SNR, WMER, Delay, Doppler)
 * - DRM mode and robustness information
 * - Service list with audio codec details
 * - Responsive mobile design
 */

// 设置插件版本（用于依赖检查）
Plugins.drm_panel._version = 1.0;

// 插件配置（可在页面中覆盖）
Plugins.drm_panel.config = {
    // 插件路径（支持自定义路径）
    pluginPath: 'static/plugins/receiver/drm_panel/',

    // 是否启用外部依赖（图片查看器等）
    enableExternalDeps: false,

    // 外部依赖配置
    externalDeps: {
        // 图片查看器（Slideshow 增强）
        imageViewer: {
            enabled: false,
            cssUrl: 'https://cdn.jsdelivr.net/npm/viewerjs@1.11.6/dist/viewer.min.css',
            jsUrl: 'https://cdn.jsdelivr.net/npm/viewerjs@1.11.6/dist/viewer.min.js'
        }
    }
};

// 插件初始化函数（异步）
Plugins.drm_panel.init = async function() {

    // 防止重复初始化
    if (Plugins.drm_panel._initialized) {
        console.warn('[DRM Panel Plugin] Already initialized, skipping duplicate init call');
        return true;
    }

    console.log('[DRM Panel Plugin] Initializing...');

    try {
        // 1. 加载外部依赖（可选，用于增强 Media 内容查看功能）
        if (Plugins.drm_panel.config.enableExternalDeps) {
            await loadExternalDependencies();
        }

        // 2. 检查是否已经有 DrmPanel 类定义（如果从 lib 目录加载）
        var drmPanelExists = typeof DrmPanel !== 'undefined';

        if (drmPanelExists) {
            // 如果已存在，直接注册
            console.log('[DRM Panel Plugin] DrmPanel class already loaded from lib/');
            registerDrmPanel();
            Plugins.drm_panel._initialized = true;
            return true;
        }

        // 3. 动态加载 DrmPanel 类
        console.log('[DRM Panel Plugin] Loading DrmPanel class...');

        // 动态加载 DrmPanel.class.js（使用配置中的路径）
        await Plugins._load_script(Plugins.drm_panel.config.pluginPath + 'DrmPanel.class.js').catch(function() {
            throw new Error('Cannot load DrmPanel.class.js');
        });

        console.log('[DRM Panel Plugin] DrmPanel class loaded successfully');
        registerDrmPanel();
        Plugins.drm_panel._initialized = true;
        return true;

    } catch (error) {
        console.error('[DRM Panel Plugin] Failed to initialize:', error);
        return false;
    }
};

/**
 * 加载外部依赖（可选）
 * 参考 Doppler 插件的依赖加载方式
 *
 * 可选依赖库：
 * - viewer.js: 图片查看器，增强 Slideshow 显示（支持缩放、平移、旋转）
 */
async function loadExternalDependencies() {
    var config = Plugins.drm_panel.config.externalDeps;

    try {
        console.log('[DRM Panel Plugin] Loading external dependencies...');

        // 加载图片查看器（用于 Slideshow 增强）
        if (config.imageViewer && config.imageViewer.enabled) {
            await Plugins._load_style(config.imageViewer.cssUrl).catch(function() {
                console.warn('[DRM Panel Plugin] Cannot load viewer.js CSS, using fallback');
            });

            await Plugins._load_script(config.imageViewer.jsUrl).catch(function() {
                console.warn('[DRM Panel Plugin] Cannot load viewer.js, using fallback image viewer');
            }).then(function() {
                console.log('[DRM Panel Plugin] Loaded external dependency: viewer.js');
            });
        } else {
            console.log('[DRM Panel Plugin] No external dependencies enabled');
        }

        return true;
    } catch (error) {
        console.warn('[DRM Panel Plugin] Failed to load external dependencies:', error);
        // 不影响核心功能，继续初始化
        return true;
    }
}

/**
 * 注册 DrmPanel 到 MetaPanel 系统
 */
function registerDrmPanel() {
    // 等待 OpenWebRX 初始化完成
    $(document).on('event:owrx_initialized', function() {

        // 检查依赖
        if (typeof MetaPanel === 'undefined') {
            console.error('[DRM Panel Plugin] MetaPanel is not available');
            return;
        }

        if (typeof DrmPanel === 'undefined') {
            console.error('[DRM Panel Plugin] DrmPanel class is not loaded');
            return;
        }

        // 1. 动态注入 HTML 容器 (如果不存在)
        if ($('#openwebrx-panel-metadata-drm').length === 0) {
            console.log('[DRM Panel Plugin] Injecting HTML container...');

            // 找到参考元素 (DAB panel)
            var $referencePanel = $('#openwebrx-panel-metadata-dab');

            if ($referencePanel.length > 0) {
                // 在 DAB panel 后面插入 DRM panel
                $('<div class="openwebrx-panel openwebrx-meta-panel" id="openwebrx-panel-metadata-drm" style="display: none;" data-panel-name="metadata-drm"></div>')
                    .insertAfter($referencePanel);
                console.log('[DRM Panel Plugin] HTML container injected successfully');
            } else {
                // 如果找不到 DAB panel，尝试插入到 panels 容器末尾
                var $panelsContainer = $('#openwebrx-panels-container-left');
                if ($panelsContainer.length > 0) {
                    $panelsContainer.append(
                        '<div class="openwebrx-panel openwebrx-meta-panel" id="openwebrx-panel-metadata-drm" style="display: none;" data-panel-name="metadata-drm"></div>'
                    );
                    console.log('[DRM Panel Plugin] HTML container appended to panels container');
                } else {
                    console.error('[DRM Panel Plugin] Cannot find suitable location to inject HTML container');
                    return;
                }
            }
        } else {
            console.log('[DRM Panel Plugin] HTML container already exists');
        }

        // 2. 注册 DRM Panel 到 MetaPanel 系统
        MetaPanel.types = MetaPanel.types || {};

        // 检查是否已经注册
        if (MetaPanel.types.drm) {
            console.log('[DRM Panel Plugin] DrmPanel already registered');
            return;
        }

        MetaPanel.types.drm = function(el) {
            return new DrmPanel(el);
        };

        console.log('[DRM Panel Plugin] Successfully registered to MetaPanel.types.drm');

        // 3. 初始化 metaPanel (确保插件正确初始化)
        $('#openwebrx-panel-metadata-drm').metaPanel();

        // 触发自定义事件，通知其他插件
        $(document).trigger('event:drm_panel_registered');
    });
}

// 提供插件信息查询接口
Plugins.drm_panel.getInfo = function() {
    return {
        name: 'DRM Panel',
        version: Plugins.drm_panel._version,
        description: 'Dream DRM status display panel with real-time signal monitoring',
        author: 'OpenWebRX+ Community',
        dependencies: [],
        features: [
            'Real-time DRM signal monitoring',
            'Status indicators',
            'Signal quality metrics',
            'DRM mode information',
            'Service list display'
        ]
    };
};

// 提供卸载功能（可选）
Plugins.drm_panel.unload = function() {
    if (typeof MetaPanel !== 'undefined' && MetaPanel.types && MetaPanel.types.drm) {
        delete MetaPanel.types.drm;
        Plugins.drm_panel._initialized = false;
        console.log('[DRM Panel Plugin] Unregistered from MetaPanel');
        $(document).trigger('event:drm_panel_unregistered');
    }
};
