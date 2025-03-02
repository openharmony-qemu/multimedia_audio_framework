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
class BluetoothAudioCaptureSourceUnitTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp();
    virtual void TearDown();

protected:
    static uint32_t id_;
    static std::shared_ptr<IAudioCaptureSource> source_;
    static IAudioSourceAttr attr_;
};

uint32_t BluetoothAudioCaptureSourceUnitTest::id_ = HDI_INVALID_ID;
std::shared_ptr<IAudioCaptureSource> BluetoothAudioCaptureSourceUnitTest::source_ = nullptr;
IAudioSourceAttr BluetoothAudioCaptureSourceUnitTest::attr_ = {};

void BluetoothAudioCaptureSourceUnitTest::SetUpTestCase()
{
    id_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_BLUETOOTH, HDI_ID_INFO_DEFAULT, true);
}

void BluetoothAudioCaptureSourceUnitTest::TearDownTestCase()
{
    HdiAdapterManager::GetInstance().ReleaseId(id_);
}

void BluetoothAudioCaptureSourceUnitTest::SetUp()
{
    source_ = HdiAdapterManager::GetInstance().GetCaptureSource(id_, true);
    if (source_ == nullptr) {
        return;
    }
}

void BluetoothAudioCaptureSourceUnitTest::TearDown()
{
    if (source_ && source_->IsInited()) {
        source_->DeInit();
    }
    source_ = nullptr;
}

/**
 * @tc.name   : Test BluetoothSource API
 * @tc.number : BluetoothSourceUnitTest_001
 * @tc.desc   : Test bluetooth source create
 */
HWTEST_F(BluetoothAudioCaptureSourceUnitTest, BluetoothSourceUnitTest_001, TestSize.Level1)
{
    EXPECT_TRUE(source_);
}

/**
 * @tc.name   : Test BluetoothSource API
 * @tc.number : BluetoothSourceUnitTest_002
 * @tc.desc   : Test bluetooth source deinit
 */
HWTEST_F(BluetoothAudioCaptureSourceUnitTest, BluetoothSourceUnitTest_002, TestSize.Level1)
{
    EXPECT_TRUE(source_);
    if (source_->IsInited()) {
        source_->DeInit();
    }
    EXPECT_FALSE(source_->IsInited());
}

/**
 * @tc.name   : Test BluetoothSource API
 * @tc.number : BluetoothSourceUnitTest_003
 * @tc.desc   : Test bluetooth source start, stop, resume, pause, flush, reset
 */
HWTEST_F(BluetoothAudioCaptureSourceUnitTest, BluetoothSourceUnitTest_003, TestSize.Level1)
{
    EXPECT_TRUE(source_);
    int32_t ret = source_->Start();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = source_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    ret = source_->Resume();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = source_->Pause();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = source_->Flush();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = source_->Reset();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = source_->Stop();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test DirectSink API
 * @tc.number : BluetoothSourceUnitTest_004
 * @tc.desc   : Test bluetooth source set/get volume
 */
HWTEST_F(BluetoothAudioCaptureSourceUnitTest, BluetoothSourceUnitTest_004, TestSize.Level1)
{
    EXPECT_TRUE(source_);
    int32_t ret = source_->SetVolume(0.0f, 0.0f);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = source_->SetVolume(0.0f, 1.0f);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = source_->SetVolume(1.0f, 0.0f);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = source_->SetVolume(1.0f, 1.0f);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    float left;
    float right;
    ret = source_->GetVolume(left, right);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
}

/**
 * @tc.name   : Test BluetoothSource API
 * @tc.number : BluetoothSourceUnitTest_005
 * @tc.desc   : Test bluetooth source set/get mute
 */
HWTEST_F(BluetoothAudioCaptureSourceUnitTest, BluetoothSourceUnitTest_005, TestSize.Level1)
{
    EXPECT_TRUE(source_);
    int32_t ret = source_->SetMute(false);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    bool isMute;
    ret = source_->GetMute(isMute);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
}

/**
 * @tc.name   : Test BluetoothSource API
 * @tc.number : BluetoothSourceUnitTest_006
 * @tc.desc   : Test bluetooth source set audio scene
 */
HWTEST_F(BluetoothAudioCaptureSourceUnitTest, BluetoothSourceUnitTest_006, TestSize.Level1)
{
    EXPECT_TRUE(source_);
    int32_t ret = source_->SetAudioScene(AUDIO_SCENE_DEFAULT, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name   : Test BluetoothSource API
 * @tc.number : BluetoothSourceUnitTest_007
 * @tc.desc   : Test bluetooth source update active device
 */
HWTEST_F(BluetoothAudioCaptureSourceUnitTest, BluetoothSourceUnitTest_007, TestSize.Level1)
{
    EXPECT_TRUE(source_);
    int32_t ret = source_->UpdateActiveDevice(DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

} // namespace AudioStandard
} // namespace OHOS
