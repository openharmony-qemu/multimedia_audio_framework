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
shared_ptr<AudioDeviceDescriptor> UserSelectRouter::GetMediaRenderDevice(StreamUsage streamUsage, int32_t clientUID)
{
    if (streamUsage == STREAM_USAGE_RINGTONE || streamUsage == STREAM_USAGE_VOICE_RINGTONE) {
        AUDIO_INFO_LOG("Ringtone skip user select");
        return make_shared<AudioDeviceDescriptor>();
    }
    shared_ptr<AudioDeviceDescriptor> perDev_ =
        AudioStateManager::GetAudioStateManager().GetPreferredMediaRenderDevice();
    vector<shared_ptr<AudioDeviceDescriptor>> mediaDevices =
        AudioDeviceManager::GetAudioDeviceManager().GetAvailableDevicesByUsage(MEDIA_OUTPUT_DEVICES);
    if (perDev_->deviceId_ == 0) {
        AUDIO_DEBUG_LOG(" PreferredMediaRenderDevice is null");
        return make_shared<AudioDeviceDescriptor>();
    } else {
        int32_t audioId = perDev_->deviceId_;
        AUDIO_INFO_LOG(" PreferredMediaRenderDevice audioId is %{public}d", audioId);
        return RouterBase::GetPairDevice(perDev_, mediaDevices);
    }
}

shared_ptr<AudioDeviceDescriptor> UserSelectRouter::GetCallRenderDevice(StreamUsage streamUsage, int32_t clientUID)
{
    shared_ptr<AudioDeviceDescriptor> perDev_ =
        AudioStateManager::GetAudioStateManager().GetPreferredCallRenderDevice();
    vector<shared_ptr<AudioDeviceDescriptor>> callDevices =
        AudioDeviceManager::GetAudioDeviceManager().GetAvailableDevicesByUsage(CALL_OUTPUT_DEVICES);
    if (perDev_->deviceId_ == 0) {
        AUDIO_DEBUG_LOG(" PreferredCallRenderDevice is null");
        return make_shared<AudioDeviceDescriptor>();
    } else {
        int32_t audioId = perDev_->deviceId_;
        AUDIO_INFO_LOG(" PreferredCallRenderDevice audioId is %{public}d", audioId);
        return RouterBase::GetPairDevice(perDev_, callDevices);
    }
}

shared_ptr<AudioDeviceDescriptor> UserSelectRouter::GetCallCaptureDevice(SourceType sourceType, int32_t clientUID)
{
    shared_ptr<AudioDeviceDescriptor> perDev_ =
        AudioStateManager::GetAudioStateManager().GetPreferredCallCaptureDevice();
    vector<shared_ptr<AudioDeviceDescriptor>> callDevices =
        AudioDeviceManager::GetAudioDeviceManager().GetAvailableDevicesByUsage(CALL_INPUT_DEVICES);
    if (perDev_->deviceId_ == 0) {
        AUDIO_DEBUG_LOG(" PreferredCallCaptureDevice is null");
        return make_shared<AudioDeviceDescriptor>();
    } else {
        int32_t audioId = perDev_->deviceId_;
        AUDIO_INFO_LOG(" PreferredCallCaptureDevice audioId is %{public}d", audioId);
        return RouterBase::GetPairDevice(perDev_, callDevices);
    }
}

vector<std::shared_ptr<AudioDeviceDescriptor>> UserSelectRouter::GetRingRenderDevices(StreamUsage streamUsage,
    int32_t clientUID)
{
    vector<shared_ptr<AudioDeviceDescriptor>> descs;
    AudioRingerMode curRingerMode = audioPolicyManager_.GetRingerMode();
    shared_ptr<AudioDeviceDescriptor> selectedDesc =
        (streamUsage == STREAM_USAGE_VOICE_RINGTONE || streamUsage == STREAM_USAGE_RINGTONE) ?
        GetCallRenderDevice(streamUsage, clientUID) : GetMediaRenderDevice(streamUsage, clientUID);
      
    if (!selectedDesc.get()) {
        AUDIO_INFO_LOG("Have no selected connected desc, just only add default device.");
        descs.push_back(make_shared<AudioDeviceDescriptor>());
        return descs;
    }
    if (selectedDesc->getType() == DEVICE_TYPE_NONE) {
        AUDIO_INFO_LOG("Selected connected desc type is none, just only add default device.");
        descs.push_back(make_shared<AudioDeviceDescriptor>());
        return descs;
    }
    if (selectedDesc->getType() == DEVICE_TYPE_BLUETOOTH_A2DP && selectedDesc->GetDeviceCategory() == BT_SOUNDBOX) {
        AUDIO_INFO_LOG("Exclude BT soundbox device for alarm stream.");
        descs.push_back(make_shared<AudioDeviceDescriptor>());
        return descs;
    }

    if (NeedLatestConnectWithDefaultDevices(selectedDesc->getType())) {
        // Add the latest connected device.
        descs.push_back(move(selectedDesc));
        switch (streamUsage) {
            case STREAM_USAGE_ALARM:
                // Add default device at same time for alarm.
                descs.push_back(AudioDeviceManager::GetAudioDeviceManager().GetRenderDefaultDevice());
                break;
            case STREAM_USAGE_VOICE_RINGTONE:
            case STREAM_USAGE_RINGTONE:
                if (curRingerMode == RINGER_MODE_NORMAL) {
                    // Add default devices at same time only in ringer normal mode.
                    descs.push_back(AudioDeviceManager::GetAudioDeviceManager().GetRenderDefaultDevice());
                }
                break;
            default:
                AUDIO_DEBUG_LOG("Don't add default device at the same time.");
                break;
        }
    } else if (selectedDesc->getType() != DEVICE_TYPE_NONE) {
        descs.push_back(move(selectedDesc));
    } else {
        descs.push_back(make_shared<AudioDeviceDescriptor>());
    }
    return descs;
}

shared_ptr<AudioDeviceDescriptor> UserSelectRouter::GetRecordCaptureDevice(SourceType sourceType, int32_t clientUID)
{
    shared_ptr<AudioDeviceDescriptor> perDev_ =
        AudioStateManager::GetAudioStateManager().GetPreferredRecordCaptureDevice();
    vector<shared_ptr<AudioDeviceDescriptor>> recordDevices =
        AudioDeviceManager::GetAudioDeviceManager().GetAvailableDevicesByUsage(MEDIA_INPUT_DEVICES);
    if (perDev_->deviceId_ == 0) {
        AUDIO_DEBUG_LOG(" PreferredRecordCaptureDevice is null");
        return make_shared<AudioDeviceDescriptor>();
    } else {
        int32_t audioId = perDev_->deviceId_;
        AUDIO_INFO_LOG(" PreferredRecordCaptureDevice audioId is %{public}d", audioId);
        return RouterBase::GetPairDevice(perDev_, recordDevices);
    }
}

shared_ptr<AudioDeviceDescriptor> UserSelectRouter::GetToneRenderDevice(StreamUsage streamUsage, int32_t clientUID)
{
    return make_shared<AudioDeviceDescriptor>();
}

} // namespace AudioStandard
} // namespace OHOS