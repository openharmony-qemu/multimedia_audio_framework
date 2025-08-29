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
    int32_t zoneId = AudioZoneService::GetInstance().CreateAudioZone("1", context, 0);
    AudioZoneService::GetInstance().AddUidToAudioZone(zoneId, 1);
    descs = audioRouterCenter.FetchOutputDevices(STREAM_USAGE_ALARM, 1, "", ROUTER_TYPE_NONE);
    EXPECT_EQ(descs.size(), 1);
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
    EXPECT_TRUE(audioRouterCenter.IsConfigRouterStrategy(type));
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

/**
 * @tc.name  : Test SetAudioDeviceRefinerCallback.
 * @tc.number: SetAudioDeviceRefinerCallback_001
 * @tc.desc  : Test SetAudioDeviceRefinerCallback interface.
 */
HWTEST(AudioRouterCenterUnitTest, SetAudioDeviceRefinerCallback_001, TestSize.Level1)
{
    AudioRouterCenter audioRouterCenter;
    sptr<IRemoteObject> object = nullptr;
    int32_t ret = audioRouterCenter.SetAudioDeviceRefinerCallback(object);
    EXPECT_EQ(ret, ERROR);
}

class MockRouter : public RouterBase {
public:
    RouterType routerType_ = ROUTER_TYPE_NONE;
    std::shared_ptr<AudioDeviceDescriptor> mediaRenderRet_;
    std::shared_ptr<AudioDeviceDescriptor> callRenderRet_;
    std::shared_ptr<AudioDeviceDescriptor> callCaptureRet_;
    std::shared_ptr<AudioDeviceDescriptor> recordCaptureRet_;
 
    MockRouter(RouterType type = ROUTER_TYPE_DEFAULT,
        std::shared_ptr<AudioDeviceDescriptor> mediaRenderRet = nullptr,
        std::shared_ptr<AudioDeviceDescriptor> callRenderRet = nullptr,
        std::shared_ptr<AudioDeviceDescriptor> callCaptureRet = nullptr,
        std::shared_ptr<AudioDeviceDescriptor> recordCaptureRet = nullptr)
        : routerType_(type),
          mediaRenderRet_(std::move(mediaRenderRet)),
          callRenderRet_(std::move(callRenderRet)),
          callCaptureRet_(std::move(callCaptureRet)),
          recordCaptureRet_(std::move(recordCaptureRet)) {}
 
    std::shared_ptr<AudioDeviceDescriptor> GetMediaRenderDevice(StreamUsage, int32_t) override
    {
        return mediaRenderRet_;
    }

    std::shared_ptr<AudioDeviceDescriptor> GetCallRenderDevice(StreamUsage, int32_t) override
    {
        return callRenderRet_;
    }

    std::shared_ptr<AudioDeviceDescriptor> GetCallCaptureDevice(SourceType, int32_t, const uint32_t) override
    {
        return callCaptureRet_;
    }

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetRingRenderDevices(StreamUsage, int32_t) override
    {
        static const std::vector<std::shared_ptr<AudioDeviceDescriptor>> emptyVector;
        return emptyVector;
    }

    std::shared_ptr<AudioDeviceDescriptor> GetRecordCaptureDevice(SourceType, int32_t, const uint32_t) override
    {
        return recordCaptureRet_;
    }

    std::shared_ptr<AudioDeviceDescriptor> GetToneRenderDevice(StreamUsage, int32_t) override
    {
        return std::shared_ptr<AudioDeviceDescriptor>();
    }

    RouterType GetRouterType() override
    {
        return routerType_;
    }
};
 
/**
 * @tc.name  : Test FetchMediaRenderDevice.
 * @tc.number: FetchMediaRenderDevice_001
 * @tc.desc  : Test FetchMediaRenderDevice interface when desc is nullptr.
 */
HWTEST(AudioRouterCenterUnitTest, FetchMediaRenderDevice_desc_nullptr, TestSize.Level1)
{
    AudioRouterCenter center;
    center.mediaRenderRouters_.emplace_back(
        std::make_unique<MockRouter>(ROUTER_TYPE_DEFAULT, nullptr));
    RouterType rtype = ROUTER_TYPE_NONE;
    auto result = center.FetchMediaRenderDevice(STREAM_USAGE_MEDIA, 123, rtype, ROUTER_TYPE_STREAM_FILTER);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_NONE);
    EXPECT_EQ(rtype, ROUTER_TYPE_NONE);
}
 
/**
 * @tc.name  : Test FetchMediaRenderDevice.
 * @tc.number: FetchMediaRenderDevice_002
 * @tc.desc  : Test FetchMediaRenderDevice interface when desc->deviceType_ is DEVICE_TYPE_NONE.
 */
HWTEST(AudioRouterCenterUnitTest, FetchMediaRenderDevice_deviceType_NONE, TestSize.Level1)
{
    AudioRouterCenter center;
    auto invalidDesc = std::make_shared<AudioDeviceDescriptor>();
    invalidDesc->deviceType_ = DEVICE_TYPE_NONE;
    center.mediaRenderRouters_.emplace_back(
        std::make_unique<MockRouter>(ROUTER_TYPE_DEFAULT, invalidDesc));
    RouterType rtype = ROUTER_TYPE_NONE;
    auto result = center.FetchMediaRenderDevice(STREAM_USAGE_MEDIA, 123, rtype, ROUTER_TYPE_STREAM_FILTER);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_NONE);
    EXPECT_EQ(rtype, ROUTER_TYPE_NONE);
}
 
/**
 * @tc.name  : Test FetchMediaRenderDevice.
 * @tc.number: FetchMediaRenderDevice_003
 * @tc.desc  : Test FetchMediaRenderDevice interface when desc->deviceType_ is valid.
 */
HWTEST(AudioRouterCenterUnitTest, FetchMediaRenderDevice_deviceType_valid, TestSize.Level1)
{
    AudioRouterCenter center;
    auto validDesc = std::make_shared<AudioDeviceDescriptor>();
    validDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    center.mediaRenderRouters_.emplace_back(
        std::make_unique<MockRouter>(ROUTER_TYPE_DEFAULT, validDesc));
    RouterType rtype = ROUTER_TYPE_NONE;
    auto result = center.FetchMediaRenderDevice(STREAM_USAGE_MEDIA, 123, rtype, ROUTER_TYPE_STREAM_FILTER);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result, validDesc);
    EXPECT_EQ(rtype, ROUTER_TYPE_DEFAULT);
}
 
/**
 * @tc.name  : Test FetchMediaRenderDevice.
 * @tc.number: FetchMediaRenderDevice_004
 * @tc.desc  : Test FetchMediaRenderDevice interface when routerType == bypassType.
 */
HWTEST(AudioRouterCenterUnitTest, FetchMediaRenderDevice_bypassType_skip, TestSize.Level1)
{
    AudioRouterCenter center;
    auto validDesc = std::make_shared<AudioDeviceDescriptor>();
    validDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    center.mediaRenderRouters_.emplace_back(
        std::make_unique<MockRouter>(ROUTER_TYPE_STREAM_FILTER, validDesc));
    RouterType rtype = ROUTER_TYPE_NONE;
    auto result = center.FetchMediaRenderDevice(STREAM_USAGE_MEDIA, 123, rtype, ROUTER_TYPE_STREAM_FILTER);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_NONE);
    EXPECT_EQ(rtype, ROUTER_TYPE_NONE);
}
 
/**
 * @tc.name  : Test FetchCallRenderDevice.
 * @tc.number: FetchCallRenderDevice_001
 * @tc.desc  : Test FetchCallRenderDevice interface when desc is nullptr.
 */
HWTEST(AudioRouterCenterUnitTest, FetchCallRenderDevice_desc_nullptr, TestSize.Level1)
{
    AudioRouterCenter center;
    center.callRenderRouters_.emplace_back(
        std::make_unique<MockRouter>(ROUTER_TYPE_DEFAULT, nullptr, nullptr));
    RouterType rtype = ROUTER_TYPE_NONE;
    auto result = center.FetchCallRenderDevice(STREAM_USAGE_MEDIA, 123,
        rtype, ROUTER_TYPE_STREAM_FILTER, ROUTER_TYPE_PACKAGE_FILTER);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_NONE);
    EXPECT_EQ(rtype, ROUTER_TYPE_NONE);
}
 
/**
 * @tc.name  : Test FetchCallRenderDevice.
 * @tc.number: FetchCallRenderDevice_002
 * @tc.desc  : Test FetchCallRenderDevice interface when desc->deviceType_ is DEVICE_TYPE_NONE.
 */
HWTEST(AudioRouterCenterUnitTest, FetchCallRenderDevice_deviceType_NONE, TestSize.Level1)
{
    AudioRouterCenter center;
    auto invalidDesc = std::make_shared<AudioDeviceDescriptor>();
    invalidDesc->deviceType_ = DEVICE_TYPE_NONE;
    center.callRenderRouters_.emplace_back(
        std::make_unique<MockRouter>(ROUTER_TYPE_DEFAULT, nullptr, invalidDesc));
    RouterType rtype = ROUTER_TYPE_NONE;
    auto result = center.FetchCallRenderDevice(STREAM_USAGE_MEDIA, 123,
        rtype, ROUTER_TYPE_STREAM_FILTER, ROUTER_TYPE_PACKAGE_FILTER);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_NONE);
    EXPECT_EQ(rtype, ROUTER_TYPE_NONE);
}
 
/**
 * @tc.name  : Test FetchCallRenderDevice.
 * @tc.number: FetchCallRenderDevice_003
 * @tc.desc  : Test FetchCallRenderDevice interface when desc->deviceType_ is valid.
 */
HWTEST(AudioRouterCenterUnitTest, FetchCallRenderDevice_deviceType_valid, TestSize.Level1)
{
    AudioRouterCenter center;
    auto validDesc = std::make_shared<AudioDeviceDescriptor>();
    validDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    center.callRenderRouters_.emplace_back(
        std::make_unique<MockRouter>(ROUTER_TYPE_DEFAULT, nullptr, validDesc));
    RouterType rtype = ROUTER_TYPE_NONE;
    auto result = center.FetchCallRenderDevice(STREAM_USAGE_MEDIA, 123,
        rtype, ROUTER_TYPE_STREAM_FILTER, ROUTER_TYPE_PACKAGE_FILTER);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result, validDesc);
    EXPECT_EQ(rtype, ROUTER_TYPE_DEFAULT);
}
 
/**
 * @tc.name  : Test FetchCallRenderDevice.
 * @tc.number: FetchCallRenderDevice_004
 * @tc.desc  : Test FetchCallRenderDevice interface when routerType == bypassType.
 */
HWTEST(AudioRouterCenterUnitTest, FetchCallRenderDevice_bypassType_skip, TestSize.Level1)
{
    AudioRouterCenter center;
    auto validDesc = std::make_shared<AudioDeviceDescriptor>();
    validDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    center.callRenderRouters_.emplace_back(
        std::make_unique<MockRouter>(ROUTER_TYPE_STREAM_FILTER, nullptr, validDesc));
    RouterType rtype = ROUTER_TYPE_NONE;
    auto result = center.FetchCallRenderDevice(STREAM_USAGE_MEDIA, 123,
        rtype, ROUTER_TYPE_STREAM_FILTER, ROUTER_TYPE_PACKAGE_FILTER);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_NONE);
    EXPECT_EQ(rtype, ROUTER_TYPE_NONE);
}
 
/**
 * @tc.name  : Test FetchCallCaptureDevice.
 * @tc.number: FetchCallCaptureDevice_001
 * @tc.desc  : Test FetchCallCaptureDevice interface when desc is nullptr.
 */
HWTEST(AudioRouterCenterUnitTest, FetchCallCaptureDevice_desc_nullptr, TestSize.Level1)
{
    AudioRouterCenter center;
    center.callCaptureRouters_.emplace_back(
        std::make_unique<MockRouter>(ROUTER_TYPE_DEFAULT, nullptr, nullptr, nullptr));
    RouterType rtype = ROUTER_TYPE_NONE;
    auto result = center.FetchCallCaptureDevice(SOURCE_TYPE_MIC, 123, rtype, 0);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_NONE);
    EXPECT_EQ(rtype, ROUTER_TYPE_NONE);
}
 
/**
 * @tc.name  : Test FetchCallCaptureDevice.
 * @tc.number: FetchCallCaptureDevice_002
 * @tc.desc  : Test FetchCallCaptureDevice interface when desc->deviceType_ is DEVICE_TYPE_NONE.
 */
HWTEST(AudioRouterCenterUnitTest, FetchCallCaptureDevice_deviceType_NONE, TestSize.Level1)
{
    AudioRouterCenter center;
    auto invalidDesc = std::make_shared<AudioDeviceDescriptor>();
    invalidDesc->deviceType_ = DEVICE_TYPE_NONE;
    center.callCaptureRouters_.emplace_back(
        std::make_unique<MockRouter>(ROUTER_TYPE_DEFAULT, nullptr, nullptr, invalidDesc));
    RouterType rtype = ROUTER_TYPE_NONE;
    auto result = center.FetchCallCaptureDevice(SOURCE_TYPE_MIC, 123, rtype, 0);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_NONE);
    EXPECT_EQ(rtype, ROUTER_TYPE_NONE);
}
 
/**
 * @tc.name  : Test FetchCallCaptureDevice.
 * @tc.number: FetchCallCaptureDevice_003
 * @tc.desc  : Test FetchCallCaptureDevice interface when desc->deviceType_ is valid.
 */
HWTEST(AudioRouterCenterUnitTest, FetchCallCaptureDevice_deviceType_valid, TestSize.Level1)
{
    AudioRouterCenter center;
    auto validDesc = std::make_shared<AudioDeviceDescriptor>();
    validDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    center.callCaptureRouters_.emplace_back(
        std::make_unique<MockRouter>(ROUTER_TYPE_DEFAULT, nullptr, nullptr, validDesc));
    RouterType rtype = ROUTER_TYPE_NONE;
    auto result = center.FetchCallCaptureDevice(SOURCE_TYPE_MIC, 123, rtype, 0);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result, validDesc);
    EXPECT_EQ(rtype, ROUTER_TYPE_DEFAULT);
}
 
/**
 * @tc.name  : Test FetchVoiceMessageCaptureDevice.
 * @tc.number: FetchVoiceMessageCaptureDevice_001
 * @tc.desc  : Test FetchVoiceMessageCaptureDevice interface when desc is nullptr.
 */
HWTEST(AudioRouterCenterUnitTest, FetchVoiceMessageCaptureDevice_desc_nullptr, TestSize.Level1)
{
    AudioRouterCenter center;
    center.voiceMessageRouters_.emplace_back(
        std::make_unique<MockRouter>(ROUTER_TYPE_DEFAULT, nullptr, nullptr, nullptr, nullptr));
    RouterType rtype = ROUTER_TYPE_NONE;
    auto result = center.FetchVoiceMessageCaptureDevice(SOURCE_TYPE_MIC, 123, rtype, 0);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_NONE);
    EXPECT_EQ(rtype, ROUTER_TYPE_NONE);
}
 
/**
 * @tc.name  : Test FetchVoiceMessageCaptureDevice.
 * @tc.number: FetchVoiceMessageCaptureDevice_002
 * @tc.desc  : Test FetchVoiceMessageCaptureDevice interface when desc->deviceType_ is DEVICE_TYPE_NONE.
 */
HWTEST(AudioRouterCenterUnitTest, FetchVoiceMessageCaptureDevice_deviceType_NONE, TestSize.Level1)
{
    AudioRouterCenter center;
    auto invalidDesc = std::make_shared<AudioDeviceDescriptor>();
    invalidDesc->deviceType_ = DEVICE_TYPE_NONE;
    center.voiceMessageRouters_.emplace_back(
        std::make_unique<MockRouter>(ROUTER_TYPE_DEFAULT, nullptr, nullptr, nullptr, invalidDesc));
    RouterType rtype = ROUTER_TYPE_NONE;
    auto result = center.FetchVoiceMessageCaptureDevice(SOURCE_TYPE_MIC, 123, rtype, 0);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->deviceType_, DEVICE_TYPE_NONE);
    EXPECT_EQ(rtype, ROUTER_TYPE_NONE);
}
 
/**
 * @tc.name  : Test FetchVoiceMessageCaptureDevice.
 * @tc.number: FetchVoiceMessageCaptureDevice_003
 * @tc.desc  : Test FetchVoiceMessageCaptureDevice interface when desc->deviceType_ is valid.
 */
HWTEST(AudioRouterCenterUnitTest, FetchVoiceMessageCaptureDevice_deviceType_valid, TestSize.Level1)
{
    AudioRouterCenter center;
    auto validDesc = std::make_shared<AudioDeviceDescriptor>();
    validDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    center.voiceMessageRouters_.emplace_back(
        std::make_unique<MockRouter>(ROUTER_TYPE_DEFAULT, nullptr, nullptr, nullptr, validDesc));
    RouterType rtype = ROUTER_TYPE_NONE;
    auto result = center.FetchVoiceMessageCaptureDevice(SOURCE_TYPE_MIC, 123, rtype, 0);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result, validDesc);
    EXPECT_EQ(rtype, ROUTER_TYPE_DEFAULT);
}
} // namespace AudioStandard
} // namespace OHOS
