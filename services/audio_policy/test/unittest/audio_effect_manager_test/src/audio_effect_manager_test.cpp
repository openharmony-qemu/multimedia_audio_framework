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
#include "audio_effect_manager_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioEffectManagerTest::SetUpTestCase(void) {}
void AudioEffectManagerTest::TearDownTestCase(void) {}
void AudioEffectManagerTest::SetUp(void) {}
void AudioEffectManagerTest::TearDown(void) {}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_001.
* @tc.desc  : Test EffectManagerInit interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_001, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
    audioEffectManager_->EffectManagerInit();
    EXPECT_NE(audioEffectManager_, nullptr);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_002.
* @tc.desc  : Test VerifySceneMappingItem interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_002, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
    SceneMappingItem item;
    audioEffectManager_->VerifySceneMappingItem(item);
    EXPECT_NE(audioEffectManager_, nullptr);

    item.name = "STREAM_USAGE_UNKNOWN";
    audioEffectManager_->VerifySceneMappingItem(item);
    EXPECT_NE(audioEffectManager_, nullptr);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_003.
* @tc.desc  : Test UpdateEffectChains interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_003, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
    std::vector<std::string> availableLayout;
    audioEffectManager_->UpdateEffectChains(availableLayout);
    EXPECT_NE(audioEffectManager_, nullptr);

    EffectChain effectChain;
    audioEffectManager_->supportedEffectConfig_.effectChains.push_back(effectChain);
    for (auto it = audioEffectManager_->supportedEffectConfig_.effectChains.begin();
        it != audioEffectManager_->supportedEffectConfig_.effectChains.end(); ++it) {
            it->apply.push_back("test");
    }
    audioEffectManager_->UpdateEffectChains(availableLayout);
    EXPECT_NE(audioEffectManager_, nullptr);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_004.
* @tc.desc  : Test UpdateAvailableAEConfig interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_004, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
    OriginalEffectConfig aeConfig;
    SceneMappingItem sceneMappingItem;
    aeConfig.postProcess.sceneMap.push_back(sceneMappingItem);
    audioEffectManager_->UpdateAvailableAEConfig(aeConfig);
    EXPECT_NE(audioEffectManager_, nullptr);

    Stream newStream;
    audioEffectManager_->supportedEffectConfig_.postProcessNew.stream.push_back(newStream);
    audioEffectManager_->UpdateAvailableAEConfig(aeConfig);
    EXPECT_NE(audioEffectManager_, nullptr);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_005.
* @tc.desc  : Test UpdateUnsupportedModePre interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_005, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
    PreStreamScene preStreamScene;
    OriginalEffectConfig aeConfig;
    aeConfig.preProcess.defaultScenes.push_back(preStreamScene);
    audioEffectManager_->UpdateAvailableAEConfig(aeConfig);
    EXPECT_NE(audioEffectManager_, nullptr);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_006.
* @tc.desc  : Test UpdateUnsupportedModePre/UpdateUnsupportedDevicePre interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_006, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
    PreStreamScene preStreamScene;
    preStreamScene.mode.push_back("ENHANCE");
    preStreamScene.stream = "SCENE_VOIP_UP";
    OriginalEffectConfig aeConfig;
    aeConfig.preProcess.defaultScenes.push_back(preStreamScene);
    audioEffectManager_->UpdateAvailableAEConfig(aeConfig);
    EXPECT_NE(audioEffectManager_, nullptr);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_007.
* @tc.desc  : Test UpdateUnsupportedModePre/UpdateUnsupportedDevicePre interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_007, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
    PreStreamScene preStreamScene;
    preStreamScene.mode.push_back("ENHANCE_NONE");
    preStreamScene.stream = "SCENE_VOIP_UP";
    OriginalEffectConfig aeConfig;
    aeConfig.preProcess.defaultScenes.push_back(preStreamScene);
    audioEffectManager_->UpdateAvailableAEConfig(aeConfig);
    EXPECT_NE(audioEffectManager_, nullptr);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_008.
* @tc.desc  : Test UpdateUnsupportedModePre/UpdateUnsupportedDevicePre interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_008, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
    PreStreamScene preStreamScene;
    preStreamScene.mode.push_back("ENHANCE_DEFAULT");
    preStreamScene.stream = "SCENE_VOIP_UP";
    OriginalEffectConfig aeConfig;
    Device device;
    std::vector<Device> newDevice;
    newDevice.push_back(device);
    preStreamScene.device.push_back(newDevice);
    aeConfig.preProcess.defaultScenes.push_back(preStreamScene);
    audioEffectManager_->UpdateAvailableAEConfig(aeConfig);
    EXPECT_NE(audioEffectManager_, nullptr);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_009.
* @tc.desc  : Test UpdateUnsupportedModePost interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_009, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
    PostStreamScene postStreamScene;
    OriginalEffectConfig aeConfig;
    aeConfig.postProcess.defaultScenes.push_back(postStreamScene);
    audioEffectManager_->UpdateAvailableAEConfig(aeConfig);
    EXPECT_NE(audioEffectManager_, nullptr);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_010.
* @tc.desc  : Test UpdateUnsupportedModePost/UpdateUnsupportedDevicePost interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_010, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
    PostStreamScene postStreamScene;
    postStreamScene.mode.push_back("EFFECT");
    postStreamScene.stream = "SCENE_OTHERS";
    OriginalEffectConfig aeConfig;
    aeConfig.postProcess.defaultScenes.push_back(postStreamScene);
    audioEffectManager_->UpdateAvailableAEConfig(aeConfig);
    EXPECT_NE(audioEffectManager_, nullptr);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_011.
* @tc.desc  : Test UpdateUnsupportedModePost/UpdateUnsupportedDevicePost interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_011, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
    PostStreamScene postStreamScene;
    postStreamScene.mode.push_back("EFFECT_NONE");
    postStreamScene.stream = "SCENE_OTHERS";
    OriginalEffectConfig aeConfig;
    aeConfig.postProcess.defaultScenes.push_back(postStreamScene);
    audioEffectManager_->UpdateAvailableAEConfig(aeConfig);
    EXPECT_NE(audioEffectManager_, nullptr);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_012.
* @tc.desc  : Test UpdateUnsupportedModePost/UpdateUnsupportedDevicePost interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_012, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
    PostStreamScene postStreamScene;
    postStreamScene.mode.push_back("EFFECT_DEFAULT");
    postStreamScene.stream = "SCENE_OTHERS";
    OriginalEffectConfig aeConfig;
    Device device;
    std::vector<Device> newDevice;
    newDevice.push_back(device);
    postStreamScene.device.push_back(newDevice);
    aeConfig.postProcess.defaultScenes.push_back(postStreamScene);
    audioEffectManager_->UpdateAvailableAEConfig(aeConfig);
    EXPECT_NE(audioEffectManager_, nullptr);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_013.
* @tc.desc  : Test UpdateAvailableSceneMapPost interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_013, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
    OriginalEffectConfig aeConfig;
    SceneMappingItem sceneMappingItem;
    sceneMappingItem.name = "STREAM_USAGE_MEDIA";
    sceneMappingItem.sceneType = "sceneType";
    audioEffectManager_->postSceneTypeSet_.push_back("sceneType");
    aeConfig.postProcess.sceneMap.push_back(sceneMappingItem);
    audioEffectManager_->UpdateAvailableAEConfig(aeConfig);
    EXPECT_NE(audioEffectManager_, nullptr);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_014.
* @tc.desc  : Test UpdateDuplicateBypassMode interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_014, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
    Stream newStream1;
    Stream newStream2;
    ProcessNew processNew;
    StreamEffectMode streamEffect;
    processNew.stream.push_back(newStream1);
    streamEffect.mode = "EFFECT_NONE";
    newStream2.streamEffectMode.push_back(streamEffect);
    processNew.stream.push_back(newStream2);
    audioEffectManager_->UpdateDuplicateBypassMode(processNew);
    EXPECT_NE(audioEffectManager_, nullptr);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_015.
* @tc.desc  : Test UpdateDuplicateMode interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_015, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
    Stream newStream;
    ProcessNew processNew;
    StreamEffectMode streamEffect;
    newStream.streamEffectMode.push_back(streamEffect);
    processNew.stream.push_back(newStream);
    audioEffectManager_->UpdateDuplicateMode(processNew);
    EXPECT_NE(audioEffectManager_, nullptr);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_016.
* @tc.desc  : Test UpdateDuplicateDevice interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_016, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
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
    audioEffectManager_->UpdateDuplicateDevice(processNew);
    EXPECT_NE(audioEffectManager_, nullptr);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_017.
* @tc.desc  : Test UpdateDuplicateScene interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_017, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
    Stream newStream1;
    Stream newStream2;
    newStream1.scene = "test";
    newStream1.priority = NORMAL_SCENE;
    newStream2.scene = "test";
    newStream2.priority = NORMAL_SCENE;
    ProcessNew processNew;
    processNew.stream.push_back(newStream1);
    processNew.stream.push_back(newStream2);
    audioEffectManager_->UpdateDuplicateScene(processNew);
    EXPECT_NE(audioEffectManager_, nullptr);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_018.
* @tc.desc  : Test UpdateDuplicateDefaultScene interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_018, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
    ProcessNew processNew;
    Stream newStream0;
    Stream newStream1;
    Stream newStream2;
    newStream0.priority = NORMAL_SCENE;
    newStream1.priority = DEFAULT_SCENE;
    newStream2.priority = DEFAULT_SCENE;
    processNew.stream.push_back(newStream0);
    audioEffectManager_->UpdateDuplicateDefaultScene(processNew);
    EXPECT_NE(audioEffectManager_, nullptr);

    processNew.stream.push_back(newStream1);
    processNew.stream.push_back(newStream2);
    audioEffectManager_->UpdateDuplicateDefaultScene(processNew);
    EXPECT_NE(audioEffectManager_, nullptr);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_019.
* @tc.desc  : Test UpdateUnavailableEffectChains/UpdateUnavailableEffectChainsRecord interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_019, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
    Stream newStream;
    ProcessNew processNew;
    StreamEffectMode streamEffect;
    newStream.streamEffectMode.push_back(streamEffect);
    processNew.stream.push_back(newStream);
    std::vector<std::string> availableLayout;
    audioEffectManager_->UpdateUnavailableEffectChains(availableLayout, processNew);
    EXPECT_NE(audioEffectManager_, nullptr);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_020.
* @tc.desc  : Test UpdateUnavailableEffectChains/UpdateUnavailableEffectChainsRecord interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_020, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
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
    audioEffectManager_->UpdateUnavailableEffectChains(availableLayout, processNew);
    EXPECT_NE(audioEffectManager_, nullptr);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_021.
* @tc.desc  : Test UpdateUnavailableEffectChains/UpdateUnavailableEffectChainsRecord interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_021, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
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
    audioEffectManager_->UpdateUnavailableEffectChains(availableLayout, processNew);
    EXPECT_NE(audioEffectManager_, nullptr);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_022.
* @tc.desc  : Test UpdateSupportedEffectProperty interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_022, TestSize.Level1)
{
    Device device;
    Effect effect;
    EffectChain effectChain;
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
    std::unordered_map<std::string, std::set<std::pair<std::string, std::string>>> device2PropertySet;
    audioEffectManager_->UpdateSupportedEffectProperty(device, device2PropertySet);
    EXPECT_NE(audioEffectManager_, nullptr);

    device.chain = "test";
    effectChain.name = "test";
    effectChain.apply.push_back("test");
    audioEffectManager_->supportedEffectConfig_.effectChains.push_back(effectChain);
    audioEffectManager_->UpdateSupportedEffectProperty(device, device2PropertySet);
    EXPECT_NE(audioEffectManager_, nullptr);

    effect.name = "test";
    device.type = "test";
    effect.effectProperty.push_back("test1");
    audioEffectManager_->availableEffects_.push_back(effect);
    audioEffectManager_->UpdateSupportedEffectProperty(device, device2PropertySet);
    EXPECT_NE(audioEffectManager_, nullptr);

    std::set<std::pair<std::string, std::string>> deviceSet = {{"test1", "value1"}, {"test2", "value2"}};
    device2PropertySet.insert({"test", deviceSet});
    audioEffectManager_->UpdateSupportedEffectProperty(device, device2PropertySet);
    EXPECT_NE(audioEffectManager_, nullptr);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_023.
* @tc.desc  : Test UpdateDuplicateProcessNew interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_023, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
    Stream newStream;
    ProcessNew processNew;
    StreamEffectMode streamEffect;
    newStream.streamEffectMode.push_back(streamEffect);
    processNew.stream.push_back(newStream);
    std::vector<std::string> availableLayout;
    audioEffectManager_->UpdateDuplicateProcessNew(availableLayout, processNew);
    EXPECT_NE(audioEffectManager_, nullptr);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_024.
* @tc.desc  : Test BuildAvailableAEConfig interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_024, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
    audioEffectManager_->BuildAvailableAEConfig();
    EXPECT_NE(audioEffectManager_, nullptr);

    EffectChain effectChain;
    PreStreamScene preStreamScene;
    PostStreamScene postStreamScene;
    audioEffectManager_->oriEffectConfig_.effectChains.push_back(effectChain);
    audioEffectManager_->oriEffectConfig_.preProcess.defaultScenes.push_back(preStreamScene);
    audioEffectManager_->oriEffectConfig_.preProcess.normalScenes.push_back(preStreamScene);
    audioEffectManager_->oriEffectConfig_.postProcess.defaultScenes.push_back(postStreamScene);
    audioEffectManager_->oriEffectConfig_.postProcess.normalScenes.push_back(postStreamScene);
    audioEffectManager_->BuildAvailableAEConfig();
    EXPECT_NE(audioEffectManager_, nullptr);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_025.
* @tc.desc  : Test ConstructEffectChainMode interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_025, TestSize.Level1)
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
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
    audioEffectManager_->ConstructEffectChainMode(mode, sceneType, effectChainMgrParam);
    EXPECT_NE(audioEffectManager_, nullptr);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_026.
* @tc.desc  : Test ConstructDefaultEffectProperty interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_026, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
    EffectChain effectChain;
    std::string chainName = "chainName";
    std::unordered_map<std::string, std::string> effectDefaultProperty;
    audioEffectManager_->supportedEffectConfig_.effectChains.push_back(effectChain);
    audioEffectManager_->ConstructDefaultEffectProperty(chainName, effectDefaultProperty);
    EXPECT_NE(audioEffectManager_, nullptr);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_027.
* @tc.desc  : Test ConstructDefaultEffectProperty interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_027, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
    EffectChain effectChain;
    effectChain.name = "chainName";
    effectChain.apply.push_back("test");
    std::string chainName = "chainName";
    Effect effect;
    audioEffectManager_->availableEffects_.push_back(effect);
    std::unordered_map<std::string, std::string> effectDefaultProperty;
    audioEffectManager_->supportedEffectConfig_.effectChains.push_back(effectChain);
    audioEffectManager_->ConstructDefaultEffectProperty(chainName, effectDefaultProperty);
    EXPECT_NE(audioEffectManager_, nullptr);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_028.
* @tc.desc  : Test ConstructDefaultEffectProperty interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_028, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
    EffectChain effectChain;
    effectChain.name = "chainName";
    effectChain.apply.push_back("test");
    effectChain.apply.push_back("test");
    std::string chainName = "chainName";
    Effect effect;
    effect.name = "test";
    effect.effectProperty.push_back("test");
    audioEffectManager_->availableEffects_.push_back(effect);
    std::unordered_map<std::string, std::string> effectDefaultProperty;
    audioEffectManager_->supportedEffectConfig_.effectChains.push_back(effectChain);
    audioEffectManager_->ConstructDefaultEffectProperty(chainName, effectDefaultProperty);
    EXPECT_NE(audioEffectManager_, nullptr);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_029.
* @tc.desc  : Test ConstructDefaultEffectProperty interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_029, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
    EffectChain effectChain;
    effectChain.name = "chainName";
    effectChain.apply.push_back("test");
    std::string chainName = "chainName";
    Effect effect;
    effect.name = "test";
    audioEffectManager_->availableEffects_.push_back(effect);
    std::unordered_map<std::string, std::string> effectDefaultProperty;
    audioEffectManager_->supportedEffectConfig_.effectChains.push_back(effectChain);
    audioEffectManager_->ConstructDefaultEffectProperty(chainName, effectDefaultProperty);
    EXPECT_NE(audioEffectManager_, nullptr);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_030.
* @tc.desc  : Test ConstructEffectChainManagerParam interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_030, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
    Stream stream1;
    Stream stream2;
    stream1.scene = "test";
    stream1.priority = PRIOR_SCENE;
    stream2.scene = "test";
    stream2.priority = DEFAULT_SCENE;
    EffectChainManagerParam effectChainManagerParam;
    audioEffectManager_->supportedEffectConfig_.postProcessNew.stream.push_back(stream1);
    audioEffectManager_->supportedEffectConfig_.postProcessNew.stream.push_back(stream2);
    audioEffectManager_->ConstructEffectChainManagerParam(effectChainManagerParam);
    EXPECT_NE(audioEffectManager_, nullptr);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_031.
* @tc.desc  : Test ConstructEnhanceChainManagerParam interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_031, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
    Stream stream1;
    Stream stream2;
    stream1.scene = "test";
    stream1.priority = PRIOR_SCENE;
    stream2.scene = "test";
    stream2.priority = DEFAULT_SCENE;
    EffectChainManagerParam effectChainManagerParam;
    audioEffectManager_->supportedEffectConfig_.preProcessNew.stream.push_back(stream1);
    audioEffectManager_->supportedEffectConfig_.preProcessNew.stream.push_back(stream2);
    audioEffectManager_->ConstructEnhanceChainManagerParam(effectChainManagerParam);
    EXPECT_NE(audioEffectManager_, nullptr);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_032.
* @tc.desc  : Test AddSupportedPropertyByDeviceInner interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_032, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
    std::set<std::pair<std::string, std::string>> mergedSet;
    const std::unordered_map<std::string, std::set<std::pair<std::string, std::string>>> device2PropertySet;
    int32_t result = audioEffectManager_->AddSupportedPropertyByDeviceInner(DeviceType::DEVICE_TYPE_MAX,
        mergedSet, device2PropertySet);
    EXPECT_EQ(result, -1);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_033.
* @tc.desc  : Test AddSupportedPropertyByDeviceInner interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_033, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
    std::set<std::pair<std::string, std::string>> mergedSet;
    std::unordered_map<std::string, std::set<std::pair<std::string, std::string>>> device2PropertySet;
    std::set<std::pair<std::string, std::string>> device2Property;
    device2PropertySet.insert({"DEVICE_TYPE_DEFAULT", device2Property});
    int32_t result = audioEffectManager_->AddSupportedPropertyByDeviceInner(DeviceType::DEVICE_TYPE_INVALID,
        mergedSet, device2PropertySet);
    EXPECT_EQ(result, AUDIO_OK);
}

/**
* @tc.name  : Test AudioEffectManager.
* @tc.number: AudioEffectManager_034.
* @tc.desc  : Test AddSupportedPropertyByDeviceInner interfaces.
*/
HWTEST(AudioEffectManagerTest, AudioEffectManager_034, TestSize.Level1)
{
    auto audioEffectManager_ = std::make_shared<AudioEffectManager>();
    std::set<std::pair<std::string, std::string>> mergedSet;
    std::unordered_map<std::string, std::set<std::pair<std::string, std::string>>> device2PropertySet;
    std::set<std::pair<std::string, std::string>> device2Property;
    device2PropertySet.insert({"DEVICE_TYPE_DEFAULT", device2Property});
    int32_t result = audioEffectManager_->AddSupportedPropertyByDeviceInner(DeviceType::DEVICE_TYPE_NONE,
        mergedSet, device2PropertySet);
    EXPECT_EQ(result, AUDIO_OK);
}
} // namespace AudioStandard
} // namespace OHOS
