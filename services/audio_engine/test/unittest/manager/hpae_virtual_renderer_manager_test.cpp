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

#include <atomic>
#include <chrono>
#include <gtest/gtest.h>
#include <string>
#include <thread>
#include "test_case_common.h"
#include "audio_errors.h"
#include "hpae_virtual_renderer_manager.h"
#include "hpae_node_common.h"
#include "hpae_mocks.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
using namespace testing::ext;
using namespace testing;
namespace {
constexpr int32_t FRAME_LENGTH_960 = 960;
constexpr uint32_t TEST_SESSION_ID = 123456;
constexpr uint32_t INVALID_SESSION_ID = 9999;
constexpr int32_t SLEEP_MS_20 = 20;
constexpr int32_t SLEEP_MS_40 = 40;

class HpaeVirtualRendererManagerTest : public testing::Test {
public:
    void SetUp() {}
    void TearDown() {}
};

static void InitStreamInfo(HpaeStreamInfo &streamInfo)
{
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.sessionId = TEST_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    streamInfo.deviceName = "Virtual_Render";
    streamInfo.uid = 1000;
}

static void WaitForMsgProcessing(std::shared_ptr<HpaeVirtualRendererManager> &manager)
{
    int waitCount = 0;
    const int waitCountThd = 5;
    while (manager->IsMsgProcessing()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS_20));
        waitCount++;
        if (waitCount >= waitCountThd) {
            break;
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS_40));
    EXPECT_EQ(manager->IsMsgProcessing(), false);
}

HWTEST_F(HpaeVirtualRendererManagerTest, CreateStreamAndGetSinkInputInfo, TestSize.Level1)
{
    auto manager = std::make_shared<HpaeVirtualRendererManager>();
    EXPECT_EQ(manager->Init(), SUCCESS);
    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo);
    EXPECT_EQ(manager->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager);
    HpaeSinkInputInfo sinkInputInfo;
    EXPECT_EQ(manager->GetSinkInputInfo(TEST_SESSION_ID, sinkInputInfo), SUCCESS);
    EXPECT_EQ(sinkInputInfo.nodeInfo.channels == streamInfo.channels, true);
    EXPECT_EQ(sinkInputInfo.nodeInfo.frameLen == streamInfo.frameLen, true);
    EXPECT_EQ(sinkInputInfo.nodeInfo.sessionId == TEST_SESSION_ID, true);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state == HPAE_SESSION_PREPARED, true);
    EXPECT_EQ(manager->DeInit(), SUCCESS);
}

HWTEST_F(HpaeVirtualRendererManagerTest, CreateStreamWithoutInit, TestSize.Level1)
{
    auto manager = std::make_shared<HpaeVirtualRendererManager>();
    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo);
    EXPECT_NE(manager->CreateStream(streamInfo), SUCCESS);
}

HWTEST_F(HpaeVirtualRendererManagerTest, DestroyStreamRemovesSession, TestSize.Level1)
{
    auto manager = std::make_shared<HpaeVirtualRendererManager>();
    EXPECT_EQ(manager->Init(), SUCCESS);
    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo);
    EXPECT_EQ(manager->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager);
    EXPECT_EQ(manager->DestroyStream(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager);
    HpaeSinkInputInfo sinkInputInfo;
    EXPECT_NE(manager->GetSinkInputInfo(TEST_SESSION_ID, sinkInputInfo), SUCCESS);
    EXPECT_EQ(manager->DeInit(), SUCCESS);
}

HWTEST_F(HpaeVirtualRendererManagerTest, GetSinkInputInfoUnknownSession, TestSize.Level1)
{
    auto manager = std::make_shared<HpaeVirtualRendererManager>();
    EXPECT_EQ(manager->Init(), SUCCESS);
    HpaeSinkInputInfo sinkInputInfo;
    EXPECT_NE(manager->GetSinkInputInfo(INVALID_SESSION_ID, sinkInputInfo), SUCCESS);
    EXPECT_EQ(manager->DeInit(), SUCCESS);
}

HWTEST_F(HpaeVirtualRendererManagerTest, GetAllSinkInputsInfoEmpty, TestSize.Level1)
{
    auto manager = std::make_shared<HpaeVirtualRendererManager>();
    EXPECT_EQ(manager->GetAllSinkInputsInfo().empty(), true);
}

HWTEST_F(HpaeVirtualRendererManagerTest, GetSinkInfoName, TestSize.Level1)
{
    auto manager = std::make_shared<HpaeVirtualRendererManager>();
    HpaeSinkInfo sinkInfo = manager->GetSinkInfo();
    EXPECT_EQ(sinkInfo.deviceName == "Virtual_Render", true);
}
}  // namespace
