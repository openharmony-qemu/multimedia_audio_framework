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

using namespace testing::ext;
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
} // namespace AudioStandard
} // namespace OHOS
