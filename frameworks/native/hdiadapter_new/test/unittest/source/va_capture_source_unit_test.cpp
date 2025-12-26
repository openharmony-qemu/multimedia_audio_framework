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
    virtual void SetUp();
    virtual void TearDown();

protected:
    static uint32_t id_;
    static std::shared_ptr<IAudioCaptureSource> source_;
    static IAudioSourceAttr attr_;
};

uint32_t VACaptureSourceUnitTest::id_ = HDI_INVALID_ID;
std::shared_ptr<IAudioCaptureSource> VACaptureSourceUnitTest::source_ = nullptr;
IAudioSourceAttr VACaptureSourceUnitTest::attr_ = {};

void VACaptureSourceUnitTest::SetUpTestCase()
{
    id_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_VA, HDI_ID_INFO_VA, true);
}

void VACaptureSourceUnitTest::TearDownTestCase()
{
    HdiAdapterManager::GetInstance().ReleaseId(id_);
}

void VACaptureSourceUnitTest::SetUp()
{
    source_ = HdiAdapterManager::GetInstance().GetCaptureSource(id_, true);
    if (source_ == nullptr) {
        return;
    }
}

void VACaptureSourceUnitTest::TearDown()
{
    if (source_ && source_->IsInited()) {
        source_->DeInit();
    }
    source_ = nullptr;
}

/**
 * @tc.name   : Test VASource API
 * @tc.number : VACaptureSourceUnitTest_001
 * @tc.desc   : Test va source create
 */
HWTEST_F(VACaptureSourceUnitTest, VACaptureSourceUnitTest_001, TestSize.Level1)
{
    EXPECT_TRUE(source_);
}

/**
 * @tc.name   : Test BluetoothSource API
 * @tc.number : VACaptureSourceUnitTest_002
 * @tc.desc   : Test va source deinit
 */
HWTEST_F(VACaptureSourceUnitTest, VACaptureSourceUnitTest_002, TestSize.Level1)
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
HWTEST_F(VACaptureSourceUnitTest, VACaptureSourceUnitTest_003, TestSize.Level1)
{
    EXPECT_TRUE(source_);
    int32_t ret = source_->Start();
    EXPECT_NE(ret, SUCCESS);
    ret = source_->Stop();
    EXPECT_NE(ret, SUCCESS);
    ret = source_->Resume();
    EXPECT_EQ(ret, SUCCESS);
    ret = source_->Pause();
    EXPECT_EQ(ret, SUCCESS);
    ret = source_->Flush();
    EXPECT_EQ(ret, SUCCESS);
    ret = source_->Reset();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test DirectSink API
 * @tc.number : BluetoothSourceUnitTest_004
 * @tc.desc   : Test bluetooth source set/get volume
 */
HWTEST_F(VACaptureSourceUnitTest, VACaptureSourceUnitTest_004, TestSize.Level1)
{
    EXPECT_TRUE(source_);
    int32_t ret = source_->SetVolume(0.0f, 0.0f);
    EXPECT_EQ(ret, SUCCESS);
    float left;
    float right;
    ret = source_->GetVolume(left, right);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test BluetoothSource API
 * @tc.number : BluetoothSourceUnitTest_005
 * @tc.desc   : Test bluetooth source set/get mute
 */
HWTEST_F(VACaptureSourceUnitTest, VACaptureSourceUnitTest_005, TestSize.Level1)
{
    EXPECT_TRUE(source_);
    int32_t ret = source_->SetMute(false);
    EXPECT_EQ(ret, SUCCESS);
    bool isMute;
    ret = source_->GetMute(isMute);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test BluetoothSource API
 * @tc.number : BluetoothSourceUnitTest_006
 * @tc.desc   : Test bluetooth source set audio scene
 */
HWTEST_F(VACaptureSourceUnitTest, VACaptureSourceUnitTest_006, TestSize.Level1)
{
    EXPECT_TRUE(source_);
    int32_t ret = source_->SetAudioScene(AUDIO_SCENE_DEFAULT);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test BluetoothSource API
 * @tc.number : BluetoothSourceUnitTest_007
 * @tc.desc   : Test bluetooth source update active device
 */
HWTEST_F(VACaptureSourceUnitTest, VACaptureSourceUnitTest_007, TestSize.Level1)
{
    EXPECT_TRUE(source_);
    int32_t ret = source_->UpdateActiveDevice(DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test BluetoothSource API
 * @tc.number : BluetoothSourceUnitTest_008
 * @tc.desc   : Test bluetooth source set invalid state
 */
HWTEST_F(VACaptureSourceUnitTest, VACaptureSourceUnitTest_008, TestSize.Level1)
{
    EXPECT_TRUE(source_);
    source_->SetInvalidState();
    (void)source_->Init(attr_);
    source_->DeInit();
    EXPECT_FALSE(source_->IsInited());
}

} // namespace AudioStandard
} // namespace OHOS