/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "AppSelectRouter"
#endif

#include "app_select_router.h"

#include "audio_usr_select_manager.h"
#include "audio_policy_service.h"
#include "audio_bundle_manager.h"
#include "audio_router_select_strategy.h"
#include "hisysevent.h"
#include "media_monitor_manager.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
shared_ptr<AudioDeviceDescriptor> AppSelectRouter::GetMediaRenderDevice(StreamUsage streamUsage, int32_t clientUID,
    const uint32_t sessionID)
{
    return make_shared<AudioDeviceDescriptor>();
}

shared_ptr<AudioDeviceDescriptor> AppSelectRouter::GetCallRenderDevice(StreamUsage streamUsage, int32_t clientUID,
    const uint32_t sessionID)
{
    return make_shared<AudioDeviceDescriptor>();
}

shared_ptr<AudioDeviceDescriptor> AppSelectRouter::GetCallCaptureDevice(SourceType sourceType, int32_t clientUID,
    const uint32_t sessionID)
{
    return make_shared<AudioDeviceDescriptor>();
}

vector<std::shared_ptr<AudioDeviceDescriptor>> AppSelectRouter::GetRingRenderDevices(StreamUsage streamUsage,
    int32_t clientUID)
{
    vector<shared_ptr<AudioDeviceDescriptor>> descs;
    return descs;
}

shared_ptr<AudioDeviceDescriptor> AppSelectRouter::GetRecordCaptureDevice(SourceType sourceType, int32_t clientUID,
    const uint32_t sessionID)
{
    vector<shared_ptr<AudioDeviceDescriptor>> allDevices =
        AudioDeviceManager::GetAudioDeviceManager().GetConnectedDevices();
    RouterBase::ClearConflictDevice(allDevices, GetRouterType());
    auto device = AudioRouterSelectStrategy::GetInstance().GetMediaInputDevice(clientUID, sessionID, sourceType);
    CHECK_AND_RETURN_RET(device == nullptr || device->deviceType_ == DEVICE_TYPE_NONE,
        RouterBase::GetPairDevice(device, allDevices, MEDIA_INPUT_DEVICES));
    return make_shared<AudioDeviceDescriptor>();
}

shared_ptr<AudioDeviceDescriptor> AppSelectRouter::GetToneRenderDevice(StreamUsage streamUsage, int32_t clientUID)
{
    return make_shared<AudioDeviceDescriptor>();
}

} // namespace AudioStandard
} // namespace OHOS
