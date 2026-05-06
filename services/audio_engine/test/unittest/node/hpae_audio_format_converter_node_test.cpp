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
#include "audio_proresampler.h"
#include "securec.h"

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
constexpr uint32_t SAMPLE_RATE_8010 = 8010;
constexpr uint32_t FRAME_LEN_20MS = 20;
constexpr uint32_t MILLISECOND_PER_SECOND = 1000;

// Helper: create a standard 48kHz stereo->stereo converter node
static std::shared_ptr<HpaeAudioFormatConverterNode> CreateStdConverter()
{
    HpaeNodeInfo pre;
    pre.samplingRate = SAMPLE_RATE_48000;
    pre.frameLen = DEFAULT_FRAMELEN_SECOND;
    pre.channels = STEREO;
    HpaeNodeInfo out;
    return std::make_shared<HpaeAudioFormatConverterNode>(pre, out);
}

// Helper: create a converter with specified in/out rates and channels
static std::shared_ptr<HpaeAudioFormatConverterNode> CreateConverter(
    uint32_t inRate, uint32_t outRate, uint32_t inCh, uint32_t outCh)
{
    HpaeNodeInfo pre;
    pre.samplingRate = static_cast<AudioSamplingRate>(inRate);
    pre.frameLen = inRate * FRAME_LEN_20MS / MILLISECOND_PER_SECOND;
    pre.channels = static_cast<AudioChannel>(inCh);
    HpaeNodeInfo out;
    out.samplingRate = static_cast<AudioSamplingRate>(outRate);
    out.frameLen = outRate * FRAME_LEN_20MS / MILLISECOND_PER_SECOND;
    out.channels = static_cast<AudioChannel>(outCh);
    return std::make_shared<HpaeAudioFormatConverterNode>(pre, out);
}

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

/*
 * @tc.name  : Test UpdateTmpOutPcmBufferInfo API.
 * @tc.type  : FUNC
 * @tc.number: UpdateTmpOutPcmBufferInfoTest_005.
 * @tc.desc  : Test UpdateTmpOutPcmBufferInfo, when need tmpOutput Buffer, rate and channel change, frameLen is 0
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, UpdateTmpOutPcmBufferInfoTest_005, TestSize.Level0)
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
    PcmBufferInfo pcmBufferInfo(CHANNEL_6, 0, SAMPLE_RATE_48010);
    HpaePcmBuffer input(pcmBufferInfo);
    converterNode->CheckAndUpdateInfo(&input);
    // tmpOutBuf_ used for downmix output, changed
    EXPECT_EQ(converterNode->tmpOutBuf_.GetSampleRate(), input.GetSampleRate()); // downmix, sampleRate unchange
    EXPECT_EQ(converterNode->tmpOutBuf_.GetFrameLen(), DEFAULT_FRAMELEN_48010); // downmix, frameLen unchange
    EXPECT_EQ(converterNode->tmpOutBuf_.GetChannelCount(), outputNodeInfo.channels); // downmix, channel change

    // resample, and then upmix
    PcmBufferInfo pcmBufferInfo1(MONO, 0, SAMPLE_RATE_48010);
    HpaePcmBuffer input1(pcmBufferInfo1);
    converterNode->CheckAndUpdateInfo(&input1);
    // tmpOutBuf_ used for resample output, changed
    EXPECT_EQ(converterNode->tmpOutBuf_.GetSampleRate(), outputNodeInfo.samplingRate); // resample, sampleRate change
    EXPECT_EQ(converterNode->tmpOutBuf_.GetFrameLen(), outputNodeInfo.frameLen); // resample, frameLen change
    EXPECT_EQ(converterNode->tmpOutBuf_.GetChannelCount(), input1.GetChannelCount()); // resample, channel unchange
}

/**
 * @tc.name  : FaultCode_SignalProcess_EmptyInputs
 * @tc.type  : FUNC
 * @tc.number: FaultCode_SignalProcess_EmptyInputs
 * @tc.desc  : Test SignalProcess with empty inputs vector,
 *             should return &silenceData_.
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, FaultCode_SignalProcess_EmptyInputs, TestSize.Level1)
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
    std::vector<HpaePcmBuffer *> inputs;
    HpaePcmBuffer *result = converterNode->SignalProcess(inputs);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : FaultCode_SignalProcess_NullFirstInput
 * @tc.type  : FUNC
 * @tc.number: FaultCode_SignalProcess_NullFirstInput
 * @tc.desc  : Test SignalProcess with nullptr as first input,
 *             should return &silenceData_.
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, FaultCode_SignalProcess_NullFirstInput, TestSize.Level1)
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
    std::vector<HpaePcmBuffer *> inputs = {nullptr};
    HpaePcmBuffer *result = converterNode->SignalProcess(inputs);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : FaultCode_SignalProcess_NullResampler
 * @tc.type  : FUNC
 * @tc.number: FaultCode_SignalProcess_NullResampler
 * @tc.desc  : Test SignalProcess when resampler_ is null,
 *             should report FAULT_STATE_INCONSISTENT/PLAY/QUERY/ERR_NULL_POINTER fault code.
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, FaultCode_SignalProcess_NullResampler, TestSize.Level1)
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
    // Force resampler_ to null to test the fault path
    converterNode->resampler_.reset();

    PcmBufferInfo pcmBufferInfo(STEREO, DEFAULT_FRAMELEN_FIRST, SAMPLE_RATE_44100);
    HpaePcmBuffer input(pcmBufferInfo);
    input.GetPcmDataBuffer();
    std::vector<HpaePcmBuffer *> inputs = {&input};
    HpaePcmBuffer *result = converterNode->SignalProcess(inputs);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : FaultCode_CheckUpdateOutInfo_SetOutChannelInfoFail
 * @tc.type  : FUNC
 * @tc.number: FaultCode_CheckUpdateOutInfo_SetOutChannelInfoFail
 * @tc.desc  : Test CheckUpdateOutInfo when SetOutChannelInfo fails,
 *             should report PLAY_SEND_STATE_ILLEGAL fault code.
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, FaultCode_CheckUpdateOutInfo_SetOutChannelInfoFail, TestSize.Level1)
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
    // nodeFormatInfoCallback_ is null by default, so CheckUpdateOutInfo returns false
    bool result = converterNode->CheckUpdateOutInfo();
    EXPECT_EQ(result, false);
}

// =========================================================================
// InitDualBuffer Tests
// =========================================================================

/**
 * @tc.name  : InitDualBuffer_NullResampler
 * @tc.type  : FUNC
 * @tc.number: InitDualBuffer_NullResampler
 * @tc.desc  : InitDualBuffer returns early when resampler_ is null
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, InitDualBuffer_NullResampler, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    converterNode->resampler_.reset();
    converterNode->unprocessedBuffer_.resize(100, 1.0f);
    converterNode->InitDualBuffer();
    // Should return early, buffer not re-initialized
    EXPECT_EQ(converterNode->resamplerInputThreshold_, 0u);
}

/**
 * @tc.name  : InitDualBuffer_SameRateSameChannel
 * @tc.type  : FUNC
 * @tc.number: InitDualBuffer_SameRateSameChannel
 * @tc.desc  : InitDualBuffer with 48kHz stereo->stereo, check thresholds and buffers
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, InitDualBuffer_SameRateSameChannel, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    // 48kHz stereo->stereo: threshold = 960, outputFrameLen20ms = 960
    EXPECT_EQ(converterNode->resamplerInputThreshold_, 960u);
    EXPECT_EQ(converterNode->outputFrameLen20ms_, 960u);
    EXPECT_GT(converterNode->unprocessedBuffer_.size(), 0u);
    EXPECT_GT(converterNode->processedBuffer_.size(), 0u);
    EXPECT_EQ(converterNode->unprocessedFrames_, 0u);
    EXPECT_EQ(converterNode->processedFrames_, 0u);
    EXPECT_EQ(converterNode->processedReadPos_, 0u);
    EXPECT_GT(converterNode->pullAheadMaxCredit_, 0u);
}

/**
 * @tc.name  : InitDualBuffer_11025HzResample
 * @tc.type  : FUNC
 * @tc.number: InitDualBuffer_11025HzResample
 * @tc.desc  : InitDualBuffer with 11025Hz->48kHz, check threshold (441) and output frame len
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, InitDualBuffer_11025HzResample, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_11025, SAMPLE_RATE_48000, STEREO, STEREO);
    EXPECT_EQ(converterNode->resamplerInputThreshold_, 441u);
    EXPECT_EQ(converterNode->outputFrameLen20ms_, 960u);
}

/**
 * @tc.name  : InitDualBuffer_ChannelConversionDownmix
 * @tc.type  : FUNC
 * @tc.number: InitDualBuffer_ChannelConversionDownmix
 * @tc.desc  : InitDualBuffer with 6ch->2ch channel conversion
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, InitDualBuffer_ChannelConversionDownmix, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, CHANNEL_6, STEREO);
    EXPECT_GT(converterNode->resamplerInputThreshold_, 0u);
    EXPECT_EQ(converterNode->outputFrameLen20ms_, 960u);
}

/**
 * @tc.name  : InitDualBuffer_ChannelConversionUpmix
 * @tc.type  : FUNC
 * @tc.number: InitDualBuffer_ChannelConversionUpmix
 * @tc.desc  : InitDualBuffer with mono->stereo upmix
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, InitDualBuffer_ChannelConversionUpmix, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_48000, SAMPLE_RATE_48000, MONO, STEREO);
    EXPECT_EQ(converterNode->resamplerInputThreshold_, 960u);
    EXPECT_EQ(converterNode->outputFrameLen20ms_, 960u);
}

// =========================================================================
// CalculateResamplerInputThreshold Tests
// =========================================================================

/**
 * @tc.name  : CalcThreshold_NullResampler
 * @tc.type  : FUNC
 * @tc.number: CalcThreshold_NullResampler
 * @tc.desc  : Returns 0 when resampler_ is null
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, CalcThreshold_NullResampler, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    converterNode->resampler_.reset();
    EXPECT_EQ(converterNode->CalculateResamplerInputThreshold(), 0u);
}

/**
 * @tc.name  : CalcThreshold_SameRate
 * @tc.type  : FUNC
 * @tc.number: CalcThreshold_SameRate
 * @tc.desc  : Returns 20ms threshold for same rate (48kHz->48kHz)
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, CalcThreshold_SameRate, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    EXPECT_EQ(converterNode->CalculateResamplerInputThreshold(), 960u);
}

/**
 * @tc.name  : CalcThreshold_11025Hz
 * @tc.type  : FUNC
 * @tc.number: CalcThreshold_11025Hz
 * @tc.desc  : Returns 40ms threshold for 11025Hz input
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, CalcThreshold_11025Hz, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_11025, SAMPLE_RATE_48000, STEREO, STEREO);
    EXPECT_EQ(converterNode->CalculateResamplerInputThreshold(), 441u);
}

/**
 * @tc.name  : CalcThreshold_NonStandardRate
 * @tc.type  : FUNC
 * @tc.number: CalcThreshold_NonStandardRate
 * @tc.desc  : Returns 100ms threshold for non-standard rate (8010Hz)
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, CalcThreshold_NonStandardRate, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_8010, SAMPLE_RATE_48000, STEREO, STEREO);
    EXPECT_EQ(converterNode->CalculateResamplerInputThreshold(), 801u);
}

/**
 * @tc.name  : CalcThreshold_StandardRate
 * @tc.type  : FUNC
 * @tc.number: CalcThreshold_StandardRate
 * @tc.desc  : Returns 20ms threshold for standard rate (44100Hz, multiple of 50)
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, CalcThreshold_StandardRate, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, STEREO, STEREO);
    EXPECT_EQ(converterNode->CalculateResamplerInputThreshold(), 882u);
}

// =========================================================================
// AppendToUnprocessedBuffer Tests
// =========================================================================

/**
 * @tc.name  : AppendUnprocessed_ZeroFrameLen
 * @tc.type  : FUNC
 * @tc.number: AppendUnprocessed_ZeroFrameLen
 * @tc.desc  : AppendToUnprocessedBuffer with zero frameLen returns early
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, AppendUnprocessed_ZeroFrameLen, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    float data[10] = {1.0f};
    uint32_t beforeFrames = converterNode->unprocessedFrames_;
    converterNode->AppendToUnprocessedBuffer(data, 0, STEREO);
    EXPECT_EQ(converterNode->unprocessedFrames_, beforeFrames);
}

/**
 * @tc.name  : AppendUnprocessed_NullData
 * @tc.type  : FUNC
 * @tc.number: AppendUnprocessed_NullData
 * @tc.desc  : AppendToUnprocessedBuffer with null data returns early
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, AppendUnprocessed_NullData, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    uint32_t beforeFrames = converterNode->unprocessedFrames_;
    converterNode->AppendToUnprocessedBuffer(nullptr, 960, STEREO);
    EXPECT_EQ(converterNode->unprocessedFrames_, beforeFrames);
}

/**
 * @tc.name  : AppendUnprocessed_NormalAppend
 * @tc.type  : FUNC
 * @tc.number: AppendUnprocessed_NormalAppend
 * @tc.desc  : Normal append increments unprocessedFrames_ and copies data
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, AppendUnprocessed_NormalAppend, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    std::vector<float> data(960 * STEREO, 0.5f);
    converterNode->AppendToUnprocessedBuffer(data.data(), 960, STEREO);
    EXPECT_EQ(converterNode->unprocessedFrames_, 960u);
    EXPECT_FLOAT_EQ(converterNode->unprocessedBuffer_[0], 0.5f);
}

/**
 * @tc.name  : AppendUnprocessed_RequiresResize
 * @tc.type  : FUNC
 * @tc.number: AppendUnprocessed_RequiresResize
 * @tc.desc  : Append larger than current buffer capacity triggers resize
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, AppendUnprocessed_RequiresResize, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    size_t originalSize = converterNode->unprocessedBuffer_.size();
    // Create data larger than original capacity
    uint32_t largeFrameLen = static_cast<uint32_t>(originalSize / STEREO + 100);
    std::vector<float> data(largeFrameLen * STEREO, 0.25f);
    converterNode->AppendToUnprocessedBuffer(data.data(), largeFrameLen, STEREO);
    EXPECT_EQ(converterNode->unprocessedFrames_, largeFrameLen);
    EXPECT_GE(converterNode->unprocessedBuffer_.size(), static_cast<size_t>(largeFrameLen) * STEREO);
}

/**
 * @tc.name  : AppendUnprocessed_CreditDecrement
 * @tc.type  : FUNC
 * @tc.number: AppendUnprocessed_CreditDecrement
 * @tc.desc  : pullAheadCredit_ is decremented when > 0 after append
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, AppendUnprocessed_CreditDecrement, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    uint32_t creditBefore = converterNode->pullAheadCredit_;
    ASSERT_GT(creditBefore, 0u);
    std::vector<float> data(960 * STEREO, 0.5f);
    converterNode->AppendToUnprocessedBuffer(data.data(), 960, STEREO);
    EXPECT_EQ(converterNode->pullAheadCredit_, creditBefore - 1);
}

// =========================================================================
// AppendToProcessedBuffer Tests
// =========================================================================

/**
 * @tc.name  : AppendProcessed_NormalAppend
 * @tc.type  : FUNC
 * @tc.number: AppendProcessed_NormalAppend
 * @tc.desc  : Normal append to processedBuffer_ increments processedFrames_
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, AppendProcessed_NormalAppend, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    std::vector<float> data(960 * STEREO, 0.3f);
    converterNode->AppendToProcessedBuffer(data.data(), 960, STEREO);
    EXPECT_EQ(converterNode->processedFrames_, 960u);
    EXPECT_FLOAT_EQ(converterNode->processedBuffer_[0], 0.3f);
}

/**
 * @tc.name  : AppendProcessed_RequiresResize
 * @tc.type  : FUNC
 * @tc.number: AppendProcessed_RequiresResize
 * @tc.desc  : Append triggers resize when processedBuffer_ too small
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, AppendProcessed_RequiresResize, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    size_t originalSize = converterNode->processedBuffer_.size();
    uint32_t largeFrameLen = static_cast<uint32_t>(originalSize / STEREO + 100);
    std::vector<float> data(largeFrameLen * STEREO, 0.4f);
    converterNode->AppendToProcessedBuffer(data.data(), largeFrameLen, STEREO);
    EXPECT_EQ(converterNode->processedFrames_, largeFrameLen);
    EXPECT_GE(converterNode->processedBuffer_.size(), static_cast<size_t>(largeFrameLen) * STEREO);
}

// =========================================================================
// ExtractOutputFromProcessedBuffer Tests
// =========================================================================

/**
 * @tc.name  : ExtractOutput_InsufficientData
 * @tc.type  : FUNC
 * @tc.number: ExtractOutput_InsufficientData
 * @tc.desc  : Returns false when processedFrames_ < outputFrameLen20ms_
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, ExtractOutput_InsufficientData, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    converterNode->processedFrames_ = 100;
    converterNode->outputFrameLen20ms_ = 960;
    EXPECT_FALSE(converterNode->ExtractOutputFromProcessedBuffer());
}

/**
 * @tc.name  : ExtractOutput_SuccessExactMatch
 * @tc.type  : FUNC
 * @tc.number: ExtractOutput_SuccessExactMatch
 * @tc.desc  : Successful extraction with exact match, resets readPos
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, ExtractOutput_SuccessExactMatch, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    // Set up processedBuffer_ with exactly outputFrameLen20ms_ frames
    uint32_t frameLen = converterNode->outputFrameLen20ms_;
    uint32_t channels = STEREO;
    std::vector<float> testData(frameLen * channels, 0.7f);
    converterNode->processedBuffer_ = testData;
    converterNode->processedFrames_ = frameLen;
    converterNode->processedReadPos_ = 0;
    uint32_t creditBefore = converterNode->pullAheadCredit_;
    converterNode->pullAheadMaxCredit_ = creditBefore + 1;

    EXPECT_TRUE(converterNode->ExtractOutputFromProcessedBuffer());
    EXPECT_EQ(converterNode->processedFrames_, 0u);
    EXPECT_EQ(converterNode->processedReadPos_, 0u);
}

/**
 * @tc.name  : ExtractOutput_RemainingDataNoCompaction
 * @tc.type  : FUNC
 * @tc.number: ExtractOutput_RemainingDataNoCompaction
 * @tc.desc  : Extraction leaves remaining data, no compaction when readPos small
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, ExtractOutput_RemainingDataNoCompaction, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    uint32_t frameLen = converterNode->outputFrameLen20ms_;
    uint32_t channels = STEREO;
    // Put 2x the required frames
    std::vector<float> testData(frameLen * 2 * channels, 0.8f);
    converterNode->processedBuffer_ = testData;
    converterNode->processedFrames_ = frameLen * 2;
    converterNode->processedReadPos_ = 0;

    EXPECT_TRUE(converterNode->ExtractOutputFromProcessedBuffer());
    EXPECT_EQ(converterNode->processedFrames_, frameLen);
    EXPECT_EQ(converterNode->processedReadPos_, frameLen);
}

/**
 * @tc.name  : ExtractOutput_TriggersCompaction
 * @tc.type  : FUNC
 * @tc.number: ExtractOutput_TriggersCompaction
 * @tc.desc  : Extraction triggers compaction when readPos exceeds half buffer
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, ExtractOutput_TriggersCompaction, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    uint32_t frameLen = converterNode->outputFrameLen20ms_;
    uint32_t channels = STEREO;
    // Create large buffer, set readPos past midpoint
    size_t bufSize = frameLen * 4 * channels;
    std::vector<float> testData(bufSize, 0.9f);
    converterNode->processedBuffer_ = testData;
    converterNode->processedFrames_ = frameLen * 2;
    // Set readPos high enough to trigger compaction
    converterNode->processedReadPos_ = frameLen * 3;

    EXPECT_TRUE(converterNode->ExtractOutputFromProcessedBuffer());
}

/**
 * @tc.name  : ExtractOutput_CreditAtMax
 * @tc.type  : FUNC
 * @tc.number: ExtractOutput_CreditAtMax
 * @tc.desc  : pullAheadCredit_ not incremented when already at max
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, ExtractOutput_CreditAtMax, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    uint32_t frameLen = converterNode->outputFrameLen20ms_;
    std::vector<float> testData(frameLen * STEREO, 1.0f);
    converterNode->processedBuffer_ = testData;
    converterNode->processedFrames_ = frameLen;
    converterNode->processedReadPos_ = 0;
    // Set credit at max
    converterNode->pullAheadCredit_ = 5;
    converterNode->pullAheadMaxCredit_ = 5;

    EXPECT_TRUE(converterNode->ExtractOutputFromProcessedBuffer());
    EXPECT_EQ(converterNode->pullAheadCredit_, 5u);
}

/**
 * @tc.name  : ExtractOutput_CreditIncrement
 * @tc.type  : FUNC
 * @tc.number: ExtractOutput_CreditIncrement
 * @tc.desc  : pullAheadCredit_ incremented when below max after extraction
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, ExtractOutput_CreditIncrement, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    uint32_t frameLen = converterNode->outputFrameLen20ms_;
    std::vector<float> testData(frameLen * STEREO, 1.0f);
    converterNode->processedBuffer_ = testData;
    converterNode->processedFrames_ = frameLen;
    converterNode->processedReadPos_ = 0;
    converterNode->pullAheadCredit_ = 2;
    converterNode->pullAheadMaxCredit_ = 5;

    EXPECT_TRUE(converterNode->ExtractOutputFromProcessedBuffer());
    EXPECT_EQ(converterNode->pullAheadCredit_, 3u);
}

// =========================================================================
// RemoveConsumedInputFrames Tests
// =========================================================================

/**
 * @tc.name  : RemoveConsumed_AllConsumed
 * @tc.type  : FUNC
 * @tc.number: RemoveConsumed_AllConsumed
 * @tc.desc  : When all frames consumed, unprocessedFrames_ resets to 0
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, RemoveConsumed_AllConsumed, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    converterNode->unprocessedFrames_ = 960;
    converterNode->RemoveConsumedInputFrames(960, STEREO);
    EXPECT_EQ(converterNode->unprocessedFrames_, 0u);
}

/**
 * @tc.name  : RemoveConsumed_PartialConsumption
 * @tc.type  : FUNC
 * @tc.number: RemoveConsumed_PartialConsumption
 * @tc.desc  : Partial consumption moves remaining data to front
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, RemoveConsumed_PartialConsumption, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    uint32_t totalFrames = 960;
    uint32_t channels = STEREO;
    std::vector<float> data(totalFrames * channels, 0.5f);
    converterNode->unprocessedBuffer_ = data;
    converterNode->unprocessedFrames_ = totalFrames;

    converterNode->RemoveConsumedInputFrames(480, channels);
    EXPECT_EQ(converterNode->unprocessedFrames_, 480u);
}

// =========================================================================
// CompactProcessedBuffer Tests
// =========================================================================

/**
 * @tc.name  : CompactProcessed_NormalCompaction
 * @tc.type  : FUNC
 * @tc.number: CompactProcessed_NormalCompaction
 * @tc.desc  : Compaction moves data to front and resets readPos
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, CompactProcessed_NormalCompaction, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    uint32_t channels = STEREO;
    uint32_t remaining = 480;
    converterNode->processedFrames_ = remaining;
    converterNode->processedReadPos_ = 960;
    std::vector<float> data((960 + remaining) * channels, 0.6f);
    converterNode->processedBuffer_ = data;

    converterNode->CompactProcessedBuffer(channels);
    EXPECT_EQ(converterNode->processedReadPos_, 0u);
}

// =========================================================================
// MoveUnprocessedToProcessed Tests
// =========================================================================

/**
 * @tc.name  : MoveUnprocessed_NormalMove
 * @tc.type  : FUNC
 * @tc.number: MoveUnprocessed_NormalMove
 * @tc.desc  : Moves data from unprocessed to processed buffer
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, MoveUnprocessed_NormalMove, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    uint32_t frames = 480;
    uint32_t channels = STEREO;
    std::vector<float> data(frames * channels, 0.7f);
    converterNode->unprocessedBuffer_ = data;
    converterNode->unprocessedFrames_ = frames;

    converterNode->MoveUnprocessedToProcessed(channels);
    EXPECT_EQ(converterNode->unprocessedFrames_, 0u);
    EXPECT_EQ(converterNode->processedFrames_, frames);
}

/**
 * @tc.name  : MoveUnprocessed_BufferNeedsResize
 * @tc.type  : FUNC
 * @tc.number: MoveUnprocessed_BufferNeedsResize
 * @tc.desc  : Move triggers resize when processedBuffer_ too small
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, MoveUnprocessed_BufferNeedsResize, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    uint32_t frames = 2000;
    uint32_t channels = STEREO;
    std::vector<float> data(frames * channels, 0.8f);
    converterNode->unprocessedBuffer_ = data;
    converterNode->unprocessedFrames_ = frames;
    converterNode->processedBuffer_.clear();

    converterNode->MoveUnprocessedToProcessed(channels);
    EXPECT_EQ(converterNode->unprocessedFrames_, 0u);
    EXPECT_EQ(converterNode->processedFrames_, frames);
    EXPECT_GE(converterNode->processedBuffer_.size(), static_cast<size_t>(frames) * channels);
}

// =========================================================================
// ProcessResampleLoop Tests
// =========================================================================

/**
 * @tc.name  : ProcessResampleLoop_NullResampler
 * @tc.type  : FUNC
 * @tc.number: ProcessResampleLoop_NullResampler
 * @tc.desc  : Returns immediately when resampler_ is null
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, ProcessResampleLoop_NullResampler, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    converterNode->resampler_.reset();
    converterNode->unprocessedFrames_ = 960;
    converterNode->ProcessResampleLoop();
    // Should not crash, processedFrames unchanged
    EXPECT_EQ(converterNode->processedFrames_, 0u);
}

/**
 * @tc.name  : ProcessResampleLoop_SameRateSameChannel
 * @tc.type  : FUNC
 * @tc.number: ProcessResampleLoop_SameRateSameChannel
 * @tc.desc  : Same rate same channel uses passthrough (MoveUnprocessedToProcessed)
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, ProcessResampleLoop_SameRateSameChannel, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    uint32_t frames = 960;
    std::vector<float> data(frames * STEREO, 0.5f);
    converterNode->unprocessedBuffer_ = data;
    converterNode->unprocessedFrames_ = frames;

    converterNode->ProcessResampleLoop();
    EXPECT_EQ(converterNode->unprocessedFrames_, 0u);
    EXPECT_EQ(converterNode->processedFrames_, frames);
}

/**
 * @tc.name  : ProcessResampleLoop_SameRateDiffChannel
 * @tc.type  : FUNC
 * @tc.number: ProcessResampleLoop_SameRateDiffChannel
 * @tc.desc  : Same rate different channels calls ProcessSameRateChannelConversion
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, ProcessResampleLoop_SameRateDiffChannel, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_48000, SAMPLE_RATE_48000, CHANNEL_6, STEREO);
    uint32_t frames = converterNode->resamplerInputThreshold_;
    ASSERT_GT(frames, 0u);
    std::vector<float> data(frames * CHANNEL_6, 0.5f);
    converterNode->unprocessedBuffer_ = data;
    converterNode->unprocessedFrames_ = frames;

    converterNode->ProcessResampleLoop();
    EXPECT_EQ(converterNode->unprocessedFrames_, 0u);
    EXPECT_GT(converterNode->processedFrames_, 0u);
}

/**
 * @tc.name  : ProcessResampleLoop_DiffRateSameChannel
 * @tc.type  : FUNC
 * @tc.number: ProcessResampleLoop_DiffRateSameChannel
 * @tc.desc  : Different rate same channel calls ProcessDiffRateResample
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, ProcessResampleLoop_DiffRateSameChannel, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, STEREO, STEREO);
    uint32_t frames = converterNode->resamplerInputThreshold_;
    ASSERT_GT(frames, 0u);
    std::vector<float> data(frames * STEREO, 0.5f);
    converterNode->unprocessedBuffer_ = data;
    converterNode->unprocessedFrames_ = frames;

    converterNode->ProcessResampleLoop();
    EXPECT_EQ(converterNode->unprocessedFrames_, 0u);
    EXPECT_GT(converterNode->processedFrames_, 0u);
}

/**
 * @tc.name  : ProcessResampleLoop_DiffRateDiffChannel
 * @tc.type  : FUNC
 * @tc.number: ProcessResampleLoop_DiffRateDiffChannel
 * @tc.desc  : Different rate and different channel calls ProcessDiffRateResample
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, ProcessResampleLoop_DiffRateDiffChannel, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, CHANNEL_6, STEREO);
    uint32_t frames = converterNode->resamplerInputThreshold_;
    ASSERT_GT(frames, 0u);
    std::vector<float> data(frames * CHANNEL_6, 0.5f);
    converterNode->unprocessedBuffer_ = data;
    converterNode->unprocessedFrames_ = frames;

    converterNode->ProcessResampleLoop();
    EXPECT_EQ(converterNode->unprocessedFrames_, 0u);
    EXPECT_GT(converterNode->processedFrames_, 0u);
}

/**
 * @tc.name  : ProcessResampleLoop_SameRateZeroFrames
 * @tc.type  : FUNC
 * @tc.number: ProcessResampleLoop_SameRateZeroFrames
 * @tc.desc  : Same rate same channel with zero unprocessed frames
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, ProcessResampleLoop_SameRateZeroFrames, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    converterNode->unprocessedFrames_ = 0;

    converterNode->ProcessResampleLoop();
    EXPECT_EQ(converterNode->unprocessedFrames_, 0u);
    EXPECT_EQ(converterNode->processedFrames_, 0u);
}

// =========================================================================
// ResampleWithChannelConversion Tests (5 paths)
// =========================================================================

/**
 * @tc.name  : RWCC_Passthrough
 * @tc.type  : FUNC
 * @tc.number: RWCC_Passthrough
 * @tc.desc  : Same rate same channel - passthrough (memcpy)
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, RWCC_Passthrough, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    uint32_t frameLen = 960;
    uint32_t channels = STEREO;
    std::vector<float> srcData(frameLen * channels, 0.5f);
    std::vector<float> dstData(frameLen * channels, 0.0f);

    int32_t ret = converterNode->ResampleWithChannelConversion(
        srcData.data(), frameLen, dstData.data(), frameLen, channels);
    EXPECT_EQ(ret, 0);
    EXPECT_FLOAT_EQ(dstData[0], 0.5f);
}

/**
 * @tc.name  : RWCC_ResampleOnly
 * @tc.type  : FUNC
 * @tc.number: RWCC_ResampleOnly
 * @tc.desc  : Different rate same channel - resample only
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, RWCC_ResampleOnly, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, STEREO, STEREO);
    uint32_t inFrameLen = 882; // 44100 * 20ms
    uint32_t outFrameLen = 960; // 48000 * 20ms
    std::vector<float> srcData(inFrameLen * STEREO, 0.5f);
    std::vector<float> dstData(outFrameLen * STEREO, 0.0f);

    int32_t ret = converterNode->ResampleWithChannelConversion(
        srcData.data(), inFrameLen, dstData.data(), outFrameLen, STEREO);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : RWCC_ChannelConvertOnly
 * @tc.type  : FUNC
 * @tc.number: RWCC_ChannelConvertOnly
 * @tc.desc  : Same rate different channel - channel conversion only
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, RWCC_ChannelConvertOnly, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_48000, SAMPLE_RATE_48000, CHANNEL_6, STEREO);
    uint32_t frameLen = 960;
    std::vector<float> srcData(frameLen * CHANNEL_6, 0.5f);
    std::vector<float> dstData(frameLen * STEREO, 0.0f);

    int32_t ret = converterNode->ResampleWithChannelConversion(
        srcData.data(), frameLen, dstData.data(), frameLen, STEREO);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : RWCC_DownmixThenResample
 * @tc.type  : FUNC
 * @tc.number: RWCC_DownmixThenResample
 * @tc.desc  : Downmix (6ch->2ch) then resample (44100->48000)
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, RWCC_DownmixThenResample, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, CHANNEL_6, STEREO);
    uint32_t inFrameLen = 882;
    uint32_t outFrameLen = 960;
    std::vector<float> srcData(inFrameLen * CHANNEL_6, 0.5f);
    std::vector<float> dstData(outFrameLen * STEREO, 0.0f);

    int32_t ret = converterNode->ResampleWithChannelConversion(
        srcData.data(), inFrameLen, dstData.data(), outFrameLen, STEREO);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : RWCC_ResampleThenUpmix
 * @tc.type  : FUNC
 * @tc.number: RWCC_ResampleThenUpmix
 * @tc.desc  : Resample (44100->48000) then upmix (mono->stereo)
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, RWCC_ResampleThenUpmix, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, MONO, STEREO);
    uint32_t inFrameLen = 882;
    uint32_t outFrameLen = 960;
    std::vector<float> srcData(inFrameLen * MONO, 0.5f);
    std::vector<float> dstData(outFrameLen * STEREO, 0.0f);

    int32_t ret = converterNode->ResampleWithChannelConversion(
        srcData.data(), inFrameLen, dstData.data(), outFrameLen, MONO);
    EXPECT_EQ(ret, 0);
}

// =========================================================================
// ProcessSameRateChannelConversion Tests
// =========================================================================

/**
 * @tc.name  : ProcessSameRate_SuccessfulConversion
 * @tc.type  : FUNC
 * @tc.number: ProcessSameRate_SuccessfulConversion
 * @tc.desc  : Successful channel conversion loop with multiple threshold batches
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, ProcessSameRate_SuccessfulConversion, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_48000, SAMPLE_RATE_48000, CHANNEL_6, STEREO);
    uint32_t threshold = converterNode->resamplerInputThreshold_;
    ASSERT_GT(threshold, 0u);
    // Put 2x threshold worth of data
    std::vector<float> data(threshold * 2 * CHANNEL_6, 0.5f);
    converterNode->unprocessedBuffer_ = data;
    converterNode->unprocessedFrames_ = threshold * 2;

    converterNode->ProcessSameRateChannelConversion(CHANNEL_6, STEREO);
    EXPECT_EQ(converterNode->unprocessedFrames_, 0u);
    EXPECT_GT(converterNode->processedFrames_, 0u);
}

// =========================================================================
// ProcessDiffRateResample Tests
// =========================================================================

/**
 * @tc.name  : ProcessDiffRate_SuccessfulResample
 * @tc.type  : FUNC
 * @tc.number: ProcessDiffRate_SuccessfulResample
 * @tc.desc  : Successful resample loop from 44100 to 48000
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, ProcessDiffRate_SuccessfulResample, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, STEREO, STEREO);
    uint32_t threshold = converterNode->resamplerInputThreshold_;
    ASSERT_GT(threshold, 0u);
    std::vector<float> data(threshold * STEREO, 0.5f);
    converterNode->unprocessedBuffer_ = data;
    converterNode->unprocessedFrames_ = threshold;

    converterNode->ProcessDiffRateResample(STEREO, STEREO,
        SAMPLE_RATE_44100, SAMPLE_RATE_48000);
    EXPECT_EQ(converterNode->unprocessedFrames_, 0u);
    EXPECT_GT(converterNode->processedFrames_, 0u);
}

/**
 * @tc.name  : ProcessDiffRate_ZeroInRate
 * @tc.type  : FUNC
 * @tc.number: ProcessDiffRate_ZeroInRate
 * @tc.desc  : ProcessDiffRateResample returns early when inRate is 0 (divide-by-zero guard)
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, ProcessDiffRate_ZeroInRate, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, STEREO, STEREO);
    converterNode->unprocessedFrames_ = 882;

    // Call with inRate=0, should return early without crash
    converterNode->ProcessDiffRateResample(STEREO, STEREO, 0, SAMPLE_RATE_48000);
    // unprocessedFrames_ unchanged (no processing happened)
    EXPECT_EQ(converterNode->unprocessedFrames_, 882u);
}

// =========================================================================
// ReconfigTmpOutForChannelConversion Tests
// =========================================================================

/**
 * @tc.name  : ReconfigTmpOut_DownmixPath
 * @tc.type  : FUNC
 * @tc.number: ReconfigTmpOut_DownmixPath
 * @tc.desc  : Reconfig for downmix then resample (inCh > outCh)
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, ReconfigTmpOut_DownmixPath, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, CHANNEL_6, STEREO);
    uint32_t threshold = converterNode->resamplerInputThreshold_;
    ASSERT_GT(threshold, 0u);
    // tmpOutBuf_ should have been configured during construction
    EXPECT_GT(converterNode->tmpOutBuf_.GetFrameLen(), 0u);
}

/**
 * @tc.name  : ReconfigTmpOut_UpmixPath
 * @tc.type  : FUNC
 * @tc.number: ReconfigTmpOut_UpmixPath
 * @tc.desc  : Reconfig for resample then upmix (inCh < outCh)
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, ReconfigTmpOut_UpmixPath, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, MONO, STEREO);
    // tmpOutBuf_ should have been configured during construction
    EXPECT_GT(converterNode->tmpOutBuf_.GetFrameLen(), 0u);
}

// =========================================================================
// DoProcess Tests
// =========================================================================

/**
 * @tc.name  : DoProcess_SilenceWhenNoInput
 * @tc.type  : FUNC
 * @tc.number: DoProcess_SilenceWhenNoInput
 * @tc.desc  : DoProcess outputs silence when no valid input (no upstream connected)
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, DoProcess_SilenceWhenNoInput, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    // No upstream connected, so hasValidInput will be false
    converterNode->DoProcess();
    // Should not crash
}

/**
 * @tc.name  : DoProcess_DrainPath
 * @tc.type  : FUNC
 * @tc.number: DoProcess_DrainPath
 * @tc.desc  : DoProcess drains processedBuffer_ when no input but has buffered data
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, DoProcess_DrainPath, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    // Populate processedBuffer_ with enough data
    uint32_t frameLen = converterNode->outputFrameLen20ms_;
    std::vector<float> testData(frameLen * STEREO, 0.5f);
    converterNode->processedBuffer_ = testData;
    converterNode->processedFrames_ = frameLen;
    converterNode->processedReadPos_ = 0;

    converterNode->DoProcess();
    // After drain, processedFrames should be 0 (exact match)
    EXPECT_EQ(converterNode->processedFrames_, 0u);
}

/**
 * @tc.name  : DoProcess_ProcessDisabled
 * @tc.type  : FUNC
 * @tc.number: DoProcess_ProcessDisabled
 * @tc.desc  : DoProcess outputs silence when enableProcess_ is false
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, DoProcess_ProcessDisabled, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    converterNode->enableProcess_ = false;
    // Populate processedBuffer_ - but drain path also checks enableProcess_
    uint32_t frameLen = converterNode->outputFrameLen20ms_;
    std::vector<float> testData(frameLen * STEREO, 0.5f);
    converterNode->processedBuffer_ = testData;
    converterNode->processedFrames_ = frameLen;

    converterNode->DoProcess();
    // processedFrames_ unchanged because enableProcess_ is false, drain skipped
    EXPECT_EQ(converterNode->processedFrames_, frameLen);
}

/**
 * @tc.name  : DoProcess_DrainInsufficientData
 * @tc.type  : FUNC
 * @tc.number: DoProcess_DrainInsufficientData
 * @tc.desc  : DoProcess with insufficient processed data falls through to silence
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, DoProcess_DrainInsufficientData, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    converterNode->processedFrames_ = 100; // Less than outputFrameLen20ms_

    converterNode->DoProcess();
    // Should not crash, outputs silence
}

// =========================================================================
// UpdateOutChannelInfo Tests
// =========================================================================

/**
 * @tc.name  : UpdateOutChannelInfo_NeedsResamplerUpdate
 * @tc.type  : FUNC
 * @tc.number: UpdateOutChannelInfo_NeedsResamplerUpdate
 * @tc.desc  : UpdateOutChannelInfo updates resampler channels when they differ
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, UpdateOutChannelInfo_NeedsResamplerUpdate, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_48000, SAMPLE_RATE_48000, CHANNEL_6, STEREO);
    // Change out channel info to mono
    bool result = converterNode->UpdateOutChannelInfo(MONO, CH_LAYOUT_MONO);
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : UpdateOutChannelInfo_SameChannels
 * @tc.type  : FUNC
 * @tc.number: UpdateOutChannelInfo_SameChannels
 * @tc.desc  : UpdateOutChannelInfo succeeds without resampler update when channels same
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, UpdateOutChannelInfo_SameChannels, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    bool result = converterNode->UpdateOutChannelInfo(STEREO, CH_LAYOUT_STEREO);
    EXPECT_TRUE(result);
}

// =========================================================================
// AccumulateInputAndPullAhead Tests
// =========================================================================

/**
 * @tc.name  : Accumulate_NormalInput
 * @tc.type  : FUNC
 * @tc.number: Accumulate_NormalInput
 * @tc.desc  : AccumulateInputAndPullAhead appends data to unprocessedBuffer_
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, Accumulate_NormalInput, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    PcmBufferInfo pcmInfo(STEREO, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_48000);
    HpaePcmBuffer input(pcmInfo);
    input.GetPcmDataBuffer(); // Ensure buffer is allocated
    float *buf = input.GetPcmDataBuffer();
    for (size_t i = 0; i < DEFAULT_FRAMELEN_SECOND * STEREO; i++) {
        buf[i] = 0.5f;
    }

    converterNode->AccumulateInputAndPullAhead(&input);
    EXPECT_EQ(converterNode->unprocessedFrames_, DEFAULT_FRAMELEN_SECOND);
}

/**
 * @tc.name  : Accumulate_PullAheadNeeded
 * @tc.type  : FUNC
 * @tc.number: Accumulate_PullAheadNeeded
 * @tc.desc  : Pull-ahead triggered when below threshold for diff rate
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, Accumulate_PullAheadNeeded, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, STEREO, STEREO);
    uint32_t smallFrameLen = 100; // Much less than threshold (882)
    PcmBufferInfo pcmInfo(STEREO, smallFrameLen, SAMPLE_RATE_44100);
    HpaePcmBuffer input(pcmInfo);
    input.GetPcmDataBuffer();

    converterNode->AccumulateInputAndPullAhead(&input);
    // Should have triggered pull-ahead attempt (but no upstream, so it won't pull more)
    EXPECT_EQ(converterNode->unprocessedFrames_, smallFrameLen);
}

/**
 * @tc.name  : Accumulate_DeadlockPrevention
 * @tc.type  : FUNC
 * @tc.number: Accumulate_DeadlockPrevention
 * @tc.desc  : Deadlock prevention pull when credit=0, unprocessed<threshold, processed=0
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, Accumulate_DeadlockPrevention, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, STEREO, STEREO);
    converterNode->pullAheadCredit_ = 0;
    converterNode->pullAheadMaxCredit_ = 2;
    converterNode->processedFrames_ = 0;

    uint32_t smallFrameLen = 100;
    PcmBufferInfo pcmInfo(STEREO, smallFrameLen, SAMPLE_RATE_44100);
    HpaePcmBuffer input(pcmInfo);
    input.GetPcmDataBuffer();

    converterNode->AccumulateInputAndPullAhead(&input);
    // Should attempt TryPullOneFrame for deadlock prevention
    EXPECT_EQ(converterNode->unprocessedFrames_, smallFrameLen);
}

/**
 * @tc.name  : Accumulate_NoDeadlockPullWhenProcessedData
 * @tc.type  : FUNC
 * @tc.number: Accumulate_NoDeadlockPullWhenProcessedData
 * @tc.desc  : No deadlock prevention pull when processedFrames_ > 0
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, Accumulate_NoDeadlockPullWhenProcessedData, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, STEREO, STEREO);
    converterNode->pullAheadCredit_ = 0;
    converterNode->pullAheadMaxCredit_ = 2;
    converterNode->processedFrames_ = 500; // Has processed data

    uint32_t smallFrameLen = 100;
    PcmBufferInfo pcmInfo(STEREO, smallFrameLen, SAMPLE_RATE_44100);
    HpaePcmBuffer input(pcmInfo);
    input.GetPcmDataBuffer();

    converterNode->AccumulateInputAndPullAhead(&input);
    EXPECT_EQ(converterNode->unprocessedFrames_, smallFrameLen);
}

// =========================================================================
// SignalProcess with inputs.size() != 1
// =========================================================================

/**
 * @tc.name  : SignalProcess_MultipleInputs
 * @tc.type  : FUNC
 * @tc.number: SignalProcess_MultipleInputs
 * @tc.desc  : SignalProcess with more than 1 input logs warning but continues
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, SignalProcess_MultipleInputs, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    PcmBufferInfo pcmInfo(STEREO, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_48000);
    HpaePcmBuffer input(pcmInfo);
    input.GetPcmDataBuffer();

    std::vector<HpaePcmBuffer *> inputs = {&input, &input};
    HpaePcmBuffer *result = converterNode->SignalProcess(inputs);
    EXPECT_NE(result, nullptr);
}

// =========================================================================
// SetDownmixNormalization Test
// =========================================================================

/**
 * @tc.name  : SetDownmixNormalization_Test
 * @tc.type  : FUNC
 * @tc.number: SetDownmixNormalization_Test
 * @tc.desc  : SetDownmixNormalization does not crash
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, SetDownmixNormalization_Test, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    converterNode->SetDownmixNormalization(true);
    converterNode->SetDownmixNormalization(false);
}

// =========================================================================
// RegisterCallback Test
// =========================================================================

/**
 * @tc.name  : RegisterCallback_Test
 * @tc.type  : FUNC
 * @tc.number: RegisterCallback_Test
 * @tc.desc  : RegisterCallback stores the callback pointer
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, RegisterCallback_Test, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    EXPECT_EQ(converterNode->nodeFormatInfoCallback_, nullptr);
    converterNode->RegisterCallback(nullptr);
    EXPECT_EQ(converterNode->nodeFormatInfoCallback_, nullptr);
}

// =========================================================================
// PullAheadUntilThresholdOrFailure Tests
// =========================================================================

/**
 * @tc.name  : PullAhead_ZeroInputFrameLen
 * @tc.type  : FUNC
 * @tc.number: PullAhead_ZeroInputFrameLen
 * @tc.desc  : PullAheadUntilThresholdOrFailure returns early when inputFrameLen is 0
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, PullAhead_ZeroInputFrameLen, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    converterNode->pullAheadCredit_ = 5;
    converterNode->unprocessedFrames_ = 100;

    converterNode->PullAheadUntilThresholdOrFailure(0);
    // Nothing changed
    EXPECT_EQ(converterNode->unprocessedFrames_, 100u);
}

/**
 * @tc.name  : PullAhead_ZeroCredit
 * @tc.type  : FUNC
 * @tc.number: PullAhead_ZeroCredit
 * @tc.desc  : PullAheadUntilThresholdOrFailure returns early when credit is 0
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, PullAhead_ZeroCredit, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    converterNode->pullAheadCredit_ = 0;
    converterNode->unprocessedFrames_ = 100;

    converterNode->PullAheadUntilThresholdOrFailure(960);
    EXPECT_EQ(converterNode->unprocessedFrames_, 100u);
}
}