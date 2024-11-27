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

#include <thread>
#include <gtest/gtest.h>
#include "offline_audio_effect_manager.h"
#include "audio_errors.h"
#include <iostream>

using namespace testing::ext;
using namespace std;
namespace OHOS {
namespace AudioStandard {
namespace {
    const std::string NORMAL_CHAIN_NAME = "abcdefg";
    constexpr int32_t NORMAL_BUFFER_SIZE = 7680;
    constexpr AudioStreamInfo NORMAL_STREAM_INFO(
        AudioSamplingRate::SAMPLE_RATE_48000, AudioEncodingType::ENCODING_PCM,
        AudioSampleFormat::SAMPLE_S16LE, AudioChannel::STEREO, AudioChannelLayout::CH_LAYOUT_STEREO);
}
class OfflineAudioEffectManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void OfflineAudioEffectManagerUnitTest::SetUpTestCase(void) {}
void OfflineAudioEffectManagerUnitTest::TearDownTestCase(void) {}
void OfflineAudioEffectManagerUnitTest::SetUp(void) {}
void OfflineAudioEffectManagerUnitTest::TearDown(void) {}

class OfflineAudioEffectChainUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
protected:
    shared_ptr<OfflineAudioEffectChain> chain_ = nullptr;
};

shared_ptr<OfflineAudioEffectManager> manager_ = nullptr;

void OfflineAudioEffectChainUnitTest::SetUpTestCase(void)
{
    manager_ = OfflineAudioEffectManager::GetInstance();
}

void OfflineAudioEffectChainUnitTest::TearDownTestCase(void)
{
    manager_ = nullptr;
}

void OfflineAudioEffectChainUnitTest::SetUp(void)
{
    chain_ = manager_->CreateOfflineAudioEffectChain(NORMAL_CHAIN_NAME);
}

void OfflineAudioEffectChainUnitTest::TearDown(void)
{
    if (chain_ != nullptr) {
        chain_->Release();
        chain_ = nullptr;
    }
}

/**
* @tc.name  : Test GetOfflineAudioEffectChains API
* @tc.type  : FUNC
* @tc.number: OfflineAudioEffectManager_001
* @tc.desc  : Test OfflineAudioEffectManager interface.
*/
HWTEST(OfflineAudioEffectManagerUnitTest, OfflineAudioEffectManager_001, TestSize.Level1)
{
    auto manager = OfflineAudioEffectManager::GetInstance();
    EXPECT_NE(nullptr, manager);
    EXPECT_GE(manager->GetOfflineAudioEffectChains().size(), 0);
}

/**
* @tc.name  : Test CreateOfflineAudioEffectChain API
* @tc.type  : FUNC
* @tc.number: OfflineAudioEffectManager_002
* @tc.desc  : Test OfflineAudioEffectManager interface.
*/
HWTEST(OfflineAudioEffectManagerUnitTest, OfflineAudioEffectManager_002, TestSize.Level1)
{
    auto chain = OfflineAudioEffectManager::GetInstance()->CreateOfflineAudioEffectChain(NORMAL_CHAIN_NAME);
    EXPECT_NE(nullptr, chain);
}

/**
* @tc.name  : Test OfflineAudioEffectChain API
* @tc.type  : FUNC
* @tc.number: OfflineAudioEffectChain_001
* @tc.desc  : Test OfflineAudioEffectChain interface.
*/
HWTEST_F(OfflineAudioEffectChainUnitTest, OfflineAudioEffectChain_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, chain_);
    int32_t ret = chain_->Configure(NORMAL_STREAM_INFO, NORMAL_STREAM_INFO);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test OfflineAudioEffectChain API
* @tc.type  : FUNC
* @tc.number: OfflineAudioEffectChain_002
* @tc.desc  : Test OfflineAudioEffectChain interface.
*/
HWTEST_F(OfflineAudioEffectChainUnitTest, OfflineAudioEffectChain_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, chain_);
    EXPECT_EQ(SUCCESS, chain_->Configure(NORMAL_STREAM_INFO, NORMAL_STREAM_INFO));
    EXPECT_EQ(SUCCESS, chain_->Prepare());
}

/**
* @tc.name  : Test OfflineAudioEffectChain API
* @tc.type  : FUNC
* @tc.number: OfflineAudioEffectChain_003
* @tc.desc  : Test OfflineAudioEffectChain interface.
*/
HWTEST_F(OfflineAudioEffectChainUnitTest, OfflineAudioEffectChain_003, TestSize.Level1)
{
    uint32_t inSize = 0;
    uint32_t outSize = 0;
    EXPECT_NE(nullptr, chain_);
    EXPECT_EQ(SUCCESS, chain_->Configure(NORMAL_STREAM_INFO, NORMAL_STREAM_INFO));
    EXPECT_EQ(SUCCESS, chain_->Prepare());
    EXPECT_EQ(SUCCESS, chain_->GetEffectBufferSize(inSize, outSize));
    EXPECT_GT(inSize, 0);
    EXPECT_GT(outSize, 0);
    uint8_t *inBuffer = new uint8_t[inSize];
    uint8_t *outBuffer = new uint8_t[outSize];
    for (uint32_t i = 0; i < inSize; i++) {
        inBuffer[i] = 1;
    }
    EXPECT_EQ(ERR_INVALID_PARAM, chain_->Process(nullptr, inSize, outBuffer, outSize));
    EXPECT_EQ(ERR_INVALID_PARAM, chain_->Process(inBuffer, inSize + 1, outBuffer, outSize));
    EXPECT_EQ(ERR_INVALID_PARAM, chain_->Process(inBuffer, inSize, nullptr, outSize));
    EXPECT_EQ(ERR_INVALID_PARAM, chain_->Process(inBuffer, inSize, outBuffer, outSize + 1));
    EXPECT_EQ(SUCCESS, chain_->Process(inBuffer, inSize, outBuffer, outSize));
    delete []inBuffer;
    delete []outBuffer;
}
} // namespace AudioStandard
} // namespace OHOS
