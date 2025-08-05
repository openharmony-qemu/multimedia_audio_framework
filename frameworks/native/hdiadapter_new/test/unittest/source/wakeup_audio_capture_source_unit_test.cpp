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
class WakeupAudioCaptureSourceUnitTest : public testing::Test {
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

uint32_t WakeupAudioCaptureSourceUnitTest::id_ = HDI_INVALID_ID;
std::shared_ptr<IAudioCaptureSource> WakeupAudioCaptureSourceUnitTest::source_ = nullptr;
IAudioSourceAttr WakeupAudioCaptureSourceUnitTest::attr_ = {};

void WakeupAudioCaptureSourceUnitTest::SetUpTestCase()
{
    id_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_WAKEUP, "test", true);
}

void WakeupAudioCaptureSourceUnitTest::TearDownTestCase()
{
    HdiAdapterManager::GetInstance().ReleaseId(id_);
}

void WakeupAudioCaptureSourceUnitTest::SetUp()
{
    source_ = HdiAdapterManager::GetInstance().GetCaptureSource(id_, true);
    if (source_ == nullptr) {
        return;
    }
    attr_.adapterName = "primary";
    attr_.sampleRate = 48000; // 48000: sample rate
    attr_.channel = 2; // 2: channel
    attr_.format = SAMPLE_S16LE;
    attr_.channelLayout = 3; // 3: channel layout
    attr_.deviceType = DEVICE_TYPE_MIC;
    attr_.openMicSpeaker = 1;
    source_->Init(attr_);
}

void WakeupAudioCaptureSourceUnitTest::TearDown()
{
    if (source_ && source_->IsInited()) {
        source_->DeInit();
    }
    source_ = nullptr;
}

/**
 * @tc.name   : Test WakeupSource API
 * @tc.number : WakeupSourceUnitTest_001
 * @tc.desc   : Test wakeup source create
 */
HWTEST_F(WakeupAudioCaptureSourceUnitTest, WakeupSourceUnitTest_001, TestSize.Level1)
{
    EXPECT_TRUE(source_);
}

/**
* @tc.name   : Test WakeupSource API
* @tc.number : WakeupSourceUnitTest_002
* @tc.desc   : Test wakeup source init
*/
HWTEST_F(WakeupAudioCaptureSourceUnitTest, WakeupSourceUnitTest_002, TestSize.Level1)
{
    EXPECT_TRUE(source_ && source_->IsInited());
}

/**
* @tc.name   : Test WakeupSource API
* @tc.number : WakeupSourceUnitTest_003
* @tc.desc   : Test wakeup source start, stop, resume, pause, flush, reset
*/
HWTEST_F(WakeupAudioCaptureSourceUnitTest, WakeupSourceUnitTest_003, TestSize.Level1)
{
    EXPECT_TRUE(source_ && source_->IsInited());
    int32_t ret = source_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = source_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    ret = source_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = source_->Resume();
    EXPECT_EQ(ret, SUCCESS);
    ret = source_->Pause();
    EXPECT_NE(ret, SUCCESS);
    ret = source_->Flush();
    EXPECT_NE(ret, SUCCESS);
    ret = source_->Reset();
    EXPECT_NE(ret, SUCCESS);
    ret = source_->Stop();
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name   : Test WakeupSource API
* @tc.number : WakeupSourceUnitTest_004
* @tc.desc   : Test wakeup source capture frame
*/
HWTEST_F(WakeupAudioCaptureSourceUnitTest, WakeupSourceUnitTest_004, TestSize.Level1)
{
    EXPECT_TRUE(source_ && source_->IsInited());
    uint64_t replyBytes = 0;
    std::vector<char> buffer{'8', '8', '8', '8', '8', '8', '8', '8'};
    int32_t ret = source_->CaptureFrame(buffer.data(), buffer.size(), replyBytes);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test WakeupSource API
 * @tc.number : WakeupSourceUnitTest_005
 * @tc.desc   : Test wakeup source get param
 */
HWTEST_F(WakeupAudioCaptureSourceUnitTest, WakeupSourceUnitTest_005, TestSize.Level1)
{
    EXPECT_TRUE(source_ && source_->IsInited());
    std::string param = source_->GetAudioParameter(USB_DEVICE, "");
    EXPECT_EQ(param, "");
}

/**
 * @tc.name   : Test WakeupSource API
 * @tc.number : WakeupSourceUnitTest_006
 * @tc.desc   : Test wakeup source set/get volume
 */
HWTEST_F(WakeupAudioCaptureSourceUnitTest, WakeupSourceUnitTest_006, TestSize.Level1)
{
    EXPECT_TRUE(source_);
    int32_t ret = source_->SetVolume(1.0f, 1.0f);
    EXPECT_NE(ret, SUCCESS);
    float left;
    float right;
    ret = source_->GetVolume(left, right);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test WakeupSource API
 * @tc.number : WakeupSourceUnitTest_007
 * @tc.desc   : Test wakeup source set/get mute
 */
HWTEST_F(WakeupAudioCaptureSourceUnitTest, WakeupSourceUnitTest_007, TestSize.Level1)
{
    EXPECT_TRUE(source_);
    int32_t ret = source_->SetMute(false);
    EXPECT_EQ(ret, SUCCESS);
    bool mute = false;
    ret = source_->GetMute(mute);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_FALSE(mute);
}
 
/**
 * @tc.name   : Test WakeupSource API
 * @tc.number : WakeupSourceUnitTest_008
 * @tc.desc   : Test wakeup source get transaction id
 */
HWTEST_F(WakeupAudioCaptureSourceUnitTest, WakeupSourceUnitTest_008, TestSize.Level1)
{
    EXPECT_TRUE(source_);
    uint64_t transId = source_->GetTransactionId();
    EXPECT_NE(transId, 0);
}
 
/**
 * @tc.name   : Test WakeupSource API
 * @tc.number : WakeupSourceUnitTest_009
 * @tc.desc   : Test wakeup source get max amplitude
 */
HWTEST_F(WakeupAudioCaptureSourceUnitTest, WakeupSourceUnitTest_009, TestSize.Level1)
{
    EXPECT_TRUE(source_);
    float maxAmplitude = source_->GetMaxAmplitude();
    EXPECT_EQ(maxAmplitude, 0.0f);
}
 
/**
 * @tc.name   : Test WakeupSource API
 * @tc.number : WakeupSourceUnitTest_010
 * @tc.desc   : Test wakeup source set audio scene
 */
HWTEST_F(WakeupAudioCaptureSourceUnitTest, WakeupSourceUnitTest_010, TestSize.Level1)
{
    EXPECT_TRUE(source_);
    int32_t ret = source_->SetAudioScene(AUDIO_SCENE_DEFAULT);
    EXPECT_EQ(ret, SUCCESS);
}
 
/**
 * @tc.name   : Test WakeupSource API
 * @tc.number : WakeupSourceUnitTest_011
 * @tc.desc   : Test wakeup source update source type
 */
HWTEST_F(WakeupAudioCaptureSourceUnitTest, WakeupSourceUnitTest_011, TestSize.Level1)
{
    EXPECT_TRUE(source_);
    int32_t ret = source_->UpdateSourceType(SOURCE_TYPE_MIC);
    EXPECT_EQ(ret, SUCCESS);
}
 
/**
 * @tc.name   : Test WakeupSource API
 * @tc.number : WakeupSourceUnitTest_012
 * @tc.desc   : Test wakeup source update apps uid
 */
HWTEST_F(WakeupAudioCaptureSourceUnitTest, WakeupSourceUnitTest_012, TestSize.Level1)
{
    EXPECT_TRUE(source_);
    vector<int32_t> appsUid;
    int32_t ret = source_->UpdateAppsUid(appsUid);
    EXPECT_EQ(ret, SUCCESS);
}

} // namespace AudioStandard
} // namespace OHOS
