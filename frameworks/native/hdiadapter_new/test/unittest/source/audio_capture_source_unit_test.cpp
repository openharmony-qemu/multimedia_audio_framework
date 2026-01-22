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

#include <iostream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "audio_utils.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include "source/audio_capture_source.h"
#include "capturer_clock_manager.h"
#include "audio_stream_enum.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

const uint32_t DEFAULT_SIZE = 8;
const uint32_t UNIQUE_ID_INTERVAL = 8;
static const uint32_t TEST_CAPUTRE_ID = 1;
static const uint32_t TEST_STREAM_ID = 100000;

class AudioCaptureSourceUnitTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() {}
    virtual void TearDown() {}

    void InitPrimarySource();
    void DeInitPrimarySource();
    void InitUsbSource();
    void DeInitUsbSource();

    static int32_t AudioCaptureCaptureFrameEc001(
        struct IAudioCapture *self, const struct AudioFrameLen *frameLen, struct AudioCaptureFrameInfo *frameInfo);
    static int32_t AudioCaptureCaptureFrameEc002(
        struct IAudioCapture *self, const struct AudioFrameLen *frameLen, struct AudioCaptureFrameInfo *frameInfo);
    static int32_t AudioCaptureCaptureFrameEc003(
        struct IAudioCapture *self, const struct AudioFrameLen *frameLen, struct AudioCaptureFrameInfo *frameInfo);
protected:
    static uint32_t primaryId_;
    static uint32_t usbId_;
    static std::shared_ptr<IAudioCaptureSource> primarySource_;
    static std::shared_ptr<IAudioCaptureSource> usbSource_;
    static std::shared_ptr<AudioCapturerSourceClock> audioSrcClock_;
    static IAudioSourceAttr attr_;
};

uint32_t AudioCaptureSourceUnitTest::primaryId_ = 0;
uint32_t AudioCaptureSourceUnitTest::usbId_ = 0;
std::shared_ptr<IAudioCaptureSource> AudioCaptureSourceUnitTest::primarySource_ = nullptr;
std::shared_ptr<IAudioCaptureSource> AudioCaptureSourceUnitTest::usbSource_ = nullptr;
std::shared_ptr<AudioCapturerSourceClock> AudioCaptureSourceUnitTest::audioSrcClock_ = nullptr;
IAudioSourceAttr AudioCaptureSourceUnitTest::attr_ = {};

void AudioCaptureSourceUnitTest::SetUpTestCase()
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    primaryId_ = manager.GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_DEFAULT, true);
    usbId_ = manager.GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_USB, true);
}

void AudioCaptureSourceUnitTest::TearDownTestCase()
{
    HdiAdapterManager::GetInstance().ReleaseId(primaryId_);
    HdiAdapterManager::GetInstance().ReleaseId(usbId_);
}

void AudioCaptureSourceUnitTest::InitPrimarySource()
{
    primarySource_ = HdiAdapterManager::GetInstance().GetCaptureSource(primaryId_, true);
    if (primarySource_ == nullptr) {
        return;
    }
    attr_.adapterName = "primary";
    attr_.sampleRate = 48000; // 48000: sample rate
    attr_.channel = 2; // 2: channel
    attr_.format = SAMPLE_S16LE;
    attr_.channelLayout = CH_LAYOUT_UNKNOWN;
    attr_.deviceType = DEVICE_TYPE_MIC;
    attr_.openMicSpeaker = 1;
    primarySource_->Init(attr_);
}

void AudioCaptureSourceUnitTest::DeInitPrimarySource()
{
    if (primarySource_ && primarySource_->IsInited()) {
        primarySource_->DeInit();
    }
    primarySource_ = nullptr;
}

void AudioCaptureSourceUnitTest::InitUsbSource()
{
    usbSource_ = HdiAdapterManager::GetInstance().GetCaptureSource(usbId_, true);
    if (usbSource_ == nullptr) {
        return;
    }
    attr_.adapterName = "usb";
    attr_.channel = 2; // 2: channel
    usbSource_->Init(attr_);
}

void AudioCaptureSourceUnitTest::DeInitUsbSource()
{
    if (usbSource_ && usbSource_->IsInited()) {
        usbSource_->DeInit();
    }
    usbSource_ = nullptr;
}

int32_t AudioCaptureSourceUnitTest::AudioCaptureCaptureFrameEc001(
    struct IAudioCapture *self, const struct AudioFrameLen *frameLen, struct AudioCaptureFrameInfo *frameInfo)
{
    return 0;
}

int32_t AudioCaptureSourceUnitTest::AudioCaptureCaptureFrameEc002(
    struct IAudioCapture *self, const struct AudioFrameLen frameLen, struct AudioCaptureFrameInfo frameInfo)
{
    int8_t frameEc = nullptr;
    int8_t frame = nullptr;
    frameEc = new int8_t[DEFAULT_SIZE];
    frame = new int8_t[6]; // 6: frame
    frameInfo->frameEc = frameEc;
    frameInfo->frame = frame;
    frameInfo->replyBytes = DEFAULT_SIZE;
    return 0;
}

int32_t AudioCaptureSourceUnitTest::AudioCaptureCaptureFrameEc003(
    struct IAudioCapture self, const struct AudioFrameLen frameLen, struct AudioCaptureFrameInfo frameInfo)
{
    int8_t frameEc = nullptr;
    int8_t frame = nullptr;
    frameEc = new int8_t[DEFAULT_SIZE];
    frame = new int8_t[DEFAULT_SIZE];
    frameInfo->frameEc = frameEc;
    frameInfo->frame = frame;
    frameInfo->replyBytes = 1024; // 1024: replyBytes
    return 0;
}

/**
 * @tc.name   : Test PrimarySource API
 * @tc.number : PrimarySourceUnitTest_001
 * @tc.desc   : Test primary source create
 */
HWTEST_F(AudioCaptureSourceUnitTest, PrimarySourceUnitTest_001, TestSize.Level1)
{
    InitPrimarySource();
    EXPECT_TRUE(primarySource_ != nullptr);
    DeInitPrimarySource();
}

/**
 * @tc.name   : Test PrimarySource API
 * @tc.number : PrimarySourceUnitTest_002
 * @tc.desc   : Test primary source init
 */
HWTEST_F(AudioCaptureSourceUnitTest, PrimarySourceUnitTest_002, TestSize.Level1)
{
    InitPrimarySource();
    EXPECT_TRUE(primarySource_ && primarySource_->IsInited());
    DeInitPrimarySource();
}

/**
 * @tc.name   : Test PrimarySource API
 * @tc.number : PrimarySourceUnitTest_003
 * @tc.desc   : Test primary source deinit
 */
HWTEST_F(AudioCaptureSourceUnitTest, PrimarySourceUnitTest_003, TestSize.Level1)
{
    InitPrimarySource();
    EXPECT_TRUE(primarySource_ && primarySource_->IsInited());
    primarySource_->DeInit();
    int32_t ret = primarySource_->Init(attr_);
    EXPECT_EQ(ret, SUCCESS);
    ret = primarySource_->Init(attr_);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(primarySource_->IsInited());
    DeInitPrimarySource();
}

/**
 * @tc.name   : Test PrimarySource API
 * @tc.number : PrimarySourceUnitTest_004
 * @tc.desc   : Test primary source start, stop, resume, pause, flush, reset
 */
HWTEST_F(AudioCaptureSourceUnitTest, PrimarySourceUnitTest_004, TestSize.Level1)
{
    InitPrimarySource();
    EXPECT_TRUE(primarySource_ && primarySource_->IsInited());
    int32_t ret = primarySource_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = primarySource_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    ret = primarySource_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = primarySource_->Resume();
    EXPECT_EQ(ret, SUCCESS);
    ret = primarySource_->Pause();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    ret = primarySource_->Flush();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    ret = primarySource_->Reset();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    ret = primarySource_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    DeInitPrimarySource();
}

/**
 * @tc.name   : Test PrimarySource API
 * @tc.number : PrimarySourceUnitTest_005
 * @tc.desc   : Test primary source get param
 */
HWTEST_F(AudioCaptureSourceUnitTest, PrimarySourceUnitTest_005, TestSize.Level1)
{
    InitPrimarySource();
    EXPECT_TRUE(primarySource_ && primarySource_->IsInited());
    std::string param = primarySource_->GetAudioParameter(USB_DEVICE, "");
    EXPECT_EQ(param, "");
    DeInitPrimarySource();
}

/**
 * @tc.name   : Test PrimarySource API
 * @tc.number : PrimarySourceUnitTest_006
 * @tc.desc   : Test primary source set volume
 */
HWTEST_F(AudioCaptureSourceUnitTest, PrimarySourceUnitTest_006, TestSize.Level1)
{
    InitPrimarySource();
    EXPECT_TRUE(primarySource_ && primarySource_->IsInited());
    int32_t ret = primarySource_->SetVolume(1.0f, 1.0f);
    EXPECT_NE(ret, SUCCESS);
    DeInitPrimarySource();
}

/**
 * @tc.name   : Test PrimarySource API
 * @tc.number : PrimarySourceUnitTest_007
 * @tc.desc   : Test primary source set/get mute
 */
HWTEST_F(AudioCaptureSourceUnitTest, PrimarySourceUnitTest_007, TestSize.Level1)
{
    InitPrimarySource();
    EXPECT_TRUE(primarySource_ && primarySource_->IsInited());
    int32_t ret = primarySource_->SetMute(false);
    EXPECT_EQ(ret, SUCCESS);
    bool mute = false;
    ret = primarySource_->GetMute(mute);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_FALSE(mute);
    DeInitPrimarySource();
}

/**
 * @tc.name   : Test PrimarySource API
 * @tc.number : PrimarySourceUnitTest_008
 * @tc.desc   : Test primary source get transaction id
 */
HWTEST_F(AudioCaptureSourceUnitTest, PrimarySourceUnitTest_008, TestSize.Level1)
{
    InitPrimarySource();
    EXPECT_TRUE(primarySource_ && primarySource_->IsInited());
    uint64_t transId = primarySource_->GetTransactionId();
    EXPECT_NE(transId, 0);
    DeInitPrimarySource();
}

/**
 * @tc.name   : Test PrimarySource API
 * @tc.number : PrimarySourceUnitTest_009
 * @tc.desc   : Test primary source get max amplitude
 */
HWTEST_F(AudioCaptureSourceUnitTest, PrimarySourceUnitTest_009, TestSize.Level1)
{
    InitPrimarySource();
    EXPECT_TRUE(primarySource_ && primarySource_->IsInited());
    float maxAmplitude = primarySource_->GetMaxAmplitude();
    EXPECT_EQ(maxAmplitude, 0.0f);
    DeInitPrimarySource();
}

/**
 * @tc.name   : Test PrimarySource API
 * @tc.number : PrimarySourceUnitTest_010
 * @tc.desc   : Test primary source set audio scene
 */
HWTEST_F(AudioCaptureSourceUnitTest, PrimarySourceUnitTest_010, TestSize.Level1)
{
    InitPrimarySource();
    EXPECT_TRUE(primarySource_ && primarySource_->IsInited());
    int32_t ret = primarySource_->SetAudioScene(AUDIO_SCENE_DEFAULT);
    EXPECT_EQ(ret, SUCCESS);
    DeInitPrimarySource();
}

/**
 * @tc.name   : Test PrimarySource API
 * @tc.number : PrimarySourceUnitTest_011
 * @tc.desc   : Test primary source update source type
 */
HWTEST_F(AudioCaptureSourceUnitTest, PrimarySourceUnitTest_011, TestSize.Level1)
{
    InitPrimarySource();
    EXPECT_TRUE(primarySource_ && primarySource_->IsInited());
    int32_t ret = primarySource_->UpdateSourceType(SOURCE_TYPE_MIC);
    EXPECT_EQ(ret, SUCCESS);
    DeInitPrimarySource();
}

/**
 * @tc.name   : Test PrimarySource API
 * @tc.number : PrimarySourceUnitTest_012
 * @tc.desc   : Test primary source update apps uid
 */
HWTEST_F(AudioCaptureSourceUnitTest, PrimarySourceUnitTest_012, TestSize.Level1)
{
    InitPrimarySource();
    EXPECT_TRUE(primarySource_ && primarySource_->IsInited());
    vector<int32_t> appsUid;
    int32_t ret = primarySource_->UpdateAppsUid(appsUid);
    EXPECT_EQ(ret, SUCCESS);
    DeInitPrimarySource();
}

/**
 * @tc.name   : Test PrimarySource API
 * @tc.number : PrimarySourceUnitTest_013
 * @tc.desc   : Test primary source IsCaptureInvalid
 */
HWTEST_F(AudioCaptureSourceUnitTest, PrimarySourceUnitTest_013, TestSize.Level1)
{
    InitPrimarySource();
    EXPECT_TRUE(primarySource_ && primarySource_->IsInited());
    EXPECT_TRUE(primarySource_->IsCaptureInvalid());
    primarySource_->DeInit();
    EXPECT_FALSE(primarySource_->IsCaptureInvalid());
    DeInitPrimarySource();
}

/**
 * @tc.name   : Test PrimarySource API
 * @tc.number : PrimarySourceUnitTest_014
 * @tc.desc   : Test primary source GetUniqueIdBySourceType
 */
HWTEST_F(AudioCaptureSourceUnitTest, PrimarySourceUnitTest_014, TestSize.Level1)
{
    InitPrimarySource();
    attr_.sourceType = SOURCE_TYPE_VOICE_TRANSCRIPTION;
    EXPECT_TRUE(primarySource_ && primarySource_->IsInited());
    uint32 streamId = AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_VOICE_TRANSCRIPTION * UNIQUE_ID_INTERVAL;
    EXPECT_EQ(primarySource_->GetUniqueIdBySourceType(), streamId);
    DeInitPrimarySource();
}

/**
@tc.name : Test PrimarySource API
@tc.number : PrimarySourceUnitTest_015
@tc.desc : Test CheckFrameInfoLen
*/
HWTEST_F(AudioCaptureSourceUnitTest, CheckFrameInfoLen_001, TestSize.Level1)
{
    auto source_ = std::make_shared<AudioCaptureSource>(10004, "device001");
    ASSERT_NE(source_, nullptr);

    uint64_t replyBytes = 0;
    std::vector buffer{'8', '8', '8', '8', '8', '8', '8', '8'};
    std::vector bufferEc{'8', '8', '8', '8', '8', '8', '8', '8'};
    FrameDesc fdesc = {
        .frame = buffer.data(),
        .frameLen = buffer.size(),
    };
    FrameDesc fdescEc = {
        .frame = bufferEc.data(),
        .frameLen = bufferEc.size(),
    };
    struct AudioCaptureFrameInfo frameInfo = {
        .replyBytes = 10,
    };
    int32_t status = source_->CheckFrameInfoLen(&fdesc, replyBytes, &fdescEc, frameInfo);
    EXPECT_EQ(status, ERR_INVALID_READ);
    frameInfo.frame = new int8_t[DEFAULT_SIZE];
    status = source_->CheckFrameInfoLen(&fdesc, replyBytes, &fdescEc, frameInfo);
    EXPECT_EQ(status, ERR_INVALID_READ);
    delete[] frameInfo.frame;
}

/**
@tc.name : Test PrimarySource API
@tc.number : PrimarySourceUnitTest_015
@tc.desc : Test CheckFrameInfoLen
*/
HWTEST_F(AudioCaptureSourceUnitTest, CheckFrameInfoLen_002, TestSize.Level1)
{
    auto source_ = std::make_shared<AudioCaptureSource>(10004, "device001");
    ASSERT_NE(source_, nullptr);

    uint64_t replyBytes = 0;
    std::vector buffer{'8', '8', '8', '8', '8', '8', '8', '8'};
    std::vector bufferEc{'8', '8', '8', '8', '8', '8', '8', '8'};
    FrameDesc fdesc = {
        .frame = buffer.data(),
        .frameLen = buffer.size(),
    };
    FrameDesc fdescEc = {
        .frame = bufferEc.data(),
        .frameLen = bufferEc.size(),
    };
    struct AudioCaptureFrameInfo frameInfo = {
        .frame = new int8_t[4],
        .replyBytes = 1024,
    };
    int32_t status = source_->CheckFrameInfoLen(&fdesc, replyBytes, &fdescEc, frameInfo);
    EXPECT_EQ(status, SUCCESS);
    delete[] frameInfo.frame;
}

/*
@tc.name : Test PrimarySource API
@tc.number : PrimarySourceUnitTest_015
@tc.desc : Test CaptureFrameEC
*/
HWTEST_F(AudioCaptureSourceUnitTest, CaptureFrameWithEc_001, TestSize.Level1)
{
    auto source_ = std::make_shared<AudioCaptureSource>(10004, "device001");
    ASSERT_NE(source_, nullptr);
    attr_.adapterName = "primary";
    attr_.sampleRate = 48000; // 48000: sample rate
    attr_.channel = 2; // 2: channel
    attr_.format = SAMPLE_S16LE;
    attr_.channelLayout = 3; // 3: channel layout
    attr_.deviceType = DEVICE_TYPE_MIC;
    attr_.openMicSpeaker = 1;
    attr_.sourceType = SOURCE_TYPE_OFFLOAD_CAPTURE;
    source_->Init(attr_);
    ASSERT_NE(source_->audioCapture_, nullptr);
    source_->audioCapture_->CaptureFrameEc = AudioCaptureCaptureFrameEc001;

    uint64_t replyBytes = 0;
    uint64_t replyBytesEc = 0;
    std::vector buffer{'8', '8', '8', '8', '8', '8', '8', '8'};
    std::vector bufferEc{'8', '8', '8', '8', '8', '8', '8', '8'};
    FrameDesc fdesc = {
        .frame = buffer.data(),
        .frameLen = buffer.size(),
    };
    FrameDesc fdescEc = {
        .frame = bufferEc.data(),
        .frameLen = bufferEc.size(),
    };
    int32_t ret = source_->CaptureFrameWithEc(&fdesc, replyBytes, &fdescEc, replyBytesEc);
    EXPECT_EQ(ret, SUCCESS);
    source_->audioSrcClock_ = std::make_shared<AudioCapturerSourceClock>();
    CapturerClockManager::GetInstance().RegisterAudioSourceClock(0, source_->audioSrcClock_);
    source_->audioSrcClock_->frameCnt_ = 0;
    source_->audioCapture_->CaptureFrameEc = AudioCaptureCaptureFrameEc002;
    ret = source_->CaptureFrameWithEc(&fdesc, replyBytes, &fdescEc, replyBytesEc);
    EXPECT_EQ(ret, SUCCESS);
    source_->audioCapture_->CaptureFrameEc = AudioCaptureCaptureFrameEc003;
    ret = source_->CaptureFrameWithEc(&fdesc, replyBytes, &fdescEc, replyBytesEc);
    EXPECT_EQ(ret, SUCCESS);
    source_->DeInit();
}

/**
@tc.name : Test PrimarySource API
@tc.number : PrimarySourceUnitTest_015
@tc.desc : Test CaptureFrameEC
*/
HWTEST_F(AudioCaptureSourceUnitTest, CaptureFrameWithEc_002, TestSize.Level1)
{
    auto source_ = std::make_shared<AudioCaptureSource>(10004, "device001");
    ASSERT_NE(source_, nullptr);
    attr_.adapterName = "primary";
    attr_.sampleRate = 48000; // 48000: sample rate
    attr_.channel = 2; // 2: channel
    attr_.format = SAMPLE_S16LE;
    attr_.channelLayout = 3; // 3: channel layout
    attr_.deviceType = DEVICE_TYPE_MIC;
    attr_.openMicSpeaker = 1;
    attr_.sourceType = SOURCE_TYPE_LIVE;
    source_->Init(attr_);
    ASSERT_NE(source_->audioCapture_, nullptr);

    uint64_t replyBytes = 0;
    uint64_t replyBytesEc = 0;
    std::vector buffer{'8', '8', '8', '8', '8', '8', '8', '8'};
    std::vector bufferEc{'8', '8', '8', '8', '8', '8', '8', '8'};
    FrameDesc fdesc = {
        .frame = buffer.data(),
        .frameLen = buffer.size(),
    };
    FrameDesc fdescEc = {
        .frame = bufferEc.data(),
        .frameLen = bufferEc.size(),
    };
    source_->audioCapture_->CaptureFrameEc = AudioCaptureCaptureFrameEc001;
    int32_t ret = source_->CaptureFrameWithEc(&fdesc, replyBytes, &fdescEc, replyBytesEc);
    EXPECT_EQ(ret, ERR_INVALID_READ);
    source_->audioSrcClock_ = std::make_shared<AudioCapturerSourceClock>();
    CapturerClockManager::GetInstance().RegisterAudioSourceClock(0, source_->audioSrcClock_);
    source_->audioSrcClock_->frameCnt_ = 0;
    source_->audioCapture_->CaptureFrameEc = AudioCaptureCaptureFrameEc002;
    ret = source_->CaptureFrameWithEc(&fdesc, replyBytes, &fdescEc, replyBytesEc);
    EXPECT_EQ(ret, ERR_INVALID_READ);
    source_->audioCapture_->CaptureFrameEc = AudioCaptureCaptureFrameEc003;
    ret = source_->CaptureFrameWithEc(&fdesc, replyBytes, &fdescEc, replyBytesEc);
    EXPECT_EQ(ret, SUCCESS);
    source_->DeInit();
}

/**
@tc.name : Test PrimarySource API
@tc.number : PrimarySourceUnitTest_015
@tc.desc : Test CaptureFrameEC
*/
HWTEST_F(AudioCaptureSourceUnitTest, CaptureFrameWithEc_003, TestSize.Level1)
{
    auto source_ = std::make_shared<AudioCaptureSource>(10004, "device001");
    ASSERT_NE(source_, nullptr);
    attr_.adapterName = "primary";
    attr_.sampleRate = 48000; // 48000: sample rate
    attr_.channel = 2; // 2: channel
    attr_.format = SAMPLE_S16LE;
    attr_.channelLayout = 3; // 3: channel layout
    attr_.deviceType = DEVICE_TYPE_MIC;
    attr_.openMicSpeaker = 1;
    attr_.sourceType = SOURCE_TYPE_EC;
    source_->Init(attr_);
    ASSERT_NE(source_->audioCapture_, nullptr);

    uint64_t replyBytes = 0;
    uint64_t replyBytesEc = 0;
    std::vector buffer{'8', '8', '8', '8', '8', '8', '8', '8'};
    std::vector bufferEc{'8', '8', '8', '8', '8', '8', '8', '8'};
    FrameDesc fdesc = {
        .frame = buffer.data(),
        .frameLen = buffer.size(),
    };
    FrameDesc fdescEc = {
        .frame = bufferEc.data(),
        .frameLen = bufferEc.size(),
    };
    source_->audioCapture_->CaptureFrameEc = AudioCaptureCaptureFrameEc001;
    int32_t ret = source_->CaptureFrameWithEc(&fdesc, replyBytes, &fdescEc, replyBytesEc);
    EXPECT_EQ(ret, SUCCESS);
    source_->audioSrcClock_ = std::make_shared<AudioCapturerSourceClock>();
    CapturerClockManager::GetInstance().RegisterAudioSourceClock(0, source_->audioSrcClock_);
    source_->audioSrcClock_->frameCnt_ = 0;
    source_->audioCapture_->CaptureFrameEc = AudioCaptureCaptureFrameEc002;
    ret = source_->CaptureFrameWithEc(&fdesc, replyBytes, &fdescEc, replyBytesEc);
    EXPECT_EQ(ret, SUCCESS);
    source_->audioCapture_->CaptureFrameEc = AudioCaptureCaptureFrameEc003;
    ret = source_->CaptureFrameWithEc(&fdesc, replyBytes, &fdescEc, replyBytesEc);
    EXPECT_EQ(ret, SUCCESS);
    source_->DeInit();
}

/**
 * @tc.name   : Test UsbSource API
 * @tc.number : UsbSourceUnitTest_001
 * @tc.desc   : Test usb source create
 */
HWTEST_F(AudioCaptureSourceUnitTest, UsbSourceUnitTest_001, TestSize.Level1)
{
    InitUsbSource();
    EXPECT_TRUE(usbSource_);
    DeInitUsbSource();
}

/**
* @tc.name   : Test UsbSource API
* @tc.number : UsbSourceUnitTest_002
* @tc.desc   : Test usb source init
*/
HWTEST_F(AudioCaptureSourceUnitTest, UsbSourceUnitTest_002, TestSize.Level1)
{
    InitUsbSource();
    EXPECT_TRUE(usbSource_);
    DeInitUsbSource();
}

/**
* @tc.name   : Test UsbSource API
* @tc.number : UsbSourceUnitTest_003
* @tc.desc   : Test usb source deinit
*/
HWTEST_F(AudioCaptureSourceUnitTest, UsbSourceUnitTest_003, TestSize.Level1)
{
    InitUsbSource();
    EXPECT_TRUE(usbSource_);
    usbSource_->DeInit();
    int32_t ret = usbSource_->Init(attr_);
    EXPECT_EQ(ret, SUCCESS);
    ret = usbSource_->Init(attr_);
    EXPECT_EQ(ret, SUCCESS);
    DeInitUsbSource();
}

/**
* @tc.name   : Test UsbSource API
* @tc.number : UsbSourceUnitTest_004
* @tc.desc   : Test usb source start, stop, resume, pause, flush, reset
*/
HWTEST_F(AudioCaptureSourceUnitTest, UsbSourceUnitTest_004, TestSize.Level1)
{
    InitUsbSource();
    EXPECT_TRUE(usbSource_);
    int32_t ret = usbSource_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = usbSource_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    ret = usbSource_->Resume();
    EXPECT_EQ(ret, SUCCESS);
    ret = usbSource_->Pause();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    ret = usbSource_->Flush();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    ret = usbSource_->Reset();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    ret = usbSource_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    DeInitUsbSource();
}

/**
 * @tc.name   : Test GetChannelCountByChannelLayout API
 * @tc.number : GetChannelCountByChannelLayout_001
 * @tc.desc   : Test GetChannelCountByChannelLayout API
 */
HWTEST_F(AudioCaptureSourceUnitTest, GetChannelCountByChannelLayout_001, TestSize.Level1)
{
    uint64_t channelCount = 0;
    
    channelCount = AudioCaptureSource::GetChannelCountByChannelLayout(AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    EXPECT_EQ(channelCount, AudioChannel::CHANNEL_UNKNOW);

    channelCount = AudioCaptureSource::GetChannelCountByChannelLayout(AudioChannelLayout::CH_LAYOUT_MONO);
    EXPECT_EQ(channelCount, AudioChannel::MONO);

    channelCount = AudioCaptureSource::GetChannelCountByChannelLayout(AudioChannelLayout::CH_LAYOUT_STEREO);
    EXPECT_EQ(channelCount, AudioChannel::STEREO);

    channelCount = AudioCaptureSource::GetChannelCountByChannelLayout(AudioChannelLayout::CH_LAYOUT_4POINT0);
    EXPECT_EQ(channelCount, AudioChannel::CHANNEL_4);
    channelCount = AudioCaptureSource::GetChannelCountByChannelLayout(AudioChannelLayout::CH_LAYOUT_QUAD);
    EXPECT_EQ(channelCount, AudioChannel::CHANNEL_4);
}

/**
 * @tc.name   : Test GetUniqueIdBySourceType API
 * @tc.number : GetUniqueIdBySourceType_001
 * @tc.desc   : Test GetUniqueIdBySourceType API
 */
HWTEST_F(AudioCaptureSourceUnitTest, GetUniqueIdBySourceType_001, TestSize.Level1)
{
    auto captureSource = std::make_shared<AudioCaptureSource>(1);
    uint32_t id = 0;
    captureSource->attr_.sourceType = SOURCE_TYPE_EC;
    captureSource->attr_.hdiSourceType = "AUDIO_INPUT_DEFAULT_TYPE";
    id = captureSource->GetUniqueIdBySourceType();
    EXPECT_EQ(id, AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_EC * UNIQUE_ID_INTERVAL);
}

/**
 * @tc.name   : Test GetUniqueIdBySourceType API
 * @tc.number : GetUniqueIdBySourceType_002
 * @tc.desc   : Test GetUniqueIdBySourceType API
 */
HWTEST_F(AudioCaptureSourceUnitTest, GetUniqueIdBySourceType_002, TestSize.Level1)
{
    auto captureSource = std::make_shared<AudioCaptureSource>(1);
    uint32_t id = 0;
    captureSource->attr_.sourceType = SOURCE_TYPE_MIC_REF;
    captureSource->attr_.hdiSourceType = "AUDIO_INPUT_DEFAULT_TYPE";
    id = captureSource->GetUniqueIdBySourceType();
    EXPECT_EQ(id, AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_MIC_REF * UNIQUE_ID_INTERVAL);
}

/**
 * @tc.name   : Test GetUniqueIdBySourceType API
 * @tc.number : GetUniqueIdBySourceType_003
 * @tc.desc   : Test GetUniqueIdBySourceType API
 */
HWTEST_F(AudioCaptureSourceUnitTest, GetUniqueIdBySourceType_003, TestSize.Level1)
{
    auto captureSource = std::make_shared<AudioCaptureSource>(1);
    uint32_t id = 0;
    captureSource->attr_.sourceType = SOURCE_TYPE_WAKEUP;
    captureSource->attr_.hdiSourceType = "AUDIO_INPUT_DEFAULT_TYPE";
    id = captureSource->GetUniqueIdBySourceType();
    EXPECT_EQ(id, AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_WAKEUP * UNIQUE_ID_INTERVAL);
}

/**
 * @tc.name   : Test GetUniqueIdBySourceType API
 * @tc.number : GetUniqueIdBySourceType_004
 * @tc.desc   : Test GetUniqueIdBySourceType API
 */
HWTEST_F(AudioCaptureSourceUnitTest, GetUniqueIdBySourceType_004, TestSize.Level1)
{
    auto captureSource = std::make_shared<AudioCaptureSource>(1);
    uint32_t id = 0;
    captureSource->attr_.sourceType = SOURCE_TYPE_VOICE_TRANSCRIPTION;
    captureSource->attr_.hdiSourceType = "AUDIO_INPUT_VOICE_TRANSCRIPTION";
    id = captureSource->GetUniqueIdBySourceType();
    EXPECT_EQ(id, AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_VOICE_TRANSCRIPTION * UNIQUE_ID_INTERVAL);
}

/**
 * @tc.name   : Test GetUniqueIdBySourceType API
 * @tc.number : GetUniqueIdBySourceType_005
 * @tc.desc   : Test GetUniqueIdBySourceType API
 */
HWTEST_F(AudioCaptureSourceUnitTest, GetUniqueIdBySourceType_005, TestSize.Level1)
{
    auto captureSource = std::make_shared<AudioCaptureSource>(1);
    uint32_t id = 0;
    captureSource->attr_.sourceType = SOURCE_TYPE_UNPROCESSED;
    captureSource->attr_.hdiSourceType = "AUDIO_INPUT_RAW_TYPE";
    id = captureSource->GetUniqueIdBySourceType();
    EXPECT_EQ(id, AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_UNPROCESS * UNIQUE_ID_INTERVAL);
}

/**
 * @tc.name   : Test GetUniqueIdBySourceType API
 * @tc.number : GetUniqueIdBySourceType_006
 * @tc.desc   : Test GetUniqueIdBySourceType API
 */
HWTEST_F(AudioCaptureSourceUnitTest, GetUniqueIdBySourceType_006, TestSize.Level1)
{
    auto captureSource = std::make_shared<AudioCaptureSource>(1);
    uint32_t id = 0;
    captureSource->attr_.sourceType = SOURCE_TYPE_OFFLOAD_CAPTURE;
    captureSource->attr_.hdiSourceType = "AUDIO_INPUT_DEFAULT_TYPE";
    id = captureSource->GetUniqueIdBySourceType();
    EXPECT_EQ(id, AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_OFFLOAD_CAPTURE * UNIQUE_ID_INTERVAL);
}

/**
 * @tc.name   : Test GetUniqueIdBySourceType API
 * @tc.number : GetUniqueIdBySourceType_007
 * @tc.desc   : Test GetUniqueIdBySourceType API
 */
HWTEST_F(AudioCaptureSourceUnitTest, GetUniqueIdBySourceType_007, TestSize.Level1)
{
    auto captureSource = std::make_shared<AudioCaptureSource>(1);
    uint32_t id = 0;
    captureSource->attr_.sourceType = SOURCE_TYPE_INVALID;
    captureSource->attr_.hdiSourceType = "AUDIO_INPUT_DEFAULT_TYPE";
    id = captureSource->GetUniqueIdBySourceType();
    EXPECT_EQ(id, AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_PRIMARY * UNIQUE_ID_INTERVAL);
}

/**
 * @tc.name   : Test GetUniqueIdBySourceType API
 * @tc.number : GetUniqueIdBySourceType_008
 * @tc.desc   : Test GetUniqueIdBySourceType API
 */
HWTEST_F(AudioCaptureSourceUnitTest, GetUniqueIdBySourceType_008, TestSize.Level1)
{
    auto captureSource = std::make_shared<AudioCaptureSource>(1);
    uint32_t id = 0;
    captureSource->attr_.sourceType = SOURCE_TYPE_INVALID;
    captureSource->attr_.hdiSourceType = "";
    id = captureSource->GetUniqueIdBySourceType();
    EXPECT_EQ(id, AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_PRIMARY * UNIQUE_ID_INTERVAL);
}

/**
 * @tc.name   : Test GetUniqueIdBySourceType API
 * @tc.number : GetUniqueIdBySourceType_009
 * @tc.desc   : Test GetUniqueIdBySourceType API
 */
HWTEST_F(AudioCaptureSourceUnitTest, GetUniqueIdBySourceType_009, TestSize.Level1)
{
    auto captureSource = std::make_shared<AudioCaptureSource>(1);
    uint32_t id = 0;
    captureSource->attr_.sourceType = SOURCE_TYPE_ULTRASONIC;
    captureSource->attr_.hdiSourceType = "AUIDO_INPUT_ULTRASONIC_TYPE";
    id = captureSource->GetUniqueIdBySourceType();
    EXPECT_EQ(id, AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_ULTRASONIC * UNIQUE_ID_INTERVAL);
}

/**
 * @tc.name   : Test GetUniqueIdBySourceType API
 * @tc.number : GetUniqueIdBySourceType_010
 * @tc.desc   : Test GetUniqueIdBySourceType API
 */
HWTEST_F(AudioCaptureSourceUnitTest, GetUniqueIdBySourceType_010, TestSize.Level1)
{
    auto captureSource = std::make_shared<AudioCaptureSource>(1);
    uint32_t id = 0;
    captureSource->attr_.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    captureSource->attr_.hdiSourceType = "AUIDO_INPUT_VOICE_RECOGNITION_TYPE";
    id = captureSource->GetUniqueIdBySourceType();
    EXPECT_EQ(id, AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_VOICE_RECOGNITION * UNIQUE_ID_INTERVAL);
}

/**
 * @tc.name   : Test GetUniqueIdBySourceType API
 * @tc.number : GetUniqueIdBySourceType_011
 * @tc.desc   : Test GetUniqueIdBySourceType API
 */
HWTEST_F(AudioCaptureSourceUnitTest, GetUniqueIdBySourceType_011, TestSize.Level1)
{
    auto captureSource = std::make_shared<AudioCaptureSource>(1);
    uint32_t id = 0;
    captureSource->attr_.sourceType = SOURCE_TYPE_UNPROCESSED_VOICE_ASSISTANT;
    captureSource->attr_.hdiSourceType = "AUDIO_INPUT_RAW_AI_TYPE";
    id = captureSource->GetUniqueIdBySourceType();
    EXPECT_EQ(id, AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_UNPROCESSED_VOICE_ASSISTANT * UNIQUE_ID_INTERVAL);
}


/**
 * @tc.name   : Test PrimarySource API
 * @tc.number : ChangePipeStream_001
 * @tc.desc   : Test ChangePipeStream() add, change and remove cases
 */
HWTEST_F(AudioCaptureSourceUnitTest, ChangePipeStream_001, TestSize.Level2)
{
    InitPrimarySource();
    EXPECT_TRUE(primarySource_ && primarySource_->IsInited());

    primarySource_->InitPipeInfo(TEST_CAPUTRE_ID, HDI_ADAPTER_TYPE_PRIMARY, AUDIO_INPUT_FLAG_NORMAL);

    primarySource_->ChangePipeStream(STREAM_CHANGE_TYPE_ADD,
        TEST_STREAM_ID, SOURCE_TYPE_MIC, CAPTURER_PREPARED);
    auto pipeInfo = primarySource_->GetInputPipeInfo();
    EXPECT_EQ(1, pipeInfo->GetStreams().size());

    primarySource_->ChangePipeStream(STREAM_CHANGE_TYPE_STATE_CHANGE,
        TEST_STREAM_ID, SOURCE_TYPE_MIC, CAPTURER_RUNNING);
    pipeInfo = primarySource_->GetInputPipeInfo();
    auto streams = pipeInfo->GetStreams();
    if (streams.find(TEST_STREAM_ID) == streams.end()) {
        DeInitPrimarySource();
        FAIL();
    }
    EXPECT_EQ(CAPTURER_RUNNING, streams[TEST_STREAM_ID].state_);

    primarySource_->ChangePipeStream(STREAM_CHANGE_TYPE_REMOVE,
        TEST_STREAM_ID, SOURCE_TYPE_MIC, CAPTURER_PREPARED);
    pipeInfo = primarySource_->GetInputPipeInfo();
    EXPECT_EQ(0, pipeInfo->GetStreams().size());

    DeInitPrimarySource();
}

/**
 * @tc.name   : Test PrimarySource API
 * @tc.number : ChangePipeStream_002
 * @tc.desc   : Test ChangePipeStream() remove all cases
 */
HWTEST_F(AudioCaptureSourceUnitTest, ChangePipeStream_002, TestSize.Level2)
{
    InitPrimarySource();
    EXPECT_TRUE(primarySource_ && primarySource_->IsInited());

    primarySource_->InitPipeInfo(TEST_CAPUTRE_ID, HDI_ADAPTER_TYPE_PRIMARY, AUDIO_INPUT_FLAG_NORMAL);

    primarySource_->ChangePipeStream(STREAM_CHANGE_TYPE_ADD,
        TEST_STREAM_ID, SOURCE_TYPE_MIC, CAPTURER_PREPARED);
    auto pipeInfo = primarySource_->GetInputPipeInfo();
    EXPECT_EQ(1, pipeInfo->GetStreams().size());

    primarySource_->ChangePipeStream(STREAM_CHANGE_TYPE_REMOVE_ALL,
        TEST_STREAM_ID, SOURCE_TYPE_MIC, CAPTURER_RUNNING);
    pipeInfo = primarySource_->GetInputPipeInfo();
    EXPECT_EQ(0, pipeInfo->GetStreams().size());

    DeInitPrimarySource();
}

/**
 * @tc.name   : Test PrimarySource API
 * @tc.number : ChangePipeStream_003
 * @tc.desc   : Test ChangePipeStream() abnormal case
 */
HWTEST_F(AudioCaptureSourceUnitTest, ChangePipeStream_003, TestSize.Level4)
{
    InitPrimarySource();
    EXPECT_TRUE(primarySource_ && primarySource_->IsInited());

    primarySource_->InitPipeInfo(TEST_CAPUTRE_ID, HDI_ADAPTER_TYPE_PRIMARY, AUDIO_INPUT_FLAG_NORMAL);

    primarySource_->ChangePipeStream(static_cast<StreamChangeType>(STREAM_CHANGE_TYPE_STATE_CHANGE + 1),
        TEST_STREAM_ID, SOURCE_TYPE_MIC, CAPTURER_PREPARED);
    auto pipeInfo = primarySource_->GetInputPipeInfo();
    EXPECT_EQ(0, pipeInfo->GetStreams().size());

    DeInitPrimarySource();
}
} // namespace AudioStandard
} // namespace OHOS
