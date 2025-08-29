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
#include "audio_adapter_manager_unit_test.h"
#include "audio_stream_descriptor.h"
#include "audio_interrupt_service.h"
#include "audio_adapter_manager_handler.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

static AudioAdapterManager *audioAdapterManager_;

void AudioAdapterManagerUnitTest::SetUpTestCase(void) {}
void AudioAdapterManagerUnitTest::TearDownTestCase(void) {}

std::shared_ptr<AudioInterruptService> GetTnterruptServiceTest()
{
    return std::make_shared<AudioInterruptService>();
}
/**
 * @tc.name: IsAppVolumeMute_001
 * @tc.desc: Test IsAppVolumeMute when owned is true.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, IsAppVolumeMute_001, TestSize.Level1)
{
    int32_t appUid = 12345;
    bool owned = true;
    bool isMute = true;
    bool result = AudioAdapterManager::GetInstance().IsAppVolumeMute(appUid, owned, isMute);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: IsAppVolumeMute_002
 * @tc.desc: Test IsAppVolumeMute when owned is false.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, IsAppVolumeMute_002, TestSize.Level1)
{
    int32_t appUid = 12345;
    bool owned = false;
    bool isMute = true;
    bool result = AudioAdapterManager::GetInstance().IsAppVolumeMute(appUid, owned, isMute);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: SaveSpecifiedDeviceVolume_001
 * @tc.desc: Test SaveSpecifiedDeviceVolume when deviceType is different from currentActiveDevice.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, SaveSpecifiedDeviceVolume_001, TestSize.Level1)
{
    audioAdapterManager_->Init();
    audioAdapterManager_->currentActiveDevice_.deviceType_ = DEVICE_TYPE_SPEAKER;
    AudioStreamType streamType = STREAM_MUSIC;
    int32_t volumeLevel = 5;
    DeviceType deviceType = DEVICE_TYPE_WIRED_HEADSET;
    int32_t minVolume = audioAdapterManager_->GetMinVolumeLevel(streamType);
    int32_t maxVolume = audioAdapterManager_->GetMaxVolumeLevel(streamType);
    ASSERT_TRUE(volumeLevel >= minVolume && volumeLevel <= maxVolume);
    int32_t result = audioAdapterManager_->SaveSpecifiedDeviceVolume(streamType, volumeLevel, deviceType);
    ASSERT_EQ(result, 0);
}

/**
 * @tc.name: SaveSpecifiedDeviceVolume_002
 * @tc.desc: Test SaveSpecifiedDeviceVolume when deviceType is same as currentActiveDevice.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, SaveSpecifiedDeviceVolume_002, TestSize.Level1)
{
    audioAdapterManager_->currentActiveDevice_.deviceType_ = DEVICE_TYPE_WIRED_HEADSET;
    AudioStreamType streamType = STREAM_MUSIC;
    int32_t volumeLevel = 5;
    DeviceType deviceType = DEVICE_TYPE_WIRED_HEADSET;
    int32_t minVolume = audioAdapterManager_->GetMinVolumeLevel(streamType);
    int32_t maxVolume = audioAdapterManager_->GetMaxVolumeLevel(streamType);
    ASSERT_TRUE(volumeLevel >= minVolume && volumeLevel <= maxVolume);
    int32_t result = audioAdapterManager_->SaveSpecifiedDeviceVolume(streamType, volumeLevel, deviceType);
    ASSERT_EQ(result, 0);
}

/**
 * @tc.name: HandleStreamMuteStatus_001
 * @tc.desc: Test HandleStreamMuteStatus when deviceType is not DEVICE_TYPE_NONE.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, HandleStreamMuteStatus_001, TestSize.Level1)
{
    AudioStreamType streamType = STREAM_MUSIC;
    bool mute = true;
    StreamUsage streamUsage = STREAM_USAGE_UNKNOWN;
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioAdapterManager::GetInstance().HandleStreamMuteStatus(streamType, mute, streamUsage, deviceType);
    EXPECT_TRUE(mute);
}

/**
 * @tc.name: HandleStreamMuteStatus_002
 * @tc.desc: Test HandleStreamMuteStatus when deviceType is DEVICE_TYPE_NONE.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, HandleStreamMuteStatus_002, TestSize.Level1)
{
    AudioStreamType streamType = STREAM_MUSIC;
    bool mute = true;
    StreamUsage streamUsage = STREAM_USAGE_UNKNOWN;
    DeviceType deviceType = DEVICE_TYPE_NONE;
    AudioAdapterManager::GetInstance().HandleStreamMuteStatus(streamType, mute, streamUsage, deviceType);
    EXPECT_TRUE(mute);
}

/**
 * @tc.name: IsHandleStreamMute_001
 * @tc.desc: Test IsHandleStreamMute when streamType is STREAM_VOICE_CALL.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, IsHandleStreamMute_001, TestSize.Level1)
{
    AudioStreamType streamType = STREAM_VOICE_CALL;
    bool mute = true;
    StreamUsage streamUsage = STREAM_USAGE_UNKNOWN;
    int32_t SUCCESS = 0;
    int32_t result = audioAdapterManager_->IsHandleStreamMute(streamType, mute, streamUsage);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: IsHandleStreamMute_002
 * @tc.desc: Test IsHandleStreamMute when streamType is STREAM_VOICE_CALL.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, IsHandleStreamMute_002, TestSize.Level1)
{
    AudioStreamType streamType = STREAM_VOICE_CALL;
    bool mute = false;
    StreamUsage streamUsage = STREAM_USAGE_UNKNOWN;
    int32_t result = audioAdapterManager_->IsHandleStreamMute(streamType, mute, streamUsage);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name: SetOffloadVolume_001
 * @tc.desc: Test SetOffloadVolume.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetOffloadVolume_001, TestSize.Level1)
{
    audioAdapterManager_->Init();
    AudioStreamType streamType = STREAM_MUSIC;
    float volumeDb = 1;
    audioAdapterManager_->SetOffloadVolume(streamType, volumeDb, "offload");

    streamType = STREAM_SPEECH;
    audioAdapterManager_->SetOffloadVolume(streamType, volumeDb, "offload");

    streamType = STREAM_SYSTEM;
    audioAdapterManager_->currentActiveDevice_.deviceType_ = DEVICE_TYPE_DP;
    audioAdapterManager_->SetOffloadVolume(streamType, volumeDb, "offload");

    streamType = STREAM_SYSTEM;
    audioAdapterManager_->currentActiveDevice_.deviceType_ = DEVICE_TYPE_SPEAKER;
    auto interruptServiceTest = GetTnterruptServiceTest();
    audioAdapterManager_->audioServerProxy_ = interruptServiceTest->GetAudioServerProxy();
    ASSERT_NE(audioAdapterManager_->audioServerProxy_, nullptr);
    audioAdapterManager_->SetOffloadVolume(streamType, volumeDb, "offload");
}

/**
 * @tc.name: SetOffloadSessionId_001
 * @tc.desc: Test SetOffloadSessionId.
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetOffloadSessionId_001, TestSize.Level1)
{
    uint32_t sessionId = MIN_STREAMID - 1;
    AudioAdapterManager::GetInstance().SetOffloadSessionId(sessionId);

    sessionId = MAX_STREAMID + 1;
    AudioAdapterManager::GetInstance().SetOffloadSessionId(sessionId);

    sessionId = MIN_STREAMID + 1;
    AudioAdapterManager::GetInstance().SetOffloadSessionId(sessionId);
}

/**
 * @tc.name: SetVolumeForSwitchDevice_001
 * @tc.desc: Test SetVolumeForSwitchDevice
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetVolumeForSwitchDevice_001, TestSize.Level1)
{
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = DEVICE_TYPE_SPEAKER;
    deviceDescriptor.networkId_ = "LocalDevice";

    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->SetVolumeForSwitchDevice(deviceDescriptor);
    EXPECT_EQ(audioAdapterManager->currentActiveDevice_.deviceType_, DEVICE_TYPE_SPEAKER);

    deviceDescriptor.networkId_ = "RemoteDevice";
    audioAdapterManager->SetVolumeForSwitchDevice(deviceDescriptor);
    EXPECT_EQ(audioAdapterManager->currentActiveDevice_.deviceType_, DEVICE_TYPE_SPEAKER);

    deviceDescriptor.networkId_ = "LocalDevice";
    audioAdapterManager->SetVolumeForSwitchDevice(deviceDescriptor);
    EXPECT_EQ(audioAdapterManager->currentActiveDevice_.deviceType_, DEVICE_TYPE_SPEAKER);

    deviceDescriptor.deviceType_ = DEVICE_TYPE_DP;
    audioAdapterManager->SetVolumeForSwitchDevice(deviceDescriptor);
    EXPECT_EQ(audioAdapterManager->currentActiveDevice_.deviceType_, DEVICE_TYPE_DP);

    deviceDescriptor.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioAdapterManager->SetVolumeForSwitchDevice(deviceDescriptor);
    EXPECT_EQ(audioAdapterManager->currentActiveDevice_.deviceType_, DEVICE_TYPE_BLUETOOTH_A2DP);

    deviceDescriptor.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    audioAdapterManager->SetVolumeForSwitchDevice(deviceDescriptor);
    EXPECT_EQ(audioAdapterManager->currentActiveDevice_.deviceType_, DEVICE_TYPE_BLUETOOTH_SCO);
}

/**
 * @tc.name: SetAdjustVolumeForZone_001
 * @tc.desc: Test SetAdjustVolumeForZone
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetAdjustVolumeForZone_001, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    auto ret = audioAdapterManager->SetAdjustVolumeForZone(0);
    EXPECT_EQ(ret, SUCCESS);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    std::shared_ptr<AudioDeviceDescriptor> desc =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_REMOTE_CAST, OUTPUT_DEVICE);
    desc->networkId_ = "LocalDevice";
    devices.push_back(desc);
    AudioZoneService::GetInstance().BindDeviceToAudioZone(zoneId1_, devices);
    AudioConnectedDevice::GetInstance().AddConnectedDevice(desc);
    AudioZoneService::GetInstance().UpdateDeviceFromGlobalForAllZone(desc);

    ret = audioAdapterManager->SetAdjustVolumeForZone(zoneId2_);
    EXPECT_NE(ret, SUCCESS);

    audioAdapterManager->volumeDataExtMaintainer_[desc->GetKey()] = std::make_shared<VolumeDataMaintainer>();
    ret = audioAdapterManager->SetAdjustVolumeForZone(zoneId2_);
    EXPECT_NE(ret, SUCCESS);

    audioAdapterManager->volumeDataExtMaintainer_.clear();
    desc->networkId_ = "RemoteDevice";
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;

    audioAdapterManager->volumeDataExtMaintainer_[desc->GetKey()] = std::make_shared<VolumeDataMaintainer>();
    ret = audioAdapterManager->SetAdjustVolumeForZone(zoneId2_);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name: SetSystemVolumeLevel_001
 * @tc.desc: Test CheckAndUpdateRemoteDeviceVolume
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetSystemVolumeLevel_001, TestSize.Level1)
{
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = DEVICE_TYPE_SPEAKER;
    deviceDescriptor.networkId_ = "LocalDevice";
    int32_t testVolumeLevel = 10;
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->SetActiveDeviceDescriptor(deviceDescriptor);
    audioAdapterManager->SetSystemVolumeLevel(STREAM_MUSIC, testVolumeLevel);
    EXPECT_EQ(audioAdapterManager->volumeDataMaintainer_.GetStreamVolume(STREAM_MUSIC), testVolumeLevel);

    deviceDescriptor.networkId_ = "RemoteDevice";
    testVolumeLevel = 5;
    audioAdapterManager->SetActiveDeviceDescriptor(deviceDescriptor);
    audioAdapterManager->SetSystemVolumeLevel(STREAM_MUSIC, testVolumeLevel);
    EXPECT_EQ(audioAdapterManager->volumeDataMaintainer_.GetStreamVolume(STREAM_MUSIC), testVolumeLevel);
}

/**
 * @tc.name: SetAbsVolumeMute_001
 * @tc.desc: Test SetAbsVolumeMute
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetAbsVolumeMute_001, TestSize.Level1)
{
    audioAdapterManager_->currentActiveDevice_.deviceType_ = DEVICE_TYPE_NEARLINK;
    bool mute = true;

    audioAdapterManager_->SetAbsVolumeMute(mute);
    int32_t ret = audioAdapterManager_->SetVolumeDb(STREAM_MUSIC);

    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name: UpdateSinkArgs_001
 * @tc.desc: Test UpdateSinkArgs all args have value
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerUnitTest, UpdateSinkArgs_001, TestSize.Level1)
{
    AudioModuleInfo info;
    info.name = "hello";
    info.adapterName = "world";
    info.className = "CALSS";
    info.fileName = "sink.so";
    info.sinkLatency = "300ms";
    info.networkId = "ASD**G124";
    info.deviceType = "AE00";
    info.extra = "1:13:2";
    info.needEmptyChunk = true;
    std::string ret {};
    AudioAdapterManager::UpdateSinkArgs(info, ret);
    EXPECT_EQ(ret,
    " sink_name=hello"
    " adapter_name=world"
    " device_class=CALSS"
    " file_path=sink.so"
    " sink_latency=300ms"
    " network_id=ASD**G124"
    " device_type=AE00"
    " split_mode=1:13:2"
    " need_empty_chunk=1");
}

/**
 * @tc.name: UpdateSinkArgs_002
 * @tc.desc: Test UpdateSinkArgs no value: network_id
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerUnitTest, UpdateSinkArgs_002, TestSize.Level1)
{
    AudioModuleInfo info;
    std::string ret {};
    AudioAdapterManager::UpdateSinkArgs(info, ret);
    EXPECT_EQ(ret, " network_id=LocalDevice");
}

/**
 * @tc.name: Test AudioAdapterManager
 * @tc.desc: HandleHearingAidVolume_001
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerUnitTest, HandleHearingAidVolume_001, TestSize.Level1)
{
    audioAdapterManager_->currentActiveDevice_.deviceType_ = DEVICE_TYPE_HEARING_AID;
    AudioStreamType streamType = STREAM_MUSIC;
    int defaultVolume =
        static_cast<int>(std::ceil(audioAdapterManager_->GetMaxVolumeLevel(STREAM_MUSIC) * 0.8));
    audioAdapterManager_->HandleHearingAidVolume(streamType);
    EXPECT_EQ(audioAdapterManager_->volumeDataMaintainer_.GetStreamVolume(STREAM_MUSIC), defaultVolume);
}

/**
 * @tc.name: Test SetInnerStreamMute
 * @tc.desc: SetInnerStreamMute_001
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetInnerStreamMute_001, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    AudioStreamType streamType = STREAM_MUSIC;
    bool mute = true;
    StreamUsage streamUsage = STREAM_USAGE_MUSIC;
    audioAdapterManager->SetInnerStreamMute(streamType, mute, streamUsage);
    EXPECT_EQ(audioAdapterManager->GetStreamMute(streamType), mute);
}

/**
 * @tc.name: Test SetInnerStreamMute
 * @tc.desc: SetInnerStreamMute_002
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetInnerStreamMute_002, TestSize.Level4)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->currentActiveDevice_.deviceType_ = DEVICE_TYPE_NEARLINK;
    AudioStreamType streamType = STREAM_MUSIC;
    bool mute = true;
    StreamUsage streamUsage = STREAM_USAGE_MUSIC;
    audioAdapterManager->SetInnerStreamMute(streamType, mute, streamUsage);
    EXPECT_EQ(audioAdapterManager->GetStreamMute(streamType), mute);
}

/**
 * @tc.name: Test SetSystemVolumeDegree
 * @tc.desc: SetSystemVolumeDegree_001
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetSystemVolumeDegree_001, TestSize.Level4)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    ASSERT_NE(audioAdapterManager, nullptr);
    AudioStreamType streamType = STREAM_MUSIC;
    int32_t volumeDegree = 44;
    auto ret = audioAdapterManager->SetSystemVolumeDegree(streamType, volumeDegree);
    EXPECT_EQ(ret, SUCCESS);

    ret = audioAdapterManager->SetSystemVolumeDegree(streamType, volumeDegree);
    EXPECT_EQ(ret, SUCCESS);

    ret = audioAdapterManager->GetSystemVolumeDegree(streamType);
    EXPECT_EQ(ret, volumeDegree);

    ret = audioAdapterManager->GetMinVolumeDegree(streamType);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: Test SetSleVoliceStatusFlag
 * @tc.desc: SetSleVoliceStatusFlag_001
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetSleVoliceStatusFlag_001, TestSize.Level4)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->currentActiveDevice_.deviceType_ = DEVICE_TYPE_NEARLINK;
    AudioStreamType streamType = STREAM_MUSIC;
    audioAdapterManager->SetSleVoiceStatusFlag(false);
    int32_t ret = audioAdapterManager->SetVolumeDb(streamType);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name: Test SetSleVoliceStatusFlag
 * @tc.desc: SetSleVoliceStatusFlag_002
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetSleVoliceStatusFlag_002, TestSize.Level4)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->currentActiveDevice_.deviceType_ = DEVICE_TYPE_NEARLINK;
    AudioStreamType streamType = STREAM_VOICE_CALL;
    audioAdapterManager->SetSleVoiceStatusFlag(false);
    int32_t ret = audioAdapterManager->SetVolumeDb(streamType);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name: Test SetSleVoliceStatusFlag
 * @tc.desc: SetSleVoliceStatusFlag_003
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetSleVoliceStatusFlag_003, TestSize.Level4)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->currentActiveDevice_.deviceType_ = DEVICE_TYPE_NEARLINK;
    AudioStreamType streamType = STREAM_MUSIC;
    audioAdapterManager->SetSleVoiceStatusFlag(true);
    int32_t ret = audioAdapterManager->SetVolumeDb(streamType);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name: Test SetSleVoliceStatusFlag
 * @tc.desc: SetSleVoliceStatusFlag_004
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerUnitTest, SetSleVoliceStatusFlag_004, TestSize.Level4)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->currentActiveDevice_.deviceType_ = DEVICE_TYPE_NEARLINK;
    AudioStreamType streamType = STREAM_VOICE_CALL;
    audioAdapterManager->SetSleVoiceStatusFlag(true);
    int32_t ret = audioAdapterManager->SetVolumeDb(streamType);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name: Test GetMaxVolumeLevel
 * @tc.number: GetMaxVolumeLevel_001
 * @tc.type: FUNC
 * @tc.desc: the volumeType is STREAM_APP, return appConfigVolume_.maxVolume.
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetMaxVolumeLevel_001, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    int32_t ret = audioAdapterManager->GetMaxVolumeLevel(STREAM_APP, DEVICE_TYPE_NONE);
    EXPECT_EQ(ret, audioAdapterManager->appConfigVolume_.maxVolume);
}

/**
 * @tc.name: Test GetMaxVolumeLevel
 * @tc.number: GetMaxVolumeLevel_002
 * @tc.type: FUNC
 * @tc.desc: the device maxLevel is valid, return the device maxLevel.
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetMaxVolumeLevel_002, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    AudioVolumeType volumeType = STREAM_VOICE_CALL;
    DeviceVolumeType deviceType = SPEAKER_VOLUME_TYPE;
    if (audioAdapterManager->streamVolumeInfos_.end() != audioAdapterManager->streamVolumeInfos_.find(volumeType)) {
        if ((audioAdapterManager->streamVolumeInfos_[volumeType] != nullptr) &&
            (audioAdapterManager->streamVolumeInfos_[volumeType]->deviceVolumeInfos.end() !=
            audioAdapterManager->streamVolumeInfos_[volumeType]->deviceVolumeInfos.find(deviceType)) &&
            (audioAdapterManager->streamVolumeInfos_[volumeType]->deviceVolumeInfos[deviceType] != nullptr)) {
            audioAdapterManager->streamVolumeInfos_[volumeType]->deviceVolumeInfos[deviceType]->maxLevel = 10;
        }
    }

    int32_t ret = audioAdapterManager->GetMaxVolumeLevel(volumeType, DEVICE_TYPE_SPEAKER);
    EXPECT_NE(ret, 10);
}

/**
 * @tc.name: Test GetMaxVolumeLevel
 * @tc.number: GetMaxVolumeLevel_003
 * @tc.type: FUNC
 * @tc.desc: the device maxLevel is not valid, return maxVolumeIndexMap_[volumeType].
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetMaxVolumeLevel_003, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    int32_t ret = audioAdapterManager->GetMaxVolumeLevel(STREAM_ALARM, DEVICE_TYPE_NONE);
    EXPECT_NE(ret, audioAdapterManager->maxVolumeIndexMap_[STREAM_ALARM]);
}

/**
 * @tc.name: Test GetMaxVolumeLevel
 * @tc.number: GetMaxVolumeLevel_004
 * @tc.type: FUNC
 * @tc.desc: the volume Type is not valid, return maxVolumeIndexMap_[STREAM_MUSIC].
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetMaxVolumeLevel_004, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    int32_t ret = audioAdapterManager->GetMaxVolumeLevel(STREAM_DEFAULT, DEVICE_TYPE_NONE);
    EXPECT_NE(ret, audioAdapterManager->maxVolumeIndexMap_[STREAM_MUSIC]);
}

/**
 * @tc.name: Test GetMinVolumeLevel
 * @tc.number: GetMinVolumeLevel_001
 * @tc.type: FUNC
 * @tc.desc: the volumeType is STREAM_APP, return appConfigVolume_.minVolume.
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetMinVolumeLevel_001, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    int32_t ret = audioAdapterManager->GetMinVolumeLevel(STREAM_APP, DEVICE_TYPE_NONE);
    EXPECT_EQ(ret, audioAdapterManager->appConfigVolume_.minVolume);
}

/**
 * @tc.name: Test GetMinVolumeLevel
 * @tc.number: GetMinVolumeLevel_002
 * @tc.type: FUNC
 * @tc.desc: the device maxLevel is valid, return the device maxLevel.
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetMinVolumeLevel_002, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    AudioVolumeType volumeType = STREAM_VOICE_CALL;
    DeviceVolumeType deviceType = SPEAKER_VOLUME_TYPE;
    if (audioAdapterManager->streamVolumeInfos_.end() != audioAdapterManager->streamVolumeInfos_.find(volumeType)) {
        if ((audioAdapterManager->streamVolumeInfos_[volumeType] != nullptr) &&
            (audioAdapterManager->streamVolumeInfos_[volumeType]->deviceVolumeInfos.end() !=
            audioAdapterManager->streamVolumeInfos_[volumeType]->deviceVolumeInfos.find(deviceType)) &&
            (audioAdapterManager->streamVolumeInfos_[volumeType]->deviceVolumeInfos[deviceType] != nullptr)) {
            audioAdapterManager->streamVolumeInfos_[volumeType]->deviceVolumeInfos[deviceType]->minLevel = 2;
        }
    }

    int32_t ret = audioAdapterManager->GetMinVolumeLevel(volumeType, DEVICE_TYPE_SPEAKER);
    EXPECT_NE(ret, 2);
}

/**
 * @tc.name: Test GetMinVolumeLevel
 * @tc.number: GetMinVolumeLevel_003
 * @tc.type: FUNC
 * @tc.desc: the device maxLevel is not valid, return minVolumeIndexMap_[volumeType].
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetMinVolumeLevel_003, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    int32_t ret = audioAdapterManager->GetMinVolumeLevel(STREAM_ALARM, DEVICE_TYPE_NONE);
    EXPECT_EQ(ret, audioAdapterManager->minVolumeIndexMap_[STREAM_ALARM]);
}

/**
 * @tc.name: Test GetMinVolumeLevel
 * @tc.number: GetMinVolumeLevel_004
 * @tc.type: FUNC
 * @tc.desc: the volume Type is not valid, return minVolumeIndexMap_[STREAM_MUSIC].
 */
HWTEST_F(AudioAdapterManagerUnitTest, GetMinVolumeLevel_004, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    int32_t ret = audioAdapterManager->GetMinVolumeLevel(STREAM_DEFAULT, DEVICE_TYPE_NONE);
    EXPECT_NE(ret, audioAdapterManager->minVolumeIndexMap_[STREAM_MUSIC]);
}

} // namespace AudioStandard
} // namespace OHOS
