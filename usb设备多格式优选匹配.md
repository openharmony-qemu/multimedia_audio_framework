# USB 设备多格式优选匹配

## 一、需求分析

### 1.1 背景

当前无论连接何种 ARM USB 设备,底层 HAL 在设备上线时只上报给框架固定的单一格式:采样率(48k)、位宽(16bit)、声道(双声道)。框架也只能按照这个固定格式来打开通路。而许多专业 USB 设备支持更高清的格式(> 48k,甚至 192k/32bit),这导致当应用播放高清音频时,即便连接的 USB 设备支持对应的采样率等,经过框架后也无法做到"无损"体验。

### 1.2 目标

联合底层 HAL(底层上报 USB 设备支持的所有格式参数),对能支持更高音频格式规格的 USB 设备,根据应用播放的音频格式来动态调整框架通路的格式。

### 1.3 预期表现

- USB 设备上线时,查询解析出其支持的所有格式并保存。
- 当应用播放高清资源(比如 192k 24bit)时,与设备支持的格式进行优选匹配。
- 比如设备支持 44.1k、48k、96k、192k,16bit、24bit、32bit,则选用 192k + 24bit 来打开 USB 设备通路。

### 1.4 并发管控补充需求

USB 设备也支持低时延(fast)通路。当 USB 设备上已有 fast 流在运行时,新增高规格音频流(采样率 > 48000)需要触发低时延流回落到普通通路。

### 1.5 已有核心处理逻辑

1. USB 设备支持格式解析核心代码:`AudioDeviceDescriptor::UpdateDeviceInfoByAudioParameters`——当前仍以固定的单个格式进行解析。
2. 打开通路过程核心函数:`AudioCoreService::ActivateOutputDevice` → `AudioArmDeviceBehavior::ActivateDevice`,通路打开核心函数 `OpenPortAndInsertIOHandle`。
3. 每条流创建必走流程:`AudioPolicyServer::CreateRendererClient`,其中建立通路与重建通路过程是重点。

---

## 二、方案分析

### 2.1 当前实现链路(端到端)

#### 2.1.1 设备上线格式解析

- 入口:`AudioDeviceStatus::HandleLocalDeviceConnected`(`services/audio_policy/server/domain/device/src/audio_device_status.cpp:432`)
- 对 `DEVICE_TYPE_USB_ARM_HEADSET`,调用 `GetAudioParameterProxy(LOCAL_NETWORK_ID, USB_DEVICE, condition)` 获取 HAL 参数
- HAL 参数形如:`sink_rate:48000;sink_format:AUDIO_FORMAT_PCM_16_BIT;sink_chans:2;sink_mmap:1;`
- `AudioDeviceDescriptor::UpdateDeviceInfoByAudioParameters`(`audio_device_descriptor.cpp:399`)按 `;` 分割、`:` 取 kv,每个 key 单值,产出一条 `DeviceStreamInfo`,`audioStreamInfo_` clear 后只 push_back 一条
- `BuildCapabilitiesFromDeviceStreamInfo`(`audio_device_descriptor.cpp:911`)对 USB arm 逐条生成 capability,但只有一条

#### 2.1.2 USB arm 端口首次打开(设备连接时)

```
OnPreferredOutputDeviceUpdated → FetchAndActivateOutputDevice → ActivateOutputDevice
  (audio_core_service_private.cpp:3401)
  → AudioDeviceFactory::ActivateDevice (audio_device_factory.cpp:54)
    → AudioArmDeviceBehavior::ActivateDevice (audio_arm_device_behavior.cpp:84)
      → ProcessModuleActivation (:203)
        → UpdateArmModuleInfo (:157)
          取 deviceDesc->GetAudioStreamInfo().back() 的:
            rate   = *samplingRate.begin()   ← 只取第一个采样率
            format = deviceStreamInfo.format ← 单值
            channel= GetMainChannelLayout()  ← 只取第一个
          填入 moduleInfo.rate/format/channels/bufferSize
        → OpenPortAndInsertIOHandle (audio_iohandle_map.cpp:199)
          以 moduleInfo 单一格式打开 HDI 端口
```

关键:USB arm 端口在**设备连接时**以固定格式打开,此时**无应用流上下文**。

#### 2.1.3 应用流创建通路选择(CreateRendererClient)

```
AudioPolicyServer::CreateRendererClient (audio_policy_server.cpp:3587)
  → EventEntry::CreateRendererClient (audio_core_service_entry.cpp:121) [eventMutex_ 锁内]
    → AudioCoreService::CreateRendererClient (audio_core_service.cpp:722)
      ├─ UpdateStreamDevicesForCreate (core_service_private.cpp:3591)
      │    → audioRouterCenter_.FetchOutputDevices → 设备描述符携带原始 audioStreamInfo_
      ├─ [仅 A2DP] ActivateOutputDevice (:754) ← USB arm 不在此激活!
      └─ FetchRendererPipeAndExecute (core_service_private.cpp:997)
           → audioPipeSelector_->FetchPipeAndExecute (pipe_selector.cpp:691)
             ├─ ScanPipeListForStreamDesc → DecideFinalRouteFlag → ProcessConcurrency [并发判定]
             ├─ GetStreamPropInfoWithBusSelector → GetStreamPropInfo (config_manager.cpp:870)
             │    → temp = desc->streamInfo_ 与 XML 配置 pipe streamPropInfos 匹配
             │      ⚠ 不参考设备 HAL 能力!USB arm 的 XML 配置通常是固定 48k/16bit
             ├─ UpdateDeviceStreamInfo (pipe_selector.cpp:769)  ⚠ 关键覆写点!
             │    → temp->audioStreamInfo_ = {streamPropInfo 派生的单格式}
             │      设备描述符的多格式能力被覆写为 streamPropInfo 单值!
             ├─ FindExistingPipe (pipe_selector.cpp ~2300)
             │    → 已有 USB pipe → MatchFastArmusbPipe (:2395)
             │      ⚠ 仅当 deviceDesc->macAddress_ != pipeInfo->moduleInfo_.macAddress
             │        即"设备 MAC 变化"才 reload;格式变化不触发!
             │    → AddStreamToPipe(pipeInfo, streamDesc, PIPE_ACTION_UPDATE) 加流,不重开
             └─ 无匹配 → ConvertStreamDescToPipeInfo → PIPE_ACTION_NEW
                  → UpdateModuleInfoWithDevice (pipe_selector.cpp:2192)
                    ⚠ 读 deviceDesc->audioStreamInfo_.back()(已被覆写为 streamPropInfo 值)
           → 按 pipeAction 分发:
             NEW → ProcessOutputPipeNew → OpenNewAudioPortAndRoute
             UPDATE → ProcessOutputPipeUpdate → 加流
             RELOAD → ProcessOutputPipeReload (:869)
               ⚠ 检查 engineFlag==1(ProAudio),非 ProAudio 直接 return
```

### 2.2 关键发现

#### 发现 1:数据结构已部分具备多格式能力

| 数据结构 | 现状 | 多格式支持 |
| --- | --- | --- |
| `audioStreamInfo_` (`std::list<DeviceStreamInfo>`) | 解析时 clear 后只 push_back 1 条 | 结构支持多条,解析未用 |
| `DeviceStreamInfo.samplingRate` (`std::set`) | 解析只 insert 1 个 | 结构支持多个 |
| `DeviceStreamInfo.channelLayout` (`std::set`) | 解析只 insert 1 个 | 结构支持多个 |
| `DeviceStreamInfo.format` (`AudioSampleFormat` 单值) | 单值 | 不支持多 format,需新增 set |
| `AudioModuleInfo.supportedRate_` / `supportedChannelLayout_` (`std::set`) | 配置 XML 解析时填充,USB 动态通路未使用 | 已有基础设施,未接入 |

#### 发现 2:三个核心瓶颈

1. **HAL 上报契约是单值**:`UpdateDeviceInfoByAudioParameters` 假定每个 key 只有一个值。
2. **应用格式未参与通路打开**:`ProcessModuleActivation`/`UpdateArmModuleInfo` 只接收 `deviceDesc`,丢失了 `streamDesc->streamInfo_`(应用请求的 192k/24bit)。同样的单格式提取逻辑还出现在 `AudioPipeSelector::UpdateModuleInfoWithDevice`(pipe_selector.cpp:2109)。
3. **缺少格式优选匹配函数**:全仓库无 `SelectBestFormat`/`MatchFormat` 类逻辑。

#### 发现 3:与原设想的主要出入

1. **USB arm 端口的首次打开不在 CreateRendererClient 中**:端口在设备连接时经 `OnPreferredOutputDeviceUpdated → ActivateOutputDevice → AudioArmDeviceBehavior::ActivateDevice → OpenPortAndInsertIOHandle` 打开,此时无应用流,用 HAL 固定格式。应用后续创建高清流时端口已存在,走 PIPE_ACTION_UPDATE 加流,不重开。

2. **格式协商不参考设备 HAL 能力**:`GetStreamPropInfo`(config_manager.cpp:870)将应用 `desc->streamInfo_` 与 XML 配置的 pipe streamPropInfos 匹配,不读设备 `audioStreamInfo_`。`UpdateDeviceStreamInfo`(pipe_selector.cpp:769)覆写设备描述符的 `audioStreamInfo_` 为 streamPropInfo 单值,后续 `UpdateModuleInfoWithDevice` 读到的是覆写值。

3. **没有格式不匹配触发的 reload**:`MatchFastArmusbPipe` 仅当设备 MAC 变化才 reload;`IsPipeFormatMatch` 存在但只用于 remote/special pipe;`ProcessOutputPipeReload` 受 `engineFlag==1` 门控(当前环境恒为 1,不阻断)。

4. **两层格式需同步**:HDI 端口格式(AudioArmDeviceBehavior)与 pipe/stream 格式(GetStreamPropInfo)当前互不同步,都不参考设备 HAL 多格式能力。

#### 发现 4:并发与 fast 回落机制现成

- `DetermineConcurrencyAction`(pipe_selector.cpp:1743)是并发判定入口,经 `GetConcurrencyAction` 查表得到 PLAY_BOTH/CONCEDE_INCOMING/CONCEDE_EXISTING/CONCEDE_BOTH。
- `HandleConcedeExisting`(:1867)是"已有流回落普通"的执行体:设 MOVE/RECREATE + `ResetToNormalRoute(true)`。
- 现有 `UpdateActionForHdPlay`(:2499)和 `GetIncomingStreamPriority`(:1725)仅覆盖 remote offload 场景,不覆盖 USB arm fast 流。
- 并发判定(`ScanPipeListForStreamDesc`→`DecideFinalRouteFlag`→`ProcessConcurrency`)早于格式选择(`UpdateDeviceStreamInfo`),即 fast 流先回落到 normal route,再进入格式匹配/reload 流程。

### 2.3 已确认的关键事实

- `GetEngineFlag()` 当前环境恒返回 1(ProAudio 永远生效),`ProcessOutputPipeReload` 的门控不阻断。
- `ProcessOutputPipeReload`(:869)调 `audioPolicyManager_.ReloadAudioPort(pipeInfo->moduleInfo_, paIndex)`,用含新格式的 moduleInfo 重开。
- `AudioArmDeviceBehavior::ReloadArmAudioPort`(arm_device_behavior.cpp:117)用 `ReloadA2dpAudioPort` API,不受 engineFlag 限制。
- A2DP 的 `ReloadOrOpenAudioPort`(a2dp_device_behavior.cpp:277)模式:engineFlag==1 走 Reload,否则走 Open。
- `ProcessInputPipeReload`(:1221)无 engineFlag 门控,始终 ReloadAudioPort(输入侧参照)。

---

## 三、方案设计

### 3.1 设计原则

- 仅输出(播放)通路,不涉及输入/录制侧。
- 按上报内容自动判断:多值走新匹配,单值走原逻辑(无需功能开关,向后兼容)。
- 尽量精确匹配,不超采样:采样率取设备支持且 ≤ 应用请求的最大值;format/channel 不支持则降级到设备最高。
- 任何格式提升都触发 reload(最灵活,需静音过渡)。
- 设备连接时首次打开维持 HAL 单值(无应用流上下文),首条高清流到达时 reload 升级。
- 复用现有 fast 流回落机制(`HandleConcedeExisting`),在并发判定侧补触发条件。

### 3.2 数据结构设计

`DeviceStreamInfo`(`interfaces/inner_api/native/audiocommon/include/audio_device_stream_info.h`)新增 `std::set<AudioSampleFormat> formats`:

- 保留原 `format` 单值:解析时设为首个 format,供旧逻辑、IPC 序列化和客户端 capabilities 使用。
- `formats` set:多 format 集合,仅服务端内存使用,**不参与 IPC 序列化**(`Marshalling`/`Unmarshalling` 不变,wire 格式不变)。
- `audioStreamInfo_`(`std::list<DeviceStreamInfo>`)维持单条,`GetDeviceStreamInfo()`/`.rbegin()` 语义不变。
- `BuildCapabilitiesFromDeviceStreamInfo` 不改,客户端仍看单值。

### 3.3 HAL 多值上报契约

沿用现有 key,值用 `|` 分隔多值,例如:

```
sink_rate:48000|96000|192000;
sink_format:AUDIO_FORMAT_PCM_16_BIT|AUDIO_FORMAT_PCM_24_BIT|AUDIO_FORMAT_PCM_32_BIT;
sink_chans:2|4;
sink_mmap:1
```

单值(无 `|`)时完全退化为现状。

### 3.4 格式优选匹配规则

`SelectBestOutputFormat(appStreamInfo, deviceCaps)` 输出 `(rate, format, channelLayout)`:

1. **采样率**:取设备支持且 ≤ 应用请求 rate 的最大值;若设备全部 > 应用请求,取设备最小(避免上采样,贴近应用)。
2. **位宽**:应用请求 format 在设备 `formats` set 中则用之;否则取设备支持且 ≤ 应用位宽的最大;都没有取设备最大;`formats` 空回退单值 `format`。
3. **声道**:应用请求 channel 在设备支持中则用之;否则取设备支持且 ≤ 应用 channel 的最大;都没有取设备最小。
4. 设备能力为空或仅单值时,返回该单值(退化为现状)。

### 3.5 fast 流并发回落规则

- 触发条件(全部满足):
  - existingStream 为 FAST route(`routeFlag_ & AUDIO_OUTPUT_FLAG_FAST`)
  - existingStream 设备为 `DEVICE_TYPE_USB_ARM_HEADSET`
  - incomingStream 为高规格:`streamInfo_.samplingRate > AudioSamplingRate::SAMPLE_RATE_48000`(**仅采样率,不计 format/channel**)
  - 设备支持该高规格(设备 `audioStreamInfo_.samplingRate` set 含 ≥ 应用请求 rate 的值)
  - `IsSameAdapter(existingStream, incomingStream)`(同一 USB 设备)
  - → action = CONCEDE_EXISTING
- 回落范围:USB 设备上**全部** fast 流回落。
- 底层约束:USB HDI 适配器同一时刻只能跑一个采样率;normal pipe reload 到 >48k 时 fast pipe 无法维持 48k,故 fast 流须回落。48k 内的 format 变化(如 48k/24bit)不影响 fast pipe,可共存,不触发回落。
- 非 USB 设备、设备不支持高格式、incoming rate ≤ 48k 时均不触发回落。

### 3.6 改动点总览

| # | 改动点 | 文件 | 关键内容 |
| --- | --- | --- | --- |
| ① | 增 formats set | `audio_device_stream_info.h` | 服务端内存用,不序列化 |
| ② | HAL 多值解析 | `audio_device_descriptor.cpp:399` | `\|` 分隔多值;format 入 `formats` set + `format`=首值;单值退化 |
| ③ | SelectBestOutputFormat | `audio_definition_policy_utils.cpp/.h` | rate≤应用最大;format/channel 降级;`formats`空回退`format` |
| ④ | UpdateDeviceStreamInfo 注入 | `audio_pipe_selector.cpp:769` | arm USB 分支:调③刷新 streamPropInfo + 设 audioStreamInfo_;非 arm USB 维持原逻辑 |
| ⑤ | FindExistingPipe 格式不匹配 reload | `audio_pipe_selector.cpp ~2300` | 不匹配→更新 moduleInfo+PIPE_ACTION_RELOAD;匹配→UPDATE |
| ⑥ | reload 执行(不改) | — | engineFlag 恒1,ProcessOutputPipeReload 可用 |
| ⑦ | 首次打开维持 HAL 单值 | `audio_arm_device_behavior.cpp:157` | 无应用流,用 format 单值 |
| ⑧ | UpdateModuleInfoWithDevice 核对 | `audio_pipe_selector.cpp:2109` | 经④后已是优选格式,理清调用序避免二次覆写 |
| ⑨ | USB arm fast 流并发回落 | `audio_pipe_selector.cpp:1743` | existing FAST + USB arm + incoming rate>48k + 设备支持 + 同设备 → CONCEDE_EXISTING;全部 fast 流回落 |

### 3.7 完整链路(改动后)

```
设备连接(无应用流):
  UpdateDeviceInfoByAudioParameters [②] → 解析多值,formats set + format=首值
  AudioArmDeviceBehavior::ActivateDevice → UpdateArmModuleInfo [⑦] → HAL 单值打开端口

应用创建高清流 CreateRendererClient → FetchPipeAndExecute:
  ScanPipeListForStreamDesc → ProcessConcurrency
    [⑨] existing fast(USB arm) + incoming(rate>48k,设备支持) → CONCEDE_EXISTING
      → HandleConcedeExisting: 全部 fast 流 MOVE + ResetToNormalRoute
  GetStreamPropInfoWithBusSelector → streamPropInfo(XML)
  UpdateDeviceStreamInfo [④] → arm USB: SelectBestOutputFormat [③] 刷新 streamPropInfo 为高格式
  FindExistingPipe [⑤] → 格式不匹配 → PIPE_ACTION_RELOAD
  分发: ProcessOutputPipeReload [⑥] → ReloadAudioPort(高格式)
  结果:高规格流无损;已回落 fast 流重采样到高格式
```

---

## 四、方案详细描述

### 4.1 改动点 ①:DeviceStreamInfo 增 formats set

- **文件**:`interfaces/inner_api/native/audiocommon/include/audio_device_stream_info.h`
- **内容**:在 `DeviceStreamInfo` 结构体新增 `std::set<AudioSampleFormat> formats;`(默认空)
- **约束**:
  - `Marshalling`/`Unmarshalling` **不动**(wire 格式不变);反序列化后 `formats` 为空,匹配侧回退用 `format`
  - `GetDeviceStreamInfo()`/`.rbegin()` 语义不变(`audioStreamInfo_` 仍维持单条)
  - `BuildCapabilitiesFromDeviceStreamInfo` 不改,客户端仍看单值

### 4.2 改动点 ②:HAL 多值参数解析

- **文件**:`services/audio_policy/server/domain/device/src/audio_device_descriptor.cpp:399`(`UpdateDeviceInfoByAudioParameters`)
- **内容**:
  - value 按 `|` 分隔多值
  - `sink_rate`/`source_rate` 多值循环 insert `samplingRate` set
  - `sink_format`/`source_format` 多值逐个 `ParseAudioFormat`(:85)+ `g_formatStrToEnum`(:98)后 insert `formats` set;`format` 设为首个值
  - `sink_chans`/`source_chans` 多值循环 insert `channelLayout` set
  - `device_name`/`*_mmap` 解析不变
- **退化兼容**:单值(无 `|`)→ 各 set 含 1 元素,`format` 同现状

### 4.3 改动点 ③:格式优选匹配函数

- **位置**:`AudioDefinitionPolicyUtils`(`services/audio_policy/server/domain/pipe/src/audio_definition_policy_utils.cpp` + `.h`),复用其 `formatStrToEnum`/`enumToFormatStr`,避免新增本地映射表
- **签名草案**:
  ```cpp
  struct SelectedDeviceFormat {
      AudioSamplingRate rate;
      AudioSampleFormat format;
      AudioChannelLayout channelLayout;
  };
  static SelectedDeviceFormat SelectBestOutputFormat(
      const AudioStreamInfo &appStreamInfo,
      const std::list<DeviceStreamInfo> &deviceCaps);
  ```
- **规则**:见 3.4 节
- **回退**:`formats` set 为空时回退用单值 `format`(覆盖旧 HAL 单值场景)

### 4.4 改动点 ④:UpdateDeviceStreamInfo 注入设备能力(核心注入点)

- **文件**:`services/audio_policy/server/domain/pipe/src/audio_pipe_selector.cpp:769`(`UpdateDeviceStreamInfo`)
- **内容**:新增 arm USB 分支,判断 `streamDesc` 的设备为 `DEVICE_TYPE_USB_ARM_HEADSET` 时:
  1. 从设备描述符的 `audioStreamInfo_`(此时仍是原始多格式能力,未被覆写)取 `formats` set
  2. 调 `SelectBestOutputFormat(streamDesc->streamInfo_, deviceDesc->GetAudioStreamInfo())`
  3. 用结果刷新 `streamPropInfo`(覆盖 XML 钳制的固定值为优选格式)
  4. 用结果设 `temp->audioStreamInfo_`(单条,匹配后格式),替代原"用 streamPropInfo 覆写"逻辑
- **非 arm USB**:维持原逻辑(`audioStreamInfo_` = streamPropInfo 派生)
- **关键**:此处在 `GetStreamPropInfoWithBusSelector` 之后、`FindExistingPipe` 之前执行,设备描述符原始能力此时未被覆写

### 4.5 改动点 ⑤:FindExistingPipe 格式不匹配触发 reload(核心触发点)

- **文件**:`audio_pipe_selector.cpp` `FindExistingPipe`(~2300-2364)
- **内容**:找到已有 USB arm pipe 后,增加格式校验:
  - 用 `IsPipeFormatMatch`(:2423,已有)或类似逻辑,比较新选格式(streamPropInfo)与已有 pipe 的 `moduleInfo_.rate/format/channels`
  - 不匹配 → 更新 `pipeInfo->moduleInfo_` 为新格式 + `pipeInfo->InitAudioStreamInfo()` + `PIPE_ACTION_RELOAD`(参照 `MatchFastArmusbPipe`:2404-2406 模式)
  - 匹配 → 维持 `PIPE_ACTION_UPDATE`(复用,加流)
- **保留**:`MatchFastArmusbPipe` 原有 MAC 变化 reload 逻辑

### 4.6 改动点 ⑥:reload 执行(不改)

- `PIPE_ACTION_RELOAD` → `ProcessOutputPipeReload`(`audio_core_service_private.cpp:869`)
- `engineFlag` 恒为 1(ProAudio 永远生效),门控不阻断
- `audioPolicyManager_.ReloadAudioPort(pipeInfo->moduleInfo_, paIndex)` 用含新格式的 moduleInfo 重开;更新 paIndex;`ProcessOutputPipeUpdate` 处理流
- **需验证**:`ReloadAudioPort`(通用)对 regular USB arm module 是否等效于 `ReloadArmAudioPort`(`ReloadA2dpAudioPort`);参照 `MatchFastArmusbPipe` 已用 PIPE_ACTION_RELOAD 的先例

### 4.7 改动点 ⑦:首次打开维持 HAL 单值

- **文件**:`services/audio_policy/server/domain/device/src/pnp/audio_arm_device_behavior.cpp:157`(`UpdateArmModuleInfo`)
- **内容**:设备连接时无应用流,不调 `SelectBestOutputFormat`,用设备 `format` 单值(HAL 上报首个)打开
- **策略**:首条高清流到达时由改动点⑤触发 reload 升级(有短暂静音过渡)

### 4.8 改动点 ⑧:UpdateModuleInfoWithDevice 核对

- **文件**:`audio_pipe_selector.cpp:2109`(`UpdateModuleInfoWithDevice`)
- **内容**:经改动点④后 `audioStreamInfo_` 已是优选格式,提取逻辑可复用
- **注意**:需理清与改动点④的调用顺序,避免对 moduleInfo 二次覆写冲突。该函数 USB arm 分支(:2117-2147)当前读 `audioStreamInfo_.back()` 的 `.begin()`/单值 format/`GetMainChannelLayout()`,改动点④后该值为优选格式

### 4.9 改动点 ⑨:USB arm fast 流并发回落

- **文件**:`audio_pipe_selector.cpp:1743`(`DetermineConcurrencyAction`)
- **内容**:新增辅助函数(如 `UpdateActionForArmUsbFastConcede`),与 `UpdateActionForHdPlay`(:1755)并列调用
- **触发条件**:见 3.5 节
- **执行链路**:
  ```
  DetermineConcurrencyAction → CONCEDE_EXISTING [⑨新增判定]
    ↓
  ProcessConcurrency → HandleConcedeExisting (:1867)
    ↓
  existingStream(fast) → AUDIO_STREAM_ACTION_MOVE + ResetToNormalRoute(true)
    → fast 流移到 normal USB arm pipe
    ↓
  后续 FetchPipeAndExecute:
    incomingStream(高规格) + 已回落的 fast 流 均在 normal USB arm pipe
    UpdateDeviceStreamInfo [④] → SelectBestOutputFormat → 192k/24bit
    FindExistingPipe [⑤] → 格式不匹配 → PIPE_ACTION_RELOAD
    ProcessOutputPipeReload → 重开为 192k/24bit
    ↓
  结果:
    - 高规格流:原生 192k/24bit,无损 ✓
    - 已回落 fast 流:48k/16bit 重采样到 192k/24bit(上采样,回落的代价,可接受)
  ```
- **与已有逻辑关系**:
  - `HandleFastStreamLimitOnCreate`(:807):判 fast 流数量超限→新流降级。与本规则不冲突
  - `UpdateActionForHdPlay`(:2499):仅 offload+remote HD。本规则补充 USB arm fast 场景,互不重叠
  - `AdjustFastRouteForUltraFastIfNeeded`(:748):ultra-fast 路由调整。本规则在其后生效

### 4.10 不改动项

- `ProcessOutputPipeReload` / `engineFlag` 门控(恒1,通用路径可用)
- `AudioIOHandleMap::OpenPortAndInsertIOHandle`(通用,按 moduleInfo 打开)
- `DeviceStreamInfo::Marshalling`/`Unmarshalling`(wire 不变)
- `BuildCapabilitiesFromDeviceStreamInfo`(客户端看单值)
- 输入/source 侧

### 4.11 验证要求

| 验证项 | 目标 |
| --- | --- |
| 多值解析 | `UpdateDeviceInfoByAudioParameters` 多值→多元素 set;单值退化 |
| 优选匹配 | `SelectBestOutputFormat`:精确/降级/空回退/跨 format |
| 注入点 | `UpdateDeviceStreamInfo` arm USB:streamPropInfo 刷新为优选;非 arm USB 不变 |
| reload 触发 | `FindExistingPipe`:格式不匹配→PIPE_ACTION_RELOAD;匹配→UPDATE |
| reload 执行 | `ProcessOutputPipeReload` 实际生效(paIndex 更新) |
| 首次打开 | `UpdateArmModuleInfo` HAL 单值打开 |
| fast 回落 | existing fast + incoming rate>48k(设备支持)→全部 fast MOVE+降级 normal |
| 不回落 | incoming rate≤48k → 不触发;设备不支持 → 不触发;非 USB → 不触发 |
| 回归 | 非 arm USB 设备、IPC Marshalling、现有 USB arm/fast 用例 |
| 静态/模糊 | `git diff --check` + `test/fuzztest/audiopolicy_fuzzer/`(非法格式串/空值) |

### 4.12 实现顺序

① → ② → ③ → ④ → ⑤ → ⑦ → ⑧ → ⑨,每步配单测,最后统一跑验证矩阵。

### 4.13 残留风险

1. `ReloadAudioPort`(通用)对 regular USB arm module 是否等效于 `ReloadArmAudioPort`(`ReloadA2dpAudioPort`)——实现时验证
2. `UpdateDeviceStreamInfo` 改动后,非 arm USB 依赖覆写行为的逻辑需回归
3. fast 流回落后上采样到高格式的听感影响
4. `UpdateModuleInfoWithDevice`(pipe_selector.cpp:2109)与改动点④对 moduleInfo 的二次写入——实现时理清调用顺序
5. `ParseAudioFormat`(audio_device_descriptor.cpp:85)与测试用例 `AUDIO_FORMAT_TYPE_PCM_16_BIT`(带 TYPE)既有不匹配,多格式解析需统一 HAL 串规范

### 4.14 源码入口

- 设备格式解析:`services/audio_policy/server/domain/device/src/audio_device_descriptor.cpp`(`UpdateDeviceInfoByAudioParameters`、`ParseAudioFormat`、`BuildCapabilitiesFromDeviceStreamInfo`)
- 设备上线:`services/audio_policy/server/domain/device/src/audio_device_status.cpp`(`HandleLocalDeviceConnected`)
- ARM 设备行为:`services/audio_policy/server/domain/device/src/pnp/audio_arm_device_behavior.cpp`(`ActivateDevice`、`UpdateArmModuleInfo`、`ProcessModuleActivation`、`ReloadArmAudioPort`)
- 通路选择:`services/audio_policy/server/domain/pipe/src/audio_pipe_selector.cpp`(`FetchPipeAndExecute`、`UpdateDeviceStreamInfo`、`FindExistingPipe`、`UpdateModuleInfoWithDevice`、`DetermineConcurrencyAction`、`HandleConcedeExisting`、`MatchFastArmusbPipe`)
- 格式工具:`services/audio_policy/server/domain/pipe/src/audio_definition_policy_utils.cpp`(`formatStrToEnum`/`enumToFormatStr`)
- 流属性协商:`services/audio_policy/server/domain/pipe/src/audio_core_config_manager.cpp`(`GetStreamPropInfo`、`ConvertDeviceStreamInfoToStreamPropInfo`)
- 通路执行:`services/audio_policy/server/service/service_main/src/audio_core_service_private.cpp`(`CreateRendererClient`、`FetchRendererPipeAndExecute`、`ProcessOutputPipeReload`、`ActivateOutputDevice`)
- IOHandle:`services/audio_policy/server/domain/pipe/src/audio_iohandle_map.cpp`(`OpenPortAndInsertIOHandle`)
- 数据结构:`interfaces/inner_api/native/audiocommon/include/audio_device_stream_info.h`(`DeviceStreamInfo`)、`interfaces/inner_api/native/audiocommon/include/audio_device_descriptor.h`(`audioStreamInfo_`)
