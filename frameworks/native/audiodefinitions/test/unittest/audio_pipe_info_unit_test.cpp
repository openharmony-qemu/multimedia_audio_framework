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

#include "audio_pipe_info.h"

#include <cinttypes>

#include <gtest/gtest.h>

#include "audio_common_log.h"
#include "audio_definitions_unit_test_utils.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace AudioStandard {

static const uint32_t TEST_OUTPUT_PIPE_ID = 0;
static const std::string TEST_OUTPUT_PIPE_NAME = "primary_output";
static const uint32_t TEST_OUTPUT_PIPE_ROUTE = AUDIO_OUTPUT_FLAG_NORMAL;
static const std::string TEST_OUTPUT_PIPE_ADAPTER = "primary";
static const uint32_t TEST_INPUT_PIPE_ID = 4096;
static const std::string TEST_INPUT_PIPE_NAME = "primary_input";
static const uint32_t TEST_INPUT_PIPE_ROUTE = AUDIO_INPUT_FLAG_NORMAL;
static const std::string TEST_INPUT_PIPE_ADAPTER = "primary";

class AudioPipeInfoUnitTest : public ::testing::Test {
public:
    static void SetUpTestCase(){};
    static void TearDownTestCase(){};
    virtual void SetUp();
    virtual void TearDown();

private:
    std::shared_ptr<AudioPipeInfo> testOutputPipe_ = nullptr;
    std::shared_ptr<AudioPipeInfo> testInputPipe_ = nullptr;
};

void AudioPipeInfoUnitTest::SetUp()
{
    testOutputPipe_ = std::make_shared<AudioPipeInfo>();
    // Replace by constructor or builder in future
    testOutputPipe_->id_ = TEST_OUTPUT_PIPE_ID;
    testOutputPipe_->pipeRole_ = PIPE_ROLE_OUTPUT;
    testOutputPipe_->name_ = TEST_OUTPUT_PIPE_NAME;
    testOutputPipe_->routeFlag_ = TEST_OUTPUT_PIPE_ROUTE;
    testOutputPipe_->adapterName_ = TEST_OUTPUT_PIPE_ADAPTER;

    testInputPipe_ = std::make_shared<AudioPipeInfo>();
    // Replace by constructor or builder in future
    testInputPipe_->id_ = TEST_INPUT_PIPE_ID;
    testInputPipe_->pipeRole_ = PIPE_ROLE_INPUT;
    testInputPipe_->name_ = TEST_INPUT_PIPE_NAME;
    testInputPipe_->routeFlag_ = TEST_INPUT_PIPE_ROUTE;
    testInputPipe_->adapterName_ = TEST_INPUT_PIPE_ADAPTER;
}

void AudioPipeInfoUnitTest::TearDown()
{
    testOutputPipe_ = nullptr;
    testInputPipe_ = nullptr;
}

/**
 * @tc.name   : AudioPipeInfo_AllSimpleGet_001
 * @tc.number : AllSimpleGet_001
 * @tc.desc   : Test all simple Get() funcs by default output pipe
 */
HWTEST_F(AudioPipeInfoUnitTest, AllSimpleGet_001, TestSize.Level2)
{
    EXPECT_EQ(TEST_OUTPUT_PIPE_ID, testOutputPipe_->GetId());

    EXPECT_EQ(true, testOutputPipe_->IsOutput());

    EXPECT_EQ(TEST_OUTPUT_PIPE_NAME, testOutputPipe_->GetName());

    EXPECT_EQ(TEST_OUTPUT_PIPE_ADAPTER, testOutputPipe_->GetAdapterName());

    EXPECT_EQ(PIPE_ACTION_DEFAULT, testOutputPipe_->GetAction());

    EXPECT_EQ(TEST_OUTPUT_PIPE_ROUTE, testOutputPipe_->GetRoute());
}

/**
 * @tc.name   : AudioPipeInfo_AllSimpleSet_001
 * @tc.number : AllSimpleSet_001
 * @tc.desc   : Test all simple Set() funcs by default output pipe
 */
HWTEST_F(AudioPipeInfoUnitTest, AllSimpleSet_001, TestSize.Level2)
{
    testOutputPipe_->SetAction(PIPE_ACTION_UPDATE);
    EXPECT_EQ(PIPE_ACTION_UPDATE, testOutputPipe_->GetAction());
}

/**
 * @tc.name   : AudioPipeInfo_AllOutputRoute_001
 * @tc.number : AllOutputRoute_001
 * @tc.desc   : Test all simple route funcs by default output pipe
 */
HWTEST_F(AudioPipeInfoUnitTest, AllOutputRoute_001, TestSize.Level2)
{
    testOutputPipe_->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    EXPECT_EQ(true, testOutputPipe_->IsRouteNormal());

    testOutputPipe_->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    EXPECT_EQ(false, testOutputPipe_->IsRouteNormal());
}

/**
 * @tc.name   : AudioPipeInfo_AllInputRoute_001
 * @tc.number : AllInputRoute_001
 * @tc.desc   : Test all simple route funcs by default input pipe
 */
HWTEST_F(AudioPipeInfoUnitTest, AllInputRoute_001, TestSize.Level2)
{
    testInputPipe_->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    EXPECT_EQ(true, testInputPipe_->IsRouteNormal());

    testInputPipe_->routeFlag_ = AUDIO_INPUT_FLAG_FAST;
    EXPECT_EQ(false, testInputPipe_->IsRouteNormal());
}

/**
 * @tc.name   : AudioPipeInfo_AllSimpleAdapter_001
 * @tc.number : AllSimpleAdapter_001
 * @tc.desc   : Test all simple adapter funcs by default input pipe
 */
HWTEST_F(AudioPipeInfoUnitTest, AllSimpleAdapter_001, TestSize.Level2)
{
    EXPECT_EQ(true, testOutputPipe_->IsSameAdapter(TEST_OUTPUT_PIPE_ADAPTER));

    EXPECT_EQ(false, testOutputPipe_->IsSameAdapter(""));
}

/**
 * @tc.name   : AudioPipeInfo_Dump_001
 * @tc.number : Dump_001
 * @tc.desc   : Test dump funcs by default output pipe
 */
HWTEST_F(AudioPipeInfoUnitTest, Dump_001, TestSize.Level3)
{
    std::string dumpStr;
    testOutputPipe_->Dump(dumpStr);
    EXPECT_NE("", dumpStr);

    dumpStr = "";
    auto stream = AudioDefinitionsUnitTestUtil::GenerateCommonStream(AUDIO_MODE_PLAYBACK);
    testOutputPipe_->AddStream(stream);
    testOutputPipe_->Dump(dumpStr);
    EXPECT_NE("", dumpStr);
}

/**
 * @tc.name   : AudioPipeInfo_Dump_002
 * @tc.number : Dump_002
 * @tc.desc   : Test dump funcs by default input pipe
 */
HWTEST_F(AudioPipeInfoUnitTest, Dump_002, TestSize.Level3)
{
    std::string dumpStr;
    testInputPipe_->Dump(dumpStr);
    EXPECT_NE("", dumpStr);

    dumpStr = "";
    auto stream = AudioDefinitionsUnitTestUtil::GenerateCommonStream(AUDIO_MODE_RECORD);
    testInputPipe_->AddStream(stream);
    testInputPipe_->Dump(dumpStr);
    EXPECT_NE("", dumpStr);
}

/**
 * @tc.name   : AudioPipeInfo_ToString_001
 * @tc.number : ToString_001
 * @tc.desc   : Test ToString() by default output pipe
 */
HWTEST_F(AudioPipeInfoUnitTest, ToString_001, TestSize.Level3)
{
    std::string out = testOutputPipe_->ToString();
    EXPECT_NE("", out);
}

/**
 * @tc.name   : AudioPipeInfo_StreamOperation_001
 * @tc.number : StreamOperation_001
 * @tc.desc   : Test stream operation funcs by default output pipe
 */
HWTEST_F(AudioPipeInfoUnitTest, StreamOperation_001, TestSize.Level2)
{
    auto stream = AudioDefinitionsUnitTestUtil::GenerateCommonStream(AUDIO_MODE_PLAYBACK);
    testOutputPipe_->AddStream(stream);
    EXPECT_EQ(true, testOutputPipe_->ContainStream(stream->GetSessionId()));

    testOutputPipe_->RemoveStream(stream->GetSessionId());
    EXPECT_EQ(false, testOutputPipe_->ContainStream(stream->GetSessionId()));
}

/**
 * @tc.name   : AudioPipeInfo_StreamOperation_002
 * @tc.number : StreamOperation_002
 * @tc.desc   : Test stream operation funcs by default output pipe in abnormal situations
 */
HWTEST_F(AudioPipeInfoUnitTest, StreamOperation_002, TestSize.Level4)
{
    auto stream = AudioDefinitionsUnitTestUtil::GenerateCommonStream(AUDIO_MODE_PLAYBACK);
    testOutputPipe_->RemoveStream(stream->GetSessionId());
    EXPECT_EQ(false, testOutputPipe_->ContainStream(stream->GetSessionId()));
}

/**
 * @tc.name   : AudioPipeInfo_IsSameRole_001
 * @tc.number : IsSameRole_001
 * @tc.desc   : Test role check func by default output pipe in different cases
 */
HWTEST_F(AudioPipeInfoUnitTest, IsSameRole_001, TestSize.Level4)
{
    // Test nullptr case
    EXPECT_EQ(false, testOutputPipe_->IsSameRole(nullptr));

    // Test same role case
    auto playbackStream = AudioDefinitionsUnitTestUtil::GenerateCommonStream(AUDIO_MODE_PLAYBACK);
    EXPECT_EQ(true, testOutputPipe_->IsSameRole(playbackStream));

    // Test different role case
    auto recordStream = AudioDefinitionsUnitTestUtil::GenerateCommonStream(AUDIO_MODE_RECORD);
    EXPECT_EQ(false, testOutputPipe_->IsSameRole(recordStream));
}
} // namespace AudioStandard
} // namespace OHOS