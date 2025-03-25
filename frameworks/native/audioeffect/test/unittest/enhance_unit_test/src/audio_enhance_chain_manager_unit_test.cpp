/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#undef LOG_TAG
#define LOG_TAG "AudioEnhanceChainManagerUnitTest"

#include "audio_enhance_chain_manager_unit_test.h"
#include <gtest/gtest.h>
#include "audio_enhance_chain_manager.h"
#include "audio_enhance_chain.h"
#include <chrono>
#include <thread>
#include <fstream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "audio_effect.h"
#include "audio_effect_log.h"
#include "audio_errors.h"

#define VALID_SCENEKEY_CODE 4563402752
#define INVALID_SCENEKEY_CODE 23333
#define VALID_BUFFER_SIZE 1000
#define INVALID_BUFFER_SIZE 10000
#define MAX_EXTRA_NUM 3
#define NEED_FALG true

using namespace std;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {

AudioEnhanceChainManager* AudioEnhanceChainManagerUnitTest::manager_ = nullptr;
std::vector<EffectChain> AudioEnhanceChainManagerUnitTest::enhanceChains_;
EffectChainManagerParam AudioEnhanceChainManagerUnitTest::managerParam_;
std::vector<std::shared_ptr<AudioEffectLibEntry>> AudioEnhanceChainManagerUnitTest::enhanceLibraryList_;
AudioEnhanceDeviceAttr AudioEnhanceChainManagerUnitTest::deviceAttr;
constexpr uint32_t CAPTURER_ID_MASK = 0x0000FF00;
constexpr uint32_t SCENE_TYPE_OFFSET = 32;
constexpr uint32_t CAPTURER_ID_OFFSET = 16;
const std::string MAINKEY_DEVICE_STATUS = "device_status";
const std::string SUBKEY_FOLD_STATE = "fold_state";

void AudioEnhanceChainManagerUnitTest::SetUpTestSuite(void)
{
    EffectChain testChain;
    testChain.name = "EFFECTCHAIN_RECORD";
    testChain.apply = {"record"};
    enhanceChains_.emplace_back(testChain);

    managerParam_.maxExtraNum = MAX_EXTRA_NUM;
    managerParam_.defaultSceneName = "SCENE_DEFAULT";
    managerParam_.priorSceneList = {};
    managerParam_.sceneTypeToChainNameMap = {{"SCENE_RECORD_&_ENHANCE_DEFAULT", "EFFECTCHAIN_RECORD"}};
    managerParam_.effectDefaultProperty = {
        {"effect1", "property1"}, {"effect2", "property2"}, {"effect3", "property3"}
    };
    deviceAttr = {
        SAMPLE_RATE_48000, CHANNEL_4, SAMPLE_S16LE,
        NEED_FALG, SAMPLE_RATE_48000, CHANNEL_4, SAMPLE_S16LE,
        NEED_FALG, SAMPLE_RATE_48000, CHANNEL_4, SAMPLE_S16LE
    };
    enhanceLibraryList_ = {};
}

void AudioEnhanceChainManagerUnitTest::TearDownTestSuite(void) {}

void AudioEnhanceChainManagerUnitTest::SetUp(void)
{
    manager_ = AudioEnhanceChainManager::GetInstance();
    manager_->InitAudioEnhanceChainManager(enhanceChains_, managerParam_, enhanceLibraryList_);
}

void AudioEnhanceChainManagerUnitTest::TearDown(void)
{
    manager_ = AudioEnhanceChainManager::GetInstance();
    manager_->ResetInfo();
}

/*
 * tc.name   : Test CreateAudioEnhanceChainDynamic API
 * tc.number : CreateAudioEnhanceChainDynamic_001
 * tc.desc   : Test CreateAudioEnhanceChainDynamic interface(using wrong input case).
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, CreateAudioEnhanceChainDynamic_001, TestSize.Level1)
{
    uint64_t invalidKeyCode = INVALID_SCENEKEY_CODE;
    int32_t result = manager_->CreateAudioEnhanceChainDynamic(invalidKeyCode, deviceAttr);
    EXPECT_EQ(result, ERROR);
}

/*
 * tc.name   : Test CreateAudioEnhanceChainDynamic API
 * tc.number : CreateAudioEnhanceChainDynamic_002
 * tc.desc   : Test CreateAudioEnhanceChainDynamic interface(create enhanceChain success but add handles fail).
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, CreateAudioEnhanceChainDynamic_002, TestSize.Level1)
{
    uint64_t validKeyCode = VALID_SCENEKEY_CODE;
    int32_t result = manager_->CreateAudioEnhanceChainDynamic(validKeyCode, deviceAttr);
    EXPECT_EQ(result, ERROR);
}

/*
 * tc.name   : Test CreateAudioEnhanceChainDynamic API
 * tc.number : CreateAudioEnhanceChainDynamic_003
 * tc.desc   : Test CreateAudioEnhanceChainDynamic when no algo on audio_framework.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, CreateAudioEnhanceChainDynamic_003, TestSize.Level1)
{
    manager_->ResetInfo();
    uint64_t invalidKeyCode = INVALID_SCENEKEY_CODE;
    int32_t result = manager_->CreateAudioEnhanceChainDynamic(invalidKeyCode, deviceAttr);
    EXPECT_EQ(result, ERROR);
}

/*
 * tc.name   : Test CreateAudioEnhanceChainDynamic API
 * tc.number : CreateAudioEnhanceChainDynamic_004
 * tc.desc   : Test CreateAudioEnhanceChainDynamic interface(using correct input case).
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, CreateAudioEnhanceChainDynamic_004, TestSize.Level1)
{
    uint64_t validKeyCode = VALID_SCENEKEY_CODE;
    std::string scene = "SCENE_RECORD";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain =
        std::make_shared<AudioEnhanceChain>(scene, algoParam, deviceAttr, defaultFlag);
    manager_->sceneTypeToEnhanceChainMap_.insert_or_assign(validKeyCode, audioEnhanceChain);
    manager_->sceneTypeToEnhanceChainCountMap_.insert_or_assign(validKeyCode, 1);
    int32_t result = manager_->CreateAudioEnhanceChainDynamic(validKeyCode, deviceAttr);
    EXPECT_EQ(result, ERROR);
}

/*
 * tc.name   : Test ExistAudioEnhanceChain API
 * tc.number : ExistAudioEnhanceChain_001
 * tc.desc   : Test ExistAudioEnhanceChain interface with a nonexistent ehanceChain.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ExistAudioEnhanceChain_001, TestSize.Level1)
{
    uint64_t validKeyCode = VALID_SCENEKEY_CODE;
    uint64_t invalidKeyCode = INVALID_SCENEKEY_CODE;

    manager_->CreateAudioEnhanceChainDynamic(validKeyCode, deviceAttr);
    bool result = manager_->ExistAudioEnhanceChain(invalidKeyCode);
    EXPECT_EQ(result, false);
}

/*
 * tc.name   : Test AudioEnhanceChainGetAlgoConfig API
 * tc.number : AudioEnhanceChainGetAlgoConfig_001
 * tc.desc   : Test AudioEnhanceChainGetAlgoConfig interface with an invalid scene key code.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, AudioEnhanceChainGetAlgoConfig_001, TestSize.Level1)
{
    AudioBufferConfig micConfig = {};
    AudioBufferConfig ecConfig = {};
    AudioBufferConfig micRefConfig = {};
    uint64_t validKeyCode = VALID_SCENEKEY_CODE;
    uint64_t invalidKeyCode = INVALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validKeyCode, deviceAttr);

    int32_t result = manager_->AudioEnhanceChainGetAlgoConfig(invalidKeyCode, micConfig, ecConfig, micRefConfig);
    EXPECT_EQ(result, ERROR);
}

/*
 * tc.name   : Test ApplyAudioEnhanceChain API
 * tc.number : ApplyAudioEnhanceChain_003
 * tc.desc   : Test ApplyAudioEnhanceChain interface when the enhanceChain has no handles.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ApplyAudioEnhanceChain_003, TestSize.Level1)
{
    uint64_t validSceneKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validSceneKeyCode, deviceAttr);
    manager_->InitEnhanceBuffer();

    uint32_t bufferSize = VALID_BUFFER_SIZE;
    std::vector<uint8_t> dummyData(bufferSize, 0xAA);
    manager_->CopyToEnhanceBuffer(dummyData.data(), dummyData.size());
    int32_t result = manager_->ApplyAudioEnhanceChain(validSceneKeyCode, dummyData.size());
    EXPECT_EQ(result, ERROR);
}

/*
 * tc.name   : Test SetOutputDevice API
 * tc.number : SetOutputDevice_001
 * tc.desc   : Ensures the function sets a new output device correctly and returns success.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetOutputDevice_001, TestSize.Level1)
{
    uint32_t renderId = 0;
    DeviceType newDeviceType = DEVICE_TYPE_SPEAKER;
    int32_t result = manager_->SetOutputDevice(renderId, newDeviceType);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test SetOutputDevice API
 * tc.number : SetOutputDevice_002
 * tc.desc   : Ensures the function recognizes the same output device setting and returns success without redundancy.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetOutputDevice_002, TestSize.Level1)
{
    uint32_t renderId = 0;
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;

    manager_->SetOutputDevice(renderId, deviceType);
    int32_t result = manager_->SetOutputDevice(renderId, deviceType);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test SetVolumeInfo API
 * tc.number : SetVolumeInfo_001
 * tc.desc   : Ensures the function sets volume information correctly for music stream type.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetVolumeInfo_001, TestSize.Level1)
{
    AudioVolumeType volumeType = STREAM_MUSIC;
    float systemVolume = 0.75f;
    int32_t result = manager_->SetVolumeInfo(volumeType, systemVolume);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test SetVolumeInfo API
 * tc.number : SetVolumeInfo_002
 * tc.desc   : Ensures the function sets volume information correctly for alarm stream type.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetVolumeInfo_002, TestSize.Level1)
{
    AudioVolumeType volumeType = STREAM_ALARM;
    float systemVolume = 0.5f;
    int32_t result = manager_->SetVolumeInfo(volumeType, systemVolume);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test SetVolumeInfo API
 * tc.number : SetVolumeInfo_003
 * tc.desc   : Ensures the function sets volume information correctly for system sound stream type.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetVolumeInfo_003, TestSize.Level1)
{
    AudioVolumeType volumeType = STREAM_SYSTEM;
    float systemVolume = 1.0f;
    int32_t result = manager_->SetVolumeInfo(volumeType, systemVolume);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test SetVolumeInfo API
 * tc.number : SetVolumeInfo_004
 * tc.desc   : Ensures the function sets volume information correctly for system sound stream type.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetVolumeInfo_004, TestSize.Level1)
{
    AudioVolumeType volumeType = STREAM_SYSTEM;
    float systemVolume = 1.0f;
    uint64_t validSceneKeyCode = VALID_SCENEKEY_CODE;
    std::string scene = "SCENE_RECORD";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>(scene,
        algoParam, deviceAttr, defaultFlag);
    manager_->sceneTypeAndModeToEnhanceChainNameMap_[scene] = "test";
    manager_->sceneTypeToEnhanceChainMap_[validSceneKeyCode] = audioEnhanceChain;
    int32_t result = manager_->SetVolumeInfo(volumeType, systemVolume);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test SetAudioEnhanceProperty API
 * tc.number : SetAudioEnhanceProperty_001
 * tc.desc   : Ensures the function handles an empty property array correctly.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetAudioEnhanceProperty_001, TestSize.Level1)
{
    uint64_t validSceneKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validSceneKeyCode, deviceAttr);

    AudioEnhancePropertyArray propertyArray;
    int32_t result = manager_->SetAudioEnhanceProperty(propertyArray);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test SetAudioEnhanceProperty API
 * tc.number : SetAudioEnhanceProperty_002
 * tc.desc   : Tests setting a property with valid enhanceClass but invalid enhanceProp.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetAudioEnhanceProperty_002, TestSize.Level1)
{
    uint64_t validSceneKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validSceneKeyCode, deviceAttr);

    AudioEnhancePropertyArray propertyArray;
    propertyArray.property.push_back({"record", "123"});
    int32_t result = manager_->SetAudioEnhanceProperty(propertyArray);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test SetAudioEnhanceProperty API
 * tc.number : SetAudioEnhanceProperty_003
 * tc.desc   : Tests setting a property with invalid enhanceClass and valid enhanceProp.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetAudioEnhanceProperty_003, TestSize.Level1)
{
    uint64_t validSceneKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validSceneKeyCode, deviceAttr);

    AudioEnhancePropertyArray propertyArray;
    propertyArray.property.push_back({"invalidEffect", "property1"});
    int32_t result = manager_->SetAudioEnhanceProperty(propertyArray);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test SetAudioEnhanceProperty API
 * tc.number : SetAudioEnhanceProperty_004
 * tc.desc   : Tests setting a property with both valid enhanceClass and enhanceProp.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetAudioEnhanceProperty_004, TestSize.Level1)
{
    uint64_t validSceneKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validSceneKeyCode, deviceAttr);

    AudioEnhancePropertyArray propertyArray;
    propertyArray.property.push_back({"effect1", "property1"});
    int32_t result = manager_->SetAudioEnhanceProperty(propertyArray);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test SetAudioEnhanceProperty API
 * tc.number : SetAudioEnhanceProperty_005
 * tc.desc   : Ensures the function handles an empty AudioEffectPropertyArrayV3  correctly.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetAudioEnhanceProperty_005, TestSize.Level1)
{
    uint64_t validSceneKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validSceneKeyCode, deviceAttr);

    AudioEffectPropertyArrayV3 propertyArray;
    int32_t result = manager_->SetAudioEnhanceProperty(propertyArray);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test SetAudioEnhanceProperty API
 * tc.number : SetAudioEnhanceProperty_006
 * tc.desc   : Tests setting a AudioEffectPropertyV3 with invalid enhanceClass and valid enhanceProp.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetAudioEnhanceProperty_006, TestSize.Level1)
{
    uint64_t validSceneKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validSceneKeyCode, deviceAttr);

    AudioEffectPropertyArrayV3 propertyArray;
    propertyArray.property.push_back({"invalidEffect", "property1"});
    int32_t result = manager_->SetAudioEnhanceProperty(propertyArray);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test SetAudioEnhanceProperty API
 * tc.number : SetAudioEnhanceProperty_007
 * tc.desc   : Tests setting a AudioEffectPropertyV3 with both valid enhanceClass and enhanceProp.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetAudioEnhanceProperty_007, TestSize.Level1)
{
    uint64_t validSceneKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validSceneKeyCode, deviceAttr);

    AudioEffectPropertyArrayV3 propertyArray;
    propertyArray.property.push_back({"effect1", "property1"});
    int32_t result = manager_->SetAudioEnhanceProperty(propertyArray);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test GetAudioEnhanceProperty API
 * tc.number : GetAudioEnhanceProperty_001
 * tc.desc   : Ensures the function correctly retrieves an empty property list if no properties are set.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, GetAudioEnhanceProperty_001, TestSize.Level1)
{
    AudioEnhancePropertyArray propertyArray;
    int32_t result = manager_->GetAudioEnhanceProperty(propertyArray);
    EXPECT_EQ(propertyArray.property.size(), 3);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test GetAudioEnhanceProperty API
 * tc.number : GetAudioEnhanceProperty_002
 * tc.desc   : Tests retrieving a property list when properties are set.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, GetAudioEnhanceProperty_002, TestSize.Level1)
{
    AudioEnhancePropertyArray propertiesToSet;
    propertiesToSet.property.push_back({"effect1", "property4"});
    propertiesToSet.property.push_back({"effect2", "property5"});

    manager_->SetAudioEnhanceProperty(propertiesToSet);
    AudioEnhancePropertyArray propertyArray;
    int32_t result = manager_->GetAudioEnhanceProperty(propertyArray);
    EXPECT_EQ(propertyArray.property.size(), 3);
    EXPECT_EQ(propertyArray.property[0].enhanceClass, "effect1");
    EXPECT_EQ(propertyArray.property[0].enhanceProp, "property4");
    EXPECT_EQ(propertyArray.property[1].enhanceClass, "effect2");
    EXPECT_EQ(propertyArray.property[1].enhanceProp, "property5");
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test GetAudioEnhanceProperty API
 * tc.number : GetAudioEnhanceProperty_003
 * tc.desc   : Ensures the function correctly retrieves an empty property list if no properties are set.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, GetAudioEnhanceProperty_003, TestSize.Level1)
{
    AudioEffectPropertyArrayV3 propertyArray;
    int32_t result = manager_->GetAudioEnhanceProperty(propertyArray);
    EXPECT_EQ(propertyArray.property.size(), 3);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test GetAudioEnhanceProperty API
 * tc.number : GetAudioEnhanceProperty_004
 * tc.desc   : Tests retrieving a property list when properties are set.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, GetAudioEnhanceProperty_004, TestSize.Level1)
{
    AudioEffectPropertyArrayV3 propertiesToSet;
    propertiesToSet.property.push_back({"effect1", "property4"});
    propertiesToSet.property.push_back({"effect2", "property5"});

    manager_->SetAudioEnhanceProperty(propertiesToSet);
    AudioEffectPropertyArrayV3 propertyArray;
    int32_t result = manager_->GetAudioEnhanceProperty(propertyArray);
    EXPECT_EQ(propertyArray.property.size(), 3);
    EXPECT_EQ(propertyArray.property[0].name, "effect1");
    EXPECT_EQ(propertyArray.property[0].category, "property4");
    EXPECT_EQ(propertyArray.property[1].name, "effect2");
    EXPECT_EQ(propertyArray.property[1].category, "property5");
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: ConstructDeviceEnhances_001
 * @tc.desc: Test ConstructDeviceEnhances when enhance is not found in enhancePropertyMap_, enter if branch.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ConstructDeviceEnhances_001, TestSize.Level1)
{
    manager_->sceneTypeAndModeToEnhanceChainNameMap_.clear();
    manager_->sceneTypeAndModeToEnhanceChainNameMap_["SCENE_RECORD_&_ENHANCE_DEFAULT"] = "EFFECTCHAIN_RECORD";
    manager_->enhanceChainToEnhancesMap_.clear();
    manager_->enhanceChainToEnhancesMap_["EFFECTCHAIN_RECORD"] = {"effect1", "effect2"};
    manager_->enhancePropertyMap_.clear();
    manager_->enhancePropertyMap_["effect1"] = "property1";
    manager_->ConstructDeviceEnhances();
    EXPECT_TRUE(manager_->withDeviceEnhances_.count("effect2") == 0);
}

/**
 * @tc.name: ConstructDeviceEnhances_002
 * @tc.desc: Test ConstructDeviceEnhances when enhance is found in enhancePropertyMap_, do not enter if branch.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ConstructDeviceEnhances_002, TestSize.Level1)
{
    manager_->sceneTypeAndModeToEnhanceChainNameMap_.clear();
    manager_->sceneTypeAndModeToEnhanceChainNameMap_["SCENE_RECORD_&_ENHANCE_DEFAULT"] = "EFFECTCHAIN_RECORD";
    manager_->enhanceChainToEnhancesMap_.clear();
    manager_->enhanceChainToEnhancesMap_["EFFECTCHAIN_RECORD"] = {"effect1", "effect2"};
    manager_->enhancePropertyMap_.clear();
    manager_->enhancePropertyMap_["effect1"] = "property1";
    manager_->enhancePropertyMap_["effect2"] = "property2";
    manager_->ConstructDeviceEnhances();
    EXPECT_TRUE(manager_->withDeviceEnhances_.count("effect2") == 0);
}

/**
 * @tc.name: UpdateEnhancePropertyMapFromDb_001
 * @tc.desc: Test UpdateEnhancePropertyMapFromDb when deviceTypeName is empty, enter the first if branch.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, UpdateEnhancePropertyMapFromDb_001, TestSize.Level1)
{
    manager_->enhancePropertyMap_.clear();
    manager_->enhancePropertyMap_["effect1"] = "property1";
    manager_->withDeviceEnhances_.clear();
    manager_->UpdateEnhancePropertyMapFromDb(static_cast<DeviceType>(999));
    EXPECT_EQ(manager_->enhancePropertyMap_.size(), 1U);
}

/**
 * @tc.name: UpdateEnhancePropertyMapFromDb_002
 * @tc.desc: Test UpdateEnhancePropertyMapFromDb when enhance is not in withDeviceEnhances_,
 *  enter the else branch of the second if.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, UpdateEnhancePropertyMapFromDb_002, TestSize.Level1)
{
    manager_->enhancePropertyMap_.clear();
    manager_->enhancePropertyMap_["effect1"] = "property1";
    manager_->withDeviceEnhances_.clear();
    manager_->withDeviceEnhances_.insert("effect2");
    manager_->UpdateEnhancePropertyMapFromDb(DEVICE_TYPE_MIC);
    EXPECT_EQ(manager_->enhancePropertyMap_.size(), 1U);
}

/**
 * @tc.name: UpdateEnhancePropertyMapFromDb_003
 * @tc.desc: Test UpdateEnhancePropertyMapFromDb when settingProvider.GetStringValue returns SUCCESS,
 *  enter the third if branch.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, UpdateEnhancePropertyMapFromDb_003, TestSize.Level1)
{
    manager_->enhancePropertyMap_.clear();
    manager_->enhancePropertyMap_["effect1"] = "property1";
    manager_->withDeviceEnhances_.clear();
    manager_->withDeviceEnhances_.insert("effect1");
    manager_->UpdateEnhancePropertyMapFromDb(DEVICE_TYPE_MIC);
    EXPECT_EQ(manager_->enhancePropertyMap_.size(), 1U);
}

/**
 * @tc.name: UpdateEnhancePropertyMapFromDb_004
 * @tc.desc: Test UpdateEnhancePropertyMapFromDb when settingProvider.GetStringValue returns failure and
 *  settingProvider.PutStringValue returns SUCCESS, enter the else branch of the third if but not the inner if.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, UpdateEnhancePropertyMapFromDb_004, TestSize.Level1)
{
    manager_->enhancePropertyMap_.clear();
    manager_->enhancePropertyMap_["effect1"] = "property1";
    manager_->defaultPropertyMap_.clear();
    manager_->defaultPropertyMap_["effect1"] = "default_property";
    manager_->withDeviceEnhances_.clear();
    manager_->withDeviceEnhances_.insert("effect1");
    manager_->UpdateEnhancePropertyMapFromDb(DEVICE_TYPE_MIC);
    EXPECT_EQ(manager_->enhancePropertyMap_.size(), 1U);
}

/**
 * @tc.name: InitEnhanceBuffer_001
 * @tc.desc: Test InitEnhanceBuffer when enhanceBuffer_ is nullptr and there are valid chains.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, InitEnhanceBuffer_001, TestSize.Level1)
{
    manager_->sceneTypeToEnhanceChainMap_.clear();
    auto chain = std::make_shared<AudioEnhanceChain>("scene1", AudioEnhanceParamAdapter(),
        AudioEnhanceDeviceAttr(), false);
    manager_->sceneTypeToEnhanceChainMap_[1] = chain;
    manager_->enhanceBuffer_ = nullptr;
    int32_t result = manager_->InitEnhanceBuffer();
    EXPECT_EQ(result, SUCCESS);
    EXPECT_NE(manager_->enhanceBuffer_, nullptr);
}

/**
 * @tc.name: InitEnhanceBuffer_002
 * @tc.desc: Test InitEnhanceBuffer when enhanceBuffer_ is not nullptr and len, lenEc,
 *  lenMicRef are not larger than current buffer sizes.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, InitEnhanceBuffer_002, TestSize.Level1)
{
    manager_->sceneTypeToEnhanceChainMap_.clear();
    auto chain = std::make_shared<AudioEnhanceChain>("scene1", AudioEnhanceParamAdapter(),
        AudioEnhanceDeviceAttr(), false);
    manager_->sceneTypeToEnhanceChainMap_[1] = chain;
    manager_->enhanceBuffer_ = std::make_unique<EnhanceBuffer>();
    manager_->enhanceBuffer_->micBufferIn.resize(200);
    manager_->enhanceBuffer_->micBufferOut.resize(200);
    manager_->enhanceBuffer_->ecBuffer.resize(200);
    manager_->enhanceBuffer_->micRefBuffer.resize(200);
    int32_t result = manager_->InitEnhanceBuffer();
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: InitEnhanceBuffer_003
 * @tc.desc: Test InitEnhanceBuffer when enhanceBuffer_ is not nullptr and len, lenEc,
 *  lenMicRef are partially larger than current buffer sizes.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, InitEnhanceBuffer_003, TestSize.Level1)
{
    manager_->sceneTypeToEnhanceChainMap_.clear();
    auto chain = std::make_shared<AudioEnhanceChain>("scene1", AudioEnhanceParamAdapter(),
        AudioEnhanceDeviceAttr(), false);
    manager_->sceneTypeToEnhanceChainMap_[1] = chain;

    manager_->enhanceBuffer_ = std::make_unique<EnhanceBuffer>();
    manager_->enhanceBuffer_->micBufferIn.resize(100);
    manager_->enhanceBuffer_->micBufferOut.resize(100);
    manager_->enhanceBuffer_->ecBuffer.resize(100);
    manager_->enhanceBuffer_->micRefBuffer.resize(100);
    int32_t result = manager_->InitEnhanceBuffer();
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: InitEnhanceBuffer_004
 * @tc.desc: Test InitEnhanceBuffer when enhanceBuffer_ is not nullptr and len, lenEc,
 *  lenMicRef are all larger than current buffer sizes.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, InitEnhanceBuffer_004, TestSize.Level1)
{
    manager_->sceneTypeToEnhanceChainMap_.clear();
    auto chain = std::make_shared<AudioEnhanceChain>("scene1", AudioEnhanceParamAdapter(),
        AudioEnhanceDeviceAttr(), false);
    manager_->sceneTypeToEnhanceChainMap_[1] = chain;
    manager_->enhanceBuffer_ = std::make_unique<EnhanceBuffer>();
    manager_->enhanceBuffer_->micBufferIn.resize(50);
    manager_->enhanceBuffer_->micBufferOut.resize(50);
    manager_->enhanceBuffer_->ecBuffer.resize(50);
    manager_->enhanceBuffer_->micRefBuffer.resize(50);
    int32_t result = manager_->InitEnhanceBuffer();
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: ParseSceneKeyCode_001
 * @tc.desc: Test ParseSceneKeyCode when capturerDevice is valid and supported.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ParseSceneKeyCode_001, TestSize.Level1)
{
    uint64_t sceneKeyCode = (static_cast<uint64_t>(SCENE_RECORD) << SCENE_TYPE_OFFSET) |
                            (static_cast<uint64_t>(1) << CAPTURER_ID_OFFSET) |
                            static_cast<uint64_t>(1);
    std::string sceneType, capturerDeviceStr, rendererDeviceStr;
    uint32_t captureId = 1;
    uint32_t renderId = 1;
    DeviceType capturerDevice = DEVICE_TYPE_MIC;
    DeviceType rendererDevice = DEVICE_TYPE_SPEAKER;
    manager_->captureIdToDeviceMap_[captureId] = capturerDevice;
    manager_->renderIdToDeviceMap_[renderId] = rendererDevice;
    int32_t result = manager_->ParseSceneKeyCode(sceneKeyCode, sceneType, capturerDeviceStr, rendererDeviceStr);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(capturerDeviceStr, "DEVICE_TYPE_MIC");
    EXPECT_EQ(rendererDeviceStr, "DEVICE_TYPE_SPEAKER");
}

/**
 * @tc.name: ParseSceneKeyCode_002
 * @tc.desc: Test ParseSceneKeyCode when capturerDevice is DEVICE_TYPE_INVALID.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ParseSceneKeyCode_002, TestSize.Level1)
{
    uint64_t sceneKeyCode = (static_cast<uint64_t>(SCENE_RECORD) << SCENE_TYPE_OFFSET) |
                            (static_cast<uint64_t>(1) << CAPTURER_ID_OFFSET) |
                            static_cast<uint64_t>(1);
    std::string sceneType, capturerDeviceStr, rendererDeviceStr;
    uint32_t captureId = 1;
    uint32_t renderId = 1;
    DeviceType capturerDevice = DEVICE_TYPE_INVALID;
    DeviceType rendererDevice = DEVICE_TYPE_SPEAKER;
    manager_->captureIdToDeviceMap_[captureId] = capturerDevice;
    manager_->renderIdToDeviceMap_[renderId] = rendererDevice;
    int32_t result = manager_->ParseSceneKeyCode(sceneKeyCode, sceneType, capturerDeviceStr, rendererDeviceStr);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(capturerDeviceStr, "DEVICE_TYPE_MIC");
    EXPECT_EQ(rendererDeviceStr, "DEVICE_TYPE_SPEAKER");
}

/**
 * @tc.name: ParseSceneKeyCode_003
 * @tc.desc: Test ParseSceneKeyCode when capturerDevice is DEVICE_TYPE_NONE.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ParseSceneKeyCode_003, TestSize.Level1)
{
    uint64_t sceneKeyCode = (static_cast<uint64_t>(SCENE_RECORD) << SCENE_TYPE_OFFSET) |
                            (static_cast<uint64_t>(1) << CAPTURER_ID_OFFSET) |
                            static_cast<uint64_t>(1);
    std::string sceneType, capturerDeviceStr, rendererDeviceStr;
    uint32_t captureId = 1;
    uint32_t renderId = 1;
    DeviceType capturerDevice = DEVICE_TYPE_NONE;
    DeviceType rendererDevice = DEVICE_TYPE_SPEAKER;
    manager_->captureIdToDeviceMap_[captureId] = capturerDevice;
    manager_->renderIdToDeviceMap_[renderId] = rendererDevice;
    int32_t result = manager_->ParseSceneKeyCode(sceneKeyCode, sceneType, capturerDeviceStr, rendererDeviceStr);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(capturerDeviceStr, "DEVICE_TYPE_MIC");
    EXPECT_EQ(rendererDeviceStr, "DEVICE_TYPE_SPEAKER");
}

/**
 * @tc.name: ParseSceneKeyCode_004
 * @tc.desc: Test ParseSceneKeyCode when capturerDevice is not in supportDeviceType.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ParseSceneKeyCode_004, TestSize.Level1)
{
    uint64_t sceneKeyCode = (static_cast<uint64_t>(SCENE_RECORD) << SCENE_TYPE_OFFSET) |
                            (static_cast<uint64_t>(1) << CAPTURER_ID_OFFSET) |
                            static_cast<uint64_t>(1);
    std::string sceneType, capturerDeviceStr, rendererDeviceStr;
    uint32_t captureId = 1;
    uint32_t renderId = 1;
    DeviceType capturerDevice = DEVICE_TYPE_USB_HEADSET;
    DeviceType rendererDevice = DEVICE_TYPE_SPEAKER;
    manager_->captureIdToDeviceMap_[captureId] = capturerDevice;
    manager_->renderIdToDeviceMap_[renderId] = rendererDevice;
    int32_t result = manager_->ParseSceneKeyCode(sceneKeyCode, sceneType, capturerDeviceStr, rendererDeviceStr);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: CreateAudioEnhanceChainDynamic_005
 * @tc.desc: Test CreateAudioEnhanceChainDynamic when sceneTypeAndModeToEnhanceChainNameMap_.size() == 0.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, CreateAudioEnhanceChainDynamic_005, TestSize.Level1)
{
    manager_->sceneTypeAndModeToEnhanceChainNameMap_.clear();
    uint64_t sceneKeyCode = 12345;
    AudioEnhanceDeviceAttr deviceAttr;
    int32_t result = manager_->CreateAudioEnhanceChainDynamic(sceneKeyCode, deviceAttr);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name: CreateAudioEnhanceChainDynamic_006
 * @tc.desc: Test CreateAudioEnhanceChainDynamic when entering the outer if branch and the inner if branch.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, CreateAudioEnhanceChainDynamic_006, TestSize.Level1)
{
    uint64_t sceneKeyCode = 12345;
    AudioEnhanceDeviceAttr deviceAttr;
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>("scene1",
        AudioEnhanceParamAdapter(), AudioEnhanceDeviceAttr(), false);
    audioEnhanceChain->standByEnhanceHandles_.clear();
    manager_->sceneTypeToEnhanceChainMap_[sceneKeyCode] = audioEnhanceChain;
    int32_t result = manager_->CreateAudioEnhanceChainDynamic(sceneKeyCode, deviceAttr);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name: CreateAudioEnhanceChainDynamic_007
 * @tc.desc: Test CreateAudioEnhanceChainDynamic when entering the outer if branch but not the inner if branch.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, CreateAudioEnhanceChainDynamic_007, TestSize.Level1)
{
    uint64_t sceneKeyCode = 12345;
    AudioEnhanceDeviceAttr deviceAttr;
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>("scene1",
        AudioEnhanceParamAdapter(), AudioEnhanceDeviceAttr(), false);
    audioEnhanceChain->standByEnhanceHandles_.push_back(nullptr);
    manager_->sceneTypeToEnhanceChainMap_[sceneKeyCode] = audioEnhanceChain;
    int32_t result = manager_->CreateAudioEnhanceChainDynamic(sceneKeyCode, deviceAttr);
    EXPECT_EQ(result, audioEnhanceChain->IsDefaultChain());
}

/**
 * @tc.name: CreateAudioEnhanceChainDynamic_008
 * @tc.desc: Test CreateAudioEnhanceChainDynamic when entering the if (!createFlag) branch.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, CreateAudioEnhanceChainDynamic_008, TestSize.Level1)
{
    uint64_t sceneKeyCode = 12345;
    AudioEnhanceDeviceAttr deviceAttr;
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>("scene1",
        AudioEnhanceParamAdapter(), AudioEnhanceDeviceAttr(), false);
    uint32_t captureId = (sceneKeyCode & CAPTURER_ID_MASK) >> CAPTURER_ID_OFFSET;
    manager_->captureId2DefaultChainCount_[captureId] = 1;
    manager_->captureId2DefaultChain_[captureId] = audioEnhanceChain;
    int32_t result = manager_->CreateAudioEnhanceChainDynamic(sceneKeyCode, deviceAttr);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name: GetEnhanceChainNameBySceneCode_001
 * @tc.desc: Test GetEnhanceChainNameBySceneCode when ParseSceneKeyCode fails.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, GetEnhanceChainNameBySceneCode_001, TestSize.Level1)
{
    uint64_t sceneKeyCode = 12345;
    bool defaultFlag = false;
    std::string result = manager_->GetEnhanceChainNameBySceneCode(sceneKeyCode, defaultFlag);
    EXPECT_EQ(result, "ENHANCE_NONE");
}

/**
 * @tc.name: GetEnhanceChainNameBySceneCode_002
 * @tc.desc: Test GetEnhanceChainNameBySceneCode when defaultFlag is true.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, GetEnhanceChainNameBySceneCode_002, TestSize.Level1)
{
    uint64_t sceneKeyCode = (static_cast<uint64_t>(SCENE_RECORD) << SCENE_TYPE_OFFSET) |
                            (static_cast<uint64_t>(1) << CAPTURER_ID_OFFSET) |
                            static_cast<uint64_t>(1);
    bool defaultFlag = true;

    manager_->defaultScene_ = "DEFAULT_SCENE";
    uint32_t captureId = 1;
    uint32_t renderId = 1;
    DeviceType capturerDevice = DEVICE_TYPE_MIC;
    DeviceType rendererDevice = DEVICE_TYPE_SPEAKER;
    manager_->captureIdToDeviceMap_[captureId] = capturerDevice;
    manager_->renderIdToDeviceMap_[renderId] = rendererDevice;

    manager_->sceneTypeAndModeToEnhanceChainNameMap_.clear();
    manager_->sceneTypeAndModeToEnhanceChainNameMap_["DEFAULT_SCENE_&_ENHANCE_DEFAULT"] = "CHAIN_1";
    manager_->enhanceChainToEnhancesMap_.clear();
    manager_->enhanceChainToEnhancesMap_["CHAIN_1"] = {"effect1", "effect2"};
    std::string result = manager_->GetEnhanceChainNameBySceneCode(sceneKeyCode, defaultFlag);
    EXPECT_EQ(result, "CHAIN_1");
}

/**
 * @tc.name: GetEnhanceChainNameBySceneCode_003
 * @tc.desc: Test GetEnhanceChainNameBySceneCode when the first enhanceChainKey is not found.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, GetEnhanceChainNameBySceneCode_003, TestSize.Level1)
{
    uint64_t sceneKeyCode = (static_cast<uint64_t>(SCENE_RECORD) << SCENE_TYPE_OFFSET) |
                            (static_cast<uint64_t>(1) << CAPTURER_ID_OFFSET) |
                            static_cast<uint64_t>(1);
    bool defaultFlag = false;

    uint32_t captureId = 1;
    uint32_t renderId = 1;
    DeviceType capturerDevice = DEVICE_TYPE_MIC;
    DeviceType rendererDevice = DEVICE_TYPE_SPEAKER;
    manager_->captureIdToDeviceMap_[captureId] = capturerDevice;
    manager_->renderIdToDeviceMap_[renderId] = rendererDevice;
    manager_->sceneTypeAndModeToEnhanceChainNameMap_.clear();
    manager_->sceneTypeAndModeToEnhanceChainNameMap_["SCENE_RECORD_&_ENHANCE_DEFAULT"] = "CHAIN_1";
    std::string result = manager_->GetEnhanceChainNameBySceneCode(sceneKeyCode, defaultFlag);
    EXPECT_EQ(result, "ENHANCE_NONE");
}

/**
 * @tc.name: GetEnhanceChainNameBySceneCode_004
 * @tc.desc: Test GetEnhanceChainNameBySceneCode when the enhanceChainKey is not found.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, GetEnhanceChainNameBySceneCode_004, TestSize.Level1)
{
    uint64_t sceneKeyCode = (static_cast<uint64_t>(SCENE_RECORD) << SCENE_TYPE_OFFSET) |
                            (static_cast<uint64_t>(1) << CAPTURER_ID_OFFSET) |
                            static_cast<uint64_t>(1);
    bool defaultFlag = false;
    uint32_t captureId = 1;
    uint32_t renderId = 1;
    DeviceType capturerDevice = DEVICE_TYPE_MIC;
    DeviceType rendererDevice = DEVICE_TYPE_SPEAKER;
    manager_->captureIdToDeviceMap_[captureId] = capturerDevice;
    manager_->renderIdToDeviceMap_[renderId] = rendererDevice;

    manager_->sceneTypeAndModeToEnhanceChainNameMap_.clear();
    manager_->sceneTypeAndModeToEnhanceChainNameMap_["SCENE_RECORD_&_ENHANCE_DEFAULT_&_DEVICE_1"] = "CHAIN_1";
    std::string result = manager_->GetEnhanceChainNameBySceneCode(sceneKeyCode, defaultFlag);
    EXPECT_EQ(result, "ENHANCE_NONE");
}

/**
 * @tc.name: GetEnhanceChainNameBySceneCode_005
 * @tc.desc: Test GetEnhanceChainNameBySceneCode when the enhanceChainName is found but not in
 *  enhanceChainToEnhancesMap_.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, GetEnhanceChainNameBySceneCode_005, TestSize.Level1)
{
    uint64_t sceneKeyCode = (static_cast<uint64_t>(SCENE_RECORD) << SCENE_TYPE_OFFSET) |
                            (static_cast<uint64_t>(1) << CAPTURER_ID_OFFSET) |
                            static_cast<uint64_t>(1);
    bool defaultFlag = false;
    uint32_t captureId = 1;
    uint32_t renderId = 1;
    DeviceType capturerDevice = DEVICE_TYPE_MIC;
    DeviceType rendererDevice = DEVICE_TYPE_SPEAKER;
    manager_->captureIdToDeviceMap_[captureId] = capturerDevice;
    manager_->renderIdToDeviceMap_[renderId] = rendererDevice;
    manager_->sceneTypeAndModeToEnhanceChainNameMap_.clear();
    manager_->sceneTypeAndModeToEnhanceChainNameMap_["SCENE_RECORD_&_ENHANCE_DEFAULT"] = "CHAIN_1";
    manager_->enhanceChainToEnhancesMap_.clear();
    std::string result = manager_->GetEnhanceChainNameBySceneCode(sceneKeyCode, defaultFlag);
    EXPECT_EQ(result, "ENHANCE_NONE");
}

/**
 * @tc.name: GetEnhanceChainNameBySceneCode_006
 * @tc.desc: Test GetEnhanceChainNameBySceneCode when the enhanceChainName is found.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, GetEnhanceChainNameBySceneCode_006, TestSize.Level1)
{
    uint64_t sceneKeyCode = (static_cast<uint64_t>(SCENE_RECORD) << SCENE_TYPE_OFFSET) |
                            (static_cast<uint64_t>(1) << CAPTURER_ID_OFFSET) |
                            static_cast<uint64_t>(1);
    bool defaultFlag = false;
    uint32_t captureId = 1;
    uint32_t renderId = 1;
    DeviceType capturerDevice = DEVICE_TYPE_MIC;
    DeviceType rendererDevice = DEVICE_TYPE_SPEAKER;
    manager_->captureIdToDeviceMap_[captureId] = capturerDevice;
    manager_->renderIdToDeviceMap_[renderId] = rendererDevice;
    manager_->sceneTypeAndModeToEnhanceChainNameMap_.clear();
    manager_->sceneTypeAndModeToEnhanceChainNameMap_["SCENE_RECORD_&_ENHANCE_DEFAULT"] = "CHAIN_1";
    manager_->enhanceChainToEnhancesMap_.clear();
    manager_->enhanceChainToEnhancesMap_["CHAIN_1"] = {"effect1", "effect2"};
    std::string result = manager_->GetEnhanceChainNameBySceneCode(sceneKeyCode, defaultFlag);
    EXPECT_EQ(result, "CHAIN_1");
}

/**
 * @tc.name: GetDeviceNameByCaptureId_001
 * @tc.desc: Test GetDeviceNameByCaptureId when captureId exists in captureIdToDeviceNameMap_.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, GetDeviceNameByCaptureId_001, TestSize.Level1)
{
    uint32_t captureId = 1;
    std::string deviceName;
    manager_->captureIdToDeviceNameMap_[captureId] = "DEVICE_NAME_1";
    manager_->GetDeviceNameByCaptureId(captureId, deviceName);
    EXPECT_EQ(deviceName, "DEVICE_NAME_1");
}

/**
 * @tc.name: GetDeviceNameByCaptureId_002
 * @tc.desc: Test GetDeviceNameByCaptureId when captureId does not exist in captureIdToDeviceNameMap_.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, GetDeviceNameByCaptureId_002, TestSize.Level1)
{
    uint32_t captureId = 1;
    std::string deviceName;
    manager_->captureIdToDeviceNameMap_.clear();
    manager_->GetDeviceNameByCaptureId(captureId, deviceName);
    EXPECT_EQ(deviceName, "");
}

/**
 * @tc.name: CreateEnhanceChainInner_001
 * @tc.desc: Test CreateEnhanceChainInner when ParseSceneKeyCode fails.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, CreateEnhanceChainInner_001, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain;
    uint64_t sceneKeyCode = 12345;
    AudioEnhanceDeviceAttr deviceAttr;
    bool createFlag = false;
    bool defaultFlag = false;
    int32_t result = manager_->CreateEnhanceChainInner(audioEnhanceChain, sceneKeyCode, deviceAttr,
        createFlag, defaultFlag);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name: CreateEnhanceChainInner_002
 * @tc.desc: Test CreateEnhanceChainInner when captureId2DefaultChainCount_[captureId] != 0.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, CreateEnhanceChainInner_002, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain;
    uint64_t sceneKeyCode = (static_cast<uint64_t>(SCENE_RECORD) << SCENE_TYPE_OFFSET) |
                            (static_cast<uint64_t>(1) << CAPTURER_ID_OFFSET) |
                            static_cast<uint64_t>(1);
    AudioEnhanceDeviceAttr deviceAttr;
    bool createFlag = false;
    bool defaultFlag = false;

    uint32_t captureId = 1;
    uint32_t renderId = 1;
    DeviceType capturerDevice = DEVICE_TYPE_MIC;
    DeviceType rendererDevice = DEVICE_TYPE_SPEAKER;
    manager_->captureIdToDeviceMap_[captureId] = capturerDevice;
    manager_->renderIdToDeviceMap_[renderId] = rendererDevice;

    manager_->captureId2DefaultChainCount_[captureId] = 1;
    std::shared_ptr<AudioEnhanceChain> defaultChain = std::make_shared<AudioEnhanceChain>("defaultScene",
        AudioEnhanceParamAdapter(), AudioEnhanceDeviceAttr(), true);
    manager_->captureId2DefaultChain_[captureId] = defaultChain;
    int32_t result = manager_->CreateEnhanceChainInner(audioEnhanceChain, sceneKeyCode, deviceAttr,
        createFlag, defaultFlag);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: CreateEnhanceChainInner_003
 * @tc.desc: Test CreateEnhanceChainInner when creating the default enhance chain.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, CreateEnhanceChainInner_003, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain;
    uint64_t sceneKeyCode = (static_cast<uint64_t>(SCENE_RECORD) << SCENE_TYPE_OFFSET) |
                            (static_cast<uint64_t>(1) << CAPTURER_ID_OFFSET) |
                            static_cast<uint64_t>(1);
    AudioEnhanceDeviceAttr deviceAttr;
    bool createFlag = false;
    bool defaultFlag = false;

    uint32_t captureId = 1;
    uint32_t renderId = 1;
    DeviceType capturerDevice = DEVICE_TYPE_MIC;
    DeviceType rendererDevice = DEVICE_TYPE_SPEAKER;
    manager_->captureIdToDeviceMap_[captureId] = capturerDevice;
    manager_->renderIdToDeviceMap_[renderId] = rendererDevice;

    manager_->captureId2SceneCount_[captureId] = manager_->normalSceneLimit_;
    manager_->captureId2DefaultChainCount_[captureId] = 0;
    int32_t result = manager_->CreateEnhanceChainInner(audioEnhanceChain, sceneKeyCode, deviceAttr,
        createFlag, defaultFlag);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: CreateEnhanceChainInner_004
 * @tc.desc: Test CreateEnhanceChainInner when creating a normal scene enhance chain.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, CreateEnhanceChainInner_004, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain;
    uint64_t sceneKeyCode = (static_cast<uint64_t>(SCENE_RECORD) << SCENE_TYPE_OFFSET) |
                            (static_cast<uint64_t>(1) << CAPTURER_ID_OFFSET) |
                            static_cast<uint64_t>(1);
    AudioEnhanceDeviceAttr deviceAttr;
    bool createFlag = false;
    bool defaultFlag = false;

    uint32_t captureId = 1;
    uint32_t renderId = 1;
    DeviceType capturerDevice = DEVICE_TYPE_MIC;
    DeviceType rendererDevice = DEVICE_TYPE_SPEAKER;
    manager_->captureIdToDeviceMap_[captureId] = capturerDevice;
    manager_->renderIdToDeviceMap_[renderId] = rendererDevice;

    manager_->captureId2SceneCount_[captureId] = 0;
    manager_->normalSceneLimit_ = 1;
    int32_t result = manager_->CreateEnhanceChainInner(audioEnhanceChain, sceneKeyCode, deviceAttr,
        createFlag, defaultFlag);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: CreateEnhanceChainInner_005
 * @tc.desc: Test CreateEnhanceChainInner when creating a prior scene enhance chain.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, CreateEnhanceChainInner_005, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain;
    uint64_t sceneKeyCode = (static_cast<uint64_t>(SCENE_RECORD) << SCENE_TYPE_OFFSET) |
                            (static_cast<uint64_t>(1) << CAPTURER_ID_OFFSET) |
                            static_cast<uint64_t>(1);
    AudioEnhanceDeviceAttr deviceAttr;
    bool createFlag = false;
    bool defaultFlag = false;

    uint32_t captureId = 1;
    uint32_t renderId = 1;
    DeviceType capturerDevice = DEVICE_TYPE_MIC;
    DeviceType rendererDevice = DEVICE_TYPE_SPEAKER;
    manager_->captureIdToDeviceMap_[captureId] = capturerDevice;
    manager_->renderIdToDeviceMap_[renderId] = rendererDevice;

    std::string sceneType = "PRIOR_SCENE";
    manager_->priorSceneSet_.insert(sceneType);
    int32_t result = manager_->CreateEnhanceChainInner(audioEnhanceChain, sceneKeyCode, deviceAttr,
        createFlag, defaultFlag);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: AddAudioEnhanceChainHandles_001
 * @tc.desc: Test AddAudioEnhanceChainHandles when createEffect fails.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, AddAudioEnhanceChainHandles_001, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>("scene1",
        AudioEnhanceParamAdapter(), AudioEnhanceDeviceAttr(), false);
    std::string enhanceChain = "EFFECTCHAIN_1";
    int32_t result = manager_->AddAudioEnhanceChainHandles(audioEnhanceChain, enhanceChain);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name: FreeEnhanceBuffer_001
 * @tc.desc: Test FreeEnhanceBuffer when enhanceBuffer_ is not nullptr.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, FreeEnhanceBuffer_001, TestSize.Level1)
{
    manager_->enhanceBuffer_ = std::make_unique<EnhanceBuffer>();
    manager_->enhanceBuffer_->micBufferIn.resize(100);
    manager_->enhanceBuffer_->micBufferOut.resize(100);
    manager_->enhanceBuffer_->ecBuffer.resize(100);
    manager_->enhanceBuffer_->micRefBuffer.resize(100);
    int32_t result = manager_->FreeEnhanceBuffer();
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(manager_->enhanceBuffer_, nullptr);
}

/**
 * @tc.name: ReleaseAudioEnhanceChainDynamic_001
 * @tc.desc: Test ReleaseAudioEnhanceChainDynamic when sceneTypeAndModeToEnhanceChainNameMap_.size() == 0.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ReleaseAudioEnhanceChainDynamic_001, TestSize.Level1)
{
    manager_->isInitialized_ = false;
    manager_->sceneTypeAndModeToEnhanceChainNameMap_.clear();
    uint64_t sceneKeyCode = 12345;
    int32_t result = manager_->ReleaseAudioEnhanceChainDynamic(sceneKeyCode);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name: ReleaseAudioEnhanceChainDynamic_002
 * @tc.desc: Test ReleaseAudioEnhanceChainDynamic when chainMapIter is end or chainMapIter->second is nullptr.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ReleaseAudioEnhanceChainDynamic_002, TestSize.Level1)
{
    uint64_t sceneKeyCode = 12345;
    manager_->sceneTypeAndModeToEnhanceChainNameMap_["SCENE_1_&_ENHANCE_DEFAULT"] = "CHAIN_1";
    int32_t result = manager_->ReleaseAudioEnhanceChainDynamic(sceneKeyCode);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: ReleaseAudioEnhanceChainDynamic_003
 * @tc.desc: Test ReleaseAudioEnhanceChainDynamic when chainCountIter->second > 1.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ReleaseAudioEnhanceChainDynamic_003, TestSize.Level1)
{
    uint64_t sceneKeyCode = 12345;
    manager_->sceneTypeToEnhanceChainCountMap_[sceneKeyCode] = 2;
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>("scene1",
        AudioEnhanceParamAdapter(), AudioEnhanceDeviceAttr(), false);
    manager_->sceneTypeToEnhanceChainMap_[sceneKeyCode] = audioEnhanceChain;
    manager_->sceneTypeAndModeToEnhanceChainNameMap_["SCENE_1_&_ENHANCE_DEFAULT"] = "CHAIN_1";
    int32_t result = manager_->ReleaseAudioEnhanceChainDynamic(sceneKeyCode);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(manager_->sceneTypeToEnhanceChainCountMap_[sceneKeyCode], 1);
}

/**
 * @tc.name: ReleaseAudioEnhanceChainDynamic_004
 * @tc.desc: Test ReleaseAudioEnhanceChainDynamic when DeleteEnhanceChainInner fails.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ReleaseAudioEnhanceChainDynamic_004, TestSize.Level1)
{
    uint64_t sceneKeyCode = 12345;
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>("scene1",
        AudioEnhanceParamAdapter(), AudioEnhanceDeviceAttr(), false);
    manager_->sceneTypeToEnhanceChainMap_[sceneKeyCode] = audioEnhanceChain;
    manager_->sceneTypeToEnhanceChainCountMap_[sceneKeyCode] = 1;
    manager_->sceneTypeAndModeToEnhanceChainNameMap_["SCENE_1_&_ENHANCE_DEFAULT"] = "CHAIN_1";
    int32_t result = manager_->ReleaseAudioEnhanceChainDynamic(sceneKeyCode);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name: DeleteEnhanceChainInner_001
 * @tc.desc: Test DeleteEnhanceChainInner when ParseSceneKeyCode fails.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, DeleteEnhanceChainInner_001, TestSize.Level1)
{
    // 前置条件设置
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain;
    uint64_t sceneKeyCode = 12345;

    // 调用函数
    int32_t result = manager_->DeleteEnhanceChainInner(audioEnhanceChain, sceneKeyCode);

    // 验证结果
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name: ExistAudioEnhanceChain_002
 * @tc.desc: Test ExistAudioEnhanceChain when sceneKeyCode exists in sceneTypeToEnhanceChainMap_.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ExistAudioEnhanceChain_002, TestSize.Level1)
{
    uint64_t sceneKeyCode = 12345;
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>("scene1",
        AudioEnhanceParamAdapter(), AudioEnhanceDeviceAttr(), false);
    manager_->sceneTypeToEnhanceChainMap_.clear();
    manager_->sceneTypeToEnhanceChainMap_[sceneKeyCode] = audioEnhanceChain;
    manager_->isInitialized_ = true;
    bool result = manager_->ExistAudioEnhanceChain(sceneKeyCode);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: AudioEnhanceChainGetAlgoConfig_002
 * @tc.desc: Test AudioEnhanceChainGetAlgoConfig when sceneKeyCode exists in sceneTypeToEnhanceChainMap_.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, AudioEnhanceChainGetAlgoConfig_002, TestSize.Level1)
{
    uint64_t sceneKeyCode = 12345;
    AudioBufferConfig micConfig;
    AudioBufferConfig ecConfig;
    AudioBufferConfig micRefConfig;
    manager_->isInitialized_ = true;
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>("scene1",
        AudioEnhanceParamAdapter(), AudioEnhanceDeviceAttr(), false);
    manager_->sceneTypeToEnhanceChainMap_.clear();
    manager_->sceneTypeToEnhanceChainMap_[sceneKeyCode] = audioEnhanceChain;
    int32_t result = manager_->AudioEnhanceChainGetAlgoConfig(sceneKeyCode, micConfig, ecConfig, micRefConfig);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: IsEmptyEnhanceChain_001
 * @tc.desc: Test IsEmptyEnhanceChain when sceneTypeAndModeToEnhanceChainNameMap_ is empty.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, IsEmptyEnhanceChain_001, TestSize.Level1)
{
    manager_->isInitialized_ = true;
    manager_->sceneTypeAndModeToEnhanceChainNameMap_.clear();
    bool result = manager_->IsEmptyEnhanceChain();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsEmptyEnhanceChain_002
 * @tc.desc: Test IsEmptyEnhanceChain when sceneTypeAndModeToEnhanceChainNameMap_ is empty.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, IsEmptyEnhanceChain_002, TestSize.Level1)
{
    manager_->isInitialized_ = true;
    manager_->sceneTypeAndModeToEnhanceChainNameMap_.clear();
    bool result = manager_->IsEmptyEnhanceChain();
    EXPECT_TRUE(result);
}
/**
 * @tc.name: IsEmptyEnhanceChain_003
 * @tc.desc: Test IsEmptyEnhanceChain when chain is nullptr in the for loop.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, IsEmptyEnhanceChain_003, TestSize.Level1)
{
    manager_->isInitialized_ = true;
    manager_->sceneTypeAndModeToEnhanceChainNameMap_.clear();
    manager_->sceneTypeAndModeToEnhanceChainNameMap_["scene1"] = "chain1";

    manager_->sceneTypeToEnhanceChainMap_.clear();
    manager_->sceneTypeToEnhanceChainMap_[12345] = nullptr;
    bool result = manager_->IsEmptyEnhanceChain();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsEmptyEnhanceChain_004
 * @tc.desc: Test IsEmptyEnhanceChain when chain is not nullptr but IsEmptyEnhanceHandles() returns true.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, IsEmptyEnhanceChain_004, TestSize.Level1)
{
    manager_->isInitialized_ = true;
    manager_->sceneTypeAndModeToEnhanceChainNameMap_.clear();
    manager_->sceneTypeAndModeToEnhanceChainNameMap_["scene1"] = "chain1";

    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>("scene1",
        AudioEnhanceParamAdapter(), AudioEnhanceDeviceAttr(), false);
    manager_->sceneTypeToEnhanceChainMap_.clear();
    manager_->sceneTypeToEnhanceChainMap_[12345] = audioEnhanceChain;
    bool result = manager_->IsEmptyEnhanceChain();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: CopyToEnhanceBuffer_001
 * @tc.desc: Test CopyToEnhanceBuffer when enhanceBuffer_ is nullptr.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, CopyToEnhanceBuffer_001, TestSize.Level1)
{
    uint64_t validKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validKeyCode, deviceAttr);
    manager_->InitEnhanceBuffer();
    manager_->enhanceBuffer_ = nullptr;
    void *data = nullptr;
    uint32_t length = 100;
    int32_t result = manager_->CopyToEnhanceBuffer(data, length);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name: CopyToEnhanceBuffer_002
 * @tc.desc: Test CopyToEnhanceBuffer when enhanceBuffer_ is not nullptr.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, CopyToEnhanceBuffer_002, TestSize.Level1)
{
    uint64_t validKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validKeyCode, deviceAttr);
    manager_->InitEnhanceBuffer();
    manager_->enhanceBuffer_ = std::make_unique<EnhanceBuffer>();
    manager_->enhanceBuffer_->micBufferIn.resize(1024);
    void *data = malloc(100);
    uint32_t length = 100;
    int32_t result = manager_->CopyToEnhanceBuffer(data, length);
    EXPECT_EQ(result, SUCCESS);
    free(data);
}

/**
 * @tc.name: CopyEcToEnhanceBuffer_001
 * @tc.desc: Test CopyEcToEnhanceBuffer when enhanceBuffer_ is nullptr.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, CopyEcToEnhanceBuffer_001, TestSize.Level1)
{
    manager_->enhanceBuffer_ = nullptr;
    void *data = nullptr;
    uint32_t length = 100;
    int32_t result = manager_->CopyEcToEnhanceBuffer(data, length);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name: CopyEcToEnhanceBuffer_002
 * @tc.desc: Test CopyEcToEnhanceBuffer when enhanceBuffer_ is not nullptr.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, CopyEcToEnhanceBuffer_002, TestSize.Level1)
{
    manager_->enhanceBuffer_ = std::make_unique<EnhanceBuffer>();
    manager_->enhanceBuffer_->ecBuffer.resize(1024);

    void *data = malloc(100);
    uint32_t length = 100;
    int32_t result = manager_->CopyEcToEnhanceBuffer(data, length);
    EXPECT_EQ(result, SUCCESS);
    free(data);
}

/**
 * @tc.name: CopyMicRefToEnhanceBuffer_001
 * @tc.desc: Test CopyMicRefToEnhanceBuffer when enhanceBuffer_ is nullptr.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, CopyMicRefToEnhanceBuffer_001, TestSize.Level1)
{
    manager_->enhanceBuffer_ = nullptr;
    void *data = nullptr;
    uint32_t length = 100;

    int32_t result = manager_->CopyMicRefToEnhanceBuffer(data, length);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name: CopyMicRefToEnhanceBuffer_002
 * @tc.desc: Test CopyMicRefToEnhanceBuffer when enhanceBuffer_ is not nullptr.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, CopyMicRefToEnhanceBuffer_002, TestSize.Level1)
{
    manager_->enhanceBuffer_ = std::make_unique<EnhanceBuffer>();
    manager_->enhanceBuffer_->micRefBuffer.resize(1024);
    void *data = malloc(100);
    uint32_t length = 100;

    int32_t result = manager_->CopyMicRefToEnhanceBuffer(data, length);
    EXPECT_EQ(result, SUCCESS);
    free(data);
}


/**
 * @tc.name: CopyFromEnhanceBuffer_001
 * @tc.desc: Test CopyFromEnhanceBuffer when enhanceBuffer_ is nullptr.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, CopyFromEnhanceBuffer_001, TestSize.Level1)
{
    manager_->enhanceBuffer_ = nullptr;
    void *data = malloc(100);
    uint32_t length = 100;

    int32_t result = manager_->CopyFromEnhanceBuffer(data, length);
    EXPECT_EQ(result, ERROR);
    free(data);
}

/**
 * @tc.name: CopyFromEnhanceBuffer_002
 * @tc.desc: Test CopyFromEnhanceBuffer when length > enhanceBuffer_->micBufferOut.size().
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, CopyFromEnhanceBuffer_002, TestSize.Level1)
{
    manager_->enhanceBuffer_ = std::make_unique<EnhanceBuffer>();
    manager_->enhanceBuffer_->micBufferOut.resize(100);
    void *data = malloc(200);
    uint32_t length = 200;

    int32_t result = manager_->CopyFromEnhanceBuffer(data, length);
    EXPECT_EQ(result, ERROR);
    free(data);
}

/**
 * @tc.name: CopyFromEnhanceBuffer_003
 * @tc.desc: Test CopyFromEnhanceBuffer when enhanceBuffer_ is not nullptr and length
 *  <= enhanceBuffer_->micBufferOut.size().
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, CopyFromEnhanceBuffer_003, TestSize.Level1)
{
    manager_->enhanceBuffer_ = std::make_unique<EnhanceBuffer>();
    manager_->enhanceBuffer_->micBufferOut.resize(1024);
    void *data = malloc(100);
    uint32_t length = 100;

    int32_t result = manager_->CopyFromEnhanceBuffer(data, length);
    EXPECT_EQ(result, SUCCESS);
    free(data);
}

/**
 * @tc.name: ApplyAudioEnhanceChain_001
 * @tc.desc: Test ApplyAudioEnhanceChain when chainMapIter == sceneTypeToEnhanceChainMap_.end().
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ApplyAudioEnhanceChain_001, TestSize.Level1)
{
    manager_->enhanceBuffer_ = std::make_unique<EnhanceBuffer>();
    manager_->enhanceBuffer_->micBufferIn.resize(1024);
    manager_->enhanceBuffer_->micBufferOut.resize(1024);
    uint64_t sceneKeyCode = 12345;
    uint32_t length = 100;
    manager_->sceneTypeToEnhanceChainMap_.clear();

    int32_t result = manager_->ApplyAudioEnhanceChain(sceneKeyCode, length);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name: ApplyAudioEnhanceChain_002
 * @tc.desc: Test ApplyAudioEnhanceChain when chainMapIter->second == nullptr.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ApplyAudioEnhanceChain_002, TestSize.Level1)
{
    manager_->enhanceBuffer_ = std::make_unique<EnhanceBuffer>();
    manager_->enhanceBuffer_->micBufferIn.resize(1024);
    manager_->enhanceBuffer_->micBufferOut.resize(1024);
    uint64_t sceneKeyCode = 12345;
    uint32_t length = 100;
    manager_->sceneTypeToEnhanceChainMap_.clear();
    manager_->sceneTypeToEnhanceChainMap_[sceneKeyCode] = nullptr;

    int32_t result = manager_->ApplyAudioEnhanceChain(sceneKeyCode, length);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name: UpdatePropertyAndSendToAlgo_001
 * @tc.desc: Test UpdatePropertyAndSendToAlgo when enhanceChain is not nullptr.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, UpdatePropertyAndSendToAlgo_001, TestSize.Level1)
{
    manager_->isInitialized_ = true;
    DeviceType inputDevice = DeviceType::DEVICE_TYPE_MIC;
    manager_->enhancePropertyMap_.clear();
    manager_->enhancePropertyMap_["effect1"] = "property1";
    manager_->sceneTypeToEnhanceChainMap_.clear();
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>("scene1",
        AudioEnhanceParamAdapter(), AudioEnhanceDeviceAttr(), false);
    manager_->sceneTypeToEnhanceChainMap_[12345] = audioEnhanceChain;

    int32_t result = manager_->UpdatePropertyAndSendToAlgo(inputDevice);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: UpdatePropertyAndSendToAlgo_002
 * @tc.desc: Test UpdatePropertyAndSendToAlgo when enhanceChain is nullptr.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, UpdatePropertyAndSendToAlgo_002, TestSize.Level1)
{
    manager_->isInitialized_ = true;
    DeviceType inputDevice = DeviceType::DEVICE_TYPE_MIC;
    manager_->enhancePropertyMap_.clear();
    manager_->enhancePropertyMap_["effect1"] = "property1";

    manager_->sceneTypeToEnhanceChainMap_.clear();
    manager_->sceneTypeToEnhanceChainMap_[12345] = nullptr;
    int32_t result = manager_->UpdatePropertyAndSendToAlgo(inputDevice);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: SetInputDevice_001
 * @tc.desc: Test SetInputDevice when captureId is not in captureIdToDeviceMap_.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetInputDevice_001, TestSize.Level1)
{
    uint32_t captureId = 12345;
    DeviceType inputDevice = DeviceType::DEVICE_TYPE_MIC;
    std::string deviceName = "mic_device";
    manager_->captureIdToDeviceMap_.clear();

    int32_t result = manager_->SetInputDevice(captureId, inputDevice, deviceName);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: SetInputDevice_002
 * @tc.desc: Test SetInputDevice when captureId is in captureIdToDeviceMap_ and device is same.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetInputDevice_002, TestSize.Level1)
{
    uint32_t captureId = 12345;
    DeviceType inputDevice = DeviceType::DEVICE_TYPE_MIC;
    std::string deviceName = "mic_device";

    manager_->captureIdToDeviceMap_.clear();
    manager_->captureIdToDeviceMap_[captureId] = inputDevice;
    int32_t result = manager_->SetInputDevice(captureId, inputDevice, deviceName);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: SetInputDevice_003
 * @tc.desc: Test SetInputDevice when entering the innermost if branch of the for loop.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetInputDevice_003, TestSize.Level1)
{
    uint32_t captureId = 12345;
    DeviceType inputDevice = DeviceType::DEVICE_TYPE_MIC;
    std::string deviceName = "mic_device";

    manager_->captureIdToDeviceMap_.clear();
    manager_->captureIdToDeviceMap_[captureId] = DeviceType::DEVICE_TYPE_SPEAKER;
    manager_->sceneTypeToEnhanceChainMap_.clear();
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>("scene1",
        AudioEnhanceParamAdapter(), AudioEnhanceDeviceAttr(), false);
    uint64_t sceneKeyCode = (static_cast<uint64_t>(captureId) << 8) | 0x01;
    manager_->sceneTypeToEnhanceChainMap_[sceneKeyCode] = audioEnhanceChain;
    int32_t result = manager_->SetInputDevice(captureId, inputDevice, deviceName);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: SetInputDevice_004
 * @tc.desc: Test SetInputDevice when entering the first layer if branch of the for loop but
 *  not the innermost if branch.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetInputDevice_004, TestSize.Level1)
{
    uint32_t captureId = 12345;
    DeviceType inputDevice = DeviceType::DEVICE_TYPE_MIC;
    std::string deviceName = "mic_device";

    manager_->captureIdToDeviceMap_.clear();
    manager_->captureIdToDeviceMap_[captureId] = DeviceType::DEVICE_TYPE_SPEAKER;
    manager_->sceneTypeToEnhanceChainMap_.clear();
    uint64_t sceneKeyCode = (static_cast<uint64_t>(captureId) << 8) | 0x01;
    manager_->sceneTypeToEnhanceChainMap_[sceneKeyCode] = nullptr;
    int32_t result = manager_->SetInputDevice(captureId, inputDevice, deviceName);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: SetInputDevice_005
 * @tc.desc: Test SetInputDevice when not entering the first layer if branch of the for loop.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetInputDevice_005, TestSize.Level1)
{
    uint32_t captureId = 12345;
    DeviceType inputDevice = DeviceType::DEVICE_TYPE_MIC;
    std::string deviceName = "mic_device";
    manager_->captureIdToDeviceMap_.clear();
    manager_->captureIdToDeviceMap_[captureId] = DeviceType::DEVICE_TYPE_SPEAKER;

    manager_->sceneTypeToEnhanceChainMap_.clear();
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>("scene1",
        AudioEnhanceParamAdapter(), AudioEnhanceDeviceAttr(), false);
    uint64_t sceneKeyCode = (static_cast<uint64_t>(captureId + 1) << 8) | 0x01;
    manager_->sceneTypeToEnhanceChainMap_[sceneKeyCode] = audioEnhanceChain;
    int32_t result = manager_->SetInputDevice(captureId, inputDevice, deviceName);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: SetInputDevice_006
 * @tc.desc: Test SetInputDevice when entering the last if branch.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetInputDevice_006, TestSize.Level1)
{
    uint32_t captureId = 12345;
    DeviceType inputDevice = DeviceType::DEVICE_TYPE_MIC;
    std::string deviceName = "mic_device";

    manager_->captureIdToDeviceMap_.clear();
    manager_->captureIdToDeviceMap_[captureId] = DeviceType::DEVICE_TYPE_SPEAKER;
    manager_->sceneTypeToEnhanceChainMap_.clear();
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>("scene1",
        AudioEnhanceParamAdapter(), AudioEnhanceDeviceAttr(), false);
    uint64_t sceneKeyCode = (static_cast<uint64_t>(captureId) << 8) | 0x01;
    manager_->sceneTypeToEnhanceChainMap_[sceneKeyCode] = audioEnhanceChain;
    int32_t result = manager_->SetInputDevice(captureId, inputDevice, deviceName);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: SetInputDevice_007
 * @tc.desc: Test SetInputDevice when not entering the last if branch.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetInputDevice_007, TestSize.Level1)
{
    uint32_t captureId = 12345;
    DeviceType inputDevice = DeviceType::DEVICE_TYPE_MIC;
    std::string deviceName = "mic_device";
    manager_->captureIdToDeviceMap_.clear();
    manager_->captureIdToDeviceMap_[captureId] = DeviceType::DEVICE_TYPE_SPEAKER;

    manager_->sceneTypeToEnhanceChainMap_.clear();
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>("scene1",
        AudioEnhanceParamAdapter(), AudioEnhanceDeviceAttr(), false);
    uint64_t sceneKeyCode = (static_cast<uint64_t>(captureId) << 8) | 0x01;
    manager_->sceneTypeToEnhanceChainMap_[sceneKeyCode] = audioEnhanceChain;
    int32_t result = manager_->SetInputDevice(captureId, inputDevice, deviceName);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: SetVolumeInfo_005
 * @tc.desc: Test SetVolumeInfo when sceneTypeAndModeToEnhanceChainNameMap_ and sceneTypeToEnhanceChainMap_
 *  are not empty.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetVolumeInfo_005, TestSize.Level1)
{
    manager_->volumeType_ = STREAM_MUSIC;
    manager_->systemVol_ = 0.0f;
    manager_->sceneTypeAndModeToEnhanceChainNameMap_.clear();
    manager_->sceneTypeAndModeToEnhanceChainNameMap_["scene1"] = "chain1";
    manager_->sceneTypeToEnhanceChainMap_.clear();
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>("scene1",
        AudioEnhanceParamAdapter(), AudioEnhanceDeviceAttr(), false);
    manager_->sceneTypeToEnhanceChainMap_[12345] = audioEnhanceChain;

    int32_t result = manager_->SetVolumeInfo(STREAM_MUSIC, 0.5f);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(manager_->volumeType_, STREAM_MUSIC);
    EXPECT_FLOAT_EQ(manager_->systemVol_, 0.5f);
}

/**
 * @tc.name: SetMicrophoneMuteInfo_001
 * @tc.desc: Test SetMicrophoneMuteInfo when enhanceChain is not nullptr.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetMicrophoneMuteInfo_001, TestSize.Level1)
{
    manager_->isMute_ = false;
    manager_->systemVol_ = 0.5f;
    manager_->sceneTypeToEnhanceChainMap_.clear();
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>("scene1",
        AudioEnhanceParamAdapter(), AudioEnhanceDeviceAttr(), false);
    manager_->sceneTypeToEnhanceChainMap_[12345] = audioEnhanceChain;

    int32_t result = manager_->SetMicrophoneMuteInfo(true);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_TRUE(manager_->isMute_);
}

/**
 * @tc.name: SetMicrophoneMuteInfo_002
 * @tc.desc: Test SetMicrophoneMuteInfo when enhanceChain is nullptr.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetMicrophoneMuteInfo_002, TestSize.Level1)
{
    manager_->isMute_ = false;
    manager_->systemVol_ = 0.5f;
    manager_->sceneTypeToEnhanceChainMap_.clear();
    manager_->sceneTypeToEnhanceChainMap_[12345] = nullptr;

    int32_t result = manager_->SetMicrophoneMuteInfo(true);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_TRUE(manager_->isMute_);
}

/**
 * @tc.name: SetStreamVolumeInfo_001
 * @tc.desc: Test SetStreamVolumeInfo when sceneTypeAndModeToEnhanceChainNameMap_ is empty.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetStreamVolumeInfo_001, TestSize.Level1)
{
    manager_->sessionId_ = 0;
    manager_->streamVol_ = 0.0f;
    manager_->sceneTypeAndModeToEnhanceChainNameMap_.clear();
    manager_->sceneTypeToEnhanceChainMap_.clear();
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>("scene1",
        AudioEnhanceParamAdapter(), AudioEnhanceDeviceAttr(), false);
    manager_->sceneTypeToEnhanceChainMap_[12345] = audioEnhanceChain;

    int32_t result = manager_->SetStreamVolumeInfo(1, 0.5f);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(manager_->sessionId_, 1);
    EXPECT_FLOAT_EQ(manager_->streamVol_, 0.5f);
}

/**
 * @tc.name: SetStreamVolumeInfo_002
 * @tc.desc: Test SetStreamVolumeInfo when sceneTypeToEnhanceChainMap_ is empty.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetStreamVolumeInfo_002, TestSize.Level1)
{
    manager_->sessionId_ = 0;
    manager_->streamVol_ = 0.0f;
    manager_->sceneTypeAndModeToEnhanceChainNameMap_.clear();
    manager_->sceneTypeAndModeToEnhanceChainNameMap_["scene1"] = "chain1";
    manager_->sceneTypeToEnhanceChainMap_.clear();

    int32_t result = manager_->SetStreamVolumeInfo(1, 0.5f);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(manager_->sessionId_, 1);
    EXPECT_FLOAT_EQ(manager_->streamVol_, 0.5f);
}

/**
 * @tc.name: SetStreamVolumeInfo_003
 * @tc.desc: Test SetStreamVolumeInfo when neither sceneTypeAndModeToEnhanceChainNameMap_ nor
 *  sceneTypeToEnhanceChainMap_ is empty.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetStreamVolumeInfo_003, TestSize.Level1)
{
    manager_->sessionId_ = 0;
    manager_->streamVol_ = 0.0f;
    manager_->sceneTypeAndModeToEnhanceChainNameMap_.clear();
    manager_->sceneTypeAndModeToEnhanceChainNameMap_["scene1"] = "chain1";
    manager_->sceneTypeToEnhanceChainMap_.clear();
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>("scene1",
        AudioEnhanceParamAdapter(), AudioEnhanceDeviceAttr(), false);
    manager_->sceneTypeToEnhanceChainMap_[12345] = audioEnhanceChain;

    int32_t result = manager_->SetStreamVolumeInfo(1, 0.5f);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(manager_->sessionId_, 1);
    EXPECT_FLOAT_EQ(manager_->streamVol_, 0.5f);
}

/**
 * @tc.name: SetAudioEnhanceProperty_008
 * @tc.desc: Test SetAudioEnhanceProperty when not entering the if branches in the for loop.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetAudioEnhanceProperty_008, TestSize.Level1)
{
    AudioEffectPropertyArrayV3 propertyArray;
    propertyArray.property.resize(1);
    propertyArray.property[0].name = "effect1";
    propertyArray.property[0].category = "category1";
    manager_->withDeviceEnhances_.insert("effect1");
    manager_->enhancePropertyMap_.clear();

    int32_t result = manager_->SetAudioEnhanceProperty(propertyArray, DeviceType::DEVICE_TYPE_MIC);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(manager_->enhancePropertyMap_["effect1"], "category1");
}

/**
 * @tc.name: SetAudioEnhanceProperty_009
 * @tc.desc: Test SetAudioEnhanceProperty when not entering the if branches in the for loop.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetAudioEnhanceProperty_009, TestSize.Level1)
{
    AudioEnhancePropertyArray propertyArray;
    propertyArray.property.resize(1);
    propertyArray.property[0].enhanceClass = "effect1";
    propertyArray.property[0].enhanceProp = "prop1";
    manager_->withDeviceEnhances_.insert("effect1");
    manager_->enhancePropertyMap_.clear();

    int32_t result = manager_->SetAudioEnhanceProperty(propertyArray, DeviceType::DEVICE_TYPE_MIC);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(manager_->enhancePropertyMap_["effect1"], "prop1");
}
/**
 * @tc.name: SetAudioEnhancePropertyToChains_001
 * @tc.desc: Test SetAudioEnhancePropertyToChains when enhanceChain is not nullptr.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetAudioEnhancePropertyToChains_001, TestSize.Level1)
{
    AudioEffectPropertyV3 property;
    property.name = "effect1";
    property.category = "category1";
    manager_->sceneTypeToEnhanceChainMap_.clear();
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>("scene1",
        AudioEnhanceParamAdapter(), AudioEnhanceDeviceAttr(), false);
    manager_->sceneTypeToEnhanceChainMap_[12345] = audioEnhanceChain;

    int32_t result = manager_->SetAudioEnhancePropertyToChains(property);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: SetAudioEnhancePropertyToChains_002
 * @tc.desc: Test SetAudioEnhancePropertyToChains when enhanceChain is nullptr.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetAudioEnhancePropertyToChains_002, TestSize.Level1)
{
    AudioEffectPropertyV3 property;
    property.name = "effect1";
    property.category = "category1";
    manager_->sceneTypeToEnhanceChainMap_.clear();
    manager_->sceneTypeToEnhanceChainMap_[12345] = nullptr;
    int32_t result = manager_->SetAudioEnhancePropertyToChains(property);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: SetAudioEnhancePropertyToChains_003
 * @tc.desc: Test SetAudioEnhancePropertyToChains when enhanceChain is not nullptr.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetAudioEnhancePropertyToChains_003, TestSize.Level1)
{
    AudioEnhanceProperty property;
    property.enhanceClass = "effect1";
    property.enhanceProp = "prop1";
    manager_->sceneTypeToEnhanceChainMap_.clear();
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>("scene1",
        AudioEnhanceParamAdapter(), AudioEnhanceDeviceAttr(), false);
    manager_->sceneTypeToEnhanceChainMap_[12345] = audioEnhanceChain;

    int32_t result = manager_->SetAudioEnhancePropertyToChains(property);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: SetAudioEnhancePropertyToChains_004
 * @tc.desc: Test SetAudioEnhancePropertyToChains when enhanceChain is nullptr.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetAudioEnhancePropertyToChains_004, TestSize.Level1)
{
    AudioEnhanceProperty property;
    property.enhanceClass = "effect1";
    property.enhanceProp = "prop1";
    manager_->sceneTypeToEnhanceChainMap_.clear();
    manager_->sceneTypeToEnhanceChainMap_[12345] = nullptr;

    int32_t result = manager_->SetAudioEnhancePropertyToChains(property);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: GetDeviceTypeName_001
 * @tc.desc: Test GetDeviceTypeName when deviceType is not found in supportDeviceType.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, GetDeviceTypeName_001, TestSize.Level1)
{
    DeviceType deviceType = static_cast<DeviceType>(99);
    std::string deviceName;
    manager_->GetDeviceTypeName(deviceType, deviceName);
    EXPECT_TRUE(deviceName.empty());
}

/**
 * @tc.name: GetAudioEnhanceProperty_005
 * @tc.desc: Test GetAudioEnhanceProperty when entering the for loop but not the internal if branch.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, GetAudioEnhanceProperty_005, TestSize.Level1)
{
    AudioEffectPropertyArrayV3 propertyArray;
    DeviceType deviceType = DeviceType::DEVICE_TYPE_NONE;
    manager_->enhancePropertyMap_.clear();
    manager_->enhancePropertyMap_["effect1"] = "";
    manager_->enhancePropertyMap_["effect2"] = "";

    int32_t result = manager_->GetAudioEnhanceProperty(propertyArray, deviceType);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_TRUE(propertyArray.property.empty());
}

/**
 * @tc.name: GetAudioEnhanceProperty_006
 * @tc.desc: Test GetAudioEnhanceProperty when entering the if branch in the for loop.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, GetAudioEnhanceProperty_006, TestSize.Level1)
{
    AudioEnhancePropertyArray propertyArray;
    DeviceType deviceType = DeviceType::DEVICE_TYPE_NONE;
    manager_->enhancePropertyMap_.clear();
    manager_->enhancePropertyMap_["effect1"] = "prop1";
    manager_->enhancePropertyMap_["effect2"] = "prop2";
    int32_t result = manager_->GetAudioEnhanceProperty(propertyArray, deviceType);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(propertyArray.property.size(), 2);
}

/**
 * @tc.name: ApplyAudioEnhanceChainDefault_001
 * @tc.desc: Test ApplyAudioEnhanceChainDefault when chainMapIter == captureId2DefaultChain_.end().
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ApplyAudioEnhanceChainDefault_001, TestSize.Level1)
{
    manager_->enhanceBuffer_ = std::make_unique<EnhanceBuffer>();
    manager_->enhanceBuffer_->micBufferIn.resize(1024);
    manager_->enhanceBuffer_->micBufferOut.resize(1024);
    uint32_t captureId = 12345;
    uint32_t length = 100;
    manager_->captureId2DefaultChain_.clear();

    int32_t result = manager_->ApplyAudioEnhanceChainDefault(captureId, length);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name: ApplyAudioEnhanceChainDefault_002
 * @tc.desc: Test ApplyAudioEnhanceChainDefault when chainMapIter->second == nullptr.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ApplyAudioEnhanceChainDefault_002, TestSize.Level1)
{
    manager_->enhanceBuffer_ = std::make_unique<EnhanceBuffer>();
    manager_->enhanceBuffer_->micBufferIn.resize(1024);
    manager_->enhanceBuffer_->micBufferOut.resize(1024);
    uint32_t captureId = 12345;
    uint32_t length = 100;
    manager_->captureId2DefaultChain_.clear();
    manager_->captureId2DefaultChain_[captureId] = nullptr;

    int32_t result = manager_->ApplyAudioEnhanceChainDefault(captureId, length);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name: UpdateExtraSceneType_001
 * @tc.desc: Test UpdateExtraSceneType when entering the else branch of the outermost if statement.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, UpdateExtraSceneType_001, TestSize.Level1)
{
    std::string mainkey = "other_mainkey";
    std::string subkey = "other_subkey";
    std::string extraSceneType = "extra_scene";
    manager_->UpdateExtraSceneType(mainkey, subkey, extraSceneType);
    EXPECT_NE(manager_, nullptr);
}

/**
 * @tc.name: UpdateExtraSceneType_002
 * @tc.desc: Test UpdateExtraSceneType when entering the first if branch of the outermost if statement and
 *  the first if branch in the for loop.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, UpdateExtraSceneType_002, TestSize.Level1)
{
    std::string mainkey = MAINKEY_DEVICE_STATUS;
    std::string subkey = SUBKEY_FOLD_STATE;
    std::string extraSceneType = "123";
    manager_->sceneTypeToEnhanceChainMap_.clear();
    manager_->sceneTypeToEnhanceChainMap_[12345] = nullptr;

    manager_->UpdateExtraSceneType(mainkey, subkey, extraSceneType);
    EXPECT_EQ(manager_->foldState_, 123);
}

/**
 * @tc.name: UpdateExtraSceneType_003
 * @tc.desc: Test UpdateExtraSceneType when entering the else branch of the first if statement
 *  inside the outermost if branch.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, UpdateExtraSceneType_003, TestSize.Level1)
{
    std::string mainkey = MAINKEY_DEVICE_STATUS;
    std::string subkey = SUBKEY_FOLD_STATE;
    std::string extraSceneType = "invalid";
    manager_->UpdateExtraSceneType(mainkey, subkey, extraSceneType);
    EXPECT_NE(manager_, nullptr);
}

/**
 * @tc.name: UpdateExtraSceneType_004
 * @tc.desc: Test UpdateExtraSceneType when entering the second if branch in the for loop inside
 *  the outermost if branch.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, UpdateExtraSceneType_004, TestSize.Level1)
{
    std::string mainkey = MAINKEY_DEVICE_STATUS;
    std::string subkey = SUBKEY_FOLD_STATE;
    std::string extraSceneType = "123";
    manager_->sceneTypeToEnhanceChainMap_.clear();
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>("scene1",
        AudioEnhanceParamAdapter(), AudioEnhanceDeviceAttr(), false);
    manager_->sceneTypeToEnhanceChainMap_[12345] = audioEnhanceChain;

    manager_->UpdateExtraSceneType(mainkey, subkey, extraSceneType);
    EXPECT_EQ(manager_->foldState_, 123);
}

/**
 * @tc.name: UpdateExtraSceneType_005
 * @tc.desc: Test UpdateExtraSceneType when entering the for loop inside the outermost if branch
 *  but not entering any if branches.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, UpdateExtraSceneType_005, TestSize.Level1)
{
    std::string mainkey = MAINKEY_DEVICE_STATUS;
    std::string subkey = SUBKEY_FOLD_STATE;
    std::string extraSceneType = "123";
    manager_->sceneTypeToEnhanceChainMap_.clear();
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>("scene1",
        AudioEnhanceParamAdapter(), AudioEnhanceDeviceAttr(), false);
    manager_->sceneTypeToEnhanceChainMap_[12345] = audioEnhanceChain;

    manager_->UpdateExtraSceneType(mainkey, subkey, extraSceneType);
    EXPECT_EQ(manager_->foldState_, 123);
}

/**
 * @tc.name: SendInitCommand_001
 * @tc.desc: Test SendInitCommand when enhanceChain is not nullptr.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SendInitCommand_001, TestSize.Level1)
{
    manager_->sceneTypeToEnhanceChainMap_.clear();
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>("scene1",
        AudioEnhanceParamAdapter(), AudioEnhanceDeviceAttr(), false);
    manager_->sceneTypeToEnhanceChainMap_[12345] = audioEnhanceChain;
    int32_t result = manager_->SendInitCommand();
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: SendInitCommand_002
 * @tc.desc: Test SendInitCommand when enhanceChain is nullptr.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SendInitCommand_002, TestSize.Level1)
{
    manager_->sceneTypeToEnhanceChainMap_.clear();
    manager_->sceneTypeToEnhanceChainMap_[12345] = nullptr;
    int32_t result = manager_->SendInitCommand();
    EXPECT_EQ(result, SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS