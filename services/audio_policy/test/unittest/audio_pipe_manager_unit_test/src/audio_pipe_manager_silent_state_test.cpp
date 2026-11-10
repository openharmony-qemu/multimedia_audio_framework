/*
* Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "audio_policy_utils.h"
#include "audio_pipe_manager_unit_test.h"
#include "audio_stream_descriptor.h"
#include "audio_pipe_info.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
constexpr uint32_t SILENT_TEST_SESSION_ID_1 = 982001;
constexpr uint32_t SILENT_TEST_SESSION_ID_2 = 982002;
constexpr uint32_t SILENT_TEST_SESSION_ID_3 = 982003;
constexpr uint32_t SILENT_TEST_SESSION_ID_4 = 982004;

static std::shared_ptr<AudioStreamDescriptor> MakeSilentStateStream(uint32_t sessionId,
    StreamUsage usage = STREAM_USAGE_NOTIFICATION)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = sessionId;
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->rendererInfo_.streamUsage = usage;
    return streamDesc;
}

static void AppendStreamToPipe(AudioPipeManager &pipeManager,
    const std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipeInfo->streamDescriptors_.push_back(streamDesc);
    pipeManager.curPipeList_.push_back(pipeInfo);
}

/**
 * @tc.name: SetMuteSilentState_001
 * @tc.desc: Test SetMuteSilentState updates mute flag, silent state and change detection.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, SetMuteSilentState_001, TestSize.Level1)
{
    AudioPipeManager pipeManager;
    auto streamDesc = MakeSilentStateStream(SILENT_TEST_SESSION_ID_1);
    AppendStreamToPipe(pipeManager, streamDesc);

    EXPECT_TRUE(pipeManager.SetMuteSilentState(SILENT_TEST_SESSION_ID_1, true));
    EXPECT_TRUE(streamDesc->GetMuteSilentState());
    EXPECT_TRUE(pipeManager.IsStreamSilent(SILENT_TEST_SESSION_ID_1));

    EXPECT_TRUE(pipeManager.SetMuteSilentState(SILENT_TEST_SESSION_ID_1, false));
    EXPECT_FALSE(streamDesc->GetMuteSilentState());
    EXPECT_FALSE(pipeManager.IsStreamSilent(SILENT_TEST_SESSION_ID_1));

    EXPECT_FALSE(pipeManager.SetMuteSilentState(SILENT_TEST_SESSION_ID_1, false));
    EXPECT_FALSE(pipeManager.SetMuteSilentState(SILENT_TEST_SESSION_ID_2, true));
}

/**
 * @tc.name: SetDataTransSilentState_001
 * @tc.desc: Test SetDataTransSilentState updates data transfer flag, silent state and change detection.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, SetDataTransSilentState_001, TestSize.Level1)
{
    AudioPipeManager pipeManager;
    auto streamDesc = MakeSilentStateStream(SILENT_TEST_SESSION_ID_1);
    AppendStreamToPipe(pipeManager, streamDesc);

    EXPECT_TRUE(pipeManager.SetDataTransSilentState(SILENT_TEST_SESSION_ID_1, true));
    EXPECT_TRUE(streamDesc->GetDataTransSilentState());
    EXPECT_TRUE(pipeManager.IsStreamSilent(SILENT_TEST_SESSION_ID_1));

    EXPECT_TRUE(pipeManager.SetDataTransSilentState(SILENT_TEST_SESSION_ID_1, false));
    EXPECT_FALSE(streamDesc->GetDataTransSilentState());
    EXPECT_FALSE(pipeManager.IsStreamSilent(SILENT_TEST_SESSION_ID_1));

    EXPECT_FALSE(pipeManager.SetDataTransSilentState(SILENT_TEST_SESSION_ID_1, false));
    EXPECT_FALSE(pipeManager.SetDataTransSilentState(SILENT_TEST_SESSION_ID_2, true));
}

/**
 * @tc.name: SetMuteSilentState_002
 * @tc.desc: Test mute flag composes with notification volume silent flag via UpdateSilentState.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, SetMuteSilentState_002, TestSize.Level1)
{
    AudioPipeManager pipeManager;
    auto streamDesc = MakeSilentStateStream(SILENT_TEST_SESSION_ID_1);
    AppendStreamToPipe(pipeManager, streamDesc);

    pipeManager.NotificationVolumeSilentClient(SILENT_TEST_SESSION_ID_1);
    EXPECT_TRUE(pipeManager.IsStreamSilent(SILENT_TEST_SESSION_ID_1));

    EXPECT_FALSE(pipeManager.SetMuteSilentState(SILENT_TEST_SESSION_ID_1, true));
    EXPECT_TRUE(pipeManager.IsStreamSilent(SILENT_TEST_SESSION_ID_1));

    // notification volume silent flag still holds, isSilent_ keeps true and no change reported
    EXPECT_FALSE(pipeManager.SetMuteSilentState(SILENT_TEST_SESSION_ID_1, false));
    EXPECT_TRUE(streamDesc->GetNotificationVolumeSilentState());
    EXPECT_TRUE(pipeManager.IsStreamSilent(SILENT_TEST_SESSION_ID_1));

    EXPECT_TRUE(pipeManager.ClearNotificationVolumeSilentFlag());
    EXPECT_FALSE(pipeManager.IsStreamSilent(SILENT_TEST_SESSION_ID_1));
}

/**
 * @tc.name: NotificationVolumeSilentClient_001
 * @tc.desc: Test NotificationVolumeSilentClient marks notification volume silent and silent state.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, NotificationVolumeSilentClient_001, TestSize.Level1)
{
    AudioPipeManager pipeManager;
    auto streamDesc = MakeSilentStateStream(SILENT_TEST_SESSION_ID_1);
    AppendStreamToPipe(pipeManager, streamDesc);

    EXPECT_FALSE(streamDesc->GetNotificationVolumeSilentState());
    EXPECT_NO_THROW(pipeManager.NotificationVolumeSilentClient(SILENT_TEST_SESSION_ID_1));
    EXPECT_TRUE(streamDesc->GetNotificationVolumeSilentState());
    EXPECT_TRUE(streamDesc->IsSilent());
    EXPECT_TRUE(pipeManager.IsStreamSilent(SILENT_TEST_SESSION_ID_1));
}

/**
 * @tc.name: NotificationVolumeSilentClient_002
 * @tc.desc: Test NotificationVolumeSilentClient with unknown session does not crash.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, NotificationVolumeSilentClient_002, TestSize.Level1)
{
    AudioPipeManager pipeManager;
    auto streamDesc = MakeSilentStateStream(SILENT_TEST_SESSION_ID_1);
    AppendStreamToPipe(pipeManager, streamDesc);

    EXPECT_NO_THROW(pipeManager.NotificationVolumeSilentClient(SILENT_TEST_SESSION_ID_4));
    EXPECT_FALSE(streamDesc->GetNotificationVolumeSilentState());
    EXPECT_FALSE(pipeManager.IsStreamSilent(SILENT_TEST_SESSION_ID_1));
    EXPECT_FALSE(pipeManager.IsStreamSilent(SILENT_TEST_SESSION_ID_4));
}

/**
 * @tc.name: NotificationVolumeSilentClient_003
 * @tc.desc: Test NotificationVolumeSilentClient on already mute-silent stream keeps silent.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, NotificationVolumeSilentClient_003, TestSize.Level1)
{
    AudioPipeManager pipeManager;
    auto streamDesc = MakeSilentStateStream(SILENT_TEST_SESSION_ID_1);
    AppendStreamToPipe(pipeManager, streamDesc);

    EXPECT_TRUE(pipeManager.SetMuteSilentState(SILENT_TEST_SESSION_ID_1, true));
    EXPECT_NO_THROW(pipeManager.NotificationVolumeSilentClient(SILENT_TEST_SESSION_ID_1));
    EXPECT_TRUE(streamDesc->GetMuteSilentState());
    EXPECT_TRUE(streamDesc->GetNotificationVolumeSilentState());
    EXPECT_TRUE(pipeManager.IsStreamSilent(SILENT_TEST_SESSION_ID_1));
}

/**
 * @tc.name: ClearNotificationVolumeSilentFlag_001
 * @tc.desc: Test ClearNotificationVolumeSilentFlag with empty pipe list returns false.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, ClearNotificationVolumeSilentFlag_001, TestSize.Level1)
{
    AudioPipeManager pipeManager;
    EXPECT_FALSE(pipeManager.ClearNotificationVolumeSilentFlag());
}

/**
 * @tc.name: ClearNotificationVolumeSilentFlag_002
 * @tc.desc: Test ClearNotificationVolumeSilentFlag without marked stream returns false and changes nothing.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, ClearNotificationVolumeSilentFlag_002, TestSize.Level1)
{
    AudioPipeManager pipeManager;
    auto streamDesc = MakeSilentStateStream(SILENT_TEST_SESSION_ID_1, STREAM_USAGE_MUSIC);
    AppendStreamToPipe(pipeManager, streamDesc);
    EXPECT_TRUE(pipeManager.SetDataTransSilentState(SILENT_TEST_SESSION_ID_1, true));

    EXPECT_FALSE(pipeManager.ClearNotificationVolumeSilentFlag());
    EXPECT_FALSE(streamDesc->GetNotificationVolumeSilentState());
    EXPECT_TRUE(pipeManager.IsStreamSilent(SILENT_TEST_SESSION_ID_1));
}

/**
 * @tc.name: ClearNotificationVolumeSilentFlag_003
 * @tc.desc: Test ClearNotificationVolumeSilentFlag clears marked stream and reports exit silent once.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, ClearNotificationVolumeSilentFlag_003, TestSize.Level1)
{
    AudioPipeManager pipeManager;
    auto streamDesc = MakeSilentStateStream(SILENT_TEST_SESSION_ID_1);
    AppendStreamToPipe(pipeManager, streamDesc);
    pipeManager.NotificationVolumeSilentClient(SILENT_TEST_SESSION_ID_1);
    EXPECT_TRUE(pipeManager.IsStreamSilent(SILENT_TEST_SESSION_ID_1));

    EXPECT_TRUE(pipeManager.ClearNotificationVolumeSilentFlag());
    EXPECT_FALSE(streamDesc->GetNotificationVolumeSilentState());
    EXPECT_FALSE(pipeManager.IsStreamSilent(SILENT_TEST_SESSION_ID_1));

    EXPECT_FALSE(pipeManager.ClearNotificationVolumeSilentFlag());
}

/**
 * @tc.name: ClearNotificationVolumeSilentFlag_004
 * @tc.desc: Test ClearNotificationVolumeSilentFlag handles nullptr pipe and descriptor safely.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, ClearNotificationVolumeSilentFlag_004, TestSize.Level1)
{
    AudioPipeManager pipeManager;
    pipeManager.curPipeList_.push_back(nullptr);

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->streamDescriptors_.push_back(nullptr);
    pipeManager.curPipeList_.push_back(pipeInfo);

    auto markedStream = MakeSilentStateStream(SILENT_TEST_SESSION_ID_1);
    AppendStreamToPipe(pipeManager, markedStream);
    pipeManager.NotificationVolumeSilentClient(SILENT_TEST_SESSION_ID_1);

    auto normalStream = MakeSilentStateStream(SILENT_TEST_SESSION_ID_2, STREAM_USAGE_MUSIC);
    AppendStreamToPipe(pipeManager, normalStream);

    bool exitSilent = false;
    EXPECT_NO_THROW(exitSilent = pipeManager.ClearNotificationVolumeSilentFlag());
    EXPECT_TRUE(exitSilent);
    EXPECT_FALSE(markedStream->GetNotificationVolumeSilentState());
    EXPECT_FALSE(normalStream->GetNotificationVolumeSilentState());
    EXPECT_FALSE(pipeManager.IsStreamSilent(SILENT_TEST_SESSION_ID_1));
}

/**
 * @tc.name: ClearNotificationVolumeSilentFlag_005
 * @tc.desc: Test ClearNotificationVolumeSilentFlag clears all marked streams in different pipes.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, ClearNotificationVolumeSilentFlag_005, TestSize.Level1)
{
    AudioPipeManager pipeManager;
    auto streamDesc1 = MakeSilentStateStream(SILENT_TEST_SESSION_ID_1);
    AppendStreamToPipe(pipeManager, streamDesc1);
    auto streamDesc2 = MakeSilentStateStream(SILENT_TEST_SESSION_ID_2);
    AppendStreamToPipe(pipeManager, streamDesc2);
    auto streamDesc3 = MakeSilentStateStream(SILENT_TEST_SESSION_ID_3, STREAM_USAGE_MUSIC);
    AppendStreamToPipe(pipeManager, streamDesc3);

    pipeManager.NotificationVolumeSilentClient(SILENT_TEST_SESSION_ID_1);
    pipeManager.NotificationVolumeSilentClient(SILENT_TEST_SESSION_ID_2);
    EXPECT_TRUE(pipeManager.IsStreamSilent(SILENT_TEST_SESSION_ID_1));
    EXPECT_TRUE(pipeManager.IsStreamSilent(SILENT_TEST_SESSION_ID_2));

    EXPECT_TRUE(pipeManager.ClearNotificationVolumeSilentFlag());
    EXPECT_FALSE(streamDesc1->GetNotificationVolumeSilentState());
    EXPECT_FALSE(streamDesc2->GetNotificationVolumeSilentState());
    EXPECT_FALSE(pipeManager.IsStreamSilent(SILENT_TEST_SESSION_ID_1));
    EXPECT_FALSE(pipeManager.IsStreamSilent(SILENT_TEST_SESSION_ID_2));
    EXPECT_FALSE(pipeManager.IsStreamSilent(SILENT_TEST_SESSION_ID_3));
}

/**
 * @tc.name: ClearNotificationVolumeSilentFlag_006
 * @tc.desc: Test ClearNotificationVolumeSilentFlag with mute silent stream reports no exit silent.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, ClearNotificationVolumeSilentFlag_006, TestSize.Level1)
{
    AudioPipeManager pipeManager;
    auto streamDesc = MakeSilentStateStream(SILENT_TEST_SESSION_ID_1);
    AppendStreamToPipe(pipeManager, streamDesc);

    pipeManager.NotificationVolumeSilentClient(SILENT_TEST_SESSION_ID_1);
    EXPECT_FALSE(pipeManager.SetMuteSilentState(SILENT_TEST_SESSION_ID_1, true));

    // stream keeps silent due to mute flag, no exit silent reported
    EXPECT_FALSE(pipeManager.ClearNotificationVolumeSilentFlag());
    EXPECT_FALSE(streamDesc->GetNotificationVolumeSilentState());
    EXPECT_TRUE(pipeManager.IsStreamSilent(SILENT_TEST_SESSION_ID_1));
}

/**
 * @tc.name: UpdateSilentState_001
 * @tc.desc: Test UpdateSilentState composes all three silent cause flags.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, UpdateSilentState_001, TestSize.Level1)
{
    AudioStreamDescriptor streamDesc;
    EXPECT_FALSE(streamDesc.IsSilent());

    streamDesc.isMuteSilent_ = true;
    streamDesc.UpdateSilentState();
    EXPECT_TRUE(streamDesc.IsSilent());

    streamDesc.isDataTransSilent_ = true;
    streamDesc.UpdateSilentState();
    EXPECT_TRUE(streamDesc.IsSilent());

    streamDesc.isNotificationVolumeSilent_ = true;
    streamDesc.UpdateSilentState();
    EXPECT_TRUE(streamDesc.IsSilent());

    streamDesc.isMuteSilent_ = false;
    streamDesc.isDataTransSilent_ = false;
    streamDesc.UpdateSilentState();
    EXPECT_TRUE(streamDesc.IsSilent());
    EXPECT_TRUE(streamDesc.GetNotificationVolumeSilentState());

    streamDesc.isNotificationVolumeSilent_ = false;
    streamDesc.UpdateSilentState();
    EXPECT_FALSE(streamDesc.IsSilent());
    EXPECT_FALSE(streamDesc.GetNotificationVolumeSilentState());
}
} // namespace AudioStandard
} // namespace OHOS
