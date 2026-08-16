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

AudioCapturerDebugInfo CreateTestCapturerDebugInfo()
{
    AudioCapturerDebugInfo info;
    info.sessionId = TEST_SESSION_ID + 1;
    info.samplingRate = TEST_CAPTURER_SAMPLING_RATE;
    info.channels = AudioChannel::MONO;
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
 * @tc.owner : audio_framework_common
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
 * @tc.owner : audio_framework_common
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
 * @tc.owner : audio_framework_common
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
 * @tc.owner : audio_framework_common
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
 * @tc.owner : audio_framework_common
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
 * @tc.owner : audio_framework_common
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
 * @tc.owner : audio_framework_common
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
 * @tc.owner : audio_framework_common
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
 * @tc.owner : audio_framework_common
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
 * @tc.owner : audio_framework_common
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
 * @tc.owner : audio_framework_common
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
 * @tc.owner : audio_framework_common
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
 * @tc.owner : audio_framework_common
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
 * @tc.owner : audio_framework_common
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
 * @tc.owner : audio_framework_common
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
 * @tc.owner : audio_framework_common
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

// ==================== NAPI PrintAppInfo Tests ====================

/**
* @tc.name  : Test NAPI printAppInfo API with valid fd
* @tc.number: Napi_PrintAppInfo_WithFd
* @tc.desc  : Test PrintAppAudioDebugInfo interface. Returns SUCCESS and produces non-empty output via pipe.
 * @tc.owner : audio_framework_common
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
 * @tc.owner : audio_framework_common
*/
HWTEST_F(AudioDebuggingManagerAiut, Napi_PrintAppInfo_NegativeFd, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    // NAPI: printAppInfo(-1) should succeed (falls back to log output)
    int32_t ret = AudioDebugManager::GetInstance().PrintAppAudioDebugInfo(-1);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAppAudioDebugInfo with -1 failed, ret=%d", ret);
}

// ==================== OHAudio C API PrintRendererInfo Tests ====================

/**
* @tc.name  : Test OH_AudioDebuggingManager_PrintRendererInfo API with null manager
* @tc.number: OH_PrintRendererInfo_NullManager
* @tc.desc  : Test OH_AudioDebuggingManager_PrintRendererInfo interface.
 * @tc.owner : audio_framework_common
*             Returns AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM when manager is null.
*/
HWTEST_F(AudioDebuggingManagerAiut, OH_PrintRendererInfo_NullManager, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioRenderer *renderer = nullptr;
    OH_AudioCommon_Result result = OH_AudioDebuggingManager_PrintRendererInfo(nullptr, renderer, -1);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "expected INVALID_PARAM, got %d", result);
}

/**
* @tc.name  : Test OH_AudioDebuggingManager_PrintRendererInfo API with null renderer
* @tc.number: OH_PrintRendererInfo_NullRenderer
* @tc.desc  : Test OH_AudioDebuggingManager_PrintRendererInfo interface.
 * @tc.owner : audio_framework_common
*             Returns AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM when renderer is null.
*/
HWTEST_F(AudioDebuggingManagerAiut, OH_PrintRendererInfo_NullRenderer, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioDebuggingManager *manager = nullptr;
    OH_AudioManager_GetAudioDebuggingManager(&manager);
    AIUT_ASSERT_NE_INFO_LOG(manager, nullptr, "GetAudioDebuggingManager returned null");

    OH_AudioCommon_Result result = OH_AudioDebuggingManager_PrintRendererInfo(manager, nullptr, -1);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "expected INVALID_PARAM, got %d", result);
}

/**
* @tc.name  : Test OH_AudioDebuggingManager_PrintRendererInfo API with invalid renderer
* @tc.number: OH_PrintRendererInfo_InvalidRenderer
* @tc.desc  : Test OH_AudioDebuggingManager_PrintRendererInfo interface.
 * @tc.owner : audio_framework_common
*             Returns AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE when renderer is invalid.
*/
HWTEST_F(AudioDebuggingManagerAiut, OH_PrintRendererInfo_InvalidRenderer, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioDebuggingManager *manager = nullptr;
    OH_AudioManager_GetAudioDebuggingManager(&manager);
    AIUT_ASSERT_NE_INFO_LOG(manager, nullptr, "GetAudioDebuggingManager returned null");

    // Create a fake renderer pointer (not a valid OHAudioRenderer)
    OH_AudioRenderer *fakeRenderer = reinterpret_cast<OH_AudioRenderer *>(0x1234);
    OH_AudioCommon_Result result = OH_AudioDebuggingManager_PrintRendererInfo(manager, fakeRenderer, -1);
    AIUT_EXPECT_TRUE_INFO_LOG(result == AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE ||
        result == AUDIOCOMMON_RESULT_ERROR_SYSTEM,
        "unexpected result=%d", result);
}

// ==================== OHAudio C API PrintCapturerInfo Tests ====================

/**
* @tc.name  : Test OH_AudioDebuggingManager_PrintCapturerInfo API with null manager
* @tc.number: OH_PrintCapturerInfo_NullManager
* @tc.desc  : Test OH_AudioDebuggingManager_PrintCapturerInfo interface.
 * @tc.owner : audio_framework_common
*             Returns AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM when manager is null.
*/
HWTEST_F(AudioDebuggingManagerAiut, OH_PrintCapturerInfo_NullManager, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioCapturer *capturer = nullptr;
    OH_AudioCommon_Result result = OH_AudioDebuggingManager_PrintCapturerInfo(nullptr, capturer, -1);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "expected INVALID_PARAM, got %d", result);
}

/**
* @tc.name  : Test OH_AudioDebuggingManager_PrintCapturerInfo API with null capturer
* @tc.number: OH_PrintCapturerInfo_NullCapturer
* @tc.desc  : Test OH_AudioDebuggingManager_PrintCapturerInfo interface.
 * @tc.owner : audio_framework_common
*             Returns AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM when capturer is null.
*/
HWTEST_F(AudioDebuggingManagerAiut, OH_PrintCapturerInfo_NullCapturer, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioDebuggingManager *manager = nullptr;
    OH_AudioManager_GetAudioDebuggingManager(&manager);
    AIUT_ASSERT_NE_INFO_LOG(manager, nullptr, "GetAudioDebuggingManager returned null");

    OH_AudioCommon_Result result = OH_AudioDebuggingManager_PrintCapturerInfo(manager, nullptr, -1);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "expected INVALID_PARAM, got %d", result);
}

/**
* @tc.name  : Test OH_AudioDebuggingManager_PrintCapturerInfo API with invalid capturer
* @tc.number: OH_PrintCapturerInfo_InvalidCapturer
* @tc.desc  : Test OH_AudioDebuggingManager_PrintCapturerInfo interface.
 * @tc.owner : audio_framework_common
*             Returns AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE when capturer is invalid.
*/
HWTEST_F(AudioDebuggingManagerAiut, OH_PrintCapturerInfo_InvalidCapturer, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioDebuggingManager *manager = nullptr;
    OH_AudioManager_GetAudioDebuggingManager(&manager);
    AIUT_ASSERT_NE_INFO_LOG(manager, nullptr, "GetAudioDebuggingManager returned null");

    // Create a fake capturer pointer (not a valid OHAudioCapturer)
    OH_AudioCapturer *fakeCapturer = reinterpret_cast<OH_AudioCapturer *>(0x5678);
    OH_AudioCommon_Result result = OH_AudioDebuggingManager_PrintCapturerInfo(manager, fakeCapturer, -1);
    AIUT_EXPECT_TRUE_INFO_LOG(result == AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE ||
        result == AUDIOCOMMON_RESULT_ERROR_SYSTEM,
        "unexpected result=%d", result);
}

// ==================== OHAudio C API PrintSessionInfo Tests ====================

/**
* @tc.name  : Test OH_AudioDebuggingManager_PrintSessionInfo API with null manager
* @tc.number: OH_PrintSessionInfo_NullManager
* @tc.desc  : Test OH_AudioDebuggingManager_PrintSessionInfo interface.
 * @tc.owner : audio_framework_common
*             Returns AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM when manager is null.
*/
HWTEST_F(AudioDebuggingManagerAiut, OH_PrintSessionInfo_NullManager, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioSessionManager *session = nullptr;
    OH_AudioCommon_Result result = OH_AudioDebuggingManager_PrintSessionInfo(nullptr, session, -1);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "expected INVALID_PARAM, got %d", result);
}

/**
* @tc.name  : Test OH_AudioDebuggingManager_PrintSessionInfo API with null session
* @tc.number: OH_PrintSessionInfo_NullSession
* @tc.desc  : Test OH_AudioDebuggingManager_PrintSessionInfo interface.
 * @tc.owner : audio_framework_common
*             Returns AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM when session is null.
*/
HWTEST_F(AudioDebuggingManagerAiut, OH_PrintSessionInfo_NullSession, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioDebuggingManager *manager = nullptr;
    OH_AudioManager_GetAudioDebuggingManager(&manager);
    AIUT_ASSERT_NE_INFO_LOG(manager, nullptr, "GetAudioDebuggingManager returned null");

    OH_AudioCommon_Result result = OH_AudioDebuggingManager_PrintSessionInfo(manager, nullptr, -1);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "expected INVALID_PARAM, got %d", result);
}

/**
* @tc.name  : Test OH_AudioDebuggingManager_PrintSessionInfo API with invalid session
* @tc.number: OH_PrintSessionInfo_InvalidSession
* @tc.desc  : Test OH_AudioDebuggingManager_PrintSessionInfo interface.
 * @tc.owner : audio_framework_common
*             Returns AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM when session is invalid.
*/
HWTEST_F(AudioDebuggingManagerAiut, OH_PrintSessionInfo_InvalidSession, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioDebuggingManager *manager = nullptr;
    OH_AudioManager_GetAudioDebuggingManager(&manager);
    AIUT_ASSERT_NE_INFO_LOG(manager, nullptr, "GetAudioDebuggingManager returned null");

    // Create a fake session pointer (not a valid OHAudioSessionManager)
    OH_AudioSessionManager *fakeSession = reinterpret_cast<OH_AudioSessionManager *>(0x9ABC);
    OH_AudioCommon_Result result = OH_AudioDebuggingManager_PrintSessionInfo(manager, fakeSession, -1);
    AIUT_EXPECT_TRUE_INFO_LOG(result == AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM ||
        result == AUDIOCOMMON_RESULT_ERROR_SYSTEM,
        "unexpected result=%d", result);
}

/**
* @tc.name  : Test OH_AudioDebuggingManager_PrintSessionInfo API with valid session via pipe
* @tc.number: OH_PrintSessionInfo_WithPipe
* @tc.desc  : Test OH_AudioDebuggingManager_PrintSessionInfo interface.
 * @tc.owner : audio_framework_common
*             Returns SUCCESS or ERROR_SYSTEM when session callback is registered.
*/
HWTEST_F(AudioDebuggingManagerAiut, OH_PrintSessionInfo_WithPipe, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    OH_AudioDebuggingManager *manager = nullptr;
    OH_AudioManager_GetAudioDebuggingManager(&manager);
    AIUT_ASSERT_NE_INFO_LOG(manager, nullptr, "GetAudioDebuggingManager returned null");

    // Register a session callback so PrintSessionInfo has data to print
    auto sessionCallback = std::make_shared<TestSessionDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(sessionCallback, nullptr, "make_shared SessionCallback failed");
    sessionCallback->testInfo_ = CreateTestSessionDebugInfo();
    AudioDebugManager::GetInstance().SetAudioSessionCallback(sessionCallback);

    // Use a fake session pointer - ConvertSessionManager does reinterpret_cast, so it won't be null
    // The actual session pointer value doesn't matter since PrintAudioSessionDebugInfo uses the registered callback
    OH_AudioSessionManager *fakeSession = reinterpret_cast<OH_AudioSessionManager *>(0x1000);

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    OH_AudioCommon_Result result = OH_AudioDebuggingManager_PrintSessionInfo(manager, fakeSession, pipefd[1]);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_TRUE_INFO_LOG(result == AUDIOCOMMON_RESULT_SUCCESS ||
        result == AUDIOCOMMON_RESULT_ERROR_SYSTEM,
        "unexpected result=%d", result);
}

// ==================== WriteAudioDebugInfo Edge Cases ====================

/**
* @tc.name  : Test WriteAudioDebugInfo with large debug info via pipe
* @tc.number: AudioDebugManager_WriteDebugInfo_Large
* @tc.desc  : Test WriteAudioDebugInfo interface. Handles large output correctly.
 * @tc.owner : audio_framework_common
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_WriteDebugInfo_Large, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto rendererCallback = std::make_shared<TestRendererDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(rendererCallback, nullptr, "make_shared RendererCallback failed");
    rendererCallback->testInfo_ = CreateTestRendererDebugInfo();
    // Add multiple focus history entries to make output larger
    for (int i = 0; i < 10; ++i) {
        AudioFocusBehavior behavior;
        behavior.streamId = TEST_STREAM_DEBUG_ID + i;
        behavior.pid = TEST_PID + i;
        behavior.uid = TEST_APP_UID + i;
        rendererCallback->testInfo_.focusHistory.push_back(behavior);
    }
    uint32_t rendererKey = 0;
    AudioDebugManager::GetInstance().RegisterAudioRenderer(rendererKey, rendererCallback);

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    int32_t ret = AudioDebugManager::GetInstance().PrintAudioRendererDebugInfo(rendererKey, pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAudioRendererDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_NE_INFO_LOG(output.find("audioRenderer"), std::string::npos, "output missing 'audioRenderer'");
    AIUT_EXPECT_NE_INFO_LOG(output.find("focusHistory"), std::string::npos, "output missing 'focusHistory'");

    AudioDebugManager::GetInstance().UnregisterAudioRenderer(rendererKey);
}

// ==================== Unregister Edge Cases ====================

/**
* @tc.name  : Test UnregisterAudioRenderer with non-existent key
* @tc.number: AudioDebugManager_UnregisterRenderer_NotFound
* @tc.desc  : Test UnregisterAudioRenderer interface. Returns ERR_ILLEGAL_STATE for non-existent key.
 * @tc.owner : audio_framework_common
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_UnregisterRenderer_NotFound, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int32_t ret = AudioDebugManager::GetInstance().UnregisterAudioRenderer(99999);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_ILLEGAL_STATE, "expected ERR_ILLEGAL_STATE, got %d", ret);
}

/**
* @tc.name  : Test UnregisterAudioCapturer with non-existent key
* @tc.number: AudioDebugManager_UnregisterCapturer_NotFound
* @tc.desc  : Test UnregisterAudioCapturer interface. Returns ERR_ILLEGAL_STATE for non-existent key.
 * @tc.owner : audio_framework_common
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_UnregisterCapturer_NotFound, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int32_t ret = AudioDebugManager::GetInstance().UnregisterAudioCapturer(99999);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_ILLEGAL_STATE, "expected ERR_ILLEGAL_STATE, got %d", ret);
}

/**
* @tc.name  : Test UnregisterAudioLoopback with non-existent key
* @tc.number: AudioDebugManager_UnregisterLoopback_NotFound
* @tc.desc  : Test UnregisterAudioLoopback interface. UnregisterAudioLoopback always returns SUCCESS.
 * @tc.owner : audio_framework_common
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_UnregisterLoopback_NotFound, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int32_t ret = AudioDebugManager::GetInstance().UnregisterAudioLoopback(99999);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "expected SUCCESS, got %d", ret);
}

/**
* @tc.name  : Test RegisterAudioRenderer with null callback
* @tc.number: AudioDebugManager_RegisterRenderer_NullCallback
* @tc.desc  : Test RegisterAudioRenderer interface. Returns ERR_INVALID_PARAM when callback is null.
 * @tc.owner : audio_framework_common
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_RegisterRenderer_NullCallback, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    uint32_t key = 0;
    int32_t ret = AudioDebugManager::GetInstance().RegisterAudioRenderer(key, nullptr);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM, "expected ERR_INVALID_PARAM, got %d", ret);
}

/**
* @tc.name  : Test RegisterAudioCapturer with null callback
* @tc.number: AudioDebugManager_RegisterCapturer_NullCallback
* @tc.desc  : Test RegisterAudioCapturer interface. Returns ERR_INVALID_PARAM when callback is null.
 * @tc.owner : audio_framework_common
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_RegisterCapturer_NullCallback, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    uint32_t key = 0;
    int32_t ret = AudioDebugManager::GetInstance().RegisterAudioCapturer(key, nullptr);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM, "expected ERR_INVALID_PARAM, got %d", ret);
}

/**
* @tc.name  : Test RegisterAudioLoopback with null callback
* @tc.number: AudioDebugManager_RegisterLoopback_NullCallback
* @tc.desc  : Test RegisterAudioLoopback interface. Returns ERR_INVALID_PARAM when callback is null.
 * @tc.owner : audio_framework_common
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_RegisterLoopback_NullCallback, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    uint32_t key = 0;
    int32_t ret = AudioDebugManager::GetInstance().RegisterAudioLoopback(key, nullptr);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM, "expected ERR_INVALID_PARAM, got %d", ret);
}

/**
* @tc.name  : Test SetAudioSessionCallback with null callback
* @tc.number: AudioDebugManager_SetSessionCallback_Null
* @tc.desc  : Test SetAudioSessionCallback interface. Returns ERR_INVALID_PARAM when callback is null.
 * @tc.owner : audio_framework_common
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_SetSessionCallback_Null, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int32_t ret = AudioDebugManager::GetInstance().SetAudioSessionCallback(nullptr);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM, "expected ERR_INVALID_PARAM, got %d", ret);
}

// ==================== GetDebugInfo Error Paths ====================

/**
* @tc.name  : Test GetAudioRendererDebugInfo with non-existent key
* @tc.number: AudioDebugManager_GetRendererDebugInfo_NotFound
* @tc.desc  : Test GetAudioRendererDebugInfo interface. Returns ERR_ILLEGAL_STATE for non-existent key.
 * @tc.owner : audio_framework_common
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_GetRendererDebugInfo_NotFound, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AudioRendererDebugInfo debugInfo;
    auto &privateRef = static_cast<AudioDebugManagerPrivate &>(AudioDebugManager::GetInstance());
    int32_t ret = privateRef.GetAudioRendererDebugInfo(99999, debugInfo);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_ILLEGAL_STATE, "expected ERR_ILLEGAL_STATE, got %d", ret);
}

/**
* @tc.name  : Test GetAudioCapturerDebugInfo with non-existent key
* @tc.number: AudioDebugManager_GetCapturerDebugInfo_NotFound
* @tc.desc  : Test GetAudioCapturerDebugInfo interface. Returns ERR_ILLEGAL_STATE for non-existent key.
 * @tc.owner : audio_framework_common
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_GetCapturerDebugInfo_NotFound, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AudioCapturerDebugInfo debugInfo;
    auto &privateRef = static_cast<AudioDebugManagerPrivate &>(AudioDebugManager::GetInstance());
    int32_t ret = privateRef.GetAudioCapturerDebugInfo(99999, debugInfo);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_ILLEGAL_STATE, "expected ERR_ILLEGAL_STATE, got %d", ret);
}

/**
* @tc.name  : Test GetAudioLoopbackDebugInfo with non-existent key
* @tc.number: AudioDebugManager_GetLoopbackDebugInfo_NotFound
* @tc.desc  : Test GetAudioLoopbackDebugInfo interface. Returns ERR_ILLEGAL_STATE for non-existent key.
 * @tc.owner : audio_framework_common
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_GetLoopbackDebugInfo_NotFound, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AudioLoopbackDebugInfo debugInfo;
    auto &privateRef = static_cast<AudioDebugManagerPrivate &>(AudioDebugManager::GetInstance());
    int32_t ret = privateRef.GetAudioLoopbackDebugInfo(99999, debugInfo);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_ILLEGAL_STATE, "expected ERR_ILLEGAL_STATE, got %d", ret);
}

/**
* @tc.name  : Test GetAudioSessionDebugInfo without callback registered
* @tc.number: AudioDebugManager_GetSessionDebugInfo_NoCallback
* @tc.desc  : Test GetAudioSessionDebugInfo interface. Returns ERR_ILLEGAL_STATE when no callback registered.
 * @tc.owner : audio_framework_common
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_GetSessionDebugInfo_NoCallback, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    // First set null to clear any existing callback
    AudioDebugManager::GetInstance().SetAudioSessionCallback(nullptr);
    AudioSessionDebugInfo debugInfo;
    auto &privateRef = static_cast<AudioDebugManagerPrivate &>(AudioDebugManager::GetInstance());
    int32_t ret = privateRef.GetAudioSessionDebugInfo(debugInfo);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_ILLEGAL_STATE, "expected ERR_ILLEGAL_STATE, got %d", ret);
}

// ==================== PrintDebugInfo Error Paths ====================

/**
* @tc.name  : Test PrintAudioRendererDebugInfo with non-existent key
* @tc.number: AudioDebugManager_PrintRendererDebugInfo_NotFound
* @tc.desc  : Test PrintAudioRendererDebugInfo interface. Returns ERR_ILLEGAL_STATE for non-existent key.
 * @tc.owner : audio_framework_common
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_PrintRendererDebugInfo_NotFound, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int32_t ret = AudioDebugManager::GetInstance().PrintAudioRendererDebugInfo(99999, -1);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_ILLEGAL_STATE, "expected ERR_ILLEGAL_STATE, got %d", ret);
}

/**
* @tc.name  : Test PrintAudioCapturerDebugInfo with non-existent key
* @tc.number: AudioDebugManager_PrintCapturerDebugInfo_NotFound
* @tc.desc  : Test PrintAudioCapturerDebugInfo interface. Returns ERR_ILLEGAL_STATE for non-existent key.
 * @tc.owner : audio_framework_common
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_PrintCapturerDebugInfo_NotFound, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int32_t ret = AudioDebugManager::GetInstance().PrintAudioCapturerDebugInfo(99999, -1);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_ILLEGAL_STATE, "expected ERR_ILLEGAL_STATE, got %d", ret);
}

/**
* @tc.name  : Test PrintAudioLoopbackDebugInfo with non-existent key
* @tc.number: AudioDebugManager_PrintLoopbackDebugInfo_NotFound
* @tc.desc  : Test PrintAudioLoopbackDebugInfo interface. Returns ERR_ILLEGAL_STATE for non-existent key.
 * @tc.owner : audio_framework_common
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_PrintLoopbackDebugInfo_NotFound, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int32_t ret = AudioDebugManager::GetInstance().PrintAudioLoopbackDebugInfo(99999, -1);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_ILLEGAL_STATE, "expected ERR_ILLEGAL_STATE, got %d", ret);
}

/**
* @tc.name  : Test PrintAudioSessionDebugInfo without callback
* @tc.number: AudioDebugManager_PrintSessionDebugInfo_NoCallback
* @tc.desc  : Test PrintAudioSessionDebugInfo interface. Returns ERR_ILLEGAL_STATE when no callback registered.
 * @tc.owner : audio_framework_common
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_PrintSessionDebugInfo_NoCallback, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    // First set null to clear any existing callback
    AudioDebugManager::GetInstance().SetAudioSessionCallback(nullptr);
    int32_t ret = AudioDebugManager::GetInstance().PrintAudioSessionDebugInfo(-1);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_ILLEGAL_STATE, "expected ERR_ILLEGAL_STATE, got %d", ret);
}

// ==================== PrintAllDebugInfo Tests ====================

/**
* @tc.name  : Test PrintAllAudioRenderersDebugInfo with registered renderers
* @tc.number: AudioDebugManager_PrintAllRenderers_WithCallback
* @tc.desc  : Test PrintAllAudioRenderersDebugInfo interface. Returns SUCCESS when renderer callback registered.
 * @tc.owner : audio_framework_common
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_PrintAllRenderers_WithCallback, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto rendererCallback = std::make_shared<TestRendererDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(rendererCallback, nullptr, "make_shared RendererCallback failed");
    rendererCallback->testInfo_ = CreateTestRendererDebugInfo();
    uint32_t rendererKey = 0;
    AudioDebugManager::GetInstance().RegisterAudioRenderer(rendererKey, rendererCallback);

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    auto &privateRef = static_cast<AudioDebugManagerPrivate &>(AudioDebugManager::GetInstance());
    int32_t ret = privateRef.PrintAllAudioRenderersDebugInfo(pipefd[1]);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAllAudioRenderersDebugInfo failed, ret=%d", ret);
    AIUT_EXPECT_NE_INFO_LOG(output.find("audioRenderer"), std::string::npos, "output missing 'audioRenderer'");

    AudioDebugManager::GetInstance().UnregisterAudioRenderer(rendererKey);
}

/**
* @tc.name  : Test PrintAllAudioCapturersDebugInfo with registered capturers
* @tc.number: AudioDebugManager_PrintAllCapturers_WithCallback
* @tc.desc  : Test PrintAllAudioCapturersDebugInfo interface. Returns SUCCESS when capturer callback registered.
 * @tc.owner : audio_framework_common
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_PrintAllCapturers_WithCallback, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto capturerCallback = std::make_shared<TestCapturerDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(capturerCallback, nullptr, "make_shared CapturerCallback failed");
    capturerCallback->testInfo_ = CreateTestCapturerDebugInfo();
    uint32_t capturerKey = 0;
    AudioDebugManager::GetInstance().RegisterAudioCapturer(capturerKey, capturerCallback);

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    auto &privateRef = static_cast<AudioDebugManagerPrivate &>(AudioDebugManager::GetInstance());
    int32_t ret = privateRef.PrintAllAudioCapturersDebugInfo(pipefd[1]);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAllAudioCapturersDebugInfo failed, ret=%d", ret);
    AIUT_EXPECT_NE_INFO_LOG(output.find("audioCapturer"), std::string::npos, "output missing 'audioCapturer'");

    AudioDebugManager::GetInstance().UnregisterAudioCapturer(capturerKey);
}

/**
* @tc.name  : Test PrintAllAudioLoopbacksDebugInfo with registered loopbacks
* @tc.number: AudioDebugManager_PrintAllLoopbacks_WithCallback
* @tc.desc  : Test PrintAllAudioLoopbacksDebugInfo interface. Returns SUCCESS when loopback callback registered.
 * @tc.owner : audio_framework_common
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_PrintAllLoopbacks_WithCallback, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto loopbackCallback = std::make_shared<TestLoopbackDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(loopbackCallback, nullptr, "make_shared LoopbackCallback failed");
    loopbackCallback->testInfo_.appUid = TEST_APP_UID;
    loopbackCallback->testInfo_.appName = "TestLoopback";
    uint32_t loopbackKey = 0;
    AudioDebugManager::GetInstance().RegisterAudioLoopback(loopbackKey, loopbackCallback);

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    auto &privateRef = static_cast<AudioDebugManagerPrivate &>(AudioDebugManager::GetInstance());
    int32_t ret = privateRef.PrintAllAudioLoopbacksDebugInfo(pipefd[1]);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAllAudioLoopbacksDebugInfo failed, ret=%d", ret);
    AIUT_EXPECT_NE_INFO_LOG(output.find("audioLoopback"), std::string::npos, "output missing 'audioLoopback'");

    AudioDebugManager::GetInstance().UnregisterAudioLoopback(loopbackKey);
}

// ==================== Callback Error Return Tests ====================

/**
* @tc.name  : Test GetAudioRendererDebugInfo with callback returning error
* @tc.number: AudioDebugManager_GetRendererDebugInfo_CallbackError
* @tc.desc  : Test GetAudioRendererDebugInfo interface. Returns error when callback returns error.
 * @tc.owner : audio_framework_common
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_GetRendererDebugInfo_CallbackError, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto rendererCallback = std::make_shared<TestRendererDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(rendererCallback, nullptr, "make_shared RendererCallback failed");
    rendererCallback->retCode_ = ERR_OPERATION_FAILED;
    uint32_t rendererKey = 0;
    AudioDebugManager::GetInstance().RegisterAudioRenderer(rendererKey, rendererCallback);

    AudioRendererDebugInfo debugInfo;
    auto &privateRef = static_cast<AudioDebugManagerPrivate &>(AudioDebugManager::GetInstance());
    int32_t ret = privateRef.GetAudioRendererDebugInfo(rendererKey, debugInfo);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_OPERATION_FAILED, "expected ERR_OPERATION_FAILED, got %d", ret);

    AudioDebugManager::GetInstance().UnregisterAudioRenderer(rendererKey);
}

/**
* @tc.name  : Test GetAudioCapturerDebugInfo with callback returning error
* @tc.number: AudioDebugManager_GetCapturerDebugInfo_CallbackError
* @tc.desc  : Test GetAudioCapturerDebugInfo interface. Returns error when callback returns error.
 * @tc.owner : audio_framework_common
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_GetCapturerDebugInfo_CallbackError, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto capturerCallback = std::make_shared<TestCapturerDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(capturerCallback, nullptr, "make_shared CapturerCallback failed");
    capturerCallback->retCode_ = ERR_OPERATION_FAILED;
    uint32_t capturerKey = 0;
    AudioDebugManager::GetInstance().RegisterAudioCapturer(capturerKey, capturerCallback);

    AudioCapturerDebugInfo debugInfo;
    auto &privateRef = static_cast<AudioDebugManagerPrivate &>(AudioDebugManager::GetInstance());
    int32_t ret = privateRef.GetAudioCapturerDebugInfo(capturerKey, debugInfo);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_OPERATION_FAILED, "expected ERR_OPERATION_FAILED, got %d", ret);

    AudioDebugManager::GetInstance().UnregisterAudioCapturer(capturerKey);
}

/**
* @tc.name  : Test GetAudioLoopbackDebugInfo with callback returning error
* @tc.number: AudioDebugManager_GetLoopbackDebugInfo_CallbackError
* @tc.desc  : Test GetAudioLoopbackDebugInfo interface. Returns error when callback returns error.
 * @tc.owner : audio_framework_common
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_GetLoopbackDebugInfo_CallbackError, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto loopbackCallback = std::make_shared<TestLoopbackDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(loopbackCallback, nullptr, "make_shared LoopbackCallback failed");
    loopbackCallback->retCode_ = ERR_OPERATION_FAILED;
    uint32_t loopbackKey = 0;
    AudioDebugManager::GetInstance().RegisterAudioLoopback(loopbackKey, loopbackCallback);

    AudioLoopbackDebugInfo debugInfo;
    auto &privateRef = static_cast<AudioDebugManagerPrivate &>(AudioDebugManager::GetInstance());
    int32_t ret = privateRef.GetAudioLoopbackDebugInfo(loopbackKey, debugInfo);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_OPERATION_FAILED, "expected ERR_OPERATION_FAILED, got %d", ret);

    AudioDebugManager::GetInstance().UnregisterAudioLoopback(loopbackKey);
}

/**
* @tc.name  : Test GetAudioSessionDebugInfo with callback returning error
* @tc.number: AudioDebugManager_GetSessionDebugInfo_CallbackError
* @tc.desc  : Test GetAudioSessionDebugInfo interface. Returns error when callback returns error.
 * @tc.owner : audio_framework_common
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_GetSessionDebugInfo_CallbackError, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto sessionCallback = std::make_shared<TestSessionDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(sessionCallback, nullptr, "make_shared SessionCallback failed");
    sessionCallback->retCode_ = ERR_OPERATION_FAILED;
    AudioDebugManager::GetInstance().SetAudioSessionCallback(sessionCallback);

    AudioSessionDebugInfo debugInfo;
    auto &privateRef = static_cast<AudioDebugManagerPrivate &>(AudioDebugManager::GetInstance());
    int32_t ret = privateRef.GetAudioSessionDebugInfo(debugInfo);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_OPERATION_FAILED, "expected ERR_OPERATION_FAILED, got %d", ret);
}

// ==================== PrintfAudioDebugInfo Edge Cases ====================

/**
* @tc.name  : Test PrintfAudioDebugInfo with negative fd fallback
* @tc.number: AudioDebugManager_PrintfAudioDebugInfo_NegativeFd
* @tc.desc  : Test PrintfAudioDebugInfo interface. Falls back to log output with negative fd.
 * @tc.owner : audio_framework_common
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_PrintfAudioDebugInfo_NegativeFd, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &privateRef = static_cast<AudioDebugManagerPrivate &>(AudioDebugManager::GetInstance());
    int32_t ret = privateRef.PrintfAudioDebugInfo(-1, "test debug info");
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintfAudioDebugInfo with -1 failed, ret=%d", ret);
}

/**
* @tc.name  : Test PrintfAudioDebugInfo with valid fd via pipe
* @tc.number: AudioDebugManager_PrintfAudioDebugInfo_ValidFd
* @tc.desc  : Test PrintfAudioDebugInfo interface. Writes to fd and returns SUCCESS.
 * @tc.owner : audio_framework_common
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_PrintfAudioDebugInfo_ValidFd, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    auto &privateRef = static_cast<AudioDebugManagerPrivate &>(AudioDebugManager::GetInstance());
    int32_t ret = privateRef.PrintfAudioDebugInfo(pipefd[1], "test debug info content");
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintfAudioDebugInfo failed, ret=%d", ret);
    AIUT_EXPECT_NE_INFO_LOG(output.find("test debug info content"), std::string::npos, "output missing content");
}

// ==================== ParserDebugInfo Tests ====================

/**
* @tc.name  : Test ParserAudioRendererDebugInfo with default info
* @tc.number: AudioDebugManager_ParserRendererDebugInfo_Default
* @tc.desc  : Test ParserAudioRendererDebugInfo interface. Returns non-empty string with default info.
 * @tc.owner : audio_framework_common
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_ParserRendererDebugInfo_Default, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AudioRendererDebugInfo debugInfo;
    auto &privateRef = static_cast<AudioDebugManagerPrivate &>(AudioDebugManager::GetInstance());
    std::string result = privateRef.ParserAudioRendererDebugInfo(debugInfo);
    AIUT_EXPECT_FALSE_INFO_LOG(result.empty(), "ParserAudioRendererDebugInfo returned empty string");
    AIUT_EXPECT_NE_INFO_LOG(result.find("audioRenderer"), std::string::npos, "result missing 'audioRenderer'");
}

/**
* @tc.name  : Test ParserAudioCapturerDebugInfo with default info
* @tc.number: AudioDebugManager_ParserCapturerDebugInfo_Default
* @tc.desc  : Test ParserAudioCapturerDebugInfo interface. Returns non-empty string with default info.
 * @tc.owner : audio_framework_common
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_ParserCapturerDebugInfo_Default, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AudioCapturerDebugInfo debugInfo;
    auto &privateRef = static_cast<AudioDebugManagerPrivate &>(AudioDebugManager::GetInstance());
    std::string result = privateRef.ParserAudioCapturerDebugInfo(debugInfo);
    AIUT_EXPECT_FALSE_INFO_LOG(result.empty(), "ParserAudioCapturerDebugInfo returned empty string");
    AIUT_EXPECT_NE_INFO_LOG(result.find("audioCapturer"), std::string::npos, "result missing 'audioCapturer'");
}

/**
* @tc.name  : Test ParserAudioLoopbackDebugInfo with default info
* @tc.number: AudioDebugManager_ParserLoopbackDebugInfo_Default
* @tc.desc  : Test ParserAudioLoopbackDebugInfo interface. Returns non-empty string with default info.
 * @tc.owner : audio_framework_common
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_ParserLoopbackDebugInfo_Default, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AudioLoopbackDebugInfo debugInfo;
    auto &privateRef = static_cast<AudioDebugManagerPrivate &>(AudioDebugManager::GetInstance());
    std::string result = privateRef.ParserAudioLoopbackDebugInfo(debugInfo);
    AIUT_EXPECT_FALSE_INFO_LOG(result.empty(), "ParserAudioLoopbackDebugInfo returned empty string");
    AIUT_EXPECT_NE_INFO_LOG(result.find("audioLoopback"), std::string::npos, "result missing 'audioLoopback'");
}

/**
* @tc.name  : Test ParserAudioSessionDebugInfo with INVALID scene
* @tc.number: AudioDebugManager_ParserSessionDebugInfo_InvalidScene
* @tc.desc  : Test ParserAudioSessionDebugInfo interface. Handles INVALID scene correctly.
 * @tc.owner : audio_framework_common
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_ParserSessionDebugInfo_InvalidScene, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AudioSessionDebugInfo debugInfo;
    debugInfo.audioSessionScene = AudioSessionScene::INVALID;
    auto &privateRef = static_cast<AudioDebugManagerPrivate &>(AudioDebugManager::GetInstance());
    std::string result = privateRef.ParserAudioSessionDebugInfo(debugInfo);
    AIUT_EXPECT_FALSE_INFO_LOG(result.empty(), "ParserAudioSessionDebugInfo returned empty string");
    AIUT_EXPECT_NE_INFO_LOG(result.find("audioSession"), std::string::npos, "result missing 'audioSession'");
    // With INVALID scene, fakeFocusState should not be printed
    AIUT_EXPECT_EQ_INFO_LOG(result.find("fakeFocusState"), std::string::npos,
        "fakeFocusState should not appear for INVALID scene");
}

/**
* @tc.name  : Test ParserAudioSessionDebugInfo with MEDIA scene
* @tc.number: AudioDebugManager_ParserSessionDebugInfo_MediaScene
* @tc.desc  : Test ParserAudioSessionDebugInfo interface. Handles MEDIA scene correctly.
 * @tc.owner : audio_framework_common
*/
HWTEST_F(AudioDebuggingManagerAiut, AudioDebugManager_ParserSessionDebugInfo_MediaScene, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AudioSessionDebugInfo debugInfo;
    debugInfo.audioSessionScene = AudioSessionScene::MEDIA;
    auto &privateRef = static_cast<AudioDebugManagerPrivate &>(AudioDebugManager::GetInstance());
    std::string result = privateRef.ParserAudioSessionDebugInfo(debugInfo);
    AIUT_EXPECT_FALSE_INFO_LOG(result.empty(), "ParserAudioSessionDebugInfo returned empty string");
    // With non-INVALID scene, fakeFocusState should be printed
    AIUT_EXPECT_NE_INFO_LOG(result.find("fakeFocusState"), std::string::npos,
        "fakeFocusState should appear for MEDIA scene");
}

} // namespace AudioStandard
} // namespace OHOS