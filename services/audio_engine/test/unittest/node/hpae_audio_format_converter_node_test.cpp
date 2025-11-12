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
#include "hpae_audio_format_converter_node.h"
#include "test_case_common.h"
#include "hpae_node_common.h"
#include "audio_stream_info.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
using namespace testing::ext;
using namespace testing;

class HpaeAudioFormatConverterNodeTest : public testing::Test {
public:
    void SetUp();
    void TearDown();

    HpaeNodeInfo preNodeInfo_;
};

void HpaeAudioFormatConverterNodeTest::SetUp()
{}

void HpaeAudioFormatConverterNodeTest::TearDown()
{}

namespace {
const size_t DEFAULT_FRAMELEN_FIRST = 882;
const size_t DEFAULT_FRAMELEN_SECOND = 960;
const size_t DEFAULT_FRAMELEN_11025 = 441;
const size_t DEFAULT_FRAMELEN_48010 = 4801;
constexpr uint32_t SAMPLE_RATE_48010 = 48010;

/*
 * @tc.name  : Test CheckUpdateInInfo API.
 * @tc.type  : FUNC
 * @tc.number: CheckUpdateInInfoTest_001.
 * @tc.desc  : Test CheckUpdateInInfoInfo, when sampleRate = resampler_->GetInRate()
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, CheckUpdateInInfoTest_001, TestSize.Level0)
{
    HpaeNodeInfo preNodeInfo;
    preNodeInfo.samplingRate = SAMPLE_RATE_48000;
    preNodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    preNodeInfo.channels = STEREO;
    HpaeNodeInfo outputNodeInfo;
    auto converterNode = std::make_shared<HpaeAudioFormatConverterNode>(preNodeInfo, outputNodeInfo);
    EXPECT_EQ(converterNode->preNodeInfo_.samplingRate, SAMPLE_RATE_48000);
    EXPECT_EQ(converterNode->preNodeInfo_.frameLen, DEFAULT_FRAMELEN_SECOND);

    PcmBufferInfo pcmBufferInfo(STEREO, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_48000);
    HpaePcmBuffer input(pcmBufferInfo);

    EXPECT_FALSE(converterNode->CheckUpdateInInfo(&input));
}

/*
 * @tc.name  : Test CheckUpdateInInfo API.
 * @tc.type  : FUNC
 * @tc.number: CheckUpdateInInfoTest_002.
 * @tc.desc  : Test CheckUpdateInInfoInfo, when sampleRate != resampler_->GetInRate()
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, CheckUpdateInInfoTest_002, TestSize.Level0)
{
    HpaeNodeInfo preNodeInfo;
    preNodeInfo.samplingRate = SAMPLE_RATE_48000;
    preNodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    preNodeInfo.channels = STEREO;
    HpaeNodeInfo outputNodeInfo;
    auto converterNode = std::make_shared<HpaeAudioFormatConverterNode>(preNodeInfo, outputNodeInfo);
    EXPECT_EQ(converterNode->preNodeInfo_.samplingRate, SAMPLE_RATE_48000);
    EXPECT_EQ(converterNode->preNodeInfo_.frameLen, DEFAULT_FRAMELEN_SECOND);

    PcmBufferInfo pcmBufferInfo(STEREO, DEFAULT_FRAMELEN_FIRST, SAMPLE_RATE_44100);
    HpaePcmBuffer input(pcmBufferInfo);

    EXPECT_TRUE(converterNode->CheckUpdateInInfo(&input));
    EXPECT_EQ(converterNode->preNodeInfo_.samplingRate, SAMPLE_RATE_44100);
    EXPECT_EQ(converterNode->preNodeInfo_.frameLen, DEFAULT_FRAMELEN_FIRST);
}

/*
 * @tc.name  : Test CheckUpdateInInfo API.
 * @tc.type  : FUNC
 * @tc.number: CheckUpdateInInfoTest_003.
 * @tc.desc  : Test CheckUpdateInInfoInfo, when sampleRate != resampler_->GetInRate() and input frameLen is 0
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, CheckUpdateInInfoTest_003, TestSize.Level0)
{
    HpaeNodeInfo preNodeInfo;
    preNodeInfo.samplingRate = SAMPLE_RATE_48000;
    preNodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    preNodeInfo.channels = STEREO;
    HpaeNodeInfo outputNodeInfo;
    auto converterNode = std::make_shared<HpaeAudioFormatConverterNode>(preNodeInfo, outputNodeInfo);
    EXPECT_EQ(converterNode->preNodeInfo_.samplingRate, SAMPLE_RATE_48000);
    EXPECT_EQ(converterNode->preNodeInfo_.frameLen, DEFAULT_FRAMELEN_SECOND);

    // test 11025, 0 frameLen data
    PcmBufferInfo pcmBufferInfo(STEREO, 0, SAMPLE_RATE_11025);
    HpaePcmBuffer input(pcmBufferInfo);
    EXPECT_TRUE(converterNode->CheckUpdateInInfo(&input));
    EXPECT_EQ(converterNode->preNodeInfo_.samplingRate, SAMPLE_RATE_11025);
    EXPECT_EQ(converterNode->preNodeInfo_.frameLen, DEFAULT_FRAMELEN_11025);
    // test 10hz 100ms customSampleRate, 0 frameLen data
    PcmBufferInfo pcmBufferInfo1(STEREO, 0, SAMPLE_RATE_48010);
    HpaePcmBuffer input1(pcmBufferInfo1);
    EXPECT_TRUE(converterNode->CheckUpdateInInfo(&input1));
    EXPECT_EQ(converterNode->preNodeInfo_.samplingRate, SAMPLE_RATE_48010);
    EXPECT_EQ(converterNode->preNodeInfo_.frameLen, DEFAULT_FRAMELEN_48010);
}

/*
 * @tc.name  : Test UpdateTmpOutPcmBufferInfo API.
 * @tc.type  : FUNC
 * @tc.number: UpdateTmpOutPcmBufferInfoTest_001.
 * @tc.desc  : Test UpdateTmpOutPcmBufferInfo, when do not need tmpOutput Buffer, channels unchange, only resample
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, UpdateTmpOutPcmBufferInfoTest_001, TestSize.Level0)
{
    HpaeNodeInfo preNodeInfo;
    preNodeInfo.samplingRate = SAMPLE_RATE_44100;
    preNodeInfo.frameLen = DEFAULT_FRAMELEN_FIRST;
    preNodeInfo.channels = STEREO;
    HpaeNodeInfo outputNodeInfo;
    outputNodeInfo.samplingRate = SAMPLE_RATE_48000;
    outputNodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    outputNodeInfo.channels = STEREO;

    auto converterNode = std::make_shared<HpaeAudioFormatConverterNode>(preNodeInfo, outputNodeInfo);
    EXPECT_EQ(converterNode->tmpOutBuf_.GetSampleRate(), outputNodeInfo.samplingRate);
    EXPECT_EQ(converterNode->tmpOutBuf_.GetFrameLen(), outputNodeInfo.frameLen);
    EXPECT_EQ(converterNode->tmpOutBuf_.GetChannelCount(), outputNodeInfo.channels);

    PcmBufferInfo pcmBufferInfo(STEREO, DEFAULT_FRAMELEN_11025, SAMPLE_RATE_11025);
    HpaePcmBuffer input(pcmBufferInfo);
    converterNode->CheckAndUpdateInfo(&input);
    // tmpOutBuf_ unchanged and unused
    EXPECT_EQ(converterNode->tmpOutBuf_.GetSampleRate(), outputNodeInfo.samplingRate);
    EXPECT_EQ(converterNode->tmpOutBuf_.GetFrameLen(), outputNodeInfo.frameLen);
    EXPECT_EQ(converterNode->tmpOutBuf_.GetChannelCount(), outputNodeInfo.channels);
    // preNodeInfo_ changed
    EXPECT_EQ(converterNode->preNodeInfo_.samplingRate, SAMPLE_RATE_11025);
    EXPECT_EQ(converterNode->preNodeInfo_.frameLen, DEFAULT_FRAMELEN_11025);
}

/*
 * @tc.name  : Test UpdateTmpOutPcmBufferInfo API.
 * @tc.type  : FUNC
 * @tc.number: UpdateTmpOutPcmBufferInfoTest_002.
 * @tc.desc  : Test UpdateTmpOutPcmBufferInfo, when do not need tmpOutput Buffer, rate unchange, only channelConvert
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, UpdateTmpOutPcmBufferInfoTest_002, TestSize.Level0)
{
    HpaeNodeInfo preNodeInfo;
    preNodeInfo.samplingRate = SAMPLE_RATE_44100;
    preNodeInfo.frameLen = DEFAULT_FRAMELEN_FIRST;
    preNodeInfo.channels = STEREO;
    HpaeNodeInfo outputNodeInfo;
    outputNodeInfo.samplingRate = SAMPLE_RATE_48000;
    outputNodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    outputNodeInfo.channels = STEREO;

    auto converterNode = std::make_shared<HpaeAudioFormatConverterNode>(preNodeInfo, outputNodeInfo);
    EXPECT_EQ(converterNode->tmpOutBuf_.GetSampleRate(), outputNodeInfo.samplingRate);
    EXPECT_EQ(converterNode->tmpOutBuf_.GetFrameLen(), outputNodeInfo.frameLen);
    EXPECT_EQ(converterNode->tmpOutBuf_.GetChannelCount(), outputNodeInfo.channels);

    PcmBufferInfo pcmBufferInfo(CHANNEL_6, DEFAULT_FRAMELEN_FIRST, SAMPLE_RATE_48000);
    HpaePcmBuffer input(pcmBufferInfo);
    converterNode->CheckAndUpdateInfo(&input);
    // only downmix, tmpOutBuf_ unchanged and unused
    EXPECT_EQ(converterNode->tmpOutBuf_.GetSampleRate(), outputNodeInfo.samplingRate);
    EXPECT_EQ(converterNode->tmpOutBuf_.GetFrameLen(), outputNodeInfo.frameLen);
    EXPECT_EQ(converterNode->tmpOutBuf_.GetChannelCount(), outputNodeInfo.channels);
    
    PcmBufferInfo pcmBufferInfo1(MONO, DEFAULT_FRAMELEN_FIRST, SAMPLE_RATE_48000);
    HpaePcmBuffer input1(pcmBufferInfo1);
    converterNode->CheckAndUpdateInfo(&input1);
    // only upmix, tmpOutBuf_ unchanged and unused
    EXPECT_EQ(converterNode->tmpOutBuf_.GetSampleRate(), outputNodeInfo.samplingRate);
    EXPECT_EQ(converterNode->tmpOutBuf_.GetFrameLen(), outputNodeInfo.frameLen);
    EXPECT_EQ(converterNode->tmpOutBuf_.GetChannelCount(), outputNodeInfo.channels);
}

/*
 * @tc.name  : Test UpdateTmpOutPcmBufferInfo API.
 * @tc.type  : FUNC
 * @tc.number: UpdateTmpOutPcmBufferInfoTest_003.
 * @tc.desc  : Test UpdateTmpOutPcmBufferInfo, when need tmpOutput Buffer, rate and channel change
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, UpdateTmpOutPcmBufferInfoTest_003, TestSize.Level0)
{
    HpaeNodeInfo preNodeInfo;
    preNodeInfo.samplingRate = SAMPLE_RATE_44100;
    preNodeInfo.frameLen = DEFAULT_FRAMELEN_FIRST;
    preNodeInfo.channels = STEREO;
    HpaeNodeInfo outputNodeInfo;
    outputNodeInfo.samplingRate = SAMPLE_RATE_48000;
    outputNodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    outputNodeInfo.channels = STEREO;

    auto converterNode = std::make_shared<HpaeAudioFormatConverterNode>(preNodeInfo, outputNodeInfo);
    EXPECT_EQ(converterNode->tmpOutBuf_.GetSampleRate(), outputNodeInfo.samplingRate);
    EXPECT_EQ(converterNode->tmpOutBuf_.GetFrameLen(), outputNodeInfo.frameLen);
    EXPECT_EQ(converterNode->tmpOutBuf_.GetChannelCount(), outputNodeInfo.channels);

    // downmix, and then resample
    PcmBufferInfo pcmBufferInfo(CHANNEL_6, DEFAULT_FRAMELEN_11025, SAMPLE_RATE_11025);
    HpaePcmBuffer input(pcmBufferInfo);
    converterNode->CheckAndUpdateInfo(&input);
    // tmpOutBuf_ used for downmix output, changed
    EXPECT_EQ(converterNode->tmpOutBuf_.GetSampleRate(), input.GetSampleRate()); // downmix, sampleRate unchange
    EXPECT_EQ(converterNode->tmpOutBuf_.GetFrameLen(), input.GetFrameLen()); // downmix, frameLen unchange
    EXPECT_EQ(converterNode->tmpOutBuf_.GetChannelCount(), outputNodeInfo.channels); // downmix, channel change

    // resample, and then upmix
    PcmBufferInfo pcmBufferInfo1(MONO, DEFAULT_FRAMELEN_11025, SAMPLE_RATE_11025);
    HpaePcmBuffer input1(pcmBufferInfo1);
    converterNode->CheckAndUpdateInfo(&input1);
    // tmpOutBuf_ used for resample output, changed
    EXPECT_EQ(converterNode->tmpOutBuf_.GetSampleRate(), outputNodeInfo.samplingRate); // resample, sampleRate change
    EXPECT_EQ(converterNode->tmpOutBuf_.GetFrameLen(), outputNodeInfo.frameLen); // resample, frameLen change
    EXPECT_EQ(converterNode->tmpOutBuf_.GetChannelCount(), input1.GetChannelCount()); // resample, channel unchange
}

/*
 * @tc.name  : Test UpdateTmpOutPcmBufferInfo API.
 * @tc.type  : FUNC
 * @tc.number: UpdateTmpOutPcmBufferInfoTest_004.
 * @tc.desc  : Test UpdateTmpOutPcmBufferInfo, when need tmpOutput Buffer, rate and channel change, customSampleRate
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, UpdateTmpOutPcmBufferInfoTest_004, TestSize.Level0)
{
    HpaeNodeInfo preNodeInfo;
    preNodeInfo.samplingRate = SAMPLE_RATE_44100;
    preNodeInfo.frameLen = DEFAULT_FRAMELEN_FIRST;
    preNodeInfo.channels = STEREO;
    HpaeNodeInfo outputNodeInfo;
    outputNodeInfo.samplingRate = SAMPLE_RATE_48000;
    outputNodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    outputNodeInfo.channels = STEREO;

    auto converterNode = std::make_shared<HpaeAudioFormatConverterNode>(preNodeInfo, outputNodeInfo);
    EXPECT_EQ(converterNode->tmpOutBuf_.GetSampleRate(), outputNodeInfo.samplingRate);
    EXPECT_EQ(converterNode->tmpOutBuf_.GetFrameLen(), outputNodeInfo.frameLen);
    EXPECT_EQ(converterNode->tmpOutBuf_.GetChannelCount(), outputNodeInfo.channels);

    // downmix, and then resample
    PcmBufferInfo pcmBufferInfo(CHANNEL_6, DEFAULT_FRAMELEN_48010, SAMPLE_RATE_48010);
    HpaePcmBuffer input(pcmBufferInfo);
    converterNode->CheckAndUpdateInfo(&input);
    // tmpOutBuf_ used for downmix output, changed
    EXPECT_EQ(converterNode->tmpOutBuf_.GetSampleRate(), input.GetSampleRate()); // downmix, sampleRate unchange
    EXPECT_EQ(converterNode->tmpOutBuf_.GetFrameLen(), input.GetFrameLen()); // downmix, frameLen unchange
    EXPECT_EQ(converterNode->tmpOutBuf_.GetChannelCount(), outputNodeInfo.channels); // downmix, channel change

    // resample, and then upmix
    PcmBufferInfo pcmBufferInfo1(MONO, DEFAULT_FRAMELEN_48010, SAMPLE_RATE_48010);
    HpaePcmBuffer input1(pcmBufferInfo1);
    converterNode->CheckAndUpdateInfo(&input1);
    // tmpOutBuf_ used for resample output, changed
    EXPECT_EQ(converterNode->tmpOutBuf_.GetSampleRate(), outputNodeInfo.samplingRate); // resample, sampleRate change
    EXPECT_EQ(converterNode->tmpOutBuf_.GetFrameLen(), outputNodeInfo.frameLen); // resample, frameLen change
    EXPECT_EQ(converterNode->tmpOutBuf_.GetChannelCount(), input1.GetChannelCount()); // resample, channel unchange
}
}