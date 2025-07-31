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

#include "audio_stream_collector.h"
#include "istandard_client_tracker.h"
#include "audio_client_tracker_callback_listener.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {

AudioStreamCollector audioStreamCollector_;
const int32_t NUM_2 = 2;
typedef void (*TestPtr)(const uint8_t *, size_t);

const vector<RendererState> g_testRendererState = {
    RENDERER_INVALID,
    RENDERER_NEW,
    RENDERER_PREPARED,
    RENDERER_RUNNING,
    RENDERER_STOPPED,
    RENDERER_RELEASED,
    RENDERER_PAUSED,
};

const vector<AudioPipeType> g_testPipeTypes = {
    PIPE_TYPE_UNKNOWN,
    PIPE_TYPE_NORMAL_OUT,
    PIPE_TYPE_NORMAL_IN,
    PIPE_TYPE_LOWLATENCY_OUT,
    PIPE_TYPE_LOWLATENCY_IN,
    PIPE_TYPE_DIRECT_OUT,
    PIPE_TYPE_DIRECT_IN,
    PIPE_TYPE_CALL_OUT,
    PIPE_TYPE_CALL_IN,
    PIPE_TYPE_OFFLOAD,
    PIPE_TYPE_MULTICHANNEL,
    PIPE_TYPE_HIGHRESOLUTION,
    PIPE_TYPE_SPATIALIZATION,
    PIPE_TYPE_DIRECT_MUSIC,
    PIPE_TYPE_DIRECT_VOIP,
};

const vector<StreamUsage> g_testStreamUsages = {
    STREAM_USAGE_INVALID,
    STREAM_USAGE_UNKNOWN,
    STREAM_USAGE_MEDIA,
    STREAM_USAGE_MUSIC,
    STREAM_USAGE_VOICE_COMMUNICATION,
    STREAM_USAGE_VOICE_ASSISTANT,
    STREAM_USAGE_ALARM,
    STREAM_USAGE_VOICE_MESSAGE,
    STREAM_USAGE_NOTIFICATION_RINGTONE,
    STREAM_USAGE_RINGTONE,
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
    STREAM_USAGE_VOICE_CALL_ASSISTANT,
    STREAM_USAGE_MAX,
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
    DEVICE_TYPE_MAX,
};

const vector<DeviceRole> g_testDeviceRoles = {
    DEVICE_ROLE_NONE,
    INPUT_DEVICE,
    OUTPUT_DEVICE,
    DEVICE_ROLE_MAX,
};

const vector<ContentType> g_testContentTypes = {
    CONTENT_TYPE_UNKNOWN,
    CONTENT_TYPE_SPEECH,
    CONTENT_TYPE_MUSIC,
    CONTENT_TYPE_MOVIE,
    CONTENT_TYPE_SONIFICATION,
    CONTENT_TYPE_RINGTONE,
    CONTENT_TYPE_PROMPT,
    CONTENT_TYPE_GAME,
    CONTENT_TYPE_DTMF,
    CONTENT_TYPE_ULTRASONIC,
};

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

const vector<SourceType> g_testSourceTypes = {
    SOURCE_TYPE_INVALID,
    SOURCE_TYPE_MIC,
    SOURCE_TYPE_VOICE_RECOGNITION,
    SOURCE_TYPE_PLAYBACK_CAPTURE,
    SOURCE_TYPE_WAKEUP,
    SOURCE_TYPE_VOICE_CALL,
    SOURCE_TYPE_VOICE_COMMUNICATION,
    SOURCE_TYPE_ULTRASONIC,
    SOURCE_TYPE_VIRTUAL_CAPTURE, // only for voice call
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

void AudioStreamCollectorAddRendererStreamFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioStreamChangeInfo streamChangeInfo;
    uint32_t randIntValue = static_cast<uint32_t>(size) % NUM_2;
    streamChangeInfo.audioRendererChangeInfo.clientUID = randIntValue;
    streamChangeInfo.audioRendererChangeInfo.sessionId = randIntValue++;
    streamChangeInfo.audioRendererChangeInfo.channelCount = randIntValue++;
    streamChangeInfo.audioRendererChangeInfo.createrUID = randIntValue--;
    uint32_t index = static_cast<uint32_t>(size) % g_testRendererState.size();
    streamChangeInfo.audioRendererChangeInfo.rendererState = g_testRendererState[index];
    index = static_cast<uint32_t>(size) % g_testPipeTypes.size();
    streamChangeInfo.audioRendererChangeInfo.rendererInfo.pipeType = g_testPipeTypes[index];
    audioStreamCollector_.AddRendererStream(streamChangeInfo);
}

void AudioStreamCollectorGetRendererStreamInfoFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioStreamChangeInfo streamChangeInfo;
    int32_t randIntValue = static_cast<int32_t>(size);
    streamChangeInfo.audioCapturerChangeInfo.clientUID = randIntValue;
    streamChangeInfo.audioCapturerChangeInfo.sessionId = randIntValue + 1;
    AudioRendererChangeInfo rendererInfo;
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();

    rendererChangeInfo->clientUID = randIntValue;
    rendererChangeInfo->createrUID = randIntValue;
    rendererChangeInfo->sessionId = randIntValue + 1;
    audioStreamCollector_.audioRendererChangeInfos_.clear();
    audioStreamCollector_.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));
    audioStreamCollector_.GetRendererStreamInfo(streamChangeInfo, rendererInfo);
}

void AudioStreamCollectorGetCapturerStreamInfoFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioStreamChangeInfo streamChangeInfo;
    int32_t randIntValue = static_cast<int32_t>(size);
    streamChangeInfo.audioCapturerChangeInfo.clientUID = randIntValue;
    streamChangeInfo.audioCapturerChangeInfo.sessionId = randIntValue + 1;
    AudioCapturerChangeInfo capturerChangeInfo;
    shared_ptr<AudioCapturerChangeInfo> rendererChangeInfo = make_shared<AudioCapturerChangeInfo>();

    rendererChangeInfo->clientUID = randIntValue;
    rendererChangeInfo->createrUID = randIntValue;
    rendererChangeInfo->sessionId = randIntValue + 1;
    audioStreamCollector_.audioCapturerChangeInfos_.clear();
    audioStreamCollector_.audioCapturerChangeInfos_.push_back(move(rendererChangeInfo));
    audioStreamCollector_.GetCapturerStreamInfo(streamChangeInfo, capturerChangeInfo);
}

void AudioStreamCollectorGetPipeTypeFuzzTest(const uint8_t *rawData, size_t size)
{
    int32_t randIntValue = static_cast<int32_t>(size);
    int32_t sessionId = randIntValue;
    uint32_t index = static_cast<uint32_t>(size) % g_testPipeTypes.size();
    AudioPipeType pipeType = g_testPipeTypes[index];
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->clientUID = randIntValue / NUM_2;
    rendererChangeInfo->createrUID = randIntValue / NUM_2;
    rendererChangeInfo->sessionId = randIntValue;
    audioStreamCollector_.audioRendererChangeInfos_.clear();
    audioStreamCollector_.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));
    audioStreamCollector_.GetPipeType(sessionId, pipeType);
}

void AudioStreamCollectorExistStreamForPipeFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size) % g_testPipeTypes.size();
    AudioPipeType pipeType = g_testPipeTypes[index];
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    AudioStreamChangeInfo streamChangeInfo;
    int32_t randIntValue = static_cast<int32_t>(size);
    streamChangeInfo.audioRendererChangeInfo.clientUID = randIntValue;
    streamChangeInfo.audioRendererChangeInfo.sessionId = randIntValue + 1;
    index = static_cast<uint32_t>(size) % g_testRendererState.size();
    streamChangeInfo.audioRendererChangeInfo.rendererState = g_testRendererState[index];

    bool result = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    if (result) {
        rendererChangeInfo->createrUID = streamChangeInfo.audioRendererChangeInfo.createrUID;
        rendererChangeInfo->clientUID = streamChangeInfo.audioRendererChangeInfo.clientUID;
        audioStreamCollector_.audioRendererChangeInfos_.clear();
        audioStreamCollector_.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));
        audioStreamCollector_.audioRendererChangeInfos_[0]->rendererInfo.pipeType = pipeType;
    }
    audioStreamCollector_.ExistStreamForPipe(pipeType);
}

void AudioStreamCollectorGetRendererDeviceInfoFuzzTest(const uint8_t *rawData, size_t size)
{
    int32_t randIntValue = static_cast<int32_t>(size);
    int32_t sessionId = randIntValue;
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();

    bool result = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    if (result) {
        rendererChangeInfo->clientUID = randIntValue;
        rendererChangeInfo->createrUID = randIntValue;
        rendererChangeInfo->sessionId = randIntValue + 1;
        uint32_t index = static_cast<uint32_t>(size) % g_testPipeTypes.size();
        rendererChangeInfo->rendererInfo.pipeType = g_testPipeTypes[index];
        audioStreamCollector_.audioRendererChangeInfos_.clear();
        audioStreamCollector_.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));
    }
    audioStreamCollector_.GetRendererDeviceInfo(sessionId, deviceInfo);
}

void AudioStreamCollectorAddCapturerStreamFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioStreamChangeInfo streamChangeInfo;
    int32_t randIntValue = static_cast<int32_t>(size) % NUM_2;
    streamChangeInfo.audioRendererChangeInfo.clientUID = randIntValue;
    streamChangeInfo.audioRendererChangeInfo.sessionId = randIntValue++;
    streamChangeInfo.audioRendererChangeInfo.channelCount = randIntValue++;
    streamChangeInfo.audioRendererChangeInfo.createrUID = randIntValue--;
    uint32_t index = static_cast<uint32_t>(size) % g_testRendererState.size();
    streamChangeInfo.audioRendererChangeInfo.rendererState = g_testRendererState[index];
    index = static_cast<uint32_t>(size) % g_testPipeTypes.size();
    streamChangeInfo.audioRendererChangeInfo.rendererInfo.pipeType = g_testPipeTypes[index];
    audioStreamCollector_.AddCapturerStream(streamChangeInfo);
}

void AudioStreamCollectorSendCapturerInfoEventFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioDeviceDescriptor inputDeviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    shared_ptr<AudioCapturerChangeInfo> captureChangeInfo = make_shared<AudioCapturerChangeInfo>();
    int32_t randIntValue = static_cast<int32_t>(size);
    captureChangeInfo->clientUID = randIntValue;
    captureChangeInfo->createrUID = randIntValue / NUM_2;
    captureChangeInfo->sessionId = randIntValue / NUM_2 + 1;
    captureChangeInfo->inputDeviceInfo = inputDeviceInfo;
    audioStreamCollector_.audioCapturerChangeInfos_.clear();
    audioStreamCollector_.audioCapturerChangeInfos_.push_back(captureChangeInfo);

    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    audioCapturerChangeInfos.push_back(captureChangeInfo);
    audioStreamCollector_.SendCapturerInfoEvent(audioCapturerChangeInfos);
}

void AudioStreamCollectorRegisterTrackerFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioMode audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    AudioStreamChangeInfo streamChangeInfo;
    int32_t randIntValue = static_cast<int32_t>(size);
    streamChangeInfo.audioRendererChangeInfo.clientUID = randIntValue / NUM_2;
    streamChangeInfo.audioRendererChangeInfo.sessionId = randIntValue;
    uint32_t index = static_cast<uint32_t>(size) % g_testRendererState.size();
    streamChangeInfo.audioRendererChangeInfo.rendererState = g_testRendererState[index];
    sptr<IRemoteObject> clientTrackerObj = nullptr;

    audioStreamCollector_.RegisterTracker(audioMode, streamChangeInfo, clientTrackerObj);
    audioStreamCollector_.UpdateTracker(audioMode, streamChangeInfo);
}

void AudioStreamCollectorSetRendererStreamParamFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioStreamChangeInfo streamChangeInfo;
    int32_t randIntValue = static_cast<int32_t>(size);
    streamChangeInfo.audioRendererChangeInfo.clientUID = randIntValue / NUM_2;
    streamChangeInfo.audioRendererChangeInfo.sessionId = randIntValue;
    uint32_t index = static_cast<uint32_t>(size) % g_testRendererState.size();
    streamChangeInfo.audioRendererChangeInfo.rendererState = g_testRendererState[index];
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();

    audioStreamCollector_.SetRendererStreamParam(streamChangeInfo, rendererChangeInfo);
}

void AudioStreamCollectorSetCapturerStreamParamFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioStreamChangeInfo streamChangeInfo;
    int32_t randIntValue = static_cast<int32_t>(size);
    streamChangeInfo.audioRendererChangeInfo.clientUID = randIntValue / NUM_2;
    streamChangeInfo.audioRendererChangeInfo.sessionId = randIntValue;
    uint32_t index = static_cast<uint32_t>(size) % g_testRendererState.size();
    streamChangeInfo.audioRendererChangeInfo.rendererState = g_testRendererState[index];
    shared_ptr<AudioCapturerChangeInfo> rendererChangeInfo = make_shared<AudioCapturerChangeInfo>();

    audioStreamCollector_.SetCapturerStreamParam(streamChangeInfo, rendererChangeInfo);
}

void AudioStreamCollectorResetRendererStreamDeviceInfoFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioDeviceDescriptor outputDeviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();

    int32_t randIntValue = static_cast<int32_t>(size);
    rendererChangeInfo->clientUID = randIntValue / NUM_2;
    rendererChangeInfo->createrUID = randIntValue / NUM_2;
    rendererChangeInfo->sessionId = randIntValue;
    uint32_t index = static_cast<uint32_t>(size) % g_testPipeTypes.size();
    rendererChangeInfo->rendererInfo.pipeType = g_testPipeTypes[index];
    audioStreamCollector_.audioRendererChangeInfos_.clear();
    audioStreamCollector_.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    audioStreamCollector_.ResetRendererStreamDeviceInfo(outputDeviceInfo);
}

void AudioStreamCollectorResetCapturerStreamDeviceInfoFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioDeviceDescriptor outputDeviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    shared_ptr<AudioCapturerChangeInfo> rendererChangeInfo = make_shared<AudioCapturerChangeInfo>();

    int32_t randIntValue = static_cast<int32_t>(size);
    rendererChangeInfo->clientUID = randIntValue / NUM_2;
    rendererChangeInfo->createrUID = randIntValue / NUM_2;
    rendererChangeInfo->sessionId = randIntValue;
    audioStreamCollector_.audioCapturerChangeInfos_.clear();
    audioStreamCollector_.audioCapturerChangeInfos_.push_back(move(rendererChangeInfo));
    audioStreamCollector_.ResetCapturerStreamDeviceInfo(outputDeviceInfo);
}

void AudioStreamCollectorCheckRendererStateInfoChangedFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioStreamChangeInfo streamChangeInfo;
    int32_t randIntValue = static_cast<int32_t>(size);
    streamChangeInfo.audioRendererChangeInfo.clientUID = randIntValue / NUM_2;
    streamChangeInfo.audioRendererChangeInfo.sessionId = randIntValue;
    uint32_t index = static_cast<uint32_t>(size) % g_testRendererState.size();
    streamChangeInfo.audioRendererChangeInfo.rendererState = g_testRendererState[index];
    audioStreamCollector_.CheckRendererStateInfoChanged(streamChangeInfo);
}

void AudioStreamCollectorCheckRendererInfoChangedFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioStreamChangeInfo streamChangeInfo;
    int32_t randIntValue = static_cast<int32_t>(size);
    streamChangeInfo.audioRendererChangeInfo.clientUID = randIntValue / NUM_2;
    streamChangeInfo.audioRendererChangeInfo.sessionId = randIntValue;
    uint32_t index = static_cast<uint32_t>(size) % g_testRendererState.size();
    streamChangeInfo.audioRendererChangeInfo.rendererState = g_testRendererState[index];
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();

    bool result = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    if (result) {
        rendererChangeInfo->createrUID = randIntValue / NUM_2;
        index = static_cast<uint32_t>(size) % g_testPipeTypes.size();
        rendererChangeInfo->rendererInfo.pipeType = g_testPipeTypes[index];
        audioStreamCollector_.audioRendererChangeInfos_.clear();
        audioStreamCollector_.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));
    }

    audioStreamCollector_.CheckRendererInfoChanged(streamChangeInfo);
}

void AudioStreamCollectorResetRingerModeMuteFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size) % g_testRendererState.size();
    RendererState rendererState = g_testRendererState[index];
    index = static_cast<uint32_t>(size) % g_testStreamUsages.size();
    StreamUsage streamUsage = g_testStreamUsages[index];
    audioStreamCollector_.ResetRingerModeMute(rendererState, streamUsage);
}

void AudioStreamCollectorUpdateRendererStreamInternalFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioStreamChangeInfo streamChangeInfo;
    int32_t randIntValue = static_cast<int32_t>(size);
    streamChangeInfo.audioRendererChangeInfo.clientUID = randIntValue / NUM_2;
    streamChangeInfo.audioRendererChangeInfo.sessionId = randIntValue;
    uint32_t index = static_cast<uint32_t>(size) % g_testRendererState.size();
    streamChangeInfo.audioRendererChangeInfo.rendererState = g_testRendererState[index];
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    bool result = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    if (result) {
        rendererChangeInfo->createrUID = randIntValue / NUM_2;
        index = static_cast<uint32_t>(size) % g_testPipeTypes.size();
        rendererChangeInfo->rendererInfo.pipeType = g_testPipeTypes[index];
        audioStreamCollector_.audioRendererChangeInfos_.clear();
        audioStreamCollector_.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));
    }

    audioStreamCollector_.UpdateRendererStreamInternal(streamChangeInfo);
}

void AudioStreamCollectorUpdateCapturerStreamInternalFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioStreamChangeInfo streamChangeInfo;
    int32_t randIntValue = static_cast<int32_t>(size);
    streamChangeInfo.audioCapturerChangeInfo.clientUID = randIntValue % NUM_2;
    streamChangeInfo.audioCapturerChangeInfo.sessionId = randIntValue;
    streamChangeInfo.audioCapturerChangeInfo.prerunningState = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    shared_ptr<AudioCapturerChangeInfo> capturerChangeInfo = make_shared<AudioCapturerChangeInfo>();

    bool result = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    if (result) {
        capturerChangeInfo->clientUID = randIntValue % NUM_2;
        capturerChangeInfo->sessionId = randIntValue;
        capturerChangeInfo->prerunningState = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
        audioStreamCollector_.audioCapturerChangeInfos_.clear();
        audioStreamCollector_.audioCapturerChangeInfos_.push_back(move(capturerChangeInfo));
    }

    audioStreamCollector_.UpdateCapturerStreamInternal(streamChangeInfo);
}

void AudioStreamCollectorUpdateTrackerFuzzTest(const uint8_t *rawData, size_t size)
{
    vector<AudioMode> audioModes = {
        AUDIO_MODE_PLAYBACK,
        AUDIO_MODE_RECORD,
    };
    uint32_t index = static_cast<uint32_t>(size) % audioModes.size();
    AudioMode audioMode = audioModes[index];
    AudioDeviceDescriptor audioDev(AudioDeviceDescriptor::DEVICE_INFO);
    audioStreamCollector_.UpdateTracker(audioMode, audioDev);

    int32_t randIntValue = static_cast<int32_t>(size);
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioRendererChangeInfo.clientUID = randIntValue / NUM_2;
    streamChangeInfo.audioRendererChangeInfo.sessionId = randIntValue;
    index = static_cast<uint32_t>(size) % g_testRendererState.size();
    streamChangeInfo.audioRendererChangeInfo.rendererState = g_testRendererState[index];
    sptr<IRemoteObject> clientTrackerObj = nullptr;

    audioStreamCollector_.RegisterTracker(audioMode, streamChangeInfo, clientTrackerObj);
    audioStreamCollector_.UpdateTracker(audioMode, streamChangeInfo);
}

void AudioStreamCollectorUpdateRendererDeviceInfoFuzzTest(const uint8_t *rawData, size_t size)
{
    shared_ptr<AudioDeviceDescriptor> outputDeviceInfoPtr = make_shared<AudioDeviceDescriptor>(
        AudioDeviceDescriptor::DEVICE_INFO);
    uint32_t index = static_cast<uint32_t>(size) % g_testDeviceTypes.size();
    outputDeviceInfoPtr->deviceType_ = g_testDeviceTypes[index];
    auto info1 = std::make_unique<AudioRendererChangeInfo>();
    info1->outputDeviceInfo.deviceType_ = g_testDeviceTypes[index / NUM_2];
    audioStreamCollector_.audioRendererChangeInfos_.clear();
    audioStreamCollector_.audioRendererChangeInfos_.push_back(std::move(info1));
    auto info2 = std::make_unique<AudioRendererChangeInfo>();
    info2->outputDeviceInfo.deviceType_ = g_testDeviceTypes[(index + 1) / NUM_2];
    audioStreamCollector_.audioRendererChangeInfos_.push_back(std::move(info2));
    audioStreamCollector_.UpdateRendererDeviceInfo(outputDeviceInfoPtr);

    AudioDeviceDescriptor outputDeviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    int32_t randIntValue = static_cast<int32_t>(size);
    int32_t clientUID = randIntValue / NUM_2;
    int32_t sessionId = randIntValue;
    rendererChangeInfo->clientUID = randIntValue / NUM_2;
    rendererChangeInfo->createrUID = randIntValue / NUM_2;
    rendererChangeInfo->sessionId = randIntValue;
    rendererChangeInfo->outputDeviceInfo = outputDeviceInfo;
    audioStreamCollector_.audioRendererChangeInfos_.clear();
    audioStreamCollector_.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));
    audioStreamCollector_.UpdateRendererDeviceInfo(clientUID, sessionId, outputDeviceInfo);
}

void AudioStreamCollectorUpdateCapturerDeviceInfoFuzzTest(const uint8_t *rawData, size_t size)
{
    shared_ptr<AudioDeviceDescriptor> inputDeviceInfoPtr = make_shared<AudioDeviceDescriptor>(
        AudioDeviceDescriptor::DEVICE_INFO);
    uint32_t index = static_cast<uint32_t>(size) % g_testDeviceTypes.size();
    inputDeviceInfoPtr->deviceType_ = g_testDeviceTypes[index];
    auto info1 = std::make_unique<AudioCapturerChangeInfo>();
    info1->inputDeviceInfo.deviceType_ = g_testDeviceTypes[index / NUM_2];
    audioStreamCollector_.audioCapturerChangeInfos_.clear();
    audioStreamCollector_.audioCapturerChangeInfos_.push_back(std::move(info1));
    auto info2 = std::make_unique<AudioCapturerChangeInfo>();
    info2->inputDeviceInfo.deviceType_ = g_testDeviceTypes[(index + 1) / NUM_2];
    audioStreamCollector_.audioCapturerChangeInfos_.push_back(std::move(info2));
    audioStreamCollector_.UpdateCapturerDeviceInfo(inputDeviceInfoPtr);

    AudioDeviceDescriptor inputDeviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    shared_ptr<AudioCapturerChangeInfo> captureChangeInfo = make_shared<AudioCapturerChangeInfo>();
    int32_t randIntValue = static_cast<int32_t>(size);
    captureChangeInfo->clientUID = randIntValue / NUM_2;
    captureChangeInfo->createrUID = randIntValue / NUM_2;
    captureChangeInfo->sessionId = randIntValue;
    captureChangeInfo->inputDeviceInfo = inputDeviceInfo;
    audioStreamCollector_.audioCapturerChangeInfos_.clear();
    audioStreamCollector_.audioCapturerChangeInfos_.push_back(move(captureChangeInfo));
    int32_t clientUID = randIntValue / NUM_2;
    int32_t sessionId = randIntValue;
    AudioDeviceDescriptor outputDeviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    index = static_cast<uint32_t>(size) % g_testDeviceRoles.size();
    outputDeviceInfo.deviceRole_ = g_testDeviceRoles[index];

    audioStreamCollector_.UpdateCapturerDeviceInfo(clientUID, sessionId, outputDeviceInfo);
}

void AudioStreamCollectorUpdateRendererPipeInfoFuzzTest(const uint8_t *rawData, size_t size)
{
    int32_t randIntValue = static_cast<int32_t>(size);
    int32_t sessionId = randIntValue;
    uint32_t index = static_cast<uint32_t>(size) % g_testPipeTypes.size();
    AudioPipeType normalPipe = g_testPipeTypes[index];
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();

    rendererChangeInfo->clientUID = randIntValue / NUM_2;
    rendererChangeInfo->createrUID = randIntValue / NUM_2;
    rendererChangeInfo->sessionId = randIntValue;
    rendererChangeInfo->rendererInfo.pipeType = g_testPipeTypes[index / NUM_2];
    audioStreamCollector_.audioRendererChangeInfos_.clear();
    audioStreamCollector_.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    audioStreamCollector_.UpdateRendererPipeInfo(sessionId, normalPipe);
}

void AudioStreamCollectorUpdateAppVolumeFuzzTest(const uint8_t *rawData, size_t size)
{
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    int32_t randIntValue = static_cast<int32_t>(size);
    rendererChangeInfo->clientUID = randIntValue / NUM_2;
    rendererChangeInfo->createrUID = randIntValue / NUM_2;
    rendererChangeInfo->sessionId = randIntValue;
    rendererChangeInfo->outputDeviceInfo = AudioDeviceDescriptor(AudioDeviceDescriptor::DEVICE_INFO);
    audioStreamCollector_.audioRendererChangeInfos_.clear();
    audioStreamCollector_.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    int32_t appUid = randIntValue / NUM_2;
    int32_t volume = randIntValue % NUM_2;
    audioStreamCollector_.UpdateAppVolume(appUid, volume);
}

void AudioStreamCollectorGetStreamTypeFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size) % g_testContentTypes.size();
    ContentType contentType = g_testContentTypes[index];
    index = static_cast<uint32_t>(size) % g_testStreamUsages.size();
    StreamUsage streamUsage = g_testStreamUsages[index];
    audioStreamCollector_.GetStreamType(contentType, streamUsage);

    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    int32_t randIntValue = static_cast<int32_t>(size);
    rendererChangeInfo->clientUID = randIntValue;
    rendererChangeInfo->createrUID = randIntValue;
    rendererChangeInfo->sessionId = randIntValue + 1;
    audioStreamCollector_.audioRendererChangeInfos_.clear();
    audioStreamCollector_.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));
    int32_t sessionId = randIntValue;
    audioStreamCollector_.GetStreamType(sessionId);
}

void AudioStreamCollectorGetSessionIdsOnRemoteDeviceByStreamUsageFuzzTest(const uint8_t *rawData, size_t size)
{
    vector<InterruptHint> testInterruptHints = {
        INTERRUPT_HINT_NONE,
        INTERRUPT_HINT_RESUME,
        INTERRUPT_HINT_PAUSE,
        INTERRUPT_HINT_STOP,
        INTERRUPT_HINT_DUCK,
        INTERRUPT_HINT_UNDUCK,
        INTERRUPT_HINT_MUTE,
        INTERRUPT_HINT_UNMUTE
    };
    uint32_t index = static_cast<uint32_t>(size);
    StreamUsage streamUsage = g_testStreamUsages[index % g_testStreamUsages.size()];
    DeviceType deviceType = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    DeviceRole role = g_testDeviceRoles[index % g_testDeviceRoles.size()];
    AudioDeviceDescriptor outputDeviceInfo(deviceType, role, 0, 0, "RemoteDevice");
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    int32_t randIntValue = static_cast<int32_t>(size);
    rendererChangeInfo->clientUID = randIntValue / NUM_2;
    rendererChangeInfo->createrUID = randIntValue / NUM_2;
    rendererChangeInfo->sessionId = randIntValue;
    rendererChangeInfo->outputDeviceInfo = outputDeviceInfo;
    rendererChangeInfo->rendererInfo.streamUsage = streamUsage;
    audioStreamCollector_.audioRendererChangeInfos_.clear();
    audioStreamCollector_.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));
    
    audioStreamCollector_.GetSessionIdsOnRemoteDeviceByStreamUsage(streamUsage);
    audioStreamCollector_.GetSessionIdsOnRemoteDeviceByDeviceType(deviceType);
}

void AudioStreamCollectorIsOffloadAllowedFuzzTest(const uint8_t *rawData, size_t size)
{
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    int32_t randIntValue = static_cast<int32_t>(size);
    int32_t sessionId = randIntValue / NUM_2;
    rendererChangeInfo->createrUID = randIntValue / NUM_2;
    rendererChangeInfo->clientUID = randIntValue / NUM_2;
    rendererChangeInfo->sessionId = randIntValue;
    audioStreamCollector_.audioRendererChangeInfos_.clear();
    audioStreamCollector_.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    audioStreamCollector_.IsOffloadAllowed(sessionId);
}

void AudioStreamCollectorGetChannelCountFuzzTest(const uint8_t *rawData, size_t size)
{
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    int32_t randIntValue = static_cast<int32_t>(size);
    int32_t sessionId = randIntValue / NUM_2;
    rendererChangeInfo->createrUID = randIntValue / NUM_2;
    rendererChangeInfo->clientUID = randIntValue / NUM_2;
    rendererChangeInfo->sessionId = randIntValue;
    audioStreamCollector_.audioRendererChangeInfos_.clear();
    audioStreamCollector_.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    audioStreamCollector_.GetChannelCount(sessionId);
}

void AudioStreamCollectorGetCurrentRendererChangeInfosFuzzTest(const uint8_t *rawData, size_t size)
{
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    std::vector<shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    int32_t randIntValue = static_cast<int32_t>(size);
    rendererChangeInfo->createrUID = randIntValue / NUM_2;
    rendererChangeInfo->clientUID = randIntValue / NUM_2;
    rendererChangeInfo->sessionId = randIntValue;
    audioStreamCollector_.audioRendererChangeInfos_.clear();
    audioStreamCollector_.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    audioStreamCollector_.GetCurrentRendererChangeInfos(rendererChangeInfos);
}

void AudioStreamCollectorGetCurrentCapturerChangeInfosFuzzTest(const uint8_t *rawData, size_t size)
{
    shared_ptr<AudioCapturerChangeInfo> rendererChangeInfo = make_shared<AudioCapturerChangeInfo>();
    std::vector<shared_ptr<AudioCapturerChangeInfo>> rendererChangeInfos;
    int32_t randIntValue = static_cast<int32_t>(size);
    rendererChangeInfo->createrUID = randIntValue / NUM_2;
    rendererChangeInfo->clientUID = randIntValue / NUM_2;
    rendererChangeInfo->sessionId = randIntValue;
    audioStreamCollector_.audioCapturerChangeInfos_.clear();
    audioStreamCollector_.audioCapturerChangeInfos_.push_back(move(rendererChangeInfo));
    audioStreamCollector_.GetCurrentCapturerChangeInfos(rendererChangeInfos);
}

void AudioStreamCollectorRegisteredTrackerClientDiedFuzzTest(const uint8_t *rawData, size_t size)
{
    int32_t randIntValue = static_cast<int32_t>(size);
    int32_t uid = randIntValue / NUM_2;
    int32_t pid = randIntValue / NUM_2;
    audioStreamCollector_.GetLastestRunningCallStreamUsage();
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();

    rendererChangeInfo->clientUID = randIntValue / NUM_2;
    rendererChangeInfo->createrUID = randIntValue / NUM_2;
    rendererChangeInfo->clientPid = randIntValue / NUM_2;
    rendererChangeInfo->sessionId = randIntValue;
    audioStreamCollector_.audioRendererChangeInfos_.clear();
    audioStreamCollector_.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));
    audioStreamCollector_.RegisteredTrackerClientDied(uid, pid);
}

void AudioStreamCollectorGetAndCompareStreamTypeFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size);
    StreamUsage targetUsage = g_testStreamUsages[index % g_testStreamUsages.size()];
    AudioRendererInfo rendererInfo;
    rendererInfo.contentType = g_testContentTypes[index % g_testContentTypes.size()];
    rendererInfo.streamUsage = g_testStreamUsages[index % g_testStreamUsages.size()];
    audioStreamCollector_.GetAndCompareStreamType(targetUsage, rendererInfo);
}

void AudioStreamCollectorGetUidFuzzTest(const uint8_t *rawData, size_t size)
{
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    int32_t randIntValue = static_cast<int32_t>(size);
    int32_t sessionId = randIntValue;

    rendererChangeInfo->createrUID = randIntValue / NUM_2;
    rendererChangeInfo->clientUID = randIntValue / NUM_2;
    rendererChangeInfo->sessionId = randIntValue;
    audioStreamCollector_.audioRendererChangeInfos_.clear();
    audioStreamCollector_.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));
    audioStreamCollector_.GetUid(sessionId);
}

void AudioStreamCollectorResumeStreamStateFuzzTest(const uint8_t *rawData, size_t size)
{
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    int32_t randIntValue = static_cast<int32_t>(size);
    rendererChangeInfo->clientUID = randIntValue / NUM_2;
    rendererChangeInfo->createrUID = randIntValue / NUM_2;
    rendererChangeInfo->sessionId = randIntValue;
    audioStreamCollector_.audioRendererChangeInfos_.clear();
    audioStreamCollector_.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));
    audioStreamCollector_.ResumeStreamState();
}

void AudioStreamCollectorUpdateStreamStateFuzzTest(const uint8_t *rawData, size_t size)
{
    vector<StreamSetState> testStreamSetState = {
        STREAM_PAUSE,
        STREAM_RESUME,
        STREAM_MUTE,
        STREAM_UNMUTE,
    };
    int32_t randIntValue = static_cast<int32_t>(size);
    int32_t clientUid = randIntValue / NUM_2;
    StreamSetStateEventInternal event;
    uint32_t index = static_cast<uint32_t>(size);
    event.streamSetState = testStreamSetState[index % testStreamSetState.size()];
    event.streamUsage = g_testStreamUsages[index % g_testStreamUsages.size()];
    auto changeInfo = std::make_unique<AudioRendererChangeInfo>();
    changeInfo->clientUID = clientUid;
    changeInfo->rendererInfo.streamUsage = g_testStreamUsages[index % g_testStreamUsages.size()];
    changeInfo->sessionId = randIntValue % NUM_2;
    audioStreamCollector_.audioRendererChangeInfos_.clear();
    audioStreamCollector_.audioRendererChangeInfos_.push_back(std::move(changeInfo));
    audioStreamCollector_.UpdateStreamState(clientUid, event);
}

void AudioStreamCollectorHandleAppStateChangeFuzzTest(const uint8_t *rawData, size_t size)
{
    int32_t randIntValue = static_cast<int32_t>(size);
    int32_t clientUid = randIntValue / NUM_2;
    int32_t clientPid = static_cast<int32_t>(size);
    uint32_t index = static_cast<uint32_t>(size);
    bool notifyMute = static_cast<bool>(index % NUM_2);
    auto changeInfo = std::make_unique<AudioRendererChangeInfo>();
    changeInfo->clientUID = clientUid;
    changeInfo->clientPid = clientPid;
    changeInfo->rendererInfo.streamUsage = g_testStreamUsages[index % g_testStreamUsages.size()];
    changeInfo->sessionId = randIntValue;
    changeInfo->backMute = static_cast<bool>(index % NUM_2);
    audioStreamCollector_.audioRendererChangeInfos_.clear();
    audioStreamCollector_.audioRendererChangeInfos_.push_back(std::move(changeInfo));
    bool hasBackTask = static_cast<int32_t>(size) % NUM_2;
    bool mute = static_cast<bool>(index % NUM_2);
    audioStreamCollector_.HandleAppStateChange(clientUid, clientPid, mute, notifyMute, hasBackTask);
}

void AudioStreamCollectorHandleFreezeStateChangeFuzzTest(const uint8_t *rawData, size_t size)
{
    int32_t randIntValue = static_cast<int32_t>(size);
    int32_t clientPid = randIntValue / NUM_2;
    uint32_t index = static_cast<uint32_t>(size);
    bool hasSession = static_cast<bool>(index % NUM_2);
    auto changeInfo = std::make_unique<AudioRendererChangeInfo>();
    changeInfo->clientPid = clientPid;
    changeInfo->rendererInfo.streamUsage = g_testStreamUsages[index % g_testStreamUsages.size()];
    changeInfo->sessionId = randIntValue / NUM_2;
    audioStreamCollector_.audioRendererChangeInfos_.clear();
    audioStreamCollector_.audioRendererChangeInfos_.push_back(std::move(changeInfo));
    audioStreamCollector_.HandleFreezeStateChange(clientPid, static_cast<bool>(index % NUM_2), hasSession);
}

void AudioStreamCollectorHandleBackTaskStateChangeFuzzTest(const uint8_t *rawData, size_t size)
{
    static uint32_t stepSize = 0;
    int32_t randIntValue = static_cast<int32_t>(size);
    int32_t clientUid = randIntValue / NUM_2;
    uint32_t index = static_cast<uint32_t>(size);
    bool hasSession = static_cast<bool>(index % NUM_2);
    auto changeInfo = std::make_unique<AudioRendererChangeInfo>();
    changeInfo->clientUID = clientUid;
    changeInfo->rendererInfo.streamUsage = g_testStreamUsages[index % g_testStreamUsages.size()];
    changeInfo->sessionId = randIntValue / NUM_2;

    changeInfo->backMute = static_cast<bool>((index + stepSize++) % NUM_2);
    audioStreamCollector_.audioRendererChangeInfos_.clear();
    audioStreamCollector_.audioRendererChangeInfos_.push_back(std::move(changeInfo));
    audioStreamCollector_.HandleBackTaskStateChange(clientUid, hasSession);
}

void AudioStreamCollectorHandleStartStreamMuteStateFuzzTest(const uint8_t *rawData, size_t size)
{
    int32_t randIntValue = static_cast<int32_t>(size);
    int32_t clientUid = randIntValue;
    int32_t createrUID = randIntValue;
    int32_t clientPid = randIntValue;
    uint32_t index = static_cast<uint32_t>(size);
    bool mute = static_cast<bool>(index % NUM_2);
    auto changeInfo = std::make_unique<AudioRendererChangeInfo>();
    changeInfo->clientUID = clientUid;
    changeInfo->createrUID = createrUID;
    changeInfo->clientPid = clientPid;
    changeInfo->rendererInfo.streamUsage = g_testStreamUsages[index % g_testStreamUsages.size()];
    changeInfo->sessionId = randIntValue / NUM_2;
    audioStreamCollector_.audioRendererChangeInfos_.clear();
    audioStreamCollector_.audioRendererChangeInfos_.push_back(std::move(changeInfo));
    audioStreamCollector_.HandleStartStreamMuteState(clientUid, clientPid, mute, mute);
}

void AudioStreamCollectorIsStreamActiveFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size);
    AudioStreamType volumeType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    int32_t randIntValue = static_cast<int32_t>(size);
    rendererChangeInfo->createrUID = randIntValue / NUM_2;
    rendererChangeInfo->clientUID = randIntValue / NUM_2;
    rendererChangeInfo->sessionId = randIntValue;
    rendererChangeInfo->rendererState = g_testRendererState[index % g_testRendererState.size()];
    audioStreamCollector_.audioRendererChangeInfos_.clear();
    audioStreamCollector_.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));
    audioStreamCollector_.IsStreamActive(volumeType);
}

void AudioStreamCollectorGetRunningStreamFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size);
    AudioRendererInfo rendererInfo;
    rendererInfo.contentType = g_testContentTypes[index % g_testContentTypes.size()];
    rendererInfo.streamUsage = g_testStreamUsages[index % g_testStreamUsages.size()];
    std::unique_ptr<AudioRendererChangeInfo> info = std::make_unique<AudioRendererChangeInfo>();
    int32_t randIntValue = static_cast<int32_t>(size);
    info->sessionId = randIntValue;
    info->rendererState = g_testRendererState[index % g_testRendererState.size()];
    info->rendererInfo = rendererInfo;
    info->channelCount = randIntValue % NUM_2;

    audioStreamCollector_.audioRendererChangeInfos_.clear();
    audioStreamCollector_.audioRendererChangeInfos_.push_back(std::move(info));
    audioStreamCollector_.GetRunningStream(g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()], 0);
}

void AudioStreamCollectorGetStreamTypeFromSourceTypeFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size);
    audioStreamCollector_.GetStreamTypeFromSourceType(g_testSourceTypes[index % g_testSourceTypes.size()]);
}

void AudioStreamCollectorSetGetLowPowerVolumeFuzzTest(const uint8_t *rawData, size_t size)
{
    int32_t randIntValue = static_cast<int32_t>(size);
    int32_t streamId = randIntValue / NUM_2;

    audioStreamCollector_.SetLowPowerVolume(streamId, static_cast<float>(size));
    audioStreamCollector_.GetLowPowerVolume(streamId);
}

void AudioStreamCollectorSetOffloadModeFuzzTest(const uint8_t *rawData, size_t size)
{
    int32_t randIntValue = static_cast<int32_t>(size);
    int32_t streamId = randIntValue / NUM_2;
    int32_t state = randIntValue / NUM_2 - 1;
    bool isAppBack = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);

    audioStreamCollector_.SetOffloadMode(streamId, state, isAppBack);
}

void AudioStreamCollectorUnsetOffloadModeFuzzTest(const uint8_t *rawData, size_t size)
{
    int32_t randIntValue = static_cast<int32_t>(size);
    int32_t streamId = randIntValue % NUM_2;
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioRendererChangeInfo.clientUID = randIntValue / NUM_2;
    streamChangeInfo.audioRendererChangeInfo.sessionId = randIntValue;
    uint32_t index = static_cast<uint32_t>(size);
    streamChangeInfo.audioRendererChangeInfo.rendererState = g_testRendererState[index % g_testRendererState.size()];
    sptr<IRemoteObject> object;
    sptr<IStandardClientTracker> listener = iface_cast<IStandardClientTracker>(object);
    std::shared_ptr<AudioClientTracker> callback = std::make_shared<ClientTrackerCallbackListener>(listener);
    int32_t clientId = streamChangeInfo.audioRendererChangeInfo.sessionId;
    audioStreamCollector_.clientTracker_[clientId] = callback;
    audioStreamCollector_.UnsetOffloadMode(streamId);
}

void AudioStreamCollectorGetSingleStreamVolumeFuzzTest(const uint8_t *rawData, size_t size)
{
    int32_t streamId = static_cast<int32_t>(size);
    audioStreamCollector_.GetSingleStreamVolume(streamId);
}

void AudioStreamCollectorUpdateCapturerInfoMuteStatusFuzzTest(const uint8_t *rawData, size_t size)
{
    auto changeInfo = std::make_unique<AudioCapturerChangeInfo>();
    int32_t randIntValue = static_cast<int32_t>(size);
    uint32_t index = static_cast<uint32_t>(size);
    changeInfo->clientUID = randIntValue;
    changeInfo->muted = static_cast<bool>(index % NUM_2);
    changeInfo->sessionId = randIntValue / NUM_2;
    changeInfo->capturerInfo.sourceType = g_testSourceTypes[index % g_testSourceTypes.size()];
    changeInfo->inputDeviceInfo.deviceType_ = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    audioStreamCollector_.audioCapturerChangeInfos_.clear();
    audioStreamCollector_.audioCapturerChangeInfos_.push_back(std::move(changeInfo));
    audioStreamCollector_.audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    audioStreamCollector_.UpdateCapturerInfoMuteStatus(randIntValue, true);
}

void AudioStreamCollectorIsCallStreamUsageFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size) % g_testStreamUsages.size();
    StreamUsage usage = g_testStreamUsages[index];

    audioStreamCollector_.IsCallStreamUsage(usage);
}

void AudioStreamCollectorGetRunningStreamUsageNoUltrasonicFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size);

    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    int32_t randIntValue = static_cast<int32_t>(size);
    rendererChangeInfo->createrUID = randIntValue / NUM_2;
    rendererChangeInfo->clientUID = randIntValue / NUM_2;
    rendererChangeInfo->sessionId = randIntValue;
    rendererChangeInfo->rendererState = g_testRendererState[index % g_testRendererState.size()];
    rendererChangeInfo->rendererInfo.streamUsage = g_testStreamUsages[index % g_testStreamUsages.size()];
    audioStreamCollector_.audioRendererChangeInfos_.clear();
    audioStreamCollector_.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    audioStreamCollector_.GetRunningStreamUsageNoUltrasonic();
}

void AudioStreamCollectorGetRunningSourceTypeNoUltrasonicFuzzTest(const uint8_t *rawData, size_t size)
{
    const vector<CapturerState> testCapturerStates = {
        CAPTURER_INVALID,
        CAPTURER_NEW,
        CAPTURER_PREPARED,
        CAPTURER_RUNNING,
        CAPTURER_STOPPED,
        CAPTURER_RELEASED,
        CAPTURER_PAUSED,
    };
    uint32_t index = static_cast<uint32_t>(size);
    int32_t randIntValue = static_cast<int32_t>(size);

    auto changeInfo = std::make_unique<AudioCapturerChangeInfo>();
    changeInfo->clientUID = randIntValue;
    changeInfo->sessionId = randIntValue / NUM_2;
    changeInfo->capturerState = testCapturerStates[index % testCapturerStates.size()];
    changeInfo->capturerInfo.sourceType = g_testSourceTypes[index % g_testSourceTypes.size()];
    audioStreamCollector_.audioCapturerChangeInfos_.clear();
    audioStreamCollector_.audioCapturerChangeInfos_.push_back(move(changeInfo));

    audioStreamCollector_.GetRunningSourceTypeNoUltrasonic();
}

void AudioStreamCollectorGetLastestRunningCallStreamUsageFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size);

    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    int32_t randIntValue = static_cast<int32_t>(size);
    rendererChangeInfo->createrUID = randIntValue / NUM_2;
    rendererChangeInfo->clientUID = randIntValue / NUM_2;
    rendererChangeInfo->sessionId = randIntValue;
    rendererChangeInfo->rendererState = g_testRendererState[index % g_testRendererState.size()];
    rendererChangeInfo->rendererInfo.streamUsage = g_testStreamUsages[index % g_testStreamUsages.size()];
    audioStreamCollector_.audioRendererChangeInfos_.clear();
    audioStreamCollector_.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    audioStreamCollector_.GetLastestRunningCallStreamUsage();
}

void AudioStreamCollectorGetAllRendererSessionIDForUIDFuzzTest(const uint8_t *rawData, size_t size)
{
    int32_t randIntValue = static_cast<int32_t>(size);
    int32_t uid = randIntValue / NUM_2;
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();

    rendererChangeInfo->clientUID = randIntValue /NUM_2;
    rendererChangeInfo->createrUID = randIntValue /NUM_2;
    rendererChangeInfo->sessionId = randIntValue;
    audioStreamCollector_.audioRendererChangeInfos_.clear();
    audioStreamCollector_.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    audioStreamCollector_.GetAllRendererSessionIDForUID(uid);
}

void AudioStreamCollectorGetAllCapturerSessionIDForUIDFuzzTest(const uint8_t *rawData, size_t size)
{
    int32_t randIntValue = static_cast<int32_t>(size);
    int32_t uid = randIntValue / NUM_2;
    shared_ptr<AudioCapturerChangeInfo> capturerChangeInfo = make_shared<AudioCapturerChangeInfo>();

    capturerChangeInfo->clientUID = randIntValue /NUM_2;
    capturerChangeInfo->createrUID = randIntValue /NUM_2;
    capturerChangeInfo->sessionId = randIntValue;
    audioStreamCollector_.audioCapturerChangeInfos_.clear();
    audioStreamCollector_.audioCapturerChangeInfos_.push_back(move(capturerChangeInfo));

    audioStreamCollector_.GetAllCapturerSessionIDForUID(uid);
}

void AudioStreamCollectorChangeVoipCapturerStreamToNormalFuzzTest(const uint8_t *rawData, size_t size)
{
    shared_ptr<AudioCapturerChangeInfo> rendererChangeInfo = make_shared<AudioCapturerChangeInfo>();
    int32_t randIntValue = static_cast<int32_t>(size);
    uint32_t index = static_cast<uint32_t>(size);
    rendererChangeInfo->clientUID = randIntValue / NUM_2;
    rendererChangeInfo->createrUID = randIntValue / NUM_2;
    rendererChangeInfo->sessionId = randIntValue;
    rendererChangeInfo->capturerInfo.sourceType = g_testSourceTypes[index % g_testSourceTypes.size()];
    audioStreamCollector_.audioCapturerChangeInfos_.clear();
    audioStreamCollector_.audioCapturerChangeInfos_.push_back(move(rendererChangeInfo));

    audioStreamCollector_.ChangeVoipCapturerStreamToNormal();
}

void AudioStreamCollectorHasVoipRendererStreamFuzzTest(const uint8_t *rawData, size_t size)
{
    int32_t randIntValue = static_cast<int32_t>(size);
    int32_t uid = randIntValue / NUM_2;
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();

    rendererChangeInfo->clientUID = randIntValue / NUM_2;
    rendererChangeInfo->createrUID = randIntValue / NUM_2;
    rendererChangeInfo->sessionId = randIntValue;
    rendererChangeInfo->rendererInfo.originalFlag = randIntValue % NUM_2;
    audioStreamCollector_.audioRendererChangeInfos_.clear();
    audioStreamCollector_.audioRendererChangeInfos_.push_back(move(rendererChangeInfo));

    audioStreamCollector_.HasVoipRendererStream();
}

void AudioStreamCollectorIsMediaPlayingFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size);
    AudioRendererInfo rendererInfo;
    rendererInfo.contentType = g_testContentTypes[index % g_testContentTypes.size()];
    rendererInfo.streamUsage = g_testStreamUsages[index % g_testStreamUsages.size()];
    std::unique_ptr<AudioRendererChangeInfo> info = std::make_unique<AudioRendererChangeInfo>();
    int32_t randIntValue = static_cast<int32_t>(size);
    info->sessionId = randIntValue % NUM_2;
    info->rendererState = g_testRendererState[index % g_testRendererState.size()];
    info->rendererInfo = rendererInfo;
    info->channelCount = randIntValue % NUM_2;
    audioStreamCollector_.audioRendererChangeInfos_.clear();
    audioStreamCollector_.audioRendererChangeInfos_.push_back(std::move(info));
    audioStreamCollector_.IsMediaPlaying();
}

void AudioStreamCollectorIsVoipStreamActiveFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size);
    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = g_testStreamUsages[index % g_testStreamUsages.size()];
    std::unique_ptr<AudioRendererChangeInfo> info = std::make_unique<AudioRendererChangeInfo>();
    int32_t randIntValue = static_cast<int32_t>(size);
    info->sessionId = randIntValue % NUM_2;
    info->rendererState = g_testRendererState[index % g_testRendererState.size()];
    info->rendererInfo = rendererInfo;
    info->channelCount = randIntValue % NUM_2;
    audioStreamCollector_.audioRendererChangeInfos_.clear();
    audioStreamCollector_.audioRendererChangeInfos_.push_back(std::move(info));
    audioStreamCollector_.IsVoipStreamActive();
}

void AudioStreamCollectorCheckVoiceCallActiveFuzzTest(const uint8_t *rawData, size_t size)
{
    int32_t randIntValue = static_cast<int32_t>(size);
    int32_t clientPid = randIntValue / NUM_2;
    uint32_t index = static_cast<uint32_t>(size);
    auto changeInfo = std::make_unique<AudioRendererChangeInfo>();
    changeInfo->clientPid = clientPid;
    changeInfo->rendererInfo.streamUsage = g_testStreamUsages[index % g_testStreamUsages.size()];
    changeInfo->sessionId = randIntValue / NUM_2;
    audioStreamCollector_.audioRendererChangeInfos_.clear();
    audioStreamCollector_.audioRendererChangeInfos_.push_back(std::move(changeInfo));
    audioStreamCollector_.CheckVoiceCallActive(clientPid);
}

} // namespace AudioStandard
} // namesapce OHOS

OHOS::AudioStandard::TestPtr g_testPtrs[] = {
    OHOS::AudioStandard::AudioStreamCollectorAddRendererStreamFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorGetRendererStreamInfoFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorGetCapturerStreamInfoFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorGetPipeTypeFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorExistStreamForPipeFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorGetRendererDeviceInfoFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorAddCapturerStreamFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorSendCapturerInfoEventFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorRegisterTrackerFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorSetRendererStreamParamFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorSetCapturerStreamParamFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorResetRendererStreamDeviceInfoFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorResetCapturerStreamDeviceInfoFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorCheckRendererStateInfoChangedFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorCheckRendererInfoChangedFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorResetRingerModeMuteFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorUpdateRendererStreamInternalFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorUpdateCapturerStreamInternalFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorUpdateTrackerFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorUpdateRendererDeviceInfoFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorUpdateCapturerDeviceInfoFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorUpdateRendererPipeInfoFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorUpdateAppVolumeFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorGetStreamTypeFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorGetSessionIdsOnRemoteDeviceByStreamUsageFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorIsOffloadAllowedFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorGetChannelCountFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorGetCurrentRendererChangeInfosFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorGetCurrentCapturerChangeInfosFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorRegisteredTrackerClientDiedFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorGetAndCompareStreamTypeFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorGetUidFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorResumeStreamStateFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorUpdateStreamStateFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorHandleAppStateChangeFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorHandleFreezeStateChangeFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorHandleBackTaskStateChangeFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorHandleStartStreamMuteStateFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorIsStreamActiveFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorGetRunningStreamFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorGetStreamTypeFromSourceTypeFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorSetGetLowPowerVolumeFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorSetOffloadModeFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorUnsetOffloadModeFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorGetSingleStreamVolumeFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorUpdateCapturerInfoMuteStatusFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorIsCallStreamUsageFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorGetRunningStreamUsageNoUltrasonicFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorGetRunningSourceTypeNoUltrasonicFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorGetLastestRunningCallStreamUsageFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorGetAllRendererSessionIDForUIDFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorGetAllCapturerSessionIDForUIDFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorChangeVoipCapturerStreamToNormalFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorHasVoipRendererStreamFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorIsMediaPlayingFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorIsVoipStreamActiveFuzzTest,
    OHOS::AudioStandard::AudioStreamCollectorCheckVoiceCallActiveFuzzTest,
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