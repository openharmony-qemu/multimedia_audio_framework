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
#include <memory>
#include "audio_effect_log.h"
#include "audio_effect_chain.h"
#include "audio_effect_chain_adapter.h"
#include "audio_effect_chain_manager.h"
#include "audio_enhance_chain.h"
#include "audio_enhance_chain_adapter.h"
#include "audio_enhance_chain_manager.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
const int32_t LIMITSIZE = 4;
const string EXTRASCENETYPE = "2";
const uint32_t AUDIOEFFECTSCENE_LENGTH = 6;
const uint32_t CHANNEL_NUM = 10;
constexpr uint32_t INFOCHANNELS = 2;
constexpr uint64_t INFOCHANNELLAYOUT = 0x3;
const int32_t VALID_BUFFER_SIZE = 1000;
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

void AudioEffectChainEnhance(std::shared_ptr<AudioEffectChain> audioEffectChain)
{
    std::string sceneType = "SCENE_MUSIC";
#ifdef SENSOR_ENABLE
    std::shared_ptr<HeadTracker> headTracker = nullptr;
    headTracker = std::make_shared<HeadTracker>();
    audioEffectChain = std::make_shared<AudioEffectChain>(sceneType, headTracker);
#else
    audioEffectChain = std::make_shared<AudioEffectChain>(sceneType);
#endif

    int32_t streamUsage = GetData<int32_t>();
    audioEffectChain->SetStreamUsage(streamUsage);

    std::string effect = "";
    std::string property = "";
    audioEffectChain->SetEffectProperty(effect, property);

    audioEffectChain->UpdateEffectParam();
    audioEffectChain->UpdateMultichannelIoBufferConfigInner();

    float volume = GetData<float>();
    audioEffectChain->GetFinalVolume();
    audioEffectChain->SetFinalVolume(volume);
}

void AudioEffectChainEnhanceFuzzTest()
{
    std::shared_ptr<AudioEffectChain> audioEffectChain = nullptr;
    std::string sceneType = "SCENE_MUSIC";
#ifdef SENSOR_ENABLE
    std::shared_ptr<HeadTracker> headTracker = nullptr;
    headTracker = std::make_shared<HeadTracker>();
    audioEffectChain = std::make_shared<AudioEffectChain>(sceneType, headTracker);
#else
    audioEffectChain = std::make_shared<AudioEffectChain>(sceneType);
#endif
    audioEffectChain->SetEffectMode("EFFECT_DEFAULT");
    audioEffectChain->SetExtraSceneType(EXTRASCENETYPE);
    uint32_t currSceneTypeInt = GetData<uint32_t>();
    currSceneTypeInt = (currSceneTypeInt % AUDIOEFFECTSCENE_LENGTH);
    AudioEffectScene currSceneType = static_cast<AudioEffectScene>(currSceneTypeInt);
    audioEffectChain->SetEffectCurrSceneType(currSceneType);
    AudioEffectChainEnhance(audioEffectChain);
}

void AudioEffectChainManagerEnhanceFuzzTest()
{
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = std::make_shared<AudioEffectVolume>();
    AudioEffectChainManager::GetInstance()->EffectDspVolumeUpdate(audioEffectVolume);

    std::string mainkey = "audio_effect";
    std::string subkey = "update_audio_effect_type";
    std::string extraSceneType = "0";
    AudioEffectChainManager::GetInstance()->UpdateParamExtra(mainkey, subkey, extraSceneType);
    AudioEffectChainManager::GetInstance()->SetSpatializationSceneTypeToChains();
    AudioEffectChainManager::GetInstance()->UpdateStreamUsage();
    AudioEffectChainManager::GetInstance()->UpdateCurrSceneTypeAndStreamUsageForDsp();

    std::string sceneType = "SCENE_DEFAULT";
    AudioEffectChainManager::GetInstance()->GetSceneTypeToChainCount(sceneType);

    std::set<std::string> sessions = {"12345", "67890", "34567"};
    uint32_t maxSessionID = GetData<uint32_t>();
    const std::string sessionID = "12345";
    std::string sceneTypeMax = "EFFECT_NONE";
    SessionEffectInfo sessionEffectInfo = {
        "EFFECT_NONE",
        "SCENE_MOVIE",
        INFOCHANNELS,
        INFOCHANNELLAYOUT,
        "0",
    };

    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_[sessionID] = sessionEffectInfo;
    const std::string scenePairType = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    AudioEffectChainManager::GetInstance()->FindMaxSessionID(maxSessionID, sceneTypeMax, scenePairType, sessions);
}

void AudioEffectChainAdapterFuzzTest()
{
    struct BufferAttr bufferAttr;
    char sceneType[] = "SCENE_MUSIC";
    EffectChainManagerProcess(sceneType, &bufferAttr);

    const char *sceneTypeExist = "SCENE_MUSIC";
    EffectChainManagerGetSceneCount(sceneTypeExist);

    const char *effectMode = "EFFECT_DEFAULT";
    EffectChainManagerExist(sceneTypeExist, effectMode);

    const uint64_t channelLayout = CH_LAYOUT_MONO;
    pa_channel_map processCm;
    ConvertChLayoutToPaChMap(channelLayout, &processCm);

    const uint64_t channelLayout2 = CH_LAYOUT_STEREO;
    pa_channel_map processCm2;
    ConvertChLayoutToPaChMap(channelLayout2, &processCm2);
    EffectChainManagerEffectUpdate();
    EffectChainManagerStreamUsageUpdate();
}

void AudioEnhanceChainMoreFuzzTest()
{
    std::string scene = "scene";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain =
        std::make_shared<AudioEnhanceChain>(scene, algoParam, deviceAttr, true);

    std::string inputDevice = "inputDevice";
    std::string deviceName = "deviceName";
    audioEnhanceChain->SetInputDevice(inputDevice, deviceName);

    uint8_t *src = new uint8_t[LIMITSIZE];
    uint32_t channel = CHANNEL_NUM;
    uint8_t *dst = new uint8_t[LIMITSIZE];
    uint32_t offset = 0;
    audioEnhanceChain->DeinterleaverData(src, channel, dst, offset);

    std::unique_ptr<EnhanceBuffer> enhanceBuffer = std::make_unique<EnhanceBuffer>();
    audioEnhanceChain->GetOneFrameInputData(enhanceBuffer);

    uint32_t length = GetData<uint32_t>();
    audioEnhanceChain->ApplyEnhanceChain(enhanceBuffer, length);

    std::string enhanceSet = "enhanceSet";
    std::string propertySet = "propertySet";
    audioEnhanceChain->SetEnhanceProperty(enhanceSet, propertySet);
}

void AudioEnhanceChainManagerMoreFuzzTest()
{
    AudioEnhanceChainManager *audioEnhanceChainMananger = AudioEnhanceChainManager::GetInstance();

    audioEnhanceChainMananger->enhanceBuffer_ = std::make_unique<EnhanceBuffer>();
    uint32_t lengthBuffer = VALID_BUFFER_SIZE;
    std::vector<uint8_t> dummyData(lengthBuffer, 0xAA);
    audioEnhanceChainMananger->CopyToEnhanceBuffer(dummyData.data(), lengthBuffer);
    audioEnhanceChainMananger->CopyEcToEnhanceBuffer(dummyData.data(), lengthBuffer);
    audioEnhanceChainMananger->CopyMicRefToEnhanceBuffer(dummyData.data(), lengthBuffer);
    audioEnhanceChainMananger->CopyFromEnhanceBuffer(dummyData.data(), lengthBuffer);

    uint32_t sceneKeyCode = GetData<uint32_t>();
    uint32_t length = GetData<uint32_t>();
    audioEnhanceChainMananger->ApplyAudioEnhanceChain(sceneKeyCode, length);
    audioEnhanceChainMananger->UpdatePropertyAndSendToAlgo(DEVICE_TYPE_EARPIECE);

    uint32_t captureId = GetData<uint32_t>();
    audioEnhanceChainMananger->ApplyAudioEnhanceChainDefault(captureId, length);
}

void AudioEnhanceChainAdapterMoreFuzzTest()
{
    void *data = nullptr;
    uint32_t length = GetData<uint32_t>();
    CopyToEnhanceBufferAdapter(data, length);
    CopyEcdataToEnhanceBufferAdapter(data, length);
    CopyMicRefdataToEnhanceBufferAdapter(data, length);
    CopyFromEnhanceBufferAdapter(data, length);

    uint32_t sceneKeyCode = GetData<uint32_t>();
    EnhanceChainManagerProcess(sceneKeyCode, length);

    uint32_t captureId = GetData<uint32_t>();
    EnhanceChainManagerProcessDefault(captureId, length);
}

typedef void (*TestFuncs[6])();

TestFuncs g_testFuncs = {
    AudioEffectChainEnhanceFuzzTest,
    AudioEffectChainManagerEnhanceFuzzTest,
    AudioEffectChainAdapterFuzzTest,
    AudioEnhanceChainMoreFuzzTest,
    AudioEnhanceChainManagerMoreFuzzTest,
    AudioEnhanceChainAdapterMoreFuzzTest,
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
