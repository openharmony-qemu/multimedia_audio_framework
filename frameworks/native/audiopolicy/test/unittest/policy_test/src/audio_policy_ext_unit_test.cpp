/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "iaudio_policy_client.h"
#include "audio_policy_unit_test.h"
#include "audio_system_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "standard_client_tracker_stub.h"
#include "audio_policy_client_stub_impl.h"
#include "audio_adapter_manager.h"
#include "audio_policy_manager.h"
#include "audio_capturer.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
    constexpr uint32_t MAX_RENDERER_INSTANCES = 128;
    constexpr int32_t SPEAKER_SAMPLING_RATE = 48000;
    constexpr int32_t SAMPLING_RATE_ERROR_CODE = -1;
}

class AudioPolicyExtUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static void InitializeCapturerOptions(AudioCapturerOptions &capturerOptions);
};

void AudioPolicyExtUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void AudioPolicyExtUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void AudioPolicyExtUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioPolicyExtUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

void AudioPolicyExtUnitTest::InitializeCapturerOptions(AudioCapturerOptions &capturerOptions)
{
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_96000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    capturerOptions.capturerInfo.capturerFlags = 0;
    return;
}

/**
 * @tc.name  : Test UpdateStreamState
 * @tc.number: UpdateStreamState_001
 * @tc.desc  : Test UpdateStreamState interface. Returns ret.
 */
HWTEST(AudioPolicyExtUnitTest, UpdateStreamState_001, TestSize.Level1)
{
    int32_t clientUid = 0;
    int32_t ret = AudioPolicyManager::GetInstance().UpdateStreamState(clientUid,
        StreamSetState::STREAM_PAUSE, StreamUsage::STREAM_USAGE_INVALID);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test UpdateStreamState
 * @tc.number: UpdateStreamState_002
 * @tc.desc  : Test UpdateStreamState interface. Returns ret.
 */
HWTEST(AudioPolicyExtUnitTest, UpdateStreamState_002, TestSize.Level1)
{
    int32_t clientUid = 1;
    int32_t ret = AudioPolicyManager::GetInstance().UpdateStreamState(clientUid,
        StreamSetState::STREAM_PAUSE, StreamUsage::STREAM_USAGE_UNKNOWN);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test UpdateStreamState
 * @tc.number: UpdateStreamState_003
 * @tc.desc  : Test UpdateStreamState interface. Returns ret.
 */
HWTEST(AudioPolicyExtUnitTest, UpdateStreamState_003, TestSize.Level1)
{
    int32_t clientUid = 2;
    int32_t ret = AudioPolicyManager::GetInstance().UpdateStreamState(clientUid,
        StreamSetState::STREAM_RESUME, StreamUsage::STREAM_USAGE_MEDIA);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test UpdateStreamState
 * @tc.number: UpdateStreamState_004
 * @tc.desc  : Test UpdateStreamState interface. Returns ret.
 */
HWTEST(AudioPolicyExtUnitTest, UpdateStreamState_004, TestSize.Level1)
{
    int32_t clientUid = 3;
    int32_t ret = AudioPolicyManager::GetInstance().UpdateStreamState(clientUid,
        StreamSetState::STREAM_RESUME, StreamUsage::STREAM_USAGE_MUSIC);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test GetVolumeGroupInfos via legal state
 * @tc.number: GetVolumeGroupInfos_001
 * @tc.desc  : Test GetVolumeGroupInfos interface. Get volume group infos and return ret.
 */
HWTEST(AudioPolicyExtUnitTest, GetVolumeGroupInfos_001, TestSize.Level1)
{
    std::vector<sptr<VolumeGroupInfo>> infos;
    std::string networkId = "";
    int32_t ret = AudioPolicyManager::GetInstance().GetVolumeGroupInfos(networkId, infos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(infos.size(), 0);
}

/**
 * @tc.name  : Test GetVolumeGroupInfos via legal state
 * @tc.number: GetVolumeGroupInfos_002
 * @tc.desc  : Test GetVolumeGroupInfos interface. Get volume group infos and return ret.
 */
HWTEST(AudioPolicyExtUnitTest, GetVolumeGroupInfos_002, TestSize.Level1)
{
    std::vector<sptr<VolumeGroupInfo>> infos;
    std::string networkId = LOCAL_NETWORK_ID;
    int32_t ret = AudioPolicyManager::GetInstance().GetVolumeGroupInfos(networkId, infos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_GT(infos.size(), 0);
}

/**
 * @tc.name  : Test GetNetworkIdByGroupId via legal state
 * @tc.number: GetNetworkIdByGroupId_001
 * @tc.desc  : Test GetNetworkIdByGroupId interface. Get networkId by groupId and return ret.
 */
HWTEST(AudioPolicyExtUnitTest, GetNetworkIdByGroupId_001, TestSize.Level1)
{
    std::vector<sptr<VolumeGroupInfo>> infos;
    std::string networkId = LOCAL_NETWORK_ID;
    int32_t ret = AudioPolicyManager::GetInstance().GetVolumeGroupInfos(networkId, infos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_GT(infos.size(), 0);
    int32_t groupId = infos[0]->volumeGroupId_;
    ret = AudioPolicyManager::GetInstance().GetNetworkIdByGroupId(groupId, networkId);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test GetNetworkIdByGroupId via illegal state
 * @tc.number: GetNetworkIdByGroupId_002
 * @tc.desc  : Test GetNetworkIdByGroupId interface. Get networkId by groupId and return ret.
 */
HWTEST(AudioPolicyExtUnitTest, GetNetworkIdByGroupId_002, TestSize.Level1)
{
    std::vector<sptr<VolumeGroupInfo>> infos;
    std::string networkId = "";
    int32_t ret = AudioPolicyManager::GetInstance().GetVolumeGroupInfos(networkId, infos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(infos.size(), 0);
    int32_t groupId = -1;
    ret = AudioPolicyManager::GetInstance().GetNetworkIdByGroupId(groupId, networkId);
    EXPECT_EQ(ERROR, ret);
}

/**
 * @tc.name  : Test SetClientCallbacksEnable via illegal state
 * @tc.number: SetClientCallbacksEnable_001
 * @tc.desc  : Test SetClientCallbacksEnable interface. Set callback enable and return ret.
 */
HWTEST(AudioPolicyExtUnitTest, SetClientCallbacksEnable_001, TestSize.Level1)
{
    int32_t ret = AudioPolicyManager::GetInstance().SetClientCallbacksEnable(CALLBACK_UNKNOWN, false);
    EXPECT_EQ(AUDIO_ERR, ret);
    ret = AudioPolicyManager::GetInstance().SetClientCallbacksEnable(CALLBACK_UNKNOWN, true);
    EXPECT_EQ(AUDIO_ERR, ret);
}

/**
 * @tc.name  : Test SetClientCallbacksEnable via illegal state
 * @tc.number: SetClientCallbacksEnable_002
 * @tc.desc  : Test SetClientCallbacksEnable interface. Set callback enable and return ret.
 */
HWTEST(AudioPolicyExtUnitTest, SetClientCallbacksEnable_002, TestSize.Level1)
{
    int32_t ret = AudioPolicyManager::GetInstance().SetClientCallbacksEnable(CALLBACK_MAX, false);
    EXPECT_EQ(AUDIO_ERR, ret);
    ret = AudioPolicyManager::GetInstance().SetClientCallbacksEnable(CALLBACK_MAX, true);
    EXPECT_EQ(AUDIO_ERR, ret);
}

/**
 * @tc.name  : Test SetClientCallbacksEnable via legal state
 * @tc.number: SetClientCallbacksEnable_003
 * @tc.desc  : Test SetClientCallbacksEnable interface. Set callback enable and return ret.
 */
HWTEST(AudioPolicyExtUnitTest, SetClientCallbacksEnable_003, TestSize.Level1)
{
    int32_t ret = AudioPolicyManager::GetInstance().SetClientCallbacksEnable(CALLBACK_FOCUS_INFO_CHANGE, false);
    EXPECT_EQ(AUDIO_OK, ret);
    ret = AudioPolicyManager::GetInstance().SetClientCallbacksEnable(CALLBACK_FOCUS_INFO_CHANGE, true);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
 * @tc.name  : Test SetClientCallbacksEnable via legal state
 * @tc.number: SetClientCallbacksEnable_004
 * @tc.desc  : Test SetClientCallbacksEnable interface. Set callback enable and return ret.
 */
HWTEST(AudioPolicyExtUnitTest, SetClientCallbacksEnable_004, TestSize.Level1)
{
    int32_t ret = AudioPolicyManager::GetInstance().SetClientCallbacksEnable(CALLBACK_RENDERER_STATE_CHANGE, false);
    EXPECT_EQ(AUDIO_OK, ret);
    ret = AudioPolicyManager::GetInstance().SetClientCallbacksEnable(CALLBACK_RENDERER_STATE_CHANGE, true);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
 * @tc.name  : Test SetClientCallbacksEnable via legal state
 * @tc.number: SetClientCallbacksEnable_005
 * @tc.desc  : Test SetClientCallbacksEnable interface. Set callback enable and return ret.
 */
HWTEST(AudioPolicyExtUnitTest, SetClientCallbacksEnable_005, TestSize.Level1)
{
    int32_t ret = AudioPolicyManager::GetInstance().SetClientCallbacksEnable(CALLBACK_CAPTURER_STATE_CHANGE, false);
    EXPECT_EQ(AUDIO_OK, ret);
    ret = AudioPolicyManager::GetInstance().SetClientCallbacksEnable(CALLBACK_CAPTURER_STATE_CHANGE, true);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
 * @tc.name  : Test GetMaxRendererInstances via legal state
 * @tc.number: GetMaxRendererInstances_001
 * @tc.desc  : Test GetMaxRendererInstances interface.Get max renderer instances and return ret.
 */
HWTEST(AudioPolicyExtUnitTest, GetMaxRendererInstances_001, TestSize.Level1)
{
    int32_t ret = AudioPolicyManager::GetInstance().GetMaxRendererInstances();
    EXPECT_EQ(MAX_RENDERER_INSTANCES, ret);
}

/**
 * @tc.name  : Test QueryEffectSceneMode via legal state
 * @tc.number: QueryEffectSceneMode_001
 * @tc.desc  : Test QueryEffectSceneMode interface.Query effect scene mode and return ret.
 */
HWTEST(AudioPolicyExtUnitTest, QueryEffectSceneMode_001, TestSize.Level1)
{
    SupportedEffectConfig supportedEffectConfig;
    int32_t ret = AudioPolicyManager::GetInstance().QueryEffectSceneMode(supportedEffectConfig);
    EXPECT_EQ(0, ret);
}

/**
 * @tc.name  : Test GetMaxAmplitude via legal state
 * @tc.number: GetMaxAmplitude_001
 * @tc.desc  : Test GetMaxAmplitude interface.Query effect scene mode and return ret.
 */
HWTEST(AudioPolicyExtUnitTest, GetMaxAmplitude_001, TestSize.Level1)
{
    int32_t ret = AudioPolicyManager::GetInstance().GetMaxAmplitude(0);
    EXPECT_EQ(0, ret);
}

/**
 * @tc.name  : Test GetMinStreamVolume via legal state
 * @tc.number: GetMinStreamVolume_001
 * @tc.desc  : Test GetMinStreamVolume interface.get min stream volume and return ret.
 */
HWTEST(AudioPolicyExtUnitTest, GetMinStreamVolume_001, TestSize.Level1)
{
    float minStreamVolume = AudioPolicyManager::GetInstance().GetMinStreamVolume();
    float maxStreamVolume = AudioPolicyManager::GetInstance().GetMaxStreamVolume();
    EXPECT_LT(minStreamVolume, maxStreamVolume);
}

/**
 * @tc.name  : Test RecoverAudioPolicyCallbackClient
 * @tc.number: RecoverAudioPolicyCallbackClient_001
 * @tc.desc  : Test RecoverAudioPolicyCallbackClient interface.
 */
HWTEST(AudioPolicyExtUnitTest, RecoverAudioPolicyCallbackClient_001, TestSize.Level1)
{
    int32_t ret;
    int32_t volumeLevel = 4;
    ret = AudioPolicyManager::GetInstance().SetSystemVolumeLevel(AudioVolumeType::STREAM_MUSIC, volumeLevel);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioPolicyManager::GetInstance().GetSystemVolumeLevel(AudioVolumeType::STREAM_MUSIC);
    EXPECT_EQ(volumeLevel, ret);

    AudioPolicyManager::GetInstance().RecoverAudioPolicyCallbackClient();

    ret = AudioPolicyManager::GetInstance().GetSystemVolumeLevel(AudioVolumeType::STREAM_MUSIC);
    EXPECT_EQ(volumeLevel, ret);
}

/**
 * @tc.name  : Test RecoverAudioPolicyCallbackClient
 * @tc.number: RecoverAudioPolicyCallbackClient_002
 * @tc.desc  : Test RecoverAudioPolicyCallbackClient interface abnormal branch.
 */
HWTEST(AudioPolicyExtUnitTest, RecoverAudioPolicyCallbackClient_002, TestSize.Level3)
{
    int32_t ret;
    int32_t volumeLevel = 4;
    ret = AudioPolicyManager::GetInstance().SetSystemVolumeLevel(AudioVolumeType::STREAM_MUSIC, volumeLevel);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioPolicyManager::GetInstance().GetSystemVolumeLevel(AudioVolumeType::STREAM_MUSIC);
    EXPECT_EQ(volumeLevel, ret);

    AudioPolicyManager::GetInstance().audioPolicyClientStubCB_ = nullptr;
    AudioPolicyManager::GetInstance().RecoverAudioPolicyCallbackClient();

    ret = AudioPolicyManager::GetInstance().GetSystemVolumeLevel(AudioVolumeType::STREAM_MUSIC);
    EXPECT_EQ(volumeLevel, ret);
}

/**
 * @tc.name  : Test AudioPolicyServerDied
 * @tc.number: AudioPolicyServerDied_001
 * @tc.desc  : Test AudioPolicyServerDied interface.
 */
HWTEST(AudioPolicyExtUnitTest, AudioPolicyServerDied_001, TestSize.Level1)
{
    int32_t pid = getpid();
    int32_t uid = getuid();
    AudioPolicyManager::GetInstance().AudioPolicyServerDied(pid, uid);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors =
        AudioPolicyManager::GetInstance().GetDevices(DeviceFlag::INPUT_DEVICES_FLAG);
    EXPECT_TRUE(audioDeviceDescriptors.size() > 0);
}

/**
 * @tc.name  : Test SetRingerModeLegacy
 * @tc.number: SetRingerModeLegacy_001
 * @tc.desc  : Test SetRingerModeLegacy interface.
 */
HWTEST(AudioPolicyExtUnitTest, SetRingerModeLegacy_001, TestSize.Level1)
{
    int32_t ret = AudioPolicyManager::GetInstance().SetRingerModeLegacy(AudioRingerMode::RINGER_MODE_SILENT);
    EXPECT_EQ(SUCCESS, ret);
    AudioRingerMode ringerMode = AudioPolicyManager::GetInstance().GetRingerMode();
    EXPECT_TRUE(ringerMode == AudioRingerMode::RINGER_MODE_SILENT);

    ret = AudioPolicyManager::GetInstance().SetRingerModeLegacy(AudioRingerMode::RINGER_MODE_NORMAL);
    EXPECT_EQ(SUCCESS, ret);
    ringerMode = AudioPolicyManager::GetInstance().GetRingerMode();
    EXPECT_TRUE(ringerMode == AudioRingerMode::RINGER_MODE_NORMAL);
}

/**
 * @tc.name  : Test GetSessionInfoInFocus
 * @tc.number: GetSessionInfoInFocus_001
 * @tc.desc  : Test GetSessionInfoInFocus interface.
 */
HWTEST(AudioPolicyExtUnitTest, GetSessionInfoInFocus_001, TestSize.Level1)
{
    AudioInterrupt audioInterrupt;
    int32_t zoneID = 0;
    int32_t ret = AudioPolicyManager::GetInstance().GetSessionInfoInFocus(audioInterrupt, zoneID);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test IsMicrophoneMuteLegacy
 * @tc.number: IsMicrophoneMuteLegacy_001
 * @tc.desc  : Test IsMicrophoneMuteLegacy interface abnormal branch.
 */
HWTEST(AudioPolicyExtUnitTest, IsMicrophoneMuteLegacy_001, TestSize.Level3)
{
    int32_t ret = AudioPolicyManager::GetInstance().SetMicrophoneMute(true);
    EXPECT_EQ(ret, SUCCESS);
    bool muteStatus = AudioPolicyManager::GetInstance().IsMicrophoneMuteLegacy();
    EXPECT_EQ(muteStatus, true);

    ret = AudioPolicyManager::GetInstance().SetMicrophoneMute(false);
    EXPECT_EQ(ret, SUCCESS);
    AudioPolicyManager::GetInstance().isAudioPolicyClientRegisted_ = false;
    muteStatus = AudioPolicyManager::GetInstance().IsMicrophoneMuteLegacy();
    EXPECT_EQ(muteStatus, false);
}

/**
 * @tc.name  : Test IsMicrophoneMute
 * @tc.number: IsMicrophoneMute_001
 * @tc.desc  : Test IsMicrophoneMute interface abnormal branch.
 */
HWTEST(AudioPolicyExtUnitTest, IsMicrophoneMute_001, TestSize.Level3)
{
    int32_t result = AudioPolicyManager::GetInstance().SetMicrophoneMute(true);
    EXPECT_EQ(result, SUCCESS);
    bool muteStatus = AudioPolicyManager::GetInstance().IsMicrophoneMute();
    EXPECT_EQ(muteStatus, true);

    result = AudioPolicyManager::GetInstance().SetMicrophoneMute(false);
    EXPECT_EQ(result, SUCCESS);
    AudioPolicyManager::GetInstance().isAudioPolicyClientRegisted_ = false;
    muteStatus = AudioPolicyManager::GetInstance().IsMicrophoneMute();
    EXPECT_EQ(muteStatus, false);
}

/**
 * @tc.name  : Test GetDevicesInner
 * @tc.number: GetDevicesInner_001
 * @tc.desc  : Test GetDevicesInner interface callback.
 */
HWTEST(AudioPolicyExtUnitTest, GetDevicesInner_001, TestSize.Level1)
{
    DeviceFlag deviceFlag = ALL_DEVICES_FLAG;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;

    devices = AudioPolicyManager::GetInstance().GetDevices(deviceFlag);
    EXPECT_TRUE(devices.size() > 0);

    devices = AudioPolicyManager::GetInstance().GetDevicesInner(deviceFlag);
    EXPECT_TRUE(devices.size() <= 0);
}

/**
 * @tc.name  : Test RegisterFocusInfoChangeCallback
 * @tc.number: RegisterFocusInfoChangeCallback_001
 * @tc.desc  : Test RegisterFocusInfoChangeCallback interface legal situation.
 * @tc.type  : FUNC
 */
HWTEST(AudioPolicyExtUnitTest, RegisterFocusInfoChangeCallback_001, TestSize.Level1)
{
    int32_t clientId = getpid();
    std::shared_ptr<AudioFocusInfoChangeCallback> callback = nullptr;
    int32_t ret = AudioPolicyManager::GetInstance().RegisterFocusInfoChangeCallback(clientId, callback);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);

    callback = make_shared<AudioFocusInfoChangeCallbackTest>();

    AudioPolicyManager::GetInstance().isAudioPolicyClientRegisted_ = false;
    ret = AudioPolicyManager::GetInstance().RegisterFocusInfoChangeCallback(clientId, callback);
    EXPECT_EQ(ret, SUCCESS);

    ret = AudioPolicyManager::GetInstance().RegisterFocusInfoChangeCallback(clientId, callback);
    EXPECT_EQ(ret, SUCCESS);

    ret = AudioPolicyManager::GetInstance().UnregisterFocusInfoChangeCallback(clientId);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetPreferredOutputDeviceChangeCallback
 * @tc.number: SetPreferredOutputDeviceChangeCallback_001
 * @tc.desc  : Test SetPreferredOutputDeviceChangeCallback interface abnormal branch.
 * @tc.type  : FUNC
 */
HWTEST(AudioPolicyExtUnitTest, SetPreferredOutputDeviceChangeCallback_001, TestSize.Level3)
{
    int32_t ret = -1;
    AudioRendererInfo rendererInfo;
    ret = AudioPolicyManager::GetInstance().SetPreferredOutputDeviceChangeCallback(rendererInfo, nullptr);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);

    std::shared_ptr<AudioPreferredOutputDeviceChangeCallback> callback =
        std::make_shared<AudioPreferredOutputDeviceChangeCallbackTest>();

    AudioPolicyManager::GetInstance().isAudioPolicyClientRegisted_ = false;
    ret = AudioPolicyManager::GetInstance().SetPreferredOutputDeviceChangeCallback(rendererInfo, callback);
    EXPECT_EQ(SUCCESS, ret);

    ret = AudioPolicyManager::GetInstance().SetPreferredOutputDeviceChangeCallback(rendererInfo, callback);
    EXPECT_EQ(SUCCESS, ret);

    ret = AudioPolicyManager::GetInstance().UnsetPreferredOutputDeviceChangeCallback(callback);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetPreferredInputDeviceChangeCallback
 * @tc.number: SetPreferredInputDeviceChangeCallback_001
 * @tc.desc  : Test SetPreferredInputDeviceChangeCallback interface abnormal branch.
 * @tc.type  : FUNC
 */
HWTEST(AudioPolicyExtUnitTest, SetPreferredInputDeviceChangeCallback_001, TestSize.Level3)
{
    AudioCapturerInfo capturerInfo;
    int32_t ret = AudioPolicyManager::GetInstance().SetPreferredInputDeviceChangeCallback(capturerInfo, nullptr);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);

    std::shared_ptr<AudioPreferredInputDeviceChangeCallbackTest> callback =
        std::make_shared<AudioPreferredInputDeviceChangeCallbackTest>();

    AudioPolicyManager::GetInstance().isAudioPolicyClientRegisted_ = false;
    ret = AudioPolicyManager::GetInstance().SetPreferredInputDeviceChangeCallback(capturerInfo, callback);
    EXPECT_EQ(SUCCESS, ret);

    ret = AudioPolicyManager::GetInstance().SetPreferredInputDeviceChangeCallback(capturerInfo, callback);
    EXPECT_EQ(SUCCESS, ret);

    ret = AudioPolicyManager::GetInstance().UnsetPreferredInputDeviceChangeCallback(callback);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetMicStateChangeCallback
 * @tc.number: SetMicStateChangeCallback_001
 * @tc.desc  : Test SetMicStateChangeCallback interface abnormal branch.
 * @tc.type  : FUNC
 */
HWTEST(AudioPolicyExtUnitTest, SetMicStateChangeCallback_001, TestSize.Level3)
{
    int32_t clientId = getpid();
    int32_t ret = AudioPolicyManager::GetInstance().SetMicStateChangeCallback(clientId, nullptr);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);

    std::shared_ptr<AudioManagerMicStateChangeCallbackTest> callback =
        std::make_shared<AudioManagerMicStateChangeCallbackTest>();

    AudioPolicyManager::GetInstance().isAudioPolicyClientRegisted_ = false;
    ret = AudioPolicyManager::GetInstance().SetMicStateChangeCallback(clientId, callback);
    EXPECT_EQ(SUCCESS, ret);

    ret = AudioPolicyManager::GetInstance().SetMicStateChangeCallback(clientId, callback);
    EXPECT_EQ(SUCCESS, ret);

    ret = AudioPolicyManager::GetInstance().UnsetMicStateChangeCallback(callback);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test GetAudioCapturerMicrophoneDescriptors
 * @tc.number: GetAudioCapturerMicrophoneDescriptors_001
 * @tc.desc  : Test GetAudioCapturerMicrophoneDescriptors interface legal situation.
 * @tc.type  : FUNC
 */
HWTEST(AudioPolicyExtUnitTest, GetAudioCapturerMicrophoneDescriptors_001, TestSize.Level1)
{
    AudioCapturerOptions capturerOptions;
    AudioPolicyExtUnitTest::InitializeCapturerOptions(capturerOptions);

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(nullptr, audioCapturer);

    bool isStarted = audioCapturer->Start();
    EXPECT_EQ(true, isStarted);

    auto inputDeviceDescriptors = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::INPUT_DEVICES_FLAG);
    ASSERT_NE(inputDeviceDescriptors.size(), 0);

    std::vector<sptr<MicrophoneDescriptor>> microphoneDescriptors = audioCapturer->GetCurrentMicrophones();
    EXPECT_GT(microphoneDescriptors.size(), 0);
    audioCapturer->Release();
}

/**
 * @tc.name  : Test RegisterSpatializationEnabledEventListener
 * @tc.number: RegisterSpatializationEnabledEventListener_001
 * @tc.desc  : Test RegisterSpatializationEnabledEventListener interface abnormal branch.
 * @tc.type  : FUNC
 */
HWTEST(AudioPolicyExtUnitTest, RegisterSpatializationEnabledEventListener_001, TestSize.Level3)
{
    std::shared_ptr<AudioSpatializationEnabledChangeCallback> callback = nullptr;
    int32_t ret = AudioPolicyManager::GetInstance().RegisterSpatializationEnabledEventListener(callback);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);

    callback = std::make_shared<AudioSpatializationEnabledChangeCallbackTest>();

    AudioPolicyManager::GetInstance().isAudioPolicyClientRegisted_ = false;
    ret = AudioPolicyManager::GetInstance().RegisterSpatializationEnabledEventListener(callback);
    EXPECT_EQ(SUCCESS, ret);

    ret = AudioPolicyManager::GetInstance().RegisterSpatializationEnabledEventListener(callback);
    EXPECT_EQ(SUCCESS, ret);

    ret = AudioPolicyManager::GetInstance().UnregisterSpatializationEnabledEventListener();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test UnregisterSpatializationEnabledEventListener
 * @tc.number: UnregisterSpatializationEnabledEventListener_001
 * @tc.desc  : Test UnregisterSpatializationEnabledEventListener interface abnormal branch.
 * @tc.type  : FUNC
 */
HWTEST(AudioPolicyExtUnitTest, UnregisterSpatializationEnabledEventListener_001, TestSize.Level3)
{
    int32_t ret = -1;
    std::shared_ptr<AudioSpatializationEnabledChangeCallback> callback =
        std::make_shared<AudioSpatializationEnabledChangeCallbackTest>();

    ret = AudioPolicyManager::GetInstance().RegisterSpatializationEnabledEventListener(callback);
    EXPECT_EQ(SUCCESS, ret);

    ret = AudioPolicyManager::GetInstance().UnregisterSpatializationEnabledEventListener();
    EXPECT_EQ(SUCCESS, ret);

    AudioPolicyManager::GetInstance().audioPolicyClientStubCB_ = nullptr;
    ret = AudioPolicyManager::GetInstance().UnregisterSpatializationEnabledEventListener();
    EXPECT_EQ(SUCCESS, ret);

    AudioPolicyManager::GetInstance().isAudioPolicyClientRegisted_ = false;
    ret = AudioPolicyManager::GetInstance().RegisterSpatializationEnabledEventListener(callback);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test RegisterSpatializationEnabledForCurrentDeviceEventListener
 * @tc.number: RegisterSpatializationEnabledForCurrentDeviceEventListener_001
 * @tc.desc  : Test RegisterSpatializationEnabledForCurrentDeviceEventListener interface abnormal branch.
 * @tc.type  : FUNC
 */
HWTEST(AudioPolicyExtUnitTest, RegisterSpatializationEnabledForCurrentDeviceEventListener_001, TestSize.Level3)
{
    std::shared_ptr<AudioSpatializationEnabledChangeForCurrentDeviceCallback> callback = nullptr;
    int32_t ret = AudioPolicyManager::
        GetInstance().RegisterSpatializationEnabledForCurrentDeviceEventListener(callback);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);

    callback = std::make_shared<AudioSpatialEnabledChangeForCurrDeviceCbTest>();

    AudioPolicyManager::GetInstance().isAudioPolicyClientRegisted_ = false;
    ret = AudioPolicyManager::GetInstance().RegisterSpatializationEnabledForCurrentDeviceEventListener(callback);
    EXPECT_EQ(SUCCESS, ret);

    ret = AudioPolicyManager::GetInstance().RegisterSpatializationEnabledForCurrentDeviceEventListener(callback);
    EXPECT_EQ(SUCCESS, ret);

    ret = AudioPolicyManager::GetInstance().UnregisterSpatializationEnabledForCurrentDeviceEventListener();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test UnregisterSpatializationEnabledForCurrentDeviceEventListener
 * @tc.number: UnregisterSpatializationEnabledForCurrentDeviceEventListener001
 * @tc.desc  : Test UnregisterSpatializationEnabledForCurrentDeviceEventListener interface abnormal branch.
 * @tc.type  : FUNC
 */
HWTEST(AudioPolicyExtUnitTest, UnregisterSpatializationEnabledForCurrentDeviceEventListener001, TestSize.Level3)
{
    int32_t ret = -1;
    std::shared_ptr<AudioSpatializationEnabledChangeForCurrentDeviceCallback> callback =
        std::make_shared<AudioSpatialEnabledChangeForCurrDeviceCbTest>();

    ret = AudioPolicyManager::GetInstance().RegisterSpatializationEnabledForCurrentDeviceEventListener(callback);
    EXPECT_EQ(SUCCESS, ret);

    ret = AudioPolicyManager::GetInstance().UnregisterSpatializationEnabledForCurrentDeviceEventListener();
    EXPECT_EQ(SUCCESS, ret);

    AudioPolicyManager::GetInstance().audioPolicyClientStubCB_ = nullptr;
    ret = AudioPolicyManager::GetInstance().UnregisterSpatializationEnabledForCurrentDeviceEventListener();
    EXPECT_EQ(SUCCESS, ret);

    AudioPolicyManager::GetInstance().isAudioPolicyClientRegisted_ = false;
    ret = AudioPolicyManager::GetInstance().RegisterSpatializationEnabledForCurrentDeviceEventListener(callback);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test RegisterHeadTrackingEnabledEventListener
 * @tc.number: RegisterHeadTrackingEnabledEventListener_001
 * @tc.desc  : Test RegisterHeadTrackingEnabledEventListener interface abnormal branch.
 * @tc.type  : FUNC
 */
HWTEST(AudioPolicyExtUnitTest, RegisterHeadTrackingEnabledEventListener_001, TestSize.Level3)
{
    std::shared_ptr<AudioHeadTrackingEnabledChangeCallback> callback = nullptr;
    int32_t ret = AudioPolicyManager::GetInstance().RegisterHeadTrackingEnabledEventListener(callback);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);

    callback = std::make_shared<AudioHeadTrackingEnabledChangeCallbackTest>();

    AudioPolicyManager::GetInstance().isAudioPolicyClientRegisted_ = false;
    ret = AudioPolicyManager::GetInstance().RegisterHeadTrackingEnabledEventListener(callback);
    EXPECT_EQ(SUCCESS, ret);

    ret = AudioPolicyManager::GetInstance().RegisterHeadTrackingEnabledEventListener(callback);
    EXPECT_EQ(SUCCESS, ret);

    ret = AudioPolicyManager::GetInstance().UnregisterHeadTrackingEnabledEventListener();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test UnregisterHeadTrackingEnabledEventListener
 * @tc.number: UnregisterHeadTrackingEnabledEventListener_001
 * @tc.desc  : Test UnregisterHeadTrackingEnabledEventListener interface abnormal branch.
 * @tc.type  : FUNC
 */
HWTEST(AudioPolicyExtUnitTest, UnregisterHeadTrackingEnabledEventListener_001, TestSize.Level3)
{
    int32_t ret = -1;
    std::shared_ptr<AudioHeadTrackingEnabledChangeCallback> callback =
        std::make_shared<AudioHeadTrackingEnabledChangeCallbackTest>();

    ret = AudioPolicyManager::GetInstance().RegisterHeadTrackingEnabledEventListener(callback);
    EXPECT_EQ(SUCCESS, ret);

    ret = AudioPolicyManager::GetInstance().UnregisterHeadTrackingEnabledEventListener();
    EXPECT_EQ(SUCCESS, ret);

    AudioPolicyManager::GetInstance().audioPolicyClientStubCB_ = nullptr;
    ret = AudioPolicyManager::GetInstance().UnregisterHeadTrackingEnabledEventListener();
    EXPECT_EQ(SUCCESS, ret);

    AudioPolicyManager::GetInstance().isAudioPolicyClientRegisted_ = false;
    ret = AudioPolicyManager::GetInstance().RegisterHeadTrackingEnabledEventListener(callback);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test GetHardwareOutputSamplingRate
 * @tc.number: GetHardwareOutputSamplingRate_001
 * @tc.desc  : Test GetHardwareOutputSamplingRate interface.
 * @tc.type  : FUNC
 */
HWTEST(AudioPolicyExtUnitTest, GetHardwareOutputSamplingRate_001, TestSize.Level1)
{
    int32_t ret = -1;
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    ret = AudioPolicyManager::GetInstance().GetHardwareOutputSamplingRate(desc);
    EXPECT_NE(SUCCESS, ret);

    auto outputDeviceDescriptors = AudioPolicyManager::GetInstance().GetDevices(DeviceFlag::OUTPUT_DEVICES_FLAG);

    int32_t samplingRate;
    if (outputDeviceDescriptors.size() > 0) {
        for (auto outputDescriptor : outputDeviceDescriptors) {
            if (outputDescriptor->deviceType_ == DeviceType::DEVICE_TYPE_SPEAKER) {
                desc->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
                desc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
                samplingRate = AudioPolicyManager::GetInstance().GetHardwareOutputSamplingRate(desc);
                EXPECT_EQ(SPEAKER_SAMPLING_RATE, samplingRate);
            }
        }
    }

    auto inputDeviceDescriptors = AudioPolicyManager::GetInstance().GetDevices(DeviceFlag::INPUT_DEVICES_FLAG);
    if (inputDeviceDescriptors.size() > 0) {
        for (auto inputDescriptor : inputDeviceDescriptors) {
            desc->deviceType_ = inputDescriptor->deviceType_;
            desc->deviceRole_ = inputDescriptor->deviceRole_;
            samplingRate = AudioPolicyManager::GetInstance().GetHardwareOutputSamplingRate(desc);
            EXPECT_EQ(SAMPLING_RATE_ERROR_CODE, samplingRate);
        }
    }
}

/**
 * @tc.name  : Test IsAbsVolumeScene
 * @tc.number: IsAbsVolumeScene_001
 * @tc.desc  : Test IsAbsVolumeScene interface.
 * @tc.type  : FUNC
 */
HWTEST(AudioPolicyExtUnitTest, IsAbsVolumeScene_001, TestSize.Level1)
{
    bool ret = AudioPolicyManager::GetInstance().IsAbsVolumeScene();
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name  : Test IsSpatializationSupported
 * @tc.number: IsSpatializationSupported_001
 * @tc.desc  : Test IsSpatializationSupported interface.
 * @tc.type  : FUNC
 */
HWTEST(AudioPolicyExtUnitTest, IsSpatializationSupported_001, TestSize.Level1)
{
    bool ret = AudioPolicyManager::GetInstance().IsSpatializationSupported();
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name  : Test IsSpatializationSupportedForDevice
 * @tc.number: IsSpatializationSupportedForDevice_001
 * @tc.desc  : Test IsSpatializationSupportedForDevice interface.
 * @tc.type  : FUNC
 */
HWTEST(AudioPolicyExtUnitTest, IsSpatializationSupportedForDevice_001, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors =
        AudioPolicyManager::GetInstance().GetDevices(DeviceFlag::OUTPUT_DEVICES_FLAG);

    if (audioDeviceDescriptors.size() > 0) {
        for (auto outputDevice : audioDeviceDescriptors) {
            if (outputDevice->deviceType_ != DeviceType::DEVICE_TYPE_SPEAKER) {
                continue;
            }
            if ((outputDevice->macAddress_).c_str() != nullptr) {
                bool ret =
                    AudioPolicyManager::GetInstance().IsSpatializationSupportedForDevice(outputDevice->macAddress_);
                EXPECT_EQ(false, ret);
            }
        }
    }
}

/**
 * @tc.name  : Test IsHeadTrackingSupported
 * @tc.number: IsHeadTrackingSupported_001
 * @tc.desc  : Test IsHeadTrackingSupported interface.
 * @tc.type  : FUNC
 */
HWTEST(AudioPolicyExtUnitTest, IsHeadTrackingSupported_001, TestSize.Level1)
{
    bool ret = AudioPolicyManager::GetInstance().IsHeadTrackingSupported();
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name  : Test IsHeadTrackingSupportedForDevice
 * @tc.number: IsHeadTrackingSupportedForDevice_001
 * @tc.desc  : Test IsHeadTrackingSupportedForDevice interface.
 * @tc.type  : FUNC
 */
HWTEST(AudioPolicyExtUnitTest, IsHeadTrackingSupportedForDevice_001, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors =
        AudioPolicyManager::GetInstance().GetDevices(DeviceFlag::OUTPUT_DEVICES_FLAG);
    ASSERT_NE(audioDeviceDescriptors.size(), 0);

    for (auto outputDevice : audioDeviceDescriptors) {
        if (outputDevice->deviceType_ != DeviceType::DEVICE_TYPE_SPEAKER) {
            continue;
        }
        if ((outputDevice->macAddress_).c_str() != nullptr) {
            bool isSupported =
                AudioPolicyManager::GetInstance().IsHeadTrackingSupportedForDevice(outputDevice->macAddress_);
            EXPECT_EQ(false, isSupported);
        }
    }
}

/**
 * @tc.name  : Test UpdateSpatialDeviceState
 * @tc.number: UpdateSpatialDeviceState_001
 * @tc.desc  : Test UpdateSpatialDeviceState interface.
 * @tc.type  : FUNC
 */
HWTEST(AudioPolicyExtUnitTest, UpdateSpatialDeviceState_001, TestSize.Level1)
{
    AudioSpatialDeviceState audioSpatialDeviceState;
    int32_t ret = AudioPolicyManager::GetInstance().UpdateSpatialDeviceState(audioSpatialDeviceState);
    EXPECT_EQ(false, ret);
}

} // namespace AudioStandard
} // namespace OHOS
