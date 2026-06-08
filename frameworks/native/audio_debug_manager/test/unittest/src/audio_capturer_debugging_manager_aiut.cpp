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

#include "audio_capturer_debugging_manager_aiut.h"
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
constexpr uint32_t TEST_SESSION_ID = 200;
constexpr uint32_t TEST_CAPTURER_SAMPLING_RATE = 16000;
constexpr size_t TEST_CAPTURER_BUFFER_SIZE = 4096;
}

class TestCapturerDebugCallback : public AudioCapturerDebugCallback {
public:
    int32_t GetCapturerDebugInfo(AudioCapturerDebugInfo &debugInfo) override
    {
        debugInfo = testInfo_;
        return retCode_;
    }
    AudioCapturerDebugInfo testInfo_;
    int32_t retCode_ = SUCCESS;
};

AudioCapturerDebugInfo CreateTestCapturerDebugInfo()
{
    AudioCapturerDebugInfo info;
    info.sessionId = TEST_SESSION_ID + 1;
    info.samplingRate = TEST_CAPTURER_SAMPLING_RATE;
    info.channels = AudioChannel::CHANNEL_1;
    info.format = AudioSampleFormat::SAMPLE_S16LE;
    info.encoding = AudioEncodingType::ENCODING_PCM;
    info.channelLayout = AudioChannelLayout::CH_LAYOUT_MONO;
    info.sourceType = SOURCE_TYPE_MIC;
    info.capturerFlag = AUDIO_FLAG_NORMAL;
    info.bufferSize = TEST_CAPTURER_BUFFER_SIZE;
    info.overflowCount = 0;
    info.muteWhenInterrupted = false;
    info.pauseWhenInterrupted = false;
    info.pipeRole = PIPE_ROLE_INPUT;
    info.focusState = AudioFocuState::ACTIVE;
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

void AudioCapturerDebuggingManagerAiut::SetUpTestCase(void) {}
void AudioCapturerDebuggingManagerAiut::TearDownTestCase(void) {}
void AudioCapturerDebuggingManagerAiut::SetUp(void) {}
void AudioCapturerDebuggingManagerAiut::TearDown(void) {}

// ==================== RegisterAudioCapturer Tests ====================

/**
* @tc.name  : Test RegisterAudioCapturer API with null callback
* @tc.number: AudioDebugManager_RegisterCapturer_NullCallback
* @tc.desc  : Test RegisterAudioCapturer interface. Returns ERR_INVALID_PARAM when callback is null.
*/
HWTEST_F(AudioCapturerDebuggingManagerAiut, AudioDebugManager_RegisterCapturer_NullCallback, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    uint32_t key = 0;
    int32_t ret = AudioDebugManager::GetInstance().RegisterAudioCapturer(key, nullptr);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM, "expected ERR_INVALID_PARAM, got ret=%d", ret);
}

/**
* @tc.name  : Test RegisterAudioCapturer API with valid callback
* @tc.number: AudioDebugManager_RegisterCapturer_Success
* @tc.desc  : Test RegisterAudioCapturer interface. Returns SUCCESS and generates non-zero key on success.
*/
HWTEST_F(AudioCapturerDebuggingManagerAiut, AudioDebugManager_RegisterCapturer_Success, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<TestCapturerDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    uint32_t key = 0;
    int32_t ret = AudioDebugManager::GetInstance().RegisterAudioCapturer(key, callback);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "RegisterAudioCapturer failed, ret=%d", ret);
    AIUT_EXPECT_NE_INFO_LOG(key, 0u, "key not generated");

    AudioDebugManager::GetInstance().UnregisterAudioCapturer(key);
}

// ==================== UnregisterAudioCapturer Tests ====================

/**
* @tc.name  : Test UnregisterAudioCapturer API with non-existent key
* @tc.number: AudioDebugManager_UnregisterCapturer_NotFound
* @tc.desc  : Test UnregisterAudioCapturer interface. Returns ERR_ILLEGAL_STATE when key is not found.
*/
HWTEST_F(AudioCapturerDebuggingManagerAiut, AudioDebugManager_UnregisterCapturer_NotFound, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int32_t ret = AudioDebugManager::GetInstance().UnregisterAudioCapturer(99999);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_ILLEGAL_STATE, "expected ERR_ILLEGAL_STATE, got ret=%d", ret);
}

/**
* @tc.name  : Test UnregisterAudioCapturer API with valid key
* @tc.number: AudioDebugManager_UnregisterCapturer_Success
* @tc.desc  : Test UnregisterAudioCapturer interface. Returns SUCCESS when unregistering a registered capturer.
*/
HWTEST_F(AudioCapturerDebuggingManagerAiut, AudioDebugManager_UnregisterCapturer_Success, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<TestCapturerDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    uint32_t key = 0;
    AudioDebugManager::GetInstance().RegisterAudioCapturer(key, callback);

    int32_t ret = AudioDebugManager::GetInstance().UnregisterAudioCapturer(key);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "UnregisterAudioCapturer failed, ret=%d", ret);
}

// ==================== PrintAudioCapturerDebugInfo Tests ====================

/**
* @tc.name  : Test PrintAudioCapturerDebugInfo API with invalid key
* @tc.number: AudioDebugManager_PrintCapturer_InvalidKey
* @tc.desc  : Test PrintAudioCapturerDebugInfo interface. Returns ERR_ILLEGAL_STATE when key is not registered.
*/
HWTEST_F(AudioCapturerDebuggingManagerAiut, AudioDebugManager_PrintCapturer_InvalidKey, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int32_t ret = AudioDebugManager::GetInstance().PrintAudioCapturerDebugInfo(99999, -1);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_ILLEGAL_STATE, "expected ERR_ILLEGAL_STATE, got ret=%d", ret);
}

/**
* @tc.name  : Test PrintAudioCapturerDebugInfo API with pipe fd
* @tc.number: AudioDebugManager_PrintCapturer_WithPipe
* @tc.desc  : Test PrintAudioCapturerDebugInfo interface.
*             Returns SUCCESS and output contains capturer debug info fields.
*/
HWTEST_F(AudioCapturerDebuggingManagerAiut, AudioDebugManager_PrintCapturer_WithPipe, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<TestCapturerDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    callback->testInfo_ = CreateTestCapturerDebugInfo();

    uint32_t key = 0;
    int32_t ret = AudioDebugManager::GetInstance().RegisterAudioCapturer(key, callback);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "RegisterAudioCapturer failed, ret=%d", ret);

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    ret = AudioDebugManager::GetInstance().PrintAudioCapturerDebugInfo(key, pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAudioCapturerDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_NE_INFO_LOG(output.find("audioCapturer"), std::string::npos,
        "output missing 'audioCapturer'");
    AIUT_EXPECT_NE_INFO_LOG(output.find("SOURCE_TYPE_MIC"), std::string::npos,
        "output missing 'SOURCE_TYPE_MIC'");
    AIUT_EXPECT_NE_INFO_LOG(output.find("PIPE_ROLE_INPUT"), std::string::npos,
        "output missing 'PIPE_ROLE_INPUT'");

    AudioDebugManager::GetInstance().UnregisterAudioCapturer(key);
}

/**
* @tc.name  : Test PrintAudioCapturerDebugInfo API with negative fd
* @tc.number: AudioDebugManager_PrintCapturer_NegativeFd
* @tc.desc  : Test PrintAudioCapturerDebugInfo interface. Returns SUCCESS with negative fd, falling back to log output.
*/
HWTEST_F(AudioCapturerDebuggingManagerAiut, AudioDebugManager_PrintCapturer_NegativeFd, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<TestCapturerDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    callback->testInfo_ = CreateTestCapturerDebugInfo();

    uint32_t key = 0;
    AudioDebugManager::GetInstance().RegisterAudioCapturer(key, callback);

    int32_t ret = AudioDebugManager::GetInstance().PrintAudioCapturerDebugInfo(key, -1);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintCapturerDebugInfo with -1 failed, ret=%d", ret);

    AudioDebugManager::GetInstance().UnregisterAudioCapturer(key);
}

/**
* @tc.name  : Test PrintAudioCapturerDebugInfo API when callback returns error
* @tc.number: AudioDebugManager_PrintCapturer_CallbackError
* @tc.desc  : Test PrintAudioCapturerDebugInfo interface.
*             Returns ERR_ILLEGAL_STATE when callback GetCapturerDebugInfo fails.
*/
HWTEST_F(AudioCapturerDebuggingManagerAiut, AudioDebugManager_PrintCapturer_CallbackError, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<TestCapturerDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    callback->retCode_ = ERR_ILLEGAL_STATE;

    uint32_t key = 0;
    AudioDebugManager::GetInstance().RegisterAudioCapturer(key, callback);

    int32_t ret = AudioDebugManager::GetInstance().PrintAudioCapturerDebugInfo(key, -1);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_ILLEGAL_STATE, "expected ERR_ILLEGAL_STATE, got ret=%d", ret);

    AudioDebugManager::GetInstance().UnregisterAudioCapturer(key);
}

// ==================== PrintAllAudioCapturersDebugInfo Tests ====================

/**
* @tc.name  : Test PrintAllAudioCapturersDebugInfo API with no capturers
* @tc.number: AudioDebugManager_PrintAllCapturers_NoCapturers
* @tc.desc  : Test PrintAllAudioCapturersDebugInfo interface. Returns SUCCESS when no capturers are registered.
*/
HWTEST_F(AudioCapturerDebuggingManagerAiut, AudioDebugManager_PrintAllCapturers_NoCapturers, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    int32_t ret = AudioDebugManager::GetInstance().PrintAllAudioCapturersDebugInfo(pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAllCapturersDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);
    close(pipefd[0]);
}

/**
* @tc.name  : Test PrintAllAudioCapturersDebugInfo API with registered callback
* @tc.number: AudioDebugManager_PrintAllCapturers_WithCallback
* @tc.desc  : Test PrintAllAudioCapturersDebugInfo interface. Returns SUCCESS and output contains audioCapturer info.
*/
HWTEST_F(AudioCapturerDebuggingManagerAiut, AudioDebugManager_PrintAllCapturers_WithCallback, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<TestCapturerDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    callback->testInfo_ = CreateTestCapturerDebugInfo();

    uint32_t key = 0;
    AudioDebugManager::GetInstance().RegisterAudioCapturer(key, callback);

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    int32_t ret = AudioDebugManager::GetInstance().PrintAllAudioCapturersDebugInfo(pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAllCapturersDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_NE_INFO_LOG(output.find("audioCapturer"), std::string::npos, "output missing 'audioCapturer'");

    AudioDebugManager::GetInstance().UnregisterAudioCapturer(key);
}

// ==================== Capturer DebugInfo Content Verification ====================

/**
* @tc.name  : Test CapturerDebugInfo contains muteWhenInterrupted and pauseWhenInterrupted fields
* @tc.number: AudioDebugManager_CapturerDebugInfo_MuteWhenInterrupted
* @tc.desc  : Test PrintAudioCapturerDebugInfo interface.
*             Output contains true values for muteWhenInterrupted and pauseWhenInterrupted.
*/
HWTEST_F(AudioCapturerDebuggingManagerAiut, AudioDebugManager_CapturerDebugInfo_MuteWhenInterrupted, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<TestCapturerDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    callback->testInfo_ = CreateTestCapturerDebugInfo();
    callback->testInfo_.muteWhenInterrupted = true;
    callback->testInfo_.pauseWhenInterrupted = true;

    uint32_t key = 0;
    AudioDebugManager::GetInstance().RegisterAudioCapturer(key, callback);

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    int32_t ret = AudioDebugManager::GetInstance().PrintAudioCapturerDebugInfo(key, pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAudioCapturerDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_NE_INFO_LOG(output.find("true"), std::string::npos, "output missing 'true'");

    AudioDebugManager::GetInstance().UnregisterAudioCapturer(key);
}

/**
* @tc.name  : Test CapturerDebugInfo contains focusHistory field
* @tc.number: AudioDebugManager_CapturerDebugInfo_FocusHistory
* @tc.desc  : Test PrintAudioCapturerDebugInfo interface. Output contains focusHistory and INTERRUPT_HINT_DUCK info.
*/
HWTEST_F(AudioCapturerDebuggingManagerAiut, AudioDebugManager_CapturerDebugInfo_FocusHistory, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<TestCapturerDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    callback->testInfo_ = CreateTestCapturerDebugInfo();

    AudioFocusBehavior behavior;
    behavior.streamId = 600;
    behavior.pid = 200;
    behavior.uid = TEST_APP_UID;
    behavior.streamType = STREAM_VOICE_CALL;
    behavior.sourceType = SOURCE_TYPE_VOICE_CALL;
    behavior.hintType = INTERRUPT_HINT_DUCK;
    behavior.timestamp = 1111111111;
    callback->testInfo_.focusHistory.push_back(behavior);

    uint32_t key = 0;
    AudioDebugManager::GetInstance().RegisterAudioCapturer(key, callback);

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    int32_t ret = AudioDebugManager::GetInstance().PrintAudioCapturerDebugInfo(key, pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAudioCapturerDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_NE_INFO_LOG(output.find("INTERRUPT_HINT_DUCK"), std::string::npos,
        "output missing 'INTERRUPT_HINT_DUCK'");

    AudioDebugManager::GetInstance().UnregisterAudioCapturer(key);
}

// ==================== Capturer Re-Register ====================

/**
* @tc.name  : Test re-registering a capturer after unregister
* @tc.number: AudioDebugManager_Capturer_ReRegister
* @tc.desc  : Test RegisterAudioCapturer interface.
*             Successfully re-registers after unregistering, with a new unique key.
*/
HWTEST_F(AudioCapturerDebuggingManagerAiut, AudioDebugManager_Capturer_ReRegister, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto callback1 = std::make_shared<TestCapturerDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback1, nullptr, "make_shared callback1 failed");
    uint32_t key1 = 0;
    AudioDebugManager::GetInstance().RegisterAudioCapturer(key1, callback1);
    AudioDebugManager::GetInstance().UnregisterAudioCapturer(key1);

    auto callback2 = std::make_shared<TestCapturerDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback2, nullptr, "make_shared callback2 failed");
    uint32_t key2 = 0;
    int32_t ret = AudioDebugManager::GetInstance().RegisterAudioCapturer(key2, callback2);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "Re-register failed, ret=%d", ret);
    AIUT_EXPECT_NE_INFO_LOG(key2, 0u, "key2 not generated");

    AudioDebugManager::GetInstance().UnregisterAudioCapturer(key2);
}

// ==================== OHAudio C API Capturer Tests ====================

/**
* @tc.name  : Test OH_AudioDebuggingManager_PrintCapturerInfo API with null manager
* @tc.number: OH_PrintCapturerInfo_NullManager
* @tc.desc  : Test OH_AudioDebuggingManager_PrintCapturerInfo interface.
*             Returns AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM when manager is null.
*/
HWTEST_F(AudioCapturerDebuggingManagerAiut, OH_PrintCapturerInfo_NullManager, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioCommon_Result result = OH_AudioDebuggingManager_PrintCapturerInfo(nullptr, nullptr, -1);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "expected INVALID_PARAM, got %d", result);
}

/**
* @tc.name  : Test OH_AudioDebuggingManager_PrintCapturerInfo API with null capturer
* @tc.number: OH_PrintCapturerInfo_NullCapturer
* @tc.desc  : Test OH_AudioDebuggingManager_PrintCapturerInfo interface.
*             Returns AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM when capturer is null.
*/
HWTEST_F(AudioCapturerDebuggingManagerAiut, OH_PrintCapturerInfo_NullCapturer, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioDebuggingManager *manager = nullptr;
    OH_AudioManager_GetAudioDebuggingManager(&manager);
    AIUT_ASSERT_NE_INFO_LOG(manager, nullptr, "GetAudioDebuggingManager returned null");

    OH_AudioCommon_Result result = OH_AudioDebuggingManager_PrintCapturerInfo(manager, nullptr, -1);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "expected INVALID_PARAM, got %d", result);
}

/**
* @tc.name  : Test OH_AudioDebuggingManager_PrintCapturerInfo API with invalid capturer
* @tc.number: OH_PrintCapturerInfo_InvalidCapturer
* @tc.desc  : Test OH_AudioDebuggingManager_PrintCapturerInfo interface.
*             Returns ERROR_ILLEGAL_STATE or ERROR_SYSTEM with invalid capturer pointer.
*/
HWTEST_F(AudioCapturerDebuggingManagerAiut, OH_PrintCapturerInfo_InvalidCapturer, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    OH_AudioDebuggingManager *manager = nullptr;
    OH_AudioManager_GetAudioDebuggingManager(&manager);
    AIUT_ASSERT_NE_INFO_LOG(manager, nullptr, "GetAudioDebuggingManager returned null");

    OH_AudioCapturer *fakeCapturer = reinterpret_cast<OH_AudioCapturer *>(0xCAFE);
    OH_AudioCommon_Result result = OH_AudioDebuggingManager_PrintCapturerInfo(manager, fakeCapturer, -1);
    AIUT_EXPECT_TRUE_INFO_LOG(
        result == AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE || result == AUDIOCOMMON_RESULT_ERROR_SYSTEM,
        "unexpected result=%d", result);
}

/**
* @tc.name  : Test OH_AudioDebuggingManager_PrintCapturerInfo API with both null params
* @tc.number: OH_PrintCapturerInfo_BothNull
* @tc.desc  : Test OH_AudioDebuggingManager_PrintCapturerInfo interface.
*             Returns AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM when both params are null.
*/
HWTEST_F(AudioCapturerDebuggingManagerAiut, OH_PrintCapturerInfo_BothNull, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioCommon_Result result = OH_AudioDebuggingManager_PrintCapturerInfo(nullptr, nullptr, -1);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "expected INVALID_PARAM, got %d", result);
}

// ==================== Taihe Capturer Tests ====================

/**
* @tc.name  : Test Taihe PrintCapturerInfo API gets key and prints
* @tc.number: Taihe_PrintCapturerInfo_GetKeyAndPrint
* @tc.desc  : Test PrintAudioCapturerDebugInfo interface.
*             Returns SUCCESS and output contains audioCapturer info via key lookup.
*/
HWTEST_F(AudioCapturerDebuggingManagerAiut, Taihe_PrintCapturerInfo_GetKeyAndPrint, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    // Taihe: PrintCapturerInfo gets key via capturer->GetCapturerDebugKey(),
    //        then calls PrintAudioCapturerDebugInfo(key, fd)
    auto callback = std::make_shared<TestCapturerDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    callback->testInfo_.sessionId = 200;
    callback->testInfo_.sourceType = SOURCE_TYPE_MIC;

    uint32_t key = 0;
    int32_t ret = AudioDebugManager::GetInstance().RegisterAudioCapturer(key, callback);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "RegisterAudioCapturer failed, ret=%d", ret);

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    ret = AudioDebugManager::GetInstance().PrintAudioCapturerDebugInfo(key, pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAudioCapturerDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_NE_INFO_LOG(output.find("audioCapturer"), std::string::npos, "output missing 'audioCapturer'");

    AudioDebugManager::GetInstance().UnregisterAudioCapturer(key);
}

// ==================== NAPI Capturer Tests ====================

/**
* @tc.name  : Test NAPI printCapturerInfo API gets key and prints
* @tc.number: Napi_PrintCapturerInfo_GetKeyAndPrint
* @tc.desc  : Test PrintAudioCapturerDebugInfo interface.
*             Returns SUCCESS and output contains audioCapturer info via key lookup.
*/
HWTEST_F(AudioCapturerDebuggingManagerAiut, Napi_PrintCapturerInfo_GetKeyAndPrint, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    // NAPI: printCapturerInfo(capturer, fd) -> napiCapturer->audioCapturer_->GetCapturerDebugKey()
    //       then AudioDebugManager::GetInstance().PrintAudioCapturerDebugInfo(key, fd)
    auto callback = std::make_shared<TestCapturerDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    callback->testInfo_.sessionId = 200;
    callback->testInfo_.sourceType = SOURCE_TYPE_MIC;

    uint32_t key = 0;
    int32_t ret = AudioDebugManager::GetInstance().RegisterAudioCapturer(key, callback);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "RegisterAudioCapturer failed, ret=%d", ret);
    AIUT_EXPECT_NE_INFO_LOG(key, 0u, "key not generated");

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    ret = AudioDebugManager::GetInstance().PrintAudioCapturerDebugInfo(key, pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAudioCapturerDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_NE_INFO_LOG(output.find("audioCapturer"), std::string::npos, "output missing 'audioCapturer'");

    AudioDebugManager::GetInstance().UnregisterAudioCapturer(key);
}

/**
* @tc.name  : Test NAPI printCapturerInfo API with invalid key
* @tc.number: Napi_PrintCapturerInfo_InvalidKey
* @tc.desc  : Test PrintAudioCapturerDebugInfo interface. Returns ERR_ILLEGAL_STATE when capturer key is not registered.
*/
HWTEST_F(AudioCapturerDebuggingManagerAiut, Napi_PrintCapturerInfo_InvalidKey, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    int32_t ret = AudioDebugManager::GetInstance().PrintAudioCapturerDebugInfo(99999, -1);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_ILLEGAL_STATE, "expected ERR_ILLEGAL_STATE, got ret=%d", ret);
}

} // namespace AudioStandard
} // namespace OHOS
