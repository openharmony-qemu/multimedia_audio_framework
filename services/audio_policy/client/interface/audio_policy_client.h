/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef ST_AUDIO_POLICY_CLIENT_H
#define ST_AUDIO_POLICY_CLIENT_H

#include <cstdint>

namespace OHOS {
namespace AudioStandard {
static const int32_t UPDATE_CALLBACK_CLIENT = 0;
static const int32_t API_VERSION_MAX = 1000;

enum class AudioPolicyClientCode {
    ON_VOLUME_KEY_EVENT = 0,
    ON_FOCUS_INFO_CHANGED,
    ON_FOCUS_REQUEST_CHANGED,
    ON_FOCUS_ABANDON_CHANGED,
    ON_DEVICE_CHANGE,
    ON_APP_VOLUME_CHANGE,
    ON_ACTIVE_VOLUME_TYPE_CHANGE,
    ON_RINGERMODE_UPDATE,
    ON_MIC_STATE_UPDATED,
    ON_ACTIVE_OUTPUT_DEVICE_UPDATED,
    ON_ACTIVE_INPUT_DEVICE_UPDATED,
    ON_RENDERERSTATE_CHANGE,
    ON_CAPTURERSTATE_CHANGE,
    ON_RENDERER_DEVICE_CHANGE,
    ON_RECREATE_RENDERER_STREAM_EVENT,
    ON_RECREATE_CAPTURER_STREAM_EVENT,
    ON_HEAD_TRACKING_DEVICE_CHANGE,
    ON_SPATIALIZATION_ENABLED_CHANGE,
    ON_SPATIALIZATION_ENABLED_CHANGE_FOR_ANY_DEVICE,
    ON_HEAD_TRACKING_ENABLED_CHANGE,
    ON_HEAD_TRACKING_ENABLED_CHANGE_FOR_ANY_DEVICE,
    ON_NN_STATE_CHANGE,
    ON_AUDIO_SESSION_DEACTIVE,
    ON_MICRO_PHONE_BLOCKED,
    ON_AUDIO_SCENE_CHANGED,
    ON_SPATIALIZATION_ENABLED_CHANGE_FOR_CURRENT_DEVICE,
    ON_FORMAT_UNSUPPORTED_ERROR,
    ON_STREAM_VOLUME_CHANGE,
    ON_SYSTEM_VOLUME_CHANGE,
    ON_AUDIO_SESSION_STATE_CHANGED,
    ON_AUDIO_SESSION_CURRENT_DEVICE_CHANGED,
    AUDIO_POLICY_CLIENT_CODE_MAX = ON_AUDIO_SESSION_CURRENT_DEVICE_CHANGED,
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_POLICY_CLIENT_H
