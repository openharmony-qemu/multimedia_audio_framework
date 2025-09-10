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
#include "audio_adapter_manager_ext_unit_test.h"
#include "audio_stream_descriptor.h"
#include "audio_interrupt_service.h"
#include "audio_adapter_manager_handler.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

static AudioAdapterManager *audioAdapterManager_;

void AudioAdapterManagerExtUnitTest::SetUpTestCase(void) {}
void AudioAdapterManagerExtUnitTest::TearDownTestCase(void) {}

std::shared_ptr<AudioInterruptService> GetInterruptServiceTest()
{
    return std::make_shared<AudioInterruptService>();
}

/**
 * @tc.name: SetAudioVolume_001
 * @tc.desc: Test SetAudioVolume
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, SetAudioVolume_001, TestSize.Level4)
{
    auto adapterManager = std::make_shared<AudioAdapterManager>();
    adapterManager->currentActiveDevice_.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    adapterManager->isAbsVolumeScene_ = true;
    VolumeUtils::isPCVolumeEnable_ = true;
    float temp = 1.0f;
    EXPECT_NO_THROW(
        adapterManager->SetAudioVolume(STREAM_MUSIC, temp);
    );
}

/**
 * @tc.name: SetAudioVolume_002
 * @tc.desc: Test SetAudioVolume
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, SetAudioVolume_002, TestSize.Level4)
{
    auto adapterManager = std::make_shared<AudioAdapterManager>();
    adapterManager->currentActiveDevice_.deviceType_ = DEVICE_TYPE_SPEAKER;
    adapterManager->currentActiveDevice_.networkId_ = "";
    float temp = 1.0f;
    VolumeUtils::isPCVolumeEnable_ = true;
    EXPECT_NO_THROW(
        adapterManager->SetAudioVolume(STREAM_MUSIC, temp);
    );
}

/**
 * @tc.name: SetAudioVolume_003
 * @tc.desc: Test SetAudioVolume
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, SetAudioVolume_003, TestSize.Level4)
{
    auto adapterManager = std::make_shared<AudioAdapterManager>();
    adapterManager->currentActiveDevice_.deviceType_ = DEVICE_TYPE_NEARLINK;
    float temp = 1.0f;
    VolumeUtils::isPCVolumeEnable_ = true;
    adapterManager->isSleVoiceStatus_ = false;
    EXPECT_NO_THROW(
        adapterManager->SetAudioVolume(STREAM_MUSIC, temp);
    );
}

/**
 * @tc.name: SetAudioVolume_004
 * @tc.desc: Test SetAudioVolume
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, SetAudioVolume_004, TestSize.Level4)
{
    auto adapterManager = std::make_shared<AudioAdapterManager>();
    adapterManager->currentActiveDevice_.deviceType_ = DEVICE_TYPE_NEARLINK;
    float temp = 1.0f;
    VolumeUtils::isPCVolumeEnable_ = false;
    EXPECT_NO_THROW(
        adapterManager->SetAudioVolume(STREAM_VOICE_COMMUNICATION, temp);
    );
}

/**
 * @tc.name: SetAudioVolumeThree_001
 * @tc.desc: Test SetAudioVolume
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, SetAudioVolumeThree_001, TestSize.Level4)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    EXPECT_NE(audioAdapterManager, nullptr);
    std::shared_ptr<AudioDeviceDescriptor> desc =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    desc->networkId_ = "";
    AudioStreamType streamType = STREAM_MUSIC;
    float volumeDb = 0.0f;
    audioAdapterManager->volumeDataExtMaintainer_[desc->GetKey()] = std::make_shared<VolumeDataMaintainer>();
    EXPECT_NO_THROW(
        audioAdapterManager->SetAudioVolume(desc, streamType, volumeDb);
    );
}

/**
 * @tc.name: SetAudioVolumeThree_002
 * @tc.desc: Test SetAudioVolume
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, SetAudioVolumeThree_002, TestSize.Level4)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    EXPECT_NE(audioAdapterManager, nullptr);
    std::shared_ptr<AudioDeviceDescriptor> desc =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    AudioStreamType streamType = STREAM_MUSIC;
    desc->networkId_ = "LocalDevice";
    float volumeDb = 0.0f;
    audioAdapterManager->volumeDataExtMaintainer_[desc->GetKey()] = std::make_shared<VolumeDataMaintainer>();
    EXPECT_NO_THROW(
        audioAdapterManager->SetAudioVolume(desc, streamType, volumeDb);
    );
}

/**
 * @tc.name: SetAudioVolumeThree_003
 * @tc.desc: Test SetAudioVolume
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, SetAudioVolumeThree_003, TestSize.Level4)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    EXPECT_NE(audioAdapterManager, nullptr);
    std::shared_ptr<AudioDeviceDescriptor> desc =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_NONE, OUTPUT_DEVICE);
    AudioStreamType streamType = STREAM_MUSIC;
    desc->networkId_ = "LocalDevice";
    float volumeDb = 0.0f;
    audioAdapterManager->volumeDataExtMaintainer_[desc->GetKey()] = std::make_shared<VolumeDataMaintainer>();
    EXPECT_NO_THROW(
        audioAdapterManager->SetAudioVolume(desc, streamType, volumeDb);
    );
}

/**
 * @tc.name: SetOffloadVolume_001
 * @tc.desc: Test SetOffloadVolume
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, SetOffloadVolume_001, TestSize.Level4)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    EXPECT_NE(audioAdapterManager, nullptr);
    AudioStreamType streamType = STREAM_MUSIC;
    float volumeDb = 0.0f;
    std::string deviceClass = "";
    std::string networkId = "";
    EXPECT_NO_THROW(
        audioAdapterManager->SetOffloadVolume(streamType, volumeDb, deviceClass, networkId);
    );
}

/**
 * @tc.name: SetOffloadVolume_002
 * @tc.desc: Test SetOffloadVolume
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, SetOffloadVolume_002, TestSize.Level4)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    EXPECT_NE(audioAdapterManager, nullptr);
    AudioStreamType streamType = STREAM_SPEECH;
    float volumeDb = 0.0f;
    std::string deviceClass = "";
    std::string networkId = "";
    EXPECT_NO_THROW(
        audioAdapterManager->SetOffloadVolume(streamType, volumeDb, deviceClass, networkId);
    );
}

/**
 * @tc.name: SetOffloadVolume_003
 * @tc.desc: Test SetOffloadVolume
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, SetOffloadVolume_003, TestSize.Level4)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    EXPECT_NE(audioAdapterManager, nullptr);
    AudioStreamType streamType = STREAM_RING;
    float volumeDb = 0.0f;
    std::string deviceClass = "";
    std::string networkId = "";
    uint32_t sessionId = MIN_STREAMID + 1;
    audioAdapterManager->SetOffloadSessionId(sessionId);
    EXPECT_NO_THROW(
        audioAdapterManager->SetOffloadVolume(streamType, volumeDb, deviceClass, networkId);
    );
}

/**
 * @tc.name: SetOffloadVolume_004
 * @tc.desc: Test SetOffloadVolume
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, SetOffloadVolume_004, TestSize.Level4)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    EXPECT_NE(audioAdapterManager, nullptr);
    AudioStreamType streamType = STREAM_RING;
    float volumeDb = 0.0f;
    std::string deviceClass = "";
    std::string networkId = "";
    uint32_t sessionId = 1;
    audioAdapterManager->SetOffloadSessionId(sessionId);
    EXPECT_NO_THROW(
        audioAdapterManager->SetOffloadVolume(streamType, volumeDb, deviceClass, networkId);
    );
}

/**
 * @tc.name: SetDoubleRingVolumeDb_002
 * @tc.desc: Test SetDoubleRingVolumeDb
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, SetDoubleRingVolumeDb_002, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    EXPECT_NE(audioAdapterManager, nullptr);
    audioAdapterManager->Init();

    uint32_t sessionId = MIN_STREAMID + 1;
    audioAdapterManager->SetOffloadSessionId(sessionId);
    audioAdapterManager->ResetOffloadSessionId();
    audioAdapterManager->currentActiveDevice_.deviceType_ = DEVICE_TYPE_SPEAKER;
    AudioStreamType streamType = STREAM_RING;
    int32_t volumeLevel = 5;
    audioAdapterManager->useNonlinearAlgo_ = true;
    int32_t result = audioAdapterManager->SetDoubleRingVolumeDb(streamType, volumeLevel);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: SetStreamMuteInternal_001
 * @tc.desc: Test SetStreamMuteInternal
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, SetStreamMuteInternal_001, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    EXPECT_NE(audioAdapterManager, nullptr);
    std::shared_ptr<AudioDeviceDescriptor> desc =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_NONE, OUTPUT_DEVICE);
    audioAdapterManager->volumeDataExtMaintainer_[desc->GetKey()] = std::make_shared<VolumeDataMaintainer>();
    int32_t res = audioAdapterManager->SetRingerModeInternal(RINGER_MODE_VIBRATE);
    EXPECT_EQ(res, SUCCESS);
    bool mute = true;
    AudioStreamType streamType = STREAM_RING;
    StreamUsage streamUsage = STREAM_USAGE_ALARM;
    DeviceType deviceType = DEVICE_TYPE_WIRED_HEADSET;
    int32_t ret = audioAdapterManager->SetStreamMuteInternal(desc, streamType, mute, streamUsage, deviceType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name: SetStreamMuteInternal_002
 * @tc.desc: Test SetStreamMuteInternal
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, SetStreamMuteInternal_002, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    EXPECT_NE(audioAdapterManager, nullptr);
    std::shared_ptr<AudioDeviceDescriptor> desc =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_NONE, OUTPUT_DEVICE);
    audioAdapterManager->volumeDataExtMaintainer_[desc->GetKey()] = std::make_shared<VolumeDataMaintainer>();
    int32_t res = audioAdapterManager->SetRingerModeInternal(RINGER_MODE_VIBRATE);
    EXPECT_EQ(res, SUCCESS);
    bool mute = false;
    AudioStreamType streamType = STREAM_RING;
    StreamUsage streamUsage = STREAM_USAGE_ALARM;
    DeviceType deviceType = DEVICE_TYPE_WIRED_HEADSET;
    int32_t ret = audioAdapterManager->SetStreamMuteInternal(desc, streamType, mute, streamUsage, deviceType);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name: SetStreamMuteInternal_003
 * @tc.desc: Test SetStreamMuteInternal
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, SetStreamMuteInternal_003, TestSize.Level1)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    EXPECT_NE(audioAdapterManager, nullptr);
    audioAdapterManager->Init();
    std::shared_ptr<AudioDeviceDescriptor> desc =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_NONE, OUTPUT_DEVICE);
    audioAdapterManager->volumeDataExtMaintainer_[desc->GetKey()] = std::make_shared<VolumeDataMaintainer>();
    int32_t res = audioAdapterManager->SetRingerModeInternal(RINGER_MODE_VIBRATE);
    EXPECT_EQ(res, SUCCESS);
    bool mute = false;
    AudioStreamType streamType = STREAM_RING;
    StreamUsage streamUsage = STREAM_USAGE_ALARM;
    DeviceType deviceType = DEVICE_TYPE_WIRED_HEADSET;
    int32_t ret = audioAdapterManager->SetStreamMuteInternal(desc, streamType, mute, streamUsage, deviceType);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name: SetStreamMuteInternal_004
 * @tc.desc: Test SetStreamMuteInternal
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, SetStreamMuteInternal_004, TestSize.Level4)
{
    auto adapterManager = std::make_shared<AudioAdapterManager>();
    adapterManager->currentActiveDevice_.volumeBehavior_.databaseVolumeName = "databaseVolumeName";
    VolumeUtils::isPCVolumeEnable_ = true;
    adapterManager->currentActiveDevice_.volumeBehavior_.isVolumeControlDisabled = false;
    EXPECT_NO_THROW(
        adapterManager->SetStreamMuteInternal(STREAM_MUSIC, false, STREAM_USAGE_MEDIA, DEVICE_TYPE_NONE, "");
    );
}

/**
 * @tc.name: SetAppRingMuted_001
 * @tc.desc: Test SetAppRingMuted
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, SetAppRingMuted_001, TestSize.Level4)
{
    StreamVolumeParams volumeParams;
    volumeParams.streamType = STREAM_RING;
    volumeParams.uid = 42;
    AudioVolume::GetInstance()->AddStreamVolume(volumeParams);

    ASSERT_EQ(AudioAdapterManager::GetInstance().SetAppRingMuted(42, true), SUCCESS);
    ASSERT_EQ(AudioAdapterManager::GetInstance().SetAppRingMuted(42, false), SUCCESS);
}

/**
 * @tc.name: IsAppRingMuted_001
 * @tc.desc: Test IsAppRingMuted
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, IsAppRingMuted_001, TestSize.Level4)
{
    StreamVolumeParams volumeParams;
    volumeParams.streamType = STREAM_RING;
    volumeParams.uid = 42;
    AudioVolume::GetInstance()->AddStreamVolume(volumeParams);

    auto &adapterManager = AudioAdapterManager::GetInstance();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    std::shared_ptr<AudioDeviceDescriptor> desc =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_REMOTE_CAST, OUTPUT_DEVICE);
    desc->networkId_ = "NotLocalDevice";
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    devices.push_back(desc);
    AudioZoneService::GetInstance().BindDeviceToAudioZone(zoneId1_, devices);

    ASSERT_EQ(AudioAdapterManager::GetInstance().SetAppRingMuted(42, true), SUCCESS);
    ASSERT_EQ(AudioAdapterManager::GetInstance().IsAppRingMuted(42), true);
    ASSERT_EQ(AudioAdapterManager::GetInstance().IsAppRingMuted(43), false);
    ASSERT_EQ(AudioAdapterManager::GetInstance().SetAppRingMuted(42, false), SUCCESS);
    ASSERT_EQ(AudioAdapterManager::GetInstance().IsAppRingMuted(42), false);
    ASSERT_EQ(AudioAdapterManager::GetInstance().IsAppRingMuted(43), false);
}

/**
 * @tc.name: GetVolumeAdjustZoneId_001
 * @tc.desc: Test GetVolumeAdjustZoneId
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, GetVolumeAdjustZoneId_001, TestSize.Level4)
{
    StreamVolumeParams volumeParams;
    volumeParams.streamType = STREAM_RING;
    volumeParams.uid = 42;
    AudioVolume::GetInstance()->AddStreamVolume(volumeParams);

    auto &adapterManager = AudioAdapterManager::GetInstance();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    std::shared_ptr<AudioDeviceDescriptor> desc =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_REMOTE_CAST, OUTPUT_DEVICE);
    desc->networkId_ = "LocalDevice";
    devices.push_back(desc);
    AudioZoneService::GetInstance().BindDeviceToAudioZone(zoneId1_, devices);
    AudioConnectedDevice::GetInstance().AddConnectedDevice(desc);
    AudioZoneService::GetInstance().UpdateDeviceFromGlobalForAllZone(desc);
    ASSERT_EQ(adapterManager.GetVolumeAdjustZoneId(), 0);
    ASSERT_EQ(adapterManager.SetAdjustVolumeForZone(zoneId1_), SUCCESS);
    ASSERT_EQ(adapterManager.GetVolumeAdjustZoneId(), zoneId1_);
}

/**
 * @tc.name: SetAdjustVolumeForZone_002
 * @tc.desc: Test SetAdjustVolumeForZone
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, SetAdjustVolumeForZone_002, TestSize.Level4)
{
    StreamVolumeParams volumeParams;
    volumeParams.streamType = STREAM_RING;
    volumeParams.uid = 42;
    AudioVolume::GetInstance()->AddStreamVolume(volumeParams);

    auto &adapterManager = AudioAdapterManager::GetInstance();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    std::shared_ptr<AudioDeviceDescriptor> desc =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_REMOTE_CAST, OUTPUT_DEVICE);
    desc->networkId_ = "LocalDevice";
    devices.push_back(desc);
    AudioZoneService::GetInstance().BindDeviceToAudioZone(zoneId1_, devices);
    AudioConnectedDevice::GetInstance().AddConnectedDevice(desc);
    AudioZoneService::GetInstance().UpdateDeviceFromGlobalForAllZone(desc);
    ASSERT_EQ(adapterManager.SetAdjustVolumeForZone(zoneId1_), SUCCESS);
    ASSERT_EQ(adapterManager.GetVolumeAdjustZoneId(), zoneId1_);
}
} // namespace AudioStandard
} // namespace OHOS
