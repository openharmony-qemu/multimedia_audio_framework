/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "AudioStreamManagerUnitTest"
#endif

#include "audio_stream_manager_unit_test.h"

#include <chrono>
#include <thread>

#include "audio_capturer.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_log.h"
#include "audio_renderer.h"
#include "audio_stream_manager.h"
#include "audio_system_manager.h"
#include "refbase.h"

using namespace std;
using namespace std::chrono;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
    AudioStreamManager *g_audioManagerInstance = nullptr;
    int g_isCallbackReceived = false;
    constexpr uint32_t MIN_DEVICE_ID = 1;
    constexpr int32_t VALUE_NEGATIVE = -1;
    constexpr int32_t RENDERER_FLAG = 0;
    constexpr int32_t CAPTURER_FLAG = 0;
    constexpr int32_t WAIT_TIME = 3;
    constexpr int32_t VALUE_HUNDRED = 100;
    std::string g_callbackName("");
    std::mutex g_mutex;
    std::condition_variable g_condVar;
    vector<unique_ptr<AudioRendererChangeInfo>> g_audioRendererChangeInfosRcvd;
    vector<unique_ptr<AudioCapturerChangeInfo>> g_audioCapturerChangeInfosRcvd;
}

AudioRendererStateChangeCallbackTest::AudioRendererStateChangeCallbackTest(const std::string &testCaseName)
    : testCaseName_(testCaseName) {}
AudioCapturerStateChangeCallbackTest::AudioCapturerStateChangeCallbackTest(const std::string &testCaseName)
    : testCaseName_(testCaseName) {}

void AudioStreamManagerUnitTest::SetUpTestCase(void)
{
    g_audioManagerInstance = AudioStreamManager::GetInstance();
    if (g_audioManagerInstance == nullptr) {
        AUDIO_ERR_LOG("AudioStreamManagerUnitTest:  AudioStreamManager get instance fails");
        return;
    }
}
void AudioStreamManagerUnitTest::TearDownTestCase(void) {}
void AudioStreamManagerUnitTest::SetUp(void) {}
void AudioStreamManagerUnitTest::TearDown(void) {}

void AudioStreamManagerUnitTest::InitializeRendererOptions(AudioRendererOptions &rendererOptions)
{
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererOptions.rendererInfo.rendererFlags = RENDERER_FLAG;
    return;
}

void AudioRendererStateChangeCallbackTest::OnRendererStateChange(
    const std::vector<std::unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos)
{
    g_audioRendererChangeInfosRcvd.clear();
    for (const auto &changeInfo : audioRendererChangeInfos) {
        g_audioRendererChangeInfosRcvd.push_back(std::make_unique<AudioRendererChangeInfo>(*changeInfo));
    }

    g_isCallbackReceived = true;
}

void AudioStreamManagerUnitTest::WaitForCallback()
{
    std::unique_lock<std::mutex> lock(g_mutex);
    g_condVar.wait_until(lock, std::chrono::system_clock::now() + std::chrono::minutes(1),
        []() { return g_isCallbackReceived == true; });
}

void AudioStreamManagerUnitTest::InitializeCapturerOptions(AudioCapturerOptions &capturerOptions)
{
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_96000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;
    return;
}

void AudioCapturerStateChangeCallbackTest::OnCapturerStateChange(
    const std::vector<std::unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    g_audioCapturerChangeInfosRcvd.clear();
    for (const auto &changeInfo : audioCapturerChangeInfos) {
        g_audioCapturerChangeInfosRcvd.push_back(std::make_unique<AudioCapturerChangeInfo>(*changeInfo));
    }
    g_isCallbackReceived = true;
}

/**
* @tc.name  : Test RegisterAudioRendererEventListener API
* @tc.number: AudioStreamChangeListnerRegisterAudioRendererEventListener_001
* @tc.desc  : Test RegisterAudioRendererEventListener interface with valid parameters
*/
HWTEST_F(AudioStreamManagerUnitTest, AudioStreamChangeListnerRegisterAudioRendererEventListener_001, TestSize.Level0)
{
    int callBackSetResult = -1;
    std::string testCaseName("AudioStreamChangeListnerRegisterAudioRendererEventListener_001");

    vector<unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;

    auto audioRendererStateChangeCallbackTest = make_shared<AudioRendererStateChangeCallbackTest>(testCaseName);
    callBackSetResult = g_audioManagerInstance->RegisterAudioRendererEventListener(getpid(),
        audioRendererStateChangeCallbackTest);
    EXPECT_EQ(SUCCESS, callBackSetResult);

    g_audioManagerInstance->UnregisterAudioRendererEventListener(getpid());
}

/**
* @tc.name  : Test RegisterAudioRendererEventListener API
* @tc.number: AudioStreamChangeListnerRegisterAudioRendererEventListener_002
* @tc.desc  : Test RegisterAudioRendererEventListener interface after unregister event
*/
HWTEST_F(AudioStreamManagerUnitTest, AudioStreamChangeListnerRegisterAudioRendererEventListener_002, TestSize.Level1)
{
    int callBackSetResult = -1;
    std::string testCaseName("AudioStreamChangeListnerRegisterAudioRendererEventListener_002");

    vector<unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;

    auto audioRendererStateChangeCallbackTest = make_shared<AudioRendererStateChangeCallbackTest>(testCaseName);
    callBackSetResult = g_audioManagerInstance->RegisterAudioRendererEventListener(getpid(),
        audioRendererStateChangeCallbackTest);
    EXPECT_EQ(SUCCESS, callBackSetResult);

    g_audioManagerInstance->UnregisterAudioRendererEventListener(getpid());

    auto audioRendererStateChangeCallbackTest2 = make_shared<AudioRendererStateChangeCallbackTest>(testCaseName);
    callBackSetResult = g_audioManagerInstance->RegisterAudioRendererEventListener(getpid(),
        audioRendererStateChangeCallbackTest2);
    EXPECT_EQ(SUCCESS, callBackSetResult);

    g_audioManagerInstance->UnregisterAudioRendererEventListener(getpid());
}

/**
* @tc.name  : Test RegisterAudioRendererEventListener API
* @tc.number: AudioStreamChangeListnerRegisterAudioRendererEventListener_003
* @tc.desc  : Test RegisterAudioRendererEventListener interface with null callback
*/
HWTEST_F(AudioStreamManagerUnitTest, AudioStreamChangeListnerRegisterAudioRendererEventListener_003, TestSize.Level1)
{
    int callBackSetResult = -1;

    callBackSetResult = g_audioManagerInstance->RegisterAudioRendererEventListener(getpid(), nullptr);
    EXPECT_NE(SUCCESS, callBackSetResult);
}

/**
* @tc.name  : Test RegisterAudioRendererEventListener API
* @tc.number: AudioStreamChangeListnerRegisterAudioRendererEventListener_004
* @tc.desc  : Test RegisterAudioRendererEventListener interface with valid parameter after nullptr callback
*/
HWTEST_F(AudioStreamManagerUnitTest, AudioStreamChangeListnerRegisterAudioRendererEventListener_004, TestSize.Level1)
{
    int callBackSetResult = -1;
    std::string testCaseName("AudioStreamChangeListnerRegisterAudioRendererEventListener_004");

    callBackSetResult = g_audioManagerInstance->RegisterAudioRendererEventListener(getpid(), nullptr);
    EXPECT_NE(SUCCESS, callBackSetResult);

    auto audioRendererStateChangeCallbackTest2 = make_shared<AudioRendererStateChangeCallbackTest>(testCaseName);
    callBackSetResult = g_audioManagerInstance->RegisterAudioRendererEventListener(getpid(),
        audioRendererStateChangeCallbackTest2);
    EXPECT_EQ(SUCCESS, callBackSetResult);

    g_audioManagerInstance->UnregisterAudioRendererEventListener(getpid());
}

/**
* @tc.name  : Test UnregisterAudioRendererEventListener API
* @tc.number: Audio_Stream_Change_Listner_UnregisterAudioRendererEventListener_001
* @tc.desc  : Test UnregisterAudioRendererEventListener interface with valid parameters
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_UnregisterAudioRendererEventListener_001,
    TestSize.Level0)
{
    int callBackSetResult = -1;
    int callBackUnSetResult = -1;
    std::string testCaseName("Audio_Stream_Change_Listner_UnregisterAudioRendererEventListener_001");

    auto audioRendererStateChangeCallbackTest2 = make_shared<AudioRendererStateChangeCallbackTest>(testCaseName);
    callBackSetResult = g_audioManagerInstance->RegisterAudioRendererEventListener(getpid(),
        audioRendererStateChangeCallbackTest2);
    EXPECT_EQ(SUCCESS, callBackSetResult);

    callBackUnSetResult = g_audioManagerInstance->UnregisterAudioRendererEventListener(getpid());
    EXPECT_EQ(SUCCESS, callBackUnSetResult);
}

/**
* @tc.name  : Test UnregisterAudioRendererEventListener API
* @tc.number: Audio_Stream_Change_Listner_UnregisterAudioRendererEventListener_002
* @tc.desc  : Test UnregisterAudioRendererEventListener interface without Register event
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_UnregisterAudioRendererEventListener_002,
    TestSize.Level1)
{
    int callBackUnSetResult = -1;

    callBackUnSetResult = g_audioManagerInstance->UnregisterAudioRendererEventListener(getpid());
    EXPECT_EQ(SUCCESS, callBackUnSetResult);

    callBackUnSetResult = g_audioManagerInstance->UnregisterAudioRendererEventListener(VALUE_NEGATIVE);
    EXPECT_EQ(SUCCESS, callBackUnSetResult);
}

/**
* @tc.name  : Test UnregisterAudioRendererEventListener API
* @tc.number: Audio_Stream_Change_Listner_UnregisterAudioRendererEventListener_003
* @tc.desc  : Test UnregisterAudioRendererEventListener interface multiple register and unregister calls
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_UnregisterAudioRendererEventListener_003,
    TestSize.Level1)
{
    int callBackSetResult = -1;
    int callBackUnSetResult = -1;
    std::string testCaseName("Audio_Stream_Change_Listner_UnregisterAudioRendererEventListener_003");

    auto audioRendererStateChangeCallbackTest = make_shared<AudioRendererStateChangeCallbackTest>(testCaseName);
    callBackSetResult = g_audioManagerInstance->RegisterAudioRendererEventListener(getpid(),
        audioRendererStateChangeCallbackTest);
    EXPECT_EQ(SUCCESS, callBackSetResult);

    callBackUnSetResult = g_audioManagerInstance->UnregisterAudioRendererEventListener(getpid());
    EXPECT_EQ(SUCCESS, callBackUnSetResult);

    callBackSetResult = g_audioManagerInstance->RegisterAudioRendererEventListener(getpid(), nullptr);
    EXPECT_NE(SUCCESS, callBackSetResult);

    auto audioRendererStateChangeCallbackTest2 = make_shared<AudioRendererStateChangeCallbackTest>(testCaseName);
    callBackSetResult = g_audioManagerInstance->RegisterAudioRendererEventListener(getpid(),
        audioRendererStateChangeCallbackTest2);
    EXPECT_EQ(SUCCESS, callBackSetResult);

    callBackUnSetResult = g_audioManagerInstance->UnregisterAudioRendererEventListener(getpid());
    EXPECT_EQ(SUCCESS, callBackUnSetResult);

    callBackUnSetResult = g_audioManagerInstance->UnregisterAudioRendererEventListener(getpid());
    EXPECT_EQ(SUCCESS, callBackUnSetResult);
}

/**
* @tc.name  : Test GetCurrentRendererChangeInfos API
* @tc.number: Audio_Stream_Change_Listner_GetCurrentRendererChangeInfos_001
* @tc.desc  : Test GetCurrentRendererChangeInfos interface for single active stream information in prepared state
*             RENDERER_PREPARED:1, RENDERER_RUNNING:2, RENDERER_STOPPED:3, RENDERER_RELEASED:4, RENDERER_PAUSED:5
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_GetCurrentRendererChangeInfos_001, TestSize.Level0)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;
    vector<unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;

    AudioStreamManagerUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    ret = AudioStreamManager::GetInstance()->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(1, static_cast<int32_t>(audioRendererChangeInfos.size()));
    EXPECT_EQ(1, audioRendererChangeInfos[0]->rendererState);
    EXPECT_NE(0, audioRendererChangeInfos[0]->tokenId);
    audioRendererChangeInfos.clear();
    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME));
    ret = AudioStreamManager::GetInstance()->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(0, static_cast<int32_t>(audioRendererChangeInfos.size()));
}

/**
* @tc.name  : Test GetCurrentRendererChangeInfos API
* @tc.number: Audio_Stream_Change_Listner_GetCurrentRendererChangeInfos_002
* @tc.desc  : Test GetCurrentRendererChangeInfos interface while no streams active
*             RENDERER_PREPARED:1, RENDERER_RUNNING:2, RENDERER_STOPPED:3, RENDERER_RELEASED:4, RENDERER_PAUSED:5
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_GetCurrentRendererChangeInfos_002, TestSize.Level1)
{
    int32_t ret = -1;
    vector<unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;

    ret = AudioStreamManager::GetInstance()->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(0, static_cast<int32_t>(audioRendererChangeInfos.size()));
}

/**
* @tc.name  : Test GetCurrentRendererChangeInfos API
* @tc.number: Audio_Stream_Change_Listner_GetCurrentRendererChangeInfos_003
* @tc.desc  : Test GetCurrentRendererChangeInfos interface for single active renderer stream in running state
*             RENDERER_PREPARED:1, RENDERER_RUNNING:2, RENDERER_STOPPED:3, RENDERER_RELEASED:4, RENDERER_PAUSED:5
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_GetCurrentRendererChangeInfos_003, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;
    vector<unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;

    AudioStreamManagerUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);
    ret = AudioStreamManager::GetInstance()->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(1, static_cast<int32_t>(audioRendererChangeInfos.size()));
    EXPECT_EQ(2, audioRendererChangeInfos[0]->rendererState);
    EXPECT_NE(0, audioRendererChangeInfos[0]->tokenId);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
    audioRendererChangeInfos.clear();
    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME));
    ret = AudioStreamManager::GetInstance()->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(0, static_cast<int32_t>(audioRendererChangeInfos.size()));
}

/**
* @tc.name  : Test GetCurrentRendererChangeInfos API
* @tc.number: Audio_Stream_Change_Listner_GetCurrentRendererChangeInfos_004
* @tc.desc  : Test GetCurrentRendererChangeInfos interface for single active renderer stream in stopped state
*             RENDERER_PREPARED:1, RENDERER_RUNNING:2, RENDERER_STOPPED:3, RENDERER_RELEASED:4, RENDERER_PAUSED:5
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_GetCurrentRendererChangeInfos_004, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;
    vector<unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;

    AudioStreamManagerUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);
    ret = AudioStreamManager::GetInstance()->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    EXPECT_EQ(1, static_cast<int32_t>(audioRendererChangeInfos.size()));
    EXPECT_EQ(3, audioRendererChangeInfos[0]->rendererState);
    EXPECT_NE(0, audioRendererChangeInfos[0]->tokenId);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
    audioRendererChangeInfos.clear();
    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME));
    ret = AudioStreamManager::GetInstance()->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(0, static_cast<int32_t>(audioRendererChangeInfos.size()));
}

/**
* @tc.name  : Test GetCurrentRendererChangeInfos API
* @tc.number: Audio_Stream_Change_Listner_GetCurrentRendererChangeInfos_005
* @tc.desc  : Test GetCurrentRendererChangeInfos interface for single active renderer stream in released state
*             RENDERER_PREPARED:1, RENDERER_RUNNING:2, RENDERER_STOPPED:3, RENDERER_RELEASED:4, RENDERER_PAUSED:5
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_GetCurrentRendererChangeInfos_005, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;
    vector<unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;

    AudioStreamManagerUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME));
    ret = AudioStreamManager::GetInstance()->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(0, static_cast<int32_t>(audioRendererChangeInfos.size()));
}

/**
* @tc.name  : Test GetCurrentRendererChangeInfos API
* @tc.number: Audio_Stream_Change_Listner_GetCurrentRendererChangeInfos_006
* @tc.desc  : Test GetCurrentRendererChangeInfos interface for two active renderer stream information
*             RENDERER_PREPARED:1, RENDERER_RUNNING:2, RENDERER_STOPPED:3, RENDERER_RELEASED:4, RENDERER_PAUSED:5
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_GetCurrentRendererChangeInfos_006, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;
    vector<unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;

    AudioStreamManagerUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer1 = AudioRenderer::Create(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer1);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer1->Start();
    EXPECT_EQ(true, isStarted);
    isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    audioRendererChangeInfos.clear();
    ret = AudioStreamManager::GetInstance()->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(2, static_cast<int32_t>(audioRendererChangeInfos.size()));
    EXPECT_EQ(2, audioRendererChangeInfos[0]->rendererState);
    EXPECT_EQ(2, audioRendererChangeInfos[1]->rendererState);
    EXPECT_NE(0, audioRendererChangeInfos[0]->tokenId);

    bool isStopped = audioRenderer1->Stop();
    EXPECT_EQ(true, isStopped);

    audioRendererChangeInfos.clear();
    ret = AudioStreamManager::GetInstance()->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(2, static_cast<int32_t>(audioRendererChangeInfos.size()));
    EXPECT_EQ(3, audioRendererChangeInfos[0]->rendererState);
    EXPECT_EQ(2, audioRendererChangeInfos[1]->rendererState);
    EXPECT_NE(0, audioRendererChangeInfos[0]->tokenId);

    isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioRenderer1->Release();
    EXPECT_EQ(true, isReleased);

    isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME));
    audioRendererChangeInfos.clear();
    ret = AudioStreamManager::GetInstance()->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(0, static_cast<int32_t>(audioRendererChangeInfos.size()));
}

/**
* @tc.name  : Test GetCurrentRendererChangeInfos API
* @tc.number: Audio_Stream_Change_Listner_GetCurrentRendererChangeInfos_007
* @tc.desc  : Test GetCurrentRendererChangeInfos interface to display streams details
*             RENDERER_PREPARED:1, RENDERER_RUNNING:2, RENDERER_STOPPED:3, RENDERER_RELEASED:4, RENDERER_PAUSED:5
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_GetCurrentRendererChangeInfos_007, TestSize.Level1)
{
    int32_t ret = -1;
    vector<unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;

    ret = AudioStreamManager::GetInstance()->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    EXPECT_EQ(SUCCESS, ret);

    if (audioRendererChangeInfos.size() > 0) {
        AUDIO_DEBUG_LOG("AudioStreamManagerTest: audioRendererChangeInfos Number of entries %{public}u",
        static_cast<int32_t>(audioRendererChangeInfos.size()));
        uint32_t index = 0;
        for (auto it = audioRendererChangeInfos.begin(); it != audioRendererChangeInfos.end(); it++) {
            AudioRendererChangeInfo audioRendererChangeInfo = **it;
            AUDIO_DEBUG_LOG("audioRendererChangeInfos[%{public}d]", index++);
            AUDIO_DEBUG_LOG("clientUID = %{public}d", audioRendererChangeInfo.clientUID);
            AUDIO_DEBUG_LOG("sessionId = %{public}d", audioRendererChangeInfo.sessionId);
            AUDIO_DEBUG_LOG("rendererState = %{public}d", audioRendererChangeInfo.rendererState);
        }
    } else {
        AUDIO_DEBUG_LOG("AudioStreamManagerTest: audioRendererChangeInfos: No Active Streams");
    }
}

/**
* @tc.name  : Test GetCurrentRendererChangeInfos API
* @tc.number: Audio_Stream_Change_Listner_GetCurrentRendererChangeInfos_Stability_001
* @tc.desc  : Test GetCurrentRendererChangeInfos interface for single active renderer stream in running state
*             RENDERER_PREPARED:1, RENDERER_RUNNING:2, RENDERER_STOPPED:3, RENDERER_RELEASED:4, RENDERER_PAUSED:5
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_GetCurrentRendererChangeInfos_Stability_001,
    TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;
    vector<unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;

    AudioStreamManagerUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    for (int32_t i = 0; i < VALUE_HUNDRED; i++) {
        ret = AudioStreamManager::GetInstance()->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
        EXPECT_EQ(SUCCESS, ret);
        EXPECT_EQ(1, static_cast<int32_t>(audioRendererChangeInfos.size()));
        EXPECT_EQ(2, audioRendererChangeInfos[0]->rendererState);
        audioRendererChangeInfos.clear();
    }

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
    audioRendererChangeInfos.clear();
    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME));
    ret = AudioStreamManager::GetInstance()->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(0, static_cast<int32_t>(audioRendererChangeInfos.size()));
}

/**
* @tc.name  : Test GetCurrentRendererChangeInfos API
* @tc.number: Audio_Stream_Change_Listner_GetCurrentRendererChangeDeviceInfos_001
* @tc.desc  : Test GetCurrentRendererChangeInfos interface for getting device information for current stream
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_GetCurrentRendererChangeDeviceInfos_001,
    TestSize.Level1)
{
    int32_t ret = -1;
    AudioRendererOptions rendererOptions;
    vector<unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;

    AudioStreamManagerUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    for (int32_t i = 0; i < VALUE_HUNDRED; i++) {
        ret = AudioStreamManager::GetInstance()->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
        EXPECT_EQ(SUCCESS, ret);
        EXPECT_EQ(1, static_cast<int32_t>(audioRendererChangeInfos.size()));
        EXPECT_EQ(2, audioRendererChangeInfos[0]->rendererState);
        EXPECT_EQ(audioRendererChangeInfos[0]->outputDeviceInfo.deviceRole, DeviceRole::OUTPUT_DEVICE);
        EXPECT_EQ(audioRendererChangeInfos[0]->outputDeviceInfo.deviceType, DeviceType::DEVICE_TYPE_SPEAKER);
        EXPECT_GE(audioRendererChangeInfos[0]->outputDeviceInfo.deviceId, MIN_DEVICE_ID);
        EXPECT_EQ(true, (*audioRendererChangeInfos[0]->outputDeviceInfo.audioStreamInfo.samplingRate.rbegin()
            >= SAMPLE_RATE_8000) ||
            ((*audioRendererChangeInfos[0]->outputDeviceInfo.audioStreamInfo.samplingRate.begin()
            <= SAMPLE_RATE_96000)));
        EXPECT_EQ(audioRendererChangeInfos[0]->outputDeviceInfo.audioStreamInfo.encoding,
            AudioEncodingType::ENCODING_PCM);
        EXPECT_EQ(true, (*audioRendererChangeInfos[0]->outputDeviceInfo.audioStreamInfo.channels.rbegin() >= MONO)
            && ((*audioRendererChangeInfos[0]->outputDeviceInfo.audioStreamInfo.channels.begin() <= CHANNEL_8)));
        EXPECT_EQ(true, (audioRendererChangeInfos[0]->outputDeviceInfo.audioStreamInfo.format >= SAMPLE_U8)
            && ((audioRendererChangeInfos[0]->outputDeviceInfo.audioStreamInfo.format <= SAMPLE_F32LE)));
        audioRendererChangeInfos.clear();
    }

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
    audioRendererChangeInfos.clear();
    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME));
    ret = AudioStreamManager::GetInstance()->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(0, static_cast<int32_t>(audioRendererChangeInfos.size()));
}

/**
* @tc.name  : Test Feature RendererStateChangeCallback
* @tc.number: Audio_Stream_Change_Listner_RendererStateChangeCallbackTest_001
* @tc.desc  : Test RendererStateChangeCallback interface in prepared state
*             RENDERER_PREPARED:1, RENDERER_RUNNING:2, RENDERER_STOPPED:3, RENDERER_RELEASED:4, RENDERER_PAUSED:5
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_RendererStateChangeCallbackTest_001, TestSize.Level0)
{
    int32_t ret = -1;
    int callBackSetResult;
    std::string testCaseName("Audio_Stream_Change_Listner_RendererStateChangeCallbackTest_001");
    vector<unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;

    g_callbackName = testCaseName;

    auto audioRendererStateChangeCallbackTest = make_shared<AudioRendererStateChangeCallbackTest>(testCaseName);
    callBackSetResult = g_audioManagerInstance->RegisterAudioRendererEventListener(getpid(),
        audioRendererStateChangeCallbackTest);
    EXPECT_EQ(SUCCESS, callBackSetResult);

    AudioRendererOptions rendererOptions;
    AudioStreamManagerUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);
    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME));
    if (audioRenderer != nullptr) {
        // Wait here for callback. If not callback for 2 mintues, will skip this step
        AudioStreamManagerUnitTest::WaitForCallback();
        g_isCallbackReceived = false;
        EXPECT_EQ(1, static_cast<int32_t>(g_audioRendererChangeInfosRcvd.size()));
        EXPECT_EQ(1, g_audioRendererChangeInfosRcvd[0]->rendererState);
        EXPECT_STREQ(g_callbackName.c_str(), testCaseName.c_str());
    }

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME));
    ret = AudioStreamManager::GetInstance()->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(0, static_cast<int32_t>(audioRendererChangeInfos.size()));

    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME));
    g_audioManagerInstance->UnregisterAudioRendererEventListener(getpid());
}

/**
* @tc.name  : Test Feature RendererStateChangeCallback
* @tc.number: Audio_Stream_Change_Listner_RendererStateChangeCallbackTest_002
* @tc.desc  : Test RendererStateChangeCallback interface in running state
*             RENDERER_PREPARED:1, RENDERER_RUNNING:2, RENDERER_STOPPED:3, RENDERER_RELEASED:4, RENDERER_PAUSED:5
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_RendererStateChangeCallbackTest_002, TestSize.Level1)
{
    int32_t ret = -1;
    int callBackSetResult;
    std::string testCaseName("Audio_Stream_Change_Listner_RendererStateChangeCallbackTest_002");
    vector<unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;

    g_callbackName = testCaseName;

    auto audioRendererStateChangeCallbackTest = make_shared<AudioRendererStateChangeCallbackTest>(testCaseName);
    callBackSetResult = g_audioManagerInstance->RegisterAudioRendererEventListener(getpid(),
        audioRendererStateChangeCallbackTest);
    EXPECT_EQ(SUCCESS, callBackSetResult);

    AudioRendererOptions rendererOptions;
    AudioStreamManagerUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);
    if (isStarted == SUCCESS) {
        // Wait here for callback. If not callback for 2 mintues, will skip this step
        AudioStreamManagerUnitTest::WaitForCallback();
        g_isCallbackReceived = false;
        EXPECT_EQ(SUCCESS, ret);
        EXPECT_EQ(1, static_cast<int32_t>(g_audioRendererChangeInfosRcvd.size()));
        EXPECT_EQ(2, g_audioRendererChangeInfosRcvd[0]->rendererState);
        EXPECT_STREQ(g_callbackName.c_str(), testCaseName.c_str());
    }

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    ret = AudioStreamManager::GetInstance()->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(0, static_cast<int32_t>(audioRendererChangeInfos.size()));

    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME));
    g_audioManagerInstance->UnregisterAudioRendererEventListener(getpid());
}

/**
* @tc.name  : Test Feature RendererStateChangeCallback
* @tc.number: Audio_Stream_Change_Listner_RendererStateChangeCallbackTest_003
* @tc.desc  : Test RendererStateChangeCallback interface in stop state
*             RENDERER_PREPARED:1, RENDERER_RUNNING:2, RENDERER_STOPPED:3, RENDERER_RELEASED:4, RENDERER_PAUSED:5
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_RendererStateChangeCallbackTest_003, TestSize.Level1)
{
    int32_t ret = -1;
    [[maybe_unused]] int callBackSetResult;
    std::string testCaseName("Audio_Stream_Change_Listner_RendererStateChangeCallbackTest_003");
    AudioRendererOptions rendererOptions;
    vector<unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;

    g_callbackName = testCaseName;

    AudioStreamManagerUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    auto audioRendererStateChangeCallbackTest = make_shared<AudioRendererStateChangeCallbackTest>(testCaseName);
    callBackSetResult = g_audioManagerInstance->RegisterAudioRendererEventListener(getpid(),
        audioRendererStateChangeCallbackTest);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);
    if (isStopped == SUCCESS) {
        // Wait here for callback. If not callback for 2 mintues, will skip this step
        AudioStreamManagerUnitTest::WaitForCallback();
        g_isCallbackReceived = false;
        EXPECT_EQ(SUCCESS, ret);
        EXPECT_EQ(1, static_cast<int32_t>(g_audioRendererChangeInfosRcvd.size()));
        EXPECT_EQ(3, g_audioRendererChangeInfosRcvd[0]->rendererState);
        EXPECT_STREQ(g_callbackName.c_str(), testCaseName.c_str());
    }

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);

    ret = AudioStreamManager::GetInstance()->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(0, static_cast<int32_t>(audioRendererChangeInfos.size()));

    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME));
    g_audioManagerInstance->UnregisterAudioRendererEventListener(getpid());
}

/**
* @tc.name  : Test Feature RendererStateChangeCallback
* @tc.number: Audio_Stream_Change_Listner_RendererStateChangeCallbackTest_001
* @tc.desc  : Test RendererStateChangeCallback interface in release state
*             RENDERER_PREPARED:1, RENDERER_RUNNING:2, RENDERER_STOPPED:3, RENDERER_RELEASED:4, RENDERER_PAUSED:5
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_RendererStateChangeCallbackTest_004, TestSize.Level1)
{
    int32_t ret = -1;
    [[maybe_unused]] int callBackSetResult;
    std::string testCaseName("Audio_Stream_Change_Listner_RendererStateChangeCallbackTest_004");
    AudioRendererOptions rendererOptions;
    vector<unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;

    g_callbackName = testCaseName;

    AudioStreamManagerUnitTest::InitializeRendererOptions(rendererOptions);
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    auto audioRendererStateChangeCallbackTest = make_shared<AudioRendererStateChangeCallbackTest>(testCaseName);
    callBackSetResult = g_audioManagerInstance->RegisterAudioRendererEventListener(getpid(),
        audioRendererStateChangeCallbackTest);

    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioRenderer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioRenderer->Release();
    EXPECT_EQ(true, isReleased);
    if (isStarted == SUCCESS) {
        // Wait here for callback. If not callback for 2 mintues, will skip this step
        g_isCallbackReceived = false;
        AudioStreamManagerUnitTest::WaitForCallback();
        EXPECT_EQ(SUCCESS, ret);
        EXPECT_EQ(1, static_cast<int32_t>(g_audioRendererChangeInfosRcvd.size()));
        EXPECT_EQ(4, g_audioRendererChangeInfosRcvd[0]->rendererState);
        EXPECT_STREQ(g_callbackName.c_str(), testCaseName.c_str());
    }

    ret = AudioStreamManager::GetInstance()->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(0, static_cast<int32_t>(audioRendererChangeInfos.size()));

    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME));
    g_audioManagerInstance->UnregisterAudioRendererEventListener(getpid());
}

// Capturer Listener Unit Cases
/**
* @tc.name  : Test RegisterAudioCapturerEventListener API
* @tc.number: AudioStreamChangeListnerRegisterAudioCapturerEventListener_001
* @tc.desc  : Test RegisterAudioCapturerEventListener interface with valid parameters
*/
HWTEST_F(AudioStreamManagerUnitTest, AudioStreamChangeListnerRegisterAudioCapturerEventListener_001, TestSize.Level0)
{
    int callBackSetResult = -1;
    std::string testCaseName("AudioStreamChangeListnerRegisterAudioCapturerEventListener_001");

    vector<unique_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;

    auto audioCapturerStateChangeCallbackTest = make_shared<AudioCapturerStateChangeCallbackTest>(testCaseName);
    callBackSetResult = g_audioManagerInstance->RegisterAudioCapturerEventListener(getpid(),
        audioCapturerStateChangeCallbackTest);
    EXPECT_EQ(SUCCESS, callBackSetResult);

    g_audioManagerInstance->UnregisterAudioCapturerEventListener(getpid());
}

/**
* @tc.name  : Test RegisterAudioCapturerEventListener API
* @tc.number: AudioStreamChangeListnerRegisterAudioCapturerEventListener_002
* @tc.desc  : Test RegisterAudioCapturerEventListener interface after unregister
*/
HWTEST_F(AudioStreamManagerUnitTest, AudioStreamChangeListnerRegisterAudioCapturerEventListener_002, TestSize.Level1)
{
    int callBackSetResult = -1;
    std::string testCaseName("AudioStreamChangeListnerRegisterAudioCapturerEventListener_002");

    vector<unique_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;

    auto audioCapturerStateChangeCallbackTest = make_shared<AudioCapturerStateChangeCallbackTest>(testCaseName);
    callBackSetResult = g_audioManagerInstance->RegisterAudioCapturerEventListener(getpid(),
        audioCapturerStateChangeCallbackTest);
    EXPECT_EQ(SUCCESS, callBackSetResult);

    g_audioManagerInstance->UnregisterAudioCapturerEventListener(getpid());

    auto audioCapturerStateChangeCallbackTest2 = make_shared<AudioCapturerStateChangeCallbackTest>(testCaseName);
    callBackSetResult = g_audioManagerInstance->RegisterAudioCapturerEventListener(getpid(),
        audioCapturerStateChangeCallbackTest2);
    EXPECT_EQ(SUCCESS, callBackSetResult);

    g_audioManagerInstance->UnregisterAudioCapturerEventListener(getpid());
}

/**
* @tc.name  : Test RegisterAudioCapturerEventListener API
* @tc.number: AudioStreamChangeListnerRegisterAudioCapturerEventListener_003
* @tc.desc  : Test RegisterAudioCapturerEventListener interface with null callback
*/
HWTEST_F(AudioStreamManagerUnitTest, AudioStreamChangeListnerRegisterAudioCapturerEventListener_003, TestSize.Level1)
{
    int callBackSetResult = -1;

    callBackSetResult = g_audioManagerInstance->RegisterAudioCapturerEventListener(getpid(), nullptr);
    EXPECT_NE(SUCCESS, callBackSetResult);
}

/**
* @tc.name  : Test RegisterAudioCapturerEventListener API
* @tc.number: AudioStreamChangeListnerRegisterAudioCapturerEventListener_004
* @tc.desc  : Test RegisterAudioCapturerEventListener interface with valid parameter after nullptr callback
*/
HWTEST_F(AudioStreamManagerUnitTest, AudioStreamChangeListnerRegisterAudioCapturerEventListener_004, TestSize.Level1)
{
    int callBackSetResult = -1;
    std::string testCaseName("AudioStreamChangeListnerRegisterAudioCapturerEventListener_004");

    callBackSetResult = g_audioManagerInstance->RegisterAudioCapturerEventListener(getpid(), nullptr);
    EXPECT_NE(SUCCESS, callBackSetResult);

    auto audioCapturerStateChangeCallbackTest2 = make_shared<AudioCapturerStateChangeCallbackTest>(testCaseName);
    callBackSetResult = g_audioManagerInstance->RegisterAudioCapturerEventListener(getpid(),
        audioCapturerStateChangeCallbackTest2);
    EXPECT_EQ(SUCCESS, callBackSetResult);

    g_audioManagerInstance->UnregisterAudioCapturerEventListener(getpid());
}

/**
* @tc.name  : Test UnregisterAudioCapturerEventListener API
* @tc.number: Audio_Stream_Change_Listner_UnregisterAudioCapturerEventListener_001
* @tc.desc  : Test UnregisterAudioCapturerEventListener interface with valid parameters
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_UnregisterAudioCapturerEventListener_001,
    TestSize.Level0)
{
    int callBackSetResult = -1;
    int callBackUnSetResult = -1;
    std::string testCaseName("Audio_Stream_Change_Listner_UnregisterAudioCapturerEventListener_001");

    vector<unique_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;

    callBackSetResult = g_audioManagerInstance->RegisterAudioCapturerEventListener(getpid(), nullptr);
    EXPECT_NE(SUCCESS, callBackSetResult);

    auto audioCapturerStateChangeCallbackTest2 = make_shared<AudioCapturerStateChangeCallbackTest>(testCaseName);
    callBackSetResult = g_audioManagerInstance->RegisterAudioCapturerEventListener(getpid(),
        audioCapturerStateChangeCallbackTest2);
    EXPECT_EQ(SUCCESS, callBackSetResult);

    callBackUnSetResult = g_audioManagerInstance->UnregisterAudioCapturerEventListener(getpid());
    EXPECT_EQ(SUCCESS, callBackUnSetResult);
}

/**
* @tc.name  : Test UnregisterAudioCapturerEventListener API
* @tc.number: Audio_Stream_Change_Listner_UnregisterAudioCapturerEventListener_002
* @tc.desc  : Test UnregisterAudioCapturerEventListener interface without register event
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_UnregisterAudioCapturerEventListener_002,
    TestSize.Level1)
{
    int callBackUnSetResult = -1;

    callBackUnSetResult = g_audioManagerInstance->UnregisterAudioCapturerEventListener(getpid());
    EXPECT_EQ(SUCCESS, callBackUnSetResult);

    callBackUnSetResult = g_audioManagerInstance->UnregisterAudioCapturerEventListener(VALUE_NEGATIVE);
    EXPECT_EQ(SUCCESS, callBackUnSetResult);
}

/**
* @tc.name  : Test UnregisterAudioCapturerEventListener API
* @tc.number: Audio_Stream_Change_Listner_UnregisterAudioCapturerEventListener_003
* @tc.desc  : Test UnregisterAudioCapturerEventListener interface with multiple register and unregister
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_UnregisterAudioCapturerEventListener_003,
    TestSize.Level1)
{
    int callBackSetResult = -1;
    int callBackUnSetResult = -1;
    std::string testCaseName("Audio_Stream_Change_Listner_UnregisterAudioCapturerEventListener_003");

    auto audioCapturerStateChangeCallbackTest = make_shared<AudioCapturerStateChangeCallbackTest>(testCaseName);
    callBackSetResult = g_audioManagerInstance->RegisterAudioCapturerEventListener(getpid(),
        audioCapturerStateChangeCallbackTest);
    EXPECT_EQ(SUCCESS, callBackSetResult);

    callBackUnSetResult = g_audioManagerInstance->UnregisterAudioCapturerEventListener(getpid());
    EXPECT_EQ(SUCCESS, callBackUnSetResult);

    callBackSetResult = g_audioManagerInstance->RegisterAudioCapturerEventListener(getpid(), nullptr);
    EXPECT_NE(SUCCESS, callBackSetResult);

    auto audioCapturerStateChangeCallbackTest2 = make_shared<AudioCapturerStateChangeCallbackTest>(testCaseName);
    callBackSetResult = g_audioManagerInstance->RegisterAudioCapturerEventListener(getpid(),
        audioCapturerStateChangeCallbackTest2);
    EXPECT_EQ(SUCCESS, callBackSetResult);

    callBackUnSetResult = g_audioManagerInstance->UnregisterAudioCapturerEventListener(getpid());
    EXPECT_EQ(SUCCESS, callBackUnSetResult);

    callBackUnSetResult = g_audioManagerInstance->UnregisterAudioCapturerEventListener(getpid());
    EXPECT_EQ(SUCCESS, callBackUnSetResult);
}

/**
* @tc.name  : Test GetCurrentCapturerChangeInfos API
* @tc.number: Audio_Stream_Change_Listner_GetCurrentCapturerChangeInfos_001
* @tc.desc  : Test GetCurrentCapturerChangeInfos interface for single active stream in prepared state
*             CAPTURER_PREPARED:1 CAPTURER_RUNNING:2 CAPTURER_STOPPED:3 CAPTURER_RELEASED:4 CAPTURER_PAUSED:5
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_GetCurrentCapturerChangeInfos_001, TestSize.Level0)
{
    int32_t ret = -1;
    AudioCapturerOptions capturerOptions;
    vector<unique_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;

    AudioStreamManagerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    ret = AudioStreamManager::GetInstance()->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(1, static_cast<int32_t>(audioCapturerChangeInfos.size()));
    EXPECT_EQ(1, audioCapturerChangeInfos[0]->capturerState);
    audioCapturerChangeInfos.clear();
    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);
    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME));
    ret = AudioStreamManager::GetInstance()->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(0, static_cast<int32_t>(audioCapturerChangeInfos.size()));
}

/**
* @tc.name  : Test GetCurrentCapturerChangeInfos API
* @tc.number: Audio_Stream_Change_Listner_GetCurrentCapturerChangeInfos_002
* @tc.desc  : Test GetCurrentCapturerChangeInfos interface while no stream active
*             CAPTURER_PREPARED:1 CAPTURER_RUNNING:2 CAPTURER_STOPPED:3 CAPTURER_RELEASED:4 CAPTURER_PAUSED:5
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_GetCurrentCapturerChangeInfos_002, TestSize.Level1)
{
    int32_t ret = -1;
    vector<unique_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;

    ret = AudioStreamManager::GetInstance()->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(0, static_cast<int32_t>(audioCapturerChangeInfos.size()));
}

/**
* @tc.name  : Test GetCurrentCapturerChangeInfos API
* @tc.number: Audio_Stream_Change_Listner_GetCurrentCapturerChangeInfos_003
* @tc.desc  : Test GetCurrentCapturerChangeInfos interface for single active stream in running state
*             CAPTURER_PREPARED:1 CAPTURER_RUNNING:2 CAPTURER_STOPPED:3 CAPTURER_RELEASED:4 CAPTURER_PAUSED:5
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_GetCurrentCapturerChangeInfos_003, TestSize.Level1)
{
    int32_t ret = -1;
    AudioCapturerOptions capturerOptions;
    vector<unique_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;

    AudioStreamManagerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);
    ret = AudioStreamManager::GetInstance()->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(1, static_cast<int32_t>(audioCapturerChangeInfos.size()));
    EXPECT_EQ(2, audioCapturerChangeInfos[0]->capturerState);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);
    audioCapturerChangeInfos.clear();
    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME));
    ret = AudioStreamManager::GetInstance()->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(0, static_cast<int32_t>(audioCapturerChangeInfos.size()));
}

/**
* @tc.name  : Test GetCurrentCapturerChangeInfos API
* @tc.number: Audio_Stream_Change_Listner_GetCurrentCapturerChangeInfos_004
* @tc.desc  : Test GetCurrentCapturerChangeInfos interface for single active stream in pause state
*             CAPTURER_PREPARED:1 CAPTURER_RUNNING:2 CAPTURER_STOPPED:3 CAPTURER_RELEASED:4 CAPTURER_PAUSED:5
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_GetCurrentCapturerChangeInfos_004, TestSize.Level1)
{
    int32_t ret = -1;
    AudioCapturerOptions capturerOptions;
    vector<unique_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;

    AudioStreamManagerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    bool isPaused = audioCapturer->Pause();
    EXPECT_EQ(true, isPaused);

    ret = AudioStreamManager::GetInstance()->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(1, static_cast<int32_t>(audioCapturerChangeInfos.size()));
    EXPECT_EQ(5, audioCapturerChangeInfos[0]->capturerState);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);
    audioCapturerChangeInfos.clear();
    std::this_thread::sleep_for(std::chrono::seconds(4));
    ret = AudioStreamManager::GetInstance()->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(0, static_cast<int32_t>(audioCapturerChangeInfos.size()));
}

/**
* @tc.name  : Test GetCurrentCapturerChangeInfos API
* @tc.number: Audio_Stream_Change_Listner_GetCurrentCapturerChangeInfos_005
* @tc.desc  : Test GetCurrentCapturerChangeInfos interface for single active stream in stop state
*             CAPTURER_PREPARED:1 CAPTURER_RUNNING:2 CAPTURER_STOPPED:3 CAPTURER_RELEASED:4 CAPTURER_PAUSED:5
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_GetCurrentCapturerChangeInfos_005, TestSize.Level1)
{
    int32_t ret = -1;
    AudioCapturerOptions capturerOptions;
    vector<unique_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;

    AudioStreamManagerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);
    ret = AudioStreamManager::GetInstance()->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    EXPECT_EQ(1, static_cast<int32_t>(audioCapturerChangeInfos.size()));
    EXPECT_EQ(3, audioCapturerChangeInfos[0]->capturerState);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);
    audioCapturerChangeInfos.clear();
    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME));
    ret = AudioStreamManager::GetInstance()->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(0, static_cast<int32_t>(audioCapturerChangeInfos.size()));
}

/**
* @tc.name  : Test GetCurrentCapturerChangeInfos API
* @tc.number: Audio_Stream_Change_Listner_GetCurrentCapturerChangeInfos_006
* @tc.desc  : Test GetCurrentCapturerChangeInfos interface for single active stream in stop state in release state
*             CAPTURER_PREPARED:1 CAPTURER_RUNNING:2 CAPTURER_STOPPED:3 CAPTURER_RELEASED:4 CAPTURER_PAUSED:5
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_GetCurrentCapturerChangeInfos_006, TestSize.Level1)
{
    int32_t ret = -1;
    AudioCapturerOptions capturerOptions;
    vector<unique_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;

    AudioStreamManagerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);
    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME));
    ret = AudioStreamManager::GetInstance()->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(0, static_cast<int32_t>(audioCapturerChangeInfos.size()));
}

/**
* @tc.name  : Test GetCurrentCapturerChangeInfos API
* @tc.number: Audio_Stream_Change_Listner_GetCurrentCapturerChangeInfos_007
* @tc.desc  : Test GetCurrentCapturerChangeInfos interface. Returns current two capturer stream information
*             CAPTURER_PREPARED:1 CAPTURER_RUNNING:2 CAPTURER_STOPPED:3 CAPTURER_RELEASED:4 CAPTURER_PAUSED:5
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_GetCurrentCapturerChangeInfos_007, TestSize.Level1)
{
    int32_t ret = -1;
    AudioCapturerOptions capturerOptions;
    vector<unique_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;

    AudioStreamManagerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioFirstCapturer = AudioCapturer::Create(capturerOptions);
    unique_ptr<AudioCapturer> audioSecondCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioFirstCapturer);
    ASSERT_NE(nullptr, audioSecondCapturer);

    bool isStarted = audioFirstCapturer->Start();
    EXPECT_EQ(true, isStarted);
    isStarted = audioSecondCapturer->Start();
    EXPECT_EQ(true, isStarted);

    audioCapturerChangeInfos.clear();
    ret = AudioStreamManager::GetInstance()->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(2, static_cast<int32_t>(audioCapturerChangeInfos.size()));
    EXPECT_EQ(2, audioCapturerChangeInfos[0]->capturerState);

    bool isStopped = audioFirstCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    isStopped = audioSecondCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioFirstCapturer->Release();
    EXPECT_EQ(true, isReleased);

    isReleased = audioSecondCapturer->Release();
    EXPECT_EQ(true, isReleased);

    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME));
    audioCapturerChangeInfos.clear();
    ret = AudioStreamManager::GetInstance()->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(0, static_cast<int32_t>(audioCapturerChangeInfos.size()));
}

/**
* @tc.name  : TestGetCurrentCapturerChangeInfos API
* @tc.number: Audio_Stream_Change_Listner_GetCurrentCapturerChangeInfos_008
* @tc.desc  : Test GetCurrentRendererChangeInfos interface to display streams details
*             CAPTURER_PREPARED:1 CAPTURER_RUNNING:2 CAPTURER_STOPPED:3 CAPTURER_RELEASED:4 CAPTURER_PAUSED:5
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_GetCurrentCapturerChangeInfos_008, TestSize.Level1)
{
    int32_t ret = -1;
    vector<unique_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;

    ret = AudioStreamManager::GetInstance()->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    EXPECT_EQ(SUCCESS, ret);

    if (audioCapturerChangeInfos.size() > 0) {
        AUDIO_DEBUG_LOG("AudioStreamManagerTest: audioCapturerChangeInfos Number of entries %{public}u",
        static_cast<int32_t>(audioCapturerChangeInfos.size()));
        uint32_t index = 0;
        for (auto it = audioCapturerChangeInfos.begin(); it != audioCapturerChangeInfos.end(); it++) {
            AudioCapturerChangeInfo audioCapturerChangeInfo = **it;
            AUDIO_DEBUG_LOG("audioCapturerChangeInfos[%{public}d]", index++);
            AUDIO_DEBUG_LOG("clientUID = %{public}d", audioCapturerChangeInfo.clientUID);
            AUDIO_DEBUG_LOG("sessionId = %{public}d", audioCapturerChangeInfo.sessionId);
            AUDIO_DEBUG_LOG("capturerState = %{public}d", audioCapturerChangeInfo.capturerState);
        }
    } else {
        AUDIO_DEBUG_LOG("AudioStreamManagerTest: audioCapturerChangeInfos: No Active Streams");
    }
}

/**
* @tc.name  : Test GetCurrentCapturerChangeInfos API
* @tc.number: Audio_Audio_Stream_Change_Listner_GetCurrentCapturerChangeInfos__Stability_001
* @tc.desc  : Test GetCurrentCapturerChangeInfos interface stability
*             CAPTURER_PREPARED:1 CAPTURER_RUNNING:2 CAPTURER_STOPPED:3 CAPTURER_RELEASED:4 CAPTURER_PAUSED:5
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_GetCurrentCapturerChangeInfos__Stability_001,
    TestSize.Level1)
{
    int32_t ret = -1;
    AudioCapturerOptions capturerOptions;
    vector<unique_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;

    AudioStreamManagerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    for (int32_t i = 0; i < VALUE_HUNDRED; i++) {
        ret = AudioStreamManager::GetInstance()->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
        EXPECT_EQ(SUCCESS, ret);
        EXPECT_EQ(1, static_cast<int32_t>(audioCapturerChangeInfos.size()));
        EXPECT_EQ(2, audioCapturerChangeInfos[0]->capturerState);
        audioCapturerChangeInfos.clear();
    }

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);
    audioCapturerChangeInfos.clear();
    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME));
    ret = AudioStreamManager::GetInstance()->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(0, static_cast<int32_t>(audioCapturerChangeInfos.size()));
}

/**
* @tc.name  : Test GetCurrentCapturerChangeInfos API
* @tc.number: AudioStreamChangeListnerGetCurrentCapturerChangeDeviceInfos_001
* @tc.desc  : Test GetCurrentCapturerChangeInfos interface for getting device information for current stream
*/
HWTEST_F(AudioStreamManagerUnitTest, AudioStreamChangeListnerGetCurrentCapturerChangeDeviceInfos_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioCapturerOptions capturerOptions;
    vector<unique_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;

    AudioStreamManagerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);
    ret = AudioStreamManager::GetInstance()->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(1, static_cast<int32_t>(audioCapturerChangeInfos.size()));
    EXPECT_EQ(2, audioCapturerChangeInfos[0]->capturerState);
    EXPECT_EQ(audioCapturerChangeInfos[0]->inputDeviceInfo.deviceRole, DeviceRole::INPUT_DEVICE);
    EXPECT_EQ(audioCapturerChangeInfos[0]->inputDeviceInfo.deviceType, DeviceType::DEVICE_TYPE_MIC);
    EXPECT_GE(audioCapturerChangeInfos[0]->inputDeviceInfo.deviceId, MIN_DEVICE_ID);
    EXPECT_EQ(true, (*audioCapturerChangeInfos[0]->inputDeviceInfo.audioStreamInfo.samplingRate.rbegin()
        >= SAMPLE_RATE_8000)
        || ((*audioCapturerChangeInfos[0]->inputDeviceInfo.audioStreamInfo.samplingRate.begin() <= SAMPLE_RATE_96000)));
    EXPECT_EQ(audioCapturerChangeInfos[0]->inputDeviceInfo.audioStreamInfo.encoding, AudioEncodingType::ENCODING_PCM);
    EXPECT_EQ(true, (*audioCapturerChangeInfos[0]->inputDeviceInfo.audioStreamInfo.channels.rbegin() >= MONO)
        && ((*audioCapturerChangeInfos[0]->inputDeviceInfo.audioStreamInfo.channels.begin() <= CHANNEL_8)));
    EXPECT_EQ(true, (audioCapturerChangeInfos[0]->inputDeviceInfo.audioStreamInfo.format >= SAMPLE_U8)
        && ((audioCapturerChangeInfos[0]->inputDeviceInfo.audioStreamInfo.format <= SAMPLE_F32LE)));

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);
    audioCapturerChangeInfos.clear();
    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME));
    ret = AudioStreamManager::GetInstance()->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(0, static_cast<int32_t>(audioCapturerChangeInfos.size()));
}

/**
* @tc.name  : Test Feature CapturerStateChangeCallback
* @tc.number: Audio_Stream_Change_Listner_CapturerStateChangeCallbackTest_001
* @tc.desc  : Test CapturerStateChangeCallback interface in prepared state
*             CAPTURER_PREPARED:1 CAPTURER_RUNNING:2 CAPTURER_STOPPED:3 CAPTURER_RELEASED:4 CAPTURER_PAUSED:5
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_CapturerStateChangeCallbackTest_001, TestSize.Level0)
{
    int32_t ret = -1;
    int callBackSetResult;
    std::string testCaseName("Audio_Stream_Change_Listner_CapturerStateChangeCallbackTest_001");
    vector<unique_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;

    g_callbackName = testCaseName;

    auto audioCapturerStateChangeCallbackTest = make_shared<AudioCapturerStateChangeCallbackTest>(testCaseName);
    callBackSetResult = g_audioManagerInstance->RegisterAudioCapturerEventListener(getpid(),
        audioCapturerStateChangeCallbackTest);
    EXPECT_EQ(SUCCESS, callBackSetResult);

    AudioCapturerOptions capturerOptions;
    AudioStreamManagerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);
    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME));
    if (audioCapturer != nullptr) {
        // Wait here for callback. If not callback for 2 mintues, will skip this step
        AudioStreamManagerUnitTest::WaitForCallback();
        g_isCallbackReceived = false;
        EXPECT_EQ(1, static_cast<int32_t>(g_audioCapturerChangeInfosRcvd.size()));
        EXPECT_EQ(1, g_audioCapturerChangeInfosRcvd[0]->capturerState);
        EXPECT_STREQ(g_callbackName.c_str(), testCaseName.c_str());
    }

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);

    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME));
    ret = AudioStreamManager::GetInstance()->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(0, static_cast<int32_t>(audioCapturerChangeInfos.size()));

    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME));
    ret = g_audioManagerInstance->UnregisterAudioCapturerEventListener(getpid());
    EXPECT_EQ(SUCCESS, ret);
    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME));
    audioCapturer.reset();
}

/**
* @tc.name  : Test Feature CapturerStateChangeCallback
* @tc.number: Audio_Stream_Change_Listner_CapturerStateChangeCallbackTest_002
* @tc.desc  : Test CapturerStateChangeCallbackT interface in running state
*             CAPTURER_PREPARED:1 CAPTURER_RUNNING:2 CAPTURER_STOPPED:3 CAPTURER_RELEASED:4 CAPTURER_PAUSED:5
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_CapturerStateChangeCallbackTest_002, TestSize.Level1)
{
    int32_t ret = -1;
    int callBackSetResult;
    std::string testCaseName("Audio_Stream_Change_Listner_CapturerStateChangeCallbackTest_002");
    vector<unique_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;

    g_callbackName = testCaseName;

    auto audioCapturerStateChangeCallbackTest = make_shared<AudioCapturerStateChangeCallbackTest>(testCaseName);
    callBackSetResult = g_audioManagerInstance->RegisterAudioCapturerEventListener(getpid(),
        audioCapturerStateChangeCallbackTest);
    EXPECT_EQ(SUCCESS, callBackSetResult);

    AudioCapturerOptions capturerOptions;
    AudioStreamManagerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);
    if (isStarted == SUCCESS) {
        // Wait here for callback. If not callback for 2 mintues, will skip this step
        AudioStreamManagerUnitTest::WaitForCallback();
        g_isCallbackReceived = false;
        EXPECT_EQ(SUCCESS, ret);
        EXPECT_EQ(1, static_cast<int32_t>(g_audioCapturerChangeInfosRcvd.size()));
        EXPECT_EQ(2, g_audioCapturerChangeInfosRcvd[0]->capturerState);
        EXPECT_STREQ(g_callbackName.c_str(), testCaseName.c_str());
    }

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);

    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME));
    ret = AudioStreamManager::GetInstance()->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(0, static_cast<int32_t>(audioCapturerChangeInfos.size()));

    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME));
    g_audioManagerInstance->UnregisterAudioCapturerEventListener(getpid());
}

/**
* @tc.name  : Test Feature CapturerStateChangeCallback
* @tc.number: Audio_Stream_Change_Listner_CapturerStateChangeCallbackTest_003
* @tc.desc  : Test CapturerStateChangeCallbackT interface in stop state
*             CAPTURER_PREPARED:1 CAPTURER_RUNNING:2 CAPTURER_STOPPED:3 CAPTURER_RELEASED:4 CAPTURER_PAUSED:5
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_CapturerStateChangeCallbackTest_003,
    TestSize.Level1)
{
    int32_t ret = -1;
    [[maybe_unused]] int callBackSetResult;
    std::string testCaseName("Audio_Stream_Change_Listner_CapturerStateChangeCallbackTest_003");
    AudioCapturerOptions capturerOptions;
    vector<unique_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;

    g_callbackName = testCaseName;

    AudioStreamManagerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    auto audioCapturerStateChangeCallbackTest = make_shared<AudioCapturerStateChangeCallbackTest>(testCaseName);
    callBackSetResult = g_audioManagerInstance->RegisterAudioCapturerEventListener(getpid(),
        audioCapturerStateChangeCallbackTest);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);
    if (isStopped == SUCCESS) {
        // Wait here for callback. If not callback for 2 mintues, will skip this step
        AudioStreamManagerUnitTest::WaitForCallback();
        g_isCallbackReceived = false;
        EXPECT_EQ(SUCCESS, ret);
        EXPECT_EQ(1, static_cast<int32_t>(g_audioCapturerChangeInfosRcvd.size()));
        EXPECT_EQ(3, g_audioCapturerChangeInfosRcvd[0]->capturerState);
        EXPECT_STREQ(g_callbackName.c_str(), testCaseName.c_str());
    }

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);

    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME));
    ret = AudioStreamManager::GetInstance()->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(0, static_cast<int32_t>(audioCapturerChangeInfos.size()));

    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME));
    g_audioManagerInstance->UnregisterAudioCapturerEventListener(getpid());
}

/**
* @tc.name  : Test Feature CapturerStateChangeCallback
* @tc.number: Audio_Stream_Change_Listner_CapturerStateChangeCallbackTest_004
* @tc.desc  : Test CapturerStateChangeCallbackT interface in stop state
*             CAPTURER_PREPARED:1 CAPTURER_RUNNING:2 CAPTURER_STOPPED:3 CAPTURER_RELEASED:4 CAPTURER_PAUSED:5
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_Change_Listner_CapturerStateChangeCallbackTest_004, TestSize.Level1)
{
    int32_t ret = -1;
    [[maybe_unused]] int callBackSetResult;
    std::string testCaseName("Audio_Stream_Change_Listner_CapturerStateChangeCallbackTest_004");
    AudioCapturerOptions capturerOptions;
    vector<unique_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;

    g_callbackName = testCaseName;

    AudioStreamManagerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    auto audioCapturerStateChangeCallbackTest = make_shared<AudioCapturerStateChangeCallbackTest>(testCaseName);
    callBackSetResult = g_audioManagerInstance->RegisterAudioCapturerEventListener(getpid(),
        audioCapturerStateChangeCallbackTest);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    bool isStopped = audioCapturer->Stop();
    EXPECT_EQ(true, isStopped);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);
    if (isStarted == SUCCESS) {
        // Wait here for callback. If not callback for 2 mintues, will skip this step
        g_isCallbackReceived = false;
        AudioStreamManagerUnitTest::WaitForCallback();
        EXPECT_EQ(SUCCESS, ret);
        EXPECT_EQ(1, static_cast<int32_t>(g_audioCapturerChangeInfosRcvd.size()));
        EXPECT_EQ(4, g_audioCapturerChangeInfosRcvd[0]->capturerState);
        EXPECT_STREQ(g_callbackName.c_str(), testCaseName.c_str());
    }

    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME));
    ret = AudioStreamManager::GetInstance()->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(0, static_cast<int32_t>(audioCapturerChangeInfos.size()));

    std::this_thread::sleep_for(std::chrono::seconds(WAIT_TIME));
    g_audioManagerInstance->UnregisterAudioCapturerEventListener(getpid());
    std::this_thread::sleep_for(std::chrono::seconds(10));
}

/**
* @tc.name  : Test IsAudioRendererLowLatencySupported API
* @tc.number: Audio_Stream_IsAudioRendererLowLatencySupported_001
* @tc.desc  : Test IsAudioRendererLowLatencySupported interface
*/
HWTEST_F(AudioStreamManagerUnitTest, Audio_Stream_IsAudioRendererLowLatencySupported_001, TestSize.Level1)
{
    AudioCapturerOptions capturerOptions;

    AudioStreamManagerUnitTest::InitializeCapturerOptions(capturerOptions);
    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    AudioStreamInfo audioStreamInfo;
    bool ret = AudioStreamManager::GetInstance()->IsAudioRendererLowLatencySupported(audioStreamInfo);
    EXPECT_EQ(true, ret);

    bool isReleased = audioCapturer->Release();
    EXPECT_EQ(true, isReleased);
}

/**
 * @tc.name  : Test GetHardwareOutputSamplingRate API
 * @tc.type  : FUNC
 * @tc.number: GetHardwareOutputSamplingRate_001
 * @tc.desc  : Test GetHardwareOutputSamplingRate interface.
 */
HWTEST_F(AudioStreamManagerUnitTest, GetHardwareOutputSamplingRate_001, TestSize.Level1)
{
    int32_t ret = VALUE_NEGATIVE;
    sptr<AudioStandard::AudioDeviceDescriptor> desc = new AudioStandard::AudioDeviceDescriptor();
    auto outputDeviceDescriptors = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::OUTPUT_DEVICES_FLAG);
    if (outputDeviceDescriptors.size() > 0) {
        for (auto outputDescriptor : outputDeviceDescriptors) {
            if (outputDescriptor->deviceType_ == DeviceType::DEVICE_TYPE_SPEAKER) {
                desc->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
                desc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
                ret = AudioStreamManager::GetInstance()->GetHardwareOutputSamplingRate(desc);
            }
        }
        EXPECT_NE(VALUE_NEGATIVE, ret);
    }
}

/**
 * @tc.name  : Test GetHardwareOutputSamplingRate API
 * @tc.type  : FUNC
 * @tc.number: GetHardwareOutputSamplingRate_002
 * @tc.desc  : Test GetHardwareOutputSamplingRate interface.
 */
HWTEST_F(AudioStreamManagerUnitTest, GetHardwareOutputSamplingRate_002, TestSize.Level1)
{
    sptr<AudioStandard::AudioDeviceDescriptor> desc = new AudioStandard::AudioDeviceDescriptor();
    int32_t ret = AudioStreamManager::GetInstance()->GetHardwareOutputSamplingRate(desc);
    EXPECT_EQ(VALUE_NEGATIVE, ret);
}

/**
 * @tc.name  : Test GetHardwareOutputSamplingRate API
 * @tc.type  : FUNC
 * @tc.number: GetHardwareOutputSamplingRate_003
 * @tc.desc  : Test GetHardwareOutputSamplingRate interface for inputdevice.
 */
HWTEST_F(AudioStreamManagerUnitTest, GetHardwareOutputSamplingRate_003, TestSize.Level1)
{
    int32_t ret = VALUE_NEGATIVE;
    sptr<AudioStandard::AudioDeviceDescriptor> desc = new AudioStandard::AudioDeviceDescriptor();
    auto outputDeviceDescriptors = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::OUTPUT_DEVICES_FLAG);
    if (outputDeviceDescriptors.size() > 0) {
        for (auto outputDescriptor : outputDeviceDescriptors) {
            desc->deviceType_ = outputDescriptor->deviceType_;
            desc->deviceRole_ = DeviceRole::INPUT_DEVICE;
            ret = AudioStreamManager::GetInstance()->GetHardwareOutputSamplingRate(desc);
        }
        EXPECT_EQ(VALUE_NEGATIVE, ret);
    }
}
} // namespace AudioStandard
} // namespace OHOS
