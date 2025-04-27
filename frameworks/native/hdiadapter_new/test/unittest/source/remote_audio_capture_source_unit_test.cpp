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
class RemoteAudioCaptureSourceUnitTest : public testing::Test {
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

uint32_t RemoteAudioCaptureSourceUnitTest::id_ = HDI_INVALID_ID;
std::shared_ptr<IAudioCaptureSource> RemoteAudioCaptureSourceUnitTest::source_ = nullptr;
IAudioSourceAttr RemoteAudioCaptureSourceUnitTest::attr_ = {};

void RemoteAudioCaptureSourceUnitTest::SetUpTestCase()
{
    id_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_REMOTE, "test", true);
}

void RemoteAudioCaptureSourceUnitTest::TearDownTestCase()
{
    HdiAdapterManager::GetInstance().ReleaseId(id_);
}

void RemoteAudioCaptureSourceUnitTest::SetUp()
{
    source_ = HdiAdapterManager::GetInstance().GetCaptureSource(id_, true);
    if (source_ == nullptr) {
        return;
    }
    attr_.adapterName = "test";
    attr_.channel = 2; // 2: channel
    source_->Init(attr_);
}

void RemoteAudioCaptureSourceUnitTest::TearDown()
{
    if (source_ && source_->IsInited()) {
        source_->DeInit();
    }
    source_ = nullptr;
}

/**
 * @tc.name   : Test RemoteSource API
 * @tc.number : RemoteSourceUnitTest_001
 * @tc.desc   : Test remote source create
 */
HWTEST_F(RemoteAudioCaptureSourceUnitTest, RemoteSourceUnitTest_001, TestSize.Level1)
{
    EXPECT_TRUE(source_ && source_->IsInited());
}

/**
 * @tc.name   : Test RemoteSource API
 * @tc.number : RemoteSourceUnitTest_002
 * @tc.desc   : Test remote source init
 */
HWTEST_F(RemoteAudioCaptureSourceUnitTest, RemoteSourceUnitTest_002, TestSize.Level1)
{
    EXPECT_TRUE(source_ && source_->IsInited());
    source_->DeInit();
    int32_t ret = source_->Init(attr_);
    EXPECT_EQ(ret, SUCCESS);
    ret = source_->Init(attr_);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(source_->IsInited());
}

/**
 * @tc.name   : Test RemoteSource API
 * @tc.number : RemoteSourceUnitTest_003
 * @tc.desc   : Test remote source start, stop, resume, pause, flush, reset
 */
HWTEST_F(RemoteAudioCaptureSourceUnitTest, RemoteSourceUnitTest_003, TestSize.Level1)
{
    EXPECT_TRUE(source_);
    int32_t ret = source_->Start();
    EXPECT_EQ(ret, ERR_NOT_STARTED);
    ret = source_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    ret = source_->Resume();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
    ret = source_->Pause();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
    ret = source_->Flush();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
    ret = source_->Reset();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
    ret = source_->Stop();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test RemoteSource API
 * @tc.number : RemoteSourceUnitTest_004
 * @tc.desc   : Test remote source capture frame
 */
HWTEST_F(RemoteAudioCaptureSourceUnitTest, RemoteSourceUnitTest_004, TestSize.Level1)
{
    EXPECT_TRUE(source_ && source_->IsInited());
    char *buffer = nullptr;
    uint64_t requestBytes = 0;
    uint64_t replyBytes = 0;
    int32_t ret = source_->CaptureFrame(buffer, requestBytes, replyBytes);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
}

/**
 * @tc.name   : Test RemoteSource API
 * @tc.number : RemoteSourceUnitTest_005
 * @tc.desc   : Test remote source set/get volume
 */
HWTEST_F(RemoteAudioCaptureSourceUnitTest, RemoteSourceUnitTest_005, TestSize.Level1)
{
    EXPECT_TRUE(source_ && source_->IsInited());
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
 * @tc.name   : Test RemoteSource API
 * @tc.number : RemoteSourceUnitTest_006
 * @tc.desc   : Test remote source set/get mute
 */
HWTEST_F(RemoteAudioCaptureSourceUnitTest, RemoteSourceUnitTest_006, TestSize.Level1)
{
    EXPECT_TRUE(source_ && source_->IsInited());
    int32_t ret = source_->SetMute(false);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    bool isMute;
    ret = source_->GetMute(isMute);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
}

/**
 * @tc.name   : Test RemoteSource API
 * @tc.number : RemoteSourceUnitTest_007
 * @tc.desc   : Test remote source set audio scene
 */
HWTEST_F(RemoteAudioCaptureSourceUnitTest, RemoteSourceUnitTest_007, TestSize.Level1)
{
    EXPECT_TRUE(source_ && source_->IsInited());
    int32_t ret = source_->SetAudioScene(AUDIO_SCENE_DEFAULT, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
}

/**
 * @tc.name   : Test RemoteSource API
 * @tc.number : RemoteSourceUnitTest_008
 * @tc.desc   : Test remote source update active device
 */
HWTEST_F(RemoteAudioCaptureSourceUnitTest, RemoteSourceUnitTest_008, TestSize.Level1)
{
    EXPECT_TRUE(source_ && source_->IsInited());
    int32_t ret = source_->UpdateActiveDevice(DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

} // namespace AudioStandard
} // namespace OHOS
