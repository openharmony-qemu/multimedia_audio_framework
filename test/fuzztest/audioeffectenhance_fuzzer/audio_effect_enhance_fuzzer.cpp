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
// Enum-range moduli used to clamp fuzz ints into valid DeviceType / AudioVolumeType ranges.
const int32_t DEVICE_TYPE_RANGE = 20;
const int32_t VOLUME_TYPE_RANGE = 10;
// EffectChainManagerParam.maxExtraNum default value.
const uint32_t DEFAULT_MAX_EXTRA_NUM = 3;
// Dummy buffer length and fill byte used by AudioEnhanceChainManagerMoreFuzzTest.
const uint32_t DUMMY_BUFFER_LEN = 1000;
const uint8_t DUMMY_DATA_FILL_BYTE = 0x08;
vector<EffectChain> DEFAULT_EFFECT_CHAINS = {{"EFFECTCHAIN_SPK_MUSIC", {}, ""}, {"EFFECTCHAIN_BT_MUSIC", {}, ""}};
vector<shared_ptr<AudioEffectLibEntry>> DEFAULT_EFFECT_LIBRARY_LIST = {};

EffectChainManagerParam DEFAULT_EFFECT_CHAIN_MANAGER_PARAM{
    DEFAULT_MAX_EXTRA_NUM,
    "SCENE_DEFAULT",
    {},
    {{"SCENE_MOVIE_&_EFFECT_DEFAULT_&_DEVICE_TYPE_SPEAKER", "EFFECTCHAIN_SPK_MUSIC"},
        {"SCENE_MOVIE_&_EFFECT_DEFAULT_&_DEVICE_TYPE_BLUETOOTH_A2DP", "EFFECTCHAIN_BT_MUSIC"}},
    {{"effect1", "property1"}, {"effect4", "property5"}, {"effect1", "property4"}}
};

// ENHANCE-specific Init config for AudioEnhanceChainManagerImpl (Plan A+, no faked
// AudioEffectLibrary). sceneType "SCENE_RECORD"/"SCENE_VOIP_UP" are the values of
// AudioEnhanceScene::SCENE_RECORD(=1)/SCENE_VOIP_UP(=0) (audio_effect.h:485/484) in the
// GetEnhanceSupportedSceneType() whitelist (audio_effect_map.cpp:43-44). The
// chainConfigInfoMap_ keys "SCENE_RECORD_&_ENHANCE_DEFAULT"/"SCENE_VOIP_UP_&_ENHANCE_DEFAULT"
// match the fallback lookup in GetEnhanceNamesBySceneCode
// (audio_enhance_chain_manager_impl.cpp:335). capturerDevice DEVICE_TYPE_MIC is in the
// GetSupportedDeviceType() whitelist (audio_device_type.cpp:36) so ParseSceneKeyCode succeeds.
// Non-empty libList (entries with null audioEffectLibHandle) lets FindEnhanceLib match
// (:78) so InitAudioEnhanceChainManager fills enhanceConfigInfoMap_ (:184-189),
// AddAudioEnhanceChainHandles resolves each enhance (:391-395), and
// SetRelateWithDevicePropForEnhance's VOIP_UP inner loop (:110-119) sets relateWithDevice.
// The null libHandle still makes CreateAllEnhanceModule return ERROR
// (audio_enhance_chain.cpp:383-386), so CreateAudioEnhanceChainDynamic keeps the failure
// path -- no chain enters ChainPool.
shared_ptr<AudioEffectLibEntry> MakeFakeEnhanceLibEntry(const string &libName,
    const vector<string> &effectNames)
{
    auto entry = make_shared<AudioEffectLibEntry>();
    entry->libraryName = libName;
    entry->effectName = effectNames;
    entry->audioEffectLibHandle = nullptr;
    return entry;
}

vector<EffectChain> ENHANCE_EFFECT_CHAINS = {
    {"ENHANCECHAIN_RECORD", {"enhance_record"}, ""},
    {"ENHANCECHAIN_VOIP", {"enhance_voip"}, ""},
};
vector<shared_ptr<AudioEffectLibEntry>> ENHANCE_LIBRARY_LIST = {
    MakeFakeEnhanceLibEntry("fake_lib_record", {"enhance_record"}),
    MakeFakeEnhanceLibEntry("fake_lib_voip", {"enhance_voip"}),
};

EffectChainManagerParam ENHANCE_CHAIN_MANAGER_PARAM{
    DEFAULT_MAX_EXTRA_NUM,
    "SCENE_DEFAULT",
    {},
    {{"SCENE_RECORD_&_ENHANCE_DEFAULT", "ENHANCECHAIN_RECORD"},
        {"SCENE_VOIP_UP_&_ENHANCE_DEFAULT", "ENHANCECHAIN_VOIP"}},
    {{"enhance_record", "default_prop"}, {"enhance_voip", "voip_prop"}},
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
    AudioEffectChainManager::GetInstance()->ResetInfo();
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

    bool isMute = (GetData<uint32_t>() % NUM_2 == 0);
    uint32_t systemVol = GetData<uint32_t>();
    audioEnhanceChain->SetEnhanceParam(isMute, systemVol);

    uint32_t foldState = GetData<uint32_t>();
    audioEnhanceChain->SetFoldState(foldState);

    audioEnhanceChain->SetThreadHandler(nullptr);
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

    const uint32_t bufferLen = DUMMY_BUFFER_LEN;
    std::vector<uint8_t> dummyData(bufferLen, DUMMY_DATA_FILL_BYTE);
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

void AudioEnhanceReleaseChainFuzzTest()
{
    AudioEnhanceChainManager *mgr = AudioEnhanceChainManager::GetInstance();
    if (mgr == nullptr) {
        return;
    }
    uint64_t sceneKeyCode = GetData<uint64_t>();
    mgr->ReleaseAudioEnhanceChainDynamic(sceneKeyCode);
}

void AudioEnhanceDeviceVolumeFuzzTest()
{
    AudioEnhanceChainManager *mgr = AudioEnhanceChainManager::GetInstance();
    if (mgr == nullptr) {
        return;
    }
    uint32_t captureId = GetData<uint32_t>();
    DeviceType inputDevice = static_cast<DeviceType>(GetData<int32_t>() % DEVICE_TYPE_RANGE);
    mgr->SetInputDevice(captureId, inputDevice);

    uint32_t renderId = GetData<uint32_t>();
    DeviceType outputDevice = static_cast<DeviceType>(GetData<int32_t>() % DEVICE_TYPE_RANGE);
    mgr->SetOutputDevice(renderId, outputDevice);

    AudioVolumeType volumeType = static_cast<AudioVolumeType>(GetData<int32_t>() % VOLUME_TYPE_RANGE);
    float systemVol = GetData<float>();
    mgr->SetVolumeInfo(volumeType, systemVol);

    bool isMute = GetData<bool>();
    mgr->SetMicrophoneMuteInfo(isMute);

    uint32_t sessionId = GetData<uint32_t>();
    float streamVol = GetData<float>();
    mgr->SetStreamVolumeInfo(sessionId, streamVol);

    mgr->ResetInfo();
}

void AudioEnhanceStreamControlFuzzTest()
{
    AudioEnhanceChainManager *mgr = AudioEnhanceChainManager::GetInstance();
    if (mgr == nullptr) {
        return;
    }
    mgr->NotifyChainToStopStream();
    mgr->ResetInfo();
}

void AudioEnhanceMuteStateFuzzTest()
{
    AudioEnhanceChainManager *mgr = AudioEnhanceChainManager::GetInstance();
    if (mgr == nullptr) {
        return;
    }
    uint64_t sceneKeyCode = GetData<uint64_t>();
    bool isMute = GetData<bool>();
    mgr->SetAppsMuteStateToChainById(sceneKeyCode, isMute);
}

// InitAudioEnhanceChainManager on the enhance manager singleton (uses the file-level DEFAULT_*
// params). Covers the init body (158-197). ResetInfo at the end restores clean state so the
// shared singleton does not disturb other tests that assume an un-initialized manager.
void EnhanceManagerInitFuzzTest()
{
    AudioEnhanceChainManager *mgr = AudioEnhanceChainManager::GetInstance();
    if (mgr == nullptr) {
        return;
    }
    mgr->InitAudioEnhanceChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_EFFECT_CHAIN_MANAGER_PARAM,
        DEFAULT_EFFECT_LIBRARY_LIST);
    mgr->ResetInfo();
}

// AudioEnhanceChainGetAlgoConfig (426-438) + SetAudioEnhanceProperty (582-607, non-empty array ->
// loop body) + GetAudioEnhanceProperty (644-659, non-NONE deviceType -> db-update branch).
void EnhanceManagerAlgoConfigAndPropertyFuzzTest()
{
    AudioEnhanceChainManager *mgr = AudioEnhanceChainManager::GetInstance();
    if (mgr == nullptr) {
        return;
    }
    uint64_t sceneKeyCode = GetData<uint64_t>();
    AudioBufferConfig micConfig;
    AudioBufferConfig ecConfig;
    AudioBufferConfig micRefConfig;
    mgr->AudioEnhanceChainGetAlgoConfig(sceneKeyCode, micConfig, ecConfig, micRefConfig);

    AudioEffectPropertyArray propArray;
    AudioEffectProperty prop;
    prop.name = "effect1";
    prop.category = "category1";
    propArray.property.push_back(prop); // non-empty -> SetAudioEnhanceProperty loop body runs
    mgr->SetAudioEnhanceProperty(propArray, DEVICE_TYPE_SPEAKER);

    AudioEffectPropertyArray getResult;
    mgr->GetAudioEnhanceProperty(getResult, DEVICE_TYPE_SPEAKER); // non-NONE -> db-update branch
}

// Max mic channel count for EnhanceManagerCreateChainFuzzTest. AudioEnhanceChain::
// InitAudioEnhanceChain multiplies micChannels * byteLenPerFrame(=1920) inside resize()
// (audio_enhance_chain.cpp:110/111/113), so an unconstrained fuzz uint32 overflows that
// product (ubsan mul-overflow). Real mic channel counts are small.
constexpr uint32_t ENHANCE_MIC_CHANNEL_MAX = 16;

// Bit offsets for sceneKeyCode encoding, mirroring the anonymous-namespace
// constants in audio_enhance_chain_manager_impl.cpp:41-42 (not exported, so
// redefined here). sceneKeyCode = sceneCode<<SCENE_TYPE_OFFSET |
// captureId<<CAPTURER_ID_OFFSET | renderId.
constexpr uint32_t SCENE_TYPE_OFFSET = 32;
constexpr uint32_t CAPTURER_ID_OFFSET = 16;

// Build a sceneKeyCode from sceneCode/captureId/renderId per the encoding in
// audio_enhance_chain_manager_impl.cpp:41-44.
uint64_t BuildEnhanceSceneKeyCode(uint32_t sceneCode, uint32_t captureId, uint32_t renderId)
{
    return (static_cast<uint64_t>(sceneCode) << SCENE_TYPE_OFFSET) |
           (static_cast<uint64_t>(captureId) << CAPTURER_ID_OFFSET) |
           static_cast<uint64_t>(renderId);
}

// Covers CreateAudioEnhanceChainDynamic (282-316) and its private callees:
// CreateEnhanceChainInner (346-382, NORMAL_SCENE path), GetEnhanceNamesBySceneCode
// (318-344, both specific-device miss at 333-334 and no-device hit at 336-337),
// ParseSceneKeyCode (199-246, success path), GetThreadHandlerByScene (248-268, create
// path), AddAudioEnhanceChainHandles (384-407, nullptr libHandle -> ERROR at 404),
// ReleaseThreadHandlerByScene (270-280, erase path at 278). With empty libList the
// chain is not added to ChainPool, so CreateAudioEnhanceChainDynamic takes the failure
// path (306-310) and ResetInfo fully restores singleton state.
void EnhanceManagerCreateChainFuzzTest()
{
    AudioEnhanceChainManager *mgr = AudioEnhanceChainManager::GetInstance();
    if (mgr == nullptr) {
        return;
    }
    mgr->InitAudioEnhanceChainManager(ENHANCE_EFFECT_CHAINS, ENHANCE_CHAIN_MANAGER_PARAM,
        ENHANCE_LIBRARY_LIST);

    // Register captureId -> DEVICE_TYPE_MIC so ParseSceneKeyCode resolves it (216-222).
    uint32_t captureId = GetData<uint32_t>() & 0xFFFF;
    mgr->SetInputDevice(captureId, DEVICE_TYPE_MIC);

    // sceneCode = SCENE_RECORD (1) is in GetEnhanceSupportedSceneType() whitelist.
    uint64_t sceneKeyCode = BuildEnhanceSceneKeyCode(SCENE_RECORD, captureId, 0);
    AudioEnhanceDeviceAttr deviceAttr = {};
    deviceAttr.micRate = GetData<uint32_t>();
    // Clamp micChannels to 1..ENHANCE_MIC_CHANNEL_MAX: it feeds micChannels * byteLenPerFrame
    // resize() multipliers in AudioEnhanceChain::InitAudioEnhanceChain (audio_enhance_chain.cpp:
    // 110/111/113), which overflow uint32 on an unconstrained fuzz value.
    deviceAttr.micChannels = GetData<uint32_t>() % ENHANCE_MIC_CHANNEL_MAX + 1;
    mgr->CreateAudioEnhanceChainDynamic(sceneKeyCode, deviceAttr);

    mgr->ResetInfo();
}

// Covers UpdateExtraSceneType (661-695) across all branches and the SendFoldStateToChain
// (697-705) / SendPowerStateToChain (707-715) entry points. ChainPool is empty so the
// send-to-chain loop bodies do not run, but enhancePara_.foldState/powerState are set
// (680, 687). FOLD_STATE_EXPAND(1)..FOLD_STATE_MIDDLE(3) and power range [0,100] come
// from audio_effect.h:80-82 and BATTERY_CAPACITY (impl.cpp:46).
void EnhanceManagerUpdateExtraSceneTypeFuzzTest()
{
    AudioEnhanceChainManager *mgr = AudioEnhanceChainManager::GetInstance();
    if (mgr == nullptr) {
        return;
    }
    const std::string mainkey = "device_status";

    // fold_state valid range [FOLD_STATE_EXPAND, FOLD_STATE_MIDDLE].
    uint32_t foldSpan = FOLD_STATE_MIDDLE - FOLD_STATE_EXPAND + 1; // 3
    uint32_t foldVal = FOLD_STATE_EXPAND + (GetData<uint32_t>() % foldSpan); // 1..3
    mgr->UpdateExtraSceneType(mainkey, "fold_state", std::to_string(foldVal));

    // power_state valid range [0, BATTERY_CAPACITY(100)].
    const uint32_t batteryCapacity = 100;
    uint32_t powerVal = GetData<uint32_t>() % (batteryCapacity + 1); // 0..100
    mgr->UpdateExtraSceneType(mainkey, "power_state", std::to_string(powerVal));

    // empty string -> early return (665-668).
    mgr->UpdateExtraSceneType(mainkey, "fold_state", "");

    // non-numeric -> from_chars fails -> return (670-673).
    mgr->UpdateExtraSceneType(mainkey, "power_state", "abc");

    // out-of-range fold (4 > FOLD_STATE_MIDDLE) -> else branch (690-694).
    mgr->UpdateExtraSceneType(mainkey, "fold_state", "4");

    // out-of-range power (101 > batteryCapacity) -> else branch.
    mgr->UpdateExtraSceneType(mainkey, "power_state", "101");

    // mismatched mainkey/subkey -> else branch.
    mgr->UpdateExtraSceneType("wrong_key", "wrong_sub", "5");

    mgr->ResetInfo();
}

// Covers SetInputDevice (489-534): first registration (495-499), same-device early
// return (500-503), different-device update path (505-533) which triggers
// UpdatePropertyAndSendToAlgo (474-487), and out-of-whitelist device -> ERROR
// (508-512). DEVICE_TYPE_MIC/SPEAKER are in GetSupportedDeviceType() whitelist
// (audio_device_type.cpp:36,30). Init populates enhancePropertyMap_ so the
// UpdatePropertyAndSendToAlgo loop (478-485) iterates (ChainPool empty -> inner
// chain loop skipped). ResetInfo restores state.
void EnhanceManagerSetInputDeviceCoreFuzzTest()
{
    AudioEnhanceChainManager *mgr = AudioEnhanceChainManager::GetInstance();
    if (mgr == nullptr) {
        return;
    }
    mgr->InitAudioEnhanceChainManager(ENHANCE_EFFECT_CHAINS, ENHANCE_CHAIN_MANAGER_PARAM,
        ENHANCE_LIBRARY_LIST);

    uint32_t captureId = GetData<uint32_t>() & 0xFFFF;

    // Branch 1: captureId absent -> insert + return SUCCESS (495-499).
    mgr->SetInputDevice(captureId, DEVICE_TYPE_MIC);

    // Branch 2: same device -> early return SUCCESS (500-503).
    mgr->SetInputDevice(captureId, DEVICE_TYPE_MIC);

    // Branch 3: different device (SPEAKER, in whitelist) -> update + UpdatePropertyAndSendToAlgo.
    mgr->SetInputDevice(captureId, DEVICE_TYPE_SPEAKER);

    // Branch 4: device not in GetSupportedDeviceType() whitelist -> return ERROR (508-512).
    const int32_t outOfRangeDevice = 200;
    DeviceType invalidDevice = static_cast<DeviceType>(outOfRangeDevice);
    mgr->SetInputDevice(captureId, invalidDevice);

    mgr->ResetInfo();
}

using FuzzFunc = decltype(AudioEffectChainEnhanceFuzzTest);
FuzzFunc *g_fuzzFuncs[] = {
    AudioEffectChainEnhanceFuzzTest,
    AudioEffectChainManagerEnhanceFuzzTest,
    AudioEnhanceChainMoreFuzzTest,
    AudioEnhanceChainManagerMoreFuzzTest,
    AudioEnhanceChainAdapterMoreFuzzTest,
    AudioEnhanceChainSetParaFuzzTest,
    AudioEnhanceChainGetParaFuzzTest,
    ChainPoolFuzzTest,
    ThreadHandlerFuzzTest,
    AudioEnhanceReleaseChainFuzzTest,
    AudioEnhanceDeviceVolumeFuzzTest,
    AudioEnhanceStreamControlFuzzTest,
    AudioEnhanceMuteStateFuzzTest,
    EnhanceManagerInitFuzzTest,
    EnhanceManagerAlgoConfigAndPropertyFuzzTest,
    EnhanceManagerCreateChainFuzzTest,
    EnhanceManagerUpdateExtraSceneTypeFuzzTest,
    EnhanceManagerSetInputDeviceCoreFuzzTest,
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
