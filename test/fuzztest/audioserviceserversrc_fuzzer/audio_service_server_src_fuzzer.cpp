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
#include "audio_policy_manager_listener_stub_impl.h"
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
#include "audio_playback_engine.h"
#include "pro_renderer_stream_impl.h"
#include "oh_audio_buffer.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
constexpr int32_t DEFAULT_STREAM_ID = 10;
static std::unique_ptr<NoneMixEngine> playbackEngine_ = nullptr;
static std::unique_ptr<AudioPlaybackEngine> audioPlaybackEngine_ = nullptr;
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

void ReleaseNoneEngine()
{
    if (playbackEngine_ != nullptr) {
        playbackEngine_->Stop();
        playbackEngine_ = nullptr;
    }
}

void ReleaseAudioPlaybackEngine()
{
    if (audioPlaybackEngine_ != nullptr) {
        audioPlaybackEngine_->Stop();
        audioPlaybackEngine_ = nullptr;
    }
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
    ReleaseNoneEngine();
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
    ReleaseNoneEngine();
}

void NoneMixEngineStopFuzzTest()
{
    playbackEngine_ = std::make_unique<NoneMixEngine>();
    playbackEngine_->Stop();
    ReleaseNoneEngine();
}

void NoneMixEnginePauseFuzzTest()
{
    playbackEngine_ = std::make_unique<NoneMixEngine>();
    playbackEngine_->Pause();
    ReleaseNoneEngine();
}

void NoneMixEngineFlushFuzzTest()
{
    playbackEngine_ = std::make_unique<NoneMixEngine>();
    playbackEngine_->Flush();
    ReleaseNoneEngine();
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
    ReleaseNoneEngine();
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
    ReleaseNoneEngine();
}

void PlaybackEngineInitFuzzTest()
{
    if (playbackEngine_ != nullptr) {
        return;
    }
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    audioPlaybackEngine_ = std::make_unique<AudioPlaybackEngine>();
    bool isVoip = GetData<bool>();
    audioPlaybackEngine_->Init(deviceInfo, isVoip);
    ReleaseAudioPlaybackEngine();
}

void PlaybackEngineStartFuzzTest()
{
    audioPlaybackEngine_ = std::make_unique<AudioPlaybackEngine>();
    audioPlaybackEngine_->Start();
    ReleaseAudioPlaybackEngine();
}

void PlaybackEngineStopFuzzTest()
{
    audioPlaybackEngine_ = std::make_unique<AudioPlaybackEngine>();
    audioPlaybackEngine_->Stop();
    ReleaseAudioPlaybackEngine();
}

void PlaybackEnginePauseFuzzTest()
{
    audioPlaybackEngine_ = std::make_unique<AudioPlaybackEngine>();
    audioPlaybackEngine_->Pause();
    ReleaseAudioPlaybackEngine();
}

void PlaybackEngineFlushFuzzTest()
{
    audioPlaybackEngine_ = std::make_unique<AudioPlaybackEngine>();
    audioPlaybackEngine_->Flush();
    ReleaseAudioPlaybackEngine();
}

void PlaybackEngineIsPlaybackEngineRunningFuzzTest()
{
    audioPlaybackEngine_ = std::make_unique<AudioPlaybackEngine>();
    audioPlaybackEngine_->IsPlaybackEngineRunning();
    ReleaseAudioPlaybackEngine();
}

void PlaybackEngineGetLatencyFuzzTest()
{
    audioPlaybackEngine_ = std::make_unique<AudioPlaybackEngine>();
    audioPlaybackEngine_->GetLatency();
    ReleaseAudioPlaybackEngine();
}

void PlaybackEngineAddRendererFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    rendererStream->InitParams();
    uint32_t num = GetData<uint32_t>();
    rendererStream->SetStreamIndex(num);
    rendererStream->Start();
    audioPlaybackEngine_ = std::make_unique<AudioPlaybackEngine>();
    audioPlaybackEngine_->AddRenderer(rendererStream);
    ReleaseAudioPlaybackEngine();
}

void PlaybackEngineRemoveRendererFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    rendererStream->InitParams();
    uint32_t num = GetData<uint32_t>();
    rendererStream->SetStreamIndex(num);
    rendererStream->Start();
    audioPlaybackEngine_ = std::make_unique<AudioPlaybackEngine>();
    audioPlaybackEngine_->AddRenderer(rendererStream);
    audioPlaybackEngine_->RemoveRenderer(rendererStream);
    ReleaseAudioPlaybackEngine();
}

void ResourceServiceAudioWorkgroupCheckFuzzTest()
{
    int32_t pid = GetData<int32_t>();
    AudioResourceService::GetInstance()->AudioWorkgroupCheck(pid);
}

void ResourceServiceCreateAudioWorkgroupFuzzTest()
{
    int32_t pid = GetData<int32_t>();
    sptr<IRemoteObject> remoteObject = nullptr;
    AudioResourceService::GetInstance()->CreateAudioWorkgroup(pid, remoteObject);
}

void ResourceServiceReleaseAudioWorkgroupFuzzTest()
{
    int32_t pid = GetData<int32_t>();
    int32_t workgroupId = GetData<int32_t>();
    AudioResourceService::GetInstance()->ReleaseAudioWorkgroup(pid, workgroupId);
}

void ResourceServiceAddThreadToGroupFuzzTest()
{
    int32_t pid = GetData<int32_t>();
    int32_t workgroupId = GetData<int32_t>();
    int32_t tokenId = GetData<int32_t>();
    AudioResourceService::GetInstance()->AddThreadToGroup(pid, workgroupId, tokenId);
}

void ResourceServiceRemoveThreadFromGroupFuzzTest()
{
    int32_t pid = GetData<int32_t>();
    int32_t workgroupId = GetData<int32_t>();
    int32_t tokenId = GetData<int32_t>();
    AudioResourceService::GetInstance()->RemoveThreadFromGroup(pid, workgroupId, tokenId);
}

void ResourceServiceStartGroupFuzzTest()
{
    int32_t pid = GetData<int32_t>();
    int32_t workgroupId = GetData<int32_t>();
    uint64_t startTime = GetData<uint64_t>();
    uint64_t deadlineTime = GetData<uint64_t>();
    AudioResourceService::GetInstance()->StartGroup(pid, workgroupId, startTime, deadlineTime);
}

void ResourceServiceStopGroupFuzzTest()
{
    int32_t pid = GetData<int32_t>();
    int32_t workgroupId = GetData<int32_t>();
    AudioResourceService::GetInstance()->StopGroup(pid, workgroupId);
}

void ResourceServiceGetAudioWorkgroupPtrFuzzTest()
{
    int32_t pid = GetData<int32_t>();
    int32_t workgroupId = GetData<int32_t>();
    AudioResourceService::GetInstance()->GetAudioWorkgroupPtr(pid, workgroupId);
}

void ResourceServiceGetThreadsNumPerProcessFuzzTest()
{
    int32_t pid = GetData<int32_t>();
    AudioResourceService::GetInstance()->GetThreadsNumPerProcess(pid);
}

void ResourceServiceIsProcessHasSystemPermissionFuzzTest()
{
    int32_t pid = GetData<int32_t>();
    AudioResourceService::GetInstance()->IsProcessHasSystemPermission(pid);
}

void ResourceServiceRegisterAudioWorkgroupMonitorFuzzTest()
{
    int32_t pid = GetData<int32_t>();
    int32_t groupId = GetData<int32_t>();
    sptr<IRemoteObject> object = nullptr;
    AudioResourceService::GetInstance()->RegisterAudioWorkgroupMonitor(pid, groupId, object);
}

void RenderInServerGetLastAudioDurationFuzzTest()
{
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
    renderer->GetLastAudioDuration();
}

void RenderInServerHandleOperationStartedFuzzTest()
{
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
    renderer->HandleOperationStarted();
}

void RenderInServerStandByCheckFuzzTest()
{
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
    renderer->StandByCheck();
}

void RenderInServerShouldEnableStandByFuzzTest()
{
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
    renderer->ShouldEnableStandBy();
}

void RenderInServerGetStandbyStatusFuzzTest()
{
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
	
    bool isStandby = GetData<bool>();
    int64_t enterStandbyTime = GetData<int64_t>();
    renderer->GetStandbyStatus(isStandby, enterStandbyTime);
}

void RenderInServerWriteMuteDataSysEventFuzzTest()
{
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
    BufferDesc desc;
    desc.buffer = nullptr;
    desc.bufLength = 0;
    desc.dataLength =0;
    renderer->WriteMuteDataSysEvent(desc);
}

void RenderInServerInnerCaptureEnqueueBufferFuzzTest()
{
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
    BufferDesc bufferDesc;
    bufferDesc.buffer = nullptr;
    bufferDesc.bufLength = 0;
    bufferDesc.dataLength =0;
    CaptureInfo captureInfo;
    int32_t innerCapId = GetData<int32_t>();
    renderer->InnerCaptureEnqueueBuffer(bufferDesc, captureInfo, innerCapId);
}

void RenderInServerInnerCaptureOtherStreamFuzzTest()
{
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
    BufferDesc bufferDesc;
    bufferDesc.buffer = nullptr;
    bufferDesc.bufLength = 0;
    bufferDesc.dataLength =0;
    CaptureInfo captureInfo;
    int32_t innerCapId = GetData<int32_t>();
    renderer->InnerCaptureOtherStream(bufferDesc, captureInfo, innerCapId);
}

void RenderInServerOtherStreamEnqueueFuzzTest()
{
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
	
    unsigned char inputData[] = "test";
    BufferDesc bufferDesc;
    bufferDesc.buffer = inputData;
    bufferDesc.bufLength = 0;
    bufferDesc.dataLength =0;
    renderer->OtherStreamEnqueue(bufferDesc);
}

void RenderInServerIsInvalidBufferFuzzTest()
{
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
	
    unsigned char inputData[] = "test";
    BufferDesc bufferDesc;
    bufferDesc.buffer = inputData;
    bufferDesc.bufLength = 0;
    bufferDesc.dataLength =0;
    renderer->IsInvalidBuffer(bufferDesc.buffer, bufferDesc.bufLength);
}

void RenderInServerDualToneStreamInStartFuzzTest()
{
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
	
    renderer->dualToneStreamInStart();
}

void RenderInServerRecordStandbyTimeFuzzTest()
{
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
    bool isStandby = false;
    bool isStandbyStart = GetData<bool>();
    renderer->RecordStandbyTime(isStandby, isStandbyStart);
}

void ProRendererGetStreamFramesWrittenFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    uint64_t framesWritten = GetData<uint64_t>();
    rendererStream->GetStreamFramesWritten(framesWritten);
}

void ProRendererGetCurrentTimeStampFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    uint64_t timestamp = GetData<uint64_t>();
    rendererStream->GetCurrentTimeStamp(timestamp);
}

void ProRendererGetCurrentPositionFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    uint64_t framePosition = GetData<uint64_t>();
    uint64_t timestamp = GetData<uint64_t>();
    uint64_t latency = GetData<uint64_t>();
    uint32_t base = GetData<uint32_t>();
    rendererStream->GetCurrentPosition(framePosition, timestamp, latency, base);
}

void ProRendererGetLatencyFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    uint64_t latency = GetData<uint64_t>();
    rendererStream->GetLatency(latency);
}

void ProRendererSetAudioEffectModeFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t effectMode = GetData<int32_t>();
    rendererStream->SetAudioEffectMode(effectMode);
}

void ProRendererGetAudioEffectModeFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t effectMode = GetData<int32_t>();
    rendererStream->GetAudioEffectMode(effectMode);
}

void ProRendererSetPrivacyTypeFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t privacyType = GetData<int32_t>();
    rendererStream->SetPrivacyType(privacyType);
}

void ProRendererGetPrivacyTypeFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t privacyType = GetData<int32_t>();
    rendererStream->GetPrivacyType(privacyType);
}

void ProRendererSetSpeedFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    float speed = GetData<float>();
    rendererStream->SetSpeed(speed);
}

void ProRendererDequeueBufferFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    size_t length = GetData<size_t>();
    rendererStream->DequeueBuffer(length);
}

void ProRendererEnqueueBufferFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    BufferDesc bufferDesc;
    bufferDesc.buffer = nullptr;
    bufferDesc.bufLength = 0;
    bufferDesc.dataLength =0;
    rendererStream->EnqueueBuffer(bufferDesc);
}

void ProRendererGetMinimumBufferSizeFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    size_t minBufferSize = GetData<size_t>();
    rendererStream->GetMinimumBufferSize(minBufferSize);
}

void ProRendererGetByteSizePerFrameFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    size_t byteSizePerFrame = GetData<size_t>();
    rendererStream->GetByteSizePerFrame(byteSizePerFrame);
}

void ProRendererGetSpanSizePerFrameFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    size_t spanSizeInFrame = GetData<size_t>();
    rendererStream->GetSpanSizePerFrame(spanSizeInFrame);
}

void ProRendererGetStreamIndexFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    rendererStream->GetStreamIndex();
}

void ProRendererOffloadSetVolumeFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    float volume = GetData<float>();
    rendererStream->OffloadSetVolume(volume);
}

void ProRendererSetOffloadDataCallbackStateFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t state = GetData<int32_t>();
    rendererStream->SetOffloadDataCallbackState(state);
}
 
void ProRendererUpdateSpatializationStateFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    bool spatializationEnabled = GetData<bool>();
    bool headTrackingEnabled = GetData<bool>();
    rendererStream->UpdateSpatializationState(spatializationEnabled, headTrackingEnabled);
}

void ProRendererGetAudioTimeFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    uint64_t framePos = GetData<uint64_t>();
    int64_t sec = GetData<int64_t>();
    int64_t nanoSec = GetData<int64_t>();
    rendererStream->GetAudioTime(framePos, sec, nanoSec);
}

void ProRendererPeekFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t index = GetData<int32_t>();
    std::vector<char> audioBuffer = {0x01, 0x02, 0x03, 0x04, 0x05};
    rendererStream->Peek(&audioBuffer, index);
}

void ProRendererReturnIndexFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t index = GetData<int32_t>();
    rendererStream->ReturnIndex(index);
}

void ProRendererSetClientVolumeFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    float clientVolume = GetData<float>();
    rendererStream->SetClientVolume(clientVolume);
}

void ProRendererSetLoudnessGainFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    float loudnessGain = GetData<float>();
    rendererStream->SetLoudnessGain(loudnessGain);
}

void ProRendererUpdateMaxLengthFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    uint32_t maxLength = GetData<uint32_t>();
    rendererStream->UpdateMaxLength(maxLength);
}

void ProRendererPopSinkBufferFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t index = GetData<int32_t>();
    std::vector<char> audioBuffer = {0x01, 0x02, 0x03, 0x04, 0x05};
    rendererStream->PopSinkBuffer(&audioBuffer, index);
}

void ProRendererGetStreamVolumeFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    rendererStream->GetStreamVolume();
}

typedef void (*TestFuncs[66])();

TestFuncs g_testFuncs = {
    DeviceFuzzTestSetUp,
    DirectAudioPlayBackEngineStateFuzzTest,
    NoneMixEngineStartFuzzTest,
    NoneMixEngineStopFuzzTest,
    NoneMixEnginePauseFuzzTest,
    NoneMixEngineFlushFuzzTest,
    NoneMixEngineAddRendererFuzzTest,
    NoneMixEngineRemoveRendererFuzzTest,
    PlaybackEngineInitFuzzTest,
    PlaybackEngineStartFuzzTest,
    PlaybackEngineStopFuzzTest,
    PlaybackEnginePauseFuzzTest,
    PlaybackEngineFlushFuzzTest,
    PlaybackEngineIsPlaybackEngineRunningFuzzTest,
    PlaybackEngineGetLatencyFuzzTest,
    PlaybackEngineAddRendererFuzzTest,
    PlaybackEngineRemoveRendererFuzzTest,
    ResourceServiceAudioWorkgroupCheckFuzzTest,
    ResourceServiceCreateAudioWorkgroupFuzzTest,
    ResourceServiceReleaseAudioWorkgroupFuzzTest,
    ResourceServiceAddThreadToGroupFuzzTest,
    ResourceServiceRemoveThreadFromGroupFuzzTest,
    ResourceServiceStartGroupFuzzTest,
    ResourceServiceStopGroupFuzzTest,
    ResourceServiceGetAudioWorkgroupPtrFuzzTest,
    ResourceServiceGetThreadsNumPerProcessFuzzTest,
    ResourceServiceIsProcessHasSystemPermissionFuzzTest,
    ResourceServiceRegisterAudioWorkgroupMonitorFuzzTest,
    RenderInServerGetLastAudioDurationFuzzTest,
    RenderInServerHandleOperationStartedFuzzTest,
    RenderInServerStandByCheckFuzzTest,
    RenderInServerShouldEnableStandByFuzzTest,
    RenderInServerGetStandbyStatusFuzzTest,
    RenderInServerWriteMuteDataSysEventFuzzTest,
    RenderInServerInnerCaptureEnqueueBufferFuzzTest,
    RenderInServerInnerCaptureOtherStreamFuzzTest,
    RenderInServerOtherStreamEnqueueFuzzTest,
    RenderInServerIsInvalidBufferFuzzTest,
    RenderInServerDualToneStreamInStartFuzzTest,
    RenderInServerRecordStandbyTimeFuzzTest,
    ProRendererGetStreamFramesWrittenFuzzTest,
    ProRendererGetCurrentTimeStampFuzzTest,
    ProRendererGetCurrentPositionFuzzTest,
    ProRendererGetLatencyFuzzTest,
    ProRendererSetAudioEffectModeFuzzTest,
    ProRendererGetAudioEffectModeFuzzTest,
    ProRendererSetPrivacyTypeFuzzTest,
    ProRendererGetPrivacyTypeFuzzTest,
    ProRendererSetSpeedFuzzTest,
    ProRendererDequeueBufferFuzzTest,
    ProRendererEnqueueBufferFuzzTest,
    ProRendererGetMinimumBufferSizeFuzzTest,
    ProRendererGetByteSizePerFrameFuzzTest,
    ProRendererGetSpanSizePerFrameFuzzTest,
    ProRendererGetStreamIndexFuzzTest,
    ProRendererOffloadSetVolumeFuzzTest,
    ProRendererSetOffloadDataCallbackStateFuzzTest,
    ProRendererUpdateSpatializationStateFuzzTest,
    ProRendererGetAudioTimeFuzzTest,
    ProRendererPeekFuzzTest,
    ProRendererReturnIndexFuzzTest,
    ProRendererSetClientVolumeFuzzTest,
    ProRendererSetLoudnessGainFuzzTest,
    ProRendererUpdateMaxLengthFuzzTest,
    ProRendererPopSinkBufferFuzzTest,
    ProRendererGetStreamVolumeFuzzTest,
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
