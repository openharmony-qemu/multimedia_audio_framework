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
#include "securec.h"

#include "audio_effect_log.h"
#include "audio_effect_chain.h"
#ifdef SUPPORT_OLD_ENGINE
#include "audio_effect_chain_adapter.h"
#include "audio_enhance_chain_adapter.h"
#endif
#include "audio_effect_chain_manager.h"
#include "audio_enhance_chain.h"
#include "audio_enhance_chain_manager.h"
#include "chain_pool.h"
#include "thread_handler.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
const int32_t LIMITSIZE = 4;
const string EXTRASCENETYPE = "2";
const uint32_t AUDIOEFFECTSCENE_LENGTH = 6;
const uint32_t CHANNEL_NUM = 10;
constexpr uint32_t INFOCHANNELS = 2;
constexpr uint64_t INFOCHANNELLAYOUT = 0x3;
static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
const int32_t NUM_2 = 2;
vector<EffectChain> DEFAULT_EFFECT_CHAINS = {{"EFFECTCHAIN_SPK_MUSIC", {}, ""}, {"EFFECTCHAIN_BT_MUSIC", {}, ""}};
vector<shared_ptr<AudioEffectLibEntry>> DEFAULT_EFFECT_LIBRARY_LIST = {};

EffectChainManagerParam DEFAULT_EFFECT_CHAIN_MANAGER_PARAM{
    3,
    "SCENE_DEFAULT",
    {},
    {{"SCENE_MOVIE_&_EFFECT_DEFAULT_&_DEVICE_TYPE_SPEAKER", "EFFECTCHAIN_SPK_MUSIC"},
        {"SCENE_MOVIE_&_EFFECT_DEFAULT_&_DEVICE_TYPE_BLUETOOTH_A2DP", "EFFECTCHAIN_BT_MUSIC"}},
    {{"effect1", "property1"}, {"effect4", "property5"}, {"effect1", "property4"}}
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
    };

    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_[sessionID] = sessionEffectInfo;
    const std::string scenePairType = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    AudioEffectChainManager::GetInstance()->FindMaxSessionID(maxSessionID, sceneTypeMax, scenePairType, sessions);
}

void AudioEffectChainAdapterFuzzTest()
{
#ifdef SUPPORT_OLD_ENGINE
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
#endif
}

void ConvertChLayoutToPaChMapFuzzTest()
{
#ifdef SUPPORT_OLD_ENGINE
    const char *sceneType = "SCENE_MUSIC";
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    EffectChainManagerInitCb(sceneType);

    const uint64_t channelLayout = CH_LAYOUT_MONO;
    pa_channel_map processCm;
    ConvertChLayoutToPaChMap(channelLayout, &processCm);

    const uint64_t channelLayout2 = CH_LAYOUT_STEREO;
    pa_channel_map processCm2;
    ConvertChLayoutToPaChMap(channelLayout2, &processCm2);

    AudioEffectChainManager::GetInstance()->ResetInfo();
#endif
}

void EffectChainManagerDeleteSessionInfoFuzzTest()
{
#ifdef SUPPORT_OLD_ENGINE
    const char *sceneType = "SCENE_MUSIC";
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    EffectChainManagerInitCb(sceneType);

    const char *sessionid = "123456";
    SessionInfoPack pack = {2, "3", "EFFECT_DEFAULT", "true", "1", "1"};
    EffectChainManagerAddSessionInfo(sceneType, sessionid, pack);

    sceneType = nullptr;
    // Intentionally test with nullptr for fuzzing
    EffectChainManagerDeleteSessionInfo(sceneType, sessionid);
    AudioEffectChainManager::GetInstance()->ResetInfo();
#endif
}

void EffectChainManagerReturnEffectChannelInfoFuzzTest()
{
#ifdef SUPPORT_OLD_ENGINE
    const char *sceneType = "test_scene";
    uint32_t channels = 0;
    EffectChainManagerReturnEffectChannelInfo(sceneType, &channels, nullptr);
#endif
}

void EffectChainManagerSceneCheckFuzzTest()
{
#ifdef SUPPORT_OLD_ENGINE
    EffectChainManagerSceneCheck(nullptr, nullptr);
#endif
}

void EffectChainManagerGetSceneCountFuzzTest()
{
#ifdef SUPPORT_OLD_ENGINE
    EffectChainManagerGetSceneCount(nullptr);
#endif
}

void AudioEnhanceChainMoreFuzzTest()
{
    std::string scene = "scene";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    uint64_t chainId = GetData<uint64_t>();
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain =
        std::make_shared<AudioEnhanceChain>(chainId, scene, ScenePriority::PRIOR_SCENE, algoParam, deviceAttr);
    if (audioEnhanceChain == nullptr) {
        return;
    }

    std::vector<uint8_t> src(LIMITSIZE);
    std::vector<uint8_t> dst(LIMITSIZE);
    uint32_t channel = CHANNEL_NUM;
    uint32_t offset = 0;
    audioEnhanceChain->DeinterleaverData(src.data(), channel, dst.data(), offset);

    EnhanceTransBuffer transBuf = {};
    transBuf.micData = dst.data();
    transBuf.micDataLen = dst.size();
    audioEnhanceChain->ApplyEnhanceChain(transBuf);

    std::vector<uint8_t> outBuf(LIMITSIZE);
    audioEnhanceChain->GetOutputDataFromChain(outBuf.data(), outBuf.size());
}

void AudioEnhanceChainSetParaFuzzTest()
{
    std::string scene = "RECORD";
    AudioEnhanceParamAdapter algoParam = {};
    AudioEnhanceDeviceAttr deviceAttr = {};
    uint64_t chainId = GetData<uint64_t>();
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain =
        std::make_shared<AudioEnhanceChain>(chainId, scene, ScenePriority::NORMAL_SCENE, algoParam, deviceAttr);
    if (audioEnhanceChain == nullptr) {
        return;
    }

    std::vector<EnhanceModulePara> moduleParas;
    audioEnhanceChain->CreateAllEnhanceModule(moduleParas);

    std::string inputDevice = "inputDevice";
    std::string deviceName = "deviceName";
    audioEnhanceChain->SetInputDevice(inputDevice, deviceName);

    std::string effect = "voip_up";
    std::string prop = "test_prop";
    audioEnhanceChain->SetEnhanceProperty(effect, prop);

    bool isMute = (GetData<uint32_t>() % 2 == 0);
    uint32_t systemVol = GetData<uint32_t>();
    audioEnhanceChain->SetEnhanceParam(isMute, systemVol);

    uint32_t foldState = GetData<uint32_t>();
    audioEnhanceChain->SetFoldState(foldState);

    audioEnhanceChain->SetThreadHandler(nullptr);
    audioEnhanceChain->InitCommand();
}

void AudioEnhanceChainGetParaFuzzTest()
{
    std::string scene = "VOIP_UP";
    AudioEnhanceParamAdapter algoParam = {};
    AudioEnhanceDeviceAttr deviceAttr = {};
    uint64_t chainId = GetData<uint64_t>();
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain =
        std::make_shared<AudioEnhanceChain>(chainId, scene, ScenePriority::NORMAL_SCENE, algoParam, deviceAttr);
    if (audioEnhanceChain == nullptr) {
        return;
    }

    audioEnhanceChain->IsEmptyEnhanceHandles();
    audioEnhanceChain->GetChainId();
    audioEnhanceChain->GetScenePriority();

    AudioBufferConfig micConfig = {};
    AudioBufferConfig ecConfig = {};
    AudioBufferConfig micRefConfig = {};
    audioEnhanceChain->GetAlgoConfig(micConfig, ecConfig, micRefConfig);
}

void ChainPoolFuzzTest()
{
    ChainPool::GetInstance().AddChain(nullptr);

    std::string scene = "TRANS";
    AudioEnhanceParamAdapter algoParam = {};
    AudioEnhanceDeviceAttr deviceAttr = {};
    uint64_t chainId = GetData<uint64_t>();
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain =
        std::make_shared<AudioEnhanceChain>(chainId, scene, ScenePriority::NORMAL_SCENE, algoParam, deviceAttr);

    ChainPool::GetInstance().AddChain(audioEnhanceChain);
    ChainPool::GetInstance().GetChainById(chainId);
    ChainPool::GetInstance().GetAllChain();
    ChainPool::GetInstance().DeleteChain(chainId);
}

void ThreadHandlerFuzzTest()
{
    std::string name = "test_thread";
    auto handler = ThreadHandler::NewInstance(name);
    if (handler == nullptr) {
        return;
    }

    auto task1 = []() {
        AUDIO_INFO_LOG("execute task 1");
    };
    handler->PostTask(task1);

    auto task2 = []() {
        AUDIO_INFO_LOG("execute task 2");
    };
    handler->EnsureTask(task2);
}

void AudioEnhanceChainManagerMoreFuzzTest()
{
    AudioEnhanceChainManager *audioEnhanceChainMananger = AudioEnhanceChainManager::GetInstance();

    const uint32_t bufferLen = 1000;
    std::vector<uint8_t> dummyData(bufferLen, 0x08);
    EnhanceTransBuffer transBuf = {};
    transBuf.micData = dummyData.data();
    transBuf.micDataLen = dummyData.size();

    uint64_t sceneKeyCode = GetData<uint64_t>();
    audioEnhanceChainMananger->ApplyEnhanceChainById(sceneKeyCode, transBuf);

    std::vector<uint8_t> output(bufferLen);
    audioEnhanceChainMananger->GetChainOutputDataById(sceneKeyCode, output.data(), output.size());
}

void AudioEnhanceChainAdapterMoreFuzzTest()
{
#ifdef SUPPORT_OLD_ENGINE
    void *data = nullptr;
    uint32_t length = GetData<uint32_t>();
    CopyToEnhanceBufferAdapter(data, length);
    CopyEcdataToEnhanceBufferAdapter(data, length);
    CopyMicRefdataToEnhanceBufferAdapter(data, length);
    CopyFromEnhanceBufferAdapter(data, length);

    uint64_t sceneKeyCode = GetData<uint64_t>();
    EnhanceChainManagerProcess(sceneKeyCode, length);

    uint32_t captureId = GetData<uint32_t>();
    EnhanceChainManagerProcessDefault(captureId, length);
#endif
}

using FuzzFunc = decltype(AudioEffectChainEnhanceFuzzTest);
FuzzFunc *g_fuzzFuncs[] = {
    AudioEffectChainEnhanceFuzzTest,
    AudioEffectChainManagerEnhanceFuzzTest,
    AudioEffectChainAdapterFuzzTest,
    ConvertChLayoutToPaChMapFuzzTest,
    EffectChainManagerDeleteSessionInfoFuzzTest,
    EffectChainManagerReturnEffectChannelInfoFuzzTest,
    EffectChainManagerSceneCheckFuzzTest,
    EffectChainManagerGetSceneCountFuzzTest,
    AudioEnhanceChainMoreFuzzTest,
    AudioEnhanceChainManagerMoreFuzzTest,
    AudioEnhanceChainAdapterMoreFuzzTest,
    AudioEnhanceChainSetParaFuzzTest,
    AudioEnhanceChainGetParaFuzzTest,
    ChainPoolFuzzTest,
    ThreadHandlerFuzzTest,
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
    uint32_t len = GetArrLength(g_fuzzFuncs);
    if (len > 0) {
        g_fuzzFuncs[code % len]();
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
