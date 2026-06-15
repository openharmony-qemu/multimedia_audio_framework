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

#include "remote_audio_capture_source_aiut.h"
#include "aiut_log.h"
#include "source/remote_audio_capture_source.h"
#include "source/i_audio_capture_source.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

namespace {
constexpr uint32_t TEST_CAPTURE_ID = 0;
} // namespace

// ==================== RemoteAudioCaptureSourceAiut ====================

void RemoteAudioCaptureSourceAiut::SetUpTestCase(void) {}
void RemoteAudioCaptureSourceAiut::TearDownTestCase(void) {}
void RemoteAudioCaptureSourceAiut::SetUp(void) {}
void RemoteAudioCaptureSourceAiut::TearDown(void) {}

// ==================== Construction / Destruction Tests ====================

/**
 * @tc.name  : Test RemoteAudioCaptureSource construction
 * @tc.number: RemoteCaptureSource_Constructor_001
 * @tc.desc  : Test RemoteAudioCaptureSource constructor creates uninitialized object.
 */
HWTEST_F(RemoteAudioCaptureSourceAiut, RemoteCaptureSource_Constructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<RemoteAudioCaptureSource>(TEST_CAPTURE_ID, "test");
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    AIUT_EXPECT_FALSE_INFO_LOG(source->IsInited(), "newly constructed source should not be inited");
}

/**
 * @tc.name  : Test RemoteAudioCaptureSource destruction
 * @tc.number: RemoteCaptureSource_Destructor_001
 * @tc.desc  : Test destructor on uninitialized source does not crash.
 */
HWTEST_F(RemoteAudioCaptureSourceAiut, RemoteCaptureSource_Destructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<RemoteAudioCaptureSource>(TEST_CAPTURE_ID, "test");
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    source = nullptr;
    AIUT_EXPECT_EQ_INFO_LOG(source, nullptr, "source should be null after reset");
}

// ==================== SetInvalidState Tests ====================

/**
 * @tc.name  : Test RemoteAudioCaptureSource SetInvalidState and Start
 * @tc.number: RemoteCaptureSource_SetInvalidState_001
 * @tc.desc  : Test SetInvalidState followed by Start returns ERR_NOT_STARTED.
 */
HWTEST_F(RemoteAudioCaptureSourceAiut, RemoteCaptureSource_SetInvalidState_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<RemoteAudioCaptureSource>(TEST_CAPTURE_ID, "test");
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    source->SetInvalidState();
    source->sourceInited_.store(true);
    int32_t ret = source->Start();
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_STARTED,
        "expected ERR_NOT_STARTED after SetInvalidState, got %d", ret);

    source->captureInited_.store(true);
    ret = source->Start();
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_STARTED,
        "expected ERR_NOT_STARTED with captureInited only, got %d", ret);

    source->validState_.store(true);
    ret = source->Start();
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_HANDLE,
        "expected ERR_INVALID_HANDLE with all states true but no capture, got %d", ret);
}

// ==================== Lifecycle Tests ====================

/**
 * @tc.name  : Test RemoteAudioCaptureSource Start without init
 * @tc.number: RemoteCaptureSource_StartWithoutInit_001
 * @tc.desc  : Test Start() before Init() returns ERR_NOT_STARTED.
 */
HWTEST_F(RemoteAudioCaptureSourceAiut, RemoteCaptureSource_StartWithoutInit_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<RemoteAudioCaptureSource>(TEST_CAPTURE_ID, "test");
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->Start();
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_STARTED,
        "expected ERR_NOT_STARTED for Start without init, got %d", ret);
}

/**
 * @tc.name  : Test RemoteAudioCaptureSource Stop without init
 * @tc.number: RemoteCaptureSource_StopWithoutInit_001
 * @tc.desc  : Test Stop() without Init() returns SUCCESS.
 */
HWTEST_F(RemoteAudioCaptureSourceAiut, RemoteCaptureSource_StopWithoutInit_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<RemoteAudioCaptureSource>(TEST_CAPTURE_ID, "test");
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->Stop();
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "expected SUCCESS for Stop without init, got %d", ret);
}

/**
 * @tc.name  : Test RemoteAudioCaptureSource Resume without init
 * @tc.number: RemoteCaptureSource_ResumeWithoutInit_001
 * @tc.desc  : Test Resume() without Init() returns ERR_ILLEGAL_STATE.
 */
HWTEST_F(RemoteAudioCaptureSourceAiut, RemoteCaptureSource_ResumeWithoutInit_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<RemoteAudioCaptureSource>(TEST_CAPTURE_ID, "test");
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->Resume();
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_ILLEGAL_STATE,
        "expected ERR_ILLEGAL_STATE for Resume without init, got %d", ret);
}

/**
 * @tc.name  : Test RemoteAudioCaptureSource Pause without init
 * @tc.number: RemoteCaptureSource_PauseWithoutInit_001
 * @tc.desc  : Test Pause() without Init() returns ERR_ILLEGAL_STATE.
 */
HWTEST_F(RemoteAudioCaptureSourceAiut, RemoteCaptureSource_PauseWithoutInit_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<RemoteAudioCaptureSource>(TEST_CAPTURE_ID, "test");
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->Pause();
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_ILLEGAL_STATE,
        "expected ERR_ILLEGAL_STATE for Pause without init, got %d", ret);
}

/**
 * @tc.name  : Test RemoteAudioCaptureSource CaptureFrame without init
 * @tc.number: RemoteCaptureSource_CaptureFrameWithoutInit_001
 * @tc.desc  : Test CaptureFrame() without Init() returns ERR_INVALID_HANDLE.
 */
HWTEST_F(RemoteAudioCaptureSourceAiut, RemoteCaptureSource_CaptureFrameWithoutInit_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<RemoteAudioCaptureSource>(TEST_CAPTURE_ID, "test");
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    char *buffer = nullptr;
    uint64_t requestBytes = 0;
    uint64_t replyBytes = 0;
    int32_t ret = source->CaptureFrame(buffer, requestBytes, replyBytes);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_HANDLE,
        "expected ERR_INVALID_HANDLE for CaptureFrame without init, got %d", ret);
}

/**
 * @tc.name  : Test RemoteAudioCaptureSource SetVolume without init
 * @tc.number: RemoteCaptureSource_SetVolumeWithoutInit_001
 * @tc.desc  : Test SetVolume() without Init() returns ERR_INVALID_HANDLE.
 */
HWTEST_F(RemoteAudioCaptureSourceAiut, RemoteCaptureSource_SetVolumeWithoutInit_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<RemoteAudioCaptureSource>(TEST_CAPTURE_ID, "test");
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->SetVolume(1.0f, 1.0f);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_HANDLE,
        "expected ERR_INVALID_HANDLE for SetVolume without init, got %d", ret);
}

/**
 * @tc.name  : Test RemoteAudioCaptureSource SetMute without init
 * @tc.number: RemoteCaptureSource_SetMuteWithoutInit_001
 * @tc.desc  : Test SetMute() without Init() returns ERR_INVALID_HANDLE.
 */
HWTEST_F(RemoteAudioCaptureSourceAiut, RemoteCaptureSource_SetMuteWithoutInit_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<RemoteAudioCaptureSource>(TEST_CAPTURE_ID, "test");
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->SetMute(false);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_HANDLE,
        "expected ERR_INVALID_HANDLE for SetMute without init, got %d", ret);
}

/**
 * @tc.name  : Test RemoteAudioCaptureSource SetAudioScene without init
 * @tc.number: RemoteCaptureSource_SetAudioSceneWithoutInit_001
 * @tc.desc  : Test SetAudioScene() without Init() returns ERR_INVALID_HANDLE.
 */
HWTEST_F(RemoteAudioCaptureSourceAiut, RemoteCaptureSource_SetAudioSceneWithoutInit_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<RemoteAudioCaptureSource>(TEST_CAPTURE_ID, "test");
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->SetAudioScene(AUDIO_SCENE_DEFAULT);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_HANDLE,
        "expected ERR_INVALID_HANDLE for SetAudioScene without init, got %d", ret);
}

/**
 * @tc.name  : Test RemoteAudioCaptureSource UpdateActiveDevice
 * @tc.number: RemoteCaptureSource_UpdateActiveDevice_001
 * @tc.desc  : Test UpdateActiveDevice returns ERR_NOT_SUPPORTED.
 */
HWTEST_F(RemoteAudioCaptureSourceAiut, RemoteCaptureSource_UpdateActiveDevice_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<RemoteAudioCaptureSource>(TEST_CAPTURE_ID, "test");
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->UpdateActiveDevice(DEVICE_TYPE_SPEAKER);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED,
        "expected ERR_NOT_SUPPORTED for UpdateActiveDevice, got %d", ret);
}

/**
 * @tc.name  : Test RemoteAudioCaptureSource UpdateAppsUid
 * @tc.number: RemoteCaptureSource_UpdateAppsUid_001
 * @tc.desc  : Test UpdateAppsUid with empty and non-empty uid lists.
 */
HWTEST_F(RemoteAudioCaptureSourceAiut, RemoteCaptureSource_UpdateAppsUid_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<RemoteAudioCaptureSource>(TEST_CAPTURE_ID, "test");
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
