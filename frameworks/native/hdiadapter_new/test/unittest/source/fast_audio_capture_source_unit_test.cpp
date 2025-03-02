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
class FastAudioCaptureSourceUnitTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() {}
    virtual void TearDown() {}

    void InitFastSource();
    void DeInitFastSource();
    void InitFastVoipSource();
    void DeInitFastVoipSource();

protected:
    static uint32_t fastId_;
    static uint32_t fastVoipId_;
    static std::shared_ptr<IAudioCaptureSource> fastSource_;
    static std::shared_ptr<IAudioCaptureSource> fastVoipSource_;
    static IAudioSourceAttr attr_;
};

uint32_t FastAudioCaptureSourceUnitTest::fastId_ = 0;
uint32_t FastAudioCaptureSourceUnitTest::fastVoipId_ = 0;
std::shared_ptr<IAudioCaptureSource> FastAudioCaptureSourceUnitTest::fastSource_ = nullptr;
std::shared_ptr<IAudioCaptureSource> FastAudioCaptureSourceUnitTest::fastVoipSource_ = nullptr;
IAudioSourceAttr FastAudioCaptureSourceUnitTest::attr_ = {};

void FastAudioCaptureSourceUnitTest::SetUpTestCase()
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    fastId_ = manager.GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_FAST, HDI_ID_INFO_DEFAULT, true);
    fastVoipId_ = manager.GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_FAST, HDI_ID_INFO_VOIP, true);
}

void FastAudioCaptureSourceUnitTest::TearDownTestCase()
{
    HdiAdapterManager::GetInstance().ReleaseId(fastId_);
    HdiAdapterManager::GetInstance().ReleaseId(fastVoipId_);
}

void FastAudioCaptureSourceUnitTest::InitFastSource()
{
    fastSource_ = HdiAdapterManager::GetInstance().GetCaptureSource(fastId_, true);
    if (fastSource_ == nullptr) {
        return;
    }
    attr_.adapterName = "primary";
    attr_.sampleRate = 48000; // 48000: sample rate
    attr_.channel = 2; // 2: channel
    attr_.format = SAMPLE_S16LE;
    attr_.channelLayout = 3; // 3: channel layout
    attr_.deviceType = DEVICE_TYPE_MIC;
    attr_.openMicSpeaker = 1;
    fastSource_->Init(attr_);
}

void FastAudioCaptureSourceUnitTest::DeInitFastSource()
{
    if (fastSource_ && fastSource_->IsInited()) {
        fastSource_->DeInit();
    }
    fastSource_ = nullptr;
}

void FastAudioCaptureSourceUnitTest::InitFastVoipSource()
{
    fastVoipSource_ = HdiAdapterManager::GetInstance().GetCaptureSource(fastVoipId_, true);
    if (fastVoipSource_ == nullptr) {
        return;
    }
    attr_.adapterName = "primary";
    attr_.sampleRate = 48000; // 48000: sample rate
    attr_.channel = 2; // 2: channel
    attr_.format = SAMPLE_S16LE;
    attr_.channelLayout = 3; // 3: channel layout
    attr_.deviceType = DEVICE_TYPE_MIC;
    attr_.openMicSpeaker = 1;
    fastVoipSource_->Init(attr_);
}

void FastAudioCaptureSourceUnitTest::DeInitFastVoipSource()
{
    if (fastVoipSource_ && fastVoipSource_->IsInited()) {
        fastVoipSource_->DeInit();
    }
    fastVoipSource_ = nullptr;
}

/**
 * @tc.name   : Test FastSource API
 * @tc.number : FastSourceUnitTest_001
 * @tc.desc   : Test fast source create
 */
HWTEST_F(FastAudioCaptureSourceUnitTest, FastSourceUnitTest_001, TestSize.Level1)
{
    InitFastSource();
    EXPECT_TRUE(fastSource_ != nullptr);
    DeInitFastSource();
}

/**
 * @tc.name   : Test FastSource API
 * @tc.number : FastSourceUnitTest_002
 * @tc.desc   : Test fast source init
 */
HWTEST_F(FastAudioCaptureSourceUnitTest, FastSourceUnitTest_002, TestSize.Level1)
{
    InitFastSource();
    EXPECT_TRUE(fastSource_ && fastSource_->IsInited());
    DeInitFastSource();
}
 
/**
 * @tc.name   : Test FastSource API
 * @tc.number : FastSourceUnitTest_003
 * @tc.desc   : Test fast source start, stop, resume, pause, flush, reset
 */
HWTEST_F(FastAudioCaptureSourceUnitTest, FastSourceUnitTest_003, TestSize.Level1)
{
    InitFastSource();
    EXPECT_TRUE(fastSource_ && fastSource_->IsInited());
    int32_t ret = fastSource_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = fastSource_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    ret = fastSource_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = fastSource_->Resume();
    EXPECT_EQ(ret, SUCCESS);
    ret = fastSource_->Pause();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    ret = fastSource_->Flush();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    ret = fastSource_->Reset();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    ret = fastSource_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    DeInitFastSource();
}

/**
 * @tc.name   : Test FastSource API
 * @tc.number : FastSourceUnitTest_004
 * @tc.desc   : Test fast source set volume
 */
HWTEST_F(FastAudioCaptureSourceUnitTest, FastSourceUnitTest_004, TestSize.Level1)
{
    InitFastSource();
    EXPECT_TRUE(fastSource_ && fastSource_->IsInited());
    int32_t ret = fastSource_->SetVolume(1.0f, 1.0f);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
    DeInitFastSource();
}

/**
 * @tc.name   : Test FastVoipSource API
 * @tc.number : FastVoipSourceUnitTest_001
 * @tc.desc   : Test fast voip source create
 */
HWTEST_F(FastAudioCaptureSourceUnitTest, FastVoipSourceUnitTest_001, TestSize.Level1)
{
    InitFastVoipSource();
    EXPECT_TRUE(fastVoipSource_ != nullptr);
    DeInitFastVoipSource();
}

/**
 * @tc.name   : Test FastVoipSource API
 * @tc.number : FastVoipSourceUnitTest_002
 * @tc.desc   : Test fast voip source init
 */
HWTEST_F(FastAudioCaptureSourceUnitTest, FastVoipSourceUnitTest_002, TestSize.Level1)
{
    InitFastVoipSource();
    EXPECT_TRUE(fastVoipSource_ && fastVoipSource_->IsInited());
    DeInitFastVoipSource();
}
 
/**
 * @tc.name   : Test FastVoipSource API
 * @tc.number : FastVoipSourceUnitTest_003
 * @tc.desc   : Test fast voip source start, stop, resume, pause, flush, reset
 */
HWTEST_F(FastAudioCaptureSourceUnitTest, FastVoipSourceUnitTest_003, TestSize.Level1)
{
    InitFastVoipSource();
    EXPECT_TRUE(fastVoipSource_ && fastVoipSource_->IsInited());
    int32_t ret = fastVoipSource_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = fastVoipSource_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    ret = fastVoipSource_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = fastVoipSource_->Resume();
    EXPECT_EQ(ret, SUCCESS);
    ret = fastVoipSource_->Pause();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    ret = fastVoipSource_->Flush();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    ret = fastVoipSource_->Reset();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    DeInitFastSource();
}

/**
 * @tc.name   : Test FastVoipSource API
 * @tc.number : FastVoipSourceUnitTest_004
 * @tc.desc   : Test fast voipsource set volume
 */
HWTEST_F(FastAudioCaptureSourceUnitTest, FastVoipSourceUnitTest_004, TestSize.Level1)
{
    InitFastVoipSource();
    EXPECT_TRUE(fastVoipSource_ && fastVoipSource_->IsInited());
    int32_t ret = fastVoipSource_->SetVolume(1.0f, 1.0f);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
    DeInitFastVoipSource();
}

} // namespace AudioStandard
} // namespace OHOS
