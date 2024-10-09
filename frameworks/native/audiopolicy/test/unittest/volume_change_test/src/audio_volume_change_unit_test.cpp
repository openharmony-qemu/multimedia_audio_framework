/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef LOG_TAG
#define LOG_TAG "AudioVolumeChangeUnitTest"
#endif

#include <sys/stat.h>

#include "audio_errors.h"
#include "audio_info.h"
#include "audio_log.h"
#include "audio_system_manager.h"

#include "audio_volume_change_unit_test.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
    AudioSystemManager *g_audioManagerInstance = nullptr;

    int32_t g_streamType(0);
    int32_t g_volumeLevel(0);
    bool g_isUpdateUi(false);
    int32_t g_volumeGroupId(0);
    std::string g_networkId(LOCAL_NETWORK_ID);
    bool g_isCallbackReceived(false);

    std::string g_callbackName("");
    std::mutex g_mutex;
    std::condition_variable g_condVar;
} // namespace

ApplicationCallback::ApplicationCallback(const std::string &testCaseName) : testCaseName_(testCaseName) {}

void ApplicationCallback::OnVolumeKeyEvent(VolumeEvent volumeEvent)
{
    g_isCallbackReceived = true;
    g_streamType = volumeEvent.volumeType;
    g_volumeLevel = volumeEvent.volume;
    g_callbackName = testCaseName_;
    g_isUpdateUi = volumeEvent.updateUi;
    g_volumeGroupId = volumeEvent.volumeGroupId;
    g_networkId = volumeEvent.networkId;
    g_condVar.notify_all();
}

void AudioVolumeChangeUnitTest::WaitForCallback()
{
    std::unique_lock<std::mutex> lock(g_mutex);
    g_condVar.wait_until(lock, std::chrono::system_clock::now() + std::chrono::minutes(1),
        []() { return g_isCallbackReceived == true; });
}

void AudioVolumeChangeUnitTest::SetUpTestCase(void)
{
    g_audioManagerInstance = AudioSystemManager::GetInstance();
    if (g_audioManagerInstance == nullptr) {
        AUDIO_ERR_LOG("AudioSystemManager instance not available");
        return;
    }
}

void AudioVolumeChangeUnitTest::TearDownTestCase(void)
{
    g_audioManagerInstance = nullptr;
}

// SetUp:Execute before each test case
void AudioVolumeChangeUnitTest::SetUp() {}

void AudioVolumeChangeUnitTest::TearDown(void) {}

/*
 * Feature: AudioVolumeChangeUnitTest
 * Function: Set volume for AudioStreamType::STREAM_MUSIC
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:
 */
HWTEST_F(AudioVolumeChangeUnitTest,  volumeChange_test_001, TestSize.Level1)
{
    int result;
    int callBackSetResult;
    std::string testCaseName("volumeChange_test_001");
    g_isCallbackReceived = false;
    AudioStreamType streamType = AudioStreamType::STREAM_MUSIC;
    AudioVolumeType volumeType
        = static_cast<AudioVolumeType>(streamType);
    int volume = 10;
    g_callbackName = testCaseName;
    bool isUpdateUi = false;
    auto appCallback = make_shared<ApplicationCallback>(testCaseName);
    callBackSetResult = g_audioManagerInstance->RegisterVolumeKeyEventCallback(getpid(), appCallback);
    result = g_audioManagerInstance->SetVolume(volumeType, volume);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(callBackSetResult, SUCCESS);
    if (result == SUCCESS) {
        // Wait here for callback. If not callback for 2 mintues, will skip this step
        AudioVolumeChangeUnitTest::WaitForCallback();
        EXPECT_EQ(streamType, g_streamType);
        EXPECT_EQ(volume, g_volumeLevel);
        EXPECT_EQ(isUpdateUi, g_isUpdateUi);
        EXPECT_STREQ(g_callbackName.c_str(), testCaseName.c_str());
    }
    g_audioManagerInstance->UnregisterVolumeKeyEventCallback(getpid());
}

/*
 * Feature: AudioVolumeChangeUnitTest
 * Function: Set volume for AudioStreamType::STREAM_RING
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:
 */
HWTEST_F(AudioVolumeChangeUnitTest,  volumeChange_test_002, TestSize.Level1)
{
    int result;
    int callBackSetResult;
    std::string testCaseName("volumeChange_test_002");
    g_isCallbackReceived = false;
    AudioStreamType streamType = AudioStreamType::STREAM_RING;
    AudioVolumeType volumeType
        = static_cast<AudioVolumeType>(streamType);
    int volume = 10;
    g_callbackName = testCaseName;
    bool isUpdateUi = false;
    auto appCallback = make_shared<ApplicationCallback>(testCaseName);
    callBackSetResult = g_audioManagerInstance->RegisterVolumeKeyEventCallback(getpid(), appCallback);
    result = g_audioManagerInstance->SetVolume(volumeType, volume);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(callBackSetResult, SUCCESS);
    if (result == SUCCESS) {
        // Wait here for callback. If not callback for 2 mintues, will skip this step
        AudioVolumeChangeUnitTest::WaitForCallback();
        EXPECT_EQ(streamType, g_streamType);
        EXPECT_EQ(volume, g_volumeLevel);
        EXPECT_EQ(isUpdateUi, g_isUpdateUi);
        EXPECT_STREQ(g_callbackName.c_str(), testCaseName.c_str());
    }
    g_audioManagerInstance->UnregisterVolumeKeyEventCallback(getpid());
}

/*
 * Feature: AudioVolumeChangeUnitTest
 * Function: Set volume for AudioStreamType::STREAM_VOICE_CALL
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:
 */
HWTEST_F(AudioVolumeChangeUnitTest,  volumeChange_test_003, TestSize.Level1)
{
    int result;
    int callBackSetResult;
    std::string testCaseName("volumeChange_test_003");
    g_isCallbackReceived = false;
    AudioStreamType streamType = AudioStreamType::STREAM_VOICE_CALL;
    AudioVolumeType volumeType = static_cast<AudioVolumeType>(streamType);
    int volume = 10;
    g_callbackName = testCaseName;
    bool isUpdateUi = false;
    auto appCallback = make_shared<ApplicationCallback>(testCaseName);
    callBackSetResult = g_audioManagerInstance->RegisterVolumeKeyEventCallback(getpid(), appCallback);
    result = g_audioManagerInstance->SetVolume(volumeType, volume);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(callBackSetResult, SUCCESS);
    if (result == SUCCESS) {
        // Wait here for callback. If not callback for 2 mintues, will skip this step
        AudioVolumeChangeUnitTest::WaitForCallback();
        EXPECT_EQ(streamType, g_streamType);
        EXPECT_EQ(volume, g_volumeLevel);
        EXPECT_EQ(isUpdateUi, g_isUpdateUi);
        EXPECT_STREQ(g_callbackName.c_str(), testCaseName.c_str());
    }
    g_audioManagerInstance->UnregisterVolumeKeyEventCallback(getpid());
}
} // namespace AudioStandard
} // namespace OHOS
