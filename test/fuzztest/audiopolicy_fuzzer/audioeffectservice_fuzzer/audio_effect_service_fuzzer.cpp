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
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;
const size_t MAX_STRING_LEN = 64;
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

template<typename T>
const T& PickValue(FuzzedDataProvider& fdp, const std::vector<T>& values)
{
    return values[fdp.ConsumeIntegralInRange<size_t>(0, values.size() - 1)];
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

void AudioEffectServiceFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioEffectService> audioEffectService = std::make_shared<AudioEffectService>();
    if (audioEffectService == nullptr) {
        return;
    }
    audioEffectService->EffectServiceInit();
    audioEffectService->BuildAvailableAEConfig();
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

void AudioEffectServiceGetOriginalEffectConfigFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioEffectService> audioEffectService = std::make_shared<AudioEffectService>();
    if (audioEffectService == nullptr) {
        return;
    }

    OriginalEffectConfig oriEffectConfig;
    audioEffectService->GetOriginalEffectConfig(oriEffectConfig);
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

void AudioEffectServiceQueryEffectManagerSceneModeFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioEffectService> audioEffectService = std::make_shared<AudioEffectService>();
    if (audioEffectService == nullptr) {
        return;
    }

    SupportedEffectConfig supportedEffectConfig;
    audioEffectService->QueryEffectManagerSceneMode(supportedEffectConfig);
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
    stream1.scene = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    stream1.priority = PickValue(fdp, testScenePriorities);
    stream2.scene = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    stream2.priority = PickValue(fdp, testScenePriorities);
    EffectChainManagerParam effectChainManagerParam;
    audioEffectService->supportedEffectConfig_.postProcessNew.stream.push_back(stream1);
    audioEffectService->supportedEffectConfig_.postProcessNew.stream.push_back(stream2);
    audioEffectService->ConstructEffectChainManagerParam(effectChainManagerParam);
}

void AudioEffectServiceConstructEffectChainModeFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioEffectService> audioEffectService = std::make_shared<AudioEffectService>();
    if (audioEffectService == nullptr) {
        return;
    }

    Device device1;
    Device device2;
    device1.chain = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    device1.type = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    device2.chain = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    device2.type = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    StreamEffectMode mode;
    mode.mode = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    mode.devicePort.push_back(device1);
    mode.devicePort.push_back(device2);
    std::string sceneType = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    EffectChainManagerParam effectChainMgrParam;
    audioEffectService->ConstructEffectChainMode(mode, sceneType, effectChainMgrParam);
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
    stream1.scene = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    stream1.priority = PickValue(fdp, testScenePriorities);
    stream2.scene = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    stream2.priority = PickValue(fdp, testScenePriorities);
    EffectChainManagerParam effectChainManagerParam;
    audioEffectService->supportedEffectConfig_.preProcessNew.stream.push_back(stream1);
    audioEffectService->supportedEffectConfig_.preProcessNew.stream.push_back(stream2);
    audioEffectService->ConstructEnhanceChainManagerParam(effectChainManagerParam);
}

void AudioEffectServiceAddSupportedAudioEffectPropertyByDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioEffectService> audioEffectService = std::make_shared<AudioEffectService>();
    if (audioEffectService == nullptr || g_testDeviceTypes.empty()) {
        return;
    }

    std::set<std::pair<std::string, std::string>> mergedSet;
    DeviceType deviceType = PickValue(fdp, g_testDeviceTypes);
    audioEffectService->AddSupportedAudioEffectPropertyByDevice(deviceType, mergedSet);
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
    device2PropertySet.insert({fdp.ConsumeRandomLengthString(MAX_STRING_LEN), device2Property});
    DeviceType deviceType = PickValue(fdp, g_testDeviceTypes);
    audioEffectService->AddSupportedPropertyByDeviceInner(deviceType, mergedSet, device2PropertySet);
}

void AudioEffectServiceAddSupportedAudioEnhancePropertyByDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioEffectService> audioEffectService = std::make_shared<AudioEffectService>();
    if (audioEffectService == nullptr || g_testDeviceTypes.empty()) {
        return;
    }

    std::set<std::pair<std::string, std::string>> mergedSet;
    DeviceType deviceType = PickValue(fdp, g_testDeviceTypes);
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

void AudioEffectServiceUpdateAvailableAEConfigFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioEffectService> audioEffectService = std::make_shared<AudioEffectService>();
    if (audioEffectService == nullptr) {
        return;
    }
    OriginalEffectConfig aeConfig;
    SceneMappingItem sceneMappingItem;
    PreStreamScene preStreamScene;
    preStreamScene.stream = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    preStreamScene.mode.push_back(fdp.ConsumeRandomLengthString(MAX_STRING_LEN));
    aeConfig.preProcess.defaultScenes.push_back(preStreamScene);
    aeConfig.preProcess.priorScenes.push_back(preStreamScene);
    aeConfig.preProcess.normalScenes.push_back(preStreamScene);
    aeConfig.postProcess.sceneMap.push_back(sceneMappingItem);
    audioEffectService->UpdateAvailableAEConfig(aeConfig);
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
    std::string chainName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    std::string applyName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    std::string propertyName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    std::string propertyValue = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    device.chain = chainName;
    effectChain.name = chainName;
    effectChain.apply.push_back(applyName);
    effect.effectProperty.push_back(propertyName);
    std::set<std::pair<std::string, std::string>> deviceSet = {{propertyName, propertyValue}};
    device2PropertySet.insert({chainName, deviceSet});
    audioEffectService->supportedEffectConfig_.effectChains.push_back(effectChain);
    audioEffectService->UpdateSupportedEffectProperty(device, device2PropertySet);
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

    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
extern "C" int LLVMFuzzerInitialize(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::Init();
    return 0;
}
