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
#define LOG_TAG "SourceAdapter"
#endif

#include "source/source_adapter.h"
#include "audio_errors.h"
#include "audio_hdi_log.h"
#include "source/i_audio_capture_source.h"
#include "manager/hdi_adapter_manager.h"
#include "capturer_clock_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

using namespace OHOS::AudioStandard;

static inline std::shared_ptr<IAudioCaptureSource> GetCaptureSource(uint32_t captureId)
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    return manager.GetCaptureSource(captureId, true);
}

int32_t InitSourceAdapter(struct SourceAdapter *adapter, const char *deviceClass, const int32_t sourceType,
    const char *info)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr, ERR_INVALID_HANDLE, "adapter is nullptr");

    adapter->captureId = HDI_INVALID_ID;
    adapter->deviceClass = strdup(deviceClass);
    CHECK_AND_RETURN_RET_LOG(adapter->deviceClass != nullptr, ERR_OPERATION_FAILED, "strdup fail");
    if (info == nullptr) {
        adapter->captureId = HdiAdapterManager::GetInstance().GetCaptureIdByDeviceClass(deviceClass,
            static_cast<SourceType>(sourceType), HDI_ID_INFO_DEFAULT, true);
    } else {
        adapter->captureId = HdiAdapterManager::GetInstance().GetCaptureIdByDeviceClass(deviceClass,
            static_cast<SourceType>(sourceType), std::string(info), true);
    }
    std::shared_ptr<IAudioCaptureSource> source = GetCaptureSource(adapter->captureId);
    if (source == nullptr) {
        AUDIO_ERR_LOG("get source fail, deviceClass: %{public}s, info: %{public}s, captureId: %{public}u",
            deviceClass, info, adapter->captureId);
        HdiAdapterManager::GetInstance().ReleaseId(adapter->captureId);
        return ERR_OPERATION_FAILED;
    }
    adapter->attr = nullptr;
    return SUCCESS;
}

void DeInitSourceAdapter(struct SourceAdapter *adapter)
{
    CHECK_AND_RETURN_LOG(adapter != nullptr, "adapter is nullptr");
    HdiAdapterManager::GetInstance().ReleaseId(adapter->captureId);
    if (adapter->deviceClass != nullptr) {
        free(const_cast<char *>(adapter->deviceClass));
        adapter->deviceClass = nullptr;
    }
}

int32_t SourceAdapterInit(struct SourceAdapter *adapter, const struct SourceAdapterAttr *attr)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->captureId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");
    CHECK_AND_RETURN_RET_LOG(attr != nullptr, ERR_INVALID_PARAM, "attr is nullptr");
    std::shared_ptr<IAudioCaptureSource> source = GetCaptureSource(adapter->captureId);
    CHECK_AND_RETURN_RET_LOG(source != nullptr, ERR_INVALID_HANDLE, "get source fail");
    if (source->IsInited()) {
        return SUCCESS;
    }

    IAudioSourceAttr sourceAttr = {};
    sourceAttr.adapterName = attr->adapterName;
    sourceAttr.openMicSpeaker = attr->openMicSpeaker;
    sourceAttr.format = static_cast<AudioSampleFormat>(attr->format);
    sourceAttr.sampleRate = attr->sampleRate;
    sourceAttr.channel = attr->channel;
    sourceAttr.volume = attr->volume;
    sourceAttr.bufferSize = attr->bufferSize;
    sourceAttr.isBigEndian = attr->isBigEndian;
    sourceAttr.filePath = attr->filePath;
    sourceAttr.deviceNetworkId = attr->deviceNetworkId;
    sourceAttr.deviceType = attr->deviceType;
    sourceAttr.sourceType = attr->sourceType;
    sourceAttr.channelLayout = attr->channelLayout;
    sourceAttr.hasEcConfig = attr->hasEcConfig;
    sourceAttr.formatEc = static_cast<AudioSampleFormat>(attr->formatEc);
    sourceAttr.sampleRateEc = attr->sampleRateEc;
    sourceAttr.channelEc = attr->channelEc;

    return source->Init(sourceAttr);
}

void SourceAdapterDeInit(struct SourceAdapter *adapter)
{
    CHECK_AND_RETURN_LOG(adapter != nullptr && adapter->captureId != HDI_INVALID_ID, "invalid adapter");
    std::shared_ptr<IAudioCaptureSource> source = GetCaptureSource(adapter->captureId);
    CHECK_AND_RETURN_LOG(source != nullptr, "get source fail");
    if (!source->IsInited()) {
        return;
    }

    source->DeInit();
}

int32_t SourceAdapterStart(struct SourceAdapter *adapter)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->captureId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");
    std::shared_ptr<IAudioCaptureSource> source = GetCaptureSource(adapter->captureId);
    CHECK_AND_RETURN_RET_LOG(source != nullptr, ERR_INVALID_HANDLE, "get source fail");
    CHECK_AND_RETURN_RET_LOG(source->IsInited(), ERR_ILLEGAL_STATE, "source not init");

    return source->Start();
}

int32_t SourceAdapterStop(struct SourceAdapter *adapter)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->captureId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");
    std::shared_ptr<IAudioCaptureSource> source = GetCaptureSource(adapter->captureId);
    CHECK_AND_RETURN_RET_LOG(source != nullptr, ERR_INVALID_HANDLE, "get source fail");
    CHECK_AND_RETURN_RET_LOG(source->IsInited(), ERR_ILLEGAL_STATE, "source not init");

    return source->Stop();
}

int32_t SourceAdapterCaptureFrame(struct SourceAdapter *adapter, char *frame, uint64_t requestBytes,
    uint64_t *replyBytes)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->captureId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");
    std::shared_ptr<IAudioCaptureSource> source = GetCaptureSource(adapter->captureId);
    CHECK_AND_RETURN_RET_LOG(source != nullptr, ERR_INVALID_HANDLE, "get source fail");
    CHECK_AND_RETURN_RET_LOG(source->IsInited(), ERR_ILLEGAL_STATE, "source not init");

    return source->CaptureFrame(frame, requestBytes, *replyBytes);
}

int32_t SourceAdapterCaptureFrameWithEc(struct SourceAdapter *adapter, struct SourceAdapterFrameDesc *fdesc,
    uint64_t *replyBytes, struct SourceAdapterFrameDesc *fdescEc, uint64_t *replyBytesEc)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->captureId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");
    std::shared_ptr<IAudioCaptureSource> source = GetCaptureSource(adapter->captureId);
    CHECK_AND_RETURN_RET_LOG(source != nullptr, ERR_INVALID_HANDLE, "get source fail");
    CHECK_AND_RETURN_RET_LOG(source->IsInited(), ERR_ILLEGAL_STATE, "source not init");

    return source->CaptureFrameWithEc(reinterpret_cast<FrameDesc *>(fdesc), *replyBytes,
        reinterpret_cast<FrameDesc *>(fdescEc), *replyBytesEc);
}

int32_t SourceAdapterSetVolume(struct SourceAdapter *adapter, float left, float right)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->captureId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");
    std::shared_ptr<IAudioCaptureSource> source = GetCaptureSource(adapter->captureId);
    CHECK_AND_RETURN_RET_LOG(source != nullptr, ERR_INVALID_HANDLE, "get source fail");
    CHECK_AND_RETURN_RET_LOG(source->IsInited(), ERR_ILLEGAL_STATE, "source not init");

    return source->SetVolume(left, right);
}

int32_t SourceAdapterGetVolume(struct SourceAdapter *adapter, float *left, float *right)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->captureId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");
    std::shared_ptr<IAudioCaptureSource> source = GetCaptureSource(adapter->captureId);
    CHECK_AND_RETURN_RET_LOG(source != nullptr, ERR_INVALID_HANDLE, "get source fail");
    CHECK_AND_RETURN_RET_LOG(source->IsInited(), ERR_ILLEGAL_STATE, "source not init");
    CHECK_AND_RETURN_RET_LOG(left != nullptr && right != nullptr, ERR_INVALID_PARAM, "invalid param");

    return source->GetVolume(*left, *right);
}

int32_t SourceAdapterSetMute(struct SourceAdapter *adapter, bool isMute)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->captureId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");
    std::shared_ptr<IAudioCaptureSource> source = GetCaptureSource(adapter->captureId);
    CHECK_AND_RETURN_RET_LOG(source != nullptr, ERR_INVALID_HANDLE, "get source fail");
    CHECK_AND_RETURN_RET_LOG(source->IsInited(), ERR_ILLEGAL_STATE, "source not init");

    return source->SetMute(isMute);
}

bool SourceAdapterGetMute(struct SourceAdapter *adapter)
{
    bool isMute = false;
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->captureId != HDI_INVALID_ID, isMute, "invalid adapter");
    std::shared_ptr<IAudioCaptureSource> source = GetCaptureSource(adapter->captureId);
    CHECK_AND_RETURN_RET_LOG(source != nullptr, isMute, "get source fail");
    CHECK_AND_RETURN_RET_LOG(source->IsInited(), isMute, "source not init");

    source->GetMute(isMute);
    return isMute;
}

int32_t SourceAdapterUpdateAppsUid(struct SourceAdapter *adapter, const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE],
    const size_t size)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->captureId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");
    std::shared_ptr<IAudioCaptureSource> source = GetCaptureSource(adapter->captureId);
    CHECK_AND_RETURN_RET_LOG(source != nullptr, ERR_INVALID_HANDLE, "get source fail");
    CHECK_AND_RETURN_RET_LOG(source->IsInited(), ERR_ILLEGAL_STATE, "source not init");

    return source->UpdateAppsUid(appsUid, size);
}

int32_t SourceAdapterUpdateSessionUid(struct SourceAdapter *adapter, const int32_t sessionId[PA_MAX_OUTPUTS_PER_SOURCE],
    const size_t size)
{
    CHECK_AND_RETURN_RET_LOG(adapter != nullptr && adapter->captureId != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "invalid adapter");

    std::shared_ptr<AudioSourceClock> clock =
        CapturerClockManager::GetInstance().GetAudioSourceClock(adapter->captureId);
    CHECK_AND_RETURN_RET_LOG(clock != nullptr, ERR_INVALID_HANDLE, "AudioSourceClock unfound!");

    std::vector<int32_t> sessionIdList;
    for (size_t i = 0; i < size; i++) {
        sessionIdList.push_back(sessionId[i]);
    }
    clock->UpdateSessionId(sessionIdList);

    return SUCCESS;
}

#ifdef __cplusplus
}
#endif
