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
#include "sink/file_audio_render_sink.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
class FileAudioRenderSinkUnitTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp();
    virtual void TearDown();

protected:
    static uint32_t id_;
    static std::shared_ptr<IAudioRenderSink> sink_;
    static IAudioSinkAttr attr_;
};

uint32_t FileAudioRenderSinkUnitTest::id_ = HDI_INVALID_ID;
std::shared_ptr<IAudioRenderSink> FileAudioRenderSinkUnitTest::sink_ = nullptr;
IAudioSinkAttr FileAudioRenderSinkUnitTest::attr_ = {};

void FileAudioRenderSinkUnitTest::SetUpTestCase()
{
    id_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_FILE, "test", true);
}

void FileAudioRenderSinkUnitTest::TearDownTestCase()
{
    HdiAdapterManager::GetInstance().ReleaseId(id_);
}

void FileAudioRenderSinkUnitTest::SetUp()
{
    sink_ = HdiAdapterManager::GetInstance().GetRenderSink(id_, true);
    if (sink_ == nullptr) {
        return;
    }
    attr_.filePath = "/data/log/hilog/test.txt";
    sink_->Init(attr_);
}

void FileAudioRenderSinkUnitTest::TearDown()
{
    if (sink_ && sink_->IsInited()) {
        sink_->DeInit();
    }
    sink_ = nullptr;
}

/**
 * @tc.name   : Test FileSink API
 * @tc.number : FileSinkUnitTest_001
 * @tc.desc   : Test file sink create
 */
HWTEST_F(FileAudioRenderSinkUnitTest, FileSinkUnitTest_001, TestSize.Level1)
{
    EXPECT_TRUE(sink_ && sink_->IsInited());
}

/**
 * @tc.name   : Test FileSink API
 * @tc.number : FileSinkUnitTest_002
 * @tc.desc   : Test file sink init
 */
HWTEST_F(FileAudioRenderSinkUnitTest, FileSinkUnitTest_002, TestSize.Level1)
{
    EXPECT_TRUE(sink_ && sink_->IsInited());
    sink_->DeInit();
    int32_t ret = sink_->Init(attr_);
    EXPECT_EQ(ret, SUCCESS);
    ret = sink_->Init(attr_);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(sink_->IsInited());
}

/**
 * @tc.name   : Test FileSink API
 * @tc.number : FileSinkUnitTest_003
 * @tc.desc   : Test file sink start, stop, resume, pause, flush, reset
 */
HWTEST_F(FileAudioRenderSinkUnitTest, FileSinkUnitTest_003, TestSize.Level1)
{
    EXPECT_TRUE(sink_ && sink_->IsInited());
    int32_t ret = sink_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = sink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    ret = sink_->Resume();
    EXPECT_EQ(ret, SUCCESS);
    ret = sink_->Pause();
    EXPECT_EQ(ret, SUCCESS);
    ret = sink_->Flush();
    EXPECT_EQ(ret, SUCCESS);
    ret = sink_->Reset();
    EXPECT_EQ(ret, SUCCESS);
    ret = sink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    sink_->DeInit();
    attr_.filePath = "/invalid_path/test.txt";
    sink_->Init(attr_);
    ret = sink_->Start();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = sink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test FileSink API
 * @tc.number : FileSinkUnitTest_004
 * @tc.desc   : Test file sink render frame
 */
HWTEST_F(FileAudioRenderSinkUnitTest, FileSinkUnitTest_004, TestSize.Level1)
{
    EXPECT_TRUE(sink_ && sink_->IsInited());
    uint64_t writeLen = 0;
    std::vector<char> buffer{'8', '8', '8', '8', '8', '8', '8', '8'};
    int32_t ret = sink_->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = sink_->RenderFrame(*buffer.data(), buffer.size(), writeLen);
    EXPECT_EQ(ret, SUCCESS);
    sink_->SetAudioMonoState(false);
    sink_->SetAudioBalanceValue(false);
    ret = sink_->RenderFrame(*buffer.data(), buffer.size(), writeLen);
    EXPECT_EQ(ret, SUCCESS);
    ret = sink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test FileSink API
 * @tc.number : FileSinkUnitTest_005
 * @tc.desc   : Test file sink suspend, restore
 */
HWTEST_F(FileAudioRenderSinkUnitTest, FileSinkUnitTest_005, TestSize.Level1)
{
    EXPECT_TRUE(sink_ && sink_->IsInited());
    int32_t ret = sink_->SuspendRenderSink();
    EXPECT_EQ(ret, SUCCESS);
    ret = sink_->RestoreRenderSink();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test FileSink API
 * @tc.number : FileSinkUnitTest_006
 * @tc.desc   : Test file sink set/get param
 */
HWTEST_F(FileAudioRenderSinkUnitTest, FileSinkUnitTest_006, TestSize.Level1)
{
    EXPECT_TRUE(sink_ && sink_->IsInited());
    sink_->SetAudioParameter(NONE, "", "");
    std::string param = sink_->GetAudioParameter(NONE, "");
    EXPECT_EQ(param, "");
}

/**
 * @tc.name   : Test DirectSink API
 * @tc.number : FileSinkUnitTest_007
 * @tc.desc   : Test file sink set/get volume
 */
HWTEST_F(FileAudioRenderSinkUnitTest, FileSinkUnitTest_007, TestSize.Level1)
{
    EXPECT_TRUE(sink_ && sink_->IsInited());
    int32_t ret = sink_->SetVolume(0.0f, 0.0f);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
    float left;
    float right;
    ret = sink_->GetVolume(left, right);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name   : Test FileSink API
 * @tc.number : FileSinkUnitTest_008
 * @tc.desc   : Test file sink set/get audio scene
 */
HWTEST_F(FileAudioRenderSinkUnitTest, FileSinkUnitTest_008, TestSize.Level1)
{
    EXPECT_TRUE(sink_ && sink_->IsInited());
    int32_t ret = sink_->SetAudioScene(AUDIO_SCENE_DEFAULT);
    EXPECT_EQ(ret, SUCCESS);
    ret = sink_->GetAudioScene();
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name   : Test FileSink API
 * @tc.number : FileSinkUnitTest_009
 * @tc.desc   : Test file sink update active device
 */
HWTEST_F(FileAudioRenderSinkUnitTest, FileSinkUnitTest_009, TestSize.Level1)
{
    EXPECT_TRUE(sink_ && sink_->IsInited());
    std::vector<DeviceType> deviceTypes = { DEVICE_TYPE_SPEAKER };
    int32_t ret = sink_->UpdateActiveDevice(deviceTypes);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name   : Test FileSink API
 * @tc.number : FileSinkUnitTest_010
 * @tc.desc   : Test file sink update apps uid
 */
HWTEST_F(FileAudioRenderSinkUnitTest, FileSinkUnitTest_010, TestSize.Level1)
{
    EXPECT_TRUE(sink_&& sink_->IsInited());
    vector<int32_t> appsUid;
    int32_t ret = sink_->UpdateAppsUid(appsUid);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
    int32_t appsUidArr[10] = { 0 }; // 10: array size
    ret = sink_->UpdateAppsUid(appsUidArr, 10); // 10: array size
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name   : Test FileSink API
 * @tc.number : FileSinkUnitTest_011
 * @tc.desc   : Test file sink dump info
 */
HWTEST_F(FileAudioRenderSinkUnitTest, FileSinkUnitTest_011, TestSize.Level1)
{
    EXPECT_TRUE(sink_&& sink_->IsInited());
    std::string info;
    sink_->DumpInfo(info);
    EXPECT_EQ(info.empty(), false);
}

/**
 * @tc.name   : Test FileSink API
 * @tc.number : FileSinkUnitTest_012
 * @tc.desc   : Test file sink start with file
 */
HWTEST_F(FileAudioRenderSinkUnitTest, FileSinkUnitTest_012, TestSize.Level1)
{
    FileAudioRenderSink fileSink;
    fileSink.filePath_ = "/data/data/utTestFilel.pcm";
    int32_t res = fileSink.Start();
    EXPECT_EQ(res, SUCCESS);

    res = fileSink.Stop();
    EXPECT_EQ(res, SUCCESS);

    fileSink.DeInit();
}

/**
 * @tc.name   : Test FileSink API
 * @tc.number : FileSinkUnitTest_013
 * @tc.desc   : Test file sink start without file
 */
HWTEST_F(FileAudioRenderSinkUnitTest, FileSinkUnitTest_013, TestSize.Level1)
{
    FileAudioRenderSink fileSink;
    fileSink.filePath_ = "/data/data/utTestFilel.pcm";
    int32_t res = fileSink.Start();
    EXPECT_NE(res, SUCCESS);

    res = fileSink.Stop();
    EXPECT_EQ(res, SUCCESS);

    fileSink.DeInit();
}

} // namespace AudioStandard
} // namespace OHOS
