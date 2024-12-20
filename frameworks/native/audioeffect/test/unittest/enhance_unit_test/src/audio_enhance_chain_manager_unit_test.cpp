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
#include "audio_utils.h"
#include "audio_log.h"
#include "audio_errors.h"

#define VALID_SCENEKEY_CODE 68864
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
    uint32_t invalidKeyCode = INVALID_SCENEKEY_CODE;
    int32_t result = manager_->CreateAudioEnhanceChainDynamic(invalidKeyCode, deviceAttr);
    EXPECT_EQ(result, ERROR);
}

/*
 * tc.name   : Test CreateAudioEnhanceChainDynamic API
 * tc.number : CreateAudioEnhanceChainDynamic_002
 * tc.desc   : Test CreateAudioEnhanceChainDynamic interface(using correct input case).
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, CreateAudioEnhanceChainDynamic_002, TestSize.Level1)
{
    uint32_t validKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validKeyCode, deviceAttr);
}

/*
 * tc.name   : Test CreateAudioEnhanceChainDynamic API
 * tc.number : CreateAudioEnhanceChainDynamic_003
 * tc.desc   : Test CreateAudioEnhanceChainDynamic when no algo on audio_framework.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, CreateAudioEnhanceChainDynamic_003, TestSize.Level1)
{
    manager_->ResetInfo();
    uint32_t invalidKeyCode = INVALID_SCENEKEY_CODE;
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
    uint32_t validKeyCode = VALID_SCENEKEY_CODE;
    std::string scene = "SCENE_RECORD";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>(scene, algoParam, deviceAttr, defaultFlag);
    manager_->sceneTypeToEnhanceChainMap_.insert_or_assign(validKeyCode, audioEnhanceChain);
    manager_->sceneTypeToEnhanceChainCountMap_.insert_or_assign(validKeyCode, 1);
    int32_t result = manager_->CreateAudioEnhanceChainDynamic(validKeyCode, deviceAttr);
    EXPECT_EQ(result, ERROR);
}

/*
 * tc.name   : Test ReleaseAudioEnhanceChainDynamic API
 * tc.number : ReleaseAudioEnhanceChainDynamic_001
 * tc.desc   : Test ReleaseAudioEnhanceChainDynamic interface when has not been initialized.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ReleaseAudioEnhanceChainDynamic_001, TestSize.Level1)
{
    manager_->ResetInfo();
    uint32_t validKeyCode = VALID_SCENEKEY_CODE;
    int32_t result = manager_->ReleaseAudioEnhanceChainDynamic(validKeyCode);
    EXPECT_EQ(result, ERROR);
}

/*
 * tc.name   : Test ReleaseAudioEnhanceChainDynamic API
 * tc.number : ReleaseAudioEnhanceChainDynamic_002
 * tc.desc   : Test ReleaseAudioEnhanceChainDynamic interface after correctly creating an enhanceChain.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ReleaseAudioEnhanceChainDynamic_002, TestSize.Level1)
{
    uint32_t validKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validKeyCode, deviceAttr);

    int32_t result = manager_->ReleaseAudioEnhanceChainDynamic(validKeyCode);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test ReleaseAudioEnhanceChainDynamic API
 * tc.number : ReleaseAudioEnhanceChainDynamic_003
 * tc.desc   : Test ReleaseAudioEnhanceChainDynamic interface after correctly creating two enhanceChain.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ReleaseAudioEnhanceChainDynamic_003, TestSize.Level1)
{
    uint32_t validKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validKeyCode, deviceAttr);
    manager_->CreateAudioEnhanceChainDynamic(validKeyCode, deviceAttr);
    int32_t result = manager_->ReleaseAudioEnhanceChainDynamic(validKeyCode);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test ReleaseAudioEnhanceChainDynamic API
 * tc.number : ReleaseAudioEnhanceChainDynamic_004
 * tc.desc   : Test ReleaseAudioEnhanceChainDynamic interface after correctly creating two enhanceChain.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ReleaseAudioEnhanceChainDynamic_004, TestSize.Level1)
{
    uint32_t validKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validKeyCode, deviceAttr);
    manager_->sceneTypeAndModeToEnhanceChainNameMap_.clear();
    int32_t result = manager_->ReleaseAudioEnhanceChainDynamic(validKeyCode);
    EXPECT_EQ(result, ERROR);
}

/*
 * tc.name   : Test ReleaseAudioEnhanceChainDynamic API
 * tc.number : ReleaseAudioEnhanceChainDynamic_005
 * tc.desc   : Test ReleaseAudioEnhanceChainDynamic interface after correctly creating two enhanceChain.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ReleaseAudioEnhanceChainDynamic_005, TestSize.Level1)
{
    uint32_t validKeyCode = VALID_SCENEKEY_CODE;
    manager_->sceneTypeToEnhanceChainCountMap_= {{VALID_SCENEKEY_CODE, 2}};
    std::string scene = "SCENE_RECORD";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;
std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>(scene,
        algoParam, deviceAttr, defaultFlag);
    manager_->sceneTypeToEnhanceChainMap_.insert_or_assign(validKeyCode, audioEnhanceChain);
    int32_t result = manager_->ReleaseAudioEnhanceChainDynamic(validKeyCode);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test ReleaseAudioEnhanceChainDynamic API
 * tc.number : ReleaseAudioEnhanceChainDynamic_006
 * tc.desc   : Test ReleaseAudioEnhanceChainDynamic interface after correctly creating two enhanceChain.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ReleaseAudioEnhanceChainDynamic_006, TestSize.Level1)
{
    uint32_t validKeyCode = VALID_SCENEKEY_CODE;
    manager_->sceneTypeToEnhanceChainCountMap_= {{VALID_SCENEKEY_CODE, 1}};
    std::string scene = "SCENE_RECORD";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;
std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>(scene,
        algoParam, deviceAttr, defaultFlag);
    manager_->sceneTypeToEnhanceChainMap_.insert_or_assign(validKeyCode, audioEnhanceChain);
    int32_t result = manager_->ReleaseAudioEnhanceChainDynamic(validKeyCode);
    EXPECT_EQ(result, SUCCESS);
}
/*
 * tc.name   : Test ReleaseAudioEnhanceChainDynamic API
 * tc.number : ReleaseAudioEnhanceChainDynamic_007
 * tc.desc   : Test ReleaseAudioEnhanceChainDynamic interface after correctly creating two enhanceChain.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ReleaseAudioEnhanceChainDynamic_007, TestSize.Level1)
{
    uint32_t validKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validKeyCode, deviceAttr);
    manager_->sceneTypeAndModeToEnhanceChainNameMap_.clear();
    int32_t result = manager_->ReleaseAudioEnhanceChainDynamic(validKeyCode);
    EXPECT_EQ(result, ERROR);
}

/*
 * tc.name   : Test ExistAudioEnhanceChain API
 * tc.number : ExistAudioEnhanceChain_001
 * tc.desc   : Test ExistAudioEnhanceChain interface with a nonexistent ehanceChain.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ExistAudioEnhanceChain_001, TestSize.Level1)
{
    uint32_t validKeyCode = VALID_SCENEKEY_CODE;
    uint32_t invalidKeyCode = INVALID_SCENEKEY_CODE;

    manager_->CreateAudioEnhanceChainDynamic(validKeyCode, deviceAttr);
    bool result = manager_->ExistAudioEnhanceChain(invalidKeyCode);
    EXPECT_EQ(result, false);
}

/*
 * tc.name   : Test ExistAudioEnhanceChain API
 * tc.number : ExistAudioEnhanceChain_002
 * tc.desc   : Test ExistAudioEnhanceChain interface after correctly creating an ehanceChain.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ExistAudioEnhanceChain_002, TestSize.Level1)
{
    uint32_t validKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validKeyCode, deviceAttr);
    manager_->ExistAudioEnhanceChain(validKeyCode);
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
    uint32_t validKeyCode = VALID_SCENEKEY_CODE;
    uint32_t invalidKeyCode = INVALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validKeyCode, deviceAttr);

    int32_t result = manager_->AudioEnhanceChainGetAlgoConfig(invalidKeyCode, micConfig, ecConfig, micRefConfig);
    EXPECT_EQ(result, ERROR);
}

/*
 * tc.name   : Test AudioEnhanceChainGetAlgoConfig API
 * tc.number : AudioEnhanceChainGetAlgoConfig_002
 * tc.desc   : Test AudioEnhanceChainGetAlgoConfig interface with a valid scene key code.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, AudioEnhanceChainGetAlgoConfig_002, TestSize.Level1)
{
    AudioBufferConfig micConfig = {};
    AudioBufferConfig ecConfig = {};
    AudioBufferConfig micRefConfig = {};
    uint32_t validKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validKeyCode, deviceAttr);
    manager_->AudioEnhanceChainGetAlgoConfig(validKeyCode, micConfig, ecConfig, micRefConfig);
}

/*
 * tc.name   : Test IsEmptyEnhanceChain API
 * tc.number : IsEmptyEnhanceChain_001
 * tc.desc   : Ensures the function returns true when there are no audio enhance chains.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, IsEmptyEnhanceChain_001, TestSize.Level1)
{
    manager_->isInitialized_ = true;
    bool result = manager_->IsEmptyEnhanceChain();
    EXPECT_EQ(result, true);
}

/*
 * tc.name   : Test IsEmptyEnhanceChain API
 * tc.number : IsEmptyEnhanceChain_002
 * tc.desc   : Ensures the function returns false when there is at least one audio enhance chain.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, IsEmptyEnhanceChain_002, TestSize.Level1)
{
    uint32_t validKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validKeyCode, deviceAttr);
    manager_->IsEmptyEnhanceChain();
}

/*
 * tc.name   : Test IsEmptyEnhanceChain API
 * tc.number : IsEmptyEnhanceChain_003
 * tc.desc   : Ensures the function returns false when there is at least one audio enhance chain.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, IsEmptyEnhanceChain_003, TestSize.Level1)
{
    manager_->isInitialized_ = false;
    bool result = manager_->IsEmptyEnhanceChain();
    EXPECT_EQ(result, true);
}

/*
 * tc.name   : Test IsEmptyEnhanceChain API
 * tc.number : IsEmptyEnhanceChain_004
 * tc.desc   : Ensures the function returns false when there is at least one audio enhance chain.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, IsEmptyEnhanceChain_005, TestSize.Level1)
{
    manager_->isInitialized_ = true;
    manager_->sceneTypeAndModeToEnhanceChainNameMap_.clear();
    bool result = manager_->IsEmptyEnhanceChain();
    EXPECT_EQ(result, true);
}

/*
 * tc.name   : Test IsEmptyEnhanceChain API
 * tc.number : IsEmptyEnhanceChain_004
 * tc.desc   : Ensures the function returns false when there is at least one audio enhance chain.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, IsEmptyEnhanceChain_006, TestSize.Level1)
{
    uint32_t validKeyCode = VALID_SCENEKEY_CODE;
    std::string scene = "SCENE_RECORD";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>(scene, algoParam, deviceAttr, defaultFlag);
    manager_->isInitialized_ = true;
    manager_->sceneTypeToEnhanceChainMap_.insert_or_assign(validKeyCode, audioEnhanceChain);
    bool result = manager_->IsEmptyEnhanceChain();
    EXPECT_EQ(result, true);
}

/*
 * tc.name   : Test InitEnhanceBuffer API
 * tc.number : InitEnhanceBuffer_001
 * tc.desc   : Test InitEnhanceBuffer interface
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, InitEnhanceBuffer_001, TestSize.Level1)
{
    uint32_t validKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validKeyCode, deviceAttr);
    int32_t result = manager_->InitEnhanceBuffer();
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test CopyToEnhanceBuffer API
 * tc.number : CopyToEnhanceBuffer_001
 * tc.desc   : Ensures data is correctly copied to the internal buffer when it is initialized.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, CopyToEnhanceBuffer_001, TestSize.Level1)
{
    uint32_t validKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validKeyCode, deviceAttr);
    manager_->InitEnhanceBuffer();

    uint32_t bufferSize = VALID_BUFFER_SIZE;
    std::vector<uint8_t> dummyData(bufferSize, 0xAA);
    int32_t result = manager_->CopyToEnhanceBuffer(dummyData.data(), bufferSize);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test CopyToEnhanceBuffer API
 * tc.number : CopyToEnhanceBuffer_002
 * tc.desc   : Ensures the function returns an error when the internal buffer is not initialized.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, CopyToEnhanceBuffer_002, TestSize.Level1)
{
    uint32_t bufferSize = VALID_BUFFER_SIZE;
    std::vector<uint8_t> dummyData(bufferSize, 0xBB);
    int32_t result = manager_->CopyToEnhanceBuffer(dummyData.data(), bufferSize);
    EXPECT_EQ(result, ERROR);
}

/*
 * tc.name   : Test CopyToEnhanceBuffer API
 * tc.number : CopyToEnhanceBuffer_003
 * tc.desc   : Ensures the function returns an error when trying to copy more data than the buffer can hold.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, CopyToEnhanceBuffer_003, TestSize.Level1)
{
    uint32_t validKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validKeyCode, deviceAttr);
    manager_->InitEnhanceBuffer();

    uint32_t bufferSize = INVALID_BUFFER_SIZE;
    std::vector<uint8_t> oversizedData(bufferSize, 0xAA);
    int32_t result = manager_->CopyToEnhanceBuffer(oversizedData.data(), oversizedData.size());
    EXPECT_EQ(result, ERROR);
}

/*
 * tc.name   : Test CopyFromEnhanceBuffer API
 * tc.number : CopyFromEnhanceBuffer_001
 * tc.desc   : Ensures the function returns an error when the internal buffer is not initialized.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, CopyFromEnhanceBuffer_001, TestSize.Level1)
{
    uint32_t bufferSize = VALID_BUFFER_SIZE;
    std::vector<uint8_t> outputData(bufferSize);
    int32_t result = manager_->CopyFromEnhanceBuffer(outputData.data(), outputData.size());
    EXPECT_EQ(result, ERROR);
}

/*
 * tc.name   : Test CopyFromEnhanceBuffer API
 * tc.number : CopyFromEnhanceBuffer_002
 * tc.desc   : Ensures the function returns an error when trying to copy more data than the buffer contains.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, CopyFromEnhanceBuffer_002, TestSize.Level1)
{
    uint32_t validKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validKeyCode, deviceAttr);
    manager_->InitEnhanceBuffer();

    uint32_t bufferSize = INVALID_BUFFER_SIZE;
    std::vector<uint8_t> outputData(bufferSize);
    int32_t result = manager_->CopyFromEnhanceBuffer(outputData.data(), outputData.size());
    EXPECT_EQ(result, ERROR);
}

/*
 * tc.name   : Test CopyFromEnhanceBuffer API
 * tc.number : CopyFromEnhanceBuffer_003
 * tc.desc   : Ensures the function returns an error when the data pointer is null.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, CopyFromEnhanceBuffer_003, TestSize.Level1)
{
    uint32_t validKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validKeyCode, deviceAttr);
    manager_->InitEnhanceBuffer();

    uint32_t bufferSize = VALID_BUFFER_SIZE;
    int8_t* data = nullptr;
    int32_t result = manager_->CopyFromEnhanceBuffer(data, bufferSize);
    EXPECT_EQ(result, ERROR);
}

/*
 * tc.name   : Test CopyFromEnhanceBuffer API
 * tc.number : CopyFromEnhanceBuffer_004
 * tc.desc   : Ensures the function successfully copies data from the internal buffer under normal conditions.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, CopyFromEnhanceBuffer_004, TestSize.Level1)
{
    uint32_t validKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validKeyCode, deviceAttr);
    manager_->InitEnhanceBuffer();

    uint32_t bufferSize = VALID_BUFFER_SIZE;
    std::vector<uint8_t> dummyData(bufferSize, 0xAA);
    manager_->CopyToEnhanceBuffer(dummyData.data(), dummyData.size());
    manager_->ApplyAudioEnhanceChain(validKeyCode, dummyData.size());

    std::vector<uint8_t> outputData(bufferSize);
    int32_t result = manager_->CopyFromEnhanceBuffer(outputData.data(), outputData.size());
    EXPECT_EQ(result, SUCCESS);
}


/*
 * tc.name   : Test ApplyAudioEnhanceChain API
 * tc.number : ApplyAudioEnhanceChain_001
 * tc.desc   : Ensures the function returns an error when the scene key does not exist in the map.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ApplyAudioEnhanceChain_001, TestSize.Level1)
{
    uint32_t validSceneKeyCode = VALID_SCENEKEY_CODE;
    uint32_t invalidSceneKeyCode = INVALID_SCENEKEY_CODE;
    uint32_t bufferSize = VALID_BUFFER_SIZE;
    manager_->CreateAudioEnhanceChainDynamic(validSceneKeyCode, deviceAttr);
    manager_->InitEnhanceBuffer();

    std::vector<uint8_t> dummyData(bufferSize, 0x00);
    manager_->CopyToEnhanceBuffer(dummyData.data(), dummyData.size());
    int32_t result = manager_->ApplyAudioEnhanceChain(invalidSceneKeyCode, dummyData.size());
    EXPECT_EQ(result, ERROR);
}

/*
 * tc.name   : Test ApplyAudioEnhanceChain API
 * tc.number : ApplyAudioEnhanceChain_002
 * tc.desc   : Test ApplyAudioEnhanceChain interface when the input length exceeds the buffer capacity.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ApplyAudioEnhanceChain_002, TestSize.Level1)
{
    uint32_t validSceneKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validSceneKeyCode, deviceAttr);
    manager_->InitEnhanceBuffer();

    uint32_t bufferSize = INVALID_BUFFER_SIZE;
    std::vector<uint8_t> dummyData(bufferSize, 0x00);
    manager_->CopyToEnhanceBuffer(dummyData.data(), dummyData.size());
    manager_->ApplyAudioEnhanceChain(validSceneKeyCode, dummyData.size());
}

/*
 * tc.name   : Test ApplyAudioEnhanceChain API
 * tc.number : ApplyAudioEnhanceChain_003
 * tc.desc   : Ensures the function successfully applies the audio enhance chain under normal conditions.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ApplyAudioEnhanceChain_003, TestSize.Level1)
{
    uint32_t validSceneKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validSceneKeyCode, deviceAttr);
    manager_->InitEnhanceBuffer();

    uint32_t bufferSize = VALID_BUFFER_SIZE;
    std::vector<uint8_t> dummyData(bufferSize, 0xAA);
    manager_->CopyToEnhanceBuffer(dummyData.data(), dummyData.size());
    manager_->ApplyAudioEnhanceChain(validSceneKeyCode, dummyData.size());
}

/*
 * tc.name   : Test SetInputDevice API
 * tc.number : SetInputDevice_001
 * tc.desc   : Ensures the function sets a new input device correctly and returns success.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetInputDevice_001, TestSize.Level1)
{
    uint32_t captureId = 13;
    DeviceType newDeviceType = DEVICE_TYPE_MIC;
    int32_t result = manager_->SetInputDevice(captureId, newDeviceType);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test SetInputDevice API
 * tc.number : SetInputDevice_002
 * tc.desc   : Ensures the function recognizes the same input device setting and returns success without redundancy.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetInputDevice_002, TestSize.Level1)
{
    uint32_t captureId = 13;
    DeviceType deviceType = DEVICE_TYPE_MIC;

    manager_->SetInputDevice(captureId, deviceType);
    int32_t result = manager_->SetInputDevice(captureId, deviceType);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test SetInputDevice API
 * tc.number : SetInputDevice_003
 * tc.desc   : Ensures the function recognizes different input device setting and returns success without redundancy.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetInputDevice_003, TestSize.Level1)
{
    uint32_t captureId = 13;
    DeviceType deviceType = DEVICE_TYPE_MIC;

    manager_->SetInputDevice(captureId, deviceType);
    deviceType = DEVICE_TYPE_USB_HEADSET;
    int32_t result = manager_->SetInputDevice(captureId, deviceType);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test SetInputDevice API
 * tc.number : SetInputDevice_004
 * tc.desc   : Ensure that the function, when handling input devices,
 * is able to correctly identify and return failure when setting an incorrect input device
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetInputDevice_004, TestSize.Level1)
{
    uint32_t captureId = 13;
    DeviceType deviceType = DEVICE_TYPE_MIC;

    manager_->SetInputDevice(captureId, deviceType);
    deviceType = DEVICE_TYPE_REMOTE_CAST;
    int32_t result = manager_->SetInputDevice(captureId, deviceType);
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
 * tc.name   : Test SetMicrophoneMuteInfo API
 * tc.number : SetMicrophoneMuteInfo_001
 * tc.desc   : Ensures the function correctly sets the microphone to mute.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetMicrophoneMuteInfo_001, TestSize.Level1)
{
    bool isMute = true;
    uint32_t validKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validKeyCode, deviceAttr);
    int32_t result = manager_->SetMicrophoneMuteInfo(isMute);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test SetMicrophoneMuteInfo API
 * tc.number : SetMicrophoneMuteInfo_002
 * tc.desc   : Ensures the function correctly sets the microphone to unmute.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetMicrophoneMuteInfo_002, TestSize.Level1)
{
    bool isMute = false;
    uint32_t validKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validKeyCode, deviceAttr);
    int32_t result = manager_->SetMicrophoneMuteInfo(isMute);
    EXPECT_EQ(result, SUCCESS);
}


/*
 * tc.name   : Test SetStreamVolumeInfo API
 * tc.number : SetStreamVolumeInfo_001
 * tc.desc   : Ensures the function correctly sets the stream volume for a valid session.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetStreamVolumeInfo_001, TestSize.Level1)
{
    uint32_t sessionId = 1;
    float streamVolume = 0.5f;
    int32_t result = manager_->SetStreamVolumeInfo(sessionId, streamVolume);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test SetStreamVolumeInfo API
 * tc.number : SetStreamVolumeInfo_002
 * tc.desc   : Ensures the function correctly sets the stream volume for a valid session.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetStreamVolumeInfo_002, TestSize.Level1)
{
    uint32_t sessionId = 1;
    float streamVolume = 0.5f;
    uint32_t validSceneKeyCode = VALID_SCENEKEY_CODE;
    std::string scene = "SCENE_RECORD";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;
std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>(scene,
        algoParam, deviceAttr, defaultFlag);
    manager_->sceneTypeToEnhanceChainMap_.insert_or_assign(validSceneKeyCode, audioEnhanceChain);
    manager_->sceneTypeAndModeToEnhanceChainNameMap_.insert_or_assign("test", "test");
    int32_t result = manager_->SetStreamVolumeInfo(sessionId, streamVolume);
    EXPECT_EQ(result, SUCCESS);
}
/*
 * tc.name   : Test SetAudioEnhanceProperty API
 * tc.number : SetAudioEnhanceProperty_001
 * tc.desc   : Ensures the function handles an empty property array correctly.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetAudioEnhanceProperty_001, TestSize.Level1)
{
    uint32_t validSceneKeyCode = VALID_SCENEKEY_CODE;
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
    uint32_t validSceneKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validSceneKeyCode, deviceAttr);

    AudioEnhancePropertyArray propertyArray;
    propertyArray.property.push_back({"record", "123"});
    manager_->SetAudioEnhanceProperty(propertyArray);
}

/*
 * tc.name   : Test SetAudioEnhanceProperty API
 * tc.number : SetAudioEnhanceProperty_003
 * tc.desc   : Tests setting a property with invalid enhanceClass and valid enhanceProp.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetAudioEnhanceProperty_003, TestSize.Level1)
{
    uint32_t validSceneKeyCode = VALID_SCENEKEY_CODE;
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
    uint32_t validSceneKeyCode = VALID_SCENEKEY_CODE;
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
    uint32_t validSceneKeyCode = VALID_SCENEKEY_CODE;
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
    uint32_t validSceneKeyCode = VALID_SCENEKEY_CODE;
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
    uint32_t validSceneKeyCode = VALID_SCENEKEY_CODE;
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

/*
 * tc.name   : Test ApplyAudioEnhanceChainDefault API
 * tc.number : ApplyAudioEnhanceChainDefault_001
 * tc.desc   : Tests retrieving a property list when properties are set.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ApplyAudioEnhanceChainDefault_001, TestSize.Level1)
{
    AudioEnhancePropertyArray propertiesToSet;
    uint32_t validSceneKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validSceneKeyCode, deviceAttr);
    manager_->InitEnhanceBuffer();
    manager_->SetAudioEnhanceProperty(propertiesToSet);
    manager_->enhanceBuffer_ = std::make_unique<EnhanceBuffer>();
    manager_->enhanceBuffer_->micBufferIn.resize(1024);
    manager_->enhanceBuffer_->micBufferOut.resize(1024);
    manager_->enhanceBuffer_->ecBuffer.resize(1024);
    manager_->enhanceBuffer_->micRefBuffer.resize(1024);
    uint32_t captureId = 1;
    uint32_t length = 1024;
    int32_t result = manager_->ApplyAudioEnhanceChainDefault(captureId, length);
    EXPECT_EQ(result, ERROR);
}

/*
 * tc.name   : Test ApplyAudioEnhanceChainDefault API
 * tc.number : ApplyAudioEnhanceChainDefault_002
 * tc.desc   : Tests retrieving a property list when properties are set.
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ApplyAudioEnhanceChainDefault_002, TestSize.Level1)
{
    AudioEnhancePropertyArray propertiesToSet;
    uint32_t validSceneKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validSceneKeyCode, deviceAttr);
    manager_->InitEnhanceBuffer();
    manager_->SetAudioEnhanceProperty(propertiesToSet);
    manager_->enhanceBuffer_ = std::make_unique<EnhanceBuffer>();
    manager_->enhanceBuffer_->micBufferIn.resize(1024);
    manager_->enhanceBuffer_->micBufferOut.resize(1024);
    manager_->enhanceBuffer_->ecBuffer.resize(1024);
    manager_->enhanceBuffer_->micRefBuffer.resize(1024);
    uint32_t captureId = 1;
    uint32_t length = 1024;
    std::string scene = "SCENE_RECORD";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;
std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>(scene,
        algoParam, deviceAttr, defaultFlag);
    manager_->captureId2DefaultChain_.insert_or_assign(captureId, audioEnhanceChain);
    int32_t result = manager_->ApplyAudioEnhanceChainDefault(captureId, length);
    EXPECT_EQ(result, ERROR);
}
/*
 * tc.name   : Test InitEnhanceBuffer API
 * tc.number : InitEnhanceBuffer_002
 * tc.desc   : Test InitEnhanceBuffer interface
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, InitEnhanceBuffer_002, TestSize.Level1)
{
    AudioEnhancePropertyArray propertiesToSet;
    uint32_t validKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validKeyCode, deviceAttr);
    manager_->SetAudioEnhanceProperty(propertiesToSet);
    manager_->enhanceBuffer_ = std::make_unique<EnhanceBuffer>();
    manager_->enhanceBuffer_->micBufferIn.resize(0);
    manager_->enhanceBuffer_->micBufferOut.resize(0);
    manager_->enhanceBuffer_->ecBuffer.resize(0);
    manager_->enhanceBuffer_->micRefBuffer.resize(0);
    int32_t result = manager_->InitEnhanceBuffer();
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test SendInitCommand API
 * tc.number : SendInitCommand_001
 * tc.desc   : Test ISendInitCommand interface
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SendInitCommand_001, TestSize.Level1)
{
    uint32_t validSceneKeyCode = VALID_SCENEKEY_CODE;
    manager_->CreateAudioEnhanceChainDynamic(validSceneKeyCode, deviceAttr);
    
    int32_t result = manager_->SendInitCommand();
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test DeleteEnhanceChainInner API
 * tc.number : DeleteEnhanceChainInner_001
 * tc.desc   : Test DeleteEnhanceChainInner interface
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, DeleteEnhanceChainInner_001, TestSize.Level1)
{
    uint32_t invalidSceneKeyCode = VALID_SCENEKEY_CODE * 16;
    std::string scene = "SCENE_RECORD";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>(scene,
        algoParam, deviceAttr, defaultFlag);
    int32_t result = manager_->DeleteEnhanceChainInner(audioEnhanceChain, invalidSceneKeyCode);
    EXPECT_EQ(result, ERROR);
}

/*
 * tc.name   : Test DeleteEnhanceChainInner API
 * tc.number : DeleteEnhanceChainInner_002
 * tc.desc   : Test DeleteEnhanceChainInner interface
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, DeleteEnhanceChainInner_002, TestSize.Level1)
{
    uint32_t validSceneKeyCode = VALID_SCENEKEY_CODE;
    std::string scene = "SCENE_RECORD";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;
std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>(scene,
        algoParam, deviceAttr, defaultFlag);
    int32_t result = manager_->DeleteEnhanceChainInner(audioEnhanceChain, validSceneKeyCode);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test DeleteEnhanceChainInner API
 * tc.number : DeleteEnhanceChainInner_003
 * tc.desc   : Test DeleteEnhanceChainInner interface
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, DeleteEnhanceChainInner_003, TestSize.Level1)
{
    uint32_t validSceneKeyCode = VALID_SCENEKEY_CODE;
    std::string scene = "SCENE_RECORD";
    manager_->priorSceneSet_.insert(scene);
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;
std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>(scene,
        algoParam, deviceAttr, defaultFlag);
    int32_t result = manager_->DeleteEnhanceChainInner(audioEnhanceChain, validSceneKeyCode);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test SetAudioEnhancePropertyToChains API
 * tc.number : SetAudioEnhancePropertyToChains_001
 * tc.desc   : Test SetAudioEnhancePropertyToChains interface
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetAudioEnhancePropertyToChains_001, TestSize.Level1)
{
    AudioEnhanceProperty propert;
    propert.enhanceClass = "effect1";
    propert.enhanceProp = "property1";
    int32_t result = manager_->SetAudioEnhancePropertyToChains(propert);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test SetAudioEnhancePropertyToChains API
 * tc.number : SetAudioEnhancePropertyToChains_002
 * tc.desc   : Test SetAudioEnhancePropertyToChains interface
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetAudioEnhancePropertyToChains_002, TestSize.Level1)
{
    uint32_t validSceneKeyCode = VALID_SCENEKEY_CODE;
    AudioEnhanceProperty propert;
    propert.enhanceClass = "effect1";
    propert.enhanceProp = "property1";
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain ;
    
    manager_->sceneTypeToEnhanceChainMap_.insert_or_assign(validSceneKeyCode, audioEnhanceChain);
    int32_t result = manager_->SetAudioEnhancePropertyToChains(propert);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test SetAudioEnhancePropertyToChains API
 * tc.number : SetAudioEnhancePropertyToChains_003
 * tc.desc   : Test SetAudioEnhancePropertyToChains interface
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetAudioEnhancePropertyToChains_003, TestSize.Level1)
{
    uint32_t validSceneKeyCode = VALID_SCENEKEY_CODE;
    AudioEnhanceProperty propert;
    propert.enhanceClass = "effect1";
    propert.enhanceProp = "property1";
    std::string scene = "SCENE_RECORD";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;
std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>(scene,
        algoParam, deviceAttr, defaultFlag);
    manager_->sceneTypeToEnhanceChainMap_.insert_or_assign(validSceneKeyCode, audioEnhanceChain);
    int32_t result = manager_->SetAudioEnhancePropertyToChains(propert);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test SetAudioEnhancePropertyToChains API
 * tc.number : SetAudioEnhancePropertyToChains_004
 * tc.desc   : Test SetAudioEnhancePropertyToChains interface
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetAudioEnhancePropertyToChains_004, TestSize.Level1)
{
    AudioEffectPropertyV3 propert;
    propert.name = "effect1";
    propert.category = "property1";
    int32_t result = manager_->SetAudioEnhancePropertyToChains(propert);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test SetAudioEnhancePropertyToChains API
 * tc.number : SetAudioEnhancePropertyToChains_005
 * tc.desc   : Test SetAudioEnhancePropertyToChains interface
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetAudioEnhancePropertyToChains_005, TestSize.Level1)
{
    uint32_t validSceneKeyCode = VALID_SCENEKEY_CODE;
    AudioEffectPropertyV3 propert;
    propert.name = "effect1";
    propert.category = "property1";
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain ;
    manager_->sceneTypeToEnhanceChainMap_.insert_or_assign(validSceneKeyCode, audioEnhanceChain);
    int32_t result = manager_->SetAudioEnhancePropertyToChains(propert);
    EXPECT_EQ(result, SUCCESS);
}

/*
 * tc.name   : Test SetAudioEnhancePropertyToChains API
 * tc.number : SetAudioEnhancePropertyToChains_006
 * tc.desc   : Test SetAudioEnhancePropertyToChains interface
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, SetAudioEnhancePropertyToChains_006, TestSize.Level1)
{
    uint32_t validSceneKeyCode = VALID_SCENEKEY_CODE;
    AudioEffectPropertyV3 propert;
    propert.name = "effect1";
    propert.category = "property1";
    std::string scene = "SCENE_RECORD";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;
std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = std::make_shared<AudioEnhanceChain>(scene,
        algoParam, deviceAttr, defaultFlag);
    manager_->sceneTypeToEnhanceChainMap_.insert_or_assign(validSceneKeyCode, audioEnhanceChain);
    int32_t result = manager_->SetAudioEnhancePropertyToChains(propert);
    EXPECT_EQ(result, SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS