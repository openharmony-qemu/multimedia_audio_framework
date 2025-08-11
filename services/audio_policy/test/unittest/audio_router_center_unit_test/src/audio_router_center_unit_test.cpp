dd
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

#include "audio_router_center_unit_test.h"
#include "audio_errors.h"
#include "audio_policy_log.h"
#include "audio_zone_service.h"

#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioRouterCenterUnitTest::SetUpTestCase(void) {}
void AudioRouterCenterUnitTest::TearDownTestCase(void) {}
void AudioRouterCenterUnitTest::SetUp(void) {}
void AudioRouterCenterUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: AudioRouterCenter_001
 * @tc.desc  : Test NeedSkipSelectAudioOutputDeviceRefined interface.
 */
HWTEST(AudioRouterCenterUnitTest, AudioRouterCenter_001, TestSize.Level1)
{
    AudioRouterCenter audioRouterCenter;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    EXPECT_FALSE(audioRouterCenter.NeedSkipSelectAudioOutputDeviceRefined(STREAM_USAGE_INVALID, descs));

    AudioPolicyManagerFactory::GetAudioPolicyManager().SetRingerMode(RINGER_MODE_VIBRATE);
    EXPECT_FALSE(audioRouterCenter.NeedSkipSelectAudioOutputDeviceRefined(STREAM_USAGE_INVALID, descs));
    EXPECT_FALSE(audioRouterCenter.NeedSkipSelectAudioOutputDeviceRefined(STREAM_USAGE_ALARM, descs));

    descs.push_back(std::move(desc));
    descs.front()->deviceType_ = DEVICE_TYPE_SPEAKER;
    EXPECT_FALSE(audioRouterCenter.NeedSkipSelectAudioOutputDeviceRefined(STREAM_USAGE_ALARM, descs));

    descs.front()->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    EXPECT_TRUE(audioRouterCenter.NeedSkipSelectAudioOutputDeviceRefined(STREAM_USAGE_ALARM, descs));
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: AudioRouterCenter_002
 * @tc.desc  : Test IsMediaFollowCallStrategy interface.
 */
HWTEST(AudioRouterCenterUnitTest, AudioRouterCenter_002, TestSize.Level1)
{
    AudioRouterCenter audioRouterCenter;
    EXPECT_TRUE(audioRouterCenter.IsMediaFollowCallStrategy(AUDIO_SCENE_PHONE_CALL));
    EXPECT_TRUE(audioRouterCenter.IsMediaFollowCallStrategy(AUDIO_SCENE_PHONE_CHAT));
    EXPECT_FALSE(audioRouterCenter.IsMediaFollowCallStrategy(AUDIO_SCENE_RINGING));
    EXPECT_FALSE(audioRouterCenter.IsMediaFollowCallStrategy(AUDIO_SCENE_VOICE_RINGING));
    EXPECT_FALSE(audioRouterCenter.IsMediaFollowCallStrategy(AUDIO_SCENE_DEFAULT));
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: AudioRouterCenter_003
 * @tc.desc  : Test FetchOutputDevices interface.
 */
HWTEST(AudioRouterCenterUnitTest, AudioRouterCenter_003, TestSize.Level1)
{
    AudioRouterCenter audioRouterCenter;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    descs = audioRouterCenter.FetchOutputDevices(STREAM_USAGE_ALARM, 0, "", ROUTER_TYPE_NONE);
    EXPECT_EQ(descs.size(), 1);
    std::shared_ptr<AudioPolicyServerHandler> handler = std::make_shared<AudioPolicyServerHandler>();
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    AudioZoneService::GetInstance().Init(handler, interruptService);
    AudioZoneContext context;
    int32_t zoneId = AudioZoneService::GetInstance().CreateAudioZone("1", context);
    AudioZoneService::GetInstance().AddUidToAudioZone(zoneId, 1);
    descs = audioRouterCenter.FetchOutputDevices(STREAM_USAGE_ALARM, 1, "", ROUTER_TYPE_NONE);
    EXPECT_EQ(descs.size(), 0);
}

/**
 * @tc.name  : Test StreamFilterRouter.
 * @tc.number: AudioRouterCenter_004
 * @tc.desc  : Test IsConfigRouterStrategy interface.
 */
HWTEST(AudioRouterCenterUnitTest, AudioRouterCenter_004, TestSize.Level1)
{
    AudioRouterCenter audioRouterCenter;
    EXPECT_TRUE(audioRouterCenter.IsConfigRouterStrategy(SOURCE_TYPE_MIC));
    EXPECT_TRUE(audioRouterCenter.IsConfigRouterStrategy(SOURCE_TYPE_VOICE_COMMUNICATION));
    EXPECT_TRUE(audioRouterCenter.IsConfigRouterStrategy(SOURCE_TYPE_VOICE_MESSAGE));
    EXPECT_FALSE(audioRouterCenter.IsConfigRouterStrategy(SOURCE_TYPE_INVALID));
}

/**
 * @tc.name  : Test IsMediaFollowCallStrategy.
 * @tc.number: AudioRouterCenter_005
 * @tc.desc  : Test IsMediaFollowCallStrategy interface.
 */
HWTEST(AudioRouterCenterUnitTest, AudioRouterCenter_005, TestSize.Level1)
{
    AudioRouterCenter audioRouterCenter;

    EXPECT_TRUE(audioRouterCenter.IsMediaFollowCallStrategy(AUDIO_SCENE_PHONE_CALL));
    EXPECT_TRUE(audioRouterCenter.IsMediaFollowCallStrategy(AUDIO_SCENE_PHONE_CHAT));
    EXPECT_FALSE(audioRouterCenter.IsMediaFollowCallStrategy(AUDIO_SCENE_RINGING));
    EXPECT_FALSE(audioRouterCenter.IsMediaFollowCallStrategy(AUDIO_SCENE_VOICE_RINGING));
    EXPECT_FALSE(audioRouterCenter.IsMediaFollowCallStrategy(AUDIO_SCENE_DEFAULT));
}

/**
 * @tc.name  : Test IsConfigRouterStrategy.
 * @tc.number: AudioRouterCenter_006
 * @tc.desc  : Test IsConfigRouterStrategy interface.
 */
HWTEST(AudioRouterCenterUnitTest, AudioRouterCenter_006, TestSize.Level1)
{
    AudioRouterCenter audioRouterCenter;
    SourceType type = SOURCE_TYPE_MIC;
    EXPECT_TRUE(audioRouterCenter.IsConfigRouterStrategy(type));
    type = SOURCE_TYPE_VOICE_COMMUNICATION;
    EXPECT_TRUE(audioRouterCenter.IsConfigRouterStrategy(type));
    type = SOURCE_TYPE_VOICE_MESSAGE;
    EXPECT_FALSE(audioRouterCenter.IsConfigRouterStrategy(type));
    type = SOURCE_TYPE_INVALID;
    EXPECT_FALSE(audioRouterCenter.IsConfigRouterStrategy(type));
}

/**
 * @tc.name  : Test FetchRingRenderDevices.
 * @tc.number: FetchRingRenderDevices_001
 * @tc.desc  : Test FetchRingRenderDevices interface.
 */
HWTEST(AudioRouterCenterUnitTest, FetchRingRenderDevices_001, TestSize.Level1)
{
    AudioRouterCenter audioRouterCenter;
    StreamUsage streamUsage = STREAM_USAGE_RINGTONE;
    int32_t clientUID = 1000;
    RouterType routerType;
    auto result = audioRouterCenter.FetchRingRenderDevices(streamUsage, clientUID, routerType);
    EXPECT_EQ(result.front()->deviceType_, DEVICE_TYPE_NONE);
}

/**
 * @tc.name  : Test FetchVoiceMessageCaptureDevice.
 * @tc.number: FetchVoiceMessageCaptureDevice_001
 * @tc.desc  : Test FetchVoiceMessageCaptureDevice interface.
 */
HWTEST(AudioRouterCenterUnitTest, FetchVoiceMessageCaptureDevice_001, TestSize.Level1)
{
    AudioRouterCenter audioRouterCenter;
    SourceType sourceType = SOURCE_TYPE_MIC;
    int32_t clientUID = 1000;
    RouterType routerType;
    uint32_t sessionID = 123;
    auto result = audioRouterCenter.FetchVoiceMessageCaptureDevice(sourceType, clientUID, routerType, sessionID);
    EXPECT_NE(result, nullptr);
}
} // namespace AudioStandard
} // namespace OHOS
 