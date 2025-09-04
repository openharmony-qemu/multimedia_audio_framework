/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

 #include "audio_manager_unit_test.h"

 #include "audio_errors.h"
 #include "audio_info.h"
 #include "audio_renderer.h"
 #include "audio_capturer.h"
 #include "audio_stream_manager.h"
 #include "audio_utils.h"
 
 #include <chrono>
 #include <thread>
 #include <fstream>
 #include <gtest/gtest.h>
 #include <gmock/gmock.h>
 
 using namespace std;
 using namespace testing::ext;
 using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace {
    constexpr uint32_t CONTENT_TYPE_UPPER_INVALID = 1000;
    constexpr uint32_t STREAM_USAGE_UPPER_INVALID = 1000;
    constexpr uint32_t STREAM_TYPE_UPPER_INVALID = 1000;
    constexpr uint32_t CONTENT_TYPE_LOWER_INVALID = -1;
    constexpr uint32_t STREAM_USAGE_LOWER_INVALID = -1;
    constexpr uint32_t STREAM_TYPE_LOWER_INVALID = -1;
    constexpr uid_t UID_PREEMPT_SA = 7015;
    int g_isCallbackReceived = false;
    std::mutex g_mutex;
    std::condition_variable g_condVar;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> g_audioFocusInfoList;
}

void AudioManagerInterruptUnitTest::SetUpTestCase(void) {}
void AudioManagerInterruptUnitTest::TearDownTestCase(void) {}
void AudioManagerInterruptUnitTest::SetUp(void) {}
void AudioManagerInterruptUnitTest::TearDown(void) {}

AudioRendererOptions AudioManagerInterruptUnitTest::InitializeRendererOptionsForMusic()
{
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererOptions.rendererInfo.rendererFlags = 0;
    return rendererOptions;
}

AudioRendererOptions AudioManagerInterruptUnitTest::InitializeRendererOptionsForRing()
{
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_RINGTONE;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_NOTIFICATION_RINGTONE;
    rendererOptions.rendererInfo.rendererFlags = 0;
    return rendererOptions;
}

void AudioManagerInterruptUnitTest::WaitForCallback()
{
    std::unique_lock<std::mutex> lock(g_mutex);
    g_condVar.wait_until(lock, std::chrono::system_clock::now() + std::chrono::minutes(1),
        []() { return g_isCallbackReceived == true; });
}

void AudioFocusInfoChangeCallbackTest::OnAudioFocusInfoChange(
    const std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList)
{
    g_audioFocusInfoList.clear();
    g_audioFocusInfoList = focusInfoList;
    g_isCallbackReceived = true;
}

/**
 * @tc.name   : Test ActivateAudioInterrupt API
 * @tc.number : ActivateAudioInterrupt_001
 * @tc.desc   : Test ActivateAudioInterrupt interface.
 */
HWTEST(AudioManagerInterruptUnitTest, ActivateAudioInterrupt_001, TestSize.Level1)
{
    AudioInterrupt audioInterrupt;
    audioInterrupt.contentType = CONTENT_TYPE_RINGTONE;
    audioInterrupt.streamUsage = STREAM_USAGE_NOTIFICATION_RINGTONE;
    audioInterrupt.audioFocusType.streamType = STREAM_ACCESSIBILITY;
    auto ret = AudioSystemManager::GetInstance()->ActivateAudioInterrupt(audioInterrupt);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test DeactivateAudioInterrupt API
 * @tc.number : DeactivateAudioInterrupt_001
 * @tc.desc   : Test DeactivateAudioInterrupt interface.
 */
HWTEST(AudioManagerInterruptUnitTest, DeactivateAudioInterrupt_001, TestSize.Level1)
{
    AudioInterrupt audioInterrupt;
    audioInterrupt.contentType = CONTENT_TYPE_RINGTONE;
    audioInterrupt.streamUsage = STREAM_USAGE_NOTIFICATION_RINGTONE;
    audioInterrupt.audioFocusType.streamType = STREAM_ACCESSIBILITY;
    auto ret = AudioSystemManager::GetInstance()->DeactivateAudioInterrupt(audioInterrupt);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test ActivatePreemptMode API
 * @tc.number : ActivatePreemptMode_001
 * @tc.desc   : Test ActivatePreemptMode interface, SUCCESS return, if uid: 7015
 */
HWTEST(AudioManagerInterruptUnitTest, ActivatePreemptMode_001, TestSize.Level1)
{
    int32_t setUidRet = setuid(UID_PREEMPT_SA);
    std::cout << "setUidRet: " << setUidRet << std::endl;
    auto ret = AudioSystemManager::GetInstance()->ActivatePreemptMode();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test ActivatePreemptMode API
 * @tc.number : ActivatePreemptMode_002
 * @tc.desc   : Test ActivatePreemptMode interface, ERROR return, if not preempt uid: 7015
 */
HWTEST(AudioManagerInterruptUnitTest, ActivatePreemptMode_002, TestSize.Level1)
{
    int32_t setUidRet = setuid(0);
    std::cout << "setUidRet: " << setUidRet << std::endl;
    auto ret = AudioSystemManager::GetInstance()->ActivatePreemptMode();
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name   : Test DeactivatePreemptMode API
 * @tc.number : DeactivatePreemptMode_001
 * @tc.desc   : Test DeactivatePreemptMode interface, SUCCESS return, if uid: 7015
 */
HWTEST(AudioManagerInterruptUnitTest, DeactivatePreemptMode_001, TestSize.Level1)
{
    int32_t setUidRet = setuid(UID_PREEMPT_SA);
    std::cout << "setUidRet: " << setUidRet << std::endl;
    auto ret = AudioSystemManager::GetInstance()->DeactivatePreemptMode();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test DeactivatePreemptMode API
 * @tc.number : DeactivatePreemptMode_002
 * @tc.desc   : Test DeactivatePreemptMode interface, ERROR return, if not preempt uid: 7015
 */
HWTEST(AudioManagerInterruptUnitTest, DeactivatePreemptMode_002, TestSize.Level1)
{
    int32_t setUidRet = setuid(0);
    std::cout << "setUidRet: " << setUidRet << std::endl;
    auto ret = AudioSystemManager::GetInstance()->DeactivatePreemptMode();
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name   : Test SetAudioInterruptCallback API
 * @tc.number : SetAudioInterruptCallback_001
 * @tc.desc   : Test SetAudioInterruptCallback interface.
 */
HWTEST(AudioManagerInterruptUnitTest, SetAudioInterruptCallback_001, TestSize.Level1)
{
    uint32_t sessionId = 0;
    std::shared_ptr<AudioInterruptCallback> callback = nullptr;
    uint32_t clientUid = 1;
    int32_t zoneID = 1;
    auto ret = AudioSystemManager::GetInstance()->SetAudioInterruptCallback(sessionId, callback,
        clientUid, zoneID);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
 * @tc.name   : Test UnsetAudioInterruptCallback API
 * @tc.number : UnsetAudioInterruptCallback_001
 * @tc.desc   : Test UnsetAudioInterruptCallback interface.
 */
HWTEST(AudioManagerInterruptUnitTest, UnsetAudioInterruptCallback_001, TestSize.Level1)
{
    int32_t zoneId = 1;
    uint32_t sessionId = 1;
    auto ret = AudioSystemManager::GetInstance()->UnsetAudioInterruptCallback(zoneId, sessionId);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
 * @tc.name   : Test RequestIndependentInterrupt API
 * @tc.number : RequestIndependentInterrupt_001
 * @tc.desc   : Test RequestIndependentInterrupt interface.
 */
HWTEST(AudioManagerInterruptUnitTest, RequestIndependentInterrupt_001, TestSize.Level1)
{
    FocusType FocusType = FocusType::FOCUS_TYPE_RECORDING;
    auto ret = AudioSystemManager::GetInstance()->RequestIndependentInterrupt(FocusType);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name   : Test AbandonIndependentInterrupt API
 * @tc.number : AbandonIndependentInterrupt_001
 * @tc.desc   : Test AbandonIndependentInterrupt interface.
 */
HWTEST(AudioManagerInterruptUnitTest, AbandonIndependentInterrupt_001, TestSize.Level1)
{
    FocusType FocusType = FocusType::FOCUS_TYPE_RECORDING;
    auto ret = AudioSystemManager::GetInstance()->AbandonIndependentInterrupt(FocusType);
    EXPECT_TRUE(ret);
}

/**
* @tc.name   : Test SetAudioManagerInterruptCallback API
* @tc.number : SetAudioManagerInterruptCallback_001
* @tc.desc   : Test SetAudioManagerInterruptCallback interface with valid parameters
*/
HWTEST(AudioManagerInterruptUnitTest, SetAudioManagerInterruptCallback_001, TestSize.Level1)
{
    shared_ptr<AudioManagerCallback> interruptCallback = make_shared<AudioManagerCallbackImpl>();
    auto ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallback);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name   : Test SetAudioManagerInterruptCallback API
* @tc.number : SetAudioManagerInterruptCallback_002
* @tc.desc   : Test SetAudioManagerInterruptCallback interface with null callback pointer as parameter
*/
HWTEST(AudioManagerInterruptUnitTest, SetAudioManagerInterruptCallback_002, TestSize.Level1)
{
    auto ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(nullptr);
    EXPECT_NE(SUCCESS, ret);
}

/**
* @tc.name   : Test SetAudioManagerInterruptCallback API
* @tc.number : SetAudioManagerInterruptCallback_003
* @tc.desc   : Test SetAudioManagerInterruptCallback interface with Multiple Set
*/
HWTEST(AudioManagerInterruptUnitTest, SetAudioManagerInterruptCallback_003, TestSize.Level1)
{
    shared_ptr<AudioManagerCallback> interruptCallback = make_shared<AudioManagerCallbackImpl>();
    auto ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallback);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallback);
    EXPECT_EQ(SUCCESS, ret);
    shared_ptr<AudioManagerCallback> interruptCallbackNew = make_shared<AudioManagerCallbackImpl>();
    ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallbackNew);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name   : Test UnsetAudioManagerInterruptCallback API
* @tc.number : UnsetAudioManagerInterruptCallback_001
* @tc.desc   : Test UnsetAudioManagerInterruptCallback interface with Set and Unset callback
*/
HWTEST(AudioManagerInterruptUnitTest, UnsetAudioManagerInterruptCallback_001, TestSize.Level1)
{
    shared_ptr<AudioManagerCallback> interruptCallback = make_shared<AudioManagerCallbackImpl>();
    auto ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallback);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->UnsetAudioManagerInterruptCallback();
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name   : Test UnsetAudioManagerInterruptCallback API
* @tc.number : UnsetAudioManagerInterruptCallback_002
* @tc.desc   : Test UnsetAudioManagerInterruptCallback interface with Multiple Unset
*/
HWTEST(AudioManagerInterruptUnitTest, UnsetAudioManagerInterruptCallback_002, TestSize.Level1)
{
    shared_ptr<AudioManagerCallback> interruptCallback = make_shared<AudioManagerCallbackImpl>();
    auto ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallback);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->UnsetAudioManagerInterruptCallback();
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->UnsetAudioManagerInterruptCallback();
    EXPECT_EQ(ERR_INVALID_OPERATION, ret);
}

/**
* @tc.name   : Test UnsetAudioManagerInterruptCallback API
* @tc.number : UnsetAudioManagerInterruptCallback_003
* @tc.desc   : Test UnsetAudioManagerInterruptCallback interface without set interrupt call
*/
HWTEST(AudioManagerInterruptUnitTest, UnsetAudioManagerInterruptCallback_003, TestSize.Level1)
{
    auto ret = AudioSystemManager::GetInstance()->UnsetAudioManagerInterruptCallback();
    EXPECT_EQ(ERR_INVALID_OPERATION, ret);
}

/**
* @tc.name   : Test RequestAudioFocus API
* @tc.number : RequestAudioFocus_001
* @tc.desc   : Test RequestAudioFocus interface with valid parameters
*/
HWTEST(AudioManagerInterruptUnitTest, RequestAudioFocus_001, TestSize.Level1)
{
    AudioInterrupt audioInterrupt;
    audioInterrupt.contentType = CONTENT_TYPE_RINGTONE;
    audioInterrupt.streamUsage = STREAM_USAGE_NOTIFICATION_RINGTONE;
    audioInterrupt.audioFocusType.streamType = STREAM_ACCESSIBILITY;
    shared_ptr<AudioManagerCallback> interruptCallback = make_shared<AudioManagerCallbackImpl>();
    auto ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallback);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name   : Test RequestAudioFocus API
* @tc.number : RequestAudioFocus_002
* @tc.desc   : Test RequestAudioFocus interface with invalid parameters
*/
HWTEST(AudioManagerInterruptUnitTest, RequestAudioFocus_002, TestSize.Level1)
{
    AudioInterrupt audioInterrupt;
    constexpr int32_t INVALID_CONTENT_TYPE = 10;
    audioInterrupt.contentType = static_cast<ContentType>(INVALID_CONTENT_TYPE);
    audioInterrupt.streamUsage = STREAM_USAGE_NOTIFICATION_RINGTONE;
    audioInterrupt.audioFocusType.streamType = STREAM_ACCESSIBILITY;
    shared_ptr<AudioManagerCallback> interruptCallback = make_shared<AudioManagerCallbackImpl>();
    auto ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallback);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_NE(SUCCESS, ret);
}

/**
* @tc.name   : Test RequestAudioFocus API
* @tc.number : RequestAudioFocus_003
* @tc.desc   : Test RequestAudioFocus interface with boundary values for content type, stream usage
*             and stream type
*/
HWTEST(AudioManagerInterruptUnitTest, RequestAudioFocus_003, TestSize.Level1)
{
    AudioInterrupt audioInterrupt;
    audioInterrupt.contentType = static_cast<ContentType>(CONTENT_TYPE_UPPER_INVALID);
    audioInterrupt.streamUsage = STREAM_USAGE_UNKNOWN;
    audioInterrupt.audioFocusType.streamType = STREAM_VOICE_CALL;

    shared_ptr<AudioManagerCallback> interruptCallback = make_shared<AudioManagerCallbackImpl>();
    auto ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallback);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_NE(SUCCESS, ret);
    audioInterrupt.contentType = static_cast<ContentType>(CONTENT_TYPE_LOWER_INVALID);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_NE(SUCCESS, ret);

    audioInterrupt.contentType = CONTENT_TYPE_RINGTONE;
    audioInterrupt.streamUsage = static_cast<StreamUsage>(STREAM_USAGE_UPPER_INVALID);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_NE(SUCCESS, ret);
    audioInterrupt.streamUsage = static_cast<StreamUsage>(STREAM_USAGE_LOWER_INVALID);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_NE(SUCCESS, ret);

    audioInterrupt.streamUsage = STREAM_USAGE_NOTIFICATION_RINGTONE;
    audioInterrupt.audioFocusType.streamType = static_cast<AudioStreamType>(STREAM_TYPE_UPPER_INVALID);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_NE(SUCCESS, ret);
    audioInterrupt.audioFocusType.streamType = static_cast<AudioStreamType>(STREAM_TYPE_LOWER_INVALID);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_NE(SUCCESS, ret);

    audioInterrupt.contentType = CONTENT_TYPE_UNKNOWN;
    audioInterrupt.streamUsage = STREAM_USAGE_UNKNOWN;
    audioInterrupt.audioFocusType.streamType = STREAM_VOICE_CALL;
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);

    audioInterrupt.contentType = CONTENT_TYPE_RINGTONE;
    audioInterrupt.streamUsage = STREAM_USAGE_NOTIFICATION_RINGTONE;
    audioInterrupt.audioFocusType.streamType = STREAM_ACCESSIBILITY;
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name   : Test RequestAudioFocus API
* @tc.number : RequestAudioFocus_004
* @tc.desc   : Test RequestAudioFocus interface with back to back requests
*/
HWTEST(AudioManagerInterruptUnitTest, RequestAudioFocus_004, TestSize.Level1)
{
    AudioInterrupt audioInterrupt;
    audioInterrupt.contentType = CONTENT_TYPE_RINGTONE;
    audioInterrupt.streamUsage = STREAM_USAGE_NOTIFICATION_RINGTONE;
    audioInterrupt.audioFocusType.streamType = STREAM_ACCESSIBILITY;
    shared_ptr<AudioManagerCallback> interruptCallback = make_shared<AudioManagerCallbackImpl>();
    auto ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallback);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
    shared_ptr<AudioManagerCallback> interruptCallbackNew = make_shared<AudioManagerCallbackImpl>();
    ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallbackNew);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name   : Test AbandonAudioFocus API
* @tc.number : AbandonAudioFocus_001
* @tc.desc   : Test AbandonAudioFocus interface with valid parameters
*/
HWTEST(AudioManagerInterruptUnitTest, AbandonAudioFocus_001, TestSize.Level1)
{
    AudioInterrupt audioInterrupt;
    audioInterrupt.contentType = CONTENT_TYPE_RINGTONE;
    audioInterrupt.streamUsage = STREAM_USAGE_NOTIFICATION_RINGTONE;
    audioInterrupt.audioFocusType.streamType = STREAM_ACCESSIBILITY;
    shared_ptr<AudioManagerCallback> interruptCallback = make_shared<AudioManagerCallbackImpl>();
    auto ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallback);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->AbandonAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name   : Test AbandonAudioFocus API
* @tc.number : AbandonAudioFocus_002
* @tc.desc   : Test AbandonAudioFocus interface with invalid parameters
*/
HWTEST(AudioManagerInterruptUnitTest, AbandonAudioFocus_002, TestSize.Level1)
{
    AudioInterrupt audioInterrupt;
    audioInterrupt.contentType = CONTENT_TYPE_RINGTONE;
    audioInterrupt.streamUsage = STREAM_USAGE_NOTIFICATION_RINGTONE;
    audioInterrupt.audioFocusType.streamType = STREAM_ACCESSIBILITY;
    shared_ptr<AudioManagerCallback> interruptCallback = make_shared<AudioManagerCallbackImpl>();
    auto ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallback);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
    constexpr int32_t INVALID_CONTENT_TYPE = 10;
    audioInterrupt.contentType = static_cast<ContentType>(INVALID_CONTENT_TYPE);
    ret = AudioSystemManager::GetInstance()->AbandonAudioFocus(audioInterrupt);
    EXPECT_NE(SUCCESS, ret);
}

/**
* @tc.name   : Test AbandonAudioFocus API
* @tc.number : AbandonAudioFocus_003
* @tc.desc   : Test AbandonAudioFocus interface with invalid parameters
*/
HWTEST(AudioManagerInterruptUnitTest, AbandonAudioFocus_003, TestSize.Level1)
{
    AudioInterrupt audioInterrupt;
    audioInterrupt.contentType = CONTENT_TYPE_RINGTONE;
    audioInterrupt.streamUsage = STREAM_USAGE_NOTIFICATION_RINGTONE;
    audioInterrupt.audioFocusType.streamType = STREAM_ACCESSIBILITY;

    shared_ptr<AudioManagerCallback> interruptCallback = make_shared<AudioManagerCallbackImpl>();
    auto ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallback);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);

    audioInterrupt.contentType = static_cast<ContentType>(CONTENT_TYPE_UPPER_INVALID);
    ret = AudioSystemManager::GetInstance()->AbandonAudioFocus(audioInterrupt);
    EXPECT_NE(SUCCESS, ret);
    audioInterrupt.contentType = static_cast<ContentType>(CONTENT_TYPE_LOWER_INVALID);
    ret = AudioSystemManager::GetInstance()->AbandonAudioFocus(audioInterrupt);
    EXPECT_NE(SUCCESS, ret);

    audioInterrupt.contentType = CONTENT_TYPE_RINGTONE;
    audioInterrupt.streamUsage = static_cast<StreamUsage>(STREAM_USAGE_UPPER_INVALID);
    ret = AudioSystemManager::GetInstance()->AbandonAudioFocus(audioInterrupt);
    EXPECT_NE(SUCCESS, ret);
    audioInterrupt.streamUsage = static_cast<StreamUsage>(STREAM_USAGE_LOWER_INVALID);
    ret = AudioSystemManager::GetInstance()->AbandonAudioFocus(audioInterrupt);
    EXPECT_NE(SUCCESS, ret);

    audioInterrupt.streamUsage = STREAM_USAGE_NOTIFICATION_RINGTONE;
    audioInterrupt.audioFocusType.streamType = static_cast<AudioStreamType>(STREAM_TYPE_UPPER_INVALID);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_NE(SUCCESS, ret);
    audioInterrupt.audioFocusType.streamType = static_cast<AudioStreamType>(STREAM_TYPE_LOWER_INVALID);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_NE(SUCCESS, ret);


    audioInterrupt.contentType = CONTENT_TYPE_UNKNOWN;
    audioInterrupt.streamUsage = STREAM_USAGE_UNKNOWN;
    audioInterrupt.audioFocusType.streamType = STREAM_VOICE_CALL;
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->AbandonAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);

    audioInterrupt.contentType = CONTENT_TYPE_RINGTONE;
    audioInterrupt.streamUsage = STREAM_USAGE_NOTIFICATION_RINGTONE;
    audioInterrupt.audioFocusType.streamType = STREAM_VOICE_CALL;
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->AbandonAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name   : Test AbandonAudioFocus API
* @tc.number : AbandonAudioFocus_004
* @tc.desc   : Test AbandonAudioFocus interface multiple requests
*/
HWTEST(AudioManagerInterruptUnitTest, AbandonAudioFocus_004, TestSize.Level1)
{
    AudioInterrupt audioInterrupt;
    audioInterrupt.contentType = CONTENT_TYPE_RINGTONE;
    audioInterrupt.streamUsage = STREAM_USAGE_NOTIFICATION_RINGTONE;
    audioInterrupt.audioFocusType.streamType = STREAM_ACCESSIBILITY;
    shared_ptr<AudioManagerCallback> interruptCallback = make_shared<AudioManagerCallbackImpl>();
    auto ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallback);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->AbandonAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->AbandonAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
    shared_ptr<AudioManagerCallback> interruptCallbackNew = make_shared<AudioManagerCallbackImpl>();
    ret = AudioSystemManager::GetInstance()->SetAudioManagerInterruptCallback(interruptCallbackNew);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->RequestAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->AbandonAudioFocus(audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name    : GetAudioFocusInfoList_001
 * @tc.desc    : Test get audio focus info list
 * @tc.type    : FUNC
 * @tc.require : issueI6GYJT
 */
HWTEST(AudioManagerInterruptUnitTest, GetAudioFocusInfoList_001, TestSize.Level1)
{
    std::list<std::pair<AudioInterrupt, AudioFocuState>> focusInfoList;
    int32_t ret = AudioSystemManager::GetInstance()->GetAudioFocusInfoList(focusInfoList);

    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name    : GetAudioFocusInfoList_002
 * @tc.desc    : Test get audio focus info list
 * @tc.type    : FUNC
 * @tc.require : issueI6GYJT
 */
HWTEST(AudioManagerInterruptUnitTest, GetAudioFocusInfoList_002, TestSize.Level1)
{
    AudioRendererOptions ringOptions = AudioManagerInterruptUnitTest::InitializeRendererOptionsForRing();
    unique_ptr<AudioRenderer> audioRendererForRing = AudioRenderer::Create(ringOptions);
    ASSERT_NE(nullptr, audioRendererForRing);

    std::list<std::pair<AudioInterrupt, AudioFocuState>> focusInfoList = {};
    AudioSystemManager::GetInstance()->GetAudioFocusInfoList(focusInfoList);
    EXPECT_EQ(focusInfoList.size(), 0);

    bool isStartedforRing = audioRendererForRing->Start();
    EXPECT_EQ(true, isStartedforRing);

    int32_t ret = AudioSystemManager::GetInstance()->GetAudioFocusInfoList(focusInfoList);
    EXPECT_EQ(focusInfoList.size(), 1);
    for (auto it = focusInfoList.begin(); it != focusInfoList.end(); ++it) {
        EXPECT_EQ(it->first.audioFocusType.streamType, AudioStreamType::STREAM_RING);
        EXPECT_EQ(it->second, AudioFocuState::ACTIVE);
    }

    AudioRendererOptions musicOptions = AudioManagerInterruptUnitTest::InitializeRendererOptionsForMusic();
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(musicOptions);
    ASSERT_NE(nullptr, audioRenderer);
    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);
    ret = AudioSystemManager::GetInstance()->GetAudioFocusInfoList(focusInfoList);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(focusInfoList.size(), 3);
    for (auto it = focusInfoList.begin(); it != focusInfoList.end(); ++it) {
        if (it->first.audioFocusType.streamType == AudioStreamType::STREAM_RING) {
            EXPECT_EQ(it->second, AudioFocuState::ACTIVE);
        } else if (it->first.audioFocusType.streamType == AudioStreamType::STREAM_MUSIC) {
            EXPECT_EQ(it->second, AudioFocuState::ACTIVE);
        } else {
            EXPECT_TRUE(false);
        }
    }

    audioRendererForRing->Stop();
    audioRendererForRing->Release();
    ret = AudioSystemManager::GetInstance()->GetAudioFocusInfoList(focusInfoList);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(focusInfoList.size(), 4);
    for (auto it = focusInfoList.begin(); it != focusInfoList.end(); ++it) {
        if (it->first.audioFocusType.streamType == AudioStreamType::STREAM_MUSIC) {
            EXPECT_EQ(it->second, AudioFocuState::ACTIVE);
        }
    }

    audioRenderer->Stop();
    audioRenderer->Release();
    ret = AudioSystemManager::GetInstance()->GetAudioFocusInfoList(focusInfoList);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(focusInfoList.size(), 4);
}

/**
 * @tc.name    : RegisterFocusInfoChangeCallback_001
 * @tc.desc    : Test register focus info change callback
 * @tc.type    : FUNC
 * @tc.require : issueI6GYJT
 */
HWTEST(AudioManagerInterruptUnitTest, RegisterFocusInfoChangeCallback_001, TestSize.Level1)
{
    std::shared_ptr<AudioFocusInfoChangeCallback> callback = make_shared<AudioFocusInfoChangeCallbackTest>();
    auto ret = AudioSystemManager::GetInstance()->RegisterFocusInfoChangeCallback(callback);
    EXPECT_EQ(ret, SUCCESS);

    ret = AudioSystemManager::GetInstance()->UnregisterFocusInfoChangeCallback();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name    : RegisterFocusInfoChangeCallback_002
 * @tc.desc    : Test register focus info change callback
 * @tc.type    : FUNC
 * @tc.require : issueI6GYJT
 */
HWTEST(AudioManagerInterruptUnitTest, RegisterFocusInfoChangeCallback_002, TestSize.Level1)
{
    std::shared_ptr<AudioFocusInfoChangeCallback> callback = make_shared<AudioFocusInfoChangeCallbackTest>();
    auto ret = AudioSystemManager::GetInstance()->RegisterFocusInfoChangeCallback(callback);
    EXPECT_EQ(ret, SUCCESS);

    AudioRendererOptions musicOptions1 = AudioManagerInterruptUnitTest::InitializeRendererOptionsForMusic();
    AudioRendererOptions musicOptions2 = AudioManagerInterruptUnitTest::InitializeRendererOptionsForMusic();
    unique_ptr<AudioRenderer> audioRenderer1 = AudioRenderer::Create(musicOptions1);
    ASSERT_NE(nullptr, audioRenderer1);

    bool isStarted = audioRenderer1->Start();
    EXPECT_EQ(true, isStarted);

    uint32_t streamId1 = -1;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    if (audioRenderer1 != nullptr) {
        // Wait here for callback. If not callback for 2 mintues, will skip this step
        AudioManagerInterruptUnitTest::WaitForCallback();
        g_isCallbackReceived = false;
        EXPECT_EQ(g_audioFocusInfoList.size(), 1);
        for (auto it = g_audioFocusInfoList.begin(); it != g_audioFocusInfoList.end(); ++it) {
            if (it->first.audioFocusType.streamType == AudioStreamType::STREAM_MUSIC) {
                streamId1 = it->first.streamId;
                EXPECT_EQ(it->second, AudioFocuState::ACTIVE);
            } else {
                EXPECT_TRUE(false);
            }
        }
    }

    unique_ptr<AudioRenderer> audioRenderer2 = AudioRenderer::Create(musicOptions2);
    ASSERT_NE(nullptr, audioRenderer2);
    isStarted = audioRenderer2->Start();
    EXPECT_EQ(true, isStarted);
    audioRenderer1->Stop();
    audioRenderer1->Release();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    if (audioRenderer1 != nullptr && audioRenderer2 != nullptr) {
        // Wait here for callback. If not callback for 2 mintues, will skip this step
        AudioManagerInterruptUnitTest::WaitForCallback();
        g_isCallbackReceived = false;
        EXPECT_EQ(g_audioFocusInfoList.size(), 1);
        for (auto it = g_audioFocusInfoList.begin(); it != g_audioFocusInfoList.end(); ++it) {
            if (it->first.audioFocusType.streamType == AudioStreamType::STREAM_MUSIC) {
                EXPECT_EQ(it->second, AudioFocuState::ACTIVE);
                EXPECT_TRUE(streamId1 != it->first.streamId);
            }
        }
    }

    audioRenderer2->Stop();
    audioRenderer2->Release();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    if (audioRenderer1 != nullptr) {
        // Wait here for callback. If not callback for 2 mintues, will skip this step
        AudioManagerInterruptUnitTest::WaitForCallback();
        g_isCallbackReceived = false;
        EXPECT_EQ(g_audioFocusInfoList.size(), 0);
    }

    ret = AudioSystemManager::GetInstance()->UnregisterFocusInfoChangeCallback(callback);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name    : RegisterFocusInfoChangeCallback_003
 * @tc.desc    : Test register focus info change callback
 * @tc.type    : FUNC
 * @tc.require : issueI6GYJT
 */
HWTEST(AudioManagerInterruptUnitTest, RegisterFocusInfoChangeCallback_003, TestSize.Level1)
{
    std::shared_ptr<AudioFocusInfoChangeCallback> callback = make_shared<AudioFocusInfoChangeCallbackTest>();
    auto ret = AudioSystemManager::GetInstance()->RegisterFocusInfoChangeCallback(callback);
    EXPECT_EQ(ret, SUCCESS);

    AudioRendererOptions ringOptions = AudioManagerInterruptUnitTest::InitializeRendererOptionsForRing();
    unique_ptr<AudioRenderer> audioRendererForRing = AudioRenderer::Create(ringOptions);
    ASSERT_NE(nullptr, audioRendererForRing);
    bool isStartedforRing = audioRendererForRing->Start();
    EXPECT_EQ(true, isStartedforRing);

    std::this_thread::sleep_for(std::chrono::seconds(2));
    if (audioRendererForRing != nullptr) {
        // Wait here for callback. If not callback for 2 mintues, will skip this step
        AudioManagerInterruptUnitTest::WaitForCallback();
        g_isCallbackReceived = false;
        EXPECT_EQ(g_audioFocusInfoList.size(), 1);
        for (auto it = g_audioFocusInfoList.begin(); it != g_audioFocusInfoList.end(); ++it) {
            if (it->first.audioFocusType.streamType == AudioStreamType::STREAM_RING) {
                EXPECT_EQ(it->second, AudioFocuState::ACTIVE);
            } else {
                EXPECT_TRUE(false);
            }
        }
    }

    AudioRendererOptions musicOptions = AudioManagerInterruptUnitTest::InitializeRendererOptionsForMusic();
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(musicOptions);
    ASSERT_NE(nullptr, audioRenderer);
    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    std::this_thread::sleep_for(std::chrono::seconds(2));
    if (audioRenderer != nullptr && audioRendererForRing != nullptr) {
        // Wait here for callback. If not callback for 2 mintues, will skip this step
        AudioManagerInterruptUnitTest::WaitForCallback();
        g_isCallbackReceived = false;
        EXPECT_EQ(g_audioFocusInfoList.size(), 2);
        for (auto it = g_audioFocusInfoList.begin(); it != g_audioFocusInfoList.end(); ++it) {
            if (it->first.audioFocusType.streamType == AudioStreamType::STREAM_RING) {
                EXPECT_EQ(it->second, AudioFocuState::ACTIVE);
            }
        }
    }

    audioRenderer->Stop();
    audioRenderer->Release();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    if (audioRendererForRing != nullptr) {
        // Wait here for callback. If not callback for 2 mintues, will skip this step
        AudioManagerInterruptUnitTest::WaitForCallback();
        g_isCallbackReceived = false;
        EXPECT_EQ(g_audioFocusInfoList.size(), 1);
        for (auto it = g_audioFocusInfoList.begin(); it != g_audioFocusInfoList.end(); ++it) {
            if (it->first.audioFocusType.streamType == AudioStreamType::STREAM_RING) {
                EXPECT_EQ(it->second, AudioFocuState::ACTIVE);
            } else {
                EXPECT_TRUE(false);
            }
        }
    }

    audioRendererForRing->Stop();
    audioRendererForRing->Release();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    if (audioRendererForRing != nullptr) {
        // Wait here for callback. If not callback for 2 mintues, will skip this step
        AudioManagerInterruptUnitTest::WaitForCallback();
        g_isCallbackReceived = false;
        EXPECT_EQ(g_audioFocusInfoList.size(), 0);
    }

    ret = AudioSystemManager::GetInstance()->UnregisterFocusInfoChangeCallback(callback);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name    : RegisterFocusInfoChangeCallback_004
 * @tc.desc    : Test register focus info change callback
 * @tc.type    : FUNC
 * @tc.require : issueI6GYJT
 */
HWTEST(AudioManagerInterruptUnitTest, RegisterFocusInfoChangeCallback_004, TestSize.Level1)
{
    std::shared_ptr<AudioFocusInfoChangeCallback> callback = make_shared<AudioFocusInfoChangeCallbackTest>();
    auto ret = AudioSystemManager::GetInstance()->RegisterFocusInfoChangeCallback(callback);
    EXPECT_EQ(ret, SUCCESS);

    AudioRendererOptions musicOptions = AudioManagerInterruptUnitTest::InitializeRendererOptionsForMusic();
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(musicOptions);
    ASSERT_NE(nullptr, audioRenderer);
    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    std::this_thread::sleep_for(std::chrono::seconds(2));
    if (audioRenderer != nullptr) {
        // Wait here for callback. If not callback for 2 mintues, will skip this step
        AudioManagerInterruptUnitTest::WaitForCallback();
        g_isCallbackReceived = false;
        EXPECT_EQ(g_audioFocusInfoList.size(), 1);
        for (auto it = g_audioFocusInfoList.begin(); it != g_audioFocusInfoList.end(); ++it) {
            if (it->first.audioFocusType.streamType == AudioStreamType::STREAM_MUSIC) {
                EXPECT_EQ(it->second, AudioFocuState::ACTIVE);
            } else {
                EXPECT_TRUE(false);
            }
        }
    }

    AudioRendererOptions ringOptions = AudioManagerInterruptUnitTest::InitializeRendererOptionsForRing();
    unique_ptr<AudioRenderer> audioRendererForRing = AudioRenderer::Create(ringOptions);
    ASSERT_NE(nullptr, audioRendererForRing);
    bool isStartedforRing = audioRendererForRing->Start();
    EXPECT_EQ(true, isStartedforRing);

    std::this_thread::sleep_for(std::chrono::seconds(2));
    if (audioRendererForRing != nullptr) {
        // Wait here for callback. If not callback for 2 mintues, will skip this step
        AudioManagerInterruptUnitTest::WaitForCallback();
        g_isCallbackReceived = false;
        EXPECT_EQ(g_audioFocusInfoList.size(), 2);
        for (auto it = g_audioFocusInfoList.begin(); it != g_audioFocusInfoList.end(); ++it) {
            if (it->first.audioFocusType.streamType == AudioStreamType::STREAM_RING) {
                EXPECT_EQ(it->second, AudioFocuState::ACTIVE);
            }
        }
    }

    audioRendererForRing->Stop();
    audioRendererForRing->Release();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    if (audioRenderer != nullptr) {
        // Wait here for callback. If not callback for 2 mintues, will skip this step
        AudioManagerInterruptUnitTest::WaitForCallback();
        g_isCallbackReceived = false;
        EXPECT_EQ(g_audioFocusInfoList.size(), 1);
        for (auto it = g_audioFocusInfoList.begin(); it != g_audioFocusInfoList.end(); ++it) {
            if (it->first.audioFocusType.streamType == AudioStreamType::STREAM_MUSIC) {
                EXPECT_EQ(it->second, AudioFocuState::ACTIVE);
            } else {
                EXPECT_TRUE(false);
            }
        }
    }

    audioRenderer->Stop();
    audioRenderer->Release();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    if (audioRenderer != nullptr) {
        // Wait here for callback. If not callback for 2 mintues, will skip this step
        AudioManagerInterruptUnitTest::WaitForCallback();
        g_isCallbackReceived = false;
        EXPECT_EQ(g_audioFocusInfoList.size(), 0);
    }

    ret = AudioSystemManager::GetInstance()->UnregisterFocusInfoChangeCallback(callback);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name    : RegisterFocusInfoChangeCallback_005
 * @tc.desc    : Test register two focus info change callback
 * @tc.type    : FUNC
 * @tc.require : issueI6GYJT
 */
HWTEST(AudioManagerInterruptUnitTest, RegisterFocusInfoChangeCallback_005, TestSize.Level1)
{
    std::shared_ptr<AudioFocusInfoChangeCallback> callback1 = make_shared<AudioFocusInfoChangeCallbackTest>();
    std::shared_ptr<AudioFocusInfoChangeCallback> callback2 = make_shared<AudioFocusInfoChangeCallbackTest>();
    auto ret = AudioSystemManager::GetInstance()->RegisterFocusInfoChangeCallback(callback1);
    EXPECT_EQ(ret, SUCCESS);
    ret = AudioSystemManager::GetInstance()->RegisterFocusInfoChangeCallback(callback2);
    EXPECT_EQ(ret, SUCCESS);

    AudioRendererOptions musicOptions = AudioManagerInterruptUnitTest::InitializeRendererOptionsForMusic();
    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(musicOptions);
    ASSERT_NE(nullptr, audioRenderer);
    bool isStarted = audioRenderer->Start();
    EXPECT_EQ(true, isStarted);

    std::list<std::pair<AudioInterrupt, AudioFocuState>> focusInfoList;
    ret = AudioSystemManager::GetInstance()->GetAudioFocusInfoList(focusInfoList);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(focusInfoList.size(), 1);

    std::this_thread::sleep_for(std::chrono::seconds(2));
    if (audioRenderer != nullptr) {
        // Wait here for callback. If not callback for 2 mintues, will skip this step
        AudioManagerInterruptUnitTest::WaitForCallback();
        g_isCallbackReceived = false;
        EXPECT_EQ(g_audioFocusInfoList.size(), 1);
    }

    ret = AudioSystemManager::GetInstance()->UnregisterFocusInfoChangeCallback(callback1);
    EXPECT_EQ(ret, SUCCESS);

    audioRenderer->Stop();
    audioRenderer->Release();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    if (audioRenderer != nullptr) {
        // Wait here for callback. If not callback for 2 mintues, will skip this step
        AudioManagerInterruptUnitTest::WaitForCallback();
        g_isCallbackReceived = false;
        EXPECT_EQ(g_audioFocusInfoList.size(), 0);
    }

    ret = AudioSystemManager::GetInstance()->UnregisterFocusInfoChangeCallback(callback2);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name    : SetAppConcurrencyMode_001
 * @tc.desc    : Test set app concurrency mode
 * @tc.type    : FUNC
 * @tc.require : issueICU2E0
 */
HWTEST(AudioManagerInterruptUnitTest, SetAppConcurrencyMode_001, TestSize.Level1)
{
    int32_t uid = 0;
    int32_t mode = 0;
    auto ret = AudioSystemManager::GetInstance()->SetAppConcurrencyMode(uid, mode);
    EXPECT_EQ(ret, ERR_SYSTEM_PERMISSION_DENIED);
}

/**
 * @tc.name    : SetAppSilentOnDisplay_001
 * @tc.desc    : Test set app slient on display
 * @tc.type    : FUNC
 * @tc.require : issueICU2E0
 */
HWTEST(AudioManagerInterruptUnitTest, SetAppSilentOnDisplay_001, TestSize.Level1)
{
    int32_t displayId = 0;
    auto ret = AudioSystemManager::GetInstance()->SetAppSilentOnDisplay(displayId);
    EXPECT_EQ(ret, ERR_SYSTEM_PERMISSION_DENIED);
}
}
}