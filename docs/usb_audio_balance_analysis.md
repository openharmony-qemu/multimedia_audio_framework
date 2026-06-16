# USB耳机音量平衡不生效问题分析与解决方案

## 问题描述

连接USB耳机时，调节音量平衡（左/右声道平衡）不生效；拔插USB耳机后，音量平衡生效。

## 根因分析

### 核心问题：`limitFunc` 拦截条件缺少USB sink类型

音量平衡的完整调用链路如下：

```
应用层 API调用
  → AudioSystemManager::SetAudioBalanceValue()
    → AudioSystemClientEngineManager::SetAudioBalanceValue()
      → IPC调用 AudioServer::SetAudioBalanceValue()
        → AudioServer::SetAudioBalanceValueInner(isAudioBalanceEnabled, audioBalance)
          → HdiAdapterManager::ProcessSink(processFunc)
            → processFunc内部先调用 limitFunc(renderId) 过滤sink类型
              → 只有满足条件的sink才会被调用 sink->SetAudioBalanceValue()
```

在 `audio_server.cpp` 的 `SetAudioBalanceValueInner` (第2038行) 和 `SetAudioMonoState` (第1982行) 中，`limitFunc` 的定义如下：

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
    return false;  // 所有其他sink类型被拦截
};
```

**USB耳机的sink标识**为 `HDI_ID_TYPE_PRIMARY` + `HDI_ID_INFO_USB`（参见 `id_handler.cpp` 第69行），但 `limitFunc` 中 `HDI_ID_TYPE_PRIMARY` 分支只允许 `default/direct/voip` 三种info值，**不包含 `"usb"`**，因此USB sink被拦截，`SetAudioBalanceValue` 不会传递到USB sink。

### 拔插后生效的原因

拔插USB耳机时，系统会重新创建USB sink实例。在新sink创建过程中，`HdiAdapterManager::DoSetSinkPrestoreInfo` (第499行) 从 `sinkPrestoreInfo_` 缓存中读取之前保存的 `audioBalance` 值，直接调用 `sink->SetAudioBalanceValue(audioBalance)` — **这里没有 `limitFunc` 过滤！** 所以新创建的sink直接获得了正确的balance值。

```cpp
void HdiAdapterManager::DoSetSinkPrestoreInfo(std::shared_ptr<IAudioRenderSink> sink, uint32_t type)
{
    float audioBalance = 0.0;
    int32_t ret = sinkPrestoreInfo_.Get(PRESTORE_INFO_AUDIO_BALANCE, audioBalance);
    if (ret == SUCCESS) {
        sink->SetAudioBalanceValue(audioBalance);  // 直接设置，无limitFunc过滤
    }
    ...
}
```

这就解释了为什么拔插后生效，但运行时调节不生效：运行时的 `SetAudioBalanceValueInner` 有 `limitFunc` 拦截USB sink，而sink初始化时的 `DoSetSinkPrestoreInfo` 没有此拦截。

### 同时存在的问题：`SetAudioMonoState` 同样缺少USB

`SetAudioMonoState` 中有完全相同的 `limitFunc` 定义（第1989-2002行），同样不包含USB sink类型，因此单声道模式在USB耳机上也存在相同的bug。

## 解决方案

### 修改点1：`SetAudioBalanceValueInner` 的 `limitFunc` 增加USB支持

在 `audio_server.cpp` 第2040-2052行的 `limitFunc` 中，`HDI_ID_TYPE_PRIMARY` 分支增加 `HDI_ID_INFO_USB`：

```cpp
if (type == HDI_ID_TYPE_PRIMARY) {
    return info == HDI_ID_INFO_DEFAULT || info == HDI_ID_INFO_DIRECT
        || info == HDI_ID_INFO_VOIP || info == HDI_ID_INFO_USB;
}
```

### 修改点2：`SetAudioMonoState` 的 `limitFunc` 增加USB支持

在 `audio_server.cpp` 第1989-2002行的 `limitFunc` 中，同样增加USB支持：

```cpp
if (type == HDI_ID_TYPE_PRIMARY) {
    return info == HDI_ID_INFO_DEFAULT || info == HDI_ID_INFO_DIRECT
        || info == HDI_ID_INFO_VOIP || info == HDI_ID_INFO_USB;
}
```

### 其他注意事项

- `HDI_ID_INFO_DP` 也属于 `HDI_ID_TYPE_PRIMARY` 但不在 `limitFunc` 中。DP设备通常是多声道输出，不适用单声道/平衡，保持现状是合理的。
- USB耳机使用的是 `AudioRenderSink`（同Primary sink相同的实现类），其 `SetAudioBalanceValue` 和 `AdjustAudioBalance` 的实现完全支持立体声平衡处理，无需额外修改sink层。
- 修改仅影响运行时动态设置的场景，不影响sink初始化时通过 `DoSetSinkPrestoreInfo` 设置的逻辑（两者将保持一致）。

## 修改文件

- `services/audio_service/server/src/audio_server.cpp`：修改 `SetAudioBalanceValueInner` 和 `SetAudioMonoState` 中的 `limitFunc`
