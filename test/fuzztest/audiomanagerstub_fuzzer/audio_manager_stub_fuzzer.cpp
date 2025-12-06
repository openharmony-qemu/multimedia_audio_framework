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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include "audio_info.h"
#include "audio_policy_server.h"
#include "audio_policy_service.h"
#include "audio_device_info.h"
#include "audio_utils.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
#include "audio_channel_blend.h"
#include "volume_ramp.h"
#include "audio_speed.h"

#include "audio_policy_utils.h"
#include "audio_stream_descriptor.h"
#include "audio_limiter_manager.h"
#include "dfx_msg_manager.h"

#include "audio_source_clock.h"
#include "capturer_clock_manager.h"
#include "hpae_policy_manager.h"
#include "audio_policy_state_monitor.h"
#include "audio_device_info.h"
#include "audio_server.h"
#include "pulseaudio_ipc_interface_code.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
bool g_hasPermission = false;
const int32_t SYSTEM_ABILITY_ID = 3001;
const bool RUN_ON_CREATE = false;

typedef void (*TestFuncs)();

template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (g_dataSize < g_pos) {
        return object;
    }
    if (RAW_DATA == nullptr || objectSize > g_dataSize - g_pos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, RAW_DATA + g_pos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_pos += objectSize;
    return object;
}

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

vector<AudioServerInterfaceCode> AudioServerInterfaceCodeVec = {
    AudioServerInterfaceCode::GET_AUDIO_PARAMETER,
    AudioServerInterfaceCode::SET_AUDIO_PARAMETER,
    AudioServerInterfaceCode::GET_EXTRA_AUDIO_PARAMETERS,
    AudioServerInterfaceCode::SET_EXTRA_AUDIO_PARAMETERS,
    AudioServerInterfaceCode::SET_MICROPHONE_MUTE,
    AudioServerInterfaceCode::SET_AUDIO_SCENE,
    AudioServerInterfaceCode::UPDATE_ROUTE_REQ,
    AudioServerInterfaceCode::UPDATE_ROUTES_REQ,
    AudioServerInterfaceCode::UPDATE_DUAL_TONE_REQ,
    AudioServerInterfaceCode::GET_TRANSACTION_ID,
    AudioServerInterfaceCode::SET_PARAMETER_CALLBACK,
    AudioServerInterfaceCode::GET_REMOTE_AUDIO_PARAMETER,
    AudioServerInterfaceCode::SET_REMOTE_AUDIO_PARAMETER,
    AudioServerInterfaceCode::NOTIFY_DEVICE_INFO,
    AudioServerInterfaceCode::CHECK_REMOTE_DEVICE_STATE,
    AudioServerInterfaceCode::SET_VOICE_VOLUME,
    AudioServerInterfaceCode::SET_AUDIO_MONO_STATE,
    AudioServerInterfaceCode::SET_AUDIO_BALANCE_VALUE,
    AudioServerInterfaceCode::CREATE_AUDIOPROCESS,
    AudioServerInterfaceCode::LOAD_AUDIO_EFFECT_LIBRARIES,
    AudioServerInterfaceCode::CREATE_AUDIO_EFFECT_CHAIN_MANAGER,
    AudioServerInterfaceCode::SET_OUTPUT_DEVICE_SINK,
    AudioServerInterfaceCode::SET_ACTIVE_OUTPUT_DEVICE,
    AudioServerInterfaceCode::CREATE_PLAYBACK_CAPTURER_MANAGER,
    AudioServerInterfaceCode::REGISET_POLICY_PROVIDER,
    AudioServerInterfaceCode::REGISET_CORE_SERVICE_PROVIDER,
    AudioServerInterfaceCode::SET_WAKEUP_CLOSE_CALLBACK,
    AudioServerInterfaceCode::UPDATE_SPATIALIZATION_STATE,
    AudioServerInterfaceCode::UPDATE_SPATIAL_DEVICE_TYPE,
    AudioServerInterfaceCode::OFFLOAD_SET_VOLUME,
    AudioServerInterfaceCode::NOTIFY_STREAM_VOLUME_CHANGED,
    AudioServerInterfaceCode::SET_SPATIALIZATION_SCENE_TYPE,
    AudioServerInterfaceCode::GET_MAX_AMPLITUDE,
    AudioServerInterfaceCode::RESET_ROUTE_FOR_DISCONNECT,
    AudioServerInterfaceCode::GET_EFFECT_LATENCY,
    AudioServerInterfaceCode::UPDATE_LATENCY_TIMESTAMP,
    AudioServerInterfaceCode::SET_ASR_AEC_MODE,
    AudioServerInterfaceCode::GET_ASR_AEC_MODE,
    AudioServerInterfaceCode::SET_ASR_NOISE_SUPPRESSION_MODE,
    AudioServerInterfaceCode::SET_OFFLOAD_MODE,
    AudioServerInterfaceCode::UNSET_OFFLOAD_MODE,
    AudioServerInterfaceCode::CHECK_HIBERNATE_STATE,
    AudioServerInterfaceCode::GET_ASR_NOISE_SUPPRESSION_MODE,
    AudioServerInterfaceCode::SET_ASR_WHISPER_DETECTION_MODE,
    AudioServerInterfaceCode::GET_ASR_WHISPER_DETECTION_MODE,
    AudioServerInterfaceCode::SET_ASR_VOICE_CONTROL_MODE,
    AudioServerInterfaceCode::SET_ASR_VOICE_MUTE_MODE,
    AudioServerInterfaceCode::IS_WHISPERING,
    AudioServerInterfaceCode::GET_EFFECT_OFFLOAD_ENABLED,
    AudioServerInterfaceCode::GET_AUDIO_EFFECT_PROPERTY_V3,
    AudioServerInterfaceCode::SET_AUDIO_EFFECT_PROPERTY_V3,
    AudioServerInterfaceCode::GET_AUDIO_ENHANCE_PROPERTY,
    AudioServerInterfaceCode::GET_AUDIO_EFFECT_PROPERTY,
    AudioServerInterfaceCode::SET_AUDIO_ENHANCE_PROPERTY,
    AudioServerInterfaceCode::SET_AUDIO_EFFECT_PROPERTY,
    AudioServerInterfaceCode::SUSPEND_RENDERSINK,
    AudioServerInterfaceCode::RESTORE_RENDERSINK,
    AudioServerInterfaceCode::LOAD_HDI_EFFECT_MODEL,
    AudioServerInterfaceCode::UPDATE_EFFECT_BT_OFFLOAD_SUPPORTED,
    AudioServerInterfaceCode::SET_SINK_MUTE_FOR_SWITCH_DEVICE,
    AudioServerInterfaceCode::SET_ROTATION_TO_EFFECT,
    AudioServerInterfaceCode::UPDATE_SESSION_CONNECTION_STATE,
    AudioServerInterfaceCode::SET_SINGLE_STREAM_MUTE,
    AudioServerInterfaceCode::RESTORE_SESSION,
    AudioServerInterfaceCode::CREATE_IPC_OFFLINE_STREAM,
    AudioServerInterfaceCode::GET_OFFLINE_AUDIO_EFFECT_CHAINS,
    AudioServerInterfaceCode::GET_STANDBY_STATUS,
    AudioServerInterfaceCode::GENERATE_SESSION_ID,
    AudioServerInterfaceCode::GET_ALL_SINK_INPUTS,
    AudioServerInterfaceCode::SET_DEFAULT_ADAPTER_ENABLE,
    AudioServerInterfaceCode::NOTIFY_ACCOUNTS_CHANGED,
    AudioServerInterfaceCode::NOTIFY_AUDIO_POLICY_READY,
    AudioServerInterfaceCode::SET_CAPTURE_LIMIT,
    AudioServerInterfaceCode::LOAD_HDI_ADAPTER,
    AudioServerInterfaceCode::UNLOAD_HDI_ADAPTER,
    AudioServerInterfaceCode::CHECK_CAPTURE_LIMIT,
    AudioServerInterfaceCode::RELEASE_CAPTURE_LIMIT,
    AudioServerInterfaceCode::CREATE_HDI_SINK_PORT,
    AudioServerInterfaceCode::CREATE_SINK_PORT,
    AudioServerInterfaceCode::CREATE_HDI_SOURCE_PORT,
    AudioServerInterfaceCode::CREATE_SOURCE_PORT,
    AudioServerInterfaceCode::DESTROY_HDI_PORT,
    AudioServerInterfaceCode::DEVICE_CONNECTED_FLAG,
    AudioServerInterfaceCode::SET_DM_DEVICE_TYPE,
    AudioServerInterfaceCode::REGISTER_DATATRANSFER_STATE_PARAM,
    AudioServerInterfaceCode::UNREGISTER_DATATRANSFER_STATE_PARAM,
    AudioServerInterfaceCode::REGISTER_DATATRANSFER_CALLBACK,
    AudioServerInterfaceCode::NOTIFY_SETTINGS_DATA_READY,
    AudioServerInterfaceCode::IS_ACOSTIC_ECHO_CAMCELER_SUPPORTED,
    AudioServerInterfaceCode::SET_SESSION_MUTE_STATE,
    AudioServerInterfaceCode::NOTIFY_MUTE_STATE_CHANGE,
    AudioServerInterfaceCode::FORCE_STOP_AUDIO_STREAM,
    AudioServerInterfaceCode::CREATE_AUDIOWORKGROUP,
    AudioServerInterfaceCode::RELEASE_AUDIOWORKGROUP,
    AudioServerInterfaceCode::ADD_THREAD_TO_AUDIOWORKGROUP,
    AudioServerInterfaceCode::REMOVE_THREAD_FROM_AUDIOWORKGROUP,
    AudioServerInterfaceCode::START_AUDIOWORKGROUP,
    AudioServerInterfaceCode::STOP_AUDIOWORKGROUP,
    AudioServerInterfaceCode::SET_BT_HDI_INVALID_STATE,
    AudioServerInterfaceCode::SET_KARAOKE_PARAMETERS,
    AudioServerInterfaceCode::IS_AUDIO_LOOPBACK_SUPPORTED,
    AudioServerInterfaceCode::AUDIO_SERVER_CODE_MAX,
};

void GetPermission()
{
    if (!g_hasPermission) {
        uint64_t tokenId;
        constexpr int perNum = 10;
        const char *perms[perNum] = {
            "ohos.permission.MICROPHONE",
            "ohos.permission.MANAGE_INTELLIGENT_VOICE",
            "ohos.permission.MANAGE_AUDIO_CONFIG",
            "ohos.permission.MICROPHONE_CONTROL",
            "ohos.permission.MODIFY_AUDIO_SETTINGS",
            "ohos.permission.ACCESS_NOTIFICATION_POLICY",
            "ohos.permission.USE_BLUETOOTH",
            "ohos.permission.CAPTURE_VOICE_DOWNLINK_AUDIO",
            "ohos.permission.RECORD_VOICE_CALL",
            "ohos.permission.MANAGE_SYSTEM_AUDIO_EFFECTS",
        };

        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = 10,
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .processName = "audiofuzztest",
            .aplStr = "system_basic",
        };
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
        g_hasPermission = true;
    }
}

void OnRemoteRequestFuzzTest()
{
    GetPermission();
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    uint32_t audioServerInterfaceCodeCount = GetData<uint32_t>() % AudioServerInterfaceCodeVec.size();
    AudioServerInterfaceCode audioServerInterfaceCode = AudioServerInterfaceCodeVec[audioServerInterfaceCodeCount];
    uint32_t format = static_cast<uint32_t>(audioServerInterfaceCode);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    audioServer->SetAsrNoiseSuppressionMode(0);
    audioServer->OnRemoteRequest(format, data, reply, option);
}

TestFuncs g_testFuncs[] = {
    OnRemoteRequestFuzzTest,
};

void FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return;
    }

    // initialize data
    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;

    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testFuncs);
    if (len > 0) {
        g_testFuncs[code % len]();
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }

    return;
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
