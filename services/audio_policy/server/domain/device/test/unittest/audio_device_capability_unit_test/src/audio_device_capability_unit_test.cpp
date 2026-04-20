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
#include "audio_device_capability_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
void AudioDeviceCapabilityUnitTest::SetUpTestCase(void) {}
void AudioDeviceCapabilityUnitTest::TearDownTestCase(void) {}
void AudioDeviceCapabilityUnitTest::SetUp(void) {}
void AudioDeviceCapabilityUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test FromJsonString.
 * @tc.number: FromJsonString.
 * @tc.desc  : Test FromJsonString interface.
 */
HWTEST_F(AudioDeviceCapabilityUnitTest, FromJsonString, TestSize.Level4)
{
    RemoteDeviceCapability capability;
    std::string json = R"({
        "test": 2
    })";
    EXPECT_EQ(capability.protocol_, -1);
    json = R"({
        "stream_info": "[{\"type\":1, \"id\":\"123\"}]",
        "support_remote_volume": true,
        "init_volume": 50,
        "init_mute_status": true,
        "support_hilink_control": 0,
        "device_name": "Smart Speaker",
        "protocol": 2
    })";
    capability.FromJsonString(json);
    EXPECT_EQ(capability.protocol_, 2);
    EXPECT_EQ(capability.isSupportHiLinkControl_, false);

    json = R"({
        "stream_info": 1,
        "support_remote_volume": "1",
        "init_volume": "1",
        "init_mute_status": "1",
        "support_hilink_control": 1,
        "device_name": 1,
        "protocol": "1"
    })";
    capability.FromJsonString(json);
    EXPECT_EQ(capability.protocol_, 2);
    EXPECT_EQ(capability.isSupportHiLinkControl_, true);
}

/**
 * @tc.name  : FromJsonString_HdPlaybackMode_001
 * @tc.number: FromJsonString_HdPlaybackMode_001
 * @tc.desc  : Test FromJsonString when hd_play_mode is not a number type.
 */
HWTEST_F(AudioDeviceCapabilityUnitTest, FromJsonString_HdPlaybackMode_001, TestSize.Level1)
{
    RemoteDeviceCapability capability;
    std::string json = R"({
        "hd_play_mode": "invalid_string"
    })";
    capability.FromJsonString(json);
    EXPECT_EQ(capability.hdPlaybackMode_, 0);
}

/**
 * @tc.name  : FromJsonString_HdPlaybackMode_002
 * @tc.number: FromJsonString_HdPlaybackMode_002
 * @tc.desc  : Test FromJsonString when hd_play_mode is a valid number.
 */
HWTEST_F(AudioDeviceCapabilityUnitTest, FromJsonString_HdPlaybackMode_002, TestSize.Level1)
{
    RemoteDeviceCapability capability;
    std::string json = R"({
        "hd_play_mode": 2
    })";
    capability.FromJsonString(json);
    EXPECT_EQ(capability.hdPlaybackMode_, 2);
}
}
}
