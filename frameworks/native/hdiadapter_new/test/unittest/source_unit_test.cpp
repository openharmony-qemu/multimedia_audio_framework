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
class SourceUnitTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() {}
    virtual void TearDown() {}

    void TestAction(uint32_t id);
    void TestSetAndGet(uint32_t id);

protected:
    static uint32_t fastCaptureId_;
    static uint32_t usbCaptureId_;
    static uint32_t btCaptureId_;
    static uint32_t remoteCaptureId_;
};

uint32_t SourceUnitTest::fastCaptureId_ = HDI_INVALID_ID;
uint32_t SourceUnitTest::usbCaptureId_ = HDI_INVALID_ID;
uint32_t SourceUnitTest::btCaptureId_ = HDI_INVALID_ID;
uint32_t SourceUnitTest::remoteCaptureId_ = HDI_INVALID_ID;

void SourceUnitTest::SetUpTestCase()
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    fastCaptureId_ = manager.GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_FAST, HDI_ID_INFO_DEFAULT, true);
    usbCaptureId_ = manager.GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_USB, true);
    btCaptureId_ = manager.GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_BLUETOOTH, HDI_ID_INFO_DEFAULT, true);
    remoteCaptureId_ = manager.GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_REMOTE, "test", true);
}

void SourceUnitTest::TearDownTestCase()
{
    for (auto id : { fastCaptureId_, usbCaptureId_, btCaptureId_, remoteCaptureId_ }) {
        if (id != HDI_INVALID_ID) {
            HdiAdapterManager::GetInstance().ReleaseId(id);
        }
    }
}

void SourceUnitTest::TestAction(uint32_t id)
{
    std::shared_ptr<IAudioCaptureSource> source = HdiAdapterManager::GetInstance().GetCaptureSource(id, true);
    ASSERT_NE(source, nullptr);
    EXPECT_EQ(source->IsInited(), false);

    int32_t ret = SUCCESS;
    if (id != remoteCaptureId_) {
        ret = source->Start();
        EXPECT_NE(ret, SUCCESS);
    }

    ret = source->Stop();
    EXPECT_EQ(ret, SUCCESS);

    ret = source->Resume();
    EXPECT_NE(ret, SUCCESS);

    ret = source->Pause();
    EXPECT_NE(ret, SUCCESS);

    ret = source->Flush();
    EXPECT_NE(ret, SUCCESS);

    ret = source->Reset();
    EXPECT_NE(ret, SUCCESS);
}

void SourceUnitTest::TestSetAndGet(uint32_t id)
{
    std::shared_ptr<IAudioCaptureSource> source = HdiAdapterManager::GetInstance().GetCaptureSource(id, true);
    ASSERT_NE(source, nullptr);
    EXPECT_EQ(source->IsInited(), false);

    float left = 0;
    float right = 0;
    auto ret = source->SetVolume(left, right);
    EXPECT_NE(ret, SUCCESS);

    ret = source->GetVolume(left, right);
    EXPECT_NE(ret, SUCCESS);

    ret = source->SetMute(true);
    if (id == fastCaptureId_ || id == btCaptureId_) {
        EXPECT_NE(ret, SUCCESS);
    } else {
        EXPECT_EQ(ret, SUCCESS);
    }

    bool isMute = false;
    ret = source->GetMute(isMute);
    EXPECT_NE(ret, SUCCESS);

    uint64_t transactionId = 0;
    transactionId = source->GetTransactionId();

    float amplitude = source->GetMaxAmplitude();
    EXPECT_EQ(amplitude, 0.0);

    DeviceType activeDevice = DEVICE_TYPE_MIC;
    ret = source->SetAudioScene(AUDIO_SCENE_DEFAULT, activeDevice);
    if (id == usbCaptureId_) {
        EXPECT_EQ(ret, SUCCESS);
    } else {
        EXPECT_NE(ret, SUCCESS);
    }

    ret = source->UpdateActiveDevice(activeDevice);
    EXPECT_NE(ret, SUCCESS);

    std::string address = "";
    source->SetAddress(address);
}

/**
 * @tc.name   : Test Source API
 * @tc.number : SourceUnitTest_001
 * @tc.desc   : Test FastAudioCaptureSource action
 */
HWTEST_F(SourceUnitTest, SourceUnitTest_001, TestSize.Level1)
{
    TestAction(fastCaptureId_);
}

/**
 * @tc.name   : Test Source API
 * @tc.number : SourceUnitTest_002
 * @tc.desc   : Test FastAudioCaptureSource set/get operation
 */
HWTEST_F(SourceUnitTest, SourceUnitTest_002, TestSize.Level1)
{
    TestSetAndGet(fastCaptureId_);
}

/**
 * @tc.name   : Test Source API
 * @tc.number : SourceUnitTest_003
 * @tc.desc   : Test AudioCaptureSource_usb action
 */
HWTEST_F(SourceUnitTest, SourceUnitTest_003, TestSize.Level1)
{
    TestAction(usbCaptureId_);
}

/**
 * @tc.name   : Test Source API
 * @tc.number : SourceUnitTest_004
 * @tc.desc   : Test AudioCaptureSource_usb set/get operation
 */
HWTEST_F(SourceUnitTest, SourceUnitTest_004, TestSize.Level1)
{
    TestSetAndGet(usbCaptureId_);
}

/**
 * @tc.name   : Test Source API
 * @tc.number : SourceUnitTest_005
 * @tc.desc   : Test BluetoothAudioCaptureSource action
 */
HWTEST_F(SourceUnitTest, SourceUnitTest_005, TestSize.Level1)
{
    TestAction(btCaptureId_);
}

/**
 * @tc.name   : Test Source API
 * @tc.number : SourceUnitTest_006
 * @tc.desc   : Test BluetoothAudioCaptureSource set/get operation
 */
HWTEST_F(SourceUnitTest, SourceUnitTest_006, TestSize.Level1)
{
    TestSetAndGet(btCaptureId_);
}

/**
 * @tc.name   : Test Source API
 * @tc.number : SourceUnitTest_007
 * @tc.desc   : Test RemoteAudioCaptureSource action
 */
HWTEST_F(SourceUnitTest, SourceUnitTest_007, TestSize.Level1)
{
    TestAction(remoteCaptureId_);
}

/**
 * @tc.name   : Test Source API
 * @tc.number : SourceUnitTest_008
 * @tc.desc   : Test RemoteAudioCaptureSource set/get operation
 */
HWTEST_F(SourceUnitTest, SourceUnitTest_008, TestSize.Level1)
{
    TestSetAndGet(remoteCaptureId_);
}

} // namespace AudioStandard
} // namespace OHOS
