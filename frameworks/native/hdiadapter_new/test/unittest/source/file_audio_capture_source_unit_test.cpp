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
class FileAudioCaptureSourceUnitTest : public testing::Test {
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

uint32_t FileAudioCaptureSourceUnitTest::id_ = HDI_INVALID_ID;
std::shared_ptr<IAudioCaptureSource> FileAudioCaptureSourceUnitTest::source_ = nullptr;
IAudioSourceAttr FileAudioCaptureSourceUnitTest::attr_ = {};

void FileAudioCaptureSourceUnitTest::SetUpTestCase()
{
    id_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_CAPTURE, HDI_ID_TYPE_FILE, "test", true);
}

void FileAudioCaptureSourceUnitTest::TearDownTestCase()
{
    HdiAdapterManager::GetInstance().ReleaseId(id_);
}

void FileAudioCaptureSourceUnitTest::SetUp()
{
    source_ = HdiAdapterManager::GetInstance().GetCaptureSource(id_, true);
    if (source_ == nullptr) {
        return;
    }
    attr_.filePath = "/data/log/hilog/test.txt";
    source_->Init(attr_);
}

void FileAudioCaptureSourceUnitTest::TearDown()
{
    if (source_ && source_->IsInited()) {
        source_->DeInit();
    }
    source_ = nullptr;
}

/**
 * @tc.name   : Test FileSource API
 * @tc.number : FileSourceUnitTest_001
 * @tc.desc   : Test file source create
 */
HWTEST_F(FileAudioCaptureSourceUnitTest, FileSourceUnitTest_001, TestSize.Level1)
{
    EXPECT_TRUE(source_ && source_->IsInited());
}

/**
 * @tc.name   : Test FileSource API
 * @tc.number : FileSourceUnitTest_002
 * @tc.desc   : Test file source init
 */
HWTEST_F(FileAudioCaptureSourceUnitTest, FileSourceUnitTest_002, TestSize.Level1)
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
 * @tc.name   : Test FileSource API
 * @tc.number : FileSourceUnitTest_003
 * @tc.desc   : Test file source start, stop, resume, pause, flush, reset
 */
HWTEST_F(FileAudioCaptureSourceUnitTest, FileSourceUnitTest_003, TestSize.Level1)
{
    EXPECT_TRUE(source_ && source_->IsInited());
    int32_t ret = source_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = source_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    ret = source_->Resume();
    EXPECT_EQ(ret, SUCCESS);
    ret = source_->Pause();
    EXPECT_EQ(ret, SUCCESS);
    ret = source_->Flush();
    EXPECT_EQ(ret, SUCCESS);
    ret = source_->Reset();
    EXPECT_EQ(ret, SUCCESS);
    ret = source_->Stop();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test FileSource API
 * @tc.number : FileSourceUnitTest_004
 * @tc.desc   : Test file source capture frame
 */
HWTEST_F(FileAudioCaptureSourceUnitTest, FileSourceUnitTest_004, TestSize.Level1)
{
    EXPECT_TRUE(source_ && source_->IsInited());
    uint64_t replyBytes = 0;
    uint64_t replyBytesEc = 0;
    std::vector<char> buffer{'8', '8', '8', '8', '8', '8', '8', '8'};
    std::vector<char> bufferEc{'8', '8', '8', '8', '8', '8', '8', '8'};
    int32_t ret = source_->CaptureFrame(buffer.data(), buffer.size(), replyBytes);
    EXPECT_EQ(ret, SUCCESS);
    FrameDesc desc = {
        .frame = buffer.data(),
        .frameLen = buffer.size(),
    };
    FrameDesc descEc = {
        .frame = bufferEc.data(),
        .frameLen = bufferEc.size(),
    };
    ret = source_->CaptureFrameWithEc(&desc, replyBytes, &descEc, replyBytesEc);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name   : Test DirectSource API
 * @tc.number : FileSourceUnitTest_005
 * @tc.desc   : Test file source set/get volume
 */
HWTEST_F(FileAudioCaptureSourceUnitTest, FileSourceUnitTest_005, TestSize.Level1)
{
    EXPECT_TRUE(source_ && source_->IsInited());
    int32_t ret = source_->SetVolume(0.0f, 0.0f);
    EXPECT_EQ(ret, SUCCESS);
    float left;
    float right;
    ret = source_->GetVolume(left, right);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test FileSource API
 * @tc.number : FileSourceUnitTest_006
 * @tc.desc   : Test file source set/get mute
 */
HWTEST_F(FileAudioCaptureSourceUnitTest, FileSourceUnitTest_006, TestSize.Level1)
{
    EXPECT_TRUE(source_ && source_->IsInited());
    int32_t ret = source_->SetMute(false);
    EXPECT_EQ(ret, SUCCESS);
    bool mute;
    ret = source_->GetMute(mute);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test FileSource API
 * @tc.number : FileSourceUnitTest_007
 * @tc.desc   : Test file source get transaction id
 */
HWTEST_F(FileAudioCaptureSourceUnitTest, FileSourceUnitTest_007, TestSize.Level1)
{
    EXPECT_TRUE(source_ && source_->IsInited());
    uint64_t transId = source_->GetTransactionId();
    EXPECT_EQ(transId, -1);
}

/**
 * @tc.name   : Test FileSource API
 * @tc.number : FileSourceUnitTest_008
 * @tc.desc   : Test file source get presentation position
 */
HWTEST_F(FileAudioCaptureSourceUnitTest, FileSourceUnitTest_008, TestSize.Level1)
{
    EXPECT_TRUE(source_ && source_->IsInited());
    uint64_t frame = 10;
    int64_t timeSec = 10;
    int64_t timeNanoSec = 10;
    int32_t ret = source_->GetPresentationPosition(frame, timeSec, timeNanoSec);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test FileSource API
 * @tc.number : FileSourceUnitTest_009
 * @tc.desc   : Test file source get max amplitude
 */
HWTEST_F(FileAudioCaptureSourceUnitTest, FileSourceUnitTest_009, TestSize.Level1)
{
    EXPECT_TRUE(source_ && source_->IsInited());
    float maxAmplitude = source_->GetMaxAmplitude();
    EXPECT_EQ(maxAmplitude, 0.0f);
}

/**
 * @tc.name   : Test FileSource API
 * @tc.number : FileSourceUnitTest_010
 * @tc.desc   : Test file source set audio scene
 */
HWTEST_F(FileAudioCaptureSourceUnitTest, FileSourceUnitTest_010, TestSize.Level1)
{
    EXPECT_TRUE(source_ && source_->IsInited());
    int32_t ret = source_->SetAudioScene(AUDIO_SCENE_DEFAULT);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test FileSource API
 * @tc.number : FileSourceUnitTest_011
 * @tc.desc   : Test file source update active device
 */
HWTEST_F(FileAudioCaptureSourceUnitTest, FileSourceUnitTest_011, TestSize.Level1)
{
    EXPECT_TRUE(source_ && source_->IsInited());
    int32_t ret = source_->UpdateActiveDevice(DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test FileSource API
 * @tc.number : FileSourceUnitTest_012
 * @tc.desc   : Test file source update apps uid
 */
HWTEST_F(FileAudioCaptureSourceUnitTest, FileSourceUnitTest_012, TestSize.Level1)
{
    EXPECT_TRUE(source_&& source_->IsInited());
    vector<int32_t> appsUid;
    int32_t ret = source_->UpdateAppsUid(appsUid);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
    int32_t appsUidArr[10] = { 0 }; // 10: array size
    ret = source_->UpdateAppsUid(appsUidArr, 10); // 10: array size
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name   : Test FileSource API
 * @tc.number : FileSourceUnitTest_013
 * @tc.desc   : Test file source dump info
 */
HWTEST_F(FileAudioCaptureSourceUnitTest, FileSourceUnitTest_013, TestSize.Level1)
{
    EXPECT_TRUE(source_&& source_->IsInited());
    std::string info;
    source_->DumpInfo(info);
    EXPECT_EQ(info.empty(), false);
}

} // namespace AudioStandard
} // namespace OHOS
