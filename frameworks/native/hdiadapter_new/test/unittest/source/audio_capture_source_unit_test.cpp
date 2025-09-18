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

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

const uint32_t DEFAULT_SIZE = 8;
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
    struct IAudioCapture *self, const struct AudioFrameLen *frameLen, struct AudioCaptureFrameInfo *frameInfo)
{
    int8_t* frameEc = nullptr;
    frameEc = new int8_t[DEFAULT_SIZE];
    frameInfo->frameEc = frameEc;
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

@tc.desc : Test offload source CaptureFrameEC
*/
HWTEST_F(AudioCaptureSourceUnitTest, PrimarySourceUnitTest_015, TestSize.Level1)
{
    auto offloadSource_ = std::make_shared<AudioCaptureSource>(10004, "device001");
    if (offloadSource_ == nullptr) {
        return;
    }
    attr_.adapterName = "primary";
    attr_.sampleRate = 48000; // 48000: sample rate
    attr_.channel = 2; // 2: channel
    attr_.format = SAMPLE_S16LE;
    attr_.channelLayout = 3; // 3: channel layout
    attr_.deviceType = DEVICE_TYPE_MIC;
    attr_.openMicSpeaker = 1;
    attr_.sourceType = SOURCE_TYPE_OFFLOAD_CAPTURE;
    offloadSource_->Init(attr_);
    offloadSource_->audioCapture_->CaptureFrameEc = AudioCaptureCaptureFrameEc001;

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
    EXPECT_EQ(offloadSource_->CaptureFrameWithEc(&fdesc, replyBytes, &fdescEc, replyBytesEc), SUCCESS);
    audioSrcClock_ = std::make_shared<AudioCapturerSourceClock>();
    CapturerClockManager::GetInstance().RegisterAudioSourceClock(0, audioSrcClock_);
    offloadSource_->audioCapture_->CaptureFrameEc = AudioCaptureCaptureFrameEc002;
    EXPECT_EQ(offloadSource_->CaptureFrameWithEc(&fdesc, replyBytes, &fdescEc, replyBytesEc), SUCCESS);
    offloadSource_->attr_.sourceType = SOURCE_TYPE_LIVE;
    EXPECT_EQ(offloadSource_->CaptureFrameWithEc(&fdesc, replyBytes, &fdescEc, replyBytesEc), SUCCESS);
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
    EXPECT_NE(ret, SUCCESS);
    ret = usbSource_->Init(attr_);
    EXPECT_NE(ret, SUCCESS);
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
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = usbSource_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    ret = usbSource_->Resume();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = usbSource_->Pause();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = usbSource_->Flush();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = usbSource_->Reset();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
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

} // namespace AudioStandard
} // namespace OHOS
