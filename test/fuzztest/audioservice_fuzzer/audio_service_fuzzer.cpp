/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "audio_endpoint.h"
#include "audio_endpoint_private.h"
#include "audio_service.h"
#include "audio_thread_task.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

const int32_t NUM_2 = 2;
const int32_t AUDIOCHANNELSIZE = 17;
const int32_t ENDPOINTTYPESIZE = 4;
const int32_t SAVE_FOREGROUND_LIST_NUM = 11;
static const std::string THREAD_NAME = "FuzzTestThreadName";
static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
typedef void (*TestPtr)();
constexpr size_t MAX_RANDOM_STRING_LENGTH = 128;
constexpr size_t MAX_STOP_AUDIO_TYPE = 3;

const vector<AudioStreamType> g_testAudioStreamTypes = {
    STREAM_DEFAULT,
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
    STREAM_VOICE_RING,
    STREAM_VOICE_CALL_ASSISTANT,
    STREAM_CAMCORDER,
    STREAM_APP,
    STREAM_TYPE_MAX,
    STREAM_ALL,
};

const vector<AudioSamplingRate> g_testAudioSamplingRates = {
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
};

const vector<AudioSampleFormat> g_testAudioSampleFormats = {
    SAMPLE_U8,
    SAMPLE_S16LE,
    SAMPLE_S24LE,
    SAMPLE_S32LE,
    SAMPLE_F32LE,
    INVALID_WIDTH,
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

const vector<SourceType> g_testSourceTypes = {
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
    SOURCE_TYPE_MAX,
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

template<>
std::string GetData<std::string>()
{
    const size_t len = GetData<size_t>() / MAX_RANDOM_STRING_LENGTH;
    std::string ret(len, ' ');
    for (auto &c : ret) {
        c = GetData<char>();
    }
    return ret;
}

#ifdef HAS_FEATURE_INNERCAPTURER
void AudioServiceOnProcessReleaseFuzzTest()
{
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioService == nullptr) {
        return;
    }
    static const vector<AudioEncodingType> testAudioEncodingTypes = {
        ENCODING_INVALID,
        ENCODING_PCM,
        ENCODING_AUDIOVIVID,
        ENCODING_EAC3,
    };
    AudioProcessConfig config = {};
    config.privacyType = static_cast<AudioPrivacyType>(GetData<uint32_t>() % NUM_2);
    config.audioMode = static_cast<AudioMode>(GetData<uint32_t>() % NUM_2);
    config.streamType = g_testAudioStreamTypes[GetData<uint32_t>() % g_testAudioStreamTypes.size()];
    config.streamInfo.channels = static_cast<AudioChannel>(GetData<uint32_t>() % AUDIOCHANNELSIZE);
    config.streamInfo.samplingRate = g_testAudioSamplingRates[GetData<uint32_t>() % g_testAudioSamplingRates.size()];
    config.streamInfo.format = g_testAudioSampleFormats[GetData<uint32_t>() % g_testAudioSampleFormats.size()];
    config.streamInfo.encoding = testAudioEncodingTypes[GetData<uint32_t>() % testAudioEncodingTypes.size()];
    auto audioProcess = audioService->GetAudioProcess(config);
    bool isSwitchStream = GetData<bool>();
    audioService->OnProcessRelease(audioProcess, isSwitchStream);
}

void AudioServiceCheckInnerCapForRendererFuzzTest()
{
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioService == nullptr) {
        return;
    }
    AudioProcessConfig processConfig;
    uint32_t sessionId = GetData<uint32_t>();
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
    audioService->CheckInnerCapForRenderer(sessionId, renderer);
}

void AudioServiceCheckInnerCapForProcessFuzzTest()
{
    AudioProcessConfig config = {};
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    AudioStreamInfo audioStreamInfo = { SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO, CH_LAYOUT_STEREO };
    deviceInfo.networkId_ = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpoint> audioEndpointPtr = AudioEndpoint::CreateEndpoint(
        AudioEndpoint::TYPE_MMAP, 0, config, deviceInfo, audioStreamInfo);
    AudioProcessConfig configProcess = {};
    sptr<AudioProcessInServer> audioProcess =  AudioProcessInServer::Create(configProcess,
        AudioService::GetInstance());
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioProcess == nullptr || audioEndpointPtr == nullptr || audioService == nullptr) {
        return;
    }
    audioService->CheckInnerCapForProcess(audioProcess, audioEndpointPtr);
}

void AudioServiceLinkProcessToEndpointFuzzTest()
{
    AudioProcessConfig config = {};
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    AudioStreamInfo audioStreamInfo = { SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO, CH_LAYOUT_STEREO };
    deviceInfo.networkId_ = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpoint> audioEndpointPtr = AudioEndpoint::CreateEndpoint(
        AudioEndpoint::TYPE_MMAP, 0, config, deviceInfo, audioStreamInfo);
    AudioProcessConfig configProcess = {};
    sptr<AudioProcessInServer> audioProcess =  AudioProcessInServer::Create(configProcess,
        AudioService::GetInstance());
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioProcess == nullptr || audioEndpointPtr == nullptr || audioService == nullptr) {
        return;
    }
    audioService->LinkProcessToEndpoint(audioProcess, audioEndpointPtr);
}

void AudioServiceUnlinkProcessToEndpointFuzzTest()
{
    AudioProcessConfig config = {};
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    AudioStreamInfo audioStreamInfo = { SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO, CH_LAYOUT_STEREO };
    deviceInfo.networkId_ = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpoint> audioEndpointPtr = AudioEndpoint::CreateEndpoint(
        AudioEndpoint::TYPE_MMAP, 0, config, deviceInfo, audioStreamInfo);
    AudioProcessConfig configProcess = {};
    sptr<AudioProcessInServer> audioProcess =  AudioProcessInServer::Create(configProcess,
        AudioService::GetInstance());
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioProcess == nullptr || audioEndpointPtr == nullptr || audioService == nullptr) {
        return;
    }
    audioService->UnlinkProcessToEndpoint(audioProcess, audioEndpointPtr);
}

void AudioServiceGetDeviceInfoForProcessFuzzTest()
{
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioService == nullptr) {
        return;
    }
    AudioProcessConfig config = {};
    config.originalSessionId = GetData<uint32_t>() / NUM_2;
    config.privacyType = static_cast<AudioPrivacyType>(GetData<uint32_t>() % NUM_2);
    config.capturerInfo.sourceType = g_testSourceTypes[GetData<uint32_t>() % g_testSourceTypes.size()];
    config.streamInfo.channels = static_cast<AudioChannel>(GetData<uint32_t>() % AUDIOCHANNELSIZE);
    config.streamInfo.samplingRate = g_testAudioSamplingRates[GetData<uint32_t>() % g_testAudioSamplingRates.size()];
    config.streamInfo.format = g_testAudioSampleFormats[GetData<uint32_t>() % g_testAudioSampleFormats.size()];

    AudioStreamInfo info;
    audioService->GetDeviceInfoForProcess(config, info);
}

void AudioServiceGetMaxAmplitudeFuzzTest()
{
    AudioProcessConfig config = {};
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    AudioStreamInfo audioStreamInfo = { SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO, CH_LAYOUT_STEREO };
    deviceInfo.networkId_ = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpoint> audioEndpointPtr = AudioEndpoint::CreateEndpoint(
        AudioEndpoint::TYPE_MMAP, 0, config, deviceInfo, audioStreamInfo);
    AudioProcessConfig configProcess = {};
    sptr<AudioProcessInServer> audioProcess =  AudioProcessInServer::Create(configProcess,
        AudioService::GetInstance());
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioProcess == nullptr || audioEndpointPtr == nullptr || audioService == nullptr) {
        return;
    }
    bool isOutputDevice = GetData<bool>();

    audioService->linkedPairedList_.clear();
    audioService->linkedPairedList_.push_back(make_pair(audioProcess, audioEndpointPtr));
    audioService->GetMaxAmplitude(isOutputDevice);
}

void AudioServiceGetCapturerBySessionIDFuzzTest()
{
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioService == nullptr) {
        return;
    }
    uint32_t sessionID = GetData<uint32_t>();
    audioService->allRendererMap_.clear();
    audioService->allCapturerMap_.insert(make_pair(
        sessionID, std::make_shared<CapturerInServer>(AudioProcessConfig(), std::weak_ptr<IStreamListener>())));
    audioService->GetCapturerBySessionID(sessionID);
}

void AudioServiceSetOffloadModeFuzzTest()
{
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioService == nullptr) {
        return;
    }
    uint32_t sessionId = GetData<uint32_t>();
    int32_t state = GetData<int32_t>();
    bool isAppBack = GetData<bool>();
    std::shared_ptr<CapturerInServer> capturer = nullptr;
    audioService->InsertCapturer(state, capturer);
    audioService->SetOffloadMode(sessionId, state, isAppBack);
}

#endif // HAS_FEATURE_INNERCAPTURER

void AudioServiceGetReleaseDelayTimeFuzzTest()
{
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioService == nullptr) {
        return;
    }
    AudioProcessConfig clientConfig = {};
    std::shared_ptr<AudioEndpointInner> endpoint = std::make_shared<AudioEndpointInner>(
        static_cast<AudioEndpoint::EndpointType>(GetData<uint32_t>() % ENDPOINTTYPESIZE),
        GetData<uint64_t>(), clientConfig.audioMode);
    endpoint->deviceInfo_.deviceType_ = g_testDeviceTypes[GetData<uint32_t>() % g_testDeviceTypes.size()];
    bool isSwitchStream = GetData<bool>();
    bool isRecord = GetData<bool>();
    audioService->GetReleaseDelayTime(endpoint, isSwitchStream, isRecord);
}

void AudioServiceRemoveIdFromMuteControlSetFuzzTest()
{
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioService == nullptr) {
        return;
    }
    uint32_t sessionId = GetData<uint32_t>();
    audioService->RemoveIdFromMuteControlSet(sessionId);
}

void AudioServiceCheckRenderSessionMuteStateFuzzTest()
{
    AudioProcessConfig processConfig;
    uint32_t sessionId = GetData<uint32_t>();
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioService == nullptr) {
        return;
    }
    audioService->UpdateMuteControlSet(sessionId, true);

    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
    audioService->CheckRenderSessionMuteState(sessionId, renderer);
}

void AudioServiceCheckCaptureSessionMuteStateFuzzTest()
{
    AudioProcessConfig processConfig;
    uint32_t sessionId = GetData<uint32_t>();
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioService == nullptr) {
        return;
    }
    audioService->UpdateMuteControlSet(sessionId, true);

    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<CapturerInServer> capturerInServer =
        std::make_shared<CapturerInServer>(processConfig, streamListener);
    std::shared_ptr<CapturerInServer> capturer = capturerInServer;
    audioService->CheckCaptureSessionMuteState(sessionId, capturer);
}

void AudioServiceCheckFastSessionMuteStateFuzzTest()
{
    AudioProcessConfig processConfig;
    uint32_t sessionId = GetData<uint32_t>();
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioService == nullptr) {
        return;
    }
    audioService->UpdateMuteControlSet(sessionId, true);

    sptr<AudioProcessInServer> audioprocess = AudioProcessInServer::Create(processConfig, audioService.get());
    audioService->CheckFastSessionMuteState(sessionId, audioprocess);
}

void AudioServiceIsMuteSwitchStreamFuzzTest()
{
    AudioProcessConfig processConfig;
    uint32_t sessionId = GetData<uint32_t>();
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioService == nullptr) {
        return;
    }
    audioService->muteSwitchStreams_.insert(sessionId);
    audioService->IsMuteSwitchStream(sessionId);
}

void AudioServiceInsertRendererFuzzTest()
{
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioService == nullptr) {
        return;
    }
    uint32_t sessionId = GetData<uint32_t>();
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;

    audioService->InsertRenderer(sessionId, renderer);
}

void AudioServiceSaveForegroundListFuzzTest()
{
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioService == nullptr) {
        return;
    }
    std::vector<std::string> list;
    bool isTestError = GetData<bool>();
    if (isTestError) {
        list.assign(SAVE_FOREGROUND_LIST_NUM, "example_string");
    } else {
        list.push_back("test_string");
    }

    audioService->SaveForegroundList(list);
}

void AudioServiceMatchForegroundListFuzzTest()
{
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioService == nullptr) {
        return;
    }
    uint32_t uid = GetData<uint32_t>();
    string bundleName = "test_bundle";
    audioService->foregroundSet_.insert(bundleName);

    audioService->MatchForegroundList(bundleName, uid);
    audioService->InForegroundList(uid);
}

void AudioServiceUpdateForegroundStateFuzzTest()
{
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioService == nullptr) {
        return;
    }
    std::string dumpString = "test_dump_string";
    audioService->foregroundSet_.insert("_success");
    audioService->DumpForegroundList(dumpString);

    uint32_t appTokenId = GetData<uint32_t>();
    bool isActive = GetData<bool>();
    audioService->UpdateForegroundState(appTokenId, isActive);
}

void AudioServiceRemoveRendererFuzzTest()
{
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioService == nullptr) {
        return;
    }
    uint32_t sessionId = GetData<uint32_t>();
    audioService->allRendererMap_.clear();
    audioService->allCapturerMap_.insert(make_pair(
        sessionId, std::make_shared<CapturerInServer>(AudioProcessConfig(), std::weak_ptr<IStreamListener>())));
    audioService->RemoveRenderer(sessionId);
}

void AudioServiceInsertCapturerFuzzTest()
{
    uint32_t sessionId = GetData<uint32_t>();
    AudioProcessConfig processConfig;

    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<CapturerInServer> capturerInServer =
        std::make_shared<CapturerInServer>(processConfig, streamListener);
    std::shared_ptr<CapturerInServer> capturer = capturerInServer;
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioService == nullptr) {
        return;
    }
    audioService->InsertCapturer(sessionId, capturer);
}

void AudioServiceAddFilteredRenderFuzzTest()
{
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioService == nullptr) {
        return;
    }
    audioService->filteredRendererMap_.clear();

    int32_t innerCapId = GetData<int32_t>();
    std::shared_ptr<RendererInServer> renderer = nullptr;
    audioService->AddFilteredRender(innerCapId, renderer);
    audioService->filteredRendererMap_.clear();
}

void AudioServiceShouldBeInnerCapFuzzTest()
{
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioService == nullptr) {
        return;
    }

    AudioProcessConfig rendererConfig;
    rendererConfig.privacyType = static_cast<AudioPrivacyType>(GetData<uint32_t>() % NUM_2);
    std::set<int32_t> beCapIds;
    audioService->ShouldBeInnerCap(rendererConfig, beCapIds);
}

void AudioServiceCheckDisableFastInnerFuzzTest()
{
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioService == nullptr) {
        return;
    }

    AudioProcessConfig clientConfig = {};
    std::shared_ptr<AudioEndpointInner> endpoint = std::make_shared<AudioEndpointInner>(AudioEndpoint::TYPE_VOIP_MMAP,
        GetData<uint64_t>(), clientConfig.audioMode);
    audioService->CheckDisableFastInner(endpoint);
}

void AudioServiceFilterAllFastProcessFuzzTest()
{
    static const vector<DeviceRole> g_testDeviceTypes = {
        DEVICE_ROLE_NONE,
        INPUT_DEVICE,
        OUTPUT_DEVICE,
        DEVICE_ROLE_MAX,
    };
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioService == nullptr) {
        return;
    }
    AudioProcessConfig config = {};
    config.audioMode = static_cast<AudioMode>(GetData<uint32_t>() % NUM_2);
    sptr<AudioProcessInServer> audioprocess =  AudioProcessInServer::Create(config, audioService.get());
    AudioProcessConfig clientConfig = {};
    std::shared_ptr<AudioEndpointInner> endpoint = std::make_shared<AudioEndpointInner>(AudioEndpoint::TYPE_VOIP_MMAP,
        GetData<uint64_t>(), clientConfig.audioMode);
    endpoint->deviceInfo_.deviceRole_ = g_testDeviceTypes[GetData<uint32_t>() % g_testDeviceTypes.size()];
    audioService->linkedPairedList_.clear();
    audioService->linkedPairedList_.push_back(std::make_pair(audioprocess, endpoint));

    audioService->endpointList_.clear();
    audioService->endpointList_.insert(std::make_pair("endpoint", endpoint));
    audioService->FilterAllFastProcess();
}

void AudioServiceHandleFastCaptureFuzzTest()
{
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioService == nullptr) {
        return;
    }
    audioService->filteredRendererMap_.clear();

    std::set<int32_t> captureIds = {1};
    AudioProcessConfig config = {};
    sptr<AudioProcessInServer> audioprocess =  AudioProcessInServer::Create(config, audioService.get());

    AudioProcessConfig clientConfig = {};
    std::shared_ptr<AudioEndpointInner> endpoint = std::make_shared<AudioEndpointInner>(AudioEndpoint::TYPE_VOIP_MMAP,
        GetData<uint64_t>(), clientConfig.audioMode);

    audioService->HandleFastCapture(captureIds, audioprocess, endpoint);
}

void AudioServiceOnUpdateInnerCapListFuzzTest()
{
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioService == nullptr) {
        return;
    }
    bool isTestError = GetData<bool>();
    std::shared_ptr<RendererInServer> renderer;
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    if (isTestError) {
        renderer = nullptr;
    } else {
        renderer = std::make_shared<RendererInServer>(processConfig, streamListener);
    }
    std::vector<std::weak_ptr<RendererInServer>> rendererVector;
    rendererVector.push_back(renderer);
    int32_t innerCapId = GetData<int32_t>();
    audioService->filteredRendererMap_.clear();
    audioService->endpointList_.clear();
    audioService->filteredRendererMap_.insert(std::make_pair(innerCapId, rendererVector));
    audioService->OnUpdateInnerCapList(innerCapId);
}

void AudioServiceEnableDualToneListFuzzTest()
{
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioService == nullptr) {
        return;
    }
    bool isTestError = GetData<bool>();
    std::shared_ptr<RendererInServer> renderer;
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    if (isTestError) {
        renderer = nullptr;
    } else {
        renderer = std::make_shared<RendererInServer>(processConfig, streamListener);
    }

    int32_t sessionId = GetData<int32_t>();
    audioService->allRendererMap_.clear();
    audioService->allRendererMap_.insert(std::make_pair(sessionId, renderer));
    audioService->EnableDualStream(sessionId, "Speaker");
}

void AudioServiceDisableDualToneListFuzzTest()
{
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioService == nullptr) {
        return;
    }
    bool isTestError = GetData<bool>();
    std::shared_ptr<RendererInServer> renderer;
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    if (isTestError) {
        renderer = nullptr;
    } else {
        renderer = std::make_shared<RendererInServer>(processConfig, streamListener);
    }

    int32_t sessionId = GetData<int32_t>();
    audioService->DisableDualStream(sessionId);
}

void AudioServiceNotifyStreamVolumeChangedFuzzTest()
{
    AudioProcessConfig config = {};
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    AudioStreamInfo audioStreamInfo = { SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO, CH_LAYOUT_STEREO };
    deviceInfo.networkId_ = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpoint> audioEndpointPtr = AudioEndpoint::CreateEndpoint(
        AudioEndpoint::TYPE_MMAP, 0, config, deviceInfo, audioStreamInfo);
    AudioProcessConfig configProcess = {};
    sptr<AudioProcessInServer> audioProcess =  AudioProcessInServer::Create(configProcess,
        AudioService::GetInstance());
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioEndpointPtr == nullptr || audioService == nullptr) {
        return;
    }
    float volume = GetData<float>();

    audioService->endpointList_.insert(make_pair("testendpoint", audioEndpointPtr));
    AudioStreamType streamType = g_testAudioStreamTypes[GetData<uint32_t>() % g_testAudioStreamTypes.size()];
    audioService->NotifyStreamVolumeChanged(streamType, volume);
}

void AudioServiceDumpFuzzTest()
{
    AudioProcessConfig config = {};
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    AudioStreamInfo audioStreamInfo = { SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO, CH_LAYOUT_STEREO };
    deviceInfo.networkId_ = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpoint> audioEndpointPtr = AudioEndpoint::CreateEndpoint(
        AudioEndpoint::TYPE_MMAP, 0, config, deviceInfo, audioStreamInfo);
    AudioProcessConfig configProcess = {};
    sptr<AudioProcessInServer> audioProcess =  AudioProcessInServer::Create(configProcess,
        AudioService::GetInstance());
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioProcess == nullptr || audioEndpointPtr == nullptr || audioService == nullptr) {
        return;
    }
    std::string dumpString = "abcdefg";
    AudioPlaybackCaptureConfig playbackCaptureConfig;
    audioService->workingConfigs_.insert(make_pair(GetData<int32_t>(), playbackCaptureConfig));
    config.audioMode = static_cast<AudioMode>(GetData<uint32_t>() % NUM_2);
    std::shared_ptr<AudioEndpointInner> endpointInner = std::make_shared<AudioEndpointInner>(
        AudioEndpoint::TYPE_VOIP_MMAP, GetData<uint64_t>(), config.audioMode);
    audioService->linkedPairedList_.clear();
    audioService->linkedPairedList_.push_back(std::make_pair(audioProcess, endpointInner));
    audioService->endpointList_.clear();
    audioService->endpointList_.insert(make_pair("testendpoint", audioEndpointPtr));
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> renderer = std::make_shared<RendererInServer>(config, streamListener);
    audioService->allRendererMap_.insert(std::make_pair(GetData<int32_t>(), renderer));
    audioService->Dump(dumpString);
}

void AudioServiceGetCreatedAudioStreamMostUidFuzzTest()
{
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    if (audioService == nullptr) {
        return;
    }
    int32_t mostAppUid = GetData<int32_t>();
    int32_t mostAppNum = GetData<int32_t>();
    audioService->appUseNumMap_.clear();
    audioService->appUseNumMap_.insert(make_pair(mostAppUid, mostAppNum));
    audioService->GetCreatedAudioStreamMostUid(mostAppUid, mostAppNum);
}

void AudioServiceGetEndPointByTypeFuzzTest()
{
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    CHECK_AND_RETURN(audioService != nullptr);

    auto type = static_cast<AudioEndpoint::EndpointType>(GetData<uint32_t>() % ENDPOINTTYPESIZE);

    audioService->GetEndPointByType(type);
}

void AudioServiceHandleProcessInserverDualStreamEnableInnerFuzzTest()
{
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    CHECK_AND_RETURN(audioService != nullptr);
    AudioProcessConfig config = {};
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    AudioStreamInfo audioStreamInfo = { SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO, CH_LAYOUT_STEREO };
    deviceInfo.networkId_ = LOCAL_NETWORK_ID;
    std::shared_ptr<AudioEndpoint> audioEndpointPtr = AudioEndpoint::CreateEndpoint(
        AudioEndpoint::TYPE_MMAP, 0, config, deviceInfo, audioStreamInfo);
    CHECK_AND_RETURN(audioEndpointPtr != nullptr);

    std::string dupSinkName;
    for (size_t i = 0; i < GetData<int32_t>() % MAX_RANDOM_STRING_LENGTH; ++i) {
        dupSinkName += GetData<char>();
    }

    audioService->HandleProcessInserverDualStreamEnableInner(*audioEndpointPtr, dupSinkName);
}

void AudioServiceInitAllDupBufferFuzzTest()
{
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    CHECK_AND_RETURN(audioService != nullptr);

    auto innerCapId = GetData<int32_t>();

    audioService->InitAllDupBuffer(innerCapId);
}

void AudioServiceForceStopAudioStreamFuzzTest()
{
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    CHECK_AND_RETURN(audioService != nullptr);

    auto audioType = static_cast<StopAudioType>(GetData<int32_t>() % MAX_STOP_AUDIO_TYPE);

    (void)audioService->ForceStopAudioStream(audioType);
}

void AudioServiceSetLatestMuteStateFuzzTest()
{
    shared_ptr<AudioService> audioService = make_shared<AudioService>();
    CHECK_AND_RETURN(audioService != nullptr);

    auto sessionId = GetData<uint32_t>();
    bool muteFlag = GetData<int32_t>() % NUM_2;

    audioService->SetLatestMuteState(sessionId, muteFlag);
}

void AudioThreadTaskFuzzTest()
{
    std::unique_ptr<AudioThreadTask> audioThreadTask;
    audioThreadTask = std::make_unique<AudioThreadTask>(THREAD_NAME);
    CHECK_AND_RETURN(audioThreadTask != nullptr);
    auto myJob = []() {
        AUDIO_INFO_LOG("Hello Fuzz Test!");
    };
    audioThreadTask->RegisterJob(std::move(myJob));
    audioThreadTask->Start();
    audioThreadTask->CheckThreadIsRunning();
    audioThreadTask->Pause();
    audioThreadTask->Start();
    audioThreadTask->PauseAsync();
    audioThreadTask->Start();
    audioThreadTask->StopAsync();
    audioThreadTask->Start();
    audioThreadTask->Stop();
}

TestPtr g_testPtrs[] = {
#ifdef HAS_FEATURE_INNERCAPTURER
    AudioServiceCheckInnerCapForRendererFuzzTest,
    AudioServiceCheckInnerCapForProcessFuzzTest,
    AudioServiceLinkProcessToEndpointFuzzTest,
    AudioServiceUnlinkProcessToEndpointFuzzTest,
    AudioServiceGetDeviceInfoForProcessFuzzTest,
    AudioServiceGetMaxAmplitudeFuzzTest,
    AudioServiceGetCapturerBySessionIDFuzzTest,
    AudioServiceSetOffloadModeFuzzTest,
#endif
    AudioServiceGetReleaseDelayTimeFuzzTest,
    AudioServiceRemoveIdFromMuteControlSetFuzzTest,
    AudioServiceCheckRenderSessionMuteStateFuzzTest,
    AudioServiceCheckCaptureSessionMuteStateFuzzTest,
    AudioServiceCheckFastSessionMuteStateFuzzTest,
    AudioServiceIsMuteSwitchStreamFuzzTest,
    AudioServiceInsertRendererFuzzTest,
    AudioServiceSaveForegroundListFuzzTest,
    AudioServiceMatchForegroundListFuzzTest,
    AudioServiceUpdateForegroundStateFuzzTest,
    AudioServiceRemoveRendererFuzzTest,
    AudioServiceInsertCapturerFuzzTest,
    AudioServiceAddFilteredRenderFuzzTest,
    AudioServiceShouldBeInnerCapFuzzTest,
    AudioServiceCheckDisableFastInnerFuzzTest,
    AudioServiceFilterAllFastProcessFuzzTest,
    AudioServiceHandleFastCaptureFuzzTest,
    AudioServiceOnUpdateInnerCapListFuzzTest,
    AudioServiceEnableDualToneListFuzzTest,
    AudioServiceDisableDualToneListFuzzTest,
    AudioServiceNotifyStreamVolumeChangedFuzzTest,
    AudioServiceDumpFuzzTest,
    AudioServiceGetCreatedAudioStreamMostUidFuzzTest,
    AudioServiceHandleProcessInserverDualStreamEnableInnerFuzzTest,
    AudioServiceInitAllDupBufferFuzzTest,
    AudioServiceForceStopAudioStreamFuzzTest,
    AudioServiceSetLatestMuteStateFuzzTest,
#ifdef SUPPORT_LOW_LATENCY
    AudioServiceGetEndPointByTypeFuzzTest,
#endif
};

bool FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return false;
    }

    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;

    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testPtrs);
    if (len > 0) {
        g_testPtrs[code % len]();
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }
    return true;
}

} // namespace AudioStandard
} // namesapce OHOS

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
#ifdef HAS_FEATURE_INNERCAPTURER
    OHOS::AudioStandard::AudioServiceOnProcessReleaseFuzzTest();
#endif
    OHOS::AudioStandard::AudioThreadTaskFuzzTest();
    return 0;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}