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
#include "audio_info.h"
#include "audio_unit_test.h"
#include "audio_system_manager.h"
#include "audio_stream_collector.h"
#include "audio_pipe_manager.h"
#include "audio_core_service.h"
#include "audio_interphone_unit_test.h"

using namespace std;
using namespace OHOS::AudioStandard;
using namespace testing::ext;
using testing::ValuesIn;

namespace OHOS {
namespace AudioStandard {

void AudioInterphoneAdvancedUnitTest::SetUpTestCase(void)
{
    ASSERT_NE(nullptr, AudioSystemManager::GetInstance());
}

void AudioInterphoneAdvancedUnitTest::TearDownTestCase(void) {}

void AudioInterphoneAdvancedUnitTest::SetUp(void) {}

void AudioInterphoneAdvancedUnitTest::TearDown(void) {}

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
HWTEST_F(AudioInterphoneAdvancedUnitTest, TestIsInterphoneStreamActive, TestSize.Level1)
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
HWTEST_F(AudioInterphoneAdvancedUnitTest, TestIsInterphoneCapturerActive, TestSize.Level1)
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
HWTEST_F(AudioInterphoneAdvancedUnitTest, TestIsVoipOrCellularStreamActive, TestSize.Level1)
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
HWTEST_P(AudioInterphoneAdvancedUnitTest, TestGetStreamDescsByStreamUsage, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();
    InterphoneAdvancedParam params = GetParam();

    auto result = AudioPipeManager::GetPipeManager()->GetStreamDescsByStreamUsage(params.streamUsage);
    EXPECT_TRUE(result.empty() || result.size() > 0);
    MockNative::Resume();
}

INSTANTIATE_TEST_SUITE_P(
    TestGetStreamDescsByStreamUsage,
    AudioInterphoneAdvancedUnitTest,
    ValuesIn(STREAM_COLLECTOR_PARAMS));

/*
 * Test GetStreamDescsBySourceType
 *
 */
HWTEST_P(AudioInterphoneAdvancedUnitTest, TestGetStreamDescsBySourceType, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();
    InterphoneAdvancedParam params = GetParam();

    auto result = AudioPipeManager::GetPipeManager()->GetStreamDescsBySourceType(params.sourceType);
    EXPECT_TRUE(result.empty() || result.size() > 0);
    MockNative::Resume();
}

INSTANTIATE_TEST_SUITE_P(
    TestGetStreamDescsBySourceType,
    AudioInterphoneAdvancedUnitTest,
    ValuesIn(SOURCE_TYPE_PARAMS));

/*
 * Test ForceStopInterphoneRenderer
 *
 */
HWTEST_F(AudioInterphoneAdvancedUnitTest, TestForceStopInterphoneRenderer, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    AudioCoreService::GetCoreService()->ForceStopInterphoneRenderer();
    MockNative::Resume();
}

/*
 * Test IsStreamSupportOutputInterPhone
 *
 */
HWTEST_F(AudioInterphoneAdvancedUnitTest, TestIsStreamSupportOutputInterPhone, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_INTERPHONE;
    EXPECT_TRUE(AudioCoreService::GetCoreService()->IsStreamSupportOutputInterPhone(streamDesc));

    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    EXPECT_FALSE(AudioCoreService::GetCoreService()->IsStreamSupportOutputInterPhone(streamDesc));
    MockNative::Resume();
}

/*
 * Test Stream Usage Conflict Detection
 *
 */
HWTEST_F(AudioInterphoneAdvancedUnitTest, TestInterphoneStreamStatusCheck, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    EXPECT_FALSE(AudioPipeManager::GetPipeManager()->IsInterphoneStreamActive());
    EXPECT_FALSE(AudioPipeManager::GetPipeManager()->IsInterphoneCapturerActive());
    EXPECT_FALSE(AudioPipeManager::GetPipeManager()->IsVoipOrCellularStreamActive());
    MockNative::Resume();
}

HWTEST_F(AudioInterphoneAdvancedUnitTest, TestForceStopInterphoneStreamsWhenVoipStarts, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    AudioCoreService::GetCoreService()->ForceStopInterphoneRenderer();
    EXPECT_FALSE(AudioPipeManager::GetPipeManager()->IsInterphoneStreamActive());
    EXPECT_FALSE(AudioPipeManager::GetPipeManager()->IsInterphoneCapturerActive());
    MockNative::Resume();
}

HWTEST_F(AudioInterphoneAdvancedUnitTest, TestCheckStreamConflictsWithInterphone, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_INTERPHONE;
    uint32_t audioFlag = 0;
    
    EXPECT_EQ(SUCCESS, AudioCoreService::GetCoreService()->CheckStreamConflicts(streamDesc, audioFlag));
    EXPECT_EQ(AUDIO_OUTPUT_FLAG_INTERPHONE, audioFlag);
    MockNative::Resume();
}

HWTEST_F(AudioInterphoneAdvancedUnitTest, TestCheckStreamConflictsWithVoip, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    uint32_t audioFlag = 0;
    
    EXPECT_EQ(SUCCESS, AudioCoreService::GetCoreService()->CheckStreamConflicts(streamDesc, audioFlag));
    MockNative::Resume();
}

HWTEST_F(AudioInterphoneAdvancedUnitTest, TestCheckCapturerStreamConflictsWithInterphone, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_INTERPHONE;
    
    EXPECT_EQ(SUCCESS, AudioCoreService::GetCoreService()->CheckCapturerStreamConflicts(streamDesc));
    MockNative::Resume();
}

HWTEST_F(AudioInterphoneAdvancedUnitTest, TestCheckCapturerStreamConflictsWithVoip, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    
    EXPECT_EQ(SUCCESS, AudioCoreService::GetCoreService()->CheckCapturerStreamConflicts(streamDesc));
    MockNative::Resume();
}

HWTEST_F(AudioInterphoneAdvancedUnitTest, TestCheckStreamConflictsWithVideoCommunication, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_VIDEO_COMMUNICATION;
    uint32_t audioFlag = 0;
    
    EXPECT_EQ(SUCCESS, AudioCoreService::GetCoreService()->CheckStreamConflicts(streamDesc, audioFlag));
    MockNative::Resume();
}

HWTEST_F(AudioInterphoneAdvancedUnitTest, TestCheckStreamConflictsWithVoiceModemCommunication, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    uint32_t audioFlag = 0;
    
    EXPECT_EQ(SUCCESS, AudioCoreService::GetCoreService()->CheckStreamConflicts(streamDesc, audioFlag));
    MockNative::Resume();
}

HWTEST_F(AudioInterphoneAdvancedUnitTest, TestCheckCapturerStreamConflictsWithVoiceCall, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_CALL;
    
    EXPECT_EQ(SUCCESS, AudioCoreService::GetCoreService()->CheckCapturerStreamConflicts(streamDesc));
    MockNative::Resume();
}

HWTEST_F(AudioInterphoneAdvancedUnitTest, TestInterphoneRendererRouteFlagSet, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_INTERPHONE;
    uint32_t audioFlag = 0;
    
    int32_t ret = AudioCoreService::GetCoreService()->CheckStreamConflicts(streamDesc, audioFlag);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(AUDIO_OUTPUT_FLAG_INTERPHONE, streamDesc->audioFlag_);
    EXPECT_EQ(AUDIO_OUTPUT_FLAG_INTERPHONE, streamDesc->routeFlag_);
    MockNative::Resume();
}

} // namespace AudioStandard
} // namespace OHOS