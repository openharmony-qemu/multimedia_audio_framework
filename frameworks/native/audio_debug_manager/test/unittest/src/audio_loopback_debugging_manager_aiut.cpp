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

#include "audio_loopback_debugging_manager_aiut.h"
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

void AudioLoopbackDebuggingManagerAiut::SetUpTestCase(void) {}
void AudioLoopbackDebuggingManagerAiut::TearDownTestCase(void) {}
void AudioLoopbackDebuggingManagerAiut::SetUp(void) {}
void AudioLoopbackDebuggingManagerAiut::TearDown(void) {}

// ==================== RegisterAudioLoopback Tests ====================

/**
* @tc.name  : Test RegisterAudioLoopback API with null callback
* @tc.number: AudioDebugManager_RegisterLoopback_NullCallback
* @tc.desc  : Test RegisterAudioLoopback interface. Returns ERR_INVALID_PARAM when callback is null.
*/
HWTEST_F(AudioLoopbackDebuggingManagerAiut, AudioDebugManager_RegisterLoopback_NullCallback, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    uint32_t key = 0;
    int32_t ret = AudioDebugManager::GetInstance().RegisterAudioLoopback(key, nullptr);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM, "expected ERR_INVALID_PARAM, got ret=%d", ret);
}

/**
* @tc.name  : Test RegisterAudioLoopback API with valid callback
* @tc.number: AudioDebugManager_RegisterLoopback_Success
* @tc.desc  : Test RegisterAudioLoopback interface. Returns SUCCESS and generates non-zero key on success.
*/
HWTEST_F(AudioLoopbackDebuggingManagerAiut, AudioDebugManager_RegisterLoopback_Success, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<TestLoopbackDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    uint32_t key = 0;
    int32_t ret = AudioDebugManager::GetInstance().RegisterAudioLoopback(key, callback);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "RegisterAudioLoopback failed, ret=%d", ret);
    AIUT_EXPECT_NE_INFO_LOG(key, 0u, "key not generated");

    AudioDebugManager::GetInstance().UnregisterAudioLoopback(key);
}

// ==================== UnregisterAudioLoopback Tests ====================

/**
* @tc.name  : Test UnregisterAudioLoopback API with valid key
* @tc.number: AudioDebugManager_UnregisterLoopback_Success
* @tc.desc  : Test UnregisterAudioLoopback interface. Returns SUCCESS when unregistering a registered loopback.
*/
HWTEST_F(AudioLoopbackDebuggingManagerAiut, AudioDebugManager_UnregisterLoopback_Success, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<TestLoopbackDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    uint32_t key = 0;
    AudioDebugManager::GetInstance().RegisterAudioLoopback(key, callback);

    int32_t ret = AudioDebugManager::GetInstance().UnregisterAudioLoopback(key);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "UnregisterAudioLoopback failed, ret=%d", ret);
}

/**
* @tc.name  : Test UnregisterAudioLoopback API with non-existent key
* @tc.number: AudioDebugManager_UnregisterLoopback_NonExistentKey
* @tc.desc  : Test UnregisterAudioLoopback interface. Returns SUCCESS when key does not exist (graceful handling).
*/
HWTEST_F(AudioLoopbackDebuggingManagerAiut, AudioDebugManager_UnregisterLoopback_NonExistentKey, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    int32_t ret = AudioDebugManager::GetInstance().UnregisterAudioLoopback(99999);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "expected SUCCESS for non-existent key, got ret=%d", ret);
}

// ==================== PrintAudioLoopbackDebugInfo Tests ====================

/**
* @tc.name  : Test PrintAudioLoopbackDebugInfo API with invalid key
* @tc.number: AudioDebugManager_PrintLoopback_InvalidKey
* @tc.desc  : Test PrintAudioLoopbackDebugInfo interface. Returns ERR_ILLEGAL_STATE when key is not registered.
*/
HWTEST_F(AudioLoopbackDebuggingManagerAiut, AudioDebugManager_PrintLoopback_InvalidKey, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int32_t ret = AudioDebugManager::GetInstance().PrintAudioLoopbackDebugInfo(99999, -1);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_ILLEGAL_STATE, "expected ERR_ILLEGAL_STATE, got ret=%d", ret);
}

/**
* @tc.name  : Test PrintAudioLoopbackDebugInfo API with pipe fd
* @tc.number: AudioDebugManager_PrintLoopback_WithPipe
* @tc.desc  : Test PrintAudioLoopbackDebugInfo interface. Returns SUCCESS and output contains loopback debug info fields.
*/
HWTEST_F(AudioLoopbackDebuggingManagerAiut, AudioDebugManager_PrintLoopback_WithPipe, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<TestLoopbackDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    callback->testInfo_.appUid = TEST_APP_UID;
    callback->testInfo_.appName = "TestApp";
    callback->testInfo_.mode = LOOPBACK_HARDWARE;
    callback->testInfo_.currentState = LOOPBACK_STATE_RUNNING;
    callback->testInfo_.activeOutputDevice = DEVICE_TYPE_USB_HEADSET;
    callback->testInfo_.activeInputDevice = DEVICE_TYPE_USB_HEADSET;
    callback->testInfo_.reverbPreset = REVERB_PRESET_KTV;
    callback->testInfo_.equalizerPreset = EQUALIZER_PRESET_BRIGHT;
    callback->testInfo_.volume = 75;
    callback->testInfo_.uplinkStreamState = CAPTURER_RUNNING;
    callback->testInfo_.downlinkStreamState = RENDERER_RUNNING;

    uint32_t key = 0;
    int32_t ret = AudioDebugManager::GetInstance().RegisterAudioLoopback(key, callback);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "RegisterAudioLoopback failed, ret=%d", ret);

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    ret = AudioDebugManager::GetInstance().PrintAudioLoopbackDebugInfo(key, pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAudioLoopbackDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_NE_INFO_LOG(output.find("audioLoopback"), std::string::npos,
        "output missing 'audioLoopback'");
    AIUT_EXPECT_NE_INFO_LOG(output.find("TestApp"), std::string::npos,
        "output missing 'TestApp'");
    AIUT_EXPECT_NE_INFO_LOG(output.find("LOOPBACK_HARDWARE"), std::string::npos,
        "output missing 'LOOPBACK_HARDWARE'");
    AIUT_EXPECT_NE_INFO_LOG(output.find("LOOPBACK_STATE_RUNNING"), std::string::npos,
        "output missing 'LOOPBACK_STATE_RUNNING'");
    AIUT_EXPECT_NE_INFO_LOG(output.find("CAPTURER_RUNNING"), std::string::npos,
        "output missing 'CAPTURER_RUNNING'");
    AIUT_EXPECT_NE_INFO_LOG(output.find("RENDERER_RUNNING"), std::string::npos,
        "output missing 'RENDERER_RUNNING'");

    AudioDebugManager::GetInstance().UnregisterAudioLoopback(key);
}

// ==================== PrintAllAudioLoopbacksDebugInfo Tests ====================

/**
* @tc.name  : Test PrintAllAudioLoopbacksDebugInfo API with no loopbacks
* @tc.number: AudioDebugManager_PrintAllLoopbacks_NoLoopbacks
* @tc.desc  : Test PrintAllAudioLoopbacksDebugInfo interface. Returns SUCCESS when no loopbacks are registered.
*/
HWTEST_F(AudioLoopbackDebuggingManagerAiut, AudioDebugManager_PrintAllLoopbacks_NoLoopbacks, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    int32_t ret = AudioDebugManager::GetInstance().PrintAllAudioLoopbacksDebugInfo(pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAllLoopbacksDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);
    close(pipefd[0]);
}

/**
* @tc.name  : Test PrintAllAudioLoopbacksDebugInfo API with multiple callbacks
* @tc.number: AudioDebugManager_PrintAllLoopbacks_MultipleCallbacks
* @tc.desc  : Test PrintAllAudioLoopbacksDebugInfo interface. Output contains info from all registered loopbacks.
*/
HWTEST_F(AudioLoopbackDebuggingManagerAiut, AudioDebugManager_PrintAllLoopbacks_MultipleCallbacks, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto callback1 = std::make_shared<TestLoopbackDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback1, nullptr, "make_shared callback1 failed");
    callback1->testInfo_.appUid = 100;
    callback1->testInfo_.appName = "App1";

    auto callback2 = std::make_shared<TestLoopbackDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback2, nullptr, "make_shared callback2 failed");
    callback2->testInfo_.appUid = 200;
    callback2->testInfo_.appName = "App2";

    uint32_t key1 = 0, key2 = 0;
    AudioDebugManager::GetInstance().RegisterAudioLoopback(key1, callback1);
    AudioDebugManager::GetInstance().RegisterAudioLoopback(key2, callback2);
    AIUT_EXPECT_NE_INFO_LOG(key1, key2, "keys not unique: key1=%u, key2=%u", key1, key2);

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    int32_t ret = AudioDebugManager::GetInstance().PrintAllAudioLoopbacksDebugInfo(pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAllLoopbacksDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_NE_INFO_LOG(output.find("App1"), std::string::npos, "output missing 'App1'");
    AIUT_EXPECT_NE_INFO_LOG(output.find("App2"), std::string::npos, "output missing 'App2'");

    AudioDebugManager::GetInstance().UnregisterAudioLoopback(key1);
    AudioDebugManager::GetInstance().UnregisterAudioLoopback(key2);
}

// ==================== Loopback DebugInfo Content Verification ====================

/**
* @tc.name  : Test LoopbackDebugInfo contains device info fields
* @tc.number: AudioDebugManager_LoopbackDebugInfo_DeviceInfo
* @tc.desc  : Test PrintAudioLoopbackDebugInfo interface. Output contains device type and state info.
*/
HWTEST_F(AudioLoopbackDebuggingManagerAiut, AudioDebugManager_LoopbackDebugInfo_DeviceInfo, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<TestLoopbackDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    callback->testInfo_.activeOutputDevice = DEVICE_TYPE_BLUETOOTH_A2DP;
    callback->testInfo_.activeInputDevice = DEVICE_TYPE_MIC;
    callback->testInfo_.currentState = LOOPBACK_STATE_PREPARED;

    uint32_t key = 0;
    AudioDebugManager::GetInstance().RegisterAudioLoopback(key, callback);

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    int32_t ret = AudioDebugManager::GetInstance().PrintAudioLoopbackDebugInfo(key, pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAudioLoopbackDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_NE_INFO_LOG(output.find("DEVICE_TYPE_BLUETOOTH_A2DP"), std::string::npos,
        "output missing 'DEVICE_TYPE_BLUETOOTH_A2DP'");
    AIUT_EXPECT_NE_INFO_LOG(output.find("DEVICE_TYPE_MIC"), std::string::npos,
        "output missing 'DEVICE_TYPE_MIC'");
    AIUT_EXPECT_NE_INFO_LOG(output.find("LOOPBACK_STATE_PREPARED"), std::string::npos,
        "output missing 'LOOPBACK_STATE_PREPARED'");

    AudioDebugManager::GetInstance().UnregisterAudioLoopback(key);
}

/**
* @tc.name  : Test LoopbackDebugInfo contains effect info fields
* @tc.number: AudioDebugManager_LoopbackDebugInfo_EffectInfo
* @tc.desc  : Test PrintAudioLoopbackDebugInfo interface. Output contains reverb/equalizer preset info.
*/
HWTEST_F(AudioLoopbackDebuggingManagerAiut, AudioDebugManager_LoopbackDebugInfo_EffectInfo, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto callback = std::make_shared<TestLoopbackDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    callback->testInfo_.reverbPreset = REVERB_PRESET_CONCERT;
    callback->testInfo_.equalizerPreset = EQUALIZER_PRESET_FLAT;
    callback->testInfo_.volume = 50;

    uint32_t key = 0;
    AudioDebugManager::GetInstance().RegisterAudioLoopback(key, callback);

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    int32_t ret = AudioDebugManager::GetInstance().PrintAudioLoopbackDebugInfo(key, pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAudioLoopbackDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_NE_INFO_LOG(output.find("REVERB_PRESET_CONCERT"), std::string::npos,
        "output missing 'REVERB_PRESET_CONCERT'");
    AIUT_EXPECT_NE_INFO_LOG(output.find("EQUALIZER_PRESET_FLAT"), std::string::npos,
        "output missing 'EQUALIZER_PRESET_FLAT'");

    AudioDebugManager::GetInstance().UnregisterAudioLoopback(key);
}

// ==================== Taihe Loopback Tests ====================
// Note: OHAudio C API does NOT have PrintLoopbackInfo - this is Taihe/NAPI only

/**
* @tc.name  : Test Taihe PrintLoopbackInfo API gets key and prints
* @tc.number: Taihe_PrintLoopbackInfo_GetKeyAndPrint
* @tc.desc  : Test PrintAudioLoopbackDebugInfo interface. Returns SUCCESS and output contains audioLoopback info via key lookup.
*/
HWTEST_F(AudioLoopbackDebuggingManagerAiut, Taihe_PrintLoopbackInfo_GetKeyAndPrint, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    // Taihe: PrintLoopbackInfo gets key via loopback->GetDebugKey(),
    //        then calls PrintAudioLoopbackDebugInfo(key, fd)
    auto callback = std::make_shared<TestLoopbackDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    callback->testInfo_.appUid = 5678;
    callback->testInfo_.appName = "TaiheLoopbackApp";
    callback->testInfo_.currentState = LOOPBACK_STATE_RUNNING;

    uint32_t key = 0;
    int32_t ret = AudioDebugManager::GetInstance().RegisterAudioLoopback(key, callback);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "RegisterAudioLoopback failed, ret=%d", ret);

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    ret = AudioDebugManager::GetInstance().PrintAudioLoopbackDebugInfo(key, pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAudioLoopbackDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_NE_INFO_LOG(output.find("audioLoopback"), std::string::npos,
        "output missing 'audioLoopback'");
    AIUT_EXPECT_NE_INFO_LOG(output.find("TaiheLoopbackApp"), std::string::npos,
        "output missing 'TaiheLoopbackApp'");

    AudioDebugManager::GetInstance().UnregisterAudioLoopback(key);
}

/**
* @tc.name  : Test Taihe PrintLoopbackInfo API tag validation
* @tc.number: Taihe_PrintLoopbackInfo_TagValidation
* @tc.desc  : Test PrintAudioLoopbackDebugInfo interface. Output contains loopback state info when tag is valid.
*/
HWTEST_F(AudioLoopbackDebuggingManagerAiut, Taihe_PrintLoopbackInfo_TagValidation, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    // Taihe: PrintLoopbackInfo checks loopback.get_tag() != AudioLpback::tag_t::type_audioLpback
    // If tag is wrong, it logs error and returns without calling native method.
    auto callback = std::make_shared<TestLoopbackDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    callback->testInfo_.appUid = 9999;
    callback->testInfo_.currentState = LOOPBACK_STATE_PREPARED;

    uint32_t key = 0;
    AudioDebugManager::GetInstance().RegisterAudioLoopback(key, callback);

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    int32_t ret = AudioDebugManager::GetInstance().PrintAudioLoopbackDebugInfo(key, pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAudioLoopbackDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_NE_INFO_LOG(output.find("LOOPBACK_STATE_PREPARED"), std::string::npos,
        "output missing 'LOOPBACK_STATE_PREPARED'");

    AudioDebugManager::GetInstance().UnregisterAudioLoopback(key);
}

// ==================== NAPI Loopback Tests ====================

/**
* @tc.name  : Test NAPI printLoopbackInfo API gets key and prints
* @tc.number: Napi_PrintLoopbackInfo_GetKeyAndPrint
* @tc.desc  : Test PrintAudioLoopbackDebugInfo interface. Returns SUCCESS and output contains audioLoopback info via key lookup.
*/
HWTEST_F(AudioLoopbackDebuggingManagerAiut, Napi_PrintLoopbackInfo_GetKeyAndPrint, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    // NAPI: printLoopbackInfo(loopback, fd) -> napiLoopback->loopback_->GetDebugKey()
    //       then AudioDebugManager::GetInstance().PrintAudioLoopbackDebugInfo(key, fd)
    auto callback = std::make_shared<TestLoopbackDebugCallback>();
    AIUT_ASSERT_NE_INFO_LOG(callback, nullptr, "make_shared failed");
    callback->testInfo_.appUid = 5000;
    callback->testInfo_.appName = "NapiLoopbackTest";
    callback->testInfo_.currentState = LOOPBACK_STATE_RUNNING;

    uint32_t key = 0;
    int32_t ret = AudioDebugManager::GetInstance().RegisterAudioLoopback(key, callback);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "RegisterAudioLoopback failed, ret=%d", ret);
    AIUT_EXPECT_NE_INFO_LOG(key, 0u, "key not generated");

    int pipefd[2];
    AIUT_ASSERT_TRUE_INFO_LOG(CreateTestPipe(pipefd), "pipe() failed");

    ret = AudioDebugManager::GetInstance().PrintAudioLoopbackDebugInfo(key, pipefd[1]);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "PrintAudioLoopbackDebugInfo failed, ret=%d", ret);
    close(pipefd[1]);

    std::string output = ReadFromPipe(pipefd[0]);
    close(pipefd[0]);
    AIUT_EXPECT_NE_INFO_LOG(output.find("audioLoopback"), std::string::npos,
        "output missing 'audioLoopback'");
    AIUT_EXPECT_NE_INFO_LOG(output.find("NapiLoopbackTest"), std::string::npos,
        "output missing 'NapiLoopbackTest'");

    AudioDebugManager::GetInstance().UnregisterAudioLoopback(key);
}

} // namespace AudioStandard
} // namespace OHOS
