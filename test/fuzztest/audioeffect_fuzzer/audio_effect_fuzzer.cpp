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
const int32_t LIMITSIZE = 4;
const string SCENETYPEDEFAULT = "SCENE_MOVIE";
const string SCENETYPEMUSIC = "SCENE_MUSIC";
bool g_hasPermission = false;
const uint64_t COMMON_UINT64_NUM = 2;
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
    "0",
};

void InitAudioEffectChainManagerFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    string effectMode = "EFFECT_DEFAULT";
    string sceneType = "SCENE_MOVIE";
    AudioEffectScene currSceneType = SCENE_MUSIC;
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_MAP,
        DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);
    AudioEffectChainManager::GetInstance()->ExistAudioEffectChain(sceneType, effectMode);
    AudioEffectChainManager::GetInstance()->SetHdiParam(currSceneType);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void CheckAndAddSessionIDFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::string sessionID = "123456";
    AudioEffectChainManager::GetInstance()->CheckAndAddSessionID(sessionID);
    AudioEffectChainManager::GetInstance()->CheckAndRemoveSessionID(sessionID);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void CheckAndRemoveSessionIDFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    const std::string sessionID = "123456";
    AudioEffectChainManager::GetInstance()->CheckAndRemoveSessionID(sessionID);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void ReleaseAudioEffectChainDynamicFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_MAP,
        DEFAULT_EFFECT_LIBRARY_LIST);
    const std::string sceneType = "SCENE_MOVIE";
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);
    AudioEffectChainManager::GetInstance()->ReleaseAudioEffectChainDynamic(sceneType);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void ApplyAudioEffectChainFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_MAP,
        DEFAULT_EFFECT_LIBRARY_LIST);
    int numChans = *reinterpret_cast<const int*>(rawData);
    int frameLen = *reinterpret_cast<const int*>(rawData);
    float* bufIn = const_cast<float *>(reinterpret_cast<const float*>(rawData));
    float* bufOut = const_cast<float *>(reinterpret_cast<const float*>(rawData));
    uint32_t outChannels = INFOCHANNELS;
    uint64_t outChannelLayout = INFOCHANNELLAYOUT;
    auto eBufferAttr = make_unique<EffectBufferAttr>(bufIn, bufOut, numChans, frameLen, outChannels, outChannelLayout);
    const std::string sceneType = "SCENE_MOVIE";
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);
    AudioEffectChainManager::GetInstance()->ApplyAudioEffectChain(sceneType, eBufferAttr);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void SetOutputDeviceSinkFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_MAP,
        DEFAULT_EFFECT_LIBRARY_LIST);
    int32_t device = *reinterpret_cast<const int32_t *>(rawData);
    const std::string sinkName = "123456";
    AudioEffectChainManager::GetInstance()->SetOutputDeviceSink(device, sinkName);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void GetDeviceSinkNameFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioEffectChainManager::GetInstance()->GetDeviceTypeName();
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void GetOffloadEnabledFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_MAP,
        DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;

    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->SetSpkOffloadState();
    AudioEffectChainManager::GetInstance()->GetOffloadEnabled();
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void UpdateMultichannelConfigFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_MAP,
        DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(SCENETYPEDEFAULT);
    const std::string sceneType = "SCENE_MOVIE";
    AudioEffectChainManager::GetInstance()->UpdateMultichannelConfig(sceneType);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void UpdateSpatializationStateFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    bool spatializationEnabled = *reinterpret_cast<const bool*>(rawData);
    bool headTrackingEnabled = *reinterpret_cast<const bool*>(rawData + sizeof(bool));
    AudioSpatializationState spatializationState = {spatializationEnabled, headTrackingEnabled};
    
    AudioEffectChainManager::GetInstance()->UpdateSpatializationState(spatializationState);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void SetHdiParamFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioEffectScene currSceneType = SCENE_MUSIC;
    AudioEffectChainManager::GetInstance()->SetHdiParam(currSceneType);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void SessionInfoMapAddFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    const std::string sessionID = "123456";
    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    AudioEffectChainManager::GetInstance()->SessionInfoMapDelete(SCENETYPEDEFAULT, sessionID);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void SessionInfoMapDeleteFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    const std::string sessionID = "123456";
    AudioEffectChainManager::GetInstance()->SessionInfoMapDelete(SCENETYPEDEFAULT, sessionID);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void ReturnEffectChannelInfoFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    uint32_t channels = *reinterpret_cast<const uint32_t*>(rawData);
    uint64_t channelLayout = COMMON_UINT64_NUM;
    const std::string sessionID = "123456";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_MAP,
        DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(SCENETYPEDEFAULT);
    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);

    AudioEffectChainManager::GetInstance()->ReturnEffectChannelInfo(SCENETYPEDEFAULT, channels,
        channelLayout);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void ReturnMultiChannelInfoFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    uint32_t channels = *reinterpret_cast<const uint32_t*>(rawData);
    uint64_t channelLayout = COMMON_UINT64_NUM;
    const std::string sessionID = "123456";

    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    AudioEffectChainManager::GetInstance()->ReturnMultiChannelInfo(&channels, &channelLayout);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void EffectRotationUpdateFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    uint32_t rotationState = *reinterpret_cast<const uint32_t*>(rawData);
    AudioEffectChainManager::GetInstance()->EffectRotationUpdate(rotationState);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void GetLatencyFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    const std::string sessionID = "123456";
    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    AudioEffectChainManager::GetInstance()->GetLatency(sessionID);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void SetSpatializationSceneTypeFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size < sizeof(AudioSpatializationSceneType)) {
        return;
    }
    AudioSpatializationSceneType spatializationSceneType =
        *reinterpret_cast<const AudioSpatializationSceneType*>(rawData);

    AudioEffectChainManager::GetInstance()->SetSpatializationSceneType(spatializationSceneType);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void GetCurSpatializationEnabledFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioEffectChainManager::GetInstance()->GetCurSpatializationEnabled();
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void ResetEffectBufferFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioEffectChainManager::GetInstance()->ResetEffectBuffer();
}

void UpdateSpkOffloadEnabledFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_MAP,
        DEFAULT_EFFECT_LIBRARY_LIST);
    bool spkOffloadEnabled = *reinterpret_cast<const bool*>(rawData);
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = spkOffloadEnabled;
    AudioEffectChainManager::GetInstance()->UpdateDefaultAudioEffect();
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->GetOffloadEnabled();
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void UpdateDeviceInfoFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    int32_t device = *reinterpret_cast<const int32_t*>(rawData);
    string sinkName = "Speaker";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_MAP,
        DEFAULT_EFFECT_LIBRARY_LIST);
    bool isInitialized = *reinterpret_cast<const bool*>(rawData);
    AudioEffectChainManager::GetInstance()->isInitialized_ = isInitialized;
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_MAP,
        DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->UpdateDeviceInfo(device, sinkName);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void CheckAndReleaseCommonEffectChainFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_MAP,
        DEFAULT_EFFECT_LIBRARY_LIST);
    bool isCommonEffectChainExisted = *reinterpret_cast<const bool*>(rawData);
    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = isCommonEffectChainExisted;
    AudioEffectChainManager::GetInstance()->CheckAndReleaseCommonEffectChain(SCENETYPEMUSIC);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void UpdateCurrSceneTypeFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size < sizeof(AudioEffectScene)) {
        return;
    }
    AudioEffectScene currSceneType = *reinterpret_cast<const AudioEffectScene*>(rawData);
    bool spatializationEnabled = *reinterpret_cast<const bool*>(rawData);

    std::string sceneType = SCENETYPEMUSIC;
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_MAP,
        DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->spatializationEnabled_ = spatializationEnabled;
    AudioEffectChainManager::GetInstance()->UpdateCurrSceneType(currSceneType, sceneType);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void CheckSceneTypeMatchFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    const std::string sinkSceneType = SCENETYPEMUSIC;
    const std::string sceneType = SCENETYPEMUSIC;

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_MAP,
        DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->CheckSceneTypeMatch(sinkSceneType, sceneType);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

void UpdateSpatialDeviceTypeFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioSpatialDeviceType spatialDeviceType = *reinterpret_cast<const AudioSpatialDeviceType*>(rawData);

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_MAP,
        DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->UpdateSpatialDeviceType(spatialDeviceType);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}
} // namespace AudioStandard
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *rawData, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::InitAudioEffectChainManagerFuzzTest(rawData, size);
    OHOS::AudioStandard::CheckAndAddSessionIDFuzzTest(rawData, size);
    OHOS::AudioStandard::CheckAndRemoveSessionIDFuzzTest(rawData, size);
    OHOS::AudioStandard::ReleaseAudioEffectChainDynamicFuzzTest(rawData, size);
    OHOS::AudioStandard::ApplyAudioEffectChainFuzzTest(rawData, size);
    OHOS::AudioStandard::SetOutputDeviceSinkFuzzTest(rawData, size);
    OHOS::AudioStandard::GetDeviceSinkNameFuzzTest(rawData, size);
    OHOS::AudioStandard::GetOffloadEnabledFuzzTest(rawData, size);
    OHOS::AudioStandard::UpdateMultichannelConfigFuzzTest(rawData, size);
    OHOS::AudioStandard::UpdateSpatializationStateFuzzTest(rawData, size);
    OHOS::AudioStandard::SetHdiParamFuzzTest(rawData, size);
    OHOS::AudioStandard::SessionInfoMapAddFuzzTest(rawData, size);
    OHOS::AudioStandard::SessionInfoMapDeleteFuzzTest(rawData, size);
    OHOS::AudioStandard::ReturnEffectChannelInfoFuzzTest(rawData, size);
    OHOS::AudioStandard::ReturnMultiChannelInfoFuzzTest(rawData, size);
    OHOS::AudioStandard::EffectRotationUpdateFuzzTest(rawData, size);
    OHOS::AudioStandard::GetLatencyFuzzTest(rawData, size);
    OHOS::AudioStandard::SetSpatializationSceneTypeFuzzTest(rawData, size);
    OHOS::AudioStandard::GetCurSpatializationEnabledFuzzTest(rawData, size);
    OHOS::AudioStandard::ResetEffectBufferFuzzTest(rawData, size);
    OHOS::AudioStandard::UpdateSpkOffloadEnabledFuzzTest(rawData, size);
    OHOS::AudioStandard::UpdateDeviceInfoFuzzTest(rawData, size);
    OHOS::AudioStandard::CheckAndReleaseCommonEffectChainFuzzTest(rawData, size);
    OHOS::AudioStandard::UpdateCurrSceneTypeFuzzTest(rawData, size);
    OHOS::AudioStandard::CheckSceneTypeMatchFuzzTest(rawData, size);
    OHOS::AudioStandard::UpdateSpatialDeviceTypeFuzzTest(rawData, size);
    return 0;
}
