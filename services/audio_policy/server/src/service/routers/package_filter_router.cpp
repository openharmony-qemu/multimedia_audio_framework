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

#include "package_filter_router.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

unique_ptr<AudioDeviceDescriptor> PackageFilterRouter::GetMediaRenderDevice(StreamUsage streamUsage, int32_t clientUID)
{
    return make_unique<AudioDeviceDescriptor>();
}

unique_ptr<AudioDeviceDescriptor> PackageFilterRouter::GetCallRenderDevice(StreamUsage streamUsage, int32_t clientUID)
{
    return make_unique<AudioDeviceDescriptor>();
}

unique_ptr<AudioDeviceDescriptor> PackageFilterRouter::GetCallCaptureDevice(SourceType sourceType, int32_t clientUID)
{
    return make_unique<AudioDeviceDescriptor>();
}

vector<std::unique_ptr<AudioDeviceDescriptor>> PackageFilterRouter::GetRingRenderDevices(StreamUsage streamUsage,
    int32_t clientUID)
{
    vector<unique_ptr<AudioDeviceDescriptor>> descs;
    return descs;
}

unique_ptr<AudioDeviceDescriptor> PackageFilterRouter::GetRecordCaptureDevice(SourceType sourceType, int32_t clientUID)
{
    return make_unique<AudioDeviceDescriptor>();
}

unique_ptr<AudioDeviceDescriptor> PackageFilterRouter::GetToneRenderDevice(StreamUsage streamUsage, int32_t clientUID)
{
    return make_unique<AudioDeviceDescriptor>();
}

} // namespace AudioStandard
} // namespace OHOS