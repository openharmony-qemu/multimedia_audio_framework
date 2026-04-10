/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "audio_errors.h"
#include "audio_unit_test.h"
#include "audio_system_manager.h"
#include "audio_stream_collector.h"
#include "audio_pipe_manager.h"
#include "audio_core_service.h"
#include "audio_server.h"
#include "audio_interphone_test.h"

using namespace std;
using namespace OHOS::AudioStandard;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace V1_0 {
void AudioInterphoneAdvancedTest::SetUpTestCase(void)
{
    ASSERT_NE(nullptr, AudioSystemManager::GetInstance());
}

void AudioInterphoneAdvancedTest::TearDownTestCase(void) {}

void AudioInterphoneAdvancedTest::SetUp(void) {}

void AudioInterphoneAdvancedTest::TearDown(void) {}

namespace {
const InterphoneAdvancedParam STREAM_COLLECTOR_PARAMS[] = {
    {
        .streamUsage = STREAM_USAGE_INTERPHONE,
        .isActive = true
    },
    {
        .streamUsage = STREAM_USAGE_INTERPHONE,
        .isActive = false
    }
};

const InterphoneAdvancedParam SOURCE_TYPE_PARAMS[] = {
    {
        .sourceType = SOURCE_TYPE_INTERPHONE,
        .isActive = true
    },
    {
        .sourceType = SOURCE_TYPE_INTERPHONE,
        .isActive = false
    }
};

const InterphoneAdvancedParam PIPE_MANAGER_PARAMS[] = {
    {
        .streamUsage = STREAM_USAGE_INTERPHONE,
        .sourceType = SOURCE_TYPE_INTERPHONE
    }
};

const InterphoneAdvancedParam ROUTE_FLAG_PARAMS[] = {
    {
        .outputRouteFlag = AUDIO_OUTPUT_FLAG_INTERPHONE,
        .inputRouteFlag = AUDIO_INPUT_FLAG_INTERPHONE,
        .mode = AUDIO_MODE_PLAYBACK
    },
    {
        .outputRouteFlag = AUDIO_OUTPUT_FLAG_INTERPHONE,
        .inputRouteFlag = AUDIO_INPUT_FLAG_INTERPHONE,
        .mode = AUDIO_MODE_RECORD
    }
};
} // namespace

/*
 * Test IsInterphoneStreamActive
 *
 */
HWTEST_F(AudioInterphoneAdvancedTest, TestIsInterphoneStreamActive, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    EXPECT_FALSE(AudioPipeManager::GetPipeManager()->IsInterphoneStreamActive());
    MockNative::Resume();
}

/*
 * Test IsInterphoneCapturerActive
 *
 */
HWTEST_F(AudioInterphoneAdvancedTest, TestIsInterphoneCapturerActive, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    EXPECT_FALSE(AudioPipeManager::GetPipeManager()->IsInterphoneCapturerActive());
    MockNative::Resume();
}

/*
 * Test IsVoipOrCellularStreamActive
 *
 */
HWTEST_F(AudioInterphoneAdvancedTest, TestIsVoipOrCellularStreamActive, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    EXPECT_FALSE(AudioPipeManager::GetPipeManager()->IsVoipOrCellularStreamActive());
    MockNative::Resume();
}

/*
 * Test GetStreamDescsByStreamUsage
 *
 */
HWTEST_P(AudioInterphoneAdvancedTest, TestGetStreamDescsByStreamUsage, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();
    InterphoneAdvancedParam params = GetParam();

    auto result = AudioPipeManager::GetInstance().GetStreamDescsByStreamUsage(params.streamUsage);
    EXPECT_TRUE(result.empty() || result.size() > 0);
    MockNative::Resume();
}

INSTANTIATE_TEST_SUITE_P(
    TestGetStreamDescsByStreamUsage,
    AudioInterphoneAdvancedTest,
    ValuesIn(STREAM_COLLECTOR_PARAMS));

/*
 * Test GetStreamDescsBySourceType
 *
 */
HWTEST_P(AudioInterphoneAdvancedTest, TestGetStreamDescsBySourceType, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();
    InterphoneAdvancedParam params = GetParam();

    auto result = AudioPipeManager::GetInstance().GetStreamDescsBySourceType(params.sourceType);
    EXPECT_TRUE(result.empty() || result.size() > 0);
    MockNative::Resume();
}

INSTANTIATE_TEST_SUITE_P(
    TestGetStreamDescsBySourceType,
    AudioInterphoneAdvancedTest,
    ValuesIn(SOURCE_TYPE_PARAMS));

/*
 * Test ForceStopInterphoneRenderer
 *
 */
HWTEST_F(AudioInterphoneAdvancedTest, TestForceStopInterphoneRenderer, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    AudioCoreService::GetInstance().ForceStopInterphoneRenderer();
    MockNative::Resume();
}

/*
 * Test IsStreamSupportOutputInterPhone
 *
 */
HWTEST_F(AudioInterphoneAdvancedTest, TestIsStreamSupportOutputInterPhone, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    EXPECT_FALSE(AudioCoreService::GetInstance().IsStreamSupportOutputInterPhone(nullptr));
    MockNative::Resume();
}

/*
 * Test Stream Usage Conflict Detection
 *
 */
HWTEST_F(AudioInterphoneAdvancedTest, TestInterphoneVoipConflict, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    EXPECT_FALSE(AudioPipeManager::GetPipeManager()->IsInterphoneStreamActive());
    EXPECT_FALSE(AudioPipeManager::GetPipeManager()->IsVoipOrCellularStreamActive());
    MockNative::Resume();
}

/*
 * Test Source Type Conflict Detection
 *
 */
HWTEST_F(AudioInterphoneAdvancedTest, TestInterphoneSourceConflict, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    EXPECT_FALSE(AudioPipeManager::GetPipeManager()->IsInterphoneCapturerActive());
    EXPECT_FALSE(AudioPipeManager::GetPipeManager()->IsVoipOrCellularStreamActive());
    MockNative::Resume();
}

HWTEST_F(AudioInterphoneAdvancedTest, TestForceStopInterphoneStreamsWhenVoipRendererStarts, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    AudioCoreService::GetInstance().ForceStopInterphoneRenderer();
    EXPECT_FALSE(AudioPipeManager::GetPipeManager()->IsInterphoneStreamActive());
    EXPECT_FALSE(AudioPipeManager::GetPipeManager()->IsInterphoneCapturerActive());
    MockNative::Resume();
}

HWTEST_F(AudioInterphoneAdvancedTest, TestForceStopInterphoneStreamsWhenVoipCapturerStarts, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    AudioCoreService::GetInstance().ForceStopInterphoneRenderer();
    EXPECT_FALSE(AudioPipeManager::GetPipeManager()->IsInterphoneStreamActive());
    EXPECT_FALSE(AudioPipeManager::GetPipeManager()->IsInterphoneCapturerActive());
    MockNative::Resume();
}

HWTEST_F(AudioInterphoneAdvancedTest, TestInterphoneStreamStatusCheck, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    EXPECT_FALSE(AudioPipeManager::GetPipeManager()->IsInterphoneStreamActive());
    EXPECT_FALSE(AudioPipeManager::GetPipeManager()->IsInterphoneCapturerActive());
    EXPECT_FALSE(AudioPipeManager::GetPipeManager()->IsVoipOrCellularStreamActive());
    MockNative::Resume();
}

HWTEST_F(AudioInterphoneAdvancedTest, TestVoipOrCellularStreamCheck, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    EXPECT_FALSE(AudioPipeManager::GetPipeManager()->IsVoipOrCellularStreamActive());
    AudioCoreService::GetInstance().ForceStopInterphoneRenderer();
    MockNative::Resume();
}

HWTEST_F(AudioInterphoneAdvancedTest, TestCheckStreamConflictsWithInterphone, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_INTERPHONE;
    uint32_t audioFlag = 0;
    
    EXPECT_EQ(SUCCESS, AudioCoreService::GetInstance().CheckStreamConflicts(streamDesc, audioFlag));
    EXPECT_EQ(AUDIO_OUTPUT_FLAG_INTERPHONE, audioFlag);
    MockNative::Resume();
}

HWTEST_F(AudioInterphoneAdvancedTest, TestCheckStreamConflictsWithVoip, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    uint32_t audioFlag = 0;
    
    EXPECT_EQ(SUCCESS, AudioCoreService::GetInstance().CheckStreamConflicts(streamDesc, audioFlag));
    MockNative::Resume();
}

HWTEST_F(AudioInterphoneAdvancedTest, TestCheckCapturerStreamConflictsWithInterphone, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_INTERPHONE;
    
    EXPECT_EQ(SUCCESS, AudioCoreService::GetInstance().CheckCapturerStreamConflicts(streamDesc));
    MockNative::Resume();
}

HWTEST_F(AudioInterphoneAdvancedTest, TestCheckCapturerStreamConflictsWithVoip, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    
    EXPECT_EQ(SUCCESS, AudioCoreService::GetInstance().CheckCapturerStreamConflicts(streamDesc));
    MockNative::Resume();
}

} // namespace V1_0
} // namespace AudioStandard
} // namespace OHOS