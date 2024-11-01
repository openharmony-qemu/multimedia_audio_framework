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
#define LOG_TAG "AudioEffectChainUnitTest"
#endif

#include "audio_effect_chain_unit_test.h"
#include <chrono>
#include <thread>
#include <fstream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "audio_effect.h"
#include "audio_utils.h"
#include "audio_effect_log.h"
#include "audio_errors.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {

void AudioEffectChainUnitTest::SetUpTestCase(void) {}
void AudioEffectChainUnitTest::TearDownTestCase(void) {}
void AudioEffectChainUnitTest::SetUp(void) {}
void AudioEffectChainUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test AudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: AudioEffectChain_001
 * @tc.desc  : Test AudioEffectChain::ReleaseEffectChain()
 */
HWTEST(AudioEffectChainUnitTest, AudioEffectChain_001, TestSize.Level1)
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

    EXPECT_NE(audioEffectChain, nullptr);

    audioEffectChain->standByEffectHandles_ = std::vector<AudioEffectHandle>(3);
    audioEffectChain->libHandles_ = std::vector<AudioEffectLibrary *>(3);
    audioEffectChain->ReleaseEffectChain();
}

/**
 * @tc.name  : Test AudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: AudioEffectChain_002
 * @tc.desc  : Test AudioEffectChain::ReleaseEffectChain()
 */
HWTEST(AudioEffectChainUnitTest, AudioEffectChain_002, TestSize.Level1)
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

    EXPECT_NE(audioEffectChain, nullptr);

    audioEffectChain->standByEffectHandles_ = std::vector<AudioEffectHandle>(1);
    audioEffectChain->libHandles_ = std::vector<AudioEffectLibrary *>(3);
    audioEffectChain->ReleaseEffectChain();
}

/**
 * @tc.name  : Test AudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: AudioEffectChain_003
 * @tc.desc  : Test AudioEffectChain::ReleaseEffectChain()
 */
HWTEST(AudioEffectChainUnitTest, AudioEffectChain_003, TestSize.Level1)
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

    EXPECT_NE(audioEffectChain, nullptr);

    audioEffectChain->standByEffectHandles_ = std::vector<AudioEffectHandle>(3);
    audioEffectChain->libHandles_ = std::vector<AudioEffectLibrary *>(1);
    audioEffectChain->ReleaseEffectChain();
}

/**
 * @tc.name  : Test AudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: AudioEffectChain_005
 * @tc.desc  : Test AudioEffectChain::SetEffectProperty()
 */
HWTEST(AudioEffectChainUnitTest, AudioEffectChain_005, TestSize.Level1)
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

    EXPECT_NE(audioEffectChain, nullptr);

    audioEffectChain->effectNames_.push_back("MUSIC");
    std::string effect = "MUSIC";
    std::string property = "MUSIC";

    audioEffectChain->SetEffectProperty(effect, property);
}

/**
 * @tc.name  : Test AudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: AudioEffectChain_006
 * @tc.desc  : Test AudioEffectChain::SetEffectProperty()
 */
HWTEST(AudioEffectChainUnitTest, AudioEffectChain_006, TestSize.Level1)
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

    EXPECT_NE(audioEffectChain, nullptr);

    audioEffectChain->effectNames_.push_back("SCENE_MUSIC");
    std::string effect = "MUSIC";
    std::string property = "MUSIC";

    audioEffectChain->SetEffectProperty(effect, property);
}

/**
 * @tc.name  : Test AudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: AudioEffectChain_009
 * @tc.desc  : Test AudioEffectChain::ApplyEffectChain()
 */
HWTEST(AudioEffectChainUnitTest, AudioEffectChain_009, TestSize.Level1)
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

    EXPECT_NE(audioEffectChain, nullptr);

    float bufIn = 0.0;
    float bufOut = 0.0;
    uint32_t frameLen = 0;
    AudioEffectProcInfo procInfo;
    audioEffectChain->standByEffectHandles_.clear();

    audioEffectChain->ApplyEffectChain(&bufIn, &bufOut, frameLen, procInfo);
}

/**
 * @tc.name  : Test AudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: AudioEffectChain_013
 * @tc.desc  : Test AudioEffectChain::UpdateMultichannelIoBufferConfig()
 */
HWTEST(AudioEffectChainUnitTest, AudioEffectChain_013, TestSize.Level1)
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

    EXPECT_NE(audioEffectChain, nullptr);

    uint32_t channels = 0;
    uint64_t channelLayout = 0;
    audioEffectChain->ioBufferConfig_.inputCfg.channels = 0;
    audioEffectChain->ioBufferConfig_.inputCfg.channelLayout = 0;

    auto ret = audioEffectChain->UpdateMultichannelIoBufferConfig(channels, channelLayout);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: AudioEffectChain_014
 * @tc.desc  : Test AudioEffectChain::UpdateMultichannelIoBufferConfig()
 */
HWTEST(AudioEffectChainUnitTest, AudioEffectChain_014, TestSize.Level1)
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

    EXPECT_NE(audioEffectChain, nullptr);

    uint32_t channels = 0;
    uint64_t channelLayout = 0;
    audioEffectChain->ioBufferConfig_.inputCfg.channels = 0;
    audioEffectChain->ioBufferConfig_.inputCfg.channelLayout = 1;

    auto ret = audioEffectChain->UpdateMultichannelIoBufferConfig(channels, channelLayout);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: AudioEffectChain_015
 * @tc.desc  : Test AudioEffectChain::UpdateMultichannelIoBufferConfig()
 */
HWTEST(AudioEffectChainUnitTest, AudioEffectChain_015, TestSize.Level1)
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

    EXPECT_NE(audioEffectChain, nullptr);

    uint32_t channels = 0;
    uint64_t channelLayout = 0;
    audioEffectChain->ioBufferConfig_.inputCfg.channels = 1;
    audioEffectChain->ioBufferConfig_.inputCfg.channelLayout = 1;
    audioEffectChain->standByEffectHandles_.clear();

    auto ret = audioEffectChain->UpdateMultichannelIoBufferConfig(channels, channelLayout);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: AudioEffectChain_017
 * @tc.desc  : Test AudioEffectChain::SetHeadTrackingDisabled()
 */
HWTEST(AudioEffectChainUnitTest, AudioEffectChain_017, TestSize.Level1)
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

    EXPECT_NE(audioEffectChain, nullptr);

    audioEffectChain->standByEffectHandles_.clear();

    audioEffectChain->SetHeadTrackingDisabled();
}

/**
 * @tc.name  : Test AudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: AudioEffectChain_020
 * @tc.desc  : Test AudioEffectChain::InitEffectChain()
 */
HWTEST(AudioEffectChainUnitTest, AudioEffectChain_020, TestSize.Level1)
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

    EXPECT_NE(audioEffectChain, nullptr);

    audioEffectChain->standByEffectHandles_.clear();

    audioEffectChain->InitEffectChain();
}

/**
 * @tc.name  : Test AudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: AudioEffectChain_021
 * @tc.desc  : Test AudioEffectChain::UpdateMultichannelIoBufferConfigInner()
 */
HWTEST(AudioEffectChainUnitTest, AudioEffectChain_021, TestSize.Level1)
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

    EXPECT_NE(audioEffectChain, nullptr);

    auto ret = audioEffectChain->UpdateMultichannelIoBufferConfigInner();
    EXPECT_EQ(ret, ERROR);
}
} // namespace AudioStandard
} // namespace OHOS