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

#ifndef BOOT_ANIMATION_STATE_INFO_H
#define BOOT_ANIMATION_STATE_INFO_H

#include <vector>
#include <cstdint>
#include <parcel.h>
#include "audio_log.h"

namespace OHOS {
namespace AudioStandard {

constexpr int32_t BOOT_ANIMATION_UID = 1003;

enum class AudioServerBootState : int32_t {
    BEGIN,
    LOAD_LIBXML2,
    PUBLISH_AVAILABLE,
    INIT_PREEAUDIO,
    LOAD_AUDIO_PARAM_CONFIG_XML,
    STAY_RESIDENT,
    END,
    MAX,
};

using AudioServerBootInfo = std::vector<int64_t>;

enum class AudioPolicyServerBootState : int32_t {
    BEGIN,
    LOAD_LIBXML2,
    LOAD_AUDIO_INTERRUPT_POLICY_CONFIG,
    LOAD_AUDIO_VOLUME_CONFIG,
    LOAD_AUDIO_POLICY_CONFIG,
    LOAD_AUDIO_EFFECT_CONFIG,
    LOAD_AUDIO_POLICY_GLOBAL_CONFIG,
    LOAD_AUDIO_TONE_DTMF_CONFIG,
    LISTENING_DEVICE_STATUS,
    PUBLISH_AVAILABLE,
    REGISTER_ACCESS_TOKEN_CB,
    REGISTER_VOLUME_KEY_EVENT,
    INIT_VOLUME_DATABASE,
    INIT_DATABASE,
    GET_SCREEN_STATUS,
    END,
    MAX,
};

using AudioPolicyServerBootInfo = std::vector<int64_t>;

enum class BootAnimationState : int32_t {
    CREATE_RENDERER_BEGIN,
    CHECK_POLICY_SERVICE_AVAILABLE,
    VERIFY_PERMISSION_IN_CREATE,
    WAIT_FOR_SERVICE_READY,
    GET_BUNDLE_NAME,
    PREPARE_AUDIO_STREAM,
    INIT_AUDIO_STREAM,
    CREATE_FROM_LOCAL,
    SET_AUDIO_STREAM_INFO,
    CREATE_RENDERER_END,
    START_RENDERER_BEGIN,
    FETCH_OUTPUT_DEVICES,
    VERIFY_PERMISSION_IN_START,
    ACTIVATE_AUDIO_INTERRUPT,
    SET_VOLUME_DB,
    PREE_AUDIO_START,
    WAIT_FOR_DATA_CONNECTION,
    START_AUDIO_STREAM,
    START_RENDERER_END,
    MAX,
};

using BootAnimationInfo = std::vector<int64_t>;

} // namespace AudioStandard
} // namespace OHOS

#endif // BOOT_ANIMATION_STATE_INFO_H