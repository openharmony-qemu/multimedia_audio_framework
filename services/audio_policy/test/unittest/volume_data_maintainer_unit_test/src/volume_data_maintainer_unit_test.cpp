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

#include "volume_data_maintainer_unit_test.h"

#include <chrono>
#include "system_ability_definition.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "audio_zone_service.h"
#include "ipc_skeleton.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void VolumeDataMaintainerUnitTest::SetUpTestCase(void) {}
void VolumeDataMaintainerUnitTest::TearDownTestCase(void) {}
void VolumeDataMaintainerUnitTest::SetUp(void) {}
void VolumeDataMaintainerUnitTest::TearDown(void) {}


/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_008.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_008, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    int32_t affectedRet;
    bool statusRet;
    auto ret = volumeDataMaintainerRet->GetMuteAffected(affectedRet);
    EXPECT_EQ(ret, false);

    ret = volumeDataMaintainerRet->GetMuteTransferStatus(statusRet);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_009.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_009, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    int32_t affectedRet = 0;
    bool statusRet = false;
    AudioRingerMode ringerModeRet = RINGER_MODE_SILENT;
    auto ret = volumeDataMaintainerRet->SetMuteAffectedToMuteStatusDataBase(affectedRet);
    EXPECT_EQ(ret, true);

    ret = volumeDataMaintainerRet->SaveMuteTransferStatus(statusRet);
    EXPECT_EQ(ret, false);

    ret = volumeDataMaintainerRet->SaveRingerMode(ringerModeRet);
    EXPECT_EQ(ret, false);

    ret = volumeDataMaintainerRet->GetRingerMode(ringerModeRet);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_010.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_010, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    DeviceType deviceTypeRet = DEVICE_TYPE_BLUETOOTH_SCO;
    SafeStatus safeStatusRet = SAFE_UNKNOWN;
    auto ret = volumeDataMaintainerRet->SaveSafeStatus(deviceTypeRet, safeStatusRet);
    EXPECT_EQ(ret, false);

    deviceTypeRet = DEVICE_TYPE_USB_ARM_HEADSET;
    ret = volumeDataMaintainerRet->SaveSafeStatus(deviceTypeRet, safeStatusRet);
    EXPECT_EQ(ret, false);

    deviceTypeRet = DEVICE_TYPE_NONE;
    ret = volumeDataMaintainerRet->SaveSafeStatus(deviceTypeRet, safeStatusRet);
    EXPECT_EQ(ret, false);

    deviceTypeRet = DEVICE_TYPE_BLUETOOTH_A2DP;
    ret = volumeDataMaintainerRet->SaveSafeStatus(deviceTypeRet, safeStatusRet);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerDegreeUnitTest_001.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerDegreeUnitTest_001, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainer = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(nullptr, volumeDataMaintainer);
    std::shared_ptr<AudioDeviceDescriptor> device;
    int32_t volumeDegree = 1;
    VolumeScale volume{volumeDegree, volumeDegree};
    AudioStreamType streamType = STREAM_ALL;

    int32_t zoneId = 0;

    volumeDataMaintainer->SaveVolumeToMap(device, streamType, volume, zoneId);

    int32_t retVolumeDegree = volumeDataMaintainer->LoadVolumeDegreeFromMap(device, STREAM_ALL, zoneId);
    EXPECT_NE(retVolumeDegree, volumeDegree);

    EXPECT_NE(volumeDataMaintainer->LoadVolumeDegreeFromDb(device, STREAM_MUSIC), 0);

    device = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, device);
    device->deviceType_ = DEVICE_TYPE_SPEAKER;

    volumeDataMaintainer->SaveVolumeToMap(device, streamType, volume, zoneId);

    volumeDataMaintainer->SaveVolumeToMap(device, STREAM_MUSIC, volume, zoneId);
    retVolumeDegree = volumeDataMaintainer->LoadVolumeDegreeFromMap(device, streamType, zoneId);
    EXPECT_EQ(retVolumeDegree, volumeDegree);

    device->networkId_ = "111";
    EXPECT_EQ(volumeDataMaintainer->SaveVolumeToDb(device, STREAM_MUSIC, volume, zoneId), SUCCESS);

    device->volumeBehavior_.isReady = true;
    EXPECT_EQ(volumeDataMaintainer->SaveVolumeToDb(device, STREAM_MUSIC, volume, zoneId), SUCCESS);

    device->volumeBehavior_.databaseVolumeName = "Test";
    volumeDataMaintainer->SetDataShareReady(true);
    EXPECT_EQ(volumeDataMaintainer->SaveVolumeToDb(device, STREAM_MUSIC, volume, zoneId), SUCCESS);

    device->deviceType_ = DEVICE_TYPE_NONE;
    EXPECT_EQ(volumeDataMaintainer->LoadVolumeDegreeFromDb(device, STREAM_MUSIC), 0);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_011.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_011, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    DeviceType deviceTypeRet = DEVICE_TYPE_NONE;
    SafeStatus safeStatusRet = SAFE_UNKNOWN;
    auto ret = volumeDataMaintainerRet->GetSafeStatus(deviceTypeRet, safeStatusRet);
    EXPECT_EQ(ret, false);

    deviceTypeRet = DEVICE_TYPE_USB_ARM_HEADSET;
    ret = volumeDataMaintainerRet->GetSafeStatus(deviceTypeRet, safeStatusRet);
    EXPECT_EQ(ret, true);

    deviceTypeRet = DEVICE_TYPE_BLUETOOTH_A2DP;
    ret = volumeDataMaintainerRet->GetSafeStatus(deviceTypeRet, safeStatusRet);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_012.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_012, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    DeviceType deviceTypeRet = DEVICE_TYPE_BLUETOOTH_SCO;
    int64_t timeRet = 0;
    auto ret = volumeDataMaintainerRet->SaveSafeVolumeTime(deviceTypeRet, timeRet);
    EXPECT_EQ(ret, true);

    deviceTypeRet = DEVICE_TYPE_USB_ARM_HEADSET;
    ret = volumeDataMaintainerRet->SaveSafeVolumeTime(deviceTypeRet, timeRet);
    EXPECT_EQ(ret, true);

    deviceTypeRet = DEVICE_TYPE_NONE;
    ret = volumeDataMaintainerRet->SaveSafeVolumeTime(deviceTypeRet, timeRet);
    EXPECT_EQ(ret, false);

    deviceTypeRet = DEVICE_TYPE_BLUETOOTH_A2DP;
    ret = volumeDataMaintainerRet->SaveSafeVolumeTime(deviceTypeRet, timeRet);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_013.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_013, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    DeviceType deviceTypeRet = DEVICE_TYPE_NONE;
    int64_t timeRet = 0;
    auto ret = volumeDataMaintainerRet->GetSafeVolumeTime(deviceTypeRet, timeRet);
    EXPECT_EQ(ret, false);

    deviceTypeRet = DEVICE_TYPE_BLUETOOTH_SCO;
    ret = volumeDataMaintainerRet->GetSafeVolumeTime(deviceTypeRet, timeRet);
    EXPECT_EQ(ret, false);

    deviceTypeRet = DEVICE_TYPE_USB_HEADSET;
    ret = volumeDataMaintainerRet->GetSafeVolumeTime(deviceTypeRet, timeRet);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_014.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_014, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    std::string keyRet1;
    std::string valueRet1;
    auto ret = volumeDataMaintainerRet->SaveSystemSoundUrl(keyRet1, valueRet1);
    EXPECT_EQ(ret, true);

    std::string keyRet2;
    std::string valueRet2;
    ret = volumeDataMaintainerRet->GetSystemSoundUrl(keyRet2, valueRet2);
    EXPECT_EQ(ret, false);
    volumeDataMaintainerRet->RegisterCloned();
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_015.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_015, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    bool isMuteRet1 = false;
    auto ret = volumeDataMaintainerRet->SaveMicMuteState(isMuteRet1);
    EXPECT_EQ(ret, true);

    bool isMuteRet2;
    ret = volumeDataMaintainerRet->GetMicMuteState(isMuteRet2);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_016.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_016, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    DeviceType deviceTypeRet = DEVICE_TYPE_BLUETOOTH_SCO;

    auto ret = volumeDataMaintainerRet->GetDeviceGroupTypeKey(deviceTypeRet);
    std::string typeRet = "_wireless";
    EXPECT_EQ(ret, typeRet);

    deviceTypeRet = DEVICE_TYPE_USB_ARM_HEADSET;
    ret = volumeDataMaintainerRet->GetDeviceGroupTypeKey(deviceTypeRet);
    typeRet = "_wired";
    EXPECT_EQ(ret, typeRet);

    deviceTypeRet = DEVICE_TYPE_REMOTE_CAST;
    ret = volumeDataMaintainerRet->GetDeviceGroupTypeKey(deviceTypeRet);
    typeRet = "_remote_cast";
    EXPECT_EQ(ret, typeRet);
    
    deviceTypeRet = DEVICE_TYPE_NEARLINK;
    ret = volumeDataMaintainerRet->GetDeviceGroupTypeKey(deviceTypeRet);
    typeRet = "_wireless";
    EXPECT_EQ(ret, typeRet);

    deviceTypeRet = DEVICE_TYPE_NONE;
    ret = volumeDataMaintainerRet->GetDeviceGroupTypeKey(deviceTypeRet);
    typeRet = "";
    EXPECT_EQ(ret, typeRet);

    deviceTypeRet = DEVICE_TYPE_HEARING_AID;
    ret = volumeDataMaintainerRet->GetDeviceGroupTypeKey(deviceTypeRet);
    typeRet = "_hearing_aid";
    EXPECT_EQ(ret, typeRet);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_018.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_018, TestSize.Level3)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    bool isMuteRet1 = true;
    auto ret = volumeDataMaintainerRet->SaveMicMuteState(isMuteRet1);
    EXPECT_EQ(ret, true);

    bool isMuteRet2 = true;
    ret = volumeDataMaintainerRet->GetMicMuteState(isMuteRet2);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test VolumeDataMaintainer GetAppMute when appUid not found.
 * @tc.number: GetAppMute_001
 * @tc.desc  : Test VolumeDataMaintainer API when appUid is not present in appMuteStatusMap.
 */
HWTEST(VolumeDataMaintainerUnitTest, GetAppMute_001, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainer = std::make_shared<VolumeDataMaintainer>();
    int32_t appUid = 12345;
    bool isMute = false;
    volumeDataMaintainer->appMuteStatusMap_.erase(appUid);

    volumeDataMaintainer->GetAppMute(appUid, isMute);
    EXPECT_FALSE(isMute);
}

/**
 * @tc.name  : Test VolumeDataMaintainer GetAppMute when subIter.second is true.
 * @tc.number: GetAppMute_002
 * @tc.desc  : Test VolumeDataMaintainer API when subIter.second is true in the loop.
 */
HWTEST(VolumeDataMaintainerUnitTest, GetAppMute_002, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainer = std::make_shared<VolumeDataMaintainer>();
    int32_t appUid = 12345;
    bool isMute = false;
    volumeDataMaintainer->appMuteStatusMap_[appUid][STREAM_MUSIC] = true;

    volumeDataMaintainer->GetAppMute(appUid, isMute);
    EXPECT_TRUE(isMute);
}

/**
 * @tc.name  : Test VolumeDataMaintainer GetAppMute when subIter.second is false.
 * @tc.number: GetAppMute_003
 * @tc.desc  : Test VolumeDataMaintainer API when subIter.second is false in the loop.
 */
HWTEST(VolumeDataMaintainerUnitTest, GetAppMute_003, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainer = std::make_shared<VolumeDataMaintainer>();
    int32_t appUid = 12345;
    bool isMute = false;
    volumeDataMaintainer->appMuteStatusMap_[appUid][STREAM_MUSIC] = false;
    volumeDataMaintainer->appMuteStatusMap_[appUid][STREAM_RING] = false;

    volumeDataMaintainer->GetAppMute(appUid, isMute);
    EXPECT_FALSE(isMute);
}

/**
 * @tc.name  : Test VolumeDataMaintainer GetAppMuteOwned when appUid not found.
 * @tc.number: GetAppMuteOwned_001
 * @tc.desc  : Test VolumeDataMaintainer API when appUid is not present in appMuteStatusMap.
 */
HWTEST(VolumeDataMaintainerUnitTest, GetAppMuteOwned_001, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainer = std::make_shared<VolumeDataMaintainer>();
    int32_t appUid = 12345;
    bool isMute = false;
    volumeDataMaintainer->appMuteStatusMap_.erase(appUid);

    volumeDataMaintainer->GetAppMuteOwned(appUid, isMute);
    EXPECT_FALSE(isMute);
}

/**
 * @tc.name  : Test VolumeDataMaintainer GetAppMuteOwned when appUid is found.
 * @tc.number: GetAppMuteOwned_002
 * @tc.desc  : Test VolumeDataMaintainer API when appUid is present in appMuteStatusMap.
 */
HWTEST(VolumeDataMaintainerUnitTest, GetAppMuteOwned_002, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainer = std::make_shared<VolumeDataMaintainer>();
    int32_t appUid = 12345;
    bool isMute = false;
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    volumeDataMaintainer->appMuteStatusMap_[appUid][callingUid] = true;

    volumeDataMaintainer->GetAppMuteOwned(appUid, isMute);
    EXPECT_TRUE(isMute);
}

/**
 * @tc.name  : Test VolumeDataMaintainer SetMuteAffectedToMuteStatusDataBase when entering the if branch.
 * @tc.number: SetMuteAffectedToMuteStatusDataBase_001
 * @tc.desc  : Test VolumeDataMaintainer API when entering the if branch in the loop.
 */
HWTEST(VolumeDataMaintainerUnitTest, SetMuteAffectedToMuteStatusDataBase_001, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainer = std::make_shared<VolumeDataMaintainer>();
    std::lock_guard<ffrt::mutex> lock(volumeDataMaintainer->volumeMutex_);
    volumeDataMaintainer->appVolumeLevelMap_.clear();
    volumeDataMaintainer->appMuteStatusMap_.clear();

    int32_t affected = 1 << VolumeDataMaintainer::VT_STREAM_ALARM;
    bool result = volumeDataMaintainer->SetMuteAffectedToMuteStatusDataBase(affected);
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : Test VolumeDataMaintainer SetRestoreVolumeLevel when deviceType is USB_HEADSET and ret is not SUCCESS.
 * @tc.number: SetRestoreVolumeLevel_002
 * @tc.desc  : Test VolumeDataMaintainer API when deviceType is USB_HEADSET and PutIntValue fails.
 */
HWTEST(VolumeDataMaintainerUnitTest, SetRestoreVolumeLevel_002, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainer = std::make_shared<VolumeDataMaintainer>();
    std::lock_guard<ffrt::mutex> lock(volumeDataMaintainer->volumeMutex_);
    volumeDataMaintainer->appVolumeLevelMap_.clear();
    volumeDataMaintainer->appMuteStatusMap_.clear();

    DeviceType deviceType = DEVICE_TYPE_USB_HEADSET;
    int32_t volume = 5;
    bool result = volumeDataMaintainer->SetRestoreVolumeLevel(deviceType, volume);
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : Test VolumeDataMaintainer SetRestoreVolumeLevel when deviceType is
  USB_ARM_HEADSET and ret is not SUCCESS.
 * @tc.number: SetRestoreVolumeLevel_003
 * @tc.desc  : Test VolumeDataMaintainer API when deviceType is USB_ARM_HEADSET and PutIntValue fails.
 */
HWTEST(VolumeDataMaintainerUnitTest, SetRestoreVolumeLevel_003, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainer = std::make_shared<VolumeDataMaintainer>();
    std::lock_guard<ffrt::mutex> lock(volumeDataMaintainer->volumeMutex_);
    volumeDataMaintainer->appVolumeLevelMap_.clear();
    volumeDataMaintainer->appMuteStatusMap_.clear();

    DeviceType deviceType = DEVICE_TYPE_USB_ARM_HEADSET;
    int32_t volume = 5;
    bool result = volumeDataMaintainer->SetRestoreVolumeLevel(deviceType, volume);
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : Test VolumeDataMaintainer SetRestoreVolumeLevel when deviceType is not supported.
 * @tc.number: SetRestoreVolumeLevel_004
 * @tc.desc  : Test VolumeDataMaintainer API when deviceType is not supported.
 */
HWTEST(VolumeDataMaintainerUnitTest, SetRestoreVolumeLevel_004, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainer = std::make_shared<VolumeDataMaintainer>();
    std::lock_guard<ffrt::mutex> lock(volumeDataMaintainer->volumeMutex_);
    volumeDataMaintainer->appVolumeLevelMap_.clear();
    volumeDataMaintainer->appMuteStatusMap_.clear();

    DeviceType deviceType = DEVICE_TYPE_INVALID;
    int32_t volume = 5;
    bool result = volumeDataMaintainer->SetRestoreVolumeLevel(deviceType, volume);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test VolumeDataMaintainer GetRestoreVolumeLevel when deviceType is not supported.
 * @tc.number: GetRestoreVolumeLevel_001
 * @tc.desc  : Test VolumeDataMaintainer API when deviceType is not supported.
 */
HWTEST(VolumeDataMaintainerUnitTest, GetRestoreVolumeLevel_001, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainer = std::make_shared<VolumeDataMaintainer>();
    std::lock_guard<ffrt::mutex> lock(volumeDataMaintainer->volumeMutex_);
    volumeDataMaintainer->appVolumeLevelMap_.clear();
    volumeDataMaintainer->appMuteStatusMap_.clear();

    DeviceType deviceType = DEVICE_TYPE_INVALID;
    int32_t volume = 0;
    bool result = volumeDataMaintainer->GetRestoreVolumeLevel(deviceType, volume);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test VolumeDataMaintainer GetRestoreVolumeLevel when deviceType is not supported.
 * @tc.number: GetRestoreVolumeLevel_002
 * @tc.desc  : Test VolumeDataMaintainer API when deviceType is not supported.
 */
HWTEST(VolumeDataMaintainerUnitTest, GetRestoreVolumeLevel_002, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainer = std::make_shared<VolumeDataMaintainer>();
    std::lock_guard<ffrt::mutex> lock(volumeDataMaintainer->volumeMutex_);
    volumeDataMaintainer->appVolumeLevelMap_.clear();
    volumeDataMaintainer->appMuteStatusMap_.clear();

    DeviceType deviceType = DEVICE_TYPE_INVALID;
    int32_t volume = 0;
    bool result = volumeDataMaintainer->GetRestoreVolumeLevel(deviceType, volume);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test VolumeDataMaintainer.
 * @tc.number: SaveMicMuteStateTest_002.
 * @tc.desc  : Test SaveMicMuteState API.
 */
HWTEST(VolumeDataMaintainerUnitTest, SaveMicMuteStateTest_002, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    bool isMuteRet1 = true;
    auto ret = volumeDataMaintainerRet->SaveMicMuteState(isMuteRet1);
    EXPECT_EQ(ret, true);

    const int32_t invalidUserId = -1;
    const int32_t invalidUserIdMinUser = 99;
    const int32_t validUser = 1000;
    AudioSettingProvider& settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    int32_t userId = settingProvider.GetCurrentUserId(invalidUserId);
    EXPECT_NE(userId, invalidUserId);
    userId = settingProvider.GetCurrentUserId(AudioSettingProvider::MAIN_USER_ID);
    EXPECT_EQ(userId, AudioSettingProvider::MAIN_USER_ID);
    userId = settingProvider.GetCurrentUserId(invalidUserIdMinUser);
    EXPECT_NE(userId, invalidUserIdMinUser);
    userId = settingProvider.GetCurrentUserId(validUser);
    EXPECT_EQ(userId, validUser);
}

/**
 * @tc.name  : Test VolumeDataMaintainer.
 * @tc.number: SetAppStreamMuted_001.
 * @tc.desc  : Test SetAppStreamMuted and IsAppStreamMuted.
 */
HWTEST(VolumeDataMaintainerUnitTest, SetAppStreamMuted_001, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainer = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(nullptr, volumeDataMaintainer);
    const int32_t appUid = 123;
    const AudioStreamType streamType = AudioStreamType::STREAM_RING;

    // Test setting mute
    volumeDataMaintainer->SetAppStreamMuted(appUid, streamType, true);
    EXPECT_EQ(volumeDataMaintainer->IsAppStreamMuted(appUid, streamType), true);

    // Test setting unmute
    volumeDataMaintainer->SetAppStreamMuted(appUid, streamType, false);
    EXPECT_EQ(volumeDataMaintainer->IsAppStreamMuted(appUid, streamType), false);

    // Test setting mute repeatedly
    volumeDataMaintainer->SetAppStreamMuted(appUid, streamType, true);
    volumeDataMaintainer->SetAppStreamMuted(appUid, streamType, true);
    EXPECT_EQ(volumeDataMaintainer->IsAppStreamMuted(appUid, streamType), true);

    // Test setting mute for another app stream
    const int32_t anotherAppUid = 456;
    volumeDataMaintainer->SetAppStreamMuted(anotherAppUid, streamType, true);
    EXPECT_EQ(volumeDataMaintainer->IsAppStreamMuted(anotherAppUid, streamType), true);

    // Test multiple stream types
    const AudioStreamType anotherStreamType = AudioStreamType::STREAM_VOICE_COMMUNICATION;
    volumeDataMaintainer->SetAppStreamMuted(appUid, anotherStreamType, true);
    EXPECT_EQ(volumeDataMaintainer->IsAppStreamMuted(appUid, anotherStreamType), true);
    EXPECT_EQ(volumeDataMaintainer->IsAppStreamMuted(appUid, streamType), true);

    // Test delete an app entry
    volumeDataMaintainer->SetAppStreamMuted(appUid, streamType, false);
    volumeDataMaintainer->SetAppStreamMuted(appUid, anotherStreamType, false);
    EXPECT_EQ(volumeDataMaintainer->IsAppStreamMuted(appUid, streamType), false);
    EXPECT_EQ(volumeDataMaintainer->IsAppStreamMuted(appUid, anotherStreamType), false);
}

/**
 * @tc.name  : Test VolumeDataMaintainer.
 * @tc.number: SetDataShareReady_001.
 * @tc.desc  : Test SetDataShareReady API.
 */
HWTEST(VolumeDataMaintainerUnitTest, SetDataShareReady_001, TestSize.Level4)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    std::atomic<bool> isDataShareReady = false;
    volumeDataMaintainerRet->SetDataShareReady(std::atomic_load(&isDataShareReady));
    AudioSettingProvider& audioSettingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    EXPECT_FALSE(audioSettingProvider.isDataShareReady_);
}

/**
 * @tc.name  : Test VolumeDataMaintainer.
 * @tc.number: SetDataShareReady_001.
 * @tc.desc  : Test SetDataShareReady API.
 */
HWTEST(VolumeDataMaintainerUnitTest, SetDataShareReady_002, TestSize.Level4)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    std::atomic<bool> isDataShareReady = true;
    volumeDataMaintainerRet->SetDataShareReady(std::atomic_load(&isDataShareReady));
    AudioSettingProvider& audioSettingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    EXPECT_TRUE(audioSettingProvider.isDataShareReady_);
}

/**
 * @tc.name  : Test VolumeDataMaintainer.
 * @tc.number: GetAppVolume_001.
 * @tc.desc  : Test GetAppVolume API.
 */
HWTEST(VolumeDataMaintainerUnitTest, GetAppVolume_001, TestSize.Level4)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    int32_t appUid = 99999;
    int32_t volumeLevel = 1;
    volumeDataMaintainerRet->SetAppVolume(appUid, volumeLevel);
    int32_t ret = volumeDataMaintainerRet->GetAppVolume(appUid);
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name  : Test VolumeDataMaintainer.
 * @tc.number: IsSetAppVolume_001.
 * @tc.desc  : Test IsSetAppVolume API.
 */
HWTEST(VolumeDataMaintainerUnitTest, IsSetAppVolume_001, TestSize.Level4)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    int32_t appUid = 99999;
    int32_t volumeLevel = 1;
    volumeDataMaintainerRet->SetAppVolume(appUid, volumeLevel);
    bool ret = volumeDataMaintainerRet->IsSetAppVolume(appUid);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test VolumeDataMaintainer.
 * @tc.number: CheckOsAccountReady_001.
 * @tc.desc  : Test CheckOsAccountReady API.
 */
HWTEST(VolumeDataMaintainerUnitTest, CheckOsAccountReady_001, TestSize.Level4)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    bool ret = volumeDataMaintainerRet->CheckOsAccountReady();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test VolumeDataMaintainer.
 * @tc.number: GetVolumeKeyForDatabaseVolumeName_001.
 * @tc.desc  : Test GetVolumeKeyForDatabaseVolumeName API.
 */
HWTEST(VolumeDataMaintainerUnitTest, GetVolumeKeyForDatabaseVolumeName_001, TestSize.Level4)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    std::string databaseVolumeName = "VolumeDataMaintainerUnitTest";
    AudioStreamType streamType = AudioStreamType::STREAM_MUSIC;
    std::string expect = "VolumeDataMaintainerUnitTest_music_volume";
    std::string ret = volumeDataMaintainerRet->GetVolumeKeyForDatabaseVolumeName(databaseVolumeName, streamType);
    EXPECT_EQ(ret, expect);

    streamType = AudioStreamType::STREAM_DEFAULT;
    expect = "";
    ret = volumeDataMaintainerRet->GetVolumeKeyForDatabaseVolumeName(databaseVolumeName, streamType);
    EXPECT_EQ(ret, expect);
}

/**
 * @tc.name  : Test VolumeDataMaintainer.
 * @tc.number: GetMuteKeyForDatabaseVolumeName_001.
 * @tc.desc  : Test GetMuteKeyForDatabaseVolumeName API.
 */
HWTEST(VolumeDataMaintainerUnitTest, GetMuteKeyForDatabaseVolumeName_001, TestSize.Level4)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    std::string databaseVolumeName = "VolumeDataMaintainerUnitTest";
    AudioStreamType streamType = AudioStreamType::STREAM_MUSIC;
    std::string expect = "VolumeDataMaintainerUnitTest_music_mute_status";
    std::string ret = volumeDataMaintainerRet->GetMuteKeyForDatabaseVolumeName(databaseVolumeName, streamType);
    EXPECT_EQ(ret, expect);

    streamType = AudioStreamType::STREAM_DEFAULT;
    expect = "";
    ret = volumeDataMaintainerRet->GetMuteKeyForDatabaseVolumeName(databaseVolumeName, streamType);
    EXPECT_EQ(ret, expect);
}


/**
 * @tc.name  : Test VolumeDataMaintainer.
 * @tc.number: GetMuteKeyForDatabaseVolumeName_001.
 * @tc.desc  : Test GetMuteKeyForDatabaseVolumeName API.
 */
HWTEST(VolumeDataMaintainerUnitTest, GetVolumeKey, TestSize.Level4)
{
    std::shared_ptr<VolumeDataMaintainer> vd = std::make_shared<VolumeDataMaintainer>();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    EXPECT_NE(vd->GetVolumeKey(desc, STREAM_RING), "");
    EXPECT_NE(vd->GetVolumeKey(desc, STREAM_MUSIC), "");

    desc->volumeBehavior_.isReady = true;
    EXPECT_NE(vd->GetVolumeKey(desc, STREAM_MUSIC), "");
    desc->volumeBehavior_.databaseVolumeName = "Test";
    EXPECT_NE(vd->GetVolumeKey(desc, STREAM_MUSIC), "");

    desc->volumeBehavior_.databaseVolumeName = "";
    EXPECT_NE(vd->GetVolumeKey(desc, STREAM_MUSIC), "");
    VolumeUtils::SetPCVolumeEnable(true);
    EXPECT_NE(vd->GetVolumeKey(desc, STREAM_RING), "");
    EXPECT_NE(vd->GetVolumeKey(desc, STREAM_MUSIC), "");
}

/**
 * @tc.name  : Test VolumeDataMaintainer.
 * @tc.number: GetMuteKeyForDatabaseVolumeName_001.
 * @tc.desc  : Test GetMuteKeyForDatabaseVolumeName API.
 */
HWTEST(VolumeDataMaintainerUnitTest, GetMuteKey, TestSize.Level4)
{
    std::shared_ptr<VolumeDataMaintainer> vd = std::make_shared<VolumeDataMaintainer>();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    EXPECT_NE(vd->GetMuteKey(desc, STREAM_RING), "");
    EXPECT_NE(vd->GetMuteKey(desc, STREAM_MUSIC), "");

    desc->volumeBehavior_.isReady = true;
    EXPECT_NE(vd->GetMuteKey(desc, STREAM_MUSIC), "");
    desc->volumeBehavior_.databaseVolumeName = "Test";
    EXPECT_NE(vd->GetMuteKey(desc, STREAM_MUSIC), "");

    desc->volumeBehavior_.databaseVolumeName = "";
    EXPECT_NE(vd->GetMuteKey(desc, STREAM_MUSIC), "");

    VolumeUtils::SetPCVolumeEnable(true);
    EXPECT_NE(vd->GetMuteKey(desc, STREAM_RING), "");
    EXPECT_NE(vd->GetMuteKey(desc, STREAM_MUSIC), "");
}

/**
 * @tc.name  : Test VolumeDataMaintainer.
 * @tc.number: LoadDeviceMuteFromDb.
 * @tc.desc  : Test GetMuteKeyForDatabaseVolumeName API.
 */
HWTEST(VolumeDataMaintainerUnitTest, LoadDeviceMuteMapFromDb, TestSize.Level4)
{
    std::shared_ptr<VolumeDataMaintainer> vd = std::make_shared<VolumeDataMaintainer>();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();

    int32_t zoneId = 0;

    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    vd->LoadDeviceMuteMapFromDb(desc, zoneId);
    desc->deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    vd->LoadDeviceMuteMapFromDb(desc, zoneId);
    desc->volumeBehavior_.isReady = true;
    vd->LoadDeviceMuteMapFromDb(desc, zoneId);
    desc->volumeBehavior_.databaseVolumeName = "Test";
    vd->LoadDeviceMuteMapFromDb(desc, zoneId);
    desc->volumeBehavior_.isReady = false;
    vd->LoadDeviceMuteMapFromDb(desc, zoneId);
    EXPECT_EQ(vd->volumeList_.size(), 0);
}

/**
 * @tc.name  : Test VolumeDataMaintainer.
 * @tc.number: SaveMuteToDb.
 * @tc.desc  : Test GetMuteKeyForDatabaseVolumeName API.
 */
HWTEST(VolumeDataMaintainerUnitTest, SaveMuteToDb, TestSize.Level4)
{
    std::shared_ptr<VolumeDataMaintainer> vd = std::make_shared<VolumeDataMaintainer>();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();

    int32_t zoneId = 0;
    EXPECT_EQ(vd->SaveMuteToDb(desc, STREAM_MUSIC, true, zoneId), ERROR);
    desc->deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    EXPECT_EQ(vd->SaveMuteToDb(desc, STREAM_MUSIC, true, zoneId), SUCCESS);
    desc->volumeBehavior_.isReady = true;
    EXPECT_EQ(vd->SaveMuteToDb(desc, STREAM_MUSIC, true, zoneId), SUCCESS);
    desc->volumeBehavior_.databaseVolumeName = "Test";
    EXPECT_EQ(vd->SaveMuteToDb(desc, STREAM_MUSIC, true, zoneId), SUCCESS);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: LegacyHighVolumeCheckTime_001.
* @tc.desc  : Test SaveLegacyHighVolumeTime and GetLegacyHighVolumeTime.
*/
HWTEST(VolumeDataMaintainerUnitTest, LegacyHighVolumeCheckTime_001, TestSize.Level4)
{
    std::shared_ptr<VolumeDataMaintainer> vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);

    int64_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    (void)vd->SaveLegacyHighVolumeTime(now);
    int64_t loadedTime = 0;
    (void)vd->GetLegacyHighVolumeTime(loadedTime);
    SUCCEED();
}

/**
 * @tc.name  : Test VolumeDataMaintainer::GetZoneKey with invalid zoneId.
 * @tc.number: VolumeDataMaintainer_GetZoneKey_001
 * @tc.desc  : zoneId <= 0 should return empty string.
 */
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_GetZoneKey_001, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);

    // zoneId == 0 -> empty
    EXPECT_EQ(vd->GetZoneKey(0), "");

    // negative zoneId -> empty
    EXPECT_EQ(vd->GetZoneKey(-1), "");

    // create an audio zone and verify GetZoneKey returns the zone name
    AudioZoneService::GetInstance().Init(DelayedSingleton<AudioPolicyServerHandler>::GetInstance(),
        std::make_shared<AudioInterruptService>());
    AudioZoneContext context;
    const std::string zoneName = "UT_VolumeZone";
    int32_t zoneId = AudioZoneService::GetInstance().CreateAudioZone(zoneName, context, 0);
    ASSERT_GT(zoneId, 0);

    EXPECT_EQ(vd->GetZoneKey(zoneId), zoneName);

    // release zone and verify key no longer available
    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId);
    EXPECT_EQ(vd->GetZoneKey(zoneId), "");

    // release zone and verify key no longer available
    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId);
    EXPECT_EQ(vd->GetZoneKey(zoneId), "");
}

static constexpr int32_t TEST_INVALID_SUBSPACE_ID = -1;
static constexpr int32_t TEST_BASE_SUBSPACE_ID = 100000;
static constexpr int32_t TEST_SECOND_SUBSPACE_ID = 100001;
static constexpr int32_t TEST_ZERO_SUBSPACE_ID = 0;
static constexpr int32_t TEST_PRIMARY_ZONE_ID = 0;

/**
 * @tc.name  : GetSubspaceKey_DefaultInit
 * @tc.number: VolumeDataMaintainer_GetSubspaceKey_001
 * @tc.desc  : Test GetSubspaceKey with default subspaceId=-1, zoneId=0 returns empty string.
 */
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_GetSubspaceKey_001, TestSize.Level1)
{
    auto vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);
    EXPECT_EQ(vd->GetSubspaceKey(TEST_PRIMARY_ZONE_ID), "");
}

/**
 * @tc.name  : GetSubspaceKey_ValidSubspaceId
 * @tc.number: VolumeDataMaintainer_GetSubspaceKey_002
 * @tc.desc  : Test GetSubspaceKey with valid subspaceId=100000, zoneId=0 returns "_100000".
 */
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_GetSubspaceKey_002, TestSize.Level1)
{
    auto vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);
    vd->SetCurrentSubspaceId(TEST_BASE_SUBSPACE_ID);
    EXPECT_EQ(vd->GetSubspaceKey(TEST_PRIMARY_ZONE_ID), "_100000");
}

/**
 * @tc.name  : GetSubspaceKey_NonPrimaryZone
 * @tc.number: VolumeDataMaintainer_GetSubspaceKey_003
 * @tc.desc  : Test GetSubspaceKey with subspaceId set but zoneId>0 returns empty string.
 */
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_GetSubspaceKey_003, TestSize.Level1)
{
    auto vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);
    vd->SetCurrentSubspaceId(TEST_BASE_SUBSPACE_ID);
    AudioZoneService::GetInstance().Init(
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance(),
        std::make_shared<AudioInterruptService>());
    AudioZoneContext context;
    int32_t zoneId = AudioZoneService::GetInstance().CreateAudioZone("UT_SubspaceZone", context, 0);
    ASSERT_GT(zoneId, 0);
    EXPECT_EQ(vd->GetSubspaceKey(zoneId), "");
    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId);
}

/**
 * @tc.name  : GetSubspaceKey_ZeroSubspaceId
 * @tc.number: VolumeDataMaintainer_GetSubspaceKey_004
 * @tc.desc  : Test GetSubspaceKey with subspaceId=0, zoneId=0 returns "_0" per >=0 condition.
 */
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_GetSubspaceKey_004, TestSize.Level1)
{
    auto vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);
    vd->SetCurrentSubspaceId(TEST_ZERO_SUBSPACE_ID);
    EXPECT_EQ(vd->GetSubspaceKey(TEST_PRIMARY_ZONE_ID), "_0");
}

/**
 * @tc.name  : GetSubspaceKey_SubspaceIdThenReset
 * @tc.number: VolumeDataMaintainer_GetSubspaceKey_005
 * @tc.desc  : Test GetSubspaceKey returns empty after subspaceId reset to -1.
 */
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_GetSubspaceKey_005, TestSize.Level1)
{
    auto vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);
    vd->SetCurrentSubspaceId(TEST_BASE_SUBSPACE_ID);
    EXPECT_EQ(vd->GetSubspaceKey(TEST_PRIMARY_ZONE_ID), "_100000");
    vd->SetCurrentSubspaceId(TEST_INVALID_SUBSPACE_ID);
    EXPECT_EQ(vd->GetSubspaceKey(TEST_PRIMARY_ZONE_ID), "");
}

/**
 * @tc.name  : SetAndGetCurrentSubspaceId
 * @tc.number: VolumeDataMaintainer_SubspaceId_001
 * @tc.desc  : Test SetCurrentSubspaceId and GetCurrentSubspaceId round-trip.
 */
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_SubspaceId_001, TestSize.Level1)
{
    auto vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);
    EXPECT_EQ(vd->GetCurrentSubspaceId(), TEST_INVALID_SUBSPACE_ID);
    vd->SetCurrentSubspaceId(TEST_BASE_SUBSPACE_ID);
    EXPECT_EQ(vd->GetCurrentSubspaceId(), TEST_BASE_SUBSPACE_ID);
    vd->SetCurrentSubspaceId(TEST_SECOND_SUBSPACE_ID);
    EXPECT_EQ(vd->GetCurrentSubspaceId(), TEST_SECOND_SUBSPACE_ID);
    vd->SetCurrentSubspaceId(TEST_INVALID_SUBSPACE_ID);
    EXPECT_EQ(vd->GetCurrentSubspaceId(), TEST_INVALID_SUBSPACE_ID);
}

/**
 * @tc.name  : VolumeLevelKeyFormat_WithSubspace
 * @tc.number: VolumeDataMaintainer_VolumeKeyFormat_001
 * @tc.desc  : Test GetVolumeLevelInfo key format includes subspace suffix for zoneId=0.
 */
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_VolumeKeyFormat_001, TestSize.Level1)
{
    auto vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);
    vd->SetCurrentSubspaceId(TEST_BASE_SUBSPACE_ID);
    vd->SetDataShareReady(true);
    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    auto info = vd->GetVolumeLevelInfo(desc, STREAM_MUSIC, TEST_PRIMARY_ZONE_ID);
    EXPECT_TRUE(info.key.find("_100000") != std::string::npos);
}

/**
 * @tc.name  : VolumeLevelKeyFormat_NoSubspace
 * @tc.number: VolumeDataMaintainer_VolumeKeyFormat_002
 * @tc.desc  : Test GetVolumeLevelInfo key format without subspace suffix when subspaceId=-1.
 */
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_VolumeKeyFormat_002, TestSize.Level1)
{
    auto vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);
    vd->SetDataShareReady(true);
    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    auto info = vd->GetVolumeLevelInfo(desc, STREAM_MUSIC, TEST_PRIMARY_ZONE_ID);
    EXPECT_TRUE(info.key.find("_100000") == std::string::npos);
    EXPECT_TRUE(info.key.find("music_volume_builtin") != std::string::npos);
}

/**
 * @tc.name  : VolumeDegreeKeyFormat_WithSubspace
 * @tc.number: VolumeDataMaintainer_VolumeKeyFormat_003
 * @tc.desc  : Test GetVolumeDegreeInfo key format includes subspace suffix for zoneId=0.
 */
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_VolumeKeyFormat_003, TestSize.Level1)
{
    auto vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);
    vd->SetCurrentSubspaceId(TEST_BASE_SUBSPACE_ID);
    vd->SetDataShareReady(true);
    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    auto info = vd->GetVolumeDegreeInfo(desc, STREAM_MUSIC, TEST_PRIMARY_ZONE_ID);
    EXPECT_TRUE(info.key.find("_100000") != std::string::npos);
}

/**
 * @tc.name  : VolumeDegreeKeyFormat_NoSubspace
 * @tc.number: VolumeDataMaintainer_VolumeKeyFormat_004
 * @tc.desc  : Test GetVolumeDegreeInfo key format without subspace suffix when subspaceId=-1.
 */
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_VolumeKeyFormat_004, TestSize.Level1)
{
    auto vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);
    vd->SetDataShareReady(true);
    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    auto info = vd->GetVolumeDegreeInfo(desc, STREAM_MUSIC, TEST_PRIMARY_ZONE_ID);
    EXPECT_TRUE(info.key.find("_100000") == std::string::npos);
}

/**
 * @tc.name  : VolumeLevelKeyFormat_NonPrimaryZone
 * @tc.number: VolumeDataMaintainer_VolumeKeyFormat_005
 * @tc.desc  : Test GetVolumeLevelInfo key format for zoneId>0 does not include subspace suffix.
 */
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_VolumeKeyFormat_005, TestSize.Level1)
{
    auto vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);
    vd->SetCurrentSubspaceId(TEST_BASE_SUBSPACE_ID);
    vd->SetDataShareReady(true);
    AudioZoneService::GetInstance().Init(
        DelayedSingleton<AudioPolicyServerHandler>::GetInstance(),
        std::make_shared<AudioInterruptService>());
    AudioZoneContext context;
    int32_t zoneId = AudioZoneService::GetInstance().CreateAudioZone("UT_KeyZone", context, 0);
    ASSERT_GT(zoneId, 0);
    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    auto info = vd->GetVolumeLevelInfo(desc, STREAM_MUSIC, zoneId);
    EXPECT_TRUE(info.key.find("_100000") == std::string::npos);
    AudioZoneService::GetInstance().ReleaseAudioZone(zoneId);
}

/**
 * @tc.name  : VolumeValueMapKey_NoSubspaceInMemory
 * @tc.number: VolumeDataMaintainer_VolumeMapKey_001
 * @tc.desc  : Test GetVolumeValueMapKey for zoneId=0 does not include subspaceId.
 */
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_VolumeMapKey_001, TestSize.Level1)
{
    auto vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);
    vd->SetCurrentSubspaceId(TEST_BASE_SUBSPACE_ID);
    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    std::string mapKey = vd->GetVolumeValueMapKey(desc, TEST_PRIMARY_ZONE_ID);
    EXPECT_TRUE(mapKey.find("100000") == std::string::npos);
    EXPECT_TRUE(mapKey.find("zone__") != std::string::npos || mapKey.find("deviceGroup_") != std::string::npos);
}
/* ==================== SetAppVolumeMuteAndHideVolumeBar tests ==================== */

/**
 * @tc.name  : Test SetAppVolumeMuteAndHideVolumeBar with muted=true.
 * @tc.number: VolumeDataMaintainer_SetAppVolumeMuteAndHideVolumeBar_001
 * @tc.desc  : Set mute-and-hide to true. Map should contain entry for appUid.
 */
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_SetAppVolumeMuteAndHideVolumeBar_001,
    TestSize.Level1)
{
    auto vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);
    constexpr int32_t TEST_APP_UID = 1001;
    vd->SetAppVolumeMuteAndHideVolumeBar(TEST_APP_UID, true);
    EXPECT_TRUE(vd->GetAppMuteAndHideVolumeBar(TEST_APP_UID));
}

/**
 * @tc.name  : Test SetAppVolumeMuteAndHideVolumeBar with muted=false, uid not found.
 * @tc.number: VolumeDataMaintainer_SetAppVolumeMuteAndHideVolumeBar_002
 * @tc.desc  : Unmute an appUid that was never muted. No-op, should not crash.
 */
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_SetAppVolumeMuteAndHideVolumeBar_002,
    TestSize.Level1)
{
    auto vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);
    constexpr int32_t TEST_APP_UID_NOT_EXIST = 9999;
    vd->SetAppVolumeMuteAndHideVolumeBar(TEST_APP_UID_NOT_EXIST, false);
    EXPECT_FALSE(vd->GetAppMuteAndHideVolumeBar(TEST_APP_UID_NOT_EXIST));
}

/**
 * @tc.name  : Test SetAppVolumeMuteAndHideVolumeBar with muted=false, uid found, not empty after erase.
 * @tc.number: VolumeDataMaintainer_SetAppVolumeMuteAndHideVolumeBar_003
 * @tc.desc  : After mute then unmute, if multiple callers exist, entry should remain.
 */
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_SetAppVolumeMuteAndHideVolumeBar_003,
    TestSize.Level1)
{
    auto vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);
    constexpr int32_t TEST_APP_UID = 1002;
    vd->SetAppVolumeMuteAndHideVolumeBar(TEST_APP_UID, true);
    vd->SetAppVolumeMuteAndHideVolumeBar(TEST_APP_UID, false);
    EXPECT_FALSE(vd->GetAppMuteAndHideVolumeBar(TEST_APP_UID));
}

/**
 * @tc.name  : Test SetAppVolumeMuteAndHideVolumeBar with muted=false, uid found, empty after erase.
 * @tc.number: VolumeDataMaintainer_SetAppVolumeMuteAndHideVolumeBar_004
 * @tc.desc  : After mute then unmute single caller, entry should be removed from map.
 */
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_SetAppVolumeMuteAndHideVolumeBar_004,
    TestSize.Level1)
{
    auto vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);
    constexpr int32_t TEST_APP_UID = 1003;
    vd->SetAppVolumeMuteAndHideVolumeBar(TEST_APP_UID, true);
    EXPECT_TRUE(vd->GetAppMuteAndHideVolumeBar(TEST_APP_UID));
    vd->SetAppVolumeMuteAndHideVolumeBar(TEST_APP_UID, false);
    EXPECT_FALSE(vd->GetAppMuteAndHideVolumeBar(TEST_APP_UID));
}

/* ==================== GetAppMuteAndHideVolumeBar tests ==================== */

/**
 * @tc.name  : Test GetAppMuteAndHideVolumeBar with uid not found.
 * @tc.number: VolumeDataMaintainer_GetAppMuteAndHideVolumeBar_001
 * @tc.desc  : Query appUid that was never muted. Should return false.
 */
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_GetAppMuteAndHideVolumeBar_001,
    TestSize.Level1)
{
    auto vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);
    constexpr int32_t TEST_APP_UID_NOT_FOUND = 8888;
    EXPECT_FALSE(vd->GetAppMuteAndHideVolumeBar(TEST_APP_UID_NOT_FOUND));
}

/**
 * @tc.name  : Test GetAppMuteAndHideVolumeBar with uid found and not empty.
 * @tc.number: VolumeDataMaintainer_GetAppMuteAndHideVolumeBar_002
 * @tc.desc  : After setting mute-and-hide, query should return true.
 */
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_GetAppMuteAndHideVolumeBar_002,
    TestSize.Level1)
{
    auto vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);
    constexpr int32_t TEST_APP_UID = 1004;
    vd->SetAppVolumeMuteAndHideVolumeBar(TEST_APP_UID, true);
    EXPECT_TRUE(vd->GetAppMuteAndHideVolumeBar(TEST_APP_UID));
}

/* ==================== GetAppMuteAndHideOwned tests ==================== */

/**
 * @tc.name  : Test GetAppMuteAndHideOwned with uid not found.
 * @tc.number: VolumeDataMaintainer_GetAppMuteAndHideOwned_001
 * @tc.desc  : Query owned mute for appUid that was never muted. Should return false.
 */
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_GetAppMuteAndHideOwned_001,
    TestSize.Level1)
{
    auto vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);
    constexpr int32_t TEST_APP_UID_NOT_FOUND = 7777;
    EXPECT_FALSE(vd->GetAppMuteAndHideOwned(TEST_APP_UID_NOT_FOUND));
}

/**
 * @tc.name  : Test GetAppMuteAndHideOwned with uid found and owned.
 * @tc.number: VolumeDataMaintainer_GetAppMuteAndHideOwned_002
 * @tc.desc  : After setting mute-and-hide, owned query should return true (same caller UID).
 */
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_GetAppMuteAndHideOwned_002,
    TestSize.Level1)
{
    auto vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);
    constexpr int32_t TEST_APP_UID = 1005;
    vd->SetAppVolumeMuteAndHideVolumeBar(TEST_APP_UID, true);
    EXPECT_TRUE(vd->GetAppMuteAndHideOwned(TEST_APP_UID));
}

/* ==================== GetAppMuteAggregate tests (A || B short-circuit) ==================== */

/**
 * @tc.name  : Test GetAppMuteAggregate with regular mute true (A=true, short-circuit).
 * @tc.number: VolumeDataMaintainer_GetAppMuteAggregate_001
 * @tc.desc  : Regular mute is true, hide mute not evaluated. Aggregate should be true.
 */
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_GetAppMuteAggregate_001,
    TestSize.Level1)
{
    auto vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);
    constexpr int32_t TEST_APP_UID = 1006;
    vd->SetAppVolumeMuted(TEST_APP_UID, true);
    EXPECT_TRUE(vd->GetAppMuteAggregate(TEST_APP_UID));
}

/**
 * @tc.name  : Test GetAppMuteAggregate with regular false, hide true (A=false, B=true).
 * @tc.number: VolumeDataMaintainer_GetAppMuteAggregate_002
 * @tc.desc  : Regular mute false, hide mute true. Aggregate should be true.
 */
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_GetAppMuteAggregate_002,
    TestSize.Level1)
{
    auto vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);
    constexpr int32_t TEST_APP_UID = 1007;
    vd->SetAppVolumeMuteAndHideVolumeBar(TEST_APP_UID, true);
    EXPECT_TRUE(vd->GetAppMuteAggregate(TEST_APP_UID));
}

/**
 * @tc.name  : Test GetAppMuteAggregate with both false (A=false, B=false).
 * @tc.number: VolumeDataMaintainer_GetAppMuteAggregate_003
 * @tc.desc  : Neither regular nor hide mute set. Aggregate should be false.
 */
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_GetAppMuteAggregate_003,
    TestSize.Level1)
{
    auto vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);
    constexpr int32_t TEST_APP_UID = 1008;
    EXPECT_FALSE(vd->GetAppMuteAggregate(TEST_APP_UID));
}

/* ==================== GetAppMuteOwnedAggregate tests (A || B short-circuit) ==================== */

/**
 * @tc.name  : Test GetAppMuteOwnedAggregate with regular mute true (A=true, short-circuit).
 * @tc.number: VolumeDataMaintainer_GetAppMuteOwnedAggregate_001
 * @tc.desc  : Regular owned mute true, hide not evaluated. Aggregate should be true.
 */
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_GetAppMuteOwnedAggregate_001,
    TestSize.Level1)
{
    auto vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);
    constexpr int32_t TEST_APP_UID = 1009;
    vd->SetAppVolumeMuted(TEST_APP_UID, true);
    EXPECT_TRUE(vd->GetAppMuteOwnedAggregate(TEST_APP_UID));
}

/**
 * @tc.name  : Test GetAppMuteOwnedAggregate with regular false, hide true (A=false, B=true).
 * @tc.number: VolumeDataMaintainer_GetAppMuteOwnedAggregate_002
 * @tc.desc  : Regular owned mute false, hide mute true. Aggregate should be true.
 */
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_GetAppMuteOwnedAggregate_002,
    TestSize.Level1)
{
    auto vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);
    constexpr int32_t TEST_APP_UID = 1010;
    vd->SetAppVolumeMuteAndHideVolumeBar(TEST_APP_UID, true);
    EXPECT_TRUE(vd->GetAppMuteOwnedAggregate(TEST_APP_UID));
}

/**
 * @tc.name  : Test GetAppMuteOwnedAggregate with both false (A=false, B=false).
 * @tc.number: VolumeDataMaintainer_GetAppMuteOwnedAggregate_003
 * @tc.desc  : Neither regular owned nor hide mute set. Aggregate should be false.
 */
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_GetAppMuteOwnedAggregate_003,
    TestSize.Level1)
{
    auto vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);
    constexpr int32_t TEST_APP_UID = 1011;
    EXPECT_FALSE(vd->GetAppMuteOwnedAggregate(TEST_APP_UID));
}

/* ==================== ClearAppMuteAndHideByOwnerUid tests ==================== */

/**
 * @tc.name  : Test ClearAppMuteAndHideByOwnerUid with empty map.
 * @tc.number: VolumeDataMaintainer_ClearAppMuteAndHideByOwnerUid_001
 * @tc.desc  : Clear with no entries. Loop does not execute, affectedAppUids empty.
 */
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_ClearAppMuteAndHideByOwnerUid_001,
    TestSize.Level1)
{
    auto vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);
    constexpr int32_t TEST_CALLER_UID = 5000;
    std::vector<int32_t> affectedUids;
    vd->ClearAppMuteAndHideByOwnerUid(TEST_CALLER_UID, affectedUids);
    EXPECT_TRUE(affectedUids.empty());
}

/**
 * @tc.name  : Test ClearAppMuteAndHideByOwnerUid with ownedAppUid not in any callerMap.
 * @tc.number: VolumeDataMaintainer_ClearAppMuteAndHideByOwnerUid_002
 * @tc.desc  : Entry exists but callerUid not present. Not erased, affected empty.
 */
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_ClearAppMuteAndHideByOwnerUid_002,
    TestSize.Level1)
{
    auto vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);
    constexpr int32_t TEST_APP_UID = 1012;
    constexpr int32_t TEST_CALLER_UID_NOT_IN_MAP = 9999;
    vd->SetAppVolumeMuteAndHideVolumeBar(TEST_APP_UID, true);
    std::vector<int32_t> affectedUids;
    vd->ClearAppMuteAndHideByOwnerUid(TEST_CALLER_UID_NOT_IN_MAP, affectedUids);
    EXPECT_TRUE(affectedUids.empty());
    EXPECT_TRUE(vd->GetAppMuteAndHideVolumeBar(TEST_APP_UID));
}

/**
 * @tc.name  : Test ClearAppMuteAndHideByOwnerUid with ownedAppUid in callerMap, empty after erase.
 * @tc.number: VolumeDataMaintainer_ClearAppMuteAndHideByOwnerUid_003
 * @tc.desc  : Single caller muted an app. Clear removes entry, appUid in affected list.
 */
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_ClearAppMuteAndHideByOwnerUid_003,
    TestSize.Level1)
{
    auto vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);
    constexpr int32_t TEST_APP_UID = 1013;
    int32_t testCallerUid = IPCSkeleton::GetCallingUid();
    vd->SetAppVolumeMuteAndHideVolumeBar(TEST_APP_UID, true);
    std::vector<int32_t> affectedUids;
    vd->ClearAppMuteAndHideByOwnerUid(testCallerUid, affectedUids);
    EXPECT_EQ(affectedUids.size(), 1U);
    EXPECT_EQ(affectedUids[0], TEST_APP_UID);
    EXPECT_FALSE(vd->GetAppMuteAndHideVolumeBar(TEST_APP_UID));
}

/**
 * @tc.name  : Test ClearAppMuteAndHideByOwnerUid multiple apps affected.
 * @tc.number: VolumeDataMaintainer_ClearAppMuteAndHideByOwnerUid_004
 * @tc.desc  : Multiple apps muted by same caller. Clear should affect all.
 */
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_ClearAppMuteAndHideByOwnerUid_004,
    TestSize.Level1)
{
    auto vd = std::make_shared<VolumeDataMaintainer>();
    ASSERT_NE(vd, nullptr);
    constexpr int32_t TEST_APP_UID_A = 1014;
    constexpr int32_t TEST_APP_UID_B = 1015;
    int32_t testCallerUid = IPCSkeleton::GetCallingUid();
    vd->SetAppVolumeMuteAndHideVolumeBar(TEST_APP_UID_A, true);
    vd->SetAppVolumeMuteAndHideVolumeBar(TEST_APP_UID_B, true);
    std::vector<int32_t> affectedUids;
    vd->ClearAppMuteAndHideByOwnerUid(testCallerUid, affectedUids);
    EXPECT_EQ(affectedUids.size(), 2U);
    EXPECT_FALSE(vd->GetAppMuteAndHideVolumeBar(TEST_APP_UID_A));
    EXPECT_FALSE(vd->GetAppMuteAndHideVolumeBar(TEST_APP_UID_B));
}
} // AudioStandardnamespace
} // OHOSnamespace
