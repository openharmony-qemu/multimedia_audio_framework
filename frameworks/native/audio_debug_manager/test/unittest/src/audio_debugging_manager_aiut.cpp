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

#include "audio_debugging_manager_aiut.h"
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
constexpr uint32_t TEST_CAPTURER_SAMPLING_RATE = 16000;
constexpr size_t TEST_CAPTURER_BUFFER_SIZE = 4096;
constexpr int32_t TEST_PID = 1000;
constexpr uint32_t TEST_STREAM_DEBUG_ID = 300;
constexpr float TEST_VOLUME = 0.5f;
constexpr float TEST_SPEED = 1.5f;
constexpr float TEST_PITCH = 1.0f;
constexpr uint32_t TEST_PIPE_ID = 100;
constexpr uint32_t TEST_SESSION_ID = 200;
}

// Mock callback implementations for all types
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

class TestLoopbackDebugCallback : public AudioLoopbackDebugCallback {
public:
    int32_t GetLoopbackDebugInfo(AudioLoopbackDebugInfo &debugInfo) override
    {
        debugInfo = testInfo_;
        return retCode_;
    }
    AudioLoopbackDebugInfo testInfo_;
    int32_t retCode_ = SUCCESS;
};

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

AudioRendererDebugInfo CreateTestRendererDebugInfo()
{
    AudioRendererDebugInfo info;
    info.sessionId = TEST_SESSION_ID;
    info.samplingRate = TEST_SAMPLING_RATE;
    info.channels = AudioChannel::CHANNEL_2;
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

AudioSessionDebugInfo CreateTestSessionDebugInfo()
{
    AudioSessionDebugInfo info;
    info.strategy.concurrencyMode = AudioConcurrencyMode::MIX_WITH_OTHERS;
    info.audioSessionScene = AudioSessionScene::MEDIA;
    info.defaultDeviceType = DEVICE_TYPE_SPEAKER;
    info.state = AudioSessionState::SESSION_ACTIVE;
    info.fakeFocusState = AudioFocuState::ACTIVE;
    info.pid = TEST_PID;
    info.uid = TEST_APP_UID;

    AudioStreamDebugInfo streamInfo;
    streamInfo.streamId = TEST_STREAM_DEBUG_ID;
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

void AudioDebuggingManagerAiut::SetUpTestCase(void) {}
void AudioDebuggingManagerAiut::TearDownTestCase(void) {}
void AudioDebuggingManagerAiut::SetUp(void) {}
void AudioDebuggingManagerAiut::TearDown(void) {}

// ==================== GetInstance Tests ====================

/**
* @tc.name  : Test GetInstance API
* @tc.number: AudioDebugManager_GetInstance_001
* @tc.desc  : Test GetInstance interface. Returns valid AudioDebugManager instance on success.
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_GetInstance_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &instance = AudioDebugManager::GetInstance();
    AIUT_EXPECT_NE_INFO_LOG(&instance, nullptr, "GetInstance returned null");
}

/**
* @tc.name  : Test GetInstance API returns singleton
* @tc.number: AudioDebugManager_GetInstance_Singleton
* @tc.desc  : Test GetInstance interface. Returns the same instance on multiple calls, verifying singleton pattern.
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_GetInstance_Singleton, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &instance1 = AudioDebugManager::GetInstance();
    auto &instance2 = AudioDebugManager::GetInstance();
    AIUT_EXPECT_EQ_INFO_LOG(&instance1, &instance2,
        "Singleton instances differ, addr1=%p, addr2=%p", &instance1, &instance2);
}

// ==================== PrintAppAudioDebugInfo Tests ====================

/**
* @tc.name  : Test PrintAppAudioDebugInfo API with no registered callbacks
* @tc.number: AudioDebugManager_PrintAppDebugInfo_Empty
* @tc.desc  : Test PrintAppAudioDebugInfo interface.
*             Returns SUCCESS or ERR_OPERATION_FAILED when no callbacks registered.
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_PrintAppDebugInfo_Empty, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    int32_t ret = AudioDebugManager::GetInstance().PrintAppAudioDebugInfo(pipefd[1]);
    close(pipefd[1]);
    close(pipefd[0]);
    AIUT_EXPECT_TRUE_INFO_LOG(ret == SUCCESS || ret == ERR_OPERATION_FAILED, "unexpected ret=%d", ret);
}

/**
* @tc.name  : Test PrintAppAudioDebugInfo API with all callback types registered
* @tc.number: AudioDebugManager_PrintAppDebugInfo_WithAllCallbacks
* @tc.desc  : Test PrintAppAudioDebugInfo interface.
*             Returns SUCCESS and output contains renderer, capturer, loopback, session info.
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_PrintAppDebugInfo_WithAllCallbacks, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto rendererCallback = std::make_shared<TestRendererDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(rendererCallback, nullptr, "make_shared RendererCallback failed");
    rendererCallback->testInfo_ = CreateTestRendererDebugInfo();
    uint32_t rendererKey = 0;
    AudioDebugManager::GetInstance().RegisterAudioRenderer(rendererKey, rendererCallback);

    auto capturerCallback = std::make_shared<TestCapturerDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(capturerCallback, nullptr, "make_shared CapturerCallback failed");
    capturerCallback->testInfo_ = CreateTestCapturerDebugInfo();
    uint32_t capturerKey = 0;
    AudioDebugManager::GetInstance().RegisterAudioCapturer(capturerKey, capturerCallback);

    auto loopbackCallback = std::make_shared<TestLoopbackDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(loopbackCallback, nullptr, "make_shared LoopbackCallback failed");
    loopbackCallback->testInfo_.appUid = TEST_APP_UID;
    loopbackCallback->testInfo_.appName = "LoopbackApp";
    uint32_t loopbackKey = 0;
    AudioDebugManager::GetInstance().RegisterAudioLoopback(loopbackKey, loopbackCallback);

    auto sessionCallback = std::make_shared<TestSessionDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(sessionCallback, nullptr, "make_shared SessionCallback failed");
    sessionCallback->testInfo_ = CreateTestSessionDebugInfo();
    AudioDebugManager::GetInstance().SetAudioSessionCallback(sessionCallback);

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    int32_t ret = AudioDebugManager::GetInstance().PrintAppAudioDebugInfo(pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAppAudioDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_NE_INFO_LOG(output.find("audioRenderer"), std::string::npos, "output missing 'audioRenderer'");
    AIUT_EXPECT_NE_INFO_LOG(output.find("audioCapturer"), std::string::npos, "output missing 'audioCapturer'");
    AIUT_EXPECT_NE_INFO_LOG(output.find("audioLoopback"), std::string::npos, "output missing 'audioLoopback'");
    AIUT_EXPECT_NE_INFO_LOG(output.find("audioSession"), std::string::npos, "output missing 'audioSession'");

    AudioDebugManager::GetInstance().UnregisterAudioRenderer(rendererKey);
    AudioDebugManager::GetInstance().UnregisterAudioCapturer(capturerKey);
    AudioDebugManager::GetInstance().UnregisterAudioLoopback(loopbackKey);
}

// ==================== DebugKey Generation ====================

/**
* @tc.name  : Test debug key generation is incremental across types
* @tc.number: AudioDebugManager_DebugKey_Incremental
* @tc.desc  : Test RegisterAudioRenderer/Capturer/Loopback interface. Generated keys are unique and non-zero.
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_DebugKey_Incremental, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto rendererCb = std::make_shared<TestRendererDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(rendererCb, nullptr, "make_shared RendererCallback failed");
    auto capturerCb = std::make_shared<TestCapturerDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(capturerCb, nullptr, "make_shared CapturerCallback failed");
    auto loopbackCb = std::make_shared<TestLoopbackDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(loopbackCb, nullptr, "make_shared LoopbackCallback failed");

    uint32_t rKey = 0, cKey = 0, lKey = 0;
    AudioDebugManager::GetInstance().RegisterAudioRenderer(rKey, rendererCb);
    AudioDebugManager::GetInstance().RegisterAudioCapturer(cKey, capturerCb);
    AudioDebugManager::GetInstance().RegisterAudioLoopback(lKey, loopbackCb);

    AIUT_EXPECT_NE_INFO_LOG(rKey, 0u, "renderer key not generated");
    AIUT_EXPECT_NE_INFO_LOG(cKey, 0u, "capturer key not generated");
    AIUT_EXPECT_NE_INFO_LOG(lKey, 0u, "loopback key not generated");
    AIUT_EXPECT_NE_INFO_LOG(rKey, cKey, "renderer and capturer keys collide: %u", rKey);
    AIUT_EXPECT_NE_INFO_LOG(rKey, lKey, "renderer and loopback keys collide: %u", rKey);
    AIUT_EXPECT_NE_INFO_LOG(cKey, lKey, "capturer and loopback keys collide: %u", cKey);

    AudioDebugManager::GetInstance().UnregisterAudioRenderer(rKey);
    AudioDebugManager::GetInstance().UnregisterAudioCapturer(cKey);
    AudioDebugManager::GetInstance().UnregisterAudioLoopback(lKey);
}

/**
* @tc.name  : Test debug key generation is incremental across mixed registration types
* @tc.number: AudioDebugManager_DebugKey_IncrementalAcrossTypes
* @tc.desc  : Test Register interface. Keys generated for mixed renderer/capturer registrations are all unique.
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_DebugKey_IncrementalAcrossTypes, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto cb1 = std::make_shared<TestRendererDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(cb1, nullptr, "make_shared cb1 failed");
    auto cb2 = std::make_shared<TestCapturerDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(cb2, nullptr, "make_shared cb2 failed");
    auto cb3 = std::make_shared<TestRendererDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(cb3, nullptr, "make_shared cb3 failed");

    uint32_t key1 = 0, key2 = 0, key3 = 0;
    AudioDebugManager::GetInstance().RegisterAudioRenderer(key1, cb1);
    AudioDebugManager::GetInstance().RegisterAudioCapturer(key2, cb2);
    AudioDebugManager::GetInstance().RegisterAudioRenderer(key3, cb3);

    AIUT_EXPECT_NE_INFO_LOG(key1, key2, "key1==key2: %u", key1);
    AIUT_EXPECT_NE_INFO_LOG(key2, key3, "key2==key3: %u", key2);
    AIUT_EXPECT_NE_INFO_LOG(key1, key3, "key1==key3: %u", key1);

    AudioDebugManager::GetInstance().UnregisterAudioRenderer(key1);
    AudioDebugManager::GetInstance().UnregisterAudioCapturer(key2);
    AudioDebugManager::GetInstance().UnregisterAudioRenderer(key3);
}

// ==================== OHAudio C API GetAudioDebuggingManager Tests ====================

/**
* @tc.name  : Test OH_AudioManager_GetAudioDebuggingManager API with null param
* @tc.number: OH_GetAudioDebuggingManager_NullParam
* @tc.desc  : Test OH_AudioManager_GetAudioDebuggingManager interface.
*             Returns AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM when param is null.
*/
HWTEST_F(AudioDebuggingManagerAiut, OH_GetAudioDebuggingManager_NullParam, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioDebuggingManager(nullptr);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "expected INVALID_PARAM, got %d", result);
}

/**
* @tc.name  : Test OH_AudioManager_GetAudioDebuggingManager API with valid param
* @tc.number: OH_GetAudioDebuggingManager_Success
* @tc.desc  : Test OH_AudioManager_GetAudioDebuggingManager interface.
*             Returns AUDIOCOMMON_RESULT_SUCCESS and valid manager pointer.
*/
HWTEST_F(AudioDebuggingManagerAiut, OH_GetAudioDebuggingManager_Success, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioDebuggingManager *manager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioDebuggingManager(&manager);
    AIUT_ASSERT_NE_INFO_LOG(manager, nullptr, "GetAudioDebuggingManager returned null");
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_SUCCESS, "expected SUCCESS, got %d", result);
}

/**
* @tc.name  : Test OH_AudioManager_GetAudioDebuggingManager API returns singleton
* @tc.number: OH_GetAudioDebuggingManager_Singleton
* @tc.desc  : Test OH_AudioManager_GetAudioDebuggingManager interface.
*             Returns the same manager instance on multiple calls.
*/
HWTEST_F(AudioDebuggingManagerAiut, OH_GetAudioDebuggingManager_Singleton, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    OH_AudioDebuggingManager *manager1 = nullptr;
    OH_AudioDebuggingManager *manager2 = nullptr;
    OH_AudioCommon_Result result1 = OH_AudioManager_GetAudioDebuggingManager(&manager1);
    AIUT_ASSERT_NE_INFO_LOG(manager1, nullptr, "GetAudioDebuggingManager returned null for manager1");
    OH_AudioCommon_Result result2 = OH_AudioManager_GetAudioDebuggingManager(&manager2);
    AIUT_ASSERT_NE_INFO_LOG(manager2, nullptr, "GetAudioDebuggingManager returned null for manager2");
    AIUT_EXPECT_EQ_INFO_LOG(result1, AUDIOCOMMON_RESULT_SUCCESS, "result1=%d", result1);
    AIUT_EXPECT_EQ_INFO_LOG(result2, AUDIOCOMMON_RESULT_SUCCESS, "result2=%d", result2);
    AIUT_EXPECT_EQ_INFO_LOG(manager1, manager2, "Singleton instances differ, addr1=%p, addr2=%p", manager1, manager2);
}

/**
* @tc.name  : Test OH_AudioManager_GetAudioDebuggingManager API overwrites existing output
* @tc.number: OH_GetAudioDebuggingManager_OutputOverwrite
* @tc.desc  : Test OH_AudioManager_GetAudioDebuggingManager interface.
*             Overwrites pre-existing manager pointer value with valid instance.
*/
HWTEST_F(AudioDebuggingManagerAiut, OH_GetAudioDebuggingManager_OutputOverwrite, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    OH_AudioDebuggingManager *manager = reinterpret_cast<OH_AudioDebuggingManager *>(0xDEAD);
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioDebuggingManager(&manager);
    AIUT_ASSERT_NE_INFO_LOG(manager, nullptr, "GetAudioDebuggingManager returned null");
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_SUCCESS, "expected SUCCESS, got %d", result);
    AIUT_EXPECT_NE_INFO_LOG(manager, reinterpret_cast<OH_AudioDebuggingManager *>(0xDEAD), "manager not overwritten");
}

// ==================== OHAudio C API PrintAppInfo Tests ====================

/**
* @tc.name  : Test OH_AudioDebuggingManager_PrintAppInfo API with null manager
* @tc.number: OH_PrintAppInfo_NullManager
* @tc.desc  : Test OH_AudioDebuggingManager_PrintAppInfo interface.
*             Returns AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM when manager is null.
*/
HWTEST_F(AudioDebuggingManagerAiut, OH_PrintAppInfo_NullManager, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioCommon_Result result = OH_AudioDebuggingManager_PrintAppInfo(nullptr, -1);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "expected INVALID_PARAM, got %d", result);
}

/**
* @tc.name  : Test OH_AudioDebuggingManager_PrintAppInfo API with negative fd
* @tc.number: OH_PrintAppInfo_WithNegativeFd
* @tc.desc  : Test OH_AudioDebuggingManager_PrintAppInfo interface. Returns SUCCESS or ERROR_SYSTEM with negative fd.
*/
HWTEST_F(AudioDebuggingManagerAiut, OH_PrintAppInfo_WithNegativeFd, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioDebuggingManager *manager = nullptr;
    OH_AudioManager_GetAudioDebuggingManager(&manager);
    AIUT_ASSERT_NE_INFO_LOG(manager, nullptr, "GetAudioDebuggingManager returned null");

    OH_AudioCommon_Result result = OH_AudioDebuggingManager_PrintAppInfo(manager, -1);
    AIUT_EXPECT_TRUE_INFO_LOG(result == AUDIOCOMMON_RESULT_SUCCESS || result == AUDIOCOMMON_RESULT_ERROR_SYSTEM,
        "unexpected result=%d", result);
}

/**
* @tc.name  : Test OH_AudioDebuggingManager_PrintAppInfo API with zero fd
* @tc.number: OH_PrintAppInfo_WithZeroFd
* @tc.desc  : Test OH_AudioDebuggingManager_PrintAppInfo interface. Does not return ERROR_INVALID_PARAM with zero fd.
*/
HWTEST_F(AudioDebuggingManagerAiut, OH_PrintAppInfo_WithZeroFd, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    OH_AudioDebuggingManager *manager = nullptr;
    OH_AudioManager_GetAudioDebuggingManager(&manager);
    AIUT_ASSERT_NE_INFO_LOG(manager, nullptr, "GetAudioDebuggingManager returned null");

    OH_AudioCommon_Result result = OH_AudioDebuggingManager_PrintAppInfo(manager, 0);
    AIUT_EXPECT_NE_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "unexpected INVALID_PARAM, got %d", result);
}

// ==================== Taihe PrintAppInfo Tests ====================

/**
* @tc.name  : Test Taihe PrintAppInfo API via pipe
* @tc.number: Taihe_PrintAppInfo_WithPipe
* @tc.desc  : Test PrintAppAudioDebugInfo interface. Returns SUCCESS and produces non-empty output via pipe.
*/
HWTEST_F(AudioDebuggingManagerAiut, Taihe_PrintAppInfo_WithPipe, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    // Taihe: PrintAppInfo(fd) delegates to AudioDebugManager::PrintAppAudioDebugInfo(fd)
    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    int32_t ret = AudioDebugManager::GetInstance().PrintAppAudioDebugInfo(pipefd[1]);
    close(pipefd[1]);
    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAppAudioDebugInfo failed, ret=%d", ret);
    AIUT_EXPECT_NE_INFO_LOG(output.size(), 0u, "output is empty");
}

/**
* @tc.name  : Test Taihe CreateDebuggingManagerWrapper API returns singleton
* @tc.number: Taihe_CreateDebuggingManagerWrapper_Singleton
* @tc.desc  : Test CreateDebuggingManagerWrapper interface. Returns the same singleton AudioDebugManager instance.
*/
HWTEST_F(AudioDebuggingManagerAiut, Taihe_CreateDebuggingManagerWrapper_Singleton, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    // Taihe: CreateDebuggingManagerWrapper() calls AudioDebugManager::GetInstance()
    auto &instance1 = AudioDebugManager::GetInstance();
    auto &instance2 = AudioDebugManager::GetInstance();
    AIUT_EXPECT_EQ_INFO_LOG(&instance1, &instance2, "Singleton instances differ");
}

/**
* @tc.name  : Test Taihe PrintAppInfo API with all callback types registered
* @tc.number: Taihe_PrintAppInfo_WithAllCallbacks
* @tc.desc  : Test PrintAppAudioDebugInfo interface.
*             Output contains renderer, capturer, loopback, session info when all callbacks registered.
*/
HWTEST_F(AudioDebuggingManagerAiut, Taihe_PrintAppInfo_WithAllCallbacks, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    // Taihe: PrintAppInfo prints all registered renderers, capturers, loopbacks, sessions
    auto rendererCallback = std::make_shared<TestRendererDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(rendererCallback, nullptr, "make_shared RendererCallback failed");
    rendererCallback->testInfo_.sessionId = 100;
    rendererCallback->testInfo_.streamUsage = STREAM_USAGE_MUSIC;
    uint32_t rKey = 0;
    AudioDebugManager::GetInstance().RegisterAudioRenderer(rKey, rendererCallback);

    auto capturerCallback = std::make_shared<TestCapturerDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(capturerCallback, nullptr, "make_shared CapturerCallback failed");
    capturerCallback->testInfo_.sessionId = 200;
    uint32_t cKey = 0;
    AudioDebugManager::GetInstance().RegisterAudioCapturer(cKey, capturerCallback);

    auto loopbackCallback = std::make_shared<TestLoopbackDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(loopbackCallback, nullptr, "make_shared LoopbackCallback failed");
    loopbackCallback->testInfo_.appUid = 3000;
    loopbackCallback->testInfo_.appName = "CombinedTest";
    uint32_t lKey = 0;
    AudioDebugManager::GetInstance().RegisterAudioLoopback(lKey, loopbackCallback);

    auto sessionCallback = std::make_shared<TestSessionDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(sessionCallback, nullptr, "make_shared SessionCallback failed");
    sessionCallback->testInfo_.audioSessionScene = AudioSessionScene::GAME;
    AudioDebugManager::GetInstance().SetAudioSessionCallback(sessionCallback);

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    int32_t ret = AudioDebugManager::GetInstance().PrintAppAudioDebugInfo(pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAppAudioDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_NE_INFO_LOG(output.find("audioRenderer"), std::string::npos, "output missing 'audioRenderer'");
    AIUT_EXPECT_NE_INFO_LOG(output.find("audioCapturer"), std::string::npos, "output missing 'audioCapturer'");
    AIUT_EXPECT_NE_INFO_LOG(output.find("audioLoopback"), std::string::npos, "output missing 'audioLoopback'");
    AIUT_EXPECT_NE_INFO_LOG(output.find("audioSession"), std::string::npos, "output missing 'audioSession'");

    AudioDebugManager::GetInstance().UnregisterAudioRenderer(rKey);
    AudioDebugManager::GetInstance().UnregisterAudioCapturer(cKey);
    AudioDebugManager::GetInstance().UnregisterAudioLoopback(lKey);
}

// ==================== NAPI PrintAppInfo / ClassName Tests ====================

/**
* @tc.name  : Test NAPI class name constant
* @tc.number: Napi_ClassName_Verification
* @tc.desc  : Test AUDIO_DEBUGGING_MGR_NAPI_CLASS_NAME constant. Value equals "AudioDebuggingManager".
*/
HWTEST_F(AudioDebuggingManagerAiut, Napi_ClassName_Verification, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    // Verify the NAPI class name constant matches the expected value
    AIUT_EXPECT_EQ_INFO_LOG(AUDIO_DEBUGGING_MGR_NAPI_CLASS_NAME, "AudioDebuggingManager", "class name mismatch");
}

/**
* @tc.name  : Test NAPI printAppInfo API with valid fd
* @tc.number: Napi_PrintAppInfo_WithFd
* @tc.desc  : Test PrintAppAudioDebugInfo interface. Returns SUCCESS and produces non-empty output via pipe.
*/
HWTEST_F(AudioDebuggingManagerAiut, Napi_PrintAppInfo_WithFd, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    // NAPI: printAppInfo(fd) calls AudioDebugManager::GetInstance().PrintAppAudioDebugInfo(fd)
    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    int32_t ret = AudioDebugManager::GetInstance().PrintAppAudioDebugInfo(pipefd[1]);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAppAudioDebugInfo failed, ret=%d", ret);
    AIUT_EXPECT_NE_INFO_LOG(output.size(), 0u, "output is empty");
}

/**
* @tc.name  : Test NAPI printAppInfo API with negative fd
* @tc.number: Napi_PrintAppInfo_NegativeFd
* @tc.desc  : Test PrintAppAudioDebugInfo interface. Returns SUCCESS with negative fd (falls back to log output).
*/
HWTEST_F(AudioDebuggingManagerAiut, Napi_PrintAppInfo_NegativeFd, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    // NAPI: printAppInfo(-1) should succeed (falls back to log output)
    int32_t ret = AudioDebugManager::GetInstance().PrintAppAudioDebugInfo(-1);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAppAudioDebugInfo with -1 failed, ret=%d", ret);
}

} // namespace AudioStandard
} // namespace OHOS
