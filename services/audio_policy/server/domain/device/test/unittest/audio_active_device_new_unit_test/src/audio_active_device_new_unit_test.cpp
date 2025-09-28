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

#include "audio_policy_utils.h"
#include "audio_active_device_new_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
static const int32_t MEDIA_SERVICE_UID = 1013;
void AudioActiveDeviceNewUnitTest::SetUpTestCase(void) {}
void AudioActiveDeviceNewUnitTest::TearDownTestCase(void) {}
void AudioActiveDeviceNewUnitTest::SetUp(void) {}
void AudioActiveDeviceNewUnitTest::TearDown(void) {}

/**
* @tc.name  : Test GetActiveA2dpDeviceStreamInfo.
* @tc.number: AudioActiveDeviceNewUnitTest_GetActiveA2dpDeviceStreamInfo_001.
* @tc.desc  : Test GetActiveA2dpDeviceStreamInfo interface.
*/
HWTEST_F(AudioActiveDeviceNewUnitTest, GetActiveA2dpDeviceStreamInfo_001, TestSize.Level4)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    audioActiveDevice->activeBTInDevice_ = "AA:BB:CC:DD:EE:FF";
    auto& a2dpDevice = audioActiveDevice->audioA2dpDevice_;
    A2dpDeviceConfigInfo info;
    info.streamInfo.samplingRate = {SAMPLE_RATE_8000, SAMPLE_RATE_48000};
    info.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    info.streamInfo.channelLayout = {AudioChannelLayout::CH_LAYOUT_STEREO};
    info.absVolumeSupport = true;
    info.volumeLevel = 8;
    info.mute = false;
    a2dpDevice.connectedA2dpInDeviceMap_["AA:BB:CC:DD:EE:FF"] = info;
    AudioStreamInfo streamInfo;
    bool result = audioActiveDevice->GetActiveA2dpDeviceStreamInfo(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP_IN, streamInfo);

    EXPECT_EQ(result, true);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceNewUnitTest_GetMaxAmplitude_001.
* @tc.desc  : Test GetMaxAmplitude.
*/
HWTEST_F(AudioActiveDeviceNewUnitTest, GetMaxAmplitude_001, TestSize.Level4)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    audioActiveDevice->currentActiveDevice_ = AudioDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    audioActiveDevice->currentActiveDevice_.deviceId_ = 1;
    audioActiveDevice->currentActiveInputDevice_ = AudioDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE);
    audioActiveDevice->currentActiveInputDevice_.deviceId_ = 0;
    AudioInterrupt audioInterrupt;
    float result = audioActiveDevice->GetMaxAmplitude(0, audioInterrupt);
    EXPECT_EQ(result, 0);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceNewUnitTest_GetMaxAmplitude_002.
* @tc.desc  : Test GetMaxAmplitude.
*/
HWTEST_F(AudioActiveDeviceNewUnitTest, GetMaxAmplitude_002, TestSize.Level4)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    AudioInterrupt audioInterrupt;
    float result = audioActiveDevice->GetMaxAmplitude(0, audioInterrupt);
    EXPECT_NE(audioActiveDevice, nullptr);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceNewUnitTest_NotifyUserSelectionEventToBt_001.
* @tc.desc  : Test NotifyUserSelectionEventToBt.
*/
HWTEST_F(AudioActiveDeviceNewUnitTest, NotifyUserSelectionEventToBt_001, TestSize.Level4)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    EXPECT_NE(audioActiveDevice, nullptr);
    audioActiveDevice->currentActiveDevice_ = AudioDeviceDescriptor(DEVICE_TYPE_SPEAKER, INPUT_DEVICE);
    audioActiveDevice->currentActiveDevice_.deviceId_ = 1;
    auto nearLinkDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_NEARLINK, INPUT_DEVICE, 0, 0,
        "nearlink_net_001"
    );
    nearLinkDevice->macAddress_ = "AA:BB:CC:DD:EE:FF";
    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    audioActiveDevice->NotifyUserSelectionEventToBt(nearLinkDevice, streamUsage);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceNewUnitTest_NotifyUserSelectionEventForInput_001.
* @tc.desc  : Test NotifyUserSelectionEventForInput.
*/
HWTEST_F(AudioActiveDeviceNewUnitTest, NotifyUserSelectionEventForInput_001, TestSize.Level4)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    EXPECT_NE(audioActiveDevice, nullptr);
    audioActiveDevice->currentActiveInputDevice_ = AudioDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE);
    audioActiveDevice->currentActiveInputDevice_.deviceId_ = 1;
    auto scoDevice = std::make_shared<AudioDeviceDescriptor>(
        DEVICE_TYPE_BLUETOOTH_SCO,
        INPUT_DEVICE,
        0, 0, "bt_network_001"
    );
    scoDevice->macAddress_ = "AA:BB:CC:DD:EE:01";
    SourceType sourceType = SOURCE_TYPE_MIC;
    audioActiveDevice->NotifyUserSelectionEventForInput(scoDevice, sourceType);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceNewUnitTest_NotifyUserSelectionEventForInput_002.
* @tc.desc  : Test NotifyUserSelectionEventForInput.
*/
HWTEST_F(AudioActiveDeviceNewUnitTest, NotifyUserSelectionEventForInput_002, TestSize.Level4)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    EXPECT_NE(audioActiveDevice, nullptr);
    audioActiveDevice->currentActiveInputDevice_ = AudioDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE);
    audioActiveDevice->currentActiveInputDevice_.deviceId_ = 1;
    auto scoDevice = std::make_shared<AudioDeviceDescriptor>(
        DEVICE_TYPE_BLUETOOTH_A2DP_IN,
        INPUT_DEVICE,
        0, 0, "bt_network_001"
    );
    scoDevice->macAddress_ = "AA:BB:CC:DD:EE:01";
    SourceType sourceType = SOURCE_TYPE_MIC;
    audioActiveDevice->NotifyUserSelectionEventForInput(scoDevice, sourceType);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceNewUnitTest_NotifyUserSelectionEventForInput_003.
* @tc.desc  : Test NotifyUserSelectionEventForInput.
*/
HWTEST_F(AudioActiveDeviceNewUnitTest, NotifyUserSelectionEventForInput_003, TestSize.Level4)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    EXPECT_NE(audioActiveDevice, nullptr);
    audioActiveDevice->currentActiveInputDevice_ = AudioDeviceDescriptor(DEVICE_TYPE_MIC, INPUT_DEVICE);
    audioActiveDevice->currentActiveInputDevice_.deviceId_ = 1;
    auto scoDevice = std::make_shared<AudioDeviceDescriptor>(
        DEVICE_TYPE_NEARLINK_IN,
        INPUT_DEVICE,
        0, 0, "bt_network_001"
    );
    scoDevice->macAddress_ = "AA:BB:CC:DD:EE:01";
    SourceType sourceType = SOURCE_TYPE_MIC;
    audioActiveDevice->NotifyUserSelectionEventForInput(scoDevice, sourceType);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceNewUnitTest_UpdateDevice_001.
* @tc.desc  : Test UpdateDevice.
*/
HWTEST_F(AudioActiveDeviceNewUnitTest, UpdateDevice_001, TestSize.Level4)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    audioActiveDevice->currentActiveDevice_ = AudioDeviceDescriptor(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    audioActiveDevice->currentActiveDevice_.deviceId_ = 1;
    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    desc->deviceId_ = 2;
    desc->macAddress_ = "AA:BB:CC:DD:EE:01";
    desc->networkId_ = "bt_net_001";
    auto preferredDesc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_WIRED_HEADSET, OUTPUT_DEVICE);
    preferredDesc->deviceId_ = 3;
    preferredDesc->macAddress_ = "FF:EE:DD:CC:BB:01";
    preferredDesc->networkId_ = "wired_net_001";
    auto& affinityManager = audioActiveDevice->audioAffinityManager_;
    affinityManager.activeRendererDeviceMap_[1001] = preferredDesc;
    AudioStreamDeviceChangeReasonExt reason(AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE);
    auto rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->clientUID = 1001;
    bool result = audioActiveDevice->UpdateDevice(desc, reason, rendererChangeInfo);
    EXPECT_TRUE(result);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceNewUnitTest_HandleNegtiveBt_001.
* @tc.desc  : Test HandleNegtiveBt.
*/
HWTEST_F(AudioActiveDeviceNewUnitTest, HandleNegtiveBt_001, TestSize.Level4)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    EXPECT_NE(audioActiveDevice, nullptr);
    AudioDeviceDescriptor deviceDescriptor;

    deviceDescriptor.deviceType_ = DEVICE_TYPE_NEARLINK;
    audioActiveDevice->SetCurrentOutputDevice(deviceDescriptor);
    audioActiveDevice->HandleNegtiveBt(DeviceType::DEVICE_TYPE_NEARLINK);
 
    deviceDescriptor.deviceType_ = DEVICE_TYPE_NEARLINK;
    audioActiveDevice->SetCurrentOutputDevice(deviceDescriptor);
    audioActiveDevice->HandleNegtiveBt(DeviceType::DEVICE_TYPE_BLUETOOTH_SCO);

    deviceDescriptor.deviceType_ = DEVICE_TYPE_SPEAKER;
    audioActiveDevice->SetCurrentOutputDevice(deviceDescriptor);
    audioActiveDevice->HandleNegtiveBt(DeviceType::DEVICE_TYPE_NEARLINK);
    
    deviceDescriptor.deviceType_ = DEVICE_TYPE_EARPIECE;
    audioActiveDevice->SetCurrentOutputDevice(deviceDescriptor);
    audioActiveDevice->HandleNegtiveBt(DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceNewUnitTest_HandleNegtiveBt_001.
* @tc.desc  : Test SetDeviceActive.
*/
HWTEST_F(AudioActiveDeviceNewUnitTest, SetDeviceActive_001, TestSize.Level4)
{
    auto fakeDevice = std::make_shared<AudioDeviceDescriptor>();
    fakeDevice->deviceType_ = DEVICE_TYPE_SPEAKER;
    fakeDevice->deviceRole_ = OUTPUT_DEVICE;
    fakeDevice->networkId_ = "local_network";
    fakeDevice->deviceName_ = "TestSpeaker";
    auto& deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    deviceManager.AddNewDevice(fakeDevice);

    auto callDevices = AudioPolicyUtils::GetInstance().GetAvailableDevicesInner(CALL_OUTPUT_DEVICES);
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    int32_t result = audioActiveDevice->SetDeviceActive(
        DEVICE_TYPE_SPEAKER,
        false,
        1001
    );

    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceNewUnitTest_SetCallDeviceActive_001.
* @tc.desc  : Test SetCallDeviceActive.
*/
HWTEST_F(AudioActiveDeviceNewUnitTest, SetCallDeviceActive_001, TestSize.Level4)
{
    auto fakeDevice = std::make_shared<AudioDeviceDescriptor>();
    fakeDevice->deviceType_ = DEVICE_TYPE_SPEAKER;
    fakeDevice->deviceRole_ = OUTPUT_DEVICE;
    fakeDevice->macAddress_ = "";

    auto& deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    deviceManager.AddNewDevice(fakeDevice);
    auto callDevices = AudioPolicyUtils::GetInstance().GetAvailableDevicesInner(CALL_OUTPUT_DEVICES);
    EXPECT_GT(callDevices.size(), 0);

    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    int32_t result = audioActiveDevice->SetCallDeviceActive(
        DEVICE_TYPE_SPEAKER,
        true,
        ""
    );
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceNewUnitTest_SetCallDeviceActive_002.
* @tc.desc  : Test SetCallDeviceActive.
*/
HWTEST_F(AudioActiveDeviceNewUnitTest, SetCallDeviceActive_002, TestSize.Level4)
{
    auto fakeDevice = std::make_shared<AudioDeviceDescriptor>();
    fakeDevice->deviceType_ = DEVICE_TYPE_SPEAKER;
    fakeDevice->deviceRole_ = OUTPUT_DEVICE;
    fakeDevice->macAddress_ = "";

    auto& deviceManager = AudioDeviceManager::GetAudioDeviceManager();
    deviceManager.AddNewDevice(fakeDevice);
    auto callDevices = AudioPolicyUtils::GetInstance().GetAvailableDevicesInner(CALL_OUTPUT_DEVICES);
    EXPECT_GT(callDevices.size(), 0);

    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    int32_t result = audioActiveDevice->SetCallDeviceActive(
        DEVICE_TYPE_SPEAKER,
        false,
        ""
    );
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test SortDevicesByPriority.
* @tc.number: SortDevicesByPriority.
* @tc.desc  : Test SortDevicesByPriority.
*/
HWTEST_F(AudioActiveDeviceNewUnitTest, SortDevicesByPriority, TestSize.Level4)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    descs.push_back(nullptr);
    descs.push_back(nullptr);

    audioActiveDevice->SortDevicesByPriority(descs);
    EXPECT_EQ(descs.size(), 2);

    descs[0] = std::make_shared<AudioDeviceDescriptor>();
    descs[0]->deviceType_ = DEVICE_TYPE_SPEAKER;
    descs[0]->deviceRole_ = OUTPUT_DEVICE;
    descs[0]->macAddress_ = "";

    audioActiveDevice->SortDevicesByPriority(descs);
    EXPECT_EQ(descs.size(), 2);

    descs[0] = nullptr;
    descs[1] = std::make_shared<AudioDeviceDescriptor>();
    descs[1]->deviceType_ = DEVICE_TYPE_EARPIECE;
    descs[1]->deviceRole_ = OUTPUT_DEVICE;
    descs[1]->macAddress_ = "";

    audioActiveDevice->SortDevicesByPriority(descs);
    EXPECT_EQ(descs.size(), 2);

    descs[0] = std::make_shared<AudioDeviceDescriptor>();
    descs[0]->deviceType_ = DEVICE_TYPE_SPEAKER;
    descs[0]->deviceRole_ = OUTPUT_DEVICE;
    descs[0]->macAddress_ = "";

    audioActiveDevice->SortDevicesByPriority(descs);
    EXPECT_EQ(descs.size(), 2);
}

/**
* @tc.name  : Test GetDeviceForVolume.
* @tc.number: GetDeviceForVolume.
* @tc.desc  : Test GetDeviceForVolume.
*/
HWTEST_F(AudioActiveDeviceNewUnitTest, GetDeviceForVolume_001, TestSize.Level4)
{
    auto tmp = std::make_shared<AudioActiveDevice>();
    EXPECT_NE(tmp->GetDeviceForVolume(STREAM_ALL), nullptr);
    EXPECT_NE(tmp->GetDeviceForVolume(STREAM_MUSIC), nullptr);
    EXPECT_NE(tmp->GetDeviceForVolume(STREAM_RING), nullptr);
    tmp->volumeTypeDeviceMap_[STREAM_MUSIC] = {};
    EXPECT_NE(tmp->GetDeviceForVolume(STREAM_MUSIC), nullptr);
    tmp->volumeTypeDeviceMap_[STREAM_MUSIC].push_back(std::make_shared<AudioDeviceDescriptor>());
    EXPECT_NE(tmp->GetDeviceForVolume(STREAM_MUSIC), nullptr);
}

/**
* @tc.name  : Test GetDeviceForVolume.
* @tc.number: GetDeviceForVolume.
* @tc.desc  : Test GetDeviceForVolume.
*/
HWTEST_F(AudioActiveDeviceNewUnitTest, GetDeviceForVolume_002, TestSize.Level4)
{
    auto tmp = std::make_shared<AudioActiveDevice>();
    EXPECT_NE(tmp->GetDeviceForVolume(STREAM_USAGE_MUSIC), nullptr);
    tmp->streamUsageDeviceMap_[STREAM_USAGE_MUSIC] = {};
    EXPECT_NE(tmp->GetDeviceForVolume(STREAM_USAGE_MUSIC), nullptr);
    tmp->streamUsageDeviceMap_[STREAM_USAGE_MUSIC].push_back(std::make_shared<AudioDeviceDescriptor>());
    EXPECT_NE(tmp->GetDeviceForVolume(STREAM_USAGE_MUSIC), nullptr);
}

/**
* @tc.name  : Test GetDeviceForVolume.
* @tc.number: GetDeviceForVolume.
* @tc.desc  : Test GetDeviceForVolume.
*/
HWTEST_F(AudioActiveDeviceNewUnitTest, IsAvailableFrontDeviceInVector, TestSize.Level4)
{
    auto tmp = std::make_shared<AudioActiveDevice>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    EXPECT_NE(tmp->IsAvailableFrontDeviceInVector(descs), false);
    descs.push_back(nullptr);
    EXPECT_NE(tmp->IsAvailableFrontDeviceInVector(descs), false);
    descs[0] = std::make_shared<AudioDeviceDescriptor>();
    EXPECT_NE(tmp->IsAvailableFrontDeviceInVector(descs), true);
}

/**
* @tc.name  : Test GetDeviceForVolume.
* @tc.number: GetDeviceForVolume.
* @tc.desc  : Test GetDeviceForVolume.
*/
HWTEST_F(AudioActiveDeviceNewUnitTest, GetRealUid, TestSize.Level4)
{
    auto tmp = std::make_shared<AudioActiveDevice>();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->callerUid_ = 0;
    EXPECT_NE(tmp->GetRealUid(streamDesc), 0);
    streamDesc->callerUid_ = MEDIA_SERVICE_UID;
    streamDesc->appInfo_.appUid = 0;
    EXPECT_NE(tmp->GetRealUid(streamDesc), 0);
}

} // namespace AudioStandard
} // namespace OHOS
