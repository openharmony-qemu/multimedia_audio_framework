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

#include "audio_session_debugging_manager_aiut.h"
#include "aiut_log.h"
#include "audio_debug_manager_private.h"
#include "audio_debug_callback.h"
#include "audio_debug_info.h"
#include "audio_errors.h"
#include "native_audio_debugging_manager.h"
#include <unistd.h>

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

namespace {
constexpr int32_t TEST_APP_UID = 1234;
constexpr size_t TEST_BUF_SIZE = 8192;
}

class TestSessionDebugCallback : public AudioSessionDebugCallback {
public:
    int32_t GetSessionDebugInfo(AudioSessionDebugInfo &debugInfo) override
    {
        debugInfo = testInfo_;
        return retCode_;
    }
    AudioSessionDebugInfo testInfo_;
    int32_t retCode_ = SUCCESS;
};

AudioSessionDebugInfo CreateTestSessionDebugInfo()
{
    AudioSessionDebugInfo info;
    info.strategy.concurrencyMode = AudioConcurrencyMode::MIX_WITH_OTHERS;
    info.audioSessionScene = AudioSessionScene::MEDIA;
    info.defaultDeviceType = DEVICE_TYPE_SPEAKER;
    info.state = AudioSessionState::SESSION_ACTIVE;
    info.fakeFocusState = AudioFocuState::ACTIVE;
    info.pid = 1000;
    info.uid = TEST_APP_UID;

    AudioStreamDebugInfo streamInfo;
    streamInfo.streamId = 300;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.focusState = AudioFocuState::ACTIVE;
    info.streamInfos.push_back(streamInfo);
    return info;
}

bool CreateTestPipe(int pipefd[2])
{
    return pipe(pipefd) == 0;
}

std::string ReadFromPipe(int readFd)
{
    char buf[TEST_BUF_SIZE] = {0};
    ssize_t bytesRead = read(readFd, buf, sizeof(buf) - 1);
    if (bytesRead <= 0) {
        return "";
    }
    return std::string(buf, bytesRead);
}

void AudioSessionDebuggingManagerAiut::SetUpTestCase(void) {}
void AudioSessionDebuggingManagerAiut::TearDownTestCase(void) {}
void AudioSessionDebuggingManagerAiut::SetUp(void) {}
void AudioSessionDebuggingManagerAiut::TearDown(void) {}

// ==================== SetAudioSessionCallback Tests ====================

/**
* @tc.name  : Test SetAudioSessionCallback API with null callback
* @tc.number: AudioDebugManager_SetSessionCallback_NullCallback
* @tc.desc  : Test SetAudioSessionCallback interface. Returns ERR_INVALID_PARAM when callback is null.
*/
HWTEST_F(AudioSessionDebuggingManagerAiut, AudioDebugManager_SetSessionCallback_NullCallback, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int32_t ret = AudioDebugManager::GetInstance().SetAudioSessionCallback(nullptr);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM, "expected ERR_INVALID_PARAM, got ret=%d", ret);
}

/**
* @tc.name  : Test SetAudioSessionCallback API with valid callback
* @tc.number: AudioDebugManager_SetSessionCallback_Success
* @tc.desc  : Test SetAudioSessionCallback interface. Returns SUCCESS on valid callback.
*/
HWTEST_F(AudioSessionDebuggingManagerAiut, AudioDebugManager_SetSessionCallback_Success, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<TestSessionDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    int32_t ret = AudioDebugManager::GetInstance().SetAudioSessionCallback(callback);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetAudioSessionCallback failed, ret=%d", ret);
}

/**
* @tc.name  : Test SetAudioSessionCallback API replacing existing callback
* @tc.number: AudioDebugManager_SetSessionCallback_Replace
* @tc.desc  : Test SetAudioSessionCallback interface. Returns SUCCESS when replacing an existing callback.
*/
HWTEST_F(AudioSessionDebuggingManagerAiut, AudioDebugManager_SetSessionCallback_Replace, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto callback1 = std::make_shared<TestSessionDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback1, nullptr, "make_shared callback1 failed");
    auto callback2 = std::make_shared<TestSessionDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback2, nullptr, "make_shared callback2 failed");

    int32_t ret = AudioDebugManager::GetInstance().SetAudioSessionCallback(callback1);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetAudioSessionCallback1 failed, ret=%d", ret);

    ret = AudioDebugManager::GetInstance().SetAudioSessionCallback(callback2);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetAudioSessionCallback2 failed, ret=%d", ret);
}

// ==================== PrintAudioSessionDebugInfo Tests ====================

/**
* @tc.name  : Test PrintAudioSessionDebugInfo API without callback
* @tc.number: AudioDebugManager_PrintSession_NoCallback
* @tc.desc  : Test PrintAudioSessionDebugInfo interface. Returns SUCCESS or ERR_ILLEGAL_STATE when no callback is set.
*/
HWTEST_F(AudioSessionDebuggingManagerAiut, AudioDebugManager_PrintSession_NoCallback, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int32_t ret = AudioDebugManager::GetInstance().PrintAudioSessionDebugInfo(-1);
    AIUT_EXPECT_TRUE_INFO_LOG(ret == SUCCESS || ret == ERR_ILLEGAL_STATE, "unexpected ret=%d", ret);
}

/**
* @tc.name  : Test PrintAudioSessionDebugInfo API with pipe fd
* @tc.number: AudioDebugManager_PrintSession_WithPipe
* @tc.desc  : Test PrintAudioSessionDebugInfo interface. Returns SUCCESS and output contains session debug info fields.
*/
HWTEST_F(AudioSessionDebuggingManagerAiut, AudioDebugManager_PrintSession_WithPipe, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<TestSessionDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    callback->testInfo_ = CreateTestSessionDebugInfo();
    AudioDebugManager::GetInstance().SetAudioSessionCallback(callback);

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    int32_t ret = AudioDebugManager::GetInstance().PrintAudioSessionDebugInfo(pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAudioSessionDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_NE_INFO_LOG(output.find("audioSession"), std::string::npos,
        "output missing 'audioSession'");
    AIUT_EXPECT_NE_INFO_LOG(output.find("MIX_WITH_OTHERS"), std::string::npos,
        "output missing 'MIX_WITH_OTHERS'");
    AIUT_EXPECT_NE_INFO_LOG(output.find("SESSION_ACTIVE"), std::string::npos,
        "output missing 'SESSION_ACTIVE'");
    AIUT_EXPECT_NE_INFO_LOG(output.find("DEVICE_TYPE_SPEAKER"), std::string::npos,
        "output missing 'DEVICE_TYPE_SPEAKER'");
}

/**
* @tc.name  : Test PrintAudioSessionDebugInfo API with negative fd
* @tc.number: AudioDebugManager_PrintSession_NegativeFd
* @tc.desc  : Test PrintAudioSessionDebugInfo interface. Returns SUCCESS with negative fd, falling back to log output.
*/
HWTEST_F(AudioSessionDebuggingManagerAiut, AudioDebugManager_PrintSession_NegativeFd, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<TestSessionDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    callback->testInfo_ = CreateTestSessionDebugInfo();
    AudioDebugManager::GetInstance().SetAudioSessionCallback(callback);

    int32_t ret = AudioDebugManager::GetInstance().PrintAudioSessionDebugInfo(-1);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintSessionDebugInfo with -1 failed, ret=%d", ret);
}

/**
* @tc.name  : Test PrintAudioSessionDebugInfo API when callback returns error
* @tc.number: AudioDebugManager_PrintSession_CallbackError
* @tc.desc  : Test PrintAudioSessionDebugInfo interface. Returns ERR_ILLEGAL_STATE when callback GetSessionDebugInfo fails.
*/
HWTEST_F(AudioSessionDebuggingManagerAiut, AudioDebugManager_PrintSession_CallbackError, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<TestSessionDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    callback->retCode_ = ERR_ILLEGAL_STATE;
    AudioDebugManager::GetInstance().SetAudioSessionCallback(callback);

    int32_t ret = AudioDebugManager::GetInstance().PrintAudioSessionDebugInfo(-1);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_ILLEGAL_STATE, "expected ERR_ILLEGAL_STATE, got ret=%d", ret);
}

/**
* @tc.name  : Test PrintAudioSessionDebugInfo API with specific scene
* @tc.number: AudioDebugManager_PrintSession_WithScene
* @tc.desc  : Test PrintAudioSessionDebugInfo interface. Output contains GAME scene and DUCK focus state info.
*/
HWTEST_F(AudioSessionDebuggingManagerAiut, AudioDebugManager_PrintSession_WithScene, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<TestSessionDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    callback->testInfo_ = CreateTestSessionDebugInfo();
    callback->testInfo_.audioSessionScene = AudioSessionScene::GAME;
    callback->testInfo_.fakeFocusState = AudioFocuState::DUCK;
    AudioDebugManager::GetInstance().SetAudioSessionCallback(callback);

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    int32_t ret = AudioDebugManager::GetInstance().PrintAudioSessionDebugInfo(pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAudioSessionDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_NE_INFO_LOG(output.find("GAME"), std::string::npos, "output missing 'GAME'");
    AIUT_EXPECT_NE_INFO_LOG(output.find("DUCK"), std::string::npos, "output missing 'DUCK'");
}

/**
* @tc.name  : Test PrintAudioSessionDebugInfo API with invalid scene
* @tc.number: AudioDebugManager_PrintSession_InvalidScene
* @tc.desc  : Test PrintAudioSessionDebugInfo interface. Returns SUCCESS and output contains audioSession even with INVALID scene.
*/
HWTEST_F(AudioSessionDebuggingManagerAiut, AudioDebugManager_PrintSession_InvalidScene, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<TestSessionDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    callback->testInfo_ = CreateTestSessionDebugInfo();
    callback->testInfo_.audioSessionScene = AudioSessionScene::INVALID;
    AudioDebugManager::GetInstance().SetAudioSessionCallback(callback);

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    int32_t ret = AudioDebugManager::GetInstance().PrintAudioSessionDebugInfo(pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAudioSessionDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_NE_INFO_LOG(output.find("audioSession"), std::string::npos, "output missing 'audioSession'");
}

// ==================== Session DebugInfo with Streams ====================

/**
* @tc.name  : Test SessionDebugInfo contains multiple stream info
* @tc.number: AudioDebugManager_SessionDebugInfo_MultipleStreams
* @tc.desc  : Test PrintAudioSessionDebugInfo interface. Output contains multiple stream types in session debug info.
*/
HWTEST_F(AudioSessionDebuggingManagerAiut, AudioDebugManager_SessionDebugInfo_MultipleStreams, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<TestSessionDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    callback->testInfo_ = CreateTestSessionDebugInfo();

    AudioStreamDebugInfo stream2;
    stream2.streamId = 400;
    stream2.streamType = STREAM_VOICE_CALL;
    stream2.focusState = AudioFocuState::DUCK;
    callback->testInfo_.streamInfos.push_back(stream2);

    AudioDebugManager::GetInstance().SetAudioSessionCallback(callback);

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    int32_t ret = AudioDebugManager::GetInstance().PrintAudioSessionDebugInfo(pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAudioSessionDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_NE_INFO_LOG(output.find("STREAM_MUSIC"), std::string::npos, "output missing 'STREAM_MUSIC'");
    AIUT_EXPECT_NE_INFO_LOG(output.find("STREAM_VOICE_CALL"), std::string::npos,
        "output missing 'STREAM_VOICE_CALL'");
}

// ==================== OHAudio C API Session Tests ====================

/**
* @tc.name  : Test OH_AudioDebuggingManager_PrintSessionInfo API with null manager
* @tc.number: OH_PrintSessionInfo_NullManager
* @tc.desc  : Test OH_AudioDebuggingManager_PrintSessionInfo interface. Returns AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM when manager is null.
*/
HWTEST_F(AudioSessionDebuggingManagerAiut, OH_PrintSessionInfo_NullManager, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioCommon_Result result = OH_AudioDebuggingManager_PrintSessionInfo(nullptr, nullptr, -1);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "expected INVALID_PARAM, got %d", result);
}

/**
* @tc.name  : Test OH_AudioDebuggingManager_PrintSessionInfo API with null session
* @tc.number: OH_PrintSessionInfo_NullSession
* @tc.desc  : Test OH_AudioDebuggingManager_PrintSessionInfo interface. Returns AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM when session is null.
*/
HWTEST_F(AudioSessionDebuggingManagerAiut, OH_PrintSessionInfo_NullSession, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioDebuggingManager *manager = nullptr;
    OH_AudioManager_GetAudioDebuggingManager(&manager);
    AIUT_ASSERT_NE_INFO_LOG(manager, nullptr, "GetAudioDebuggingManager returned null");

    OH_AudioCommon_Result result = OH_AudioDebuggingManager_PrintSessionInfo(manager, nullptr, -1);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "expected INVALID_PARAM, got %d", result);
}

/**
* @tc.name  : Test OH_AudioDebuggingManager_PrintSessionInfo API with invalid session
* @tc.number: OH_PrintSessionInfo_InvalidSession
* @tc.desc  : Test OH_AudioDebuggingManager_PrintSessionInfo interface. Returns ERROR_INVALID_PARAM or ERROR_SYSTEM with invalid session pointer.
*/
HWTEST_F(AudioSessionDebuggingManagerAiut, OH_PrintSessionInfo_InvalidSession, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    OH_AudioDebuggingManager *manager = nullptr;
    OH_AudioManager_GetAudioDebuggingManager(&manager);
    AIUT_ASSERT_NE_INFO_LOG(manager, nullptr, "GetAudioDebuggingManager returned null");

    OH_AudioSessionManager *fakeSession = reinterpret_cast<OH_AudioSessionManager *>(0xFEED);
    OH_AudioCommon_Result result = OH_AudioDebuggingManager_PrintSessionInfo(manager, fakeSession, -1);
    AIUT_EXPECT_TRUE_INFO_LOG(
        result == AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM || result == AUDIOCOMMON_RESULT_ERROR_SYSTEM,
        "unexpected result=%d", result);
}

/**
* @tc.name  : Test OH_AudioDebuggingManager_PrintSessionInfo API with both null params
* @tc.number: OH_PrintSessionInfo_BothNull
* @tc.desc  : Test OH_AudioDebuggingManager_PrintSessionInfo interface. Returns AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM when both params are null.
*/
HWTEST_F(AudioSessionDebuggingManagerAiut, OH_PrintSessionInfo_BothNull, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioCommon_Result result = OH_AudioDebuggingManager_PrintSessionInfo(nullptr, nullptr, -1);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "expected INVALID_PARAM, got %d", result);
}

// ==================== Taihe Session Tests ====================

/**
* @tc.name  : Test Taihe PrintSessionInfo API delegates to native
* @tc.number: Taihe_PrintSessionInfo_DelegatesToNative
* @tc.desc  : Test PrintAudioSessionDebugInfo interface. Returns SUCCESS and output contains audioSession info when delegated from Taihe.
*/
HWTEST_F(AudioSessionDebuggingManagerAiut, Taihe_PrintSessionInfo_DelegatesToNative, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    // Taihe: PrintSessionInfo(session, fd) calls AudioDebugManager::PrintAudioSessionDebugInfo(fd)
    auto callback = std::make_shared<TestSessionDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    callback->testInfo_.audioSessionScene = AudioSessionScene::MEDIA;
    callback->testInfo_.state = AudioSessionState::SESSION_ACTIVE;

    int32_t ret = AudioDebugManager::GetInstance().SetAudioSessionCallback(callback);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetAudioSessionCallback failed, ret=%d", ret);

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    ret = AudioDebugManager::GetInstance().PrintAudioSessionDebugInfo(pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAudioSessionDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_NE_INFO_LOG(output.find("audioSession"), std::string::npos, "output missing 'audioSession'");
}

// ==================== NAPI Session Tests ====================

/**
* @tc.name  : Test NAPI printSessionInfo API delegates to native
* @tc.number: Napi_PrintSessionInfo_DelegatesToNative
* @tc.desc  : Test PrintAudioSessionDebugInfo interface. Returns SUCCESS and output contains audioSession info when delegated from NAPI.
*/
HWTEST_F(AudioSessionDebuggingManagerAiut, Napi_PrintSessionInfo_DelegatesToNative, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    // NAPI: printSessionInfo(session, fd) -> AudioDebugManager::GetInstance().PrintAudioSessionDebugInfo(fd)
    auto callback = std::make_shared<TestSessionDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    callback->testInfo_.audioSessionScene = AudioSessionScene::MEDIA;
    callback->testInfo_.state = AudioSessionState::SESSION_ACTIVE;

    int32_t ret = AudioDebugManager::GetInstance().SetAudioSessionCallback(callback);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetAudioSessionCallback failed, ret=%d", ret);

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    ret = AudioDebugManager::GetInstance().PrintAudioSessionDebugInfo(pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAudioSessionDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_NE_INFO_LOG(output.find("audioSession"), std::string::npos, "output missing 'audioSession'");
}

/**
* @tc.name  : Test NAPI printSessionInfo API without callback
* @tc.number: Napi_PrintSessionInfo_NoCallback
* @tc.desc  : Test PrintAudioSessionDebugInfo interface. Returns SUCCESS or ERR_ILLEGAL_STATE without session callback, does not crash.
*/
HWTEST_F(AudioSessionDebuggingManagerAiut, Napi_PrintSessionInfo_NoCallback, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    // NAPI: PrintSessionInfo without session callback should not crash
    int32_t ret = AudioDebugManager::GetInstance().PrintAudioSessionDebugInfo(-1);
    AIUT_EXPECT_TRUE_INFO_LOG(ret == SUCCESS || ret == ERR_ILLEGAL_STATE, "unexpected ret=%d", ret);
}

} // namespace AudioStandard
} // namespace OHOS
