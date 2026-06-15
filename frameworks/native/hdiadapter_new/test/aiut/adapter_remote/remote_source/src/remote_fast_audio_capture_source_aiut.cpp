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

#include "remote_fast_audio_capture_source_aiut.h"
#include "aiut_log.h"
#include "source/remote_fast_audio_capture_source.h"
#include "source/i_audio_capture_source.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

// ==================== RemoteFastAudioCaptureSourceAiut ====================

void RemoteFastAudioCaptureSourceAiut::SetUpTestCase(void) {}
void RemoteFastAudioCaptureSourceAiut::TearDownTestCase(void) {}
void RemoteFastAudioCaptureSourceAiut::SetUp(void) {}
void RemoteFastAudioCaptureSourceAiut::TearDown(void) {}

// ==================== Construction / Destruction Tests ====================

/**
 * @tc.name  : Test RemoteFastAudioCaptureSource construction
 * @tc.number: RemoteFastCaptureSource_Constructor_001
 * @tc.desc  : Test RemoteFastAudioCaptureSource constructor creates uninitialized object.
 */
HWTEST_F(RemoteFastAudioCaptureSourceAiut, RemoteFastCaptureSource_Constructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<RemoteFastAudioCaptureSource>("test");
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    AIUT_EXPECT_FALSE_INFO_LOG(source->IsInited(), "newly constructed source should not be inited");
}

/**
 * @tc.name  : Test RemoteFastAudioCaptureSource destruction
 * @tc.number: RemoteFastCaptureSource_Destructor_001
 * @tc.desc  : Test destructor on uninitialized source does not crash.
 */
HWTEST_F(RemoteFastAudioCaptureSourceAiut, RemoteFastCaptureSource_Destructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<RemoteFastAudioCaptureSource>("test");
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    source = nullptr;
    AIUT_EXPECT_EQ_INFO_LOG(source, nullptr, "source should be null after reset");
}

// ==================== SetInvalidState Tests ====================

/**
 * @tc.name  : Test RemoteFastAudioCaptureSource SetInvalidState and Init/Start
 * @tc.number: RemoteFastCaptureSource_SetInvalidState_001
 * @tc.desc  : Test SetInvalidState followed by Init and Start returns ERR_NOT_STARTED.
 */
HWTEST_F(RemoteFastAudioCaptureSourceAiut, RemoteFastCaptureSource_SetInvalidState_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<RemoteFastAudioCaptureSource>("test");
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    IAudioSourceAttr attr = {};
    attr.adapterName = "test";
    attr.channel = 2;
    source->SetInvalidState();
    int32_t ret = source->Init(attr);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_STARTED,
        "expected ERR_NOT_STARTED for Init after SetInvalidState, got %d", ret);
    ret = source->Start();
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_STARTED,
        "expected ERR_NOT_STARTED for Start after SetInvalidState, got %d", ret);

    source->captureInited_.store(true);
    ret = source->Init(attr);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_STARTED,
        "expected ERR_NOT_STARTED for Init with captureInited only, got %d", ret);
    ret = source->Start();
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_STARTED,
        "expected ERR_NOT_STARTED for Start with captureInited only, got %d", ret);

    source->validState_.store(true);
    ret = source->Init(attr);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS,
        "expected SUCCESS for Init with all states true, got %d", ret);
    ret = source->Start();
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_HANDLE,
        "expected ERR_INVALID_HANDLE for Start without capture handle, got %d", ret);
}

// ==================== Lifecycle Tests ====================

/**
 * @tc.name  : Test RemoteFastAudioCaptureSource Start without init
 * @tc.number: RemoteFastCaptureSource_StartWithoutInit_001
 * @tc.desc  : Test Start() before Init() returns ERR_NOT_STARTED.
 */
HWTEST_F(RemoteFastAudioCaptureSourceAiut, RemoteFastCaptureSource_StartWithoutInit_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<RemoteFastAudioCaptureSource>("test");
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->Start();
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_STARTED,
        "expected ERR_NOT_STARTED for Start without init, got %d", ret);
}

/**
 * @tc.name  : Test RemoteFastAudioCaptureSource Stop without init
 * @tc.number: RemoteFastCaptureSource_StopWithoutInit_001
 * @tc.desc  : Test Stop() without Init() returns SUCCESS.
 */
HWTEST_F(RemoteFastAudioCaptureSourceAiut, RemoteFastCaptureSource_StopWithoutInit_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<RemoteFastAudioCaptureSource>("test");
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->Stop();
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "expected SUCCESS for Stop without init, got %d", ret);
}

/**
 * @tc.name  : Test RemoteFastAudioCaptureSource Resume without init
 * @tc.number: RemoteFastCaptureSource_ResumeWithoutInit_001
 * @tc.desc  : Test Resume() without Init() returns ERR_ILLEGAL_STATE.
 */
HWTEST_F(RemoteFastAudioCaptureSourceAiut, RemoteFastCaptureSource_ResumeWithoutInit_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<RemoteFastAudioCaptureSource>("test");
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->Resume();
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_ILLEGAL_STATE,
        "expected ERR_ILLEGAL_STATE for Resume without init, got %d", ret);
}

/**
 * @tc.name  : Test RemoteFastAudioCaptureSource Pause without init
 * @tc.number: RemoteFastCaptureSource_PauseWithoutInit_001
 * @tc.desc  : Test Pause() without Init() returns ERR_ILLEGAL_STATE.
 */
HWTEST_F(RemoteFastAudioCaptureSourceAiut, RemoteFastCaptureSource_PauseWithoutInit_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<RemoteFastAudioCaptureSource>("test");
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->Pause();
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_ILLEGAL_STATE,
        "expected ERR_ILLEGAL_STATE for Pause without init, got %d", ret);
}

/**
 * @tc.name  : Test RemoteFastAudioCaptureSource UpdateAppsUid
 * @tc.number: RemoteFastCaptureSource_UpdateAppsUid_001
 * @tc.desc  : Test UpdateAppsUid with empty and non-empty uid lists.
 */
HWTEST_F(RemoteFastAudioCaptureSourceAiut, RemoteFastCaptureSource_UpdateAppsUid_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<RemoteFastAudioCaptureSource>("test");
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    std::vector<int32_t> appsUid;
    source->UpdateAppsUid(appsUid);
    appsUid.push_back(20000001);
    appsUid.push_back(20000002);
    source->UpdateAppsUid(appsUid);
    int32_t ret = source->Start();
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_STARTED,
        "expected ERR_NOT_STARTED for Start after UpdateAppsUid, got %d", ret);
    appsUid.clear();
    source->UpdateAppsUid(appsUid);
    ret = source->Stop();
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "expected SUCCESS for Stop, got %d", ret);
}

} // namespace AudioStandard
} // namespace OHOS
