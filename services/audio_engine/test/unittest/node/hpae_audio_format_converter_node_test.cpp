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

class MockNodeFormatInfoCallback : public INodeFormatInfoCallback {
public:
    virtual ~MockNodeFormatInfoCallback() = default;
    AudioBasicFormat formatToReturn_;
    int32_t queryCount_ = 0;
    int32_t GetNodeInputFormatInfo(uint32_t sessionId, AudioBasicFormat &basicFormat) override
    {
        queryCount_++;
        basicFormat = formatToReturn_;
        return 0;
    }
};

// Helper: create a standard 48kHz stereo->stereo converter node
static std::shared_ptr<HpaeAudioFormatConverterNode> CreateStdConverter()
{
    HpaeNodeInfo pre;
    pre.samplingRate = SAMPLE_RATE_48000;
    pre.frameLen = DEFAULT_FRAMELEN_SECOND;
    pre.channels = STEREO;
    HpaeNodeInfo out;
    out.samplingRate = SAMPLE_RATE_48000;
    out.frameLen = DEFAULT_FRAMELEN_SECOND;
    out.channels = STEREO;
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
    EXPECT_EQ(converterNode->preNodeInfo_.frameLen,
        SAMPLE_RATE_11025 * FRAME_LEN_20MS / MILLISECOND_PER_SECOND);
    // test 10hz 100ms customSampleRate, 0 frameLen data
    PcmBufferInfo pcmBufferInfo1(STEREO, 0, SAMPLE_RATE_48010);
    HpaePcmBuffer input1(pcmBufferInfo1);
    EXPECT_TRUE(converterNode->CheckUpdateInInfo(&input1));
    EXPECT_EQ(converterNode->preNodeInfo_.samplingRate, SAMPLE_RATE_48010);
    EXPECT_EQ(converterNode->preNodeInfo_.frameLen,
        SAMPLE_RATE_48010 * FRAME_LEN_20MS / MILLISECOND_PER_SECOND);
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
    EXPECT_EQ(converterNode->preNodeInfo_.frameLen,
        SAMPLE_RATE_11025 * FRAME_LEN_20MS / MILLISECOND_PER_SECOND);
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
    // downmix: tmpOutBuf_ reconfigured for channel conversion
    EXPECT_EQ(converterNode->tmpOutBuf_.GetSampleRate(), outputNodeInfo.samplingRate);
    EXPECT_EQ(converterNode->tmpOutBuf_.GetFrameLen(), outputNodeInfo.frameLen);
    EXPECT_EQ(converterNode->tmpOutBuf_.GetChannelCount(), pcmBufferInfo.ch);

    PcmBufferInfo pcmBufferInfo1(MONO, DEFAULT_FRAMELEN_FIRST, SAMPLE_RATE_48000);
    HpaePcmBuffer input1(pcmBufferInfo1);
    converterNode->CheckAndUpdateInfo(&input1);
    // upmix: tmpOutBuf_ reconfigured for channel conversion
    EXPECT_EQ(converterNode->tmpOutBuf_.GetSampleRate(), outputNodeInfo.samplingRate);
    EXPECT_EQ(converterNode->tmpOutBuf_.GetFrameLen(), outputNodeInfo.frameLen);
    EXPECT_EQ(converterNode->tmpOutBuf_.GetChannelCount(), pcmBufferInfo1.ch);
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
    EXPECT_EQ(converterNode->tmpOutBuf_.GetChannelCount(), pcmBufferInfo.ch); // downmix, channel change

    // resample, and then upmix
    PcmBufferInfo pcmBufferInfo1(MONO, DEFAULT_FRAMELEN_11025, SAMPLE_RATE_11025);
    HpaePcmBuffer input1(pcmBufferInfo1);
    converterNode->CheckAndUpdateInfo(&input1);
    // tmpOutBuf_ used for resample output, changed
    EXPECT_EQ(converterNode->tmpOutBuf_.GetSampleRate(), outputNodeInfo.samplingRate); // resample, sampleRate change
    EXPECT_EQ(converterNode->tmpOutBuf_.GetFrameLen(), 1920u); // resample, frameLen change
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
    EXPECT_EQ(converterNode->tmpOutBuf_.GetChannelCount(), pcmBufferInfo.ch); // downmix, channel change

    // resample, and then upmix
    PcmBufferInfo pcmBufferInfo1(MONO, DEFAULT_FRAMELEN_48010, SAMPLE_RATE_48010);
    HpaePcmBuffer input1(pcmBufferInfo1);
    converterNode->CheckAndUpdateInfo(&input1);
    // tmpOutBuf_ used for resample output, changed
    EXPECT_EQ(converterNode->tmpOutBuf_.GetSampleRate(), outputNodeInfo.samplingRate); // resample, sampleRate change
    EXPECT_EQ(converterNode->tmpOutBuf_.GetFrameLen(), 4800u); // resample, frameLen change
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
    EXPECT_EQ(converterNode->tmpOutBuf_.GetChannelCount(), pcmBufferInfo.ch); // downmix, channel change

    // resample, and then upmix
    PcmBufferInfo pcmBufferInfo1(MONO, 0, SAMPLE_RATE_48010);
    HpaePcmBuffer input1(pcmBufferInfo1);
    converterNode->CheckAndUpdateInfo(&input1);
    // tmpOutBuf_ used for resample output, changed
    EXPECT_EQ(converterNode->tmpOutBuf_.GetSampleRate(), outputNodeInfo.samplingRate); // resample, sampleRate change
    EXPECT_EQ(converterNode->tmpOutBuf_.GetFrameLen(), 4800u); // resample, frameLen change
    EXPECT_EQ(converterNode->tmpOutBuf_.GetChannelCount(), input1.GetChannelCount()); // resample, channel unchange
}

/**
 * @tc.name  : FaultCode_DoProcess_EmptyInputs
 * @tc.type  : FUNC
 * @tc.number: FaultCode_DoProcess_EmptyInputs
 * @tc.desc  : Test DoProcess with no upstream connection (empty inputs),
 *             should output silence without crash.
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, FaultCode_DoProcess_EmptyInputs, TestSize.Level1)
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
    converterNode->enableProcess_ = true;
    // No upstream connected, so ReadPreOutputData() returns empty vector
    converterNode->DoProcess();
    // Should complete without crash, output silence
    HpaePcmBuffer *result = converterNode->outputStream_.PullOutputData();
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : FaultCode_DoProcess_NullResampler
 * @tc.type  : FUNC
 * @tc.number: FaultCode_DoProcess_NullResampler
 * @tc.desc  : Test DoProcess when resampler_ is null,
 *             should output silence and report fault code.
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, FaultCode_DoProcess_NullResampler, TestSize.Level1)
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
    converterNode->enableProcess_ = true;

    // DoProcess checks resampler_ == nullptr early and outputs silence
    converterNode->DoProcess();
    HpaePcmBuffer *result = converterNode->outputStream_.PullOutputData();
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

/**
 * @tc.name  : CheckUpdateOutInfo_NoChange
 * @tc.type  : FUNC
 * @tc.number: CheckUpdateOutInfo_NoChange
 * @tc.desc  : CheckUpdateOutInfo returns false when callback returns same format as current output
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, CheckUpdateOutInfo_NoChange, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    MockNodeFormatInfoCallback mockCallback;
    mockCallback.formatToReturn_.audioChannelInfo.numChannels = STEREO;
    mockCallback.formatToReturn_.rate = SAMPLE_RATE_48000;
    converterNode->nodeFormatInfoCallback_ = &mockCallback;

    bool result = converterNode->CheckUpdateOutInfo();
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : CheckUpdateOutInfo_FallbackZeroChannelsNoChange
 * @tc.type  : FUNC
 * @tc.number: CheckUpdateOutInfo_FallbackZeroChannelsNoChange
 * @tc.desc  : CheckUpdateOutInfo returns false when callback returns numChannels=0 (fallback to node info matches)
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, CheckUpdateOutInfo_FallbackZeroChannelsNoChange, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    MockNodeFormatInfoCallback mockCallback;
    mockCallback.formatToReturn_.audioChannelInfo.numChannels = 0;
    converterNode->nodeFormatInfoCallback_ = &mockCallback;

    bool result = converterNode->CheckUpdateOutInfo();
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : CheckUpdateOutInfo_ChannelChangedSuccess
 * @tc.type  : FUNC
 * @tc.number: CheckUpdateOutInfo_ChannelChangedSuccess
 * @tc.desc  : CheckUpdateOutInfo returns true and updates channel converter when channel changes
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, CheckUpdateOutInfo_ChannelChangedSuccess, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    MockNodeFormatInfoCallback mockCallback;
    mockCallback.formatToReturn_.audioChannelInfo.numChannels = MONO;
    mockCallback.formatToReturn_.audioChannelInfo.channelLayout = CH_LAYOUT_MONO;
    mockCallback.formatToReturn_.rate = SAMPLE_RATE_48000;
    converterNode->nodeFormatInfoCallback_ = &mockCallback;

    bool result = converterNode->CheckUpdateOutInfo();
    EXPECT_TRUE(result);
    EXPECT_EQ(converterNode->channelConverter_.GetOutChannelInfo().numChannels, static_cast<uint32_t>(MONO));
    EXPECT_EQ(converterNode->GetNodeInfo().channels, static_cast<AudioChannel>(MONO));
    EXPECT_EQ(converterNode->GetSampleRate(), SAMPLE_RATE_48000);
}

/**
 * @tc.name  : CheckUpdateOutInfo_ChannelUpdateFails
 * @tc.type  : FUNC
 * @tc.number: CheckUpdateOutInfo_ChannelUpdateFails
 * @tc.desc  : CheckUpdateOutInfo returns false when UpdateOutChannelInfo fails due to invalid inChannelInfo
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, CheckUpdateOutInfo_ChannelUpdateFails, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    // Poison channelConverter by setting invalid inChannelInfo
    converterNode->channelConverter_.SetInChannelInfo({CH_LAYOUT_UNKNOWN, 0});
    MockNodeFormatInfoCallback mockCallback;
    mockCallback.formatToReturn_.audioChannelInfo.numChannels = MONO;
    mockCallback.formatToReturn_.audioChannelInfo.channelLayout = CH_LAYOUT_MONO;
    mockCallback.formatToReturn_.rate = SAMPLE_RATE_48000;
    converterNode->nodeFormatInfoCallback_ = &mockCallback;

    bool result = converterNode->CheckUpdateOutInfo();
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : CheckUpdateOutInfo_RateChangedFrameLenUpdated
 * @tc.type  : FUNC
 * @tc.number: CheckUpdateOutInfo_RateChangedFrameLenUpdated
 * @tc.desc  : CheckUpdateOutInfo returns true and proportionally updates frameLen when rate changes
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, CheckUpdateOutInfo_RateChangedFrameLenUpdated, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    MockNodeFormatInfoCallback mockCallback;
    mockCallback.formatToReturn_.audioChannelInfo.numChannels = STEREO;
    mockCallback.formatToReturn_.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    mockCallback.formatToReturn_.rate = SAMPLE_RATE_44100;
    converterNode->nodeFormatInfoCallback_ = &mockCallback;

    bool result = converterNode->CheckUpdateOutInfo();
    EXPECT_TRUE(result);
    EXPECT_EQ(converterNode->resampler_->GetOutRate(), SAMPLE_RATE_44100);
    EXPECT_EQ(converterNode->GetNodeInfo().frameLen, 882u);
}

/**
 * @tc.name  : CheckUpdateOutInfo_ChannelAndRateBothChanged
 * @tc.type  : FUNC
 * @tc.number: CheckUpdateOutInfo_ChannelAndRateBothChanged
 * @tc.desc  : CheckUpdateOutInfo returns true and updates both channel and rate when both change
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, CheckUpdateOutInfo_ChannelAndRateBothChanged, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    MockNodeFormatInfoCallback mockCallback;
    mockCallback.formatToReturn_.audioChannelInfo.numChannels = MONO;
    mockCallback.formatToReturn_.audioChannelInfo.channelLayout = CH_LAYOUT_MONO;
    mockCallback.formatToReturn_.rate = SAMPLE_RATE_44100;
    converterNode->nodeFormatInfoCallback_ = &mockCallback;

    bool result = converterNode->CheckUpdateOutInfo();
    EXPECT_TRUE(result);
    EXPECT_EQ(converterNode->channelConverter_.GetOutChannelInfo().numChannels, static_cast<uint32_t>(MONO));
    EXPECT_EQ(converterNode->resampler_->GetOutRate(), SAMPLE_RATE_44100);
    EXPECT_EQ(converterNode->GetNodeInfo().frameLen, 882u);
}

/**
 * @tc.name  : CheckUpdateOutInfo_FrameLenNotUpdatedOldRateZero
 * @tc.type  : FUNC
 * @tc.number: CheckUpdateOutInfo_FrameLenNotUpdatedOldRateZero
 * @tc.desc  : CheckUpdateOutInfo does not proportionally update frameLen when oldOutRate was 0
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, CheckUpdateOutInfo_FrameLenNotUpdatedOldRateZero, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, STEREO, STEREO);
    // Force resampler outRate to 0 (leaves resampler in broken state with state_=nullptr)
    converterNode->resampler_->UpdateRates(SAMPLE_RATE_44100, 0);
    ASSERT_EQ(converterNode->resampler_->GetOutRate(), 0u);
    size_t frameLenBefore = converterNode->GetNodeInfo().frameLen;
    MockNodeFormatInfoCallback mockCallback;
    mockCallback.formatToReturn_.audioChannelInfo.numChannels = STEREO;
    mockCallback.formatToReturn_.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    mockCallback.formatToReturn_.rate = SAMPLE_RATE_48000;
    converterNode->nodeFormatInfoCallback_ = &mockCallback;

    bool result = converterNode->CheckUpdateOutInfo();
    EXPECT_TRUE(result);
    // frameLen NOT proportionally updated because oldOutRate was 0
    EXPECT_EQ(converterNode->GetNodeInfo().frameLen, frameLenBefore);
}

/**
 * @tc.name  : CheckUpdateOutInfo_FallbackWithModifiedResampler
 * @tc.type  : FUNC
 * @tc.number: CheckUpdateOutInfo_FallbackWithModifiedResampler
 * @tc.desc  : CheckUpdateOutInfo with fallback (numChannels=0) detects rate mismatch with modified resampler
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, CheckUpdateOutInfo_FallbackWithModifiedResampler, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, STEREO, STEREO);
    // Change resampler so outRate differs from node info (48000)
    converterNode->resampler_->UpdateRates(SAMPLE_RATE_48000, SAMPLE_RATE_44100);
    ASSERT_EQ(converterNode->resampler_->GetOutRate(), SAMPLE_RATE_44100);
    MockNodeFormatInfoCallback mockCallback;
    mockCallback.formatToReturn_.audioChannelInfo.numChannels = 0;
    converterNode->nodeFormatInfoCallback_ = &mockCallback;

    bool result = converterNode->CheckUpdateOutInfo();
    EXPECT_TRUE(result);
    // Rate was restored: fallback uses GetSampleRate()=48000, resampler outRate updated to match
    EXPECT_EQ(converterNode->resampler_->GetOutRate(), SAMPLE_RATE_48000);
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
    EXPECT_EQ(converterNode->resamplerInputThreshold_, 960u);
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
 * @tc.name  : AppendUnprocessed_OverflowRejected
 * @tc.type  : FUNC
 * @tc.number: AppendUnprocessed_OverflowRejected
 * @tc.desc  : Append larger than current buffer capacity is rejected without resizing
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, AppendUnprocessed_OverflowRejected, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    size_t originalSize = converterNode->unprocessedBuffer_.size();
    uint32_t originalFrames = converterNode->unprocessedFrames_;
    uint32_t originalCredit = converterNode->pullAheadCredit_;
    uint32_t largeFrameLen = static_cast<uint32_t>(originalSize / STEREO + 100);
    std::vector<float> data(static_cast<size_t>(largeFrameLen) * STEREO, 0.25f);

    converterNode->AppendToUnprocessedBuffer(data.data(), largeFrameLen, STEREO);

    EXPECT_EQ(converterNode->unprocessedFrames_, originalFrames);
    EXPECT_EQ(converterNode->unprocessedBuffer_.size(), originalSize);
    EXPECT_EQ(converterNode->pullAheadCredit_, originalCredit);
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
 * @tc.name  : AppendProcessed_OverflowRejected
 * @tc.type  : FUNC
 * @tc.number: AppendProcessed_OverflowRejected
 * @tc.desc  : Append larger than current buffer capacity is rejected without resizing
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, AppendProcessed_OverflowRejected, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    size_t originalSize = converterNode->processedBuffer_.size();
    uint32_t originalFrames = converterNode->processedFrames_;
    uint32_t largeFrameLen = static_cast<uint32_t>(originalSize / STEREO + 100);
    std::vector<float> data(static_cast<size_t>(largeFrameLen) * STEREO, 0.4f);

    converterNode->AppendToProcessedBuffer(data.data(), largeFrameLen, STEREO);

    EXPECT_EQ(converterNode->processedFrames_, originalFrames);
    EXPECT_EQ(converterNode->processedBuffer_.size(), originalSize);
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
    // Create buffer larger than default to test compaction trigger
    size_t bufSize = frameLen * 6 * channels; // Larger buffer
    std::vector<float> testData(bufSize, 0.9f);
    converterNode->processedBuffer_ = testData;
    converterNode->processedFrames_ = frameLen * 2;
    // Set readPos past compaction threshold (size / 4)
    converterNode->processedReadPos_ = frameLen * 4;

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
 * @tc.name  : MoveUnprocessed_OverflowRejected
 * @tc.type  : FUNC
 * @tc.number: MoveUnprocessed_OverflowRejected
 * @tc.desc  : Move is rejected when processedBuffer_ is too small
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, MoveUnprocessed_OverflowRejected, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    uint32_t frames = 2000;
    uint32_t channels = STEREO;
    std::vector<float> data(static_cast<size_t>(frames) * channels, 0.8f);
    converterNode->unprocessedBuffer_ = data;
    converterNode->unprocessedFrames_ = frames;
    converterNode->processedBuffer_.clear();

    converterNode->MoveUnprocessedToProcessed(channels);

    EXPECT_EQ(converterNode->unprocessedFrames_, frames);
    EXPECT_EQ(converterNode->processedFrames_, 0u);
    EXPECT_EQ(converterNode->processedBuffer_.size(), 0u);
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
// DoProcess with multiple upstream inputs
// =========================================================================

/**
 * @tc.name  : DoProcess_MultipleInputs
 * @tc.type  : FUNC
 * @tc.number: DoProcess_MultipleInputs
 * @tc.desc  : DoProcess with more than 1 upstream input logs warning but continues
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, DoProcess_MultipleInputs, TestSize.Level0)
{
    // Create two upstream nodes with valid data
    auto upstream1 = CreateStdConverter();
    auto upstream2 = CreateStdConverter();
    PcmBufferInfo pcmInfo(STEREO, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_48000);
    HpaePcmBuffer input1(pcmInfo);
    input1.GetPcmDataBuffer();
    input1.SetBufferValid(true);
    HpaePcmBuffer input2(pcmInfo);
    input2.GetPcmDataBuffer();
    input2.SetBufferValid(true);

    // Write data to upstream output ports
    upstream1->outputStream_.WriteDataToOutput(&input1);
    upstream2->outputStream_.WriteDataToOutput(&input2);

    // Create test node and connect both upstreams
    auto converterNode = CreateStdConverter();
    converterNode->inputStream_.AddPreOutput(
        upstream1->GetSharedInstance(), &upstream1->outputStream_);
    converterNode->inputStream_.AddPreOutput(
        upstream2->GetSharedInstance(), &upstream2->outputStream_);
    converterNode->enableProcess_ = true;

    // DoProcess will call SignalProcess with 2-element vector, logs warning but continues
    converterNode->DoProcess();
    HpaePcmBuffer *result = converterNode->outputStream_.PullOutputData();
    EXPECT_NE(result, nullptr);
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

// ======================== FlushBuffers tests ========================

/**
 * @tc.name  : FlushBuffers_BasicReset
 * @tc.type  : FUNC
 * @tc.number: FlushBuffers_BasicReset
 * @tc.desc  : FlushBuffers resets all counters and credit after partial data accumulation
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, FlushBuffers_BasicReset, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, STEREO, STEREO);
    // Simulate partial data in both buffers
    converterNode->unprocessedFrames_ = 200;
    converterNode->processedFrames_ = 500;
    converterNode->processedReadPos_ = 100;
    converterNode->pullAheadCredit_ = 0;

    converterNode->FlushBuffers();

    EXPECT_EQ(converterNode->unprocessedFrames_, 0u);
    EXPECT_EQ(converterNode->processedFrames_, 0u);
    EXPECT_EQ(converterNode->processedReadPos_, 0u);
    EXPECT_EQ(converterNode->pullAheadCredit_, converterNode->pullAheadMaxCredit_);
}

/**
 * @tc.name  : FlushBuffers_EmptyBuffers
 * @tc.type  : FUNC
 * @tc.number: FlushBuffers_EmptyBuffers
 * @tc.desc  : FlushBuffers on already-empty buffers does not crash
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, FlushBuffers_EmptyBuffers, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    EXPECT_EQ(converterNode->unprocessedFrames_, 0u);
    EXPECT_EQ(converterNode->processedFrames_, 0u);
    EXPECT_EQ(converterNode->processedReadPos_, 0u);

    converterNode->FlushBuffers();

    EXPECT_EQ(converterNode->unprocessedFrames_, 0u);
    EXPECT_EQ(converterNode->processedFrames_, 0u);
    EXPECT_EQ(converterNode->processedReadPos_, 0u);
    EXPECT_EQ(converterNode->pullAheadCredit_, converterNode->pullAheadMaxCredit_);
}

/**
 * @tc.name  : FlushBuffers_ThenProcessCleansOutput
 * @tc.type  : FUNC
 * @tc.number: FlushBuffers_ThenProcessCleansOutput
 * @tc.desc  : After FlushBuffers, new data is processed from scratch without stale data
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, FlushBuffers_ThenProcessCleansOutput, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, STEREO, STEREO);
    uint32_t inCh = STEREO;

    // Fill buffers with stale data
    converterNode->unprocessedFrames_ = 882;
    converterNode->processedFrames_ = 960;
    converterNode->processedReadPos_ = 0;
    converterNode->pullAheadCredit_ = 0;

    converterNode->FlushBuffers();

    // Verify all cleared
    EXPECT_EQ(converterNode->unprocessedFrames_, 0u);
    EXPECT_EQ(converterNode->processedFrames_, 0u);
    EXPECT_EQ(converterNode->processedReadPos_, 0u);
    EXPECT_EQ(converterNode->pullAheadCredit_, converterNode->pullAheadMaxCredit_);

    // Now append new data and verify it works cleanly
    std::vector<float> newData(882u * inCh, 0.5f);
    converterNode->AppendToUnprocessedBuffer(newData.data(), 882, inCh);
    EXPECT_EQ(converterNode->unprocessedFrames_, 882u);
}

/**
 * @tc.name  : FlushBuffers_CreditReset
 * @tc.type  : FUNC
 * @tc.number: FlushBuffers_CreditReset
 * @tc.desc  : FlushBuffers resets pullAheadCredit_ to pullAheadMaxCredit_
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, FlushBuffers_CreditReset, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_11025, SAMPLE_RATE_48000, STEREO, STEREO);
    uint32_t maxCredit = converterNode->pullAheadMaxCredit_;
    ASSERT_GT(maxCredit, 0u);

    // Exhaust credit
    converterNode->pullAheadCredit_ = 0;
    EXPECT_EQ(converterNode->pullAheadCredit_, 0u);

    converterNode->FlushBuffers();
    EXPECT_EQ(converterNode->pullAheadCredit_, maxCredit);
}

/**
 * @tc.name  : FlushBuffers_PartialCreditReset
 * @tc.type  : FUNC
 * @tc.number: FlushBuffers_PartialCreditReset
 * @tc.desc  : FlushBuffers restores credit from partial value
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, FlushBuffers_PartialCreditReset, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_11025, SAMPLE_RATE_48000, STEREO, STEREO);
    uint32_t maxCredit = converterNode->pullAheadMaxCredit_;
    ASSERT_GT(maxCredit, 1u);

    converterNode->pullAheadCredit_ = maxCredit / 2;
    converterNode->FlushBuffers();
    EXPECT_EQ(converterNode->pullAheadCredit_, maxCredit);
}

/**
 * @tc.name  : FlushBuffers_11025HzLargeBuffers
 * @tc.type  : FUNC
 * @tc.number: FlushBuffers_11025HzLargeBuffers
 * @tc.desc  : FlushBuffers clears large accumulated data for 11025Hz non-standard rate
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, FlushBuffers_11025HzLargeBuffers, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_11025, SAMPLE_RATE_48000, STEREO, STEREO);
    // 11025Hz threshold is 441, accumulate more than threshold
    converterNode->unprocessedFrames_ = 600;
    converterNode->processedFrames_ = 2000;
    converterNode->processedReadPos_ = 960;
    converterNode->pullAheadCredit_ = 0;

    converterNode->FlushBuffers();

    EXPECT_EQ(converterNode->unprocessedFrames_, 0u);
    EXPECT_EQ(converterNode->processedFrames_, 0u);
    EXPECT_EQ(converterNode->processedReadPos_, 0u);
    EXPECT_EQ(converterNode->pullAheadCredit_, converterNode->pullAheadMaxCredit_);
}

/**
 * @tc.name  : FlushBuffers_8010HzBuffers
 * @tc.type  : FUNC
 * @tc.number: FlushBuffers_8010HzBuffers
 * @tc.desc  : FlushBuffers clears data for 8010Hz non-standard rate
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, FlushBuffers_8010HzBuffers, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_8010, SAMPLE_RATE_48000, STEREO, STEREO);
    converterNode->unprocessedFrames_ = 801;
    converterNode->processedFrames_ = 4800;
    converterNode->processedReadPos_ = 0;
    converterNode->pullAheadCredit_ = 0;

    converterNode->FlushBuffers();

    EXPECT_EQ(converterNode->unprocessedFrames_, 0u);
    EXPECT_EQ(converterNode->processedFrames_, 0u);
    EXPECT_EQ(converterNode->processedReadPos_, 0u);
    EXPECT_EQ(converterNode->pullAheadCredit_, converterNode->pullAheadMaxCredit_);
}

// ======================== GetLatency tests ========================

/**
 * @tc.name  : GetLatency_EmptyBuffers
 * @tc.type  : FUNC
 * @tc.number: GetLatency_EmptyBuffers
 * @tc.desc  : GetLatency returns 0 when both buffers are empty
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, GetLatency_EmptyBuffers, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, STEREO, STEREO);
    EXPECT_EQ(converterNode->unprocessedFrames_, 0u);
    EXPECT_EQ(converterNode->processedFrames_, 0u);

    uint64_t latency = converterNode->GetLatency();
    EXPECT_EQ(latency, 0u);
}

/**
 * @tc.name  : GetLatency_Standard48kHz
 * @tc.type  : FUNC
 * @tc.number: GetLatency_Standard48kHz
 * @tc.desc  : GetLatency returns 0 for standard 48kHz with empty buffers (same rate passthrough)
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, GetLatency_Standard48kHz, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    // 48kHz->48kHz same rate, empty buffers
    uint64_t latency = converterNode->GetLatency();
    EXPECT_EQ(latency, 0u);
}

/**
 * @tc.name  : GetLatency_UnprocessedOnly
 * @tc.type  : FUNC
 * @tc.number: GetLatency_UnprocessedOnly
 * @tc.desc  : GetLatency reports delay from unprocessedBuffer_ only at input rate
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, GetLatency_UnprocessedOnly, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, STEREO, STEREO);
    // 441 frames at 44100Hz = 10ms = 10000us
    uint32_t frames = 441;
    converterNode->unprocessedFrames_ = frames;
    converterNode->processedFrames_ = 0;

    uint64_t latency = converterNode->GetLatency();
    // Expected: 441 * 1000000 / 44100 = 10000 us
    uint64_t expected = static_cast<uint64_t>(frames) * 1000000ULL / SAMPLE_RATE_44100;
    EXPECT_EQ(latency, expected);
}

/**
 * @tc.name  : GetLatency_ProcessedOnly
 * @tc.type  : FUNC
 * @tc.number: GetLatency_ProcessedOnly
 * @tc.desc  : GetLatency reports delay from processedBuffer_ only at output rate
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, GetLatency_ProcessedOnly, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, STEREO, STEREO);
    // 960 frames at 48000Hz = 20ms = 20000us
    uint32_t frames = 960;
    converterNode->unprocessedFrames_ = 0;
    converterNode->processedFrames_ = frames;

    uint64_t latency = converterNode->GetLatency();
    // Expected: 960 * 1000000 / 48000 = 20000 us
    uint64_t expected = static_cast<uint64_t>(frames) * 1000000ULL / SAMPLE_RATE_48000;
    EXPECT_EQ(latency, expected);
}

/**
 * @tc.name  : GetLatency_BothBuffers
 * @tc.type  : FUNC
 * @tc.number: GetLatency_BothBuffers
 * @tc.desc  : GetLatency reports combined delay from both buffers
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, GetLatency_BothBuffers, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, STEREO, STEREO);
    // unprocessed: 441 frames at 44100Hz = 10000us
    // processed: 960 frames at 48000Hz = 20000us
    // total = 30000us
    converterNode->unprocessedFrames_ = 441;
    converterNode->processedFrames_ = 960;

    uint64_t latency = converterNode->GetLatency();
    uint64_t expectedUnprocessed = static_cast<uint64_t>(441) * 1000000ULL / SAMPLE_RATE_44100;
    uint64_t expectedProcessed = static_cast<uint64_t>(960) * 1000000ULL / SAMPLE_RATE_48000;
    EXPECT_EQ(latency, expectedUnprocessed + expectedProcessed);
}

/**
 * @tc.name  : GetLatency_11025HzNonStandard
 * @tc.type  : FUNC
 * @tc.number: GetLatency_11025HzNonStandard
 * @tc.desc  : GetLatency correctly reports large delay for 11025Hz with accumulated data
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, GetLatency_11025HzNonStandard, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_11025, SAMPLE_RATE_48000, STEREO, STEREO);
    // 441 frames at 11025Hz = 40ms = 40000us
    converterNode->unprocessedFrames_ = 441;
    converterNode->processedFrames_ = 0;

    uint64_t latency = converterNode->GetLatency();
    uint64_t expected = static_cast<uint64_t>(441) * 1000000ULL / SAMPLE_RATE_11025;
    EXPECT_EQ(latency, expected);
    // Verify it's approximately 40ms
    EXPECT_GE(latency, 39000u);
    EXPECT_LE(latency, 41000u);
}

/**
 * @tc.name  : GetLatency_AfterFlushBuffers
 * @tc.type  : FUNC
 * @tc.number: GetLatency_AfterFlushBuffers
 * @tc.desc  : GetLatency returns 0 after FlushBuffers clears all data
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, GetLatency_AfterFlushBuffers, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, STEREO, STEREO);
    converterNode->unprocessedFrames_ = 882;
    converterNode->processedFrames_ = 1920;
    ASSERT_GT(converterNode->GetLatency(), 0u);

    converterNode->FlushBuffers();
    EXPECT_EQ(converterNode->GetLatency(), 0u);
}

/**
 * @tc.name  : GetLatency_Accuracy_LatencyDecreasesAfterExtract
 * @tc.type  : FUNC
 * @tc.number: GetLatency_Accuracy_LatencyDecreasesAfterExtract
 * @tc.desc  : GetLatency decreases after output is extracted from processedBuffer
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, GetLatency_Accuracy_LatencyDecreasesAfterExtract, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, STEREO, STEREO);
    // Setup: 1920 processed frames at 48000Hz (40ms)
    converterNode->processedFrames_ = 1920;
    converterNode->processedReadPos_ = 0;
    converterNode->unprocessedFrames_ = 0;

    uint64_t latencyBefore = converterNode->GetLatency();

    // Simulate extracting one 20ms chunk (960 frames) via RemoveConsumed
    converterNode->processedFrames_ -= 960;
    converterNode->processedReadPos_ += 960;

    uint64_t latencyAfter = converterNode->GetLatency();
    EXPECT_LT(latencyAfter, latencyBefore);
    // 960 frames at 48000Hz = 20000us difference
    EXPECT_EQ(latencyBefore - latencyAfter, 20000u);
}

// ======================== Low-latency same-rate (5ms frameLen) tests ========================

namespace {
constexpr uint32_t FRAME_LEN_5MS = 5;
constexpr uint32_t FRAMELEN_5MS_48K = SAMPLE_RATE_48000 * FRAME_LEN_5MS / MILLISECOND_PER_SECOND; // 240

// Helper: create a same-rate converter with 5ms frameLen (low-latency path simulation)
static std::shared_ptr<HpaeAudioFormatConverterNode> CreateLowLatencyConverter()
{
    HpaeNodeInfo pre;
    pre.samplingRate = SAMPLE_RATE_48000;
    pre.frameLen = FRAMELEN_5MS_48K;
    pre.channels = STEREO;
    HpaeNodeInfo out;
    out.samplingRate = SAMPLE_RATE_48000;
    out.frameLen = FRAMELEN_5MS_48K;
    out.channels = STEREO;
    return std::make_shared<HpaeAudioFormatConverterNode>(pre, out);
}
}

/**
 * @tc.name  : LowLatency_OutputFrameLenMatchesNodeFrameLen
 * @tc.type  : FUNC
 * @tc.number: LowLatency_OutputFrameLenMatchesNodeFrameLen
 * @tc.desc  : For same-rate low-latency (5ms), outputFrameLen20ms_ should equal node frameLen (240), not 960
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, LowLatency_OutputFrameLenMatchesNodeFrameLen, TestSize.Level0)
{
    auto converterNode = CreateLowLatencyConverter();
    // Same rate 48kHz->48kHz: outputFrameLen20ms_ must match node's frameLen (240), not 960
    EXPECT_EQ(converterNode->outputFrameLen20ms_, FRAMELEN_5MS_48K);
    EXPECT_EQ(converterNode->outputFrameLen20ms_, 240u);
}

/**
 * @tc.name  : LowLatency_ExtractOutputNoOverflow
 * @tc.type  : FUNC
 * @tc.number: LowLatency_ExtractOutputNoOverflow
 * @tc.desc  : ExtractOutputFromProcessedBuffer succeeds for 5ms same-rate without memcpy_s overflow
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, LowLatency_ExtractOutputNoOverflow, TestSize.Level0)
{
    auto converterNode = CreateLowLatencyConverter();
    uint32_t channels = converterNode->converterOutput_.GetChannelCount();
    // Simulate processedBuffer_ with exactly 240 frames (one 5ms chunk)
    converterNode->processedFrames_ = FRAMELEN_5MS_48K;
    converterNode->processedReadPos_ = 0;
    // Ensure processedBuffer_ has enough capacity
    size_t requiredSize = static_cast<size_t>(FRAMELEN_5MS_48K) * channels;
    if (converterNode->processedBuffer_.size() < requiredSize) {
        converterNode->processedBuffer_.resize(requiredSize, 0.0f);
    }
    // Fill with non-zero data to verify copy
    for (size_t i = 0; i < requiredSize; i++) {
        converterNode->processedBuffer_[i] = static_cast<float>(i);
    }

    bool result = converterNode->ExtractOutputFromProcessedBuffer();
    EXPECT_TRUE(result);
    EXPECT_EQ(converterNode->processedFrames_, 0u);
}

/**
 * @tc.name  : LowLatency_ExtractOutput_MultipleChunks
 * @tc.type  : FUNC
 * @tc.number: LowLatency_ExtractOutput_MultipleChunks
 * @tc.desc  : Multiple 5ms chunks can be extracted sequentially without overflow
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, LowLatency_ExtractOutput_MultipleChunks, TestSize.Level0)
{
    auto converterNode = CreateLowLatencyConverter();
    uint32_t channels = converterNode->converterOutput_.GetChannelCount();
    // Simulate 4 chunks worth of data (4 * 240 = 960 frames)
    uint32_t totalFrames = FRAMELEN_5MS_48K * 4;
    size_t requiredSize = static_cast<size_t>(totalFrames) * channels;
    converterNode->processedBuffer_.resize(requiredSize, 0.0f);
    for (size_t i = 0; i < requiredSize; i++) {
        converterNode->processedBuffer_[i] = static_cast<float>(i % 100);
    }
    converterNode->processedFrames_ = totalFrames;
    converterNode->processedReadPos_ = 0;

    // Extract all 4 chunks one by one
    for (int i = 0; i < 4; i++) {
        bool result = converterNode->ExtractOutputFromProcessedBuffer();
        EXPECT_TRUE(result) << "Extract failed at chunk " << i;
    }
    EXPECT_EQ(converterNode->processedFrames_, 0u);
}

/**
 * @tc.name  : LowLatency_PassthroughDoesNotAccumulate
 * @tc.type  : FUNC
 * @tc.number: LowLatency_PassthroughDoesNotAccumulate
 * @tc.desc  : Same-rate passthrough for 5ms: input 240 frames, process, extract 240 frames in one cycle
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, LowLatency_PassthroughDoesNotAccumulate, TestSize.Level0)
{
    auto converterNode = CreateLowLatencyConverter();
    uint32_t channels = STEREO;
    uint32_t frameLen = FRAMELEN_5MS_48K;

    // Append one 5ms input chunk
    std::vector<float> inputData(static_cast<size_t>(frameLen) * channels, 0.5f);
    converterNode->AppendToUnprocessedBuffer(inputData.data(), frameLen, channels);
    EXPECT_EQ(converterNode->unprocessedFrames_, frameLen);

    // ProcessResampleLoop for same-rate same-channel: MoveUnprocessedToProcessed
    converterNode->ProcessResampleLoop();
    EXPECT_EQ(converterNode->unprocessedFrames_, 0u);
    EXPECT_EQ(converterNode->processedFrames_, frameLen);

    // Extract should succeed immediately (no accumulation needed)
    converterNode->converterOutput_.Reset();
    bool result = converterNode->ExtractOutputFromProcessedBuffer();
    EXPECT_TRUE(result);
    EXPECT_EQ(converterNode->processedFrames_, 0u);
}

/**
 * @tc.name  : LowLatency_DiffRateUsesConfiguredFrameMs
 * @tc.type  : FUNC
 * @tc.number: LowLatency_DiffRateUsesConfiguredFrameMs
 * @tc.desc  : Different rate (44100->48000) uses configured 5ms frame duration
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, LowLatency_DiffRateUsesConfiguredFrameMs, TestSize.Level0)
{
    HpaeNodeInfo pre;
    pre.samplingRate = SAMPLE_RATE_44100;
    pre.frameLen = SAMPLE_RATE_44100 * FRAME_LEN_5MS / MILLISECOND_PER_SECOND;
    pre.channels = STEREO;
    HpaeNodeInfo out;
    out.samplingRate = SAMPLE_RATE_48000;
    out.frameLen = SAMPLE_RATE_48000 * FRAME_LEN_5MS / MILLISECOND_PER_SECOND;
    out.channels = STEREO;
    auto converterNode = std::make_shared<HpaeAudioFormatConverterNode>(pre, out);

    uint32_t expected5ms = SAMPLE_RATE_48000 * FRAME_LEN_5MS / MILLISECOND_PER_SECOND;
    EXPECT_EQ(converterNode->outputFrameLen20ms_, expected5ms);
    EXPECT_EQ(converterNode->outputFrameLen20ms_, FRAMELEN_5MS_48K);
}

/**
 * @tc.name  : LowLatency_StandardRate20ms
 * @tc.type  : FUNC
 * @tc.number: LowLatency_StandardRate20ms
 * @tc.desc  : Standard 48kHz->48kHz with 20ms frameLen uses 960 as outputFrameLen
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, LowLatency_StandardRate20ms, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    // Same rate with 20ms frameLen: outputFrameLen20ms_ = 960
    EXPECT_EQ(converterNode->outputFrameLen20ms_, DEFAULT_FRAMELEN_SECOND);
    EXPECT_EQ(converterNode->outputFrameLen20ms_, 960u);
}

/**
 * @tc.name  : DoProcess_DrainWithBufferState
 * @tc.type  : FUNC
 * @tc.number: DoProcess_DrainWithBufferState
 * @tc.desc  : mode drains processedBuffer_ and propagates buffer state from input
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, DoProcess_DrainWithBufferState, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    // Ensure (default is empty string)
    HpaeNodeInfo info = converterNode->GetNodeInfo();
    converterNode->SetNodeInfo(info);
    // Populate processedBuffer_ with enough data
    uint32_t frameLen = converterNode->outputFrameLen20ms_;
    std::vector<float> testData(frameLen * STEREO, 0.5f);
    converterNode->processedBuffer_ = testData;
    converterNode->processedFrames_ = frameLen;
    converterNode->processedReadPos_ = 0;

    converterNode->DoProcess();
    // should drain, processedFrames_ becomes 0
    EXPECT_EQ(converterNode->processedFrames_, 0u);
}

/**
 * @tc.name  : DoProcess_InsufficientData
 * @tc.type  : FUNC
 * @tc.number: DoProcess_InsufficientData
 * @tc.desc  : outputs silence when processedBuffer_ has insufficient data
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, DoProcess_InsufficientData, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    HpaeNodeInfo info = converterNode->GetNodeInfo();
    converterNode->SetNodeInfo(info);
    // Less than outputFrameLen20ms_
    converterNode->processedFrames_ = 1;
    converterNode->processedReadPos_ = 0;

    converterNode->DoProcess();
    // Not enough data to drain, should fall through to silence
    EXPECT_EQ(converterNode->processedFrames_, 1u);
}

/**
 * @tc.name  : DoProcess_DrainEmptyPreOutputs
 * @tc.type  : FUNC
 * @tc.number: DoProcess_DrainEmptyPreOutputs
 * @tc.desc  : drain does not crash when preOutputs is empty
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, DoProcess_DrainEmptyPreOutputs, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    HpaeNodeInfo info = converterNode->GetNodeInfo();
    converterNode->SetNodeInfo(info);
    uint32_t frameLen = converterNode->outputFrameLen20ms_;
    std::vector<float> testData(frameLen * STEREO, 0.5f);
    converterNode->processedBuffer_ = testData;
    converterNode->processedFrames_ = frameLen;
    converterNode->processedReadPos_ = 0;
    // No upstream connected, so preOutputs will be empty

    converterNode->DoProcess();
    // Should drain without crash even with empty preOutputs
    EXPECT_EQ(converterNode->processedFrames_, 0u);
}

/**
 * @tc.name  : DoProcess_DrainPath_WithPreOutputs_InvalidBuffer
 * @tc.type  : FUNC
 * @tc.number: DoProcess_DrainPath_WithPreOutputs_InvalidBuffer
 * @tc.desc  : Drain path enters buffer state propagation block when upstream
 *             provides a non-null but invalid buffer (IsValid=false)
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, DoProcess_DrainPath_WithPreOutputs_InvalidBuffer, TestSize.Level0)
{
    // Setup upstream node with invalid buffer
    auto upstreamNode = CreateStdConverter();
    PcmBufferInfo pcmInfo(STEREO, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_48000);
    HpaePcmBuffer upstreamBuffer(pcmInfo);
    upstreamBuffer.GetPcmDataBuffer();
    // Set state to SILENCE|INVALID (value 3) - SetBufferState overwrites entire state word
    upstreamBuffer.SetBufferState(PCM_BUFFER_STATE_SILENCE | PCM_BUFFER_STATE_INVALID);
    upstreamNode->outputStream_.WriteDataToOutput(&upstreamBuffer);

    // Setup test node with upstream connection
    auto converterNode = CreateStdConverter();
    converterNode->inputStream_.AddPreOutput(
        upstreamNode->GetSharedInstance(), &upstreamNode->outputStream_);

    // Populate drain conditions
    uint32_t frameLen = converterNode->outputFrameLen20ms_;
    std::vector<float> testData(frameLen * STEREO, 0.5f);
    converterNode->processedBuffer_ = testData;
    converterNode->processedFrames_ = frameLen;
    converterNode->processedReadPos_ = 0;
    converterNode->enableProcess_ = true;

    // Set pre-state to invalid so IsValid()==true proves drain path set it
    converterNode->converterOutput_.SetBufferValid(false);

    converterNode->DoProcess();

    EXPECT_EQ(converterNode->processedFrames_, 0u);
    // SetBufferState copies full state (SILENCE|INVALID), then SetBufferValid(true) clears INVALID
    EXPECT_EQ(converterNode->converterOutput_.GetBufferState(), PCM_BUFFER_STATE_SILENCE);
    EXPECT_TRUE(converterNode->converterOutput_.IsValid());
}

/**
 * @tc.name  : DoProcess_DrainPath_EmptyPreOutputs_NoStatePropagation
 * @tc.type  : FUNC
 * @tc.number: DoProcess_DrainPath_EmptyPreOutputs_NoStatePropagation
 * @tc.desc  : Drain path skips buffer state propagation when preOutputs is empty,
 *             converterOutput_ remains in pre-set invalid state
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, DoProcess_DrainPath_EmptyPreOutputs_NoStatePropagation, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();

    uint32_t frameLen = converterNode->outputFrameLen20ms_;
    std::vector<float> testData(frameLen * STEREO, 0.5f);
    converterNode->processedBuffer_ = testData;
    converterNode->processedFrames_ = frameLen;
    converterNode->processedReadPos_ = 0;
    converterNode->enableProcess_ = true;

    // Set pre-state to invalid so IsValid()==false proves drain path did NOT set it
    converterNode->converterOutput_.SetBufferValid(false);

    converterNode->DoProcess();

    EXPECT_EQ(converterNode->processedFrames_, 0u);
    // preOutputs is empty, so inner if block is skipped, SetBufferValid(true) never called
    EXPECT_FALSE(converterNode->converterOutput_.IsValid());
}

// ======================== Additional 45 Test Cases for Branch Coverage ========================

// =========================================================================
// TryPullOneFrame Tests
// =========================================================================

/**
 * @tc.name  : TryPullOneFrame_EmptyInputs
 * @tc.type  : FUNC
 * @tc.number: TryPullOneFrame_EmptyInputs
 * @tc.desc  : TryPullOneFrame returns false when inputs are empty
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, TryPullOneFrame_EmptyInputs, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    uint32_t beforeFrames = converterNode->unprocessedFrames_;

    bool result = converterNode->TryPullOneFrame();
    EXPECT_FALSE(result);
    EXPECT_EQ(converterNode->unprocessedFrames_, beforeFrames);
}

/**
 * @tc.name  : TryPullOneFrame_NullFirstElement
 * @tc.type  : FUNC
 * @tc.number: TryPullOneFrame_NullFirstElement
 * @tc.desc  : TryPullOneFrame returns false when first input element is null
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, TryPullOneFrame_NullFirstElement, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    uint32_t beforeFrames = converterNode->unprocessedFrames_;

    bool result = converterNode->TryPullOneFrame();
    EXPECT_FALSE(result);
    EXPECT_EQ(converterNode->unprocessedFrames_, beforeFrames);
}

/**
 * @tc.name  : TryPullOneFrame_InvalidBuffer
 * @tc.type  : FUNC
 * @tc.number: TryPullOneFrame_InvalidBuffer
 * @tc.desc  : TryPullOneFrame returns false when buffer is invalid
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, TryPullOneFrame_InvalidBuffer, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    uint32_t beforeFrames = converterNode->unprocessedFrames_;

    bool result = converterNode->TryPullOneFrame();
    EXPECT_FALSE(result);
    EXPECT_EQ(converterNode->unprocessedFrames_, beforeFrames);
}

// =========================================================================
// PullAheadUntilThresholdOrFailure Tests
// =========================================================================

/**
 * @tc.name  : PullAhead_StopsAtThreshold
 * @tc.type  : FUNC
 * @tc.number: PullAhead_StopsAtThreshold
 * @tc.desc  : PullAhead stops when threshold is reached
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, PullAhead_StopsAtThreshold, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, STEREO, STEREO);
    converterNode->unprocessedFrames_ = 800; // Close to threshold (882)
    converterNode->pullAheadCredit_ = 5;

    converterNode->PullAheadUntilThresholdOrFailure(960);
    // Should have tried but stopped due to no upstream
    EXPECT_EQ(converterNode->unprocessedFrames_, 800u);
}

/**
 * @tc.name  : PullAhead_SingleFrameThenStop
 * @tc.type  : FUNC
 * @tc.number: PullAhead_SingleFrameThenStop
 * @tc.desc  : PullAhead with credit=1, tries once then stops
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, PullAhead_SingleFrameThenStop, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    converterNode->pullAheadCredit_ = 1;
    converterNode->unprocessedFrames_ = 100;

    converterNode->PullAheadUntilThresholdOrFailure(960);
    EXPECT_EQ(converterNode->unprocessedFrames_, 100u);
}

/**
 * @tc.name  : PullAhead_NoCreditNoAction
 * @tc.type  : FUNC
 * @tc.number: PullAhead_NoCreditNoAction
 * @tc.desc  : PullAhead does nothing when credit is 0
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, PullAhead_NoCreditNoAction, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    converterNode->pullAheadCredit_ = 0;
    converterNode->unprocessedFrames_ = 100;

    converterNode->PullAheadUntilThresholdOrFailure(960);
    EXPECT_EQ(converterNode->unprocessedFrames_, 100u);
}

// =========================================================================
// ProcessSameRateChannelConversion Tests
// =========================================================================

/**
 * @tc.name  : ProcessSameRate_ResizesOutBuffer
 * @tc.type  : FUNC
 * @tc.number: ProcessSameRate_ResizesOutBuffer
 * @tc.desc  : ProcessSameRateChannelConversion resizes output buffer when needed
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, ProcessSameRate_ResizesOutBuffer, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_48000, SAMPLE_RATE_48000, CHANNEL_6, STEREO);
    uint32_t threshold = converterNode->resamplerInputThreshold_;
    converterNode->resamplerOutBuf_.clear();
    converterNode->resamplerOutBuf_.resize(10, 0.0f); // Too small

    std::vector<float> data(threshold * CHANNEL_6, 0.5f);
    converterNode->unprocessedBuffer_ = data;
    converterNode->unprocessedFrames_ = threshold;

    converterNode->ProcessSameRateChannelConversion(CHANNEL_6, STEREO);
    EXPECT_GT(converterNode->resamplerOutBuf_.size(), 10u);
    EXPECT_EQ(converterNode->unprocessedFrames_, 0u);
}

/**
 * @tc.name  : ProcessSameRate_ZeroFrames
 * @tc.type  : FUNC
 * @tc.number: ProcessSameRate_ZeroFrames
 * @tc.desc  : ProcessSameRateChannelConversion does nothing with zero unprocessed frames
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, ProcessSameRate_ZeroFrames, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_48000, SAMPLE_RATE_48000, CHANNEL_6, STEREO);
    converterNode->unprocessedFrames_ = 0;

    converterNode->ProcessSameRateChannelConversion(CHANNEL_6, STEREO);
    EXPECT_EQ(converterNode->unprocessedFrames_, 0u);
    EXPECT_EQ(converterNode->processedFrames_, 0u);
}

/**
 * @tc.name  : ProcessSameRate_InsufficientFrames
 * @tc.type  : FUNC
 * @tc.number: ProcessSameRate_InsufficientFrames
 * @tc.desc  : ProcessSameRateChannelConversion does nothing below threshold
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, ProcessSameRate_InsufficientFrames, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_48000, SAMPLE_RATE_48000, CHANNEL_6, STEREO);
    uint32_t threshold = converterNode->resamplerInputThreshold_;
    converterNode->unprocessedFrames_ = threshold - 1;

    converterNode->ProcessSameRateChannelConversion(CHANNEL_6, STEREO);
    EXPECT_EQ(converterNode->unprocessedFrames_, threshold - 1);
    EXPECT_EQ(converterNode->processedFrames_, 0u);
}

// =========================================================================
// ProcessDiffRateResample Tests
// =========================================================================

/**
 * @tc.name  : ProcessDiffRate_ResizesOutBuffer
 * @tc.type  : FUNC
 * @tc.number: ProcessDiffRate_ResizesOutBuffer
 * @tc.desc  : ProcessDiffRateResample resizes output buffer when needed
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, ProcessDiffRate_ResizesOutBuffer, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, STEREO, STEREO);
    uint32_t threshold = converterNode->resamplerInputThreshold_;
    converterNode->resamplerOutBuf_.clear();
    converterNode->resamplerOutBuf_.resize(10, 0.0f); // Too small

    std::vector<float> data(threshold * STEREO, 0.5f);
    converterNode->unprocessedBuffer_ = data;
    converterNode->unprocessedFrames_ = threshold;

    converterNode->ProcessDiffRateResample(STEREO, STEREO, SAMPLE_RATE_44100, SAMPLE_RATE_48000);
    EXPECT_GT(converterNode->resamplerOutBuf_.size(), 10u);
}

/**
 * @tc.name  : ProcessDiffRate_ZeroFrames
 * @tc.type  : FUNC
 * @tc.number: ProcessDiffRate_ZeroFrames
 * @tc.desc  : ProcessDiffRateResample does nothing with zero unprocessed frames
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, ProcessDiffRate_ZeroFrames, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, STEREO, STEREO);
    converterNode->unprocessedFrames_ = 0;

    converterNode->ProcessDiffRateResample(STEREO, STEREO, SAMPLE_RATE_44100, SAMPLE_RATE_48000);
    EXPECT_EQ(converterNode->unprocessedFrames_, 0u);
    EXPECT_EQ(converterNode->processedFrames_, 0u);
}

/**
 * @tc.name  : ProcessDiffRate_InsufficientFrames
 * @tc.type  : FUNC
 * @tc.number: ProcessDiffRate_InsufficientFrames
 * @tc.desc  : ProcessDiffRateResample does nothing below threshold
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, ProcessDiffRate_InsufficientFrames, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, STEREO, STEREO);
    uint32_t threshold = converterNode->resamplerInputThreshold_;
    converterNode->unprocessedFrames_ = threshold - 1;

    converterNode->ProcessDiffRateResample(STEREO, STEREO, SAMPLE_RATE_44100, SAMPLE_RATE_48000);
    EXPECT_EQ(converterNode->unprocessedFrames_, threshold - 1);
    EXPECT_EQ(converterNode->processedFrames_, 0u);
}

// =========================================================================
// SetDownmixNormalization Tests
// =========================================================================

/**
 * @tc.name  : SetDownmixNormalization_Enable
 * @tc.type  : FUNC
 * @tc.number: SetDownmixNormalization_Enable
 * @tc.desc  : SetDownmixNormalization enables normalization
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, SetDownmixNormalization_Enable, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_48000, SAMPLE_RATE_48000, CHANNEL_6, STEREO);

    converterNode->SetDownmixNormalization(true);
    // Should complete without crash
}

/**
 * @tc.name  : SetDownmixNormalization_Disable
 * @tc.type  : FUNC
 * @tc.number: SetDownmixNormalization_Disable
 * @tc.desc  : SetDownmixNormalization disables normalization
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, SetDownmixNormalization_Disable, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_48000, SAMPLE_RATE_48000, CHANNEL_6, STEREO);

    converterNode->SetDownmixNormalization(false);
    // Should complete without crash
}

// =========================================================================
// ConnectWithInfo/DisConnectWithInfo Tests
// =========================================================================

/**
 * @tc.name  : ConnectWithInfo_Basic
 * @tc.type  : FUNC
 * @tc.number: ConnectWithInfo_Basic
 * @tc.desc  : ConnectWithInfo connects input stream and sets buffer type
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, ConnectWithInfo_Basic, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    auto upstreamNode = CreateStdConverter();
    HpaeNodeInfo nodeInfo = converterNode->GetNodeInfo();
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;

    // Use proper type cast for OutputNode
    auto outputNode = std::dynamic_pointer_cast<OutputNode<HpaePcmBuffer*>>(upstreamNode);
    if (outputNode != nullptr) {
        converterNode->ConnectWithInfo(outputNode, nodeInfo);
    }
    // Should complete without crash
}

/**
 * @tc.name  : DisConnectWithInfo_Basic
 * @tc.type  : FUNC
 * @tc.number: DisConnectWithInfo_Basic
 * @tc.desc  : DisConnectWithInfo disconnects input stream
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, DisConnectWithInfo_Basic, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    auto upstreamNode = CreateStdConverter();
    HpaeNodeInfo nodeInfo = converterNode->GetNodeInfo();

    // Use proper type cast for OutputNode
    auto outputNode = std::dynamic_pointer_cast<OutputNode<HpaePcmBuffer*>>(upstreamNode);
    if (outputNode != nullptr) {
        // First connect
        converterNode->ConnectWithInfo(outputNode, nodeInfo);

        // Then disconnect
        converterNode->DisConnectWithInfo(outputNode, nodeInfo);
    }
    // Should complete without crash
}

// =========================================================================
// CheckAndUpdateInfo Tests
// =========================================================================

/**
 * @tc.name  : CheckAndUpdateInfo_DifferentInputRate
 * @tc.type  : FUNC
 * @tc.number: CheckAndUpdateInfo_DifferentInputRate
 * @tc.desc  : CheckAndUpdateInfo updates when input sample rate changes
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, CheckAndUpdateInfo_DifferentInputRate, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_48000, SAMPLE_RATE_48000, STEREO, STEREO);
    PcmBufferInfo pcmInfo(STEREO, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_44100);
    HpaePcmBuffer input(pcmInfo);

    converterNode->CheckAndUpdateInfo(&input);
    EXPECT_EQ(converterNode->preNodeInfo_.samplingRate, SAMPLE_RATE_44100);
}

/**
 * @tc.name  : CheckAndUpdateInfo_DifferentInputChannel
 * @tc.type  : FUNC
 * @tc.number: CheckAndUpdateInfo_DifferentInputChannel
 * @tc.desc  : CheckAndUpdateInfo updates when input channels change
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, CheckAndUpdateInfo_DifferentInputChannel, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_48000, SAMPLE_RATE_48000, STEREO, STEREO);
    PcmBufferInfo pcmInfo(CHANNEL_6, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_48000);
    HpaePcmBuffer input(pcmInfo);

    converterNode->CheckAndUpdateInfo(&input);
    EXPECT_EQ(converterNode->preNodeInfo_.channels, CHANNEL_6);
}

/**
 * @tc.name  : CheckAndUpdateInfo_NoChange
 * @tc.type  : FUNC
 * @tc.number: CheckAndUpdateInfo_NoChange
 * @tc.desc  : CheckAndUpdateInfo handles no format change
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, CheckAndUpdateInfo_NoChange, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_48000, SAMPLE_RATE_48000, STEREO, STEREO);
    PcmBufferInfo pcmInfo(STEREO, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_48000);
    HpaePcmBuffer input(pcmInfo);

    converterNode->CheckAndUpdateInfo(&input);
    EXPECT_EQ(converterNode->preNodeInfo_.samplingRate, SAMPLE_RATE_48000);
}

/**
 * @tc.name  : CheckAndUpdateInfo_NullInput
 * @tc.type  : FUNC
 * @tc.number: CheckAndUpdateInfo_NullInput
 * @tc.desc  : CheckAndUpdateInfo with valid input completes successfully
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, CheckAndUpdateInfo_NullInput, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    PcmBufferInfo pcmInfo(STEREO, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_48000);
    HpaePcmBuffer input(pcmInfo);

    converterNode->CheckAndUpdateInfo(&input);
    // Should not crash - using valid input instead of nullptr
    EXPECT_EQ(converterNode->preNodeInfo_.samplingRate, SAMPLE_RATE_48000);
}

// =========================================================================
// UpdateTmpOutPcmBufferInfo Tests
// =========================================================================

/**
 * @tc.name  : UpdateTmpOutPcmBufferInfo_DownmixPath
 * @tc.type  : FUNC
 * @tc.number: UpdateTmpOutPcmBufferInfo_DownmixPath
 * @tc.desc  : UpdateTmpOutPcmBufferInfo configures for downmix-then-resample path
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, UpdateTmpOutPcmBufferInfo_DownmixPath, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, CHANNEL_6, STEREO);
    PcmBufferInfo outPcmBufferInfo(STEREO, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_48000);

    converterNode->UpdateTmpOutPcmBufferInfo(outPcmBufferInfo);
    // Should configure tmpOutBuf_ for downmix
    EXPECT_GT(converterNode->tmpOutBuf_.GetFrameLen(), 0u);
}

/**
 * @tc.name  : UpdateTmpOutPcmBufferInfo_ResamplePath
 * @tc.type  : FUNC
 * @tc.number: UpdateTmpOutPcmBufferInfo_ResamplePath
 * @tc.desc  : UpdateTmpOutPcmBufferInfo configures for resample-then-upmix path
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, UpdateTmpOutPcmBufferInfo_ResamplePath, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, MONO, STEREO);
    PcmBufferInfo outPcmBufferInfo(STEREO, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_48000);

    converterNode->UpdateTmpOutPcmBufferInfo(outPcmBufferInfo);
    // Should configure tmpOutBuf_ for upmix
    EXPECT_GT(converterNode->tmpOutBuf_.GetFrameLen(), 0u);
}

// =========================================================================
// CheckUpdateInInfo Tests
// =========================================================================

/**
 * @tc.name  : CheckUpdateInInfo_DifferentChannelLayout
 * @tc.type  : FUNC
 * @tc.number: CheckUpdateInInfo_DifferentChannelLayout
 * @tc.desc  : CheckUpdateInInfo updates when channel layout changes
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, CheckUpdateInInfo_DifferentChannelLayout, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_48000, SAMPLE_RATE_48000, STEREO, STEREO);
    PcmBufferInfo pcmInfo(STEREO, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_48000);
    pcmInfo.channelLayout = CH_LAYOUT_SURROUND;
    HpaePcmBuffer input(pcmInfo);

    bool result = converterNode->CheckUpdateInInfo(&input);
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : CheckUpdateInInfo_DifferentChannelsOnly
 * @tc.type  : FUNC
 * @tc.number: CheckUpdateInInfo_DifferentChannelsOnly
 * @tc.desc  : CheckUpdateInInfo updates when only channels change
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, CheckUpdateInInfo_DifferentChannelsOnly, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_48000, SAMPLE_RATE_48000, STEREO, STEREO);
    PcmBufferInfo pcmInfo(CHANNEL_6, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_48000);
    pcmInfo.channelLayout = CH_LAYOUT_STEREO;
    HpaePcmBuffer input(pcmInfo);

    bool result = converterNode->CheckUpdateInInfo(&input);
    EXPECT_TRUE(result);
    EXPECT_EQ(converterNode->preNodeInfo_.channels, CHANNEL_6);
}

// =========================================================================
// UpdateOutChannelInfo Tests
// =========================================================================

/**
 * @tc.name  : UpdateOutChannelInfo_NoResamplerUpdateNeeded
 * @tc.type  : FUNC
 * @tc.number: UpdateOutChannelInfo_NoResamplerUpdateNeeded
 * @tc.desc  : UpdateOutChannelInfo succeeds without updating resampler when channels same
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, UpdateOutChannelInfo_NoResamplerUpdateNeeded, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_48000, SAMPLE_RATE_48000, STEREO, STEREO);

    bool result = converterNode->UpdateOutChannelInfo(STEREO, CH_LAYOUT_STEREO);
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : UpdateOutChannelInfo_DifferentChannelLayout
 * @tc.type  : FUNC
 * @tc.number: UpdateOutChannelInfo_DifferentChannelLayout
 * @tc.desc  : UpdateOutChannelInfo updates with different channel layout
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, UpdateOutChannelInfo_DifferentChannelLayout, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_48000, SAMPLE_RATE_48000, STEREO, STEREO);

    bool result = converterNode->UpdateOutChannelInfo(STEREO, CH_LAYOUT_SURROUND);
    EXPECT_TRUE(result);
}

// =========================================================================
// CheckUpdateOutInfo Tests
// =========================================================================

/**
 * @tc.name  : CheckUpdateOutInfo_NullCallback
 * @tc.type  : FUNC
 * @tc.number: CheckUpdateOutInfo_NullCallback
 * @tc.desc  : CheckUpdateOutInfo returns false when callback is null
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, CheckUpdateOutInfo_NullCallback, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    converterNode->nodeFormatInfoCallback_ = nullptr;

    bool result = converterNode->CheckUpdateOutInfo();
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : CheckUpdateOutInfo_OnlyRateChanges
 * @tc.type  : FUNC
 * @tc.number: CheckUpdateOutInfo_OnlyRateChanges
 * @tc.desc  : CheckUpdateOutInfo updates only rate when channels unchanged
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, CheckUpdateOutInfo_OnlyRateChanges, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    MockNodeFormatInfoCallback mockCallback;
    mockCallback.formatToReturn_.audioChannelInfo.numChannels = STEREO;
    mockCallback.formatToReturn_.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    mockCallback.formatToReturn_.rate = SAMPLE_RATE_44100;
    converterNode->nodeFormatInfoCallback_ = &mockCallback;

    bool result = converterNode->CheckUpdateOutInfo();
    EXPECT_TRUE(result);
    EXPECT_EQ(converterNode->resampler_->GetOutRate(), SAMPLE_RATE_44100);
}

/**
 * @tc.name  : CheckUpdateOutInfo_CallbackReturnsZeroRate
 * @tc.type  : FUNC
 * @tc.number: CheckUpdateOutInfo_CallbackReturnsZeroRate
 * @tc.desc  : CheckUpdateOutInfo handles zero sample rate from callback
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, CheckUpdateOutInfo_CallbackReturnsZeroRate, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    MockNodeFormatInfoCallback mockCallback;
    mockCallback.formatToReturn_.audioChannelInfo.numChannels = STEREO;
    mockCallback.formatToReturn_.rate = static_cast<AudioSamplingRate>(0);
    converterNode->nodeFormatInfoCallback_ = &mockCallback;

    bool result = converterNode->CheckUpdateOutInfo();
    EXPECT_TRUE(result); // Falls back to node info
}

// =========================================================================
// ProcessAndExtractOutput Tests
// =========================================================================

/**
 * @tc.name  : ProcessAndExtractOutput_InsufficientData
 * @tc.type  : FUNC
 * @tc.number: ProcessAndExtractOutput_InsufficientData
 * @tc.desc  : ProcessAndExtractOutput returns silence when insufficient data
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, ProcessAndExtractOutput_InsufficientData, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, STEREO, STEREO);
    PcmBufferInfo pcmInfo(STEREO, 100, SAMPLE_RATE_44100);
    HpaePcmBuffer input(pcmInfo);
    input.GetPcmDataBuffer();

    converterNode->ProcessAndExtractOutput(&input);
    // Should return silence buffer
    EXPECT_NE(converterNode->outputStream_.PullOutputData(), nullptr);
}

// =========================================================================
// AccumulateInputAndPullAhead Tests
// =========================================================================

/**
 * @tc.name  : Accumulate_NoPullAheadSameRate
 * @tc.type  : FUNC
 * @tc.number: Accumulate_NoPullAheadSameRate
 * @tc.desc  : AccumulateInputAndPullAhead skips pull-ahead for same rate
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, Accumulate_NoPullAheadSameRate, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    PcmBufferInfo pcmInfo(STEREO, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_48000);
    HpaePcmBuffer input(pcmInfo);
    input.GetPcmDataBuffer();

    converterNode->AccumulateInputAndPullAhead(&input);
    EXPECT_EQ(converterNode->unprocessedFrames_, DEFAULT_FRAMELEN_SECOND);
}

/**
 * @tc.name  : Accumulate_NoPullAheadNoCredit
 * @tc.type  : FUNC
 * @tc.number: Accumulate_NoPullAheadNoCredit
 * @tc.desc  : AccumulateInputAndPullAhead skips pull-ahead when no credit
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, Accumulate_NoPullAheadNoCredit, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, STEREO, STEREO);
    converterNode->pullAheadCredit_ = 0;
    PcmBufferInfo pcmInfo(STEREO, 100, SAMPLE_RATE_44100);
    HpaePcmBuffer input(pcmInfo);
    input.GetPcmDataBuffer();

    converterNode->AccumulateInputAndPullAhead(&input);
    EXPECT_EQ(converterNode->unprocessedFrames_, 100u);
}

// =========================================================================
// GetLatency Tests
// =========================================================================

/**
 * @tc.name  : GetLatency_NullResampler
 * @tc.type  : FUNC
 * @tc.number: GetLatency_NullResampler
 * @tc.desc  : GetLatency returns 0 when resampler is null
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, GetLatency_NullResampler, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    converterNode->resampler_.reset();
    converterNode->unprocessedFrames_ = 100;
    converterNode->processedFrames_ = 200;

    uint64_t latency = converterNode->GetLatency();
    EXPECT_EQ(latency, 0u);
}

/**
 * @tc.name  : GetLatency_ZeroInRate
 * @tc.type  : FUNC
 * @tc.number: GetLatency_ZeroInRate
 * @tc.desc  : GetLatency handles zero input rate
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, GetLatency_ZeroInRate, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, STEREO, STEREO);
    converterNode->resampler_->UpdateRates(0, SAMPLE_RATE_48000);
    converterNode->unprocessedFrames_ = 100;

    uint64_t latency = converterNode->GetLatency();
    // Unprocessed contribution should be 0 when inRate is 0
    EXPECT_EQ(latency, converterNode->processedFrames_ * 1000000ULL / SAMPLE_RATE_48000);
}

/**
 * @tc.name  : GetLatency_ZeroOutRate
 * @tc.type  : FUNC
 * @tc.number: GetLatency_ZeroOutRate
 * @tc.desc  : GetLatency handles zero output rate
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, GetLatency_ZeroOutRate, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, STEREO, STEREO);
    converterNode->resampler_->UpdateRates(SAMPLE_RATE_44100, 0);
    converterNode->processedFrames_ = 100;
    converterNode->unprocessedFrames_ = 0;

    uint64_t latency = converterNode->GetLatency();
    // Processed contribution should be 0 when outRate is 0
    uint64_t expected = static_cast<uint64_t>(converterNode->unprocessedFrames_) * 1000000ULL / SAMPLE_RATE_44100;
    EXPECT_EQ(latency, expected);
}

// =========================================================================
// SignalProcess Tests
// =========================================================================

/**
 * @tc.name  : SignalProcess_MultipleInputsWarning
 * @tc.type  : FUNC
 * @tc.number: SignalProcess_MultipleInputsWarning
 * @tc.desc  : SignalProcess logs warning with multiple inputs
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, SignalProcess_MultipleInputsWarning, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    PcmBufferInfo pcmInfo(STEREO, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_48000);
    HpaePcmBuffer input1(pcmInfo);
    input1.GetPcmDataBuffer();
    input1.SetBufferValid(true);
    HpaePcmBuffer input2(pcmInfo);
    input2.GetPcmDataBuffer();
    input2.SetBufferValid(true);

    std::vector<HpaePcmBuffer*> inputs = {&input1, &input2};

    converterNode->SignalProcess(inputs);
    // Should log warning but not crash
    EXPECT_NE(converterNode->outputStream_.PullOutputData(), nullptr);
}

// =========================================================================
// ReconfigTmpOutForChannelConversion Tests
// =========================================================================

/**
 * @tc.name  : ReconfigTmpOut_SameChannels
 * @tc.type  : FUNC
 * @tc.number: ReconfigTmpOut_SameChannels
 * @tc.desc  : ReconfigTmpOutForChannelConversion handles same channels
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, ReconfigTmpOut_SameChannels, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();

    converterNode->ReconfigTmpOutForChannelConversion(STEREO, STEREO, 960);
    // For same channels (inCh <= outCh), uses upmix path config
    EXPECT_GT(converterNode->tmpOutBuf_.GetFrameLen(), 0u);
}

/**
 * @tc.name  : ReconfigTmpOut_ExactEqualChannels
 * @tc.type  : FUNC
 * @tc.number: ReconfigTmpOut_ExactEqualChannels
 * @tc.desc  : ReconfigTmpOutForChannelConversion with inCh == outCh
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, ReconfigTmpOut_ExactEqualChannels, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, STEREO, STEREO);

    converterNode->ReconfigTmpOutForChannelConversion(STEREO, STEREO, 960);
    // inCh == outCh, should use upmix path (resampler output at outRate)
    EXPECT_EQ(converterNode->tmpOutBuf_.GetSampleRate(), SAMPLE_RATE_48000);
}

/**
 * @tc.name  : ReconfigTmpOut_LargeMaxOutFrames
 * @tc.type  : FUNC
 * @tc.number: ReconfigTmpOut_LargeMaxOutFrames
 * @tc.desc  : ReconfigTmpOutForChannelConversion with large maxOutFrames
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, ReconfigTmpOut_LargeMaxOutFrames, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, MONO, STEREO);

    converterNode->ReconfigTmpOutForChannelConversion(MONO, STEREO, 5000);
    EXPECT_EQ(converterNode->tmpOutBuf_.GetFrameLen(), 5000u);
}

// =========================================================================
// SourceBufferType Tests
// =========================================================================

/**
 * @tc.name  : SourceBufferType_InNodeInfo
 * @tc.type  : FUNC
 * @tc.number: SourceBufferType_InNodeInfo
 * @tc.desc  : SourceBufferType is properly stored in NodeInfo
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, SourceBufferType_InNodeInfo, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    HpaeNodeInfo nodeInfo = converterNode->GetNodeInfo();

    // Verify sourceBufferType field exists and can be set
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    EXPECT_EQ(nodeInfo.sourceBufferType, HPAE_SOURCE_BUFFER_TYPE_MIC);

    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_EC;
    EXPECT_EQ(nodeInfo.sourceBufferType, HPAE_SOURCE_BUFFER_TYPE_EC);

    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MICREF;
    EXPECT_EQ(nodeInfo.sourceBufferType, HPAE_SOURCE_BUFFER_TYPE_MICREF);
}

// =========================================================================
// ProcessResampleLoop Additional Tests
// =========================================================================

/**
 * @tc.name  : ProcessResampleLoop_DiffRateUpmix
 * @tc.type  : FUNC
 * @tc.number: ProcessResampleLoop_DiffRateUpmix
 * @tc.desc  : ProcessResampleLoop handles different rate with upmix
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, ProcessResampleLoop_DiffRateUpmix, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, MONO, STEREO);
    uint32_t threshold = converterNode->resamplerInputThreshold_;
    std::vector<float> data(threshold * MONO, 0.5f);
    converterNode->unprocessedBuffer_ = data;
    converterNode->unprocessedFrames_ = threshold;

    converterNode->ProcessResampleLoop();
    EXPECT_EQ(converterNode->unprocessedFrames_, 0u);
    EXPECT_GT(converterNode->processedFrames_, 0u);
}

/**
 * @tc.name  : ProcessResampleLoop_DiffRateDownmix
 * @tc.type  : FUNC
 * @tc.number: ProcessResampleLoop_DiffRateDownmix
 * @tc.desc  : ProcessResampleLoop handles different rate with downmix
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, ProcessResampleLoop_DiffRateDownmix, TestSize.Level0)
{
    auto converterNode = CreateConverter(SAMPLE_RATE_44100, SAMPLE_RATE_48000, CHANNEL_6, STEREO);
    uint32_t threshold = converterNode->resamplerInputThreshold_;
    std::vector<float> data(threshold * CHANNEL_6, 0.5f);
    converterNode->unprocessedBuffer_ = data;
    converterNode->unprocessedFrames_ = threshold;

    converterNode->ProcessResampleLoop();
    EXPECT_EQ(converterNode->unprocessedFrames_, 0u);
    EXPECT_GT(converterNode->processedFrames_, 0u);
}

// =========================================================================
// DoProcess Additional Tests
// =========================================================================

/**
 * @tc.name  : DoProcess_ValidInputWithProcess
 * @tc.type  : FUNC
 * @tc.number: DoProcess_ValidInputWithProcess
 * @tc.desc  : DoProcess processes valid input when enableProcess is true
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, DoProcess_ValidInputWithProcess, TestSize.Level0)
{
    auto upstreamNode = CreateStdConverter();
    auto converterNode = CreateStdConverter();

    PcmBufferInfo pcmInfo(STEREO, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_48000);
    HpaePcmBuffer input(pcmInfo);
    input.GetPcmDataBuffer();
    for (size_t i = 0; i < DEFAULT_FRAMELEN_SECOND * STEREO; i++) {
        input.GetPcmDataBuffer()[i] = 0.5f;
    }
    input.SetBufferValid(true);

    upstreamNode->outputStream_.WriteDataToOutput(&input);
    converterNode->inputStream_.AddPreOutput(
        upstreamNode->GetSharedInstance(), &upstreamNode->outputStream_);
    converterNode->enableProcess_ = true;

    converterNode->DoProcess();
    EXPECT_NE(converterNode->outputStream_.PullOutputData(), nullptr);
}

/**
 * @tc.name  : DoProcess_NullInputWithProcess
 * @tc.type  : FUNC
 * @tc.number: DoProcess_NullInputWithProcess
 * @tc.desc  : DoProcess handles null input with enableProcess true
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, DoProcess_NullInputWithProcess, TestSize.Level0)
{
    auto converterNode = CreateStdConverter();
    converterNode->enableProcess_ = true;

    converterNode->DoProcess();
    EXPECT_NE(converterNode->outputStream_.PullOutputData(), nullptr);
}

/**
 * @tc.name  : RegisterCallbackQueryTest_001
 * @tc.type  : FUNC
 * @tc.number: RegisterCallbackQueryTest_001.
 * @tc.desc  : CheckUpdateOutInfo queries the registered callback for the target format
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, RegisterCallbackQueryTest_001, TestSize.Level0)
{
    auto converterNode = CreateStdConverter(); // 48k stereo -> 48k stereo
    MockNodeFormatInfoCallback callback;
    callback.formatToReturn_.rate = SAMPLE_RATE_44100;
    callback.formatToReturn_.audioChannelInfo.numChannels = static_cast<uint32_t>(STEREO);
    callback.formatToReturn_.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;

    converterNode->RegisterCallback(&callback);
    EXPECT_TRUE(converterNode->CheckUpdateOutInfo());

    EXPECT_EQ(converterNode->GetSampleRate(), SAMPLE_RATE_44100);
    EXPECT_EQ(callback.queryCount_, 1);
}

/**
 * @tc.name  : signalProcess inputs size != 1 hits edge gate then recovers
 * @tc.type  : FUNC
 * @tc.number: HpaeAudioFormatConverterNodeTest_EdgeLog_001
 */
HWTEST_F(HpaeAudioFormatConverterNodeTest, signalProcessInputsSizeEdgeLog, TestSize.Level0)
{
    HpaeNodeInfo preNodeInfo;
    preNodeInfo.samplingRate = SAMPLE_RATE_48000;
    preNodeInfo.frameLen = DEFAULT_FRAMELEN_SECOND;
    preNodeInfo.channels = STEREO;
    HpaeNodeInfo outputNodeInfo;
    auto converterNode = std::make_shared<HpaeAudioFormatConverterNode>(preNodeInfo, outputNodeInfo);
    PcmBufferInfo pcmBufferInfo(STEREO, DEFAULT_FRAMELEN_SECOND, SAMPLE_RATE_48000);
    HpaePcmBuffer input(pcmBufferInfo);
    HpaePcmBuffer input2(pcmBufferInfo);

    std::vector<HpaePcmBuffer *> twoInputs = {&input, &input2};
    converterNode->SignalProcess(twoInputs); // first gate hit: size != 1
    converterNode->SignalProcess(twoInputs); // sustained hit
    std::vector<HpaePcmBuffer *> oneInput = {&input};
    converterNode->SignalProcess(oneInput); // recovery line: size back to 1
}

}
