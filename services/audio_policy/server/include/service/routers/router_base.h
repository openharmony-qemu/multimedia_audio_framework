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

#ifndef ST_ROUTER_BASE_H
#define ST_ROUTER_BASE_H

#include "audio_system_manager.h"
#include "audio_device_manager.h"
#include "audio_policy_manager_factory.h"
#include "audio_policy_log.h"

namespace OHOS {
namespace AudioStandard {
class RouterBase {
public:
    std::string name_;
    IAudioPolicyInterface& audioPolicyManager_;
    RouterBase() : audioPolicyManager_(AudioPolicyManagerFactory::GetAudioPolicyManager()) {}
    virtual ~RouterBase() {};

    virtual std::unique_ptr<AudioDeviceDescriptor> GetMediaRenderDevice(StreamUsage streamUsage, int32_t clientUID) = 0;
    virtual std::unique_ptr<AudioDeviceDescriptor> GetCallRenderDevice(StreamUsage streamUsage, int32_t clientUID) = 0;
    virtual std::unique_ptr<AudioDeviceDescriptor> GetCallCaptureDevice(SourceType sourceType, int32_t clientUID) = 0;
    virtual vector<std::unique_ptr<AudioDeviceDescriptor>> GetRingRenderDevices(StreamUsage streamUsage,
        int32_t clientUID) = 0;
    virtual std::unique_ptr<AudioDeviceDescriptor> GetRecordCaptureDevice(SourceType sourceType, int32_t clientUID) = 0;
    virtual std::unique_ptr<AudioDeviceDescriptor> GetToneRenderDevice(StreamUsage streamUsage, int32_t clientUID) = 0;
    virtual RouterType GetRouterType() = 0;

    virtual std::string GetClassName()
    {
        return name_;
    }
    std::unique_ptr<AudioDeviceDescriptor> GetLatestConnectDeivce(
        std::vector<std::unique_ptr<AudioDeviceDescriptor>> &descs)
    {
        // remove abnormal device
        for (size_t i = 0; i < descs.size(); i++) {
            if (descs[i]->exceptionFlag_ || !descs[i]->isEnable_ ||
                (descs[i]->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO && descs[i]->connectState_ == SUSPEND_CONNECTED)) {
                descs.erase(descs.begin() + i);
                i--;
            }
        }
        if (descs.size() > 0) {
            auto compare = [&] (std::unique_ptr<AudioDeviceDescriptor> &desc1,
                std::unique_ptr<AudioDeviceDescriptor> &desc2) {
                return desc1->connectTimeStamp_ < desc2->connectTimeStamp_;
            };
            sort(descs.begin(), descs.end(), compare);
            return std::move(descs.back());
        }
        return std::make_unique<AudioDeviceDescriptor>();
    }

    std::unique_ptr<AudioDeviceDescriptor> GetPairCaptureDevice(std::unique_ptr<AudioDeviceDescriptor> &desc,
        std::vector<std::unique_ptr<AudioDeviceDescriptor>> &captureDescs)
    {
        for (auto &captureDesc : captureDescs) {
            if (captureDesc->deviceRole_ != desc->deviceRole_
                || captureDesc->deviceType_ != desc->deviceType_
                || captureDesc->networkId_ != desc->networkId_
                || captureDesc->macAddress_ != desc->macAddress_) {
                continue;
            }
            if (!captureDesc->exceptionFlag_ && captureDesc->isEnable_ &&
                (captureDesc->deviceType_ != DEVICE_TYPE_BLUETOOTH_SCO ||
                 captureDesc->connectState_ != SUSPEND_CONNECTED)) {
                return std::move(captureDesc);
            }
            AUDIO_INFO_LOG("unavailable device state, type[%{public}d] connectState[%{public}d] isEnable[%{public}d]" \
                "exceptionFlag[%{public}d]", captureDesc->deviceType_, captureDesc->connectState_,
                captureDesc->isEnable_, captureDesc->exceptionFlag_);
        }
        return std::make_unique<AudioDeviceDescriptor>();
    }
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_ROUTER_BASE_H