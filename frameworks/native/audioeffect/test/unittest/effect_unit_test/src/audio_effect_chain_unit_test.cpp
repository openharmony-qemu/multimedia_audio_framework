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
#define LOG_TAG "AudioEffectChainUnitTest"
#endif

#include "audio_effect_chain_unit_test.h"
#include <chrono>
#include <thread>
#include <fstream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "audio_effect.h"
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
 * @tc.number: AudioEffectChain_004
 * @tc.desc  : Test AudioEffectChain::SetStreamUsage()
 */
HWTEST(AudioEffectChainUnitTest, AudioEffectChain_004, TestSize.Level1)
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
    std::int32_t streamUsage = 1;
    audioEffectChain->SetStreamUsage(streamUsage);
    EXPECT_EQ(audioEffectChain->streamUsage_, STREAM_USAGE_MUSIC);
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
 * @tc.number: AudioEffectChain_007
 * @tc.desc  : Test AudioEffectChain::StoreOldEffectChainInfo()
 */
HWTEST(AudioEffectChainUnitTest, AudioEffectChain_007, TestSize.Level1)
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
    AudioEffectConfig ioBufferConfig = {};
    std::string sceneMode = "123";
    audioEffectChain->StoreOldEffectChainInfo(sceneMode, ioBufferConfig);
    EXPECT_EQ(audioEffectChain->ioBufferConfig_.inputCfg.channels, 2);
}

/**
 * @tc.name  : Test AudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: AudioEffectChain_008
 * @tc.desc  : Test AudioEffectChain::SetFinalVolumeState()
 */
HWTEST(AudioEffectChainUnitTest, AudioEffectChain_008, TestSize.Level1)
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
    bool state = 1;
    audioEffectChain->SetFinalVolumeState(state);
    EXPECT_EQ(audioEffectChain->sendFinalVolumeState_, 1);
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
 * @tc.number: AudioEffectChain_011
 * @tc.desc  : Test AudioEffectChain::SetFinalVolume()
 */
HWTEST(AudioEffectChainUnitTest, AudioEffectChain_011, TestSize.Level1)
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
    float volume = 0.5;
    audioEffectChain->SetFinalVolume(volume);
    EXPECT_EQ(audioEffectChain->finalVolume_, 0.5);
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
 * @tc.number: AudioEffectChain_016
 * @tc.desc  : Test AudioEffectChain::SetHeadTrackingDisabled()
 */
HWTEST(AudioEffectChainUnitTest, AudioEffectChain_016, TestSize.Level1)
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
 * @tc.number: AudioEffectChain_017
 * @tc.desc  : Test AudioEffectChain::CrossFadeProcess()
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
    float bufOut = 0.0;
    uint32_t frameLen = 0;
    audioEffectChain->CrossFadeProcess(&bufOut, frameLen);
}

/**
 * @tc.name  : Test AudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: AudioEffectChain_018
 * @tc.desc  : Test AudioEffectChain::CrossFadeProcess()
 */
HWTEST(AudioEffectChainUnitTest, AudioEffectChain_018, TestSize.Level1)
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
    float bufOut = 0.0;
    uint32_t frameLen = 0;
    audioEffectChain->fadingCounts_ = 1;
    audioEffectChain->CrossFadeProcess(&bufOut, frameLen);
}

/**
 * @tc.name  : Test AudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: AudioEffectChain_019
 * @tc.desc  : Test AudioEffectChain::CrossFadeProcess()
 */
HWTEST(AudioEffectChainUnitTest, AudioEffectChain_019, TestSize.Level1)
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
    float bufOut = 0.0;
    uint32_t frameLen = 0;
    audioEffectChain->fadingCounts_ = -1;
    audioEffectChain->CrossFadeProcess(&bufOut, frameLen);
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

/**
 * @tc.name  : Test AudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: AudioEffectChain_022
 * @tc.desc  : Test AudioEffectChain::SetCurrChannelNoCheck()
 */
HWTEST(AudioEffectChainUnitTest, AudioEffectChain_022, TestSize.Level1)
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

    uint32_t channels = STEREO;
    audioEffectChain->SetCurrChannelNoCheck(channels);
}

/**
 * @tc.name  : Test AudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: AudioEffectChain_023
 * @tc.desc  : Test AudioEffectChain::SetCurrChannelLayoutNoCheck()
 */
HWTEST(AudioEffectChainUnitTest, AudioEffectChain_023, TestSize.Level1)
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

    uint64_t channelLayout = CH_LAYOUT_STEREO;
    audioEffectChain->SetCurrChannelLayoutNoCheck(channelLayout);
}

/**
 * @tc.name  : Test AudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: AudioEffectChain_024
 * @tc.desc  : Test AudioEffectChain::updateDumpName()
 */
HWTEST(AudioEffectChainUnitTest, AudioEffectChain_024, TestSize.Level1)
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

    audioEffectChain->sceneType_ = "SCENE_MUSIC";
    audioEffectChain->ioBufferConfig_.inputCfg.samplingRate = 48000;
    audioEffectChain->ioBufferConfig_.inputCfg.channels = 2;
    audioEffectChain->ioBufferConfig_.outputCfg.samplingRate = 48000;
    audioEffectChain->ioBufferConfig_.outputCfg.channels = 2;
    audioEffectChain->updateDumpName();
}

/**
 * @tc.name  : Test AudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: AudioEffectChain_025
 * @tc.desc  : Test AudioEffectChain::CheckChannelLayoutByReplyInfo()
 */
HWTEST(AudioEffectChainUnitTest, AudioEffectChain_025, TestSize.Level1)
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
    int32_t replyData = -1;
    AudioEffectTransInfo replyInfo = {sizeof(int32_t), &replyData};
    auto ret = audioEffectChain->CheckChannelLayoutByReplyInfo(replyInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: AudioEffectChain_026
 * @tc.desc  : Test AudioEffectChain::CheckChannelLayoutByReplyInfo()
 */
HWTEST(AudioEffectChainUnitTest, AudioEffectChain_026, TestSize.Level1)
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
    int32_t replyData = 0;
    AudioEffectTransInfo replyInfo = {sizeof(int32_t), &replyData};
    auto ret = audioEffectChain->CheckChannelLayoutByReplyInfo(replyInfo);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: AudioEffectChain_027
 * @tc.desc  : Test AudioEffectChain::updatePrimaryChannel()
 */
HWTEST(AudioEffectChainUnitTest, AudioEffectChain_027, TestSize.Level1)
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

    auto ret = audioEffectChain->updatePrimaryChannel();
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test AudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: AudioEffectChain_028
 * @tc.desc  : Test AudioEffectChain::ReleaseEffectChain()
 */
HWTEST(AudioEffectChainUnitTest, AudioEffectChain_028, TestSize.Level1)
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

    auto effectInterface = new AudioEffectInterface();
    ASSERT_TRUE(effectInterface != nullptr);
    AudioEffectHandle effectHandle = &effectInterface;
    audioEffectChain->standByEffectHandles_.push_back(effectHandle);

    auto effectLibrary = std::make_shared<AudioEffectLibrary>();
    audioEffectChain->libHandles_.push_back(effectLibrary.get());

    audioEffectChain->ReleaseEffectChain();
    delete effectInterface;
}

/**
 * @tc.name  : Test AudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: AudioEffectChain_029
 * @tc.desc  : Test AudioEffectChain::ReleaseEffectChain()
 */
HWTEST(AudioEffectChainUnitTest, AudioEffectChain_029, TestSize.Level1)
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
    auto effectLibrary = std::make_shared<AudioEffectLibrary>();
    audioEffectChain->libHandles_.push_back(effectLibrary.get());
    audioEffectChain->ReleaseEffectChain();
}

/**
 * @tc.name  : Test AudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: AudioEffectChain_030
 * @tc.desc  : Test AudioEffectChain::CheckChannelLayoutByReplyInfo()
 */
HWTEST(AudioEffectChainUnitTest, AudioEffectChain_030, TestSize.Level1)
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

    AudioEffectTransInfo info;
    info.data = nullptr;
    auto ret = audioEffectChain->CheckChannelLayoutByReplyInfo(info);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test AudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: AudioEffectChain_031
 * @tc.desc  : Test AudioEffectChain::updatePrimaryChannel()
 */
HWTEST(AudioEffectChainUnitTest, AudioEffectChain_031, TestSize.Level1)
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
    auto ret = audioEffectChain->updatePrimaryChannel();
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test AudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: AudioEffectChain_032
 * @tc.desc  : Test AudioEffectChain::UpdateMultichannelIoBufferConfigInner()
 */
HWTEST(AudioEffectChainUnitTest, AudioEffectChain_032, TestSize.Level1)
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
    auto ret = audioEffectChain->UpdateMultichannelIoBufferConfigInner();
    EXPECT_EQ(ret, ERROR);
}
} // namespace AudioStandard
} // namespace OHOS