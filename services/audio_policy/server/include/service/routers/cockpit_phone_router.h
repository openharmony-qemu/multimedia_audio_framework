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

#ifndef ST_COCKPIT_PHONE_ROUTER_H
#define ST_COCKPIT_PHONE_ROUTER_H

#include "router_base.h"

namespace OHOS {
namespace AudioStandard {
class CockpitPhoneRouter : public RouterBase {
public:
    std::string name_ = "cockpit_phone_router";
    CockpitPhoneRouter() {};
    ~CockpitPhoneRouter() {};
    std::string GetClassName() override
    {
        return name_;
    }

    RouterType GetRouterType() override
    {
        return ROUTER_TYPE_COCKPIT_PHONE;
    }

    std::unique_ptr<AudioDeviceDescriptor> GetMediaRenderDevice(StreamUsage streamUsage, int32_t clientUID) override;
    std::unique_ptr<AudioDeviceDescriptor> GetCallRenderDevice(StreamUsage streamUsage, int32_t clientUID) override;
    std::unique_ptr<AudioDeviceDescriptor> GetCallCaptureDevice(SourceType sourceType, int32_t clientUID) override;
    vector<std::unique_ptr<AudioDeviceDescriptor>> GetRingRenderDevices(StreamUsage streamUsage,
        int32_t clientUID) override;
    std::unique_ptr<AudioDeviceDescriptor> GetRecordCaptureDevice(SourceType sourceType, int32_t clientUID) override;
    std::unique_ptr<AudioDeviceDescriptor> GetToneRenderDevice(StreamUsage streamUsage, int32_t clientUID) override;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_COCKPIT_PHONE_ROUTER_H