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

#include "audio_renderer_debugging_manager_aiut.h"
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
constexpr uint32_t TEST_SAMPLING_RATE = 44100;
constexpr float TEST_VOLUME = 0.5f;
constexpr float TEST_SPEED = 1.5f;
constexpr float TEST_PITCH = 1.0f;
constexpr uint32_t TEST_PIPE_ID = 100;
constexpr uint32_t TEST_SESSION_ID = 200;
}

class TestRendererDebugCallback : public AudioRendererDebugCallback {
public:
    int32_t GetRendererDebugInfo(AudioRendererDebugInfo &debugInfo) override
    {
        debugInfo = testInfo_;
        return retCode_;
    }
    AudioRendererDebugInfo testInfo_;
    int32_t retCode_ = SUCCESS;
};

AudioRendererDebugInfo CreateTestRendererDebugInfo()
{
    AudioRendererDebugInfo info;
    info.sessionId = TEST_SESSION_ID;
    info.samplingRate = TEST_SAMPLING_RATE;
    info.channels = AudioChannel::STEREO;
    info.format = AudioSampleFormat::SAMPLE_S16LE;
    info.encoding = AudioEncodingType::ENCODING_PCM;
    info.streamUsage = STREAM_USAGE_MUSIC;
    info.rendererFlags = AUDIO_FLAG_NORMAL;
    info.speed = TEST_SPEED;
    info.pitch = TEST_PITCH;
    info.effectMode = EFFECT_DEFAULT;
    info.pipeId = TEST_PIPE_ID;
    info.pipeRole = PIPE_ROLE_OUTPUT;
    info.pipeName = "test_pipe";
    info.routeFlag = AUDIO_OUTPUT_FLAG_NORMAL;
    info.adapterName = "test_adapter";
    info.mainDeviceType = DEVICE_TYPE_SPEAKER;
    info.volumeType = STREAM_MUSIC;
    info.streamVolume = TEST_VOLUME;
    info.systemVolume = 0.8f;
    info.volume = 0.6f;
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

void AudioRendererDebuggingManagerAiut::SetUpTestCase(void) {}
void AudioRendererDebuggingManagerAiut::TearDownTestCase(void) {}
void AudioRendererDebuggingManagerAiut::SetUp(void) {}
void AudioRendererDebuggingManagerAiut::TearDown(void) {}

// ==================== RegisterAudioRenderer Tests ====================

/**
* @tc.name  : Test RegisterAudioRenderer API with null callback
* @tc.number: AudioDebugManager_RegisterRenderer_NullCallback
* @tc.desc  : Test RegisterAudioRenderer interface. Returns ERR_INVALID_PARAM when callback is null.
*/
HWTEST_F(AudioRendererDebuggingManagerAiut, AudioDebugManager_RegisterRenderer_NullCallback, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    uint32_t key = 0;
    int32_t ret = AudioDebugManager::GetInstance().RegisterAudioRenderer(key, nullptr);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM, "expected ERR_INVALID_PARAM, got ret=%d", ret);
}

/**
* @tc.name  : Test RegisterAudioRenderer API with valid callback
* @tc.number: AudioDebugManager_RegisterRenderer_Success
* @tc.desc  : Test RegisterAudioRenderer interface. Returns SUCCESS and generates non-zero key on success.
*/
HWTEST_F(AudioRendererDebuggingManagerAiut, AudioDebugManager_RegisterRenderer_Success, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<TestRendererDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    uint32_t key = 0;
    int32_t ret = AudioDebugManager::GetInstance().RegisterAudioRenderer(key, callback);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "RegisterAudioRenderer failed, ret=%d", ret);
    AIUT_EXPECT_NE_INFO_LOG(key, 0u, "key not generated");

    AudioDebugManager::GetInstance().UnregisterAudioRenderer(key);
}

/**
* @tc.name  : Test RegisterAudioRenderer API generates unique keys
* @tc.number: AudioDebugManager_RegisterRenderer_MultipleKeys
* @tc.desc  : Test RegisterAudioRenderer interface. Multiple registrations generate unique non-zero keys.
*/
HWTEST_F(AudioRendererDebuggingManagerAiut, AudioDebugManager_RegisterRenderer_MultipleKeys, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto callback1 = std::make_shared<TestRendererDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback1, nullptr, "make_shared callback1 failed");
    auto callback2 = std::make_shared<TestRendererDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback2, nullptr, "make_shared callback2 failed");

    uint32_t key1 = 0;
    uint32_t key2 = 0;
    int32_t ret1 = AudioDebugManager::GetInstance().RegisterAudioRenderer(key1, callback1);
    int32_t ret2 = AudioDebugManager::GetInstance().RegisterAudioRenderer(key2, callback2);
    AIUT_EXPECT_EQ_INFO_LOG(ret1, SUCCESS, "Register1 failed, ret=%d", ret1);
    AIUT_EXPECT_EQ_INFO_LOG(ret2, SUCCESS, "Register2 failed, ret=%d", ret2);
    AIUT_EXPECT_NE_INFO_LOG(key1, key2, "keys not unique: key1=%u, key2=%u", key1, key2);

    AudioDebugManager::GetInstance().UnregisterAudioRenderer(key1);
    AudioDebugManager::GetInstance().UnregisterAudioRenderer(key2);
}

// ==================== UnregisterAudioRenderer Tests ====================

/**
* @tc.name  : Test UnregisterAudioRenderer API with non-existent key
* @tc.number: AudioDebugManager_UnregisterRenderer_NotFound
* @tc.desc  : Test UnregisterAudioRenderer interface. Returns ERR_ILLEGAL_STATE when key is not found.
*/
HWTEST_F(AudioRendererDebuggingManagerAiut, AudioDebugManager_UnregisterRenderer_NotFound, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int32_t ret = AudioDebugManager::GetInstance().UnregisterAudioRenderer(99999);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_ILLEGAL_STATE, "expected ERR_ILLEGAL_STATE, got ret=%d", ret);
}

/**
* @tc.name  : Test UnregisterAudioRenderer API with valid key
* @tc.number: AudioDebugManager_UnregisterRenderer_Success
* @tc.desc  : Test UnregisterAudioRenderer interface. Returns SUCCESS when unregistering a registered renderer.
*/
HWTEST_F(AudioRendererDebuggingManagerAiut, AudioDebugManager_UnregisterRenderer_Success, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<TestRendererDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    uint32_t key = 0;
    AudioDebugManager::GetInstance().RegisterAudioRenderer(key, callback);

    int32_t ret = AudioDebugManager::GetInstance().UnregisterAudioRenderer(key);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "UnregisterAudioRenderer failed, ret=%d", ret);
}

/**
* @tc.name  : Test UnregisterAudioRenderer API called twice on same key
* @tc.number: AudioDebugManager_UnregisterRenderer_DoubleUnregister
* @tc.desc  : Test UnregisterAudioRenderer interface. Returns ERR_ILLEGAL_STATE on second unregister of same key.
*/
HWTEST_F(AudioRendererDebuggingManagerAiut, AudioDebugManager_UnregisterRenderer_DoubleUnregister, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<TestRendererDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    uint32_t key = 0;
    AudioDebugManager::GetInstance().RegisterAudioRenderer(key, callback);

    int32_t ret1 = AudioDebugManager::GetInstance().UnregisterAudioRenderer(key);
    AIUT_EXPECT_EQ_INFO_LOG(ret1, SUCCESS, "first unregister failed, ret=%d", ret1);
    int32_t ret2 = AudioDebugManager::GetInstance().UnregisterAudioRenderer(key);
    AIUT_EXPECT_EQ_INFO_LOG(ret2, ERR_ILLEGAL_STATE, "second unregister should fail, got ret=%d", ret2);
}

// ==================== PrintAudioRendererDebugInfo Tests ====================

/**
* @tc.name  : Test PrintAudioRendererDebugInfo API with invalid key
* @tc.number: AudioDebugManager_PrintRenderer_InvalidKey
* @tc.desc  : Test PrintAudioRendererDebugInfo interface. Returns ERR_ILLEGAL_STATE when key is not registered.
*/
HWTEST_F(AudioRendererDebuggingManagerAiut, AudioDebugManager_PrintRenderer_InvalidKey, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int32_t ret = AudioDebugManager::GetInstance().PrintAudioRendererDebugInfo(99999, -1);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_ILLEGAL_STATE, "expected ERR_ILLEGAL_STATE, got ret=%d", ret);
}

/**
* @tc.name  : Test PrintAudioRendererDebugInfo API with pipe fd
* @tc.number: AudioDebugManager_PrintRenderer_WithPipe
* @tc.desc  : Test PrintAudioRendererDebugInfo interface.
*             Returns SUCCESS and output contains renderer debug info fields.
*/
HWTEST_F(AudioRendererDebuggingManagerAiut, AudioDebugManager_PrintRenderer_WithPipe, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<TestRendererDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    callback->testInfo_ = CreateTestRendererDebugInfo();

    uint32_t key = 0;
    int32_t ret = AudioDebugManager::GetInstance().RegisterAudioRenderer(key, callback);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "RegisterAudioRenderer failed, ret=%d", ret);

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    ret = AudioDebugManager::GetInstance().PrintAudioRendererDebugInfo(key, pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAudioRendererDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_NE_INFO_LOG(output.find("audioRenderer"), std::string::npos, "output missing 'audioRenderer'");
    AIUT_EXPECT_NE_INFO_LOG(output.find("streamId"), std::string::npos, "output missing 'streamId'");
    AIUT_EXPECT_NE_INFO_LOG(output.find(std::to_string(TEST_SAMPLING_RATE)),
        std::string::npos, "output missing sampling rate");
    AIUT_EXPECT_NE_INFO_LOG(output.find("STREAM_USAGE_MUSIC"), std::string::npos,
        "output missing 'STREAM_USAGE_MUSIC'");
    AIUT_EXPECT_NE_INFO_LOG(output.find("DEVICE_TYPE_SPEAKER"), std::string::npos,
        "output missing 'DEVICE_TYPE_SPEAKER'");

    AudioDebugManager::GetInstance().UnregisterAudioRenderer(key);
}

/**
* @tc.name  : Test PrintAudioRendererDebugInfo API with negative fd (log fallback)
* @tc.number: AudioDebugManager_PrintRenderer_NegativeFd_LogFallback
* @tc.desc  : Test PrintAudioRendererDebugInfo interface. Returns SUCCESS with negative fd, falling back to log output.
*/
HWTEST_F(AudioRendererDebuggingManagerAiut, AudioDebugManager_PrintRenderer_NegativeFd_LogFallback, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<TestRendererDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    callback->testInfo_ = CreateTestRendererDebugInfo();

    uint32_t key = 0;
    int32_t ret = AudioDebugManager::GetInstance().RegisterAudioRenderer(key, callback);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "RegisterAudioRenderer failed, ret=%d", ret);

    ret = AudioDebugManager::GetInstance().PrintAudioRendererDebugInfo(key, -1);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintRendererDebugInfo with -1 failed, ret=%d", ret);

    AudioDebugManager::GetInstance().UnregisterAudioRenderer(key);
}

/**
* @tc.name  : Test PrintAudioRendererDebugInfo API when callback returns error
* @tc.number: AudioDebugManager_PrintRenderer_CallbackError
* @tc.desc  : Test PrintAudioRendererDebugInfo interface.
*             Returns ERR_ILLEGAL_STATE when callback GetRendererDebugInfo fails.
*/
HWTEST_F(AudioRendererDebuggingManagerAiut, AudioDebugManager_PrintRenderer_CallbackError, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<TestRendererDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    callback->retCode_ = ERR_ILLEGAL_STATE;

    uint32_t key = 0;
    int32_t ret = AudioDebugManager::GetInstance().RegisterAudioRenderer(key, callback);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "RegisterAudioRenderer failed, ret=%d", ret);

    ret = AudioDebugManager::GetInstance().PrintAudioRendererDebugInfo(key, -1);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_ILLEGAL_STATE, "expected ERR_ILLEGAL_STATE, got ret=%d", ret);

    AudioDebugManager::GetInstance().UnregisterAudioRenderer(key);
}

// ==================== PrintAllAudioRenderersDebugInfo Tests ====================

/**
* @tc.name  : Test PrintAllAudioRenderersDebugInfo API with no renderers
* @tc.number: AudioDebugManager_PrintAllRenderers_NoRenderers
* @tc.desc  : Test PrintAllAudioRenderersDebugInfo interface. Returns SUCCESS when no renderers are registered.
*/
HWTEST_F(AudioRendererDebuggingManagerAiut, AudioDebugManager_PrintAllRenderers_NoRenderers, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    int32_t ret = AudioDebugManager::GetInstance().PrintAllAudioRenderersDebugInfo(pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAllRenderersDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);
    close(pipefd[0]);
}

/**
* @tc.name  : Test PrintAllAudioRenderersDebugInfo API with registered callback
* @tc.number: AudioDebugManager_PrintAllRenderers_WithCallback
* @tc.desc  : Test PrintAllAudioRenderersDebugInfo interface. Returns SUCCESS and output contains audioRenderer info.
*/
HWTEST_F(AudioRendererDebuggingManagerAiut, AudioDebugManager_PrintAllRenderers_WithCallback, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<TestRendererDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    callback->testInfo_ = CreateTestRendererDebugInfo();

    uint32_t key = 0;
    AudioDebugManager::GetInstance().RegisterAudioRenderer(key, callback);

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    int32_t ret = AudioDebugManager::GetInstance().PrintAllAudioRenderersDebugInfo(pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAllRenderersDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_NE_INFO_LOG(output.find("audioRenderer"), std::string::npos, "output missing 'audioRenderer'");

    AudioDebugManager::GetInstance().UnregisterAudioRenderer(key);
}

/**
* @tc.name  : Test PrintAllAudioRenderersDebugInfo API with multiple callbacks
* @tc.number: AudioDebugManager_PrintAllRenderers_MultipleCallbacks
* @tc.desc  : Test PrintAllAudioRenderersDebugInfo interface. Output contains info from all registered renderers.
*/
HWTEST_F(AudioRendererDebuggingManagerAiut, AudioDebugManager_PrintAllRenderers_MultipleCallbacks, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto callback1 = std::make_shared<TestRendererDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback1, nullptr, "make_shared callback1 failed");
    callback1->testInfo_ = CreateTestRendererDebugInfo();
    callback1->testInfo_.sessionId = 1001;

    auto callback2 = std::make_shared<TestRendererDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback2, nullptr, "make_shared callback2 failed");
    callback2->testInfo_ = CreateTestRendererDebugInfo();
    callback2->testInfo_.sessionId = 1002;

    uint32_t key1 = 0, key2 = 0;
    AudioDebugManager::GetInstance().RegisterAudioRenderer(key1, callback1);
    AudioDebugManager::GetInstance().RegisterAudioRenderer(key2, callback2);

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    int32_t ret = AudioDebugManager::GetInstance().PrintAllAudioRenderersDebugInfo(pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAllRenderersDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_NE_INFO_LOG(output.find("1001"), std::string::npos, "output missing '1001'");
    AIUT_EXPECT_NE_INFO_LOG(output.find("1002"), std::string::npos, "output missing '1002'");

    AudioDebugManager::GetInstance().UnregisterAudioRenderer(key1);
    AudioDebugManager::GetInstance().UnregisterAudioRenderer(key2);
}

/**
* @tc.name  : Test PrintAllAudioRenderersDebugInfo API with partial callback failure
* @tc.number: AudioDebugManager_PrintAllRenderers_PartialFailure
* @tc.desc  : Test PrintAllAudioRenderersDebugInfo interface. Returns ERR_OPERATION_FAILED when some callbacks fail.
*/
HWTEST_F(AudioRendererDebuggingManagerAiut, AudioDebugManager_PrintAllRenderers_PartialFailure, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto goodCallback = std::make_shared<TestRendererDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(goodCallback, nullptr, "make_shared goodCallback failed");
    goodCallback->testInfo_ = CreateTestRendererDebugInfo();

    auto badCallback = std::make_shared<TestRendererDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(badCallback, nullptr, "make_shared badCallback failed");
    badCallback->retCode_ = ERR_ILLEGAL_STATE;

    uint32_t key1 = 0, key2 = 0;
    AudioDebugManager::GetInstance().RegisterAudioRenderer(key1, goodCallback);
    AudioDebugManager::GetInstance().RegisterAudioRenderer(key2, badCallback);

    int32_t ret = AudioDebugManager::GetInstance().PrintAllAudioRenderersDebugInfo(-1);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_OPERATION_FAILED, "expected ERR_OPERATION_FAILED, got ret=%d", ret);

    AudioDebugManager::GetInstance().UnregisterAudioRenderer(key1);
    AudioDebugManager::GetInstance().UnregisterAudioRenderer(key2);
}

// ==================== Renderer DebugInfo Content Verification ====================

/**
* @tc.name  : Test RendererDebugInfo contains focusHistory field
* @tc.number: AudioDebugManager_RendererDebugInfo_FocusHistory
* @tc.desc  : Test PrintAudioRendererDebugInfo interface. Output contains focusHistory and INTERRUPT_HINT_PAUSE info.
*/
HWTEST_F(AudioRendererDebuggingManagerAiut, AudioDebugManager_RendererDebugInfo_FocusHistory, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<TestRendererDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    callback->testInfo_ = CreateTestRendererDebugInfo();

    AudioFocusBehavior behavior;
    behavior.streamId = 500;
    behavior.pid = 100;
    behavior.uid = TEST_APP_UID;
    behavior.streamType = STREAM_VOICE_COMMUNICATION;
    behavior.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    behavior.hintType = INTERRUPT_HINT_PAUSE;
    behavior.timestamp = 1234567890;
    callback->testInfo_.focusHistory.push_back(behavior);

    uint32_t key = 0;
    AudioDebugManager::GetInstance().RegisterAudioRenderer(key, callback);

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    int32_t ret = AudioDebugManager::GetInstance().PrintAudioRendererDebugInfo(key, pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAudioRendererDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_NE_INFO_LOG(output.find("focusHistory"), std::string::npos, "output missing 'focusHistory'");
    AIUT_EXPECT_NE_INFO_LOG(output.find("INTERRUPT_HINT_PAUSE"), std::string::npos,
        "output missing 'INTERRUPT_HINT_PAUSE'");

    AudioDebugManager::GetInstance().UnregisterAudioRenderer(key);
}

/**
* @tc.name  : Test RendererDebugInfo contains errorInfos field
* @tc.number: AudioDebugManager_RendererDebugInfo_ErrorInfos
* @tc.desc  : Test PrintAudioRendererDebugInfo interface. Output contains errorInfos and error reason text.
*/
HWTEST_F(AudioRendererDebuggingManagerAiut, AudioDebugManager_RendererDebugInfo_ErrorInfos, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<TestRendererDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    callback->testInfo_ = CreateTestRendererDebugInfo();

    AudioFaultRecord record;
    record.uid = TEST_APP_UID;
    record.errorCode = 404;
    record.errorReason = "test_error";
    record.timestamp = 9876543210;
    callback->testInfo_.errorInfos.push_back(record);

    uint32_t key = 0;
    AudioDebugManager::GetInstance().RegisterAudioRenderer(key, callback);

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    int32_t ret = AudioDebugManager::GetInstance().PrintAudioRendererDebugInfo(key, pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAudioRendererDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_NE_INFO_LOG(output.find("errorInfos"), std::string::npos, "output missing 'errorInfos'");
    AIUT_EXPECT_NE_INFO_LOG(output.find("test_error"), std::string::npos, "output missing 'test_error'");

    AudioDebugManager::GetInstance().UnregisterAudioRenderer(key);
}

// ==================== Renderer Re-Register ====================

/**
* @tc.name  : Test re-registering a renderer after unregister
* @tc.number: AudioDebugManager_Renderer_ReRegister
* @tc.desc  : Test RegisterAudioRenderer interface.
*             Successfully re-registers after unregistering, with a new unique key.
*/
HWTEST_F(AudioRendererDebuggingManagerAiut, AudioDebugManager_Renderer_ReRegister, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto callback1 = std::make_shared<TestRendererDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback1, nullptr, "make_shared callback1 failed");
    uint32_t key1 = 0;
    AudioDebugManager::GetInstance().RegisterAudioRenderer(key1, callback1);
    AudioDebugManager::GetInstance().UnregisterAudioRenderer(key1);

    auto callback2 = std::make_shared<TestRendererDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback2, nullptr, "make_shared callback2 failed");
    uint32_t key2 = 0;
    int32_t ret = AudioDebugManager::GetInstance().RegisterAudioRenderer(key2, callback2);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "Re-register failed, ret=%d", ret);
    AIUT_EXPECT_NE_INFO_LOG(key2, 0u, "key2 not generated");
    AIUT_EXPECT_NE_INFO_LOG(key2, key1, "key2==key1 after re-register: %u", key2);

    AudioDebugManager::GetInstance().UnregisterAudioRenderer(key2);
}

// ==================== OHAudio C API Renderer Tests ====================

/**
* @tc.name  : Test OH_AudioDebuggingManager_PrintRendererInfo API with null manager
* @tc.number: OH_PrintRendererInfo_NullManager
* @tc.desc  : Test OH_AudioDebuggingManager_PrintRendererInfo interface.
*             Returns AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM when manager is null.
*/
HWTEST_F(AudioRendererDebuggingManagerAiut, OH_PrintRendererInfo_NullManager, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioCommon_Result result = OH_AudioDebuggingManager_PrintRendererInfo(nullptr, nullptr, -1);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "expected INVALID_PARAM, got %d", result);
}

/**
* @tc.name  : Test OH_AudioDebuggingManager_PrintRendererInfo API with null renderer
* @tc.number: OH_PrintRendererInfo_NullRenderer
* @tc.desc  : Test OH_AudioDebuggingManager_PrintRendererInfo interface.
*             Returns AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM when renderer is null.
*/
HWTEST_F(AudioRendererDebuggingManagerAiut, OH_PrintRendererInfo_NullRenderer, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioDebuggingManager *manager = nullptr;
    OH_AudioManager_GetAudioDebuggingManager(&manager);
    AIUT_ASSERT_NE_INFO_LOG(manager, nullptr, "GetAudioDebuggingManager returned null");

    OH_AudioCommon_Result result = OH_AudioDebuggingManager_PrintRendererInfo(manager, nullptr, -1);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "expected INVALID_PARAM, got %d", result);
}


/**
* @tc.name  : Test OH_AudioDebuggingManager_PrintRendererInfo API with both null params
* @tc.number: OH_PrintRendererInfo_BothNull
* @tc.desc  : Test OH_AudioDebuggingManager_PrintRendererInfo interface.
*             Returns AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM when both params are null.
*/
HWTEST_F(AudioRendererDebuggingManagerAiut, OH_PrintRendererInfo_BothNull, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioCommon_Result result = OH_AudioDebuggingManager_PrintRendererInfo(nullptr, nullptr, -1);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "expected INVALID_PARAM, got %d", result);
}

// ==================== Taihe Renderer Tests ====================

/**
* @tc.name  : Test Taihe PrintRendererInfo API gets key and prints
* @tc.number: Taihe_PrintRendererInfo_GetKeyAndPrint
* @tc.desc  : Test PrintAudioRendererDebugInfo interface.
*             Returns SUCCESS and output contains audioRenderer info via key lookup.
*/
HWTEST_F(AudioRendererDebuggingManagerAiut, Taihe_PrintRendererInfo_GetKeyAndPrint, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    // Taihe: PrintRendererInfo gets key via renderer->GetRendererDebugKey(),
    //        then calls PrintAudioRendererDebugInfo(key, fd)
    auto callback = std::make_shared<TestRendererDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    callback->testInfo_.sessionId = 100;
    callback->testInfo_.samplingRate = 48000;
    callback->testInfo_.streamUsage = STREAM_USAGE_MUSIC;
    callback->testInfo_.mainDeviceType = DEVICE_TYPE_SPEAKER;

    uint32_t key = 0;
    int32_t ret = AudioDebugManager::GetInstance().RegisterAudioRenderer(key, callback);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "RegisterAudioRenderer failed, ret=%d", ret);

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    ret = AudioDebugManager::GetInstance().PrintAudioRendererDebugInfo(key, pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAudioRendererDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_NE_INFO_LOG(output.find("audioRenderer"), std::string::npos, "output missing 'audioRenderer'");

    AudioDebugManager::GetInstance().UnregisterAudioRenderer(key);
}

// ==================== NAPI Renderer Tests ====================

/**
* @tc.name  : Test NAPI printRendererInfo API gets key and prints
* @tc.number: Napi_PrintRendererInfo_GetKeyAndPrint
* @tc.desc  : Test PrintAudioRendererDebugInfo interface.
*             Returns SUCCESS and output contains audioRenderer info via key lookup.
*/
HWTEST_F(AudioRendererDebuggingManagerAiut, Napi_PrintRendererInfo_GetKeyAndPrint, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    // NAPI: printRendererInfo(renderer, fd) -> napiRenderer->audioRenderer_->GetRendererDebugKey()
    //       then AudioDebugManager::GetInstance().PrintAudioRendererDebugInfo(key, fd)
    auto callback = std::make_shared<TestRendererDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    callback->testInfo_.sessionId = 100;
    callback->testInfo_.streamUsage = STREAM_USAGE_MUSIC;
    callback->testInfo_.mainDeviceType = DEVICE_TYPE_SPEAKER;

    uint32_t key = 0;
    int32_t ret = AudioDebugManager::GetInstance().RegisterAudioRenderer(key, callback);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "RegisterAudioRenderer failed, ret=%d", ret);
    AIUT_EXPECT_NE_INFO_LOG(key, 0u, "key not generated");

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    ret = AudioDebugManager::GetInstance().PrintAudioRendererDebugInfo(key, pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAudioRendererDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_NE_INFO_LOG(output.find("audioRenderer"), std::string::npos, "output missing 'audioRenderer'");

    AudioDebugManager::GetInstance().UnregisterAudioRenderer(key);
}

/**
* @tc.name  : Test NAPI printRendererInfo API with invalid key
* @tc.number: Napi_PrintRendererInfo_InvalidKey
* @tc.desc  : Test PrintAudioRendererDebugInfo interface. Returns ERR_ILLEGAL_STATE when renderer key is not registered.
*/
HWTEST_F(AudioRendererDebuggingManagerAiut, Napi_PrintRendererInfo_InvalidKey, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    // NAPI: If renderer is valid but not registered, PrintAudioRendererDebugInfo returns error
    int32_t ret = AudioDebugManager::GetInstance().PrintAudioRendererDebugInfo(99999, -1);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_ILLEGAL_STATE, "expected ERR_ILLEGAL_STATE, got ret=%d", ret);
}

} // namespace AudioStandard
} // namespace OHOS
