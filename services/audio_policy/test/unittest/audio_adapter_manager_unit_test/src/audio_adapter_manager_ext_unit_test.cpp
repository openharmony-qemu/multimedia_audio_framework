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
    OffloadAdapter adapter = OFFLOAD_IN_PRIMARY;
    audioAdapterManager->SetOffloadSessionId(sessionId, adapter);
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
    OffloadAdapter adapter = OFFLOAD_IN_REMOTE;
    audioAdapterManager->SetOffloadSessionId(sessionId, adapter);
    EXPECT_NO_THROW(
        audioAdapterManager->SetOffloadVolume(streamType, volumeDb, deviceClass, networkId);
    );
}

/**
 * @tc.name: SetOffloadVolume_005
 * @tc.desc: Test SetOffloadVolume
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, SetOffloadVolume_005, TestSize.Level4)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    EXPECT_NE(audioAdapterManager, nullptr);
    AudioStreamType streamType = STREAM_MUSIC;
    float volumeDb = 0.0f;
    std::string deviceClass = "offload";
    std::string networkId = "";
    auto interruptServiceTest = GetInterruptServiceTest();
    audioAdapterManager->audioServerProxy_ = interruptServiceTest->GetAudioServerProxy();
    EXPECT_NE(audioAdapterManager->audioServerProxy_, nullptr);
    uint32_t sessionId = MIN_STREAMID + 1;
    OffloadAdapter adapter = OFFLOAD_IN_PRIMARY;
    audioAdapterManager->SetOffloadSessionId(sessionId, adapter);
    EXPECT_NO_THROW(
        audioAdapterManager->SetOffloadVolume(streamType, volumeDb, deviceClass, networkId);
    );
}

/**
 * @tc.name: SetOffloadVolume_006
 * @tc.desc: Test SetOffloadVolume
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, SetOffloadVolume_006, TestSize.Level4)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    EXPECT_NE(audioAdapterManager, nullptr);
    AudioStreamType streamType = STREAM_MUSIC;
    float volumeDb = 0.0f;
    std::string deviceClass = "remote";
    std::string networkId = "";
    auto interruptServiceTest = GetInterruptServiceTest();
    audioAdapterManager->audioServerProxy_ = interruptServiceTest->GetAudioServerProxy();
    EXPECT_NE(audioAdapterManager->audioServerProxy_, nullptr);
    uint32_t sessionId = MIN_STREAMID + 1;
    OffloadAdapter adapter = OFFLOAD_IN_REMOTE;
    audioAdapterManager->SetOffloadSessionId(sessionId, adapter);
    EXPECT_NO_THROW(
        audioAdapterManager->SetOffloadVolume(streamType, volumeDb, deviceClass, networkId);
    );
}

/**
 * @tc.name: SetOffloadVolume_007
 * @tc.desc: Test SetOffloadVolume
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, SetOffloadVolume_007, TestSize.Level4)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    EXPECT_NE(audioAdapterManager, nullptr);
    AudioStreamType streamType = STREAM_MUSIC;
    float volumeDb = 0.0f;
    std::string deviceClass = "offload";
    std::string networkId = "";
    auto interruptServiceTest = GetInterruptServiceTest();
    audioAdapterManager->audioServerProxy_ = interruptServiceTest->GetAudioServerProxy();
    EXPECT_NE(audioAdapterManager->audioServerProxy_, nullptr);
    uint32_t sessionId = MIN_STREAMID + 1;
    OffloadAdapter adapter = OFFLOAD_IN_PRIMARY;
    EXPECT_NO_THROW(
        audioAdapterManager->SetOffloadVolume(streamType, volumeDb, deviceClass, networkId);
    );
}

/**
 * @tc.name: SetOffloadVolume_008
 * @tc.desc: Test SetOffloadVolume
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, SetOffloadVolume_008, TestSize.Level4)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    EXPECT_NE(audioAdapterManager, nullptr);
    AudioStreamType streamType = STREAM_MUSIC;
    float volumeDb = 0.0f;
    std::string deviceClass = "remote";
    std::string networkId = "";
    auto interruptServiceTest = GetInterruptServiceTest();
    audioAdapterManager->audioServerProxy_ = interruptServiceTest->GetAudioServerProxy();
    EXPECT_NE(audioAdapterManager->audioServerProxy_, nullptr);
    uint32_t sessionId = MIN_STREAMID + 1;
    OffloadAdapter adapter = OFFLOAD_IN_REMOTE;
    EXPECT_NO_THROW(
        audioAdapterManager->SetOffloadVolume(streamType, volumeDb, deviceClass, networkId);
    );
}

/**
 * @tc.name: ResetOffloadSessionId_001
 * @tc.desc: Test ResetOffloadSessionId
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, ResetOffloadSessionId_001, TestSize.Level4)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    EXPECT_NE(audioAdapterManager, nullptr);

    uint32_t sessionId = MIN_STREAMID + 1;
    OffloadAdapter adapter = OFFLOAD_IN_PRIMARY;
    audioAdapterManager->SetOffloadSessionId(sessionId, adapter);
    audioAdapterManager->ResetOffloadSessionId(adapter);
    audioAdapterManager->ResetOffloadSessionId(adapter);
    EXPECT_EQ(audioAdapterManager->offloadSessionID_[adapter].has_value(), false);

    adapter = OFFLOAD_IN_REMOTE;
    audioAdapterManager->SetOffloadSessionId(sessionId, adapter);
    audioAdapterManager->ResetOffloadSessionId(adapter);
    audioAdapterManager->ResetOffloadSessionId(adapter);
    EXPECT_EQ(audioAdapterManager->offloadSessionID_[adapter].has_value(), false);
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

/**
 * @tc.name: UpdateSafeVolumeInner_001
 * @tc.desc: Test UpdateSafeVolumeInner
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, UpdateSafeVolumeInner_001, TestSize.Level4)
{
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_NEARLINK;
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->safeVolume_ = 10000;
    audioAdapterManager->isSleBoot_ = false;
    audioAdapterManager->safeStatusSle_ = SAFE_INACTIVE;

    audioAdapterManager->UpdateSafeVolumeInner(device);
    EXPECT_EQ(audioAdapterManager->isSleBoot_, false);
}

/**
 * @tc.name: UpdateSafeVolumeInner_002
 * @tc.desc: Test UpdateSafeVolumeInner
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, UpdateSafeVolumeInner_002, TestSize.Level4)
{
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_NEARLINK;
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->safeVolume_ = -1;
    audioAdapterManager->isSleBoot_ = true;
    audioAdapterManager->safeStatusSle_ = SAFE_INACTIVE;

    audioAdapterManager->UpdateSafeVolumeInner(device);
    EXPECT_EQ(audioAdapterManager->isSleBoot_, false);
}

/**
 * @tc.name: UpdateSafeVolumeInner_003
 * @tc.desc: Test UpdateSafeVolumeInner
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, UpdateSafeVolumeInner_003, TestSize.Level4)
{
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_NEARLINK;
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->safeVolume_ = -1;
    audioAdapterManager->isSleBoot_ = false;
    audioAdapterManager->safeStatusSle_ = SAFE_INACTIVE;

    audioAdapterManager->UpdateSafeVolumeInner(device);
    EXPECT_EQ(audioAdapterManager->isSleBoot_, false);
}

/**
 * @tc.name: InitSafeStatus_001
 * @tc.desc: Test InitSafeStatus
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, InitSafeStatus_001, TestSize.Level4)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    bool isFirstBoot = true;

    audioAdapterManager->InitSafeStatus(isFirstBoot);
    EXPECT_EQ(isFirstBoot, true);
}

/**
 * @tc.name: InitSafeStatus_002
 * @tc.desc: Test InitSafeStatus
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, InitSafeStatus_002, TestSize.Level4)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    bool isFirstBoot = false;

    audioAdapterManager->InitSafeStatus(isFirstBoot);
    EXPECT_EQ(isFirstBoot, false);
}

/**
 * @tc.name: InitSafeTime_001
 * @tc.desc: Test InitSafeTime
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, InitSafeTime_001, TestSize.Level4)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    bool isFirstBoot = true;

    audioAdapterManager->InitSafeTime(isFirstBoot);
    EXPECT_EQ(isFirstBoot, true);
}

/**
 * @tc.name: InitSafeTime_002
 * @tc.desc: Test InitSafeTime
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, InitSafeTime_002, TestSize.Level4)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    bool isFirstBoot = false;

    audioAdapterManager->InitSafeTime(isFirstBoot);
    EXPECT_EQ(isFirstBoot, false);
}

/**
 * @tc.name: ConvertSafeTime_001
 * @tc.desc: Test ConvertSafeTime
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, ConvertSafeTime_001, TestSize.Level4)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->safeActiveSleTime_ = 1000;

    audioAdapterManager->ConvertSafeTime();
    EXPECT_EQ(audioAdapterManager->safeActiveSleTime_, 1);
}

/**
 * @tc.name: ConvertSafeTime_002
 * @tc.desc: Test ConvertSafeTime
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, ConvertSafeTime_002, TestSize.Level4)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->safeActiveSleTime_ = 0;

    audioAdapterManager->ConvertSafeTime();
    EXPECT_EQ(audioAdapterManager->safeActiveSleTime_, 0);
}

/**
 * @tc.name: GetCurrentDeviceSafeStatus_001
 * @tc.desc: Test GetCurrentDeviceSafeStatus
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, GetCurrentDeviceSafeStatus_001, TestSize.Level4)
{
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_NEARLINK;
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->safeStatusSle_ = SAFE_INACTIVE;

    auto ret = audioAdapterManager->GetCurrentDeviceSafeStatus(device->deviceType_);
    EXPECT_EQ(ret, SAFE_INACTIVE);
}

/**
 * @tc.name: GetCurentDeviceSafeTime_001
 * @tc.desc: Test GetCurentDeviceSafeTime
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, GetCurentDeviceSafeTime_001, TestSize.Level4)
{
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_NEARLINK;
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();

    auto ret = audioAdapterManager->GetCurentDeviceSafeTime(device->deviceType_);
    EXPECT_LT(0, ret);
}

/**
 * @tc.name: GetRestoreVolumeLevel_001
 * @tc.desc: Test GetRestoreVolumeLevel
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, GetRestoreVolumeLevel_001, TestSize.Level4)
{
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_NEARLINK;
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();

    auto ret = audioAdapterManager->GetRestoreVolumeLevel(device->deviceType_);
    EXPECT_LT(0, ret);
}

/**
 * @tc.name: SetDeviceSafeStatus_001
 * @tc.desc: Test SetDeviceSafeStatus
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, SetDeviceSafeStatus_001, TestSize.Level4)
{
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_NEARLINK;
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();

    audioAdapterManager->SetDeviceSafeStatus(device->deviceType_, SAFE_INACTIVE);
    EXPECT_EQ(audioAdapterManager->safeStatusSle_, SAFE_INACTIVE);
}

/**
 * @tc.name: SetDeviceSafeTime_001
 * @tc.desc: Test SetDeviceSafeTime
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, SetDeviceSafeTime_001, TestSize.Level4)
{
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_NEARLINK;
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();

    audioAdapterManager->SetDeviceSafeTime(device->deviceType_, 120);
    EXPECT_EQ(audioAdapterManager->safeActiveSleTime_, 120);
}

/**
 * @tc.name: SetRestoreVolumeLevel_001
 * @tc.desc: Test SetRestoreVolumeLevel
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, SetRestoreVolumeLevel_001, TestSize.Level4)
{
    auto device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = DEVICE_TYPE_NEARLINK;
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();

    audioAdapterManager->SetRestoreVolumeLevel(device->deviceType_, 5);
    EXPECT_EQ(audioAdapterManager->safeActiveSleVolume_, 5);
}

/**
 * @tc.name: SafeVolumeDump_001
 * @tc.desc: Test SafeVolumeDump
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, SafeVolumeDump_001, TestSize.Level4)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->isSafeBoot_ = true;

    std::string dumpString;
    audioAdapterManager->SafeVolumeDump(dumpString);
    EXPECT_EQ(audioAdapterManager->isSafeBoot_, true);
}

/**
 * @tc.name: SafeVolumeDump_002
 * @tc.desc: Test SafeVolumeDump
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, SafeVolumeDump_002, TestSize.Level4)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->isSafeBoot_ = false;

    std::string dumpString;
    audioAdapterManager->SafeVolumeDump(dumpString);
    EXPECT_EQ(audioAdapterManager->isSafeBoot_, false);
}

/**
 * @tc.name: GetStreamMuteInternal
 * @tc.desc: Test SafeVolumeDump
 * @tc.type: FUNC
 * @tc.require: #ICDC94
 */
HWTEST_F(AudioAdapterManagerExtUnitTest, GetStreamMuteInternal, TestSize.Level4)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    auto desc = std::make_shared<AudioDeviceDescriptor>();

    EXPECT_EQ(audioAdapterManager->GetStreamMuteInternal(desc, STREAM_RING), false);
    EXPECT_EQ(audioAdapterManager->GetStreamMuteInternal(desc, STREAM_MUSIC), false);
    audioAdapterManager->ringerNoMuteDevice_ = desc;
    EXPECT_EQ(audioAdapterManager->GetStreamMuteInternal(desc, STREAM_RING), false);
    EXPECT_EQ(audioAdapterManager->GetStreamMuteInternal(desc, STREAM_MUSIC), false);
}
} // namespace AudioStandard
} // namespace OHOS
