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

#include "audio_errors.h"
#include "audio_policy_client_stub.h"
#include "audio_policy_client_stub_impl.h"
#include "audio_policy_client_stub_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioPolicyClientStubUnitTest::SetUpTestCase(void) {}
void AudioPolicyClientStubUnitTest::TearDownTestCase(void) {}
void AudioPolicyClientStubUnitTest::SetUp(void) {}
void AudioPolicyClientStubUnitTest::TearDown(void) {}

/**
* @tc.name  : Test AudioPolicyClientStub.
* @tc.number: AudioPolicyClientStubUnitTest_001.
* @tc.desc  : Test IsSameTypeForAudioSession.
*/
HWTEST_F(AudioPolicyClientStubUnitTest, AudioPolicyClientStubUnitTest_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    uint32_t updateCode = static_cast<uint32_t>(AudioPolicyClientCode::ON_HEAD_TRACKING_DEVICE_CHANGE);
    std::shared_ptr<AudioPolicyClientStub> audioPolicyClientStub = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClientStub != nullptr);
    audioPolicyClientStub->OnFirMaxRemoteRequest(updateCode, data, reply);
}

/**
* @tc.name  : Test AudioPolicyClientStub.
* @tc.number: AudioPolicyClientStubUnitTest_002.
* @tc.desc  : Test IsSameTypeForAudioSession.
*/
HWTEST_F(AudioPolicyClientStubUnitTest, AudioPolicyClientStubUnitTest_002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    uint32_t updateCode = static_cast<uint32_t>(AudioPolicyClientCode::ON_SPATIALIZATION_ENABLED_CHANGE_FOR_ANY_DEVICE);
    std::shared_ptr<AudioPolicyClientStub> audioPolicyClientStub = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClientStub != nullptr);
    audioPolicyClientStub->OnFirMaxRemoteRequest(updateCode, data, reply);
}

/**
* @tc.name  : Test AudioPolicyClientStub.
* @tc.number: AudioPolicyClientStubUnitTest_003.
* @tc.desc  : Test IsSameTypeForAudioSession.
*/
HWTEST_F(AudioPolicyClientStubUnitTest, AudioPolicyClientStubUnitTest_003, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    uint32_t updateCode = static_cast<uint32_t>(AudioPolicyClientCode::ON_HEAD_TRACKING_ENABLED_CHANGE_FOR_ANY_DEVICE);
    std::shared_ptr<AudioPolicyClientStub> audioPolicyClientStub = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClientStub != nullptr);
    audioPolicyClientStub->OnFirMaxRemoteRequest(updateCode, data, reply);
}

/**
* @tc.name  : Test AudioPolicyClientStub.
* @tc.number: AudioPolicyClientStubUnitTest_004.
* @tc.desc  : Test IsSameTypeForAudioSession.
*/
HWTEST_F(AudioPolicyClientStubUnitTest, AudioPolicyClientStubUnitTest_004, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    uint32_t updateCode = static_cast<uint32_t>(
        AudioPolicyClientCode::ON_SPATIALIZATION_ENABLED_CHANGE_FOR_CURRENT_DEVICE);
    std::shared_ptr<AudioPolicyClientStub> audioPolicyClientStub = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClientStub != nullptr);
    audioPolicyClientStub->OnFirMaxRemoteRequest(updateCode, data, reply);
}

/**
* @tc.name  : Test AudioPolicyClientStub.
* @tc.number: AudioPolicyClientStubUnitTest_005.
* @tc.desc  : Test OnFirMaxRemoteRequest.
*/
HWTEST_F(AudioPolicyClientStubUnitTest, AudioPolicyClientStubUnitTest_005, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    uint32_t updateCode = static_cast<uint32_t>(AudioPolicyClientCode::ON_NN_STATE_CHANGE);
    std::shared_ptr<AudioPolicyClientStub> audioPolicyClientStub = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClientStub != nullptr);
    audioPolicyClientStub->OnFirMaxRemoteRequest(updateCode, data, reply);
}

/**
* @tc.name  : Test AudioPolicyClientStub.
* @tc.number: AudioPolicyClientStubUnitTest_006.
* @tc.desc  : Test OnFirMaxRemoteRequest.
*/
HWTEST_F(AudioPolicyClientStubUnitTest, AudioPolicyClientStubUnitTest_006, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    uint32_t updateCode = static_cast<uint32_t>(AudioPolicyClientCode::ON_AUDIO_SESSION_DEACTIVE);
    std::shared_ptr<AudioPolicyClientStub> audioPolicyClientStub = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClientStub != nullptr);
    audioPolicyClientStub->OnFirMaxRemoteRequest(updateCode, data, reply);
}

/**
* @tc.name  : Test AudioPolicyClientStub.
* @tc.number: AudioPolicyClientStubUnitTest_007.
* @tc.desc  : Test OnFirMaxRemoteRequest.
*/
HWTEST_F(AudioPolicyClientStubUnitTest, AudioPolicyClientStubUnitTest_007, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    uint32_t updateCode = static_cast<uint32_t>(AudioPolicyClientCode::ON_AUDIO_SCENE_CHANGED);
    std::shared_ptr<AudioPolicyClientStub> audioPolicyClientStub = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClientStub != nullptr);
    audioPolicyClientStub->OnFirMaxRemoteRequest(updateCode, data, reply);
}

/**
* @tc.name  : Test AudioPolicyClientStub.
* @tc.number: AudioPolicyClientStubUnitTest_008.
* @tc.desc  : Test OnFirMaxRemoteRequest.
*/
HWTEST_F(AudioPolicyClientStubUnitTest, AudioPolicyClientStubUnitTest_008, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    uint32_t updateCode = static_cast<uint32_t>(AudioPolicyClientCode::ON_VOLUME_KEY_EVENT);
    std::shared_ptr<AudioPolicyClientStub> audioPolicyClientStub = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClientStub != nullptr);
    audioPolicyClientStub->OnFirMaxRemoteRequest(updateCode, data, reply);
}

/**
* @tc.name  : Test AudioPolicyClientStub.
* @tc.number: AudioPolicyClientStubUnitTest_009.
* @tc.desc  : Test OnMaxRemoteRequest.
*/
HWTEST_F(AudioPolicyClientStubUnitTest, AudioPolicyClientStubUnitTest_009, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    uint32_t updateCode = static_cast<uint32_t>(AudioPolicyClientCode::ON_RINGERMODE_UPDATE);
    std::shared_ptr<AudioPolicyClientStub> audioPolicyClientStub = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClientStub != nullptr);
    audioPolicyClientStub->OnMaxRemoteRequest(updateCode, data, reply);
}

/**
* @tc.name  : Test AudioPolicyClientStub.
* @tc.number: AudioPolicyClientStubUnitTest_010.
* @tc.desc  : Test OnMaxRemoteRequest.
*/
HWTEST_F(AudioPolicyClientStubUnitTest, AudioPolicyClientStubUnitTest_010, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    uint32_t updateCode = static_cast<uint32_t>(AudioPolicyClientCode::ON_MIC_STATE_UPDATED);
    std::shared_ptr<AudioPolicyClientStub> audioPolicyClientStub = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClientStub != nullptr);
    audioPolicyClientStub->OnMaxRemoteRequest(updateCode, data, reply);
}

/**
* @tc.name  : Test AudioPolicyClientStub.
* @tc.number: AudioPolicyClientStubUnitTest_011.
* @tc.desc  : Test OnMaxRemoteRequest.
*/
HWTEST_F(AudioPolicyClientStubUnitTest, AudioPolicyClientStubUnitTest_011, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    uint32_t updateCode = static_cast<uint32_t>(AudioPolicyClientCode::ON_RECREATE_RENDERER_STREAM_EVENT);
    std::shared_ptr<AudioPolicyClientStub> audioPolicyClientStub = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClientStub != nullptr);
    audioPolicyClientStub->OnMaxRemoteRequest(updateCode, data, reply);
}

/**
* @tc.name  : Test AudioPolicyClientStub.
* @tc.number: AudioPolicyClientStubUnitTest_012.
* @tc.desc  : Test OnMaxRemoteRequest.
*/
HWTEST_F(AudioPolicyClientStubUnitTest, AudioPolicyClientStubUnitTest_012, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    uint32_t updateCode = static_cast<uint32_t>(AudioPolicyClientCode::ON_DISTRIBUTED_OUTPUT_CHANGE);
    std::shared_ptr<AudioPolicyClientStub> audioPolicyClientStub = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClientStub != nullptr);
    audioPolicyClientStub->OnMaxRemoteRequest(updateCode, data, reply);
}

/**
* @tc.name  : Test AudioPolicyClientStub.
* @tc.number: AudioPolicyClientStubUnitTest_013.
* @tc.desc  : Test OnMaxRemoteRequest.
*/
HWTEST_F(AudioPolicyClientStubUnitTest, AudioPolicyClientStubUnitTest_013, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    uint32_t updateCode = static_cast<uint32_t>(AudioPolicyClientCode::ON_RECREATE_CAPTURER_STREAM_EVENT);
    std::shared_ptr<AudioPolicyClientStub> audioPolicyClientStub = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClientStub != nullptr);
    audioPolicyClientStub->OnMaxRemoteRequest(updateCode, data, reply);
}

/**
* @tc.name  : Test AudioPolicyClientStub.
* @tc.number: AudioPolicyClientStubUnitTest_014.
* @tc.desc  : Test OnMaxRemoteRequest.
*/
HWTEST_F(AudioPolicyClientStubUnitTest, AudioPolicyClientStubUnitTest_014, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    uint32_t updateCode = static_cast<uint32_t>(AudioPolicyClientCode::ON_VOLUME_KEY_EVENT);
    std::shared_ptr<AudioPolicyClientStub> audioPolicyClientStub = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClientStub != nullptr);
    audioPolicyClientStub->OnMaxRemoteRequest(updateCode, data, reply);
}

/**
* @tc.name  : Test AudioPolicyClientStub.
* @tc.number: AudioPolicyClientStubUnitTest_015.
* @tc.desc  : Test OnRemoteRequest.
*/
HWTEST_F(AudioPolicyClientStubUnitTest, AudioPolicyClientStubUnitTest_015, TestSize.Level1)
{
    int32_t max_updateCode = 1000;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = UPDATE_CALLBACK_CLIENT;

    data.WriteInterfaceToken(AudioPolicyClientStub::GetDescriptor());
    data.WriteInt32(max_updateCode);

    std::shared_ptr<AudioPolicyClientStub> audioPolicyClientStub = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClientStub != nullptr);

    auto ret = audioPolicyClientStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, -1);
}

/**
* @tc.name  : Test AudioPolicyClientStub.
* @tc.number: AudioPolicyClientStubUnitTest_016.
* @tc.desc  : Test OnRemoteRequest.
*/
HWTEST_F(AudioPolicyClientStubUnitTest, AudioPolicyClientStubUnitTest_016, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t updateCode = UPDATE_CALLBACK_CLIENT + 1;

    data.WriteInterfaceToken(AudioPolicyClientStub::GetDescriptor());

    std::shared_ptr<AudioPolicyClientStub> audioPolicyClientStub = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClientStub != nullptr);

    auto ret = audioPolicyClientStub->OnRemoteRequest(updateCode, data, reply, option);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStub.
* @tc.number: AudioPolicyClientStubUnitTest_017.
* @tc.desc  : Test OnRemoteRequest.
*/
HWTEST_F(AudioPolicyClientStubUnitTest, AudioPolicyClientStubUnitTest_017, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t updateCode = UPDATE_CALLBACK_CLIENT;

    data.WriteInterfaceToken(AudioPolicyClientStub::GetDescriptor());
    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_DEVICE_CHANGE));

    std::shared_ptr<AudioPolicyClientStub> audioPolicyClientStub = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClientStub != nullptr);

    auto ret = audioPolicyClientStub->OnRemoteRequest(updateCode, data, reply, option);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStub.
* @tc.number: AudioPolicyClientStubUnitTest_018.
* @tc.desc  : Test OnRemoteRequest.
*/
HWTEST_F(AudioPolicyClientStubUnitTest, AudioPolicyClientStubUnitTest_018, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t updateCode = UPDATE_CALLBACK_CLIENT;

    data.WriteInterfaceToken(AudioPolicyClientStub::GetDescriptor());
    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_MICRO_PHONE_BLOCKED));

    std::shared_ptr<AudioPolicyClientStub> audioPolicyClientStub = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClientStub != nullptr);

    auto ret = audioPolicyClientStub->OnRemoteRequest(updateCode, data, reply, option);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStub.
* @tc.number: AudioPolicyClientStubUnitTest_019.
* @tc.desc  : Test OnRemoteRequest.
*/
HWTEST_F(AudioPolicyClientStubUnitTest, AudioPolicyClientStubUnitTest_019, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t updateCode = UPDATE_CALLBACK_CLIENT;

    data.WriteInterfaceToken(AudioPolicyClientStub::GetDescriptor());
    data.WriteInt32(static_cast<int32_t>(AudioPolicyClientCode::ON_APP_VOLUME_CHANGE));

    std::shared_ptr<AudioPolicyClientStub> audioPolicyClientStub = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClientStub != nullptr);

    auto ret = audioPolicyClientStub->OnRemoteRequest(updateCode, data, reply, option);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyClientStub.
* @tc.number: AudioPolicyClientStubUnitTest_020.
* @tc.desc  : Test HandleHeadTrackingDeviceChange.
*/
HWTEST_F(AudioPolicyClientStubUnitTest, AudioPolicyClientStubUnitTest_020, TestSize.Level1)
{
    int size = 10;
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(size);
    std::shared_ptr<AudioPolicyClientStub> audioPolicyClientStub = std::make_shared<AudioPolicyClientStubImpl>();
    ASSERT_TRUE(audioPolicyClientStub != nullptr);
    audioPolicyClientStub->HandleHeadTrackingDeviceChange(data, reply);
}
} // namespace AudioStandard
} // namespace OHOS