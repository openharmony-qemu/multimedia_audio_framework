/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <thread>
#include <gtest/gtest.h>
#include "audio_utils.h"
#include "parameter.h"
#include "audio_channel_blend.h"
#include "volume_ramp.h"
#include "audio_speed.h"
#include "audio_errors.h"
#include "audio_performance_monitor.h"
#include "audio_stream_info.h"
#include "audio_performance_monitor_c.h"

using namespace testing::ext;
using namespace std;
namespace OHOS {
namespace AudioStandard {

class AudioUtilsPlusUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioUtilsPlusUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void AudioUtilsPlusUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void AudioUtilsPlusUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioUtilsPlusUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_001
* @tc.desc  : Test UpdateMaxAmplitude
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_001, TestSize.Level1)
{
    ConvertHdiFormat adapterFormat = SAMPLE_U8_C;
    char frame[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    uint64_t replyBytes = 10;
    float result = UpdateMaxAmplitude(adapterFormat, frame, replyBytes);

    EXPECT_NEAR(result, 0, 0.9);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_002
* @tc.desc  : Test UpdateMaxAmplitude
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_002, TestSize.Level1)
{
    ConvertHdiFormat adapterFormat = SAMPLE_S16_C;
    char frame[20] = {0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9};
    uint64_t replyBytes = 10;
    float result = UpdateMaxAmplitude(adapterFormat, frame, replyBytes);

    EXPECT_NEAR(result, 0, 0.9);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_003
* @tc.desc  : Test UpdateMaxAmplitude
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_003, TestSize.Level1)
{
    ConvertHdiFormat adapterFormat = SAMPLE_S24_C;
    char frame[30] = {0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4,
        5, 5, 5, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9};
    uint64_t replyBytes = 10;
    float result = UpdateMaxAmplitude(adapterFormat, frame, replyBytes);

    EXPECT_NEAR(result, 0, 0.9);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_004
* @tc.desc  : Test UpdateMaxAmplitude
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_004, TestSize.Level1)
{
    ConvertHdiFormat adapterFormat = SAMPLE_S32_C;
    char frame[40] = {0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4,
                    5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 9};
    uint64_t replyBytes = 10;
    float result = UpdateMaxAmplitude(adapterFormat, frame, replyBytes);

    EXPECT_NEAR(result, 0, 0.9);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_006
* @tc.desc  : Test CalculateMaxAmplitudeForPCM8Bit
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_006, TestSize.Level1)
{
    int8_t frame[2] = {3, -1};
    uint64_t nSamples = 2;
    auto result = CalculateMaxAmplitudeForPCM8Bit(frame, nSamples);

    EXPECT_NEAR(result, 0, 0.9);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_007
* @tc.desc  : Test CalculateMaxAmplitudeForPCM16Bit
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_007, TestSize.Level1)
{
    int16_t frame[2] = {3, -1};
    uint64_t nSamples = 2;
    auto result = CalculateMaxAmplitudeForPCM16Bit(frame, nSamples);

    EXPECT_NEAR(result, 0, 0.9);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_008
* @tc.desc  : Test CalculateMaxAmplitudeForPCM24Bit
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_008, TestSize.Level1)
{
    char frame[3] = {0xFF, 0xFF, 0xFF};
    uint64_t nSamples = 1;
    auto result = CalculateMaxAmplitudeForPCM24Bit(frame, nSamples);

    EXPECT_NEAR(result, 0, 2.5);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_009
* @tc.desc  : Test CalculateMaxAmplitudeForPCM24Bit
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_009, TestSize.Level1)
{
    char frame[3] = {0x00, 0x00, 0x00};
    uint64_t nSamples = 1;
    auto result = CalculateMaxAmplitudeForPCM24Bit(frame, nSamples);

    EXPECT_NEAR(result, 0, 0.9);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_010
* @tc.desc  : Test CalculateMaxAmplitudeForPCM32Bit
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_010, TestSize.Level1)
{
    int32_t frame[2] = {3, -1};
    uint64_t nSamples = 1;
    auto result = CalculateMaxAmplitudeForPCM32Bit(frame, nSamples);

    EXPECT_NEAR(result, 0, 0.9);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_011
* @tc.desc  : Test GetFormatByteSize
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_011, TestSize.Level1)
{
    int32_t format = SAMPLE_S16LE;
    auto result = GetFormatByteSize(format);

    EXPECT_EQ(result, 2);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_012
* @tc.desc  : Test GetFormatByteSize
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_012, TestSize.Level1)
{
    int32_t format = SAMPLE_S24LE;
    auto result = GetFormatByteSize(format);

    EXPECT_EQ(result, 3);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_013
* @tc.desc  : Test GetFormatByteSize
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_013, TestSize.Level1)
{
    int32_t format = SAMPLE_S32LE;
    auto result = GetFormatByteSize(format);
    EXPECT_EQ(result, 4);

    result = 0;
    format = SAMPLE_F32LE;
    result = GetFormatByteSize(format);
    EXPECT_EQ(result, 4);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_014
* @tc.desc  : Test AudioInfoDumpUtils::GetDeviceTypeName
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_014, TestSize.Level1)
{
    DeviceType deviceType = DeviceType::DEVICE_TYPE_EARPIECE;
    std::string device = "EARPIECE";
    auto result = AudioInfoDumpUtils::GetDeviceTypeName(deviceType);

    EXPECT_EQ(result, device);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_015
* @tc.desc  : Test AudioInfoDumpUtils::GetDeviceTypeName
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_015, TestSize.Level1)
{
    DeviceType deviceType = DeviceType::DEVICE_TYPE_SPEAKER;
    std::string device = "SPEAKER";
    auto result = AudioInfoDumpUtils::GetDeviceTypeName(deviceType);

    EXPECT_EQ(result, device);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_016
* @tc.desc  : Test AudioInfoDumpUtils::GetDeviceTypeName
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_016, TestSize.Level1)
{
    DeviceType deviceType = DeviceType::DEVICE_TYPE_WIRED_HEADSET;
    std::string device = "WIRED_HEADSET";
    auto result = AudioInfoDumpUtils::GetDeviceTypeName(deviceType);

    EXPECT_EQ(result, device);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_017
* @tc.desc  : Test AudioInfoDumpUtils::GetDeviceTypeName
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_017, TestSize.Level1)
{
    DeviceType deviceType = DeviceType::DEVICE_TYPE_WIRED_HEADPHONES;
    std::string device = "WIRED_HEADPHONES";
    auto result = AudioInfoDumpUtils::GetDeviceTypeName(deviceType);

    EXPECT_EQ(result, device);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_018
* @tc.desc  : Test AudioInfoDumpUtils::GetDeviceTypeName
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_018, TestSize.Level1)
{
    DeviceType deviceType = DeviceType::DEVICE_TYPE_BLUETOOTH_SCO;
    std::string device = "BLUETOOTH_SCO";
    auto result = AudioInfoDumpUtils::GetDeviceTypeName(deviceType);

    EXPECT_EQ(result, device);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_019
* @tc.desc  : Test AudioInfoDumpUtils::GetDeviceTypeName
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_019, TestSize.Level1)
{
    DeviceType deviceType = DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP;
    std::string device = "BLUETOOTH_A2DP";
    auto result = AudioInfoDumpUtils::GetDeviceTypeName(deviceType);

    EXPECT_EQ(result, device);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_020
* @tc.desc  : Test AudioInfoDumpUtils::GetDeviceTypeName
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_020, TestSize.Level1)
{
    DeviceType deviceType = DeviceType::DEVICE_TYPE_MIC;
    std::string device = "MIC";
    auto result = AudioInfoDumpUtils::GetDeviceTypeName(deviceType);

    EXPECT_EQ(result, device);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_021
* @tc.desc  : Test AudioInfoDumpUtils::GetDeviceTypeName
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_021, TestSize.Level1)
{
    DeviceType deviceType = DeviceType::DEVICE_TYPE_WAKEUP;
    std::string device = "WAKEUP";
    auto result = AudioInfoDumpUtils::GetDeviceTypeName(deviceType);

    EXPECT_EQ(result, device);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_022
* @tc.desc  : Test AudioInfoDumpUtils::GetDeviceTypeName
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_022, TestSize.Level1)
{
    DeviceType deviceType = DeviceType::DEVICE_TYPE_NONE;
    std::string device = "NONE";
    auto result = AudioInfoDumpUtils::GetDeviceTypeName(deviceType);

    EXPECT_EQ(result, device);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_023
* @tc.desc  : Test AudioInfoDumpUtils::GetDeviceTypeName
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_023, TestSize.Level1)
{
    DeviceType deviceType = DeviceType::DEVICE_TYPE_INVALID;
    std::string device = "INVALID";
    auto result = AudioInfoDumpUtils::GetDeviceTypeName(deviceType);

    EXPECT_EQ(result, device);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_024
* @tc.desc  : Test AudioInfoDumpUtils::GetDeviceTypeName
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_024, TestSize.Level1)
{
    DeviceType deviceType = DeviceType::DEVICE_TYPE_USB_ARM_HEADSET;
    std::string device = "UNKNOWN";
    auto result = AudioInfoDumpUtils::GetDeviceTypeName(deviceType);

    EXPECT_EQ(result, device);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_025
* @tc.desc  : Test AudioInfoDumpUtils::GetConnectTypeName
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_025, TestSize.Level1)
{
    ConnectType connectType = OHOS::AudioStandard::CONNECT_TYPE_LOCAL;
    std::string device = "LOCAL";
    auto result = AudioInfoDumpUtils::GetConnectTypeName(connectType);

    EXPECT_EQ(result, device);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_026
* @tc.desc  : Test AudioInfoDumpUtils::GetConnectTypeName
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_026, TestSize.Level1)
{
    ConnectType connectType = static_cast<ConnectType>(5);
    std::string device = "UNKNOWN";
    auto result = AudioInfoDumpUtils::GetConnectTypeName(connectType);

    EXPECT_EQ(result, device);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_027
* @tc.desc  : Test AudioInfoDumpUtils::GetConnectTypeName
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_027, TestSize.Level1)
{
    ConnectType connectType = static_cast<ConnectType>(5);
    std::string device = "UNKNOWN";
    auto result = AudioInfoDumpUtils::GetConnectTypeName(connectType);

    EXPECT_EQ(result, device);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_028
* @tc.desc  : Test AudioInfoDumpUtils::GetSourceName
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_028, TestSize.Level1)
{
    SourceType sourceType = SourceType::SOURCE_TYPE_INVALID;
    std::string device = "INVALID";
    auto result = AudioInfoDumpUtils::GetSourceName(sourceType);

    EXPECT_EQ(result, device);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_029
* @tc.desc  : Test AudioInfoDumpUtils::GetSourceName
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_029, TestSize.Level1)
{
    SourceType sourceType = SourceType::SOURCE_TYPE_MIC;
    std::string device = "MIC";
    auto result = AudioInfoDumpUtils::GetSourceName(sourceType);

    EXPECT_EQ(result, device);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_030
* @tc.desc  : Test AudioInfoDumpUtils::GetSourceName
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_030, TestSize.Level1)
{
    SourceType sourceType = SourceType::SOURCE_TYPE_CAMCORDER;
    std::string device = "CAMCORDER";
    auto result = AudioInfoDumpUtils::GetSourceName(sourceType);

    EXPECT_EQ(result, device);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_031
* @tc.desc  : Test AudioInfoDumpUtils::GetSourceName
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_031, TestSize.Level1)
{
    SourceType sourceType = SourceType::SOURCE_TYPE_VOICE_RECOGNITION;
    std::string device = "VOICE_RECOGNITION";
    auto result = AudioInfoDumpUtils::GetSourceName(sourceType);

    EXPECT_EQ(result, device);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_032
* @tc.desc  : Test AudioInfoDumpUtils::GetSourceName
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_032, TestSize.Level1)
{
    SourceType sourceType = SourceType::SOURCE_TYPE_ULTRASONIC;
    std::string device = "ULTRASONIC";
    auto result = AudioInfoDumpUtils::GetSourceName(sourceType);

    EXPECT_EQ(result, device);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_033
* @tc.desc  : Test AudioInfoDumpUtils::GetSourceName
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_033, TestSize.Level1)
{
    SourceType sourceType = SourceType::SOURCE_TYPE_VOICE_COMMUNICATION;
    std::string device = "VOICE_COMMUNICATION";
    auto result = AudioInfoDumpUtils::GetSourceName(sourceType);

    EXPECT_EQ(result, device);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_034
* @tc.desc  : Test AudioInfoDumpUtils::GetSourceName
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_034, TestSize.Level1)
{
    SourceType sourceType = SourceType::SOURCE_TYPE_WAKEUP;
    std::string device = "WAKEUP";
    auto result = AudioInfoDumpUtils::GetSourceName(sourceType);

    EXPECT_EQ(result, device);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioUtilsPlusUnitTest_035
* @tc.desc  : Test AudioInfoDumpUtils::GetSourceName
*/
HWTEST(AudioUtilsPlusUnitTest, AudioUtilsPlusUnitTest_035, TestSize.Level1)
{
    SourceType sourceType = SourceType::SOURCE_TYPE_MIC_REF;
    std::string device = "UNKNOWN";
    auto result = AudioInfoDumpUtils::GetSourceName(sourceType);

    EXPECT_EQ(result, device);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioPerformanaceMonitor_001
* @tc.desc  : Test AudioPerformanaceMonitor::GetInstance
*/
HWTEST(AudioUtilsPlusUnitTest, AudioPerformanaceMonitor_001, TestSize.Level3)
{
    EXPECT_NE(&AudioPerformanceMonitor::GetInstance(), nullptr);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioPerformanaceMonitor_002
* @tc.desc  : Test AudioPerformanaceMonitor::RecordSilenceState--record first time
*/
HWTEST(AudioUtilsPlusUnitTest, AudioPerformanaceMonitor_002, TestSize.Level3)
{
    uint32_t sessionId = 111111;
    EXPECT_EQ(AudioPerformanceMonitor::GetInstance().silenceDetectMap_.find(sessionId),
        AudioPerformanceMonitor::GetInstance().silenceDetectMap_.end());
    AudioPerformanceMonitor::GetInstance().RecordSilenceState(sessionId, true, PIPE_TYPE_NORMAL_OUT, 0);
    EXPECT_NE(AudioPerformanceMonitor::GetInstance().silenceDetectMap_.find(sessionId),
        AudioPerformanceMonitor::GetInstance().silenceDetectMap_.end());
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioPerformanaceMonitor_003
* @tc.desc  : Test AudioPerformanaceMonitor::RecordSilenceState--record excceds queue size
*/
HWTEST(AudioUtilsPlusUnitTest, AudioPerformanaceMonitor_003, TestSize.Level3)
{
    uint32_t sessionId = 111111;
    for (size_t i = 0; i < MAX_RECORD_QUEUE_SIZE + 1; ++i) {
        AudioPerformanceMonitor::GetInstance().RecordSilenceState(sessionId, true, PIPE_TYPE_NORMAL_OUT, 0);
    }
    EXPECT_NE(AudioPerformanceMonitor::GetInstance().silenceDetectMap_.find(sessionId),
        AudioPerformanceMonitor::GetInstance().silenceDetectMap_.end());
    EXPECT_EQ(AudioPerformanceMonitor::GetInstance().silenceDetectMap_[sessionId].historyStateDeque.size(),
        MAX_RECORD_QUEUE_SIZE);
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioPerformanaceMonitor_004
* @tc.desc  : Test AudioPerformanaceMonitor::StartSilenceMonitor
*/
HWTEST(AudioUtilsPlusUnitTest, AudioPerformanaceMonitor_004, TestSize.Level3)
{
    uint32_t sessionId = 111111;
    AudioPerformanceMonitor::GetInstance().StartSilenceMonitor(sessionId, 0);
    EXPECT_EQ(AudioPerformanceMonitor::GetInstance().silenceDetectMap_[sessionId].historyStateDeque.size(),
        static_cast<size_t>(0));
    uint32_t notExistSessionId = 111112;
    AudioPerformanceMonitor::GetInstance().StartSilenceMonitor(notExistSessionId, 0);
    EXPECT_NE(AudioPerformanceMonitor::GetInstance().silenceDetectMap_.find(notExistSessionId),
        AudioPerformanceMonitor::GetInstance().silenceDetectMap_.end());
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioPerformanaceMonitor_005
* @tc.desc  : Test AudioPerformanaceMonitor::RecordSilenceState--record false and detect as noise event
*/
HWTEST(AudioUtilsPlusUnitTest, AudioPerformanaceMonitor_005, TestSize.Level3)
{
    uint32_t sessionId = 111111;
    AudioPerformanceMonitor::GetInstance().RecordSilenceState(sessionId, false, PIPE_TYPE_NORMAL_OUT, 0);
    for (size_t i = 0; i < MIN_SILENCE_FRAME_COUNT; ++i) {
        AudioPerformanceMonitor::GetInstance().RecordSilenceState(sessionId, true, PIPE_TYPE_NORMAL_OUT, 0);
    }
    AudioPerformanceMonitor::GetInstance().RecordSilenceState(sessionId, false, PIPE_TYPE_NORMAL_OUT, 0);
    EXPECT_EQ(AudioPerformanceMonitor::GetInstance().silenceDetectMap_[sessionId].historyStateDeque.size(),
        static_cast<size_t>(0));
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioPerformanaceMonitor_006
* @tc.desc  : Test AudioPerformanaceMonitor::deleteSilenceMonitor
*/
HWTEST(AudioUtilsPlusUnitTest, AudioPerformanaceMonitor_006, TestSize.Level3)
{
    uint32_t sessionId = 111111;
    AudioPerformanceMonitor::GetInstance().DeleteSilenceMonitor(sessionId);
    EXPECT_EQ(AudioPerformanceMonitor::GetInstance().silenceDetectMap_.find(sessionId),
        AudioPerformanceMonitor::GetInstance().silenceDetectMap_.end());
}

/**
* @tc.name  : Test AudioUtilsUnitTest API
* @tc.type  : FUNC
* @tc.number: AudioPerformanaceMonitor_007
* @tc.desc  : Test AudioPerformanaceMonitor::RecordTimeStamp
*/
HWTEST(AudioUtilsPlusUnitTest, AudioPerformanaceMonitor_007, TestSize.Level3)
{
    AdapterType adapterType = ADAPTER_TYPE_PRIMARY;
    AudioPerformanceMonitor::GetInstance().RecordTimeStamp(adapterType, INIT_LASTWRITTEN_TIME);
    int64_t curTime = ClockTime::GetCurNano();
    AudioPerformanceMonitor::GetInstance().RecordTimeStamp(adapterType, curTime);
    int64_t exeedTime = ClockTime::GetCurNano() + 100000000; // add 100ms
    AudioPerformanceMonitor::GetInstance().RecordTimeStamp(adapterType, exeedTime);
    AudioPerformanceMonitor::GetInstance().DeleteOvertimeMonitor(adapterType);
    EXPECT_EQ(AudioPerformanceMonitor::GetInstance().overTimeDetectMap_.size(), static_cast<size_t>(0));
}

/**
* @tc.name  : Test RecordPaSilenceState API
* @tc.type  : FUNC
* @tc.number: RecordPaSilenceState_001
* @tc.desc  : Test RecordPaSilenceState
*/
HWTEST(AudioUtilsPlusUnitTest, RecordPaSilenceState_001, TestSize.Level3)
{
    uint32_t sessionId = 111111;
    bool isSilence = true;
    enum PA_PIPE_TYPE paPipeType = static_cast<PA_PIPE_TYPE>(100);
    uint32_t  uid = 1000;
    RecordPaSilenceState(sessionId, isSilence, PA_PIPE_TYPE_NORMAL, uid);
    RecordPaSilenceState(sessionId, isSilence, PA_PIPE_TYPE_MULTICHANNEL, uid);
    RecordPaSilenceState(sessionId, isSilence, paPipeType, uid);
    AudioPerformanceMonitor::GetInstance().DeleteSilenceMonitor(sessionId);
    EXPECT_EQ(AudioPerformanceMonitor::GetInstance().silenceDetectMap_.size(), 1);
}

/**
* @tc.name  : Test PauseSilenceMonitor API
* @tc.type  : FUNC
* @tc.number: PauseSilenceMonitor_001
* @tc.desc  : Test PauseSilenceMonitor
*/
HWTEST(AudioUtilsPlusUnitTest, PauseSilenceMonitor_001, TestSize.Level3)
{
    uint32_t sessionId = 111111;
    AudioPerformanceMonitor::GetInstance().silenceDetectMap_ = {};
    AudioPerformanceMonitor::GetInstance().PauseSilenceMonitor(sessionId);

    AudioPerformanceMonitor::GetInstance().StartSilenceMonitor(sessionId, 0);
    EXPECT_EQ(AudioPerformanceMonitor::GetInstance().silenceDetectMap_[sessionId].isRunning, true);

    AudioPerformanceMonitor::GetInstance().PauseSilenceMonitor(sessionId);
    EXPECT_EQ(AudioPerformanceMonitor::GetInstance().silenceDetectMap_[sessionId].isRunning, false);
}

/**
* @tc.name  : Test GetRunningHapNames API
* @tc.type  : FUNC
* @tc.number: GetRunningHapNames_001
* @tc.desc  : Test GetRunningHapNames
*/
HWTEST(AudioUtilsPlusUnitTest, GetRunningHapNames_001, TestSize.Level3)
{
    uint32_t sessionId1 = 1001;
    uint32_t tokenId1 = 100001;
    uint32_t sessionId2 = 1002;
    uint32_t tokenId2 = 100002;
    uint32_t sessionId3 = 1003;
    uint32_t tokenId3 = 100003;

    AudioPerformanceMonitor::GetInstance().silenceDetectMap_ = {};
    AudioPerformanceMonitor::GetInstance().StartSilenceMonitor(sessionId1, tokenId1);
    AudioPerformanceMonitor::GetInstance().RecordSilenceState(sessionId1, false, PIPE_TYPE_NORMAL_OUT, 0);
    AudioPerformanceMonitor::GetInstance().StartSilenceMonitor(sessionId2, tokenId2);
    AudioPerformanceMonitor::GetInstance().RecordSilenceState(sessionId2, false, PIPE_TYPE_NORMAL_OUT, 0);
    AudioPerformanceMonitor::GetInstance().StartSilenceMonitor(sessionId3, tokenId3);
    AudioPerformanceMonitor::GetInstance().RecordSilenceState(sessionId3, false, PIPE_TYPE_LOWLATENCY_OUT, 0);

    AudioPerformanceMonitor::GetInstance().GetRunningHapNames(ADAPTER_TYPE_PRIMARY);

    AudioPerformanceMonitor::GetInstance().PauseSilenceMonitor(sessionId1);
    EXPECT_EQ(AudioPerformanceMonitor::GetInstance().silenceDetectMap_[sessionId1].isRunning, false);

    EXPECT_EQ(AudioPerformanceMonitor::GetInstance().silenceDetectMap_[sessionId2].isRunning, true);
}

/**
* @tc.name  : Test JudgeNoise API
* @tc.type  : FUNC
* @tc.number: JudgeNoise_001
* @tc.desc  : Test JudgeNoise
*/
HWTEST(AudioUtilsPlusUnitTest, JudgeNoise_001, TestSize.Level3)
{
    uint32_t sessionId = 111111;
    AudioPerformanceMonitor::GetInstance().silenceDetectMap_ = {};

    AudioPerformanceMonitor::GetInstance().StartSilenceMonitor(sessionId, 0);
    AudioPerformanceMonitor::GetInstance().RecordSilenceState(sessionId, false, PIPE_TYPE_NORMAL_OUT, 0);
    AudioPerformanceMonitor::GetInstance().RecordSilenceState(sessionId, false, PIPE_TYPE_NORMAL_OUT, 0);

    EXPECT_EQ(AudioPerformanceMonitor::GetInstance().silenceDetectMap_[sessionId].isRunning, true);

    AudioPerformanceMonitor::GetInstance().RecordSilenceState(sessionId, true, PIPE_TYPE_LOWLATENCY_OUT, 0);
    AudioPerformanceMonitor::GetInstance().PauseSilenceMonitor(sessionId);
    EXPECT_EQ(AudioPerformanceMonitor::GetInstance().silenceDetectMap_[sessionId].isRunning, false);
}

/**
* @tc.name  : Test ReportEvent
* @tc.type  : FUNC
* @tc.number: ReportEvent_001
* @tc.desc  : Test ReportEvent
*/
HWTEST(AudioUtilsPlusUnitTest, ReportEvent_001, TestSize.Level4)
{
    AudioPerformanceMonitor::GetInstance().ReportEvent(

        static_cast<DetectEvent>(100), 1, PIPE_TYPE_UNKNOWN, ADAPTER_TYPE_UNKNOWN, 0);
    EXPECT_NE(AudioPerformanceMonitor::GetInstance().overTimeLastReportTime_, ClockTime::GetRealNano());
}

/**
* @tc.name  : Test GetStreamUsageByVolumeTypeForFetchDevice
* @tc.type  : FUNC
* @tc.number: GetStreamUsageByVolumeTypeForFetchDevice
* @tc.desc  : Test GetStreamUsageByVolumeTypeForFetchDevice
*/
HWTEST(AudioUtilsPlusUnitTest, GetStreamUsageByVolumeTypeForFetchDevice, TestSize.Level4)
{
    std::vector<StreamUsage> usages =
        VolumeUtils::GetStreamUsageByVolumeTypeForFetchDevice(STREAM_MUSIC);
    EXPECT_EQ(usages.size(), 1);
    usages = VolumeUtils::GetStreamUsageByVolumeTypeForFetchDevice(STREAM_ALL);
    EXPECT_EQ(usages.size(), 1);
}

} // namespace AudioStandard
} // namespace OHOS