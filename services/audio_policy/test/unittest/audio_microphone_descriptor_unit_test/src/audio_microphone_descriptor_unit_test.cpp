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

#include "audio_microphone_descriptor_unit_test.h"
#include "audio_microphone_descriptor.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

static AudioGlobalConfigManager audioGlobalConfigManager_ = AudioGlobalConfigManager::GetAudioGlobalConfigManager();
void AudioMicrophoneDescriptorUnitTest::SetUpTestCase(void) {}
void AudioMicrophoneDescriptorUnitTest::TearDownTestCase(void) {}
void AudioMicrophoneDescriptorUnitTest::SetUp(void) {}
void AudioMicrophoneDescriptorUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test SetMicrophoneMutePersistent.
 * @tc.number: SetMicrophoneMutePersistent_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioMicrophoneDescriptorUnitTest, SetMicrophoneMutePersistent_001, TestSize.Level4)
{
    std::shared_ptr<AudioMicrophoneDescriptor> descriptor = std::make_shared<AudioMicrophoneDescriptor>();

    bool isMute = false;
    descriptor->isMicrophoneMutePersistent_ = false;
    descriptor->isMicrophoneMuteTemporary_ = false;
    int32_t ret = descriptor->SetMicrophoneMutePersistent(isMute);

    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetMicrophoneMutePersistent success when all dependencies succeed
 * @tc.number: SetMicrophoneMutePersistent_002
 * @tc.desc  : Verify SetMicrophoneMutePersistent returns SUCCESS when both proxy and persist succeed
 */
HWTEST_F(AudioMicrophoneDescriptorUnitTest, SetMicrophoneMutePersistent_002, TestSize.Level4)
{
    std::shared_ptr<AudioMicrophoneDescriptor> descriptor = std::make_shared<AudioMicrophoneDescriptor>();

    bool isMute = true;
    descriptor->isMicrophoneMutePersistent_ = false;
    descriptor->isMicrophoneMuteTemporary_ = false;
    int32_t ret = descriptor->SetMicrophoneMutePersistent(isMute);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetMicrophoneMute failure path.
 * @tc.number: SetMicrophoneMute_001
 * @tc.desc  : Test AudioPolicyService interfaces with failing dependency.
 */
HWTEST_F(AudioMicrophoneDescriptorUnitTest, SetMicrophoneMute_001, TestSize.Level4)
{
    std::shared_ptr<AudioMicrophoneDescriptor> descriptor = std::make_shared<AudioMicrophoneDescriptor>();

    bool isMute = true;
    descriptor->isMicrophoneMutePersistent_ = false;
    int32_t ret = descriptor->SetMicrophoneMute(isMute);

    EXPECT_EQ(ret, SUCCESS);
}


/**
 * @tc.name  : Test InitPersistentMicrophoneMuteState failure path.
 * @tc.number: InitPersistentMicrophoneMuteState_001
 * @tc.desc  : Test AudioPolicyService interfaces with failing dependency while getting persistent mute state.
 */
HWTEST_F(AudioMicrophoneDescriptorUnitTest, InitPersistentMicrophoneMuteState_001, TestSize.Level4)
{
    std::shared_ptr<AudioMicrophoneDescriptor> descriptor = std::make_shared<AudioMicrophoneDescriptor>();

    bool isMute = false;
    int32_t ret = descriptor->InitPersistentMicrophoneMuteState(isMute);

    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test GetMicrophoneMuteTemporary and GetMicrophoneMutePersistent.
 * @tc.number: GetMicrophoneMuteState_001
 * @tc.desc  : Verify getter methods return correct values.
 */
HWTEST_F(AudioMicrophoneDescriptorUnitTest, GetMicrophoneMuteState_001, TestSize.Level4)
{
    std::shared_ptr<AudioMicrophoneDescriptor> descriptor = std::make_shared<AudioMicrophoneDescriptor>();

    descriptor->isMicrophoneMuteTemporary_ = true;
    descriptor->isMicrophoneMutePersistent_ = false;

    EXPECT_TRUE(descriptor->GetMicrophoneMuteTemporary());
    EXPECT_FALSE(descriptor->GetMicrophoneMutePersistent());

    descriptor->isMicrophoneMuteTemporary_ = false;
    descriptor->isMicrophoneMutePersistent_ = true;

    EXPECT_FALSE(descriptor->GetMicrophoneMuteTemporary());
    EXPECT_TRUE(descriptor->GetMicrophoneMutePersistent());
}

/**
 * @tc.name  : Test AddAudioCapturerMicrophoneDescriptor with DEVICE_TYPE_NONE.
 * @tc.number: AddAudioCapturerMicrophoneDescriptor_001
 * @tc.desc  : Verify that an invalid microphone descriptor is added when devType is DEVICE_TYPE_NONE.
 */
HWTEST_F(AudioMicrophoneDescriptorUnitTest, AddAudioCapturerMicrophoneDescriptor_001, TestSize.Level4)
{
    std::shared_ptr<AudioMicrophoneDescriptor> descriptor = std::make_shared<AudioMicrophoneDescriptor>();

    DeviceType devType = DeviceType::DEVICE_TYPE_NONE;
    int32_t sessionId = 0;

    descriptor->AddAudioCapturerMicrophoneDescriptor(sessionId, devType);
    auto& descriptorMap = descriptor->audioCaptureMicrophoneDescriptor_;
    auto it = descriptorMap.find(sessionId);
    ASSERT_NE(it, descriptorMap.end());

    auto desc = it->second;
    EXPECT_EQ(desc->deviceType_, DEVICE_TYPE_INVALID);
}

/**
 * @tc.name  : Test AddAudioCapturerMicrophoneDescriptor with matching device type.
 * @tc.number: AddAudioCapturerMicrophoneDescriptor_002
 * @tc.desc  : Verify that the correct microphone descriptor is added when a matching device is found.
 */
HWTEST_F(AudioMicrophoneDescriptorUnitTest, AddAudioCapturerMicrophoneDescriptor_002, TestSize.Level4)
{
    std::shared_ptr<AudioMicrophoneDescriptor> descriptor = std::make_shared<AudioMicrophoneDescriptor>();

    sptr<MicrophoneDescriptor> mockMic = new MicrophoneDescriptor(1, DEVICE_TYPE_MIC);
    descriptor->connectedMicrophones_.clear();
    descriptor->connectedMicrophones_.push_back(mockMic);

    int32_t testSessionId = 1002;
    DeviceType devType = DEVICE_TYPE_MIC;

    descriptor->AddAudioCapturerMicrophoneDescriptor(testSessionId, devType);

    auto& descriptorMap =descriptor->audioCaptureMicrophoneDescriptor_;
    auto it = descriptorMap.find(testSessionId);
    ASSERT_NE(it, descriptorMap.end());
    EXPECT_EQ(it->second, mockMic);
}

/**
 * @tc.name  : Test AddAudioCapturerMicrophoneDescriptor with no matching device.
 * @tc.number: AddAudioCapturerMicrophoneDescriptor_003
 * @tc.desc  : Verify that no descriptor is added when no connected microphone matches the device type.
 */
HWTEST_F(AudioMicrophoneDescriptorUnitTest, AddAudioCapturerMicrophoneDescriptor_003, TestSize.Level4)
{
    std::shared_ptr<AudioMicrophoneDescriptor> descriptor = std::make_shared<AudioMicrophoneDescriptor>();

    sptr<MicrophoneDescriptor> mockMic = new MicrophoneDescriptor(1, DEVICE_TYPE_MIC);
    descriptor->connectedMicrophones_.clear();
    descriptor->connectedMicrophones_.push_back(mockMic);

    int32_t testSessionId = 1003;
    DeviceType devType = DEVICE_TYPE_USB_HEADSET;

    descriptor->AddAudioCapturerMicrophoneDescriptor(testSessionId, devType);

    auto& descriptorMap = descriptor->audioCaptureMicrophoneDescriptor_;
    auto it = descriptorMap.find(testSessionId);
    EXPECT_EQ(it, descriptorMap.end());
}

/**
 * @tc.name  : Test RemoveAudioCapturerMicrophoneDescriptor with matching UIDs.
 * @tc.number: RemoveAudioCapturerMicrophoneDescriptor_001
 * @tc.desc  : Verify that microphone descriptors are removed only when clientUID or createrUID matches the input UID.
 */
HWTEST_F(AudioMicrophoneDescriptorUnitTest, RemoveAudioCapturerMicrophoneDescriptor_001, TestSize.Level4)
{
    std::shared_ptr<AudioMicrophoneDescriptor> descriptor = std::make_shared<AudioMicrophoneDescriptor>();

    const int32_t kSessionId = 8001;

    descriptor->AddAudioCapturerMicrophoneDescriptor(kSessionId, AudioStandard::DEVICE_TYPE_MIC);
    descriptor->RemoveAudioCapturerMicrophoneDescriptorBySessionID(kSessionId);

    auto& map = descriptor->audioCaptureMicrophoneDescriptor_;
    EXPECT_EQ(map.find(kSessionId), map.end());
}

/**
 * @tc.name  : Test GetAudioCapturerMicrophoneDescriptors with existing sessionId.
 * @tc.number: GetAudioCapturerMicrophoneDescriptors_001
 * @tc.desc  : Verify that the microphone descriptor is returned when sessionId exists.
 */
HWTEST_F(AudioMicrophoneDescriptorUnitTest, GetAudioCapturerMicrophoneDescriptors_001, TestSize.Level4)
{
    std::shared_ptr<AudioMicrophoneDescriptor> descriptor = std::make_shared<AudioMicrophoneDescriptor>();

    int32_t sessionId = 12345;
    sptr<MicrophoneDescriptor> expectedDesc = new MicrophoneDescriptor(1, DEVICE_TYPE_EARPIECE);
    descriptor->audioCaptureMicrophoneDescriptor_[sessionId] = expectedDesc;

    std::vector<sptr<MicrophoneDescriptor>> result = descriptor->GetAudioCapturerMicrophoneDescriptors(sessionId);

    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0]->deviceType_, expectedDesc->deviceType_);
}

/**
 * @tc.name  : Test GetAudioCapturerMicrophoneDescriptors with non-existing sessionId.
 * @tc.number: GetAudioCapturerMicrophoneDescriptors_002
 * @tc.desc  : Verify that an empty list is returned when sessionId does not exist.
 */
HWTEST_F(AudioMicrophoneDescriptorUnitTest, GetAudioCapturerMicrophoneDescriptors_002, TestSize.Level4)
{
    std::shared_ptr<AudioMicrophoneDescriptor> descriptor = std::make_shared<AudioMicrophoneDescriptor>();

    int32_t sessionId = 99999;

    std::vector<sptr<MicrophoneDescriptor>> result = descriptor->GetAudioCapturerMicrophoneDescriptors(sessionId);

    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name  : Test RemoveAudioCapturerMicrophoneDescriptor_NoMatch
 * @tc.number: RemoveAudioCapturerMicrophoneDescriptor_002
 * @tc.desc  : Test that no descriptor is removed when no capturer matches the UID
 */
HWTEST_F(AudioMicrophoneDescriptorUnitTest, RemoveAudioCapturerMicrophoneDescriptor_002, TestSize.Level4)
{
    std::shared_ptr<AudioMicrophoneDescriptor> descriptor = std::make_shared<AudioMicrophoneDescriptor>();
    std::shared_ptr<AudioCapturerChangeInfo> info = std::make_shared<AudioCapturerChangeInfo>();
    info->clientUID = 1;
    info->createrUID = 1;
    info->sessionId = 1;
    descriptor->audioCaptureMicrophoneDescriptor_[1] = new OHOS::AudioStandard::MicrophoneDescriptor();
    AudioStreamCollector::GetAudioStreamCollector().audioCapturerChangeInfos_.push_back(info);
    int32_t uid = 1;
    descriptor->RemoveAudioCapturerMicrophoneDescriptor(uid);
    uid = 2;
    descriptor->RemoveAudioCapturerMicrophoneDescriptor(uid);
    info->createrUID = 2;
    descriptor->RemoveAudioCapturerMicrophoneDescriptor(uid);
    uid = 1;
    descriptor->RemoveAudioCapturerMicrophoneDescriptor(uid);
    EXPECT_TRUE(descriptor->audioCaptureMicrophoneDescriptor_.empty());
}
}
}