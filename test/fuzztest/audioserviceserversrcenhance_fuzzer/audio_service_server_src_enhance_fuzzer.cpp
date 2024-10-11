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
#include "audio_service.h"
#include "ipc_stream_stub.h"
#include "ipc_stream_in_server.h"
#include "none_mix_engine.h"
#include "ipc_stream.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
const int32_t LIMITSIZE = 4;
constexpr int32_t DEFAULT_STREAM_ID = 10;
const std::u16string FORMMGR_INTERFACE_TOKEN = u"IStandardAudioService";
const uint64_t COMMON_LENGTH_NUM = 2;
const uint32_t SOURCETYPE_ENUM_NUM = 18;
const uint32_t OPERATION_ENUM_NUM = 13;
const uint32_t NUM = 1;


static AudioProcessConfig InitProcessConfig()
{
    AudioProcessConfig config;
    config.appInfo.appUid = DEFAULT_STREAM_ID;
    config.appInfo.appPid = DEFAULT_STREAM_ID;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    return config;
}

void AudioServiceMoreFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config;
    config.appInfo.appUid = DEFAULT_STREAM_ID;
    config.appInfo.appPid = DEFAULT_STREAM_ID;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_RECORD;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;

    AudioService *audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, audioServicePtr);
    std::shared_ptr<AudioProcessInServer> audioProcessInServer =
        std::make_shared<AudioProcessInServer>(config, audioServicePtr);
    audioProcessInServer->Release(true);
    audioProcessInServer->Release(false);

    uint32_t sessionId = *reinterpret_cast<const uint32_t*>(rawData);
    AudioPlaybackCaptureConfig newConfig;
    audioServicePtr->UpdateMuteControlSet(sessionId, true);
    audioServicePtr->UpdateMuteControlSet(sessionId, false);
    audioServicePtr->EnableDualToneList(sessionId);
    audioServicePtr->OnCapturerFilterChange(sessionId, newConfig);
    audioServicePtr->OnCapturerFilterRemove(sessionId);

    int32_t ret = 0;
    audioServicePtr->workingInnerCapId_ = *reinterpret_cast<const uint32_t*>(rawData);
    audioServicePtr->GetIpcStream(config, ret);
    audioServicePtr->ShouldBeInnerCap(config);
    audioServicePtr->ShouldBeDualTone(config);

    audioServicePtr->OnInitInnerCapList();
    audioServicePtr->OnUpdateInnerCapList();
    audioServicePtr->ResetAudioEndpoint();

    uint32_t sourceType_int = *reinterpret_cast<const uint32_t*>(rawData);
    sourceType_int = (sourceType_int % SOURCETYPE_ENUM_NUM) - NUM;
    SourceType sourceType = static_cast<SourceType>(sourceType_int);
    audioServicePtr->UpdateSourceType(sourceType);
}

void AudioCapturerInServerMoreFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    std::weak_ptr<IStreamListener> innerListener;
    std::shared_ptr<CapturerInServer> capturerInServer = std::make_shared<CapturerInServer>(config, innerListener);
    if (capturerInServer == nullptr) {
        return;
    }

    uint32_t operation_int = *reinterpret_cast<const uint32_t*>(rawData);
    operation_int = (operation_int % OPERATION_ENUM_NUM) - NUM;
    IOperation operation = static_cast<IOperation>(operation_int);
    capturerInServer->OnStatusUpdate(operation);

    size_t length = COMMON_LENGTH_NUM;
    capturerInServer->ReadData(length);

    AudioPlaybackCaptureConfig captureconfig;
    capturerInServer->UpdatePlaybackCaptureConfig(captureconfig);
    capturerInServer->SetNonInterruptMute(true);
    capturerInServer->RestoreSession();
}

void AudioNoneMixEngineMoreFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<NoneMixEngine> noneMixEngine = std::make_shared<NoneMixEngine>();
    noneMixEngine->isInit_ = true;
    DeviceInfo type;
    type.deviceType = DEVICE_TYPE_USB_HEADSET;
    noneMixEngine->Init(type, true);
    noneMixEngine->Start();

    noneMixEngine->isStart_ = true;
    noneMixEngine->Stop();
    noneMixEngine->PauseAsync();

    noneMixEngine->MixStreams();
    noneMixEngine->IsPlaybackEngineRunning();
    noneMixEngine->StandbySleep();
    noneMixEngine->GetDirectVoipSampleRate(AudioSamplingRate::SAMPLE_RATE_16000);
}

void AudioIpcStreamStubListenerFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(IpcStream::GetDescriptor());
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnRemoteRequest(IpcStream::ON_REGISTER_STREAM_LISTENER, data, reply, option);
}

void AudioIpcStreamStubBufferFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(IpcStream::GetDescriptor());
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnRemoteRequest(IpcStream::ON_RESOLVE_BUFFER, data, reply, option);
}

void AudioIpcStreamStubPositionFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(IpcStream::GetDescriptor());
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnRemoteRequest(IpcStream::ON_UPDATE_POSITION, data, reply, option);
}

void AudioIpcStreamStubSessionidFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(IpcStream::GetDescriptor());
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnRemoteRequest(IpcStream::ON_GET_AUDIO_SESSIONID, data, reply, option);
}

void AudioIpcStreamStubStartFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(IpcStream::GetDescriptor());
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnRemoteRequest(IpcStream::ON_START, data, reply, option);
}

void AudioIpcStreamStubPauseFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(IpcStream::GetDescriptor());
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnRemoteRequest(IpcStream::ON_PAUSE, data, reply, option);
}

void AudioIpcStreamStubStopFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(IpcStream::GetDescriptor());
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnRemoteRequest(IpcStream::ON_STOP, data, reply, option);
}

void AudioIpcStreamStubReleaseFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(IpcStream::GetDescriptor());
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnRemoteRequest(IpcStream::ON_RELEASE, data, reply, option);
}

void AudioIpcStreamStubFlushFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(IpcStream::GetDescriptor());
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnRemoteRequest(IpcStream::ON_FLUSH, data, reply, option);
}

void AudioIpcStreamStubDrainFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(IpcStream::GetDescriptor());
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnRemoteRequest(IpcStream::ON_DRAIN, data, reply, option);
}

void AudioIpcStreamStubConfigFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(IpcStream::GetDescriptor());
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnRemoteRequest(IpcStream::ON_UPDATA_PLAYBACK_CAPTURER_CONFIG, data, reply, option);
}

void AudioIpcStreamStubTimeFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(IpcStream::GetDescriptor());
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnRemoteRequest(IpcStream::OH_GET_AUDIO_TIME, data, reply, option);
}

void AudioIpcStreamStubAudioPositionFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(IpcStream::GetDescriptor());
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnRemoteRequest(IpcStream::OH_GET_AUDIO_POSITION, data, reply, option);
}

void AudioIpcStreamStubLatencyFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(IpcStream::GetDescriptor());
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnRemoteRequest(IpcStream::ON_GET_LATENCY, data, reply, option);
}

void AudioIpcStreamStubRateFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(IpcStream::GetDescriptor());
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnRemoteRequest(IpcStream::ON_SET_RATE, data, reply, option);
}

void AudioIpcStreamStubGetRateFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnMiddleCodeRemoteRequest(IpcStream::ON_GET_RATE, data, reply, option);
}

void AudioIpcStreamStubSetVolumeFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnMiddleCodeRemoteRequest(IpcStream::ON_SET_LOWPOWER_VOLUME, data, reply, option);
}

void AudioIpcStreamStubGetVolumeFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnMiddleCodeRemoteRequest(IpcStream::ON_GET_LOWPOWER_VOLUME, data, reply, option);
}

void AudioIpcStreamStubSetModeFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnMiddleCodeRemoteRequest(IpcStream::ON_SET_EFFECT_MODE, data, reply, option);
}

void AudioIpcStreamStubGetModeFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnMiddleCodeRemoteRequest(IpcStream::ON_GET_EFFECT_MODE, data, reply, option);
}

void AudioIpcStreamStubSetTypeFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnMiddleCodeRemoteRequest(IpcStream::ON_SET_PRIVACY_TYPE, data, reply, option);
}

void AudioIpcStreamStubGetTypeFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnMiddleCodeRemoteRequest(IpcStream::ON_GET_PRIVACY_TYPE, data, reply, option);
}

void AudioIpcStreamStubSetOffloadFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnMiddleCodeRemoteRequest(IpcStream::ON_SET_OFFLOAD_MODE, data, reply, option);
}

void AudioIpcStreamStubUnsetOffloadFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnMiddleCodeRemoteRequest(IpcStream::ON_UNSET_OFFLOAD_MODE, data, reply, option);
}

void AudioIpcStreamStubGetCacheFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnMiddleCodeRemoteRequest(IpcStream::ON_GET_OFFLOAD_APPROXIMATELY_CACHE_TIME, data, reply, option);
}

void AudioIpcStreamStubSetOffloadVolFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnMiddleCodeRemoteRequest(IpcStream::ON_SET_OFFLOAD_VOLUME, data, reply, option);
}

void AudioIpcStreamStubUpdateStateFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnMiddleCodeRemoteRequest(IpcStream::ON_UPDATE_SPATIALIZATION_STATE, data, reply, option);
}

void AudioIpcStreamStubGetManagerFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnMiddleCodeRemoteRequest(IpcStream::ON_GET_STREAM_MANAGER_TYPE, data, reply, option);
}

void AudioIpcStreamStubSetOthersFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnMiddleCodeRemoteRequest(IpcStream::ON_SET_SILENT_MODE_AND_MIX_WITH_OTHERS, data, reply, option);
}

void AudioIpcStreamStubSetClientFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnMiddleCodeRemoteRequest(IpcStream::ON_SET_CLIENT_VOLUME, data, reply, option);
}

void AudioIpcStreamStubSetMuteFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnMiddleCodeRemoteRequest(IpcStream::ON_SET_MUTE, data, reply, option);
}

void AudioIpcStreamStubRegisterFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStream = IpcStreamInServer::Create(config, ret);
    if (ipcStream == nullptr) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    ipcStream->OnMiddleCodeRemoteRequest(IpcStream::ON_REGISTER_THREAD_PRIORITY, data, reply, option);
}

} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *rawData, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::AudioServiceMoreFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioCapturerInServerMoreFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioNoneMixEngineMoreFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubListenerFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubBufferFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubPositionFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubSessionidFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubStartFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubPauseFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubStopFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubReleaseFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubFlushFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubDrainFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubConfigFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubTimeFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubAudioPositionFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubLatencyFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubRateFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubGetRateFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubSetVolumeFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubGetVolumeFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubSetModeFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubGetModeFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubSetTypeFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubGetTypeFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubSetOffloadFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubUnsetOffloadFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubGetCacheFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubSetOffloadVolFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubUpdateStateFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubGetManagerFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubSetOthersFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubSetClientFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubSetMuteFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioIpcStreamStubRegisterFuzzTest(rawData, size);
    return 0;
}