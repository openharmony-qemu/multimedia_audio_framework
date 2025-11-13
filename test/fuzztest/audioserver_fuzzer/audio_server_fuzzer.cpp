/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include <fuzzer/FuzzedDataProvider.h>
#include "audio_manager_base.h"
#include "audio_policy_manager_listener_stub_impl.h"
#include "audio_server.h"
#include "message_parcel.h"
#include "pulseaudio_ipc_interface_code.h"
#include "audio_service_types.h"
#include "../fuzz_utils.h"
using namespace std;
namespace OHOS {
namespace AudioStandard {
FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const std::u16string FORMMGR_INTERFACE_TOKEN = u"OHOS.AudioStandard.IAudioPolicy";
const int32_t SYSTEM_ABILITY_ID = 3001;
const bool RUN_ON_CREATE = false;
const int32_t NUM_2 = 2;
const uint32_t LIMIT_MIN = 0;
const int32_t AUDIO_DISTRIBUTED_SERVICE_ID = 3001;
const int32_t AUDIO_POLICY_SERVICE_ID = 3009;
const uint32_t LIMIT_MAX = static_cast<uint32_t>(AudioServerInterfaceCode::AUDIO_SERVER_CODE_MAX);
const int32_t MAX_BYTES = 1024;
const size_t MAX_BUNDLE_NAME_LENGTH = 64;
constexpr size_t MAX_RANDOM_STRING_LENGTH = 128;
const size_t THRESHOLD = 10;
constexpr float SCALE = 128.0f;
constexpr float BIAS  = 1.0f;
static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
typedef void (*TestPtr)(const uint8_t *, size_t);

const vector<std::string> g_testKeys = {
    "PCM_DUMP",
    "hpae_effect",
    "test",
};
const vector<DeviceType> g_testDeviceTypes = {
    DEVICE_TYPE_NONE,
    DEVICE_TYPE_INVALID,
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_WIRED_HEADPHONES,
    DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_BLUETOOTH_A2DP_IN,
    DEVICE_TYPE_MIC,
    DEVICE_TYPE_WAKEUP,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_DP,
    DEVICE_TYPE_REMOTE_CAST,
    DEVICE_TYPE_USB_DEVICE,
    DEVICE_TYPE_ACCESSORY,
    DEVICE_TYPE_REMOTE_DAUDIO,
    DEVICE_TYPE_HDMI,
    DEVICE_TYPE_LINE_DIGITAL,
    DEVICE_TYPE_NEARLINK,
    DEVICE_TYPE_NEARLINK_IN,
    DEVICE_TYPE_FILE_SINK,
    DEVICE_TYPE_FILE_SOURCE,
    DEVICE_TYPE_EXTERN_CABLE,
    DEVICE_TYPE_DEFAULT,
    DEVICE_TYPE_USB_ARM_HEADSET,
    DEVICE_TYPE_MAX
};
const vector<DeviceFlag> g_testDeviceFlags = {
    NONE_DEVICES_FLAG,
    OUTPUT_DEVICES_FLAG,
    INPUT_DEVICES_FLAG,
    ALL_DEVICES_FLAG,
    DISTRIBUTED_OUTPUT_DEVICES_FLAG,
    DISTRIBUTED_INPUT_DEVICES_FLAG,
    ALL_DISTRIBUTED_DEVICES_FLAG,
    ALL_L_D_DEVICES_FLAG,
    DEVICE_FLAG_MAX
};
const vector<HdiIdType> g_testHdiIdTypes = {
    HDI_ID_TYPE_PRIMARY,
    HDI_ID_TYPE_FAST,
    HDI_ID_TYPE_REMOTE,
    HDI_ID_TYPE_REMOTE_FAST,
    HDI_ID_TYPE_FILE,
    HDI_ID_TYPE_BLUETOOTH,
    HDI_ID_TYPE_OFFLOAD,
    HDI_ID_TYPE_EAC3,
    HDI_ID_TYPE_MULTICHANNEL,
    HDI_ID_TYPE_WAKEUP,
    HDI_ID_TYPE_ACCESSORY,
    HDI_ID_TYPE_NUM,
};

const std::vector<AudioParamKey> g_audioParamKeys = {
    NONE,
    VOLUME,
    INTERRUPT,
    PARAM_KEY_STATE,
    A2DP_SUSPEND_STATE,
    BT_HEADSET_NREC,
    BT_WBS,
    A2DP_OFFLOAD_STATE,
    GET_DP_DEVICE_INFO,
    GET_PENCIL_INFO,
    GET_UWB_INFO,
    USB_DEVICE,
    PERF_INFO,
    MMI,
    PARAM_KEY_LOWPOWER
};

const std::vector<DeviceRole> g_testDeviceRoles = {
    DEVICE_ROLE_NONE,
    INPUT_DEVICE,
    OUTPUT_DEVICE,
    DEVICE_ROLE_MAX
};

const std::vector<AudioScene> g_audioScenes = {
    AUDIO_SCENE_DEFAULT,
    AUDIO_SCENE_RINGING,
    AUDIO_SCENE_PHONE_CALL,
    AUDIO_SCENE_PHONE_CHAT,
    AUDIO_SCENE_CALL_START,
    AUDIO_SCENE_CALL_END,
    AUDIO_SCENE_VOICE_RINGING
};

const vector<std::u16string> gTestDumpArges = {
    u"-fb",
    u"test",
    u"test2",
    u"test3",
};

const vector<std::string> params = {
    "address=card2;device=0 role=1",
    "address=card2;device=0 role=2"
};

const vector<int32_t> gTestSystemAbilityId = {
    0,
    AUDIO_POLICY_SERVICE_ID,
    AUDIO_DISTRIBUTED_SERVICE_ID,
};

const vector<std::string> gTestAudioParameterKeys = {
    "AUDIO_EXT_PARAM_KEY_A2DP_OFFLOAD_CONFIG",
    "A2dpSuspended",
    "AUDIO_EXT_PARAM_KEY_LOWPOWER",
    "bt_headset_nrec",
    "bt_wbs",
    "AUDIO_EXT_PARAM_KEY_A2DP_OFFLOAD_CONFIG",
    "mmi",
    "perf_info",
};

const vector<std::string> tetsNetworkId = {
    "LocalDevice",
    "TestNetwork",
};

const vector<AudioParamKey> audioParamKey {
    NONE,
    VOLUME,
    INTERRUPT,
    PARAM_KEY_STATE,
    A2DP_SUSPEND_STATE,
    BT_HEADSET_NREC,
    BT_WBS,
    A2DP_OFFLOAD_STATE,
    GET_DP_DEVICE_INFO,
    GET_PENCIL_INFO,
    GET_UWB_INFO,
    USB_DEVICE,
    PERF_INFO,
    MMI,
    PARAM_KEY_LOWPOWER,
};

const vector<PlayerType> gPlayerType = {
    PLAYER_TYPE_DEFAULT,
    PLAYER_TYPE_SOUND_POOL,
    PLAYER_TYPE_AV_PLAYER,
};

static const vector<string> testPairs = {
    "unprocess_audio_effect",
    "test",
};

static const vector<string> testKeys = {
    "AUDIO_EXT_PARAM_KEY_LOWPOWER",
    "need_change_usb_device#C",
    "getSmartPAPOWER",
    "show_RealTime_ChipModel",
    "perf_info",
};

static const vector<AudioScene> testAudioScenes = {
    AUDIO_SCENE_INVALID,
    AUDIO_SCENE_DEFAULT,
    AUDIO_SCENE_RINGING,
    AUDIO_SCENE_PHONE_CALL,
    AUDIO_SCENE_PHONE_CHAT,
    AUDIO_SCENE_CALL_START,
    AUDIO_SCENE_CALL_END,
    AUDIO_SCENE_VOICE_RINGING,
    AUDIO_SCENE_MAX,
};
static const vector<BluetoothOffloadState> testBluetoothOffloadStates = {
    NO_A2DP_DEVICE,
    A2DP_NOT_OFFLOAD,
    A2DP_OFFLOAD,
};

const vector<string> newTestPairs = {
    "OPEN",
    "CLOSE",
    "UPLOAD",
    "test"
};

const std::vector<SourceType> g_sourceTypes = {
    SOURCE_TYPE_INVALID,
    SOURCE_TYPE_MIC,
    SOURCE_TYPE_VOICE_RECOGNITION,
    SOURCE_TYPE_PLAYBACK_CAPTURE,
    SOURCE_TYPE_WAKEUP,
    SOURCE_TYPE_VOICE_CALL,
    SOURCE_TYPE_VOICE_COMMUNICATION,
    SOURCE_TYPE_ULTRASONIC,
    SOURCE_TYPE_VIRTUAL_CAPTURE,
    SOURCE_TYPE_VOICE_MESSAGE,
    SOURCE_TYPE_REMOTE_CAST,
    SOURCE_TYPE_VOICE_TRANSCRIPTION,
    SOURCE_TYPE_CAMCORDER,
    SOURCE_TYPE_UNPROCESSED,
    SOURCE_TYPE_EC,
    SOURCE_TYPE_MIC_REF,
    SOURCE_TYPE_LIVE,
};

const std::vector<AudioSampleFormat> g_audioSampleFormats = {
    SAMPLE_U8,
    SAMPLE_S16LE,
    SAMPLE_S24LE,
    SAMPLE_S32LE,
    SAMPLE_F32LE,
    INVALID_WIDTH
};

const std::vector<AudioEncodingType> g_audioEncodingTypes = {
    ENCODING_INVALID,
    ENCODING_PCM,
    ENCODING_AUDIOVIVID,
    ENCODING_EAC3
};

const std::vector<AudioMode> g_audioModes = {
    AUDIO_MODE_PLAYBACK,
    AUDIO_MODE_RECORD
};

const std::vector<AudioChannel> g_audioChannels = {
    CHANNEL_UNKNOW,
    MONO,
    STEREO,
    CHANNEL_3,
    CHANNEL_4,
    CHANNEL_5,
    CHANNEL_6,
    CHANNEL_7,
    CHANNEL_8,
    CHANNEL_9,
    CHANNEL_10,
    CHANNEL_11,
    CHANNEL_12,
    CHANNEL_13,
    CHANNEL_14,
    CHANNEL_15,
    CHANNEL_16
};

const std::vector<StreamUsage> g_streamUsages = {
    STREAM_USAGE_INVALID,
    STREAM_USAGE_UNKNOWN,
    STREAM_USAGE_MEDIA,
    STREAM_USAGE_VOICE_COMMUNICATION,
    STREAM_USAGE_VOICE_ASSISTANT,
    STREAM_USAGE_ALARM,
    STREAM_USAGE_VOICE_MESSAGE,
    STREAM_USAGE_NOTIFICATION_RINGTONE,
    STREAM_USAGE_NOTIFICATION,
    STREAM_USAGE_ACCESSIBILITY,
    STREAM_USAGE_SYSTEM,
    STREAM_USAGE_MOVIE,
    STREAM_USAGE_GAME,
    STREAM_USAGE_AUDIOBOOK,
    STREAM_USAGE_NAVIGATION,
    STREAM_USAGE_DTMF,
    STREAM_USAGE_ENFORCED_TONE,
    STREAM_USAGE_ULTRASONIC,
    STREAM_USAGE_VIDEO_COMMUNICATION,
    STREAM_USAGE_RANGING,
    STREAM_USAGE_VOICE_MODEM_COMMUNICATION,
    STREAM_USAGE_VOICE_RINGTONE,
    STREAM_USAGE_VOICE_CALL_ASSISTANT
};

static const std::vector<DataTransferStateChangeType> g_dataTransferStateChangeTypes = {
    AUDIO_STREAM_START,
    AUDIO_STREAM_STOP,
    AUDIO_STREAM_PAUSE,
    DATA_TRANS_STOP,
    DATA_TRANS_RESUME,
};

AudioRendererDataTransferStateChangeInfo ConsumeAudioRendererDataTransferStateChangeInfo(FuzzedDataProvider &provider);

AudioRendererDataTransferStateChangeInfo ConsumeAudioRendererDataTransferStateChangeInfo(FuzzedDataProvider &provider)
{
    AudioRendererDataTransferStateChangeInfo ret;
    ret.clientPid = provider.ConsumeIntegral<int32_t>();
    ret.clientUID = provider.ConsumeIntegral<int32_t>();
    ret.sessionId = provider.ConsumeIntegral<int32_t>();
    ret.streamUsage = g_streamUsages[provider.ConsumeIntegral<uint32_t>() % g_streamUsages.size()];
    ret.stateChangeType = g_dataTransferStateChangeTypes[provider.ConsumeIntegral<uint32_t>() % g_streamUsages.size()];
    ret.isBackground = provider.ConsumeIntegral<int32_t>() % NUM_2;
    for (int i = 0; i < MAX_DATATRANS_TYPE; i++) {
        ret.badDataRatio[i] = provider.ConsumeIntegral<int32_t>();
    }
    return ret;
}

class DataTransferStateChangeCallbackInnerFuzzTest : public DataTransferStateChangeCallbackInner {
public:
    void OnDataTransferStateChange(const int32_t &callbackId,
            const AudioRendererDataTransferStateChangeInfo &info) override {}
    void OnMuteStateChange(const int32_t &callbackId, const int32_t &uid,
        const uint32_t &sessionId, const bool &isMuted) override {}
};
template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

void AudioServerFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    uint32_t code = provider.ConsumeIntegralInRange<uint32_t>(LIMIT_MIN, LIMIT_MAX);
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    auto payload = provider.ConsumeBytes<unsigned char>(MAX_BYTES);
    data.WriteBuffer(payload.data(), payload.size());
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    sptr<AudioServer> AudioServerPtr =
        sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    if (code == static_cast<uint32_t>(AudioServerInterfaceCode::SET_PARAMETER_CALLBACK)) {
        sptr<AudioPolicyManagerListenerStubImpl> focusListenerStub =
            new(std::nothrow) AudioPolicyManagerListenerStubImpl();
        sptr<IRemoteObject> object = focusListenerStub->AsObject();
        AudioServerPtr->SetParameterCallback(object);
        return;
    }
    if (code == static_cast<uint32_t>(AudioServerInterfaceCode::GET_ASR_AEC_MODE)) {
        int32_t asrAecMode = 0;
        AudioServerPtr->SetAsrAecMode(asrAecMode);
        AudioServerPtr->OnRemoteRequest(code, data, reply, option);
        return;
    }
    AudioServerPtr->OnRemoteRequest(code, data, reply, option);
    std::string netWorkId = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    if (g_audioParamKeys.empty()) {
        return;
    }
    AudioParamKey key = g_audioParamKeys[provider.ConsumeIntegral<uint32_t>() % g_audioParamKeys.size()];
    std::string condition = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    std::string value = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    AudioServerPtr->OnRenderSinkParamChange(netWorkId, key, condition, value);
}

float Convert2Float(const uint8_t *ptr)
{
    float floatValue = static_cast<float>(*ptr);
    return floatValue / SCALE - BIAS;
}

void AudioServerOffloadSetVolumeFuzzTest()
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    float volume = provider.ConsumeFloatingPoint<float>();
    data.WriteFloat(volume);
    MessageParcel reply;
    MessageOption option;
    sptr<AudioServer> AudioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::OFFLOAD_SET_VOLUME),
        data, reply, option);
}

void AudioServerNotifyStreamVolumeChangedFuzzTest()
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    uint32_t sizeMs = provider.ConsumeIntegral<uint32_t>();
    data.WriteUint32(sizeMs);
    MessageParcel reply;
    MessageOption option;
    sptr<AudioServer> AudioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::NOTIFY_STREAM_VOLUME_CHANGED),
        data, reply, option);
}

void AudioServerResetRouteForDisconnectFuzzTest()
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    int32_t deviceType = provider.ConsumeIntegral<int32_t>();
    data.WriteInt32(deviceType);
    MessageParcel reply;
    MessageOption option;
    sptr<AudioServer> AudioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::RESET_ROUTE_FOR_DISCONNECT),
        data, reply, option);
}

void AudioServerGetEffectLatencyTest()
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    std::string sessionId = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    data.WriteString(sessionId);
    sptr<AudioServer> AudioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::GET_EFFECT_LATENCY),
        data, reply, option);
}

void AudioServerUpdateLatencyTimestampTest()
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    std::string timestamp = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    bool isRenderer = provider.ConsumeBool();
    data.WriteString(timestamp);
    data.WriteBool(isRenderer);
    sptr<AudioServer> AudioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_LATENCY_TIMESTAMP),
        data, reply, option);
}

void AudioServerGetMaxAmplitudeTest()
{
    MessageParcel data;
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    bool isOutputDevice = provider.ConsumeBool();
    int32_t deviceType = provider.ConsumeIntegral<int32_t>();
    data.WriteBool(isOutputDevice);
    data.WriteInt32(deviceType);
    sptr<AudioServer> AudioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::GET_MAX_AMPLITUDE),
        data, reply, option);
}

void AudioServerCreatePlaybackCapturerManagerTest()
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    sptr<AudioServer> AudioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    int32_t deviceType = provider.ConsumeIntegral<int32_t>();
    data.WriteInt32(deviceType);
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::CREATE_PLAYBACK_CAPTURER_MANAGER),
        data, reply, option);
}

void AudioServerSetOutputDeviceSinkTest()
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    int32_t deviceType = provider.ConsumeIntegral<int32_t>();
    std::string sinkName = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    data.WriteInt32(deviceType);
    data.WriteString(sinkName);
    sptr<AudioServer> AudioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_OUTPUT_DEVICE_SINK),
        data, reply, option);
}

void AudioServerSetAudioMonoStateTest()
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    bool audioMono = provider.ConsumeBool();
    data.WriteBool(audioMono);
    sptr<AudioServer> AudioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_MONO_STATE),
        data, reply, option);
}

void AudioServerSetVoiceVolumeTest()
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    float volume = provider.ConsumeFloatingPoint<float>();
    data.WriteFloat(volume);
    sptr<AudioServer> AudioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_VOICE_VOLUME),
        data, reply, option);
}

void AudioServerCheckRemoteDeviceStateTest()
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    std::string networkId = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    if (g_testDeviceRoles.empty()) {
        return;
    }
    DeviceRole deviceRole = g_testDeviceRoles[provider.ConsumeIntegral<uint32_t>() % g_testDeviceRoles.size()];
    bool isStartDevice = provider.ConsumeBool();
    data.WriteString(networkId);
    data.WriteInt32(static_cast<int32_t>(deviceRole));
    data.WriteBool(isStartDevice);
    sptr<AudioServer> AudioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::CHECK_REMOTE_DEVICE_STATE),
        data, reply, option);
}

void AudioServerNotifyDeviceInfoTest()
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    std::string networkId = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    bool connected = provider.ConsumeBool();
    data.WriteString(networkId);
    data.WriteBool(connected);

    sptr<AudioServer> AudioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::NOTIFY_DEVICE_INFO),
        data, reply, option);
}

void AudioServerGetAudioParameterTest()
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    std::string key = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    data.WriteString(key);
    sptr<AudioServer> AudioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::GET_AUDIO_PARAMETER),
        data, reply, option);
}

void AudioServerSetAudioParameterTest()
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    std::string key = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    std::string value = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    data.WriteString(key);
    data.WriteString(value);
    sptr<AudioServer> AudioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_PARAMETER),
        data, reply, option);
}

void AudioServerSetMicrophoneMuteTest()
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    bool isMute = provider.ConsumeBool();
    data.WriteBool(isMute);
    sptr<AudioServer> AudioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_MICROPHONE_MUTE),
        data, reply, option);
}

void AudioServerSetAudioBalanceValueTest()
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    float audioBalance = provider.ConsumeFloatingPoint<float>();
    data.WriteFloat(audioBalance);
    sptr<AudioServer> AudioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_BALANCE_VALUE),
        data, reply, option);
}

void AudioServerSetAudioSceneTest()
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    if (g_audioScenes.empty()) {
        return;
    }
    AudioScene audioScene = g_audioScenes[provider.ConsumeIntegral<uint32_t>() % g_audioScenes.size()];
    data.WriteInt32(static_cast<int32_t>(audioScene));
    sptr<AudioServer> AudioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_SCENE),
        data, reply, option);
}

void AudioServerSetOffloadModeTest()
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    uint32_t sessionId = provider.ConsumeIntegral<uint32_t>();
    int32_t state = provider.ConsumeIntegral<int32_t>();
    bool isAppBack = provider.ConsumeBool();
    data.WriteUint32(sessionId);
    data.WriteInt32(state);
    data.WriteBool(isAppBack);
    sptr<AudioServer> AudioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_OFFLOAD_MODE),
        data, reply, option);
}

void AudioServerUnsetOffloadTest()
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    uint32_t sessionId = provider.ConsumeIntegral<uint32_t>();
    data.WriteUint32(sessionId);
    sptr<AudioServer> AudioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::UNSET_OFFLOAD_MODE),
        data, reply, option);
}

void AudioServerCheckHibernateStateTest()
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    bool hibernate = provider.ConsumeBool();
    data.WriteBool(hibernate);
    sptr<AudioServer> AudioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::CHECK_HIBERNATE_STATE),
        data, reply, option);
}

void AudioServerSetSessionMuteStateTest()
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    uint32_t sessionId = provider.ConsumeIntegral<uint32_t>();
    data.WriteInt32(sessionId);
    data.WriteBool(true);
    sptr<AudioServer> AudioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_SESSION_MUTE_STATE),
        data, reply, option);
}

void AudioServerNotifyMuteStateChangeTest()
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    uint32_t sessionId = provider.ConsumeIntegral<uint32_t>();
    data.WriteInt32(sessionId);
    data.WriteBool(true);

    sptr<AudioServer> AudioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::NOTIFY_MUTE_STATE_CHANGE),
        data, reply, option);
}

void AudioServerAudioWorkgroupCreateTest()
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);

    sptr<AudioServer> AudioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::CREATE_AUDIOWORKGROUP),
        data, reply, option);
}

void AudioServerAudioWorkgroupReleaseTest()
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    int32_t workgroupId = provider.ConsumeIntegral<int32_t>();
    data.WriteInt32(static_cast<int32_t>(workgroupId));

    sptr<AudioServer> AudioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::RELEASE_AUDIOWORKGROUP),
        data, reply, option);
}

void AudioServerAudioWorkgroupAddThreadTest()
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    int32_t workgroupId = provider.ConsumeIntegral<int32_t>();
    int32_t tokenId = provider.ConsumeIntegral<int32_t>();
    data.WriteInt32(static_cast<int32_t>(workgroupId));
    data.WriteInt32(static_cast<int32_t>(tokenId));

    sptr<AudioServer> AudioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::ADD_THREAD_TO_AUDIOWORKGROUP),
        data, reply, option);
}

void AudioServerAudioWorkgroupRemoveThreadTest()
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    int32_t workgroupId = provider.ConsumeIntegral<int32_t>();
    int32_t tokenId = provider.ConsumeIntegral<int32_t>();
    data.WriteInt32(static_cast<int32_t>(workgroupId));
    data.WriteInt32(static_cast<int32_t>(tokenId));

    sptr<AudioServer> AudioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::REMOVE_THREAD_FROM_AUDIOWORKGROUP),
        data, reply, option);
}

void AudioServerAudioWorkgroupStartGroupTest()
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    int32_t workgroupId = provider.ConsumeIntegral<int32_t>();
    uint64_t startTime = provider.ConsumeIntegral<uint64_t>();
    uint64_t deadlineTime = provider.ConsumeIntegral<uint64_t>();
    data.WriteInt32(static_cast<int32_t>(workgroupId));
    data.WriteUint64(static_cast<int32_t>(startTime));
    data.WriteUint64(static_cast<int32_t>(deadlineTime));

    sptr<AudioServer> AudioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::START_AUDIOWORKGROUP),
        data, reply, option);
}

void AudioServerAudioWorkgroupStopGroupTest()
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    int32_t workgroupId = provider.ConsumeIntegral<int32_t>();
    data.WriteInt32(static_cast<int32_t>(workgroupId));

    sptr<AudioServer> AudioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::STOP_AUDIOWORKGROUP),
        data, reply, option);
}

void AudioServerDumpTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    std::vector<std::u16string> args(gTestDumpArges.begin(), gTestDumpArges.begin() +
        (provider.ConsumeIntegral<uint32_t>() % gTestDumpArges.size()));
    int32_t fd = provider.ConsumeIntegral<int32_t>();

    audioServerPtr->Dump(fd, args);
}

void AudioServerGetUsbParameterTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    std::string param = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->GetUsbParameter(param);
}

void AudioServerOnAddSystemAbilityTest()
{
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);

    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    if (gTestSystemAbilityId.empty()) {
        return;
    }
    uint32_t id = provider.ConsumeIntegral<uint32_t>() % gTestSystemAbilityId.size();
    std::string deviceId = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);

    audioServerPtr->OnAddSystemAbility(gTestSystemAbilityId[id], deviceId);
}

void AudioServerSetExtraParametersTest()
{
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    std::vector<std::pair<std::string, std::string>> kvpairs;
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    if (g_testKeys.empty()) {
        return;
    }
    uint32_t id = provider.ConsumeIntegral<uint32_t>() % g_testKeys.size();
    std::string key = g_testKeys[id];
    std::pair<std::string, std::string> kvpair = std::make_pair(g_testKeys[id], g_testKeys[id]);
    kvpairs.push_back(kvpair);
    audioServerPtr->CacheExtraParameters(key, kvpairs);
    audioServerPtr->ParseAudioParameter();
}

void AudioServerSetAudioParameterByKeyTest()
{
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    if (gTestAudioParameterKeys.empty()) {
        return;
    }
    uint32_t id = provider.ConsumeIntegral<uint32_t>() % gTestAudioParameterKeys.size();
    audioServerPtr->SetAudioParameter(gTestAudioParameterKeys[id], "");
}

void AudioServerGetExtraParametersTest()
{
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    if (g_testKeys.empty()) {
        return;
    }
    uint32_t id = provider.ConsumeIntegral<uint32_t>() % g_testKeys.size();
    bool isAudioParameterParsed = provider.ConsumeIntegral<uint32_t>() % NUM_2;
    audioServerPtr->isAudioParameterParsed_.store(isAudioParameterParsed);
    std::vector<StringPair> result;
    std::vector<std::string> subKeys;
    audioServerPtr->GetExtraParameters(g_testKeys[id], subKeys, result);
}

void AudioServerGetAudioParameterByIdTest()
{
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    string str = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    if (audioParamKey.empty() || tetsNetworkId.empty()) {
        return;
    }
    uint32_t id = provider.ConsumeIntegral<uint32_t>() % audioParamKey.size();
    AudioParamKey key = static_cast<AudioParamKey>(audioParamKey[id]);
    id = provider.ConsumeIntegral<uint32_t>() % tetsNetworkId.size();
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    audioServerPtr->GetAudioParameter(tetsNetworkId[id], key, "", str);
}

void AudioServerIsFastBlockedTest()
{
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    if (gPlayerType.empty()) {
        return;
    }
    uint32_t id = provider.ConsumeIntegral<uint32_t>() % gPlayerType.size();
    audioServerPtr->IsFastBlocked(static_cast<uint32_t>(id), gPlayerType[id]);
}

void AudioServerCheckRemoteDeviceStateTestTwo()
{
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    bool isStartDevice = provider.ConsumeIntegral<uint32_t>() % NUM_2;
    if (g_testDeviceRoles.empty()) {
        return;
    }
    uint32_t id = provider.ConsumeIntegral<uint32_t>() % g_testDeviceRoles.size();
    audioServerPtr->CheckRemoteDeviceState("LocalDevice", g_testDeviceRoles[id], isStartDevice);
}

void AudioServerCreateAudioStreamTest()
{
    sptr<IRemoteObject> remoteObject = nullptr;
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->NotifyProcessStatus();
    AudioProcessConfig config;
    std::shared_ptr<PipeInfoGuard> pipeinfoGuard = std::make_shared<PipeInfoGuard>(0);
    vector<int32_t> gTestCallingUid = {
        AudioServer::VASSISTANT_UID,
        AudioServer::MEDIA_SERVICE_UID,
    };
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    if (gTestCallingUid.empty()) {
        return;
    }
    uint32_t id = provider.ConsumeIntegral<uint32_t>() % gTestCallingUid.size();
    config.audioMode = AUDIO_MODE_RECORD;
    remoteObject = audioServerPtr->CreateAudioStream(config, gTestCallingUid[id], pipeinfoGuard);

    config.audioMode = static_cast<AudioMode>(-1);
    audioServerPtr->IsNormalIpcStream(config);
    if (audioParamKey.empty()) {
        return;
    }
    id = provider.ConsumeIntegral<uint32_t>() % audioParamKey.size();
    audioServerPtr->OnRenderSinkParamChange("", audioParamKey[id], "", "");
    audioServerPtr->OnCaptureSourceParamChange("", audioParamKey[id], "", "");
    audioServerPtr->OnWakeupClose();
    id = provider.ConsumeIntegral<uint32_t>() % NUM_2;
    audioServerPtr->OnCapturerState(static_cast<bool>(id), id, id);
    audioServerPtr->SetParameterCallback(remoteObject);
    audioServerPtr->SetWakeupSourceCallback(remoteObject);
    audioServerPtr->RegiestPolicyProvider(remoteObject);
    audioServerPtr->RegistCoreServiceProvider(remoteObject);
}

void AudioServerSetSinkRenderEmptyTest()
{
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    uint32_t id = provider.ConsumeIntegral<uint32_t>() % NUM_2;
    audioServerPtr->SetSinkRenderEmpty("primary", id);
}

void AudioServerOnRenderSinkStateChangeTest()
{
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    uint32_t id = provider.ConsumeIntegral<uint32_t>() % NUM_2;
    audioServerPtr->OnRenderSinkStateChange(id, static_cast<bool>(id));
}

void AudioServerCreateHdiSinkPortTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    uint32_t renderId = provider.ConsumeIntegral<uint32_t>();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    IAudioSinkAttr attr;
    std::string deviceClass = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    std::string idInfo = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    audioServerPtr->CreateHdiSinkPort(deviceClass, idInfo, attr, renderId);
}

void AudioServerCreateHdiSourcePortTest()
{
    uint32_t captureId = 0;
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    IAudioSourceAttr attr;
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    std::string deviceClass = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    std::string idInfo = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    audioServerPtr->CreateHdiSourcePort(deviceClass, idInfo, attr, captureId);
}

void AudioServerRegisterDataTransferCallbackTest()
{
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    sptr<AudioPolicyManagerListenerStubImpl> focusListenerStub = new(std::nothrow) AudioPolicyManagerListenerStubImpl();
    sptr<IRemoteObject> object = focusListenerStub->AsObject();

    audioServerPtr->RegisterDataTransferCallback(object);
}

void AudioServerWriteServiceStartupErrorTest()
{
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->WriteServiceStartupError();
}

void AudioServerProcessKeyValuePairsTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    string key = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    string value{};
    if (testPairs.empty()) {
        return;
    }
    string pairTest = testPairs[provider.ConsumeIntegral<uint32_t>() % testPairs.size()];
    std::vector<std::pair<std::string, std::string>> kvpairs;
    kvpairs.push_back(make_pair(pairTest, "test_value"));
    set<std::string> subKeys = {"effect"};
    unordered_map<std::string, std::set<std::string>> subKeyMap;
    subKeyMap.insert(make_pair(pairTest, subKeys));

    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->ProcessKeyValuePairs(key, kvpairs, subKeyMap, value);
}

void AudioServerSetA2dpAudioParameterTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    string renderValue = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);

    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->SetA2dpAudioParameter(renderValue);
}

void AudioServerGetAudioParameterByKeyTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    string value = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    if (testKeys.empty()) {
        return;
    }
    string key = testKeys[provider.ConsumeIntegral<uint32_t>() % testKeys.size()];
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    audioServerPtr->GetAudioParameter(key, value);
}

void AudioServerGetDPParameterTest()
{
    std::string condition;
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->GetDPParameter(condition);
}

void AudioServerSetAudioSceneByDeviceTypeTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    uint32_t index = provider.ConsumeIntegral<uint32_t>();
    bool scoExcludeFlag = provider.ConsumeBool();
    if (testBluetoothOffloadStates.empty()) {
        return;
    }
    BluetoothOffloadState a2dpOffloadFlag = testBluetoothOffloadStates[index % testBluetoothOffloadStates.size()];
    if (testAudioScenes.empty()) {
        return;
    }
    AudioScene audioScene = testAudioScenes[provider.ConsumeIntegral<uint32_t>() % testAudioScenes.size()];
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->SetAudioScene(audioScene, a2dpOffloadFlag, scoExcludeFlag);
}

void AudioServerNotifyDeviceInfoFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    std::string networkId = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    bool connected = provider.ConsumeBool();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    if (audioServerPtr == nullptr) {
        return;
    }
    audioServerPtr->NotifyDeviceInfo(networkId, connected);
}

void AudioServerSetVoiceVolumeFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    float volume = provider.ConsumeFloatingPoint<float>();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    if (audioServerPtr == nullptr) {
        return;
    }
    audioServerPtr->SetVoiceVolume(volume);
}

void AudioServerCheckRemoteDeviceStateFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    std::string networkId = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    uint32_t deviceId = provider.ConsumeIntegral<uint32_t>();
    if (g_testDeviceRoles.empty()) {
        return;
    }
    DeviceRole deviceRole = g_testDeviceRoles[deviceId % g_testDeviceRoles.size()];
    bool isStartDevice = provider.ConsumeIntegral<uint32_t>();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    if (audioServerPtr == nullptr) {
        return;
    }
    audioServerPtr->CheckRemoteDeviceState(networkId, deviceRole, isStartDevice);
}

void AudioServerSetAudioBalanceValueFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    float audioBalance = provider.ConsumeFloatingPoint<float>();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    if (audioServerPtr == nullptr) {
        return;
    }
    audioServerPtr->SetAudioBalanceValue(audioBalance);
}

void AudioServerRemoveRendererDataTransferCallbackFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    int32_t testPid = provider.ConsumeIntegral<int32_t>();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->RemoveRendererDataTransferCallback(testPid);
}

void AudioServerRegisterDataTransferCallbackFuzzTest()
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    auto payload = provider.ConsumeBytes<unsigned char>(MAX_BYTES);
    data.WriteBuffer(payload.data(), payload.size());
    data.RewindRead(0);
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->RegisterDataTransferCallback(object);
}

void AudioServerRegisterDataTransferMonitorParamFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    int32_t testCallbackId = provider.ConsumeIntegral<int32_t>();
    DataTransferMonitorParam param;
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->RegisterDataTransferMonitorParam(testCallbackId, param);
}

void AudioServerUnregisterDataTransferMonitorParamFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    int32_t testCallbackId = provider.ConsumeIntegral<int32_t>();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->UnregisterDataTransferMonitorParam(testCallbackId);
}

void AudioServerOnDataTransferStateChangeFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    int32_t testPid = provider.ConsumeIntegral<int32_t>();
    int32_t testCallbackId = provider.ConsumeIntegral<int32_t>();
    AudioRendererDataTransferStateChangeInfo info;
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->OnDataTransferStateChange(testPid, testCallbackId, info);
}

void AudioServerRegisterDataTransferStateChangeCallbackFuzzTest()
{
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->RegisterDataTransferStateChangeCallback();
}

void AudioServerInitMaxRendererStreamCntPerUidFuzzTest()
{
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->InitMaxRendererStreamCntPerUid();
}

void AudioServerSetPcmDumpParameterFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    if (newTestPairs.empty()) {
        return;
    }
    string pairTest = newTestPairs[provider.ConsumeIntegral<uint32_t>() % newTestPairs.size()];
    std::vector<std::pair<std::string, std::string>> params;
    params.push_back(make_pair(pairTest, "test_value"));
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->SetPcmDumpParameter(params);
}

void AudioServerSuspendRenderSinkFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    std::string sinkName = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->SuspendRenderSink(sinkName);
}

void AudioServerRestoreRenderSinkFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    std::string sinkName = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->RestoreRenderSink(sinkName);
}

void AudioServerSetAudioParameterFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    std::string networkId = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    int32_t key = provider.ConsumeIntegral<int32_t>();
    std::string condition = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    std::string value = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->SetAudioParameter(networkId, key, condition, value);
}

void AudioServerGetTransactionIdFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    if (g_testDeviceTypes.empty() || g_testDeviceRoles.empty()) {
        return;
    }
    DeviceType deviceType = g_testDeviceTypes[provider.ConsumeIntegral<uint32_t>() % g_testDeviceTypes.size()];
    DeviceRole deviceRole = g_testDeviceRoles[provider.ConsumeIntegral<uint32_t>() % g_testDeviceRoles.size()];
    uint64_t transactionId = provider.ConsumeIntegral<uint64_t>();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->GetTransactionId(deviceType, deviceRole, transactionId);
}

void AudioServerSetIORoutesFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    if (g_testDeviceTypes.empty() || g_testDeviceFlags.empty()) {
        return;
    }
    DeviceType deviceType = g_testDeviceTypes[provider.ConsumeIntegral<uint32_t>() % g_testDeviceTypes.size()];
    DeviceFlag deviceFlag = g_testDeviceFlags[provider.ConsumeIntegral<uint32_t>() % g_testDeviceFlags.size()];
    std::vector<std::pair<DeviceType, DeviceFlag>> activeDevices;
    std::pair<DeviceType, DeviceFlag> activeDevice = std::make_pair(deviceType, deviceFlag);
    activeDevices.push_back(activeDevice);
    uint32_t index = provider.ConsumeIntegral<uint32_t>();
    if (testBluetoothOffloadStates.empty()) {
        return;
    }
    BluetoothOffloadState a2dpOffloadFlag = testBluetoothOffloadStates[index % testBluetoothOffloadStates.size()];
    std::string deviceName = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->SetIORoutes(activeDevices, a2dpOffloadFlag, deviceName);
    std::vector<DeviceType> deviceTypes = {deviceType};
    audioServerPtr->SetIORoutes(deviceType, deviceFlag, deviceTypes, a2dpOffloadFlag, deviceName);
}

void AudioServerUpdateActiveDeviceRouteFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    int32_t type = provider.ConsumeIntegral<int32_t>();
    int32_t flag = provider.ConsumeIntegral<int32_t>();
    int32_t a2dpOffloadFlag = provider.ConsumeIntegral<int32_t>();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->UpdateActiveDeviceRoute(type, flag, a2dpOffloadFlag);
}

void AudioServerUpdateActiveDevicesRouteFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    std::vector<IntPair> activeDevices;
    int32_t a2dpOffloadFlag = provider.ConsumeIntegral<int32_t>();
    std::string deviceName = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->UpdateActiveDevicesRoute(activeDevices, a2dpOffloadFlag, deviceName);
}

void AudioServerSetDmDeviceTypeFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    uint16_t dmDeviceType = provider.ConsumeIntegral<uint16_t>();
    int32_t deviceTypeIn = provider.ConsumeIntegral<int32_t>();
    std::vector<IntPair> activeDevices;
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->SetDmDeviceType(dmDeviceType, deviceTypeIn);
}

void AudioServerSetAudioMonoStateFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    bool audioMono = provider.ConsumeBool();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->SetAudioMonoState(audioMono);
}

void AudioServerGetHapBuildApiVersionFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    int32_t callerUid = provider.ConsumeIntegral<int32_t>();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->GetHapBuildApiVersion(callerUid);
}

void AudioServerResetRecordConfigFuzzTest()
{
    AudioProcessConfig config;
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->ResetRecordConfig(config);
}

void AudioServerResetProcessConfigFuzzTest()
{
    AudioProcessConfig config;
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->ResetProcessConfig(config);
}

void AudioServerCheckStreamInfoFormatFuzzTest()
{
    AudioProcessConfig config;
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CheckStreamInfoFormat(config);
}

void AudioServerCheckRendererFormatFuzzTest()
{
    AudioProcessConfig config;
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CheckRendererFormat(config);
}

void AudioServerCheckRecorderFormatFuzzTest()
{
    AudioProcessConfig config;
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CheckRecorderFormat(config);
}

void AudioServerCheckConfigFormatFuzzTest()
{
    AudioProcessConfig config;
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CheckConfigFormat(config);
}

void AudioServerSendCreateErrorInfoFuzzTest()
{
    AudioProcessConfig config;
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    int32_t errorCode = provider.ConsumeIntegral<int32_t>();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->SendCreateErrorInfo(config, errorCode);
}

void AudioServerCheckMaxRendererInstancesFuzzTest()
{
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CheckMaxRendererInstances();
}

void AudioServerCheckMaxLoopbackInstancesFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    int32_t modeCount = static_cast<int32_t>(AudioMode::AUDIO_MODE_RECORD) + 1;
    uint8_t index = provider.ConsumeIntegral<uint8_t>();
    AudioMode audioMode = static_cast<AudioMode>(index % modeCount);
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CheckMaxLoopbackInstances(audioMode);
}

void AudioServerCheckAndWaitAudioPolicyReadyFuzzTest()
{
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CheckAndWaitAudioPolicyReady();
}

void AudioServerIsSatelliteFuzzTest()
{
    AudioProcessConfig config;
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    int32_t callingUid = provider.ConsumeIntegral<int32_t>();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->IsSatellite(config, callingUid);
}

void AudioServerCreateAudioProcessFuzzTest()
{
    sptr<IRemoteObject> client = nullptr;
    AudioProcessConfig config;
    AudioPlaybackCaptureConfig filterConfig = AudioPlaybackCaptureConfig();
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    int32_t errorCode = provider.ConsumeIntegral<int32_t>();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CreateAudioProcess(config, errorCode, filterConfig, client);
}

void AudioServerCreateAudioProcessInnerFuzzTest()
{
    AudioProcessConfig config;
    AudioPlaybackCaptureConfig filterConfig = AudioPlaybackCaptureConfig();
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    int32_t errorCode = provider.ConsumeIntegral<int32_t>();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CreateAudioProcessInner(config, errorCode, filterConfig);
}

void AudioServerPermissionCheckerFuzzTest()
{
    AudioProcessConfig config;
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->PermissionChecker(config);
}

void AudioServerCheckPlaybackPermissionFuzzTest()
{
    AudioProcessConfig config;
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CheckPlaybackPermission(config);
}

void AudioServerCheckInnerRecorderPermissionFuzzTest()
{
    AudioProcessConfig config;
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CheckInnerRecorderPermission(config);
}

void AudioServerHandleCheckRecorderBackgroundCaptureFuzzTest()
{
    AudioProcessConfig config;
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    config.callerUid = provider.ConsumeIntegral<int32_t>();
    if (g_sourceTypes.empty()) {
        return;
    }
    config.capturerInfo.sourceType = g_sourceTypes[provider.ConsumeIntegral<int32_t>() % g_sourceTypes.size()];
    config.appInfo.appTokenId = provider.ConsumeIntegral<uint32_t>();
    config.appInfo.appFullTokenId = provider.ConsumeIntegral<uint64_t>();
    config.originalSessionId = provider.ConsumeIntegral<uint32_t>();
    config.appInfo.appUid = provider.ConsumeIntegral<int32_t>();
    config.appInfo.appPid = provider.ConsumeIntegral<int32_t>();

    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->HandleCheckRecorderBackgroundCapture(config);
}

void AudioServerSetForegroundListFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    std::vector<std::string> list = {provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH)};
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->SetForegroundList(list);
}

void AudioServerCreatePlaybackCapturerManagerFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    bool isSuccess = provider.ConsumeBool();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CreatePlaybackCapturerManager(isSuccess);
}

void AudioServerRegisterAudioCapturerSourceCallbackFuzzTest()
{
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->RegisterAudioCapturerSourceCallback();
}

void AudioServerRegisterAudioRendererSinkCallbackFuzzTest()
{
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->RegisterAudioRendererSinkCallback();
}

void AudioServerGetMaxAmplitudeFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    bool isOutputDevice = provider.ConsumeBool();
    std::string deviceClass = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    float maxAmplitude = provider.ConsumeFloatingPoint<float>();
    int32_t sourceType = provider.ConsumeIntegral<int32_t>();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->GetMaxAmplitude(isOutputDevice, deviceClass, sourceType, maxAmplitude);
}

void AudioServerGetVolumeDataCountFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    std::string sinkName = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    int64_t volumeData = provider.ConsumeIntegral<int64_t>();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->GetVolumeDataCount(sinkName, volumeData);
}

void AudioServerUpdateLatencyTimestampFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    std::string timestamp = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    bool isRenderer = provider.ConsumeBool();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->UpdateLatencyTimestamp(timestamp, isRenderer);
}

void AudioServerCheckHibernateStateFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    bool hibernate = provider.ConsumeBool();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CheckHibernateState(hibernate);
}

void AudioServerCreateIpcOfflineStreamFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    int32_t errorCode = provider.ConsumeIntegral<int32_t>();
    sptr<IRemoteObject> client = nullptr;
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CreateIpcOfflineStream(errorCode, client);
}

void AudioServerGetOfflineAudioEffectChainsFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    std::vector<std::string> effectChains = {provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH)};
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->GetOfflineAudioEffectChains(effectChains);
}

void AudioServerGetStandbyStatusFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    uint32_t sessionId = provider.ConsumeIntegral<uint32_t>();
    bool isStandby = provider.ConsumeBool();
    int64_t enterStandbyTime = provider.ConsumeIntegral<int64_t>();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->GetStandbyStatus(sessionId, isStandby, enterStandbyTime);
}

void AudioServerGenerateSessionIdFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    uint32_t sessionId = provider.ConsumeIntegral<uint32_t>();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->GenerateSessionId(sessionId);
}

void AudioServerNotifyAudioPolicyReadyFuzzTest()
{
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->NotifyAudioPolicyReady();
}

void AudioServerGetAllSinkInputsFuzzTest()
{
    std::vector<SinkInput> sinkInputs;
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->GetAllSinkInputs(sinkInputs);
}

void AudioServerReleaseCaptureLimitFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    int32_t innerCapId = provider.ConsumeIntegral<int32_t>();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->ReleaseCaptureLimit(innerCapId);
}

void AudioServerLoadHdiAdapterFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    uint32_t devMgrType = provider.ConsumeIntegral<uint32_t>();
    std::string adapterName = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->LoadHdiAdapter(devMgrType, adapterName);
}

void AudioServerUnloadHdiAdapterFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    uint32_t devMgrType = provider.ConsumeIntegral<uint32_t>();
    std::string adapterName = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    bool force = provider.ConsumeBool();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->UnloadHdiAdapter(devMgrType, adapterName, force);
}

void AudioServerCreateSinkPortFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    uint32_t idBase = provider.ConsumeIntegral<uint32_t>();
    uint32_t idType = provider.ConsumeIntegral<uint32_t>();
    std::string idInfo = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    IAudioSinkAttr attr;
    uint32_t renderId = provider.ConsumeIntegral<uint32_t>();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CreateSinkPort(idBase, idType, idInfo, attr, renderId);
}

void AudioServerCreateSourcePortFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    uint32_t idBase = provider.ConsumeIntegral<uint32_t>();
    uint32_t idType = provider.ConsumeIntegral<uint32_t>();
    std::string idInfo = provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH);
    IAudioSourceAttr attr;
    uint32_t captureId = provider.ConsumeIntegral<uint32_t>();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CreateSourcePort(idBase, idType, idInfo, attr, captureId);
}

void AudioServerDestroyHdiPortFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    uint32_t id = provider.ConsumeIntegral<uint32_t>();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->DestroyHdiPort(id);
}

void AudioServerSetDeviceConnectedFlagFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    bool flag = provider.ConsumeIntegral<uint32_t>();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->SetDeviceConnectedFlag(flag);
}

void AudioServerSetBtHdiInvalidStateFuzzTest()
{
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->SetBtHdiInvalidState();
}

void AudioServerCreateAudioWorkgroupFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    sptr<IRemoteObject> object = nullptr;
    int32_t workgroupId = provider.ConsumeIntegral<int32_t>();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->CreateAudioWorkgroup(object, workgroupId);
}

void AudioServerReleaseAudioWorkgroupFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    int32_t workgroupId = provider.ConsumeIntegral<int32_t>();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->ReleaseAudioWorkgroup(workgroupId);
}

void AudioServerAddThreadToGroupFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    int32_t workgroupId = provider.ConsumeIntegral<int32_t>();
    int32_t tokenId = provider.ConsumeIntegral<int32_t>();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->AddThreadToGroup(workgroupId, tokenId);
}

void AudioServerForceStopAudioStreamFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    int32_t audioType = provider.ConsumeIntegral<int32_t>();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->ForceStopAudioStream(audioType);
}

void AudioServerStartGroupFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    int32_t workgroupId = provider.ConsumeIntegral<int32_t>();
    uint64_t startTime = provider.ConsumeIntegral<uint64_t>();
    uint64_t deadlineTime = provider.ConsumeIntegral<uint64_t>();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->StartGroup(workgroupId, startTime, deadlineTime);
}

void AudioServerStopGroupFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    int32_t workgroupId = provider.ConsumeIntegral<int32_t>();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->StopGroup(workgroupId);
}

void AudioServerSetActiveOutputDeviceFuzzTest()
{
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    if (g_testDeviceTypes.empty()) {
        return;
    }
    int32_t deviceTypeId = provider.ConsumeIntegral<int32_t>() % g_testDeviceTypes.size();
    int32_t deviceType = g_testDeviceTypes[deviceTypeId];
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServerPtr->SetActiveOutputDevice(deviceType);
}
void AudioServerResetRecordConfigSourceTypeFuzzTest()
{
    AudioProcessConfig config;
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    if (g_sourceTypes.empty()) {
        return;
    }
    config.capturerInfo.sourceType = g_sourceTypes[provider.ConsumeIntegral<int32_t>() % g_sourceTypes.size()];
    audioServerPtr->ResetRecordConfig(config);
}

void AudioServerResetProcessConfigCallerUidFuzzTest()
{
    AudioProcessConfig config;
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    config.callerUid = provider.ConsumeIntegral<int32_t>();
    audioServerPtr->ResetProcessConfig(config);
}

void AudioServerCheckStreamInfoFormatNotContainFuzzTest()
{
    AudioProcessConfig config;
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    if (g_audioSampleFormats.empty() || g_audioEncodingTypes.empty() || g_audioModes.empty() ||
     g_audioChannels.empty()) {
        return;
    }
    g_audioSampleFormats[provider.ConsumeIntegral<uint32_t>() % g_audioSampleFormats.size()];
    config.streamInfo.format = g_audioSampleFormats[provider.ConsumeIntegral<uint32_t>() % g_audioSampleFormats.size()];
    config.streamInfo.encoding =
        g_audioEncodingTypes[provider.ConsumeIntegral<uint32_t>() % g_audioEncodingTypes.size()];
    config.audioMode = g_audioModes[provider.ConsumeIntegral<uint32_t>() % g_audioModes.size()];
    config.streamInfo.channels = g_audioChannels[provider.ConsumeIntegral<uint32_t>() % g_audioChannels.size()];
    audioServerPtr->CheckStreamInfoFormat(config);
}

void AudioServerCheckRendererFormatNotContainFuzzTest()
{
    AudioProcessConfig config;
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    if (g_streamUsages.empty()) {
        return;
    }
    config.rendererInfo.streamUsage = g_streamUsages[provider.ConsumeIntegral<uint32_t>() % g_streamUsages.size()];
    audioServerPtr->CheckRendererFormat(config);
}

void AudioServerCheckRecorderFormatNotContainFuzzTest()
{
    AudioProcessConfig config;
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    if (g_sourceTypes.empty()) {
        return;
    }
    config.capturerInfo.sourceType = g_sourceTypes[provider.ConsumeIntegral<int32_t>() % g_sourceTypes.size()];
    config.capturerInfo.capturerFlags = provider.ConsumeIntegral<int32_t>();
    audioServerPtr->CheckRecorderFormat(config);
}

void AudioServerCreateAudioProcessInnerAudioModeFuzzTest()
{
    AudioProcessConfig config;
    AudioPlaybackCaptureConfig filterConfig = AudioPlaybackCaptureConfig();
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    int32_t errorCode = provider.ConsumeIntegral<int32_t>();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    if (g_audioModes.empty() || g_streamUsages.empty()) {
        return;
    }
    config.audioMode = g_audioModes[provider.ConsumeIntegral<uint32_t>() % g_audioModes.size()];
    config.rendererInfo.streamUsage = g_streamUsages[provider.ConsumeIntegral<uint32_t>() % g_streamUsages.size()];
    audioServerPtr->CreateAudioProcessInner(config, errorCode, filterConfig);
}

#ifdef HAS_FEATURE_INNERCAPTURER
void AudioServerHandleCheckCaptureLimitFuzzTest()
{
    AudioProcessConfig config;
    AudioPlaybackCaptureConfig filterConfig = AudioPlaybackCaptureConfig();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    if (g_sourceTypes.empty()) {
        return;
    }
    config.capturerInfo.sourceType = g_sourceTypes[provider.ConsumeIntegral<int32_t>() % g_sourceTypes.size()];
    audioServerPtr->HandleCheckCaptureLimit(config, filterConfig);
}

void AudioServerInnerCheckCaptureLimitFuzzTest()
{
    AudioPlaybackCaptureConfig filterConfig = AudioPlaybackCaptureConfig();
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    int32_t innerCapId = provider.ConsumeIntegral<int32_t>();
    audioServerPtr->InnerCheckCaptureLimit(filterConfig, innerCapId);
}
#endif

void AudioServerIsNormalIpcStreamFuzzTest()
{
    AudioProcessConfig config;
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    if (g_audioModes.empty()) {
        return;
    }
    config.audioMode = g_audioModes[provider.ConsumeIntegral<uint32_t>() % g_audioModes.size()];
    audioServerPtr->IsNormalIpcStream(config);
}

void AudioServerCheckRemoteDeviceStateSwitchCaseFuzzTest()
{
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    bool isStartDevice = provider.ConsumeBool();
    int32_t deviceRole = provider.ConsumeIntegral<int32_t>();
    audioServerPtr->CheckRemoteDeviceState("LocalDevice", deviceRole, isStartDevice);
}

void AudioServerCheckInnerRecorderPermissionSourceTypeFuzzTest()
{
    AudioProcessConfig config;
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    if (g_sourceTypes.empty()) {
        return;
    }
    config.capturerInfo.sourceType = g_sourceTypes[provider.ConsumeIntegral<int32_t>() % g_sourceTypes.size()];
    audioServerPtr->CheckInnerRecorderPermission(config);
}

void AudioServerSetRenderWhitelistFuzzTest()
{
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    std::vector<std::string> list;
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    list.push_back(provider.ConsumeRandomLengthString(MAX_BUNDLE_NAME_LENGTH));
    audioServerPtr->SetRenderWhitelist(list);
}

void AudioServerCheckVoiceCallRecorderPermissionFuzzTest()
{
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);

    auto tokenID = static_cast<Security::AccessToken::AccessTokenID>(provider.ConsumeIntegral<uint64_t>());

    (void)audioServerPtr->CheckVoiceCallRecorderPermission(tokenID);
}

void AudioServerAddAndRemoveCaptureInjectorFuzzTest()
{
    sptr<AudioServer> audioServerPtr = sptr<AudioServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    CHECK_AND_RETURN(audioServerPtr != nullptr);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);

    auto sinkPortidx = provider.ConsumeIntegral<uint32_t>();
    auto rate = provider.ConsumeRandomLengthString(MAX_RANDOM_STRING_LENGTH);
    auto format = provider.ConsumeRandomLengthString(MAX_RANDOM_STRING_LENGTH);
    auto channels = provider.ConsumeRandomLengthString(MAX_RANDOM_STRING_LENGTH);
    auto bufferSize = provider.ConsumeRandomLengthString(MAX_RANDOM_STRING_LENGTH);

    (void)audioServerPtr->AddCaptureInjector(sinkPortidx, rate, format, channels, bufferSize);
    (void)audioServerPtr->RemoveCaptureInjector(sinkPortidx);
}

void DataTransferStateChangeCallbackInnerImplOnDataTransferStateChangeFuzzTest()
{
    DataTransferStateChangeCallbackInnerImpl dataTransferStateChangeCallbackInnerImpl;
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    
    auto callbackId = provider.ConsumeIntegral<int32_t>();
    auto info = ConsumeAudioRendererDataTransferStateChangeInfo(provider);

    dataTransferStateChangeCallbackInnerImpl.OnDataTransferStateChange(callbackId, info);
}

void DataTransferStateChangeCallbackInnerImplReportEventFuzzTest()
{
    DataTransferStateChangeCallbackInnerImpl dataTransferStateChangeCallbackInnerImpl;
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);

    auto info = ConsumeAudioRendererDataTransferStateChangeInfo(provider);

    dataTransferStateChangeCallbackInnerImpl.ReportEvent(info);
}

void PipeInfoGuardSetReleaseFlagFuzzTest()
{
    PipeInfoGuard pipeinfoGuard(0);
    FuzzedDataProvider provider(RAW_DATA, g_dataSize);

    pipeinfoGuard.SetReleaseFlag(provider.ConsumeIntegral<int32_t>() % NUM_2);
}

TestFuncs g_testFuncs[] = {
    AudioServerDumpTest,
    AudioServerGetUsbParameterTest,
    AudioServerOnAddSystemAbilityTest,
    AudioServerSetExtraParametersTest,
    AudioServerSetAudioParameterByKeyTest,
    AudioServerGetExtraParametersTest,
    AudioServerGetAudioParameterByIdTest,
    AudioServerIsFastBlockedTest,
    AudioServerCheckRemoteDeviceStateTestTwo,
    AudioServerCreateAudioStreamTest,
    AudioServerSetSinkRenderEmptyTest,
    AudioServerOnRenderSinkStateChangeTest,
    AudioServerCreateHdiSinkPortTest,
    AudioServerCreateHdiSourcePortTest,
    AudioServerFuzzTest,
    AudioServerOffloadSetVolumeFuzzTest,
    AudioServerNotifyStreamVolumeChangedFuzzTest,
    AudioServerResetRouteForDisconnectFuzzTest,
    AudioServerGetEffectLatencyTest,
    AudioServerGetMaxAmplitudeTest,
    AudioServerCreatePlaybackCapturerManagerTest,
    AudioServerSetOutputDeviceSinkTest,
    AudioServerSetAudioMonoStateTest,
    AudioServerSetVoiceVolumeTest,
    AudioServerCheckRemoteDeviceStateTest,
    AudioServerNotifyDeviceInfoTest,
    AudioServerGetAudioParameterTest,
    AudioServerSetAudioParameterTest,
    AudioServerSetMicrophoneMuteTest,
    AudioServerSetAudioBalanceValueTest,
    AudioServerSetAudioSceneTest,
    AudioServerUpdateLatencyTimestampTest,
    AudioServerSetOffloadModeTest,
    AudioServerUnsetOffloadTest,
    AudioServerCheckHibernateStateTest,
    AudioServerSetSessionMuteStateTest,
    AudioServerNotifyMuteStateChangeTest,
    AudioServerAudioWorkgroupCreateTest,
    AudioServerAudioWorkgroupReleaseTest,
    AudioServerAudioWorkgroupAddThreadTest,
    AudioServerAudioWorkgroupRemoveThreadTest,
    AudioServerAudioWorkgroupStartGroupTest,
    AudioServerAudioWorkgroupStopGroupTest,
    AudioServerRegisterDataTransferCallbackTest,
    AudioServerProcessKeyValuePairsTest,
    AudioServerSetA2dpAudioParameterTest,
    AudioServerGetAudioParameterByKeyTest,
    AudioServerGetDPParameterTest,
    AudioServerSetAudioSceneByDeviceTypeTest,
    AudioServerNotifyDeviceInfoFuzzTest,
    AudioServerSetVoiceVolumeFuzzTest,
    AudioServerCheckRemoteDeviceStateFuzzTest,
    AudioServerSetAudioBalanceValueFuzzTest,
    AudioServerRemoveRendererDataTransferCallbackFuzzTest,
    AudioServerRegisterDataTransferCallbackFuzzTest,
    AudioServerRegisterDataTransferMonitorParamFuzzTest,
    AudioServerUnregisterDataTransferMonitorParamFuzzTest,
    AudioServerOnDataTransferStateChangeFuzzTest,
    AudioServerSetAudioParameterFuzzTest,
    AudioServerGetTransactionIdFuzzTest,
    AudioServerSetIORoutesFuzzTest,
    AudioServerUpdateActiveDeviceRouteFuzzTest,
    AudioServerUpdateActiveDevicesRouteFuzzTest,
    AudioServerSetDmDeviceTypeFuzzTest,
    AudioServerSetAudioMonoStateFuzzTest,
    AudioServerGetHapBuildApiVersionFuzzTest,
    AudioServerSendCreateErrorInfoFuzzTest,
    AudioServerIsSatelliteFuzzTest,
    AudioServerCreateAudioProcessFuzzTest,
    AudioServerCreateAudioProcessInnerFuzzTest,
    AudioServerHandleCheckRecorderBackgroundCaptureFuzzTest,
    AudioServerSetForegroundListFuzzTest,
    AudioServerCreatePlaybackCapturerManagerFuzzTest,
    AudioServerGetMaxAmplitudeFuzzTest,
    AudioServerUpdateLatencyTimestampFuzzTest,
    AudioServerCheckHibernateStateFuzzTest,
    AudioServerCreateIpcOfflineStreamFuzzTest,
    AudioServerGetOfflineAudioEffectChainsFuzzTest,
    AudioServerGetStandbyStatusFuzzTest,
    AudioServerGenerateSessionIdFuzzTest,
    AudioServerReleaseCaptureLimitFuzzTest,
    AudioServerLoadHdiAdapterFuzzTest,
    AudioServerUnloadHdiAdapterFuzzTest,
    AudioServerCreateSinkPortFuzzTest,
    AudioServerCreateSourcePortFuzzTest,
    AudioServerDestroyHdiPortFuzzTest,
    AudioServerSetDeviceConnectedFlagFuzzTest,
    AudioServerCreateAudioWorkgroupFuzzTest,
    AudioServerReleaseAudioWorkgroupFuzzTest,
    AudioServerAddThreadToGroupFuzzTest,
    AudioServerForceStopAudioStreamFuzzTest,
    AudioServerStartGroupFuzzTest,
    AudioServerStopGroupFuzzTest,
    AudioServerSetActiveOutputDeviceFuzzTest,
    AudioServerResetRecordConfigSourceTypeFuzzTest,
    AudioServerResetProcessConfigCallerUidFuzzTest,
    AudioServerCheckStreamInfoFormatNotContainFuzzTest,
    AudioServerCheckRendererFormatNotContainFuzzTest,
    AudioServerCheckRecorderFormatNotContainFuzzTest,
    AudioServerCreateAudioProcessInnerAudioModeFuzzTest,
#ifdef HAS_FEATURE_INNERCAPTURER
    AudioServerHandleCheckCaptureLimitFuzzTest,
    AudioServerInnerCheckCaptureLimitFuzzTest,
#endif
    AudioServerIsNormalIpcStreamFuzzTest,
    AudioServerCheckRemoteDeviceStateSwitchCaseFuzzTest,
    AudioServerCheckInnerRecorderPermissionSourceTypeFuzzTest,
    AudioServerSetRenderWhitelistFuzzTest,
    AudioServerCheckVoiceCallRecorderPermissionFuzzTest,
    AudioServerAddAndRemoveCaptureInjectorFuzzTest,
    DataTransferStateChangeCallbackInnerImplOnDataTransferStateChangeFuzzTest,
    DataTransferStateChangeCallbackInnerImplReportEventFuzzTest,
    PipeInfoGuardSetReleaseFlagFuzzTest,
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

    FuzzedDataProvider provider(RAW_DATA, g_dataSize);
    uint32_t code = provider.ConsumeIntegral<uint32_t>();
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
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    OHOS::AudioStandard::FuzzTest(data, size);
    OHOS::AudioStandard::AudioServerWriteServiceStartupErrorTest();
    OHOS::AudioStandard::AudioServerRegisterDataTransferStateChangeCallbackFuzzTest();
    OHOS::AudioStandard::AudioServerInitMaxRendererStreamCntPerUidFuzzTest();
    OHOS::AudioStandard::AudioServerResetRecordConfigFuzzTest();
    OHOS::AudioStandard::AudioServerResetProcessConfigFuzzTest();
    OHOS::AudioStandard::AudioServerCheckStreamInfoFormatFuzzTest();
    OHOS::AudioStandard::AudioServerCheckRendererFormatFuzzTest();
    OHOS::AudioStandard::AudioServerCheckRecorderFormatFuzzTest();
    OHOS::AudioStandard::AudioServerCheckConfigFormatFuzzTest();
    OHOS::AudioStandard::AudioServerCheckMaxRendererInstancesFuzzTest();
    OHOS::AudioStandard::AudioServerCheckAndWaitAudioPolicyReadyFuzzTest();
    OHOS::AudioStandard::AudioServerPermissionCheckerFuzzTest();
    OHOS::AudioStandard::AudioServerCheckPlaybackPermissionFuzzTest();
    OHOS::AudioStandard::AudioServerCheckInnerRecorderPermissionFuzzTest();
    OHOS::AudioStandard::AudioServerRegisterAudioCapturerSourceCallbackFuzzTest();
    OHOS::AudioStandard::AudioServerRegisterAudioRendererSinkCallbackFuzzTest();
    OHOS::AudioStandard::AudioServerNotifyAudioPolicyReadyFuzzTest();
    OHOS::AudioStandard::AudioServerGetAllSinkInputsFuzzTest();
    OHOS::AudioStandard::AudioServerSetBtHdiInvalidStateFuzzTest();
    OHOS::AudioStandard::AudioServerGetDPParameterTest();
    return 0;
}