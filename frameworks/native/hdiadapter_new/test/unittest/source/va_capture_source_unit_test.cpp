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
class VACaptureSourceUnitTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() {}
    virtual void TearDown() {}

    void InitPrimarySource();
    void DeInitPrimarySource();
    void InitUsbSource();
    void DeInitUsbSource();
protected:
    static uint32_t primaryId_;
    static uint32_t usbId_;
    static std::shared_ptr<IAudioCaptureSource> primarySource_;
    static std::shared_ptr<IAudioCaptureSource> usbSource_;
    static IAudioSourceAttr attr_;
};

uint32_t VACaptureSourceUnitTest::primaryId_ = 0;
uint32_t VACaptureSourceUnitTest::usbId_ = 0;
std::shared_ptr<IAudioCaptureSource> VACaptureSourceUnitTest::primarySource_ = nullptr;
std::shared_ptr<IAudioCaptureSource> VACaptureSourceUnitTest::usbSource_ = nullptr;
IAudioSourceAttr VACaptureSourceUnitTest::attr_ = {};

void VACaptureSourceUnitTest::SetUpTestCase()
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    primaryId_ = manager.GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_DEFAULT, true);
    usbId_ = manager.GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_USB, true);
}

void VACaptureSourceUnitTest::TearDownTestCase()
{
    HdiAdapterManager::GetInstance().ReleaseId(primaryId_);
    HdiAdapterManager::GetInstance().ReleaseId(usbId_);
}

void VACaptureSourceUnitTest::InitPrimarySource()
{
    primarySource_ = HdiAdapterManager::GetInstance().GetCaptureSource(primaryId_, true);
    if (primarySource_ == nullptr) {
        return;
    }
    const uint32_t kTestRate = 48000;
    const uint32_t kTestChannel = 2;
    const uint64_t kTestChannelLayout = 3;
    attr_.adapterName  = "primary";
    attr_.sampleRate = kTestRate;
    attr_.channel = kTestChannel;
    attr_.format = SAMPLE_S16LE;
    attr_.channelLayout = kTestChannelLayout;
    attr_.deviceType = DEVICE_TYPE_MIC;
    attr_.openMicSpeaker = 1;
    primarySource_->Init(attr_);
}

void VACaptureSourceUnitTest::DeInitPrimarySource()
{
    if (primarySource_ && primarySource_->IsInited()) {
        primarySource_->DeInit();
    }
    primarySource_ = nullptr;
}

void VACaptureSourceUnitTest::InitUsbSource()
{
    usbSource_ = HdiAdapterManager::GetInstance().GetCaptureSource(usbId_, true);
    if (usbSource_ == nullptr) {
        return;
    }
    const uint32_t kTestChannel = 2;
    attr_.adapterName = "usb";
    attr_.channel = kTestChannel;
    usbSource_->Init(attr_);
}

void VACaptureSourceUnitTest::DeInitUsbSource()
{
    if (usbSource_ && usbSource_->IsInited()) {
        usbSource_->DeInit();
    }
    usbSource_ = nullptr;
}

/**
 * @tc.name   : Test PrimarySource API
 * @tc.number : PrimarySourceUnitTest_001
 * @tc.desc   : Test primary source create
 */
HWTEST_F(VACaptureSourceUnitTest, PrimarySourceUnitTest_001, TestSize.Level1)
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
HWTEST_F(VACaptureSourceUnitTest, PrimarySourceUnitTest_002, TestSize.Level1)
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
HWTEST_F(VACaptureSourceUnitTest, PrimarySourceUnitTest_003, TestSize.Level1)
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
HWTEST_F(VACaptureSourceUnitTest, PrimarySourceUnitTest_004, TestSize.Level1)
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
* @tc.name   : Test SetDmDeviceType API
* @tc.number : PrimarySourceUnitTest_0015
* @tc.desc   : Test SetDmDeviceType
*/
HWTEST_F(VACaptureSourceUnitTest, PrimarySourceUnitTest_0015, TestSize.Level1)
{
    InitPrimarySource();
    EXPECT_TRUE(primarySource_ && primarySource_->IsInited());
    int32_t ret = primarySource_->Start();
    EXPECT_EQ(ret, SUCCESS);
    
    int32_t ret2 = primarySource_->Start();
    EXPECT_EQ(ret2, SUCCESS);

    int32_t ret3 = primarySource_->Stop();
    EXPECT_EQ(ret3, SUCCESS);
    DeInitPrimarySource();
}

/**
 * @tc.name   : Test PrimarySource API
 * @tc.number : PrimarySourceUnitTest_005
 * @tc.desc   : Test primary source get param
 */
HWTEST_F(VACaptureSourceUnitTest, PrimarySourceUnitTest_005, TestSize.Level1)
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
HWTEST_F(VACaptureSourceUnitTest, PrimarySourceUnitTest_006, TestSize.Level1)
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
HWTEST_F(VACaptureSourceUnitTest, PrimarySourceUnitTest_007, TestSize.Level1)
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
HWTEST_F(VACaptureSourceUnitTest, PrimarySourceUnitTest_008, TestSize.Level1)
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
HWTEST_F(VACaptureSourceUnitTest, PrimarySourceUnitTest_009, TestSize.Level1)
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
HWTEST_F(VACaptureSourceUnitTest, PrimarySourceUnitTest_010, TestSize.Level1)
{
    InitPrimarySource();
    EXPECT_TRUE(primarySource_ && primarySource_->IsInited());
    int32_t ret = primarySource_->SetAudioScene(AUDIO_SCENE_DEFAULT);
    EXPECT_EQ(ret, SUCCESS);
    DeInitPrimarySource();
}

/**
 * @tc.name   : Test PrimarySource API
 * @tc.number : PrimarySourceUnitTest_012
 * @tc.desc   : Test primary source update apps uid
 */
HWTEST_F(VACaptureSourceUnitTest, PrimarySourceUnitTest_012, TestSize.Level1)
{
    InitPrimarySource();
    EXPECT_TRUE(primarySource_ && primarySource_->IsInited());
    vector<int32_t> appsUid;
    int32_t ret = primarySource_->UpdateAppsUid(appsUid);
    EXPECT_EQ(ret, SUCCESS);
    DeInitPrimarySource();
}

/**
 * @tc.name   : Test SetDmDeviceType API
 * @tc.number : SetDmDeviceType_001
 * @tc.desc   : Test SetDmDeviceType
 */
HWTEST_F(VACaptureSourceUnitTest, SetDmDeviceType_001, TestSize.Level1)
{
    InitPrimarySource();
    EXPECT_TRUE(primarySource_ && primarySource_->IsInited());
    primarySource_->UpdateActiveDevice(DEVICE_TYPE_MIC);
    primarySource_->SetDmDeviceType(DM_DEVICE_TYPE_DEFAULT, DEVICE_TYPE_MIC);
    primarySource_->SetDmDeviceType(DM_DEVICE_TYPE_NEARLINK_SCO, DEVICE_TYPE_MIC);
    DeInitPrimarySource();
}

} // namespace AudioStandard
} // namespace OHOS