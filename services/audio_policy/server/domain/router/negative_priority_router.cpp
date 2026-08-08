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
#define LOG_TAG "NegativePriorityRouter"
#endif

#include "negative_priority_router.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

void NegativePriorityRouter::FilterUnwearHeadphones(vector<shared_ptr<AudioDeviceDescriptor>> &descs)
{
    descs.erase(
        remove_if(descs.begin(), descs.end(),
            [] (const shared_ptr<AudioDeviceDescriptor> &desc) {
                return desc != nullptr && desc->deviceCategory_ == BT_UNWEAR_HEADPHONE;
            }),
        descs.end()
    );
}

shared_ptr<AudioDeviceDescriptor> NegativePriorityRouter::GetMediaRenderDevice(StreamUsage streamUsage,
    int32_t clientUID, const uint32_t sessionID)
{
    vector<shared_ptr<AudioDeviceDescriptor>> descs =
        AudioDeviceManager::GetAudioDeviceManager().GetMediaRenderNegativeDevices();
    FilterUnwearHeadphones(descs);
    RouterBase::ClearConflictDevice(descs, GetRouterType());
    shared_ptr<AudioDeviceDescriptor> desc = GetLatestNonExcludedConnectDevice(MEDIA_OUTPUT_DEVICES, descs);
    AUDIO_DEBUG_LOG("streamUsage %{public}d clientUID %{public}d fetch device %{public}d", streamUsage,
        clientUID, desc->deviceType_);
    return desc;
}

shared_ptr<AudioDeviceDescriptor> NegativePriorityRouter::GetCallRenderDevice(StreamUsage streamUsage,
    int32_t clientUID, const uint32_t sessionID)
{
    vector<shared_ptr<AudioDeviceDescriptor>> descs =
        AudioDeviceManager::GetAudioDeviceManager().GetCommRenderNegativeDevices();
    FilterUnwearHeadphones(descs);
    RouterBase::ClearConflictDevice(descs, GetRouterType());
    shared_ptr<AudioDeviceDescriptor> desc = GetLatestNonExcludedConnectDevice(CALL_OUTPUT_DEVICES, descs);
    AUDIO_DEBUG_LOG("streamUsage %{public}d clientUID %{public}d fetch device %{public}d", streamUsage,
        clientUID, desc->deviceType_);
    return desc;
}

shared_ptr<AudioDeviceDescriptor> NegativePriorityRouter::GetCallCaptureDevice(SourceType sourceType,
    int32_t clientUID, const uint32_t sessionID)
{
    vector<shared_ptr<AudioDeviceDescriptor>> descs =
        AudioDeviceManager::GetAudioDeviceManager().GetCommCaptureNegativeDevices();
    FilterUnwearHeadphones(descs);
    RouterBase::ClearConflictDevice(descs, GetRouterType());
    shared_ptr<AudioDeviceDescriptor> desc = GetLatestNonExcludedConnectDevice(CALL_INPUT_DEVICES, descs);
    AUDIO_DEBUG_LOG("sourceType %{public}d clientUID %{public}d fetch device %{public}d", sourceType,
        clientUID, desc->deviceType_);
    return desc;
}

vector<std::shared_ptr<AudioDeviceDescriptor>> NegativePriorityRouter::GetRingRenderDevices(StreamUsage streamUsage,
    int32_t clientUID)
{
    vector<shared_ptr<AudioDeviceDescriptor>> descs;
    vector<shared_ptr<AudioDeviceDescriptor>> curDescs;
    AudioDeviceUsage audioDevUsage = CALL_OUTPUT_DEVICES;
    if (streamUsage == STREAM_USAGE_VOICE_RINGTONE || streamUsage == STREAM_USAGE_RINGTONE ||
        streamUsage == STREAM_USAGE_VOICE_TRANSFER_RINGTONE) {
        curDescs = AudioDeviceManager::GetAudioDeviceManager().GetCommRenderNegativeDevices();
    } else {
        curDescs = AudioDeviceManager::GetAudioDeviceManager().GetMediaRenderNegativeDevices();
        audioDevUsage = MEDIA_OUTPUT_DEVICES;
    }

    FilterUnwearHeadphones(curDescs);
    RouterBase::ClearConflictDevice(curDescs, GetRouterType());
    shared_ptr<AudioDeviceDescriptor> latestConnDesc = GetLatestNonExcludedConnectDevice(audioDevUsage, curDescs);
    if (!latestConnDesc.get() || latestConnDesc->getType() == DEVICE_TYPE_NONE) {
        descs.push_back(make_shared<AudioDeviceDescriptor>());
        return descs;
    }

    if (NeedLatestConnectWithDefaultDevices(latestConnDesc->getType())) {
        descs.push_back(move(latestConnDesc));
        switch (streamUsage) {
            case STREAM_USAGE_ALARM:
                descs.push_back(AudioDeviceManager::GetAudioDeviceManager().GetRenderDefaultDevice());
                break;
            case STREAM_USAGE_VOICE_RINGTONE:
            case STREAM_USAGE_RINGTONE:
            case STREAM_USAGE_VOICE_TRANSFER_RINGTONE:
                if (audioPolicyManager_.GetRingerMode() == RINGER_MODE_NORMAL) {
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

shared_ptr<AudioDeviceDescriptor> NegativePriorityRouter::GetRecordCaptureDevice(SourceType sourceType,
    int32_t clientUID, const uint32_t sessionID)
{
    vector<shared_ptr<AudioDeviceDescriptor>> descs =
        AudioDeviceManager::GetAudioDeviceManager().GetMediaCaptureNegativeDevices();
    FilterUnwearHeadphones(descs);
    RouterBase::ClearConflictDevice(descs, GetRouterType());
    shared_ptr<AudioDeviceDescriptor> desc = GetLatestNonExcludedConnectDevice(MEDIA_INPUT_DEVICES, descs);
    CHECK_AND_RETURN_RET_LOG(desc != nullptr, make_shared<AudioDeviceDescriptor>(), "nullptr desc");
    AUDIO_DEBUG_LOG("sourceType %{public}d clientUID %{public}d fetch device %{public}d", sourceType,
        clientUID, desc->deviceType_);
    return desc;
}

shared_ptr<AudioDeviceDescriptor> NegativePriorityRouter::GetToneRenderDevice(StreamUsage streamUsage,
    int32_t clientUID)
{
    return make_shared<AudioDeviceDescriptor>();
}

} // namespace AudioStandard
} // namespace OHOS
