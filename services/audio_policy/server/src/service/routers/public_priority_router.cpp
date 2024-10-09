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
#define LOG_TAG "PublicPriorityRouter"
#endif

#include "public_priority_router.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {

unique_ptr<AudioDeviceDescriptor> PublicPriorityRouter::GetMediaRenderDevice(StreamUsage streamUsage,
    int32_t clientUID)
{
    vector<unique_ptr<AudioDeviceDescriptor>> descs;
    if (streamUsage == STREAM_USAGE_RINGTONE || streamUsage == STREAM_USAGE_VOICE_RINGTONE) {
        descs = AudioDeviceManager::GetAudioDeviceManager().GetCommRenderPublicDevices();
        if (descs.size() == 0) {
            descs = AudioDeviceManager::GetAudioDeviceManager().GetMediaRenderPublicDevices();
        }
    } else {
        descs = AudioDeviceManager::GetAudioDeviceManager().GetMediaRenderPublicDevices();
    }
    unique_ptr<AudioDeviceDescriptor> desc = GetLatestConnectDeivce(descs);
    AUDIO_DEBUG_LOG("streamUsage %{public}d clientUID %{public}d fetch device %{public}d", streamUsage,
        clientUID, desc->deviceType_);
    return desc;
}

unique_ptr<AudioDeviceDescriptor> PublicPriorityRouter::GetCallRenderDevice(StreamUsage streamUsage,
    int32_t clientUID)
{
    return make_unique<AudioDeviceDescriptor>();
}

unique_ptr<AudioDeviceDescriptor> PublicPriorityRouter::GetCallCaptureDevice(SourceType sourceType,
    int32_t clientUID)
{
    return make_unique<AudioDeviceDescriptor>();
}

vector<std::unique_ptr<AudioDeviceDescriptor>> PublicPriorityRouter::GetRingRenderDevices(StreamUsage streamUsage,
    int32_t clientUID)
{
    AudioRingerMode curRingerMode = audioPolicyManager_.GetRingerMode();
    vector<unique_ptr<AudioDeviceDescriptor>> descs;
    vector<unique_ptr<AudioDeviceDescriptor>> curDescs;
    if (streamUsage == STREAM_USAGE_VOICE_RINGTONE || streamUsage == STREAM_USAGE_RINGTONE) {
        curDescs = AudioDeviceManager::GetAudioDeviceManager().GetCommRenderPublicDevices();
    } else {
        curDescs = AudioDeviceManager::GetAudioDeviceManager().GetMediaRenderPublicDevices();
    }

    unique_ptr<AudioDeviceDescriptor> latestConnDesc = GetLatestConnectDeivce(curDescs);
    if (!latestConnDesc.get()) {
        AUDIO_INFO_LOG("Have no latest connecte desc, just only add default device.");
        descs.push_back(make_unique<AudioDeviceDescriptor>());
        return descs;
    }
    if (latestConnDesc->getType() == DEVICE_TYPE_NONE) {
        AUDIO_INFO_LOG("Latest connecte desc type is none, just only add default device.");
        descs.push_back(make_unique<AudioDeviceDescriptor>());
        return descs;
    }

    if (latestConnDesc->getType() == DEVICE_TYPE_BLUETOOTH_SCO) {
        // Add the latest connected device.
        descs.push_back(move(latestConnDesc));
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
    } else if (latestConnDesc->getType() != DEVICE_TYPE_NONE) {
        descs.push_back(move(latestConnDesc));
    } else {
        descs.push_back(make_unique<AudioDeviceDescriptor>());
    }
    return descs;
}

unique_ptr<AudioDeviceDescriptor> PublicPriorityRouter::GetRecordCaptureDevice(SourceType sourceType,
    int32_t clientUID)
{
    vector<unique_ptr<AudioDeviceDescriptor>> descs =
        AudioDeviceManager::GetAudioDeviceManager().GetMediaCapturePublicDevices();
    unique_ptr<AudioDeviceDescriptor> desc = GetLatestConnectDeivce(descs);
    AUDIO_DEBUG_LOG("sourceType %{public}d clientUID %{public}d fetch device %{public}d", sourceType,
        clientUID, desc->deviceType_);
    return desc;
}

unique_ptr<AudioDeviceDescriptor> PublicPriorityRouter::GetToneRenderDevice(StreamUsage streamUsage,
    int32_t clientUID)
{
    return make_unique<AudioDeviceDescriptor>();
}

} // namespace AudioStandard
} // namespace OHOS