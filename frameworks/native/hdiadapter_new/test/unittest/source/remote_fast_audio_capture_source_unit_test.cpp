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
class RemoteFastAudioCaptureSourceUnitTest : public testing::Test {
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

uint32_t RemoteFastAudioCaptureSourceUnitTest::id_ = HDI_INVALID_ID;
std::shared_ptr<IAudioCaptureSource> RemoteFastAudioCaptureSourceUnitTest::source_ = nullptr;
IAudioSourceAttr RemoteFastAudioCaptureSourceUnitTest::attr_ = {};

void RemoteFastAudioCaptureSourceUnitTest::SetUpTestCase()
{
    id_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_REMOTE_FAST, "test", true);
}

void RemoteFastAudioCaptureSourceUnitTest::TearDownTestCase()
{
    HdiAdapterManager::GetInstance().ReleaseId(id_);
}

void RemoteFastAudioCaptureSourceUnitTest::SetUp()
{
    source_ = HdiAdapterManager::GetInstance().GetCaptureSource(id_, true);
    if (source_ == nullptr) {
        return;
    }
    attr_.adapterName = "test";
    attr_.channel = 2; // 2: channel
    source_->Init(attr_);
}

void RemoteFastAudioCaptureSourceUnitTest::TearDown()
{
    if (source_ && source_->IsInited()) {
        source_->DeInit();
    }
    source_ = nullptr;
}

/**
 * @tc.name   : Test RemoteFastSource API
 * @tc.number : RemoteFastSourceUnitTest_001
 * @tc.desc   : Test remote fast source create
 */
HWTEST_F(RemoteFastAudioCaptureSourceUnitTest, RemoteFastSourceUnitTest_001, TestSize.Level1)
{
    EXPECT_TRUE(source_);
}

/**
 * @tc.name   : Test RemoteFastSource API
 * @tc.number : RemoteFastSourceUnitTest_002
 * @tc.desc   : Test remote fast source deinit
 */
HWTEST_F(RemoteFastAudioCaptureSourceUnitTest, RemoteFastSourceUnitTest_002, TestSize.Level1)
{
    EXPECT_TRUE(source_);
    source_->DeInit();
    EXPECT_FALSE(source_->IsInited());
}

/**
 * @tc.name   : Test RemoteFastSource API
 * @tc.number : RemoteFastSourceUnitTest_003
 * @tc.desc   : Test remote fast source start, stop, resume, pause, flush, reset
 */
HWTEST_F(RemoteFastAudioCaptureSourceUnitTest, RemoteFastSourceUnitTest_003, TestSize.Level1)
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

} // namespace AudioStandard
} // namespace OHOS
