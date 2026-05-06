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

#include <gtest/gtest.h>
#include "audio_errors.h"
#include "audio_suite_common.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;

namespace {
class AudioSuiteCommonTest : public testing::Test {
public:
    void SetUp() {};
    void TearDown() {};
};

HWTEST_F(AudioSuiteCommonTest, AudioSuiteCommonTestPushGetData_001, TestSize.Level0)
{
    AudioSuiteRingBuffer* cacheBuffer = new(std::nothrow) AudioSuiteRingBuffer(5);
    EXPECT_NE(cacheBuffer, nullptr);
    std::vector<uint8_t> data(5);
    std::vector<uint8_t> recvData(5);

    auto ret = cacheBuffer->PushData(recvData.data(), 0);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    ret = cacheBuffer->PushData(nullptr, 5);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    ret = cacheBuffer->PushData(data.data(), 5);
    EXPECT_EQ(ret, 0);
    ret = cacheBuffer->PushData(data.data(), 2);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    ret = cacheBuffer->GetData(recvData.data(), 0);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    ret = cacheBuffer->GetData(nullptr, 5);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    ret = cacheBuffer->GetData(recvData.data(), 2);
    EXPECT_EQ(ret, 0);
    ret = cacheBuffer->GetData(recvData.data(), 5);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

HWTEST_F(AudioSuiteCommonTest, AudioSuiteCommonTestResizeBuffer_001, TestSize.Level0)
{
    AudioSuiteRingBuffer* cacheBuffer = new(std::nothrow) AudioSuiteRingBuffer(5);
    EXPECT_NE(cacheBuffer, nullptr);

    auto ret = cacheBuffer->ResizeBuffer(6);
    EXPECT_EQ(ret, 0);

    ret = cacheBuffer->ClearBuffer();
    EXPECT_EQ(ret, 0);
}

HWTEST_F(AudioSuiteCommonTest, GetCurrentTimestampTest, TestSize.Level0)
{
    std::string timestamp = AudioSuiteUtil::GetCurrentTimestamp();
    EXPECT_TRUE(!timestamp.empty());
    EXPECT_TRUE(timestamp.find("-") != std::string::npos);
    EXPECT_TRUE(timestamp.find(":") != std::string::npos);
}

HWTEST_F(AudioSuiteCommonTest, GetWorkModeStringTest, TestSize.Level0)
{
    std::string editMode = AudioSuiteUtil::GetWorkModeString(PIPELINE_EDIT_MODE);
    EXPECT_EQ(editMode, "EDIT_MODE");

    std::string realtimeMode = AudioSuiteUtil::GetWorkModeString(PIPELINE_REALTIME_MODE);
    EXPECT_EQ(realtimeMode, "REALTIME_MODE");

    std::string unknownMode = AudioSuiteUtil::GetWorkModeString(static_cast<PipelineWorkMode>(999));
    EXPECT_EQ(unknownMode, "UNKNOWN");
}

HWTEST_F(AudioSuiteCommonTest, GetStateStringTest, TestSize.Level0)
{
    std::string stopped = AudioSuiteUtil::GetStateString(PIPELINE_STOPPED);
    EXPECT_EQ(stopped, "STOPPED");

    std::string running = AudioSuiteUtil::GetStateString(PIPELINE_RUNNING);
    EXPECT_EQ(running, "RUNNING");

    std::string unknown = AudioSuiteUtil::GetStateString(static_cast<AudioSuitePipelineState>(999));
    EXPECT_EQ(unknown, "UNKNOWN");
}

HWTEST_F(AudioSuiteCommonTest, GetSampleFormatStringTest, TestSize.Level0)
{
    std::string pcm16 = AudioSuiteUtil::GetSampleFormatString(SAMPLE_S16LE);
    EXPECT_EQ(pcm16, "PCM16");

    std::string pcm24 = AudioSuiteUtil::GetSampleFormatString(SAMPLE_S24LE);
    EXPECT_EQ(pcm24, "PCM24");

    std::string pcm32 = AudioSuiteUtil::GetSampleFormatString(SAMPLE_S32LE);
    EXPECT_EQ(pcm32, "PCM32");

    std::string float32 = AudioSuiteUtil::GetSampleFormatString(SAMPLE_F32LE);
    EXPECT_EQ(float32, "FLOAT32");

    std::string unknown = AudioSuiteUtil::GetSampleFormatString(static_cast<AudioSampleFormat>(999));
    EXPECT_EQ(unknown, "UNKNOWN");
}

HWTEST_F(AudioSuiteCommonTest, IsEnumTypeOptionTest, TestSize.Level0)
{
    EXPECT_TRUE(AudioSuiteUtil::IsEnumTypeOption("EnvironmentType"));
    EXPECT_TRUE(AudioSuiteUtil::IsEnumTypeOption("SoundFieldType"));
    EXPECT_TRUE(AudioSuiteUtil::IsEnumTypeOption("VoiceBeautifierType"));
    EXPECT_TRUE(AudioSuiteUtil::IsEnumTypeOption("GeneralVoiceChangeType"));
    EXPECT_FALSE(AudioSuiteUtil::IsEnumTypeOption("AudioEqualizerFrequencyBandGains"));
    EXPECT_FALSE(AudioSuiteUtil::IsEnumTypeOption("UnknownType"));
}

HWTEST_F(AudioSuiteCommonTest, ConvertEnumToStringTest, TestSize.Level0)
{
    std::string result = AudioSuiteUtil::ConvertEnumToString("EnvironmentType", "0");
    EXPECT_EQ(result, "Close");

    result = AudioSuiteUtil::ConvertEnumToString("EnvironmentType", "1");
    EXPECT_EQ(result, "Broadcast");

    result = AudioSuiteUtil::ConvertEnumToString("SoundFieldType", "0");
    EXPECT_EQ(result, "Close");

    result = AudioSuiteUtil::ConvertEnumToString("VoiceBeautifierType", "1");
    EXPECT_EQ(result, "Clear");

    result = AudioSuiteUtil::ConvertEnumToString("GeneralVoiceChangeType", "1");
    EXPECT_EQ(result, "Cute");

    result = AudioSuiteUtil::ConvertEnumToString("UnknownType", "1");
    EXPECT_EQ(result, "1");

    result = AudioSuiteUtil::ConvertEnumToString("EnvironmentType", "999");
    EXPECT_EQ(result, "999");
}

HWTEST_F(AudioSuiteCommonTest, FormatOptionsValueTest, TestSize.Level0)
{
    std::string result = AudioSuiteUtil::FormatOptionsValue("AudioEqualizerFrequencyBandGains", "0:0:0:0:0:0:0:0:0:0");
    EXPECT_TRUE(result.find("band0:0") != std::string::npos);

    result = AudioSuiteUtil::FormatOptionsValue("AudioSpaceRenderPositionParams", "1.0,2.0,3.0");
    EXPECT_TRUE(result.find("x:1.0") != std::string::npos);
    EXPECT_TRUE(result.find("y:2.0") != std::string::npos);
    EXPECT_TRUE(result.find("z:3.0") != std::string::npos);

    result = AudioSuiteUtil::FormatOptionsValue("EnvironmentType", "1");
    EXPECT_EQ(result, "Broadcast");

    result = AudioSuiteUtil::FormatOptionsValue("", "");
    EXPECT_EQ(result, "");

    result = AudioSuiteUtil::FormatOptionsValue("UnknownField", "value");
    EXPECT_EQ(result, "value");
}

HWTEST_F(AudioSuiteCommonTest, GetSampleSizeTest, TestSize.Level0)
{
    uint32_t size = AudioSuiteUtil::GetSampleSize(SAMPLE_U8);
    EXPECT_EQ(size, 1);

    size = AudioSuiteUtil::GetSampleSize(SAMPLE_S16LE);
    EXPECT_EQ(size, 2);

    size = AudioSuiteUtil::GetSampleSize(SAMPLE_S24LE);
    EXPECT_EQ(size, 3);

    size = AudioSuiteUtil::GetSampleSize(SAMPLE_S32LE);
    EXPECT_EQ(size, 4);

    size = AudioSuiteUtil::GetSampleSize(SAMPLE_F32LE);
    EXPECT_EQ(size, 4);
}
}