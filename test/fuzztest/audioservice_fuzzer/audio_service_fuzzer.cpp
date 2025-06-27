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

using namespace std;

namespace OHOS {
namespace AudioStandard {

const int32_t NUM_2 = 2;
const int32_t AUDIOCHANNELSIZE = 17;
const int32_t ENDPOINTTYPESIZE = 4;
const int32_t SAVE_FOREGROUND_LIST_NUM = 11;
typedef void (*TestPtr)(const uint8_t *, size_t);

const vector<AudioStreamType> g_testAudioStreamTypes = {
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

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

#ifdef HAS_FEATURE_INNERCAPTURER
void AudioServiceOnProcessReleaseFuzzTest(const uint8_t *rawData, size_t size)
{
    static const vector<AudioEncodingType> testAudioEncodingTypes = {
        ENCODING_INVALID,
        ENCODING_PCM,
        ENCODING_AUDIOVIVID,
        ENCODING_EAC3,
    };
    static uint32_t step = 0;
    step += size;
    AudioProcessConfig config = {};
    uint32_t index = static_cast<uint32_t>(size);
    config.privacyType = static_cast<AudioPrivacyType>(index % NUM_2);
    config.audioMode = static_cast<AudioMode>(step % NUM_2);
    config.streamType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    config.streamInfo.channels = static_cast<AudioChannel>(index % AUDIOCHANNELSIZE);
    config.streamInfo.samplingRate = g_testAudioSamplingRates[index % g_testAudioSamplingRates.size()];
    config.streamInfo.format = g_testAudioSampleFormats[index % g_testAudioSampleFormats.size()];
    config.streamInfo.encoding = testAudioEncodingTypes[index % testAudioEncodingTypes.size()];
    auto audioProcess = AudioService::GetInstance()->GetAudioProcess(config);
    bool isSwitchStream = static_cast<bool>(step % NUM_2);
    AudioService::GetInstance()->OnProcessRelease(audioProcess, isSwitchStream);
}

void AudioServiceCheckInnerCapForRendererFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioService *audioService = AudioService::GetInstance();
    AudioProcessConfig processConfig;
    uint32_t sessionId = static_cast<uint32_t>(size);
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
    audioService->CheckInnerCapForRenderer(sessionId, renderer);
}

#endif // HAS_FEATURE_INNERCAPTURER

void AudioServiceReleaseProcessFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioService *audioService = AudioService::GetInstance();

    std::string endpointName = "invalid_endpoint";
    int32_t delayTime = static_cast<int32_t>(size);
    audioService->ReleaseProcess(endpointName, delayTime);
}

void AudioServiceGetReleaseDelayTimeFuzzTest(const uint8_t *rawData, size_t size)
{
    static uint32_t step = 0;
    step += size;
    uint32_t index = static_cast<uint32_t>(size);
    AudioService *audioService = AudioService::GetInstance();

    AudioProcessConfig clientConfig = {};
    std::shared_ptr<AudioEndpointInner> endpoint = std::make_shared<AudioEndpointInner>(
        static_cast<AudioEndpoint::EndpointType>(index % ENDPOINTTYPESIZE), static_cast<uint64_t>(size), clientConfig);
    endpoint->deviceInfo_.deviceType_ = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    bool isSwitchStream = static_cast<bool>(index % NUM_2);
    bool isRecord = static_cast<bool>(step % NUM_2);
    audioService->GetReleaseDelayTime(endpoint, isSwitchStream, isRecord);
}

void AudioServiceRemoveIdFromMuteControlSetFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t sessionId = static_cast<uint32_t>(size);
    AudioService::GetInstance()->RemoveIdFromMuteControlSet(sessionId);
}

void AudioServiceCheckRenderSessionMuteStateFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioProcessConfig processConfig;
    uint32_t sessionId = static_cast<uint32_t>(size);
    AudioService *audioService = AudioService::GetInstance();
    audioService->UpdateMuteControlSet(sessionId, true);

    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;
    audioService->CheckRenderSessionMuteState(sessionId, renderer);
}

void AudioServiceCheckCaptureSessionMuteStateFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioProcessConfig processConfig;
    uint32_t sessionId = static_cast<uint32_t>(size);
    AudioService *audioService = AudioService::GetInstance();
    audioService->UpdateMuteControlSet(sessionId, true);

    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<CapturerInServer> capturerInServer =
        std::make_shared<CapturerInServer>(processConfig, streamListener);
    std::shared_ptr<CapturerInServer> capturer = capturerInServer;
    audioService->CheckCaptureSessionMuteState(sessionId, capturer);
}

void AudioServiceCheckFastSessionMuteStateFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioProcessConfig processConfig;
    uint32_t sessionId = static_cast<uint32_t>(size);
    AudioService *audioService = AudioService::GetInstance();
    audioService->UpdateMuteControlSet(sessionId, true);

    sptr<AudioProcessInServer> audioprocess = AudioProcessInServer::Create(processConfig, AudioService::GetInstance());
    audioService->CheckFastSessionMuteState(sessionId, audioprocess);
}

void AudioServiceIsMuteSwitchStreamFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioProcessConfig processConfig;
    uint32_t sessionId = static_cast<uint32_t>(size);
    AudioService *audioService = AudioService::GetInstance();
    audioService->muteSwitchStreams_.insert(sessionId);
    audioService->IsMuteSwitchStream(sessionId);
}

void AudioServiceInsertRendererFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioService *audioService = AudioService::GetInstance();
    uint32_t sessionId = static_cast<uint32_t>(size);
    AudioProcessConfig processConfig;
    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<RendererInServer> rendererInServer =
        std::make_shared<RendererInServer>(processConfig, streamListener);
    std::shared_ptr<RendererInServer> renderer = rendererInServer;

    audioService->InsertRenderer(sessionId, renderer);
}

void AudioServiceSaveForegroundListFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioService *audioService = AudioService::GetInstance();
    std::vector<std::string> list;
    bool isTestError = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    if (isTestError) {
        list.assign(SAVE_FOREGROUND_LIST_NUM, "example_string");
    } else {
        list.push_back("test_string");
    }

    audioService->SaveForegroundList(list);
}

void AudioServiceMatchForegroundListFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioService *audioService = AudioService::GetInstance();
    uint32_t uid = static_cast<uint32_t>(size);
    string bundleName = "test_bundle";
    audioService->foregroundSet_.insert(bundleName);

    audioService->MatchForegroundList(bundleName, uid);
    audioService->InForegroundList(uid);
}

void AudioServiceUpdateForegroundStateFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioService *audioService = AudioService::GetInstance();
    uint32_t appTokenId = static_cast<uint32_t>(size);
    bool isActive = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);

    audioService->UpdateForegroundState(appTokenId, isActive);
}

void AudioServiceDumpForegroundListFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioService *audioService = AudioService::GetInstance();
    std::string dumpString = "test_dump_string";
    audioService->foregroundSet_.insert("_success");
    audioService->DumpForegroundList(dumpString);
}

void AudioServiceRemoveRendererFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioService *audioService = AudioService::GetInstance();
    uint32_t sessionId = static_cast<uint32_t>(size);
    audioService->allRendererMap_.clear();
    audioService->allCapturerMap_.insert(make_pair(
        sessionId, std::make_shared<CapturerInServer>(AudioProcessConfig(), std::weak_ptr<IStreamListener>())));
    audioService->RemoveRenderer(sessionId);
}

void AudioServiceInsertCapturerFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t sessionId = static_cast<uint32_t>(size);
    AudioProcessConfig processConfig;

    std::shared_ptr<StreamListenerHolder> streamListenerHolder =
        std::make_shared<StreamListenerHolder>();
    std::weak_ptr<IStreamListener> streamListener = streamListenerHolder;
    std::shared_ptr<CapturerInServer> capturerInServer =
        std::make_shared<CapturerInServer>(processConfig, streamListener);
    std::shared_ptr<CapturerInServer> capturer = capturerInServer;
    AudioService *audioService = AudioService::GetInstance();
    audioService->InsertCapturer(sessionId, capturer);
}

void AudioServiceAddFilteredRenderFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioService *audioService = AudioService::GetInstance();
    audioService->filteredRendererMap_.clear();

    int32_t innerCapId = static_cast<int32_t>(size);
    std::shared_ptr<RendererInServer> renderer = nullptr;
    audioService->AddFilteredRender(innerCapId, renderer);
    audioService->filteredRendererMap_.clear();
}

void AudioServiceShouldBeInnerCapFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioService *audioService = AudioService::GetInstance();

    AudioProcessConfig rendererConfig;
    rendererConfig.privacyType = static_cast<AudioPrivacyType>(uint32_t(size) % NUM_2);
    std::set<int32_t> beCapIds;
    audioService->ShouldBeInnerCap(rendererConfig, beCapIds);
}

void AudioServiceCheckDisableFastInnerFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioService *audioService = AudioService::GetInstance();

    AudioProcessConfig clientConfig = {};
    std::shared_ptr<AudioEndpointInner> endpoint = std::make_shared<AudioEndpointInner>(AudioEndpoint::TYPE_VOIP_MMAP,
        static_cast<uint64_t>(size), clientConfig);
    audioService->CheckDisableFastInner(endpoint);
}

void AudioServiceFilterAllFastProcessFuzzTest(const uint8_t *rawData, size_t size)
{
    static const vector<DeviceRole> g_testDeviceTypes = {
        DEVICE_ROLE_NONE,
        INPUT_DEVICE,
        OUTPUT_DEVICE,
        DEVICE_ROLE_MAX,
    };
    AudioService *audioService = AudioService::GetInstance();

    AudioProcessConfig config = {};
    config.audioMode = static_cast<AudioMode>(static_cast<uint32_t>(size) % NUM_2);
    sptr<AudioProcessInServer> audioprocess =  AudioProcessInServer::Create(config, AudioService::GetInstance());

    AudioProcessConfig clientConfig = {};
    std::shared_ptr<AudioEndpointInner> endpoint = std::make_shared<AudioEndpointInner>(AudioEndpoint::TYPE_VOIP_MMAP,
        static_cast<uint64_t>(size), clientConfig);
    endpoint->deviceInfo_.deviceRole_ = g_testDeviceTypes[static_cast<uint32_t>(size) % g_testDeviceTypes.size()];

    audioService->linkedPairedList_.clear();
    audioService->linkedPairedList_.push_back(std::make_pair(audioprocess, endpoint));

    audioService->endpointList_.clear();
    audioService->endpointList_.insert(std::make_pair("endpoint", endpoint));
    audioService->FilterAllFastProcess();
}

void AudioServiceHandleFastCaptureFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioService *audioService = AudioService::GetInstance();
    audioService->filteredRendererMap_.clear();

    std::set<int32_t> captureIds = {1};
    AudioProcessConfig config = {};
    sptr<AudioProcessInServer> audioprocess =  AudioProcessInServer::Create(config, AudioService::GetInstance());

    AudioProcessConfig clientConfig = {};
    std::shared_ptr<AudioEndpointInner> endpoint = std::make_shared<AudioEndpointInner>(AudioEndpoint::TYPE_VOIP_MMAP,
        static_cast<uint64_t>(size), clientConfig);

    audioService->HandleFastCapture(captureIds, audioprocess, endpoint);
}

void AudioServiceOnUpdateInnerCapListFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioService *audioService = AudioService::GetInstance();
    bool isTestError = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
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
    int32_t innerCapId = static_cast<int32_t>(size);
    audioService->filteredRendererMap_.clear();
    audioService->filteredRendererMap_.insert(std::make_pair(innerCapId, rendererVector));
    audioService->OnUpdateInnerCapList(innerCapId);
}

void AudioServiceEnableDualToneListFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioService *audioService = AudioService::GetInstance();
    bool isTestError = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
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

    int32_t sessionId = static_cast<int32_t>(size);
    audioService->allRendererMap_.clear();
    audioService->allRendererMap_.insert(std::make_pair(sessionId, renderer));
    audioService->EnableDualToneList(sessionId);
}


} // namespace AudioStandard
} // namesapce OHOS

OHOS::AudioStandard::TestPtr g_testPtrs[] = {
#ifdef HAS_FEATURE_INNERCAPTURER
    OHOS::AudioStandard::AudioServiceOnProcessReleaseFuzzTest,
    OHOS::AudioStandard::AudioServiceCheckInnerCapForRendererFuzzTest,
#endif
    OHOS::AudioStandard::AudioServiceReleaseProcessFuzzTest,
    OHOS::AudioStandard::AudioServiceGetReleaseDelayTimeFuzzTest,
    OHOS::AudioStandard::AudioServiceRemoveIdFromMuteControlSetFuzzTest,
    OHOS::AudioStandard::AudioServiceCheckRenderSessionMuteStateFuzzTest,
    OHOS::AudioStandard::AudioServiceCheckCaptureSessionMuteStateFuzzTest,
    OHOS::AudioStandard::AudioServiceCheckFastSessionMuteStateFuzzTest,
    OHOS::AudioStandard::AudioServiceIsMuteSwitchStreamFuzzTest,
    OHOS::AudioStandard::AudioServiceInsertRendererFuzzTest,
    OHOS::AudioStandard::AudioServiceSaveForegroundListFuzzTest,
    OHOS::AudioStandard::AudioServiceMatchForegroundListFuzzTest,
    OHOS::AudioStandard::AudioServiceUpdateForegroundStateFuzzTest,
    OHOS::AudioStandard::AudioServiceDumpForegroundListFuzzTest,
    OHOS::AudioStandard::AudioServiceRemoveRendererFuzzTest,
    OHOS::AudioStandard::AudioServiceInsertCapturerFuzzTest,
    OHOS::AudioStandard::AudioServiceAddFilteredRenderFuzzTest,
    OHOS::AudioStandard::AudioServiceShouldBeInnerCapFuzzTest,
    OHOS::AudioStandard::AudioServiceCheckDisableFastInnerFuzzTest,
    OHOS::AudioStandard::AudioServiceFilterAllFastProcessFuzzTest,
    OHOS::AudioStandard::AudioServiceHandleFastCaptureFuzzTest,
    OHOS::AudioStandard::AudioServiceOnUpdateInnerCapListFuzzTest,
    OHOS::AudioStandard::AudioServiceEnableDualToneListFuzzTest,
};

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr || size <= 1) {
        return 0;
    }
    uint32_t len = OHOS::AudioStandard::GetArrLength(g_testPtrs);
    if (len > 0) {
        uint8_t firstByte = *data % len;
        if (firstByte >= len) {
            return 0;
        }
        data = data + 1;
        size = size - 1;
        g_testPtrs[firstByte](data, size);
    }
    return 0;
}