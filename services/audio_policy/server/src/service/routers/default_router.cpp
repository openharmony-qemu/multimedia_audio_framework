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
#define LOG_TAG "DefaultRouter"
#endif

#include "default_router.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

shared_ptr<AudioDeviceDescriptor> DefaultRouter::GetMediaRenderDevice(StreamUsage streamUsage, int32_t clientUID)
{
    shared_ptr<AudioDeviceDescriptor> desc = AudioDeviceManager::GetAudioDeviceManager().GetSelectedMediaRenderDevice();
    if (desc == nullptr) {
        desc = AudioDeviceManager::GetAudioDeviceManager().GetRenderDefaultDevice();
    }
    AUDIO_DEBUG_LOG("streamUsage %{public}d clientUID %{public}d fetch device %{public}d", streamUsage, clientUID,
        desc->deviceType_);
    return desc;
}

shared_ptr<AudioDeviceDescriptor> DefaultRouter::GetCallRenderDevice(StreamUsage streamUsage, int32_t clientUID)
{
    shared_ptr<AudioDeviceDescriptor> desc = AudioDeviceManager::GetAudioDeviceManager().GetSelectedCallRenderDevice();
    if (desc == nullptr) {
        desc = AudioDeviceManager::GetAudioDeviceManager().GetCommRenderDefaultDevice(streamUsage);
    }
    AUDIO_DEBUG_LOG("streamUsage %{public}d clientUID %{public}d fetch device %{public}d", streamUsage, clientUID,
        desc->deviceType_);
    return desc;
}

shared_ptr<AudioDeviceDescriptor> DefaultRouter::GetCallCaptureDevice(SourceType sourceType, int32_t clientUID,
    const uint32_t sessionID)
{
    shared_ptr<AudioDeviceDescriptor> desc = AudioDeviceManager::GetAudioDeviceManager().GetCaptureDefaultDevice();
    AUDIO_DEBUG_LOG("sourceType %{public}d clientUID %{public}d fetch device %{public}d", sourceType, clientUID,
        desc->deviceType_);
    return desc;
}

vector<std::shared_ptr<AudioDeviceDescriptor>> DefaultRouter::GetRingRenderDevices(StreamUsage streamUsage,
    int32_t clientUID)
{
    vector<shared_ptr<AudioDeviceDescriptor>> descs;
    descs.push_back(AudioDeviceManager::GetAudioDeviceManager().GetRenderDefaultDevice());
    return descs;
}

shared_ptr<AudioDeviceDescriptor> DefaultRouter::GetRecordCaptureDevice(SourceType sourceType, int32_t clientUID,
    const uint32_t sessionID)
{
    shared_ptr<AudioDeviceDescriptor> desc = AudioDeviceManager::GetAudioDeviceManager().GetCaptureDefaultDevice();
    AUDIO_DEBUG_LOG("sourceType %{public}d clientUID %{public}d fetch device %{public}d", sourceType, clientUID,
        desc->deviceType_);
    return desc;
}

shared_ptr<AudioDeviceDescriptor> DefaultRouter::GetToneRenderDevice(StreamUsage streamUsage, int32_t clientUID)
{
    shared_ptr<AudioDeviceDescriptor> desc = AudioDeviceManager::GetAudioDeviceManager().GetRenderDefaultDevice();
    AUDIO_DEBUG_LOG("streamUsage %{public}d clientUID %{public}d fetch device %{public}d", streamUsage, clientUID,
        desc->deviceType_);
    return desc;
}

} // namespace AudioStandard
} // namespace OHOS