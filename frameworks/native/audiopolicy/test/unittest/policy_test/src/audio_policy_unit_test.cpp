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
#define LOG_TAG "AudioPolicyUnitTest"
#endif

#include <thread>
#include "audio_errors.h"
#include "audio_info.h"
#include "parcel.h"
#include "audio_policy_client.h"
#include "audio_policy_unit_test.h"
#include "audio_system_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "audio_client_tracker_callback_stub.h"
#include "audio_policy_client_stub_impl.h"
#include "audio_adapter_manager.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
static const uint32_t LOW_LATENCY_FROM_XML = 20;
static const uint32_t HIGH_LATENCY_FROM_XML = 200;
static const uint32_t MIN_VOLUME_LEVEL = 0;
static const uint32_t MAX_VOLUME_LEVEL = 15;
void AudioPolicyUnitTest::SetUpTestCase(void) {}
void AudioPolicyUnitTest::TearDownTestCase(void) {}
void AudioPolicyUnitTest::SetUp(void) {}
void AudioPolicyUnitTest::TearDown(void) {}

void AudioPolicyUnitTest::GetIRemoteObject(sptr<IRemoteObject> &object)
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        AUDIO_ERR_LOG("GetIRemoteObject::GetSystemAbilityManager failed");
        return;
    }

    object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    if (object == nullptr) {
        AUDIO_ERR_LOG("GetIRemoteObject::object is NULL.");
        return;
    }
}

/**
 * @tc.name  : Test Audio_Policy_SetMicrophoneMuteAudioConfig_001 via illegal state
 * @tc.number: Audio_Policy_SetMicrophoneMuteAudioConfig_001
 * @tc.desc  : Test SetMicrophoneMuteAudioConfig interface. Returns success.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_SetMicrophoneMuteAudioConfig_001, TestSize.Level1)
{
    bool isMute = true;
    int32_t ret = AudioPolicyManager::GetInstance().SetMicrophoneMuteAudioConfig(isMute);
    EXPECT_EQ(SUCCESS, ret);
}

#ifdef FEATURE_DTMF_TONE
/**
 * @tc.name  : Test Audio_Policy_GetSupportedTones_001 via legal state
 * @tc.number: Audio_Policy_GetSupportedTones_001
 * @tc.desc  : Test GetSupportedTones interface. Returns success.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_GetSupportedTones_001, TestSize.Level1)
{
    std::vector<int32_t> res = AudioPolicyManager::GetInstance().GetSupportedTones("");
    EXPECT_NE(0, res.size());

    res = AudioPolicyManager::GetInstance().GetSupportedTones("cn");
    EXPECT_NE(0, res.size());
}

/**
 * @tc.name  : Test Audio_Policy_GetToneConfig_001 via legal state
 * @tc.number: Audio_Policy_GetToneConfig_001
 * @tc.desc  : Test GetToneConfig interface. Returns success.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_GetToneConfig_001, TestSize.Level1)
{
    int32_t ltonetype = 0;
    std::shared_ptr<ToneInfo> toneInfo = AudioPolicyManager::GetInstance().GetToneConfig(ltonetype, "");
    EXPECT_NE(nullptr, toneInfo);

    toneInfo = AudioPolicyManager::GetInstance().GetToneConfig(ltonetype, "cn");
    EXPECT_NE(nullptr, toneInfo);

    ltonetype = INT32_MAX;
    toneInfo = AudioPolicyManager::GetInstance().GetToneConfig(ltonetype, "");
    EXPECT_NE(nullptr, toneInfo);
}
#endif

/**
 * @tc.name  : Test Audio_Policy_IsStreamActive_001 via legal state
 * @tc.number: Audio_Policy_IsStreamActive_001
 * @tc.desc  : Test IsStreamActive interface. Returns success.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_IsStreamActive_001, TestSize.Level1)
{
    AudioStreamType streamType = AudioStreamType::STREAM_MUSIC;
    bool isStreamActive = AudioPolicyManager::GetInstance().IsStreamActive(streamType);
    EXPECT_EQ(false, isStreamActive);
}

/**
 * @tc.name  : Test Audio_Policy_SelectInputDevice_001 via illegal state
 * @tc.number: Audio_Policy_SelectInputDevice_001
 * @tc.desc  : Test SelectInputDevice interface. Returns success.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_SelectInputDevice_001, TestSize.Level1)
{
    int32_t ret;
    AudioSystemManager *audioSystemMgr = AudioSystemManager::GetInstance();
    DeviceFlag deviceFlag = DeviceFlag::INPUT_DEVICES_FLAG;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorsVector;
    audioDeviceDescriptorsVector = audioSystemMgr->GetDevices(deviceFlag);

    sptr<AudioCapturerFilter> audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();
    audioCapturerFilter->uid = DeviceFlag::INPUT_DEVICES_FLAG;

    ret = AudioPolicyManager::GetInstance().SelectInputDevice(audioCapturerFilter, audioDeviceDescriptorsVector);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Audio_Policy_GetStreamInFocus_001 via legal state
 * @tc.number: Audio_Policy_GetStreamInFocus_001
 * @tc.desc  : Test GetStreamInFocus interface. Returns success.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_GetStreamInFocus_001, TestSize.Level1)
{
    AudioPolicyManager::GetInstance().GetStreamInFocus();
}

/**
 * @tc.name  : Test Audio_Policy_Manager_IsStreamActive_001 via illegal state
 * @tc.number: Audio_Policy_Manager_IsStreamActive_001
 * @tc.desc  : Test RegisterAudioCapturerEventListener interface. Returns success.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_IsStreamActive_001, TestSize.Level1)
{
    bool isStreamActive = AudioPolicyManager::GetInstance().IsStreamActive(AudioStreamType::STREAM_MUSIC);
    EXPECT_EQ(false, isStreamActive);
}

/**
 * @tc.name  : Test Audio_Policy_Manager_SetMicrophoneMuteAudioConfig_001 via legal state
 * @tc.number: Audio_Policy_Manager_SetMicrophoneMuteAudioConfig_001
 * @tc.desc  : Test SetMicrophoneMuteAudioConfig interface. Returns success.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_SetMicrophoneMuteAudioConfig_001, TestSize.Level1)
{
    bool isMute = true;
    bool ret = AudioPolicyManager::GetInstance().SetMicrophoneMuteAudioConfig(isMute);
    EXPECT_EQ(SUCCESS, ret);
}

#ifdef FEATURE_DTMF_TONE
/**
 * @tc.name  : Test Audio_Policy_Manager_GetSupportedTones_001 via legal state
 * @tc.number: Audio_Policy_Manager_GetSupportedTones_001
 * @tc.desc  : Test GetSupportedTones interface. Returns success.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_GetSupportedTones_001, TestSize.Level1)
{
    std::vector<int32_t> res = AudioPolicyManager::GetInstance().GetSupportedTones("");
    EXPECT_NE(0, res.size());

    res = AudioPolicyManager::GetInstance().GetSupportedTones("cn");
    EXPECT_NE(0, res.size());
}

/**
 * @tc.name  : Test Audio_Policy_Manager_GetToneConfig_001 via legal state
 * @tc.number: Audio_Policy_Manager_GetToneConfig_001
 * @tc.desc  : Test GetToneConfig interface. Returns success.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_GetToneConfig_001, TestSize.Level1)
{
    int32_t ltonetype = 0;
    std::shared_ptr<ToneInfo> toneInfo = AudioPolicyManager::GetInstance().GetToneConfig(ltonetype, "");
    EXPECT_NE(nullptr, toneInfo);

    toneInfo = AudioPolicyManager::GetInstance().GetToneConfig(ltonetype, "cn");
    EXPECT_NE(nullptr, toneInfo);

    ltonetype = INT32_MAX;
    toneInfo = AudioPolicyManager::GetInstance().GetToneConfig(ltonetype, "");
    EXPECT_NE(nullptr, toneInfo);
}
#endif

/**
 * @tc.name  : Test Audio_Policy_Manager_SetDeviceChangeCallback_001 via legal state
 * @tc.number: Audio_Policy_Manager_SetDeviceChangeCallback_001
 * @tc.desc  : Test SetDeviceChangeCallback interface. Returns success.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_SetDeviceChangeCallback_001, TestSize.Level1)
{
    int32_t clientId = getpid();
    DeviceFlag flag = DeviceFlag::OUTPUT_DEVICES_FLAG;
    std::shared_ptr<AudioManagerDeviceChangeCallback> callback =
        std::make_shared<AudioManagerDeviceChangeCallbackTest>();

    int32_t ret = AudioPolicyManager::GetInstance().SetDeviceChangeCallback(clientId, flag, callback);
    EXPECT_EQ(SUCCESS, ret);

    ret = AudioPolicyManager::GetInstance().UnsetDeviceChangeCallback(clientId, flag, callback);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Audio_Policy_Manager_GetStreamInFocus_001 via legal state
 * @tc.number: Audio_Policy_Manager_GetStreamInFocus_001
 * @tc.desc  : Test GetStreamInFocus interface. Returns success.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_GetStreamInFocus_001, TestSize.Level1)
{
    AudioPolicyManager::GetInstance().GetStreamInFocus();
}

/**
 * @tc.name  : Test Audio_Policy_Manager_RegisterAudioRendererEventListener_001 via legal state
 * @tc.number: Audio_Policy_Manager_RegisterAudioRendererEventListener_001
 * @tc.desc  : Test registerAudioRendererEventListener interface. Returns success.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_RegisterAudioRendererEventListener_001, TestSize.Level1)
{
    std::shared_ptr<AudioRendererStateChangeCallback> callback =
        std::make_shared<AudioRendererStateChangeCallbackTest>();
    int32_t ret = AudioPolicyManager::GetInstance().RegisterAudioRendererEventListener(callback);
    EXPECT_EQ(SUCCESS, ret);

    ret = AudioPolicyManager::GetInstance().UnregisterAudioRendererEventListener(callback);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Audio_Policy_Manager_RegisterAudioCapturerEventListener_001 via legal state
 * @tc.number: Audio_Policy_Manager_RegisterAudioCapturerEventListener_001
 * @tc.desc  : Test RegisterAudioCapturerEventListener interface. Returns success.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_RegisterAudioCapturerEventListener_001, TestSize.Level1)
{
    int32_t clientId = getpid();
    std::shared_ptr<AudioCapturerStateChangeCallback> callback =
        std::make_shared<AudioCapturerStateChangeCallbackTest>();
    int32_t ret = AudioPolicyManager::GetInstance().RegisterAudioCapturerEventListener(clientId, callback);
    EXPECT_EQ(SUCCESS, ret);

    ret = AudioPolicyManager::GetInstance().UnregisterAudioCapturerEventListener(clientId);
    EXPECT_EQ(SUCCESS, ret);

    callback = nullptr;
    ret = AudioPolicyManager::GetInstance().RegisterAudioCapturerEventListener(clientId, callback);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
 * @tc.name  : Test RegisterAudioCapturerEventListener
 * @tc.number: Audio_Policy_Manager_RegisterAudioCapturerEventListener_002
 * @tc.desc  : Test RegisterAudioCapturerEventListener exception branch.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_RegisterAudioCapturerEventListener_002, TestSize.Level1)
{
    int32_t clientId = getpid();
    std::shared_ptr<AudioCapturerStateChangeCallback> callback =
        std::make_shared<AudioCapturerStateChangeCallbackTest>();

    AudioPolicyManager::GetInstance().isAudioPolicyClientRegisted_ = false;
    int32_t ret = AudioPolicyManager::GetInstance().RegisterAudioCapturerEventListener(clientId, callback);
    EXPECT_EQ(SUCCESS, ret);

    ret = AudioPolicyManager::GetInstance().UnregisterAudioCapturerEventListener(clientId);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Audio_Policy_GetPreferredOutputDeviceDescriptors_001 via illegal state
 * @tc.number: Audio_Policy_GetPreferredOutputDeviceDescriptors_001
 * @tc.desc  : Test GetPreferredOutputDeviceDescriptors interface. Get preferred output devices and returns deviceInfo.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_GetPreferredOutputDeviceDescriptors_001, TestSize.Level1)
{
    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = STREAM_USAGE_INVALID;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceInfo;
    deviceInfo = AudioPolicyManager::GetInstance().GetPreferredOutputDeviceDescriptors(rendererInfo);
    EXPECT_GT(deviceInfo.size(), 0);
}

/**
 * @tc.name  : Test Audio_Policy_GetPreferredOutputDeviceDescriptors_002 via illegal state
 * @tc.number: Audio_Policy_GetPreferredOutputDeviceDescriptors_002
 * @tc.desc  : Test GetPreferredOutputDeviceDescriptors interface. Get preferred output devices and returns deviceInfo.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_GetPreferredOutputDeviceDescriptors_002, TestSize.Level1)
{
    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = static_cast<StreamUsage>(-1000);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceInfo;
    deviceInfo = AudioPolicyManager::GetInstance().GetPreferredOutputDeviceDescriptors(rendererInfo);
    EXPECT_GT(deviceInfo.size(), 0);
}

/**
 * @tc.name  : Test Audio_Policy_GetPreferredOutputDeviceDescriptors_003 via illegal state
 * @tc.number: Audio_Policy_GetPreferredOutputDeviceDescriptors_003
 * @tc.desc  : Test GetPreferredOutputDeviceDescriptors interface. Get preferred output devices and returns deviceInfo.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_GetPreferredOutputDeviceDescriptors_003, TestSize.Level1)
{
    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = static_cast<StreamUsage>(1000);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceInfo;
    deviceInfo = AudioPolicyManager::GetInstance().GetPreferredOutputDeviceDescriptors(rendererInfo);
    EXPECT_GT(deviceInfo.size(), 0);
}

/**
 * @tc.name  : Test Audio_Policy_GetPreferredOutputDeviceDescriptors_004 via legal state
 * @tc.number: Audio_Policy_GetPreferredOutputDeviceDescriptors_004
 * @tc.desc  : Test GetPreferredOutputDeviceDescriptors interface. Get preferred output devices and returns deviceInfo.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_GetPreferredOutputDeviceDescriptors_004, TestSize.Level1)
{
    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceInfo;
    deviceInfo = AudioPolicyManager::GetInstance().GetPreferredOutputDeviceDescriptors(rendererInfo);
    EXPECT_GT(deviceInfo.size(), 0);
}

/**
 * @tc.name  : Test Audio_Policy_GetPreferredOutputDeviceDescriptors_005 via legal state
 * @tc.number: Audio_Policy_GetPreferredOutputDeviceDescriptors_005
 * @tc.desc  : Test GetPreferredOutputDeviceDescriptors interface. Get preferred output devices and returns deviceInfo.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_GetPreferredOutputDeviceDescriptors_005, TestSize.Level1)
{
    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceInfo;
    deviceInfo = AudioPolicyManager::GetInstance().GetPreferredOutputDeviceDescriptors(rendererInfo, true);
    EXPECT_GT(deviceInfo.size(), 0);
}

/**
 * @tc.name  : Test Audio_Policy_GetPreferredInputDeviceDescriptors_001 via illegal state
 * @tc.number: Audio_Policy_GetPreferredInputDeviceDescriptors_001
 * @tc.desc  : Test GetPreferredInputDeviceDescriptors interface. Get preferred input devices and returns deviceInfo.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_GetPreferredInputDeviceDescriptors_001, TestSize.Level1)
{
    AudioCapturerInfo capturerInfo;
    capturerInfo.sourceType = SOURCE_TYPE_INVALID;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceInfo;
    deviceInfo = AudioPolicyManager::GetInstance().GetPreferredInputDeviceDescriptors(capturerInfo);
    EXPECT_GT(deviceInfo.size(), 0);
}

/**
 * @tc.name  : Test Audio_Policy_GetPreferredInputDeviceDescriptors_002 via illegal state
 * @tc.number: Audio_Policy_GetPreferredInputDeviceDescriptors_002
 * @tc.desc  : Test GetPreferredInputDeviceDescriptors interface. Get preferred input devices and returns deviceInfo.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_GetPreferredInputDeviceDescriptors_002, TestSize.Level1)
{
    AudioCapturerInfo capturerInfo;
    capturerInfo.sourceType = static_cast<SourceType>(-1000);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceInfo;
    deviceInfo = AudioPolicyManager::GetInstance().GetPreferredInputDeviceDescriptors(capturerInfo);
    EXPECT_GT(deviceInfo.size(), 0);
}

/**
 * @tc.name  : Test Audio_Policy_GetPreferredInputDeviceDescriptors_003 via illegal state
 * @tc.number: Audio_Policy_GetPreferredInputDeviceDescriptors_003
 * @tc.desc  : Test GetPreferredInputDeviceDescriptors interface. Get preferred input devices and returns deviceInfo.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_GetPreferredInputDeviceDescriptors_003, TestSize.Level1)
{
    AudioCapturerInfo capturerInfo;
    capturerInfo.sourceType = static_cast<SourceType>(1000);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceInfo;
    deviceInfo = AudioPolicyManager::GetInstance().GetPreferredInputDeviceDescriptors(capturerInfo);
    EXPECT_GT(deviceInfo.size(), 0);
}

/**
 * @tc.name  : Test Audio_Policy_GetPreferredInputDeviceDescriptors_004 via legal state
 * @tc.number: Audio_Policy_GetPreferredInputDeviceDescriptors_004
 * @tc.desc  : Test GetPreferredInputDeviceDescriptors interface. Get preferred input devices and returns deviceInfo.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_GetPreferredInputDeviceDescriptors_004, TestSize.Level1)
{
    AudioCapturerInfo capturerInfo;
    capturerInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceInfo;
    deviceInfo = AudioPolicyManager::GetInstance().GetPreferredInputDeviceDescriptors(capturerInfo);
    EXPECT_GT(deviceInfo.size(), 0);
}

/**
 * @tc.name  : Test Audio_Policy_SetAudioManagerInterruptCallback_001 via illegal state
 * @tc.number: Audio_Policy_SetAudioManagerInterruptCallback_001
 * @tc.desc  : Test SetAudioManagerInterruptCallback interface. Returns invalid.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_SetAudioManagerInterruptCallback_001, TestSize.Level1)
{
    int32_t clientId = getpid();
    std::shared_ptr<AudioInterruptCallback> callback = nullptr;
    int32_t ret = AudioPolicyManager::GetInstance().SetAudioManagerInterruptCallback(clientId, callback);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
 * @tc.name  : Test Audio_Policy_RegisterTracker_001 via illegal state
 * @tc.number: Audio_Policy_RegisterTracker_001
 * @tc.desc  : Test RegisterTracker interface. Returns invalid.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_RegisterTracker_001, TestSize.Level1)
{
    AudioMode mode = AudioMode::AUDIO_MODE_PLAYBACK;
    AudioStreamChangeInfo streamChangeInfo;
    std::shared_ptr<AudioClientTracker> clientTrackerObj = nullptr;
    int32_t ret = AudioPolicyManager::GetInstance().RegisterTracker(mode, streamChangeInfo, clientTrackerObj);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Audio_Policy_Manager_SetRingerModeCallback_001 via illegal state
 * @tc.number: Audio_Policy_Manager_SetRingerModeCallback_001
 * @tc.desc  : Test SetRingerModeCallback interface. Returns invalid.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_SetRingerModeCallback_001, TestSize.Level1)
{
    int32_t clientId = getpid();
    std::shared_ptr<AudioRingerModeCallback> callback = nullptr;
    int32_t ret = AudioPolicyManager::GetInstance().SetRingerModeCallback(clientId, callback);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);

    ret = AudioPolicyManager::GetInstance().UnsetRingerModeCallback(clientId);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetRingerModeCallback
 * @tc.number: Audio_Policy_Manager_SetRingerModeCallback_002
 * @tc.desc  : Test SetRingerModeCallback interface abnormal branch.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_SetRingerModeCallback_002, TestSize.Level3)
{
    int32_t clientId = getpid();
    int32_t ret = -1;
    std::shared_ptr<AudioRingerModeCallback> callback = make_shared<AudioRingerModeCallbackTest>();
    AudioPolicyManager::GetInstance().isAudioPolicyClientRegisted_ = false;

    ret = AudioPolicyManager::GetInstance().SetRingerModeCallback(clientId, callback);
    EXPECT_EQ(SUCCESS, ret);

    ret = AudioPolicyManager::GetInstance().UnsetRingerModeCallback(clientId, callback);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Audio_Policy_Manager_SetDeviceChangeCallback_002 via illegal state
 * @tc.number: Audio_Policy_Manager_SetDeviceChangeCallback_002
 * @tc.desc  : Test SetDeviceChangeCallback interface. Returns invalid.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_SetDeviceChangeCallback_002, TestSize.Level1)
{
    int32_t clientId = getpid();
    DeviceFlag flag = DeviceFlag::INPUT_DEVICES_FLAG;
    std::shared_ptr<AudioManagerDeviceChangeCallback> callback = nullptr;
    int32_t ret = AudioPolicyManager::GetInstance().SetDeviceChangeCallback(clientId, flag, callback);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
 * @tc.name  : Test Audio_Policy_Manager_SetDistribuitedOutputChangeCallback_001 via illegal state
 * @tc.number: Audio_Policy_Manager_SetDistribuitedOutputChangeCallback_001
 * @tc.desc  : Test SetDistribuitedOutputChangeCallback interface. Returns invalid.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_SetDistribuitedOutputChangeCallback_001, TestSize.Level1)
{
    auto cb = make_shared<AudioDistribuitedOutputChangeCallbackTest>();
    int32_t ret = AudioPolicyManager::GetInstance().SetDistribuitedOutputChangeCallback(cb);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Audio_Policy_Manager_SetAudioManagerInterruptCallback_001 via illegal state
 * @tc.number: Audio_Policy_Manager_SetAudioManagerInterruptCallback_001
 * @tc.desc  : Test SetAudioManagerInterruptCallback interface. Returns invalid.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_SetAudioManagerInterruptCallback_001, TestSize.Level1)
{
    int32_t clientId = getpid();
    std::shared_ptr<AudioInterruptCallback> callback = nullptr;
    int32_t ret = AudioPolicyManager::GetInstance().SetAudioManagerInterruptCallback(clientId, callback);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
 * @tc.name  : Test Audio_Policy_Manager_SetVolumeKeyEventCallback_001 via illegal state
 * @tc.number: Audio_Policy_Manager_SetVolumeKeyEventCallback_001
 * @tc.desc  : Test SetVolumeKeyEventCallback interface. Returns invalid.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_SetVolumeKeyEventCallback_001, TestSize.Level1)
{
    int32_t clientPid = getpid();
    std::shared_ptr<VolumeKeyEventCallback> callback = nullptr;
    int32_t ret = AudioPolicyManager::GetInstance().SetVolumeKeyEventCallback(clientPid, callback);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
* @tc.name  : Test Audio_Policy_Manager_RegisterAudioRendererEventListener_002 via illegal state
* @tc.number: Audio_Policy_Manager_RegisterAudioRendererEventListener_002
* @tc.desc  : Test RegisterAudioRendererEventListener interface. Returns invalid.
*/
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_RegisterAudioRendererEventListener_002, TestSize.Level1)
{
    std::shared_ptr<AudioRendererStateChangeCallback> callback = nullptr;
    int32_t ret = AudioPolicyManager::GetInstance().RegisterAudioRendererEventListener(callback);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
 * @tc.name  : Test RegisterAudioRendererEventListener
 * @tc.number: Audio_Policy_Manager_RegisterAudioRendererEventListener_003
 * @tc.desc  : Test registerAudioRendererEventListener exception branch.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_RegisterAudioRendererEventListener_003, TestSize.Level1)
{
    std::shared_ptr<AudioRendererStateChangeCallback> callback =
        std::make_shared<AudioRendererStateChangeCallbackTest>();
    AudioPolicyManager::GetInstance().isAudioPolicyClientRegisted_ = false;
    int32_t ret = AudioPolicyManager::GetInstance().RegisterAudioRendererEventListener(callback);
    EXPECT_EQ(SUCCESS, ret);

    ret = AudioPolicyManager::GetInstance().UnregisterAudioRendererEventListener(callback);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test UnregisterAudioRendererEventListener
 * @tc.number: Audio_Policy_Manager_UnregisterAudioRendererEventListener_001
 * @tc.desc  : Test UnregisterAudioRendererEventListener interface.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_UnregisterAudioRendererEventListener_001, TestSize.Level1)
{
    std::shared_ptr<AudioRendererStateChangeCallback> callback1 =
        std::make_shared<AudioRendererStateChangeCallbackTest>();
    std::shared_ptr<AudioRendererStateChangeCallback> callback2 =
        std::make_shared<AudioRendererStateChangeCallbackTest>();

    int32_t ret = AudioPolicyManager::GetInstance().RegisterAudioRendererEventListener(callback1);
    EXPECT_EQ(SUCCESS, ret);

    ret = AudioPolicyManager::GetInstance().RegisterAudioRendererEventListener(callback2);
    EXPECT_EQ(SUCCESS, ret);

    std::vector<std::shared_ptr<AudioRendererStateChangeCallback>> callbacks;
    callbacks.push_back(callback1);
    callbacks.push_back(callback2);

    ret = AudioPolicyManager::GetInstance().UnregisterAudioRendererEventListener(callbacks);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Audio_Policy_Manager_GetCurrentCapturerChangeInfos_001 via illegal state
 * @tc.number: Audio_Policy_Manager_GetCurrentCapturerChangeInfos_001
 * @tc.desc  : Test GetCurrentCapturerChangeInfos interface. Returns invalid.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_GetCurrentCapturerChangeInfos_001, TestSize.Level1)
{
    vector<shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    int32_t ret = AudioPolicyManager::GetInstance().GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    EXPECT_EQ(true, audioCapturerChangeInfos.size() <= 0);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test Audio_Capturer_State_Change_001 via legal state
 * @tc.number: Audio_Capturer_State_Change_001
 * @tc.desc  : Test AudioCapturerStateChangeListenerStub interface. Returns invalid.
 */
HWTEST(AudioPolicyUnitTest, Audio_Capturer_State_Change_001, TestSize.Level1)
{
    std::shared_ptr<AudioPolicyClientStubImpl> capturerStub =
        std::make_shared<AudioPolicyClientStubImpl>();

    vector<shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    capturerStub->OnCapturerStateChange(audioCapturerChangeInfos);

    std::shared_ptr<AudioCapturerStateChangeCallbackTest> callback =
        std::make_shared<AudioCapturerStateChangeCallbackTest>();
    capturerStub->AddCapturerStateChangeCallback(callback);

    uint32_t code = static_cast<uint32_t>(AudioPolicyClientCode::ON_CAPTURERSTATE_CHANGE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int ret = capturerStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_LE(ret, 0);
}

/**
 * @tc.name  : Test Audio_Renderer_State_Change_001 via legal state
 * @tc.number: Audio_Renderer_State_Change_001
 * @tc.desc  : Test AudioRendererStateChangeListenerStub interface. Returns success.
 */
HWTEST(AudioPolicyUnitTest, Audio_Renderer_State_Change_001, TestSize.Level1)
{
    std::shared_ptr<AudioPolicyClientStubImpl> rendererStub =
        std::make_shared<AudioPolicyClientStubImpl>();

    vector<shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    rendererStub->OnRendererStateChange(audioRendererChangeInfos);

    std::shared_ptr<AudioRendererStateChangeCallbackTest> callback =
        std::make_shared<AudioRendererStateChangeCallbackTest>();
    rendererStub->AddRendererStateChangeCallback(callback);

    uint32_t code = static_cast<uint32_t>(AudioPolicyClientCode::ON_RENDERERSTATE_CHANGE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int ret = rendererStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_LE(ret, 0);
}

/**
 * @tc.name  : Test Audio_Ringermode_Update_Listener_001 via legal state
 * @tc.number: Audio_Ringermode_Update_Listener_001
 * @tc.desc  : Test AudioRingerModeUpdateListenerStub interface. Returns success.
 */
HWTEST(AudioPolicyUnitTest, Audio_Ringermode_Update_Listener_001, TestSize.Level1)
{
    std::shared_ptr<AudioPolicyClientStubImpl> ringermodeStub =
        std::make_shared<AudioPolicyClientStubImpl>();
    std::shared_ptr<AudioRingerModeCallbackTest> callback = std::make_shared<AudioRingerModeCallbackTest>();
    AudioRingerMode ringerMode = AudioRingerMode::RINGER_MODE_SILENT;

    ringermodeStub->OnRingerModeUpdated(ringerMode);

    ringermodeStub->AddRingerModeCallback(callback);

    ringermodeStub->OnRingerModeUpdated(ringerMode);

    uint32_t code = static_cast<uint32_t>(AudioPolicyClientCode::ON_RINGERMODE_UPDATE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int ret = ringermodeStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_LE(ret, 0);
}

/**
 * @tc.name  : Test Audio_Rounting_Manager_Listener_001 via legal state
 * @tc.number: Audio_Rounting_Manager_Listener_001
 * @tc.desc  : Test AudioRoutingManagerListenerStub interface. Returns success.
 */
HWTEST(AudioPolicyUnitTest, Audio_Rounting_Manager_Listener_001, TestSize.Level1)
{
    std::shared_ptr<AudioPolicyClientStubImpl> audioPolicyClientStubImpl =
        std::make_shared<AudioPolicyClientStubImpl>();
    std::shared_ptr<AudioManagerMicStateChangeCallbackTest> callback =
        std::make_shared<AudioManagerMicStateChangeCallbackTest>();
    MicStateChangeEvent micStateChangeEvent;
    micStateChangeEvent.mute = true;

    audioPolicyClientStubImpl->OnMicStateUpdated(micStateChangeEvent);

    audioPolicyClientStubImpl->AddMicStateChangeCallback(callback);

    audioPolicyClientStubImpl->OnMicStateUpdated(micStateChangeEvent);

    uint32_t code = static_cast<uint32_t>(AudioPolicyClientCode::ON_MIC_STATE_UPDATED);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int ret = audioPolicyClientStubImpl->OnRemoteRequest(code, data, reply, option);
    EXPECT_LE(ret, 0);
}

/**
 * @tc.name  : Test Audio_Volume_Group_Info_001 via legal state
 * @tc.number: Audio_Volume_Group_Info_001
 * @tc.desc  : Test VolumeGroupInfo interface. Returns success.
 */
HWTEST(AudioPolicyUnitTest, Audio_Volume_Group_Info_001, TestSize.Level1)
{
    int32_t volumeGroupId = 1;
    int32_t mappingId = 1;
    std::string groupName = "TEST_UNIT";
    std::string networkId = "UNIT";
    ConnectType type = ConnectType::CONNECT_TYPE_LOCAL;

    std::shared_ptr<VolumeGroupInfo> volumeGroupInfo =
        std::make_shared<VolumeGroupInfo>(volumeGroupId, mappingId, groupName, networkId, type);

    Parcel parcel;
    bool ret = volumeGroupInfo->Marshalling(parcel);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name  : Test Audio_Client_Tracker_Callback_Stub_001 via legal state
 * @tc.number: Audio_Client_Tracker_Callback_Stub_001
 * @tc.desc  : Test AudioClientTrackerCallbackStub interface. Returns success.
 */
HWTEST(AudioPolicyUnitTest, Audio_Client_Tracker_Callback_Stub_001, TestSize.Level1)
{
    std::shared_ptr<AudioClientTrackerCallbackStub> audioClientTrackerCallbackStub =
        std::make_shared<AudioClientTrackerCallbackStub>();

    StreamSetStateEventInternal streamSetStateEventInternal = {};
    streamSetStateEventInternal.streamUsage = STREAM_USAGE_MUSIC;
    std::weak_ptr<AudioClientTrackerTest> callback = std::make_shared<AudioClientTrackerTest>();

    float volume = 0.5;
    audioClientTrackerCallbackStub->SetLowPowerVolumeImpl(volume);
    audioClientTrackerCallbackStub->GetLowPowerVolumeImpl(volume);
    audioClientTrackerCallbackStub->GetSingleStreamVolumeImpl(volume);

    streamSetStateEventInternal.streamSetState= StreamSetState::STREAM_RESUME;
    audioClientTrackerCallbackStub->ResumeStreamImpl(streamSetStateEventInternal);

    streamSetStateEventInternal.streamSetState= StreamSetState::STREAM_PAUSE;
    audioClientTrackerCallbackStub->PausedStreamImpl(streamSetStateEventInternal);

    audioClientTrackerCallbackStub->SetClientTrackerCallback(callback);

    streamSetStateEventInternal.streamSetState= StreamSetState::STREAM_RESUME;
    audioClientTrackerCallbackStub->ResumeStreamImpl(streamSetStateEventInternal);

    streamSetStateEventInternal.streamSetState= StreamSetState::STREAM_PAUSE;
    audioClientTrackerCallbackStub->PausedStreamImpl(streamSetStateEventInternal);
}

/**
 * @tc.name  : Test Audio_Policy_SetSystemSoundUri_001
 * @tc.number: Audio_Policy_SetSystemSoundUri_001
 * @tc.desc  : Test audio policy instance
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_SetSystemSoundUri_001, TestSize.Level1)
{
    int32_t ret = -1;
    AudioRingerMode ringMode = AudioRingerMode::RINGER_MODE_SILENT;
    ret = AudioPolicyManager::GetInstance().SetRingerMode(ringMode);
    EXPECT_EQ(SUCCESS, ret);

    AudioRingerMode ringModeRet = AudioPolicyManager::GetInstance().GetRingerMode();
    EXPECT_EQ(ringMode, ringModeRet);

    const std::string key = "testkey";
    const std::string uri = "testuri";
    ret = AudioPolicyManager::GetInstance().SetSystemSoundUri(key, uri);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);

    std::string systemSoundUri = AudioPolicyManager::GetInstance().GetSystemSoundUri(key);
    EXPECT_EQ(systemSoundUri, "");
}

/**
 * @tc.name  : Test Audio_Policy_Manager_HighResolutionExist_001
 * @tc.number: Audio_Policy_Manager_HighResolutionExist_001
 * @tc.desc  : Test high resolution exist status.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_HighResolutionExist_001, TestSize.Level1)
{
    int32_t ret = AudioPolicyManager::GetInstance().SetHighResolutionExist(true);
    EXPECT_EQ(SUCCESS, ret);
    bool isHighResExist = AudioPolicyManager::GetInstance().IsHighResolutionExist();
    EXPECT_EQ(true, isHighResExist);
}

/**
 * @tc.name  : Test Audio_Policy_Manager_HighResolutionExist_002
 * @tc.number: Audio_Policy_Manager_HighResolutionExist_002
 * @tc.desc  : Test high resolution exist status.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_HighResolutionExist_002, TestSize.Level1)
{
    int32_t ret = AudioPolicyManager::GetInstance().SetHighResolutionExist(false);
    EXPECT_EQ(SUCCESS, ret);
    bool isHighResExist = AudioPolicyManager::GetInstance().IsHighResolutionExist();
    EXPECT_EQ(false, isHighResExist);
}

/**
 * @tc.name  : Test Audio_Policy_Manager_IsAbsVolumeScene_001
 * @tc.number: Audio_Policy_Manager_IsAbsVolumeScene_001
 * @tc.desc  : Test IsAbsVolumeScene interface.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_IsAbsVolumeScene_001, TestSize.Level1)
{
    bool isEnable = true;
    AudioAdapterManager::GetInstance().SetAbsVolumeScene(isEnable);
    int32_t ret = AudioAdapterManager::GetInstance().IsAbsVolumeScene();
    EXPECT_EQ(true, ret);

    isEnable = false;
    AudioAdapterManager::GetInstance().SetAbsVolumeScene(isEnable);
    ret = AudioAdapterManager::GetInstance().IsAbsVolumeScene();
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name  : Test Audio_Policy_Manager_IsSpatializationEnabled_001
 * @tc.number: Audio_Policy_Manager_IsSpatializationEnabled_001
 * @tc.desc  : Test IsSpatializationEnabled interface.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_IsSpatializationEnabled_001, TestSize.Level1)
{
    bool isEnable = true;
    int32_t ret = AudioPolicyManager::GetInstance().SetSpatializationEnabled(isEnable);
    EXPECT_EQ(SUCCESS, ret);
    isEnable = AudioPolicyManager::GetInstance().IsSpatializationEnabled();
    EXPECT_EQ(true, isEnable);

    ret = AudioPolicyManager::GetInstance().SetSpatializationEnabled(isEnable);
    EXPECT_EQ(SUCCESS, ret);
    isEnable = AudioPolicyManager::GetInstance().IsSpatializationEnabled();
    EXPECT_EQ(true, isEnable);

    isEnable = false;
    ret = AudioPolicyManager::GetInstance().SetSpatializationEnabled(isEnable);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioPolicyManager::GetInstance().IsSpatializationEnabled();
    EXPECT_EQ(false, ret);

    ret = AudioPolicyManager::GetInstance().SetSpatializationEnabled(isEnable);
    EXPECT_EQ(SUCCESS, ret);
    isEnable = AudioPolicyManager::GetInstance().IsSpatializationEnabled();
    EXPECT_EQ(false, isEnable);
}

/**
 * @tc.name  : Test Audio_Policy_Manager_IsHeadTrackingEnabled_001
 * @tc.number: Audio_Policy_Manager_IsHeadTrackingEnabled_001
 * @tc.desc  : Test IsHeadTrackingEnabled interface.
 */
HWTEST(AudioPolicyUnitTest, Audio_Policy_Manager_IsHeadTrackingEnabled_001, TestSize.Level1)
{
    bool isEnable = true;
    int32_t ret = AudioPolicyManager::GetInstance().SetHeadTrackingEnabled(isEnable);
    EXPECT_EQ(SUCCESS, ret);
    isEnable = AudioPolicyManager::GetInstance().IsHeadTrackingEnabled();
    EXPECT_EQ(true, isEnable);
    ret = AudioPolicyManager::GetInstance().SetHeadTrackingEnabled(isEnable);
    EXPECT_EQ(SUCCESS, ret);
    isEnable = AudioPolicyManager::GetInstance().IsHeadTrackingEnabled();
    EXPECT_EQ(true, isEnable);

    isEnable = false;
    ret = AudioPolicyManager::GetInstance().SetHeadTrackingEnabled(isEnable);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioPolicyManager::GetInstance().IsHeadTrackingEnabled();
    EXPECT_EQ(false, ret);
    ret = AudioPolicyManager::GetInstance().SetHeadTrackingEnabled(isEnable);
    EXPECT_EQ(SUCCESS, ret);
    isEnable = AudioPolicyManager::GetInstance().IsHeadTrackingEnabled();
    EXPECT_EQ(false, isEnable);
}

/**
 * @tc.name  : Test DisableSafeMediaVolume
 * @tc.number: DisableSafeMediaVolume_001
 * @tc.desc  : Test DisableSafeMediaVolume interface. Returns success.
 */
HWTEST(AudioPolicyUnitTest, DisableSafeMediaVolume_001, TestSize.Level1)
{
    int32_t ret = AudioPolicyManager::GetInstance().DisableSafeMediaVolume();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test GetMaxVolumeLevel
 * @tc.number: GetMaxVolumeLevel_001
 * @tc.desc  : Test GetMaxVolumeLevel interface. Returns MAX_VOLUME_LEVEL.
 */
HWTEST(AudioPolicyUnitTest, GetMaxVolumeLevel_001, TestSize.Level1)
{
    AudioVolumeType volumeType = STREAM_MUSIC;
    int32_t ret = AudioPolicyManager::GetInstance().GetMaxVolumeLevel(volumeType);
    EXPECT_TRUE(ret == MAX_VOLUME_LEVEL);
}

/**
 * @tc.name  : Test GetMaxVolumeLevel
 * @tc.number: GetMaxVolumeLevel_002
 * @tc.desc  : Test GetMaxVolumeLevel interface. Returns MAX_VOLUME_LEVEL.
 */
HWTEST(AudioPolicyUnitTest, GetMaxVolumeLevel_002, TestSize.Level1)
{
    AudioVolumeType volumeType = STREAM_ALL;
    int32_t ret = AudioPolicyManager::GetInstance().GetMaxVolumeLevel(volumeType);
    EXPECT_TRUE(ret == MAX_VOLUME_LEVEL);
}

/**
 * @tc.name  : Test GetMaxVolumeLevel
 * @tc.number: GetMaxVolumeLevel_003
 * @tc.desc  : Test GetMaxVolumeLevel interface. Returns ERR_INVALID_PARAM.
 */
HWTEST(AudioPolicyUnitTest, GetMaxVolumeLevel_003, TestSize.Level1)
{
    AudioVolumeType volumeType = static_cast<AudioVolumeType>(60);
    int32_t ret = AudioPolicyManager::GetInstance().GetMaxVolumeLevel(volumeType);
    EXPECT_TRUE(ret == ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test GetMinVolumeLevel
 * @tc.number: GetMinVolumeLevel_001
 * @tc.desc  : Test GetMinVolumeLevel interface. Returns MIN_VOLUME_LEVEL.
 */
HWTEST(AudioPolicyUnitTest, GetMinVolumeLevel_001, TestSize.Level1)
{
    AudioVolumeType volumeType = STREAM_MUSIC;
    int32_t ret = AudioPolicyManager::GetInstance().GetMinVolumeLevel(volumeType);
    EXPECT_TRUE(ret == MIN_VOLUME_LEVEL);
}

/**
 * @tc.name  : Test GetMinVolumeLevel
 * @tc.number: GetMinVolumeLevel_002
 * @tc.desc  : Test GetMinVolumeLevel interface. Returns MIN_VOLUME_LEVEL.
 */
HWTEST(AudioPolicyUnitTest, GetMinVolumeLevel_002, TestSize.Level1)
{
    AudioVolumeType volumeType = STREAM_ALL;
    int32_t ret = AudioPolicyManager::GetInstance().GetMinVolumeLevel(volumeType);
    EXPECT_TRUE(ret == MIN_VOLUME_LEVEL);
}

/**
 * @tc.name  : Test GetMinVolumeLevel
 * @tc.number: GetMinVolumeLevel_003
 * @tc.desc  : Test GetMinVolumeLevel interface. Returns ERR_INVALID_PARAM.
 */
HWTEST(AudioPolicyUnitTest, GetMinVolumeLevel_003, TestSize.Level1)
{
    AudioVolumeType volumeType = static_cast<AudioVolumeType>(60);
    int32_t ret = AudioPolicyManager::GetInstance().GetMinVolumeLevel(volumeType);
    EXPECT_TRUE(ret == ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetSystemVolumeLevel
 * @tc.number: SetSystemVolumeLevel_001
 * @tc.desc  : Test SetSystemVolumeLevel interface. Returns SUCCESS.
 */
HWTEST(AudioPolicyUnitTest, SetSystemVolumeLevel_001, TestSize.Level1)
{
    AudioVolumeType volumeType = STREAM_MUSIC;
    int32_t volumeLevel = 2;
    int32_t ret = AudioPolicyManager::GetInstance().SetSystemVolumeLevel(volumeType, volumeLevel);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetSystemVolumeLevel
 * @tc.number: SetSystemVolumeLevel_002
 * @tc.desc  : Test SetSystemVolumeLevel interface. Returns SUCCESS.
 */
HWTEST(AudioPolicyUnitTest, SetSystemVolumeLevel_002, TestSize.Level1)
{
    AudioVolumeType volumeType = STREAM_MUSIC;
    int32_t volumeLevel = 2;
    int32_t ret = AudioPolicyManager::GetInstance().SetSystemVolumeLevel(volumeType, volumeLevel, API_7);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioPolicyManager::GetInstance().SetSystemVolumeLevel(volumeType, volumeLevel, API_8);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioPolicyManager::GetInstance().SetSystemVolumeLevel(volumeType, volumeLevel, API_9);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioPolicyManager::GetInstance().SetSystemVolumeLevel(volumeType, volumeLevel, API_10);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioPolicyManager::GetInstance().SetSystemVolumeLevel(volumeType, volumeLevel, API_11);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetSystemVolumeLevel
 * @tc.number: SetSystemVolumeLevel_003
 * @tc.desc  : Test SetSystemVolumeLevel interface. Returns SUCCESS.
 */
HWTEST(AudioPolicyUnitTest, SetSystemVolumeLevel_003, TestSize.Level1)
{
    AudioVolumeType volumeType = STREAM_RING;
    int32_t volumeLevel = 2;
    int32_t ret = AudioPolicyManager::GetInstance().SetSystemVolumeLevel(volumeType, volumeLevel, API_9);
    EXPECT_EQ(SUCCESS, ret);
}
/**
 * @tc.name  : Test SetSystemVolumeLevel
 * @tc.number: SetSystemVolumeLevel_004
 * @tc.desc  : Test SetSystemVolumeLevel interface. Returns ERR_NOT_SUPPORTED.
 */
HWTEST(AudioPolicyUnitTest, SetSystemVolumeLevel_004, TestSize.Level1)
{
    AudioVolumeType volumeType = STREAM_NOTIFICATION;
    int32_t volumeLevel = 2;
    int32_t ret = AudioPolicyManager::GetInstance().SetSystemVolumeLevel(volumeType, volumeLevel, API_9);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetSystemVolumeLevel
 * @tc.number: SetSystemVolumeLevel_005
 * @tc.desc  : Test SetSystemVolumeLevel interface. Returns SUCCESS.
 */
HWTEST(AudioPolicyUnitTest, SetSystemVolumeLevel_005, TestSize.Level1)
{
    AudioVolumeType volumeType = STREAM_VOICE_CALL;
    int32_t volumeLevel = 3;
    int32_t ret = AudioPolicyManager::GetInstance().SetSystemVolumeLevel(volumeType, volumeLevel, API_9);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetSystemVolumeLevel
 * @tc.number: SetSystemVolumeLevel_006
 * @tc.desc  : Test SetSystemVolumeLevel interface. Returns ERR_NOT_SUPPORTED.
 */
HWTEST(AudioPolicyUnitTest, SetSystemVolumeLevel_006, TestSize.Level1)
{
    AudioVolumeType volumeType = STREAM_VOICE_COMMUNICATION;
    int32_t volumeLevel = 3;
    int32_t ret = AudioPolicyManager::GetInstance().SetSystemVolumeLevel(volumeType, volumeLevel, API_9);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetSystemVolumeLevel
 * @tc.number: SetSystemVolumeLevel_007
 * @tc.desc  : Test SetSystemVolumeLevel interface. Returns SUCCESS.
 */
HWTEST(AudioPolicyUnitTest, SetSystemVolumeLevel_007, TestSize.Level1)
{
    AudioVolumeType volumeType = STREAM_VOICE_ASSISTANT;
    int32_t volumeLevel = 3;
    int32_t ret = AudioPolicyManager::GetInstance().SetSystemVolumeLevel(volumeType, volumeLevel, API_9);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetSystemVolumeLevel
 * @tc.number: SetSystemVolumeLevel_008
 * @tc.desc  : Test SetSystemVolumeLevel interface. Returns SUCCESS.
 */
HWTEST(AudioPolicyUnitTest, SetSystemVolumeLevel_008, TestSize.Level1)
{
    AudioVolumeType volumeType = STREAM_ALARM;
    int32_t volumeLevel = 4;
    int32_t ret = AudioPolicyManager::GetInstance().SetSystemVolumeLevel(volumeType, volumeLevel, API_9);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetSystemVolumeLevel
 * @tc.number: SetSystemVolumeLevel_009
 * @tc.desc  : Test SetSystemVolumeLevel interface. Returns SUCCESS.
 */
HWTEST(AudioPolicyUnitTest, SetSystemVolumeLevel_009, TestSize.Level1)
{
    AudioVolumeType volumeType = STREAM_ULTRASONIC;
    int32_t volumeLevel = 4;
    int32_t ret = AudioPolicyManager::GetInstance().SetSystemVolumeLevel(volumeType, volumeLevel, API_9);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetSystemVolumeLevel
 * @tc.number: SetSystemVolumeLevel_010
 * @tc.desc  : Test SetSystemVolumeLevel interface. Returns SUCCESS.
 */
HWTEST(AudioPolicyUnitTest, SetSystemVolumeLevel_010, TestSize.Level1)
{
    AudioVolumeType volumeType = STREAM_ALL;
    int32_t volumeLevel = 5;
    int32_t ret = AudioPolicyManager::GetInstance().SetSystemVolumeLevel(volumeType, volumeLevel, API_9);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetSystemVolumeLevel
 * @tc.number: SetSystemVolumeLevel_011
 * @tc.desc  : Test SetSystemVolumeLevel interface. Returns ERR_NOT_SUPPORTED.
 */
HWTEST(AudioPolicyUnitTest, SetSystemVolumeLevel_011, TestSize.Level1)
{
    AudioVolumeType volumeType = STREAM_GAME;
    int32_t volumeLevel = 5;
    int32_t ret = AudioPolicyManager::GetInstance().SetSystemVolumeLevel(volumeType, volumeLevel, API_9);
    EXPECT_EQ(ERR_NOT_SUPPORTED, ret);
}

/**
 * @tc.name  : Test SetSystemVolumeLevel
 * @tc.number: SetSystemVolumeLevel_012
 * @tc.desc  : Test SetSystemVolumeLevel interface. Returns ERR_NOT_SUPPORTED.
 */
HWTEST(AudioPolicyUnitTest, SetSystemVolumeLevel_012, TestSize.Level1)
{
    AudioVolumeType volumeType = STREAM_MUSIC;
    int32_t volumeLevel = -1;
    int32_t ret = AudioPolicyManager::GetInstance().SetSystemVolumeLevel(volumeType, volumeLevel, API_9);
    EXPECT_EQ(ERR_NOT_SUPPORTED, ret);
}

/**
 * @tc.name  : Test SetSystemVolumeLevel
 * @tc.number: SetSystemVolumeLevel_013
 * @tc.desc  : Test SetSystemVolumeLevel interface. Returns ERR_NOT_SUPPORTED.
 */
HWTEST(AudioPolicyUnitTest, SetSystemVolumeLevel_013, TestSize.Level1)
{
    AudioVolumeType volumeType = STREAM_MUSIC;
    int32_t volumeLevel = 25;
    int32_t ret = AudioPolicyManager::GetInstance().SetSystemVolumeLevel(volumeType, volumeLevel, API_9);
    EXPECT_EQ(ERR_NOT_SUPPORTED, ret);
}

/**
 * @tc.name  : Test SetSystemVolumeLevel
 * @tc.number: SetSystemVolumeLevel_014
 * @tc.desc  : Test SetSystemVolumeLevel interface. Returns SUCCESS.
 */
HWTEST(AudioPolicyUnitTest, SetSystemVolumeLevel_014, TestSize.Level1)
{
    AudioVolumeType volumeType = STREAM_ALL;
    int32_t volumeLevel = 2;
    int32_t volumeFlag = 1;
    int32_t ret = AudioPolicyManager::GetInstance().SetSystemVolumeLevel(volumeType, volumeLevel, API_9, volumeFlag);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test GetSystemVolumeLevel
 * @tc.number: GetSystemVolumeLevel_001
 * @tc.desc  : Test GetSystemVolumeLevel interface. Returns volumeLevel.
 */
HWTEST(AudioPolicyUnitTest, GetSystemVolumeLevel_001, TestSize.Level1)
{
    int32_t ret;
    int32_t volumeLevel = 2;
    ret = AudioPolicyManager::GetInstance().SetSystemVolumeLevel(AudioVolumeType::STREAM_VOICE_CALL, volumeLevel);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioPolicyManager::GetInstance().GetSystemVolumeLevel(AudioVolumeType::STREAM_VOICE_CALL);
    EXPECT_EQ(volumeLevel, ret);
    ret = AudioPolicyManager::GetInstance().GetSystemVolumeLevel(AudioVolumeType::STREAM_VOICE_COMMUNICATION);
    EXPECT_EQ(volumeLevel, ret);
}

/**
 * @tc.name  : Test GetSystemVolumeLevel
 * @tc.number: GetSystemVolumeLevel_002
 * @tc.desc  : Test GetSystemVolumeLevel interface. Returns volumeLevel.
 */
HWTEST(AudioPolicyUnitTest, GetSystemVolumeLevel_002, TestSize.Level1)
{
    int32_t ret;
    int32_t volumeLevel = 3;
    ret = AudioPolicyManager::GetInstance().SetSystemVolumeLevel(AudioVolumeType::STREAM_RING, volumeLevel);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioPolicyManager::GetInstance().GetSystemVolumeLevel(AudioVolumeType::STREAM_RING);
    EXPECT_EQ(volumeLevel, ret);
    ret = AudioPolicyManager::GetInstance().GetSystemVolumeLevel(AudioVolumeType::STREAM_SYSTEM);
    EXPECT_EQ(volumeLevel, ret);
    ret = AudioPolicyManager::GetInstance().GetSystemVolumeLevel(AudioVolumeType::STREAM_NOTIFICATION);
    EXPECT_EQ(volumeLevel, ret);
    ret = AudioPolicyManager::GetInstance().GetSystemVolumeLevel(AudioVolumeType::STREAM_DTMF);
    EXPECT_EQ(volumeLevel, ret);
}

/**
 * @tc.name  : Test GetSystemVolumeLevel
 * @tc.number: GetSystemVolumeLevel_003
 * @tc.desc  : Test GetSystemVolumeLevel interface. Returns volumeLevel.
 */
HWTEST(AudioPolicyUnitTest, GetSystemVolumeLevel_003, TestSize.Level1)
{
    int32_t ret;
    int32_t volumeLevel = 4;
    ret = AudioPolicyManager::GetInstance().SetSystemVolumeLevel(AudioVolumeType::STREAM_MUSIC, volumeLevel);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioPolicyManager::GetInstance().GetSystemVolumeLevel(AudioVolumeType::STREAM_MUSIC);
    EXPECT_EQ(volumeLevel, ret);
    ret = AudioPolicyManager::GetInstance().GetSystemVolumeLevel(AudioVolumeType::STREAM_MEDIA);
    EXPECT_EQ(volumeLevel, ret);
    ret = AudioPolicyManager::GetInstance().GetSystemVolumeLevel(AudioVolumeType::STREAM_MOVIE);
    EXPECT_EQ(volumeLevel, ret);
    ret = AudioPolicyManager::GetInstance().GetSystemVolumeLevel(AudioVolumeType::STREAM_GAME);
    EXPECT_EQ(volumeLevel, ret);
    ret = AudioPolicyManager::GetInstance().GetSystemVolumeLevel(AudioVolumeType::STREAM_SPEECH);
    EXPECT_EQ(volumeLevel, ret);
    ret = AudioPolicyManager::GetInstance().GetSystemVolumeLevel(AudioVolumeType::STREAM_NAVIGATION);
    EXPECT_EQ(volumeLevel, ret);
    ret = AudioPolicyManager::GetInstance().GetSystemVolumeLevel(AudioVolumeType::STREAM_VOICE_MESSAGE);
    EXPECT_EQ(volumeLevel, ret);
    ret = AudioPolicyManager::GetInstance().GetSystemVolumeLevel(static_cast<AudioVolumeType>(99));
    EXPECT_EQ(volumeLevel, ret);
}

/**
 * @tc.name  : Test GetSystemVolumeLevel
 * @tc.number: GetSystemVolumeLevel_004
 * @tc.desc  : Test GetSystemVolumeLevel interface. Returns volumeLevel.
 */
HWTEST(AudioPolicyUnitTest, GetSystemVolumeLevel_004, TestSize.Level1)
{
    int32_t ret;
    int32_t volumeLevel = 5;
    ret = AudioPolicyManager::GetInstance().SetSystemVolumeLevel(AudioVolumeType::STREAM_VOICE_ASSISTANT, volumeLevel);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioPolicyManager::GetInstance().GetSystemVolumeLevel(AudioVolumeType::STREAM_VOICE_ASSISTANT);
    EXPECT_EQ(volumeLevel, ret);
}

/**
 * @tc.name  : Test GetSystemVolumeLevel
 * @tc.number: GetSystemVolumeLevel_005
 * @tc.desc  : Test GetSystemVolumeLevel interface. Returns volumeLevel.
 */
HWTEST(AudioPolicyUnitTest, GetSystemVolumeLevel_005, TestSize.Level1)
{
    int32_t ret;
    int32_t volumeLevel = 6;
    ret = AudioPolicyManager::GetInstance().SetSystemVolumeLevel(AudioVolumeType::STREAM_ALARM, volumeLevel);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioPolicyManager::GetInstance().GetSystemVolumeLevel(AudioVolumeType::STREAM_ALARM);
    EXPECT_EQ(volumeLevel, ret);
}

/**
 * @tc.name  : Test GetSystemVolumeLevel
 * @tc.number: GetSystemVolumeLevel_006
 * @tc.desc  : Test GetSystemVolumeLevel interface. Returns volumeLevel.
 */
HWTEST(AudioPolicyUnitTest, GetSystemVolumeLevel_006, TestSize.Level1)
{
    int32_t ret;
    int32_t volumeLevel = 8;
    ret = AudioPolicyManager::GetInstance().SetSystemVolumeLevel(AudioVolumeType::STREAM_ULTRASONIC, volumeLevel);
    ret = AudioPolicyManager::GetInstance().GetSystemVolumeLevel(AudioVolumeType::STREAM_ULTRASONIC);
    EXPECT_EQ(volumeLevel, ret);
}

/**
 * @tc.name  : Test GetStreamMute
 * @tc.number: GetStreamMute_001
 * @tc.desc  : Test GetStreamMute interface. Returns isMute.
 */
HWTEST(AudioPolicyUnitTest, GetStreamMute_001, TestSize.Level1)
{
    int32_t ret;
    bool isMute;
    ret = AudioPolicyManager::GetInstance().SetStreamMute(AudioVolumeType::STREAM_VOICE_CALL, true);
    EXPECT_EQ(SUCCESS, ret);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_VOICE_CALL);
    EXPECT_FALSE(isMute);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_VOICE_COMMUNICATION);
    EXPECT_FALSE(isMute);
    isMute = AudioPolicyManager::GetInstance().IsStreamActive(AudioVolumeType::STREAM_VOICE_CALL);
    EXPECT_FALSE(isMute);
    ret = AudioPolicyManager::GetInstance().SetStreamMute(AudioVolumeType::STREAM_VOICE_CALL, false);
    EXPECT_EQ(SUCCESS, ret);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_VOICE_CALL);
    EXPECT_FALSE(isMute);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_VOICE_COMMUNICATION);
    EXPECT_FALSE(isMute);
}

/**
 * @tc.name  : Test GetStreamMute
 * @tc.number: GetStreamMute_002
 * @tc.desc  : Test GetStreamMute interface. Returns isMute.
 */
HWTEST(AudioPolicyUnitTest, GetStreamMute_002, TestSize.Level1)
{
    int32_t ret;
    bool isMute;
    ret = AudioPolicyManager::GetInstance().SetStreamMute(AudioVolumeType::STREAM_RING, true);
    EXPECT_EQ(SUCCESS, ret);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_RING);
    EXPECT_TRUE(isMute);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_SYSTEM);
    EXPECT_TRUE(isMute);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_NOTIFICATION);
    EXPECT_TRUE(isMute);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_DTMF);
    EXPECT_TRUE(isMute);
    isMute = AudioPolicyManager::GetInstance().IsStreamActive(AudioVolumeType::STREAM_RING);
    EXPECT_FALSE(isMute);
    ret = AudioPolicyManager::GetInstance().SetStreamMute(AudioVolumeType::STREAM_RING, false);
    EXPECT_EQ(SUCCESS, ret);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_RING);
    EXPECT_FALSE(isMute);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_SYSTEM);
    EXPECT_FALSE(isMute);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_NOTIFICATION);
    EXPECT_FALSE(isMute);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_DTMF);
    EXPECT_FALSE(isMute);
}

/**
 * @tc.name  : Test GetStreamMute
 * @tc.number: GetStreamMute_003
 * @tc.desc  : Test GetStreamMute interface. Returns isMute.
 */
HWTEST(AudioPolicyUnitTest, GetStreamMute_003, TestSize.Level1)
{
    int32_t ret;
    bool isMute;
    ret = AudioPolicyManager::GetInstance().SetStreamMute(AudioVolumeType::STREAM_MUSIC, true);
    EXPECT_EQ(SUCCESS, ret);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_MUSIC);
    EXPECT_TRUE(isMute);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_MEDIA);
    EXPECT_TRUE(isMute);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_MOVIE);
    EXPECT_TRUE(isMute);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_GAME);
    EXPECT_TRUE(isMute);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_SPEECH);
    EXPECT_TRUE(isMute);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_NAVIGATION);
    EXPECT_TRUE(isMute);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_VOICE_MESSAGE);
    EXPECT_TRUE(isMute);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(static_cast<AudioVolumeType>(99));
    EXPECT_TRUE(isMute);
    isMute = AudioPolicyManager::GetInstance().IsStreamActive(AudioVolumeType::STREAM_MUSIC);
    EXPECT_FALSE(isMute);
    ret = AudioPolicyManager::GetInstance().SetStreamMute(AudioVolumeType::STREAM_MUSIC, false);
    EXPECT_EQ(SUCCESS, ret);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_MUSIC);
    EXPECT_FALSE(isMute);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_MEDIA);
    EXPECT_FALSE(isMute);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_MOVIE);
    EXPECT_FALSE(isMute);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_GAME);
    EXPECT_FALSE(isMute);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_SPEECH);
    EXPECT_FALSE(isMute);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_NAVIGATION);
    EXPECT_FALSE(isMute);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_VOICE_MESSAGE);
    EXPECT_FALSE(isMute);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(static_cast<AudioVolumeType>(99));
    EXPECT_FALSE(isMute);
}

/**
 * @tc.name  : Test GetStreamMute
 * @tc.number: GetStreamMute_004
 * @tc.desc  : Test GetStreamMute interface. Returns isMute.
 */
HWTEST(AudioPolicyUnitTest, GetStreamMute_004, TestSize.Level1)
{
    int32_t ret;
    bool isMute;
    ret = AudioPolicyManager::GetInstance().SetStreamMute(AudioVolumeType::STREAM_VOICE_ASSISTANT, true);
    EXPECT_EQ(SUCCESS, ret);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_VOICE_ASSISTANT);
    EXPECT_FALSE(isMute);
    isMute = AudioPolicyManager::GetInstance().IsStreamActive(AudioVolumeType::STREAM_VOICE_ASSISTANT);
    EXPECT_FALSE(isMute);
    ret = AudioPolicyManager::GetInstance().SetStreamMute(AudioVolumeType::STREAM_VOICE_ASSISTANT, false);
    EXPECT_EQ(SUCCESS, ret);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_VOICE_ASSISTANT);
    EXPECT_FALSE(isMute);
}

/**
 * @tc.name  : Test GetStreamMute
 * @tc.number: GetStreamMute_005
 * @tc.desc  : Test GetStreamMute interface. Returns isMute.
 */
HWTEST(AudioPolicyUnitTest, GetStreamMute_005, TestSize.Level1)
{
    int32_t ret;
    bool isMute;
    ret = AudioPolicyManager::GetInstance().SetStreamMute(AudioVolumeType::STREAM_ALARM, true);
    EXPECT_EQ(SUCCESS, ret);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_ALARM);
    EXPECT_FALSE(isMute);
    isMute = AudioPolicyManager::GetInstance().IsStreamActive(AudioVolumeType::STREAM_ALARM);
    EXPECT_FALSE(isMute);
    ret = AudioPolicyManager::GetInstance().SetStreamMute(AudioVolumeType::STREAM_ALARM, false);
    EXPECT_EQ(SUCCESS, ret);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_ALARM);
    EXPECT_FALSE(isMute);
}

/**
 * @tc.name  : Test GetStreamMute
 * @tc.number: GetStreamMute_006
 * @tc.desc  : Test GetStreamMute interface. Returns isMute.
 */
HWTEST(AudioPolicyUnitTest, GetStreamMute_006, TestSize.Level1)
{
    int32_t ret;
    bool isMute;
    ret = AudioPolicyManager::GetInstance().SetStreamMute(AudioVolumeType::STREAM_ULTRASONIC, true);
    EXPECT_EQ(SUCCESS, ret);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_ULTRASONIC);
    EXPECT_TRUE(isMute);
    isMute = AudioPolicyManager::GetInstance().IsStreamActive(AudioVolumeType::STREAM_ULTRASONIC);
    EXPECT_FALSE(isMute);
    ret = AudioPolicyManager::GetInstance().SetStreamMute(AudioVolumeType::STREAM_ULTRASONIC, false);
    EXPECT_EQ(SUCCESS, ret);
    isMute = AudioPolicyManager::GetInstance().GetStreamMute(AudioVolumeType::STREAM_ULTRASONIC);
    EXPECT_FALSE(isMute);
}

/**
 * @tc.name  : Test GetSelectedDeviceInfo
 * @tc.number: GetSelectedDeviceInfo_001
 * @tc.desc  : Test GetSelectedDeviceInfo interface. Returns deviceInfo.
 */
HWTEST(AudioPolicyUnitTest, GetSelectedDeviceInfo_001, TestSize.Level1)
{
    std::string deviceInfo;
    int32_t uid = -1;
    int32_t pid = -1;
    AudioStreamType streamType = AudioVolumeType::STREAM_MUSIC;
    deviceInfo = AudioPolicyManager::GetInstance().GetSelectedDeviceInfo(uid, pid, streamType);
    EXPECT_TRUE(deviceInfo == "");
}

/**
 * @tc.name  : Test GetSelectedDeviceInfo
 * @tc.number: GetSelectedDeviceInfo_002
 * @tc.desc  : Test GetSelectedDeviceInfo interface. Returns deviceInfo.
 */
HWTEST(AudioPolicyUnitTest, GetSelectedDeviceInfo_002, TestSize.Level1)
{
    std::string deviceInfo;
    int32_t uid = static_cast<int32_t>(getuid());;
    int32_t pid = getpid();
    AudioStreamType streamType = AudioVolumeType::STREAM_RING;
    deviceInfo = AudioPolicyManager::GetInstance().GetSelectedDeviceInfo(uid, pid, streamType);
    EXPECT_TRUE(deviceInfo == "");
}

/**
 * @tc.name  : Test GetActiveOutputDevice
 * @tc.number: GetActiveOutputDevice_001
 * @tc.desc  : Test GetActiveOutputDevice interface. Returns deviceType.
 */
HWTEST(AudioPolicyUnitTest, GetActiveOutputDevice_001, TestSize.Level1)
{
    DeviceType deviceType = AudioPolicyManager::GetInstance().GetActiveOutputDevice();
    EXPECT_TRUE(deviceType == DEVICE_TYPE_SPEAKER);
}

/**
 * @tc.name  : Test GetActiveInputDevice
 * @tc.number: GetActiveInputDevice_001
 * @tc.desc  : Test GetActiveInputDevice interface. Returns deviceType.
 */
HWTEST(AudioPolicyUnitTest, GetActiveInputDevice_001, TestSize.Level1)
{
    DeviceType deviceType = AudioPolicyManager::GetInstance().GetActiveInputDevice();
    EXPECT_TRUE(deviceType == DEVICE_TYPE_MIC);
}

/**
 * @tc.name  : Test GetRingerMode
 * @tc.number: GetRingerMode_001
 * @tc.desc  : Test GetRingerMode interface. Returns ringerMode.
 */
HWTEST(AudioPolicyUnitTest, GetRingerMode_001, TestSize.Level1)
{
    int32_t ret = AudioPolicyManager::GetInstance().SetRingerMode(AudioRingerMode::RINGER_MODE_SILENT);
    EXPECT_EQ(SUCCESS, ret);
    AudioRingerMode ringerMode = AudioPolicyManager::GetInstance().GetRingerMode();
    EXPECT_TRUE(ringerMode == AudioRingerMode::RINGER_MODE_SILENT);
}

/**
 * @tc.name  : Test GetRingerMode
 * @tc.number: GetRingerMode_002
 * @tc.desc  : Test GetRingerMode interface. Returns ringerMode.
 */
HWTEST(AudioPolicyUnitTest, GetRingerMode_002, TestSize.Level1)
{
    int32_t ret = AudioPolicyManager::GetInstance().SetRingerMode(AudioRingerMode::RINGER_MODE_VIBRATE);
    EXPECT_EQ(SUCCESS, ret);
    AudioRingerMode ringerMode = AudioPolicyManager::GetInstance().GetRingerMode();
    EXPECT_TRUE(ringerMode == AudioRingerMode::RINGER_MODE_VIBRATE);
}

/**
 * @tc.name  : Test GetRingerMode
 * @tc.number: GetRingerMode_003
 * @tc.desc  : Test GetRingerMode interface. Returns ringerMode.
 */
HWTEST(AudioPolicyUnitTest, GetRingerMode_003, TestSize.Level1)
{
    int32_t ret = AudioPolicyManager::GetInstance().SetRingerMode(AudioRingerMode::RINGER_MODE_NORMAL);
    EXPECT_EQ(SUCCESS, ret);
    AudioRingerMode ringerMode = AudioPolicyManager::GetInstance().GetRingerMode();
    EXPECT_TRUE(ringerMode == AudioRingerMode::RINGER_MODE_NORMAL);
}

/**
 * @tc.name  : Test SetAudioScene
 * @tc.number: SetAudioScene_001
 * @tc.desc  : Test SetAudioScene interface. Returns ret.
 */
HWTEST(AudioPolicyUnitTest, SetAudioScene_001, TestSize.Level1)
{
    int32_t ret = AudioPolicyManager::GetInstance().SetAudioScene(AudioScene::AUDIO_SCENE_INVALID);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
 * @tc.name  : Test SetAudioScene
 * @tc.number: SetAudioScene_002
 * @tc.desc  : Test SetAudioScene interface. Returns ret.
 */
HWTEST(AudioPolicyUnitTest, SetAudioScene_002, TestSize.Level1)
{
    int32_t ret = AudioPolicyManager::GetInstance().SetAudioScene(AudioScene::AUDIO_SCENE_MAX);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}


/**
 * @tc.name  : Test SetAudioScene
 * @tc.number: SetAudioScene_003
 * @tc.desc  : Test SetAudioScene interface. Returns ret.
 */
HWTEST(AudioPolicyUnitTest, SetAudioScene_003, TestSize.Level1)
{
    int32_t ret = AudioPolicyManager::GetInstance().SetAudioScene(AudioScene::AUDIO_SCENE_CALL_START);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
 * @tc.name  : Test SetAudioScene
 * @tc.number: SetAudioScene_004
 * @tc.desc  : Test SetAudioScene interface. Returns ret.
 */
HWTEST(AudioPolicyUnitTest, SetAudioScene_004, TestSize.Level1)
{
    int32_t ret = AudioPolicyManager::GetInstance().SetAudioScene(AudioScene::AUDIO_SCENE_CALL_END);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
 * @tc.name  : Test GetAudioScene
 * @tc.number: GetAudioScene_001
 * @tc.desc  : Test GetAudioScene interface. Returns audioScene.
 */
HWTEST(AudioPolicyUnitTest, GetAudioScene_001, TestSize.Level1)
{
    int32_t ret = AudioPolicyManager::GetInstance().SetAudioScene(AudioScene::AUDIO_SCENE_DEFAULT);
    EXPECT_EQ(SUCCESS, ret);
    AudioScene audioScene = AudioPolicyManager::GetInstance().GetAudioScene();
    EXPECT_TRUE(audioScene == AudioScene::AUDIO_SCENE_DEFAULT);
}

/**
 * @tc.name  : Test GetAudioScene
 * @tc.number: GetAudioScene_002
 * @tc.desc  : Test GetAudioScene interface. Returns audioScene.
 */
HWTEST(AudioPolicyUnitTest, GetAudioScene_002, TestSize.Level1)
{
    int32_t ret = AudioPolicyManager::GetInstance().SetAudioScene(AudioScene::AUDIO_SCENE_RINGING);
    EXPECT_EQ(SUCCESS, ret);
    AudioScene audioScene = AudioPolicyManager::GetInstance().GetAudioScene();
    EXPECT_TRUE(audioScene == AudioScene::AUDIO_SCENE_RINGING);
}

/**
 * @tc.name  : Test GetAudioScene
 * @tc.number: GetAudioScene_003
 * @tc.desc  : Test GetAudioScene interface. Returns audioScene.
 */
HWTEST(AudioPolicyUnitTest, GetAudioScene_003, TestSize.Level1)
{
    int32_t ret = AudioPolicyManager::GetInstance().SetAudioScene(AudioScene::AUDIO_SCENE_PHONE_CALL);
    EXPECT_EQ(SUCCESS, ret);
    AudioScene audioScene = AudioPolicyManager::GetInstance().GetAudioScene();
    EXPECT_TRUE(audioScene == AudioScene::AUDIO_SCENE_PHONE_CALL);
}

/**
 * @tc.name  : Test GetAudioScene
 * @tc.number: GetAudioScene_004
 * @tc.desc  : Test GetAudioScene interface. Returns audioScene.
 */
HWTEST(AudioPolicyUnitTest, GetAudioScene_004, TestSize.Level1)
{
    int32_t ret = AudioPolicyManager::GetInstance().SetAudioScene(AudioScene::AUDIO_SCENE_PHONE_CHAT);
    EXPECT_EQ(SUCCESS, ret);
    AudioScene audioScene = AudioPolicyManager::GetInstance().GetAudioScene();
    EXPECT_TRUE(audioScene == AudioScene::AUDIO_SCENE_PHONE_CHAT);
}

/**
 * @tc.name  : Test UpdateTracker
 * @tc.number: UpdateTracker_001
 * @tc.desc  : Test UpdateTracker interface. Returns ret.
 */
HWTEST(AudioPolicyUnitTest, UpdateTracker_001, TestSize.Level1)
{
    AudioMode audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    AudioStreamChangeInfo streamChangeInfo;
    std::shared_ptr<AudioClientTracker> clientTrackerObj = nullptr;
    int32_t ret = AudioPolicyManager::GetInstance().RegisterTracker(audioMode, streamChangeInfo, clientTrackerObj);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioPolicyManager::GetInstance().UpdateTracker(audioMode, streamChangeInfo);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test UpdateTracker
 * @tc.number: UpdateTracker_002
 * @tc.desc  : Test UpdateTracker interface. Returns ret.
 */
HWTEST(AudioPolicyUnitTest, UpdateTracker_002, TestSize.Level1)
{
    AudioMode audioMode = AudioMode::AUDIO_MODE_RECORD;
    AudioStreamChangeInfo streamChangeInfo;
    std::shared_ptr<AudioClientTracker> clientTrackerObj = nullptr;
    int32_t ret = AudioPolicyManager::GetInstance().RegisterTracker(audioMode, streamChangeInfo, clientTrackerObj);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioPolicyManager::GetInstance().UpdateTracker(audioMode, streamChangeInfo);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test UpdateTracker
 * @tc.number: UpdateTracker_003
 * @tc.desc  : Test UpdateTracker interface. Returns ret.
 */
HWTEST(AudioPolicyUnitTest, UpdateTracker_003, TestSize.Level1)
{
    AudioMode audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    AudioStreamChangeInfo streamChangeInfo;
    std::shared_ptr<AudioClientTracker> clientTrackerObj = std::make_shared<AudioClientTrackerTest>();
    int32_t ret = AudioPolicyManager::GetInstance().RegisterTracker(audioMode, streamChangeInfo, clientTrackerObj);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioPolicyManager::GetInstance().UpdateTracker(audioMode, streamChangeInfo);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test UpdateTracker
 * @tc.number: UpdateTracker_004
 * @tc.desc  : Test UpdateTracker interface. Returns ret.
 */
HWTEST(AudioPolicyUnitTest, UpdateTracker_004, TestSize.Level1)
{
    AudioMode audioMode = AudioMode::AUDIO_MODE_RECORD;
    AudioStreamChangeInfo streamChangeInfo;
    std::shared_ptr<AudioClientTracker> clientTrackerObj = std::make_shared<AudioClientTrackerTest>();
    int32_t ret = AudioPolicyManager::GetInstance().RegisterTracker(audioMode, streamChangeInfo, clientTrackerObj);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioPolicyManager::GetInstance().UpdateTracker(audioMode, streamChangeInfo);
    EXPECT_EQ(SUCCESS, ret);
}
} // namespace AudioStandard
} // namespace OHOS
