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
#include "audio_server.h"
#include "audio_service.h"
#include "audio_process_config.h"
#include "audio_utils.h"
#include "audio_stream_info.h"

namespace OHOS {
namespace AudioStandard {
const std::u16string FORMMGR_INTERFACE_TOKEN = u"IStandardAudioService";
const int32_t SYSTEM_ABILITY_ID = 3001;
const int32_t POLICY_SYSTEM_ABILITY_ID = 3009;
const uint32_t LIMIT_TWO = 2;
const bool RUN_ON_CREATE = false;

bool g_hasServerInit = false;

const uint8_t *g_baseFuzzData = nullptr;
size_t g_baseFuzzSize = 0;
size_t g_baseFuzzPos;

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

AudioServer* GetServerPtr()
{
    static AudioServer server(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    if (!g_hasServerInit) {
        server.OnAddSystemAbility(POLICY_SYSTEM_ABILITY_ID, "");
        g_hasServerInit = true;
    }
    return &server;
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

    config.capturerInfo.capturerFlags = config.rendererInfo.rendererFlags % (AUDIO_FLAG_VOIP_DIRECT + 1);

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
    ipcStream->UpdatePosition();
    uint32_t sessionId = 0;
    ipcStream->GetAudioSessionID(sessionId);
    ipcStream->Start();
    ipcStream->Pause();
    ipcStream->Stop();
    ipcStream->Release();
    ipcStream->Flush();
    ipcStream->Drain();
    AudioPlaybackCaptureConfig config = {{{STREAM_USAGE_MUSIC}, FilterMode::INCLUDE, {0}, FilterMode::INCLUDE}, false};
    ipcStream->UpdatePlaybackCaptureConfig(config);
    uint64_t framePos = 0;
    uint64_t timestamp = 0;
    uint64_t latency = 0;
    ipcStream->GetAudioTime(framePos, timestamp);
    ipcStream->GetAudioPosition(framePos, timestamp, latency);
    ipcStream->GetLatency(timestamp);
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
    std::string name = "fuzz_test";
    ipcStream->RegisterThreadPriority(0, name);
}

void DoStreamFuzzTest(const AudioProcessConfig &config, const uint8_t *rawData, size_t size)
{
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = AudioService::GetInstance()->GetIpcStream(config, ret);
    if (ipcStream == nullptr || rawData == nullptr || size < sizeof(uint32_t)) {
        return;
    }

    g_baseFuzzData = rawData;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    uint32_t code = GetData<uint32_t>() % (IpcStream::IpcStreamMsg::IPC_STREAM_MAX_MSG);

    rawData = rawData + sizeof(uint32_t);
    size = size - sizeof(uint32_t);

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);

    MessageParcel reply;
    MessageOption option;

    ipcStream->OnRemoteRequest(code, data, reply, option);

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

    AudioProcessConfig config = {};
    config.callerUid = GetData<int32_t>();
    config.appInfo = GetData<AppInfo>();
    config.streamInfo = GetData<AudioStreamInfo>();
    config.audioMode = GetData<AudioMode>();

    config.rendererInfo.contentType = GetData<ContentType>();
    config.rendererInfo.streamUsage = GetData<StreamUsage>();
    config.rendererInfo.rendererFlags = GetData<int32_t>();

    config.rendererInfo.sceneType = ""; // in plan

    config.rendererInfo.originalFlag = GetData<int32_t>();
    config.rendererInfo.pipeType = GetData<AudioPipeType>();
    config.rendererInfo.samplingRate = GetData<AudioSamplingRate>();
    config.rendererInfo.format = GetData<AudioSampleFormat>();

    config.capturerInfo.sourceType = GetData<SourceType>();
    config.capturerInfo.capturerFlags = GetData<int32_t>();
    config.capturerInfo.pipeType = GetData<AudioPipeType>();
    config.capturerInfo.samplingRate = GetData<AudioSamplingRate>();
    config.capturerInfo.encodingType = GetData<uint8_t>();
    config.capturerInfo.channelLayout = GetData<uint64_t>();
    config.capturerInfo.sceneType = ""; // in plan
    config.capturerInfo.originalFlag = GetData<int32_t>();

    config.streamType = GetData<AudioStreamType>();
    config.deviceType = GetData<DeviceType>();
    config.privacyType = GetData<AudioPrivacyType>();
    config.innerCapMode = GetData<InnerCapMode>();

    ModifyProcessConfig(config);

    int32_t errorCode = 0;
    auto remoteObj = GetServerPtr()->CreateAudioProcess(config, errorCode);
    if (remoteObj != nullptr) {
        DoStreamFuzzTest(config, rawData, size);
    }
}
} // namespace AudioStandard
} // namesapce OHOS

extern "C" int LLVMFuzzerInitialize(const uint8_t *data, size_t size)
{
    OHOS::AudioStandard::AudioFuzzTestGetPermission();
    SetParameter("persist.multimedia.audioflag.fast.disableseparate", "1");
    return 0;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::AudioServerFuzzTest(data, size);
    return 0;
}
