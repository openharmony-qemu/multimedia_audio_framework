/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "IAudioCapturerSource"
#endif

#include "i_audio_capturer_source.h"

#include <cstring>
#include <string>

#include "audio_hdi_log.h"
#include "audio_errors.h"
#include "i_audio_capturer_source_intf.h"
#include "audio_capturer_source.h"
#include "audio_capturer_file_source.h"
#include "bluetooth_capturer_source.h"
#ifdef DAUDIO_ENABLE
#include "remote_audio_capturer_source.h"
#endif

using namespace std;

namespace OHOS {
namespace AudioStandard {
IAudioCapturerSource *IAudioCapturerSource::GetInstance(const char *deviceClass, const char *deviceNetworkId,
    const SourceType sourceType, const char *sourceName)
{
    AUDIO_DEBUG_LOG("%{public}s Source:GetInstance deviceNetworkId:[%{public}s] sourceType:[%{public}d]",
        deviceClass, deviceNetworkId, sourceType);
    const char *deviceClassPrimary = "primary";
    const char *deviceClassUsb = "usb";
    const char *deviceClassA2DP = "a2dp";
#ifdef FEATURE_FILE_IO
    const char *deviceClassFile = "file_io";
#endif
#ifdef DAUDIO_ENABLE
    const char *deviceClassRemote = "remote";
#endif

    if (!strcmp(deviceClass, deviceClassPrimary)) {
        return AudioCapturerSource::GetInstance("primary", sourceType, sourceName);
    }
    if (!strcmp(deviceClass, deviceClassUsb)) {
        return AudioCapturerSource::GetInstance("usb", sourceType, sourceName);
    }
    if (!strcmp(deviceClass, deviceClassA2DP)) {
        return BluetoothCapturerSource::GetInstance();
    }
#ifdef FEATURE_FILE_IO
    if (!strcmp(deviceClass, deviceClassFile)) {
        static AudioCapturerFileSource audioCapturer;
        return &audioCapturer;
    }
#endif
#ifdef DAUDIO_ENABLE
    if (!strcmp(deviceClass, deviceClassRemote)) {
        std::string networkId = deviceNetworkId;
        RemoteAudioCapturerSource *rSource = RemoteAudioCapturerSource::GetInstance(networkId);
        return rSource;
    }
#endif
    return nullptr;
}

void IAudioCapturerSource::GetAllInstance(std::vector<IAudioCapturerSource *> &allInstance)
{
#ifdef DAUDIO_ENABLE
    RemoteAudioCapturerSource::GetAllInstance(allInstance);
#endif
    allInstance.push_back(AudioCapturerSource::GetInstance());
    allInstance.push_back(AudioCapturerSource::GetInstance("usb", SourceType::SOURCE_TYPE_MIC, "Usb_Arm_Speaker_In"));
    allInstance.push_back(BluetoothCapturerSource::GetInstance());
}

IAudioCapturerSource *IAudioCapturerSource::Create(CaptureAttr *attr)
{
    return AudioCapturerSource::Create(attr);
}

} // namespace AudioStandard
} // namesapce OHOS

#ifdef __cplusplus
extern "C" {
#endif

using namespace OHOS::AudioStandard;

int32_t FillinSourceWapper(const char *deviceClass, const char *deviceNetworkId,
    const int32_t sourceType, const char *sourceName, void **wapper)
{
    IAudioCapturerSource *iSource = IAudioCapturerSource::GetInstance(deviceClass,
        deviceNetworkId,
        static_cast<SourceType>(sourceType),
        sourceName);

    if (iSource != nullptr) {
        *wapper = static_cast<void *>(iSource);
        return SUCCESS;
    }
    return ERROR;
}

IAudioCapturerSource *iAudioCapturerSource = nullptr;

int32_t IAudioCapturerSourceInit(void *wapper, const SourceAttr *attr)
{
    int32_t ret;

    IAudioCapturerSource *iAudioCapturerSource = static_cast<IAudioCapturerSource *>(wapper);
    CHECK_AND_RETURN_RET_LOG(iAudioCapturerSource != nullptr, ERR_INVALID_HANDLE, "null audioCapturerSource");
    if (iAudioCapturerSource->IsInited()) {
        return SUCCESS;
    }

    IAudioSourceAttr iAttr = {};
    iAttr.adapterName = attr->adapterName;
    iAttr.openMicSpeaker = attr->openMicSpeaker;
    iAttr.format = attr->format;
    iAttr.sampleRate = attr->sampleRate;
    iAttr.channel = attr->channel;
    iAttr.volume = attr->volume;
    iAttr.bufferSize = attr->bufferSize;
    iAttr.isBigEndian = attr->isBigEndian;
    iAttr.filePath = attr->filePath;
    iAttr.deviceNetworkId = attr->deviceNetworkId;
    iAttr.deviceType = attr->deviceType;
    iAttr.sourceType = attr->sourceType;
    iAttr.channelLayout = attr->channelLayout;
    iAttr.hasEcConfig = attr->hasEcConfig;
    iAttr.formatEc = attr->formatEc;
    iAttr.sampleRateEc = attr->sampleRateEc;
    iAttr.channelEc = attr->channelEc;
    ret = iAudioCapturerSource->Init(iAttr);

    return ret;
}

void IAudioCapturerSourceDeInit(void *wapper)
{
    IAudioCapturerSource *iAudioCapturerSource = static_cast<IAudioCapturerSource *>(wapper);
    CHECK_AND_RETURN_LOG(iAudioCapturerSource != nullptr, "null audioCapturerSource");
    if (iAudioCapturerSource->IsInited())
        iAudioCapturerSource->DeInit();
}

int32_t IAudioCapturerSourceStop(void *wapper)
{
    IAudioCapturerSource *iAudioCapturerSource = static_cast<IAudioCapturerSource *>(wapper);
    CHECK_AND_RETURN_RET_LOG(iAudioCapturerSource != nullptr, ERR_INVALID_HANDLE, "null audioCapturerSource");
    if (!iAudioCapturerSource->IsInited())
        return SUCCESS;

    int32_t ret = iAudioCapturerSource->Stop();

    return ret;
}

int32_t IAudioCapturerSourceStart(void *wapper)
{
    IAudioCapturerSource *iAudioCapturerSource = static_cast<IAudioCapturerSource *>(wapper);
    CHECK_AND_RETURN_RET_LOG(iAudioCapturerSource != nullptr, ERR_INVALID_HANDLE, "null audioCapturerSource");
    bool isInited = iAudioCapturerSource->IsInited();
    CHECK_AND_RETURN_RET_LOG(isInited, ERR_DEVICE_INIT,
        "audioCapturer Not Inited! Init the capturer first");

    int32_t ret = iAudioCapturerSource->Start();

    return ret;
}

int32_t IAudioCapturerSourceFrame(void *wapper, char *frame, uint64_t requestBytes, uint64_t *replyBytes)
{
    IAudioCapturerSource *iAudioCapturerSource = static_cast<IAudioCapturerSource *>(wapper);
    CHECK_AND_RETURN_RET_LOG(iAudioCapturerSource != nullptr, ERR_INVALID_HANDLE, "null audioCapturerSource");
    bool isInited = iAudioCapturerSource->IsInited();
    CHECK_AND_RETURN_RET_LOG(isInited, ERR_DEVICE_INIT,
        "audioCapturer Not Inited! Init the capturer first");

    int32_t ret = iAudioCapturerSource->CaptureFrame(frame, requestBytes, *replyBytes);

    return ret;
}

int32_t IAudioCapturerSourceFrameWithEc(void *wapper,
    FrameDesc *fdesc, uint64_t *replyBytes,
    FrameDesc *fdescEc, uint64_t *replyBytesEc)
{
    IAudioCapturerSource *iAudioCapturerSource = static_cast<IAudioCapturerSource *>(wapper);
    CHECK_AND_RETURN_RET_LOG(iAudioCapturerSource != nullptr, ERR_INVALID_HANDLE, "null audioCapturerSource");
    bool isInited = iAudioCapturerSource->IsInited();
    CHECK_AND_RETURN_RET_LOG(isInited, ERR_DEVICE_INIT,
        "audioCapturer Not Inited! Init the capturer first");

    return iAudioCapturerSource->CaptureFrameWithEc(fdesc, *replyBytes, fdescEc, *replyBytesEc);
}

int32_t IAudioCapturerSourceSetVolume(void *wapper, float left, float right)
{
    IAudioCapturerSource *iAudioCapturerSource = static_cast<IAudioCapturerSource *>(wapper);
    CHECK_AND_RETURN_RET_LOG(iAudioCapturerSource != nullptr, ERR_INVALID_HANDLE, "null audioCapturerSource");
    bool isInited = iAudioCapturerSource->IsInited();
    CHECK_AND_RETURN_RET_LOG(isInited, ERR_DEVICE_INIT,
        "audioCapturer Not Inited! Init the capturer first");

    int32_t ret = iAudioCapturerSource->SetVolume(left, right);

    return ret;
}

int32_t IAudioCapturerSourceGetVolume(void *wapper, float *left, float *right)
{
    IAudioCapturerSource *iAudioCapturerSource = static_cast<IAudioCapturerSource *>(wapper);
    CHECK_AND_RETURN_RET_LOG(iAudioCapturerSource != nullptr, ERR_INVALID_HANDLE, "null audioCapturerSource");
    bool isInited = iAudioCapturerSource->IsInited();
    CHECK_AND_RETURN_RET_LOG(isInited, ERR_DEVICE_INIT,
        "audioCapturer Not Inited! Init the capturer first");
    int32_t ret = iAudioCapturerSource->GetVolume(*left, *right);

    return ret;
}

bool IAudioCapturerSourceIsMuteRequired(void *wapper)
{
    bool muteStat = false;
    IAudioCapturerSource *iAudioCapturerSource = static_cast<IAudioCapturerSource *>(wapper);
    CHECK_AND_RETURN_RET_LOG(iAudioCapturerSource != nullptr, muteStat, "null audioCapturerSource");
    bool isInited = iAudioCapturerSource->IsInited();
    CHECK_AND_RETURN_RET_LOG(isInited, muteStat,
        "audioCapturer Not Inited! Init the capturer first");
    iAudioCapturerSource->GetMute(muteStat);
    return muteStat;
}

int32_t IAudioCapturerSourceSetMute(void *wapper, bool isMute)
{
    IAudioCapturerSource *iAudioCapturerSource = static_cast<IAudioCapturerSource *>(wapper);
    CHECK_AND_RETURN_RET_LOG(iAudioCapturerSource != nullptr, ERR_INVALID_HANDLE, "null audioCapturerSource");
    bool isInited = iAudioCapturerSource->IsInited();
    CHECK_AND_RETURN_RET_LOG(isInited, ERR_DEVICE_INIT,
        "audioCapturer Not Inited! Init the capturer first");

    int32_t ret = iAudioCapturerSource->SetMute(isMute);

    return ret;
}

int32_t IAudioCapturerSourceUpdateAppsUid(void *wapper, const int32_t appsUid[MAX_MIX_CHANNELS],
    const size_t size)
{
    IAudioCapturerSource *iAudioCapturerSource = static_cast<IAudioCapturerSource *>(wapper);
    CHECK_AND_RETURN_RET_LOG(iAudioCapturerSource != nullptr, ERR_INVALID_HANDLE, "null audioCapturerSource");
    bool isInited = iAudioCapturerSource->IsInited();
    CHECK_AND_RETURN_RET_LOG(isInited, ERR_DEVICE_INIT,
        "audioCapturer Not Inited! Init the capturer first");

    int32_t ret = iAudioCapturerSource->UpdateAppsUid(appsUid, size);

    return ret;
}

int32_t IAudioCapturerSourceGetCaptureId(void *wapper, uint32_t *captureId)
{
    IAudioCapturerSource *iAudioCapturerSource = static_cast<IAudioCapturerSource *>(wapper);
    CHECK_AND_RETURN_RET_LOG(iAudioCapturerSource != nullptr, ERR_INVALID_HANDLE, "null audioCapturerSource");
    bool isInited = iAudioCapturerSource->IsInited();
    CHECK_AND_RETURN_RET_LOG(isInited, ERR_DEVICE_INIT, "audioCapturer Not Inited! Init the capturer first");

    int32_t ret = iAudioCapturerSource->GetCaptureId(*captureId);
    return ret;
}
#ifdef __cplusplus
}
#endif
