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
#include "message_parcel.h"
#include "audio_process_in_client.h"
#include "audio_param_parser.h"
#include "audio_info.h"
#include "audio_source_type.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
const int32_t LIMITSIZE = 4;
bool g_hasClientInit = false;
shared_ptr<AudioProcessInClient> g_AudioProcessInClient = nullptr;
const uint64_t COMMON_UINT64_NUM = 2;
const size_t BUFFERSIZE = 2;

void GetAudioProcessInClient(const uint8_t *rawData, size_t size)
{
    if (g_AudioProcessInClient != nullptr) {
        return;
    }
    AudioProcessConfig config;
    config.appInfo.appPid = getpid();
    config.appInfo.appUid = getuid();

    config.audioMode = AUDIO_MODE_RECORD;
    config.capturerInfo.sourceType = SOURCE_TYPE_MIC;
    config.capturerInfo.capturerFlags = STREAM_FLAG_FAST;

    config.streamInfo.channels = STEREO;
    config.streamInfo.encoding = ENCODING_PCM;
    config.streamInfo.format = SAMPLE_S16LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    g_AudioProcessInClient = AudioProcessInClient::Create(config);
    if (g_AudioProcessInClient== nullptr) {
        return;
    }
    g_AudioProcessInClient->Start();
}

void AudioClientSetVolumeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    int32_t vol = *reinterpret_cast<const int32_t*>(rawData);
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->SetVolume(vol);
    }
}

void AudioClientGetSessionIDTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t sessionID = *reinterpret_cast<const uint32_t*>(rawData);
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->GetSessionID(sessionID);
    }
}

void AudioClientGetBufferSizeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    size_t bufferSize = BUFFERSIZE;
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->GetBufferSize(bufferSize);
    }
}

void AudioClientGetFrameCountTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t frameCount = *reinterpret_cast<const uint32_t*>(rawData);
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->GetFrameCount(frameCount);
    }
}

void AudioClientGetLatencyTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint64_t latency = COMMON_UINT64_NUM;
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->GetLatency(latency);
    }
}

void AudioClientSetVolumeFloatTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    float volume = *reinterpret_cast<const float*>(rawData);
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->SetVolume(volume);
    }
}

void AudioClientGetVolumeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->GetVolume();
    }
}

void AudioClientSetDuckVolumeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    float volume = *reinterpret_cast<const float*>(rawData);
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->SetDuckVolume(volume);
    }
}

void AudioClientGetUnderflowCountTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->GetUnderflowCount();
    }
}

void AudioClientGetOverflowCountTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->GetOverflowCount();
    }
}

void AudioClientSetUnderflowCountTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t underflowCount = *reinterpret_cast<const uint32_t*>(rawData);
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->SetUnderflowCount(underflowCount);
    }
}

void AudioClientSetOverflowCountTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t overflowCount = *reinterpret_cast<const uint32_t*>(rawData);
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->SetOverflowCount(overflowCount);
    }
}

void AudioClientGetFramesWrittenTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->GetFramesWritten();
    }
}

void AudioClientGetFramesReadTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->GetFramesRead();
    }
}

void AudioClientSetApplicationCachePathTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    const std::string cachePath = "cachePath";
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->SetApplicationCachePath(cachePath);
    }
}

void AudioClientSetPreferredFrameSizeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    int32_t frameSize = *(reinterpret_cast<const int32_t*>(rawData));
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->SetPreferredFrameSize(frameSize);
    }
}

void AudioClientUpdateLatencyTimestampTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    bool isRenderer = *(reinterpret_cast<const bool*>(rawData));
    std::string timestamp = "123456";
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->UpdateLatencyTimestamp(timestamp, isRenderer);
    }
}

void AudioClientPauseTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->Pause();
    }
}

void AudioClientResumeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->Resume();
    }
}

void AudioClientStopTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    if (g_AudioProcessInClient) {
        g_AudioProcessInClient->Stop();
    }
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::GetAudioProcessInClient(data, size);
    OHOS::AudioStandard::AudioClientSetVolumeTest(data, size);
    OHOS::AudioStandard::AudioClientGetSessionIDTest(data, size);
    OHOS::AudioStandard::AudioClientGetBufferSizeTest(data, size);
    OHOS::AudioStandard::AudioClientGetFrameCountTest(data, size);
    OHOS::AudioStandard::AudioClientGetLatencyTest(data, size);
    OHOS::AudioStandard::AudioClientSetVolumeFloatTest(data, size);
    OHOS::AudioStandard::AudioClientGetVolumeTest(data, size);
    OHOS::AudioStandard::AudioClientSetDuckVolumeTest(data, size);
    OHOS::AudioStandard::AudioClientGetUnderflowCountTest(data, size);
    OHOS::AudioStandard::AudioClientGetOverflowCountTest(data, size);
    OHOS::AudioStandard::AudioClientSetUnderflowCountTest(data, size);
    OHOS::AudioStandard::AudioClientSetOverflowCountTest(data, size);
    OHOS::AudioStandard::AudioClientGetFramesWrittenTest(data, size);
    OHOS::AudioStandard::AudioClientGetFramesReadTest(data, size);
    OHOS::AudioStandard::AudioClientSetApplicationCachePathTest(data, size);
    OHOS::AudioStandard::AudioClientSetPreferredFrameSizeTest(data, size);
    OHOS::AudioStandard::AudioClientUpdateLatencyTimestampTest(data, size);
    OHOS::AudioStandard::AudioClientPauseTest(data, size);
    OHOS::AudioStandard::AudioClientResumeTest(data, size);
    OHOS::AudioStandard::AudioClientStopTest(data, size);
    return 0;
}
