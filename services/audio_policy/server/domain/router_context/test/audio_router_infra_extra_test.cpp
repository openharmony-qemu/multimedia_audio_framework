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

#include "audio_router_context_test_base.h"
#include "audio_router_infra.h"
#include "audio_info.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class AudioRouterInfraExtraTest : public AudioRouterContextTestBase {
};

HWTEST_F(AudioRouterInfraExtraTest, AudioRouterInfraExtraTest_UpdateOutputStreamState_001,
    TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1,
        STREAM_USAGE_MEDIA, RendererState::RENDERER_RUNNING);
    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_2,
        STREAM_USAGE_ALARM, RendererState::RENDERER_RUNNING);
    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_3,
        STREAM_USAGE_VOICE_COMMUNICATION, RendererState::RENDERER_RUNNING);

    auto result1 = routerInfra.GetStreamStreamUsage(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    auto result2 = routerInfra.GetStreamStreamUsage(TEST_APP_UID_10001, TEST_STREAM_ID_2);
    auto result3 = routerInfra.GetStreamStreamUsage(TEST_APP_UID_10001, TEST_STREAM_ID_3);

    EXPECT_EQ(result1, STREAM_USAGE_MEDIA);
    EXPECT_EQ(result2, STREAM_USAGE_ALARM);
    EXPECT_EQ(result3, STREAM_USAGE_VOICE_COMMUNICATION);
}

HWTEST_F(AudioRouterInfraExtraTest, AudioRouterInfraExtraTest_UpdateOutputStreamState_002,
    TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1,
        STREAM_USAGE_MEDIA, RendererState::RENDERER_RUNNING);
    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1,
        STREAM_USAGE_MEDIA, RendererState::RENDERER_STOPPED);

    auto result1 = routerInfra.GetStreamStreamUsage(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    EXPECT_EQ(result1, STREAM_USAGE_INVALID);
    auto result2 = routerInfra.GetAppRunningStreamUsages(routerInfra.GetHighestOutputPriorityApp());
    EXPECT_TRUE(result2.empty());
    auto result3 = routerInfra.GetStreamStreamUsage(TEST_APP_UID_10002, TEST_STREAM_ID_1);
    EXPECT_EQ(result3, STREAM_USAGE_INVALID);
}

HWTEST_F(AudioRouterInfraExtraTest, AudioRouterInfraExtraTest_UpdateOutputStreamState_003,
    TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    std::vector<StreamUsage> streamUsages1 = { STREAM_USAGE_MEDIA };
    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1,
        STREAM_USAGE_MEDIA, RendererState::RENDERER_RUNNING);
    auto result1 = routerInfra.GetAppRunningStreamUsages(routerInfra.GetHighestOutputPriorityApp());
    ASSERT_EQ(result1.size(), 1);
    EXPECT_EQ(result1[0], STREAM_USAGE_MEDIA);

    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1,
        STREAM_USAGE_MEDIA, RendererState::RENDERER_PAUSED);
    auto result2 = routerInfra.GetStreamStreamUsage(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    EXPECT_EQ(result2, STREAM_USAGE_INVALID);
}

HWTEST_F(AudioRouterInfraExtraTest, AudioRouterInfraExtraTest_UpdateOutputStreamState_005,
    TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    routerInfra.UpdateAppForegroundState(TEST_APP_UID_10001, true);
    routerInfra.UpdateAppForegroundState(TEST_APP_UID_10001, false);

    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10001, TEST_STREAM_ID_1,
        STREAM_USAGE_MEDIA, RendererState::RENDERER_RUNNING);
    routerInfra.UpdateOutputStreamState(TEST_APP_UID_10002, TEST_STREAM_ID_2,
        STREAM_USAGE_ALARM, RendererState::RENDERER_RUNNING);

    auto result1 = routerInfra.GetStreamStreamUsage(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    auto result2 = routerInfra.GetStreamStreamUsage(TEST_APP_UID_10002, TEST_STREAM_ID_2);

    EXPECT_EQ(result1, STREAM_USAGE_MEDIA);
    EXPECT_EQ(result2, STREAM_USAGE_ALARM);

    auto result = routerInfra.GetAppRunningStreamUsages(routerInfra.GetHighestOutputPriorityApp());
    ASSERT_EQ(result.size(), 1);
    EXPECT_TRUE(std::find(result.begin(), result.end(), STREAM_USAGE_MEDIA) == result.end());
    EXPECT_TRUE(std::find(result.begin(), result.end(), STREAM_USAGE_ALARM) != result.end());
}

HWTEST_F(AudioRouterInfraExtraTest, AudioRouterInfraExtraTest_ParamAbrnormal_001,
    TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();
    routerInfra.RefreshSelectDevice(SelectDeviceType::INVALID_TYPE, TEST_APP_UID_10001, nullptr);
    routerInfra.RefreshSelectDevice(SelectDeviceType::INVALID_TYPE, SYSTEM_UID, nullptr);
    routerInfra.RefreshSelectDevice(SelectDeviceType::INVALID_TYPE, SYSTEM_UID, nullptr);

    auto result1 = routerInfra.GetSystemSelectDevice(SelectDeviceType::INVALID_TYPE);
    EXPECT_EQ(result1.device_, nullptr);
    auto result2 = routerInfra.GetAppSelectDevice(SelectDeviceType::INVALID_TYPE, TEST_APP_UID_10001);
    EXPECT_EQ(result2.device_, nullptr);

    routerInfra.UpdatePreferredInputCategory(TEST_APP_UID_10001, PREFERRED_NONE);
    auto result5 = routerInfra.GetHighestPriorityPreferredInputCategory();
    EXPECT_EQ(result5, PREFERRED_NONE);
}

HWTEST_F(AudioRouterInfraExtraTest, AudioRouterInfraExtraTest_ParamAbrnormal_002,
    TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();

    std::vector<SourceType> sourceTypes = { SourceType::SOURCE_TYPE_MIC };
    auto result1 = routerInfra.HasHighestPriorityRunningSourceType(sourceTypes);
    EXPECT_EQ(result1, false);

    int32_t highestUid = routerInfra.GetHighestInputPriorityApp();
    auto result3 = routerInfra.GetAppRunningSourceTypes(highestUid);
    EXPECT_EQ(result3.size(), 0);
    highestUid = routerInfra.GetHighestOutputPriorityApp();
    auto result4 = routerInfra.GetAppRunningStreamUsages(highestUid);
    EXPECT_EQ(result4.size(), 0);

    auto result5 = routerInfra.GetStreamSourceType(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    EXPECT_EQ(result5, SourceType::SOURCE_TYPE_INVALID);
    auto result6 = routerInfra.GetStreamStreamUsage(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    EXPECT_EQ(result6, StreamUsage::STREAM_USAGE_INVALID);

    auto result7 = routerInfra.GetStreamSelectDevice(TEST_APP_UID_10001, TEST_STREAM_ID_1);
    EXPECT_EQ(result7, nullptr);
}

HWTEST_F(AudioRouterInfraExtraTest, AudioRouterInfraExtraTest_ParamAbrnormal_003,
    TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();
    routerInfra.ExcludeDevice(nullptr, MEDIA_OUTPUT_DEVICES);
    routerInfra.UnexcludeDevice(nullptr, MEDIA_OUTPUT_DEVICES);
    auto result1 = routerInfra.IsDeviceExcluded(nullptr, MEDIA_OUTPUT_DEVICES);
    EXPECT_EQ(result1, false);
}

HWTEST_F(AudioRouterInfraExtraTest, AudioRouterInfraExtraTest_AdjustOutputPriority_001,
    TestSize.Level1)
{
    auto &routerInfra = AudioRouterInfra::GetInstance();
    int32_t uid = TEST_APP_UID_10001;
    routerInfra.AdjustOutputPriority(uid, false, true, true);
    routerInfra.AdjustOutputPriority(uid, true, true, true);
    routerInfra.AdjustOutputPriority(uid, true, false, true);
    routerInfra.AdjustOutputPriority(uid, false, true, false);
    routerInfra.AdjustOutputPriority(uid, true, true, false);
    routerInfra.AdjustOutputPriority(uid, true, false, false);
    EXPECT_EQ(uid, TEST_APP_UID_10001);
}
}
}
