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
#include "cabin_audio_render_sink.h"
#include "audio_errors.h"
#include "parameter.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class CabinAudioRenderSinkUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() override
    {
        sink_ = std::make_unique<CabinAudioRenderSink>();
    }
    void TearDown() override
    {
        sink_ = nullptr;
    }
   
    std::unique_ptr<CabinAudioRenderSink> sink_;
};

/**
* @tc.name   : Init_001
* @tc.type   : FUNC
* @tc.desc   : Test initialization logic when 3DA direct test flag is enabled.
*/
HWTEST_F(CabinAudioRenderSinkUnitTest, Init_001, TestSize.Level1)
{
    IAudioSinkAttr attr = {};
    attr.adapterName = "primary";
    attr.sampleRate = 48000;
    attr.channel = 8;
    attr.format = SAMPLE_S16LE;
    attr.channelLayout = 1551;
    attr.deviceType = 2;
    attr.volume = 1.0f;
    attr.openMicSpeaker = 1;
    attr.encodingType = ENCODING_AUDIOVIVID_3DA_DIRECT;
    sink_->direct3DATestFlag = 1;

    int32_t ret = sink_->Init(attr);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(sink_->IsInited());
}

/**
* @tc.name   : Start_Stop_001
* @tc.type   : FUNC
* @tc.desc   : Test Start and Stop flow with testFlag enabled.
*/
HWTEST_F(CabinAudioRenderSinkUnitTest, Start_Stop_001, TestSize.Level1)
{
    IAudioSinkAttr attr = {};
    attr.adapterName = "primary";
    attr.sampleRate = 48000;
    attr.channel = 8;
    attr.format = SAMPLE_S16LE;
    attr.channelLayout = 1551;
    attr.deviceType = 2;
    attr.volume = 1.0f;
    attr.openMicSpeaker = 1;
    attr.encodingType = ENCODING_AUDIOVIVID_3DA_DIRECT;
    sink_->direct3DATestFlag = 1;
    sink_->Init(attr);

    EXPECT_EQ(sink_->Start(), SUCCESS);
    EXPECT_EQ(sink_->Start(), SUCCESS);

    EXPECT_EQ(sink_->Stop(), SUCCESS);
}

/**
* @tc.name   : RenderFrame_001
* @tc.type   : FUNC
* @tc.desc   : Test RenderFrame branch when testFlag is enabled（Mock path）
*/
HWTEST_F(CabinAudioRenderSinkUnitTest, RenderFrame_001, TestSize.Level1)
{
    IAudioSinkAttr attr = {};
    attr.adapterName = "primary";
    attr.sampleRate = 48000;
    attr.channel = 8;
    attr.format = SAMPLE_S16LE;
    attr.channelLayout = 1551;
    attr.deviceType = 2;
    attr.volume = 1.0f;
    attr.openMicSpeaker = 1;
    attr.encodingType = ENCODING_AUDIOVIVID_3DA_DIRECT;
    sink_->direct3DATestFlag = 1;

    sink_->Init(attr);
    sink_->Start();

    char testData = 'X';
    uint64_t len = 1024;
    uint64_t writeLen = 0;

    int32_t ret = sink_->RenderFrame(testData, len, writeLen);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name   : RenderFrame_002
* @tc.type   : FUNC
* @tc.desc   : Test RenderFrame branch when testFlag is disabled (Normal path)
*/
HWTEST_F(CabinAudioRenderSinkUnitTest, RenderFrame_002, TestSize.Level1)
{
    IAudioSinkAttr attr = {};
    attr.adapterName = "primary";
    attr.sampleRate = 48000;
    attr.channel = 8;
    attr.format = SAMPLE_S16LE;
    attr.channelLayout = 1551;
    attr.deviceType = 2;
    attr.volume = 1.0f;
    attr.openMicSpeaker = 1;
    attr.encodingType = ENCODING_AUDIOVIVID_3DA_DIRECT;
    sink_->direct3DATestFlag = 0;

    sink_->Init(attr);

    char testData = 0;
    uint64_t writeLen = 0;

    int32_t ret = sink_->RenderFrame(testData, 100, writeLen);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name   : FormatConversion_Test
* @tc.type   : FUNC
* @tc.desc   : Validate internal helper functions for format and bit depth conversion
*/
HWTEST_F(CabinAudioRenderSinkUnitTest, FormatConversion_Test, TestSize.Level1)
{
    EXPECT_EQ(sink_->ConvertToHdiFormat(SAMPLE_S16LE), AUDIO_FORMAT_TYPE_PCM_16_BIT);
    EXPECT_EQ(sink_->ConvertToHdiFormat(SAMPLE_S32LE), AUDIO_FORMAT_TYPE_PCM_16_BIT);

    EXPECT_EQ(sink_->PcmFormatToBit(SAMPLE_S16LE), 16);
    EXPECT_EQ(sink_->PcmFormatToBit(SAMPLE_S24LE), 24);
    EXPECT_EQ(sink_->PcmFormatToBit(SAMPLE_S32LE), 16);
}

/**
* @tc.name   : Misc_Interface_Test
* @tc.type   : FUNC
* @tc.desc   : Test getter methods and unsupported interfaces.
*/
HWTEST_F(CabinAudioRenderSinkUnitTest, Misc_Interface_Test, TestSize.Level1)
{
    uint64_t transactionId = 0;
    EXPECT_EQ(sink_->GetTransactionId(transactionId), SUCCESS);
    EXPECT_GT(transactionId, 0);

    EXPECT_EQ(sink_->Resume(), ERR_NOT_SUPPORTED);

    float left, right;
    EXPECT_EQ(sink_->GetVolume(left, right), SUCCESS);
}

/**
* @tc.name   : Pause_001
* @tc.type   : FUNC
* @tc.desc   : Test Pause interface to ensure state transition from started to paused
*/
HWTEST_F(CabinAudioRenderSinkUnitTest, Pause_001, TestSize.Level1)
{
    IAudioSinkAttr attr = {};
    attr.adapterName = "primary";
    attr.sampleRate = 48000;
    attr.channel = 8;
    attr.format = SAMPLE_S16LE;
    attr.channelLayout = 1551;
    attr.deviceType = 2;
    attr.volume = 1.0f;
    attr.openMicSpeaker = 1;
    attr.encodingType = ENCODING_AUDIOVIVID_3DA_DIRECT;
    sink_->direct3DATestFlag = 1;

    sink_->Init(attr);
    sink_->Start();
    int32_t ret = sink_->Pause();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(sink_->Stop(), SUCCESS);
}

/**
* @tc.name   : GetPresentationPosition_001
* @tc.type   : FUNC
* @tc.desc   : Test GetPresentationPosition to ensure it returns 0 as currently implemented
*/
HWTEST_F(CabinAudioRenderSinkUnitTest, GetPresentationPosition_001, TestSize.Level1)
{
    uint64_t frames = 0;
    int64_t timeSec = 0;
    int64_t timeNanoSec = 0;

    EXPECT_EQ(sink_->GetPresentationPosition(frames, timeSec, timeNanoSec), 0);
}

/**
* @tc.name   : SetMuteAndBalance_001
* @tc.type   : FUNC
* @tc.desc   : Test mute and Balance APIs (testing coverage for unsupported/empty function)
*/
HWTEST_F(CabinAudioRenderSinkUnitTest, SetMuteAndBalance_001, TestSize.Level1)
{
    // Test SetSinkMuteForSwitchDevice
    EXPECT_EQ(sink_->SetSinkMuteForSwitchDevice(true), ERR_NOT_SUPPORTED);

    // Test SetAudioBalanceValue and SetAudioMonoState(empty, just call for coverage)
    sink_->SetAudioBalanceValue(1.0f);
    sink_->SetAudioMonoState(true);
    int64_t volumeData = 0;
    EXPECT_EQ(sink_->GetVolumeDataCount(volumeData), ERR_NOT_SUPPORTED);
    sink_->SetSpeed(1.0f);
    EXPECT_EQ(sink_->Reset(), SUCCESS);
    sink_->GetMaxAmplitude();
    EXPECT_EQ(sink_->Flush(), SUCCESS);
}

/**
* @tc.name   : UpdateAppsUid_001
* @tc.type   : FUNC
* @tc.desc   : Test UpdateAppsUid with array and vector inputs
*/
HWTEST_F(CabinAudioRenderSinkUnitTest, UpdateAppsUid_001, TestSize.Level1)
{
    int32_t appsUid[2] = {1001, 1002};
    size_t size = 2;

    int32_t retArray = sink_->UpdateAppsUid(appsUid, size);
#ifdef FEATURE_POWER_MANAGER
    EXPECT_EQ(retArray, ERR_INVALID_HANDLE);
#else
    EXPECT_EQ(retArray, SUCCESS);
#endif

    std::vector<int32_t> appsUidVec = {1003, 1004};
    int32_t retVec = sink_->UpdateAppsUid(appsUidVec);
#ifdef FEATURE_POWER_MANAGER
    EXPECT_EQ(retVec, ERR_INVALID_HANDLE);
#else
    EXPECT_EQ(retVec, SUCCESS);
#endif
}

/**
* @tc.name   : DumpInfo_001
* @tc.type   : FUNC
* @tc.desc   : Test DumpInfo to ensure string formatting is correct
*/
HWTEST_F(CabinAudioRenderSinkUnitTest, DumpInfo_001, TestSize.Level1)
{
    std::string dumpString = "";
    sink_->started_ = true;
    sink_->DumpInfo(dumpString);
    
    EXPECT_NE(dumpString.find("3dadirectSink"), std::string::npos);
    EXPECT_NE(dumpString.find("started: true"), std::string::npos);
}

/**
* @tc.name   : DeInit_flagZero_001
* @tc.type   : FUNC
* @tc.desc   : Test DeInit when direct3DATestFlag is 0
*/
HWTEST_F(CabinAudioRenderSinkUnitTest, DeInit_flagZero_001, TestSize.Level1)
{
    auto sink = std::make_shared<CabinAudioRenderSink>();
    sink->sinkInited_ = true;
    sink->started_ = true;
    sink->direct3DATestFlag = 0;
    sink->attr_.adapterName = "primary";
    sink->hdiRenderId_ = 100;

    sink->DeInit();
    EXPECT_FALSE(sink->sinkInited_);
    EXPECT_FALSE(sink->started_);
    EXPECT_EQ(sink->audioRender_, nullptr);
}

/**
* @tc.name   : stop_001
* @tc.type   : FUNC
* @tc.desc   : Test stop when direct3DATestFlag is 0
*/
HWTEST_F(CabinAudioRenderSinkUnitTest, Stop_001, TestSize.Level1)
{
    auto sink = std::make_shared<CabinAudioRenderSink>();
    sink->started_ = true;
    sink->direct3DATestFlag = 0;

    int32_t ret = sink->Stop();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_FALSE(sink->started_);
}
}
}