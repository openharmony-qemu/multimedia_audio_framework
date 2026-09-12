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

/*
 * Unit tests for the HOA (ambisonic order encoded layout) guard branches added to
 * RendererInClientInner::SetAudioStreamInfo:
 *   - D1: HOA layout whose order does not match the channel count is rejected with
 *     ERR_INVALID_PARAM before any converter config IPC.
 *   - D4: HOA stream whose MCR converter failed to load the algo library is rejected
 *     with ERR_NOT_SUPPORTED and converter_ is reset to nullptr; non-HOA streams only
 *     log a warning and keep the passthrough converter.
 *
 * Environment notes:
 *   - D4 cases rely on the UT environment having no MCR algo library deployed under
 *     /system/lib64(32), so AudioSpatialChannelConverter::Init() ends with
 *     loadSuccess_ == false, and on AudioPolicyManager::GetConverterConfig() degrading
 *     to an empty config when the audio policy service is absent.
 *   - The D4 cases probe the image with the same load chain as production
 *     (GetConverterConfig -> converter Init -> IsAlgoLoaded) and skip via GTEST_SKIP
 *     on images where the MCR algo is actually loaded, so product images shipping the
 *     algo do not turn these negative cases red.
 *   - The non-HOA case only asserts that the D1/D4 guards do not misfire: the code
 *     after the guards reaches InitIpcStream() whose result depends on whether the
 *     audio service is available in the test environment, so no exact value is pinned.
 */

#include <memory>
#include <unistd.h>

#include <gtest/gtest.h>
#include "audio_errors.h"
#include "audio_policy_manager.h"
#include "audio_spatial_channel_converter.h"
#include "renderer_in_client_private.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class RendererInClientHoaUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void RendererInClientHoaUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void RendererInClientHoaUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void RendererInClientHoaUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void RendererInClientHoaUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

static AudioStreamParams BuildAudioVividParams(AudioChannelLayout channelLayout, uint8_t channels)
{
    AudioStreamParams info;
    info.format = AudioSampleFormat::SAMPLE_F32LE;
    info.encoding = AudioEncodingType::ENCODING_AUDIOVIVID;
    info.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    info.channels = channels;
    info.channelLayout = channelLayout;
    return info;
}

// Probe whether this image actually loads the MCR algo library, using the same chain
// as the production D4 decision (GetConverterConfig -> converter Init -> IsAlgoLoaded).
static bool IsMcrAlgoLoadedOnThisImage()
{
    ConverterConfig cfg = AudioPolicyManager::GetInstance().GetConverterConfig();
    AudioSpatialChannelConverter probe;
    if (!probe.Init(BuildAudioVividParams(AudioChannelLayout::CH_LAYOUT_HOA_ORDER2_ACN_N3D,
        AudioChannel::CHANNEL_9), cfg)) {
        return false;
    }
    return probe.IsAlgoLoaded();
}

static void SkipIfMcrAlgoLoaded()
{
    if (IsMcrAlgoLoadedOnThisImage()) {
        GTEST_SKIP() << "MCR algo deployed on this image; D4 hard-fail case not valid here";
    }
}

/**
 * @tc.name  : Test SetAudioStreamInfo HOA D1 guard API
 * @tc.type  : FUNC
 * @tc.number: SetAudioStreamInfo_HoaD1Reject_001
 * @tc.desc  : Test SetAudioStreamInfo rejects a second order HOA layout with mismatched
 *             stereo channel count.
 */
HWTEST(RendererInClientHoaUnitTest, SetAudioStreamInfo_HoaD1Reject_001, TestSize.Level1)
{
    auto rendererInClient = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    ASSERT_TRUE(rendererInClient != nullptr);

    AudioStreamParams info = BuildAudioVividParams(AudioChannelLayout::CH_LAYOUT_HOA_ORDER2_ACN_N3D,
        AudioChannel::STEREO);
    std::shared_ptr<AudioClientTracker> proxyObj;
    int32_t ret = rendererInClient->SetAudioStreamInfo(info, proxyObj);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
    EXPECT_EQ(rendererInClient->converter_, nullptr);
}

/**
 * @tc.name  : Test SetAudioStreamInfo HOA D1 guard API
 * @tc.type  : FUNC
 * @tc.number: SetAudioStreamInfo_HoaD1Reject_002
 * @tc.desc  : Test SetAudioStreamInfo rejects third order layout with 9 channels and first
 *             order layout with 9 channels.
 */
HWTEST(RendererInClientHoaUnitTest, SetAudioStreamInfo_HoaD1Reject_002, TestSize.Level1)
{
    auto rendererInClient = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    ASSERT_TRUE(rendererInClient != nullptr);

    std::shared_ptr<AudioClientTracker> proxyObj;
    AudioStreamParams thirdOrderInfo = BuildAudioVividParams(AudioChannelLayout::CH_LAYOUT_HOA_ORDER3_ACN_N3D,
        AudioChannel::CHANNEL_9);
    int32_t ret = rendererInClient->SetAudioStreamInfo(thirdOrderInfo, proxyObj);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    AudioStreamParams firstOrderInfo = BuildAudioVividParams(AudioChannelLayout::CH_LAYOUT_HOA_ORDER1_ACN_N3D,
        AudioChannel::CHANNEL_9);
    ret = rendererInClient->SetAudioStreamInfo(firstOrderInfo, proxyObj);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
    EXPECT_EQ(rendererInClient->converter_, nullptr);
}

/**
 * @tc.name  : Test SetAudioStreamInfo HOA D4 guard API
 * @tc.type  : FUNC
 * @tc.number: SetAudioStreamInfo_HoaD4HardFail_001
 * @tc.desc  : Test SetAudioStreamInfo returns ERR_NOT_SUPPORTED and drops the converter for
 *             a matched second order HOA stream when the MCR algo is not loaded.
 */
HWTEST(RendererInClientHoaUnitTest, SetAudioStreamInfo_HoaD4HardFail_001, TestSize.Level1)
{
    SkipIfMcrAlgoLoaded();
    auto rendererInClient = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    ASSERT_TRUE(rendererInClient != nullptr);

    AudioStreamParams info = BuildAudioVividParams(AudioChannelLayout::CH_LAYOUT_HOA_ORDER2_ACN_N3D,
        AudioChannel::CHANNEL_9);
    std::shared_ptr<AudioClientTracker> proxyObj;
    int32_t ret = rendererInClient->SetAudioStreamInfo(info, proxyObj);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
    EXPECT_EQ(rendererInClient->converter_, nullptr);
}

/**
 * @tc.name  : Test SetAudioStreamInfo HOA D4 guard API
 * @tc.type  : FUNC
 * @tc.number: SetAudioStreamInfo_HoaD4HardFail_002
 * @tc.desc  : Test SetAudioStreamInfo returns ERR_NOT_SUPPORTED for matched first order with
 *             4 channels and third order with 16 channels HOA streams.
 */
HWTEST(RendererInClientHoaUnitTest, SetAudioStreamInfo_HoaD4HardFail_002, TestSize.Level1)
{
    SkipIfMcrAlgoLoaded();
    auto rendererInClient = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    ASSERT_TRUE(rendererInClient != nullptr);

    std::shared_ptr<AudioClientTracker> proxyObj;
    AudioStreamParams firstOrderInfo = BuildAudioVividParams(AudioChannelLayout::CH_LAYOUT_HOA_ORDER1_ACN_N3D,
        AudioChannel::CHANNEL_4);
    int32_t ret = rendererInClient->SetAudioStreamInfo(firstOrderInfo, proxyObj);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
    EXPECT_EQ(rendererInClient->converter_, nullptr);

    AudioStreamParams thirdOrderInfo = BuildAudioVividParams(AudioChannelLayout::CH_LAYOUT_HOA_ORDER3_ACN_N3D,
        AudioChannel::CHANNEL_16);
    ret = rendererInClient->SetAudioStreamInfo(thirdOrderInfo, proxyObj);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
    EXPECT_EQ(rendererInClient->converter_, nullptr);
}

/**
 * @tc.name  : Test SetAudioStreamInfo non-HOA AudioVivid API
 * @tc.type  : FUNC
 * @tc.number: SetAudioStreamInfo_NonHoaAudioVivid_001
 * @tc.desc  : Test SetAudioStreamInfo keeps the converter and skips the D1 rejection for a
 *             non-HOA stereo AudioVivid stream. Only the guard behaviour is pinned because
 *             the continuation path depends on audio service availability.
 */
HWTEST(RendererInClientHoaUnitTest, SetAudioStreamInfo_NonHoaAudioVivid_001, TestSize.Level1)
{
    auto rendererInClient = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    ASSERT_TRUE(rendererInClient != nullptr);

    AudioStreamParams info = BuildAudioVividParams(AudioChannelLayout::CH_LAYOUT_STEREO, AudioChannel::STEREO);
    std::shared_ptr<AudioClientTracker> proxyObj;
    int32_t ret = rendererInClient->SetAudioStreamInfo(info, proxyObj);
    EXPECT_NE(ret, ERR_INVALID_PARAM);
    EXPECT_NE(rendererInClient->converter_, nullptr);
}

/**
 * @tc.name  : Test SetAudioStreamInfo converter construct failure API
 * @tc.type  : FUNC
 * @tc.number: SetAudioStreamInfo_ConverterConstructFail_001
 * @tc.desc  : Test SetAudioStreamInfo returns ERR_NOT_SUPPORTED when the converter fails to
 *             construct on an unsupported sample format; the converter object is kept, which
 *             distinguishes this branch from the D4 algo-unavailable guard.
 */
HWTEST(RendererInClientHoaUnitTest, SetAudioStreamInfo_ConverterConstructFail_001, TestSize.Level1)
{
    auto rendererInClient = std::make_shared<RendererInClientInner>(AudioStreamType::STREAM_MUSIC, getuid());
    ASSERT_TRUE(rendererInClient != nullptr);

    AudioStreamParams info = BuildAudioVividParams(AudioChannelLayout::CH_LAYOUT_STEREO, AudioChannel::STEREO);
    info.format = AudioSampleFormat::INVALID_WIDTH;
    std::shared_ptr<AudioClientTracker> proxyObj;
    int32_t ret = rendererInClient->SetAudioStreamInfo(info, proxyObj);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
    EXPECT_NE(rendererInClient->converter_, nullptr);
}
} // namespace AudioStandard
} // namespace OHOS
