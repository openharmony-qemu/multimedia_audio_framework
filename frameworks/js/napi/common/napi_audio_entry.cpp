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
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "audio_info.h"
#include "audio_stream_info.h"
#include "audio_device_info.h"
#include "napi_audio_enum.h"
#include "napi_audio_renderer.h"
#include "napi_toneplayer.h"
#include "napi_audio_capturer.h"
#include "napi_audio_manager.h"
#include "napi_audio_routing_manager.h"
#include "napi_audio_stream_manager.h"
#include "napi_audio_effect_manager.h"
#include "napi_audio_volume_group_manager.h"
#include "napi_audio_volume_manager.h"
#include "napi_audio_interrupt_manager.h"
#include "napi_audio_spatialization_manager.h"
#include "napi_audio_collaborative_manager.h"
#include "napi_audio_device_enhance_manager.h"
#include "napi_asr_processing_controller.h"
#include "napi_audio_session_manager.h"
#include "napi_audio_loopback.h"
#include "napi_audio_record_controller.h"

namespace OHOS {
namespace AudioStandard {
static napi_status RegisterPublicAudioEnums(napi_env env, napi_value exports)
{
    struct Member { const char *name; int32_t value; };
    static const Member select[] = {
        {"SELECT_STRATEGY_DEFAULT", SELECT_STRATEGY_DEFAULT},
        {"SELECT_STRATEGY_INDEPENDENT", SELECT_STRATEGY_INDEPENDENT},
    };
    static const Member spatial[] = {
        {"SPATIAL_AUDIO_SOURCE_TYPE_STEREO", SPATIAL_AUDIO_SOURCE_TYPE_STEREO},
        {"SPATIAL_AUDIO_SOURCE_TYPE_AUDIO_VIVID", SPATIAL_AUDIO_SOURCE_TYPE_AUDIO_VIVID},
        {"SPATIAL_AUDIO_SOURCE_TYPE_MULTI_CHANNEL", SPATIAL_AUDIO_SOURCE_TYPE_MULTI_CHANNEL},
    };
    static const Member volume[] = {{"FLAG_SHOW_SYSTEM_UI", FLAG_SHOW_SYSTEM_UI}};
    struct Enum { const char *name; const Member *members; size_t count; };
    static const Enum enums[] = {
        {"AudioDevcieSelectStrategy", select, sizeof(select) / sizeof(select[0])},
        {"SpatialAudioSourceType", spatial, sizeof(spatial) / sizeof(spatial[0])},
        {"VolumeFlag", volume, sizeof(volume) / sizeof(volume[0])},
    };
    for (const auto &entry : enums) {
        napi_value object = nullptr;
        napi_status status = napi_create_object(env, &object);
        if (status != napi_ok) {
            return status;
        }
        for (size_t index = 0; index < entry.count; ++index) {
            napi_value value = nullptr;
            status = napi_create_int32(env, entry.members[index].value, &value);
            if (status != napi_ok) {
                return status;
            }
            status = napi_set_named_property(env, object, entry.members[index].name, value);
            if (status != napi_ok) {
                return status;
            }
        }
        status = napi_set_named_property(env, exports, entry.name, object);
        if (status != napi_ok) {
            return status;
        }
    }
    return napi_ok;
}

static napi_value Init(napi_env env, napi_value exports)
{
    if (RegisterPublicAudioEnums(env, exports) != napi_ok) {
        return nullptr;
    }
    /* 各模块新增Init接口添加 */
    NapiAudioEnum::Init(env, exports);
    NapiAudioRenderer::Init(env, exports);
    NapiAudioCapturer::Init(env, exports);
#ifdef FEATURE_DTMF_TONE
    NapiTonePlayer::Init(env, exports);
#endif
    NapiAudioStreamMgr::Init(env, exports);
    NapiAudioEffectMgr::Init(env, exports);
    NapiAudioRoutingManager::Init(env, exports);
    NapiAudioVolumeGroupManager::Init(env, exports);
    NapiAudioVolumeManager::Init(env, exports);
    NapiAudioInterruptManager::Init(env, exports);
    NapiAudioSpatializationManager::Init(env, exports);
    NapiAudioManager::Init(env, exports);
    NapiAsrProcessingController::Init(env, exports);
    NapiAudioSessionMgr::Init(env, exports);
    NapiAudioCollaborativeManager::Init(env, exports);
    NapiAudioDeviceEnhanceManager::Init(env, exports);
    NapiAudioLoopback::Init(env, exports);
    NapiAudioRecordController::Init(env, exports);
    return exports;
}

static napi_module g_module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "multimedia.audio",
    .nm_priv = nullptr,
    .reserved = {0}
};

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&g_module);
}
} // namespace AudioStandard
} // namespace OHOS
