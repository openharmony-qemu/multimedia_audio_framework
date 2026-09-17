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
#include <functional>

using namespace std;

namespace OHOS {
namespace AudioStandard {

shared_ptr<AudioDeviceDescriptor> PrivacyPriorityRouter::GetMediaRenderDevice(StreamUsage streamUsage,
    int32_t clientUID, const uint32_t sessionID)
{
    vector<shared_ptr<AudioDeviceDescriptor>> descs =
        AudioDeviceManager::GetAudioDeviceManager().GetMediaRenderPrivacyDevices();
    AppendActiveScoDevice(descs);
    RouterBase::ClearConflictDevice(descs, GetRouterType());
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
}

bool PrivacyPriorityRouter::IsMediaOutputDisable(const AudioDeviceDescriptor &desc)
{
    auto &deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    vector<shared_ptr<AudioDeviceDescriptor>> descs;
    function<bool(const shared_ptr<AudioDeviceDescriptor> &)> isMediaOutDisable;

    if (IsBtInputDevice(desc.deviceType_)) {
        descs = deviceManager.GetDevicesByFilter(
            DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE, desc.macAddress_, "", CONNECTED);
        isMediaOutDisable = [](const shared_ptr<AudioDeviceDescriptor> &a2dp) {
            return a2dp != nullptr && !a2dp->isEnable_;
        };
    } else if (IsNearlinkInputDevice(desc.deviceType_)) {
        descs = deviceManager.GetDevicesByFilter(
            DEVICE_TYPE_NEARLINK, OUTPUT_DEVICE, desc.macAddress_, "", CONNECTED);
        isMediaOutDisable = [](const shared_ptr<AudioDeviceDescriptor> &nearlink) {
            return nearlink != nullptr && 0 == (nearlink->deviceUsage_ & DeviceUsage::MEDIA);
        };
    } else {
        return false;
    }
    return any_of(descs.begin(), descs.end(), isMediaOutDisable);
}

shared_ptr<AudioDeviceDescriptor> PrivacyPriorityRouter::GetCallRenderDevice(StreamUsage streamUsage,
    int32_t clientUID, const uint32_t sessionID)
{
    vector<shared_ptr<AudioDeviceDescriptor>> descs =
        AudioDeviceManager::GetAudioDeviceManager().GetCommRenderPrivacyDevices();
    RouterBase::ClearConflictDevice(descs, GetRouterType());

    if (streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION ||
        streamUsage == STREAM_USAGE_VOICE_TRANSFER_COMMUNICATION) {
        RemoveArmUsb(descs);
    }
    descs.erase(remove_if(descs.begin(), descs.end(), [streamUsage](auto &item) {
            return !AudioPolicyUtils::CheckInterphone(streamUsage, item);
        }), descs.end());

    shared_ptr<AudioDeviceDescriptor> desc = GetLatestNonExcludedConnectDevice(CALL_OUTPUT_DEVICES, descs);
    AUDIO_DEBUG_LOG("streamUsage %{public}d clientUID %{public}d fetch device %{public}d", streamUsage,
        clientUID, desc->deviceType_);
    return desc;
}

shared_ptr<AudioDeviceDescriptor> PrivacyPriorityRouter::GetCallCaptureDevice(SourceType sourceType,
    int32_t clientUID, const uint32_t sessionID)
{
    vector<shared_ptr<AudioDeviceDescriptor>> descs =
        AudioDeviceManager::GetAudioDeviceManager().GetCommCapturePrivacyDevices();
    RouterBase::ClearConflictDevice(descs, GetRouterType());
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
    if (streamUsage == STREAM_USAGE_VOICE_RINGTONE || streamUsage == STREAM_USAGE_RINGTONE ||
        streamUsage == STREAM_USAGE_VOICE_TRANSFER_RINGTONE) {
        curDescs = AudioDeviceManager::GetAudioDeviceManager().GetCommRenderPrivacyDevices();
    } else {
        curDescs = AudioDeviceManager::GetAudioDeviceManager().GetMediaRenderPrivacyDevices();
        audioDevUsage = MEDIA_OUTPUT_DEVICES;
    }

    AppendActiveScoDevice(curDescs);

    RouterBase::ClearConflictDevice(curDescs, GetRouterType());
    shared_ptr<AudioDeviceDescriptor> latestConnDesc = GetLatestNonExcludedConnectDevice(audioDevUsage, curDescs);
    if (!latestConnDesc.get() || latestConnDesc->getType() == DEVICE_TYPE_NONE) {
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
            case STREAM_USAGE_VOICE_TRANSFER_RINGTONE:
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
    int32_t clientUID, const uint32_t sessionID)
{
    const bool isScoSupportSource = Util::IsScoSupportSource(sourceType);
    const bool isSppSupportSource = Util::IsSppSupportSource(sourceType);
    auto sppDevice = AudioDeviceManager::GetAudioDeviceManager().GetSingleMicSppDevice();
    if (isScoSupportSource) {
        vector<shared_ptr<AudioDeviceDescriptor>> descs =
            AudioDeviceManager::GetAudioDeviceManager().GetRecongnitionCapturePrivacyDevices();
        if (sppDevice) {
            descs.erase(
                remove_if(descs.begin(), descs.end(), [&sppDevice](const shared_ptr<AudioDeviceDescriptor> &desc) {
                return desc != nullptr && desc->macAddress_ == sppDevice->macAddress_ &&
                    desc->deviceType_ == sppDevice->deviceType_;
                }), descs.end());
        }
        shared_ptr<AudioDeviceDescriptor> desc = GetLatestNonExcludedConnectDevice(MEDIA_INPUT_DEVICES, descs);
        if (desc && desc->deviceType_ != DEVICE_TYPE_NONE && !IsMediaOutputDisable(*desc)) {
            AUDIO_DEBUG_LOG("Recognition sourceType %{public}d clientUID %{public}d fetch device %{public}d",
                sourceType, clientUID, desc->deviceType_);
            return desc;
        }
    }

    vector<shared_ptr<AudioDeviceDescriptor>> descs =
        AudioDeviceManager::GetAudioDeviceManager().GetMediaCapturePrivacyDevices();
    if (isSppSupportSource && sppDevice) {
        descs.push_back(sppDevice);
    }
    RouterBase::ClearConflictDevice(descs, GetRouterType());
    shared_ptr<AudioDeviceDescriptor> desc = GetLatestNonExcludedConnectDevice(MEDIA_INPUT_DEVICES, descs);
    CHECK_AND_RETURN_RET_LOG(desc != nullptr, make_shared<AudioDeviceDescriptor>(), "nullptr desc");
    AUDIO_DEBUG_LOG("sourceType %{public}d clientUID %{public}d fetch device %{public}d", sourceType,
        clientUID, desc->deviceType_);

    return desc;
}

shared_ptr<AudioDeviceDescriptor> PrivacyPriorityRouter::GetToneRenderDevice(StreamUsage streamUsage,
    int32_t clientUID)
{
    return make_shared<AudioDeviceDescriptor>();
}

} // namespace AudioStandard
} // namespace OHOS
