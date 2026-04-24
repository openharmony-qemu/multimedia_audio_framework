/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HDI_ADAPTER_MANAGER_H
#define HDI_ADAPTER_MANAGER_H

#include <iostream>
#include <cstring>
#include <mutex>
#include <unordered_map>
#include <memory>
#include <optional>
#include "common/hdi_adapter_info.h"
#include "sink/i_audio_render_sink.h"
#include "source/i_audio_capture_source.h"
#include "adapter/i_device_manager.h"
#include "util/callback_wrapper.h"
#include "util/kv_pair.h"
#include "audio_engine_callback_types.h"

namespace OHOS {
namespace AudioStandard {
typedef struct RenderSinkInfo {
    std::shared_ptr<IAudioRenderSink> sink_;
    std::atomic<uint32_t> refCount_ = 0;
} RenderSinkInfo;

typedef struct CaptureSourceInfo {
    std::shared_ptr<IAudioCaptureSource> source_;
    std::atomic<uint32_t> refCount_ = 0;
} CaptureSourceInfo;

class HdiAdapterManager {
public:
    static HdiAdapterManager &GetInstance(void);

    std::shared_ptr<IDeviceManager> GetDeviceManager(HdiDeviceManagerType type);
    void ReleaseDeviceManager(HdiDeviceManagerType type);

    uint32_t GetId(HdiIdBase base, HdiIdType type, const std::string &info = HDI_ID_INFO_DEFAULT,
        bool isResident = false, bool tryCreate = true);
    uint32_t GetRenderIdByDeviceClass(const std::string &deviceClass, const std::string &info = HDI_ID_INFO_DEFAULT,
        bool isResident = false, bool tryCreate = true, uint32_t routeFlag = 0);
    uint32_t GetCaptureIdByDeviceClass(const std::string &deviceClass, const SourceType sourceType,
        const std::string &info = HDI_ID_INFO_DEFAULT, bool isResident = false,
        uint32_t routeFlag = 0, bool tryCreate = true);
    void ReleaseId(uint32_t &id);

    std::shared_ptr<IAudioRenderSink> GetRenderSink(uint32_t renderId, bool tryCreate = false);
    std::shared_ptr<IAudioCaptureSource> GetCaptureSource(uint32_t captureId, bool tryCreate = false);
    std::shared_ptr<IAudioRenderSink> GetAuxiliarySink();

    int32_t LoadAdapter(HdiDeviceManagerType type, const std::string &adapterName);
    int32_t SetAudioParameter(HdiDeviceManagerType type, const std::string &adapterName, AudioParamKey key,
        const std::string &condition, const std::string &value);
    std::string GetAudioParameter(HdiDeviceManagerType type, const std::string &adapterName, AudioParamKey key,
        const std::string &condition);
    std::optional<std::string> GetAudioParameterByKey(HdiDeviceManagerType type, const std::string &key);
    void UnloadAdapter(HdiDeviceManagerType type, const std::string &adapterName, bool force = false);
    int32_t SetExtraParameters(HdiDeviceManagerType type, const std::string &adapterName,
        const std::string &condition, const std::string &value);
    int32_t GetExtraParameters(const std::vector<std::string> &subKeys,
        std::vector<std::pair<std::string, std::string>> &result);

    int32_t ProcessSink(const std::function<int32_t(uint32_t, std::shared_ptr<IAudioRenderSink>)> &processFunc);
    float GetNormalSourceMaxAmplitude();
    int32_t ProcessSource(const std::function<int32_t(uint32_t, std::shared_ptr<IAudioCaptureSource>)> &processFunc);

    void RegistSinkCallback(HdiAdapterCallbackType type, std::shared_ptr<IAudioSinkCallback> cb,
        const std::function<bool(uint32_t)> &limitFunc = [](uint32_t id) -> bool { return false; });
    void RegistSinkCallback(HdiAdapterCallbackType type, IAudioSinkCallback *cb,
        const std::function<bool(uint32_t)> &limitFunc = [](uint32_t id) -> bool { return false; });
    void RegistSinkCallbackGenerator(HdiAdapterCallbackType type,
        const std::function<std::shared_ptr<IAudioSinkCallback>(uint32_t)> cbGenerator,
        const std::function<bool(uint32_t)> &limitFunc = [](uint32_t id) -> bool { return false; });
    void RegistSourceCallback(HdiAdapterCallbackType type, std::shared_ptr<IAudioSourceCallback> cb,
        const std::function<bool(uint32_t)> &limitFunc = [](uint32_t id) -> bool { return false; });
    void RegistSourceCallback(HdiAdapterCallbackType type, IAudioSourceCallback *cb,
        const std::function<bool(uint32_t)> &limitFunc = [](uint32_t id) -> bool { return false; });
    void RegistSourceCallbackGenerator(HdiAdapterCallbackType type,
        const std::function<std::shared_ptr<IAudioSourceCallback>(uint32_t)> cbGenerator,
        const std::function<bool(uint32_t)> &limitFunc = [](uint32_t id) -> bool { return false; });

    template<typename T>
    void UpdateSinkPrestoreInfo(const std::string infoKey, const T &info)
    {
        sinkPrestoreInfo_.Set(infoKey, info);
    }

    template<typename T>
    int32_t GetSinkPrestoreInfo(const std::string infoKey, T &info)
    {
        return sinkPrestoreInfo_.Get(infoKey, info);
    }

    void EraseSinkPrestoreInfo(const std::string infoKey)
    {
        sinkPrestoreInfo_.Erase(infoKey);
    }

    void DumpInfo(std::string &dumpString);

    int32_t GetCurrentOutputPipeChangeInfos(std::vector<std::shared_ptr<AudioOutputPipeInfo>> &pipeChangeInfos);
    int32_t GetCurrentInputPipeChangeInfos(std::vector<std::shared_ptr<AudioInputPipeInfo>> &pipeChangeInfos);

private:
    HdiAdapterManager() = default;
    ~HdiAdapterManager();
    HdiAdapterManager(const HdiAdapterManager &) = delete;
    HdiAdapterManager &operator=(const HdiAdapterManager &) = delete;
    HdiAdapterManager(HdiAdapterManager &&) = delete;
    HdiAdapterManager &operator=(HdiAdapterManager &&) = delete;

    void IncRefCount(uint32_t id);
    void DecRefCount(uint32_t id);
    void DoRegistSinkCallback(uint32_t id, std::shared_ptr<IAudioRenderSink> sink);
    void DoRegistSourceCallback(uint32_t id, std::shared_ptr<IAudioCaptureSource> source);
    void DoSetSinkPrestoreInfo(std::shared_ptr<IAudioRenderSink> sink, uint32_t type);

    void ProcessIdUseCount(uint32_t id, bool isResident, bool tryCreate);

    void SetRemoteHdiInvalidState(HdiDeviceManagerType type, const std::string &networkId, bool force);

private:
    std::unordered_map<uint32_t, RenderSinkInfo> renderSinks_;
    std::unordered_map<uint32_t, CaptureSourceInfo> captureSources_;
    std::shared_ptr<IDeviceManager> deviceManagers_[HDI_DEVICE_MANAGER_TYPE_NUM];
    std::mutex renderSinkMtx_;
    std::mutex captureSourceMtx_;
    std::mutex deviceManagerMtx_;
    // callback
    SinkCallbackWrapper sinkCbs_;
    SourceCallbackWrapper sourceCbs_;
    std::function<bool(uint32_t)> cbLimitFunc_[HDI_ID_BASE_NUM][HDI_CB_TYPE_NUM];
    // prestore
    KvPair<std::string> sinkPrestoreInfo_;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // HDI_ADAPTER_MANAGER_H
