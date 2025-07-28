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

#include <iostream>
#include <cstddef>
#include <cstdint>

#include "accesstoken_kit.h"
#include "message_parcel.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
#include "securec.h"
#include "parameter.h"

#include "audio_info.h"
#include "audio_inner_call.h"
#include "audio_server.h"
#include "audio_service.h"
#include "audio_service_types.h"
#include "audio_param_parser.h"
#include "audio_process_config.h"
#include "audio_utils.h"
#include "audio_stream_info.h"
#include "policy_provider_stub.h"

namespace OHOS {
namespace AudioStandard {
const std::u16string FORMMGR_INTERFACE_TOKEN = u"IStandardAudioService";
const int32_t SYSTEM_ABILITY_ID = 3001;
const int32_t POLICY_SYSTEM_ABILITY_ID = 3009;
const int32_t NUM_2 = 2;
const uint32_t LIMIT_TWO = 2;
const uint32_t FUZZ_TEST_UID = 10000; // for test
const uint32_t STD_OUT_FD = 1;
const bool RUN_ON_CREATE = false;

bool g_hasServerInit = false;
bool g_dumpPlayback = false;
bool g_dumpCapturer = false;

const uint8_t *g_baseFuzzData = nullptr;
size_t g_baseFuzzSize = 0;
size_t g_baseFuzzPos;

static AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_INVALID, SAMPLE_S24LE, MONO,
    AudioChannelLayout::CH_LAYOUT_UNKNOWN);

template <class T> T GetData()
{
    T object{};
    size_t objectSize = sizeof(object);
    if (g_baseFuzzData == nullptr || objectSize > g_baseFuzzSize - g_baseFuzzPos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, g_baseFuzzData + g_baseFuzzPos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_baseFuzzPos += objectSize;
    return object;
}

class MockPolicyProvider : public IPolicyProvider {
public:
    MockPolicyProvider() {};
    ~MockPolicyProvider() {};

    int32_t GetProcessDeviceInfo(const AudioProcessConfig &config, bool lockFlag,
        AudioDeviceDescriptor &deviceInfo) override;

    int32_t InitSharedVolume(std::shared_ptr<AudioSharedMemory> &buffer) override;

    int32_t NotifyCapturerAdded(AudioCapturerInfo capturerInfo, AudioStreamInfo streamInfo,
        uint32_t sessionId) override;

    int32_t NotifyWakeUpCapturerRemoved() override;

    bool IsAbsVolumeSupported() override;

    int32_t OffloadGetRenderPosition(uint32_t &delayValue, uint64_t &sendDataSize, uint32_t &timeStamp) override;

    int32_t NearlinkGetRenderPosition(uint32_t &delayValue) override;

    int32_t GetAndSaveClientType(uint32_t uid, const std::string &bundleName) override;

    int32_t GetMaxRendererInstances() override;

    int32_t NotifyCapturerRemoved(uint64_t sessionId) override;

    int32_t ClearAudioFocusBySessionID(const int32_t &sessionID) override;

    int32_t CaptureConcurrentCheck(const uint32_t &sessionID) override;

#ifdef HAS_FEATURE_INNERCAPTURER
    int32_t LoadModernInnerCapSink(int32_t innerCapId) override;

    int32_t UnloadModernInnerCapSink(int32_t innerCapId) override;
#endif

    std::shared_ptr<AudioSharedMemory> policyVolumeMap_ = nullptr;
};

int32_t MockPolicyProvider::GetProcessDeviceInfo(const AudioProcessConfig &config, bool lockFlag,
    AudioDeviceDescriptor &deviceInfo)
{
    if (config.audioMode == AUDIO_MODE_PLAYBACK) {
        deviceInfo.deviceRole_ = OUTPUT_DEVICE;
        deviceInfo.deviceType_ = DEVICE_TYPE_SPEAKER;
    } else {
        deviceInfo.deviceRole_ = INPUT_DEVICE;
        deviceInfo.deviceType_ = DEVICE_TYPE_MIC;
    }
    deviceInfo.deviceId_ = 0;
    deviceInfo.networkId_ = "LocalDevice";
    deviceInfo.deviceName_ = "testname";

    deviceInfo.audioStreamInfo_ = {{SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, CH_LAYOUT_STEREO}};
    return SUCCESS;
}

int32_t MockPolicyProvider::InitSharedVolume(std::shared_ptr<AudioSharedMemory> &buffer)
{
    size_t mapSize = IPolicyProvider::GetVolumeVectorSize() * sizeof(Volume);
    policyVolumeMap_ = AudioSharedMemory::CreateFormLocal(mapSize, "MockVolumeMap");
    buffer = policyVolumeMap_;
    return SUCCESS;
}

int32_t MockPolicyProvider::NotifyCapturerAdded(AudioCapturerInfo capturerInfo, AudioStreamInfo streamInfo,
    uint32_t sessionId)
{
    return SUCCESS;
}

int32_t MockPolicyProvider::NotifyWakeUpCapturerRemoved()
{
    return SUCCESS;
}

bool MockPolicyProvider::IsAbsVolumeSupported()
{
    return SUCCESS;
}

int32_t MockPolicyProvider::OffloadGetRenderPosition(uint32_t &delayValue, uint64_t &sendDataSize, uint32_t &timeStamp)
{
    return SUCCESS;
}

int32_t MockPolicyProvider::NearlinkGetRenderPosition(uint32_t &delayValue)
{
    return SUCCESS;
}

int32_t MockPolicyProvider::GetAndSaveClientType(uint32_t uid, const std::string &bundleName)
{
    return SUCCESS;
}

int32_t MockPolicyProvider::GetMaxRendererInstances()
{
    return SUCCESS;
}

int32_t MockPolicyProvider::NotifyCapturerRemoved(uint64_t sessionId)
{
    return SUCCESS;
}

int32_t MockPolicyProvider::ClearAudioFocusBySessionID(const int32_t &sessionID)
{
    return SUCCESS;
}

int32_t MockPolicyProvider::CaptureConcurrentCheck(const uint32_t &sessionID)
{
    return SUCCESS;
}

#ifdef HAS_FEATURE_INNERCAPTURER
int32_t MockPolicyProvider::LoadModernInnerCapSink(int32_t innerCapId)
{
    return SUCCESS;
}

int32_t MockPolicyProvider::UnloadModernInnerCapSink(int32_t innerCapId)
{
    return SUCCESS;
}
#endif

void AudioFuzzTestGetPermission()
{
    uint64_t tokenId;
    constexpr int perNum = 10;
    const char *perms[perNum] = {
        "ohos.permission.MICROPHONE",
        "ohos.permission.RECORD_VOICE_CALL",
        "ohos.permission.CAST_AUDIO_OUTPUT",
        "ohos.permission.MANAGE_INTELLIGENT_VOICE",
        "ohos.permission.MANAGE_AUDIO_CONFIG",
        "ohos.permission.MICROPHONE_CONTROL",
        "ohos.permission.MODIFY_AUDIO_SETTINGS",
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
}

AudioServer *GetServerPtr()
{
    static AudioServer server(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    if (!g_hasServerInit) {
        g_hasServerInit = true;
        server.OnAddSystemAbility(POLICY_SYSTEM_ABILITY_ID, "");
        server.RegisterAudioCapturerSourceCallback();
        std::unique_ptr<AudioParamParser> audioParamParser = std::make_unique<AudioParamParser>();
        if (audioParamParser == nullptr) {
            server.WriteServiceStartupError();
        }
        if (audioParamParser->LoadConfiguration(AudioServer::audioParameterKeys)) {
            AUDIO_INFO_LOG("Audio extra parameters load configuration successfully.");
        }

        std::vector<StringPair> kvpairs = {
            {"key1", "value1"},
            {"key2", "value2"},
            {"key3", "value3"}
        };
        server.SetExtraParameters("PCM_DUMP", kvpairs);
        server.SetExtraParameters("test", kvpairs);

        AudioInnerCall::GetInstance()->RegisterAudioServer(&server);

        server.GetHapBuildApiVersion(0);
    }
    return &server;
}

void InitAudioServer()
{
    static MockPolicyProvider mockProvider;
    sptr<PolicyProviderWrapper> wrapper = new(std::nothrow) PolicyProviderWrapper(&mockProvider);

    // call GetServerPtr()->RegiestPolicyProvider will enable capturer

    std::shared_ptr<AudioSharedMemory> buffer;
    wrapper->InitSharedVolume(buffer);
    bool ret = false;
    AudioProcessConfig config;
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    wrapper->GetProcessDeviceInfo(config, true, deviceInfo);
    wrapper->NotifyCapturerAdded(config.capturerInfo, config.streamInfo, 0);
    wrapper->NotifyWakeUpCapturerRemoved();
    wrapper->IsAbsVolumeSupported(ret);
}

void ModifyStreamInfoFormat(AudioProcessConfig &config)
{
    if (config.streamInfo.samplingRate > SAMPLE_RATE_48000) {
        config.streamInfo.samplingRate = SAMPLE_RATE_96000;
    } else {
        config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    }

    config.streamInfo.format = static_cast<AudioSampleFormat>(config.streamInfo.format % (SAMPLE_F32LE + 1));

    config.streamInfo.encoding = static_cast<AudioEncodingType>(config.streamInfo.encoding % LIMIT_TWO);

    config.streamInfo.channelLayout = CH_LAYOUT_STEREO;

    if (config.audioMode == AUDIO_MODE_PLAYBACK) {
        config.streamInfo.channels = static_cast<AudioChannel>(config.streamInfo.channels % (CHANNEL_16 + 1));
    }

    if (config.audioMode == AUDIO_MODE_RECORD) {
        config.streamInfo.channels = static_cast<AudioChannel>(config.streamInfo.channels % (CHANNEL_6 + 1));
    }
}

void ModifyRendererConfig(AudioProcessConfig &config)
{
    config.rendererInfo.streamUsage = static_cast<StreamUsage>(config.rendererInfo.streamUsage %
        (STREAM_USAGE_MAX + 1));

    config.rendererInfo.rendererFlags = config.rendererInfo.rendererFlags % (AUDIO_FLAG_VOIP_DIRECT + 1);

    config.rendererInfo.pipeType = static_cast<AudioPipeType>(config.rendererInfo.pipeType %
        (PIPE_TYPE_DIRECT_VOIP + 1));
}

void ModifyRecorderConfig(AudioProcessConfig &config)
{
    config.capturerInfo.sourceType = static_cast<SourceType>(config.capturerInfo.sourceType % (SOURCE_TYPE_MAX + 1));

    config.capturerInfo.capturerFlags = config.capturerInfo.capturerFlags % (AUDIO_FLAG_VOIP_DIRECT + 1);

    config.capturerInfo.pipeType = static_cast<AudioPipeType>(config.capturerInfo.pipeType %
        (PIPE_TYPE_DIRECT_VOIP + 1));
}

void ModifyProcessConfig(AudioProcessConfig &config)
{
    config.audioMode = static_cast<AudioMode>(config.audioMode % LIMIT_TWO);
    ModifyStreamInfoFormat(config);

    if (config.audioMode == AUDIO_MODE_PLAYBACK) {
        ModifyRendererConfig(config);
    }

    if (config.audioMode == AUDIO_MODE_RECORD) {
        ModifyRecorderConfig(config);
    }
}

void CallStreamFuncs(sptr<IpcStreamInServer> ipcStream)
{
    if (ipcStream == nullptr) {
        return;
    }

    std::shared_ptr<OHAudioBuffer> buffer = nullptr;
    ipcStream->ResolveBuffer(buffer);
    ipcStream->UpdatePosition();

    std::string name = "fuzz_test";
    ipcStream->RegisterThreadPriority(0, name, METHOD_START);
    bool ret = false;
    uint32_t sessionId = 0;
    ipcStream->GetAudioSessionID(sessionId);
    ipcStream->Start();
    ipcStream->Pause();
    ipcStream->Drain(ret);
    AudioPlaybackCaptureConfig config = {{{STREAM_USAGE_MUSIC}, FilterMode::INCLUDE, {0}, FilterMode::INCLUDE}, false};
    ipcStream->UpdatePlaybackCaptureConfig(config);
    uint64_t framePos = 0;
    uint64_t timestamp = 0;
    uint64_t latency = 0;
    int32_t base = GetData<int32_t>();
    if (ipcStream->rendererInServer_ != nullptr) {
        ipcStream->Flush();
        ipcStream->GetAudioTime(framePos, timestamp);
        ipcStream->GetAudioPosition(framePos, timestamp, latency, base);
        ipcStream->GetLatency(timestamp);
    }
    int32_t param = 0;
    ipcStream->SetRate(param);
    ipcStream->GetRate(param);
    float volume = 0.0f;
    ipcStream->SetLowPowerVolume(volume);
    ipcStream->GetLowPowerVolume(volume);
    ipcStream->SetAudioEffectMode(param);
    ipcStream->GetAudioEffectMode(param);
    ipcStream->SetPrivacyType(param);
    ipcStream->GetPrivacyType(param);
    ipcStream->SetOffloadMode(param, false);
    ipcStream->UnsetOffloadMode();
    ipcStream->GetOffloadApproximatelyCacheTime(framePos, timestamp, timestamp, timestamp);
    ipcStream->UpdateSpatializationState(true, false);
    ipcStream->GetStreamManagerType();
    ipcStream->SetSilentModeAndMixWithOthers(false);
    ipcStream->SetClientVolume();
    ipcStream->SetMute(false);
    ipcStream->SetDuckFactor(volume);
    ipcStream->Stop();
    ipcStream->Release(false);
}

void DoStreamFuzzTest(const AudioProcessConfig &config, const uint8_t *rawData, size_t size)
{
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = AudioService::GetInstance()->GetIpcStream(config, ret);
    if (ipcStream == nullptr || rawData == nullptr || size < sizeof(uint32_t)) {
        return;
    }

    uint32_t code = 20;
    rawData = rawData + sizeof(uint32_t);
    size = size - sizeof(uint32_t);

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    ipcStream->OnRemoteRequest(code, data, reply, option);

    if (config.audioMode == AUDIO_MODE_PLAYBACK && !g_dumpPlayback) {
        g_dumpPlayback = true;
        std::vector<std::u16string> args = {};
        GetServerPtr()->Dump(STD_OUT_FD, args);
    }

    if (config.audioMode == AUDIO_MODE_RECORD && !g_dumpCapturer) {
        g_dumpCapturer = true;
        std::vector<std::u16string> args = {};
        GetServerPtr()->Dump(STD_OUT_FD, args);
    }

    CallStreamFuncs(ipcStream);
}

void AudioServerFuzzTest(const uint8_t *rawData, size_t size)
{
    g_baseFuzzData = rawData;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    if (size < sizeof(AudioProcessConfig)) {
        return;
    }

    Parcel parcel;
    AudioProcessConfig config = {};
    config.callerUid = GetData<int32_t>();
    config.appInfo = GetData<AppInfo>();

    config.streamInfo = testStreamInfo;
    config.rendererInfo.rendererFlags = GetData<int32_t>();

    config.rendererInfo.sceneType = ""; // in plan

    config.rendererInfo.originalFlag = GetData<int32_t>();
    config.rendererInfo.pipeType = PIPE_TYPE_DIRECT_MUSIC;
    config.rendererInfo.contentType = static_cast<ContentType>(parcel.ReadInt32());
    config.rendererInfo.streamUsage = static_cast<StreamUsage>(parcel.ReadInt32());
    config.rendererInfo.samplingRate = static_cast<AudioSamplingRate>(parcel.ReadInt32());
    config.rendererInfo.format = static_cast<AudioSampleFormat>(parcel.ReadInt32());

    config.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    config.capturerInfo.capturerFlags = GetData<int32_t>();
    config.capturerInfo.pipeType = PIPE_TYPE_CALL_IN;
    config.capturerInfo.samplingRate = static_cast<AudioSamplingRate>(parcel.ReadInt32());
    config.capturerInfo.encodingType = GetData<uint8_t>();
    config.capturerInfo.channelLayout = GetData<uint64_t>();
    config.capturerInfo.sceneType = ""; // in plan
    config.capturerInfo.originalFlag = GetData<int32_t>();

    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    config.privacyType = static_cast<AudioPrivacyType>(GetData<uint32_t>() % NUM_2);
    config.innerCapMode = InnerCapMode::LEGACY_MUTE_CAP;

    ModifyProcessConfig(config);

    int32_t errorCode = 0;
    AudioPlaybackCaptureConfig filterConfig = AudioPlaybackCaptureConfig();
    sptr<IRemoteObject> ret = nullptr;
    CHECK_AND_RETURN(GetServerPtr() != nullptr);
    GetServerPtr()->CreateAudioProcess(config, errorCode, filterConfig, ret);
    if (ret != nullptr) {
        DoStreamFuzzTest(config, rawData, size);
    }
    if (config.appInfo.appUid == 0) {
        config.appInfo.appUid = FUZZ_TEST_UID; // to skip root pass
    }
    GetServerPtr()->CheckRecorderPermission(config);
}
} // namespace AudioStandard
} // namesapce OHOS

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    OHOS::AudioStandard::AudioFuzzTestGetPermission();
    SetParameter("persist.multimedia.audioflag.fast.disableseparate", "1");
    OHOS::AudioStandard::InitAudioServer();
    return 0;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::AudioServerFuzzTest(data, size);
    return 0;
}
