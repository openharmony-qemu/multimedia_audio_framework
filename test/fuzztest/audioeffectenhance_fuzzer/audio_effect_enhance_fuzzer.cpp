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
#include "audio_effect_chain.h"
#include "audio_effect_chain_adapter.h"
#include "audio_effect_chain_manager.h"
#include "audio_enhance_chain.h"
#include "audio_enhance_chain_adapter.h"
#include "audio_enhance_chain_manager.h"
#define VALID_BUFFER_SIZE 1000
using namespace std;

namespace OHOS {
namespace AudioStandard {
const int32_t LIMITSIZE = 4;
const string EXTRASCENETYPE = "2";
const uint32_t AUDIOEFFECTSCENE_LENGTH = 6;
const uint32_t CHANNEL_NUM = 10;
const uint32_t LENGTH_NUM = 10;
const float FLOAT_VOLUME = 1.0f;
constexpr uint32_t INFOCHANNELS = 2;
constexpr uint64_t INFOCHANNELLAYOUT = 0x3;

void AudioEffectChainEnhance(const uint8_t *rawData, size_t size, std::shared_ptr<AudioEffectChain> audioEffectChain)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    int32_t streamUsage = *reinterpret_cast<const int32_t*>(rawData);
    audioEffectChain->SetStreamUsage(streamUsage);

    std::string effect = "";
    std::string property = "";
    audioEffectChain->SetEffectProperty(effect, property);

    audioEffectChain->UpdateEffectParam();
    audioEffectChain->UpdateMultichannelIoBufferConfigInner();

    float volume = FLOAT_VOLUME;
    audioEffectChain->GetFinalVolume();
    audioEffectChain->SetFinalVolume(volume);
}

void AudioEffectChainEnhanceFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
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
    uint32_t currSceneType_int = *reinterpret_cast<const uint32_t*>(rawData);
    currSceneType_int = (currSceneType_int % AUDIOEFFECTSCENE_LENGTH);
    AudioEffectScene currSceneType = static_cast<AudioEffectScene>(currSceneType_int);
    audioEffectChain->SetEffectCurrSceneType(currSceneType);
    AudioEffectChainEnhance(rawData, size, audioEffectChain);
}

void AudioEffectChainManagerEnhanceFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

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
    uint32_t maxSessionID = 1;
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

void AudioEffectChainAdapterFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

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

void AudioEnhanceChainMoreFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

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

    uint32_t length = LENGTH_NUM;
    audioEnhanceChain->ApplyEnhanceChain(enhanceBuffer, length);

    std::string enhanceSet = "enhanceSet";
    std::string propertySet = "propertySet";
    audioEnhanceChain->SetEnhanceProperty(enhanceSet, propertySet);
}

void AudioEnhanceChainManagerMoreFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioEnhanceChainManager *audioEnhanceChainMananger = AudioEnhanceChainManager::GetInstance();

    audioEnhanceChainMananger->enhanceBuffer_ = std::make_unique<EnhanceBuffer>();
    uint32_t lengthBuffer = VALID_BUFFER_SIZE;
    std::vector<uint8_t> dummyData(lengthBuffer, 0xAA);
    audioEnhanceChainMananger->CopyToEnhanceBuffer(dummyData.data(), lengthBuffer);
    audioEnhanceChainMananger->CopyEcToEnhanceBuffer(dummyData.data(), lengthBuffer);
    audioEnhanceChainMananger->CopyMicRefToEnhanceBuffer(dummyData.data(), lengthBuffer);
    audioEnhanceChainMananger->CopyFromEnhanceBuffer(dummyData.data(), lengthBuffer);

    uint32_t sceneKeyCode = *reinterpret_cast<const uint32_t*>(rawData);
    uint32_t length = *reinterpret_cast<const uint32_t*>(rawData);
    audioEnhanceChainMananger->ApplyAudioEnhanceChain(sceneKeyCode, length);
    audioEnhanceChainMananger->UpdatePropertyAndSendToAlgo(DEVICE_TYPE_EARPIECE);

    uint32_t captureId = *reinterpret_cast<const uint32_t*>(rawData);
    audioEnhanceChainMananger->ApplyAudioEnhanceChainDefault(captureId, length);
}

void AudioEnhanceChainAdapterMoreFuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    void *data = nullptr;
    uint32_t length = 0;
    CopyToEnhanceBufferAdapter(data, length);
    CopyEcdataToEnhanceBufferAdapter(data, length);
    CopyMicRefdataToEnhanceBufferAdapter(data, length);
    CopyFromEnhanceBufferAdapter(data, length);

    uint32_t sceneKeyCode = 0;
    EnhanceChainManagerProcess(sceneKeyCode, length);

    uint32_t captureId = 0;
    EnhanceChainManagerProcessDefault(captureId, length);
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *rawData, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::AudioEffectChainEnhanceFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioEffectChainManagerEnhanceFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioEffectChainAdapterFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioEnhanceChainMoreFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioEnhanceChainManagerMoreFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioEnhanceChainAdapterMoreFuzzTest(rawData, size);
    return 0;
}
