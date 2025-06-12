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