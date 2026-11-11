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

#include <cstddef>
#include <cstdint>
#include "securec.h"

#include "audio_effect.h"
#include "audio_effect_chain.h"
#include "audio_effect_log.h"
#include "audio_effect_chain_manager.h"
#ifdef SUPPORT_OLD_ENGINE
#include "audio_enhance_chain_adapter.h"
#endif
#include "audio_enhance_chain_manager.h"
#include "audio_enhance_chain_manager_impl.h"
#include "audio_errors.h"
#include "audio_head_tracker.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

constexpr uint32_t INFOCHANNELS = 2;
constexpr uint64_t INFOCHANNELLAYOUT = 0x3;
const char* SCENETYPEMUSIC = "SCENE_MUSIC";
const char* SESSIONIDDEFAULT = "123456";
const char* EFFECTDEFAULT = "EFFECT_DEFAULT";
const uint32_t AUDIOEFFECTSCENE_LENGTH = 6;
const uint32_t AUDIOENCODINGTYPE_LENGTH = 3;
const string EXTRASCENETYPE = "2";
const int32_t DEFAULT_RATE = 48000;
const int32_t DEFAULT_CHANNEL = 4;
const int32_t DEFAULT_FORMAT = 1;
const int32_t MAX_EXTRA_NUM = 3;
const float SYSTEM_VOLINFO = 0.75f;
static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
static size_t g_count = 0;
const size_t THRESHOLD = 10;
const int32_t NUM_2 = 2;
const int32_t TEST_HANDLE_SIZE = 10;
typedef void (*TestFuncs)();

vector<EffectChain> DEFAULT_EFFECT_CHAINS = {{"EFFECTCHAIN_SPK_MUSIC", {}, ""}, {"EFFECTCHAIN_BT_MUSIC", {}, ""}};
vector<shared_ptr<AudioEffectLibEntry>> DEFAULT_EFFECT_LIBRARY_LIST = {};
EffectChainManagerParam DEFAULT_MAP{
    3,
    "SCENE_DEFAULT",
    {},
    {{"SCENE_MOVIE_&_EFFECT_DEFAULT_&_DEVICE_TYPE_SPEAKER", "EFFECTCHAIN_SPK_MUSIC"},
        {"SCENE_MOVIE_&_EFFECT_DEFAULT_&_DEVICE_TYPE_BLUETOOTH_A2DP", "EFFECTCHAIN_BT_MUSIC"}},
    {{"effect1", "property1"}, {"effect4", "property5"}, {"effect1", "property4"}}
};

EffectChainManagerParam DEFAULT_EFFECT_CHAIN_MANAGER_PARAM{
    3,
    "SCENE_DEFAULT",
    {},
    {{"SCENE_MOVIE_&_EFFECT_DEFAULT_&_DEVICE_TYPE_SPEAKER", "EFFECTCHAIN_SPK_MUSIC"},
        {"SCENE_MOVIE_&_EFFECT_DEFAULT_&_DEVICE_TYPE_BLUETOOTH_A2DP", "EFFECTCHAIN_BT_MUSIC"}},
    {{"effect1", "property1"}, {"effect4", "property5"}, {"effect1", "property4"}}
};

SessionEffectInfo DEFAULT_INFO = {
    "EFFECT_DEFAULT",
    "SCENE_MOVIE",
    INFOCHANNELS,
    INFOCHANNELLAYOUT,
};

// ENHANCE-specific Init config for AudioEnhanceChainManagerImpl.
// InitAudioEnhanceChainManager (audio_enhance_chain_manager_impl.cpp:158-197) fills
// chainConfigInfoMap_ from sceneTypeToChainNameMap keys, enhanceConfigInfoMap_ from
// libList-matched enhance names, and enhancePropertyMap_ from effectDefaultProperty.
// "SCENE_RECORD"/"SCENE_VOIP_UP" are the names of AudioEnhanceScene::SCENE_RECORD(=1)
// /SCENE_VOIP_UP(=0) in the GetEnhanceSupportedSceneType() whitelist, so the map keys
// match the fallback lookup in GetEnhanceNamesBySceneCode (impl.cpp:335). Entries carry
// a null audioEffectLibHandle so no real algorithm library is ever dlopened in fuzz.
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
    MAX_EXTRA_NUM,
    "SCENE_DEFAULT",
    {},
    {{"SCENE_RECORD_&_ENHANCE_DEFAULT", "ENHANCECHAIN_RECORD"},
        {"SCENE_VOIP_UP_&_ENHANCE_DEFAULT", "ENHANCECHAIN_VOIP"}},
    {{"enhance_record", "default_prop"}, {"enhance_voip", "voip_prop"}},
};

#define DEFAULT_NUM_CHANNEL 2
#define DEFAULT_CHANNELLAYOUT 3

const vector<DeviceType> g_testDeviceTypes = {
    DEVICE_TYPE_NONE,
    DEVICE_TYPE_INVALID,
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_WIRED_HEADPHONES,
    DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_BLUETOOTH_A2DP_IN,
    DEVICE_TYPE_MIC,
    DEVICE_TYPE_WAKEUP,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_DP,
    DEVICE_TYPE_REMOTE_CAST,
    DEVICE_TYPE_USB_DEVICE,
    DEVICE_TYPE_ACCESSORY,
    DEVICE_TYPE_REMOTE_DAUDIO,
    DEVICE_TYPE_HDMI,
    DEVICE_TYPE_LINE_DIGITAL,
    DEVICE_TYPE_NEARLINK,
    DEVICE_TYPE_NEARLINK_IN,
    DEVICE_TYPE_FILE_SINK,
    DEVICE_TYPE_FILE_SOURCE,
    DEVICE_TYPE_EXTERN_CABLE,
    DEVICE_TYPE_DEFAULT,
    DEVICE_TYPE_USB_ARM_HEADSET,
    DEVICE_TYPE_MAX,
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

void AudioEffectChainManagerFirst(std::shared_ptr<AudioEffectChain> audioEffectChain)
{
    audioEffectChain->IsEmptyEffectHandles();
    const uint32_t channels = GetData<uint32_t>();
    const uint64_t channelLayout = GetData<uint64_t>();
    audioEffectChain->UpdateMultichannelIoBufferConfig(channels, channelLayout);
    std::string sceneMode = "EFFECT_DEFAULT";
    AudioEffectConfig ioBufferConfig;
    AudioBufferConfig inputCfg;
    uint32_t samplingRate = GetData<uint32_t>();
    uint32_t channel = GetData<uint32_t>();
    uint8_t format = GetData<uint8_t>();
    uint64_t channelLayouts = GetData<uint64_t>();
    uint32_t encoding_int = GetData<uint32_t>();
    encoding_int = (encoding_int%AUDIOENCODINGTYPE_LENGTH)-1;
    AudioEncodingType encoding = static_cast<AudioEncodingType>(encoding_int);
    inputCfg.samplingRate = samplingRate;
    inputCfg.channels = channel;
    inputCfg.format = format;
    inputCfg.channelLayout = channelLayouts;
    inputCfg.encoding = encoding;
    ioBufferConfig.inputCfg = inputCfg;
    ioBufferConfig.outputCfg = inputCfg;
    audioEffectChain->StoreOldEffectChainInfo(sceneMode, ioBufferConfig);
}

void AudioEffectChainFuzzTest()
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
    uint32_t currSceneTypeInt = GetData<uint32_t>() % AUDIOEFFECTSCENE_LENGTH;
    AudioEffectScene currSceneType = static_cast<AudioEffectScene>(currSceneTypeInt);
    audioEffectChain->SetEffectCurrSceneType(currSceneType);
    AudioEffectChainManagerFirst(audioEffectChain);
}

void AudioEnhanceChainManagerCreateFuzzTest(AudioEnhanceChainManager *manager)
{
    uint64_t sceneKeyCode = GetData<uint64_t>();
    AudioEnhanceDeviceAttr deviceAttr = {};
    manager->CreateAudioEnhanceChainDynamic(sceneKeyCode, deviceAttr);

    AudioBufferConfig micConfig = {};
    AudioBufferConfig ecConfig = {};
    AudioBufferConfig micRefConfig = {};
    manager->AudioEnhanceChainGetAlgoConfig(sceneKeyCode, micConfig, ecConfig, micRefConfig);

    const uint32_t bufLen = 128;
    std::vector<uint8_t> input(bufLen);
    std::vector<uint8_t> output(bufLen);
    EnhanceTransBuffer transBuf = {};
    transBuf.micData = input.data();
    transBuf.micDataLen = input.size();
    manager->ApplyEnhanceChainById(sceneKeyCode, transBuf);
    manager->GetChainOutputDataById(sceneKeyCode, output.data(), output.size());

    manager->ReleaseAudioEnhanceChainDynamic(sceneKeyCode);
}

void AudioEnhanceChainManagerPropertyFuzzTest(AudioEnhanceChainManager *manager)
{
    uint32_t temp = GetData<uint32_t>();
    DeviceType deviceType = static_cast<DeviceType>(temp);

    AudioEffectPropertyArray propArray = {};
    manager->SetAudioEnhanceProperty(propArray, deviceType);
    manager->GetAudioEnhanceProperty(propArray, deviceType);
}

void AudioEnhanceChainFuzzTest()
{
#ifdef SUPPORT_OLD_ENGINE
    EffectChainManagerParam managerParam;
    managerParam.maxExtraNum = MAX_EXTRA_NUM;
    managerParam.defaultSceneName = "SCENE_DEFAULT";
    managerParam.priorSceneList = {};
    managerParam.sceneTypeToChainNameMap = {{"SCENE_RECORD_&_ENHANCE_DEFAULT_&_DEVICE_TYPE_MIC", "EFFECTCHAIN_RECORD"}};
    managerParam.effectDefaultProperty = {
        {"effect1", "property1"}, {"effect2", "property2"}, {"effect3", "property3"}
    };
    std::vector<std::shared_ptr<AudioEffectLibEntry>> enhanceLibraryList;
    enhanceLibraryList = {};
    AudioEnhanceChainManager *audioEnhanceChainMananger = AudioEnhanceChainManager::GetInstance();
    EffectChain testChain;
    testChain.name = "EFFECTCHAIN_RECORD";
    testChain.apply = {"record"};
    std::vector<EffectChain> enhanceChains;
    enhanceChains.emplace_back(testChain);
    audioEnhanceChainMananger->InitAudioEnhanceChainManager(enhanceChains, managerParam, enhanceLibraryList);
    DeviceAttrAdapter validAdapter = {DEFAULT_RATE, DEFAULT_CHANNEL, DEFAULT_FORMAT, true,
        DEFAULT_RATE, DEFAULT_CHANNEL, DEFAULT_FORMAT, true, DEFAULT_RATE, DEFAULT_CHANNEL, DEFAULT_FORMAT};
    EnhanceChainManagerCreateCb(AUDIOEFFECTSCENE_LENGTH, &validAdapter);
    EnhanceChainManagerReleaseCb(AUDIOEFFECTSCENE_LENGTH);
    EnhanceChainManagerExist(AUDIOEFFECTSCENE_LENGTH);
    pa_sample_spec micSpec;
    pa_sample_spec ecSpec;
    pa_sample_spec micRefSpec;
    pa_sample_spec_init(&micSpec);
    pa_sample_spec_init(&ecSpec);
    pa_sample_spec_init(&micRefSpec);
    EnhanceChainManagerCreateCb(AUDIOEFFECTSCENE_LENGTH, &validAdapter);
    EnhanceChainManagerGetAlgoConfig(AUDIOEFFECTSCENE_LENGTH, &micSpec, &ecSpec, &micRefSpec);
    EnhanceChainManagerIsEmptyEnhanceChain();
    EnhanceChainManagerInitEnhanceBuffer();
    const char *invalidScene = "SCENE_RECORD";
    uint64_t sceneTypeCode = GetData<uint64_t>();
    GetSceneTypeCode(invalidScene, &sceneTypeCode);
    AudioEnhanceChainManagerCreateFuzzTest(audioEnhanceChainMananger);
    AudioEnhanceChainManagerPropertyFuzzTest(audioEnhanceChainMananger);
#endif
}

void AudioEffectChainGetOutputChannelInfoFuzzTest()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }
    std::string sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey  = sceneType + "_&_" + "DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain = nullptr;
    #ifdef SENSOR_ENABLE
        std::shared_ptr<HeadTracker> headTracker = nullptr;
        headTracker = std::make_shared<HeadTracker>();
        audioEffectChain = std::make_shared<AudioEffectChain>(sceneType, headTracker);
    #else
        audioEffectChain = std::make_shared<AudioEffectChain>(sceneType);
    #endif
    audioEffectChainManager->sceneTypeToEffectChainMap_.insert({sceneTypeAndDeviceKey, audioEffectChain});
    uint32_t channels = GetData<uint32_t>();
    uint64_t channelLayout = GetData<uint64_t>();
    audioEffectChainManager->GetOutputChannelInfo(sceneType, channels, channelLayout);
    audioEffectChainManager->ResetInfo();
}

void AudioEffectChainStreamVolumeUpdateFuzzTest()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }

    const std::string sessionIDString = "12345";
    const float streamVolume = GetData<float>();
    audioEffectChainManager->StreamVolumeUpdate(sessionIDString, streamVolume);
    audioEffectChainManager->ResetInfo();
}

void AudioEffectChainUpdateEffectBtOffloadSupportedFuzzTest()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }

    audioEffectChainManager->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    audioEffectChainManager->btOffloadSupported_ = GetData<bool>();
    audioEffectChainManager->spatializationEnabled_ = GetData<bool>();
    bool isSupported = GetData<bool>();
    audioEffectChainManager->UpdateEffectBtOffloadSupported(isSupported);
    audioEffectChainManager->ResetInfo();
}

void AudioEffectChainLoadEffectPropertiesFuzzTest()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }

    audioEffectChainManager->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    audioEffectChainManager->LoadEffectProperties();
}

void AudioEffectChainGetAudioEffectPropertyFuzzTest()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }

    AudioEffectPropertyArray audioEffectPropertyArray = {};
    audioEffectChainManager->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    const char *sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        audioEffectChainManager->CreateAudioEffectChain(sceneType, GetData<bool>());

    audioEffectChainManager->sceneTypeToEffectChainMap_.insert({sceneTypeAndDeviceKey, audioEffectChain});
    audioEffectChainManager->InitAudioEffectChainDynamic(sceneType);
    audioEffectChainManager->effectPropertyMap_.insert(std::make_pair("SCENE_MUSIC", "property"));
    audioEffectChainManager->GetAudioEffectProperty(audioEffectPropertyArray);
    audioEffectChainManager->ResetInfo();
}

void AudioEffectChainWaitAndReleaseEffectChainFuzzTest()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }

    std::string sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::string defaultSceneTypeAndDeviceKey = "SCENE_DEFAULT_&_DEVICE_TYPE_SPEAKER";
    audioEffectChainManager->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        audioEffectChainManager->CreateAudioEffectChain(sceneType, GetData<bool>());
    audioEffectChainManager->sceneTypeToEffectChainMap_.insert({sceneTypeAndDeviceKey, audioEffectChain});
    audioEffectChainManager->sceneTypeToEffectChainCountMap_.insert({defaultSceneTypeAndDeviceKey, GetData<int32_t>()});
    audioEffectChainManager->sceneTypeToEffectChainCountMap_.insert({sceneTypeAndDeviceKey, GetData<int32_t>()});
    int32_t ret = GetData<int32_t>();
    audioEffectChainManager->WaitAndReleaseEffectChain(sceneType, sceneTypeAndDeviceKey,
        defaultSceneTypeAndDeviceKey, ret);
    audioEffectChainManager->ResetInfo();
}

void AudioEffectChainInitEffectBufferFuzzTest()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr || g_testDeviceTypes.size() == 0) {
        return;
    }

    audioEffectChainManager->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneType = "SCENE_MOVIE";
    std::string sceneTypeAndDeviceKey = "SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        audioEffectChainManager->CreateAudioEffectChain(sceneType, GetData<bool>());
    audioEffectChainManager->sessionIDToEffectInfoMap_.clear();
    string sessionID1 = "123456";
    audioEffectChainManager->deviceType_ =
        g_testDeviceTypes[GetData<uint32_t>() % g_testDeviceTypes.size()];
    audioEffectChainManager->sceneTypeToEffectChainMap_.insert({sceneTypeAndDeviceKey, audioEffectChain});
    audioEffectChainManager->SessionInfoMapAdd(sessionID1, DEFAULT_INFO);
    audioEffectChainManager->InitEffectBuffer(sessionID1);
    audioEffectChainManager->ResetInfo();
}

void AudioEffectChainCheckProcessClusterInstancesFuzzTest()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }

    std::string sceneType = "test";
    std::string  scene = "SCENE_DEFAULT";
    std::string effect = sceneType + "_&_" + audioEffectChainManager->GetDeviceTypeName();
    std::string defaultScene = scene + "_&_" + audioEffectChainManager->GetDeviceTypeName();
    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain = std::make_shared<AudioEffectChain>("123", headTracker);
    audioEffectChainManager->sceneTypeToEffectChainMap_.insert({effect, audioEffectChain});
    audioEffectChainManager->sceneTypeToEffectChainMap_.insert({defaultScene, audioEffectChain});

    audioEffectChainManager->maxEffectChainCount_ = GetData<int32_t>();
    audioEffectChainManager->isDefaultEffectChainExisted_ = GetData<bool>();
    audioEffectChainManager->CheckProcessClusterInstances(sceneType);
    audioEffectChainManager->ResetInfo();
}

void AudioEffectChainUpdateDeviceInfoFuzzTest()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }

    int32_t device = GetData<int32_t>();
    string sinkName = "Speaker";

    audioEffectChainManager->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    if (g_testDeviceTypes.size() != 0) {
        audioEffectChainManager->deviceType_ = g_testDeviceTypes[GetData<uint32_t>() % g_testDeviceTypes.size()];
    }
    audioEffectChainManager->isInitialized_ = static_cast<bool>(GetData<uint32_t>() % NUM_2);
    audioEffectChainManager->UpdateDeviceInfo(device, sinkName);
    audioEffectChainManager->ResetInfo();
}

void AudioEffectChainInitHdiStateFuzzTest()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }

    if (g_testDeviceTypes.size() != 0) {
        audioEffectChainManager->deviceType_ = g_testDeviceTypes[GetData<uint32_t>() % g_testDeviceTypes.size()];
    }
    audioEffectChainManager->GetOffloadEnabled();
    // Never null the shared singleton's audioEffectHdiParam_ (created once in ctor):
    // with a real param InitHdiState covers the HDI-unavailable backup branch too.
    audioEffectChainManager->InitHdiState();
    audioEffectChainManager->ResetInfo();
}

void AudioEffectChainEffectDspVolumeUpdateFuzzTest()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }

    audioEffectChainManager->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    const char *sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        audioEffectChainManager->CreateAudioEffectChain(sceneType, GetData<bool>());
    audioEffectChainManager->sceneTypeToEffectChainMap_.insert({sceneTypeAndDeviceKey, audioEffectChain});
    audioEffectChainManager->InitAudioEffectChainDynamic(sceneType);
    const std::string sessionID = "12345";
    audioEffectChainManager->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = std::make_shared<AudioEffectVolume>();
    audioEffectChainManager->EffectDspVolumeUpdate(audioEffectVolume);
    audioEffectChainManager->ResetInfo();
}

void AudioEffectChainEffectApVolumeUpdateFuzzTest()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }

    audioEffectChainManager->ResetInfo();
    SessionEffectInfo sessionEffectInfo;
    audioEffectChainManager->sessionIDSet_.insert("test");
    audioEffectChainManager->sessionIDSet_.insert("test1");
    audioEffectChainManager->sessionIDToEffectInfoMap_.insert({"test", sessionEffectInfo});
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = std::make_shared<AudioEffectVolume>();
    audioEffectChainManager->EffectApVolumeUpdate(audioEffectVolume);
    audioEffectChainManager->ResetInfo();
}

void AudioEffectChainSendEffectApVolumeFuzzTest()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();

    std::string scene = "test";
    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain = std::make_shared<AudioEffectChain>(scene, headTracker);
    if (audioEffectChainManager == nullptr || audioEffectChain == nullptr) {
        return;
    }
    audioEffectChain->SetCurrVolume(GetData<float>());
    audioEffectChain->SetFinalVolume(GetData<float>());

    audioEffectChainManager->sceneTypeToEffectChainMap_.insert({"test", nullptr});
    audioEffectChainManager->sceneTypeToEffectChainMap_.insert({"test1", audioEffectChain});
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = std::make_shared<AudioEffectVolume>();
    audioEffectChainManager->SendEffectApVolume(audioEffectVolume);
    audioEffectChainManager->ResetInfo();
}

void AudioEffectChainEffectRotationUpdateFuzzTest()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }
    uint32_t rotationState = GetData<uint32_t>();
    std::set<std::string> sceneType = {"123"};
    audioEffectChainManager->sceneTypeToSessionIDMap_.insert({"test", sceneType});

    audioEffectChainManager->EffectRotationUpdate(rotationState);
    audioEffectChainManager->ResetInfo();
}

void AudioEffectChainUpdateSensorStateFuzzTest()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }
    audioEffectChainManager->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    audioEffectChainManager->headTrackingEnabled_ = GetData<bool>();
    audioEffectChainManager->btOffloadEnabled_ = GetData<bool>();
    audioEffectChainManager->btOffloadEnabled_ = GetData<bool>();
    bool useDspEngine = audioEffectChainManager->btOffloadEnabled_;
    std::lock_guard<std::mutex> sensorLock(audioEffectChainManager->sensorMutex_);
    auto sensorAction = audioEffectChainManager->UpdateSensorStateLocked();
    audioEffectChainManager->ApplySensorActionLocked(sensorAction, useDspEngine);
    audioEffectChainManager->ResetInfo();
}

void AudioEffectChainSetSpatializationSceneTypeFuzzTest()
{
    static const vector<AudioSpatializationSceneType> testSpatializationSceneTypes = {
        SPATIALIZATION_SCENE_TYPE_DEFAULT,
        SPATIALIZATION_SCENE_TYPE_MUSIC,
        SPATIALIZATION_SCENE_TYPE_MOVIE,
        SPATIALIZATION_SCENE_TYPE_AUDIOBOOK,
        SPATIALIZATION_SCENE_TYPE_MAX,
    };
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }
    AudioSpatializationSceneType spatializationSceneType = SPATIALIZATION_SCENE_TYPE_DEFAULT;
    if (testSpatializationSceneTypes.size() != 0) {
        spatializationSceneType = testSpatializationSceneTypes[
            GetData<uint32_t>() % testSpatializationSceneTypes.size()];
    }

    audioEffectChainManager->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    audioEffectChainManager->spatializationEnabled_ = GetData<bool>();
    audioEffectChainManager->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioEffectChainManager->SetSpatializationSceneType(spatializationSceneType);
    audioEffectChainManager->ResetInfo();
}

void AudioEffectChainSendAudioParamToARMFuzzTest()
{
    static const vector<HdiSetParamCommandCode> testHdiSetParamCommandCode = {
        HDI_INIT,
        HDI_BYPASS,
        HDI_HEAD_MODE,
        HDI_ROOM_MODE,
        HDI_BLUETOOTH_MODE,
        HDI_DESTROY,
        HDI_UPDATE_SPATIAL_DEVICE_TYPE,
        HDI_VOLUME,
        HDI_ROTATION,
        HDI_EXTRA_SCENE_TYPE,
        HDI_SPATIALIZATION_SCENE_TYPE,
        HDI_STREAM_USAGE,
        HDI_FOLD_STATE,
        HDI_LID_STATE,
        HDI_QUERY_CHANNELLAYOUT,
    };
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }
    HdiSetParamCommandCode code;
    if (testHdiSetParamCommandCode.size() != 0) {
        code = testHdiSetParamCommandCode[GetData<uint32_t>() % testHdiSetParamCommandCode.size()];
    } else {
        code = HDI_INIT;
    }
    std::string value = "test";
    std::string scene = "123";
    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain = std::make_shared<AudioEffectChain>(scene, headTracker);

    audioEffectChainManager->sceneTypeToEffectChainMap_.insert({scene, audioEffectChain});
    audioEffectChainManager->SendAudioParamToARM(code, value);
    audioEffectChainManager->ResetInfo();
}

void AudioEffectChainUpdateParamExtraFuzzTest()
{
    static const vector<std::string> testSubKeys = {
        "update_audio_effect_type",
        "fold_state",
        "lid_state",
    };
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr || testSubKeys.size() == 0) {
        return;
    }

    bool mainKeyType = GetData<bool>();
    std::string mainkey = "audio_effect";
    std::string subkey = testSubKeys[GetData<uint32_t>() % testSubKeys.size()];
    if (mainKeyType) {
        mainkey = "device_status";
    }
    std::string value = "test";
    audioEffectChainManager->UpdateParamExtra(mainkey, subkey, value);
    audioEffectChainManager->ResetInfo();
}

void AudioEffectChainSetSpatializationSceneTypeToChainsFuzzTest()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }

    audioEffectChainManager->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    const char *sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
    audioEffectChainManager->CreateAudioEffectChain(sceneType, GetData<bool>());

    audioEffectChainManager->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    audioEffectChainManager->InitAudioEffectChainDynamic(sceneType);
    audioEffectChainManager->SetSpatializationSceneTypeToChains();
    audioEffectChainManager->ResetInfo();
}

void AudioEffectChainUpdateDefaultAudioEffectFuzzTest()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }

    audioEffectChainManager->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::set<std::string> sceneType = {"123"};
    audioEffectChainManager->sceneTypeToSessionIDMap_.insert({"test", sceneType});
    audioEffectChainManager->UpdateDefaultAudioEffect();
    audioEffectChainManager->ResetInfo();
}

void AudioEffectChainUpdateStreamUsageFuzzTest()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }

    audioEffectChainManager->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    const std::string sessionID = "12345";
    audioEffectChainManager->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    const char *sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    audioEffectChainManager->sceneTypeToSpecialEffectSet_.insert(sceneType);
    audioEffectChainManager->isDefaultEffectChainExisted_ = GetData<bool>();
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        audioEffectChainManager->CreateAudioEffectChain(sceneType, GetData<bool>());
    audioEffectChainManager->sceneTypeToEffectChainMap_.insert({sceneTypeAndDeviceKey, audioEffectChain});
    audioEffectChainManager->InitAudioEffectChainDynamic(sceneType);
    audioEffectChainManager->UpdateStreamUsage();
    audioEffectChainManager->ResetInfo();
}

void AudioEffectChainCheckSceneTypeMatchFuzzTest()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }

    const std::string sceneType = "SCENE_MUSIC";
    const std::string sinkSceneType = "SCENE_MUSIC";

    audioEffectChainManager->deviceType_ = DEVICE_TYPE_SPEAKER;
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        audioEffectChainManager->CreateAudioEffectChain(sceneType, GetData<bool>());
    audioEffectChainManager->sceneTypeToEffectChainMap_.insert({sceneTypeAndDeviceKey, audioEffectChain});
    audioEffectChainManager->sceneTypeToSpecialEffectSet_.insert(sceneType);
    audioEffectChainManager->CheckSceneTypeMatch(sinkSceneType, sceneType);
    audioEffectChainManager->ResetInfo();
}

void AudioEffectChainCheckAndReleaseCommonEffectChainFuzzTest()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }

    std::string sceneType = "test";
    std::string  scene = "SCENE_DEFAULT";
    std::string defaultSceneTypeAndDeviceKey = "SCENE_DEFAULT_&_DEVICE_TYPE_SPEAKER";
    audioEffectChainManager->isDefaultEffectChainExisted_ = GetData<bool>();

    std::string deviceTypeName = audioEffectChainManager->GetDeviceTypeName();
    std::string effectChain0 = scene + "_&_" + deviceTypeName;
    std::string effectChain1 = sceneType + "_&_" + deviceTypeName;
    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain = std::make_shared<AudioEffectChain>(scene, headTracker);
    audioEffectChainManager->sceneTypeToEffectChainMap_.insert({effectChain0, audioEffectChain});
    audioEffectChainManager->sceneTypeToEffectChainMap_.insert({effectChain1, audioEffectChain});

    audioEffectChainManager->sceneTypeToEffectChainCountMap_[defaultSceneTypeAndDeviceKey] = GetData<int32_t>();
    audioEffectChainManager->CheckAndReleaseCommonEffectChain(sceneType);
    audioEffectChainManager->ResetInfo();
}

void AudioEffectChainNotifyAndCreateAudioEffectChainFuzzTest()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }

    std::string sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    audioEffectChainManager->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    int32_t count = GetData<int32_t>();
    audioEffectChainManager->sceneTypeToEffectChainCountMap_.insert({sceneTypeAndDeviceKey, count});
    audioEffectChainManager->NotifyAndCreateAudioEffectChain(sceneType);
    audioEffectChainManager->ResetInfo();
}

void AudioEffectChainCreateAudioEffectChainDynamicInnerFuzzTest()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }

    audioEffectChainManager->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneType = "SCENE_MOVIE";
    std::string sceneTypeAndDeviceKey = "SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        audioEffectChainManager->CreateAudioEffectChain(sceneType, GetData<bool>());
    audioEffectChainManager->sessionIDToEffectInfoMap_.clear();
    string sessionID1 = "123456";
    audioEffectChainManager->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    audioEffectChainManager->sceneTypeToEffectChainMap_.insert({sceneTypeAndDeviceKey, audioEffectChain});
    audioEffectChainManager->SessionInfoMapAdd(sessionID1, DEFAULT_INFO);
    audioEffectChainManager->CreateAudioEffectChainDynamicInner(sceneType);
    audioEffectChainManager->ResetInfo();
}

// Private methods of AudioEnhanceChainManagerImpl are reached via static_cast down
// from the singleton pointer (access control is disabled by -fno-access-control in
// BUILD.gn); constructing a private stack instance would desync from the real
// singleton state (ChainPool / ThreadHandler are process-global).
void AudioEnhanceChainSetRelateWithDevicePropForEnhanceFuzzTest()
{
    auto *audioEnhanceChainManager = static_cast<AudioEnhanceChainManagerImpl *>(
        AudioEnhanceChainManager::GetInstance());
    if (audioEnhanceChainManager == nullptr) {
        return;
    }
    audioEnhanceChainManager->InitAudioEnhanceChainManager(ENHANCE_EFFECT_CHAINS,
        ENHANCE_CHAIN_MANAGER_PARAM, ENHANCE_LIBRARY_LIST);
    audioEnhanceChainManager->SetRelateWithDevicePropForEnhance();
    audioEnhanceChainManager->ResetInfo();
}

void AudioEnhanceChainUpdateEnhancePropertyMapFromDbFuzzTest()
{
    if (g_testDeviceTypes.size() == 0) {
        return;
    }
    DeviceType deviceType = g_testDeviceTypes[GetData<uint32_t>() % g_testDeviceTypes.size()];
    auto *audioEnhanceChainManager = static_cast<AudioEnhanceChainManagerImpl *>(
        AudioEnhanceChainManager::GetInstance());
    if (audioEnhanceChainManager == nullptr) {
        return;
    }
    audioEnhanceChainManager->InitAudioEnhanceChainManager(ENHANCE_EFFECT_CHAINS,
        ENHANCE_CHAIN_MANAGER_PARAM, ENHANCE_LIBRARY_LIST);
    audioEnhanceChainManager->UpdateEnhancePropertyMapFromDb(deviceType);
    audioEnhanceChainManager->ResetInfo();
}

void AudioEnhanceChainGetThreadHandlerBySceneFuzzTest()
{
    static const vector<AudioEnhanceScene> testAudioEnhanceScenes = {
        SCENE_VOIP_UP,
        SCENE_RECORD,
        SCENE_PRE_ENHANCE,
        SCENE_ASR,
        SCENE_VOICE_MESSAGE,
        SCENE_NONE,
    };
    if (testAudioEnhanceScenes.size() == 0) {
        return;
    }
    AudioEnhanceScene scene = testAudioEnhanceScenes[GetData<uint32_t>() % testAudioEnhanceScenes.size()];
    auto *audioEnhanceChainManager = static_cast<AudioEnhanceChainManagerImpl *>(
        AudioEnhanceChainManager::GetInstance());
    if (audioEnhanceChainManager == nullptr) {
        return;
    }
    audioEnhanceChainManager->InitAudioEnhanceChainManager(ENHANCE_EFFECT_CHAINS,
        ENHANCE_CHAIN_MANAGER_PARAM, ENHANCE_LIBRARY_LIST);
    // First call takes the create path (real OS_ProCap thread), second takes the
    // cached-handler path (++useCount on the same threadId).
    audioEnhanceChainManager->GetThreadHandlerByScene(scene);
    audioEnhanceChainManager->GetThreadHandlerByScene(scene);
    audioEnhanceChainManager->ResetInfo();
}

void AudioEnhanceChainCreateAudioEnhanceChainDynamicFuzzTest()
{
    AudioEnhanceChainManager *audioEnhanceChainManager = AudioEnhanceChainManager::GetInstance();
    if (audioEnhanceChainManager == nullptr) {
        return;
    }
    audioEnhanceChainManager->InitAudioEnhanceChainManager(ENHANCE_EFFECT_CHAINS,
        ENHANCE_CHAIN_MANAGER_PARAM, ENHANCE_LIBRARY_LIST);
    AudioEnhanceDeviceAttr deviceAttr;
    uint64_t sceneKeyCode = GetData<uint64_t>();
    audioEnhanceChainManager->CreateAudioEnhanceChainDynamic(sceneKeyCode, deviceAttr);
    audioEnhanceChainManager->ResetInfo();
}

void AudioEnhanceChainGetEnhanceNamesBySceneCodeFuzzTest()
{
    auto *audioEnhanceChainManager = static_cast<AudioEnhanceChainManagerImpl *>(
        AudioEnhanceChainManager::GetInstance());
    if (audioEnhanceChainManager == nullptr) {
        return;
    }
    audioEnhanceChainManager->InitAudioEnhanceChainManager(ENHANCE_EFFECT_CHAINS,
        ENHANCE_CHAIN_MANAGER_PARAM, ENHANCE_LIBRARY_LIST);
    uint64_t sceneKeyCode = GetData<uint64_t>();
    bool defaultFlag = GetData<bool>();
    audioEnhanceChainManager->GetEnhanceNamesBySceneCode(sceneKeyCode, defaultFlag);
    audioEnhanceChainManager->ResetInfo();
}

void AudioEnhanceChainCreateEnhanceChainInnerFuzzTest()
{
    auto *audioEnhanceChainManager = static_cast<AudioEnhanceChainManagerImpl *>(
        AudioEnhanceChainManager::GetInstance());
    if (audioEnhanceChainManager == nullptr) {
        return;
    }
    audioEnhanceChainManager->InitAudioEnhanceChainManager(ENHANCE_EFFECT_CHAINS,
        ENHANCE_CHAIN_MANAGER_PARAM, ENHANCE_LIBRARY_LIST);
    AudioEnhanceDeviceAttr deviceAttr;
    deviceAttr.ecChannels = GetData<uint32_t>();
    deviceAttr.micRate = GetData<uint64_t>();
    deviceAttr.needEc = GetData<bool>();
    deviceAttr.needMicRef = GetData<bool>();
    uint64_t sceneKeyCode = GetData<uint64_t>();
    audioEnhanceChainManager->CreateEnhanceChainInner(sceneKeyCode, deviceAttr);
    audioEnhanceChainManager->ResetInfo();
}

void AudioEnhanceChainAddAudioEnhanceChainHandlesFuzzTest()
{
    auto *audioEnhanceChainManager = static_cast<AudioEnhanceChainManagerImpl *>(
        AudioEnhanceChainManager::GetInstance());
    if (audioEnhanceChainManager == nullptr) {
        return;
    }
    audioEnhanceChainManager->InitAudioEnhanceChainManager(ENHANCE_EFFECT_CHAINS,
        ENHANCE_CHAIN_MANAGER_PARAM, ENHANCE_LIBRARY_LIST);
    std::string scene = "scene";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    uint64_t chainId = GetData<uint64_t>();
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain =
        std::make_shared<AudioEnhanceChain>(chainId, scene, ScenePriority::PRIOR_SCENE, algoParam, deviceAttr);
    if (audioEnhanceChain == nullptr) {
        return;
    }
    // Names must exist in enhanceConfigInfoMap_ (filled by Init) so the lib-resolve
    // loop body actually runs; the null libHandle keeps the failure branch.
    std::vector<std::string> enhanceNames;
    enhanceNames.push_back("enhance_record");
    enhanceNames.push_back("enhance_voip");
    audioEnhanceChainManager->AddAudioEnhanceChainHandles(audioEnhanceChain, enhanceNames);
    audioEnhanceChainManager->ResetInfo();
}

void AudioEnhanceChainSetAudioEnhancePropertyToChainsFuzzTest()
{
    auto *audioEnhanceChainManager = static_cast<AudioEnhanceChainManagerImpl *>(
        AudioEnhanceChainManager::GetInstance());
    if (audioEnhanceChainManager == nullptr) {
        return;
    }
    audioEnhanceChainManager->InitAudioEnhanceChainManager(ENHANCE_EFFECT_CHAINS,
        ENHANCE_CHAIN_MANAGER_PARAM, ENHANCE_LIBRARY_LIST);
    AudioEffectProperty property;
    audioEnhanceChainManager->SetAudioEnhancePropertyToChains(property);
    audioEnhanceChainManager->ResetInfo();
}

void AudioEnhanceChainApplyEnhanceChainByIdFuzzTest()
{
    AudioEnhanceChainManager *audioEnhanceChainManager = AudioEnhanceChainManager::GetInstance();
    if (audioEnhanceChainManager == nullptr) {
        return;
    }
    audioEnhanceChainManager->InitAudioEnhanceChainManager(ENHANCE_EFFECT_CHAINS,
        ENHANCE_CHAIN_MANAGER_PARAM, ENHANCE_LIBRARY_LIST);
    uint64_t sceneKeyCode = GetData<uint64_t>();
    EnhanceTransBuffer transBuf;
    audioEnhanceChainManager->ApplyEnhanceChainById(sceneKeyCode, transBuf);
    audioEnhanceChainManager->ResetInfo();
}

void SetAbsVolumeStateToEffectFuzzTest()
{
    std::string scene = "SCENE_MUSIC";
    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain = std::make_shared<AudioEffectChain>(scene, headTracker);
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChain == nullptr || audioEffectChainManager == nullptr) {
        return;
    }
    audioEffectChainManager->sceneTypeToEffectChainMap_.insert({scene, audioEffectChain});
    audioEffectChainManager->sceneTypeToEffectChainMap_.insert({"1", nullptr});
    bool absVolumeState = GetData<bool>();
    audioEffectChainManager->SetAbsVolumeStateToEffect(absVolumeState);
    audioEffectChainManager->EffectDspAbsVolumeStateUpdate(absVolumeState);
    audioEffectChainManager->EffectApAbsVolumeStateUpdate(absVolumeState);
    audioEffectChainManager->ResetInfo();
}

void ReleaseAudioEffectChainDynamicInnerFuzzTest()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }
    audioEffectChainManager->ResetInfo();
    std::string sceneType = "test";
    std::string deviceKey = sceneType + "_&_" + audioEffectChainManager->GetDeviceTypeName();
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        audioEffectChainManager->CreateAudioEffectChain(sceneType, GetData<bool>());

    audioEffectChainManager->sceneTypeToEffectChainMap_[deviceKey] = audioEffectChain;
    audioEffectChainManager->sceneTypeToEffectChainCountMap_[deviceKey] = GetData<int32_t>();

    audioEffectChainManager->isInitialized_ = GetData<bool>();
    audioEffectChainManager->ReleaseAudioEffectChainDynamicInner(sceneType);
}

void QueryEffectChannelInfoInnerFuzzTest()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }
    audioEffectChainManager->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneType = "SCENE_MOVIE";
    uint32_t channels = GetData<uint32_t>();
    uint64_t channelLayout = GetData<uint64_t>();
    audioEffectChainManager->QueryEffectChannelInfoInner(sceneType, channels, channelLayout);
    audioEffectChainManager->ResetInfo();
}

void EffectChainManagerExistAudioEffectChainInnerFuzzTest1()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }
    audioEffectChainManager->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneType = "SCENE_MOVIE";
    std::string effectMode = "EFFECT_MODE_NORMAL";
    audioEffectChainManager->ExistAudioEffectChainInner(sceneType, effectMode);
    audioEffectChainManager->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    audioEffectChainManager->ExistAudioEffectChainInner(sceneType, effectMode);
    audioEffectChainManager->ResetInfo();
}

void EffectChainManagerExistAudioEffectChainInnerFuzzTest2()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }
    std::string sceneType = "test";
    std::string effectMode = "123";

    audioEffectChainManager->ResetInfo();
    audioEffectChainManager->isInitialized_ = true;
    audioEffectChainManager->ExistAudioEffectChainInner(sceneType, effectMode);

    std::string sceneTypeAndMode = sceneType + "_&_" + effectMode + "_&_" +
        audioEffectChainManager->GetDeviceTypeName();
    audioEffectChainManager->sceneTypeAndModeToEffectChainNameMap_[sceneTypeAndMode] = "123456";

    std::shared_ptr<AudioEffectChain> audioEffectChain =
        audioEffectChainManager->CreateAudioEffectChain(sceneType, true);
    CHECK_AND_RETURN(audioEffectChain != nullptr);
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + audioEffectChainManager->GetDeviceTypeName();
    audioEffectChainManager->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    audioEffectChainManager->ExistAudioEffectChainInner(sceneType, effectMode);
    audioEffectChainManager->ResetInfo();
}

void EffectChainManagerExistAudioEffectChainInnerFuzzTest3()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }
    std::string sceneType = "test";
    std::string effectMode = "123";

    audioEffectChainManager->ResetInfo();
    audioEffectChainManager->isInitialized_ = true;
    std::string sceneTypeAndMode = sceneType + "_&_" + effectMode + "_&_" +
        audioEffectChainManager->GetDeviceTypeName();
    audioEffectChainManager->sceneTypeAndModeToEffectChainNameMap_[sceneTypeAndMode] = "123456";
    audioEffectChainManager->ExistAudioEffectChainInner(sceneType, effectMode);

    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + audioEffectChainManager->GetDeviceTypeName();
    audioEffectChainManager->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = nullptr;
    audioEffectChainManager->ExistAudioEffectChainInner(sceneType, effectMode);
    audioEffectChainManager->ResetInfo();
}

void EnhanceChainManagerUpdatePropertyAndSendToAlgoFuzzTest()
{
    auto *audioEnhanceChainManager = static_cast<AudioEnhanceChainManagerImpl *>(
        AudioEnhanceChainManager::GetInstance());
    if (audioEnhanceChainManager == nullptr) {
        return;
    }
    audioEnhanceChainManager->InitAudioEnhanceChainManager(ENHANCE_EFFECT_CHAINS,
        ENHANCE_CHAIN_MANAGER_PARAM, ENHANCE_LIBRARY_LIST);
    DeviceType deviceType = GetData<DeviceType>();
    audioEnhanceChainManager->UpdatePropertyAndSendToAlgo(deviceType);
    audioEnhanceChainManager->ResetInfo();
}

void EffectChainManagerSetEffectSystemVolumeFuzzTest()
{
    int32_t systemVolumeType = GetData<int32_t>();
    float systemVolume = GetData<float>();
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }
    audioEffectChainManager->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    audioEffectChainManager->SetEffectSystemVolume(systemVolumeType, systemVolume);
    audioEffectChainManager->ResetInfo();
}

void IsChannelLayoutSupportedForDspEffectFuzzTest()
{
    AudioChannelLayout channelLayout = CH_LAYOUT_STEREO;
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }
    audioEffectChainManager->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    audioEffectChainManager->IsChannelLayoutSupportedForDspEffect(channelLayout);
    audioEffectChainManager->ResetInfo();
}

void UpdateEarphoneProductFuzzTest()
{
    int32_t earphoneProduct = 0;
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }
    audioEffectChainManager->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    audioEffectChainManager->UpdateEarphoneProduct(earphoneProduct);
    audioEffectChainManager->ResetInfo();
}

void AudioEffectChainUpdatePersonalizedHRTFBinFuzzTest()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }

    audioEffectChainManager->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    int32_t fd = GetData<int32_t>();
    long length = GetData<long>();

    audioEffectChainManager->UpdatePersonalizedHRTFBin(fd, length);
    audioEffectChainManager->ResetInfo();
}

void AudioEffectChainUpdatePersonalizedHRTFBinToArmFuzzTest()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }

    audioEffectChainManager->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    int32_t fd = GetData<int32_t>();
    long length = GetData<long>();

    audioEffectChainManager->UpdatePersonalizedHRTFBinToArm(fd, length);
    audioEffectChainManager->ResetInfo();
}

void AudioEffectChainUpdatePersonalizedHRTFBinToHdiFuzzTest()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }

    audioEffectChainManager->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    int32_t fd = GetData<int32_t>();
    long length = GetData<long>();

    audioEffectChainManager->UpdatePersonalizedHRTFBinToHdi(fd, length);
    audioEffectChainManager->ResetInfo();
}

void AudioEffectChainUpdatePersonalizedSpatializationStateInnerFuzzTest()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }

    audioEffectChainManager->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    bool personalizedSpatializationEnabled = GetData<bool>();

    audioEffectChainManager->UpdatePersonalizedSpatializationStateInner(personalizedSpatializationEnabled);
    audioEffectChainManager->ResetInfo();
}

TestFuncs g_testFuncs[] = {
    AudioEffectChainFuzzTest,
    AudioEnhanceChainFuzzTest,
    AudioEffectChainGetOutputChannelInfoFuzzTest,
    AudioEffectChainStreamVolumeUpdateFuzzTest,
    AudioEffectChainUpdateEffectBtOffloadSupportedFuzzTest,
    AudioEffectChainLoadEffectPropertiesFuzzTest,
    AudioEffectChainGetAudioEffectPropertyFuzzTest,
    AudioEffectChainWaitAndReleaseEffectChainFuzzTest,
    AudioEffectChainInitEffectBufferFuzzTest,
    AudioEffectChainCheckProcessClusterInstancesFuzzTest,
    AudioEffectChainUpdateDeviceInfoFuzzTest,
    AudioEffectChainInitHdiStateFuzzTest,
    AudioEffectChainEffectDspVolumeUpdateFuzzTest,
    AudioEffectChainEffectApVolumeUpdateFuzzTest,
    AudioEffectChainSendEffectApVolumeFuzzTest,
    AudioEffectChainEffectRotationUpdateFuzzTest,
    AudioEffectChainUpdateSensorStateFuzzTest,
    AudioEffectChainSetSpatializationSceneTypeFuzzTest,
    AudioEffectChainSendAudioParamToARMFuzzTest,
    AudioEffectChainUpdateParamExtraFuzzTest,
    AudioEffectChainSetSpatializationSceneTypeToChainsFuzzTest,
    AudioEffectChainUpdateDefaultAudioEffectFuzzTest,
    AudioEffectChainUpdateStreamUsageFuzzTest,
    AudioEffectChainCheckSceneTypeMatchFuzzTest,
    AudioEffectChainCheckAndReleaseCommonEffectChainFuzzTest,
    AudioEffectChainNotifyAndCreateAudioEffectChainFuzzTest,
    AudioEffectChainCreateAudioEffectChainDynamicInnerFuzzTest,
    AudioEnhanceChainSetRelateWithDevicePropForEnhanceFuzzTest,
    AudioEnhanceChainUpdateEnhancePropertyMapFromDbFuzzTest,
    AudioEnhanceChainGetThreadHandlerBySceneFuzzTest,
    AudioEnhanceChainCreateAudioEnhanceChainDynamicFuzzTest,
    AudioEnhanceChainGetEnhanceNamesBySceneCodeFuzzTest,
    AudioEnhanceChainCreateEnhanceChainInnerFuzzTest,
    AudioEnhanceChainAddAudioEnhanceChainHandlesFuzzTest,
    AudioEnhanceChainSetAudioEnhancePropertyToChainsFuzzTest,
    AudioEnhanceChainApplyEnhanceChainByIdFuzzTest,
    SetAbsVolumeStateToEffectFuzzTest,
    ReleaseAudioEffectChainDynamicInnerFuzzTest,
    QueryEffectChannelInfoInnerFuzzTest,
    EffectChainManagerExistAudioEffectChainInnerFuzzTest1,
    EffectChainManagerExistAudioEffectChainInnerFuzzTest2,
    EffectChainManagerExistAudioEffectChainInnerFuzzTest3,
    EnhanceChainManagerUpdatePropertyAndSendToAlgoFuzzTest,
    EffectChainManagerSetEffectSystemVolumeFuzzTest,
    IsChannelLayoutSupportedForDspEffectFuzzTest,
    UpdateEarphoneProductFuzzTest,
    AudioEffectChainUpdatePersonalizedHRTFBinFuzzTest,
    AudioEffectChainUpdatePersonalizedHRTFBinToArmFuzzTest,
    AudioEffectChainUpdatePersonalizedHRTFBinToHdiFuzzTest,
    AudioEffectChainUpdatePersonalizedSpatializationStateInnerFuzzTest,
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

    uint32_t len = sizeof(g_testFuncs) / sizeof(TestFuncs);
    if (len > 0) {
        g_testFuncs[g_count % len]();
        g_count++;
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }
    g_count = g_count == len ? 0 : g_count;

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
