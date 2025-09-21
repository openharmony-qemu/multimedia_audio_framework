/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "audio_suite_aiss_node.h"
#include "audio_suite_aiss_algo_interface_impl.h"
#include "audio_suite_algo_interface.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using ::testing::_;
using ::testing::Return;

class MockSuiteNodeReadTapDataCallback : public SuiteNodeReadTapDataCallback {
    MOCK_METHOD(void, OnReadTapDataCallback, (void*, int32_t), (override));
};

class AudioSuiteAissNodeTest : public testing::Test {
public:
    void SetUp()
    {
        AudioFormat audioFormat;
        impl = std::make_shared<AudioSuiteAissNode>(NODE_TYPE_AUDIO_SEPARATION, audioFormat);
    };
    void TearDown()
    {
        impl = nullptr;
    };
    std::shared_ptr<AudioSuiteAissNode> impl = nullptr;
};

namespace {
    HWTEST_F(AudioSuiteAissNodeTest, InitTest, TestSize.Level0)
    {
        EXPECT_EQ(impl->Init(), SUCCESS);
        EXPECT_EQ(impl->DeInit(), SUCCESS);
    }

    HWTEST_F(AudioSuiteAissNodeTest, GetOutputPortTest, TestSize.Level0)
    {
        auto humanPort = impl->GetOutputPort(AUDIO_NODE_HUMAN_SOUND_OUTPORT_TYPE);
        ASSERT_NE(humanPort, nullptr);
        auto bkgPort = impl->GetOutputPort(AUDIO_NODE_BACKGROUND_SOUND_OUTPORT_TYPE);
        ASSERT_NE(bkgPort, nullptr);
        auto port = impl->GetOutputPort(static_cast<AudioNodePortType>(100));
        ASSERT_EQ(port, nullptr);
    }

    HWTEST_F(AudioSuiteAissNodeTest, FlushTest, TestSize.Level0)
    {
        EXPECT_EQ(impl->Flush(), SUCCESS);
    }

    HWTEST_F(AudioSuiteAissNodeTest, ResetTest, TestSize.Level0)
    {
        EXPECT_EQ(impl->Reset(), SUCCESS);
    }

    HWTEST_F(AudioSuiteAissNodeTest, TapTest, TestSize.Level0)
    {
        auto humanCallback = std::make_shared<MockSuiteNodeReadTapDataCallback>();
        auto bkgCallback = std::make_shared<MockSuiteNodeReadTapDataCallback>();
        AudioSuitePcmBuffer buffer(SAMPLE_RATE_48000, 2, CH_LAYOUT_STEREO);
        impl->HandleTapCallback(&buffer);
        EXPECT_EQ(impl->InstallTap(AUDIO_NODE_HUMAN_SOUND_OUTPORT_TYPE, humanCallback), SUCCESS);
        EXPECT_EQ(impl->InstallTap(AUDIO_NODE_BACKGROUND_SOUND_OUTPORT_TYPE, bkgCallback), SUCCESS);
        EXPECT_EQ(impl->InstallTap(static_cast<AudioNodePortType>(100), humanCallback), ERROR);
        EXPECT_EQ(impl->RemoveTap(AUDIO_NODE_HUMAN_SOUND_OUTPORT_TYPE), SUCCESS);
        EXPECT_EQ(impl->RemoveTap(AUDIO_NODE_BACKGROUND_SOUND_OUTPORT_TYPE), SUCCESS);
        EXPECT_EQ(impl->RemoveTap(static_cast<AudioNodePortType>(100)), ERROR);
    }
}