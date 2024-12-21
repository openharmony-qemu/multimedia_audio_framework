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

#ifndef LOG_TAG
#define LOG_TAG "AudioEnhanceChainUnitTest"
#endif

#include "audio_enhance_chain_unit_test.h"

#include <chrono>
#include <thread>
#include <fstream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "audio_utils.h"
#include "audio_errors.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
void AudioEnhanceChainUnitTest::SetUpTestCase(void) {}
void AudioEnhanceChainUnitTest::TearDownTestCase(void) {}
void AudioEnhanceChainUnitTest::SetUp(void) {}
void AudioEnhanceChainUnitTest::TearDown(void) {}

/**
* @tc.name  : Test AudioEnhanceChain API
* @tc.type  : FUNC
* @tc.number: AudioEnhanceChain_001
* @tc.desc  : Test AudioEnhanceChain::ReleaseEnhanceChain()
*/
HWTEST(AudioEnhanceChainUnitTest, AudioEnhanceChain_001, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = nullptr;
    std::string scene = "SCENE_VOIP_UP";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;

    audioEnhanceChain = std::make_shared<AudioEnhanceChain>(scene, algoParam, deviceAttr, defaultFlag);
    EXPECT_NE(audioEnhanceChain, nullptr);

    audioEnhanceChain->standByEnhanceHandles_ = std::vector<AudioEffectHandle>(3);
    audioEnhanceChain->enhanceLibHandles_ = std::vector<AudioEffectLibrary *>(3);
    audioEnhanceChain->ReleaseEnhanceChain();
}

/**
* @tc.name  : Test AudioEnhanceChain API
* @tc.type  : FUNC
* @tc.number: AudioEnhanceChain_002
* @tc.desc  : Test AudioEnhanceChain::ReleaseEnhanceChain()
*/
HWTEST(AudioEnhanceChainUnitTest, AudioEnhanceChain_002, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = nullptr;
    std::string sceneType = "SCENE_VOIP_UP";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;

    audioEnhanceChain = std::make_shared<AudioEnhanceChain>(sceneType, algoParam, deviceAttr, defaultFlag);
    EXPECT_NE(audioEnhanceChain, nullptr);

    audioEnhanceChain->standByEnhanceHandles_ = std::vector<AudioEffectHandle>(1);
    audioEnhanceChain->enhanceLibHandles_ = std::vector<AudioEffectLibrary *>(3);
    audioEnhanceChain->ReleaseEnhanceChain();
}

/**
* @tc.name  : Test AudioEnhanceChain API
* @tc.type  : FUNC
* @tc.number: AudioEnhanceChain_003
* @tc.desc  : Test AudioEnhanceChain::ReleaseEnhanceChain()
*/
HWTEST(AudioEnhanceChainUnitTest, AudioEnhanceChain_003, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = nullptr;
    std::string sceneType = "SCENE_VOIP_UP";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;

    audioEnhanceChain = std::make_shared<AudioEnhanceChain>(sceneType, algoParam, deviceAttr, defaultFlag);
    EXPECT_NE(audioEnhanceChain, nullptr);

    audioEnhanceChain->standByEnhanceHandles_ = std::vector<AudioEffectHandle>(3);
    audioEnhanceChain->enhanceLibHandles_ = std::vector<AudioEffectLibrary *>(1);
    audioEnhanceChain->ReleaseEnhanceChain();
}

/**
* @tc.name  : Test AudioEnhanceChain API
* @tc.type  : FUNC
* @tc.number: AudioEnhanceChain_005
* @tc.desc  : Test AudioEnhanceChain::SetEnhanceProperty()
*/
HWTEST(AudioEnhanceChainUnitTest, AudioEnhanceChain_005, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = nullptr;
    std::string sceneType = "SCENE_VOIP_UP";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;

    audioEnhanceChain = std::make_shared<AudioEnhanceChain>(sceneType, algoParam, deviceAttr, defaultFlag);
    EXPECT_NE(audioEnhanceChain, nullptr);

    audioEnhanceChain->enhanceNames_.push_back("record");
    std::string enhance = "record";
    std::string property = "123";

    int32_t result = audioEnhanceChain->SetEnhanceProperty(enhance, property);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name  : Test AudioEnhanceChain API
* @tc.type  : FUNC
* @tc.number: AudioEnhanceChain_006
* @tc.desc  : Test AudioEnhanceChain::SetEnhanceProperty()
*/
HWTEST(AudioEnhanceChainUnitTest, AudioEnhanceChain_006, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = nullptr;
    std::string sceneType = "SCENE_VOIP_UP";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;

    audioEnhanceChain = std::make_shared<AudioEnhanceChain>(sceneType, algoParam, deviceAttr, defaultFlag);
    EXPECT_NE(audioEnhanceChain, nullptr);

    audioEnhanceChain->enhanceNames_.push_back("record");
    std::string enhance = "record";
    std::string property = "123";

    int32_t result = audioEnhanceChain->SetEnhanceProperty(enhance, property);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name  : Test AudioEnhanceChain API
* @tc.type  : FUNC
* @tc.number: AudioEnhanceChain_007
* @tc.desc  : Test AudioEnhanceChain::ApplyEnhanceChain()
*/
HWTEST(AudioEnhanceChainUnitTest, AudioEnhanceChain_007, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = nullptr;
    std::string sceneType = "SCENE_VOIP_UP";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;

    audioEnhanceChain = std::make_shared<AudioEnhanceChain>(sceneType, algoParam, deviceAttr, defaultFlag);
    EXPECT_NE(audioEnhanceChain, nullptr);

    std::unique_ptr<EnhanceBuffer> enhanceBuffer = nullptr;
    int32_t length = 100;

    int32_t result = audioEnhanceChain->ApplyEnhanceChain(enhanceBuffer, length);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name  : Test AudioEnhanceChain API
* @tc.type  : FUNC
* @tc.number: AudioEnhanceChain_008
* @tc.desc  : Test AudioEnhanceChain::ApplyEnhanceChain()
*/
HWTEST(AudioEnhanceChainUnitTest, AudioEnhanceChain_008, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = nullptr;
    std::string sceneType = "SCENE_VOIP_UP";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;

    audioEnhanceChain = std::make_shared<AudioEnhanceChain>(sceneType, algoParam, deviceAttr, defaultFlag);
    EXPECT_NE(audioEnhanceChain, nullptr);

    std::unique_ptr<EnhanceBuffer> enhanceBuffer = std::make_unique<EnhanceBuffer>();
    enhanceBuffer->micBufferIn.resize(100);
    int32_t length = 100;

    int32_t result = audioEnhanceChain->ApplyEnhanceChain(enhanceBuffer, length);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name  : Test AudioEnhanceChain API
* @tc.type  : FUNC
* @tc.number: AudioEnhanceChain_009
* @tc.desc  : Test AudioEnhanceChain::ApplyEnhanceChain()
*/
HWTEST(AudioEnhanceChainUnitTest, AudioEnhanceChain_009, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = nullptr;
    std::string sceneType = "SCENE_VOIP_UP";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;

    audioEnhanceChain = std::make_shared<AudioEnhanceChain>(sceneType, algoParam, deviceAttr, defaultFlag);
    EXPECT_NE(audioEnhanceChain, nullptr);

    audioEnhanceChain->needEcFlag_ = true;
    audioEnhanceChain->deviceAttr_.micChannels = 0;
    std::unique_ptr<EnhanceBuffer> enhanceBuffer = std::make_unique<EnhanceBuffer>();
    enhanceBuffer->micBufferIn.resize(100);
    enhanceBuffer->ecBuffer.resize(100);
    int32_t length = 100;
    
    int32_t result = audioEnhanceChain->ApplyEnhanceChain(enhanceBuffer, length);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name  : Test AudioEnhanceChain API
* @tc.type  : FUNC
* @tc.number: AudioEnhanceChain_010
* @tc.desc  : Test AudioEnhanceChain::ApplyEnhanceChain()
*/
HWTEST(AudioEnhanceChainUnitTest, AudioEnhanceChain_010, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = nullptr;
    std::string sceneType = "SCENE_VOIP_UP";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;

    audioEnhanceChain = std::make_shared<AudioEnhanceChain>(sceneType, algoParam, deviceAttr, defaultFlag);
    EXPECT_NE(audioEnhanceChain, nullptr);

    audioEnhanceChain->needMicRefFlag_ = true;
    audioEnhanceChain->deviceAttr_.micChannels = 0;
    std::unique_ptr<EnhanceBuffer> enhanceBuffer = std::make_unique<EnhanceBuffer>();
    enhanceBuffer->micBufferIn.resize(100);
    enhanceBuffer->micRefBuffer.resize(100);
    int32_t length = 100;
    
    int32_t result = audioEnhanceChain->ApplyEnhanceChain(enhanceBuffer, length);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name  : Test AudioEnhanceChain API
* @tc.type  : FUNC
* @tc.number: AudioEnhanceChain_011
* @tc.desc  : Test AudioEnhanceChain::ApplyEnhanceChain()
*/
HWTEST(AudioEnhanceChainUnitTest, AudioEnhanceChain_011, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = nullptr;
    std::string sceneType = "SCENE_VOIP_UP";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;

    audioEnhanceChain = std::make_shared<AudioEnhanceChain>(sceneType, algoParam, deviceAttr, defaultFlag);
    EXPECT_NE(audioEnhanceChain, nullptr);
    
    audioEnhanceChain->standByEnhanceHandles_ = std::vector<AudioEffectHandle>(3);
    std::unique_ptr<EnhanceBuffer> enhanceBuffer = std::make_unique<EnhanceBuffer>();
    enhanceBuffer->micBufferIn.resize(100);
    int32_t length = 100;
    
    int32_t result = audioEnhanceChain->ApplyEnhanceChain(enhanceBuffer, length);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name  : Test AudioEnhanceChain API
* @tc.type  : FUNC
* @tc.number: AudioEnhanceChain_012
* @tc.desc  : Test AudioEnhanceChain::GetOneFrameInputData()
*/
HWTEST(AudioEnhanceChainUnitTest, AudioEnhanceChain_012, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = nullptr;
    std::string sceneType = "SCENE_VOIP_UP";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;

    audioEnhanceChain = std::make_shared<AudioEnhanceChain>(sceneType, algoParam, deviceAttr, defaultFlag);
    EXPECT_NE(audioEnhanceChain, nullptr);
    
    audioEnhanceChain->standByEnhanceHandles_ = std::vector<AudioEffectHandle>(3);
    std::unique_ptr<EnhanceBuffer> enhanceBuffer = std::make_unique<EnhanceBuffer>();

    
    int32_t result = audioEnhanceChain->GetOneFrameInputData(enhanceBuffer);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name  : Test AudioEnhanceChain API
* @tc.type  : FUNC
* @tc.number: AudioEnhanceChain_013
* @tc.desc  : Test AudioEnhanceChain::GetOneFrameInputData()
*/
HWTEST(AudioEnhanceChainUnitTest, AudioEnhanceChain_013, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = nullptr;
    std::string sceneType = "SCENE_VOIP_UP";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;

    audioEnhanceChain = std::make_shared<AudioEnhanceChain>(sceneType, algoParam, deviceAttr, defaultFlag);
    EXPECT_NE(audioEnhanceChain, nullptr);
    
    audioEnhanceChain->needEcFlag_ = true;
    std::unique_ptr<EnhanceBuffer> enhanceBuffer = std::make_unique<EnhanceBuffer>();
    enhanceBuffer->micBufferIn.resize(100);
    
    int32_t result = audioEnhanceChain->GetOneFrameInputData(enhanceBuffer);
    EXPECT_NE(SUCCESS, result);
}

/**
* @tc.name  : Test AudioEnhanceChain API
* @tc.type  : FUNC
* @tc.number: AudioEnhanceChain_014
* @tc.desc  : Test AudioEnhanceChain::GetOneFrameInputData()
*/
HWTEST(AudioEnhanceChainUnitTest, AudioEnhanceChain_014, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = nullptr;
    std::string sceneType = "SCENE_VOIP_UP";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;

    audioEnhanceChain = std::make_shared<AudioEnhanceChain>(sceneType, algoParam, deviceAttr, defaultFlag);
    EXPECT_NE(audioEnhanceChain, nullptr);
    
    std::unique_ptr<EnhanceBuffer> enhanceBuffer = std::make_unique<EnhanceBuffer>();
    enhanceBuffer->micBufferIn.resize(100);
    
    int32_t result = audioEnhanceChain->GetOneFrameInputData(enhanceBuffer);
    EXPECT_NE(SUCCESS, result);
}

/**
* @tc.name  : Test AudioEnhanceChain API
* @tc.type  : FUNC
* @tc.number: AudioEnhanceChain_015
* @tc.desc  : Test AudioEnhanceChain::GetOneFrameInputData()
*/
HWTEST(AudioEnhanceChainUnitTest, AudioEnhanceChain_015, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = nullptr;
    std::string sceneType = "SCENE_VOIP_UP";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;

    audioEnhanceChain = std::make_shared<AudioEnhanceChain>(sceneType, algoParam, deviceAttr, defaultFlag);
    EXPECT_NE(audioEnhanceChain, nullptr);
    
    audioEnhanceChain->needMicRefFlag_ = true;
    std::unique_ptr<EnhanceBuffer> enhanceBuffer = std::make_unique<EnhanceBuffer>();
    enhanceBuffer->micBufferIn.resize(100);
    enhanceBuffer->micRefBuffer.resize(100);

    
    int32_t result = audioEnhanceChain->GetOneFrameInputData(enhanceBuffer);
    EXPECT_NE(SUCCESS, result);
}

/**
* @tc.name  : Test AudioEnhanceChain API
* @tc.type  : FUNC
* @tc.number: AudioEnhanceChain_016
* @tc.desc  : Test AudioEnhanceChain::DeinterleaverData()
*/
HWTEST(AudioEnhanceChainUnitTest, AudioEnhanceChain_016, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = nullptr;
    std::string sceneType = "SCENE_VOIP_UP";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;

    audioEnhanceChain = std::make_shared<AudioEnhanceChain>(sceneType, algoParam, deviceAttr, defaultFlag);
    EXPECT_NE(audioEnhanceChain, nullptr);
    
    uint8_t src[10] = {0};
    uint8_t dst[10] = {0};
    uint32_t channel = 2;
    uint32_t offset = 0;

    int32_t result = audioEnhanceChain->DeinterleaverData(src, channel, dst, offset);
    EXPECT_NE(SUCCESS, result);
}

/**
* @tc.name  : Test AudioEnhanceChain API
* @tc.type  : FUNC
* @tc.number: AudioEnhanceChain_017
* @tc.desc  : Test AudioEnhanceChain::DeinterleaverData()
*/
HWTEST(AudioEnhanceChainUnitTest, AudioEnhanceChain_017, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = nullptr;
    std::string sceneType = "SCENE_VOIP_UP";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;

    audioEnhanceChain = std::make_shared<AudioEnhanceChain>(sceneType, algoParam, deviceAttr, defaultFlag);
    EXPECT_NE(audioEnhanceChain, nullptr);
    
    uint8_t *src = nullptr;
    uint8_t dst[10] = {0};
    uint32_t channel = 2;
    uint32_t offset = 0;

    int32_t result = audioEnhanceChain->DeinterleaverData(src, channel, dst, offset);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name  : Test AudioEnhanceChain API
* @tc.type  : FUNC
* @tc.number: AudioEnhanceChain_018
* @tc.desc  : Test AudioEnhanceChain::DeinterleaverData()
*/
HWTEST(AudioEnhanceChainUnitTest, AudioEnhanceChain_018, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = nullptr;
    std::string sceneType = "SCENE_VOIP_UP";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;

    audioEnhanceChain = std::make_shared<AudioEnhanceChain>(sceneType, algoParam, deviceAttr, defaultFlag);
    EXPECT_NE(audioEnhanceChain, nullptr);
    
    uint8_t src[10] = {0};
    uint8_t *dst = nullptr;
    uint32_t channel = 2;
    uint32_t offset = 0;

    int32_t result = audioEnhanceChain->DeinterleaverData(src, channel, dst, offset);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name  : Test AudioEnhanceChain API
* @tc.type  : FUNC
* @tc.number: AudioEnhanceChain_019
* @tc.desc  : Test AudioEnhanceChain::DeinterleaverData()
*/
HWTEST(AudioEnhanceChainUnitTest, AudioEnhanceChain_019, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = nullptr;
    std::string sceneType = "SCENE_VOIP_UP";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;

    audioEnhanceChain = std::make_shared<AudioEnhanceChain>(sceneType, algoParam, deviceAttr, defaultFlag);
    EXPECT_NE(audioEnhanceChain, nullptr);
    
    uint8_t src[10] = {0};
    uint8_t dst[10] = {0};
    uint32_t channel = 2;
    uint32_t offset = 10;

    int32_t result = audioEnhanceChain->DeinterleaverData(src, channel, dst, offset);
    EXPECT_EQ(ERROR, result);
}

/**
* @tc.name  : Test AudioEnhanceChain API
* @tc.type  : FUNC
* @tc.number: AudioEnhanceChain_020
* @tc.desc  : Test AudioEnhanceChain::SetInputDevice()
*/
HWTEST(AudioEnhanceChainUnitTest, AudioEnhanceChain_020, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = nullptr;
    std::string sceneType = "SCENE_VOIP_UP";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;

    audioEnhanceChain = std::make_shared<AudioEnhanceChain>(sceneType, algoParam, deviceAttr, defaultFlag);
    EXPECT_NE(audioEnhanceChain, nullptr);
    
    std::string inputDevice = "";
    std::string deviceName = "testDevice";

    int32_t result = audioEnhanceChain->SetInputDevice(inputDevice, deviceName);
    EXPECT_EQ(SUCCESS, result);
}
}
}
