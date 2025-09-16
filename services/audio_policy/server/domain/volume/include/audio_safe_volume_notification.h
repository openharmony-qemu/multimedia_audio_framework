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

#ifndef AUDIO_SAFE_VOLUME_NOTIFICATION_H
#define AUDIO_SAFE_VOLUME_NOTIFICATION_H

#include <cstdint>
#include <string>

namespace OHOS {
namespace AudioStandard {
const uint32_t SAVE_VOLUME_SYS_ABILITY_ID = 1041;
const int32_t RESTORE_VOLUME_NOTIFICATION_ID = 116000;
const int32_t INCREASE_VOLUME_NOTIFICATION_ID = 116001;
const uint32_t NOTIFICATION_BANNER_FLAG = 1 << 9;
const uint32_t NOTIFICATION_CLOSE_SOUND_FLAG = 1 << 0;
const std::string AUDIO_RESTORE_VOLUME_EVENT = "AUDIO_RESTORE_VOLUME_EVENT";
const std::string AUDIO_INCREASE_VOLUME_EVENT = "AUDIO_INCREASE_VOLUME_EVENT";
const std::string SAVE_VOLUME_SYS_ABILITY_NAME = "audio_service";

const std::string SAFE_VOLUME_ICON_ID = "safe_volume_notification_icon";
const std::string SAFE_VOLUME_MUSIC_TIMER_TITTLE_ID = "ohos_id_notification_restore_volume_tittle";
const std::string SAFE_VOLUME_INCREASE_VOLUME_TITTLE_ID = "ohos_id_notification_increase_volume_tittle";
const std::string SAFE_VOLUME_MUSIC_TIMER_TEXT_ID = "ohos_id_notification_restore_volume_context";
const std::string SAFE_VOLUME_INCREASE_VOLUME_TEXT_ID = "ohos_id_notification_increase_volume_context";
const std::string SAFE_VOLUME_RESTORE_VOL_BUTTON_ID = "ohos_id_notification_restore_volume_button";
const std::string SAFE_VOLUME_INCREASE_VOL_BUTTON_ID = "ohos_id_notification_increase_volume_button";

const int32_t ICON_WIDTH = 220;
const int32_t ICON_HEIGHT = 220;

class AudioSafeVolumeNotification {
public:
    AudioSafeVolumeNotification() = default;
    virtual ~AudioSafeVolumeNotification() = default;

    virtual void PublishSafeVolumeNotification(int32_t notificationId) = 0;
    virtual void CancelSafeVolumeNotification(int32_t notificationId) = 0;
};

typedef AudioSafeVolumeNotification* CreateSafeVolumeNotification();

const int32_t LOUD_ICON_WIDTH = 120;
const int32_t LOUD_ICON_HEIGHT = 120;

const std::string LOUD_VOLUME_ICON_ID = "loud_volume_notification_icon";
const std::string LOUD_VOLUME_FEATURE_TITTLE_ID = "notification_feature_loud_volume_tittle";
const std::string LOUD_VOLUME_ENABLE_TITTLE_ID = "notification_enable_loud_volume_tittle";

class AudioLoudVolumeNotification {
public:
    AudioLoudVolumeNotification() = default;
    virtual ~AudioLoudVolumeNotification() = default;

    virtual void PublishLoudVolumeNotification(int32_t notificationId) = 0;
};

typedef AudioLoudVolumeNotification* CreateLoudVolumeNotification();

} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SAFE_VOLUME_NOTIFICATION_H
