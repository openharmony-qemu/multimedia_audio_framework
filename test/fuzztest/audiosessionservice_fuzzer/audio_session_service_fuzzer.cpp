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

#include "audio_log.h"
#include "audio_session_service.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
typedef void (*TestPtr)();

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
    if (g_dataSize < g_pos) {
        return object;
    }
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

void AudioSessionServiceIsSameTypeForAudioSessionFuzzTest()
{
    auto audioSessionService = std::make_shared<AudioSessionService>();
    if (g_testAudioStreamTypes.size() == 0 || audioSessionService == nullptr) {
        return;
    }
    AudioStreamType incomingType = g_testAudioStreamTypes[GetData<uint32_t>() % g_testAudioStreamTypes.size()];
    AudioStreamType existedType = g_testAudioStreamTypes[GetData<uint32_t>() % g_testAudioStreamTypes.size()];
    audioSessionService->IsSameTypeForAudioSession(incomingType, existedType);
}

void AudioSessionServiceActivateAudioSessionFuzzTest()
{
    auto audioSessionService = std::make_shared<AudioSessionService>();
    if (audioSessionService == nullptr) {
        return;
    }
    int32_t callerPid = GetData<int32_t>();
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor = nullptr;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor);
    bool ifNull = GetData<bool>();
    if (ifNull) {
        audioSession = nullptr;
    }
    audioSessionService->sessionMap_.insert(make_pair(callerPid, audioSession));
    audioSessionService->ActivateAudioSession(callerPid, strategy);
}

void AudioSessionServiceDeactivateAudioSessionFuzzTest()
{
    auto audioSessionService = std::make_shared<AudioSessionService>();
    if (audioSessionService == nullptr) {
        return;
    }
    int32_t callerPid = GetData<int32_t>();
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor = nullptr;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor);
    bool isAddMap = GetData<bool>();
    if (isAddMap) {
        audioSessionService->sessionMap_.insert(make_pair(callerPid, audioSession));
    }
    audioSessionService->DeactivateAudioSession(callerPid);
}

void AudioSessionServiceIsAudioSessionActivatedFuzzTest()
{
    auto audioSessionService = std::make_shared<AudioSessionService>();
    if (audioSessionService == nullptr) {
        return;
    }
    int32_t callerPid = GetData<int32_t>();
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor = nullptr;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor);
    bool isAddMap = GetData<bool>();
    if (isAddMap) {
        audioSessionService->sessionMap_.insert(make_pair(callerPid, audioSession));
    }
    audioSessionService->IsAudioSessionActivated(callerPid);
}

void AudioSessionServiceSetSessionTimeOutCallbackFuzzTest()
{
    auto audioSessionService = std::make_shared<AudioSessionService>();
    if (audioSessionService == nullptr) {
        return;
    }

    std::shared_ptr<SessionTimeOutCallback> timeOutCallback = nullptr;
    audioSessionService->SetSessionTimeOutCallback(timeOutCallback);
}

void AudioSessionServiceGetAudioSessionByPidFuzzTest()
{
    auto audioSessionService = std::make_shared<AudioSessionService>();
    if (audioSessionService == nullptr) {
        return;
    }
    int32_t callerPid = GetData<int32_t>();
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor = nullptr;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor);
    bool isAddMap = GetData<bool>();
    if (isAddMap) {
        audioSessionService->sessionMap_.insert(make_pair(callerPid, audioSession));
    }

    audioSessionService->GetAudioSessionByPid(callerPid);
}

void AudioSessionServiceOnAudioSessionTimeOutFuzzTest()
{
    auto audioSessionService = std::make_shared<AudioSessionService>();
    if (audioSessionService == nullptr) {
        return;
    }
    int32_t callerPid = GetData<int32_t>();

    audioSessionService->OnAudioSessionTimeOut(callerPid);
}

void AudioSessionServiceAudioSessionInfoDumpFuzzTest()
{
    auto audioSessionService = std::make_shared<AudioSessionService>();
    if (audioSessionService == nullptr) {
        return;
    }
    int32_t callerPid = GetData<int32_t>();
    std::string dumpString = "test";
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor = nullptr;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor);
    bool isNull = GetData<bool>();
    if (isNull) {
        audioSession = nullptr;
    }
    audioSessionService->sessionMap_.insert(make_pair(callerPid, audioSession));
    audioSessionService->AudioSessionInfoDump(dumpString);
}

TestPtr g_testPtrs[] = {
    AudioSessionServiceIsSameTypeForAudioSessionFuzzTest,
    AudioSessionServiceActivateAudioSessionFuzzTest,
    AudioSessionServiceDeactivateAudioSessionFuzzTest,
    AudioSessionServiceIsAudioSessionActivatedFuzzTest,
    AudioSessionServiceSetSessionTimeOutCallbackFuzzTest,
    AudioSessionServiceGetAudioSessionByPidFuzzTest,
    AudioSessionServiceOnAudioSessionTimeOutFuzzTest,
    AudioSessionServiceAudioSessionInfoDumpFuzzTest,
};

void FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return;
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
    return;
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