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
#define LOG_TAG "StreamFilterRouter"
#endif

#include "stream_filter_router.h"

#include "audio_policy_service.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

shared_ptr<AudioDeviceDescriptor> StreamFilterRouter::GetMediaRenderDevice(StreamUsage streamUsage,
    int32_t clientUID)
{
    DistributedRoutingInfo routingInfo = AudioPolicyService::GetAudioPolicyService().GetDistributedRoutingRoleInfo();
    if (routingInfo.descriptor != nullptr) {
        std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = routingInfo.descriptor;
        CastType type = routingInfo.type;
        bool hasDescriptor = false;
        AUDIO_INFO_LOG("streamUsage %{public}d clientUid %{public}d",
            streamUsage, clientUID);

        switch (type) {
            case CAST_TYPE_NULL: {
                break;
            }
            case CAST_TYPE_ALL: {
                vector<shared_ptr<AudioDeviceDescriptor>> descriptors =
                    AudioDeviceManager::GetAudioDeviceManager().GetRemoteRenderDevices();
                hasDescriptor = IsIncomingDeviceInRemoteDevice(descriptors, deviceDescriptor);
                break;
            }
            case CAST_TYPE_PROJECTION: {
                if (streamUsage == STREAM_USAGE_MUSIC) {
                    vector<shared_ptr<AudioDeviceDescriptor>> descriptors =
                        AudioDeviceManager::GetAudioDeviceManager().GetRemoteRenderDevices();
                    hasDescriptor = IsIncomingDeviceInRemoteDevice(descriptors, deviceDescriptor);
                }
                break;
            }
            case CAST_TYPE_COOPERATION: {
                break;
            }
            default: {
                AUDIO_ERR_LOG("unhandled cast type: %{public}d", type);
                break;
            }
        }
        if (hasDescriptor) {
            AUDIO_INFO_LOG("Incoming device is in remote devices");
            shared_ptr<AudioDeviceDescriptor> incomingDevice = make_shared<AudioDeviceDescriptor>(deviceDescriptor);
            return incomingDevice;
        }
    }
    return make_shared<AudioDeviceDescriptor>();
}

shared_ptr<AudioDeviceDescriptor> StreamFilterRouter::GetCallRenderDevice(StreamUsage streamUsage,
    int32_t clientUID)
{
    DistributedRoutingInfo routingInfo = AudioPolicyService::GetAudioPolicyService().GetDistributedRoutingRoleInfo();
    if (routingInfo.descriptor != nullptr) {
        std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = routingInfo.descriptor;
        CastType type = routingInfo.type;
        bool hasDescriptor = false;
        AUDIO_INFO_LOG("streamUsage %{public}d clientUid %{public}d",
            streamUsage, clientUID);

        switch (type) {
            case CAST_TYPE_NULL: {
                break;
            }
            case CAST_TYPE_ALL: {
                vector<shared_ptr<AudioDeviceDescriptor>> descriptors =
                    AudioDeviceManager::GetAudioDeviceManager().GetRemoteRenderDevices();
                hasDescriptor = IsIncomingDeviceInRemoteDevice(descriptors, deviceDescriptor);
                break;
            }
            case CAST_TYPE_PROJECTION: {
                break;
            }
            case CAST_TYPE_COOPERATION: {
                break;
            }
            default: {
                AUDIO_ERR_LOG("unhandled cast type: %{public}d", type);
                break;
            }
        }
        if (hasDescriptor) {
            AUDIO_INFO_LOG("Incoming device is in remote devices");
            shared_ptr<AudioDeviceDescriptor> incomingDevice = make_shared<AudioDeviceDescriptor>(deviceDescriptor);
            return incomingDevice;
        }
    }
    return make_shared<AudioDeviceDescriptor>();
}

shared_ptr<AudioDeviceDescriptor> StreamFilterRouter::GetCallCaptureDevice(SourceType sourceType,
    int32_t clientUID, const uint32_t sessionID)
{
    DistributedRoutingInfo routingInfo = AudioPolicyService::GetAudioPolicyService().GetDistributedRoutingRoleInfo();
    if (routingInfo.descriptor != nullptr) {
        std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = routingInfo.descriptor;
        CastType type = routingInfo.type;
        bool hasDescriptor = false;
        AUDIO_INFO_LOG("sourceType %{public}d clientUid %{public}d",
            sourceType, clientUID);
        switch (type) {
            case CAST_TYPE_NULL: {
                break;
            }
            case CAST_TYPE_ALL: {
                vector<shared_ptr<AudioDeviceDescriptor>> descriptors =
                    AudioDeviceManager::GetAudioDeviceManager().GetRemoteCaptureDevices();
                hasDescriptor = IsIncomingDeviceInRemoteDevice(descriptors, deviceDescriptor);
                break;
            }
            case CAST_TYPE_PROJECTION: {
                break;
            }
            case CAST_TYPE_COOPERATION: {
                break;
            }
            default: {
                AUDIO_ERR_LOG("unhandled cast type: %{public}d", type);
                break;
            }
        }
        if (hasDescriptor) {
            AUDIO_INFO_LOG("Incoming device is in remote devices");
            shared_ptr<AudioDeviceDescriptor> incomingDevice = make_shared<AudioDeviceDescriptor>(deviceDescriptor);
            return incomingDevice;
        }
    }
    return make_shared<AudioDeviceDescriptor>();
}

vector<std::shared_ptr<AudioDeviceDescriptor>> StreamFilterRouter::GetRingRenderDevices(StreamUsage streamUsage,
    int32_t clientUID)
{
    vector<shared_ptr<AudioDeviceDescriptor>> descs;
    return descs;
}

shared_ptr<AudioDeviceDescriptor> StreamFilterRouter::GetRecordCaptureDevice(SourceType sourceType,
    int32_t clientUID, const uint32_t sessionID)
{
    DistributedRoutingInfo routingInfo = AudioPolicyService::GetAudioPolicyService().GetDistributedRoutingRoleInfo();
    if (routingInfo.descriptor != nullptr) {
        std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = routingInfo.descriptor;
        CastType type = routingInfo.type;
        bool hasDescriptor = false;
        shared_ptr<AudioDeviceDescriptor> captureDevice;
        AUDIO_INFO_LOG("sourceType %{public}d clientUid %{public}d",
            sourceType, clientUID);
        switch (type) {
            case CAST_TYPE_NULL: {
                break;
            }
            case CAST_TYPE_ALL: {
                vector<shared_ptr<AudioDeviceDescriptor>> descriptors =
                    AudioDeviceManager::GetAudioDeviceManager().GetRemoteCaptureDevices();
                captureDevice = SelectRemoteCaptureDevice(descriptors, deviceDescriptor, hasDescriptor);
                break;
            }
            case CAST_TYPE_PROJECTION: {
                if (sourceType == SOURCE_TYPE_MIC) {
                    vector<shared_ptr<AudioDeviceDescriptor>> descriptors =
                        AudioDeviceManager::GetAudioDeviceManager().GetRemoteCaptureDevices();
                    captureDevice = SelectRemoteCaptureDevice(descriptors, deviceDescriptor, hasDescriptor);
                }
                break;
            }
            case CAST_TYPE_COOPERATION: {
                break;
            }
            default: {
                AUDIO_ERR_LOG("unhandled cast type: %{public}d", type);
                break;
            }
        }
        if (hasDescriptor) {
            AUDIO_INFO_LOG("Incoming device is in remote devices");
            return captureDevice;
        }
    }
    return make_shared<AudioDeviceDescriptor>();
}

shared_ptr<AudioDeviceDescriptor> StreamFilterRouter::GetToneRenderDevice(StreamUsage streamUsage,
    int32_t clientUID)
{
    return make_shared<AudioDeviceDescriptor>();
}

bool StreamFilterRouter::IsIncomingDeviceInRemoteDevice(vector<shared_ptr<AudioDeviceDescriptor>> &descriptors,
    std::shared_ptr<AudioDeviceDescriptor> incomingDevice)
{
    for (const auto &desc : descriptors) {
        if (desc != nullptr) {
            if (desc->deviceRole_ == incomingDevice->deviceRole_ &&
                desc->deviceType_ == incomingDevice->deviceType_ &&
                desc->interruptGroupId_ == incomingDevice->interruptGroupId_ &&
                desc->volumeGroupId_ == incomingDevice->volumeGroupId_ &&
                desc->networkId_ == incomingDevice->networkId_ &&
                desc->macAddress_ == incomingDevice->macAddress_) {
                return true;
            }
        }
    }
    return false;
}

std::shared_ptr<AudioDeviceDescriptor> StreamFilterRouter::SelectRemoteCaptureDevice(
    vector<shared_ptr<AudioDeviceDescriptor>> &descriptors, std::shared_ptr<AudioDeviceDescriptor> incomingDevice,
    bool &hasDescriptor)
{
    for (auto &descriptor : descriptors) {
        if (descriptor != nullptr &&
            descriptor->networkId_ == incomingDevice->networkId_ &&
            descriptor->deviceRole_ == INPUT_DEVICE &&
            descriptor->deviceType_ == DEVICE_TYPE_MIC) {
            hasDescriptor = true;
            return make_shared<AudioDeviceDescriptor>(*descriptor);
        }
    }
    return make_shared<AudioDeviceDescriptor>();
}
} // namespace AudioStandard
} // namespace OHOS
