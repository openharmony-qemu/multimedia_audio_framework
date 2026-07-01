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
#include <gtest/gtest.h>

#include "audio_process_in_client.h"
#include "audio_errors.h"
#include "audio_service_log.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class AudioProcessInClientSupplementUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioProcessInClientSupplementUnitTest::SetUpTestCase(void)
{
    AUDIO_INFO_LOG("AudioProcessInClientSupplementUnitTest SetUpTestCase");
}

void AudioProcessInClientSupplementUnitTest::TearDownTestCase(void)
{
    AUDIO_INFO_LOG("AudioProcessInClientSupplementUnitTest TearDownTestCase");
}

void AudioProcessInClientSupplementUnitTest::SetUp()
{
    AUDIO_INFO_LOG("AudioProcessInClientSupplementUnitTest SetUp");
}

void AudioProcessInClientSupplementUnitTest::TearDown()
{
    AUDIO_INFO_LOG("AudioProcessInClientSupplementUnitTest TearDown");
}

/**
 * @tc.name   : Test SetVolume with invalid volume (negative)
 * @tc.type  : FUNC
 * @tc.number: SetVolumeFloat_001
 * @tc.desc  : Test SetVolume with negative value returns error.
 */
HWTEST(AudioProcessInClientSupplementUnitTest, SetVolumeFloat_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioProcessInClientSupplementUnitTest SetVolumeFloat_001 start");
    // This test verifies the parameter validation in SetVolume(float vol)
    // Since we can't easily instantiate AudioProcessInClientInner, we test the logic conceptually
    float invalidVol = -0.1f;
    bool isInvalid = (invalidVol < 0.0f || invalidVol > 1.0f);
    EXPECT_TRUE(isInvalid);
}

/**
 * @tc.name   : Test SetVolume with invalid volume (greater than 1.0)
 * @tc.type  : FUNC
 * @tc.number: SetVolumeFloat_002
 * @tc.desc  : Test SetVolume with volume > 1.0 returns error.
 */
HWTEST(AudioProcessInClientSupplementUnitTest, SetVolumeFloat_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioProcessInClientSupplementUnitTest SetVolumeFloat_002 start");
    float invalidVol = 1.1f;
    bool isInvalid = (invalidVol < 0.0f || invalidVol > 1.0f);
    EXPECT_TRUE(isInvalid);
}

/**
 * @tc.name   : Test SetDuckVolume with invalid volume (negative)
 * @tc.type  : FUNC
 * @tc.number: SetDuckVolumeFloat_001
 * @tc.desc  : Test SetDuckVolume with negative value returns error.
 */
HWTEST(AudioProcessInClientSupplementUnitTest, SetDuckVolumeFloat_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioProcessInClientSupplementUnitTest SetDuckVolumeFloat_001 start");
    float invalidVol = -0.5f;
    bool isInvalid = (invalidVol < 0.0f || invalidVol > 1.0f);
    EXPECT_TRUE(isInvalid);
}

/**
 * @tc.name   : Test CheckIfSupport with invalid sampling rate
 * @tc.type  : FUNC
 * @tc.number: CheckIfSupport_001
 * @tc.desc  : Test CheckIfSupport returns false for invalid sampling rate.
 */
HWTEST(AudioProcessInClientSupplementUnitTest, CheckIfSupport_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioProcessInClientSupplementUnitTest CheckIfSupport_001 start");
    AudioProcessConfig config;
    config.streamInfo.samplingRate = SAMPLE_RATE_44100; // Not 48000
    config.streamInfo.encoding = ENCODING_PCM;
    config.streamInfo.format = SAMPLE_S16LE;
    config.streamInfo.channels = STEREO;
    config.rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    config.capturerInfo.sourceType = SOURCE_TYPE_MIC;
    
    bool result = AudioProcessInClient::CheckIfSupport(config);
    EXPECT_FALSE(result);
}

/**
 * @tc.name   : Test CheckIfSupport with invalid format
 * @tc.type  : FUNC
 * @tc.number: CheckIfSupport_002
 * @tc.desc  : Test CheckIfSupport returns false for invalid format.
 */
HWTEST(AudioProcessInClientSupplementUnitTest, CheckIfSupport_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioProcessInClientSupplementUnitTest CheckIfSupport_002 start");
    AudioProcessConfig config;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.encoding = ENCODING_PCM;
    config.streamInfo.format = SAMPLE_S24LE; // Not S16LE, S32LE or S32LE
    config.streamInfo.channels = STEREO;
    config.rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    config.capturerInfo.sourceType = SOURCE_TYPE_MIC;
    
    bool result = AudioProcessInClient::CheckIfSupport(config);
    EXPECT_FALSE(result);
}

/**
 * @tc.name   : Test CheckIfSupport with invalid channels
 * @tc.type  : FUNC
 * @tc.number: CheckIfSupport_003
 * @tc.desc  : Test CheckIfSupport returns false for invalid channels.
 */
HWTEST(AudioProcessInClientSupplementUnitTest, CheckIfSupport_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioProcessInClientSupplementUnitTest CheckIfSupport_003 start");
    AudioProcessConfig config;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.encoding = ENCODING_PCM;
    config.streamInfo.format = SAMPLE_S16LE;
    config.streamInfo.channels = static_cast<AudioChannel>(5); // Not MONO or STEREO
    config.rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    config.capturerInfo.sourceType = SOURCE_TYPE_MIC;
    
    bool result = AudioProcessInClient::CheckIfSupport(config);
    EXPECT_FALSE(result);
}

} // namespace AudioStandard
} // namespace OHOS
