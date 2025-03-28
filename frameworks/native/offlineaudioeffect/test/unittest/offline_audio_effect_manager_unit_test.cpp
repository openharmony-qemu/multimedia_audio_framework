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

#include <iostream>
#include <thread>
#include <gtest/gtest.h>

#include "audio_errors.h"
#include "offline_audio_effect_manager.h"
#include "offline_audio_effect_server_chain.h"
#include "audio_stream_info.h"

using namespace testing::ext;
using namespace testing;
using namespace std;
namespace OHOS {
namespace AudioStandard {
namespace {
    const std::string INVALID_EFFECT_NAME = "0d000721";

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

shared_ptr<OfflineAudioEffectManager> g_manager = nullptr;
string g_normalName = "";

void OfflineAudioEffectChainUnitTest::SetUpTestCase(void)
{
    g_manager = make_shared<OfflineAudioEffectManager>();
    vector<string> names;
    names = g_manager->GetOfflineAudioEffectChains();
    if (names.size() > 0) {
        g_normalName = names[names.size() - 1];
    }
}

void OfflineAudioEffectChainUnitTest::TearDownTestCase(void)
{
    g_manager = nullptr;
}

void OfflineAudioEffectChainUnitTest::SetUp(void)
{
    chain_ = g_manager->CreateOfflineAudioEffectChain(g_normalName);
}

void OfflineAudioEffectChainUnitTest::TearDown(void)
{
    if (chain_ != nullptr) {
        chain_->Release();
        chain_ = nullptr;
    }
}

class OfflineAudioEffectServerChainUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
void OfflineAudioEffectServerChainUnitTest::SetUpTestCase(void) {}
void OfflineAudioEffectServerChainUnitTest::TearDownTestCase(void) {}
void OfflineAudioEffectServerChainUnitTest::SetUp(void) {}
void OfflineAudioEffectServerChainUnitTest::TearDown(void) {}
/**
 * @tc.name  : Test GetOfflineAudioEffectChains API
 * @tc.type  : FUNC
 * @tc.number: OfflineAudioEffectManager_001
 * @tc.desc  : Test OfflineAudioEffectManager interface.
 */
HWTEST(OfflineAudioEffectManagerUnitTest, OfflineAudioEffectManager_001, TestSize.Level1)
{
    auto manager = make_shared<OfflineAudioEffectManager>();
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
    auto manager = make_shared<OfflineAudioEffectManager>();
    auto chain = manager->CreateOfflineAudioEffectChain(g_normalName);
    if (g_normalName == "") {
        EXPECT_EQ(nullptr, chain);
    } else {
        EXPECT_NE(nullptr, chain);
    }
}

/**
 * @tc.name  : Test CreateOfflineAudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: OfflineAudioEffectManager_003
 * @tc.desc  : Test OfflineAudioEffectManager interface.
 */
HWTEST(OfflineAudioEffectManagerUnitTest, OfflineAudioEffectManager_003, TestSize.Level1)
{
    auto manager = make_shared<OfflineAudioEffectManager>();
    auto chain = manager->CreateOfflineAudioEffectChain(INVALID_EFFECT_NAME);
    EXPECT_EQ(nullptr, chain);
}

/**
 * @tc.name  : Test OfflineAudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: OfflineAudioEffectChain_001
 * @tc.desc  : Test OfflineAudioEffectChain interface.
 */
HWTEST_F(OfflineAudioEffectChainUnitTest, OfflineAudioEffectChain_001, TestSize.Level1)
{
    if (chain_) {
        int32_t ret = chain_->Configure(NORMAL_STREAM_INFO, NORMAL_STREAM_INFO);
        EXPECT_EQ(SUCCESS, ret);
    }
}

/**
 * @tc.name  : Test OfflineAudioEffectChain API
 * @tc.type  : FUNC
 * @tc.number: OfflineAudioEffectChain_002
 * @tc.desc  : Test OfflineAudioEffectChain interface.
 */
HWTEST_F(OfflineAudioEffectChainUnitTest, OfflineAudioEffectChain_002, TestSize.Level1)
{
    if (chain_) {
        EXPECT_EQ(SUCCESS, chain_->Configure(NORMAL_STREAM_INFO, NORMAL_STREAM_INFO));
        EXPECT_EQ(SUCCESS, chain_->Prepare());
    }
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
    if (chain_) {
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
        EXPECT_NE(SUCCESS, chain_->Process(inBuffer, inSize, outBuffer, outSize));
        delete []inBuffer;
        delete []outBuffer;
    }
}

HWTEST_F(OfflineAudioEffectServerChainUnitTest, Create_001, TestSize.Level1)
{
    std::shared_ptr<OfflineAudioEffectServerChain>  serverChain =
        std::make_shared<OfflineAudioEffectServerChain>("test");
    int32_t ret = serverChain->Create();
    EXPECT_EQ(ret, ERROR);
}

HWTEST_F(OfflineAudioEffectServerChainUnitTest, SetParam_001, TestSize.Level1)
{
    std::shared_ptr<OfflineAudioEffectServerChain>  serverChain =
        std::make_shared<OfflineAudioEffectServerChain>("test");

    AudioStreamInfo inInfo;
    inInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    inInfo.encoding = AudioEncodingType::ENCODING_PCM;
    inInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    inInfo.channels = AudioChannel::MONO;
    AudioStreamInfo outInfo;
    outInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    outInfo.encoding = AudioEncodingType::ENCODING_PCM;
    outInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    outInfo.channels = AudioChannel::MONO;
    int32_t ret = serverChain->SetParam(inInfo, outInfo);
    EXPECT_EQ(ret, ERROR);
}

HWTEST_F(OfflineAudioEffectServerChainUnitTest, GetEffectBufferSize_001, TestSize.Level1)
{
    std::shared_ptr<OfflineAudioEffectServerChain>  serverChain =
        std::make_shared<OfflineAudioEffectServerChain>("test");
    uint32_t inBufferSize;
    uint32_t outBufferSize;
    int32_t ret = serverChain->GetEffectBufferSize(inBufferSize, outBufferSize);
    EXPECT_EQ(ret, ERROR);
}

HWTEST_F(OfflineAudioEffectServerChainUnitTest, GetEffectBufferSize_002, TestSize.Level1)
{
    std::shared_ptr<OfflineAudioEffectServerChain>  serverChain =
        std::make_shared<OfflineAudioEffectServerChain>("test");
    serverChain->inBufferSize_= 1;
    uint32_t inBufferSize;
    uint32_t outBufferSize;
    int32_t ret = serverChain->GetEffectBufferSize(inBufferSize, outBufferSize);
    EXPECT_EQ(ret, ERROR);
}

HWTEST_F(OfflineAudioEffectServerChainUnitTest, GetEffectBufferSize_003, TestSize.Level1)
{
    std::shared_ptr<OfflineAudioEffectServerChain>  serverChain =
        std::make_shared<OfflineAudioEffectServerChain>("test");
    serverChain->inBufferSize_ = 1;
    serverChain->outBufferSize_ = 1;
    uint32_t inBufferSize;
    uint32_t outBufferSize;
    int32_t ret = serverChain->GetEffectBufferSize(inBufferSize, outBufferSize);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(OfflineAudioEffectServerChainUnitTest, Release_001, TestSize.Level1)
{
    std::shared_ptr<OfflineAudioEffectServerChain>  serverChain =
        std::make_shared<OfflineAudioEffectServerChain>("test");
    int32_t ret = serverChain->Release();
    EXPECT_EQ(ret, ERROR);
}
} // namespace AudioStandard
} // namespace OHOS
