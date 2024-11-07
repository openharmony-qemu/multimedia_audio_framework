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

#include "i_audio_renderer_sink.h"
#include "audio_manager_base.h"
#include "audio_policy_manager_listener_stub.h"
#include "audio_server.h"
#include "audio_service.h"
#include "audio_endpoint.h"
#include "access_token.h"
#include "message_parcel.h"
#include "audio_process_in_client.h"
#include "audio_process_in_server.h"
#include "audio_param_parser.h"
#include "none_mix_engine.h"
#include "pro_renderer_stream_impl.h"
#include "oh_audio_buffer.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
const int32_t LIMITSIZE = 4;
constexpr int32_t DEFAULT_STREAM_ID = 10;
static std::unique_ptr<NoneMixEngine> playbackEngine_ = nullptr;

void DeviceFuzzTestSetUp()
{
    if (playbackEngine_ != nullptr) {
        return;
    }
    DeviceInfo deviceInfo;
    deviceInfo.deviceType = DEVICE_TYPE_USB_HEADSET;
    playbackEngine_ = std::make_unique<NoneMixEngine>();
    playbackEngine_->Init(deviceInfo, false);
}

static AudioProcessConfig InitProcessConfig()
{
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
    return config;
}

void DirectAudioPlayBackEngineStateFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    rendererStream->InitParams();
    rendererStream->SetStreamIndex(DEFAULT_STREAM_ID);
    rendererStream->Start();
    rendererStream->Pause();
    rendererStream->Flush();
    rendererStream->Stop();
    rendererStream->Release();
}

void NoneMixEngineStartFuzzTest()
{
    playbackEngine_->Start();
}

void NoneMixEngineStopFuzzTest()
{
    playbackEngine_->Stop();
}

void NoneMixEnginePauseFuzzTest()
{
    playbackEngine_->Pause();
}

void NoneMixEngineFlushFuzzTest()
{
    playbackEngine_->Flush();
}

void NoneMixEngineAddRendererFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    rendererStream->InitParams();
    rendererStream->SetStreamIndex(DEFAULT_STREAM_ID);
    rendererStream->Start();
    playbackEngine_->AddRenderer(rendererStream);
}

void NoneMixEngineRemoveRendererFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    rendererStream->InitParams();
    rendererStream->SetStreamIndex(DEFAULT_STREAM_ID);
    rendererStream->Start();
    playbackEngine_->AddRenderer(rendererStream);
    playbackEngine_->RemoveRenderer(rendererStream);
}


void AudioEndPointSeparateStartDeviceFuzzTest(const uint8_t* rawData, size_t size,
    std::shared_ptr<AudioEndpointSeparate> audioEndpoint)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    audioEndpoint->StartDevice();
    audioEndpoint->GetEndpointName();
    audioEndpoint->ShouldInnerCap();
    audioEndpoint->EnableFastInnerCap();
    audioEndpoint->DisableFastInnerCap();
    return;
}

void AudioEndPointSeparateConfigFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    DeviceInfo deviceInfo;
    deviceInfo.deviceType = DEVICE_TYPE_USB_HEADSET;
    deviceInfo.networkId = LOCAL_NETWORK_ID;
    DeviceStreamInfo audioStreamInfo = {
        SAMPLE_RATE_48000,
        ENCODING_PCM,
        SAMPLE_S16LE,
        STEREO
    };
    deviceInfo.audioStreamInfo = audioStreamInfo;
    std::shared_ptr<AudioEndpointSeparate> audioEndpoint = nullptr;
    uint64_t id = 2;
    audioEndpoint =
        std::make_shared<AudioEndpointSeparate>(AudioEndpoint::EndpointType::TYPE_INDEPENDENT, id, STREAM_DEFAULT);
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    audioEndpoint->Config(deviceInfo);

    AudioEndPointSeparateStartDeviceFuzzTest(rawData, size, audioEndpoint);
    AudioStreamType streamType = AudioStreamType::STREAM_MUSIC;
    float volume = *reinterpret_cast<const float*>(rawData);
    audioEndpoint->SetVolume(streamType, volume);
    uint32_t spanSizeInFrame = *reinterpret_cast<const uint32_t*>(rawData);
    uint32_t totalSizeInFrame = spanSizeInFrame;
    uint32_t byteSizePerFrame = *reinterpret_cast<const uint32_t*>(rawData);
    std::shared_ptr<OHAudioBuffer> oHAudioBuffer =
        OHAudioBuffer::CreateFromLocal(totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);
    audioEndpoint->ResolveBuffer(oHAudioBuffer);
    audioEndpoint->GetBuffer();
    audioEndpoint->GetStatus();
    audioEndpoint->OnStart(processStream);
    audioEndpoint->OnPause(processStream);
    audioEndpoint->OnUpdateHandleInfo(processStream);
    audioEndpoint->LinkProcessStream(processStream);
    audioEndpoint->UnlinkProcessStream(processStream);
    audioEndpoint->GetPreferBufferInfo(totalSizeInFrame, spanSizeInFrame);
    std::string dumpString = "";
    audioEndpoint->Dump(dumpString);
    audioEndpoint->GetEndpointType();
    audioEndpoint->GetDeviceInfo();
    audioEndpoint->GetDeviceRole();
    audioEndpoint->GetMaxAmplitude();
    audioEndpoint->Release();
    audioEndpoint->StopDevice();
    return;
}

} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::DeviceFuzzTestSetUp();
    OHOS::AudioStandard::DirectAudioPlayBackEngineStateFuzzTest();
    OHOS::AudioStandard::NoneMixEngineStartFuzzTest();
    OHOS::AudioStandard::NoneMixEngineStopFuzzTest();
    OHOS::AudioStandard::NoneMixEnginePauseFuzzTest();
    OHOS::AudioStandard::NoneMixEngineFlushFuzzTest();
    OHOS::AudioStandard::NoneMixEngineAddRendererFuzzTest(data, size);
    OHOS::AudioStandard::NoneMixEngineRemoveRendererFuzzTest(data, size);
    OHOS::AudioStandard::AudioEndPointSeparateConfigFuzzTest(data, size);
    return 0;
}
