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

#include "audio_manager_base.h"
#include "audio_policy_manager_listener_stub.h"
#include "audio_server.h"
#include "audio_service.h"
#include "sink/i_audio_render_sink.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
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
constexpr int32_t DEFAULT_STREAM_ID = 10;
static std::unique_ptr<NoneMixEngine> playbackEngine_ = nullptr;
static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;

/*
* describe: get data from outside untrusted data(g_data) which size is according to sizeof(T)
* tips: only support basic type
*/
template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
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

void DeviceFuzzTestSetUp()
{
    if (playbackEngine_ != nullptr) {
        return;
    }
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    playbackEngine_ = std::make_unique<NoneMixEngine>();
    bool isVoip = GetData<bool>();
    playbackEngine_->Init(deviceInfo, isVoip);
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
    uint32_t num = GetData<uint32_t>();
    rendererStream->SetStreamIndex(num);
    rendererStream->Start();
    rendererStream->Pause();
    rendererStream->Flush();
    rendererStream->Stop();
    rendererStream->Release();
}

void NoneMixEngineStartFuzzTest()
{
    playbackEngine_ = std::make_unique<NoneMixEngine>();
    playbackEngine_->Start();
}

void NoneMixEngineStopFuzzTest()
{
    playbackEngine_ = std::make_unique<NoneMixEngine>();
    playbackEngine_->Stop();
}

void NoneMixEnginePauseFuzzTest()
{
    playbackEngine_ = std::make_unique<NoneMixEngine>();
    playbackEngine_->Pause();
}

void NoneMixEngineFlushFuzzTest()
{
    playbackEngine_ = std::make_unique<NoneMixEngine>();
    playbackEngine_->Flush();
}

void NoneMixEngineAddRendererFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    rendererStream->InitParams();
    uint32_t num = GetData<uint32_t>();
    rendererStream->SetStreamIndex(num);
    rendererStream->Start();
    playbackEngine_ = std::make_unique<NoneMixEngine>();
    playbackEngine_->AddRenderer(rendererStream);
}

void NoneMixEngineRemoveRendererFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    rendererStream->InitParams();
    uint32_t num = GetData<uint32_t>();
    rendererStream->SetStreamIndex(num);
    rendererStream->Start();
    playbackEngine_ = std::make_unique<NoneMixEngine>();
    playbackEngine_->AddRenderer(rendererStream);
    playbackEngine_->RemoveRenderer(rendererStream);
}


void AudioEndPointSeparateStartDeviceFuzzTest(std::shared_ptr<AudioEndpointSeparate> audioEndpoint)
{
    audioEndpoint->StartDevice();
    audioEndpoint->GetEndpointName();
    audioEndpoint->ShouldInnerCap(1);
    audioEndpoint->EnableFastInnerCap(1);
    audioEndpoint->DisableFastInnerCap(1);
    return;
}

void AudioEndPointSeparateConfigFuzzTest()
{
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    deviceInfo.networkId_ = LOCAL_NETWORK_ID;
    DeviceStreamInfo audioStreamInfo = {
        SAMPLE_RATE_48000,
        ENCODING_PCM,
        SAMPLE_S16LE,
        STEREO
    };
    deviceInfo.audioStreamInfo_ = audioStreamInfo;
    std::shared_ptr<AudioEndpointSeparate> audioEndpoint = nullptr;
    uint64_t id = GetData<uint64_t>();
    AudioEndpoint::EndpointType type = GetData<AudioEndpoint::EndpointType>();
    AudioStreamType streamType = GetData<AudioStreamType>();
    audioEndpoint = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    audioEndpoint->Config(deviceInfo);

    AudioEndPointSeparateStartDeviceFuzzTest(audioEndpoint);

    audioEndpoint->fastRenderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_FAST,
        HDI_ID_INFO_DEFAULT, true);
    float volume = GetData<float>();
    audioEndpoint->SetVolume(streamType, volume);

    uint32_t spanSizeInFrame = GetData<uint32_t>();
    uint32_t totalSizeInFrame = GetData<uint32_t>();
    uint32_t byteSizePerFrame = GetData<uint32_t>();
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

typedef void (*TestFuncs[9])();

TestFuncs g_testFuncs = {
    DeviceFuzzTestSetUp,
    DirectAudioPlayBackEngineStateFuzzTest,
    NoneMixEngineStartFuzzTest,
    NoneMixEngineStopFuzzTest,
    NoneMixEnginePauseFuzzTest,
    NoneMixEngineFlushFuzzTest,
    NoneMixEngineAddRendererFuzzTest,
    NoneMixEngineRemoveRendererFuzzTest,
    AudioEndPointSeparateConfigFuzzTest,
};

bool FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return false;
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

    return true;
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
