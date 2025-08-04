/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include <cstdint>
#include <thread>
#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include "audio_utils.h"
#include "parameter.h"
#include "audio_channel_blend.h"
#include "volume_ramp.h"
#include "audio_speed.h"
#include "audio_errors.h"
#include "audio_scope_exit.h"
#include "audio_safe_block_queue.h"
#include "audio_utils_c.h"

using namespace testing::ext;
using namespace testing;
using namespace std;
namespace OHOS {
namespace AudioStandard {

/**
* @tc.name  : Test SetVolumeRampConfig  API
* @tc.type  : FUNC
* @tc.number: SetVolumeRampConfig_002
* @tc.desc  : Test SetVolumeRampConfig API,
*             when rampDirection_ is RAMP_UP
*/
HWTEST(AudioUtilsUnitTest, SetVolumeRampConfig_002, TestSize.Level1)
{
    shared_ptr<VolumeRamp> volumeRamp = std::make_shared<VolumeRamp>();
    volumeRamp->SetVolumeRampConfig(10.1f, 9.9f, 4);
    EXPECT_EQ(volumeRamp->rampDirection_, RAMP_UP);
}

/**
* @tc.name  : Test GetRampVolume  API
* @tc.type  : FUNC
* @tc.number: GetRampVolume_003
* @tc.desc  : Test GetRampVolume API,
*             when ret is 0.0f
*/
HWTEST(AudioUtilsUnitTest, GetRampVolume_003, TestSize.Level1)
{
    shared_ptr<VolumeRamp> volumeRamp = std::make_shared<VolumeRamp>();
    volumeRamp->isVolumeRampActive_ = true;
    volumeRamp->initTime_ = 1;
    float ret = volumeRamp->GetRampVolume();
    EXPECT_EQ(ret, 0.0f);
}

/**
 * @tc.name  : Test audio_channel_blend API
 * @tc.type  : FUNC
 * @tc.number: audio_channel_blend_020
 * @tc.desc  : Test AudioBlend Process API,Return buffer
 *             when blendMode is MODE_BLEND_LR,channel 6
 */
HWTEST(AudioUtilsUnitTest, audio_channel_blend_020, TestSize.Level1)
{
    uint8_t b[8] = {2, 4, 6, 8, 10, 12, 14, 16};
    uint8_t format = SAMPLE_F32LE;
    uint8_t channels = CHANNEL_3;
    ChannelBlendMode blendMode = MODE_ALL_RIGHT;
    shared_ptr<AudioBlend> audioBlend = std::make_shared<AudioBlend>(blendMode, format, channels);
    audioBlend->Process(b, 8);
    EXPECT_EQ(b[0], 2);
}

/**
 * @tc.name  : Test audio_channel_blend API
 * @tc.type  : FUNC
 * @tc.number: audio_channel_blend_021
 * @tc.desc  : Test AudioBlend Process API,Return buffer
 *             when blendMode is MODE_BLEND_LR,channel 6
 */
HWTEST(AudioUtilsUnitTest, audio_channel_blend_021, TestSize.Level1)
{
    uint8_t b[8] = {2, 4, 6, 8, 10, 12, 14, 16};
    uint8_t format = INVALID_WIDTH;
    uint8_t channels = CHANNEL_3;
    ChannelBlendMode blendMode = MODE_ALL_RIGHT;
    shared_ptr<AudioBlend> audioBlend = std::make_shared<AudioBlend>(blendMode, format, channels);
    audioBlend->Process(b, 8);
    EXPECT_EQ(b[0], 2);
}

/**
 * @tc.name  : Test GetAudioFormatSize API
 * @tc.type  : FUNC
 * @tc.number: GetAudioFormatSize_001
 * @tc.desc  : Test GetAudioFormatSize
 */
HWTEST(AudioUtilsUnitTest, GetAudioFormatSize_001, TestSize.Level1)
{
    uint8_t b[8] = {2, 4, 6, 8, 10, 12, 14, 16};
    uint8_t format = INVALID_WIDTH;
    uint8_t channels = CHANNEL_3;
    ChannelBlendMode blendMode = MODE_ALL_RIGHT;
    shared_ptr<AudioBlend> audioBlend = std::make_shared<AudioBlend>(blendMode, format, channels);
    audioBlend->GetAudioFormatSize();
    EXPECT_EQ(b[0], 2);
}

/**
 * @tc.name  : Test CountVolume API
 * @tc.type  : FUNC
 * @tc.number: CountVolume_001
 * @tc.desc  : Test CountVolume
 */
HWTEST(AudioUtilsUnitTest, CountVolume_001, TestSize.Level1)
{
    std::string value = "Test";
    Trace::CountVolume(value, 0);
    Trace::CountVolume(value, 2);
    EXPECT_FALSE(static_cast<size_t>(0));
}

/**
 * @tc.name  : Test ConvertFromFloatTo24Bit API
 * @tc.type  : FUNC
 * @tc.number: ConvertFromFloatTo24Bit_001
 * @tc.desc  : Test ConvertFromFloatTo24Bit
 */
HWTEST(AudioUtilsUnitTest, ConvertFromFloatTo24Bit_001, TestSize.Level1)
{
    float a = 2.0f;
    uint8_t b = 1;
    ConvertFromFloatTo24Bit(1, &a, &b);
    a = -2.5f;
    ConvertFromFloatTo24Bit(1, &a, &b);
    EXPECT_FALSE(static_cast<size_t>(0));
}

/**
 * @tc.name  : Test IsInnerCapSinkName API
 * @tc.type  : FUNC
 * @tc.number: IsInnerCapSinkName_001
 * @tc.desc  : Test IsInnerCapSinkName
 */
HWTEST(AudioUtilsUnitTest, IsInnerCapSinkName_001, TestSize.Level1)
{
    char pattern[MAX_MEM_MALLOC_SIZE + 1] = {0};
    EXPECT_EQ(IsInnerCapSinkName(pattern), false);
}

/**
 * @tc.name  : Test IsInnerCapSinkName API
 * @tc.type  : FUNC
 * @tc.number: IsInnerCapSinkName_002
 * @tc.desc  : Test IsInnerCapSinkName
 */
HWTEST(AudioUtilsUnitTest, IsInnerCapSinkName_002, TestSize.Level1)
{
    char pattern[] = "invalid_pattern";
    EXPECT_FALSE(IsInnerCapSinkName(pattern));
}

/**
* @tc.name  : Test GetFormatByteSize API
* @tc.type  : FUNC
* @tc.number: GetFormatByteSize_005
* @tc.desc  : Test GetFormatByteSize
*/
HWTEST(AudioUtilsUnitTest, GetFormatByteSize_005, TestSize.Level0)
{
    int32_t format = SAMPLE_F32LE;
    int32_t formatByteSize = GetFormatByteSize(format);
    EXPECT_EQ(formatByteSize, 4);
}

/**
* @tc.name  : Test CloseFd API
* @tc.type  : FUNC
* @tc.number: CloseFd_001
* @tc.desc  : Test CloseFd
*/
HWTEST(AudioUtilsUnitTest, CloseFd_001, TestSize.Level0)
{
    CloseFd(STDIN_FILENO);
    EXPECT_FALSE(static_cast<size_t>(0));
}

/**
* @tc.name  : Test MockPcmData API
* @tc.type  : FUNC
* @tc.number: MockPcmData_003
* @tc.desc  : Test MockPcmData API if format is SAMPLE_S16LE
* when mockedTime_ >= MOCK_INTERVAL
*/
HWTEST(AudioUtilsUnitTest, MockPcmData_003, TestSize.Level1)
{
    std::shared_ptr<AudioLatencyMeasurement> audioLatencyMeasurement =
        std::make_shared<AudioLatencyMeasurement>(44100, 2, 16, "com.example.null", 1);
    uint8_t buffer[1024] = {};
    size_t bufferLen = sizeof(buffer);
    size_t mockInterval = 2000;

    audioLatencyMeasurement->mockedTime_ = mockInterval + 1;
    audioLatencyMeasurement->format_ = SAMPLE_S16LE;
    bool ret = audioLatencyMeasurement->MockPcmData(buffer, bufferLen);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test MockPcmData API
* @tc.type  : FUNC
* @tc.number: MockPcmData_004
* @tc.desc  : Test MockPcmData API if format is SAMPLE_S32LE
* when mockedTime_ >= MOCK_INTERVAL
*/
HWTEST(AudioUtilsUnitTest, MockPcmData_004, TestSize.Level1)
{
    std::shared_ptr<AudioLatencyMeasurement> audioLatencyMeasurement =
        std::make_shared<AudioLatencyMeasurement>(44100, 2, 16, "com.example.null", 1);
    uint8_t buffer[1024] = {};
    size_t bufferLen = sizeof(buffer);
    size_t mockInterval = 2000;

    audioLatencyMeasurement->mockedTime_ = mockInterval + 1;
    audioLatencyMeasurement->format_ = SAMPLE_S32LE;
    bool ret = audioLatencyMeasurement->MockPcmData(buffer, bufferLen);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test CallEndAndClear API
 * @tc.type  : FUNC
 * @tc.number: CallEndAndClear_001
 * @tc.desc  : Test CallEndAndClear when *cTrace is nullptr
 */
HWTEST(AudioUtilsUnitTest, CallEndAndClear_001, TestSize.Level0)
{
    CTrace *cTrace = nullptr;
    CallEndAndClear(&cTrace);
    EXPECT_TRUE(cTrace == nullptr);
}

/**
 * @tc.name  : Test CallEndAndClear API
 * @tc.type  : FUNC
 * @tc.number: CallEndAndClear_002
 * @tc.desc  : Test CallEndAndClear when **cTrace is nullptr
 */
HWTEST(AudioUtilsUnitTest, CallEndAndClear_002, TestSize.Level0)
{
    CTrace **cTrace = nullptr;
    CallEndAndClear(cTrace);
    EXPECT_TRUE(cTrace == nullptr);
}

/**
 * @tc.name  : Test AudioLatencyMeasurement API
 * @tc.type  : FUNC
 * @tc.number: AudioLatencyMeasurement_001
 * @tc.desc  : Test AudioLatencyMeasurement when **cTrace is nullptr
 */
HWTEST(AudioUtilsUnitTest, AudioLatencyMeasurement_001, TestSize.Level1)
{
    AudioLatencyMeasurement audioLatencyMeasurement(44100, 2, 16, "com.example.null", 1);
    EXPECT_EQ(audioLatencyMeasurement.sessionId_, 1);
}
} // namespace AudioStandard
} // namespace OHOS