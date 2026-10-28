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

#include "audio_core_config_manager_unit_test.h"
#include "audio_info.h"
#include "audio_core_config_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioCoreConfigManagerUnitTest::SetUpTestCase(void) {}

void AudioCoreConfigManagerUnitTest::TearDownTestCase(void) {}

void AudioCoreConfigManagerUnitTest::SetUp(void) {}

void AudioCoreConfigManagerUnitTest::TearDown(void) {}

/**
 * @tc.name  : CheckAudioRendererStreamInfoSupport_001
 * @tc.desc  : PCM params with bitrate=-1 are supported.
 */
HWTEST_F(AudioCoreConfigManagerUnitTest, CheckAudioRendererStreamInfoSupport_001, TestSize.Level0)
{
    AudioStreamInfo streamInfo(SAMPLE_RATE_44100, ENCODING_PCM, SAMPLE_S16LE, STEREO);
    EXPECT_TRUE(AudioCoreConfigManager::GetInstance().CheckAudioRendererStreamInfoSupport(streamInfo));
}

/**
 * @tc.name  : CheckAudioRendererStreamInfoSupport_002
 * @tc.desc  : Returns false for illegal format.
 */
HWTEST_F(AudioCoreConfigManagerUnitTest, CheckAudioRendererStreamInfoSupport_002, TestSize.Level0)
{
    AudioStreamInfo streamInfo(SAMPLE_RATE_44100, ENCODING_PCM, AudioSampleFormat::INVALID_WIDTH, STEREO);
    EXPECT_FALSE(AudioCoreConfigManager::GetInstance().CheckAudioRendererStreamInfoSupport(streamInfo));
}

/**
 * @tc.name  : CheckAudioRendererStreamInfoSupport_003
 * @tc.desc  : HW-decoding with bitrate=-1 skips validation and is supported.
 */
HWTEST_F(AudioCoreConfigManagerUnitTest, CheckAudioRendererStreamInfoSupport_003, TestSize.Level0)
{
    AudioStreamInfo streamInfo(SAMPLE_RATE_48000, ENCODING_EAC3, SAMPLE_S16LE, STEREO);
    EXPECT_TRUE(AudioCoreConfigManager::GetInstance().CheckAudioRendererStreamInfoSupport(streamInfo));
}

/**
 * @tc.name  : CheckAudioRendererStreamInfoSupport_004
 * @tc.desc  : HW-decoding with bitrate below the configured max bitrate is supported.
 */
HWTEST_F(AudioCoreConfigManagerUnitTest, CheckAudioRendererStreamInfoSupport_004, TestSize.Level0)
{
    AudioStreamInfo streamInfo(SAMPLE_RATE_48000, ENCODING_EAC3, SAMPLE_S16LE, STEREO);
    streamInfo.bitrate = 500000;
    AudioCoreConfigManager::GetInstance().SetHwEncodingTypeMaxBitrate(1000000);
    EXPECT_TRUE(AudioCoreConfigManager::GetInstance().CheckAudioRendererStreamInfoSupport(streamInfo));
    AudioCoreConfigManager::GetInstance().SetHwEncodingTypeMaxBitrate(0);
}

/**
 * @tc.name  : CheckAudioRendererStreamInfoSupport_005
 * @tc.desc  : Bitrate is only validated for HW-decoding types; PCM with any bitrate is supported.
 */
HWTEST_F(AudioCoreConfigManagerUnitTest, CheckAudioRendererStreamInfoSupport_005, TestSize.Level0)
{
    AudioStreamInfo streamInfo(SAMPLE_RATE_44100, ENCODING_PCM, SAMPLE_S16LE, STEREO);
    streamInfo.bitrate = 500000;
    EXPECT_TRUE(AudioCoreConfigManager::GetInstance().CheckAudioRendererStreamInfoSupport(streamInfo));
}

/**
 * @tc.name  : CheckAudioRendererStreamInfoSupport_006
 * @tc.desc  : HW-decoding with bitrate between 80% and 100% of the configured max bitrate is supported.
 */
HWTEST_F(AudioCoreConfigManagerUnitTest, CheckAudioRendererStreamInfoSupport_006, TestSize.Level0)
{
    AudioStreamInfo streamInfo(SAMPLE_RATE_48000, ENCODING_EAC3, SAMPLE_S16LE, STEREO);
    streamInfo.bitrate = 900000;
    AudioCoreConfigManager::GetInstance().SetHwEncodingTypeMaxBitrate(1000000);
    EXPECT_TRUE(AudioCoreConfigManager::GetInstance().CheckAudioRendererStreamInfoSupport(streamInfo));
    AudioCoreConfigManager::GetInstance().SetHwEncodingTypeMaxBitrate(0);
}

/**
 * @tc.name  : CheckAudioRendererStreamInfoSupport_007
 * @tc.desc  : HW-decoding with bitrate exceeding the configured max bitrate is not supported.
 */
HWTEST_F(AudioCoreConfigManagerUnitTest, CheckAudioRendererStreamInfoSupport_007, TestSize.Level0)
{
    AudioStreamInfo streamInfo(SAMPLE_RATE_48000, ENCODING_EAC3, SAMPLE_S16LE, STEREO);
    streamInfo.bitrate = 1200000;
    AudioCoreConfigManager::GetInstance().SetHwEncodingTypeMaxBitrate(1000000);
    EXPECT_FALSE(AudioCoreConfigManager::GetInstance().CheckAudioRendererStreamInfoSupport(streamInfo));
    AudioCoreConfigManager::GetInstance().SetHwEncodingTypeMaxBitrate(0);
}

/**
 * @tc.name  : CheckAudioRendererStreamInfoSupport_008
 * @tc.desc  : Channel layout whose channel count does not match channelCount is not supported.
 */
HWTEST_F(AudioCoreConfigManagerUnitTest, CheckAudioRendererStreamInfoSupport_008, TestSize.Level0)
{
    AudioStreamInfo streamInfo(SAMPLE_RATE_44100, ENCODING_PCM, SAMPLE_S16LE, STEREO, CH_LAYOUT_MONO);
    EXPECT_FALSE(AudioCoreConfigManager::GetInstance().CheckAudioRendererStreamInfoSupport(streamInfo));
}

/**
 * @tc.name  : CheckAudioRendererStreamInfoSupport_009
 * @tc.desc  : Channel layout whose channel count matches channelCount is supported.
 */
HWTEST_F(AudioCoreConfigManagerUnitTest, CheckAudioRendererStreamInfoSupport_009, TestSize.Level0)
{
    AudioStreamInfo streamInfo(SAMPLE_RATE_44100, ENCODING_PCM, SAMPLE_S16LE, STEREO, CH_LAYOUT_STEREO);
    EXPECT_TRUE(AudioCoreConfigManager::GetInstance().CheckAudioRendererStreamInfoSupport(streamInfo));
}

/**
 * @tc.name  : CheckAudioRendererStreamInfoSupport_010
 * @tc.desc  : HW-decoding with bitrate=0 is not supported.
 */
HWTEST_F(AudioCoreConfigManagerUnitTest, CheckAudioRendererStreamInfoSupport_010, TestSize.Level0)
{
    AudioStreamInfo streamInfo(SAMPLE_RATE_48000, ENCODING_EAC3, SAMPLE_S16LE, STEREO);
    streamInfo.bitrate = 0;
    EXPECT_FALSE(AudioCoreConfigManager::GetInstance().CheckAudioRendererStreamInfoSupport(streamInfo));
}

/**
 * @tc.name  : CheckAudioRendererStreamInfoSupport_011
 * @tc.desc  : HW-decoding with a negative bitrate other than -1 is not supported.
 */
HWTEST_F(AudioCoreConfigManagerUnitTest, CheckAudioRendererStreamInfoSupport_011, TestSize.Level0)
{
    AudioStreamInfo streamInfo(SAMPLE_RATE_48000, ENCODING_EAC3, SAMPLE_S16LE, STEREO);
    streamInfo.bitrate = -2;
    EXPECT_FALSE(AudioCoreConfigManager::GetInstance().CheckAudioRendererStreamInfoSupport(streamInfo));
}

/**
 * @tc.name  : CheckAudioRendererStreamInfoSupport_012
 * @tc.desc  : Returns false for unsupported sampling rate.
 */
HWTEST_F(AudioCoreConfigManagerUnitTest, CheckAudioRendererStreamInfoSupport_012, TestSize.Level0)
{
    AudioStreamInfo streamInfo(static_cast<AudioSamplingRate>(777), ENCODING_PCM, SAMPLE_S16LE, STEREO);
    EXPECT_FALSE(AudioCoreConfigManager::GetInstance().CheckAudioRendererStreamInfoSupport(streamInfo));
}

/**
 * @tc.name  : CheckAudioRendererStreamInfoSupport_013
 * @tc.desc  : Returns false for unsupported channel count.
 */
HWTEST_F(AudioCoreConfigManagerUnitTest, CheckAudioRendererStreamInfoSupport_013, TestSize.Level0)
{
    AudioStreamInfo streamInfo(SAMPLE_RATE_44100, ENCODING_PCM, SAMPLE_S16LE, CHANNEL_UNKNOW);
    EXPECT_FALSE(AudioCoreConfigManager::GetInstance().CheckAudioRendererStreamInfoSupport(streamInfo));
}

/**
 * @tc.name  : CheckAudioRendererStreamInfoSupport_014
 * @tc.desc  : Returns false for unsupported encoding type.
 */
HWTEST_F(AudioCoreConfigManagerUnitTest, CheckAudioRendererStreamInfoSupport_014, TestSize.Level0)
{
    AudioStreamInfo streamInfo(SAMPLE_RATE_44100, ENCODING_INVALID, SAMPLE_S16LE, STEREO);
    EXPECT_FALSE(AudioCoreConfigManager::GetInstance().CheckAudioRendererStreamInfoSupport(streamInfo));
}

/**
 * @tc.name  : CheckAudioRendererStreamInfoSupport_015
 * @tc.desc  : HW-decoding with bitrate equal to 80% of the configured max bitrate is supported.
 */
HWTEST_F(AudioCoreConfigManagerUnitTest, CheckAudioRendererStreamInfoSupport_015, TestSize.Level0)
{
    AudioStreamInfo streamInfo(SAMPLE_RATE_48000, ENCODING_EAC3, SAMPLE_S16LE, STEREO);
    streamInfo.bitrate = 800000;
    AudioCoreConfigManager::GetInstance().SetHwEncodingTypeMaxBitrate(1000000);
    EXPECT_TRUE(AudioCoreConfigManager::GetInstance().CheckAudioRendererStreamInfoSupport(streamInfo));
    AudioCoreConfigManager::GetInstance().SetHwEncodingTypeMaxBitrate(0);
}

/**
 * @tc.name  : CheckAudioRendererStreamInfoSupport_016
 * @tc.desc  : HW-decoding with bitrate equal to the configured max bitrate is supported.
 */
HWTEST_F(AudioCoreConfigManagerUnitTest, CheckAudioRendererStreamInfoSupport_016, TestSize.Level0)
{
    AudioStreamInfo streamInfo(SAMPLE_RATE_48000, ENCODING_EAC3, SAMPLE_S16LE, STEREO);
    streamInfo.bitrate = 1000000;
    AudioCoreConfigManager::GetInstance().SetHwEncodingTypeMaxBitrate(1000000);
    EXPECT_TRUE(AudioCoreConfigManager::GetInstance().CheckAudioRendererStreamInfoSupport(streamInfo));
    AudioCoreConfigManager::GetInstance().SetHwEncodingTypeMaxBitrate(0);
}

} // namespace AudioStandard
} // namespace OHOS
