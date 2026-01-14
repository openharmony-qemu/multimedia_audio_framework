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

#include "audio_bus_selector_unit_test.h"
#include "audio_bus_selector.h"
#include "mock_audio_pipe_manager.h"
#include "mock_audio_zone_service.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioBusSelectorUnitTest::SetUpTestCase(void) {}
void AudioBusSelectorUnitTest::TearDownTestCase(void) {}
void AudioBusSelectorUnitTest::SetUp(void) {}
void AudioBusSelectorUnitTest::TearDown(void) {}

/**
 * @tc.name: SetCustomAudioMix_001
 * @tc.desc: Test that when a valid zoneName and non-empty audioMixes are provided,
 * the function should successfully return SUCCESS.
 * @tc.type: FUNC
 * @tc.require: #10496
 */
HWTEST_F(AudioBusSelectorUnitTest, SetCustomAudioMix_001, TestSize.Level1)
{
    const std::string zoneName = "test_zone";
    std::vector<AudioMix> audioMixes;
    AudioMix mix;
    mix.busAddress = "test_bus";
    audioMixes.push_back(mix);

    int32_t result = AudioBusSelector::GetBusSelector().SetCustomAudioMix(zoneName, audioMixes);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: SetCustomAudioMix_002
 * @tc.desc: Test that the function should return ERR_INVALID_PARAM when zoneName is empty.
 * @tc.type: FUNC
 * @tc.require: #10496
 */
HWTEST_F(AudioBusSelectorUnitTest, SetCustomAudioMix_002, TestSize.Level1)
{
    const std::string zoneName = "";
    std::vector<AudioMix> audioMixes;
    AudioMix mix;
    mix.busAddress = "test_bus";
    audioMixes.push_back(mix);

    int32_t result = AudioBusSelector::GetBusSelector().SetCustomAudioMix(zoneName, audioMixes);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name: SetCustomAudioMix_003
 * @tc.desc: Test that the function should return ERR_INVALID_PARAM when zoneName is empty.
 * @tc.type: FUNC
 * @tc.require: #10496
 */
HWTEST_F(AudioBusSelectorUnitTest, SetCustomAudioMix_003, TestSize.Level1)
{
    const std::string zoneName = "test_zone";
    std::vector<AudioMix> audioMixes;

    int32_t result = AudioBusSelector::GetBusSelector().SetCustomAudioMix(zoneName, audioMixes);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name: GetBusAddressesByStreamDesc_001
 * @tc.desc: Test that the function returns an empty vector when streamDesc is null.
 * @tc.type: FUNC
 * @tc.require: #10496
 */
HWTEST_F(AudioBusSelectorUnitTest, GetBusAddressesByStreamDesc_001, TestSize.Level1)
{
    MockAudioZoneService audioZoneServiceMock;
    AudioBusSelector busSelector(audioZoneServiceMock, std::make_shared<MockAudioPipeManager>());
    EXPECT_CALL(audioZoneServiceMock, FindAudioZoneNameByUid(::testing::_)).WillOnce(::testing::Return("primary"));

    auto result = busSelector.GetBusAddressesByStreamDesc(nullptr);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: GetBusAddressesByStreamDesc_002
 * @tc.desc: Test that the function returns an empty vector when FindAudioZoneNameByUid return null.
 * @tc.type: FUNC
 * @tc.require: #10496
 */
HWTEST_F(AudioBusSelectorUnitTest, GetBusAddressesByStreamDesc_002, TestSize.Level1)
{
    MockAudioZoneService audioZoneServiceMock;
    AudioBusSelector busSelector(audioZoneServiceMock, std::make_shared<MockAudioPipeManager>());
    EXPECT_CALL(audioZoneServiceMock, FindAudioZoneNameByUid(::testing::_)).WillOnce(::testing::Return(""));

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->callerUid_ = 123;
    auto result = busSelector.GetBusAddressesByStreamDesc(streamDesc);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: GetBusAddressesByStreamDesc_003
 * @tc.desc: Test that the function returns an empty vector when FindAudioZoneNameByUid return null.
 * @tc.type: FUNC
 * @tc.require: #10496
 */
HWTEST_F(AudioBusSelectorUnitTest, GetBusAddressesByStreamDesc_003, TestSize.Level1)
{
    MockAudioZoneService audioZoneServiceMock;
    AudioBusSelector busSelector(audioZoneServiceMock, std::make_shared<MockAudioPipeManager>());
    EXPECT_CALL(audioZoneServiceMock, FindAudioZoneNameByUid(::testing::_)).WillOnce(::testing::Return("primary-2"));

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->callerUid_ = 123;
    auto result = busSelector.GetBusAddressesByStreamDesc(streamDesc);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: GetBusAddressesByStreamDesc_004
 * @tc.desc: Test that the function returns an empty vector when there is no matching AudioMix.
 * @tc.type: FUNC
 * @tc.require: #10496
 */
HWTEST_F(AudioBusSelectorUnitTest, GetBusAddressesByStreamDesc_004, TestSize.Level1)
{
    MockAudioZoneService audioZoneServiceMock;
    AudioBusSelector busSelector(audioZoneServiceMock, std::make_shared<MockAudioPipeManager>());
    EXPECT_CALL(audioZoneServiceMock, FindAudioZoneNameByUid(::testing::_)).WillOnce(::testing::Return("primary"));

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->callerUid_ = 123;
    streamDesc->rendererInfo_.encodingType = AudioEncodingType::ENCODING_PCM;
    streamDesc->rendererInfo_.streamUsage = StreamUsage::STREAM_USAGE_MUSIC;
    AudioMix audioMix;
    audioMix.streamUsages = {};
    audioMix.encodingType = AudioEncodingType::ENCODING_PCM;
    busSelector.audioMixMap_["primary"] = {audioMix};
    auto result = busSelector.GetBusAddressesByStreamDesc(streamDesc);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: GetBusAddressesByStreamDesc_005
 * @tc.desc: Test that the function returns the correct bus addresses when there is a matching AudioMix.
 * @tc.type: FUNC
 * @tc.require: #10496
 */
HWTEST_F(AudioBusSelectorUnitTest, GetBusAddressesByStreamDesc_005, TestSize.Level1)
{
    MockAudioZoneService audioZoneServiceMock;
    AudioBusSelector busSelector(audioZoneServiceMock, std::make_shared<MockAudioPipeManager>());
    EXPECT_CALL(audioZoneServiceMock, FindAudioZoneNameByUid(::testing::_)).WillOnce(::testing::Return("primary"));

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->callerUid_ = 123;
    streamDesc->rendererInfo_.encodingType = AudioEncodingType::ENCODING_PCM;
    streamDesc->rendererInfo_.streamUsage = StreamUsage::STREAM_USAGE_MUSIC;
    AudioMix audioMix;
    audioMix.streamUsages = {StreamUsage::STREAM_USAGE_MUSIC};
    audioMix.encodingType = AudioEncodingType::ENCODING_PCM;
    audioMix.deviceType = DEVICE_TYPE_NONE;
    audioMix.deviceRole = DEVICE_ROLE_NONE;
    audioMix.busAddress = "bus1";
    busSelector.audioMixMap_["primary"] = {audioMix};
    auto result = busSelector.GetBusAddressesByStreamDesc(streamDesc);
    EXPECT_FALSE(result.empty());
}

/**
 * @tc.name: GetSinkNameByStreamId_001
 * @tc.desc: Test that the function returns the corresponding module name when the streamId matches.
 * @tc.type: FUNC
 * @tc.require: #10496
 */
HWTEST_F(AudioBusSelectorUnitTest, GetSinkNameByStreamId_001, TestSize.Level1)
{
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->moduleInfo_.name = "TestModule";
    pipeInfo->streamDescriptors_.emplace_back(std::make_shared<AudioStreamDescriptor>());
    pipeInfo->streamDescriptors_.front()->sessionId_ = 123;
    const std::vector<std::shared_ptr<AudioPipeInfo>> pipeList{pipeInfo};

    MockAudioZoneService audioZoneServiceMock;
    std::shared_ptr<MockAudioPipeManager> audioPipeManagerMock = std::make_shared<MockAudioPipeManager>();
    AudioBusSelector busSelector(audioZoneServiceMock, audioPipeManagerMock);
    EXPECT_CALL(*audioPipeManagerMock, GetPipeList()).WillOnce(::testing::Return(pipeList));
    auto result = busSelector.GetSinkNameByStreamId(123);
    EXPECT_EQ(result, "TestModule");
}

/**
 * @tc.name: GetSinkNameByStreamId_002
 * @tc.desc: Test that the function returns the corresponding module name when no streamId matches.
 * @tc.type: FUNC
 * @tc.require: #10496
 */
HWTEST_F(AudioBusSelectorUnitTest, GetSinkNameByStreamId_002, TestSize.Level1)
{
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->moduleInfo_.name = "TestModule";
    pipeInfo->streamDescriptors_.emplace_back(std::make_shared<AudioStreamDescriptor>());
    pipeInfo->streamDescriptors_.front()->sessionId_ = 123;
    const std::vector<std::shared_ptr<AudioPipeInfo>> pipeList{pipeInfo};

    MockAudioZoneService audioZoneServiceMock;
    std::shared_ptr<MockAudioPipeManager> audioPipeManagerMock = std::make_shared<MockAudioPipeManager>();
    AudioBusSelector busSelector(audioZoneServiceMock, audioPipeManagerMock);
    EXPECT_CALL(*audioPipeManagerMock, GetPipeList()).WillOnce(::testing::Return(pipeList));
    auto result = busSelector.GetSinkNameByStreamId(456);
    EXPECT_EQ(result, PORT_NONE);
}
}
}
 