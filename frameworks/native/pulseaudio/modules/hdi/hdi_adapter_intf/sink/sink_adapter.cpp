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

#ifndef LOG_TAG
#define LOG_TAG "SinkAdapter"
#endif

#include "sink/sink_adapter.h"
#include <functional>
#include "audio_errors.h"
#include "audio_hdi_log.h"
#include "sink/i_audio_render_sink.h"
#include "manager/hdi_adapter_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

using namespace OHOS::AudioStandard;
typedef void OnRenderCallback(const RenderCallbackType type, int8_t *userdata);

static inline std::shared_ptr<IAudioRenderSink> GetRenderSink(uint32_t renderId)
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    return manager.GetRenderSink(renderId, true);
}

int32_t InitSinkAdapter(struct SinkAdapter *adapter, const char *deviceClass, const char *info)
{
    AUDIO_INFO_LOG("In, deviceClass: %{public}s, info: %{public}s", deviceClass, info);
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr, ERR_INVALID_HANDLE, "adapter is nullptr");

    adapter->renderId = HDI_INVALID_ID;
    adapter->deviceClass = strdup(deviceClass);
    CHECK_AND_RETURN_RET_LOG(adapter->deviceClass != nullptr, ERR_OPERATION_FAILED, "strdup fail");
    if (info == nullptr) {
        adapter->renderId = HdiAdapterManager::GetInstance().GetRenderIdByDeviceClass(deviceClass,
            HDI_ID_INFO_DEFAULT, true);
    } else {
        adapter->renderId = HdiAdapterManager::GetInstance().GetRenderIdByDeviceClass(deviceClass, std::string(info),
            true);
    }
    std::shared_ptr<IAudioRenderSink> sink = GetRenderSink(adapter->renderId);
    if (sink == nullptr) {
        AUDIO_ERR_LOG("get sink fail, deviceClass: %{public}s, info: %{public}s, renderId: %{public}u", deviceClass,
            info, adapter->renderId);
        HdiAdapterManager::GetInstance().ReleaseId(adapter->renderId);
        return ERR_OPERATION_FAILED;
    }
    return SUCCESS;
}

void DeInitSinkAdapter(struct SinkAdapter *adapter)
{
    CHECK_AND_RETURN_LOG(adapter != nullptr, "adapter is nullptr");
    HdiAdapterManager::GetInstance().ReleaseId(adapter->renderId);
    if (adapter->deviceClass != nullptr) {
        free(const_cast<char *>(adapter->deviceClass));
        adapter->deviceClass = nullptr;
    }
}

int32_t SinkAdapterInit(struct SinkAdapter *adapter, const struct SinkAdapterAttr *attr)
{
    AUDIO_INFO_LOG("In");
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->renderId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");
    CHECK_AND_RETURN_RET_LOG(attr != nullptr, ERR_INVALID_PARAM, "attr is nullptr");
    std::shared_ptr<IAudioRenderSink> sink = GetRenderSink(adapter->renderId);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_INVALID_HANDLE, "get sink fail");
    if (sink->IsInited()) {
        return SUCCESS;
    }

    IAudioSinkAttr sinkAttr = {};
    sinkAttr.adapterName = attr->adapterName;
    sinkAttr.openMicSpeaker = attr->openMicSpeaker;
    sinkAttr.format = static_cast<AudioSampleFormat>(attr->format);
    sinkAttr.sampleRate = attr->sampleRate;
    sinkAttr.channel = attr->channel;
    sinkAttr.volume = attr->volume;
    sinkAttr.deviceNetworkId = attr->deviceNetworkId;
    sinkAttr.deviceType = attr->deviceType;
    sinkAttr.channelLayout = attr->channelLayout;
    sinkAttr.aux = attr->aux;

    return sink->Init(sinkAttr);
}

void SinkAdapterDeInit(struct SinkAdapter *adapter)
{
    AUDIO_INFO_LOG("In");
    CHECK_AND_RETURN_LOG(adapter != nullptr && adapter->renderId != HDI_INVALID_ID, "invalid adapter");
    std::shared_ptr<IAudioRenderSink> sink = GetRenderSink(adapter->renderId);
    CHECK_AND_RETURN_LOG(sink != nullptr, "get sink fail");
    if (!sink->IsInited()) {
        return;
    }

    sink->DeInit();
}

int32_t SinkAdapterStart(struct SinkAdapter *adapter)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->renderId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");
    std::shared_ptr<IAudioRenderSink> sink = GetRenderSink(adapter->renderId);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_INVALID_HANDLE, "get sink fail");
    CHECK_AND_RETURN_RET_LOG(sink->IsInited(), ERR_ILLEGAL_STATE, "sink not init");

    return sink->Start();
}

int32_t SinkAdapterStop(struct SinkAdapter *adapter)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->renderId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");
    std::shared_ptr<IAudioRenderSink> sink = GetRenderSink(adapter->renderId);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_INVALID_HANDLE, "get sink fail");
    CHECK_AND_RETURN_RET_LOG(sink->IsInited(), ERR_ILLEGAL_STATE, "sink not init");

    return sink->Stop();
}

int32_t SinkAdapterResume(struct SinkAdapter *adapter)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->renderId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");
    std::shared_ptr<IAudioRenderSink> sink = GetRenderSink(adapter->renderId);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_INVALID_HANDLE, "get sink fail");
    CHECK_AND_RETURN_RET_LOG(sink->IsInited(), ERR_ILLEGAL_STATE, "sink not init");

    return sink->Resume();
}

int32_t SinkAdapterPause(struct SinkAdapter *adapter)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->renderId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");
    std::shared_ptr<IAudioRenderSink> sink = GetRenderSink(adapter->renderId);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_INVALID_HANDLE, "get sink fail");
    CHECK_AND_RETURN_RET_LOG(sink->IsInited(), ERR_ILLEGAL_STATE, "sink not init");

    return sink->Pause();
}

int32_t SinkAdapterFlush(struct SinkAdapter *adapter)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->renderId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");
    std::shared_ptr<IAudioRenderSink> sink = GetRenderSink(adapter->renderId);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_INVALID_HANDLE, "get sink fail");
    CHECK_AND_RETURN_RET_LOG(sink->IsInited(), ERR_ILLEGAL_STATE, "sink not init");

    return sink->Flush();
}

int32_t SinkAdapterReset(struct SinkAdapter *adapter)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->renderId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");
    std::shared_ptr<IAudioRenderSink> sink = GetRenderSink(adapter->renderId);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_INVALID_HANDLE, "get sink fail");
    CHECK_AND_RETURN_RET_LOG(sink->IsInited(), ERR_ILLEGAL_STATE, "sink not init");

    return sink->Reset();
}

int32_t SinkAdapterRenderFrame(struct SinkAdapter *adapter, char *data, uint64_t len, uint64_t *writeLen)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->renderId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");
    std::shared_ptr<IAudioRenderSink> sink = GetRenderSink(adapter->renderId);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_INVALID_HANDLE, "get sink fail");
    CHECK_AND_RETURN_RET_LOG(sink->IsInited(), ERR_ILLEGAL_STATE, "sink not init");

    return sink->RenderFrame(*data, len, *writeLen);
}

int32_t SinkAdapterSetVolume(struct SinkAdapter *adapter, float left, float right)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->renderId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");
    std::shared_ptr<IAudioRenderSink> sink = GetRenderSink(adapter->renderId);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_INVALID_HANDLE, "get sink fail");
    CHECK_AND_RETURN_RET_LOG(sink->IsInited(), ERR_ILLEGAL_STATE, "sink not init");

    return sink->SetVolume(left, right);
}

int32_t SinkAdapterGetVolume(struct SinkAdapter *adapter, float *left, float *right)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->renderId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");
    std::shared_ptr<IAudioRenderSink> sink = GetRenderSink(adapter->renderId);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_INVALID_HANDLE, "get sink fail");
    CHECK_AND_RETURN_RET_LOG(sink->IsInited(), ERR_ILLEGAL_STATE, "sink not init");
    CHECK_AND_RETURN_RET_LOG(left != nullptr && right != nullptr, ERR_INVALID_PARAM, "invalid param");

    return sink->GetVolume(*left, *right);
}

int32_t SinkAdapterGetLatency(struct SinkAdapter *adapter, uint32_t *latency)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->renderId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");
    std::shared_ptr<IAudioRenderSink> sink = GetRenderSink(adapter->renderId);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_INVALID_HANDLE, "get sink fail");
    CHECK_AND_RETURN_RET_LOG(sink->IsInited(), ERR_ILLEGAL_STATE, "sink not init");
    CHECK_AND_RETURN_RET_LOG(latency != nullptr, ERR_INVALID_PARAM, "invalid param");

    return sink->GetLatency(*latency);
}

int32_t SinkAdapterGetPresentationPosition(struct SinkAdapter *adapter, uint64_t *frames, int64_t *timeSec,
    int64_t *timeNanoSec)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->renderId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");
    std::shared_ptr<IAudioRenderSink> sink = GetRenderSink(adapter->renderId);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_INVALID_HANDLE, "get sink fail");
    CHECK_AND_RETURN_RET_LOG(sink->IsInited(), ERR_ILLEGAL_STATE, "sink not init");
    CHECK_AND_RETURN_RET_LOG(frames != nullptr && timeSec != nullptr && timeNanoSec != nullptr, ERR_INVALID_PARAM,
        "invalid param");

    return sink->GetPresentationPosition(*frames, *timeSec, *timeNanoSec);
}

int32_t SinkAdapterGetAudioScene(struct SinkAdapter *adapter)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->renderId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");
    std::shared_ptr<IAudioRenderSink> sink = GetRenderSink(adapter->renderId);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_INVALID_HANDLE, "get sink fail");
    CHECK_AND_RETURN_RET_LOG(sink->IsInited(), ERR_ILLEGAL_STATE, "sink not init");

    return sink->GetAudioScene();
}

int32_t SinkAdapterSetPaPower(struct SinkAdapter *adapter, int32_t flag)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->renderId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");
    std::shared_ptr<IAudioRenderSink> sink = GetRenderSink(adapter->renderId);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_INVALID_HANDLE, "get sink fail");
    CHECK_AND_RETURN_RET_LOG(sink->IsInited(), ERR_ILLEGAL_STATE, "sink not init");

    return sink->SetPaPower(flag);
}

int32_t SinkAdapterSetPriPaPower(struct SinkAdapter *adapter)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->renderId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");
    std::shared_ptr<IAudioRenderSink> sink = GetRenderSink(adapter->renderId);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_INVALID_HANDLE, "get sink fail");
    CHECK_AND_RETURN_RET_LOG(sink->IsInited(), ERR_ILLEGAL_STATE, "sink not init");

    return sink->SetPriPaPower();
}

int32_t SinkAdapterUpdateAppsUid(struct SinkAdapter *adapter, const int32_t appsUid[MAX_MIX_CHANNELS],
    const size_t size)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->renderId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");
    std::shared_ptr<IAudioRenderSink> sink = GetRenderSink(adapter->renderId);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_INVALID_HANDLE, "get sink fail");
    CHECK_AND_RETURN_RET_LOG(sink->IsInited(), ERR_ILLEGAL_STATE, "sink not init");

    return sink->UpdateAppsUid(appsUid, size);
}

int32_t SinkAdapterRegistOffloadHdiCallback(struct SinkAdapter *adapter, int8_t *rawCallback, int8_t *userdata)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->renderId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");
    std::shared_ptr<IAudioRenderSink> sink = GetRenderSink(adapter->renderId);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_INVALID_HANDLE, "get sink fail");
    CHECK_AND_RETURN_RET_LOG(sink->IsInited(), ERR_ILLEGAL_STATE, "sink not init");
    CHECK_AND_RETURN_RET_LOG(rawCallback != nullptr, ERR_INVALID_PARAM, "invalid param");

    std::function<void(const RenderCallbackType type)> callback = [rawCallback, userdata]
        (const RenderCallbackType type) {
        reinterpret_cast<OnRenderCallback *>(rawCallback)(type, userdata);
    };
    sink->RegistOffloadHdiCallback(callback);
    return SUCCESS;
}

int32_t SinkAdapterSetBufferSize(struct SinkAdapter *adapter, uint32_t sizeMs)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->renderId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");
    std::shared_ptr<IAudioRenderSink> sink = GetRenderSink(adapter->renderId);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_INVALID_HANDLE, "get sink fail");
    CHECK_AND_RETURN_RET_LOG(sink->IsInited(), ERR_ILLEGAL_STATE, "sink not init");

    return sink->SetBufferSize(sizeMs);
}

int32_t SinkAdapterLockOffloadRunningLock(struct SinkAdapter *adapter)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->renderId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");
    std::shared_ptr<IAudioRenderSink> sink = GetRenderSink(adapter->renderId);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_INVALID_HANDLE, "get sink fail");
    CHECK_AND_RETURN_RET_LOG(sink->IsInited(), ERR_ILLEGAL_STATE, "sink not init");

    return sink->LockOffloadRunningLock();
}

int32_t SinkAdapterUnLockOffloadRunningLock(struct SinkAdapter *adapter)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->renderId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");
    std::shared_ptr<IAudioRenderSink> sink = GetRenderSink(adapter->renderId);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_INVALID_HANDLE, "get sink fail");
    CHECK_AND_RETURN_RET_LOG(sink->IsInited(), ERR_ILLEGAL_STATE, "sink not init");

    return sink->UnLockOffloadRunningLock();
}

int32_t SinkAdapterSplitRenderFrame(struct SinkAdapter *adapter, char *data, uint64_t len, uint64_t *writeLen,
    const char *streamType)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->renderId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");
    std::shared_ptr<IAudioRenderSink> sink = GetRenderSink(adapter->renderId);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_INVALID_HANDLE, "get sink fail");
    CHECK_AND_RETURN_RET_LOG(sink->IsInited(), ERR_ILLEGAL_STATE, "sink not init");

    return sink->SplitRenderFrame(*data, len, *writeLen, streamType);
}

int32_t SinkSetDeviceConnectedFlag(struct SinkAdapter *adapter, bool flag)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->renderId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");
    std::shared_ptr<IAudioRenderSink> sink = GetRenderSink(adapter->renderId);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_INVALID_HANDLE, "get sink fail");
    CHECK_AND_RETURN_RET_LOG(sink->IsInited(), ERR_ILLEGAL_STATE, "sink not init");
 
    return sink->SetDeviceConnectedFlag(flag);
}

#ifdef __cplusplus
}
#endif
