/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef HPAE_REMOTE_DEVICE_CLASS_H
#define HPAE_REMOTE_DEVICE_CLASS_H

#include <string>

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
// "remote"(stereo) and "remote_multichannel"(HiCar MCH) share HPAE remote handling.
// remote_multichannel is the deviceClass of distributed HiCar MCH sinks; its deviceName is
// "<networkId>_out_multichannel", so it does NOT overlap with HpaeRendererManager::IsMchDevice()
// (which matches the *local* multichannel path by deviceName "MCH_Speaker"/"DP_MCH_speaker").
// This check is the only way distributed HiCar MCH enters the remote path
// (EFFECT_NONE scene / 20ms remote timing / appsUid tracking).
// Direct comparison is used (only two entries) to avoid unordered_set hashing cost and
// non-trivial static initialization on the DoProcess (20ms) hot path.
inline bool IsRemoteDeviceClass(const std::string &deviceClass)
{
    return deviceClass == "remote" || deviceClass == "remote_multichannel";
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS

#endif  // HPAE_REMOTE_DEVICE_CLASS_H
