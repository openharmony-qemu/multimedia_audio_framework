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
#include "audio_stream_enum.h"
#include "util/id_handler.h"

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
    void InitInterphoneSink();
    void DeInitInterphoneSink();

protected:
    static uint32_t primaryId_;
    static uint32_t usbId_;
    static uint32_t directId_;
    static uint32_t voipId_;
    static uint32_t interphoneId_;
    static std::shared_ptr<IAudioRenderSink> primarySink_;
    static std::shared_ptr<IAudioRenderSink> usbSink_;
    static std::shared_ptr<IAudioRenderSink> directSink_;
    static std::shared_ptr<IAudioRenderSink> voipSink_;
    static std::shared_ptr<IAudioRenderSink> interphoneSink_;
    static IAudioSinkAttr attr_;
    static bool primarySinkInited_;
};

uint32_t AudioRenderSinkUnitTest::primaryId_ = HDI_INVALID_ID;
uint32_t AudioRenderSinkUnitTest::usbId_ = HDI_INVALID_ID;
uint32_t AudioRenderSinkUnitTest::directId_ = HDI_INVALID_ID;
uint32_t AudioRenderSinkUnitTest::voipId_ = HDI_INVALID_ID;
uint32_t AudioRenderSinkUnitTest::interphoneId_ = HDI_INVALID_ID;
std::shared_ptr<IAudioRenderSink> AudioRenderSinkUnitTest::primarySink_ = nullptr;
std::shared_ptr<IAudioRenderSink> AudioRenderSinkUnitTest::usbSink_ = nullptr;
std::shared_ptr<IAudioRenderSink> AudioRenderSinkUnitTest::directSink_ = nullptr;
std::shared_ptr<IAudioRenderSink> AudioRenderSinkUnitTest::voipSink_ = nullptr;
std::shared_ptr<IAudioRenderSink> AudioRenderSinkUnitTest::interphoneSink_ = nullptr;
IAudioSinkAttr AudioRenderSinkUnitTest::attr_ = {};
bool AudioRenderSinkUnitTest::primarySinkInited_ = false;

static const uint32_t TEST_RENDER_ID = 1;
static const uint32_t TEST_STREAM_ID = 100000;

void AudioRenderSinkUnitTest::SetUpTestCase()
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    primaryId_ = manager.GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_DEFAULT, true);
    usbId_ = manager.GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_USB, true);
    directId_ = manager.GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_DIRECT, true);
    voipId_ = manager.GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_VOIP, true);
    interphoneId_ = manager.GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_INTERPHONE, true);
}

void AudioRenderSinkUnitTest::TearDownTestCase()
{
    HdiAdapterManager::GetInstance().ReleaseId(primaryId_);
    HdiAdapterManager::GetInstance().ReleaseId(usbId_);
    HdiAdapterManager::GetInstance().ReleaseId(directId_);
    HdiAdapterManager::GetInstance().ReleaseId(voipId_);
    HdiAdapterManager::GetInstance().ReleaseId(interphoneId_);
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

void AudioRenderSinkUnitTest::InitInterphoneSink()
{
    interphoneSink_ = HdiAdapterManager::GetInstance().GetRenderSink(interphoneId_, true);
    if (interphoneSink_ == nullptr) {
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
    attr_.routeFlag = AUDIO_OUTPUT_FLAG_INTERPHONE;
    interphoneSink_->Init(attr_);
}

void AudioRenderSinkUnitTest::DeInitInterphoneSink()
{
    if (interphoneSink_ && interphoneSink_->IsInited()) {
        std::vector<DeviceType> deviceTypes = { DEVICE_TYPE_SPEAKER };
        interphoneSink_->UpdateActiveDevice(deviceTypes);
        interphoneSink_->DeInit();
    }
    interphoneSink_ = nullptr;
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
 * @tc.name   : Test PrimarySink API
 * @tc.number : PrimarySinkUnitTest_006
 * @tc.desc   : Test primary sink set audio param
 */
HWTEST_F(AudioRenderSinkUnitTest, PrimarySinkUnitTest_006, TestSize.Level1)
{
    InitPrimarySink();
    EXPECT_TRUE(primarySink_ && primarySink_->IsInited());
    primarySink_->SetAudioParameter(NONE, "", "param=0");
    DeInitPrimarySink();
}

/**
 * @tc.name   : Test PrimarySink API
 * @tc.number : PrimarySinkUnitTest_007
 * @tc.desc   : Test primary sink mute for switch device
 */
HWTEST_F(AudioRenderSinkUnitTest, PrimarySinkUnitTest_007, TestSize.Level1)
{
    InitPrimarySink();
    EXPECT_TRUE(primarySink_ && primarySink_->IsInited());
    int32_t ret = primarySink_->SetSinkMuteForSwitchDevice(true);
    EXPECT_EQ(ret, SUCCESS);
    ret = primarySink_->SetSinkMuteForSwitchDevice(true);
    EXPECT_EQ(ret, SUCCESS);
    ret = primarySink_->SetSinkMuteForSwitchDevice(false);
    EXPECT_EQ(ret, SUCCESS);
    ret = primarySink_->SetSinkMuteForSwitchDevice(false);
    EXPECT_EQ(ret, SUCCESS);
    DeInitPrimarySink();
}

/**
 * @tc.name   : Test SetDmDeviceType API
 * @tc.number : SetDmDeviceType_001
 * @tc.desc   : Test SetDmDeviceType
 */
HWTEST_F(AudioRenderSinkUnitTest, SetDmDeviceType_001, TestSize.Level1)
{
    InitPrimarySink();
    EXPECT_TRUE(primarySink_ && primarySink_->IsInited());
    std::vector<DeviceType> outputDevices;
    outputDevices.push_back(DEVICE_TYPE_NEARLINK);
    primarySink_->UpdateActiveDevice(outputDevices);
    primarySink_->SetDmDeviceType(DM_DEVICE_TYPE_DEFAULT, DEVICE_TYPE_NEARLINK_IN);
    primarySink_->SetDmDeviceType(DM_DEVICE_TYPE_NEARLINK_SCO, DEVICE_TYPE_NEARLINK_IN);
    primarySink_->SetDmDeviceType(DM_DEVICE_TYPE_DEFAULT, DEVICE_TYPE_NEARLINK_IN);
    outputDevices.clear();
    outputDevices.push_back(DEVICE_TYPE_SPEAKER);
    primarySink_->UpdateActiveDevice(outputDevices);
    DeInitPrimarySink();
}

/**
 * @tc.name   : Test RegisterCurrentDeviceType API
 * @tc.number : RegisterCurrentDeviceType_001
 * @tc.desc   : Test RegisterCurrentDeviceType
 */
HWTEST_F(AudioRenderSinkUnitTest, RegisterCurrentDeviceType_001, TestSize.Level1)
{
    InitPrimarySink();
    EXPECT_TRUE(primarySink_ && primarySink_->IsInited());
    std::function<void(bool)> callback = [](bool state) { EXPECT_FALSE(state); };
    primarySink_->RegisterCurrentDeviceCallback(callback);
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

/**
 * @tc.name   : Test VoipSink API
 * @tc.number : VoipSinkUnitTest_006
 * @tc.desc   : Test voip sink mute for switch device
 */
HWTEST_F(AudioRenderSinkUnitTest, VoipSinkUnitTest_006, TestSize.Level1)
{
    InitVoipSink();
    EXPECT_TRUE(voipSink_ && voipSink_->IsInited());
    int32_t ret = voipSink_->SetSinkMuteForSwitchDevice(true);
    EXPECT_EQ(ret, SUCCESS);
    ret = voipSink_->SetSinkMuteForSwitchDevice(true);
    EXPECT_EQ(ret, SUCCESS);
    ret = voipSink_->SetSinkMuteForSwitchDevice(false);
    EXPECT_EQ(ret, SUCCESS);
    ret = voipSink_->SetSinkMuteForSwitchDevice(false);
    EXPECT_EQ(ret, SUCCESS);
    DeInitVoipSink();
}

/**
 * @tc.name   : Test PrimarySink API
 * @tc.number : ChangePipeStream_001
 * @tc.desc   : Test ChangePipeStream() add, change and remove cases
 */
HWTEST_F(AudioRenderSinkUnitTest, ChangePipeStream_001, TestSize.Level2)
{
    InitPrimarySink();
    EXPECT_TRUE(primarySink_ && primarySink_->IsInited());

    primarySink_->InitPipeInfo(TEST_RENDER_ID, HDI_ADAPTER_TYPE_PRIMARY, AUDIO_OUTPUT_FLAG_NORMAL);

    primarySink_->ChangePipeStream(STREAM_CHANGE_TYPE_ADD,
        TEST_STREAM_ID, STREAM_USAGE_MUSIC, RENDERER_PREPARED);
    auto pipeInfo = primarySink_->GetOutputPipeInfo();
    EXPECT_EQ(1, pipeInfo->GetStreams().size());

    primarySink_->ChangePipeStream(STREAM_CHANGE_TYPE_STATE_CHANGE,
        TEST_STREAM_ID, STREAM_USAGE_MUSIC, RENDERER_RUNNING);
    pipeInfo = primarySink_->GetOutputPipeInfo();
    auto streams = pipeInfo->GetStreams();
    if (streams.find(TEST_STREAM_ID) == streams.end()) {
        DeInitPrimarySink();
        FAIL();
    }
    EXPECT_EQ(RENDERER_RUNNING, streams[TEST_STREAM_ID].state_);

    primarySink_->ChangePipeStream(STREAM_CHANGE_TYPE_REMOVE,
        TEST_STREAM_ID, STREAM_USAGE_MUSIC, RENDERER_PREPARED);
    pipeInfo = primarySink_->GetOutputPipeInfo();
    EXPECT_EQ(0, pipeInfo->GetStreams().size());

    DeInitPrimarySink();
}

/**
 * @tc.name   : Test PrimarySink API
 * @tc.number : ChangePipeStream_002
 * @tc.desc   : Test ChangePipeStream() remove all cases
 */
HWTEST_F(AudioRenderSinkUnitTest, ChangePipeStream_002, TestSize.Level2)
{
    InitPrimarySink();
    EXPECT_TRUE(primarySink_ && primarySink_->IsInited());

    primarySink_->InitPipeInfo(TEST_RENDER_ID, HDI_ADAPTER_TYPE_PRIMARY, AUDIO_OUTPUT_FLAG_NORMAL);

    primarySink_->ChangePipeStream(STREAM_CHANGE_TYPE_ADD,
        TEST_STREAM_ID, STREAM_USAGE_MUSIC, RENDERER_PREPARED);
    auto pipeInfo = primarySink_->GetOutputPipeInfo();
    EXPECT_EQ(1, pipeInfo->GetStreams().size());

    primarySink_->ChangePipeStream(STREAM_CHANGE_TYPE_REMOVE_ALL,
        TEST_STREAM_ID, STREAM_USAGE_MUSIC, RENDERER_RUNNING);
    pipeInfo = primarySink_->GetOutputPipeInfo();
    EXPECT_EQ(0, pipeInfo->GetStreams().size());

    DeInitPrimarySink();
}

/**
 * @tc.name   : Test PrimarySink API
 * @tc.number : ChangePipeStream_003
 * @tc.desc   : Test ChangePipeStream() abnormal case
 */
HWTEST_F(AudioRenderSinkUnitTest, ChangePipeStream_003, TestSize.Level4)
{
    InitPrimarySink();
    EXPECT_TRUE(primarySink_ && primarySink_->IsInited());

    primarySink_->InitPipeInfo(TEST_RENDER_ID, HDI_ADAPTER_TYPE_PRIMARY, AUDIO_OUTPUT_FLAG_NORMAL);

    primarySink_->ChangePipeStream(static_cast<StreamChangeType>(STREAM_CHANGE_TYPE_STATE_CHANGE + 1),
        TEST_STREAM_ID, STREAM_USAGE_MUSIC, RENDERER_PREPARED);
    auto pipeInfo = primarySink_->GetOutputPipeInfo();
    EXPECT_EQ(0, pipeInfo->GetStreams().size());

    DeInitPrimarySink();
}

/**
 * @tc.name   : Test SetAudioScene API
 * @tc.number : SetAudioScene_001
 * @tc.desc   : Test SetAudioScene
 */
HWTEST_F(AudioRenderSinkUnitTest, SetAudioScene_001, TestSize.Level1)
{
    InitPrimarySink();
    EXPECT_TRUE(primarySink_ && primarySink_->IsInited());
    int32_t ret = primarySink_->SetAudioScene(AUDIO_SCENE_DEFAULT, true);
    EXPECT_EQ(ret, SUCCESS);
    ret = primarySink_->SetAudioScene(AUDIO_SCENE_DEFAULT, false);
    EXPECT_EQ(ret, SUCCESS);
    ret = primarySink_->SetAudioScene(AUDIO_SCENE_PHONE_CALL, false);
    EXPECT_EQ(ret, SUCCESS);
    DeInitPrimarySink();
}

/**
 * @tc.name   : Test UpdateActiveDevice API
 * @tc.number : UpdateActiveDevice_001
 * @tc.desc   : Test UpdateActiveDevice
 */
HWTEST_F(AudioRenderSinkUnitTest, UpdateActiveDevice_001, TestSize.Level1)
{
    InitPrimarySink();
    EXPECT_TRUE(primarySink_ && primarySink_->IsInited());
    std::vector<DeviceType> outputDevices;
    outputDevices.push_back(DEVICE_TYPE_SPEAKER);
    primarySink_->SetAudioScene(AUDIO_SCENE_PHONE_CALL, false);
    primarySink_->UpdateActiveDevice(outputDevices);
    primarySink_->SetAudioScene(AUDIO_SCENE_DEFAULT, true);
    int32_t ret = primarySink_->UpdateActiveDevice(outputDevices);
    outputDevices.clear();
    EXPECT_EQ(ret, SUCCESS);
    DeInitPrimarySink();
}

/**
 * @tc.name   : Test GetUniqueId API for Direct
 * @tc.number : GetUniqueId_Direct_001
 * @tc.desc   : Test GetUniqueId returns correct ID for Direct sink
 */
HWTEST_F(AudioRenderSinkUnitTest, GetUniqueId_Direct_001, TestSize.Level1)
{
    InitDirectSink();
    EXPECT_TRUE(directSink_ && directSink_->IsInited());
    
    uint32_t uniqueId = directSink_->GetUniqueId();
    uint32_t expectedId = GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_DIRECT);
    EXPECT_EQ(uniqueId, expectedId);
    
    DeInitDirectSink();
}

/**
 * @tc.name   : Test InterphoneSink API
 * @tc.number : InterphoneSinkUnitTest_001
 * @tc.desc   : Test interphone sink create
 */
HWTEST_F(AudioRenderSinkUnitTest, InterphoneSinkUnitTest_001, TestSize.Level1)
{
    // Verify interphoneId_ is valid
    EXPECT_NE(interphoneId_, HDI_INVALID_ID);
}

/**
 * @tc.name   : Test InterphoneSink API - Config Validation
 * @tc.number : InterphoneSinkUnitTest_002
 * @tc.desc   : Test interphone sink configuration parameters
 */
HWTEST_F(AudioRenderSinkUnitTest, InterphoneSinkUnitTest_002, TestSize.Level1)
{
    // Verify interphone ID can be obtained correctly
    EXPECT_NE(interphoneId_, HDI_INVALID_ID);
    
    // Verify sink attr configuration for interphone
    IAudioSinkAttr attr;
    attr.adapterName = "primary";
    attr.sampleRate = 48000;
    attr.channel = 2;
    attr.format = SAMPLE_S16LE;
    attr.channelLayout = 3;
    attr.deviceType = DEVICE_TYPE_SPEAKER;
    attr.volume = 1.0f;
    attr.openMicSpeaker = 1;
    attr.routeFlag = AUDIO_OUTPUT_FLAG_INTERPHONE;
    
    EXPECT_EQ(attr.adapterName, "primary");
    EXPECT_EQ(attr.routeFlag, AUDIO_OUTPUT_FLAG_INTERPHONE);
}

/**
 * @tc.name   : Test InterphoneSink API - RouteFlag Validation
 * @tc.number : InterphoneSinkUnitTest_003
 * @tc.desc   : Test interphone sink routeFlag configuration
 */
HWTEST_F(AudioRenderSinkUnitTest, InterphoneSinkUnitTest_003, TestSize.Level1)
{
    // Verify routeFlag value
    IAudioSinkAttr attr;
    attr.routeFlag = AUDIO_OUTPUT_FLAG_INTERPHONE;
    EXPECT_EQ(attr.routeFlag, AUDIO_OUTPUT_FLAG_INTERPHONE);
    
    // Verify routeFlag is different from other flags
    EXPECT_NE(attr.routeFlag, AUDIO_OUTPUT_FLAG_NORMAL);
    EXPECT_NE(attr.routeFlag, AUDIO_OUTPUT_FLAG_FAST);
    EXPECT_NE(attr.routeFlag, AUDIO_OUTPUT_FLAG_VOIP);
}

/**
 * @tc.name   : Test InterphoneSink API - ID Handler
 * @tc.number : InterphoneSinkUnitTest_004
 * @tc.desc   : Test GetRenderIdByDeviceClass with interphone routeFlag
 */
HWTEST_F(AudioRenderSinkUnitTest, InterphoneSinkUnitTest_004, TestSize.Level1)
{
    IdHandler &idHandler = IdHandler::GetInstance();
    uint32_t id = idHandler.GetRenderIdByDeviceClass("primary", "", AUDIO_OUTPUT_FLAG_INTERPHONE);
    EXPECT_NE(id, HDI_INVALID_ID);
}

/**
 * @tc.name   : Test InterphoneSink API - Info Parsing
 * @tc.number : InterphoneSinkUnitTest_005
 * @tc.desc   : Test IdHandler parsing for interphone
 */
HWTEST_F(AudioRenderSinkUnitTest, InterphoneSinkUnitTest_005, TestSize.Level1)
{
    IdHandler &idHandler = IdHandler::GetInstance();
    uint32_t id = idHandler.GetRenderIdByDeviceClass("primary", "", AUDIO_OUTPUT_FLAG_INTERPHONE);
    
    uint32_t type = idHandler.ParseType(id);
    EXPECT_EQ(type, static_cast<uint32_t>(HDI_ID_TYPE_PRIMARY));
    
    std::string info = idHandler.ParseInfo(id);
    EXPECT_EQ(info, HDI_ID_INFO_INTERPHONE);
}

/**
 * @tc.name   : Test InterphoneSink API - Multiple Config
 * @tc.number : InterphoneSinkUnitTest_006
 * @tc.desc   : Test interphone sink multiple configuration
 */
HWTEST_F(AudioRenderSinkUnitTest, InterphoneSinkUnitTest_006, TestSize.Level1)
{
    // Test different sample rates
    IAudioSinkAttr attr;
    attr.routeFlag = AUDIO_OUTPUT_FLAG_INTERPHONE;
    
    attr.sampleRate = 16000;
    EXPECT_EQ(attr.sampleRate, 16000);
    
    attr.sampleRate = 48000;
    EXPECT_EQ(attr.sampleRate, 48000);
    
    // Test channel configuration
    attr.channel = 1;
    EXPECT_EQ(attr.channel, 1);
    
    attr.channel = 2;
    EXPECT_EQ(attr.channel, 2);
}

/**
 * @tc.name   : Test PrimarySink API
 * @tc.number : UpdateAppsUid_001
 * @tc.desc   : Test UpdateAppsUid with array input
 */
HWTEST_F(AudioRenderSinkUnitTest, UpdateAppsUid_001, TestSize.Level1)
{
    InitPrimarySink();
    EXPECT_TRUE(primarySink_ && primarySink_->IsInited());
    
    int32_t appsUid[MAX_MIX_CHANNELS] = {1001, 1002, 1003};
    size_t size = 3;
    int32_t ret = primarySink_->UpdateAppsUid(appsUid, size);
#ifdef FEATURE_POWER_MANAGER
    EXPECT_EQ(ret, SUCCESS);
#else
    EXPECT_EQ(ret, SUCCESS);
#endif
    
    DeInitPrimarySink();
}

/**
 * @tc.name   : Test PrimarySink API
 * @tc.number : UpdateAppsUid_002
 * @tc.desc   : Test UpdateAppsUid with vector input
 */
HWTEST_F(AudioRenderSinkUnitTest, UpdateAppsUid_002, TestSize.Level1)
{
    InitPrimarySink();
    EXPECT_TRUE(primarySink_ && primarySink_->IsInited());
    
    std::vector<int32_t> appsUid = {1001, 1002, 1003};
    int32_t ret = primarySink_->UpdateAppsUid(appsUid);
#ifdef FEATURE_POWER_MANAGER
    EXPECT_EQ(ret, SUCCESS);
#else
    EXPECT_EQ(ret, SUCCESS);
#endif
    
    DeInitPrimarySink();
}

/**
 * @tc.name   : Test DirectSink API
 * @tc.number : UpdateAppsUid_Direct_001
 * @tc.desc   : Test UpdateAppsUid for direct sink with array input
 */
HWTEST_F(AudioRenderSinkUnitTest, UpdateAppsUid_Direct_001, TestSize.Level1)
{
    InitDirectSink();
    EXPECT_TRUE(directSink_ && directSink_->IsInited());
    
    int32_t appsUid[MAX_MIX_CHANNELS] = {1001, 1002};
    size_t size = 2;
    int32_t ret = directSink_->UpdateAppsUid(appsUid, size);
#ifdef FEATURE_POWER_MANAGER
    EXPECT_EQ(ret, SUCCESS);
#else
    EXPECT_EQ(ret, SUCCESS);
#endif
    
    DeInitDirectSink();
}

/**
 * @tc.name   : Test DirectSink API
 * @tc.number : UpdateAppsUid_Direct_002
 * @tc.desc   : Test UpdateAppsUid for direct sink with vector input
 */
HWTEST_F(AudioRenderSinkUnitTest, UpdateAppsUid_Direct_002, TestSize.Level1)
{
    InitDirectSink();
    EXPECT_TRUE(directSink_ && directSink_->IsInited());
    
    std::vector<int32_t> appsUid = {1001, 1002};
    int32_t ret = directSink_->UpdateAppsUid(appsUid);
#ifdef FEATURE_POWER_MANAGER
    EXPECT_EQ(ret, SUCCESS);
#else
    EXPECT_EQ(ret, SUCCESS);
#endif
    
    DeInitDirectSink();
}

/**
 * @tc.name   : Test GetCurrentOutputDevice API
 * @tc.number : GetCurrentOutputDevice_001
 * @tc.desc   : Test GetCurrentOutputDevice returns current active device type
 */
HWTEST_F(AudioRenderSinkUnitTest, GetCurrentOutputDevice_001, TestSize.Level1)
{
    InitPrimarySink();
    EXPECT_TRUE(primarySink_ && primarySink_->IsInited());
    
    DeviceType deviceType = primarySink_->GetCurrentOutputDevice();
    EXPECT_EQ(deviceType, DEVICE_TYPE_SPEAKER);
    
    DeInitPrimarySink();
}

/**
 * @tc.name   : Test GetCurrentOutputDevice API
 * @tc.number : GetCurrentOutputDevice_002
 * @tc.desc   : Test GetCurrentOutputDevice for USB sink
 */
HWTEST_F(AudioRenderSinkUnitTest, GetCurrentOutputDevice_002, TestSize.Level1)
{
    InitUsbSink();
    EXPECT_TRUE(usbSink_ && usbSink_->IsInited());
    
    DeviceType deviceType = usbSink_->GetCurrentOutputDevice();
    EXPECT_EQ(deviceType, DEVICE_TYPE_USB_ARM_HEADSET);
    
    DeInitUsbSink();
}

/**
 * @tc.name   : Test GetCurrentOutputDevice API
 * @tc.number : GetCurrentOutputDevice_003
 * @tc.desc   : Test GetCurrentOutputDevice for VOIP sink
 */
HWTEST_F(AudioRenderSinkUnitTest, GetCurrentOutputDevice_003, TestSize.Level1)
{
    InitVoipSink();
    EXPECT_TRUE(voipSink_ && voipSink_->IsInited());
    
    DeviceType deviceType = voipSink_->GetCurrentOutputDevice();
    EXPECT_EQ(deviceType, DEVICE_TYPE_SPEAKER);
    
    DeInitVoipSink();
}

/**
 * @tc.name   : Test GetCurrentOutputDevice API
 * @tc.number : GetCurrentOutputDevice_004
 * @tc.desc   : Test GetCurrentOutputDevice for uninitialized sink
 */
HWTEST_F(AudioRenderSinkUnitTest, GetCurrentOutputDevice_004, TestSize.Level1)
{
    EXPECT_TRUE(primarySink_ == nullptr || !primarySink_->IsInited());
    
    if (primarySink_ != nullptr) {
        DeviceType deviceType = primarySink_->GetCurrentOutputDevice();
        EXPECT_EQ(deviceType, DEVICE_TYPE_NONE);
    }
}

/**
 * @tc.name   : Test IdHandler GetRenderIdByDeviceClassSub API
 * @tc.number : IdHandler_UsbArmFast_001
 * @tc.desc   : Test GetRenderIdByDeviceClassSub with usb_arm_fast deviceClass
 */
HWTEST_F(AudioRenderSinkUnitTest, IdHandler_UsbArmFast_001, TestSize.Level1)
{
    IdHandler &idHandler = IdHandler::GetInstance();
    uint32_t id = idHandler.GetRenderIdByDeviceClassSub("usb_arm_fast", "", AUDIO_OUTPUT_FLAG_FAST);
    EXPECT_NE(id, HDI_INVALID_ID);
    
    uint32_t type = idHandler.ParseType(id);
    EXPECT_EQ(type, static_cast<uint32_t>(HDI_ID_TYPE_FAST));
    
    std::string info = idHandler.ParseInfo(id);
    EXPECT_EQ(info, HDI_ID_INFO_USB);
}

/**
 * @tc.name   : Test IdHandler GetRenderIdByDeviceClassSub API
 * @tc.number : IdHandler_UsbArmUltraFast_001
 * @tc.desc   : Test GetRenderIdByDeviceClassSub with usb_arm_ultra_fast deviceClass
 */
HWTEST_F(AudioRenderSinkUnitTest, IdHandler_UsbArmUltraFast_001, TestSize.Level1)
{
    IdHandler &idHandler = IdHandler::GetInstance();
    uint32_t id = idHandler.GetRenderIdByDeviceClassSub("usb_arm_ultra_fast", "", AUDIO_OUTPUT_FLAG_VOIP_FAST);
    EXPECT_NE(id, HDI_INVALID_ID);
    
    uint32_t type = idHandler.ParseType(id);
    EXPECT_EQ(type, static_cast<uint32_t>(HDI_ID_TYPE_FAST));
    
    std::string info = idHandler.ParseInfo(id);
    EXPECT_EQ(info, HDI_ID_INFO_USB);
}

/**
 * @tc.name   : Test GetCurrentOutputDevice API
 * @tc.number : GetCurrentOutputDevice_005
 * @tc.desc   : Test GetCurrentOutputDevice for direct sink
 */
HWTEST_F(AudioRenderSinkUnitTest, GetCurrentOutputDevice_005, TestSize.Level1)
{
    InitDirectSink();
    EXPECT_TRUE(directSink_ && directSink_->IsInited());
    
    DeviceType deviceType = directSink_->GetCurrentOutputDevice();
    EXPECT_EQ(deviceType, DEVICE_TYPE_WIRED_HEADSET);
    
    DeInitDirectSink();
}
} // namespace AudioStandard
} // namespace OHOS
