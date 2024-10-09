/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef I_AUDIO_SERVER_INTERFACE_H
#define I_AUDIO_SERVER_INTERFACE_H

#include "audio_info.h"

/* SAID: 3001 */
namespace OHOS {
namespace AudioStandard {
    enum class AudioServerInterfaceCode {
        GET_AUDIO_PARAMETER,
        SET_AUDIO_PARAMETER,
        GET_EXTRA_AUDIO_PARAMETERS,
        SET_EXTRA_AUDIO_PARAMETERS,
        SET_MICROPHONE_MUTE,
        SET_AUDIO_SCENE,
        UPDATE_ROUTE_REQ,
        UPDATE_ROUTES_REQ,
        UPDATE_DUAL_TONE_REQ,
        GET_TRANSACTION_ID,
        SET_PARAMETER_CALLBACK,
        GET_REMOTE_AUDIO_PARAMETER,
        SET_REMOTE_AUDIO_PARAMETER,
        NOTIFY_DEVICE_INFO,
        CHECK_REMOTE_DEVICE_STATE,
        SET_VOICE_VOLUME,
        SET_AUDIO_MONO_STATE,
        SET_AUDIO_BALANCE_VALUE,
        CREATE_AUDIOPROCESS,
        LOAD_AUDIO_EFFECT_LIBRARIES,
        REQUEST_THREAD_PRIORITY,
        CREATE_AUDIO_EFFECT_CHAIN_MANAGER,
        SET_OUTPUT_DEVICE_SINK,
        CREATE_PLAYBACK_CAPTURER_MANAGER,
        SET_SUPPORT_STREAM_USAGE,
        REGISET_POLICY_PROVIDER,
        SET_WAKEUP_CLOSE_CALLBACK,
        SET_CAPTURE_SILENT_STATE,
        UPDATE_SPATIALIZATION_STATE,
        UPDATE_SPATIAL_DEVICE_TYPE,
        OFFLOAD_SET_VOLUME,
        NOTIFY_STREAM_VOLUME_CHANGED,
        SET_SPATIALIZATION_SCENE_TYPE,
        GET_MAX_AMPLITUDE,
        RESET_AUDIO_ENDPOINT,
        RESET_ROUTE_FOR_DISCONNECT,
        GET_EFFECT_LATENCY,
        UPDATE_LATENCY_TIMESTAMP,
        SET_ASR_AEC_MODE,
        GET_ASR_AEC_MODE,
        SET_ASR_NOISE_SUPPRESSION_MODE,
        GET_ASR_NOISE_SUPPRESSION_MODE,
        SET_ASR_WHISPER_DETECTION_MODE,
        GET_ASR_WHISPER_DETECTION_MODE,
        SET_ASR_VOICE_CONTROL_MODE,
        SET_ASR_VOICE_MUTE_MODE,
        IS_WHISPERING,
        GET_EFFECT_OFFLOAD_ENABLED,
        SUSPEND_RENDERSINK,
        RESTORE_RENDERSINK,
        LOAD_HDI_EFFECT_MODEL,
        UPDATE_EFFECT_BT_OFFLOAD_SUPPORTED,
        SET_SINK_MUTE_FOR_SWITCH_DEVICE,
        SET_ROTATION_TO_EFFECT,
        UPDATE_SESSION_CONNECTION_STATE,
        SET_SINGLE_STREAM_MUTE,
        AUDIO_SERVER_CODE_MAX = SET_SINGLE_STREAM_MUTE,
    };
} // namespace AudioStandard
} // namespace OHOS

#endif // I_AUDIO_SERVER_INTERFACE_H
