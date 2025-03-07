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
#include "audio_effect.h"
#include "audio_effect_chain.h"
#include "audio_effect_log.h"
#include "audio_effect_chain_manager.h"
#include "audio_effect_chain_adapter.h"
#include "audio_enhance_chain_adapter.h"
#include "audio_enhance_chain_manager.h"
#include "audio_errors.h"
#include "audio_head_tracker.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

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

#define DEFAULT_NUM_CHANNEL 2
#define DEFAULT_CHANNELLAYOUT 3

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
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS, DEFAULT_MAP,
        DEFAULT_EFFECT_LIBRARY_LIST);

    const char *sceneType = SCENETYPEMUSIC;
    EffectChainManagerInitCb(sceneType);
    sceneType = nullptr;
    EffectChainManagerInitCb(sceneType);
    sceneType = "";
    EffectChainManagerInitCb(sceneType);
}

void EffectChainManagerCreateCbFuzzTest()
{
    const char *sceneType = SCENETYPEMUSIC;
    EffectChainManagerInitCb(sceneType);

    const char *sessionid = SESSIONIDDEFAULT;
    EffectChainManagerCreateCb(sceneType, sessionid);
    EffectChainManagerReleaseCb(sceneType, sessionid);
    sessionid = "";
    EffectChainManagerCreateCb(sceneType, sessionid);
    EffectChainManagerReleaseCb(sceneType, sessionid);
}

void EffectChainManagerCheckEffectOffloadFuzzTest()
{
    const char *sceneType = "";
    EffectChainManagerInitCb(sceneType);
    EffectChainManagerCheckEffectOffload();
}

void EffectChainManagerAddSessionInfoFuzzTest()
{
    EffectChainManagerInitCb(SCENETYPEMUSIC);

    SessionInfoPack pack = {2, "3", EFFECTDEFAULT, "true"};
    EffectChainManagerAddSessionInfo(SCENETYPEMUSIC, SESSIONIDDEFAULT, pack);
}

void EffectChainManagerDeleteSessionInfoFuzzTest()
{
    EffectChainManagerInitCb(SCENETYPEMUSIC);

    SessionInfoPack pack = {2, "3", SESSIONIDDEFAULT, "true"};
    EffectChainManagerAddSessionInfo(SCENETYPEMUSIC, SESSIONIDDEFAULT, pack);
    EffectChainManagerDeleteSessionInfo(SCENETYPEMUSIC, SESSIONIDDEFAULT);
}

void EffectChainManagerReturnEffectChannelInfoFuzzTest()
{
    EffectChainManagerInitCb(SCENETYPEMUSIC);

    SessionInfoPack pack = {2, "3", SESSIONIDDEFAULT, "true"};
    EffectChainManagerAddSessionInfo(SCENETYPEMUSIC, SESSIONIDDEFAULT, pack);

    uint32_t processChannels = GetData<uint32_t>();
    uint64_t processChannelLayout = GetData<uint64_t>();
    EffectChainManagerReturnEffectChannelInfo(SCENETYPEMUSIC, &processChannels, &processChannelLayout);
}

void EffectChainManagerReturnMultiChannelInfoFuzzTest()
{
    EffectChainManagerInitCb(SCENETYPEMUSIC);

    uint32_t processChannels = GetData<uint32_t>();
    uint64_t processChannelLayout = GetData<uint64_t>();
    EffectChainManagerReturnMultiChannelInfo(&processChannels, &processChannelLayout);
}

void EffectChainManagerSceneCheckFuzzTest()
{
    EffectChainManagerInitCb("SCENE_MUSIC");
    EffectChainManagerSceneCheck("SCENE_MUSIC", "SCENE_MUSIC");
}

void EffectChainManagerProcessFuzzTest()
{
    EffectChainManagerInitCb(SCENETYPEMUSIC);
}

void EffectChainManagerMultichannelUpdateFuzzTest()
{
    EffectChainManagerInitCb(SCENETYPEMUSIC);
    EffectChainManagerMultichannelUpdate(nullptr);
    EffectChainManagerMultichannelUpdate(SCENETYPEMUSIC);
}

void EffectChainManagerExistFuzzTest()
{
    EffectChainManagerInitCb(SCENETYPEMUSIC);
}

void EffectChainManagerVolumeUpdateFuzzTest()
{
    EffectChainManagerInitCb(SCENETYPEMUSIC);
    EffectChainManagerVolumeUpdate(SESSIONIDDEFAULT);
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
    audioEnhanceChainMananger->InitEnhanceBuffer();
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
}

void AudioEnhanceChainFuzzTest()
{
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
}

typedef void (*TestFuncs[14])();

TestFuncs g_testFuncs = {
    EffectChainManagerInitCbFuzzTest,
    EffectChainManagerCreateCbFuzzTest,
    EffectChainManagerCheckEffectOffloadFuzzTest,
    EffectChainManagerAddSessionInfoFuzzTest,
    EffectChainManagerDeleteSessionInfoFuzzTest,
    EffectChainManagerReturnEffectChannelInfoFuzzTest,
    EffectChainManagerReturnMultiChannelInfoFuzzTest,
    EffectChainManagerSceneCheckFuzzTest,
    EffectChainManagerProcessFuzzTest,
    EffectChainManagerMultichannelUpdateFuzzTest,
    EffectChainManagerExistFuzzTest,
    EffectChainManagerVolumeUpdateFuzzTest,
    AudioEffectChainFuzzTest,
    AudioEnhanceChainFuzzTest,
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
