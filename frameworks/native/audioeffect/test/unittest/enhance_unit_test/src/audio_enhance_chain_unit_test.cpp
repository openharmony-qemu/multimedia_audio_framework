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

const uint32_t DEFAULT_FRAME_LENGTH = 0;
const uint32_t DEFAULT_SAMPLE_RATE = 48000;
const uint32_t DEFAULT_DATA_FORMAT = 2;
const uint32_t DEFAULT_MIC_NUM = 2;
const uint32_t DEFAULT_EC_NUM = 4;
const uint32_t DEFAULT_MIC_REF_NUM = 5;
const uint32_t DEFAULT_OUT_NUM = 6;

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
    AudioEffectLibrary *audioEffectLibraryPtr = new AudioEffectLibrary();
    audioEnhanceChain->enhanceLibHandles_.emplace_back(audioEffectLibraryPtr);
    audioEnhanceChain->ReleaseEnhanceChain();
    delete audioEffectLibraryPtr;
    EXPECT_EQ(audioEnhanceChain->standByEnhanceHandles_.size(), 0);
    EXPECT_EQ(audioEnhanceChain->enhanceLibHandles_.size(), 0);
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
    EXPECT_EQ(audioEnhanceChain->standByEnhanceHandles_.size(), 0);
    EXPECT_EQ(audioEnhanceChain->enhanceLibHandles_.size(), 0);
}

/**
* @tc.name  : Test AudioEnhanceChain API
* @tc.type  : FUNC
* @tc.number: AudioEnhanceChain_004
* @tc.desc  : Test AudioEnhanceChain::AudioEnhanceChain()
*/
HWTEST(AudioEnhanceChainUnitTest, AudioEnhanceChain_004, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = nullptr;
    std::string sceneType = "SCENE_VOIP_UP";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    deviceAttr.micChannels = 1;
    bool defaultFlag = false;

    audioEnhanceChain = std::make_shared<AudioEnhanceChain>(sceneType, algoParam, deviceAttr, defaultFlag);
    EXPECT_NE(audioEnhanceChain, nullptr);
    EXPECT_EQ(audioEnhanceChain->deviceAttr_.micChannels, DEFAULT_MIC_NUM);
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
    audioEnhanceChain->deviceAttr_.micChannels = 5;
    audioEnhanceChain->deviceAttr_.micRate = 10;
    audioEnhanceChain->deviceAttr_.micFormat = 1;

    std::unique_ptr<EnhanceBuffer> enhanceBuffer = std::make_unique<EnhanceBuffer>();
    enhanceBuffer->micBufferIn.resize(100);
    int32_t length = 0;

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
    audioEnhanceChain->deviceAttr_.micChannels = 5;
    audioEnhanceChain->deviceAttr_.micRate = 10;
    audioEnhanceChain->deviceAttr_.micFormat = 1;

    audioEnhanceChain->needEcFlag_ = true;
    std::unique_ptr<EnhanceBuffer> enhanceBuffer = std::make_unique<EnhanceBuffer>();
    enhanceBuffer->micBufferIn.resize(100);
    enhanceBuffer->ecBuffer.resize(100);
    int32_t length = 0;

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
    audioEnhanceChain->deviceAttr_.micChannels = 5;
    audioEnhanceChain->deviceAttr_.micRate = 10;
    audioEnhanceChain->deviceAttr_.micFormat = 1;

    audioEnhanceChain->needMicRefFlag_ = true;
    std::unique_ptr<EnhanceBuffer> enhanceBuffer = std::make_unique<EnhanceBuffer>();
    enhanceBuffer->micBufferIn.resize(100);
    enhanceBuffer->micRefBuffer.resize(100);
    int32_t length = 0;

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
    audioEnhanceChain->deviceAttr_.micChannels = 5;
    audioEnhanceChain->deviceAttr_.micRate = 10;
    audioEnhanceChain->deviceAttr_.micFormat = 1;

    audioEnhanceChain->standByEnhanceHandles_ = std::vector<AudioEffectHandle>(3);
    std::unique_ptr<EnhanceBuffer> enhanceBuffer = std::make_unique<EnhanceBuffer>();
    enhanceBuffer->micBufferIn.resize(100);
    int32_t length = 0;

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

    std::unique_ptr<EnhanceBuffer> enhanceBuffer = std::make_unique<EnhanceBuffer>();
    enhanceBuffer->micBufferIn.resize(128, 0);

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
    enhanceBuffer->ecBuffer.resize(128, 0);
    audioEnhanceChain->needEcFlag_ = true;
    audioEnhanceChain->needMicRefFlag_ = false;

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

    std::unique_ptr<EnhanceBuffer> enhanceBuffer = std::make_unique<EnhanceBuffer>();
    enhanceBuffer->micRefBuffer.resize(128, 0);
    audioEnhanceChain->needEcFlag_ = false;
    audioEnhanceChain->needMicRefFlag_ = true;

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

    uint8_t src[128] = {0};
    uint8_t dst[128] = {0};
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
    uint32_t offset = 0;

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

/**
* @tc.name  : Test AudioEnhanceChain API
* @tc.type  : FUNC
* @tc.number: AudioEnhanceChain_021
* @tc.desc  : Test AudioEnhanceChain::SetInputDevice()
*/
HWTEST(AudioEnhanceChainUnitTest, AudioEnhanceChain_021, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = nullptr;
    std::string sceneType = "SCENE_VOIP_UP";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;

    audioEnhanceChain = std::make_shared<AudioEnhanceChain>(sceneType, algoParam, deviceAttr, defaultFlag);
    EXPECT_NE(audioEnhanceChain, nullptr);

    std::string inputDevice = "testDevice";
    std::string deviceName = "testDevice";

    int32_t result = audioEnhanceChain->SetInputDevice(inputDevice, deviceName);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name  : Test AudioEnhanceChain API
* @tc.type  : FUNC
* @tc.number: AudioEnhanceChain_025
* @tc.desc  : Test AudioEnhanceChain::SetFoldState()
*/
HWTEST(AudioEnhanceChainUnitTest, AudioEnhanceChain_025, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = nullptr;
    std::string sceneType = "SCENE_VOIP_UP";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;

    audioEnhanceChain = std::make_shared<AudioEnhanceChain>(sceneType, algoParam, deviceAttr, defaultFlag);
    EXPECT_NE(audioEnhanceChain, nullptr);

    audioEnhanceChain->algoParam_.foldState = 1;
    uint32_t state = 1;

    int32_t result = audioEnhanceChain->SetFoldState(state);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name  : Test AudioEnhanceChain API
* @tc.type  : FUNC
* @tc.number: AudioEnhanceChain_026
* @tc.desc  : Test AudioEnhanceChain::SetFoldState()
*/
HWTEST(AudioEnhanceChainUnitTest, AudioEnhanceChain_026, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = nullptr;
    std::string sceneType = "SCENE_VOIP_UP";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;

    audioEnhanceChain = std::make_shared<AudioEnhanceChain>(sceneType, algoParam, deviceAttr, defaultFlag);
    EXPECT_NE(audioEnhanceChain, nullptr);

    audioEnhanceChain->algoParam_.foldState = 0;
    uint32_t state = 1;

    int32_t result = audioEnhanceChain->SetFoldState(state);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name  : Test AudioEnhanceChain API
* @tc.type  : FUNC
* @tc.number: AudioEnhanceChain_029
* @tc.desc  : Test AudioEnhanceChain::SetEnhanceParam()
*/
HWTEST(AudioEnhanceChainUnitTest, AudioEnhanceChain_029, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = nullptr;
    std::string sceneType = "SCENE_VOIP_UP";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;

    audioEnhanceChain = std::make_shared<AudioEnhanceChain>(sceneType, algoParam, deviceAttr, defaultFlag);
    EXPECT_NE(audioEnhanceChain, nullptr);

    bool mute = false;
    uint32_t systemVol = 1;

    int32_t result = audioEnhanceChain->SetEnhanceParam(mute, systemVol);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name  : Test AudioEnhanceChain API
* @tc.type  : FUNC
* @tc.number: AudioEnhanceChain_040
* @tc.desc  : Test AudioEnhanceChain::GetAlgoConfig()
*/
HWTEST(AudioEnhanceChainUnitTest, AudioEnhanceChain_040, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = nullptr;
    std::string sceneType = "SCENE_VOIP_UP";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;

    audioEnhanceChain = std::make_shared<AudioEnhanceChain>(sceneType, algoParam, deviceAttr, defaultFlag);
    EXPECT_NE(audioEnhanceChain, nullptr);

    audioEnhanceChain->needEcFlag_ = false;
    audioEnhanceChain->needMicRefFlag_ = false;
    audioEnhanceChain->algoSupportedConfig_ = {DEFAULT_FRAME_LENGTH, DEFAULT_SAMPLE_RATE, DEFAULT_DATA_FORMAT,
        DEFAULT_MIC_NUM, DEFAULT_EC_NUM, DEFAULT_MIC_REF_NUM, DEFAULT_OUT_NUM};

    AudioBufferConfig micConfig;
    AudioBufferConfig ecConfig;
    AudioBufferConfig micRefConfig;

    audioEnhanceChain->GetAlgoConfig(micConfig, ecConfig, micRefConfig);
    EXPECT_EQ(micConfig.samplingRate, DEFAULT_SAMPLE_RATE);
    EXPECT_EQ(micConfig.channels, DEFAULT_MIC_NUM);
    EXPECT_EQ(static_cast<uint32_t>(micConfig.format), DEFAULT_DATA_FORMAT);

    EXPECT_NE(ecConfig.samplingRate, DEFAULT_SAMPLE_RATE);
    EXPECT_NE(ecConfig.channels, DEFAULT_EC_NUM);
    EXPECT_NE(static_cast<uint32_t>(ecConfig.format), DEFAULT_DATA_FORMAT);

    EXPECT_NE(micRefConfig.samplingRate, DEFAULT_SAMPLE_RATE);
    EXPECT_NE(micRefConfig.channels, DEFAULT_MIC_REF_NUM);
    EXPECT_NE(static_cast<uint32_t>(micRefConfig.format), DEFAULT_DATA_FORMAT);
}

/**
* @tc.name  : Test AudioEnhanceChain API
* @tc.type  : FUNC
* @tc.number: AudioEnhanceChain_041
* @tc.desc  : Test AudioEnhanceChain::GetAlgoConfig()
*/
HWTEST(AudioEnhanceChainUnitTest, AudioEnhanceChain_041, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = nullptr;
    std::string sceneType = "SCENE_VOIP_UP";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;

    audioEnhanceChain = std::make_shared<AudioEnhanceChain>(sceneType, algoParam, deviceAttr, defaultFlag);
    EXPECT_NE(audioEnhanceChain, nullptr);

    audioEnhanceChain->needEcFlag_ = true;
    audioEnhanceChain->needMicRefFlag_ = true;
    audioEnhanceChain->algoSupportedConfig_ = {DEFAULT_FRAME_LENGTH, DEFAULT_SAMPLE_RATE, DEFAULT_DATA_FORMAT,
        DEFAULT_MIC_NUM, DEFAULT_EC_NUM, DEFAULT_MIC_REF_NUM, DEFAULT_OUT_NUM};

    AudioBufferConfig micConfig;
    AudioBufferConfig ecConfig;
    AudioBufferConfig micRefConfig;

    audioEnhanceChain->GetAlgoConfig(micConfig, ecConfig, micRefConfig);
    EXPECT_EQ(micConfig.samplingRate, DEFAULT_SAMPLE_RATE);
    EXPECT_EQ(micConfig.channels, DEFAULT_MIC_NUM);
    EXPECT_EQ(static_cast<uint32_t>(micConfig.format), DEFAULT_DATA_FORMAT);

    EXPECT_EQ(ecConfig.samplingRate, DEFAULT_SAMPLE_RATE);
    EXPECT_EQ(ecConfig.channels, DEFAULT_EC_NUM);
    EXPECT_EQ(static_cast<uint32_t>(ecConfig.format), DEFAULT_DATA_FORMAT);

    EXPECT_EQ(micRefConfig.samplingRate, DEFAULT_SAMPLE_RATE);
    EXPECT_EQ(micRefConfig.channels, DEFAULT_MIC_REF_NUM);
    EXPECT_EQ(static_cast<uint32_t>(micRefConfig.format), DEFAULT_DATA_FORMAT);
}

/**
* @tc.name  : Test AudioEnhanceChain API
* @tc.type  : FUNC
* @tc.number: AudioEnhanceChain_043
* @tc.desc  : Test AudioEnhanceChain::SetPropertyToHandle()
*/
HWTEST(AudioEnhanceChainUnitTest, AudioEnhanceChain_043, TestSize.Level1)
{
    std::shared_ptr<AudioEnhanceChain> audioEnhanceChain = nullptr;
    std::string sceneType = "SCENE_VOIP_UP";
    AudioEnhanceParamAdapter algoParam;
    AudioEnhanceDeviceAttr deviceAttr;
    bool defaultFlag = false;

    audioEnhanceChain = std::make_shared<AudioEnhanceChain>(sceneType, algoParam, deviceAttr, defaultFlag);
    EXPECT_NE(audioEnhanceChain, nullptr);

    AudioEffectHandle handle = nullptr;
    std::string property;

    int32_t result = audioEnhanceChain->SetPropertyToHandle(handle, property);
    EXPECT_EQ(SUCCESS, result);
}
}
}