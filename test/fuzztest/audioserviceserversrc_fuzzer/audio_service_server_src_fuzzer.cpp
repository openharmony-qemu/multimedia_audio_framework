/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
#include "audio_device_descriptor.h"
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
#include "../fuzz_utils.h"
#include "core_service_handler.h"
#include "iservice_registry.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
constexpr int32_t DEFAULT_STREAM_ID = 10;
static std::unique_ptr<NoneMixEngine> playbackEngine_ = nullptr;
static std::unique_ptr<AudioPlaybackEngine> audioPlaybackEngine_ = nullptr;
FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
constexpr int32_t REQUEST_DATA_LEN = 3;
const uint32_t NUM = 1;
const int32_t AUDIO_DISTRIBUTED_SERVICE_ID = 3001;

const std::vector<AudioSamplingRate> g_audioSamplingRate = {
    SAMPLE_RATE_8000,
    SAMPLE_RATE_11025,
    SAMPLE_RATE_12000,
    SAMPLE_RATE_16000,
    SAMPLE_RATE_22050,
    SAMPLE_RATE_24000,
    SAMPLE_RATE_32000,
    SAMPLE_RATE_44100,
    SAMPLE_RATE_48000,
    SAMPLE_RATE_64000,
    SAMPLE_RATE_88200,
    SAMPLE_RATE_96000,
    SAMPLE_RATE_176400,
    SAMPLE_RATE_192000,
    SAMPLE_RATE_384000
};

const std::vector<AudioStreamType> g_audioStreamType = {
    STREAM_DEFAULT,
    STREAM_VOICE_CALL,
    STREAM_MUSIC,
    STREAM_RING,
    STREAM_MEDIA,
    STREAM_VOICE_ASSISTANT,
    STREAM_SYSTEM,
    STREAM_ALARM,
    STREAM_NOTIFICATION,
    STREAM_BLUETOOTH_SCO,
    STREAM_ENFORCED_AUDIBLE,
    STREAM_DTMF,
    STREAM_TTS,
    STREAM_ACCESSIBILITY,
    STREAM_RECORDING,
    STREAM_MOVIE,
    STREAM_GAME,
    STREAM_SPEECH,
    STREAM_SYSTEM_ENFORCED,
    STREAM_ULTRASONIC,
    STREAM_WAKEUP,
    STREAM_VOICE_MESSAGE,
    STREAM_NAVIGATION,
    STREAM_INTERNAL_FORCE_STOP,
    STREAM_SOURCE_VOICE_CALL,
    STREAM_VOICE_COMMUNICATION,
    STREAM_VOICE_RING,
    STREAM_VOICE_CALL_ASSISTANT,
    STREAM_CAMCORDER,
    STREAM_APP,
    STREAM_TYPE_MAX,
    STREAM_ALL,
};

const std::vector<AudioSampleFormat> g_audioSampleFormat = {
    SAMPLE_U8,
    SAMPLE_S16LE,
    SAMPLE_S24LE,
    SAMPLE_S32LE,
    SAMPLE_F32LE,
    INVALID_WIDTH,
};

const std::vector<LatencyFlag> g_latencyFlag = {
    LATENCY_FLAG_SHARED_BUFFER,
    LATENCY_FLAG_ENGINE,
    LATENCY_FLAG_SOFTWARE,
    LATENCY_FLAG_HARDWARE,
    LATENCY_FLAG_ALL
};

const std::vector<DeviceType> g_deviceType = {
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
    DEVICE_TYPE_HEARING_AID,
    DEVICE_TYPE_HDMI,
    DEVICE_TYPE_LINE_DIGITAL,
    DEVICE_TYPE_NEARLINK,
    DEVICE_TYPE_NEARLINK_IN,
    DEVICE_TYPE_BT_SPP,
    DEVICE_TYPE_NEARLINK_PORT,
    DEVICE_TYPE_FILE_SINK,
    DEVICE_TYPE_FILE_SOURCE,
    DEVICE_TYPE_EXTERN_CABLE,
    DEVICE_TYPE_SYSTEM_PRIVATE,
    DEVICE_TYPE_DEFAULT,
    DEVICE_TYPE_USB_ARM_HEADSET,
    DEVICE_TYPE_MAX
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

typedef void (*TestFuncs)();
/*
* describe: get data from outside untrusted data(g_data) which size is according to sizeof(T)
* tips: only support basic type
*/

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
    bool isVoip = g_fuzzUtils.GetData<bool>();
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
    uint32_t num = g_fuzzUtils.GetData<uint32_t>();
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
    uint32_t num = g_fuzzUtils.GetData<uint32_t>();
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
    uint32_t num = g_fuzzUtils.GetData<uint32_t>();
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
    bool isVoip = g_fuzzUtils.GetData<bool>();
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
    uint32_t num = g_fuzzUtils.GetData<uint32_t>();
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
    uint32_t num = g_fuzzUtils.GetData<uint32_t>();
    rendererStream->SetStreamIndex(num);
    rendererStream->Start();
    audioPlaybackEngine_ = std::make_unique<AudioPlaybackEngine>();
    audioPlaybackEngine_->AddRenderer(rendererStream);
    audioPlaybackEngine_->RemoveRenderer(rendererStream);
    ReleaseAudioPlaybackEngine();
}

void ResourceServiceAudioWorkgroupCheckFuzzTest()
{
    int32_t pid = g_fuzzUtils.GetData<int32_t>();
    AudioResourceService::GetInstance()->AudioWorkgroupCheck(pid);
}

void ResourceServiceCreateAudioWorkgroupFuzzTest()
{
    int32_t pid = g_fuzzUtils.GetData<int32_t>();
    sptr<IRemoteObject> remoteObject = nullptr;
    AudioResourceService::GetInstance()->CreateAudioWorkgroup(pid, remoteObject);
}

void ResourceServiceReleaseAudioWorkgroupFuzzTest()
{
    int32_t pid = g_fuzzUtils.GetData<int32_t>();
    int32_t workgroupId = g_fuzzUtils.GetData<int32_t>();
    AudioResourceService::GetInstance()->ReleaseAudioWorkgroup(pid, workgroupId);
}

void ResourceServiceAddThreadToGroupFuzzTest()
{
    int32_t pid = g_fuzzUtils.GetData<int32_t>();
    int32_t workgroupId = g_fuzzUtils.GetData<int32_t>();
    int32_t tokenId = g_fuzzUtils.GetData<int32_t>();
    AudioResourceService::GetInstance()->AddThreadToGroup(pid, workgroupId, tokenId);
}

void ResourceServiceRemoveThreadFromGroupFuzzTest()
{
    int32_t pid = g_fuzzUtils.GetData<int32_t>();
    int32_t workgroupId = g_fuzzUtils.GetData<int32_t>();
    int32_t tokenId = g_fuzzUtils.GetData<int32_t>();
    AudioResourceService::GetInstance()->RemoveThreadFromGroup(pid, workgroupId, tokenId);
}

void ResourceServiceStartGroupFuzzTest()
{
    int32_t pid = g_fuzzUtils.GetData<int32_t>();
    int32_t workgroupId = g_fuzzUtils.GetData<int32_t>();
    uint64_t startTime = g_fuzzUtils.GetData<uint64_t>();
    uint64_t deadlineTime = g_fuzzUtils.GetData<uint64_t>();
    AudioResourceService::GetInstance()->StartGroup(pid, workgroupId, startTime, deadlineTime);
}

void ResourceServiceStopGroupFuzzTest()
{
    int32_t pid = g_fuzzUtils.GetData<int32_t>();
    int32_t workgroupId = g_fuzzUtils.GetData<int32_t>();
    AudioResourceService::GetInstance()->StopGroup(pid, workgroupId);
}

void ResourceServiceGetAudioWorkgroupFuzzTest()
{
    int32_t pid = g_fuzzUtils.GetData<int32_t>();
    int32_t workgroupId = g_fuzzUtils.GetData<int32_t>();
    AudioResourceService::GetInstance()->GetAudioWorkgroup(pid, workgroupId);
}

void ResourceServiceGetThreadsNumPerProcessFuzzTest()
{
    int32_t pid = g_fuzzUtils.GetData<int32_t>();
    AudioResourceService::GetInstance()->GetThreadsNumPerProcess(pid);
}

void ResourceServiceIsProcessHasSystemPermissionFuzzTest()
{
    int32_t pid = g_fuzzUtils.GetData<int32_t>();
    AudioResourceService::GetInstance()->IsProcessHasSystemPermission(pid);
}

void ResourceServiceRegisterAudioWorkgroupMonitorFuzzTest()
{
    int32_t pid = g_fuzzUtils.GetData<int32_t>();
    int32_t groupId = g_fuzzUtils.GetData<int32_t>();
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
    CHECK_AND_RETURN(renderer != nullptr);

    renderer->standByEnable_ = g_fuzzUtils.GetData<bool>();
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

    bool isStandby = g_fuzzUtils.GetData<bool>();
    int64_t enterStandbyTime = g_fuzzUtils.GetData<int64_t>();
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
    CHECK_AND_RETURN(renderer != nullptr);

    BufferDesc desc;
    desc.buffer = nullptr;
    desc.bufLength = 0;
    desc.dataLength = 0;
    renderer->isInSilentState_ = g_fuzzUtils.GetData<bool>();
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
    CHECK_AND_RETURN(renderer != nullptr);

    BufferDesc bufferDesc;
    bufferDesc.buffer = nullptr;
    bufferDesc.bufLength = 0;
    bufferDesc.dataLength = 0;
    CaptureInfo captureInfo;
    AudioProcessConfig audioProcessConfig;
    audioProcessConfig.streamType = STREAM_MUSIC;
    captureInfo.dupStream = std::make_shared<ProRendererStreamImpl>(audioProcessConfig, true);
    int32_t innerCapId = g_fuzzUtils.GetData<int32_t>();
    renderer->renderEmptyCountForInnerCapToInnerCapIdMap_[innerCapId] = g_fuzzUtils.GetData<int32_t>();
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
    int32_t innerCapId = g_fuzzUtils.GetData<int32_t>();
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
    CHECK_AND_RETURN(renderer != nullptr);

    unsigned char inputData[] = "test";
    BufferDesc bufferDesc;
    bufferDesc.buffer = inputData;
    bufferDesc.bufLength = 0;
    bufferDesc.dataLength =0;
    renderer->isDualToneEnabled_ = g_fuzzUtils.GetData<bool>();
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
    bool isStandbyStart = g_fuzzUtils.GetData<bool>();
    renderer->RecordStandbyTime(isStandby, isStandbyStart);
}

void ProRendererGetStreamFramesWrittenFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    uint64_t framesWritten = g_fuzzUtils.GetData<uint64_t>();
    rendererStream->GetStreamFramesWritten(framesWritten);
}

void ProRendererGetCurrentTimeStampFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    uint64_t timestamp = g_fuzzUtils.GetData<uint64_t>();
    rendererStream->GetCurrentTimeStamp(timestamp);
}

void ProRendererGetCurrentPositionFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    uint64_t framePosition = g_fuzzUtils.GetData<uint64_t>();
    uint64_t timestamp = g_fuzzUtils.GetData<uint64_t>();
    uint64_t latency = g_fuzzUtils.GetData<uint64_t>();
    uint32_t base = g_fuzzUtils.GetData<uint32_t>();
    rendererStream->GetCurrentPosition(framePosition, timestamp, latency, base);
}

void ProRendererGetLatencyFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    rendererStream->byteSizePerFrame_ = g_fuzzUtils.GetData<size_t>();
    uint64_t latency = g_fuzzUtils.GetData<uint64_t>();
    rendererStream->GetLatency(latency);
}

void ProRendererSetAudioEffectModeFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t effectMode = g_fuzzUtils.GetData<int32_t>();
    rendererStream->SetAudioEffectMode(effectMode);
}

void ProRendererGetAudioEffectModeFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t effectMode = g_fuzzUtils.GetData<int32_t>();
    rendererStream->GetAudioEffectMode(effectMode);
}

void ProRendererSetPrivacyTypeFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t privacyType = g_fuzzUtils.GetData<int32_t>();
    rendererStream->SetPrivacyType(privacyType);
}

void ProRendererGetPrivacyTypeFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t privacyType = g_fuzzUtils.GetData<int32_t>();
    rendererStream->GetPrivacyType(privacyType);
}

void ProRendererSetSpeedFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    float speed = g_fuzzUtils.GetData<float>();
    rendererStream->SetSpeed(speed);
}

void ProRendererDequeueBufferFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    size_t length = g_fuzzUtils.GetData<size_t>();
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
    size_t minBufferSize = g_fuzzUtils.GetData<size_t>();
    rendererStream->GetMinimumBufferSize(minBufferSize);
}

void ProRendererGetByteSizePerFrameFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    size_t byteSizePerFrame = g_fuzzUtils.GetData<size_t>();
    rendererStream->GetByteSizePerFrame(byteSizePerFrame);
}

void ProRendererGetSpanSizePerFrameFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    size_t spanSizeInFrame = g_fuzzUtils.GetData<size_t>();
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
    rendererStream->OffloadSetVolume();
}

void ProRendererSetOffloadDataCallbackStateFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t state = g_fuzzUtils.GetData<int32_t>();
    rendererStream->SetOffloadDataCallbackState(state);
}

void ProRendererUpdateSpatializationStateFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    bool spatializationEnabled = g_fuzzUtils.GetData<bool>();
    bool headTrackingEnabled = g_fuzzUtils.GetData<bool>();
    rendererStream->UpdateSpatializationState(spatializationEnabled, headTrackingEnabled);
}

void ProRendererGetAudioTimeFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    uint64_t framePos = g_fuzzUtils.GetData<uint64_t>();
    int64_t sec = g_fuzzUtils.GetData<int64_t>();
    int64_t nanoSec = g_fuzzUtils.GetData<int64_t>();
    rendererStream->GetAudioTime(framePos, sec, nanoSec);
}

void ProRendererPeekFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t index = g_fuzzUtils.GetData<int32_t>();
    std::vector<char> audioBuffer = {0x01, 0x02, 0x03, 0x04, 0x05};
    rendererStream->Peek(&audioBuffer, index);
}

void ProRendererReturnIndexFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t index = g_fuzzUtils.GetData<int32_t>();
    rendererStream->ReturnIndex(index);
}

void ProRendererSetClientVolumeFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    float clientVolume = g_fuzzUtils.GetData<float>();
    rendererStream->SetClientVolume(clientVolume);
}

void ProRendererSetLoudnessGainFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    float loudnessGain = g_fuzzUtils.GetData<float>();
    rendererStream->SetLoudnessGain(loudnessGain);
}

void ProRendererUpdateMaxLengthFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    uint32_t maxLength = g_fuzzUtils.GetData<uint32_t>();
    rendererStream->UpdateMaxLength(maxLength);
}

void ProRendererPopSinkBufferFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    int32_t index = g_fuzzUtils.GetData<int32_t>();
    std::vector<char> audioBuffer = {0x01, 0x02, 0x03, 0x04, 0x05};
    rendererStream->PopSinkBuffer(&audioBuffer, index);
}

void ProRendererGetStreamVolumeFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    rendererStream->GetStreamVolume();
}

void ReConfigDupStreamCallbackFuzzTest()
{
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
    CHECK_AND_RETURN(renderer != nullptr);

    renderer->dupTotalSizeInFrame_ = g_fuzzUtils.GetData<size_t>();
    renderer->ReConfigDupStreamCallback();
}

void DoFadingOutFuzzTest()
{
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
    CHECK_AND_RETURN(renderer != nullptr);

    RingBufferWrapper bufferDesc;
    renderer->DoFadingOut(bufferDesc);
}

void PrepareOutputBufferFuzzTest()
{
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
    CHECK_AND_RETURN(renderer != nullptr);

    RingBufferWrapper bufferDesc;
    renderer->PrepareOutputBuffer(bufferDesc);
}

void CopyDataToInputBufferFuzzTest()
{
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
    CHECK_AND_RETURN(renderer != nullptr);

    int8_t inPutData[REQUEST_DATA_LEN];
    RingBufferWrapper ringBufferDesc;
    renderer->CopyDataToInputBuffer(inPutData, REQUEST_DATA_LEN, ringBufferDesc);
}

void OnWriteDataFuzzTest()
{
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
    CHECK_AND_RETURN(renderer != nullptr);

    size_t length = g_fuzzUtils.GetData<size_t>();
    renderer->OnWriteData(length);
}

void PauseFuzzTest()
{
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
    CHECK_AND_RETURN(renderer != nullptr);

    renderer->standByEnable_ = g_fuzzUtils.GetData<bool>();
    renderer->Pause();
}

void DisableAllInnerCapFuzzTest()
{
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
    CHECK_AND_RETURN(renderer != nullptr);

    renderer->DisableAllInnerCap();
}

void OnStatusUpdateFuzzTest()
{
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    uint32_t streamIndex = g_fuzzUtils.GetData<uint32_t>();
    auto StreamCallbacksPtr = std::make_shared<StreamCallbacks>(streamIndex, rendererInServer);
    CHECK_AND_RETURN(StreamCallbacksPtr != nullptr);

    IOperation operation = g_fuzzUtils.GetData<IOperation>();
    StreamCallbacksPtr->OnStatusUpdate(operation);
}

void OnWriteDataStreamsCallbackFuzzTest()
{
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    uint32_t streamIndex = g_fuzzUtils.GetData<uint32_t>();
    auto StreamCallbacksPtr = std::make_shared<StreamCallbacks>(streamIndex, rendererInServer);
    CHECK_AND_RETURN(StreamCallbacksPtr != nullptr);

    size_t length = g_fuzzUtils.GetData<size_t>();
    StreamCallbacksPtr->OnWriteData(length);
}

void GetAvailableSizeStreamsCallbackFuzzTest()
{
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    uint32_t streamIndex = g_fuzzUtils.GetData<uint32_t>();
    auto StreamCallbacksPtr = std::make_shared<StreamCallbacks>(streamIndex, rendererInServer);
    CHECK_AND_RETURN(StreamCallbacksPtr != nullptr);

    size_t length = g_fuzzUtils.GetData<size_t>();
    StreamCallbacksPtr->GetAvailableSize(length);
}

void IsHighResolutionFuzzTest()
{
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
    CHECK_AND_RETURN(renderer != nullptr);

    renderer->IsHighResolution();
}

void SetMuteFuzzTest()
{
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
    CHECK_AND_RETURN(renderer != nullptr);

    bool isMute = g_fuzzUtils.GetData<bool>();
    renderer->SetMute(isMute);
}

void SetDuckFactorFuzzTest()
{
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
    CHECK_AND_RETURN(renderer != nullptr);

    float duckFactor = g_fuzzUtils.GetData<float>();
    renderer->SetDuckFactor(duckFactor, 0);
}

void SetDefaultOutputDeviceFuzzTest()
{
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
    CHECK_AND_RETURN(renderer != nullptr);

    DeviceType defaultOutputDevice = g_fuzzUtils.GetData<DeviceType>();
    renderer->SetDefaultOutputDevice(defaultOutputDevice);
}

void SetSpeedFuzzTest()
{
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
    CHECK_AND_RETURN(renderer != nullptr);

    float speed = g_fuzzUtils.GetData<float>();
    renderer->SetSpeed(speed);
}

void StopSessionFuzzTest()
{
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
    CHECK_AND_RETURN(renderer != nullptr);

    renderer->StopSession();
}

void InitDupBufferFuzzTest()
{
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
    CHECK_AND_RETURN(renderer != nullptr);

    int32_t innerCapId = g_fuzzUtils.GetData<int32_t>();
    renderer->InitDupBuffer(innerCapId);
}

void GetDirectSampleRateFuzzTest()
{
    AudioProcessConfig config;
    config.streamType = g_audioStreamType[g_fuzzUtils.GetData<uint32_t>() % g_audioStreamType.size()];
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);

    AudioSamplingRate sampleRate = g_audioSamplingRate[g_fuzzUtils.GetData<uint32_t>() % g_audioSamplingRate.size()];
    rendererStream->GetDirectSampleRate(sampleRate);
}

void GetDirectFormatFuzzTest()
{
    AudioProcessConfig config;
    bool isDirect = g_fuzzUtils.GetData<bool>();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, isDirect);

    AudioSampleFormat format = g_audioSampleFormat[g_fuzzUtils.GetData<uint32_t>() % g_audioSampleFormat.size()];
    rendererStream->GetDirectFormat(format);
}

void DrainFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);

    bool stopFlag = g_fuzzUtils.GetData<bool>();
    rendererStream->Drain(stopFlag);
}

void SetRateFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);

    int32_t rate = g_fuzzUtils.GetData<int32_t>();
    rendererStream->SetRate(rate);
}

void RegisterStatusCallbackFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);

    std::shared_ptr<StreamListenerHolder> streamListenerHolder = nullptr;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(config, streamListenerHolder);
    rendererStream->RegisterStatusCallback(rendererInServer);
}

void RegisterWriteCallbackFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);

    std::shared_ptr<StreamListenerHolder> streamListenerHolder = nullptr;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(config, streamListenerHolder);
    rendererStream->RegisterWriteCallback(rendererInServer);
}

void AbortCallbackFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);

    int32_t abortTimes = g_fuzzUtils.GetData<int8_t>();
    rendererStream->AbortCallback(abortTimes);
}

void SetAndUnsetOffloadModeFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);

    int32_t state = g_fuzzUtils.GetData<int32_t>();
    bool isAppBack = g_fuzzUtils.GetData<bool>();
    rendererStream->SetOffloadMode(state, isAppBack);
    rendererStream->UnsetOffloadMode();
}

void GetOffloadApproximatelyCacheTimeFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);

    uint64_t timestamp = g_fuzzUtils.GetData<uint64_t>();
    uint64_t paWriteIndex = g_fuzzUtils.GetData<uint64_t>();
    uint64_t cacheTimeDsp = g_fuzzUtils.GetData<uint64_t>();
    uint64_t cacheTimePa = g_fuzzUtils.GetData<uint64_t>();
    rendererStream->GetOffloadApproximatelyCacheTime(timestamp, paWriteIndex, cacheTimeDsp, cacheTimePa);
}

void GetWritableSizeFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    rendererStream->GetWritableSize();
}

void ConvertSrcToFloatFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    rendererStream->bufferInfo_.samplePerFrame = NUM;
    rendererStream->bufferInfo_.channelCount = NUM;
    BufferDesc desc;
    desc.buffer = nullptr;
    desc.bufLength = 0;
    desc.dataLength = 0;
    rendererStream->ConvertSrcToFloat(desc);
}

void BlockStreamFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    rendererStream->BlockStream();
}

void SetSendDataEnabledFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    bool enabled = g_fuzzUtils.GetData<bool>();
    rendererStream->SetSendDataEnabled(enabled);
}

void GetLatencyWithFlagFuzzTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::shared_ptr<ProRendererStreamImpl> rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    uint64_t latency = g_fuzzUtils.GetData<uint64_t>();
    LatencyFlag flag = g_latencyFlag[g_fuzzUtils.GetData<uint32_t>() % g_latencyFlag.size()];
    rendererStream->GetLatencyWithFlag(latency, flag);
}

void ConfigCoreServiceProviderFuzzTest()
{
    CoreServiceHandler handler = CoreServiceHandler::GetInstance();
    sptr<ICoreServiceProviderIpc> provider = nullptr;
    handler.ConfigCoreServiceProvider(provider);
}

void UpdateSessionOperationFuzzTest()
{
    CoreServiceHandler handler = CoreServiceHandler::GetInstance();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    sptr<ICoreServiceProviderIpc> coreServiceProvider = iface_cast<ICoreServiceProviderIpc>(object);
    handler.ConfigCoreServiceProvider(coreServiceProvider);

    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    int32_t count = static_cast<uint32_t>(SessionOperation::SESSION_OPERATION_RELEASE) + NUM;
    SessionOperation operation = static_cast<SessionOperation>(g_fuzzUtils.GetData<uint8_t>() % count);
    int32_t opMsgCount = static_cast<uint32_t>(SessionOperationMsg::SESSION_OP_MSG_REMOVE_PIPE) + NUM;
    SessionOperationMsg opMsg = static_cast<SessionOperationMsg>(g_fuzzUtils.GetData<uint8_t>() % opMsgCount);
    handler.UpdateSessionOperation(sessionId, operation, opMsg);
}

void ReloadCaptureSessionFuzzTest()
{
    CoreServiceHandler handler = CoreServiceHandler::GetInstance();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    sptr<ICoreServiceProviderIpc> coreServiceProvider = iface_cast<ICoreServiceProviderIpc>(object);
    handler.ConfigCoreServiceProvider(coreServiceProvider);

    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    int32_t count = static_cast<uint32_t>(SessionOperation::SESSION_OPERATION_RELEASE) + NUM;
    SessionOperation operation = static_cast<SessionOperation>(g_fuzzUtils.GetData<uint8_t>() % count);
    handler.ReloadCaptureSession(sessionId, operation);
}

void CoreSerHandleretDefaultOutputDeviceFuzzTest()
{
    CoreServiceHandler handler = CoreServiceHandler::GetInstance();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    sptr<ICoreServiceProviderIpc> coreServiceProvider = iface_cast<ICoreServiceProviderIpc>(object);
    handler.ConfigCoreServiceProvider(coreServiceProvider);

    DeviceType defaultOutputDevice = g_deviceType[g_fuzzUtils.GetData<uint32_t>() % g_deviceType.size()];
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    StreamUsage streamUsage = g_streamUsages[g_fuzzUtils.GetData<uint32_t>() % g_streamUsages.size()];
    bool isRunning = g_fuzzUtils.GetData<bool>();
    bool skipForce = g_fuzzUtils.GetData<bool>();
    handler.SetDefaultOutputDevice(defaultOutputDevice, sessionId, streamUsage, isRunning, skipForce);
}

void GetAdapterAndModuleNameBySessionIdFuzzTest()
{
    CoreServiceHandler handler = CoreServiceHandler::GetInstance();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    sptr<ICoreServiceProviderIpc> coreServiceProvider = iface_cast<ICoreServiceProviderIpc>(object);
    handler.ConfigCoreServiceProvider(coreServiceProvider);

    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    handler.GetAdapterNameBySessionId(sessionId);
    handler.GetModuleNameBySessionId(sessionId);
}

void GetProcessDeviceInfoBySessionIdFuzzTest()
{
    CoreServiceHandler handler = CoreServiceHandler::GetInstance();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    sptr<ICoreServiceProviderIpc> coreServiceProvider = iface_cast<ICoreServiceProviderIpc>(object);
    handler.ConfigCoreServiceProvider(coreServiceProvider);

    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    AudioDeviceDescriptor deviceInfo;
    AudioStreamInfo info;
    bool isUltraFast = g_fuzzUtils.GetData<bool>();
    bool isReloadProcess = g_fuzzUtils.GetData<bool>();
    handler.GetProcessDeviceInfoBySessionId(sessionId, deviceInfo, info, isUltraFast, isReloadProcess);
}

void GenerateSessionIdFuzzTest()
{
    CoreServiceHandler handler = CoreServiceHandler::GetInstance();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    sptr<ICoreServiceProviderIpc> coreServiceProvider = iface_cast<ICoreServiceProviderIpc>(object);
    handler.ConfigCoreServiceProvider(coreServiceProvider);
    handler.GenerateSessionId();
}

void SetWakeUpAudioCapturerFromAudioServerFuzzTest()
{
    CoreServiceHandler handler = CoreServiceHandler::GetInstance();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    sptr<ICoreServiceProviderIpc> coreServiceProvider = iface_cast<ICoreServiceProviderIpc>(object);
    handler.ConfigCoreServiceProvider(coreServiceProvider);

    AudioProcessConfig config = InitProcessConfig();
    handler.SetWakeUpAudioCapturerFromAudioServer(config);
}

void GetPaIndexByPortNameFuzzTest()
{
    CoreServiceHandler handler = CoreServiceHandler::GetInstance();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    sptr<ICoreServiceProviderIpc> coreServiceProvider = iface_cast<ICoreServiceProviderIpc>(object);
    handler.ConfigCoreServiceProvider(coreServiceProvider);

    std::string portName = "";
    handler.GetPaIndexByPortName(portName);
}

void A2dpOffloadGetRenderPositionFuzzTest()
{
    CoreServiceHandler handler = CoreServiceHandler::GetInstance();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    sptr<ICoreServiceProviderIpc> coreServiceProvider = iface_cast<ICoreServiceProviderIpc>(object);
    handler.ConfigCoreServiceProvider(coreServiceProvider);

    uint32_t delayValue = g_fuzzUtils.GetData<uint32_t>();
    uint64_t sendDataSize = g_fuzzUtils.GetData<uint64_t>();
    uint32_t timeStamp = g_fuzzUtils.GetData<uint32_t>();
    handler.A2dpOffloadGetRenderPosition(delayValue, sendDataSize, timeStamp);
}

void SetRendererTargetFuzzTest()
{
    CoreServiceHandler handler = CoreServiceHandler::GetInstance();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    sptr<ICoreServiceProviderIpc> coreServiceProvider = iface_cast<ICoreServiceProviderIpc>(object);
    handler.ConfigCoreServiceProvider(coreServiceProvider);

    uint32_t target = g_fuzzUtils.GetData<uint32_t>();
    uint32_t lastTarget = g_fuzzUtils.GetData<uint32_t>();
    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    handler.SetRendererTarget(target, lastTarget, sessionId);
}

void StartInjectionFuzzTest()
{
    CoreServiceHandler handler = CoreServiceHandler::GetInstance();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    sptr<ICoreServiceProviderIpc> coreServiceProvider = iface_cast<ICoreServiceProviderIpc>(object);
    handler.ConfigCoreServiceProvider(coreServiceProvider);

    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    handler.StartInjection(sessionId);
}

void RemoveIdForInjectorFuzzTest()
{
    CoreServiceHandler handler = CoreServiceHandler::GetInstance();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    sptr<ICoreServiceProviderIpc> coreServiceProvider = iface_cast<ICoreServiceProviderIpc>(object);
    handler.ConfigCoreServiceProvider(coreServiceProvider);

    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    handler.RemoveIdForInjector(sessionId);
}

void ReleaseCaptureInjectorFuzzTest()
{
    CoreServiceHandler handler = CoreServiceHandler::GetInstance();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    sptr<ICoreServiceProviderIpc> coreServiceProvider = iface_cast<ICoreServiceProviderIpc>(object);
    handler.ConfigCoreServiceProvider(coreServiceProvider);

    handler.ReleaseCaptureInjector();
}

void RebuildCaptureInjectorFuzzTest()
{
    CoreServiceHandler handler = CoreServiceHandler::GetInstance();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    sptr<ICoreServiceProviderIpc> coreServiceProvider = iface_cast<ICoreServiceProviderIpc>(object);
    handler.ConfigCoreServiceProvider(coreServiceProvider);

    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    handler.RebuildCaptureInjector(sessionId);
}

void OnCheckActiveMusicTimeFuzzTest()
{
    CoreServiceHandler handler = CoreServiceHandler::GetInstance();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    sptr<ICoreServiceProviderIpc> coreServiceProvider = iface_cast<ICoreServiceProviderIpc>(object);
    handler.ConfigCoreServiceProvider(coreServiceProvider);

    std::string reason = "";
    handler.OnCheckActiveMusicTime(reason);
}

void CaptureConcurrentCheckFuzzTest()
{
    CoreServiceHandler handler = CoreServiceHandler::GetInstance();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    sptr<ICoreServiceProviderIpc> coreServiceProvider = iface_cast<ICoreServiceProviderIpc>(object);
    handler.ConfigCoreServiceProvider(coreServiceProvider);

    uint32_t sessionId = g_fuzzUtils.GetData<uint32_t>();
    handler.CaptureConcurrentCheck(sessionId);
}

vector<TestFuncs> g_testFuncs = {
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
    ResourceServiceGetAudioWorkgroupFuzzTest,
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
    ReConfigDupStreamCallbackFuzzTest,
    DoFadingOutFuzzTest,
    PrepareOutputBufferFuzzTest,
    CopyDataToInputBufferFuzzTest,
    OnWriteDataFuzzTest,
    PauseFuzzTest,
    DisableAllInnerCapFuzzTest,
    OnStatusUpdateFuzzTest,
    OnWriteDataStreamsCallbackFuzzTest,
    GetAvailableSizeStreamsCallbackFuzzTest,
    IsHighResolutionFuzzTest,
    SetMuteFuzzTest,
    SetDuckFactorFuzzTest,
    SetDefaultOutputDeviceFuzzTest,
    SetSpeedFuzzTest,
    StopSessionFuzzTest,
    InitDupBufferFuzzTest,
    GetDirectSampleRateFuzzTest,
    GetDirectFormatFuzzTest,
    DrainFuzzTest,
    SetRateFuzzTest,
    RegisterStatusCallbackFuzzTest,
    RegisterWriteCallbackFuzzTest,
    AbortCallbackFuzzTest,
    SetAndUnsetOffloadModeFuzzTest,
    GetOffloadApproximatelyCacheTimeFuzzTest,
    GetWritableSizeFuzzTest,
    ConvertSrcToFloatFuzzTest,
    BlockStreamFuzzTest,
    SetSendDataEnabledFuzzTest,
    GetLatencyWithFlagFuzzTest,
    ConfigCoreServiceProviderFuzzTest,
    UpdateSessionOperationFuzzTest,
    ReloadCaptureSessionFuzzTest,
    CoreSerHandleretDefaultOutputDeviceFuzzTest,
    GetAdapterAndModuleNameBySessionIdFuzzTest,
    GetProcessDeviceInfoBySessionIdFuzzTest,
    GenerateSessionIdFuzzTest,
    SetWakeUpAudioCapturerFromAudioServerFuzzTest,
    GetPaIndexByPortNameFuzzTest,
    A2dpOffloadGetRenderPositionFuzzTest,
    SetRendererTargetFuzzTest,
    StartInjectionFuzzTest,
    RemoveIdForInjectorFuzzTest,
    ReleaseCaptureInjectorFuzzTest,
    RebuildCaptureInjectorFuzzTest,
    OnCheckActiveMusicTimeFuzzTest,
    CaptureConcurrentCheckFuzzTest,
};
} // namespace AudioStandard
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}