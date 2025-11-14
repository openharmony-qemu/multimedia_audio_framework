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
#include "audio_server.h"
#include "../fuzz_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;
const int32_t SYSTEM_ABILITY_ID = 3001;
const bool RUN_ON_CREATE = false;

typedef void (*TestFuncs)();

vector<IStandardAudioServiceIpcCode> IStandardAudioServiceIpcCodeVec = {
    IStandardAudioServiceIpcCode::COMMAND_GET_AUDIO_PARAMETER,
    IStandardAudioServiceIpcCode::COMMAND_SET_AUDIO_PARAMETER,
    IStandardAudioServiceIpcCode::COMMAND_GET_EXTRA_PARAMETERS,
    IStandardAudioServiceIpcCode::COMMAND_SET_EXTRA_PARAMETERS,
    IStandardAudioServiceIpcCode::COMMAND_SET_MICROPHONE_MUTE,
    IStandardAudioServiceIpcCode::COMMAND_SET_AUDIO_SCENE,
    IStandardAudioServiceIpcCode::COMMAND_UPDATE_ACTIVE_DEVICE_ROUTE,
    IStandardAudioServiceIpcCode::COMMAND_UPDATE_ACTIVE_DEVICES_ROUTE,
    IStandardAudioServiceIpcCode::COMMAND_UPDATE_DUAL_TONE_STATE,
    IStandardAudioServiceIpcCode::COMMAND_GET_TRANSACTION_ID,
    IStandardAudioServiceIpcCode::COMMAND_SET_PARAMETER_CALLBACK,
    IStandardAudioServiceIpcCode::COMMAND_GET_AUDIO_PARAMETER_IN_STRING_IN_INT_IN_STRING_OUT_STRING,
    IStandardAudioServiceIpcCode::COMMAND_SET_AUDIO_PARAMETER_IN_STRING_IN_INT_IN_STRING_IN_STRING,
    IStandardAudioServiceIpcCode::COMMAND_NOTIFY_DEVICE_INFO,
    IStandardAudioServiceIpcCode::COMMAND_CHECK_REMOTE_DEVICE_STATE,
    IStandardAudioServiceIpcCode::COMMAND_SET_VOICE_VOLUME,
    IStandardAudioServiceIpcCode::COMMAND_SET_AUDIO_MONO_STATE,
    IStandardAudioServiceIpcCode::COMMAND_SET_AUDIO_BALANCE_VALUE,
    IStandardAudioServiceIpcCode::COMMAND_CREATE_AUDIO_PROCESS,
    IStandardAudioServiceIpcCode::COMMAND_LOAD_AUDIO_EFFECT_LIBRARIES,
    IStandardAudioServiceIpcCode::COMMAND_CREATE_EFFECT_CHAIN_MANAGER,
    IStandardAudioServiceIpcCode::COMMAND_SET_OUTPUT_DEVICE_SINK,
    IStandardAudioServiceIpcCode::COMMAND_SET_ACTIVE_OUTPUT_DEVICE,
    IStandardAudioServiceIpcCode::COMMAND_CREATE_PLAYBACK_CAPTURER_MANAGER,
    IStandardAudioServiceIpcCode::COMMAND_REGIEST_POLICY_PROVIDER,
    IStandardAudioServiceIpcCode::COMMAND_REGIST_CORE_SERVICE_PROVIDER,
    IStandardAudioServiceIpcCode::COMMAND_SET_WAKEUP_SOURCE_CALLBACK,
    IStandardAudioServiceIpcCode::COMMAND_UPDATE_SPATIALIZATION_STATE,
    IStandardAudioServiceIpcCode::COMMAND_UPDATE_SPATIAL_DEVICE_TYPE,
    IStandardAudioServiceIpcCode::COMMAND_OFFLOAD_SET_VOLUME,
    IStandardAudioServiceIpcCode::COMMAND_NOTIFY_STREAM_VOLUME_CHANGED,
    IStandardAudioServiceIpcCode::COMMAND_SET_SPATIALIZATION_SCENE_TYPE,
    IStandardAudioServiceIpcCode::COMMAND_GET_MAX_AMPLITUDE,
    IStandardAudioServiceIpcCode::COMMAND_RESET_ROUTE_FOR_DISCONNECT,
    IStandardAudioServiceIpcCode::COMMAND_GET_EFFECT_LATENCY,
    IStandardAudioServiceIpcCode::COMMAND_UPDATE_LATENCY_TIMESTAMP,
    IStandardAudioServiceIpcCode::COMMAND_SET_ASR_AEC_MODE,
    IStandardAudioServiceIpcCode::COMMAND_GET_ASR_AEC_MODE,
    IStandardAudioServiceIpcCode::COMMAND_SET_ASR_NOISE_SUPPRESSION_MODE,
    IStandardAudioServiceIpcCode::COMMAND_SET_OFFLOAD_MODE,
    IStandardAudioServiceIpcCode::COMMAND_UNSET_OFFLOAD_MODE,
    IStandardAudioServiceIpcCode::COMMAND_CHECK_HIBERNATE_STATE,
    IStandardAudioServiceIpcCode::COMMAND_GET_ASR_NOISE_SUPPRESSION_MODE,
    IStandardAudioServiceIpcCode::COMMAND_SET_ASR_WHISPER_DETECTION_MODE,
    IStandardAudioServiceIpcCode::COMMAND_GET_ASR_WHISPER_DETECTION_MODE,
    IStandardAudioServiceIpcCode::COMMAND_SET_ASR_VOICE_CONTROL_MODE,
    IStandardAudioServiceIpcCode::COMMAND_SET_ASR_VOICE_MUTE_MODE,
    IStandardAudioServiceIpcCode::COMMAND_IS_WHISPERING,
    IStandardAudioServiceIpcCode::COMMAND_GET_EFFECT_OFFLOAD_ENABLED,
    IStandardAudioServiceIpcCode::COMMAND_GET_AUDIO_EFFECT_PROPERTY,
    IStandardAudioServiceIpcCode::COMMAND_SET_AUDIO_EFFECT_PROPERTY,
    IStandardAudioServiceIpcCode::COMMAND_GET_AUDIO_ENHANCE_PROPERTY,
    IStandardAudioServiceIpcCode::COMMAND_GET_AUDIO_EFFECT_PROPERTY_OUT_AUDIOEFFECTPROPERTYARRAY,
    IStandardAudioServiceIpcCode::COMMAND_SET_AUDIO_ENHANCE_PROPERTY,
    IStandardAudioServiceIpcCode::COMMAND_SET_AUDIO_EFFECT_PROPERTY_IN_AUDIOEFFECTPROPERTYARRAY,
    IStandardAudioServiceIpcCode::COMMAND_SUSPEND_RENDER_SINK,
    IStandardAudioServiceIpcCode::COMMAND_RESTORE_RENDER_SINK,
    IStandardAudioServiceIpcCode::COMMAND_LOAD_HDI_EFFECT_MODEL,
    IStandardAudioServiceIpcCode::COMMAND_UPDATE_EFFECT_BT_OFFLOAD_SUPPORTED,
    IStandardAudioServiceIpcCode::COMMAND_SET_SINK_MUTE_FOR_SWITCH_DEVICE,
    IStandardAudioServiceIpcCode::COMMAND_SET_ROTATION_TO_EFFECT,
    IStandardAudioServiceIpcCode::COMMAND_UPDATE_SESSION_CONNECTION_STATE,
    IStandardAudioServiceIpcCode::COMMAND_SET_NON_INTERRUPT_MUTE,
    IStandardAudioServiceIpcCode::COMMAND_RESTORE_SESSION,
    IStandardAudioServiceIpcCode::COMMAND_CREATE_IPC_OFFLINE_STREAM,
    IStandardAudioServiceIpcCode::COMMAND_GET_OFFLINE_AUDIO_EFFECT_CHAINS,
    IStandardAudioServiceIpcCode::COMMAND_GET_STANDBY_STATUS,
    IStandardAudioServiceIpcCode::COMMAND_GENERATE_SESSION_ID,
    IStandardAudioServiceIpcCode::COMMAND_GET_ALL_SINK_INPUTS,
    IStandardAudioServiceIpcCode::COMMAND_SET_DEFAULT_ADAPTER_ENABLE,
    IStandardAudioServiceIpcCode::COMMAND_NOTIFY_ACCOUNTS_CHANGED,
    IStandardAudioServiceIpcCode::COMMAND_NOTIFY_AUDIO_POLICY_READY,
    IStandardAudioServiceIpcCode::COMMAND_SET_INNER_CAP_LIMIT,
    IStandardAudioServiceIpcCode::COMMAND_LOAD_HDI_ADAPTER,
    IStandardAudioServiceIpcCode::COMMAND_UNLOAD_HDI_ADAPTER,
    IStandardAudioServiceIpcCode::COMMAND_CHECK_CAPTURE_LIMIT,
    IStandardAudioServiceIpcCode::COMMAND_RELEASE_CAPTURE_LIMIT,
    IStandardAudioServiceIpcCode::COMMAND_CREATE_HDI_SINK_PORT,
    IStandardAudioServiceIpcCode::COMMAND_CREATE_SINK_PORT,
    IStandardAudioServiceIpcCode::COMMAND_CREATE_HDI_SOURCE_PORT,
    IStandardAudioServiceIpcCode::COMMAND_CREATE_SOURCE_PORT,
    IStandardAudioServiceIpcCode::COMMAND_DESTROY_HDI_PORT,
    IStandardAudioServiceIpcCode::COMMAND_SET_DEVICE_CONNECTED_FLAG,
    IStandardAudioServiceIpcCode::COMMAND_SET_DM_DEVICE_TYPE,
    IStandardAudioServiceIpcCode::COMMAND_REGISTER_DATA_TRANSFER_MONITOR_PARAM,
    IStandardAudioServiceIpcCode::COMMAND_UNREGISTER_DATA_TRANSFER_MONITOR_PARAM,
    IStandardAudioServiceIpcCode::COMMAND_REGISTER_DATA_TRANSFER_CALLBACK,
    IStandardAudioServiceIpcCode::COMMAND_NOTIFY_SETTINGS_DATA_READY,
    IStandardAudioServiceIpcCode::COMMAND_IS_ACOUSTIC_ECHO_CANCELER_SUPPORTED,
    IStandardAudioServiceIpcCode::COMMAND_SET_SESSION_MUTE_STATE,
    IStandardAudioServiceIpcCode::COMMAND_SET_LATEST_MUTE_STATE,
    IStandardAudioServiceIpcCode::COMMAND_FORCE_STOP_AUDIO_STREAM,
    IStandardAudioServiceIpcCode::COMMAND_CREATE_AUDIO_WORKGROUP,
    IStandardAudioServiceIpcCode::COMMAND_RELEASE_AUDIO_WORKGROUP,
    IStandardAudioServiceIpcCode::COMMAND_ADD_THREAD_TO_GROUP,
    IStandardAudioServiceIpcCode::COMMAND_REMOVE_THREAD_FROM_GROUP,
    IStandardAudioServiceIpcCode::COMMAND_START_GROUP,
    IStandardAudioServiceIpcCode::COMMAND_STOP_GROUP,
    IStandardAudioServiceIpcCode::COMMAND_SET_BT_HDI_INVALID_STATE,
    IStandardAudioServiceIpcCode::COMMAND_SET_KARAOKE_PARAMETERS,
    IStandardAudioServiceIpcCode::COMMAND_IS_AUDIO_LOOPBACK_SUPPORTED,
    IStandardAudioServiceIpcCode::COMMAND_IMPROVE_AUDIO_WORKGROUP_PRIO,
    IStandardAudioServiceIpcCode::COMMAND_RESTORE_AUDIO_WORKGROUP_PRIO,
    IStandardAudioServiceIpcCode::COMMAND_SET_RENDER_WHITELIST,
    IStandardAudioServiceIpcCode::COMMAND_SET_FOREGROUND_LIST,
    IStandardAudioServiceIpcCode::COMMAND_GET_VOLUME_DATA_COUNT,
};

void OnRemoteRequestFuzzTest()
{
    sptr<AudioServer> audioServer = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    if (audioServer == nullptr) {
        return;
    }
    MessageParcel data;
    data.WriteInterfaceToken(StandardAudioServiceStub::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    audioServer->SetAsrNoiseSuppressionMode(0);
    if (!IStandardAudioServiceIpcCodeVec.empty()) {
        for (size_t i = 0; i < IStandardAudioServiceIpcCodeVec.size(); i++) {
            IStandardAudioServiceIpcCode audioServerInterfaceCode = IStandardAudioServiceIpcCodeVec[i];
            uint32_t code = static_cast<uint32_t>(audioServerInterfaceCode);
            audioServer->OnRemoteRequest(code, data, reply, option);
        }
    }
}

vector<TestFuncs> g_testFuncs = {
    OnRemoteRequestFuzzTest
};

} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}
