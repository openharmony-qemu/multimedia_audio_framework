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
#include <thread>
#include <vector>
#include <mutex>

#include "audio_service_log.h"
#include "audio_errors.h"
#include "audio_pitch_processor.h"
#include "audio_utils.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class AudioPitchProcessorUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioPitchProcessorUnitTest::SetUpTestCase(void)
{
}

void AudioPitchProcessorUnitTest::TearDownTestCase(void)
{
}

void AudioPitchProcessorUnitTest::SetUp()
{
}

void AudioPitchProcessorUnitTest::TearDown()
{
}

static AudioStreamInfo CreateTestStreamInfo(uint32_t sampleRate = 48000, uint32_t channels = 2)
{
    AudioStreamInfo streamInfo;
    streamInfo.samplingRate = static_cast<AudioSamplingRate>(sampleRate);
    streamInfo.channels = static_cast<AudioChannel>(channels);
    streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    streamInfo.channelLayout = static_cast<AudioChannelLayout>(0);
    return streamInfo;
}

HWTEST(AudioPitchProcessorUnitTest, CreateInstance_001, TestSize.Level1)
{
    AudioStreamInfo streamInfo = CreateTestStreamInfo();
    auto processor = AudioPitchProcessor::CreateInstance(streamInfo);
    EXPECT_NE(processor, nullptr);
}

HWTEST(AudioPitchProcessorUnitTest, UnsupportedChannel_001, TestSize.Level1)
{
    AudioStreamInfo streamInfo = CreateTestStreamInfo(48000, 4);
    auto processor = AudioPitchProcessor::CreateInstance(streamInfo);
    EXPECT_NE(processor, nullptr);

    int32_t ret = processor->PitchAlgoInit();
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

HWTEST(AudioPitchProcessorUnitTest, SetPitchWithoutInit_001, TestSize.Level1)
{
    AudioStreamInfo streamInfo = CreateTestStreamInfo(48000, 2);
    auto processor = AudioPitchProcessor::CreateInstance(streamInfo);
    EXPECT_NE(processor, nullptr);

    int32_t ret = processor->SetPitch(1.5f);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

} // namespace AudioStandard
} // namespace OHOS
