/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "audio_policy_client_proxy_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioPolicyClientProxyUnitTest::SetUpTestCase(void) {}
void AudioPolicyClientProxyUnitTest::TearDownTestCase(void) {}
void AudioPolicyClientProxyUnitTest::SetUp(void) {}
void AudioPolicyClientProxyUnitTest::TearDown(void) {}

class RemoteObjectTestStub : public IRemoteObject {
public:
    RemoteObjectTestStub() : IRemoteObject(u"IRemoteObject") {}
    int32_t GetObjectRefCount() { return 0; };
    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) { return 0; };
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; };
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; };
    int Dump(int fd, const std::vector<std::u16string> &args) { return 0; };

    DECLARE_INTERFACE_DESCRIPTOR(u"RemoteObjectTestStub");
};


/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_001.
* @tc.desc  : Test OnVolumeKeyEvent.
*/
HWTEST_F(AudioPolicyClientProxyUnitTest, AudioPolicyClientProxy_001, TestSize.Level1)
{
    VolumeEvent volumeEvent;
    sptr<IRemoteObject> impl = new RemoteObjectTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnVolumeKeyEvent(volumeEvent);
    EXPECT_NE(audioPolicyClientProxy, nullptr);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_002.
* @tc.desc  : Test OnAudioFocusInfoChange.
*/
HWTEST_F(AudioPolicyClientProxyUnitTest, AudioPolicyClientProxy_002, TestSize.Level1)
{
    std::list<std::pair<AudioInterrupt, AudioFocuState>> focusInfoList;
    sptr<IRemoteObject> impl = new RemoteObjectTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnAudioFocusInfoChange(focusInfoList);
    EXPECT_NE(audioPolicyClientProxy, nullptr);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_003.
* @tc.desc  : Test OnAudioFocusRequested.
*/
HWTEST_F(AudioPolicyClientProxyUnitTest, AudioPolicyClientProxy_003, TestSize.Level1)
{
    AudioInterrupt requestFocus;
    sptr<IRemoteObject> impl = new RemoteObjectTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnAudioFocusRequested(requestFocus);
    EXPECT_NE(audioPolicyClientProxy, nullptr);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_004.
* @tc.desc  : Test OnAudioFocusAbandoned.
*/
HWTEST_F(AudioPolicyClientProxyUnitTest, AudioPolicyClientProxy_004, TestSize.Level1)
{
    AudioInterrupt requestFocus;
    sptr<IRemoteObject> impl = new RemoteObjectTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnAudioFocusAbandoned(requestFocus);
    EXPECT_NE(audioPolicyClientProxy, nullptr);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_005.
* @tc.desc  : Test OnDeviceChange.
*/
HWTEST_F(AudioPolicyClientProxyUnitTest, AudioPolicyClientProxy_005, TestSize.Level1)
{
    DeviceChangeAction deviceChangeAction;
    sptr<IRemoteObject> impl = new RemoteObjectTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnDeviceChange(deviceChangeAction);
    EXPECT_NE(audioPolicyClientProxy, nullptr);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_006.
* @tc.desc  : Test OnMicrophoneBlocked.
*/
HWTEST_F(AudioPolicyClientProxyUnitTest, AudioPolicyClientProxy_006, TestSize.Level1)
{
    MicrophoneBlockedInfo microphoneBlockedInfo;
    sptr<IRemoteObject> impl = new RemoteObjectTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnMicrophoneBlocked(microphoneBlockedInfo);
    EXPECT_NE(audioPolicyClientProxy, nullptr);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_007.
* @tc.desc  : Test OnRingerModeUpdated.
*/
HWTEST_F(AudioPolicyClientProxyUnitTest, AudioPolicyClientProxy_007, TestSize.Level1)
{
    sptr<IRemoteObject> impl = new RemoteObjectTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnRingerModeUpdated(AudioRingerMode::RINGER_MODE_NORMAL);
    EXPECT_NE(audioPolicyClientProxy, nullptr);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_008.
* @tc.desc  : Test OnMicStateUpdated.
*/
HWTEST_F(AudioPolicyClientProxyUnitTest, AudioPolicyClientProxy_008, TestSize.Level1)
{
    MicStateChangeEvent micStateChangeEvent;
    sptr<IRemoteObject> impl = new RemoteObjectTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnMicStateUpdated(micStateChangeEvent);
    EXPECT_NE(audioPolicyClientProxy, nullptr);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_009.
* @tc.desc  : Test OnPreferredOutputDeviceUpdated.
*/
HWTEST_F(AudioPolicyClientProxyUnitTest, AudioPolicyClientProxy_009, TestSize.Level1)
{
    AudioRendererInfo rendererInfo;
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    sptr<IRemoteObject> impl = new RemoteObjectTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnPreferredOutputDeviceUpdated(rendererInfo, desc);
    EXPECT_NE(audioPolicyClientProxy, nullptr);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_010.
* @tc.desc  : Test OnPreferredInputDeviceUpdated.
*/
HWTEST_F(AudioPolicyClientProxyUnitTest, AudioPolicyClientProxy_010, TestSize.Level1)
{
    AudioCapturerInfo capturerInfo;
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    sptr<IRemoteObject> impl = new RemoteObjectTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnPreferredInputDeviceUpdated(capturerInfo, desc);
    EXPECT_NE(audioPolicyClientProxy, nullptr);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_011.
* @tc.desc  : Test OnRendererStateChange.
*/
HWTEST_F(AudioPolicyClientProxyUnitTest, AudioPolicyClientProxy_011, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    std::shared_ptr<AudioRendererChangeInfo> changeInfo1 = std::make_shared<AudioRendererChangeInfo>();
    std::shared_ptr<AudioRendererChangeInfo> changeInfo2 = nullptr;
    audioRendererChangeInfos.push_back(changeInfo1);
    audioRendererChangeInfos.push_back(changeInfo2);
    sptr<IRemoteObject> impl = new RemoteObjectTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnRendererStateChange(audioRendererChangeInfos);
    EXPECT_NE(audioPolicyClientProxy, nullptr);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_012.
* @tc.desc  : Test OnCapturerStateChange.
*/
HWTEST_F(AudioPolicyClientProxyUnitTest, AudioPolicyClientProxy_012, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    std::shared_ptr<AudioCapturerChangeInfo> changeInfo1 = std::make_shared<AudioCapturerChangeInfo>();
    std::shared_ptr<AudioCapturerChangeInfo> changeInfo2 = nullptr;
    audioCapturerChangeInfos.push_back(changeInfo1);
    audioCapturerChangeInfos.push_back(changeInfo2);
    sptr<IRemoteObject> impl = new RemoteObjectTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnCapturerStateChange(audioCapturerChangeInfos);
    EXPECT_NE(audioPolicyClientProxy, nullptr);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_013.
* @tc.desc  : Test OnRendererDeviceChange.
*/
HWTEST_F(AudioPolicyClientProxyUnitTest, AudioPolicyClientProxy_013, TestSize.Level1)
{
    uint32_t sessionId = 0;
    AudioDeviceDescriptor deviceInfo;
    sptr<IRemoteObject> impl = new RemoteObjectTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnRendererDeviceChange(sessionId, deviceInfo, AudioStreamDeviceChangeReason::OVERRODE);
    EXPECT_NE(audioPolicyClientProxy, nullptr);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_014.
* @tc.desc  : Test OnRecreateRendererStreamEvent.
*/
HWTEST_F(AudioPolicyClientProxyUnitTest, AudioPolicyClientProxy_014, TestSize.Level1)
{
    uint32_t sessionId = 0;
    int32_t streamFlag = 0;
    sptr<IRemoteObject> impl = new RemoteObjectTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnRecreateRendererStreamEvent(sessionId, streamFlag,
        AudioStreamDeviceChangeReason::OVERRODE);
    EXPECT_NE(audioPolicyClientProxy, nullptr);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_015.
* @tc.desc  : Test OnRecreateCapturerStreamEvent.
*/
HWTEST_F(AudioPolicyClientProxyUnitTest, AudioPolicyClientProxy_015, TestSize.Level1)
{
    uint32_t sessionId = 0;
    int32_t streamFlag = 0;
    sptr<IRemoteObject> impl = new RemoteObjectTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnRecreateCapturerStreamEvent(sessionId, streamFlag,
        AudioStreamDeviceChangeReason::OVERRODE);
    EXPECT_NE(audioPolicyClientProxy, nullptr);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_016.
* @tc.desc  : Test OnHeadTrackingDeviceChange.
*/
HWTEST_F(AudioPolicyClientProxyUnitTest, AudioPolicyClientProxy_016, TestSize.Level1)
{
    std::unordered_map<std::string, bool> changeInfo;
    sptr<IRemoteObject> impl = new RemoteObjectTestStub();
    auto audioPolicyClientProxy = std::make_shared<AudioPolicyClientProxy>(impl);
    audioPolicyClientProxy->OnHeadTrackingDeviceChange(changeInfo);
    EXPECT_NE(audioPolicyClientProxy, nullptr);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_017.
* @tc.desc  : Test OnSpatializationEnabledChange.
*/
HWTEST_F(AudioPolicyClientProxyUnitTest, AudioPolicyClientProxy_017, TestSize.Level1)
{
    sptr<IRemoteObject> impl = new RemoteObjectTestStub();
    std::shared_ptr<IAudioPolicyClient> iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    iAudioPolicyClient->hasSystemPermission_ = false;
    iAudioPolicyClient->OnSpatializationEnabledChange(true);
    EXPECT_NE(iAudioPolicyClient, nullptr);

    iAudioPolicyClient->hasSystemPermission_ = true;
    iAudioPolicyClient->OnSpatializationEnabledChange(true);
    EXPECT_NE(iAudioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_018.
* @tc.desc  : Test OnSpatializationEnabledChangeForAnyDevice.
*/
HWTEST_F(AudioPolicyClientProxyUnitTest, AudioPolicyClientProxy_018, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    sptr<IRemoteObject> impl = new RemoteObjectTestStub();
    std::shared_ptr<IAudioPolicyClient> iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    iAudioPolicyClient->hasSystemPermission_ = false;
    iAudioPolicyClient->OnSpatializationEnabledChangeForAnyDevice(deviceDescriptor, true);
    EXPECT_NE(iAudioPolicyClient, nullptr);

    iAudioPolicyClient->hasSystemPermission_ = true;
    iAudioPolicyClient->OnSpatializationEnabledChangeForAnyDevice(deviceDescriptor, true);
    EXPECT_NE(iAudioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_019.
* @tc.desc  : Test OnSpatializationEnabledChangeForCurrentDevice.
*/
HWTEST_F(AudioPolicyClientProxyUnitTest, AudioPolicyClientProxy_019, TestSize.Level1)
{
    sptr<IRemoteObject> impl = new RemoteObjectTestStub();
    std::shared_ptr<IAudioPolicyClient> iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    iAudioPolicyClient->OnSpatializationEnabledChangeForCurrentDevice(true);
    EXPECT_NE(iAudioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_020.
* @tc.desc  : Test OnHeadTrackingEnabledChange.
*/
HWTEST_F(AudioPolicyClientProxyUnitTest, AudioPolicyClientProxy_020, TestSize.Level1)
{
    sptr<IRemoteObject> impl = new RemoteObjectTestStub();
    std::shared_ptr<IAudioPolicyClient> iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    iAudioPolicyClient->hasSystemPermission_ = false;
    iAudioPolicyClient->OnHeadTrackingEnabledChange(true);
    EXPECT_NE(iAudioPolicyClient, nullptr);

    iAudioPolicyClient->hasSystemPermission_ = true;
    iAudioPolicyClient->OnHeadTrackingEnabledChange(true);
    EXPECT_NE(iAudioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_021.
* @tc.desc  : Test OnAudioSceneChange.
*/
HWTEST_F(AudioPolicyClientProxyUnitTest, AudioPolicyClientProxy_021, TestSize.Level1)
{
    sptr<IRemoteObject> impl = new RemoteObjectTestStub();
    auto iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    iAudioPolicyClient->OnAudioSceneChange(AudioScene::AUDIO_SCENE_MAX);
    EXPECT_NE(iAudioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_022.
* @tc.desc  : Test OnHeadTrackingEnabledChangeForAnyDevice.
*/
HWTEST_F(AudioPolicyClientProxyUnitTest, AudioPolicyClientProxy_02, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    sptr<IRemoteObject> impl = new RemoteObjectTestStub();
    std::shared_ptr<IAudioPolicyClient> iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    iAudioPolicyClient->hasSystemPermission_ = false;
    iAudioPolicyClient->OnHeadTrackingEnabledChangeForAnyDevice(deviceDescriptor, true);
    EXPECT_NE(iAudioPolicyClient, nullptr);

    iAudioPolicyClient->hasSystemPermission_ = true;
    iAudioPolicyClient->OnHeadTrackingEnabledChangeForAnyDevice(deviceDescriptor, true);
    EXPECT_NE(iAudioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_023.
* @tc.desc  : Test OnNnStateChange.
*/
HWTEST_F(AudioPolicyClientProxyUnitTest, AudioPolicyClientProxy_023, TestSize.Level1)
{
    int32_t state = 0;
    sptr<IRemoteObject> impl = new RemoteObjectTestStub();
    auto iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    iAudioPolicyClient->OnNnStateChange(state);
    EXPECT_NE(iAudioPolicyClient, nullptr);
}

/**
* @tc.name  : Test AudioInputThread.
* @tc.number: AudioConcurrencyService_024.
* @tc.desc  : Test OnAudioSessionDeactive.
*/
HWTEST_F(AudioPolicyClientProxyUnitTest, AudioPolicyClientProxy_024, TestSize.Level1)
{
    AudioSessionDeactiveEvent deactiveEvent;
    sptr<IRemoteObject> impl = new RemoteObjectTestStub();
    auto iAudioPolicyClient = std::make_shared<AudioPolicyClientProxy>(impl);
    iAudioPolicyClient->OnAudioSessionDeactive(deactiveEvent);
    EXPECT_NE(iAudioPolicyClient, nullptr);
}
} // namespace AudioStandard
} // namespace OHOS