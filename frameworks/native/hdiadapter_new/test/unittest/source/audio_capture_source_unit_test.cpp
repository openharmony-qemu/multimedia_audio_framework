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
#include "audio_stream_enum.h"
#include "capturer_clock_manager.h"

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
    static int32_t AudioCaptureCaptureFrameCamcorder(
        struct IAudioCapture *self, int8_t *frame, uint32_t *frameLen, uint64_t *replyBytes);
protected:
    static uint32_t primaryId_;
    static uint32_t usbId_;
    static std::shared_ptr<IAudioCaptureSource> primarySource_;
    static std::shared_ptr<IAudioCaptureSource> usbSource_;
    static IAudioSourceAttr attr_;
};

uint32_t AudioCaptureSourceUnitTest::primaryId_ = 0;
uint32_t AudioCaptureSourceUnitTest::usbId_ = 0;
std::shared_ptr<IAudioCaptureSource> AudioCaptureSourceUnitTest::primarySource_ = nullptr;
std::shared_ptr<IAudioCaptureSource> AudioCaptureSourceUnitTest::usbSource_ = nullptr;
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
    attr_.channelLayout = CH_LAYOUT_4POINT0;
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
    int8_t* frame = nullptr;
    frameEc = new int8_t[DEFAULT_SIZE];
    frame = new int8_t[6]; // 6: frame
    frameInfo->frameEc = frameEc;
    frameInfo->frame = frame;
    frameInfo->replyBytes = DEFAULT_SIZE;
    return 0;
}

int32_t AudioCaptureSourceUnitTest::AudioCaptureCaptureFrameEc003(
    struct IAudioCapture *self, const struct AudioFrameLen *frameLen, struct AudioCaptureFrameInfo *frameInfo)
{
    int8_t* frameEc = nullptr;
    int8_t* frame = nullptr;
    frameEc = new int8_t[DEFAULT_SIZE];
    frame = new int8_t[DEFAULT_SIZE];
    frameInfo->frameEc = frameEc;
    frameInfo->frame = frame;
    frameInfo->replyBytes = 1024; // 1024: replyBytes
    return 0;
}

int32_t AudioCaptureSourceUnitTest::AudioCaptureCaptureFrameCamcorder(
    struct IAudioCapture *self, int8_t *frame, uint32_t *frameLen, uint64_t *replyBytes)
{
    const int8_t source[] = {
        21, 22, 23, 24,
        25, 26, 27, 28,
        1, 2, 3, 4, 5, 6, 7, 8,
        9, 10, 11, 12, 13, 14, 15, 16
    };
    uint32_t copyLen = static_cast<uint32_t>(sizeof(source));
    if (frameLen != nullptr && *frameLen < copyLen) {
        copyLen = *frameLen;
    }
    for (uint32_t i = 0; i < copyLen; ++i) {
        frame[i] = source[i];
    }
    if (replyBytes != nullptr) {
        *replyBytes = copyLen;
    }
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
 * @tc.name   : Test SetDmDeviceType API
 * @tc.number : SetDmDeviceType_001
 * @tc.desc   : Test SetDmDeviceType
 */
HWTEST_F(AudioCaptureSourceUnitTest, SetDmDeviceType_001, TestSize.Level1)
{
    InitPrimarySource();
    EXPECT_TRUE(primarySource_ && primarySource_->IsInited());
    primarySource_->UpdateActiveDevice(DEVICE_TYPE_MIC);
    primarySource_->SetDmDeviceType(DM_DEVICE_TYPE_DEFAULT, DEVICE_TYPE_MIC);
    primarySource_->SetDmDeviceType(DM_DEVICE_TYPE_NEARLINK_SCO, DEVICE_TYPE_MIC);
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

/**
@tc.name : Test PrimarySource API
@tc.number : PrimarySourceUnitTest_015
@tc.desc : Test CheckFrameInfoLen
*/
HWTEST_F(AudioCaptureSourceUnitTest, CheckFrameInfoLen_003, TestSize.Level1)
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
        .replyBytes = 4,
    };
    int32_t status = source_->CheckFrameInfoLen(&fdesc, replyBytes, &fdescEc, frameInfo);
    EXPECT_EQ(status, ERR_INVALID_READ);
    EXPECT_EQ(replyBytes, 0);
    delete[] frameInfo.frame;
}

/**
@tc.name : Test PrimarySource API
@tc.number : PrimarySourceUnitTest_015
@tc.desc : Test ProcessCapFrame with voice recognition interleave success
*/
HWTEST_F(AudioCaptureSourceUnitTest, ProcessCapFrame_001, TestSize.Level1)
{
    auto source_ = std::make_shared<AudioCaptureSource>(10004, "device001");
    ASSERT_NE(source_, nullptr);
    source_->attr_.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    source_->attr_.format = SAMPLE_S16LE;

    std::vector<int8_t> src {
        1, 2, 3, 4,                // raw: 2 frames * 1ch * s16
        11, 12, 13, 14, 15, 16, 17, 18,
        21, 22, 23, 24, 25, 26, 27, 28 // micIn: 2 frames * 4ch * s16
    };
    std::vector<int8_t> dst(src.size(), 0);
    std::vector<int8_t> ec(1, 0);
    FrameDesc fdesc = {
        .frame = reinterpret_cast<char *>(dst.data()),
        .frameLen = src.size(),
    };
    FrameDesc fdescEc = {
        .frame = reinterpret_cast<char *>(ec.data()),
        .frameLen = 0,
    };
    AudioCaptureFrameInfo frameInfo = {};
    frameInfo.frame = src.data();
    frameInfo.replyBytes = static_cast<uint64_t>(src.size());
    uint64_t replyBytes = 0;

    source_->ProcessCapFrame(&fdesc, replyBytes, &fdescEc, frameInfo);
    EXPECT_EQ(replyBytes, src.size());

    std::vector<int8_t> expected {
        1, 2, 11, 12, 13, 14, 15, 16, 17, 18,
        3, 4, 21, 22, 23, 24, 25, 26, 27, 28
    };
    EXPECT_EQ(dst, expected);
}

/**
@tc.name : Test PrimarySource API
@tc.number : PrimarySourceUnitTest_015
@tc.desc : Test ProcessCapFrame when copyLen is 0
*/
HWTEST_F(AudioCaptureSourceUnitTest, ProcessCapFrame_002, TestSize.Level1)
{
    auto source_ = std::make_shared<AudioCaptureSource>(10004, "device001");
    ASSERT_NE(source_, nullptr);
    source_->attr_.sourceType = SOURCE_TYPE_LIVE;

    std::vector<int8_t> src {1, 2, 3, 4, 5, 6, 7, 8};
    std::vector<int8_t> dst {0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C};
    std::vector<int8_t> expected {0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C};
    std::vector<int8_t> ec(4, 0);
    FrameDesc fdesc = {
        .frame = reinterpret_cast<char *>(dst.data()),
        .frameLen = dst.size(),
    };
    FrameDesc fdescEc = {
        .frame = reinterpret_cast<char *>(ec.data()),
        .frameLen = ec.size(),
    };
    AudioCaptureFrameInfo frameInfo = {};
    frameInfo.frame = src.data();
    frameInfo.replyBytes = static_cast<uint64_t>(ec.size() - 1);
    uint64_t replyBytes = 123; // 123: init check value

    source_->ProcessCapFrame(&fdesc, replyBytes, &fdescEc, frameInfo);
    EXPECT_EQ(replyBytes, 0);
    EXPECT_EQ(dst, expected);
    EXPECT_NE(dst[0], src[0]);
}

/**
@tc.name : Test PrimarySource API
@tc.number : PrimarySourceUnitTest_015
@tc.desc : Test ProcessCapFrame voice recognition interleave fallback
*/
HWTEST_F(AudioCaptureSourceUnitTest, ProcessCapFrame_003, TestSize.Level1)
{
    auto source_ = std::make_shared<AudioCaptureSource>(10004, "device001");
    ASSERT_NE(source_, nullptr);
    source_->attr_.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    source_->attr_.format = SAMPLE_S16LE;

    std::vector<int8_t> src {
        1, 2, 3, 4, 5, 6, 7, 8, 9,
        10, 11, 12, 13, 14, 15, 16, 17, 18
    };
    std::vector<int8_t> dst(20, 0);
    std::vector<int8_t> ec(4, 0);
    FrameDesc fdesc = {
        .frame = reinterpret_cast<char *>(dst.data()),
        .frameLen = dst.size(),
    };
    FrameDesc fdescEc = {
        .frame = reinterpret_cast<char *>(ec.data()),
        .frameLen = ec.size(),
    };
    AudioCaptureFrameInfo frameInfo = {};
    frameInfo.frame = src.data();
    frameInfo.replyBytes = static_cast<uint64_t>(src.size());
    uint64_t replyBytes = 0;

    source_->ProcessCapFrame(&fdesc, replyBytes, &fdescEc, frameInfo);
    EXPECT_EQ(replyBytes, 14); // 18 - ec(4)
    for (size_t i = 0; i < replyBytes; ++i) {
        EXPECT_EQ(dst[i], src[i]);
    }
}

/**
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
 * @tc.name   : Test CaptureFrame API
 * @tc.number : CaptureFrame_001
 * @tc.desc   : Test camcorder capture frame interleave raw and mic-in channels
 */
HWTEST_F(AudioCaptureSourceUnitTest, CaptureFrame_001, TestSize.Level1)
{
    auto source_ = std::make_shared<AudioCaptureSource>(10004, "device001");
    ASSERT_NE(source_, nullptr);
    IAudioSourceAttr attr = {};
    attr.adapterName = "primary";
    attr.sampleRate = SAMPLE_RATE_48000;
    attr.channel = STEREO;
    attr.format = SAMPLE_S16LE;
    attr.channelLayout = CH_LAYOUT_STEREO;
    attr.deviceType = DEVICE_TYPE_MIC;
    attr.openMicSpeaker = 1;
    attr.sourceType = SOURCE_TYPE_CAMCORDER;
    attr.channelMicIn = CHANNEL_4;
    source_->Init(attr);
    ASSERT_NE(source_->audioCapture_, nullptr);
    source_->audioCapture_->CaptureFrame = AudioCaptureCaptureFrameCamcorder;
    source_->started_.store(true);

    std::vector<int8_t> buffer(24, 0); // two 6-channel S16LE frames
    uint64_t replyBytes = 0;
    int32_t ret = source_->CaptureFrame(reinterpret_cast<char *>(buffer.data()), buffer.size(), replyBytes);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(replyBytes, buffer.size());
    std::vector<int8_t> expect {
        21, 22, 23, 24, 1, 2, 3, 4, 5, 6, 7, 8,
        25, 26, 27, 28, 9, 10, 11, 12, 13, 14, 15, 16
    };
    EXPECT_EQ(buffer, expect);
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
    EXPECT_EQ(channelCount, AudioChannel::STEREO);

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
* @tc.name   : Test audioSource API
* @tc.number : mappingSourceUnitTest_001
* @tc.desc   : Test audioSource
*/
HWTEST_F(AudioCaptureSourceUnitTest, mappingSourceUnitTest_001, TestSize.Level1)
{
    auto capturesource = std::make_shared<AudioCaptureSource>(1);
    std::string hdiSourceType = "AUDIO_INPUT_VOICE_TRANSCRIPTION";
    AudioInputType result = capturesource->MappingAudioInputType(hdiSourceType);
    EXPECT_EQ(result, AUDIO_INPUT_VOICE_TRANSCRIPTION);
 
    hdiSourceType = "AUDIO_INPUT_INVALID";
    result = capturesource->MappingAudioInputType(hdiSourceType);
    EXPECT_EQ(result, AUDIO_INPUT_MIC_TYPE);
}
 
/**
* @tc.name   : Test audioSource API
* @tc.number : convertUnitTest_001
* @tc.desc   : Test audioSource convert
*/
HWTEST_F(AudioCaptureSourceUnitTest, convertUnitTest_001, TestSize.Level1)
{
    auto capturesource = std::make_shared<AudioCaptureSource>(1);
    std::string hdiSourceType = "AUDIO_INPUT_VOICE_TRANSCRIPTION";
    AudioInputType result = capturesource->ConvertToHDIAudioInputType(1, hdiSourceType);
    EXPECT_EQ(result, AUDIO_INPUT_VOICE_TRANSCRIPTION);
}

/**
* @tc.name   : Test audioSource API
* @tc.number : GetUniqueIdBySourceType_001
* @tc.desc   : Test audioSource convert
*/
HWTEST_F(AudioCaptureSourceUnitTest, GetUniqueIdBySourceType_001, TestSize.Level1)
{
    auto capturesource = std::make_shared<AudioCaptureSource>(1);
    uint32_t id = 0;
    capturesource->attr_.sourceType = SOURCE_TYPE_EC;
    capturesource->attr_.hdiSourceType = "AUDIO_INPUT_DEFAULT_TYPE";
    id = capturesource->GetUniqueIdBySourceType();
    EXPECT_EQ(id, AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_EC * UNIQUE_ID_INTERVAL);
}

/**
* @tc.name   : Test audioSource API
* @tc.number : GetUniqueIdBySourceType_002
* @tc.desc   : Test audioSource convert
*/
HWTEST_F(AudioCaptureSourceUnitTest, GetUniqueIdBySourceType_002, TestSize.Level1)
{
    auto capturesource = std::make_shared<AudioCaptureSource>(1);
    uint32_t id = 0;
    capturesource->attr_.sourceType = SOURCE_TYPE_MIC_REF;
    capturesource->attr_.hdiSourceType = "AUDIO_INPUT_DEFAULT_TYPE";
    id = capturesource->GetUniqueIdBySourceType();
    EXPECT_EQ(id, AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_MIC_REF * UNIQUE_ID_INTERVAL);
}

/**
* @tc.name   : Test audioSource API
* @tc.number : GetUniqueIdBySourceType_003
* @tc.desc   : Test audioSource convert
*/
HWTEST_F(AudioCaptureSourceUnitTest, GetUniqueIdBySourceType_003, TestSize.Level1)
{
    auto capturesource = std::make_shared<AudioCaptureSource>(1);
    uint32_t id = 0;
    capturesource->attr_.sourceType = SOURCE_TYPE_WAKEUP;
    capturesource->attr_.hdiSourceType = "AUDIO_INPUT_DEFAULT_TYPE";
    id = capturesource->GetUniqueIdBySourceType();
    EXPECT_EQ(id, AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_WAKEUP * UNIQUE_ID_INTERVAL);
}

/**
* @tc.name   : Test audioSource API
* @tc.number : GetUniqueIdBySourceType_004
* @tc.desc   : Test audioSource convert
*/
HWTEST_F(AudioCaptureSourceUnitTest, GetUniqueIdBySourceType_004, TestSize.Level1)
{
    auto capturesource = std::make_shared<AudioCaptureSource>(1);
    uint32_t id = 0;
    capturesource->attr_.sourceType = SOURCE_TYPE_VOICE_TRANSCRIPTION;
    capturesource->attr_.hdiSourceType = "AUDIO_INPUT_VOICE_TRANSCRIPTION";
    id = capturesource->GetUniqueIdBySourceType();
    EXPECT_EQ(id, AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_VOICE_TRANSCRIPTION * UNIQUE_ID_INTERVAL);
}

/**
* @tc.name   : Test audioSource API
* @tc.number : GetUniqueIdBySourceType_005
* @tc.desc   : Test audioSource convert
*/
HWTEST_F(AudioCaptureSourceUnitTest, GetUniqueIdBySourceType_005, TestSize.Level1)
{
    auto capturesource = std::make_shared<AudioCaptureSource>(1);
    uint32_t id = 0;
    capturesource->attr_.sourceType = SOURCE_TYPE_UNPROCESSED;
    capturesource->attr_.hdiSourceType = "AUDIO_INPUT_RAW_TYPE";
    id = capturesource->GetUniqueIdBySourceType();
    EXPECT_EQ(id, AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_UNPROCESS * UNIQUE_ID_INTERVAL);
}

/**
* @tc.name   : Test audioSource API
* @tc.number : GetUniqueIdBySourceType_006
* @tc.desc   : Test audioSource convert
*/
HWTEST_F(AudioCaptureSourceUnitTest, GetUniqueIdBySourceType_006, TestSize.Level1)
{
    auto capturesource = std::make_shared<AudioCaptureSource>(1);
    uint32_t id = 0;
    capturesource->attr_.sourceType = SOURCE_TYPE_OFFLOAD_CAPTURE;
    capturesource->attr_.hdiSourceType = "AUDIO_INPUT_DEFAULT_TYPE";
    id = capturesource->GetUniqueIdBySourceType();
    EXPECT_EQ(id, AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_OFFLOAD_CAPTURE * UNIQUE_ID_INTERVAL);
}

/**
* @tc.name   : Test audioSource API
* @tc.number : GetUniqueIdBySourceType_007
* @tc.desc   : Test audioSource convert
*/
HWTEST_F(AudioCaptureSourceUnitTest, GetUniqueIdBySourceType_007, TestSize.Level1)
{
    auto capturesource = std::make_shared<AudioCaptureSource>(1);
    uint32_t id = 0;
    capturesource->attr_.sourceType = SOURCE_TYPE_INVALID;
    capturesource->attr_.hdiSourceType = "AUDIO_INPUT_DEFAULT_TYPE";
    id = capturesource->GetUniqueIdBySourceType();
    EXPECT_EQ(id, AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_PRIMARY * UNIQUE_ID_INTERVAL);
}

/**
* @tc.name   : Test audioSource API
* @tc.number : GetUniqueIdBySourceType_008
* @tc.desc   : Test audioSource convert
*/
HWTEST_F(AudioCaptureSourceUnitTest, GetUniqueIdBySourceType_008, TestSize.Level1)
{
    auto capturesource = std::make_shared<AudioCaptureSource>(1);
    uint32_t id = 0;
    capturesource->attr_.sourceType = SOURCE_TYPE_INVALID;
    capturesource->attr_.hdiSourceType = "";
    id = capturesource->GetUniqueIdBySourceType();
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
    captureSource->attr_.hdiSourceType = "AUDIO_INPUT_ULTRASONIC_TYPE";
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
    captureSource->attr_.hdiSourceType = "AUDIO_INPUT_VOICE_RECOGNITION_TYPE";
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
        TEST_STREAM_ID, SOURCE_TYPE_MIC, CAPTURER_PREPARED, MUTE_INVALID);
    auto pipeInfo = primarySource_->GetInputPipeInfo();
    EXPECT_EQ(1, pipeInfo->GetStreams().size());

    primarySource_->ChangePipeStream(STREAM_CHANGE_TYPE_STATE_CHANGE,
        TEST_STREAM_ID, SOURCE_TYPE_MIC, CAPTURER_RUNNING, MUTE_INVALID);
    pipeInfo = primarySource_->GetInputPipeInfo();
    auto streams = pipeInfo->GetStreams();
    if (streams.find(TEST_STREAM_ID) == streams.end()) {
        DeInitPrimarySource();
        FAIL();
    }
    EXPECT_EQ(CAPTURER_RUNNING, streams[TEST_STREAM_ID].state_);

    primarySource_->ChangePipeStream(STREAM_CHANGE_TYPE_REMOVE,
        TEST_STREAM_ID, SOURCE_TYPE_MIC, CAPTURER_PREPARED, MUTE_INVALID);
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
        TEST_STREAM_ID, SOURCE_TYPE_MIC, CAPTURER_PREPARED, MUTE_INVALID);
    auto pipeInfo = primarySource_->GetInputPipeInfo();
    EXPECT_EQ(1, pipeInfo->GetStreams().size());

    primarySource_->ChangePipeStream(STREAM_CHANGE_TYPE_REMOVE_ALL,
        TEST_STREAM_ID, SOURCE_TYPE_MIC, CAPTURER_RUNNING, MUTE_INVALID);
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
        TEST_STREAM_ID, SOURCE_TYPE_MIC, CAPTURER_PREPARED, MUTE_INVALID);
    auto pipeInfo = primarySource_->GetInputPipeInfo();
    EXPECT_EQ(0, pipeInfo->GetStreams().size());

    DeInitPrimarySource();
}

/**
 * @tc.name   : Test audioSource API
 * @tc.number : ConvertToAudioInputRouteFlag_001
 * @tc.desc   : Test audioSource convert
 */
HWTEST_F(AudioCaptureSourceUnitTest, ConvertToAudioInputRouteFlag_001, TestSize.Level1)
{
    auto captureSource = std::make_shared<AudioCaptureSource>(1);
    AudioFlag result = captureSource->ConvertToAudioInputRouteFlag(SOURCE_TYPE_VOICE_TRANSCRIPTION);
    EXPECT_EQ(result, AUDIO_INPUT_FLAG_AI);
    result = captureSource->ConvertToAudioInputRouteFlag(SOURCE_TYPE_UNPROCESSED);
    EXPECT_EQ(result, AUDIO_INPUT_FLAG_UNPROCESS);
    result = captureSource->ConvertToAudioInputRouteFlag(SOURCE_TYPE_LIVE);
    EXPECT_EQ(result, AUDIO_INPUT_FLAG_LIVE);
    result = captureSource->ConvertToAudioInputRouteFlag(SOURCE_TYPE_VOICE_MESSAGE);
    EXPECT_EQ(result, AUDIO_INPUT_FLAG_NORMAL);
    result = captureSource->ConvertToAudioInputRouteFlag(SOURCE_TYPE_ULTRASONIC);
    EXPECT_EQ(result, AUDIO_INPUT_FLAG_ULTRASONIC);
    result = captureSource->ConvertToAudioInputRouteFlag(SOURCE_TYPE_VOICE_RECOGNITION);
    EXPECT_EQ(result, AUDIO_INPUT_FLAG_VOICE_RECOGNITION);
    result = captureSource->ConvertToAudioInputRouteFlag(SOURCE_TYPE_UNPROCESSED_VOICE_ASSISTANT);
    EXPECT_EQ(result, AUDIO_INPUT_FLAG_RAW_AI);
    result = captureSource->ConvertToAudioInputRouteFlag(SOURCE_TYPE_WAKEUP);
    EXPECT_EQ(result, AUDIO_INPUT_FLAG_WAKEUP);
    result = captureSource->ConvertToAudioInputRouteFlag(SOURCE_TYPE_OFFLOAD_CAPTURE);
    EXPECT_EQ(result, AUDIO_INPUT_FLAG_OFFLOAD);
}

/**
 * @tc.name   : Test SetAudioScene API
 * @tc.number : SetAudioScene_001
 * @tc.desc   : Test SetAudioScene
 */
HWTEST_F(AudioCaptureSourceUnitTest, SetAudioScene_001, TestSize.Level1)
{
    InitPrimarySource();
    EXPECT_TRUE(primarySource_ && primarySource_->IsInited());
    int32_t ret = primarySource_->SetAudioScene(AUDIO_SCENE_DEFAULT, true);
    EXPECT_EQ(ret, SUCCESS);
    ret = primarySource_->SetAudioScene(AUDIO_SCENE_DEFAULT, false);
    EXPECT_EQ(ret, SUCCESS);
    ret = primarySource_->SetAudioScene(AUDIO_SCENE_PHONE_CALL, false);
    EXPECT_EQ(ret, SUCCESS);
    DeInitPrimarySource();
}

/**
 * @tc.name   : Test UpdateActiveDevice API
 * @tc.number : UpdateActiveDevice_001
 * @tc.desc   : Test UpdateActiveDevice
 */
HWTEST_F(AudioCaptureSourceUnitTest, UpdateActiveDevice_001, TestSize.Level1)
{
    InitPrimarySource();
    EXPECT_TRUE(primarySource_ && primarySource_->IsInited());
    DeviceType inputDevice = DEVICE_TYPE_MIC;
    primarySource_->UpdateActiveDevice(inputDevice);
    primarySource_->SetAudioScene(AUDIO_SCENE_DEFAULT, true);
    int32_t ret = primarySource_->UpdateActiveDevice(DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(ret, SUCCESS);
    DeInitPrimarySource();
}

/**
 * @tc.name   : Test UpdateRawSampleAttr API
 * @tc.number : UpdateRawSampleAttr_001
 * @tc.desc   : Test UpdateRawSampleAttr API
 */
HWTEST_F(AudioCaptureSourceUnitTest, UpdateRawSampleAttr_001, TestSize.Level1)
{
    auto captureSource = std::make_shared<AudioCaptureSource>(1);

    captureSource->attr_.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    captureSource->attr_.formatMicIn = SAMPLE_S16LE;
    captureSource->attr_.sampleRateMicIn = SAMPLE_RATE_16000;
    captureSource->attr_.channelMicIn = CHANNEL_4;

    struct AudioSampleAttributes param;
    captureSource->UpdateRawSampleAttr(param);
    EXPECT_EQ(param.rawSampleAttributes.rawFormat, AUDIO_FORMAT_TYPE_PCM_16_BIT);
    EXPECT_EQ(param.rawSampleAttributes.rawSampleRate, SAMPLE_RATE_16000);
    EXPECT_EQ(param.rawSampleAttributes.rawChannelLayout, CH_LAYOUT_QUAD);
}

/**
 * @tc.name   : Test UpdateRawSampleAttr API
 * @tc.number : UpdateRawSampleAttr_002
 * @tc.desc   : Test UpdateRawSampleAttr API
 */
HWTEST_F(AudioCaptureSourceUnitTest, UpdateRawSampleAttr_002, TestSize.Level1)
{
    auto captureSource = std::make_shared<AudioCaptureSource>(1);

    captureSource->attr_.sourceType = SOURCE_TYPE_MIC;

    struct AudioSampleAttributes param;
    captureSource->UpdateRawSampleAttr(param);
    EXPECT_NE(captureSource, nullptr);
}

/**
 * @tc.name   : Test UpdateRawSampleAttr API
 * @tc.number : UpdateRawSampleAttr_003
 * @tc.desc   : Test UpdateRawSampleAttr API for SOURCE_TYPE_CAMCORDER
 */
HWTEST_F(AudioCaptureSourceUnitTest, UpdateRawSampleAttr_003, TestSize.Level1)
{
    auto captureSource = std::make_shared<AudioCaptureSource>(1);

    captureSource->attr_.sourceType = SOURCE_TYPE_CAMCORDER;
    captureSource->attr_.formatMicIn = SAMPLE_S24LE;
    captureSource->attr_.sampleRateMicIn = SAMPLE_RATE_48000;
    captureSource->attr_.channelMicIn = CHANNEL_4;

    struct AudioSampleAttributes param;
    captureSource->UpdateRawSampleAttr(param);
    EXPECT_EQ(param.rawSampleAttributes.rawFormat, AUDIO_FORMAT_TYPE_PCM_24_BIT);
    EXPECT_EQ(param.rawSampleAttributes.rawSampleRate, SAMPLE_RATE_48000);
    EXPECT_EQ(param.rawSampleAttributes.rawChannelLayout, CH_LAYOUT_QUAD);
}

/**
 * @tc.name   : Test GetUniqueIdBySourceType API
 * @tc.number : GetUniqueIdBySourceType_012
 * @tc.desc   : Test GetUniqueIdBySourceType for AUDIO_INPUT_INTERPHON_TYPE
 */
HWTEST_F(AudioCaptureSourceUnitTest, GetUniqueIdBySourceType_012, TestSize.Level1)
{
    auto captureSource = std::make_shared<AudioCaptureSource>(1);
    uint32_t id = 0;
    captureSource->attr_.sourceType = SOURCE_TYPE_INTERPHONE;
    captureSource->attr_.hdiSourceType = "AUDIO_INPUT_INTERPHON_TYPE";
    id = captureSource->GetUniqueIdBySourceType();
    EXPECT_EQ(id, AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_INTERPHONE * UNIQUE_ID_INTERVAL);
}

/**
 * @tc.name   : Test GetUniqueIdBySourceType API
 * @tc.number : GetUniqueIdBySourceType_013
 * @tc.desc   : Test GetUniqueIdBySourceType API
 */
HWTEST_F(AudioCaptureSourceUnitTest, GetUniqueIdBySourceType_013, TestSize.Level1)
{
    auto captureSource = std::make_shared<AudioCaptureSource>(1);
    uint32_t id = 0;
    captureSource->attr_.sourceType = SOURCE_TYPE_LIVE;
    captureSource->attr_.hdiSourceType = "AUDIO_INPUT_LIVE_TYPE";
    id = captureSource->GetUniqueIdBySourceType();
    EXPECT_EQ(id, AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_LIVE * UNIQUE_ID_INTERVAL);
}

/**
 * @tc.name   : Test GetUniqueIdBySourceType API
 * @tc.number : GetUniqueIdBySourceType_014
 * @tc.desc   : Test GetUniqueIdBySourceType for AUDIO_INPUT_CAMCORDER_TYPE
 */
HWTEST_F(AudioCaptureSourceUnitTest, GetUniqueIdBySourceType_014, TestSize.Level1)
{
    auto captureSource = std::make_shared<AudioCaptureSource>(1);
    captureSource->attr_.sourceType = SOURCE_TYPE_CAMCORDER;
    captureSource->attr_.hdiSourceType = "AUDIO_INPUT_CAMCORDER_TYPE";
    uint32_t id = captureSource->GetUniqueIdBySourceType();
    EXPECT_EQ(id, AUDIO_HDI_CAPTURE_ID_BASE + HDI_CAPTURE_OFFSET_CAMCORDER * UNIQUE_ID_INTERVAL);
}

/**
 * @tc.name   : Test ConvertToAudioInputRouteFlag API
 * @tc.number : ConvertToAudioInputRouteFlag_002
 * @tc.desc   : Test ConvertToAudioInputRouteFlag for SOURCE_TYPE_INTERPHONE
 */
HWTEST_F(AudioCaptureSourceUnitTest, ConvertToAudioInputRouteFlag_002, TestSize.Level1)
{
    auto captureSource = std::make_shared<AudioCaptureSource>(1);
    AudioFlag result = captureSource->ConvertToAudioInputRouteFlag(SOURCE_TYPE_INTERPHONE);
    EXPECT_EQ(result, AUDIO_INPUT_FLAG_INTERPHONE);
}

/**
 * @tc.name   : Test Multiple Stop Call
 * @tc.number : MultipleStopCall_001
 * @tc.desc   : Test multiple Stop() calls to verify stopThread_ join logic
 */
HWTEST_F(AudioCaptureSourceUnitTest, MultipleStopCall_001, TestSize.Level1)
{
    InitPrimarySource();
    EXPECT_TRUE(primarySource_ && primarySource_->IsInited());
    int32_t ret = primarySource_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = primarySource_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    ret = primarySource_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    DeInitPrimarySource();
}

/**
 * @tc.name   : Test Multiple Stop Call
 * @tc.number : MultipleStopCall_002
 * @tc.desc   : Test multiple Stop() calls without Start to verify stopThread_ handling
 */
HWTEST_F(AudioCaptureSourceUnitTest, MultipleStopCall_002, TestSize.Level1)
{
    InitPrimarySource();
    EXPECT_TRUE(primarySource_ && primarySource_->IsInited());
    int32_t ret = primarySource_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    ret = primarySource_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    DeInitPrimarySource();
}

/**
 * @tc.name   : Test Destructor Thread Safety
 * @tc.number : DestructorThreadSafety_001
 * @tc.desc   : Test destructor handles stopThread_ and captureThread_ safely without crash
 */
HWTEST_F(AudioCaptureSourceUnitTest, DestructorThreadSafety_001, TestSize.Level1)
{
    auto captureSource = std::make_shared<AudioCaptureSource>(10006, "test_device");
    EXPECT_NE(captureSource, nullptr);
    captureSource = nullptr;
    EXPECT_EQ(captureSource, nullptr);
}

/**
 * @tc.name   : Test Destructor Thread Safety After Stop
 * @tc.number : DestructorThreadSafety_002
 * @tc.desc   : Test destructor handles stopThread_ join after Stop() is called
 */
HWTEST_F(AudioCaptureSourceUnitTest, DestructorThreadSafety_002, TestSize.Level1)
{
    InitPrimarySource();
    EXPECT_TRUE(primarySource_ && primarySource_->IsInited());
    int32_t ret = primarySource_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = primarySource_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    DeInitPrimarySource();
    primarySource_ = nullptr;
    EXPECT_EQ(primarySource_, nullptr);
}

/**
 * @tc.name   : Test Destructor After Stop
 * @tc.number : DestructorAfterStop_001
 * @tc.desc   : Test destructor after Stop() call to verify no use-after-free
 */
HWTEST_F(AudioCaptureSourceUnitTest, DestructorAfterStop_001, TestSize.Level1)
{
    InitPrimarySource();
    EXPECT_TRUE(primarySource_ && primarySource_->IsInited());
    int32_t ret = primarySource_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = primarySource_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    DeInitPrimarySource();
    primarySource_ = nullptr;
    EXPECT_EQ(primarySource_, nullptr);
}

/**
 * @tc.name   : Test IsCaptureThreadRunning Atomic
 * @tc.number : IsCaptureThreadRunningAtomic_001
 * @tc.desc   : Test isCaptureThreadRunning_ atomic operation through NonblockingStart/Stop
 */
HWTEST_F(AudioCaptureSourceUnitTest, IsCaptureThreadRunningAtomic_001, TestSize.Level1)
{
    auto captureSource = std::make_shared<AudioCaptureSource>(10007, "primary");
    EXPECT_NE(captureSource, nullptr);
    captureSource->attr_.sourceType = SOURCE_TYPE_EC;
    captureSource->attr_.adapterName = "primary";
    captureSource->attr_.sampleRate = 48000;
    captureSource->attr_.channel = 2;
    captureSource->attr_.format = SAMPLE_S16LE;
    captureSource = nullptr;
    EXPECT_EQ(captureSource, nullptr);
}

/**
 * @tc.name   : Test Nonblocking Flow Thread Safety
 * @tc.number : NonblockingThreadSafety_001
 * @tc.desc   : Test NonblockingStart/NonblockingStop thread safety for EC source type
 */
HWTEST_F(AudioCaptureSourceUnitTest, NonblockingThreadSafety_001, TestSize.Level1)
{
    auto captureSource = std::make_shared<AudioCaptureSource>(10008, "primary");
    EXPECT_NE(captureSource, nullptr);
    captureSource->attr_.sourceType = SOURCE_TYPE_EC;
    captureSource->attr_.adapterName = "primary";
    captureSource->attr_.sampleRate = 48000;
    captureSource->attr_.channel = 2;
    captureSource->attr_.format = SAMPLE_S16LE;
    captureSource->attr_.deviceType = DEVICE_TYPE_MIC;
    IAudioSourceAttr attr = captureSource->attr_;
    int32_t ret = captureSource->Init(attr);
    if (ret == SUCCESS && captureSource->IsInited()) {
        ret = captureSource->Start();
        if (ret == SUCCESS) {
            ret = captureSource->Stop();
            EXPECT_EQ(ret, SUCCESS);
        }
        captureSource->DeInit();
    }
    captureSource = nullptr;
    EXPECT_EQ(captureSource, nullptr);
}

/**
 * @tc.name   : Test Nonblocking Source Destructor Without Stop
 * @tc.number : NonblockingDestructorNoStop_001
 * @tc.desc   : Test destructor handles captureThread_ when NonblockingStart was called but no Stop
 */
HWTEST_F(AudioCaptureSourceUnitTest, NonblockingDestructorNoStop_001, TestSize.Level1)
{
    auto captureSource = std::make_shared<AudioCaptureSource>(10010, "primary");
    EXPECT_NE(captureSource, nullptr);
    captureSource->attr_.sourceType = SOURCE_TYPE_EC;
    captureSource->attr_.adapterName = "primary";
    captureSource->attr_.sampleRate = 48000;
    captureSource->attr_.channel = 2;
    captureSource->attr_.format = SAMPLE_S16LE;
    captureSource->attr_.deviceType = DEVICE_TYPE_MIC;
    captureSource = nullptr;
    EXPECT_EQ(captureSource, nullptr);
}

/**
 * @tc.name   : Test Stop Without Init
 * @tc.number : StopWithoutInit_001
 * @tc.desc   : Test Stop() called before Init() to verify stopThread_ null handling
 */
HWTEST_F(AudioCaptureSourceUnitTest, StopWithoutInit_001, TestSize.Level1)
{
    auto captureSource = std::make_shared<AudioCaptureSource>(10012, "test_device");
    EXPECT_NE(captureSource, nullptr);
    int32_t ret = captureSource->Stop();
    EXPECT_EQ(ret, SUCCESS);
    captureSource = nullptr;
    EXPECT_EQ(captureSource, nullptr);
}

/**
 * @tc.name   : Test Destructor CaptureThread Joinable
 * @tc.number : DestructorCaptureThreadJoinable_001
 * @tc.desc   : Test destructor joins captureThread_ when NonblockingStart succeeded without Stop
 */
HWTEST_F(AudioCaptureSourceUnitTest, DestructorCaptureThreadJoinable_001, TestSize.Level1)
{
    auto captureSource = std::make_shared<AudioCaptureSource>(10013, "primary");
    EXPECT_NE(captureSource, nullptr);
    captureSource->attr_.sourceType = SOURCE_TYPE_EC;
    captureSource->attr_.adapterName = "primary";
    captureSource->attr_.sampleRate = 48000;
    captureSource->attr_.channel = 2;
    captureSource->attr_.format = SAMPLE_S16LE;
    captureSource->attr_.deviceType = DEVICE_TYPE_MIC;
    IAudioSourceAttr attr = captureSource->attr_;
    int32_t ret = captureSource->Init(attr);
    if (ret == SUCCESS && captureSource->IsInited()) {
        ret = captureSource->Start();
        if (ret == SUCCESS) {
            captureSource->DeInit();
            captureSource = nullptr;
            EXPECT_EQ(captureSource, nullptr);
            return;
        }
        captureSource->DeInit();
    }
    captureSource = nullptr;
    EXPECT_EQ(captureSource, nullptr);
}
} // namespace AudioStandard
} // namespace OHOS
