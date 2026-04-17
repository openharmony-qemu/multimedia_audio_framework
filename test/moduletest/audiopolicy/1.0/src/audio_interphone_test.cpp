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
#include "audio_interphone_test.h"

using namespace std;
using namespace OHOS::AudioStandard;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace V1_0 {
void AudioInterphoneTest::SetUpTestCase(void)
{
    ASSERT_NE(nullptr, AudioSystemManager::GetInstance());
}

void AudioInterphoneTest::TearDownTestCase(void) {}

void AudioInterphoneTest::SetUp(void) {}

void AudioInterphoneTest::TearDown(void) {}

namespace {
const InterphoneParam INTERPHONE_VOLUME_PARAMS[] = {
    {
        .volume = 0.0f,
        .streamUsage = STREAM_USAGE_INTERPHONE
    },
    {
        .volume = 0.5f,
        .streamUsage = STREAM_USAGE_INTERPHONE
    },
    {
        .volume = 1.0f,
        .streamUsage = STREAM_USAGE_INTERPHONE
    }
};

const InterphoneParam INTERPHONE_MUTE_PARAMS[] = {
    {
        .streamUsage = STREAM_USAGE_INTERPHONE,
        .mute = true
    },
    {
        .streamUsage = STREAM_USAGE_INTERPHONE,
        .mute = false
    }
};

const InterphoneParam INTERPHONE_ROUTE_FLAG_PARAMS[] = {
    {
        .outputRouteFlag = AUDIO_OUTPUT_FLAG_INTERPHONE,
        .inputRouteFlag = AUDIO_INPUT_FLAG_INTERPHONE
    }
};

const InterphoneParam INTERPHONE_STREAM_TYPE_PARAMS[] = {
    {
        .streamUsage = STREAM_USAGE_INTERPHONE,
        .expectedStreamType = STREAM_VOICE_COMMUNICATION
    }
};

const InterphoneParam INTERPHONE_SOURCE_TYPE_PARAMS[] = {
    {
        .sourceType = SOURCE_TYPE_INTERPHONE,
        .expectedPipeType = PIPE_TYPE_IN_INTERPHONE
    }
};
} // namespace

/*
 * Test Interphone Volume
 *
 */
class AudioInterphoneSetVolumeTest : public AudioInterphoneTest {};

HWTEST_P(AudioInterphoneSetVolumeTest, SetInterphoneVolume, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();
    InterphoneParam params = GetParam();

    AudioVolumeType volumeType = VOLUME_TYPE_CALL;
    float volume = params.volume;

    EXPECT_EQ(AUDIO_OK, AudioSystemManager::GetInstance()->SetVolume(volumeType, volume));
    MockNative::Resume();
}

INSTANTIATE_TEST_SUITE_P(
    SetInterphoneVolume,
    AudioInterphoneSetVolumeTest,
    ValuesIn(INTERPHONE_VOLUME_PARAMS));

/*
 * Test Interphone Get Volume
 *
 */
class AudioInterphoneGetVolumeTest : public AudioInterphoneTest {};

HWTEST_P(AudioInterphoneGetVolumeTest, GetInterphoneVolume, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();
    InterphoneParam params = GetParam();

    AudioVolumeType volumeType = VOLUME_TYPE_CALL;
    float volume = params.volume;

    EXPECT_EQ(AUDIO_OK, AudioSystemManager::GetInstance()->SetVolume(volumeType, volume));
    EXPECT_EQ(volume, AudioSystemManager::GetInstance()->GetVolume(volumeType));
    MockNative::Resume();
}

INSTANTIATE_TEST_SUITE_P(
    GetInterphoneVolume,
    AudioInterphoneGetVolumeTest,
    ValuesIn(INTERPHONE_VOLUME_PARAMS));

/*
 * Test Interphone Mute
 *
 */
class AudioInterphoneSetMuteTest : public AudioInterphoneTest {};

HWTEST_P(AudioInterphoneSetMuteTest, SetInterphoneMute, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();
    InterphoneParam params = GetParam();

    AudioVolumeType volumeType = VOLUME_TYPE_CALL;
    bool mute = params.mute;

    EXPECT_EQ(AUDIO_OK, AudioSystemManager::GetInstance()->SetMute(volumeType, mute));
}

INSTANTIATE_TEST_SUITE_P(
    SetInterphoneMute,
    AudioInterphoneSetMuteTest,
    ValuesIn(INTERPHONE_MUTE_PARAMS));

/*
 * Test Interphone Route Flag
 *
 */
class AudioInterphoneRouteFlagTest : public AudioInterphoneTest {};

HWTEST_P(AudioInterphoneRouteFlagTest, TestInterphoneRouteFlag, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();
    InterphoneParam params = GetParam();

    EXPECT_TRUE(params.outputRouteFlag & AUDIO_OUTPUT_FLAG_INTERPHONE);
    EXPECT_TRUE(params.inputRouteFlag & AUDIO_INPUT_FLAG_INTERPHONE);
    MockNative::Resume();
}

INSTANTIATE_TEST_SUITE_P(
    TestInterphoneRouteFlag,
    AudioInterphoneRouteFlagTest,
    ValuesIn(INTERPHONE_ROUTE_FLAG_PARAMS));

/*
 * Test Interphone Stream Type Mapping
 *
 */
class AudioInterphoneStreamTypeTest : public AudioInterphoneTest {};

HWTEST_P(AudioInterphoneStreamTypeTest, TestInterphoneStreamType, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();
    InterphoneParam params = GetParam();

    EXPECT_EQ(STREAM_USAGE_INTERPHONE, params.streamUsage);
    EXPECT_EQ(STREAM_VOICE_COMMUNICATION, params.expectedStreamType);
    MockNative::Resume();
}

INSTANTIATE_TEST_SUITE_P(
    TestInterphoneStreamType,
    AudioInterphoneStreamTypeTest,
    ValuesIn(INTERPHONE_STREAM_TYPE_PARAMS));

/*
 * Test Interphone Source Type
 *
 */
class AudioInterphoneSourceTypeTest : public AudioInterphoneTest {};

HWTEST_P(AudioInterphoneSourceTypeTest, TestInterphoneSourceType, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();
    InterphoneParam params = GetParam();

    EXPECT_EQ(SOURCE_TYPE_INTERPHONE, params.sourceType);
    EXPECT_EQ(PIPE_TYPE_IN_INTERPHONE, params.expectedPipeType);
    MockNative::Resume();
}

INSTANTIATE_TEST_SUITE_P(
    TestInterphoneSourceType,
    AudioInterphoneSourceTypeTest,
    ValuesIn(INTERPHONE_SOURCE_TYPE_PARAMS));

/*
 * Test Interphone Stream Usage Value
 *
 */
HWTEST_F(AudioInterphoneTest, TestInterphoneStreamUsageValue, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    EXPECT_EQ(24, STREAM_USAGE_INTERPHONE);
    MockNative::Resume();
}

/*
 * Test Interphone Source Type Value
 *
 */
HWTEST_F(AudioInterphoneTest, TestInterphoneSourceTypeValue, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    EXPECT_EQ(20, SOURCE_TYPE_INTERPHONE);
    MockNative::Resume();
}

/*
 * Test Interphone Output Flag Value
 *
 */
HWTEST_F(AudioInterphoneTest, TestInterphoneOutputFlagValue, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    EXPECT_EQ(0x1000000, AUDIO_OUTPUT_FLAG_INTERPHONE);
    MockNative::Resume();
}

/*
 * Test Interphone Input Flag Value
 *
 */
HWTEST_F(AudioInterphoneTest, TestInterphoneInputFlagValue, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    EXPECT_EQ(0x800000, AUDIO_INPUT_FLAG_INTERPHONE);
    MockNative::Resume();
}

/*
 * Test Interphone Pipe Type Value
 *
 */
HWTEST_F(AudioInterphoneTest, TestInterphonePipeTypeValue, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("multimodalinput");
    MockNative::Mock();

    EXPECT_EQ(22, PIPE_TYPE_OUT_INTERPHONE);
    EXPECT_EQ(23, PIPE_TYPE_IN_INTERPHONE);
    MockNative::Resume();
}

} // namespace V1_0
} // namespace AudioStandard
} // namespace OHOS