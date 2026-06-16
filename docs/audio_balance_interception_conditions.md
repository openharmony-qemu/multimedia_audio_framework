# 音量平衡（Balance）拦截条件总结

## 概述

音量平衡功能从API调用到最终作用于PCM数据，经过多层拦截条件。以下按调用链路顺序逐一分析。

## 第一层：API入口拦截 — `AudioServer::SetAudioBalanceValue`

**文件**: `services/audio_service/server/src/audio_server.cpp` (第2023行)

| 拦截条件 | 说明 | 代码位置 |
|---|---|---|
| 权限检查 | `PermissionUtil::VerifyIsAudio()` — 非音频权限UID被拒绝 | 第2026-2028行 |
| 参数范围检查 | `audioBalance >= -1.0f && audioBalance <= 1.0f` — 超范围值被拒绝 | 第2029-2030行 |

## 第二层：场景拦截 — `isAudioBalanceEnabled`

**文件**: `services/audio_service/server/src/audio_server.cpp` (第2032-2034行)

```cpp
bool inPhoneScene = (audioScene_ == AUDIO_SCENE_PHONE_CALL || audioScene_ == AUDIO_SCENE_PHONE_CHAT);
bool isAudioBalanceEnabled = (!inPhoneScene) && (!isEarpiece_);
```

| 拦截条件 | 说明 | 效果 |
|---|---|---|
| 通话场景 | `audioScene_ == AUDIO_SCENE_PHONE_CALL || audioScene_ == AUDIO_SCENE_PHONE_CHAT` | 通话中禁用平衡，传入值0.0f |
| 听筒模式 | `isEarpiece_ == true`（当前输出设备为 `DEVICE_TYPE_EARPIECE`） | 听筒模式下禁用平衡，传入值0.0f |

当 `isAudioBalanceEnabled == false` 时，实际传给sink的balance值为 `0.0f`（即居中/无偏移），但 `audioBalanceValue_` 仍保存用户设置的原始值，以便场景切换后恢复。

**`isEarpiece_` 更新时机**: 在 `SetIORoutes` 中根据当前输出设备类型更新（第1842行）。
**`SetAudioBalanceStatus` 被调用时机**: `SetAudioSceneInner`（第1802行）和 `SetIORoutes`（第1843行）。

## 第三层：Sink类型拦截 — `limitFunc`

**文件**: `services/audio_service/server/src/audio_server.cpp` (第2040-2053行)

```cpp
auto limitFunc = [](uint32_t renderId) -> bool {
    uint32_t type = IdHandler::GetInstance().ParseType(renderId);
    std::string info = IdHandler::GetInstance().ParseInfo(renderId);
    if (type == HDI_ID_TYPE_PRIMARY) {
        return info == HDI_ID_INFO_DEFAULT || info == HDI_ID_INFO_DIRECT || info == HDI_ID_INFO_VOIP;
    }
    if (type == HDI_ID_TYPE_BLUETOOTH) {
        return info == HDI_ID_INFO_DEFAULT;
    }
    if (type == HDI_ID_TYPE_OFFLOAD) {
        return info == HDI_ID_INFO_DEFAULT;
    }
    return false;
};
```

| Sink类型 | Info值 | 是否允许 | 对应设备 |
|---|---|---|---|
| `HDI_ID_TYPE_PRIMARY` | `"default"` | ✅ | 内置扬声器/有线耳机 |
| `HDI_ID_TYPE_PRIMARY` | `"direct"` | ✅ | 低延迟直通输出 |
| `HDI_ID_TYPE_PRIMARY` | `"voip"` | ✅ | VoIP通话输出 |
| `HDI_ID_TYPE_PRIMARY` | `"usb"` | ❌ | USB耳机（**此为当前bug**） |
| `HDI_ID_TYPE_PRIMARY` | `"dp"` | ❌ | DP输出（多声道，不适用平衡） |
| `HDI_ID_TYPE_PRIMARY` | `"interphone"` | ❌ | 对讲机输出 |
| `HDI_ID_TYPE_PRIMARY` | `"voice_call_assistant"` | ❌ | 通话辅助 |
| `HDI_ID_TYPE_BLUETOOTH` | `"default"` | ✅ | 蓝牙A2DP |
| `HDI_ID_TYPE_OFFLOAD` | `"default"` | ✅ | Offload输出 |

## 第四层：Sink内部拦截 — `AudioRenderSink::SetAudioBalanceValue`

**文件**: `frameworks/native/hdiadapter_new/sink/audio_render_sink.cpp` (第472-491行)

| 拦截条件 | 说明 | 效果 |
|---|---|---|
| `audioBalance == 0.0f` | 平衡值为0（居中） | `audioBalanceState_` 设为false，`RenderFrame` 不执行 `AdjustAudioBalance` |
| 非立体声 | `attr_.channel != STEREO_CHANNEL_COUNT` (2) | `AdjustAudioBalance` 内部 `CHECK_AND_RETURN`，不做处理 |

当 `audioBalance != 0.0f` 时，`audioBalanceState_` 设为true，计算左/右声道系数：
- `audioBalance > 0` → 减弱左声道（`leftBalanceCoef_ = 1.0 - audioBalance`）
- `audioBalance < 0` → 减弱右声道（`rightBalanceCoef_ = 1.0 + audioBalance`，注意audioBalance为负值）

## 第五层：渲染帧拦截 — `RenderFrame` 中的 `deviceConnectedFlag_` 和 `switchDeviceMute_`

**文件**: `frameworks/native/hdiadapter_new/sink/audio_render_sink.cpp` (第294-344行)

| 拦截条件 | 说明 | 效果 |
|---|---|---|
| `switchDeviceMute_ == true` | 设备切换过程中的静音状态 | PCM数据被memset为0（静音） |
| `deviceConnectedFlag_ == true` | 设备连接标记 | PCM数据被memset为0（静音） |

注意：这两个条件不影响balance的计算，它们是在balance处理**之后**将整个帧数据清零。即如果处于设备切换静音期，无论balance是否生效，输出都是静音的。

## PrestoreInfo机制（初始化时无拦截）

**文件**: `frameworks/native/hdiadapter_new/manager/hdi_adapter_manager.cpp` (第499-516行)

`DoSetSinkPrestoreInfo` 在新sink创建时从 `sinkPrestoreInfo_` 缓存中读取balance值直接设置，**没有任何limitFunc过滤**。这是拔插USB耳机后balance生效的原因——运行时动态设置有limitFunc拦截，但初始化时没有。

## `SetAudioMonoState` 拦截条件

`SetAudioMonoState` (第1982行) 有完全相同的权限检查、limitFunc拦截（同样缺少USB），以及 `ProcessSink` 机制。唯一的区别是没有场景拦截（无 `inPhoneScene`/`isEarpiece_` 条件）。
