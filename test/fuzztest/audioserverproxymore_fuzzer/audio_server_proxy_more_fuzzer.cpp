/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "securec.h"
#include "audio_log.h"
#include "audio_info.h"
#include "audio_device_info.h"
#include "audio_server_proxy.h"
#include "audio_router_select_strategy.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
typedef void (*TestPtr)();

const vector<SourceType> g_testSourceTypes = {
    SOURCE_TYPE_INVALID,
    SOURCE_TYPE_MIC,
    SOURCE_TYPE_VOICE_COMMUNICATION,
    SOURCE_TYPE_VOICE_RECOGNITION,
    SOURCE_TYPE_VOICE_CALL,
};

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
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

void SetAudioSceneProxyFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    AudioRouterSelectStrategy::GetInstance().SetScoExcluded(true);
    AudioScene audioScene = static_cast<AudioScene>(GetData<int32_t>());
    BluetoothOffloadState state = static_cast<BluetoothOffloadState>(GetData<int32_t>());
    int32_t phoneCallSceneType = GetData<int32_t>();
    audioServerProxy.SetAudioSceneProxy(audioScene, state, phoneCallSceneType);
}

void CreateCallTransferProxyFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    audioServerProxy.CreateCallTransferProxy();
}

void GetMaxAmplitudeProxyFuzzTest()
{
    if (g_testSourceTypes.size() == 0) {
        return;
    }
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    bool flag = GetData<bool>();
    std::string portName = "testPortName";
    audioServerProxy.GetMaxAmplitudeProxy(flag, portName);
}

void NotifyTaskIdInfoProxyFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    std::string taskId = "testTaskId";
    bool connected = GetData<bool>();
    audioServerProxy.NotifyTaskIdInfoProxy(taskId, connected);
}

void GetPrivacyTypeFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    uint32_t sessionId = GetData<uint32_t>();
    AudioPrivacyType privacyType = GetData<AudioPrivacyType>();
    audioServerProxy.GetPrivacyType(sessionId, privacyType);
}

void UnRegistAdapterManagerCallbackFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    std::string networkId = "testNetworkId";
    audioServerProxy.UnRegistAdapterManagerCallback(networkId);
}

TestPtr g_testPtrs[] = {
    SetAudioSceneProxyFuzzTest,
    CreateCallTransferProxyFuzzTest,
    GetMaxAmplitudeProxyFuzzTest,
    NotifyTaskIdInfoProxyFuzzTest,
    GetPrivacyTypeFuzzTest,
    UnRegistAdapterManagerCallbackFuzzTest,
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
    }
    return;
}

} // namespace AudioStandard
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
