# KiwiSDR DRM 实现分析

对比 KiwiSDR 和我们实现的 Dream DRM 状态接口。

## 关键发现

**答案：KiwiSDR 修改了 Dream 后端数据输出格式**

KiwiSDR 没有使用标准的 Dream 输出，而是在 `ConsoleIO.cpp` 中实现了自定义的 JSON 格式。

## KiwiSDR 数据格式

### JSON 结构（来自 ConsoleIO.cpp）

```json
{
  "io": 0,
  "time": 0,
  "frame": 0,
  "FAC": 0,
  "SDC": 0,
  "MSC": 0,
  "if": -34.7,
  "snr": 10.6,
  "mod": 0,
  "occ": 3,
  "ilv": 0,
  "sdc": 0,
  "msc": 1,
  "plb": 0,
  "pla": 0,
  "nas": 1,
  "nds": 1,
  "epg": 0,
  "svc": [
    {
      "cur": 1,
      "dat": -1,
      "ac": 3,
      "id": "3EA",
      "lbl": "CNR-1",
      "ep": 0.0,
      "ad": 1,
      "br": 11.60
    }
  ],
  "msg": "Now playing..."
}
```

### 字段对比

| KiwiSDR | 我们的实现 | 说明 |
|---------|-----------|------|
| `io`, `time`, `frame`, `FAC`, `SDC`, `MSC` | `status.io`, `status.time`, `status.frame`, `status.fac`, `status.sdc`, `status.msc` | ✅ 相同概念 |
| `if` | `signal.if_level_db` | ✅ IF 信号电平 |
| `snr` | `signal.snr_db` | ✅ 信噪比 |
| `mod` | `mode.robustness` | ✅ 鲁棒性模式 (0=A, 1=B, 2=C, 3=D) |
| `occ` | `mode.bandwidth` | ✅ 带宽索引 (0-5) |
| `ilv` | `mode.interleaver` | ✅ 交织器 (0=Long, 1=Short) |
| `sdc` | `coding.sdc_qam` | ✅ SDC QAM 模式 |
| `msc` | `coding.msc_qam` | ✅ MSC QAM 模式 |
| `pla`, `plb` | `coding.protection_a`, `coding.protection_b` | ✅ 保护等级 |
| `nas`, `nds` | `services.audio`, `services.data` | ✅ 服务数量 |
| `svc[].cur` | - | ❌ KiwiSDR 特有：当前选择的服务 |
| `svc[].dat` | - | ❌ KiwiSDR 特有：数据服务状态 |
| `svc[].ac` | `service_list[].audio_coding` | ✅ 音频编码类型 |
| `svc[].id` | `service_list[].id` | ✅ 服务 ID（十六进制） |
| `svc[].lbl` | `service_list[].label` | ✅ 服务名称（URL 编码） |
| `svc[].ep` | - | ❌ KiwiSDR 特有：UEP 百分比 |
| `svc[].ad` | `service_list[].is_audio` | ✅ 音频/数据标志 |
| `svc[].br` | `service_list[].bitrate_kbps` | ✅ 比特率 |
| `msg` | `service_list[].text` | ⚠️ **关键区别** |

## 关键区别

### 1. 文本消息位置

**KiwiSDR**:
- 顶层字段 `"msg"`: 当前音频服务的文本消息（全局）
- 来源：`service.AudioParam.strTextMessage`

**我们的实现**:
- 每个服务的 `"text"` 字段：服务级别的文本消息
- 更符合 DRM 规范（每个服务可以有独立的文本）

### 2. 服务选择

**KiwiSDR**:
- `svc[].cur`: 标记当前选择的音频服务
- `svc[].dat`: 当前选择的数据服务状态
- 前端需要知道用户选择了哪个服务

**我们的实现**:
- 不关心服务选择（由前端/用户决定）
- 只提供所有服务的状态信息

### 3. UEP 支持

**KiwiSDR**:
- `svc[].ep`: UEP (Unequal Error Protection) 百分比
- 计算自 `Parameters.PartABLenRatio(i) * 100`

**我们的实现**:
- 只提供 `protection_a` 和 `protection_b`
- 不提供 UEP/EEP 百分比信息

## 前端实现对比

### KiwiSDR 前端（DRM.js）

```javascript
case "drm_status_cb":
    var deco = kiwi_decodeURIComponent('DRM', param[1]);
    var o = kiwi_JSON_parse('drm_status_cb', deco);

    // 状态指示灯
    drm_all_status(o.io, o.time, o.frame, o.FAC, o.SDC, o.MSC);

    // 信号质量
    w3_innerHTML('id-drm-if_level', 'IF Level: '+ o.if.toFixed(1) +' dB');
    w3_innerHTML('id-drm-snr', o.snr? ('SNR: '+ o.snr.toFixed(1) +' dB') : '');

    // 模式和调制
    w3_color('id-drm-mod-'+ ['A','B','C','D'][o.mod], 'black', 'lime');
    w3_color('id-drm-occ-'+ ['4.5','5','9','10','18','20'][o.occ], 'black', 'lime');
    w3_color('id-drm-ilv-'+ ['L','S'][o.ilv], 'black', 'lime');
    w3_color('id-drm-sdc-'+ ['4','16'][o.sdc], 'black', 'lime');
    w3_color('id-drm-msc-'+ ['16','64'][o.msc-1], 'black', 'lime');

    // 保护等级
    w3_innerHTML('id-drm-prot', sprintf('Protect: A=%d B=%d', o.pla, o.plb));

    // 服务统计
    w3_innerHTML('id-drm-nsvc', sprintf('Services: A=%d D=%d', o.nas, o.nds));

    // 服务列表
    o.svc.forEach(function(ao, i) {
        var label = kiwi_clean_html(kiwi_clean_newline(decodeURIComponent(ao.lbl)));
        s = drm.EAudCod[ao.ac] + ' (' + ao.id + ') ' + label +
            (ao.ep? (' UEP (' + ao.ep.toFixed(1) + '%) ') : ' EEP ') +
            (ao.ad? 'Audio ':'Data ') + ao.br.toFixed(2) + ' kbps';
        w3_color(el, null, ao.cur? 'mediumSlateBlue' : '');
    });

    // 文本消息（全局）
    w3_innerHTML('id-drm-msgs', o.msg? kiwi_clean_html(kiwi_clean_newline(decodeURIComponent(o.msg))) : '');
```

**关键特点**：
- URL 编码/解码：`decodeURIComponent(ao.lbl)`, `decodeURIComponent(o.msg)`
- HTML 清理：`kiwi_clean_html(kiwi_clean_newline(...))`
- 颜色高亮当前服务：`ao.cur? 'mediumSlateBlue' : ''`
- UEP/EEP 显示：`ao.ep? (' UEP (' + ao.ep.toFixed(1) + '%) ') : ' EEP '`

### 我们的实现建议（基于 INTEGRATION.md）

```javascript
class DrmPanel {
    update(status) {
        // 状态指示灯
        html += this.indicator(s.status.io, 'IO');
        html += this.indicator(s.status.time, 'Time');
        html += this.indicator(s.status.frame, 'Frame');
        html += this.indicator(s.status.fac, 'FAC');
        html += this.indicator(s.status.sdc, 'SDC');
        html += this.indicator(s.status.msc, 'MSC');

        // 信号质量
        html += `<div>IF: ${s.signal.if_level_db.toFixed(1)} dB</div>`;
        html += `<div>SNR: ${s.signal.snr_db.toFixed(1)} dB</div>`;

        // 模式信息
        const modeStr = ['A', 'B', 'C', 'D'][s.mode.robustness];
        html += `<div>Mode: ${modeStr}</div>`;
        html += `<div>BW: ${s.mode.bandwidth_khz} kHz</div>`;

        // 服务列表
        s.service_list.forEach(svc => {
            const codec = svc.is_audio ?
                ['AAC', 'OPUS', 'RESERVED', 'xHE-AAC'][svc.audio_coding] : 'Data';
            html += `<strong>${svc.label}</strong> (${svc.id})`;
            html += `<br>${codec} @ ${svc.bitrate_kbps.toFixed(2)} kbps`;
            if (svc.text) html += `<br>${svc.text}`;
        });
    }
}
```

**关键特点**：
- 直接使用 JSON 数据（无需解码）
- 更清晰的结构化数据
- 服务级别的文本消息（更符合 DRM 规范）

## 后端实现对比

### KiwiSDR 后端（ConsoleIO.cpp）

```cpp
// 构建 JSON 字符串
sb = kstr_asprintf(sb,
    "{\"io\":%d,\"time\":%d,\"frame\":%d,\"FAC\":%d,\"SDC\":%d,\"MSC\":%d,\"if\":%.1f",
    io, time, frame, FAC, SDC, MSC, if_level);

if (signal) {
    sb = kstr_asprintf(sb,
        ",\"snr\":%.1f,\"mod\":%d,\"occ\":%d,\"ilv\":%d,\"sdc\":%d,\"msc\":%d,\"plb\":%d,\"pla\":%d,\"nas\":%d,\"nds\":%d",
        rSNR, mod, occ, ilv, sdc, msc, plb, pla, nas, nds);
}

sb = kstr_cat(sb, ",\"svc\":[");
for (int i = 0; i < MAX_NUM_SERVICES; i++) {
    if (service.IsActive()) {
        char *strLabel = kiwi_str_encode((char *) service.strLabel.c_str());
        sb = kstr_asprintf(sb,
            "\"cur\":%d,\"dat\":%d,\"ac\":%d,\"id\":\"%X\",\"lbl\":\"%s\",\"ep\":%.1f,\"ad\":%d,\"br\":%.2f",
            (i == iCurAudService)? 1:0,
            (i == iCurDataService)? datStat : -1,
            ac, ID, strLabel, rPartABLenRat * 100, br_audio? 1:0, rBitRate
        );
        free(strLabel);
    }
}

if (signal && text_msg_utf8_enc)
    sb = kstr_asprintf(sb, ",\"msg\":\"%s\"", text_msg_utf8_enc);

// 通过 WebSocket 发送
DRM_msg_encoded(DRM_MSG_STATUS, "drm_status_cb", sb);
```

**关键特点**：
- 手动构建 JSON 字符串（不使用 JSON 库）
- URL 编码标签：`kiwi_str_encode()`
- 通过 KiwiSDR 自定义消息系统发送：`DRM_msg_encoded()`

### 我们的实现（StatusBroadcast.cpp）

```cpp
// 使用 nlohmann/json 库构建结构化数据
json j;
j["timestamp"] = time(nullptr);

// 状态
j["status"]["io"] = io_status;
j["status"]["time"] = time_status;
j["status"]["frame"] = frame_status;
j["status"]["fac"] = fac_status;
j["status"]["sdc"] = sdc_status;
j["status"]["msc"] = msc_status;

// 信号
j["signal"]["if_level_db"] = if_level;
j["signal"]["snr_db"] = snr;

// 模式（仅有信号时）
if (signal) {
    j["mode"]["robustness"] = robustness;
    j["mode"]["bandwidth"] = bandwidth;
    j["mode"]["bandwidth_khz"] = bandwidth_khz;
    j["mode"]["interleaver"] = interleaver;
}

// 服务列表
json services = json::array();
for (int i = 0; i < MAX_NUM_SERVICES; i++) {
    if (service.IsActive()) {
        json svc;
        svc["id"] = service_id_hex;
        svc["label"] = service.strLabel;  // 直接 UTF-8，无编码
        svc["is_audio"] = is_audio;
        if (is_audio) svc["audio_coding"] = audio_coding;
        svc["bitrate_kbps"] = bitrate;
        if (has_text) svc["text"] = service.AudioParam.strTextMessage;
        services.push_back(svc);
    }
}
j["service_list"] = services;

// 通过 Unix Domain Socket 广播
std::string json_str = j.dump() + "\n";
broadcast(json_str);
```

**关键特点**：
- 使用 JSON 库（更安全、更易维护）
- 直接 UTF-8 字符串（无需编码）
- 通过 Unix Domain Socket 广播（标准 IPC）
- 服务级别的文本消息（更符合规范）

## 优势对比

### KiwiSDR 方式

**优点**：
- 集成到 KiwiSDR 的 WebSocket 消息系统
- 当前服务选择状态（`cur`, `dat`）
- UEP 百分比信息（`ep`）

**缺点**：
- 手动构建 JSON（易出错）
- 需要 URL 编码/解码
- 紧耦合到 KiwiSDR 架构
- 文本消息全局化（不符合 DRM 规范）

### 我们的方式

**优点**：
- 使用 JSON 库（安全、标准）
- 直接 UTF-8（无需编码）
- Unix Domain Socket（标准 IPC，解耦）
- 服务级别文本消息（符合 DRM 规范）
- 多用户支持（多实例 socket）
- 独立于 OpenWebRX 架构

**缺点**：
- 不包含服务选择状态（由前端管理）
- 不包含 UEP 百分比（可添加）

## 集成建议

### OpenWebRX 集成策略

基于 KiwiSDR 的经验，我们应该：

1. **保持 Unix Domain Socket 架构**
   - 解耦设计优于紧耦合
   - 支持多用户（每个用户独立 socket）

2. **前端直接使用 JSON**
   - 无需 URL 编码/解码
   - 更简单的数据处理

3. **考虑添加字段**（如果需要）：
   - `service_list[].current`: 标记用户选择的服务
   - `service_list[].uep_percent`: UEP 百分比（如果需要）

4. **WebSocket 集成**（参考 INTEGRATION.md）：
   ```python
   class DrmStatusMonitor(threading.Thread):
       def _process_status(self, json_str):
           status = json.loads(json_str)
           for callback in self.callbacks:
               callback(status)  # 直接传递，无需编码
   ```

5. **前端面板**（参考 KiwiSDR UI）：
   - 状态指示灯：绿色（正常）、黄色（警告）、红色（错误）、灰色（未同步）
   - 信号质量图表：IF Level, SNR 实时曲线
   - 模式信息：高亮当前模式、带宽、交织器
   - 服务列表：显示所有服务，高亮当前播放

## 结论

**KiwiSDR 修改了后端数据输出**，在 `ConsoleIO.cpp` 中实现了自定义 JSON 格式，通过 KiwiSDR 的 WebSocket 消息系统发送。

**我们的实现更优**：
- ✅ 标准化的 JSON 格式
- ✅ 解耦的 Unix Domain Socket 架构
- ✅ 多用户支持
- ✅ 符合 DRM 规范的服务级别文本消息
- ✅ 更易维护和扩展

**下一步**：按照 INTEGRATION.md 实现 OpenWebRX 集成，参考 KiwiSDR 前端的 UI 设计和用户体验。
