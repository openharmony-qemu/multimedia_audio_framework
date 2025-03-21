/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "message_parcel.h"
#include "audio_param_parser.h"
#include "audio_info.h"
#include "audio_source_type.h"
#include "audio_process_in_client.h"
#include "fast_audio_stream.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
shared_ptr<AudioProcessInClient> g_AudioProcessInClient = nullptr;
shared_ptr<FastAudioStream> g_FastAudioStream = nullptr;
static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;

/*
* describe: get data from outside untrusted data(RAW_DATA) which size is according to sizeof(T)
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

void GetAudioProcessInClient()
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
    g_FastAudioStream = std::make_shared<FastAudioStream>(config.streamType,
        AUDIO_MODE_RECORD, config.appInfo.appUid);
    g_AudioProcessInClient = AudioProcessInClient::Create(config, g_FastAudioStream);
    if (g_AudioProcessInClient== nullptr) {
        return;
    }
    g_AudioProcessInClient->Start();

    int32_t vol = GetData<int32_t>();
    g_AudioProcessInClient->SetVolume(vol);

    uint32_t sessionID = GetData<uint32_t>();
    g_AudioProcessInClient->GetSessionID(sessionID);

    size_t bufferSize = GetData<size_t>();
    g_AudioProcessInClient->GetBufferSize(bufferSize);

    uint32_t frameCount = GetData<uint32_t>();
    g_AudioProcessInClient->GetFrameCount(frameCount);

    uint64_t latency = GetData<uint32_t>();
    g_AudioProcessInClient->GetLatency(latency);

    float volume = GetData<float>();
    g_AudioProcessInClient->SetVolume(volume);
    g_AudioProcessInClient->GetVolume();
    g_AudioProcessInClient->SetDuckVolume(volume);
    g_AudioProcessInClient->GetUnderflowCount();
    g_AudioProcessInClient->GetOverflowCount();

    uint32_t flowCount = GetData<uint32_t>();
    g_AudioProcessInClient->SetUnderflowCount(flowCount);
    g_AudioProcessInClient->SetOverflowCount(flowCount);
    g_AudioProcessInClient->GetFramesWritten();
    g_AudioProcessInClient->GetFramesRead();

    int32_t frameSize = GetData<int32_t>();
    g_AudioProcessInClient->SetPreferredFrameSize(frameSize);
}

void AudioClientUpdateLatencyTimestampTest()
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
    g_FastAudioStream = std::make_shared<FastAudioStream>(config.streamType,
        AUDIO_MODE_RECORD, config.appInfo.appUid);
    g_AudioProcessInClient = AudioProcessInClient::Create(config, g_FastAudioStream);
    if (g_AudioProcessInClient== nullptr) {
        return;
    }

    bool isRenderer = GetData<bool>();
    std::string timestamp = "123456";
    g_AudioProcessInClient->UpdateLatencyTimestamp(timestamp, isRenderer);
    g_AudioProcessInClient->Pause();
    g_AudioProcessInClient->Resume();
    g_AudioProcessInClient->Stop();
}

typedef void (*TestFuncs[2])();

TestFuncs g_testFuncs = {
    GetAudioProcessInClient,
    AudioClientUpdateLatencyTimestampTest,
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
