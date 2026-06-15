/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "file_audio_capture_source_aiut.h"
#include "aiut_log.h"
#include "source/file_audio_capture_source.h"
#include "source/i_audio_capture_source.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

// ==================== FileAudioCaptureSourceAiut ====================

void FileAudioCaptureSourceAiut::SetUpTestCase(void) {}
void FileAudioCaptureSourceAiut::TearDownTestCase(void) {}
void FileAudioCaptureSourceAiut::SetUp(void) {}
void FileAudioCaptureSourceAiut::TearDown(void) {}

// ==================== Construction / Destruction Tests ====================

/**
 * @tc.name  : Test FileAudioCaptureSource construction
 * @tc.number: FileCaptureSource_Constructor_001
 * @tc.desc  : Test FileAudioCaptureSource default constructor creates uninitialized object.
 */
HWTEST_F(FileAudioCaptureSourceAiut, FileCaptureSource_Constructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<FileAudioCaptureSource>();
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    AIUT_EXPECT_FALSE_INFO_LOG(source->IsInited(), "newly constructed source should not be inited");
}

/**
 * @tc.name  : Test FileAudioCaptureSource destruction
 * @tc.number: FileCaptureSource_Destructor_001
 * @tc.desc  : Test destructor on uninitialized source does not crash.
 */
HWTEST_F(FileAudioCaptureSourceAiut, FileCaptureSource_Destructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<FileAudioCaptureSource>();
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    source = nullptr;
    AIUT_EXPECT_EQ_INFO_LOG(source, nullptr, "source should be null after reset");
}

// ==================== Lifecycle Tests ====================

/**
 * @tc.name  : Test FileAudioCaptureSource Start without init
 * @tc.number: FileCaptureSource_StartWithoutInit_001
 * @tc.desc  : Test Start() before Init() returns error.
 */
HWTEST_F(FileAudioCaptureSourceAiut, FileCaptureSource_StartWithoutInit_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<FileAudioCaptureSource>();
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->Start();
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS,
        "Start without init should not return SUCCESS, got %d", ret);
}

/**
 * @tc.name  : Test FileAudioCaptureSource Stop without init
 * @tc.number: FileCaptureSource_StopWithoutInit_001
 * @tc.desc  : Test Stop() without Init() returns SUCCESS.
 */
HWTEST_F(FileAudioCaptureSourceAiut, FileCaptureSource_StopWithoutInit_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<FileAudioCaptureSource>();
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->Stop();
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "expected SUCCESS for Stop without init, got %d", ret);
}

/**
 * @tc.name  : Test FileAudioCaptureSource Resume without init
 * @tc.number: FileCaptureSource_ResumeWithoutInit_001
 * @tc.desc  : Test Resume() without Init() returns SUCCESS.
 */
HWTEST_F(FileAudioCaptureSourceAiut, FileCaptureSource_ResumeWithoutInit_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<FileAudioCaptureSource>();
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->Resume();
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "expected SUCCESS for Resume without init, got %d", ret);
}

/**
 * @tc.name  : Test FileAudioCaptureSource Pause without init
 * @tc.number: FileCaptureSource_PauseWithoutInit_001
 * @tc.desc  : Test Pause() without Init() returns SUCCESS.
 */
HWTEST_F(FileAudioCaptureSourceAiut, FileCaptureSource_PauseWithoutInit_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<FileAudioCaptureSource>();
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->Pause();
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "expected SUCCESS for Pause without init, got %d", ret);
}

/**
 * @tc.name  : Test FileAudioCaptureSource Flush without init
 * @tc.number: FileCaptureSource_FlushWithoutInit_001
 * @tc.desc  : Test Flush() without Init() returns SUCCESS.
 */
HWTEST_F(FileAudioCaptureSourceAiut, FileCaptureSource_FlushWithoutInit_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<FileAudioCaptureSource>();
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->Flush();
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "expected SUCCESS for Flush without init, got %d", ret);
}

/**
 * @tc.name  : Test FileAudioCaptureSource Reset without init
 * @tc.number: FileCaptureSource_ResetWithoutInit_001
 * @tc.desc  : Test Reset() without Init() returns SUCCESS.
 */
HWTEST_F(FileAudioCaptureSourceAiut, FileCaptureSource_ResetWithoutInit_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<FileAudioCaptureSource>();
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->Reset();
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "expected SUCCESS for Reset without init, got %d", ret);
}

/**
 * @tc.name  : Test FileAudioCaptureSource UpdateAppsUid without init
 * @tc.number: FileCaptureSource_UpdateAppsUid_001
 * @tc.desc  : Test UpdateAppsUid without Init() returns ERR_NOT_SUPPORTED.
 */
HWTEST_F(FileAudioCaptureSourceAiut, FileCaptureSource_UpdateAppsUid_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<FileAudioCaptureSource>();
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    std::vector<int32_t> appsUid;
    int32_t ret = source->UpdateAppsUid(appsUid);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED,
        "expected ERR_NOT_SUPPORTED for UpdateAppsUid without init, got %d", ret);
}

} // namespace AudioStandard
} // namespace OHOS
