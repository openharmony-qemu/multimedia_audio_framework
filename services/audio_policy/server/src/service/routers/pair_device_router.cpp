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
#define LOG_TAG "PairDeviceRouter"
#endif

#include "pair_device_router.h"

#include "audio_policy_service.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

unique_ptr<AudioDeviceDescriptor> PairDeviceRouter::GetMediaRenderDevice(StreamUsage streamUsage, int32_t clientUID)
{
    return make_unique<AudioDeviceDescriptor>();
}

unique_ptr<AudioDeviceDescriptor> PairDeviceRouter::GetCallRenderDevice(StreamUsage streamUsage, int32_t clientUID)
{
    return make_unique<AudioDeviceDescriptor>();
}

unique_ptr<AudioDeviceDescriptor> PairDeviceRouter::GetCallCaptureDevice(SourceType sourceType, int32_t clientUID)
{
    unique_ptr<AudioDeviceDescriptor> desc =
        AudioPolicyService::GetAudioPolicyService().GetActiveOutputDeviceDescriptor();
    std::shared_ptr<AudioDeviceDescriptor> pairDevice = desc->pairDeviceDescriptor_;
    if (pairDevice != nullptr && pairDevice->connectState_ != SUSPEND_CONNECTED && !pairDevice->exceptionFlag_ &&
        pairDevice->isEnable_) {
        AUDIO_DEBUG_LOG("sourceType %{public}d clientUID %{public}d fetch device %{public}d", sourceType, clientUID,
            pairDevice->deviceType_);
        return make_unique<AudioDeviceDescriptor>(*pairDevice);
    }
    return make_unique<AudioDeviceDescriptor>();
}

vector<std::unique_ptr<AudioDeviceDescriptor>> PairDeviceRouter::GetRingRenderDevices(StreamUsage streamUsage,
    int32_t clientUID)
{
    vector<unique_ptr<AudioDeviceDescriptor>> descs;
    return descs;
}

unique_ptr<AudioDeviceDescriptor> PairDeviceRouter::GetRecordCaptureDevice(SourceType sourceType, int32_t clientUID)
{
    return make_unique<AudioDeviceDescriptor>();
}

unique_ptr<AudioDeviceDescriptor> PairDeviceRouter::GetToneRenderDevice(StreamUsage streamUsage, int32_t clientUID)
{
    return make_unique<AudioDeviceDescriptor>();
}

} // namespace AudioStandard
} // namespace OHOS