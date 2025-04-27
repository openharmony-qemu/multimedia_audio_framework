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

#ifndef LOG_TAG
#define LOG_TAG "AudioEnhanceChainAdapterUnitTest"
#endif

#include "audio_enhance_chain_adapter_unit_test.h"

#include <chrono>
#include <thread>
#include <fstream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "audio_effect.h"
#include "audio_enhance_chain_manager.h"
#include "audio_errors.h"

using namespace std;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {

namespace {
    const uint64_t VALID_SCENEKEY = 4563402752;
    const uint64_t INVALID_SCENEKEY = 0;
    const int32_t VALID_CAPTUREID = 4096;
    const int32_t INVALID_CAPTUREID = 0;
    const int32_t DEFAULT_RATE = 48000;
    const int32_t DEFAULT_CHANNEL = 4;
    const int32_t DEFAULT_FORMAT = 1;
    const int32_t VALID_BUFFER_SIZE = 1000;
    const int32_t MAX_EXTRA_NUM = 3;

    AudioEnhanceChainManager* manager = nullptr;
    std::vector<EffectChain> enhanceChains;
    EffectChainManagerParam managerParam;
    std::vector<std::shared_ptr<AudioEffectLibEntry>> enhanceLibraryList;
    struct DeviceAttrAdapter validAdapter;
}

void AudioEnhanceChainAdapterUnitTest::SetUpTestCase(void)
{
    EffectChain testChain;
    testChain.name = "EFFECTCHAIN_RECORD";
    testChain.apply = {"record"};
    enhanceChains.emplace_back(testChain);

    managerParam.maxExtraNum = MAX_EXTRA_NUM;
    managerParam.defaultSceneName = "SCENE_DEFAULT";
    managerParam.priorSceneList = {};
    managerParam.sceneTypeToChainNameMap = {{"SCENE_RECORD_&_ENHANCE_DEFAULT_&_DEVICE_TYPE_MIC", "EFFECTCHAIN_RECORD"}};
    managerParam.effectDefaultProperty = {
        {"effect1", "property1"}, {"effect2", "property2"}, {"effect3", "property3"}
    };
    enhanceLibraryList = {};

    validAdapter = {DEFAULT_RATE, DEFAULT_CHANNEL, DEFAULT_FORMAT, true,
        DEFAULT_RATE, DEFAULT_CHANNEL, DEFAULT_FORMAT, true, DEFAULT_RATE, DEFAULT_CHANNEL, DEFAULT_FORMAT};
}

void AudioEnhanceChainAdapterUnitTest::TearDownTestCase(void) {}

void AudioEnhanceChainAdapterUnitTest::SetUp(void)
{
    manager = AudioEnhanceChainManager::GetInstance();
    manager->InitAudioEnhanceChainManager(enhanceChains, managerParam, enhanceLibraryList);
}

void AudioEnhanceChainAdapterUnitTest::TearDown(void)
{
    manager = AudioEnhanceChainManager::GetInstance();
    manager->ResetInfo();
}

/**
* @tc.name   : Test EnhanceChainManagerCreateCb API
* @tc.number : EnhanceChainManagerCreateCb_001
* @tc.desc   : Test EnhanceChainManagerCreateCb interface with invalid scene key code.
*/
HWTEST_F(AudioEnhanceChainAdapterUnitTest, EnhanceChainManagerCreateCb_001, TestSize.Level0)
{
    int32_t result = EnhanceChainManagerCreateCb(INVALID_SCENEKEY, &validAdapter);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EnhanceChainManagerCreateCb API
* @tc.number : EnhanceChainManagerCreateCb_002
* @tc.desc   : Test EnhanceChainManagerCreateCb interface with valid scene key code.
*/
HWTEST_F(AudioEnhanceChainAdapterUnitTest, EnhanceChainManagerCreateCb_002, TestSize.Level1)
{
    int32_t result = EnhanceChainManagerCreateCb(VALID_SCENEKEY, &validAdapter);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EnhanceChainManagerCreateCb API
* @tc.number : EnhanceChainManagerReleaseCb_001
* @tc.desc   : Test EnhanceChainManagerReleaseCb interface without create.
*/
HWTEST_F(AudioEnhanceChainAdapterUnitTest, EnhanceChainManagerReleaseCb_001, TestSize.Level1)
{
    int32_t result = EnhanceChainManagerReleaseCb(INVALID_SCENEKEY);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test EnhanceChainManagerCreateCb API
* @tc.number : EnhanceChainManagerReleaseCb_002
* @tc.desc   : Test EnhanceChainManagerCreateCb interface with create.
*/
HWTEST_F(AudioEnhanceChainAdapterUnitTest, EnhanceChainManagerReleaseCb_002, TestSize.Level1)
{
    EnhanceChainManagerCreateCb(VALID_SCENEKEY, &validAdapter);
    int32_t result = EnhanceChainManagerReleaseCb(VALID_SCENEKEY);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test EnhanceChainManagerExist API
* @tc.number : EnhanceChainManagerExist_001
* @tc.desc   : Test EnhanceChainManagerExist interface.
*/
HWTEST_F(AudioEnhanceChainAdapterUnitTest, EnhanceChainManagerExist_001, TestSize.Level1)
{
    EnhanceChainManagerCreateCb(VALID_SCENEKEY, &validAdapter);

    bool result = EnhanceChainManagerExist(INVALID_SCENEKEY);
    EXPECT_EQ(false, result);
}

/**
* @tc.name   : Test EnhanceChainManagerExist API
* @tc.number : EnhanceChainManagerExist_002
* @tc.desc   : Test EnhanceChainManagerExist interface.
*/
HWTEST_F(AudioEnhanceChainAdapterUnitTest, EnhanceChainManagerExist_002, TestSize.Level1)
{
    EnhanceChainManagerCreateCb(VALID_SCENEKEY, &validAdapter);

    bool result = EnhanceChainManagerExist(VALID_SCENEKEY);
    EXPECT_EQ(false, result);
}

/**
* @tc.name   : Test EnhanceChainManagerGetAlgoConfig API
* @tc.number : EnhanceChainManagerGetAlgoConfig001
* @tc.desc   : Test EnhanceChainManagerGetAlgoConfig interface.
*/
HWTEST_F(AudioEnhanceChainAdapterUnitTest, EnhanceChainManagerGetAlgoConfig_001, TestSize.Level1)
{
    EnhanceChainManagerCreateCb(VALID_SCENEKEY, &validAdapter);

    pa_sample_spec micSpec;
    pa_sample_spec ecSpec;
    pa_sample_spec micRefSpec;

    pa_sample_spec_init(&micSpec);
    pa_sample_spec_init(&ecSpec);
    pa_sample_spec_init(&micRefSpec);

    int32_t result = EnhanceChainManagerGetAlgoConfig(INVALID_SCENEKEY, &micSpec, &ecSpec, &micRefSpec);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EnhanceChainManagerGetAlgoConfig API
* @tc.number : EnhanceChainManagerGetAlgoConfig002
* @tc.desc   : Test EnhanceChainManagerGetAlgoConfig interface.
*/
HWTEST_F(AudioEnhanceChainAdapterUnitTest, EnhanceChainManagerGetAlgoConfig_002, TestSize.Level1)
{
    EnhanceChainManagerCreateCb(VALID_SCENEKEY, &validAdapter);

    pa_sample_spec micSpec;
    pa_sample_spec ecSpec;
    pa_sample_spec micRefSpec;

    pa_sample_spec_init(&micSpec);
    pa_sample_spec_init(&ecSpec);
    pa_sample_spec_init(&micRefSpec);

    std::string scene = "SCENE_RECORD";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain =
        std::make_shared<AudioEnhanceChain>(scene, algoParam, deviceAttr, defaultFlag);
    manager->sceneTypeToEnhanceChainMap_.insert_or_assign(VALID_SCENEKEY, audioEnhanceChain);
    manager->sceneTypeToEnhanceChainCountMap_.insert_or_assign(VALID_SCENEKEY, 1);
    int32_t result = EnhanceChainManagerGetAlgoConfig(VALID_SCENEKEY, &micSpec, &ecSpec, &micRefSpec);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test EnhanceChainManagerIsEmptyEnhanceChain API
* @tc.number : EnhanceChainManagerIsEmptyEnhanceChain001
* @tc.desc   : Test EnhanceChainManagerIsEmptyEnhanceChain interface.
*/
HWTEST_F(AudioEnhanceChainAdapterUnitTest, EnhanceChainManagerIsEmptyEnhanceChain_001, TestSize.Level1)
{
    EnhanceChainManagerCreateCb(INVALID_SCENEKEY, &validAdapter);

    bool result = EnhanceChainManagerIsEmptyEnhanceChain();
    EXPECT_EQ(true, result);
}

/**
* @tc.name   : Test EnhanceChainManagerIsEmptyEnhanceChain API
* @tc.number : EnhanceChainManagerIsEmptyEnhanceChain002
* @tc.desc   : Test EnhanceChainManagerIsEmptyEnhanceChain interface.
*/
HWTEST_F(AudioEnhanceChainAdapterUnitTest, EnhanceChainManagerIsEmptyEnhanceChain_002, TestSize.Level1)
{
    EnhanceChainManagerCreateCb(VALID_SCENEKEY, &validAdapter);

    bool result = EnhanceChainManagerIsEmptyEnhanceChain();
    EXPECT_EQ(true, result);
}

/**
* @tc.name   : Test EnhanceChainManagerInitEnhanceBuffer API
* @tc.number : EnhanceChainManagerInitEnhanceBuffer001
* @tc.desc   : Test EnhanceChainManagerInitEnhanceBuffer interface.
*/
HWTEST_F(AudioEnhanceChainAdapterUnitTest, EnhanceChainManagerInitEnhanceBuffer_001, TestSize.Level1)
{
    EnhanceChainManagerCreateCb(INVALID_SCENEKEY, &validAdapter);

    int32_t result = EnhanceChainManagerInitEnhanceBuffer();
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EnhanceChainManagerInitEnhanceBuffer API
* @tc.number : EnhanceChainManagerInitEnhanceBuffer002
* @tc.desc   : Test EnhanceChainManagerInitEnhanceBuffer interface.
*/
HWTEST_F(AudioEnhanceChainAdapterUnitTest, EnhanceChainManagerInitEnhanceBuffer_002, TestSize.Level1)
{
    EnhanceChainManagerCreateCb(VALID_SCENEKEY, &validAdapter);

    int32_t result = EnhanceChainManagerInitEnhanceBuffer();
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test CopyToEnhanceBufferAdapter API
* @tc.number : CopyToEnhanceBufferAdapter001
* @tc.desc   : Test CopyToEnhanceBufferAdapter interface.
*/
HWTEST_F(AudioEnhanceChainAdapterUnitTest, CopyToEnhanceBufferAdapter_001, TestSize.Level1)
{
    void *data = nullptr;
    uint32_t length = 0;
    int32_t result = CopyToEnhanceBufferAdapter(data, length);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test CopyToEnhanceBufferAdapter API
* @tc.number : CopyToEnhanceBufferAdapter002
* @tc.desc   : Test CopyToEnhanceBufferAdapter interface.
*/
HWTEST_F(AudioEnhanceChainAdapterUnitTest, CopyToEnhanceBufferAdapter_002, TestSize.Level1)
{
    EnhanceChainManagerCreateCb(VALID_SCENEKEY, &validAdapter);
    EnhanceChainManagerInitEnhanceBuffer();

    uint32_t bufferSize = VALID_BUFFER_SIZE;
    std::vector<uint8_t> dummyData(bufferSize, 0xAA);
    int32_t result = CopyToEnhanceBufferAdapter(dummyData.data(), bufferSize);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test CopyEcdataToEnhanceBufferAdapter API
* @tc.number : CopyEcdataToEnhanceBufferAdapter001
* @tc.desc   : Test CopyEcdataToEnhanceBufferAdapter interface.
*/
HWTEST_F(AudioEnhanceChainAdapterUnitTest, CopyEcdataToEnhanceBufferAdapter_001, TestSize.Level1)
{
    void *data = nullptr;
    uint32_t length = 0;
    int32_t result = CopyEcdataToEnhanceBufferAdapter(data, length);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test CopyEcdataToEnhanceBufferAdapter API
* @tc.number : CopyEcdataToEnhanceBufferAdapter002
* @tc.desc   : Test CopyEcdataToEnhanceBufferAdapter interface.
*/
HWTEST_F(AudioEnhanceChainAdapterUnitTest, CopyEcdataToEnhanceBufferAdapter_002, TestSize.Level1)
{
    EnhanceChainManagerCreateCb(VALID_SCENEKEY, &validAdapter);
    EnhanceChainManagerInitEnhanceBuffer();

    uint32_t bufferSize = VALID_BUFFER_SIZE;
    std::vector<uint8_t> dummyData(bufferSize, 0xAA);
    int32_t result = CopyEcdataToEnhanceBufferAdapter(dummyData.data(), bufferSize);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test CopyMicRefdataToEnhanceBufferAdapter API
* @tc.number : CopyMicRefdataToEnhanceBufferAdapter001
* @tc.desc   : Test CopyMicRefdataToEnhanceBufferAdapter interface.
*/
HWTEST_F(AudioEnhanceChainAdapterUnitTest, CopyMicRefdataToEnhanceBufferAdapter_001, TestSize.Level1)
{
    void *data = nullptr;
    uint32_t length = 0;
    int32_t result = CopyMicRefdataToEnhanceBufferAdapter(data, length);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test CopyMicRefdataToEnhanceBufferAdapter API
* @tc.number : CopyMicRefdataToEnhanceBufferAdapter002
* @tc.desc   : Test CopyMicRefdataToEnhanceBufferAdapter interface.
*/
HWTEST_F(AudioEnhanceChainAdapterUnitTest, CopyMicRefdataToEnhanceBufferAdapter_002, TestSize.Level1)
{
    EnhanceChainManagerCreateCb(VALID_SCENEKEY, &validAdapter);
    EnhanceChainManagerInitEnhanceBuffer();

    uint32_t bufferSize = VALID_BUFFER_SIZE;
    std::vector<uint8_t> dummyData(bufferSize, 0xAA);
    int32_t result = CopyMicRefdataToEnhanceBufferAdapter(dummyData.data(), bufferSize);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test CopyFromEnhanceBufferAdapter API
* @tc.number : CopyFromEnhanceBufferAdapter001
* @tc.desc   : Test CopyFromEnhanceBufferAdapter interface.
*/
HWTEST_F(AudioEnhanceChainAdapterUnitTest, CopyFromEnhanceBufferAdapter_001, TestSize.Level1)
{
    uint32_t bufferSize = VALID_BUFFER_SIZE;
    std::vector<uint8_t> dummyData(bufferSize);
    int32_t result = CopyFromEnhanceBufferAdapter(dummyData.data(), bufferSize);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test CopyFromEnhanceBufferAdapter API
* @tc.number : CopyFromEnhanceBufferAdapter002
* @tc.desc   : Test CopyFromEnhanceBufferAdapter interface.
*/
HWTEST_F(AudioEnhanceChainAdapterUnitTest, CopyFromEnhanceBufferAdapter_002, TestSize.Level1)
{
    EnhanceChainManagerCreateCb(VALID_SCENEKEY, &validAdapter);
    EnhanceChainManagerInitEnhanceBuffer();

    uint32_t bufferSize = VALID_BUFFER_SIZE;
    std::vector<uint8_t> dummyData(bufferSize, 0xAA);
    CopyToEnhanceBufferAdapter(dummyData.data(), bufferSize);

    std::vector<uint8_t> outputData(bufferSize);
    int32_t result = CopyFromEnhanceBufferAdapter(outputData.data(), outputData.size());
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EnhanceChainManagerProcess API
* @tc.number : EnhanceChainManagerProcess001
* @tc.desc   : Test EnhanceChainManagerProcess interface.
*/
HWTEST_F(AudioEnhanceChainAdapterUnitTest, EnhanceChainManagerProcess_001, TestSize.Level1)
{
    EnhanceChainManagerCreateCb(VALID_SCENEKEY, &validAdapter);
    EnhanceChainManagerInitEnhanceBuffer();

    uint32_t bufferSize = VALID_BUFFER_SIZE;
    std::vector<uint8_t> dummyData(bufferSize, 0xAA);
    CopyToEnhanceBufferAdapter(dummyData.data(), bufferSize);

    int32_t result = EnhanceChainManagerProcess(INVALID_SCENEKEY, bufferSize);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EnhanceChainManagerProcess API
* @tc.number : EnhanceChainManagerProcess002
* @tc.desc   : Test EnhanceChainManagerProcess interface.
*/
HWTEST_F(AudioEnhanceChainAdapterUnitTest, EnhanceChainManagerProcess_002, TestSize.Level1)
{
    EnhanceChainManagerCreateCb(VALID_SCENEKEY, &validAdapter);
    EnhanceChainManagerInitEnhanceBuffer();

    uint32_t bufferSize = VALID_BUFFER_SIZE;
    std::vector<uint8_t> dummyData(bufferSize, 0xAA);
    CopyToEnhanceBufferAdapter(dummyData.data(), bufferSize);

    int32_t result = EnhanceChainManagerProcess(VALID_SCENEKEY, bufferSize);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test GetSceneTypeCode API
* @tc.number : GetSceneTypeCode001
* @tc.desc   : Test GetSceneTypeCode interface.
*/
HWTEST_F(AudioEnhanceChainAdapterUnitTest, GetSceneTypeCode_001, TestSize.Level1)
{
    const char *invalidScene = "NONE";
    uint64_t sceneTypeCode;
    int32_t result = GetSceneTypeCode(invalidScene, &sceneTypeCode);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test GetSceneTypeCode API
* @tc.number : GetSceneTypeCode002
* @tc.desc   : Test GetSceneTypeCode interface.
*/
HWTEST_F(AudioEnhanceChainAdapterUnitTest, GetSceneTypeCode_002, TestSize.Level1)
{
    const char *invalidScene = "SCENE_RECORD";
    uint64_t sceneTypeCode;
    int32_t result = GetSceneTypeCode(invalidScene, &sceneTypeCode);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test EnhanceChainManagerProcessDefault API
* @tc.number : EnhanceChainManagerProcessDefault001
* @tc.desc   : Test EnhanceChainManagerProcessDefault interface.
*/
HWTEST_F(AudioEnhanceChainAdapterUnitTest, EnhanceChainManagerProcessDefault_001, TestSize.Level1)
{
    EnhanceChainManagerCreateCb(VALID_SCENEKEY, &validAdapter);
    EnhanceChainManagerInitEnhanceBuffer();

    uint32_t bufferSize = VALID_BUFFER_SIZE;
    std::vector<uint8_t> dummyData(bufferSize, 0xAA);
    CopyToEnhanceBufferAdapter(dummyData.data(), bufferSize);

    int32_t result = EnhanceChainManagerProcessDefault(INVALID_CAPTUREID, bufferSize);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name   : Test EnhanceChainManagerProcessDefault API
* @tc.number : EnhanceChainManagerProcessDefault002
* @tc.desc   : Test EnhanceChainManagerProcessDefault interface.
*/
HWTEST_F(AudioEnhanceChainAdapterUnitTest, EnhanceChainManagerProcessDefault_002, TestSize.Level1)
{
    EnhanceChainManagerCreateCb(VALID_SCENEKEY, &validAdapter);
    EnhanceChainManagerInitEnhanceBuffer();

    uint32_t bufferSize = VALID_BUFFER_SIZE;
    std::vector<uint8_t> dummyData(bufferSize, 0xAA);
    CopyToEnhanceBufferAdapter(dummyData.data(), bufferSize);

    int32_t result = EnhanceChainManagerProcessDefault(VALID_CAPTUREID, bufferSize);
    EXPECT_EQ(ERROR, result);
}
} // namespace AudioStandard
} // namespace OHOS