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
#define LOG_TAG "PrivacyPriorityRouter"
#endif

#include "privacy_priority_router.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

shared_ptr<AudioDeviceDescriptor> PrivacyPriorityRouter::GetMediaRenderDevice(StreamUsage streamUsage,
    int32_t clientUID)
{
    vector<shared_ptr<AudioDeviceDescriptor>> descs =
        AudioDeviceManager::GetAudioDeviceManager().GetMediaRenderPrivacyDevices();
    shared_ptr<AudioDeviceDescriptor> desc = GetLatestNonExcludedConnectDevice(MEDIA_OUTPUT_DEVICES, descs);
    AUDIO_DEBUG_LOG("streamUsage %{public}d clientUID %{public}d fetch device %{public}d", streamUsage,
        clientUID, desc->deviceType_);
    return desc;
}

void PrivacyPriorityRouter::RemoveArmUsb(vector<shared_ptr<AudioDeviceDescriptor>> &descs)
{
    auto isPresent = [] (const shared_ptr<AudioDeviceDescriptor> &desc) {
        CHECK_AND_RETURN_RET_LOG(desc != nullptr, false, "Invalid device descriptor");
        return desc->deviceType_ == DEVICE_TYPE_USB_ARM_HEADSET;
    };

    auto removeBeginIt = std::remove_if(descs.begin(), descs.end(), isPresent);
    size_t deleteNum = static_cast<uint32_t>(descs.end() - removeBeginIt);
    descs.erase(removeBeginIt, descs.end());
    AUDIO_INFO_LOG("Remove %{public}zu desc from privacy list", deleteNum);
}

shared_ptr<AudioDeviceDescriptor> PrivacyPriorityRouter::GetCallRenderDevice(StreamUsage streamUsage,
    int32_t clientUID)
{
    vector<shared_ptr<AudioDeviceDescriptor>> descs =
        AudioDeviceManager::GetAudioDeviceManager().GetCommRenderPrivacyDevices();

    if (streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION) {
        RemoveArmUsb(descs);
    }

    shared_ptr<AudioDeviceDescriptor> desc = GetLatestNonExcludedConnectDevice(CALL_OUTPUT_DEVICES, descs);
    AUDIO_DEBUG_LOG("streamUsage %{public}d clientUID %{public}d fetch device %{public}d", streamUsage,
        clientUID, desc->deviceType_);
    return desc;
}

shared_ptr<AudioDeviceDescriptor> PrivacyPriorityRouter::GetCallCaptureDevice(SourceType sourceType,
    int32_t clientUID)
{
    vector<shared_ptr<AudioDeviceDescriptor>> descs =
        AudioDeviceManager::GetAudioDeviceManager().GetCommCapturePrivacyDevices();
    shared_ptr<AudioDeviceDescriptor> desc = GetLatestNonExcludedConnectDevice(CALL_INPUT_DEVICES, descs);
    AUDIO_DEBUG_LOG("sourceType %{public}d clientUID %{public}d fetch device %{public}d", sourceType,
        clientUID, desc->deviceType_);
    return desc;
}

vector<std::shared_ptr<AudioDeviceDescriptor>> PrivacyPriorityRouter::GetRingRenderDevices(StreamUsage streamUsage,
    int32_t clientUID)
{
    AudioRingerMode curRingerMode = audioPolicyManager_.GetRingerMode();
    vector<shared_ptr<AudioDeviceDescriptor>> descs;
    vector<shared_ptr<AudioDeviceDescriptor>> curDescs;
    AudioDeviceUsage audioDevUsage = CALL_OUTPUT_DEVICES;
    if (streamUsage == STREAM_USAGE_VOICE_RINGTONE || streamUsage == STREAM_USAGE_RINGTONE) {
        curDescs = AudioDeviceManager::GetAudioDeviceManager().GetCommRenderPrivacyDevices();
    } else {
        curDescs = AudioDeviceManager::GetAudioDeviceManager().GetMediaRenderPrivacyDevices();
        audioDevUsage = MEDIA_OUTPUT_DEVICES;
    }

    shared_ptr<AudioDeviceDescriptor> latestConnDesc = GetLatestNonExcludedConnectDevice(audioDevUsage, curDescs);
    if (!latestConnDesc.get()) {
        AUDIO_INFO_LOG("Have no latest connecte desc, just only add default device.");
        descs.push_back(make_shared<AudioDeviceDescriptor>());
        return descs;
    }
    if (latestConnDesc->getType() == DEVICE_TYPE_NONE) {
        AUDIO_INFO_LOG("Latest connecte desc type is none, just only add default device.");
        descs.push_back(make_shared<AudioDeviceDescriptor>());
        return descs;
    }

    if (NeedLatestConnectWithDefaultDevices(latestConnDesc->getType())) {
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
        descs.push_back(make_shared<AudioDeviceDescriptor>());
    }
    return descs;
}

shared_ptr<AudioDeviceDescriptor> PrivacyPriorityRouter::GetRecordCaptureDevice(SourceType sourceType,
    int32_t clientUID)
{
    if (Util::IsScoSupportSource(sourceType)) {
        vector<shared_ptr<AudioDeviceDescriptor>> descs =
            AudioDeviceManager::GetAudioDeviceManager().GetRecongnitionCapturePrivacyDevices();
        shared_ptr<AudioDeviceDescriptor> desc = GetLatestNonExcludedConnectDevice(CALL_INPUT_DEVICES, descs);
        if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
            AUDIO_DEBUG_LOG("Recognition sourceType %{public}d clientUID %{public}d fetch device %{public}d",
                sourceType, clientUID, desc->deviceType_);
            return desc;
        }
    }
    vector<shared_ptr<AudioDeviceDescriptor>> descs =
        AudioDeviceManager::GetAudioDeviceManager().GetMediaCapturePrivacyDevices();
    shared_ptr<AudioDeviceDescriptor> desc = GetLatestNonExcludedConnectDevice(MEDIA_INPUT_DEVICES, descs);
    AUDIO_DEBUG_LOG("sourceType %{public}d clientUID %{public}d fetch device %{public}d", sourceType,
        clientUID, desc->deviceType_);

    // bluetooth sco device, need skip
    if (desc == nullptr) {
        AUDIO_ERR_LOG("nullptr desc");
        return make_shared<AudioDeviceDescriptor>();
    }
    if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
        AUDIO_INFO_LOG("record scene, skip bluetooth sco router");
        return make_shared<AudioDeviceDescriptor>();
    }

    return desc;
}

shared_ptr<AudioDeviceDescriptor> PrivacyPriorityRouter::GetToneRenderDevice(StreamUsage streamUsage,
    int32_t clientUID)
{
    return make_shared<AudioDeviceDescriptor>();
}

} // namespace AudioStandard
} // namespace OHOS