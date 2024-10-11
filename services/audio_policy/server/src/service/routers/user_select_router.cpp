/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "UserSelectRouter"
#endif

#include "user_select_router.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
unique_ptr<AudioDeviceDescriptor> UserSelectRouter::GetMediaRenderDevice(StreamUsage streamUsage, int32_t clientUID)
{
    if (streamUsage == STREAM_USAGE_RINGTONE || streamUsage == STREAM_USAGE_VOICE_RINGTONE) {
        AUDIO_INFO_LOG("Ringtone skip user select");
        return make_unique<AudioDeviceDescriptor>();
    }
    unique_ptr<AudioDeviceDescriptor> perDev_ =
        AudioStateManager::GetAudioStateManager().GetPreferredMediaRenderDevice();
    vector<unique_ptr<AudioDeviceDescriptor>> mediaDevices =
        AudioDeviceManager::GetAudioDeviceManager().GetAvailableDevicesByUsage(MEDIA_OUTPUT_DEVICES);
    if (perDev_->deviceId_ == 0) {
        AUDIO_DEBUG_LOG(" PreferredMediaRenderDevice is null");
        return make_unique<AudioDeviceDescriptor>();
    } else {
        int32_t audioId = perDev_->deviceId_;
        AUDIO_INFO_LOG(" PreferredMediaRenderDevice audioId is %{public}d", audioId);
        return RouterBase::GetPairCaptureDevice(perDev_, mediaDevices);
    }
}

unique_ptr<AudioDeviceDescriptor> UserSelectRouter::GetCallRenderDevice(StreamUsage streamUsage, int32_t clientUID)
{
    unique_ptr<AudioDeviceDescriptor> perDev_ =
        AudioStateManager::GetAudioStateManager().GetPreferredCallRenderDevice();
    vector<unique_ptr<AudioDeviceDescriptor>> callDevices =
        AudioDeviceManager::GetAudioDeviceManager().GetAvailableDevicesByUsage(CALL_OUTPUT_DEVICES);
    if (perDev_->deviceId_ == 0) {
        AUDIO_DEBUG_LOG(" PreferredCallRenderDevice is null");
        return make_unique<AudioDeviceDescriptor>();
    } else {
        int32_t audioId = perDev_->deviceId_;
        AUDIO_INFO_LOG(" PreferredCallRenderDevice audioId is %{public}d", audioId);
        return RouterBase::GetPairCaptureDevice(perDev_, callDevices);
    }
}

unique_ptr<AudioDeviceDescriptor> UserSelectRouter::GetCallCaptureDevice(SourceType sourceType, int32_t clientUID)
{
    unique_ptr<AudioDeviceDescriptor> perDev_ =
        AudioStateManager::GetAudioStateManager().GetPreferredCallCaptureDevice();
    vector<unique_ptr<AudioDeviceDescriptor>> callDevices =
        AudioDeviceManager::GetAudioDeviceManager().GetAvailableDevicesByUsage(CALL_INPUT_DEVICES);
    if (perDev_->deviceId_ == 0) {
        AUDIO_DEBUG_LOG(" PreferredCallCaptureDevice is null");
        return make_unique<AudioDeviceDescriptor>();
    } else {
        int32_t audioId = perDev_->deviceId_;
        AUDIO_INFO_LOG(" PreferredCallCaptureDevice audioId is %{public}d", audioId);
        return RouterBase::GetPairCaptureDevice(perDev_, callDevices);
    }
}

vector<std::unique_ptr<AudioDeviceDescriptor>> UserSelectRouter::GetRingRenderDevices(StreamUsage streamUsage,
    int32_t clientUID)
{
    vector<unique_ptr<AudioDeviceDescriptor>> descs;
    return descs;
}

unique_ptr<AudioDeviceDescriptor> UserSelectRouter::GetRecordCaptureDevice(SourceType sourceType, int32_t clientUID)
{
    unique_ptr<AudioDeviceDescriptor> perDev_ =
        AudioStateManager::GetAudioStateManager().GetPreferredRecordCaptureDevice();
    vector<unique_ptr<AudioDeviceDescriptor>> recordDevices =
        AudioDeviceManager::GetAudioDeviceManager().GetAvailableDevicesByUsage(MEDIA_INPUT_DEVICES);
    if (perDev_->deviceId_ == 0) {
        AUDIO_DEBUG_LOG(" PreferredRecordCaptureDevice is null");
        return make_unique<AudioDeviceDescriptor>();
    } else {
        int32_t audioId = perDev_->deviceId_;
        AUDIO_INFO_LOG(" PreferredRecordCaptureDevice audioId is %{public}d", audioId);
        return RouterBase::GetPairCaptureDevice(perDev_, recordDevices);
    }
}

unique_ptr<AudioDeviceDescriptor> UserSelectRouter::GetToneRenderDevice(StreamUsage streamUsage, int32_t clientUID)
{
    return make_unique<AudioDeviceDescriptor>();
}

} // namespace AudioStandard
} // namespace OHOS