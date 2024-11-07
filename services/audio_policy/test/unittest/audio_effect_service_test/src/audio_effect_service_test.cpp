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
#include "audio_effect.h"
#include "audio_effect_service_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioEffectServiceTest::SetUpTestCase(void) {}
void AudioEffectServiceTest::TearDownTestCase(void) {}
void AudioEffectServiceTest::SetUp(void) {}
void AudioEffectServiceTest::TearDown(void) {}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: AudioEffectService_001.
* @tc.desc  : Test EffectServiceInit interfaces.
*/
HWTEST(AudioEffectServiceTest, AudioEffectService_001, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    audioEffectService_->EffectServiceInit();
    EXPECT_NE(audioEffectService_, nullptr);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_002.
* @tc.desc  : Test VerifySceneMappingItem interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_002, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    SceneMappingItem item;
    audioEffectService_->VerifySceneMappingItem(item);
    EXPECT_NE(audioEffectService_, nullptr);

    item.name = "STREAM_USAGE_UNKNOWN";
    audioEffectService_->VerifySceneMappingItem(item);
    EXPECT_NE(audioEffectService_, nullptr);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_003.
* @tc.desc  : Test UpdateEffectChains interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_003, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    std::vector<std::string> availableLayout;
    audioEffectService_->UpdateEffectChains(availableLayout);
    EXPECT_NE(audioEffectService_, nullptr);

    EffectChain effectChain;
    audioEffectService_->supportedEffectConfig_.effectChains.push_back(effectChain);
    for (auto it = audioEffectService_->supportedEffectConfig_.effectChains.begin();
        it != audioEffectService_->supportedEffectConfig_.effectChains.end(); ++it) {
            it->apply.push_back("test");
    }
    audioEffectService_->UpdateEffectChains(availableLayout);
    EXPECT_NE(audioEffectService_, nullptr);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_004.
* @tc.desc  : Test UpdateAvailableAEConfig interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_004, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    OriginalEffectConfig aeConfig;
    SceneMappingItem sceneMappingItem;
    aeConfig.postProcess.sceneMap.push_back(sceneMappingItem);
    audioEffectService_->UpdateAvailableAEConfig(aeConfig);
    EXPECT_NE(audioEffectService_, nullptr);

    Stream newStream;
    audioEffectService_->supportedEffectConfig_.postProcessNew.stream.push_back(newStream);
    audioEffectService_->UpdateAvailableAEConfig(aeConfig);
    EXPECT_NE(audioEffectService_, nullptr);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_005.
* @tc.desc  : Test UpdateUnsupportedModePre interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_005, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    PreStreamScene preStreamScene;
    OriginalEffectConfig aeConfig;
    aeConfig.preProcess.defaultScenes.push_back(preStreamScene);
    audioEffectService_->UpdateAvailableAEConfig(aeConfig);
    EXPECT_NE(audioEffectService_, nullptr);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_006.
* @tc.desc  : Test UpdateUnsupportedModePre/UpdateUnsupportedDevicePre interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_006, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    PreStreamScene preStreamScene;
    preStreamScene.mode.push_back("ENHANCE");
    preStreamScene.stream = "SCENE_VOIP_UP";
    OriginalEffectConfig aeConfig;
    aeConfig.preProcess.defaultScenes.push_back(preStreamScene);
    audioEffectService_->UpdateAvailableAEConfig(aeConfig);
    EXPECT_NE(audioEffectService_, nullptr);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_007.
* @tc.desc  : Test UpdateUnsupportedModePre/UpdateUnsupportedDevicePre interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_007, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    PreStreamScene preStreamScene;
    preStreamScene.mode.push_back("ENHANCE_NONE");
    preStreamScene.stream = "SCENE_VOIP_UP";
    OriginalEffectConfig aeConfig;
    aeConfig.preProcess.defaultScenes.push_back(preStreamScene);
    audioEffectService_->UpdateAvailableAEConfig(aeConfig);
    EXPECT_NE(audioEffectService_, nullptr);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_008.
* @tc.desc  : Test UpdateUnsupportedModePre/UpdateUnsupportedDevicePre interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_008, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    PreStreamScene preStreamScene;
    preStreamScene.mode.push_back("ENHANCE_DEFAULT");
    preStreamScene.stream = "SCENE_VOIP_UP";
    OriginalEffectConfig aeConfig;
    Device device;
    std::vector<Device> newDevice;
    newDevice.push_back(device);
    preStreamScene.device.push_back(newDevice);
    aeConfig.preProcess.defaultScenes.push_back(preStreamScene);
    audioEffectService_->UpdateAvailableAEConfig(aeConfig);
    EXPECT_NE(audioEffectService_, nullptr);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_009.
* @tc.desc  : Test UpdateUnsupportedModePost interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_009, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    PostStreamScene postStreamScene;
    OriginalEffectConfig aeConfig;
    aeConfig.postProcess.defaultScenes.push_back(postStreamScene);
    audioEffectService_->UpdateAvailableAEConfig(aeConfig);
    EXPECT_NE(audioEffectService_, nullptr);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_010.
* @tc.desc  : Test UpdateUnsupportedModePost/UpdateUnsupportedDevicePost interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_010, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    PostStreamScene postStreamScene;
    postStreamScene.mode.push_back("EFFECT");
    postStreamScene.stream = "SCENE_OTHERS";
    OriginalEffectConfig aeConfig;
    aeConfig.postProcess.defaultScenes.push_back(postStreamScene);
    audioEffectService_->UpdateAvailableAEConfig(aeConfig);
    EXPECT_NE(audioEffectService_, nullptr);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_011.
* @tc.desc  : Test UpdateUnsupportedModePost/UpdateUnsupportedDevicePost interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_011, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    PostStreamScene postStreamScene;
    postStreamScene.mode.push_back("EFFECT_NONE");
    postStreamScene.stream = "SCENE_OTHERS";
    OriginalEffectConfig aeConfig;
    aeConfig.postProcess.defaultScenes.push_back(postStreamScene);
    audioEffectService_->UpdateAvailableAEConfig(aeConfig);
    EXPECT_NE(audioEffectService_, nullptr);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_012.
* @tc.desc  : Test UpdateUnsupportedModePost/UpdateUnsupportedDevicePost interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_012, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    PostStreamScene postStreamScene;
    postStreamScene.mode.push_back("EFFECT_DEFAULT");
    postStreamScene.stream = "SCENE_OTHERS";
    OriginalEffectConfig aeConfig;
    Device device;
    std::vector<Device> newDevice;
    newDevice.push_back(device);
    postStreamScene.device.push_back(newDevice);
    aeConfig.postProcess.defaultScenes.push_back(postStreamScene);
    audioEffectService_->UpdateAvailableAEConfig(aeConfig);
    EXPECT_NE(audioEffectService_, nullptr);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_013.
* @tc.desc  : Test UpdateAvailableSceneMapPost interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_013, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    OriginalEffectConfig aeConfig;
    SceneMappingItem sceneMappingItem;
    sceneMappingItem.name = "STREAM_USAGE_MEDIA";
    sceneMappingItem.sceneType = "sceneType";
    audioEffectService_->postSceneTypeSet_.push_back("sceneType");
    aeConfig.postProcess.sceneMap.push_back(sceneMappingItem);
    audioEffectService_->UpdateAvailableAEConfig(aeConfig);
    EXPECT_NE(audioEffectService_, nullptr);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_014.
* @tc.desc  : Test UpdateDuplicateBypassMode interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_014, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    Stream newStream1;
    Stream newStream2;
    ProcessNew processNew;
    StreamEffectMode streamEffect;
    processNew.stream.push_back(newStream1);
    streamEffect.mode = "EFFECT_NONE";
    newStream2.streamEffectMode.push_back(streamEffect);
    processNew.stream.push_back(newStream2);
    audioEffectService_->UpdateDuplicateBypassMode(processNew);
    EXPECT_NE(audioEffectService_, nullptr);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_015.
* @tc.desc  : Test UpdateDuplicateMode interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_015, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    Stream newStream;
    ProcessNew processNew;
    StreamEffectMode streamEffect;
    newStream.streamEffectMode.push_back(streamEffect);
    processNew.stream.push_back(newStream);
    audioEffectService_->UpdateDuplicateMode(processNew);
    EXPECT_NE(audioEffectService_, nullptr);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_016.
* @tc.desc  : Test UpdateDuplicateDevice interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_016, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    Device device1;
    Device device2;
    Stream newStream;
    ProcessNew processNew;
    StreamEffectMode streamEffect;
    device1.type = "test";
    device2.type = "test";
    streamEffect.devicePort.push_back(device1);
    streamEffect.devicePort.push_back(device2);
    newStream.streamEffectMode.push_back(streamEffect);
    processNew.stream.push_back(newStream);
    audioEffectService_->UpdateDuplicateDevice(processNew);
    EXPECT_NE(audioEffectService_, nullptr);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_017.
* @tc.desc  : Test UpdateDuplicateScene interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_017, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    Stream newStream1;
    Stream newStream2;
    newStream1.scene = "test";
    newStream1.priority = NORMAL_SCENE;
    newStream2.scene = "test";
    newStream2.priority = NORMAL_SCENE;
    ProcessNew processNew;
    processNew.stream.push_back(newStream1);
    processNew.stream.push_back(newStream2);
    audioEffectService_->UpdateDuplicateScene(processNew);
    EXPECT_NE(audioEffectService_, nullptr);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_018.
* @tc.desc  : Test UpdateDuplicateDefaultScene interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_018, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    ProcessNew processNew;
    Stream newStream0;
    Stream newStream1;
    Stream newStream2;
    newStream0.priority = NORMAL_SCENE;
    newStream1.priority = DEFAULT_SCENE;
    newStream2.priority = DEFAULT_SCENE;
    processNew.stream.push_back(newStream0);
    audioEffectService_->UpdateDuplicateDefaultScene(processNew);
    EXPECT_NE(audioEffectService_, nullptr);

    processNew.stream.push_back(newStream1);
    processNew.stream.push_back(newStream2);
    audioEffectService_->UpdateDuplicateDefaultScene(processNew);
    EXPECT_NE(audioEffectService_, nullptr);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_019.
* @tc.desc  : Test UpdateUnavailableEffectChains/UpdateUnavailableEffectChainsRecord interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_019, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    Stream newStream;
    ProcessNew processNew;
    StreamEffectMode streamEffect;
    newStream.streamEffectMode.push_back(streamEffect);
    processNew.stream.push_back(newStream);
    std::vector<std::string> availableLayout;
    audioEffectService_->UpdateUnavailableEffectChains(availableLayout, processNew);
    EXPECT_NE(audioEffectService_, nullptr);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_020.
* @tc.desc  : Test UpdateUnavailableEffectChains/UpdateUnavailableEffectChainsRecord interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_020, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    Device device1;
    Device device2;
    device1.chain = "test";
    device2.chain = "test";
    Stream newStream;
    ProcessNew processNew;
    StreamEffectMode streamEffect;
    streamEffect.devicePort.push_back(device1);
    streamEffect.devicePort.push_back(device2);
    newStream.streamEffectMode.push_back(streamEffect);
    processNew.stream.push_back(newStream);
    std::vector<std::string> availableLayout;
    audioEffectService_->UpdateUnavailableEffectChains(availableLayout, processNew);
    EXPECT_NE(audioEffectService_, nullptr);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_021.
* @tc.desc  : Test UpdateUnavailableEffectChains/UpdateUnavailableEffectChainsRecord interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_021, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    Device device1;
    Device device2;
    device1.chain = "test1";
    device2.chain = "test2";
    Stream newStream;
    ProcessNew processNew;
    StreamEffectMode streamEffect;
    streamEffect.devicePort.push_back(device1);
    streamEffect.devicePort.push_back(device2);
    newStream.streamEffectMode.push_back(streamEffect);
    processNew.stream.push_back(newStream);
    std::vector<std::string> availableLayout;
    availableLayout.push_back("test1");
    audioEffectService_->UpdateUnavailableEffectChains(availableLayout, processNew);
    EXPECT_NE(audioEffectService_, nullptr);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_022.
* @tc.desc  : Test UpdateSupportedEffectProperty interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_022, TestSize.Level1)
{
    Device device;
    Effect effect;
    EffectChain effectChain;
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    std::unordered_map<std::string, std::set<std::pair<std::string, std::string>>> device2PropertySet;
    audioEffectService_->UpdateSupportedEffectProperty(device, device2PropertySet);
    EXPECT_NE(audioEffectService_, nullptr);

    device.chain = "test";
    effectChain.name = "test";
    effectChain.apply.push_back("test");
    audioEffectService_->supportedEffectConfig_.effectChains.push_back(effectChain);
    audioEffectService_->UpdateSupportedEffectProperty(device, device2PropertySet);
    EXPECT_NE(audioEffectService_, nullptr);

    effect.name = "test";
    device.type = "test";
    effect.effectProperty.push_back("test1");
    audioEffectService_->availableEffects_.push_back(effect);
    audioEffectService_->UpdateSupportedEffectProperty(device, device2PropertySet);
    EXPECT_NE(audioEffectService_, nullptr);

    std::set<std::pair<std::string, std::string>> deviceSet = {{"test1", "value1"}, {"test2", "value2"}};
    device2PropertySet.insert({"test", deviceSet});
    audioEffectService_->UpdateSupportedEffectProperty(device, device2PropertySet);
    EXPECT_NE(audioEffectService_, nullptr);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_023.
* @tc.desc  : Test UpdateDuplicateProcessNew interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_023, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    Stream newStream;
    ProcessNew processNew;
    StreamEffectMode streamEffect;
    newStream.streamEffectMode.push_back(streamEffect);
    processNew.stream.push_back(newStream);
    std::vector<std::string> availableLayout;
    audioEffectService_->UpdateDuplicateProcessNew(availableLayout, processNew);
    EXPECT_NE(audioEffectService_, nullptr);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_024.
* @tc.desc  : Test BuildAvailableAEConfig interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_024, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    audioEffectService_->BuildAvailableAEConfig();
    EXPECT_NE(audioEffectService_, nullptr);

    EffectChain effectChain;
    PreStreamScene preStreamScene;
    PostStreamScene postStreamScene;
    audioEffectService_->oriEffectConfig_.effectChains.push_back(effectChain);
    audioEffectService_->oriEffectConfig_.preProcess.defaultScenes.push_back(preStreamScene);
    audioEffectService_->oriEffectConfig_.preProcess.normalScenes.push_back(preStreamScene);
    audioEffectService_->oriEffectConfig_.postProcess.defaultScenes.push_back(postStreamScene);
    audioEffectService_->oriEffectConfig_.postProcess.normalScenes.push_back(postStreamScene);
    audioEffectService_->BuildAvailableAEConfig();
    EXPECT_NE(audioEffectService_, nullptr);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_025.
* @tc.desc  : Test ConstructEffectChainMode interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_025, TestSize.Level1)
{
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
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    audioEffectService_->ConstructEffectChainMode(mode, sceneType, effectChainMgrParam);
    EXPECT_NE(audioEffectService_, nullptr);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_026.
* @tc.desc  : Test ConstructDefaultEffectProperty interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_026, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    EffectChain effectChain;
    std::string chainName = "chainName";
    std::unordered_map<std::string, std::string> effectDefaultProperty;
    audioEffectService_->supportedEffectConfig_.effectChains.push_back(effectChain);
    audioEffectService_->ConstructDefaultEffectProperty(chainName, effectDefaultProperty);
    EXPECT_NE(audioEffectService_, nullptr);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_027.
* @tc.desc  : Test ConstructDefaultEffectProperty interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_027, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    EffectChain effectChain;
    effectChain.name = "chainName";
    effectChain.apply.push_back("test");
    std::string chainName = "chainName";
    Effect effect;
    audioEffectService_->availableEffects_.push_back(effect);
    std::unordered_map<std::string, std::string> effectDefaultProperty;
    audioEffectService_->supportedEffectConfig_.effectChains.push_back(effectChain);
    audioEffectService_->ConstructDefaultEffectProperty(chainName, effectDefaultProperty);
    EXPECT_NE(audioEffectService_, nullptr);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_028.
* @tc.desc  : Test ConstructDefaultEffectProperty interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_028, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    EffectChain effectChain;
    effectChain.name = "chainName";
    effectChain.apply.push_back("test");
    effectChain.apply.push_back("test");
    std::string chainName = "chainName";
    Effect effect;
    effect.name = "test";
    effect.effectProperty.push_back("test");
    audioEffectService_->availableEffects_.push_back(effect);
    std::unordered_map<std::string, std::string> effectDefaultProperty;
    audioEffectService_->supportedEffectConfig_.effectChains.push_back(effectChain);
    audioEffectService_->ConstructDefaultEffectProperty(chainName, effectDefaultProperty);
    EXPECT_NE(audioEffectService_, nullptr);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_029.
* @tc.desc  : Test ConstructDefaultEffectProperty interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_029, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    EffectChain effectChain;
    effectChain.name = "chainName";
    effectChain.apply.push_back("test");
    std::string chainName = "chainName";
    Effect effect;
    effect.name = "test";
    audioEffectService_->availableEffects_.push_back(effect);
    std::unordered_map<std::string, std::string> effectDefaultProperty;
    audioEffectService_->supportedEffectConfig_.effectChains.push_back(effectChain);
    audioEffectService_->ConstructDefaultEffectProperty(chainName, effectDefaultProperty);
    EXPECT_NE(audioEffectService_, nullptr);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_030.
* @tc.desc  : Test ConstructEffectChainManagerParam interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_030, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    Stream stream1;
    Stream stream2;
    stream1.scene = "test";
    stream1.priority = PRIOR_SCENE;
    stream2.scene = "test";
    stream2.priority = DEFAULT_SCENE;
    EffectChainManagerParam effectChainManagerParam;
    audioEffectService_->supportedEffectConfig_.postProcessNew.stream.push_back(stream1);
    audioEffectService_->supportedEffectConfig_.postProcessNew.stream.push_back(stream2);
    audioEffectService_->ConstructEffectChainManagerParam(effectChainManagerParam);
    EXPECT_NE(audioEffectService_, nullptr);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_031.
* @tc.desc  : Test ConstructEnhanceChainManagerParam interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_031, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    Stream stream1;
    Stream stream2;
    stream1.scene = "test";
    stream1.priority = PRIOR_SCENE;
    stream2.scene = "test";
    stream2.priority = DEFAULT_SCENE;
    EffectChainManagerParam effectChainManagerParam;
    audioEffectService_->supportedEffectConfig_.preProcessNew.stream.push_back(stream1);
    audioEffectService_->supportedEffectConfig_.preProcessNew.stream.push_back(stream2);
    audioEffectService_->ConstructEnhanceChainManagerParam(effectChainManagerParam);
    EXPECT_NE(audioEffectService_, nullptr);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_032.
* @tc.desc  : Test AddSupportedPropertyByDeviceInner interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_032, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    std::set<std::pair<std::string, std::string>> mergedSet;
    const std::unordered_map<std::string, std::set<std::pair<std::string, std::string>>> device2PropertySet;
    int32_t result = audioEffectService_->AddSupportedPropertyByDeviceInner(DeviceType::DEVICE_TYPE_MAX,
        mergedSet, device2PropertySet);
    EXPECT_EQ(result, -1);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_033.
* @tc.desc  : Test AddSupportedPropertyByDeviceInner interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_033, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    std::set<std::pair<std::string, std::string>> mergedSet;
    std::unordered_map<std::string, std::set<std::pair<std::string, std::string>>> device2PropertySet;
    std::set<std::pair<std::string, std::string>> device2Property;
    device2PropertySet.insert({"DEVICE_TYPE_DEFAULT", device2Property});
    int32_t result = audioEffectService_->AddSupportedPropertyByDeviceInner(DeviceType::DEVICE_TYPE_INVALID,
        mergedSet, device2PropertySet);
    EXPECT_EQ(result, AUDIO_OK);
}

/**
* @tc.name  : Test AudioEffectService.
* @tc.number: audioEffectService_034.
* @tc.desc  : Test AddSupportedPropertyByDeviceInner interfaces.
*/
HWTEST(AudioEffectServiceTest, audioEffectService_034, TestSize.Level1)
{
    auto audioEffectService_ = std::make_shared<AudioEffectService>();
    std::set<std::pair<std::string, std::string>> mergedSet;
    std::unordered_map<std::string, std::set<std::pair<std::string, std::string>>> device2PropertySet;
    std::set<std::pair<std::string, std::string>> device2Property;
    device2PropertySet.insert({"DEVICE_TYPE_DEFAULT", device2Property});
    int32_t result = audioEffectService_->AddSupportedPropertyByDeviceInner(DeviceType::DEVICE_TYPE_NONE,
        mergedSet, device2PropertySet);
    EXPECT_EQ(result, AUDIO_OK);
}
} // namespace AudioStandard
} // namespace OHOS
