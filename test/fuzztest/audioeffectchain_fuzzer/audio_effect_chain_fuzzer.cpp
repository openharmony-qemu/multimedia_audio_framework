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
#include "audio_effect_chain_adapter.h"
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

void EffectChainManagerInitCbFuzzTest()
{
#ifdef SUPPORT_OLD_ENGINE
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_MAP,
        DEFAULT_EFFECT_LIBRARY_LIST);

    const char *sceneType = SCENETYPEMUSIC;
    EffectChainManagerInitCb(sceneType);
    sceneType = nullptr;
    EffectChainManagerInitCb(sceneType);
    sceneType = "";
    EffectChainManagerInitCb(sceneType);
#endif
}

void EffectChainManagerCreateCbFuzzTest()
{
#ifdef SUPPORT_OLD_ENGINE
    const char *sceneType = SCENETYPEMUSIC;
    EffectChainManagerInitCb(sceneType);

    const char *sessionid = SESSIONIDDEFAULT;
    EffectChainManagerCreateCb(sceneType, sessionid);
    EffectChainManagerReleaseCb(sceneType, sessionid);
    sessionid = "";
    EffectChainManagerCreateCb(sceneType, sessionid);
    EffectChainManagerReleaseCb(sceneType, sessionid);
#endif
}

void EffectChainManagerCheckEffectOffloadFuzzTest()
{
#ifdef SUPPORT_OLD_ENGINE
    const char *sceneType = "";
    EffectChainManagerInitCb(sceneType);
    EffectChainManagerCheckEffectOffload();
#endif
}

void EffectChainManagerAddSessionInfoFuzzTest()
{
#ifdef SUPPORT_OLD_ENGINE
    EffectChainManagerInitCb(SCENETYPEMUSIC);

    SessionInfoPack pack = {2, "3", EFFECTDEFAULT, "true"};
    EffectChainManagerAddSessionInfo(SCENETYPEMUSIC, SESSIONIDDEFAULT, pack);
#endif
}

void EffectChainManagerDeleteSessionInfoFuzzTest()
{
#ifdef SUPPORT_OLD_ENGINE
    EffectChainManagerInitCb(SCENETYPEMUSIC);

    SessionInfoPack pack = {2, "3", SESSIONIDDEFAULT, "true"};
    EffectChainManagerAddSessionInfo(SCENETYPEMUSIC, SESSIONIDDEFAULT, pack);
    EffectChainManagerDeleteSessionInfo(SCENETYPEMUSIC, SESSIONIDDEFAULT);
#endif
}

void EffectChainManagerReturnEffectChannelInfoFuzzTest()
{
#ifdef SUPPORT_OLD_ENGINE
    EffectChainManagerInitCb(SCENETYPEMUSIC);

    SessionInfoPack pack = {2, "3", SESSIONIDDEFAULT, "true"};
    EffectChainManagerAddSessionInfo(SCENETYPEMUSIC, SESSIONIDDEFAULT, pack);

    uint32_t processChannels = GetData<uint32_t>();
    uint64_t processChannelLayout = GetData<uint64_t>();
    EffectChainManagerReturnEffectChannelInfo(SCENETYPEMUSIC, &processChannels, &processChannelLayout);
#endif
}

void EffectChainManagerSceneCheckFuzzTest()
{
#ifdef SUPPORT_OLD_ENGINE
    EffectChainManagerInitCb("SCENE_MUSIC");
    EffectChainManagerSceneCheck("SCENE_MUSIC", "SCENE_MUSIC");
#endif
}

void EffectChainManagerProcessFuzzTest()
{
#ifdef SUPPORT_OLD_ENGINE
    EffectChainManagerInitCb(SCENETYPEMUSIC);
#endif
}

void EffectChainManagerMultichannelUpdateFuzzTest()
{
#ifdef SUPPORT_OLD_ENGINE
    EffectChainManagerInitCb(SCENETYPEMUSIC);
    EffectChainManagerMultichannelUpdate(nullptr);
    EffectChainManagerMultichannelUpdate(SCENETYPEMUSIC);
#endif
}

void EffectChainManagerExistFuzzTest()
{
#ifdef SUPPORT_OLD_ENGINE
    EffectChainManagerInitCb(SCENETYPEMUSIC);
#endif
}

void EffectChainManagerVolumeUpdateFuzzTest()
{
#ifdef SUPPORT_OLD_ENGINE
    EffectChainManagerInitCb(SCENETYPEMUSIC);
    EffectChainManagerVolumeUpdate(SESSIONIDDEFAULT);
#endif
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
    uint32_t currSceneType_int = GetData<uint32_t>();
    currSceneType_int = (currSceneType_int % AUDIOEFFECTSCENE_LENGTH);
    AudioEffectScene currSceneType = GetData<AudioEffectScene>();
    audioEffectChain->SetEffectCurrSceneType(currSceneType);
    AudioEffectChainManagerFirst(audioEffectChain);
}

void AudioEnhanceChainManagerFuzzTest(AudioEnhanceChainManager *audioEnhanceChainMananger)
{
    AudioEnhancePropertyArray propertyArray;
    AudioVolumeType volumeType = GetData<AudioVolumeType>();
    audioEnhanceChainMananger->SetVolumeInfo(volumeType, SYSTEM_VOLINFO);
    bool isMute = true;
    audioEnhanceChainMananger->SetMicrophoneMuteInfo(isMute);
    isMute = false;
    audioEnhanceChainMananger->SetMicrophoneMuteInfo(isMute);
    uint32_t renderId = GetData<uint32_t>();
    DeviceType newDeviceType = GetData<DeviceType>();
    audioEnhanceChainMananger->SetOutputDevice(renderId, newDeviceType);
    audioEnhanceChainMananger->GetAudioEnhanceProperty(propertyArray);
    audioEnhanceChainMananger->ResetInfo();
    audioEnhanceChainMananger->SetInputDevice(DEFAULT_CHANNEL, newDeviceType);
    uint32_t sessionId = GetData<uint32_t>();
    audioEnhanceChainMananger->SetStreamVolumeInfo(sessionId, 0);
    std::string mainKey = "mainKey";
    std::string subKey = "subKey";
    std::string extraSceneType = "extraSceneType";
    audioEnhanceChainMananger->UpdateExtraSceneType(mainKey, subKey, extraSceneType);
    audioEnhanceChainMananger->SendInitCommand();
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

    AudioEffectPropertyArrayV3 propV3Array = {};
    manager->SetAudioEnhanceProperty(propV3Array, deviceType);
    manager->GetAudioEnhanceProperty(propV3Array, deviceType);

    AudioEnhancePropertyArray propArray = {};
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
    AudioEnhanceChainManagerFuzzTest(audioEnhanceChainMananger);
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
    uint32_t channels;
    uint64_t channelLayout;
    audioEffectChainManager->GetOutputChannelInfo(sceneType, channels, channelLayout);
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

void AudioEffectChainSetAudioEffectPropertyFuzzTest()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }
    AudioEffectProperty  audioEffectProperty1 = {
        .effectClass = "testClass1",
        .effectProp = "testProp1",
    };
    AudioEffectProperty  audioEffectProperty2 = {
        .effectClass = "testClass2",
        .effectProp = "testProp2",
    };

    AudioEffectPropertyArray audioEffectPropertyArray = {};
    audioEffectPropertyArray.property.push_back(audioEffectProperty1);
    audioEffectPropertyArray.property.push_back(audioEffectProperty2);
    audioEffectChainManager->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    const char *sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        audioEffectChainManager->CreateAudioEffectChain(sceneType, GetData<bool>());
    audioEffectChainManager->sceneTypeToEffectChainMap_.insert({sceneTypeAndDeviceKey, audioEffectChain});
    audioEffectChainManager->InitAudioEffectChainDynamic(sceneType);
    audioEffectChainManager->SetAudioEffectProperty(audioEffectPropertyArray);
}

void AudioEffectChainGetAudioEffectPropertyFuzzTest()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }

    AudioEffectPropertyArrayV3 audioEffectPropertyArrayV3 = {};
    audioEffectChainManager->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    const char *sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        audioEffectChainManager->CreateAudioEffectChain(sceneType, GetData<bool>());

    audioEffectChainManager->sceneTypeToEffectChainMap_.insert({sceneTypeAndDeviceKey, audioEffectChain});
    audioEffectChainManager->InitAudioEffectChainDynamic(sceneType);
    audioEffectChainManager->effectPropertyMap_.insert(std::make_pair("SCENE_MUSIC", "property"));
    audioEffectChainManager->GetAudioEffectProperty(audioEffectPropertyArrayV3);
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
}

void AudioEffectChainInitHdiStateFuzzTest()
{
    AudioEffectChainManager audioEffectChainManager;

    if (g_testDeviceTypes.size() != 0) {
        audioEffectChainManager.deviceType_ = g_testDeviceTypes[GetData<uint32_t>() % g_testDeviceTypes.size()];
    }
    audioEffectChainManager.GetOffloadEnabled();
    audioEffectChainManager.audioEffectHdiParam_ = nullptr;
    audioEffectChainManager.InitHdiState();
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
    audioEffectChainManager->UpdateSensorState();
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
}

void AudioEffectChainCheckAndReleaseCommonEffectChainFuzzTest()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    if (audioEffectChainManager == nullptr) {
        return;
    }

    std::string sceneType = "test";
    std::string  scene = "SCENE_DEFAULT";
    audioEffectChainManager->isDefaultEffectChainExisted_ = GetData<bool>();

    std::string deviceTypeName = audioEffectChainManager->GetDeviceTypeName();
    std::string effectChain0 = scene + "_&_" + deviceTypeName;
    std::string effectChain1 = sceneType + "_&_" + deviceTypeName;
    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain = std::make_shared<AudioEffectChain>(scene, headTracker);
    audioEffectChainManager->sceneTypeToEffectChainMap_.insert({effectChain0, audioEffectChain});
    audioEffectChainManager->sceneTypeToEffectChainMap_.insert({effectChain1, audioEffectChain});

    audioEffectChainManager->defaultEffectChainCount_ = GetData<int32_t>();
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
}

void AudioEnhanceChainSetRelateWithDevicePropForEnhanceFuzzTest()
{
    AudioEnhanceChainManagerImpl audioEnhanceChainManagerImpl;
    std::string sceneAndMode = "SCENE_VOIP_UP_&_ENHANCE_DEFAULT";
    EnhanceChainConfigInfo enhanceChainConfigInfo;
    enhanceChainConfigInfo.chainName = "ENHANCE_DEFAULT";
    enhanceChainConfigInfo.chainLabel = "SCENE_VOIP_UP";
    audioEnhanceChainManagerImpl.chainConfigInfoMap_.insert({sceneAndMode, enhanceChainConfigInfo});
    audioEnhanceChainManagerImpl.SetRelateWithDevicePropForEnhance();
}

void AudioEnhanceChainUpdateEnhancePropertyMapFromDbFuzzTest()
{
    if (g_testDeviceTypes.size() == 0) {
        return;
    }
    DeviceType deviceType = g_testDeviceTypes[GetData<uint32_t>() % g_testDeviceTypes.size()];
    AudioEnhanceChainManagerImpl audioEnhanceChainManagerImpl;
    audioEnhanceChainManagerImpl.enhancePropertyMap_.insert({"SCENE_VOIP_UP_&_DEVICE_TYPE_MIC", "ENHANCE_DEFAULT"});
    audioEnhanceChainManagerImpl.UpdateEnhancePropertyMapFromDb(deviceType);
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
    uint32_t sceneIndex = GetData<uint32_t>() % testAudioEnhanceScenes.size();
    AudioEnhanceScene scene = testAudioEnhanceScenes[GetData<uint32_t>() % testAudioEnhanceScenes.size()];
    AudioEnhanceChainManagerImpl audioEnhanceChainManagerImpl;
    std::shared_ptr<ThreadHandler> threadHandler = ThreadHandler::NewInstance("testThreadHandler");
    std::pair<std::shared_ptr<ThreadHandler>, uint32_t> threadHandlerPair =
        std::make_pair(threadHandler, GetData<uint32_t>());
    audioEnhanceChainManagerImpl.threadHandlerMap_.insert({sceneIndex, threadHandlerPair});
    audioEnhanceChainManagerImpl.threadHandlerMap_.insert({sceneIndex, threadHandlerPair});
    audioEnhanceChainManagerImpl.GetThreadHandlerByScene(scene);
}

void AudioEnhanceChainCreateAudioEnhanceChainDynamicFuzzTest()
{
    AudioEnhanceChainManagerImpl audioEnhanceChainManagerImpl;
    AudioEnhanceDeviceAttr deviceAttr;
    uint64_t sceneKeyCode = GetData<uint64_t>();
    audioEnhanceChainManagerImpl.CreateAudioEnhanceChainDynamic(sceneKeyCode, deviceAttr);
}

void AudioEnhanceChainGetEnhanceNamesBySceneCodeFuzzTest()
{
    AudioEnhanceChainManagerImpl audioEnhanceChainManagerImpl;
    uint64_t sceneKeyCode = GetData<uint64_t>();
    bool defaultFlag = GetData<bool>();
    audioEnhanceChainManagerImpl.GetEnhanceNamesBySceneCode(sceneKeyCode, defaultFlag);
}

void AudioEnhanceChainCreateEnhanceChainInnerFuzzTest()
{
    AudioEnhanceChainManagerImpl audioEnhanceChainManagerImpl;
    AudioEnhanceDeviceAttr deviceAttr;
    deviceAttr.ecChannels = GetData<uint32_t>();
    deviceAttr.micRate = GetData<uint64_t>();
    deviceAttr.needEc = GetData<bool>();
    deviceAttr.needMicRef = GetData<bool>();
    uint64_t sceneKeyCode = GetData<uint64_t>();
    audioEnhanceChainManagerImpl.CreateEnhanceChainInner(sceneKeyCode, deviceAttr);
}

void AudioEnhanceChainAddAudioEnhanceChainHandlesFuzzTest()
{
    AudioEnhanceChainManagerImpl audioEnhanceChainManagerImpl;
    std::string scene = "scene";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    uint64_t chainId = GetData<uint64_t>();
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain =
        std::make_shared<AudioEnhanceChain>(chainId, scene, ScenePriority::PRIOR_SCENE, algoParam, deviceAttr);
    if (audioEnhanceChain == nullptr) {
        return;
    }
    std::vector<std::string> enhanceNames;
    enhanceNames.push_back("enhance1");
    enhanceNames.push_back("enhance2");
    audioEnhanceChainManagerImpl.AddAudioEnhanceChainHandles(audioEnhanceChain, enhanceNames);
}

void AudioEnhanceChainSetAudioEnhancePropertyToChainsFuzzTest()
{
    AudioEnhanceChainManagerImpl audioEnhanceChainManagerImpl;
    AudioEffectPropertyV3 property;
    audioEnhanceChainManagerImpl.SetAudioEnhancePropertyToChains(property);
}

void AudioEnhanceChainApplyEnhanceChainByIdFuzzTest()
{
    AudioEnhanceChainManagerImpl audioEnhanceChainManagerImpl;
    uint64_t sceneKeyCode = GetData<uint64_t>();
    EnhanceTransBuffer transBuf;
    audioEnhanceChainManagerImpl.ApplyEnhanceChainById(sceneKeyCode, transBuf);
}

void AudioEnhanceChainUpdateExtraSceneTypeFuzzTest()
{
    AudioEnhanceChainManagerImpl audioEnhanceChainManagerImpl;
    std::string mainkey = "audio_effect";
    std::string subkey = "update_audio_effect_type";
    std::string extraSceneType = "SCENE_VOIP_UP";
    audioEnhanceChainManagerImpl.UpdateExtraSceneType(mainkey, subkey, extraSceneType);
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
}

void EnhanceChainManagerUpdatePropertyAndSendToAlgoFuzzTest()
{
    AudioEnhanceChainManagerImpl audioEnhanceChainManagerImpl;
    audioEnhanceChainManagerImpl.enhancePropertyMap_.insert({"SCENE_VOIP_UP_&_DEVICE_TYPE_MIC", "ENHANCE_DEFAULT"});
    DeviceType deviceType = GetData<DeviceType>();
    audioEnhanceChainManagerImpl.UpdatePropertyAndSendToAlgo(deviceType);
}

TestFuncs g_testFuncs[] = {
    EffectChainManagerInitCbFuzzTest,
    EffectChainManagerCreateCbFuzzTest,
    EffectChainManagerCheckEffectOffloadFuzzTest,
    EffectChainManagerAddSessionInfoFuzzTest,
    EffectChainManagerDeleteSessionInfoFuzzTest,
    EffectChainManagerReturnEffectChannelInfoFuzzTest,
    EffectChainManagerSceneCheckFuzzTest,
    EffectChainManagerProcessFuzzTest,
    EffectChainManagerMultichannelUpdateFuzzTest,
    EffectChainManagerExistFuzzTest,
    EffectChainManagerVolumeUpdateFuzzTest,
    AudioEffectChainFuzzTest,
    AudioEnhanceChainFuzzTest,
    AudioEffectChainGetOutputChannelInfoFuzzTest,
    AudioEffectChainStreamVolumeUpdateFuzzTest,
    AudioEffectChainUpdateEffectBtOffloadSupportedFuzzTest,
    AudioEffectChainLoadEffectPropertiesFuzzTest,
    AudioEffectChainSetAudioEffectPropertyFuzzTest,
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
    AudioEnhanceChainUpdateExtraSceneTypeFuzzTest,
    SetAbsVolumeStateToEffectFuzzTest,
    ReleaseAudioEffectChainDynamicInnerFuzzTest,
    QueryEffectChannelInfoInnerFuzzTest,
    EffectChainManagerExistAudioEffectChainInnerFuzzTest1,
    EffectChainManagerExistAudioEffectChainInnerFuzzTest2,
    EffectChainManagerExistAudioEffectChainInnerFuzzTest3,
    EnhanceChainManagerUpdatePropertyAndSendToAlgoFuzzTest,
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
