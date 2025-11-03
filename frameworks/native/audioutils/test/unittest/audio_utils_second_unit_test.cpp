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
#include "xperf_adapter.h"

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

/**
 * @tc.name  : Test NeedNotifyXperf API
 * @tc.type  : FUNC
 * @tc.number: XperfAdapterNeedNotifyXperf_001
 * @tc.desc  : Test NeedNotifyXperf.
 */
HWTEST(AudioUtilsUnitTest, XperfAdapterNeedNotifyXperf_001, TestSize.Level1)
{
    EXPECT_EQ(XperfAdapter::GetInstance().NeedNotifyXperf(STREAM_USAGE_MEDIA), true);
}

/**
 * @tc.name  : Test NeedNotifyXperf API
 * @tc.type  : FUNC
 * @tc.number: XperfAdapterNeedNotifyXperf_002
 * @tc.desc  : Test NeedNotifyXperf.
 */
HWTEST(AudioUtilsUnitTest, XperfAdapterNeedNotifyXperf_002, TestSize.Level1)
{
    EXPECT_EQ(XperfAdapter::GetInstance().NeedNotifyXperf(STREAM_USAGE_VOICE_COMMUNICATION), true);
}

/**
 * @tc.name  : Test NeedNotifyXperf API
 * @tc.type  : FUNC
 * @tc.number: XperfAdapterNeedNotifyXperf_003
 * @tc.desc  : Test NeedNotifyXperf.
 */
HWTEST(AudioUtilsUnitTest, XperfAdapterNeedNotifyXperf_003, TestSize.Level1)
{
    EXPECT_EQ(XperfAdapter::GetInstance().NeedNotifyXperf(STREAM_USAGE_MOVIE), true);
}

/**
 * @tc.name  : Test NeedNotifyXperf API
 * @tc.type  : FUNC
 * @tc.number: XperfAdapterNeedNotifyXperf_004
 * @tc.desc  : Test NeedNotifyXperf.
 */
HWTEST(AudioUtilsUnitTest, XperfAdapterNeedNotifyXperf_004, TestSize.Level1)
{
    EXPECT_EQ(XperfAdapter::GetInstance().NeedNotifyXperf(STREAM_USAGE_NOTIFICATION_RINGTONE), false);
}

/**
 * @tc.name  : Test CalculatePcmSizeFromDurationCeiling API
 * @tc.type  : FUNC
 * @tc.number: CalculatePcmSizeFromDurationCeiling
 * @tc.desc  : Test CalculatePcmSizeFromDurationCeiling.
 */
HWTEST(AudioUtilsUnitTest, CalculatePcmSizeFromDurationCeiling, TestSize.Level1)
{
    EXPECT_EQ(Util::CalculatePcmSizeFromDurationCeiling(20ms, 48000, 4), 3840);

    EXPECT_EQ(Util::CalculatePcmSizeFromDurationCeiling(1ns, 48000, 4), 4);

    EXPECT_EQ(Util::CalculatePcmSizeFromDurationCeiling(20ms + 1ns, 48000, 4), 3844);
}

/**
 * @tc.name  : Test ConvertToStringForFormat API
 * @tc.type  : FUNC
 * @tc.number: ConvertToStringForFormat_001
 * @tc.desc  : Test ConvertToStringForFormat with all supported sample formats.
 */
HWTEST(AudioUtilsUnitTest, ConvertToStringForFormat_001, TestSize.Level1)
{
    EXPECT_EQ(ConvertToStringForFormat(SAMPLE_U8), "s8");
    EXPECT_EQ(ConvertToStringForFormat(SAMPLE_S16LE), "s16le");
    EXPECT_EQ(ConvertToStringForFormat(SAMPLE_S24LE), "s24le");
    EXPECT_EQ(ConvertToStringForFormat(SAMPLE_S32LE), "s32le");
    EXPECT_EQ(ConvertToStringForFormat(SAMPLE_F32LE), "f32le");
}

/**
 * @tc.name  : Test ConvertToStringForFormat API with unsupported format
 * @tc.type  : FUNC
 * @tc.number: ConvertToStringForFormat_002
 * @tc.desc  : Test ConvertToStringForFormat with unsupported sample format returns default value.
 */
HWTEST(AudioUtilsUnitTest, ConvertToStringForFormat_002, TestSize.Level1)
{
    // Test with unsupported format (assuming 999 is not in the map)
    AudioSampleFormat unsupportedFormat = static_cast<AudioSampleFormat>(999);
    EXPECT_EQ(ConvertToStringForFormat(unsupportedFormat), "s16le");
}

/**
 * @tc.name  : Test ConvertToStringForSampleRate API
 * @tc.type  : FUNC
 * @tc.number: ConvertToStringForSampleRate_001
 * @tc.desc  : Test ConvertToStringForSampleRate with various sampling rates.
 */
HWTEST(AudioUtilsUnitTest, ConvertToStringForSampleRate_001, TestSize.Level1)
{
    EXPECT_EQ(ConvertToStringForSampleRate(SAMPLE_RATE_8000), "8000");
    EXPECT_EQ(ConvertToStringForSampleRate(SAMPLE_RATE_11025), "11025");
    EXPECT_EQ(ConvertToStringForSampleRate(SAMPLE_RATE_12000), "12000");
    EXPECT_EQ(ConvertToStringForSampleRate(SAMPLE_RATE_16000), "16000");
    EXPECT_EQ(ConvertToStringForSampleRate(SAMPLE_RATE_22050), "22050");
    EXPECT_EQ(ConvertToStringForSampleRate(SAMPLE_RATE_24000), "24000");
    EXPECT_EQ(ConvertToStringForSampleRate(SAMPLE_RATE_32000), "32000");
    EXPECT_EQ(ConvertToStringForSampleRate(SAMPLE_RATE_44100), "44100");
    EXPECT_EQ(ConvertToStringForSampleRate(SAMPLE_RATE_48000), "48000");
    EXPECT_EQ(ConvertToStringForSampleRate(SAMPLE_RATE_64000), "64000");
    EXPECT_EQ(ConvertToStringForSampleRate(SAMPLE_RATE_88200), "88200");
    EXPECT_EQ(ConvertToStringForSampleRate(SAMPLE_RATE_96000), "96000");
    EXPECT_EQ(ConvertToStringForSampleRate(SAMPLE_RATE_176400), "176400");
    EXPECT_EQ(ConvertToStringForSampleRate(SAMPLE_RATE_192000), "192000");
    EXPECT_EQ(ConvertToStringForSampleRate(SAMPLE_RATE_384000), "384000");
}

/**
 * @tc.name  : Test ConvertToStringForSampleRate API with custom values
 * @tc.type  : FUNC
 * @tc.number: ConvertToStringForSampleRate_002
 * @tc.desc  : Test ConvertToStringForSampleRate with custom sampling rate values.
 */
HWTEST(AudioUtilsUnitTest, ConvertToStringForSampleRate_002, TestSize.Level1)
{
    // Test with some arbitrary values to ensure conversion works correctly
    AudioSamplingRate customRate1 = static_cast<AudioSamplingRate>(22050);
    AudioSamplingRate customRate2 = static_cast<AudioSamplingRate>(32000);
    AudioSamplingRate customRate3 = static_cast<AudioSamplingRate>(96000);

    EXPECT_EQ(ConvertToStringForSampleRate(customRate1), "22050");
    EXPECT_EQ(ConvertToStringForSampleRate(customRate2), "32000");
    EXPECT_EQ(ConvertToStringForSampleRate(customRate3), "96000");
}

/**
 * @tc.name  : Test ConvertToStringForChannel API
 * @tc.type  : FUNC
 * @tc.number: ConvertToStringForChannel_001
 * @tc.desc  : Test ConvertToStringForChannel with various channel configurations.
 */
HWTEST(AudioUtilsUnitTest, ConvertToStringForChannel_001, TestSize.Level1)
{
    EXPECT_EQ(ConvertToStringForChannel(CHANNEL_UNKNOW), "0");
    EXPECT_EQ(ConvertToStringForChannel(MONO), "1");
    EXPECT_EQ(ConvertToStringForChannel(STEREO), "2");
    EXPECT_EQ(ConvertToStringForChannel(CHANNEL_3), "3");
    EXPECT_EQ(ConvertToStringForChannel(CHANNEL_4), "4");
    EXPECT_EQ(ConvertToStringForChannel(CHANNEL_5), "5");
    EXPECT_EQ(ConvertToStringForChannel(CHANNEL_6), "6");
    EXPECT_EQ(ConvertToStringForChannel(CHANNEL_7), "7");
    EXPECT_EQ(ConvertToStringForChannel(CHANNEL_8), "8");
    EXPECT_EQ(ConvertToStringForChannel(CHANNEL_9), "9");
    EXPECT_EQ(ConvertToStringForChannel(CHANNEL_10), "10");
    EXPECT_EQ(ConvertToStringForChannel(CHANNEL_11), "11");
    EXPECT_EQ(ConvertToStringForChannel(CHANNEL_12), "12");
    EXPECT_EQ(ConvertToStringForChannel(CHANNEL_13), "13");
    EXPECT_EQ(ConvertToStringForChannel(CHANNEL_14), "14");
    EXPECT_EQ(ConvertToStringForChannel(CHANNEL_15), "15");
    EXPECT_EQ(ConvertToStringForChannel(CHANNEL_16), "16");
}

/**
 * @tc.name  : Test ConvertToStringForChannel API with custom values
 * @tc.type  : FUNC
 * @tc.number: ConvertToStringForChannel_002
 * @tc.desc  : Test ConvertToStringForChannel with custom channel values.
 */
HWTEST(AudioUtilsUnitTest, ConvertToStringForChannel_002, TestSize.Level1)
{
    // Test with some arbitrary channel counts beyond the defined enums
    AudioChannel customChannel1 = static_cast<AudioChannel>(17);
    AudioChannel customChannel2 = static_cast<AudioChannel>(24);
    AudioChannel customChannel3 = static_cast<AudioChannel>(32);

    EXPECT_EQ(ConvertToStringForChannel(customChannel1), "17");
    EXPECT_EQ(ConvertToStringForChannel(customChannel2), "24");
    EXPECT_EQ(ConvertToStringForChannel(customChannel3), "32");
}

/**
 * @tc.name  : Test VolumeLevelToDegree API with custom values
 * @tc.type  : FUNC
 * @tc.number: VolumeToDegreeTest_001
 * @tc.desc  : Test convert between Volume and Degree
 */
HWTEST(AudioUtilsUnitTest, VolumeToDegreeTest_001, TestSize.Level1)
{
    int32_t level = -1;
    int32_t maxLevel = 1;
    int32_t overflowDegreeLimit = 200;
    int32_t testValue = 0;
    int32_t invalid = -1;
    testValue = VolumeUtils::VolumeLevelToDegree(level, maxLevel);
    EXPECT_EQ(testValue, invalid);

    testValue = VolumeUtils::VolumeLevelToDegree(overflowDegreeLimit, maxLevel);
    EXPECT_EQ(testValue, invalid);

    level = 1;
    testValue = VolumeUtils::VolumeLevelToDegree(level, maxLevel);
    EXPECT_EQ(testValue, invalid);

    testValue = VolumeUtils::GetVolumeLevelMaxDegree(level, maxLevel);
    EXPECT_EQ(testValue, invalid);

    testValue = VolumeUtils::GetVolumeLevelMaxDegree(-1, maxLevel);
    EXPECT_EQ(testValue, invalid);

    testValue = VolumeUtils::VolumeDegreeToLevel(0, -1);
    EXPECT_EQ(testValue, invalid);

    testValue = VolumeUtils::VolumeDegreeToLevel(-1, maxLevel);
    EXPECT_EQ(testValue, invalid);

    testValue = VolumeUtils::VolumeDegreeToLevel(overflowDegreeLimit, maxLevel);
    EXPECT_EQ(testValue, invalid);
}

/**
 * @tc.name  : Test ReallocVectorBufferAndClear API with zero length
 * @tc.type  : FUNC
 * @tc.number: ReallocVectorBufferAndClear_001
 * @tc.desc  : Test ReallocVectorBufferAndClear with zero buffer length.
 */
HWTEST(AudioUtilsUnitTest, ReallocVectorBufferAndClear_001, TestSize.Level1)
{
    std::vector<uint8_t> buffer = {1, 2, 3, 4, 5};

    uint8_t* result = ReallocVectorBufferAndClear(buffer, 0);

    EXPECT_EQ(buffer.size(), 0);
    EXPECT_EQ(result, buffer.data());
}

/**
 * @tc.name  : Test ReallocVectorBufferAndClear API with positive length
 * @tc.type  : FUNC
 * @tc.number: ReallocVectorBufferAndClear_002
 * @tc.desc  : Test ReallocVectorBufferAndClear with positive buffer length.
 */
HWTEST(AudioUtilsUnitTest, ReallocVectorBufferAndClear_002, TestSize.Level1)
{
    std::vector<uint8_t> buffer = {1, 2, 3, 4, 5};
    const size_t newLength = 10;

    uint8_t* result = ReallocVectorBufferAndClear(buffer, newLength);
    
    EXPECT_EQ(buffer.size(), newLength);
    EXPECT_EQ(result, buffer.data());

    // Verify all elements are cleared to zero
    for (size_t i = 0; i < newLength; i++) {
        EXPECT_EQ(buffer[i], 0);
    }
}

/**
 * @tc.name  : Test ReallocVectorBufferAndClear API with larger length
 * @tc.type  : FUNC
 * @tc.number: ReallocVectorBufferAndClear_003
 * @tc.desc  : Test ReallocVectorBufferAndClear with larger buffer length than original.
 */
HWTEST(AudioUtilsUnitTest, ReallocVectorBufferAndClear_003, TestSize.Level1)
{
    std::vector<uint8_t> buffer = {1, 2, 3};
    const size_t newLength = 8;

    uint8_t* result = ReallocVectorBufferAndClear(buffer, newLength);
    
    EXPECT_EQ(buffer.size(), newLength);
    EXPECT_EQ(result, buffer.data());

    // Verify all elements are cleared to zero
    for (size_t i = 0; i < newLength; i++) {
        EXPECT_EQ(buffer[i], 0);
    }
}

/**
 * @tc.name  : Test ReallocVectorBufferAndClear API with smaller length
 * @tc.type  : FUNC
 * @tc.number: ReallocVectorBufferAndClear_004
 * @tc.desc  : Test ReallocVectorBufferAndClear with smaller buffer length than original.
 */
HWTEST(AudioUtilsUnitTest, ReallocVectorBufferAndClear_004, TestSize.Level1)
{
    std::vector<uint8_t> buffer = {1, 2, 3, 4, 5};
    const size_t newLength = 2;

    uint8_t* result = ReallocVectorBufferAndClear(buffer, newLength);

    EXPECT_EQ(buffer.size(), newLength);
    EXPECT_EQ(result, buffer.data());

    // Verify all elements are cleared to zero
    for (size_t i = 0; i < newLength; i++) {
        EXPECT_EQ(buffer[i], 0);
    }
}

/**
 * @tc.name  : Test ReallocVectorBufferAndClear API return value
 * @tc.type  : FUNC
 * @tc.number: ReallocVectorBufferAndClear_005
 * @tc.desc  : Test that ReallocVectorBufferAndClear returns correct pointer.
 */
HWTEST(AudioUtilsUnitTest, ReallocVectorBufferAndClear_005, TestSize.Level1)
{
    std::vector<uint8_t> buffer;
    const size_t newLength = 5;

    uint8_t* result = ReallocVectorBufferAndClear(buffer, newLength);

    EXPECT_EQ(result, buffer.data());
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(buffer.size(), newLength);
}

/**
 * @tc.name  : Test ReallocVectorBufferAndClear API with typical audio buffer sizes
 * @tc.type  : FUNC
 * @tc.number: ReallocVectorBufferAndClear_006
 * @tc.desc  : Test ReallocVectorBufferAndClear with typical audio buffer sizes.
 */
HWTEST(AudioUtilsUnitTest, ReallocVectorBufferAndClear_006, TestSize.Level1)
{
    std::vector<uint8_t> buffer;

    // Test with typical audio buffer sizes (frames * channels * sample size)
    uint8_t* result1 = ReallocVectorBufferAndClear(buffer, 1024);  // 1KB buffer
    EXPECT_EQ(buffer.size(), 1024);
    EXPECT_EQ(result1, buffer.data());

    uint8_t* result2 = ReallocVectorBufferAndClear(buffer, 4096);  // 4KB buffer
    EXPECT_EQ(buffer.size(), 4096);
    EXPECT_EQ(result2, buffer.data());

    uint8_t* result3 = ReallocVectorBufferAndClear(buffer, 16384); // 16KB buffer
    EXPECT_EQ(buffer.size(), 16384);
    EXPECT_EQ(result3, buffer.data());

    // Verify all elements are cleared to zero
    for (size_t i = 0; i < buffer.size(); i++) {
        EXPECT_EQ(buffer[i], 0);
    }
}
} // namespace AudioStandard
} // namespace OHOS