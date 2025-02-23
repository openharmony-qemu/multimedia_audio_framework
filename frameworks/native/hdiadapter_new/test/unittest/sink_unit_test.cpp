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
class SinkUnitTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() {}
    virtual void TearDown() {}

    void TestAction(uint32_t id);
    void TestSetAndGet(uint32_t id);
    void TestSetAndGetFirst(uint32_t id);
    void TestSetAndGetSecond(uint32_t id);

protected:
    static uint32_t fastRenderId_;
    static uint32_t usbRenderId_;
    static uint32_t btRenderId_;
    static uint32_t btMmapRenderId_;
    static uint32_t remoteRenderId_;
};

uint32_t SinkUnitTest::fastRenderId_ = HDI_INVALID_ID;
uint32_t SinkUnitTest::usbRenderId_ = HDI_INVALID_ID;
uint32_t SinkUnitTest::btRenderId_ = HDI_INVALID_ID;
uint32_t SinkUnitTest::btMmapRenderId_ = HDI_INVALID_ID;
uint32_t SinkUnitTest::remoteRenderId_ = HDI_INVALID_ID;

void SinkUnitTest::SetUpTestCase()
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    fastRenderId_ = manager.GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_FAST, HDI_ID_INFO_DEFAULT, true);
    usbRenderId_ = manager.GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_PRIMARY, HDI_ID_INFO_USB, true);
    btRenderId_ = manager.GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_BLUETOOTH, HDI_ID_INFO_DEFAULT, true);
    btMmapRenderId_ = manager.GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_BLUETOOTH, HDI_ID_INFO_MMAP, true);
    remoteRenderId_ = manager.GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_REMOTE, "test", true);
}

void SinkUnitTest::TearDownTestCase()
{
    for (auto id : { fastRenderId_, usbRenderId_, btRenderId_, btMmapRenderId_, remoteRenderId_ }) {
        if (id != HDI_INVALID_ID) {
            HdiAdapterManager::GetInstance().ReleaseId(id);
        }
    }
}

void SinkUnitTest::TestAction(uint32_t id)
{
    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(id, true);
    ASSERT_NE(sink, nullptr);
    EXPECT_EQ(sink->IsInited(), false);

    auto ret = sink->SuspendRenderSink();
    EXPECT_EQ(ret, SUCCESS);

    ret = sink->RestoreRenderSink();
    EXPECT_EQ(ret, SUCCESS);

    sink->ResetActiveDeviceForDisconnect(DEVICE_TYPE_SPEAKER);

    ret = sink->Start();
    if (id == remoteRenderId_) {
        EXPECT_EQ(ret, SUCCESS);
    } else {
        EXPECT_NE(ret, SUCCESS);
    }

    ret = sink->Stop();
    EXPECT_EQ(ret, SUCCESS);

    ret = sink->Resume();
    EXPECT_NE(ret, SUCCESS);

    ret = sink->Pause();
    EXPECT_NE(ret, SUCCESS);

    ret = sink->Flush();
    EXPECT_NE(ret, SUCCESS);

    ret = sink->Reset();
    EXPECT_NE(ret, SUCCESS);
}

void SinkUnitTest::TestSetAndGet(uint32_t id)
{
    TestSetAndGetFirst(id);
    TestSetAndGetSecond(id);
}

void SinkUnitTest::TestSetAndGetFirst(uint32_t id)
{
    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(id, true);
    ASSERT_NE(sink, nullptr);
    EXPECT_EQ(sink->IsInited(), false);

    std::string condition = "";
    std::string value = "";
    sink->SetAudioParameter(AudioParamKey::VOLUME, condition, value);

    float left = 0;
    float right = 0;
    auto ret = sink->SetVolume(left, right);
    if (id == remoteRenderId_) {
        EXPECT_EQ(ret, SUCCESS);
    } else {
        EXPECT_NE(ret, SUCCESS);
    }

    uint32_t latency = 0;
    ret = sink->GetLatency(latency);
    if (id == remoteRenderId_) {
        EXPECT_EQ(ret, SUCCESS);
    } else {
        EXPECT_NE(ret, SUCCESS);
    }

    uint64_t transactionId = 0;
    ret = sink->GetTransactionId(transactionId);
    if (id == usbRenderId_ || id == btRenderId_ || id == btMmapRenderId_) {
        EXPECT_NE(ret, ERR_NOT_SUPPORTED);
    } else {
        EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
    }

    float amplitude = sink->GetMaxAmplitude();
    EXPECT_EQ(amplitude, 0.0);

    sink->SetAudioMonoState(true);
    sink->SetAudioBalanceValue(1);

    ret = sink->SetSinkMuteForSwitchDevice(false);
    EXPECT_EQ(ret, SUCCESS);

    std::vector<DeviceType> activeDevices = { DEVICE_TYPE_SPEAKER, DEVICE_TYPE_EARPIECE };
    ret = sink->SetAudioScene(AUDIO_SCENE_DEFAULT, activeDevices);
    if (id == fastRenderId_ || id == usbRenderId_ || id == remoteRenderId_) {
        EXPECT_EQ(ret, SUCCESS);
    } else {
        EXPECT_NE(ret, SUCCESS);
    }
}

void SinkUnitTest::TestSetAndGetSecond(uint32_t id)
{
    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(id, true);
    ASSERT_NE(sink, nullptr);
    EXPECT_EQ(sink->IsInited(), false);

    auto ret = sink->GetAudioScene();
    EXPECT_NE(ret, SUCCESS);

    std::vector<DeviceType> activeDevices = { DEVICE_TYPE_SPEAKER, DEVICE_TYPE_EARPIECE };
    ret = sink->UpdateActiveDevice(activeDevices);
    EXPECT_NE(ret, SUCCESS);

    ret = sink->SetPaPower(1);
    EXPECT_NE(ret, SUCCESS);

    ret = sink->SetPriPaPower();
    EXPECT_NE(ret, SUCCESS);

    ret = sink->SetRenderEmpty(20000); // 20000: test
    EXPECT_EQ(ret, SUCCESS);

    std::string address = "";
    sink->SetAddress(address);
}

/**
 * @tc.name   : Test Sink API
 * @tc.number : SinkUnitTest_001
 * @tc.desc   : Test FastAudioRenderSink action
 */
HWTEST_F(SinkUnitTest, SinkUnitTest_001, TestSize.Level1)
{
    TestAction(fastRenderId_);
}

/**
 * @tc.name   : Test Sink API
 * @tc.number : SinkUnitTest_002
 * @tc.desc   : Test FastAudioRenderSink set/get operation
 */
HWTEST_F(SinkUnitTest, SinkUnitTest_002, TestSize.Level1)
{
    TestSetAndGet(fastRenderId_);
}

/**
 * @tc.name   : Test Sink API
 * @tc.number : SinkUnitTest_003
 * @tc.desc   : Test AudioRenderSink_usb action
 */
HWTEST_F(SinkUnitTest, SinkUnitTest_003, TestSize.Level1)
{
    TestAction(usbRenderId_);
}

/**
 * @tc.name   : Test Sink API
 * @tc.number : SinkUnitTest_004
 * @tc.desc   : Test AudioRenderSink_usb set/get operation
 */
HWTEST_F(SinkUnitTest, SinkUnitTest_004, TestSize.Level1)
{
    TestSetAndGet(usbRenderId_);
}

/**
 * @tc.name   : Test Sink API
 * @tc.number : SinkUnitTest_005
 * @tc.desc   : Test BluetoothAudioRenderSink action
 */
HWTEST_F(SinkUnitTest, SinkUnitTest_005, TestSize.Level1)
{
    TestAction(btRenderId_);
}

/**
 * @tc.name   : Test Sink API
 * @tc.number : SinkUnitTest_006
 * @tc.desc   : Test BluetoothAudioRenderSink set/get operation
 */
HWTEST_F(SinkUnitTest, SinkUnitTest_006, TestSize.Level1)
{
    TestSetAndGet(btRenderId_);
}

/**
 * @tc.name   : Test Sink API
 * @tc.number : SinkUnitTest_007
 * @tc.desc   : Test BluetoothAudioRenderSink_mmap action
 */
HWTEST_F(SinkUnitTest, SinkUnitTest_007, TestSize.Level1)
{
    TestAction(btMmapRenderId_);
}

/**
 * @tc.name   : Test Sink API
 * @tc.number : SinkUnitTest_008
 * @tc.desc   : Test BluetoothAudioRenderSink_mmap set/get operation
 */
HWTEST_F(SinkUnitTest, SinkUnitTest_008, TestSize.Level1)
{
    TestSetAndGet(btMmapRenderId_);
}

/**
 * @tc.name   : Test Sink API
 * @tc.number : SinkUnitTest_009
 * @tc.desc   : Test RemoteAudioRenderSink action
 */
HWTEST_F(SinkUnitTest, SinkUnitTest_009, TestSize.Level1)
{
    TestAction(remoteRenderId_);
}

/**
 * @tc.name   : Test Sink API
 * @tc.number : SinkUnitTest_010
 * @tc.desc   : Test RemoteAudioRenderSink set/get operation
 */
HWTEST_F(SinkUnitTest, SinkUnitTest_010, TestSize.Level1)
{
    TestSetAndGet(remoteRenderId_);
}

} // namespace AudioStandard
} // namespace OHOS
