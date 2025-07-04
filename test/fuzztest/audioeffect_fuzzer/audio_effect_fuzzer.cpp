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
#include <chrono>
#include <thread>
#include <fstream>

#include "audio_effect.h"
#include "audio_utils.h"
#include "audio_effect_log.h"
#include "audio_effect_chain_manager.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
constexpr uint32_t INFOCHANNELS = 2;
constexpr uint64_t INFOCHANNELLAYOUT = 0x3;
const string SCENETYPEDEFAULT = "SCENE_MOVIE";
const string SCENETYPEMUSIC = "SCENE_MUSIC";
static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;

vector<EffectChain> DEFAULT_EFFECT_CHAINS = {{"EFFECTCHAIN_SPK_MUSIC", {}, ""}, {"EFFECTCHAIN_BT_MUSIC", {}, ""}};
EffectChainManagerParam DEFAULT_MAP{
    3,
    "SCENE_DEFAULT",
    {},
    {{"SCENE_MOVIE_&_EFFECT_DEFAULT_&_DEVICE_TYPE_SPEAKER", "EFFECTCHAIN_SPK_MUSIC"},
        {"SCENE_MOVIE_&_EFFECT_DEFAULT_&_DEVICE_TYPE_BLUETOOTH_A2DP", "EFFECTCHAIN_BT_MUSIC"}},
    {{"effect1", "property1"}, {"effect4", "property5"}, {"effect1", "property4"}}
};

vector<shared_ptr<AudioEffectLibEntry>> DEFAULT_EFFECT_LIBRARY_LIST = {};
SessionEffectInfo DEFAULT_INFO = {
    "EFFECT_DEFAULT",
    SCENETYPEDEFAULT,
    INFOCHANNELS,
    INFOCHANNELLAYOUT,
};

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

void InitAudioEffectChainManagerFuzzTest()
{
    string effectMode = "EFFECT_DEFAULT";
    string sceneType = "SCENE_MOVIE";
    AudioEffectScene currSceneType = GetData<AudioEffectScene>();
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_MAP,
        DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);
    AudioEffectChainManager::GetInstance()->ExistAudioEffectChain(sceneType, effectMode);
    AudioEffectChainManager::GetInstance()->SetHdiParam(currSceneType);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void CheckAndAddSessionIDFuzzTest()
{
    std::string sessionID = "123456";
    AudioEffectChainManager::GetInstance()->CheckAndAddSessionID(sessionID);
    AudioEffectChainManager::GetInstance()->CheckAndRemoveSessionID(sessionID);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void CheckAndRemoveSessionIDFuzzTest()
{
    const std::string sessionID = "123456";
    AudioEffectChainManager::GetInstance()->CheckAndRemoveSessionID(sessionID);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void ReleaseAudioEffectChainDynamicFuzzTest()
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_MAP,
        DEFAULT_EFFECT_LIBRARY_LIST);
    const std::string sceneType = "SCENE_MOVIE";
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);
    AudioEffectChainManager::GetInstance()->ReleaseAudioEffectChainDynamic(sceneType);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void ApplyAudioEffectChainFuzzTest()
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_MAP,
        DEFAULT_EFFECT_LIBRARY_LIST);
    int numChans = GetData<int>();
    int frameLen = GetData<int>();
    float* bufIn = GetData<float*>();
    float* bufOut = GetData<float*>();
    uint32_t outChannels = INFOCHANNELS;
    uint64_t outChannelLayout = INFOCHANNELLAYOUT;
    const std::string sceneType = "SCENE_MOVIE";
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void SetOutputDeviceSinkFuzzTest()
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_MAP,
        DEFAULT_EFFECT_LIBRARY_LIST);
    int32_t device = GetData<int32_t>();
    const std::string sinkName = "123456";
    AudioEffectChainManager::GetInstance()->SetOutputDeviceSink(device, sinkName);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void GetDeviceSinkNameFuzzTest()
{
    AudioEffectChainManager::GetInstance()->GetDeviceTypeName();
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void GetOffloadEnabledFuzzTest()
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_MAP,
        DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;

    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->SetSpkOffloadState();
    AudioEffectChainManager::GetInstance()->GetOffloadEnabled();
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void UpdateMultichannelConfigFuzzTest()
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_MAP,
        DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(SCENETYPEDEFAULT);
    const std::string sceneType = "SCENE_MOVIE";
    AudioEffectChainManager::GetInstance()->UpdateMultichannelConfig(sceneType);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void UpdateSpatializationStateFuzzTest()
{
    bool spatializationEnabled = GetData<bool>();
    bool headTrackingEnabled = GetData<bool>();

    AudioSpatializationState spatializationState = {spatializationEnabled, headTrackingEnabled};
    
    AudioEffectChainManager::GetInstance()->UpdateSpatializationState(spatializationState);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void SetHdiParamFuzzTest()
{
    AudioEffectScene currSceneType = GetData<AudioEffectScene>();
    AudioEffectChainManager::GetInstance()->SetHdiParam(currSceneType);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void SessionInfoMapAddFuzzTest()
{
    const std::string sessionID = "123456";
    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    AudioEffectChainManager::GetInstance()->SessionInfoMapDelete(SCENETYPEDEFAULT, sessionID);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void SessionInfoMapDeleteFuzzTest()
{
    const std::string sessionID = "123456";
    AudioEffectChainManager::GetInstance()->SessionInfoMapDelete(SCENETYPEDEFAULT, sessionID);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void ReturnEffectChannelInfoFuzzTest()
{
    uint32_t channels = GetData<uint32_t>();
    uint64_t channelLayout = GetData<uint64_t>();
    const std::string sessionID = "123456";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_MAP,
        DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(SCENETYPEDEFAULT);
    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);

    AudioEffectChainManager::GetInstance()->ReturnEffectChannelInfo(SCENETYPEDEFAULT, channels,
        channelLayout);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void ReturnMultiChannelInfoFuzzTest()
{
    uint32_t channels = GetData<uint32_t>();
    uint64_t channelLayout = GetData<uint64_t>();
    const std::string sessionID = "123456";

    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    AudioEffectChainManager::GetInstance()->ReturnMultiChannelInfo(&channels, &channelLayout);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void EffectRotationUpdateFuzzTest()
{
    uint32_t rotationState = GetData<uint32_t>();
    AudioEffectChainManager::GetInstance()->EffectRotationUpdate(rotationState);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void GetLatencyFuzzTest()
{
    const std::string sessionID = "123456";
    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    AudioEffectChainManager::GetInstance()->GetLatency(sessionID);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void SetSpatializationSceneTypeFuzzTest()
{
    AudioSpatializationSceneType spatializationSceneType = GetData<AudioSpatializationSceneType>();

    AudioEffectChainManager::GetInstance()->SetSpatializationSceneType(spatializationSceneType);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void UpdateSpkOffloadEnabledFuzzTest()
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_MAP,
        DEFAULT_EFFECT_LIBRARY_LIST);
    bool spkOffloadEnabled = GetData<bool>();
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = spkOffloadEnabled;
    AudioEffectChainManager::GetInstance()->UpdateDefaultAudioEffect();
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->GetOffloadEnabled();
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void UpdateDeviceInfoFuzzTest()
{
    int32_t device = GetData<int32_t>();
    string sinkName = "Speaker";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_MAP,
        DEFAULT_EFFECT_LIBRARY_LIST);
    bool isInitialized = GetData<bool>();
    AudioEffectChainManager::GetInstance()->isInitialized_ = isInitialized;
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_MAP,
        DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->UpdateDeviceInfo(device, sinkName);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void CheckAndReleaseCommonEffectChainFuzzTest()
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_MAP,
        DEFAULT_EFFECT_LIBRARY_LIST);
    bool isCommonEffectChainExisted = GetData<bool>();
    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = isCommonEffectChainExisted;
    AudioEffectChainManager::GetInstance()->CheckAndReleaseCommonEffectChain(SCENETYPEMUSIC);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void UpdateCurrSceneTypeFuzzTest()
{
    AudioEffectScene currSceneType = GetData<AudioEffectScene>();
    bool spatializationEnabled = GetData<bool>();

    std::string sceneType = SCENETYPEMUSIC;
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_MAP,
        DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->spatializationEnabled_ = spatializationEnabled;
    AudioEffectChainManager::GetInstance()->UpdateCurrSceneType(currSceneType, sceneType);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void CheckSceneTypeMatchFuzzTest()
{
    const std::string sinkSceneType = SCENETYPEMUSIC;
    const std::string sceneType = SCENETYPEMUSIC;

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_MAP,
        DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->CheckSceneTypeMatch(sinkSceneType, sceneType);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void UpdateSpatialDeviceTypeFuzzTest()
{
    AudioSpatialDeviceType spatialDeviceType = GetData<AudioSpatialDeviceType>();

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_MAP,
        DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->UpdateSpatialDeviceType(spatialDeviceType);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

typedef void (*TestFuncs[22])();

TestFuncs g_testFuncs = {
    InitAudioEffectChainManagerFuzzTest,
    CheckAndAddSessionIDFuzzTest,
    CheckAndRemoveSessionIDFuzzTest,
    ApplyAudioEffectChainFuzzTest,
    SetOutputDeviceSinkFuzzTest,
    GetDeviceSinkNameFuzzTest,
    GetOffloadEnabledFuzzTest,
    UpdateMultichannelConfigFuzzTest,
    UpdateSpatializationStateFuzzTest,
    SetHdiParamFuzzTest,
    SessionInfoMapAddFuzzTest,
    SessionInfoMapDeleteFuzzTest,
    ReturnEffectChannelInfoFuzzTest,
    ReturnMultiChannelInfoFuzzTest,
    EffectRotationUpdateFuzzTest,
    GetLatencyFuzzTest,
    SetSpatializationSceneTypeFuzzTest,
    UpdateSpkOffloadEnabledFuzzTest,
    CheckAndReleaseCommonEffectChainFuzzTest,
    UpdateCurrSceneTypeFuzzTest,
    CheckSceneTypeMatchFuzzTest,
    UpdateSpatialDeviceTypeFuzzTest
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
