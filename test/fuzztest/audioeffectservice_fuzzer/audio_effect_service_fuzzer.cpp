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
#include "audio_effect_service.h"
#include "audio_inner_call.h"
#include <fuzzer/FuzzedDataProvider.h>
using namespace std;

namespace OHOS {
namespace AudioStandard {
using namespace std;
static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;
typedef void (*TestPtr)();

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
    DEVICE_TYPE_MAX
};

template<class T>
uint32_t GetArrLength(T &arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
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

void AudioEffectServiceFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioEffectService> audioEffectService = std::make_shared<AudioEffectService>();
    if (audioEffectService == nullptr) {
        return;
    }
    audioEffectService->EffectServiceInit();
    audioEffectService->BuildAvailableAEConfig();
}

void AudioEffectServiceGetAvailableEffectsFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioEffectService> audioEffectService = std::make_shared<AudioEffectService>();
    if (audioEffectService == nullptr) {
        return;
    }

    std::vector<Effect> availableEffects;
    audioEffectService->GetAvailableEffects(availableEffects);
}

void AudioEffectServiceGetOriginalEffectConfigFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioEffectService> audioEffectService = std::make_shared<AudioEffectService>();
    if (audioEffectService == nullptr) {
        return;
    }

    OriginalEffectConfig oriEffectConfig;
    audioEffectService->GetOriginalEffectConfig(oriEffectConfig);
}

void AudioEffectServiceUpdateAvailableEffectsFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioEffectService> audioEffectService = std::make_shared<AudioEffectService>();
    if (audioEffectService == nullptr) {
        return;
    }

    std::vector<Effect> newAvailableEffects;
    audioEffectService->UpdateAvailableEffects(newAvailableEffects);
}

void AudioEffectServiceQueryEffectManagerSceneModeFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioEffectService> audioEffectService = std::make_shared<AudioEffectService>();
    if (audioEffectService == nullptr) {
        return;
    }

    SupportedEffectConfig supportedEffectConfig;
    audioEffectService->QueryEffectManagerSceneMode(supportedEffectConfig);
}

void AudioEffectServiceGetSupportedEffectConfigFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioEffectService> audioEffectService = std::make_shared<AudioEffectService>();
    if (audioEffectService == nullptr) {
        return;
    }

    SupportedEffectConfig supportedEffectConfig;
    audioEffectService->GetSupportedEffectConfig(supportedEffectConfig);
}

void AudioEffectServiceSetMasterSinkAvailableFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioEffectService> audioEffectService = std::make_shared<AudioEffectService>();
    if (audioEffectService == nullptr) {
        return;
    }

    audioEffectService->SetMasterSinkAvailable();
    audioEffectService->SetEffectChainManagerAvailable();
    audioEffectService->CanLoadEffectSinks();
}

void AudioEffectServiceConstructEffectChainModeFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioEffectService> audioEffectService = std::make_shared<AudioEffectService>();
    if (audioEffectService == nullptr) {
        return;
    }

    Device device1;
    Device device2;
    device2.chain = "chain";
    device2.type = "DEVICE_TYPE_DEFAULT";
    StreamEffectMode mode;
    mode.mode = "mode";
    mode.devicePort.push_back(device1);
    mode.devicePort.push_back(device2);
    std::string sceneType = "sceneType";
    EffectChainManagerParam effectChainMgrParam;
    audioEffectService->ConstructEffectChainMode(mode, sceneType, effectChainMgrParam);
}

void AudioEffectServiceConstructEffectChainManagerParamFuzzTest(FuzzedDataProvider& fdp)
{
    static const vector<ScenePriority> testScenePriorities = {
        DEFAULT_SCENE,
        PRIOR_SCENE,
        NORMAL_SCENE,
    };
    std::shared_ptr<AudioEffectService> audioEffectService = std::make_shared<AudioEffectService>();
    if (audioEffectService == nullptr || testScenePriorities.empty()) {
        return;
    }

    Stream stream1;
    Stream stream2;
    stream1.scene = "test";
    stream1.priority = testScenePriorities[GetData<uint32_t>() % testScenePriorities.size()];
    stream2.scene = "test";
    stream2.priority = testScenePriorities[GetData<uint32_t>() % testScenePriorities.size()];
    EffectChainManagerParam effectChainManagerParam;
    audioEffectService->supportedEffectConfig_.postProcessNew.stream.push_back(stream1);
    audioEffectService->supportedEffectConfig_.postProcessNew.stream.push_back(stream2);
    audioEffectService->ConstructEffectChainManagerParam(effectChainManagerParam);
}

void AudioEffectServiceConstructEnhanceChainManagerParamFuzzTest(FuzzedDataProvider& fdp)
{
    static const vector<ScenePriority> testScenePriorities = {
        DEFAULT_SCENE,
        PRIOR_SCENE,
        NORMAL_SCENE,
    };
    std::shared_ptr<AudioEffectService> audioEffectService = std::make_shared<AudioEffectService>();
    if (audioEffectService == nullptr || testScenePriorities.empty()) {
        return;
    }

    Stream stream1;
    Stream stream2;
    stream1.scene = "test";
    stream1.priority = testScenePriorities[GetData<uint32_t>() % testScenePriorities.size()];
    stream2.scene = "test";
    stream2.priority = testScenePriorities[GetData<uint32_t>() % testScenePriorities.size()];
    EffectChainManagerParam effectChainManagerParam;
    audioEffectService->supportedEffectConfig_.preProcessNew.stream.push_back(stream1);
    audioEffectService->supportedEffectConfig_.preProcessNew.stream.push_back(stream2);
    audioEffectService->ConstructEnhanceChainManagerParam(effectChainManagerParam);
}

void AudioEffectServiceAddSupportedPropertyByDeviceInnerFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioEffectService> audioEffectService = std::make_shared<AudioEffectService>();
    if (audioEffectService == nullptr || g_testDeviceTypes.empty()) {
        return;
    }

    std::set<std::pair<std::string, std::string>> mergedSet;
    std::unordered_map<std::string, std::set<std::pair<std::string, std::string>>> device2PropertySet;
    std::set<std::pair<std::string, std::string>> device2Property;
    device2PropertySet.insert({"DEVICE_TYPE_DEFAULT", device2Property});
    DeviceType deviceType = g_testDeviceTypes[GetData<uint32_t>() % g_testDeviceTypes.size()];
    audioEffectService->AddSupportedPropertyByDeviceInner(deviceType, mergedSet, device2PropertySet);
}

void AudioEffectServiceAddSupportedAudioEffectPropertyByDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioEffectService> audioEffectService = std::make_shared<AudioEffectService>();
    if (audioEffectService == nullptr || g_testDeviceTypes.empty()) {
        return;
    }

    std::set<std::pair<std::string, std::string>> mergedSet;
    DeviceType deviceType = g_testDeviceTypes[GetData<uint32_t>() % g_testDeviceTypes.size()];
    audioEffectService->AddSupportedAudioEffectPropertyByDevice(deviceType, mergedSet);
}

void AudioEffectServiceAddSupportedAudioEnhancePropertyByDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioEffectService> audioEffectService = std::make_shared<AudioEffectService>();
    if (audioEffectService == nullptr || g_testDeviceTypes.empty()) {
        return;
    }

    std::set<std::pair<std::string, std::string>> mergedSet;
    DeviceType deviceType = g_testDeviceTypes[GetData<uint32_t>() % g_testDeviceTypes.size()];
    audioEffectService->AddSupportedAudioEnhancePropertyByDevice(deviceType, mergedSet);
}

void AudioEffectServiceUpdateUnavailableEffectChainsFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioEffectService> audioEffectService = std::make_shared<AudioEffectService>();
    if (audioEffectService == nullptr || g_testDeviceTypes.empty()) {
        return;
    }

    Stream newStream;
    ProcessNew processNew;
    StreamEffectMode streamEffect;
    newStream.streamEffectMode.push_back(streamEffect);
    processNew.stream.push_back(newStream);
    std::vector<std::string> availableLayout;
    audioEffectService->UpdateUnavailableEffectChains(availableLayout, processNew);
}

void AudioEffectServiceUpdateSupportedEffectPropertyFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioEffectService> audioEffectService = std::make_shared<AudioEffectService>();
    if (audioEffectService == nullptr || g_testDeviceTypes.empty()) {
        return;
    }

    Device device;
    Effect effect;
    EffectChain effectChain;
    std::unordered_map<std::string, std::set<std::pair<std::string, std::string>>> device2PropertySet;
    device.chain = "test";
    effectChain.name = "test";
    effectChain.apply.push_back("test");
    effect.effectProperty.push_back("test1");
    std::set<std::pair<std::string, std::string>> deviceSet = {{"test1", "value1"}, {"test2", "value2"}};
    device2PropertySet.insert({"test", deviceSet});
    audioEffectService->supportedEffectConfig_.effectChains.push_back(effectChain);
    audioEffectService->UpdateSupportedEffectProperty(device, device2PropertySet);
}

void AudioEffectServiceUpdateAvailableAEConfigFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioEffectService> audioEffectService = std::make_shared<AudioEffectService>();
    if (audioEffectService == nullptr) {
        return;
    }
    OriginalEffectConfig aeConfig;
    SceneMappingItem sceneMappingItem;
    PreStreamScene preStreamScene;
    preStreamScene.stream = "SCENE_VOIP_UP";
    preStreamScene.mode.push_back("ENHANCE_NONE");
    aeConfig.preProcess.defaultScenes.push_back(preStreamScene);
    aeConfig.preProcess.priorScenes.push_back(preStreamScene);
    aeConfig.preProcess.normalScenes.push_back(preStreamScene);
    aeConfig.postProcess.sceneMap.push_back(sceneMappingItem);
    audioEffectService->UpdateAvailableAEConfig(aeConfig);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    AudioEffectServiceFuzzTest,
    AudioEffectServiceGetAvailableEffectsFuzzTest,
    AudioEffectServiceGetOriginalEffectConfigFuzzTest,
    AudioEffectServiceUpdateAvailableEffectsFuzzTest,
    AudioEffectServiceQueryEffectManagerSceneModeFuzzTest,
    AudioEffectServiceGetSupportedEffectConfigFuzzTest,
    AudioEffectServiceSetMasterSinkAvailableFuzzTest,
    AudioEffectServiceConstructEffectChainModeFuzzTest,
    AudioEffectServiceConstructEffectChainManagerParamFuzzTest,
    AudioEffectServiceConstructEnhanceChainManagerParamFuzzTest,
    AudioEffectServiceAddSupportedPropertyByDeviceInnerFuzzTest,
    AudioEffectServiceAddSupportedAudioEffectPropertyByDeviceFuzzTest,
    AudioEffectServiceAddSupportedAudioEnhancePropertyByDeviceFuzzTest,
    AudioEffectServiceUpdateUnavailableEffectChainsFuzzTest,
    AudioEffectServiceUpdateSupportedEffectPropertyFuzzTest,
    AudioEffectServiceUpdateAvailableAEConfigFuzzTest,
    });
    func(fdp);
}
void Init(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return;
    }
    RAW_DATA = data;
    g_dataSize = size;
    g_pos = 0;
}
void Init()
{
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::Init(data, size);
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
extern "C" int LLVMFuzzerInitialize(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::Init();
    return 0;
}
