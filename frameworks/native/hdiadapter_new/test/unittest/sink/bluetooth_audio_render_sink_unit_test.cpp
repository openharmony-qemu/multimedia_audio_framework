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
class BluetoothAudioRenderSinkUnitTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp();
    virtual void TearDown();

protected:
    static uint32_t id_;
    static std::shared_ptr<IAudioRenderSink> sink_;
    static IAudioSinkAttr attr_;
    static uint32_t hearingAidId_;
    static std::shared_ptr<IAudioRenderSink> hearingAidSink_;
};

uint32_t BluetoothAudioRenderSinkUnitTest::id_ = HDI_INVALID_ID;
std::shared_ptr<IAudioRenderSink> BluetoothAudioRenderSinkUnitTest::sink_ = nullptr;
IAudioSinkAttr BluetoothAudioRenderSinkUnitTest::attr_ = {};
uint32_t BluetoothAudioRenderSinkUnitTest::hearingAidId_ = HDI_INVALID_ID;
std::shared_ptr<IAudioRenderSink> BluetoothAudioRenderSinkUnitTest::hearingAidSink_ = nullptr;

void BluetoothAudioRenderSinkUnitTest::SetUpTestCase()
{
    id_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_BLUETOOTH, HDI_ID_INFO_DEFAULT, true);
    hearingAidId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER,
        HDI_ID_TYPE_BLUETOOTH, HDI_ID_INFO_HEARING_AID, true);
}

void BluetoothAudioRenderSinkUnitTest::TearDownTestCase()
{
    HdiAdapterManager::GetInstance().ReleaseId(id_);
    HdiAdapterManager::GetInstance().ReleaseId(hearingAidId_);
}

void BluetoothAudioRenderSinkUnitTest::SetUp()
{
    sink_ = HdiAdapterManager::GetInstance().GetRenderSink(id_, true);
    if (sink_ == nullptr) {
        return;
    }
    attr_.adapterName = "bt_a2dp";
    attr_.channel = 2; // 2: channel
    sink_->Init(attr_);
}

void BluetoothAudioRenderSinkUnitTest::TearDown()
{
    if (sink_ && sink_->IsInited()) {
        sink_->DeInit();
    }
    sink_ = nullptr;
}

/**
 * @tc.name   : Test BluetoothSink API
 * @tc.number : BluetoothSinkUnitTest_001
 * @tc.desc   : Test bluetooth sink create
 */
HWTEST_F(BluetoothAudioRenderSinkUnitTest, BluetoothSinkUnitTest_001, TestSize.Level1)
{
    EXPECT_TRUE(sink_);
}

/**
 * @tc.name   : Test BluetoothSink API
 * @tc.number : BluetoothSinkUnitTest_002
 * @tc.desc   : Test bluetooth sink init/deinit
 */
HWTEST_F(BluetoothAudioRenderSinkUnitTest, BluetoothSinkUnitTest_002, TestSize.Level1)
{
    EXPECT_TRUE(sink_);
    sink_->DeInit();
    (void)sink_->Init(attr_);
    (void)sink_->Init(attr_);
    sink_->DeInit();
    EXPECT_FALSE(sink_->IsInited());
}

/**
 * @tc.name   : Test BluetoothSink API
 * @tc.number : BluetoothSinkUnitTest_003
 * @tc.desc   : Test bluetooth sink start, stop, resume, pause, flush, reset
 */
HWTEST_F(BluetoothAudioRenderSinkUnitTest, BluetoothSinkUnitTest_003, TestSize.Level1)
{
    EXPECT_TRUE(sink_);
    int32_t ret = sink_->Start();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = sink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    ret = sink_->Resume();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = sink_->Pause();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = sink_->Flush();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = sink_->Reset();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = sink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test DirectSink API
 * @tc.number : BluetoothSinkUnitTest_004
 * @tc.desc   : Test bluetooth sink set/get volume
 */
HWTEST_F(BluetoothAudioRenderSinkUnitTest, BluetoothSinkUnitTest_004, TestSize.Level1)
{
    EXPECT_TRUE(sink_);
    int32_t ret = sink_->SetVolume(0.0f, 0.0f);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = sink_->SetVolume(0.0f, 1.0f);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = sink_->SetVolume(1.0f, 0.0f);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = sink_->SetVolume(1.0f, 1.0f);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    float left;
    float right;
    ret = sink_->GetVolume(left, right);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test BluetoothSink API
 * @tc.number : BluetoothSinkUnitTest_005
 * @tc.desc   : Test bluetooth sink set audio scene
 */
HWTEST_F(BluetoothAudioRenderSinkUnitTest, BluetoothSinkUnitTest_005, TestSize.Level1)
{
    EXPECT_TRUE(sink_);
    int32_t ret = sink_->SetAudioScene(AUDIO_SCENE_DEFAULT);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name   : Test BluetoothSink API
 * @tc.number : BluetoothSinkUnitTest_006
 * @tc.desc   : Test bluetooth sink update active device
 */
HWTEST_F(BluetoothAudioRenderSinkUnitTest, BluetoothSinkUnitTest_006, TestSize.Level1)
{
    EXPECT_TRUE(sink_);
    std::vector<DeviceType> deviceTypes = { DEVICE_TYPE_SPEAKER };
    int32_t ret = sink_->UpdateActiveDevice(deviceTypes);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name   : Test BluetoothSink API
 * @tc.number : BluetoothSinkUnitTest_007
 * @tc.desc   : Test bluetooth sink set invalid state
 */
HWTEST_F(BluetoothAudioRenderSinkUnitTest, BluetoothSinkUnitTest_007, TestSize.Level1)
{
    EXPECT_TRUE(sink_);
    sink_->SetInvalidState();
    (void)sink_->Init(attr_);
    sink_->DeInit();
    EXPECT_FALSE(sink_->IsInited());
}

/**
 * @tc.name   : Test BluetoothSink API
 * @tc.number : BluetoothSinkUnitTest_008
 * @tc.desc   : Test bluetooth sink set invalid state
 */
HWTEST_F(BluetoothAudioRenderSinkUnitTest, BluetoothSinkUnitTest_008, TestSize.Level1)
{
    hearingAidSink_ = HdiAdapterManager::GetInstance().GetRenderSink(hearingAidId_, true);
    EXPECT_TRUE(hearingAidSink_);
    
    attr_.adapterName = "hearing_aid";
    attr_.channel = 2;
    attr_.sampleRate = 16000;
    attr_.format = SAMPLE_S16LE;
    attr_.deviceType = DEVICE_TYPE_HEARING_AID;
    hearingAidSink_->Init(attr_);
    
    hearingAidSink_->SetInvalidState();
    (void)hearingAidSink_->Init(attr_);
    hearingAidSink_->DeInit();
    EXPECT_FALSE(hearingAidSink_->IsInited());
    hearingAidSink_ = nullptr;
}

/**
 * @tc.name   : Test BluetoothSink API
 * @tc.number : BluetoothSinkUnitTest_009
 * @tc.desc   : Test bluetooth sink IsSinkInited
 */
HWTEST_F(BluetoothAudioRenderSinkUnitTest, BluetoothSinkUnitTest_009, TestSize.Level1)
{
    EXPECT_TRUE(sink_);
    EXPECT_FALSE(sink_->IsSinkInited());
    sink_->DeInit();
    EXPECT_FALSE(sink_->IsInited());
    EXPECT_FALSE(sink_->IsSinkInited());
}

} // namespace AudioStandard
} // namespace OHOS
