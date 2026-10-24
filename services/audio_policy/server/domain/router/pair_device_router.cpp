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

#include "audio_bluetooth_manager.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

shared_ptr<AudioDeviceDescriptor> PairDeviceRouter::GetMediaRenderDevice(StreamUsage streamUsage, int32_t clientUID,
    const uint32_t sessionID)
{
    return make_shared<AudioDeviceDescriptor>();
}

shared_ptr<AudioDeviceDescriptor> PairDeviceRouter::GetCallRenderDevice(StreamUsage streamUsage, int32_t clientUID,
    const uint32_t sessionID)
{
    return make_shared<AudioDeviceDescriptor>();
}

shared_ptr<AudioDeviceDescriptor> PairDeviceRouter::GetCallCaptureDevice(SourceType sourceType, int32_t clientUID,
    const uint32_t sessionID)
{
    auto id = AudioRouterSelectStrategy::GetInstance().Get1stCurrentOutputDevice(clientUID).deviceId_;
    auto desc = AudioDeviceManager::GetAudioDeviceManager().FindConnectedDeviceById(id);
    CHECK_AND_RETURN_RET_LOG(desc, make_shared<AudioDeviceDescriptor>(), "desc is nullptr");
    std::shared_ptr<AudioDeviceDescriptor> pairDevice = desc->pairDeviceDescriptor_;
    CHECK_AND_RETURN_RET(pairDevice, make_shared<AudioDeviceDescriptor>());
    // SIMULATED_CONNECTED devices are virtual and must not be used for call capture
    CHECK_AND_RETURN_RET(pairDevice->connectState_ != SIMULATED_CONNECTED, make_shared<AudioDeviceDescriptor>());
    bool deviceAvailable = pairDevice->connectState_ != SUSPEND_CONNECTED && !pairDevice->exceptionFlag_ &&
        pairDevice->isEnable_;
    if (deviceAvailable) {
        CHECK_AND_RETURN_RET_LOG(pairDevice->deviceType_ != DEVICE_TYPE_BLUETOOTH_SCO ||
            Bluetooth::AudioHfpManager::IsAudioScoStateConnect(),
            make_shared<AudioDeviceDescriptor>(), "sco is not connected or connecting");

        AUDIO_DEBUG_LOG("sourceType %{public}d clientUID %{public}d fetch device %{public}d", sourceType, clientUID,
            pairDevice->deviceType_);
        return make_shared<AudioDeviceDescriptor>(*pairDevice);
    }
    return make_shared<AudioDeviceDescriptor>();
}

vector<std::shared_ptr<AudioDeviceDescriptor>> PairDeviceRouter::GetRingRenderDevices(StreamUsage streamUsage,
    int32_t clientUID)
{
    vector<shared_ptr<AudioDeviceDescriptor>> descs;
    return descs;
}

shared_ptr<AudioDeviceDescriptor> PairDeviceRouter::GetRecordCaptureDevice(SourceType sourceType, int32_t clientUID,
    const uint32_t sessionID)
{
    return make_shared<AudioDeviceDescriptor>();
}

shared_ptr<AudioDeviceDescriptor> PairDeviceRouter::GetToneRenderDevice(StreamUsage streamUsage, int32_t clientUID)
{
    return make_shared<AudioDeviceDescriptor>();
}

} // namespace AudioStandard
} // namespace OHOS
