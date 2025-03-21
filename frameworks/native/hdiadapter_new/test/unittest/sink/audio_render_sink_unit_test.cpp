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

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
class AudioRenderSinkUnitTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() {}
    virtual void TearDown() {}

    void InitPrimarySink();
    void DeInitPrimarySink();
    void InitUsbSink();
    void DeInitUsbSink();
    void InitDirectSink();
    void DeInitDirectSink();
    void InitVoipSink();
    void DeInitVoipSink();

protected:
    static uint32_t primaryId_;
    static uint32_t usbId_;
    static uint32_t directId_;
    static uint32_t voipId_;
    static std::shared_ptr<IAudioRenderSink> primarySink_;
    static std::shared_ptr<IAudioRenderSink> usbSink_;
    static std::shared_ptr<IAudioRenderSink> directSink_;
    static std::shared_ptr<IAudioRenderSink> voipSink_;
    static IAudioSinkAttr attr_;
    static bool primarySinkInited_;
};

uint32_t AudioRenderSinkUnitTest::primaryId_ = HDI_INVALID_ID;
uint32_t AudioRenderSinkUnitTest::usbId_ = HDI_INVALID_ID;
uint32_t AudioRenderSinkUnitTest::directId_ = HDI_INVALID_ID;
uint32_t AudioRenderSinkUnitTest::voipId_ = HDI_INVALID_ID;
std::shared_ptr<IAudioRenderSink> AudioRenderSinkUnitTest::primarySink_ = nullptr;
std::shared_ptr<IAudioRenderSink> AudioRenderSinkUnitTest::usbSink_ = nullptr;
std::shared_ptr<IAudioRenderSink> AudioRenderSinkUnitTest::directSink_ = nullptr;
std::shared_ptr<IAudioRenderSink> AudioRenderSinkUnitTest::voipSink_ = nullptr;
IAudioSinkAttr AudioRenderSinkUnitTest::attr_ = {};
bool AudioRenderSinkUnitTest::primarySinkInited_ = false;

void AudioRenderSinkUnitTest::SetUpTestCase()
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    primaryId_ = manager.GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_DEFAULT, true);
    usbId_ = manager.GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_USB, true);
    directId_ = manager.GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_DIRECT, true);
    voipId_ = manager.GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_VOIP, true);
}

void AudioRenderSinkUnitTest::TearDownTestCase()
{
    HdiAdapterManager::GetInstance().ReleaseId(primaryId_);
    HdiAdapterManager::GetInstance().ReleaseId(usbId_);
    HdiAdapterManager::GetInstance().ReleaseId(directId_);
    HdiAdapterManager::GetInstance().ReleaseId(voipId_);
}

void AudioRenderSinkUnitTest::InitPrimarySink()
{
    primarySink_ = HdiAdapterManager::GetInstance().GetRenderSink(primaryId_, true);
    if (primarySink_ == nullptr) {
        return;
    }
    if (!primarySink_->IsInited()) {
        attr_.adapterName = "primary";
        attr_.sampleRate = 48000; // 48000: sample rate
        attr_.channel = 2; // 2: channel
        attr_.format = SAMPLE_S16LE;
        attr_.channelLayout = 3; // 3: channel layout
        attr_.deviceType = DEVICE_TYPE_SPEAKER;
        attr_.volume = 1.0f;
        attr_.openMicSpeaker = 1;
        primarySink_->Init(attr_);
    } else {
        primarySinkInited_ = true;
    }
}

void AudioRenderSinkUnitTest::DeInitPrimarySink()
{
    if (primarySink_ && primarySink_->IsInited() && !primarySinkInited_) {
        std::vector<DeviceType> deviceTypes = { DEVICE_TYPE_SPEAKER };
        primarySink_->UpdateActiveDevice(deviceTypes);
        primarySink_->DeInit();
    }
    primarySink_ = nullptr;
}

void AudioRenderSinkUnitTest::InitUsbSink()
{
    usbSink_ = HdiAdapterManager::GetInstance().GetRenderSink(usbId_, true);
    if (usbSink_ == nullptr) {
        return;
    }
    attr_.adapterName = "primary";
    attr_.sampleRate = 48000; // 48000: sample rate
    attr_.channel = 2; // 2: channel
    attr_.format = SAMPLE_S16LE;
    attr_.channelLayout = 3; // 3: channel layout
    attr_.deviceType = DEVICE_TYPE_USB_HEADSET;
    attr_.volume = 1.0f;
    attr_.openMicSpeaker = 1;
    usbSink_->Init(attr_);
}

void AudioRenderSinkUnitTest::DeInitUsbSink()
{
    if (usbSink_ && usbSink_->IsInited()) {
        std::vector<DeviceType> deviceTypes = { DEVICE_TYPE_SPEAKER };
        usbSink_->UpdateActiveDevice(deviceTypes);
        usbSink_->DeInit();
    }
    usbSink_ = nullptr;
}

void AudioRenderSinkUnitTest::InitDirectSink()
{
    directSink_ = HdiAdapterManager::GetInstance().GetRenderSink(directId_, true);
    if (directSink_ == nullptr) {
        return;
    }
    attr_.adapterName = "primary";
    attr_.sampleRate = 48000; // 48000: sample rate
    attr_.channel = 2; // 2: channel
    attr_.format = SAMPLE_S32LE;
    attr_.channelLayout = 3; // 3: channel layout
    attr_.deviceType = DEVICE_TYPE_WIRED_HEADSET;
    attr_.volume = 1.0f;
    attr_.openMicSpeaker = 1;
    directSink_->Init(attr_);
}

void AudioRenderSinkUnitTest::DeInitDirectSink()
{
    if (directSink_ && directSink_->IsInited()) {
        std::vector<DeviceType> deviceTypes = { DEVICE_TYPE_SPEAKER };
        directSink_->UpdateActiveDevice(deviceTypes);
        directSink_->DeInit();
    }
    directSink_ = nullptr;
}

void AudioRenderSinkUnitTest::InitVoipSink()
{
    voipSink_ = HdiAdapterManager::GetInstance().GetRenderSink(voipId_, true);
    if (voipSink_ == nullptr) {
        return;
    }
    attr_.adapterName = "primary";
    attr_.sampleRate = 48000; // 48000: sample rate
    attr_.channel = 2; // 2: channel
    attr_.format = SAMPLE_S16LE;
    attr_.channelLayout = 3; // 3: channel layout
    attr_.deviceType = DEVICE_TYPE_SPEAKER;
    attr_.volume = 1.0f;
    attr_.openMicSpeaker = 1;
    voipSink_->Init(attr_);
}

void AudioRenderSinkUnitTest::DeInitVoipSink()
{
    if (voipSink_ && voipSink_->IsInited()) {
        std::vector<DeviceType> deviceTypes = { DEVICE_TYPE_SPEAKER };
        voipSink_->UpdateActiveDevice(deviceTypes);
        voipSink_->DeInit();
    }
    voipSink_ = nullptr;
}

/**
 * @tc.name   : Test PrimarySink API
 * @tc.number : PrimarySinkUnitTest_001
 * @tc.desc   : Test primary sink create
 */
HWTEST_F(AudioRenderSinkUnitTest, PrimarySinkUnitTest_001, TestSize.Level1)
{
    InitPrimarySink();
    EXPECT_TRUE(primarySink_ != nullptr);
    DeInitPrimarySink();
}

/**
 * @tc.name   : Test PrimarySink API
 * @tc.number : PrimarySinkUnitTest_002
 * @tc.desc   : Test primary sink init
 */
HWTEST_F(AudioRenderSinkUnitTest, PrimarySinkUnitTest_002, TestSize.Level1)
{
    InitPrimarySink();
    EXPECT_TRUE(primarySink_ && primarySink_->IsInited());
    if (!primarySinkInited_) {
        primarySink_->DeInit();
        attr_.deviceType = DEVICE_TYPE_BLUETOOTH_SCO;
        int32_t ret = primarySink_->Init(attr_);
        EXPECT_EQ(ret, SUCCESS);
        ret = primarySink_->Init(attr_);
        EXPECT_EQ(ret, SUCCESS);
        EXPECT_TRUE(primarySink_->IsInited());
    }
    DeInitPrimarySink();
}

/**
 * @tc.name   : Test PrimarySink API
 * @tc.number : PrimarySinkUnitTest_003
 * @tc.desc   : Test primary sink start, stop
 */
HWTEST_F(AudioRenderSinkUnitTest, PrimarySinkUnitTest_003, TestSize.Level1)
{
    InitPrimarySink();
    EXPECT_TRUE(primarySink_ && primarySink_->IsInited());
    int32_t ret = primarySink_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = primarySink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    ret = primarySink_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = primarySink_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = primarySink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    DeInitPrimarySink();
}

/**
 * @tc.name   : Test PrimarySink API
 * @tc.number : PrimarySinkUnitTest_004
 * @tc.desc   : Test primary sink resume
 */
HWTEST_F(AudioRenderSinkUnitTest, PrimarySinkUnitTest_004, TestSize.Level1)
{
    InitPrimarySink();
    EXPECT_TRUE(primarySink_ && primarySink_->IsInited());
    int32_t ret = primarySink_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = primarySink_->Resume();
    EXPECT_EQ(ret, SUCCESS);
    ret = primarySink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    DeInitPrimarySink();
}

/**
 * @tc.name   : Test PrimarySink API
 * @tc.number : PrimarySinkUnitTest_005
 * @tc.desc   : Test primary sink set volume
 */
HWTEST_F(AudioRenderSinkUnitTest, PrimarySinkUnitTest_005, TestSize.Level1)
{
    InitPrimarySink();
    EXPECT_TRUE(primarySink_ && primarySink_->IsInited());
    int32_t ret = primarySink_->SetVolume(1.0f, 1.0f);
    EXPECT_NE(ret, SUCCESS);
    DeInitPrimarySink();
}

/**
 * @tc.name   : Test UsbSink API
 * @tc.number : UsbSinkUnitTest_001
 * @tc.desc   : Test usb sink create
 */
HWTEST_F(AudioRenderSinkUnitTest, UsbSinkUnitTest_001, TestSize.Level1)
{
    InitUsbSink();
    EXPECT_TRUE(usbSink_ != nullptr);
    DeInitUsbSink();
}

/**
 * @tc.name   : Test UsbSink API
 * @tc.number : UsbSinkUnitTest_002
 * @tc.desc   : Test usb sink init
 */
HWTEST_F(AudioRenderSinkUnitTest, UsbSinkUnitTest_002, TestSize.Level1)
{
    InitUsbSink();
    EXPECT_TRUE(usbSink_ && usbSink_->IsInited());
    usbSink_->DeInit();
    attr_.deviceType = DEVICE_TYPE_BLUETOOTH_SCO;
    int32_t ret = usbSink_->Init(attr_);
    EXPECT_EQ(ret, SUCCESS);
    ret = usbSink_->Init(attr_);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(usbSink_->IsInited());
    DeInitUsbSink();
}

/**
 * @tc.name   : Test UsbSink API
 * @tc.number : UsbSinkUnitTest_003
 * @tc.desc   : Test usb sink start, stop
 */
HWTEST_F(AudioRenderSinkUnitTest, UsbSinkUnitTest_003, TestSize.Level1)
{
    InitUsbSink();
    EXPECT_TRUE(usbSink_ && usbSink_->IsInited());
    int32_t ret = usbSink_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = usbSink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    ret = usbSink_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = usbSink_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = usbSink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    DeInitUsbSink();
}

/**
 * @tc.name   : Test UsbSink API
 * @tc.number : UsbSinkUnitTest_004
 * @tc.desc   : Test usb sink resume
 */
HWTEST_F(AudioRenderSinkUnitTest, UsbSinkUnitTest_004, TestSize.Level1)
{
    InitUsbSink();
    EXPECT_TRUE(usbSink_ && usbSink_->IsInited());
    int32_t ret = usbSink_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = usbSink_->Resume();
    EXPECT_EQ(ret, SUCCESS);
    ret = usbSink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    DeInitUsbSink();
}

/**
 * @tc.name   : Test UsbSink API
 * @tc.number : UsbSinkUnitTest_005
 * @tc.desc   : Test usb sink set volume
 */
HWTEST_F(AudioRenderSinkUnitTest, UsbSinkUnitTest_005, TestSize.Level1)
{
    InitUsbSink();
    EXPECT_TRUE(usbSink_ && usbSink_->IsInited());
    int32_t ret = usbSink_->SetVolume(1.0f, 1.0f);
    EXPECT_NE(ret, SUCCESS);
    DeInitUsbSink();
}

/**
 * @tc.name   : Test DirectSink API
 * @tc.number : DirectSinkUnitTest_001
 * @tc.desc   : Test direct sink create
 */
HWTEST_F(AudioRenderSinkUnitTest, DirectSinkUnitTest_001, TestSize.Level1)
{
    InitDirectSink();
    EXPECT_TRUE(directSink_ != nullptr);
    DeInitDirectSink();
}

/**
 * @tc.name   : Test DirectSink API
 * @tc.number : DirectSinkUnitTest_002
 * @tc.desc   : Test direct sink init
 */
HWTEST_F(AudioRenderSinkUnitTest, DirectSinkUnitTest_002, TestSize.Level1)
{
    InitDirectSink();
    EXPECT_TRUE(directSink_ && directSink_->IsInited());
    directSink_->DeInit();
    int32_t ret = directSink_->Init(attr_);
    EXPECT_EQ(ret, SUCCESS);
    ret = directSink_->Init(attr_);
    EXPECT_EQ(ret, SUCCESS);
    attr_.sampleRate = 192000;
    ret = directSink_->Init(attr_);
    EXPECT_EQ(ret, SUCCESS);
    attr_.deviceType = DEVICE_TYPE_USB_HEADSET;
    ret = directSink_->Init(attr_);
    EXPECT_TRUE(directSink_->IsInited());
    DeInitDirectSink();
}

/**
 * @tc.name   : Test DirectSink API
 * @tc.number : DirectSinkUnitTest_003
 * @tc.desc   : Test direct sink deinit
 */
HWTEST_F(AudioRenderSinkUnitTest, DirectSinkUnitTest_003, TestSize.Level1)
{
    InitDirectSink();
    EXPECT_TRUE(directSink_ && directSink_->IsInited());
    directSink_->DeInit();
    EXPECT_FALSE(directSink_->IsInited());
    DeInitDirectSink();
}

/**
 * @tc.name   : Test DirectSink API
 * @tc.number : DirectSinkUnitTest_004
 * @tc.desc   : Test direct sink start, stop
 */
HWTEST_F(AudioRenderSinkUnitTest, DirectSinkUnitTest_004, TestSize.Level1)
{
    InitDirectSink();
    EXPECT_TRUE(directSink_ && directSink_->IsInited());
    int32_t ret = directSink_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = directSink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    ret = directSink_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = directSink_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = directSink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    DeInitDirectSink();
}

/**
 * @tc.name   : Test DirectSink API
 * @tc.number : DirectSinkUnitTest_005
 * @tc.desc   : Test direct sink resume
 */
HWTEST_F(AudioRenderSinkUnitTest, DirectSinkUnitTest_005, TestSize.Level1)
{
    InitDirectSink();
    EXPECT_TRUE(directSink_ && directSink_->IsInited());
    int32_t ret = directSink_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = directSink_->Resume();
    EXPECT_EQ(ret, SUCCESS);
    ret = directSink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    DeInitDirectSink();
}

/**
 * @tc.name   : Test DirectSink API
 * @tc.number : DirectSinkUnitTest_006
 * @tc.desc   : Test direct sink render frame
 */
HWTEST_F(AudioRenderSinkUnitTest, DirectSinkUnitTest_006, TestSize.Level1)
{
    InitDirectSink();
    EXPECT_TRUE(directSink_ && directSink_->IsInited());
    uint64_t writeLen = 0;
    std::vector<char> buffer{'8', '8', '8', '8', '8', '8', '8', '8'};
    int32_t ret = directSink_->RenderFrame(*buffer.data(), buffer.size(), writeLen);
    EXPECT_EQ(ret, SUCCESS);
    directSink_->SetAudioMonoState(true);
    directSink_->SetAudioBalanceValue(1.0f);
    ret = directSink_->RenderFrame(*buffer.data(), buffer.size(), writeLen);
    EXPECT_EQ(ret, SUCCESS);
    directSink_->SetAudioBalanceValue(-1.0f);
    ret = directSink_->RenderFrame(*buffer.data(), buffer.size(), writeLen);
    EXPECT_EQ(ret, SUCCESS);
    attr_.format = SAMPLE_U8;
    ret = directSink_->Init(attr_);
    EXPECT_EQ(ret, SUCCESS);
    ret = directSink_->RenderFrame(*buffer.data(), buffer.size(), writeLen);
    EXPECT_EQ(ret, SUCCESS);
    attr_.format = SAMPLE_S16LE;
    ret = directSink_->Init(attr_);
    EXPECT_EQ(ret, SUCCESS);
    ret = directSink_->RenderFrame(*buffer.data(), buffer.size(), writeLen);
    EXPECT_EQ(ret, SUCCESS);
    attr_.format = SAMPLE_S24LE;
    ret = directSink_->Init(attr_);
    EXPECT_EQ(ret, SUCCESS);
    ret = directSink_->RenderFrame(*buffer.data(), buffer.size(), writeLen);
    EXPECT_EQ(ret, SUCCESS);
    attr_.format = INVALID_WIDTH;
    ret = directSink_->Init(attr_);
    EXPECT_EQ(ret, SUCCESS);
    ret = directSink_->RenderFrame(*buffer.data(), buffer.size(), writeLen);
    EXPECT_EQ(ret, SUCCESS);
    DeInitDirectSink();
}

/**
 * @tc.name   : Test DirectSink API
 * @tc.number : DirectSinkUnitTest_007
 * @tc.desc   : Test direct sink set volume
 */
HWTEST_F(AudioRenderSinkUnitTest, DirectSinkUnitTest_007, TestSize.Level1)
{
    InitDirectSink();
    EXPECT_TRUE(directSink_ && directSink_->IsInited());
    int32_t ret = directSink_->SetVolume(0.0f, 0.0f);
    EXPECT_EQ(ret, SUCCESS);
    ret = directSink_->SetVolume(0.0f, 1.0f);
    EXPECT_EQ(ret, SUCCESS);
    ret = directSink_->SetVolume(1.0f, 0.0f);
    EXPECT_EQ(ret, SUCCESS);
    ret = directSink_->SetVolume(1.0f, 1.0f);
    EXPECT_EQ(ret, SUCCESS);
    DeInitDirectSink();
}

/**
 * @tc.name   : Test DirectSink API
 * @tc.number : DirectSinkUnitTest_008
 * @tc.desc   : Test direct sink resume, pause, reset, flush, get presentation position, set pa power
 */
HWTEST_F(AudioRenderSinkUnitTest, DirectSinkUnitTest_008, TestSize.Level1)
{
    InitDirectSink();
    EXPECT_TRUE(directSink_ && directSink_->IsInited());
    int32_t ret = directSink_->Reset();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    ret = directSink_->Flush();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    ret = directSink_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = directSink_->Resume();
    EXPECT_EQ(ret, SUCCESS);
    ret = directSink_->Pause();
    EXPECT_NE(ret, SUCCESS);
    ret = directSink_->Resume();
    EXPECT_EQ(ret, SUCCESS);
    uint64_t frame = 10;
    int64_t timeSec = 10;
    int64_t timeNanoSec = 10;
    ret = directSink_->GetPresentationPosition(frame, timeSec, timeNanoSec);
    EXPECT_EQ(ret, SUCCESS);
    ret = directSink_->SetPaPower(1);
    EXPECT_EQ(ret, SUCCESS);
    ret = directSink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    DeInitDirectSink();
}

/**
 * @tc.name   : Test VoipSink API
 * @tc.number : VoipSinkUnitTest_001
 * @tc.desc   : Test voip sink create
 */
HWTEST_F(AudioRenderSinkUnitTest, VoipSinkUnitTest_001, TestSize.Level1)
{
    InitVoipSink();
    EXPECT_TRUE(voipSink_ != nullptr);
    DeInitVoipSink();
}

/**
 * @tc.name   : Test VoipSink API
 * @tc.number : VoipSinkUnitTest_002
 * @tc.desc   : Test voip sink init
 */
HWTEST_F(AudioRenderSinkUnitTest, VoipSinkUnitTest_002, TestSize.Level1)
{
    InitVoipSink();
    EXPECT_TRUE(voipSink_ && voipSink_->IsInited());
    voipSink_->DeInit();
    attr_.deviceType = DEVICE_TYPE_BLUETOOTH_SCO;
    int32_t ret = voipSink_->Init(attr_);
    EXPECT_EQ(ret, SUCCESS);
    ret = voipSink_->Init(attr_);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(voipSink_->IsInited());
    DeInitVoipSink();
}

/**
 * @tc.name   : Test VoipSink API
 * @tc.number : VoipSinkUnitTest_003
 * @tc.desc   : Test voip sink start, stop
 */
HWTEST_F(AudioRenderSinkUnitTest, VoipSinkUnitTest_003, TestSize.Level1)
{
    InitVoipSink();
    EXPECT_TRUE(voipSink_ && voipSink_->IsInited());
    int32_t ret = voipSink_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = voipSink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    ret = voipSink_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = voipSink_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = voipSink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    DeInitVoipSink();
}

/**
 * @tc.name   : Test VoipSink API
 * @tc.number : VoipSinkUnitTest_004
 * @tc.desc   : Test voip sink resume
 */
HWTEST_F(AudioRenderSinkUnitTest, VoipSinkUnitTest_004, TestSize.Level1)
{
    InitVoipSink();
    EXPECT_TRUE(voipSink_ && voipSink_->IsInited());
    int32_t ret = voipSink_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = voipSink_->Resume();
    EXPECT_EQ(ret, SUCCESS);
    ret = voipSink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    DeInitVoipSink();
}

/**
 * @tc.name   : Test VoipSink API
 * @tc.number : VoipSinkUnitTest_005
 * @tc.desc   : Test voip sink set volume
 */
HWTEST_F(AudioRenderSinkUnitTest, VoipSinkUnitTest_005, TestSize.Level1)
{
    InitVoipSink();
    EXPECT_TRUE(voipSink_ && voipSink_->IsInited());
    int32_t ret = voipSink_->SetVolume(1.0f, 1.0f);
    EXPECT_EQ(ret, SUCCESS);
    DeInitVoipSink();
}

} // namespace AudioStandard
} // namespace OHOS
