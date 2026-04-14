/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef SAFE_VOLUME_MANAGER_H
#define SAFE_VOLUME_MANAGER_H

#include <cstdint>
#include "audio_device_info.h"

namespace OHOS {
namespace AudioStandard {

class AudioActiveDevice;
class IAudioPolicyInterface;

class SafeVolumeManager {
public:
    SafeVolumeManager();
    ~SafeVolumeManager() = default;

    void PublishSafeVolumeNotification(int32_t notificationId);
    void CancelSafeVolumeNotification(int32_t notificationId);
    void PublishLegacyHighVolumeNotification();
    void CancelLegacyHighVolumeNotification();
    bool DeviceIsSupportSafeVolume();
    bool IsWiredHeadSet(const DeviceType &deviceType);
    bool IsBlueTooth(const DeviceType &deviceType);
    bool IsNearLink(const DeviceType &deviceType);

private:
    enum class LegacyHighVolumeAction { PUBLISH, CANCEL };
    void HandleLegacyHighVolumeNotification(LegacyHighVolumeAction action);
    static constexpr const char* LIB_NAME = "libaudio_safe_volume_notification_impl.z.so";
    AudioActiveDevice& audioActiveDevice_;
    IAudioPolicyInterface& audioPolicyManager_;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // SAFE_VOLUME_MANAGER_H