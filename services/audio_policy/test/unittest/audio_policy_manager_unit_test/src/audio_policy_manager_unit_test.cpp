/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "audio_errors.h"
#include "audio_policy_manager_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioPolicyManagerUnitTest::SetUpTestCase(void) {}
void AudioPolicyManagerUnitTest::TearDownTestCase(void) {}
void AudioPolicyManagerUnitTest::SetUp(void) {}
void AudioPolicyManagerUnitTest::TearDown(void) {}

/**
* @tc.name  : Test AudioPolicyManager.
* @tc.number: AudioPolicyManagerUnitTest_001.
* @tc.desc  : Test GetOutputDevice.
*/
HWTEST(AudioPolicyManager, AudioPolicyManagerUnitTest_001, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    audioPolicyManager_->GetOutputDevice(audioRendererFilter);
    EXPECT_NE(audioPolicyManager_,  nullptr);
}

/**
* @tc.name  : Test AudioPolicyManager.
* @tc.number: AudioPolicyManagerUnitTest_002.
* @tc.desc  : Test GetInputDevice.
*/
HWTEST(AudioPolicyManager, AudioPolicyManagerUnitTest_002, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    sptr<AudioCapturerFilter> audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();
    audioPolicyManager_->GetInputDevice(audioCapturerFilter);
    EXPECT_NE(audioPolicyManager_,  nullptr);
}

/**
* @tc.name  : Test AudioPolicyManager.
* @tc.number: AudioPolicyManagerUnitTest_003.
* @tc.desc  : Test SetRingerModeCallback.
*/
HWTEST(AudioPolicyManager, AudioPolicyManagerUnitTest_003, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    int32_t clientId = getpid();
    std::shared_ptr<AudioRingerModeCallback> callback = std::make_shared<ConcreteAudioRingerModeCallback>();
    audioPolicyManager_->SetRingerModeCallback(clientId, callback, API_VERSION::API_9);
    EXPECT_NE(audioPolicyManager_,  nullptr);
}

/**
* @tc.name  : Test AudioPolicyManager.
* @tc.number: AudioPolicyManagerUnitTest_004.
* @tc.desc  : Test SetMicrophoneBlockedCallback.
*/
HWTEST(AudioPolicyManager, AudioPolicyManagerUnitTest_004, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    int32_t clientId = getpid();
    std::shared_ptr<AudioManagerMicrophoneBlockedCallback> callback =
        std::make_shared<ConcreteAudioManagerMicrophoneBlockedCallback>();
    audioPolicyManager_->SetMicrophoneBlockedCallback(clientId, callback);
    EXPECT_NE(audioPolicyManager_,  nullptr);

    audioPolicyManager_->isAudioPolicyClientRegisted_.store(true);
    audioPolicyManager_->SetMicrophoneBlockedCallback(clientId, callback);
    EXPECT_NE(audioPolicyManager_,  nullptr);

    audioPolicyManager_->audioPolicyClientStubCB_ = new(std::nothrow) AudioPolicyClientStubImpl();
    audioPolicyManager_->SetMicrophoneBlockedCallback(clientId, callback);
    EXPECT_NE(audioPolicyManager_,  nullptr);
}

/**
* @tc.name  : Test AudioPolicyManager.
* @tc.number: AudioPolicyManagerUnitTest_005.
* @tc.desc  : Test UnsetPreferredInputDeviceChangeCallback.
*/
HWTEST(AudioPolicyManager, AudioPolicyManagerUnitTest_005, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    audioPolicyManager_->audioPolicyClientStubCB_ = nullptr;
    int32_t result = audioPolicyManager_->UnsetPreferredInputDeviceChangeCallback();
    EXPECT_EQ(result,  SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyManager.
* @tc.number: AudioPolicyManagerUnitTest_006.
* @tc.desc  : Test UnsetMicrophoneBlockedCallback.
*/
HWTEST(AudioPolicyManager, AudioPolicyManagerUnitTest_006, TestSize.Level1)
{
    int32_t clientId = getpid();
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    std::shared_ptr<AudioManagerMicrophoneBlockedCallback> callback =
        std::make_shared<ConcreteAudioManagerMicrophoneBlockedCallback>();
    audioPolicyManager_->audioPolicyClientStubCB_ = nullptr;
    int32_t result = audioPolicyManager_->UnsetMicrophoneBlockedCallback(clientId, callback);
    EXPECT_EQ(result,  SUCCESS);

    audioPolicyManager_->audioPolicyClientStubCB_ = new(std::nothrow) AudioPolicyClientStubImpl();
    result = audioPolicyManager_->UnsetMicrophoneBlockedCallback(clientId, callback);
    EXPECT_EQ(result,  SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyManager.
* @tc.number: AudioPolicyManagerUnitTest_007.
* @tc.desc  : Test UnregisterAudioRendererEventListener.
*/
HWTEST(AudioPolicyManager, AudioPolicyManagerUnitTest_007, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    std::shared_ptr<AudioRendererStateChangeCallback> callback;
    auto audioPolicyClient_ = std::make_shared<AudioPolicyClientStubImpl>();
    audioPolicyClient_->AddRendererStateChangeCallback(callback);
    audioPolicyManager_->audioPolicyClientStubCB_ = new(std::nothrow) AudioPolicyClientStubImpl();
    audioPolicyManager_->isAudioRendererEventListenerRegistered = true;
    audioPolicyManager_->UnregisterAudioRendererEventListener(callback);
    EXPECT_NE(audioPolicyManager_, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManager.
* @tc.number: AudioPolicyManagerUnitTest_008.
* @tc.desc  : Test UnregisterAudioRendererEventListener.
*/
HWTEST(AudioPolicyManager, AudioPolicyManagerUnitTest_008, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    std::shared_ptr<AudioRendererStateChangeCallback> callback =
        std::make_shared<ConcreteAudioRendererStateChange>();
    audioPolicyManager_->audioPolicyClientStubCB_ = nullptr;
    audioPolicyManager_->isAudioRendererEventListenerRegistered = true;
    audioPolicyManager_->UnregisterAudioRendererEventListener(callback);
    EXPECT_NE(audioPolicyManager_, nullptr);

    audioPolicyManager_->audioPolicyClientStubCB_ = new(std::nothrow) AudioPolicyClientStubImpl();
    audioPolicyManager_->isAudioRendererEventListenerRegistered = false;
    audioPolicyManager_->UnregisterAudioRendererEventListener(callback);
    EXPECT_NE(audioPolicyManager_, nullptr);

    auto audioPolicyClient_ = std::make_shared<AudioPolicyClientStubImpl>();
    audioPolicyClient_->AddRendererStateChangeCallback(callback);
    audioPolicyManager_->audioPolicyClientStubCB_ = new(std::nothrow) AudioPolicyClientStubImpl();
    audioPolicyManager_->isAudioRendererEventListenerRegistered = true;
    audioPolicyManager_->UnregisterAudioRendererEventListener(callback);
    EXPECT_NE(audioPolicyManager_, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManager.
* @tc.number: AudioPolicyManagerUnitTest_009.
* @tc.desc  : Test UnregisterAudioCapturerEventListener.
*/
HWTEST(AudioPolicyManager, AudioPolicyManagerUnitTest_009, TestSize.Level1)
{
    int32_t clientId = getpid();
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    auto audioPolicyClient_ = std::make_shared<AudioPolicyClientStubImpl>();
    auto cb = std::make_shared<ConcreteAudioCapturerStateChangeCallback>();
    audioPolicyClient_->AddCapturerStateChangeCallback(cb);
    audioPolicyManager_->audioPolicyClientStubCB_ = new(std::nothrow) AudioPolicyClientStubImpl();
    audioPolicyManager_->isAudioCapturerEventListenerRegistered = false;
    audioPolicyManager_->UnregisterAudioCapturerEventListener(clientId);
    EXPECT_NE(audioPolicyManager_, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManager.
* @tc.number: AudioPolicyManagerUnitTest_010.
* @tc.desc  : Test SetDistributedRoutingRoleCallback.
*/
HWTEST(AudioPolicyManager, AudioPolicyManagerUnitTest_010, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    int result = audioPolicyManager_->SetDistributedRoutingRoleCallback(nullptr);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioPolicyManager.
* @tc.number: AudioPolicyManagerUnitTest_012.
* @tc.desc  : Test GetConverterConfig.
*/
HWTEST(AudioPolicyManager, AudioPolicyManagerUnitTest_012, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    audioPolicyManager_->GetConverterConfig();
    EXPECT_NE(audioPolicyManager_, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManager.
* @tc.number: AudioPolicyManagerUnitTest_013.
* @tc.desc  : Test ActivateAudioSession.
*/
HWTEST(AudioPolicyManager, AudioPolicyManagerUnitTest_013, TestSize.Level1)
{
    AudioSessionStrategy strategy;
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    audioPolicyManager_->ActivateAudioSession(strategy);
    EXPECT_NE(audioPolicyManager_, nullptr);

    audioPolicyManager_->isAudioPolicyClientRegisted_.store(true);
    audioPolicyManager_->ActivateAudioSession(strategy);
    EXPECT_NE(audioPolicyManager_, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManager.
* @tc.number: AudioPolicyManagerUnitTest_014.
* @tc.desc  : Test SetAudioSessionCallback.
*/
HWTEST(AudioPolicyManager, AudioPolicyManagerUnitTest_014, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    auto audioSessionCallback = std::make_shared<ConcreteAudioSessionCallback>();
    audioPolicyManager_->isAudioPolicyClientRegisted_.store(true);
    audioPolicyManager_->audioPolicyClientStubCB_ = nullptr;
    int32_t result = audioPolicyManager_->SetAudioSessionCallback(audioSessionCallback);
    EXPECT_EQ(result, ERROR_ILLEGAL_STATE);

    audioPolicyManager_->audioPolicyClientStubCB_ = new(std::nothrow) AudioPolicyClientStubImpl();
    audioPolicyManager_->isAudioPolicyClientRegisted_.store(false);
    result = audioPolicyManager_->SetAudioSessionCallback(audioSessionCallback);
    EXPECT_NE(audioPolicyManager_, nullptr);

    audioPolicyManager_->isAudioPolicyClientRegisted_.store(true);
    std::shared_ptr<AudioSessionCallback> cb = std::make_shared<ConcreteAudioSessionCallback>();
    audioPolicyManager_->audioPolicyClientStubCB_->audioSessionCallbackList_.push_back(cb);
    result = audioPolicyManager_->SetAudioSessionCallback(audioSessionCallback);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyManager.
* @tc.number: AudioPolicyManagerUnitTest_015.
* @tc.desc  : Test UnsetAudioSessionCallback.
*/
HWTEST(AudioPolicyManager, AudioPolicyManagerUnitTest_015, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    audioPolicyManager_->audioPolicyClientStubCB_ = nullptr;
    int32_t result = audioPolicyManager_->UnsetAudioSessionCallback();
    EXPECT_EQ(result, ERROR_ILLEGAL_STATE);

    audioPolicyManager_->audioPolicyClientStubCB_ = new(std::nothrow) AudioPolicyClientStubImpl();
    result = audioPolicyManager_->UnsetAudioSessionCallback();
    EXPECT_EQ(result, SUCCESS);

    std::shared_ptr<AudioSessionCallback> cb = std::make_shared<ConcreteAudioSessionCallback>();
    audioPolicyManager_->audioPolicyClientStubCB_->audioSessionCallbackList_.push_back(cb);
    result = audioPolicyManager_->UnsetAudioSessionCallback();
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyManager.
* @tc.number: AudioPolicyManagerUnitTest_016.
* @tc.desc  : Test UnsetAudioSessionCallback.
*/
HWTEST(AudioPolicyManager, AudioPolicyManagerUnitTest_016, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    std::shared_ptr<AudioSessionCallback> audioSessionCallback =
        std::make_shared<ConcreteAudioSessionCallback>();
    audioPolicyManager_->audioPolicyClientStubCB_ = nullptr;
    int32_t result = audioPolicyManager_->UnsetAudioSessionCallback(audioSessionCallback);
    EXPECT_EQ(result, ERROR_ILLEGAL_STATE);

    audioPolicyManager_->audioPolicyClientStubCB_ = new(std::nothrow) AudioPolicyClientStubImpl();
    result = audioPolicyManager_->UnsetAudioSessionCallback(audioSessionCallback);
    EXPECT_EQ(result, SUCCESS);

    std::shared_ptr<AudioSessionCallback> cb = std::make_shared<ConcreteAudioSessionCallback>();
    audioPolicyManager_->audioPolicyClientStubCB_->audioSessionCallbackList_.push_back(cb);
    result = audioPolicyManager_->UnsetAudioSessionCallback(audioSessionCallback);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyManager.
* @tc.number: AudioPolicyManagerUnitTest_017.
* @tc.desc  : Test RegisterHeadTrackingDataRequestedEventListener.
*/
HWTEST(AudioPolicyManager, AudioPolicyManagerUnitTest_017, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    std::string macAddress = "macAddress";
    auto callback = std::make_shared<ConcreteHeadTrackingDataRequestedChangeCallback>();
    audioPolicyManager_->RegisterHeadTrackingDataRequestedEventListener(macAddress, callback);
    EXPECT_NE(audioPolicyManager_, nullptr);

    audioPolicyManager_->isAudioPolicyClientRegisted_ = true;
    audioPolicyManager_->RegisterHeadTrackingDataRequestedEventListener(macAddress, callback);
    EXPECT_NE(audioPolicyManager_, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManager.
* @tc.number: AudioPolicyManagerUnitTest_018.
* @tc.desc  : Test UnregisterHeadTrackingDataRequestedEventListener.
*/
HWTEST(AudioPolicyManager, AudioPolicyManagerUnitTest_018, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    std::string macAddress = "macAddress";
    audioPolicyManager_->audioPolicyClientStubCB_ = nullptr;
    audioPolicyManager_->UnregisterHeadTrackingDataRequestedEventListener(macAddress);
    EXPECT_NE(audioPolicyManager_, nullptr);

    audioPolicyManager_->audioPolicyClientStubCB_ = new(std::nothrow) AudioPolicyClientStubImpl();
    audioPolicyManager_->UnregisterHeadTrackingDataRequestedEventListener(macAddress);
    EXPECT_NE(audioPolicyManager_, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManager.
* @tc.number: AudioPolicyManagerUnitTest_019.
* @tc.desc  : Test SetAudioDeviceRefinerCallback.
*/
HWTEST(AudioPolicyManager, AudioPolicyManagerUnitTest_019, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    int32_t result = audioPolicyManager_->SetAudioDeviceRefinerCallback(nullptr);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    std::shared_ptr<AudioDeviceRefiner> callback = std::make_shared<ConcreteAudioDeviceRefiner>();
    audioPolicyManager_->SetAudioDeviceRefinerCallback(callback);
    EXPECT_NE(audioPolicyManager_, nullptr);
}

/**
* @tc.name  : Test AudioPolicyManager.
* @tc.number: AudioPolicyManagerUnitTest_020.
* @tc.desc  : Test SetAudioDeviceRefinerCallback.
*/
HWTEST(AudioPolicyManager, AudioPolicyManagerUnitTest_020, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    int32_t result = audioPolicyManager_->SetAudioDeviceAnahsCallback(nullptr);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    std::shared_ptr<AudioDeviceAnahs> callback = std::make_shared<ConcreteAudioDeviceAnahs>();
    audioPolicyManager_->SetAudioDeviceAnahsCallback(nullptr);
    EXPECT_NE(audioPolicyManager_, nullptr);
}

/**
* @tc.name  : Test GetOutputDevice.
* @tc.number: GetOutputDevice.
* @tc.desc  : Test GetOutputDevice.
*/
HWTEST(AudioPolicyManager, GetOutputDevice_001, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();

    sptr<AudioRendererFilter> audioRendererFilter = new AudioRendererFilter();
    audioPolicyManager_->GetOutputDevice(audioRendererFilter);
    EXPECT_NE(audioPolicyManager_, nullptr);
}

/**
* @tc.name  : Test GetInputDevice.
* @tc.number: GetInputDevice.
* @tc.desc  : Test GetInputDevice.
*/
HWTEST(AudioPolicyManager, GetInputDevice_001, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();

    sptr<AudioCapturerFilter> audioRendererFilter = new AudioCapturerFilter();
    audioPolicyManager_->GetInputDevice(audioRendererFilter);
    EXPECT_NE(audioPolicyManager_, nullptr);
}

/**
* @tc.name  : Test SetMicrophoneBlockedCallback.
* @tc.number: SetMicrophoneBlockedCallback.
* @tc.desc  : Test SetMicrophoneBlockedCallback.
*/
HWTEST(AudioPolicyManager, SetMicrophoneBlockedCallback_001, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();

    const int32_t clientId = 111;
    std::shared_ptr<AudioManagerMicrophoneBlockedCallback> callback_ =
        std::make_shared<ConcreteAudioManagerMicrophoneBlockedCallback>();
    const std::shared_ptr<AudioManagerMicrophoneBlockedCallback> callback = callback_;
    int32_t ret = audioPolicyManager_->SetMicrophoneBlockedCallback(clientId, callback);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test UnsetMicrophoneBlockedCallback.
* @tc.number: UnsetMicrophoneBlockedCallback.
* @tc.desc  : Test UnsetMicrophoneBlockedCallback.
*/
HWTEST(AudioPolicyManager, UnsetMicrophoneBlockedCallback_001, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();

    const int32_t clientId = 111;
    std::shared_ptr<AudioManagerMicrophoneBlockedCallback> callback_ =
        std::make_shared<ConcreteAudioManagerMicrophoneBlockedCallback>();
    const std::shared_ptr<AudioManagerMicrophoneBlockedCallback> callback = callback_;
    int32_t ret = audioPolicyManager_->UnsetMicrophoneBlockedCallback(clientId, callback);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test ActivateAudioSession.
* @tc.number: ActivateAudioSession.
* @tc.desc  : Test ActivateAudioSession.
*/
HWTEST(AudioPolicyManager, ActivateAudioSession_001, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();

    const AudioSessionStrategy strategy;
    int32_t ret = audioPolicyManager_->ActivateAudioSession(strategy);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test SetAudioSessionCallback.
* @tc.number: SetAudioSessionCallback.
* @tc.desc  : Test SetAudioSessionCallback.
*/
HWTEST(AudioPolicyManager, SetAudioSessionCallback_001, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();

    const std::shared_ptr<AudioSessionCallback> audioSessionCallback =
        std::make_shared<ConcreteAudioSessionCallback>();
    int32_t ret = audioPolicyManager_->SetAudioSessionCallback(audioSessionCallback);
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test SetAudioSessionCallback.
* @tc.number: SetAudioSessionCallback.
* @tc.desc  : Test SetAudioSessionCallback.
*/
HWTEST(AudioPolicyManager, SetAudioSessionCallback_002, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();

    const std::shared_ptr<AudioSessionCallback> audioSessionCallback =
        std::make_shared<ConcreteAudioSessionCallback>();
    audioPolicyManager_->isAudioPolicyClientRegisted_ = true;
    int32_t ret = audioPolicyManager_->SetAudioSessionCallback(audioSessionCallback);
    EXPECT_EQ(ret, ERROR_ILLEGAL_STATE);
}

/**
* @tc.name  : Test SetAudioSessionCallback.
* @tc.number: SetAudioSessionCallback.
* @tc.desc  : Test SetAudioSessionCallback.
*/
HWTEST(AudioPolicyManager, SetAudioSessionCallback_003, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();

    const std::shared_ptr<AudioSessionCallback> audioSessionCallback =
        std::make_shared<ConcreteAudioSessionCallback>();
    audioPolicyManager_->isAudioPolicyClientRegisted_ = true;
    audioPolicyManager_->audioPolicyClientStubCB_ = new AudioPolicyClientStubImpl();
    int32_t ret = audioPolicyManager_->SetAudioSessionCallback(audioSessionCallback);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test UnsetAudioSessionCallback.
* @tc.number: UnsetAudioSessionCallback.
* @tc.desc  : Test UnsetAudioSessionCallback.
*/
HWTEST(AudioPolicyManager, UnsetAudioSessionCallback_001, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();

    int32_t ret = audioPolicyManager_->UnsetAudioSessionCallback();
    EXPECT_EQ(ret, ERROR_ILLEGAL_STATE);
}

/**
* @tc.name  : Test UnsetAudioSessionCallback.
* @tc.number: UnsetAudioSessionCallback.
* @tc.desc  : Test UnsetAudioSessionCallback.
*/
HWTEST(AudioPolicyManager, UnsetAudioSessionCallback_002, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();

    audioPolicyManager_->audioPolicyClientStubCB_ = new AudioPolicyClientStubImpl();
    int32_t ret = audioPolicyManager_->UnsetAudioSessionCallback();
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test UnsetAudioSessionCallback.
* @tc.number: UnsetAudioSessionCallback.
* @tc.desc  : Test UnsetAudioSessionCallback.
*/
HWTEST(AudioPolicyManager, UnsetAudioSessionCallback_ptr_001, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    const std::shared_ptr<AudioSessionCallback> audioSessionCallback =
        std::make_shared<ConcreteAudioSessionCallback>();

    int32_t ret = audioPolicyManager_->UnsetAudioSessionCallback(audioSessionCallback);
    EXPECT_EQ(ret, ERROR_ILLEGAL_STATE);
}

/**
* @tc.name  : Test UnsetAudioSessionCallback.
* @tc.number: UnsetAudioSessionCallback.
* @tc.desc  : Test UnsetAudioSessionCallback.
*/
HWTEST(AudioPolicyManager, UnsetAudioSessionCallback_ptr_002, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    const std::shared_ptr<AudioSessionCallback> audioSessionCallback =
        std::make_shared<ConcreteAudioSessionCallback>();

    audioPolicyManager_->audioPolicyClientStubCB_ = new AudioPolicyClientStubImpl();
    int32_t ret = audioPolicyManager_->UnsetAudioSessionCallback(audioSessionCallback);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyManager.
* @tc.number: SetAudioSceneChangeCallbackTest_001.
* @tc.desc  : Test SetAudioSceneChangeCallback.
*/
HWTEST(AudioPolicyManager, SetAudioSceneChangeCallbackTest_001, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    int32_t clientId = getpid();
    std::shared_ptr<AudioManagerAudioSceneChangedCallback> callback =
        std::make_shared<ConcreteAudioManagerAudioSceneChangedCallback>();
    audioPolicyManager_->SetAudioSceneChangeCallback(clientId, callback);
    EXPECT_NE(audioPolicyManager_,  nullptr);
}

/**
* @tc.name  : Test AudioPolicyManager.
* @tc.number: UnsetAudioSceneChangeCallbackTest_001.
* @tc.desc  : Test UnsetAudioSceneChangeCallback.
*/
HWTEST(AudioPolicyManager, UnsetAudioSceneChangeCallbackTest_001, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    std::shared_ptr<AudioManagerAudioSceneChangedCallback> callback =
        std::make_shared<ConcreteAudioManagerAudioSceneChangedCallback>();
    audioPolicyManager_->audioPolicyClientStubCB_ = nullptr;
    int32_t result = audioPolicyManager_->UnsetAudioSceneChangeCallback(callback);
    EXPECT_EQ(result,  SUCCESS);

    audioPolicyManager_->audioPolicyClientStubCB_ = new(std::nothrow) AudioPolicyClientStubImpl();
    result = audioPolicyManager_->UnsetAudioSceneChangeCallback(callback);
    EXPECT_EQ(result,  SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS
