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

#include "system_ability_definition.h"
#include "audio_errors.h"
#include "audio_utils.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void VolumeDataMaintainerUnitTest::SetUpTestCase(void) {}
void VolumeDataMaintainerUnitTest::TearDownTestCase(void) {}
void VolumeDataMaintainerUnitTest::SetUp(void) {}
void VolumeDataMaintainerUnitTest::TearDown(void) {}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_001.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_001, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    DeviceType typeRet = DEVICE_TYPE_NONE;
    AudioStreamType streamTypeRet = STREAM_DEFAULT;
    int32_t volumeLevelRet = 0;
    auto ret = volumeDataMaintainerRet->SaveVolume(typeRet, streamTypeRet, volumeLevelRet);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_002.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_002, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    DeviceType typeRet = DEVICE_TYPE_DP;
    AudioStreamType streamTypeRet = STREAM_MUSIC;
    int32_t volumeLevelRet = 0;
    auto ret = volumeDataMaintainerRet->SaveVolume(typeRet, streamTypeRet, volumeLevelRet);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_003.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_003, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    DeviceType deviceTypeRet = DEVICE_TYPE_NONE;
    AudioStreamType streamTypeRet = STREAM_DEFAULT;
    auto ret = volumeDataMaintainerRet->GetVolume(deviceTypeRet, streamTypeRet);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_004.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_004, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    DeviceType deviceTypeRet = DEVICE_TYPE_NONE;
    AudioStreamType streamTypeRet = STREAM_RING;
    bool muteStatusRet = false;
    auto ret = volumeDataMaintainerRet->SaveMuteStatus(deviceTypeRet, streamTypeRet, muteStatusRet);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_005.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_005, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    DeviceType deviceTypeRet = DEVICE_TYPE_DP;
    AudioStreamType streamTypeRet = STREAM_MUSIC;
    bool muteStatusRet = false;
    auto ret = volumeDataMaintainerRet->SaveMuteStatus(deviceTypeRet, streamTypeRet, muteStatusRet);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_006.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_006, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    DeviceType deviceTypeRet = DEVICE_TYPE_DP;
    AudioStreamType streamTypeRet = STREAM_DEFAULT;
    auto ret = volumeDataMaintainerRet->GetMuteStatusInternal(deviceTypeRet, streamTypeRet);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_007.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_007, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    DeviceType deviceTypeRet = DEVICE_TYPE_DP;
    AudioStreamType streamTypeRet = STREAM_MUSIC;
    auto ret = volumeDataMaintainerRet->GetMuteStatusInternal(deviceTypeRet, streamTypeRet);
    EXPECT_EQ(ret, false);
}

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
    EXPECT_EQ(ret, false);

    deviceTypeRet = DEVICE_TYPE_BLUETOOTH_A2DP;
    ret = volumeDataMaintainerRet->GetSafeStatus(deviceTypeRet, safeStatusRet);
    EXPECT_EQ(ret, false);
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
    EXPECT_EQ(ret, false);

    deviceTypeRet = DEVICE_TYPE_USB_ARM_HEADSET;
    ret = volumeDataMaintainerRet->SaveSafeVolumeTime(deviceTypeRet, timeRet);
    EXPECT_EQ(ret, false);

    deviceTypeRet = DEVICE_TYPE_NONE;
    ret = volumeDataMaintainerRet->SaveSafeVolumeTime(deviceTypeRet, timeRet);
    EXPECT_EQ(ret, false);

    deviceTypeRet = DEVICE_TYPE_BLUETOOTH_A2DP;
    ret = volumeDataMaintainerRet->SaveSafeVolumeTime(deviceTypeRet, timeRet);
    EXPECT_EQ(ret, false);
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
    EXPECT_EQ(ret, false);

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
    EXPECT_EQ(ret, false);

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

    auto ret = volumeDataMaintainerRet->GetDeviceTypeName(deviceTypeRet);
    std::string typeRet = "_wireless";
    EXPECT_EQ(ret, typeRet);

    deviceTypeRet = DEVICE_TYPE_USB_ARM_HEADSET;
    ret = volumeDataMaintainerRet->GetDeviceTypeName(deviceTypeRet);
    typeRet = "_wired";
    EXPECT_EQ(ret, typeRet);

    deviceTypeRet = DEVICE_TYPE_REMOTE_CAST;
    ret = volumeDataMaintainerRet->GetDeviceTypeName(deviceTypeRet);
    typeRet = "_remote_cast";
    EXPECT_EQ(ret, typeRet);
    
    deviceTypeRet = DEVICE_TYPE_NEARLINK;
    ret = volumeDataMaintainerRet->GetDeviceTypeName(deviceTypeRet);
    typeRet = "_wireless";
    EXPECT_EQ(ret, typeRet);

    deviceTypeRet = DEVICE_TYPE_NONE;
    ret = volumeDataMaintainerRet->GetDeviceTypeName(deviceTypeRet);
    typeRet = "";
    EXPECT_EQ(ret, typeRet);

    deviceTypeRet = DEVICE_TYPE_HEARING_AID;
    ret = volumeDataMaintainerRet->GetDeviceTypeName(deviceTypeRet);
    typeRet = "_hearing_aid";
    EXPECT_EQ(ret, typeRet);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_017.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_017, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    DeviceType deviceTypeRet = DEVICE_TYPE_BLUETOOTH_SCO;
    AudioStreamType streamTypeRet = STREAM_DEFAULT;

    auto ret = volumeDataMaintainerRet->GetVolumeKeyForDataShare(deviceTypeRet, streamTypeRet);
    std::string typeRet = "";
    EXPECT_EQ(ret, typeRet);

    streamTypeRet = STREAM_VOICE_ASSISTANT;
    ret = volumeDataMaintainerRet->GetVolumeKeyForDataShare(deviceTypeRet, streamTypeRet);
    typeRet = "voice_assistant_volume_wireless_sco";
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
    EXPECT_EQ(ret, false);

    bool isMuteRet2 = true;
    ret = volumeDataMaintainerRet->GetMicMuteState(isMuteRet2);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test VolumeDataMaintainer SaveMuteStatusInternal when muteKey is empty.
* @tc.number: VolumeDataMaintainer_SaveMuteStatusInternal_001.
* @tc.desc  : Test VolumeDataMaintainer API when GetMuteKeyForDataShare returns empty string.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_SaveMuteStatusInternal_001, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainer = std::make_shared<VolumeDataMaintainer>();
    DeviceType deviceType = DEVICE_TYPE_NONE;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    bool muteStatus = true;
    bool result = volumeDataMaintainer->SaveMuteStatusInternal(deviceType, streamType, muteStatus);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : Test VolumeDataMaintainer SaveMuteStatusInternal when muteKey is valid.
* @tc.number: VolumeDataMaintainer_SaveMuteStatusInternal_002.
* @tc.desc  : Test VolumeDataMaintainer API when GetMuteKeyForDataShare returns valid string and save succeeds.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_SaveMuteStatusInternal_002, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainer = std::make_shared<VolumeDataMaintainer>();
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_SCO;
    AudioStreamType streamType = AudioStreamType::STREAM_MUSIC;
    bool muteStatus = true;
    bool result = volumeDataMaintainer->SaveMuteStatusInternal(deviceType, streamType, muteStatus);
    EXPECT_TRUE(result);
}

/**
* @tc.name  : Test VolumeDataMaintainer SaveMuteStatusInternal when muteKey is valid.
* @tc.number: VolumeDataMaintainer_SaveMuteStatusInternal_003.
* @tc.desc  : Test VolumeDataMaintainer API when GetMuteKeyForDataShare returns valid string and save succeeds.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_SaveMuteStatusInternal_003, TestSize.Level3)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainer = std::make_shared<VolumeDataMaintainer>();
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_SCO;
    AudioStreamType streamType = AudioStreamType::STREAM_MUSIC;
    bool muteStatus = true;
    VolumeUtils::SetPCVolumeEnable(true);
    bool result = volumeDataMaintainer->SaveMuteStatusInternal(deviceType, streamType, muteStatus);
    EXPECT_TRUE(result);
}

/**
* @tc.name  : Test VolumeDataMaintainer SaveMuteStatusInternal when muteKey is valid.
* @tc.number: VolumeDataMaintainer_SaveMuteStatusInternal_004.
* @tc.desc  : Test VolumeDataMaintainer API when GetMuteKeyForDataShare returns valid string and save succeeds.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_SaveMuteStatusInternal_004, TestSize.Level3)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainer = std::make_shared<VolumeDataMaintainer>();
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_SCO;
    AudioStreamType streamType = AudioStreamType::STREAM_VOICE_ASSISTANT;
    bool muteStatus = true;
    VolumeUtils::SetPCVolumeEnable(true);
    bool result = volumeDataMaintainer->SaveMuteStatusInternal(deviceType, streamType, muteStatus);
    EXPECT_TRUE(result);
}


/**
* @tc.name  : Test VolumeDataMaintainer SaveMuteStatusInternal when muteKey is valid.
* @tc.number: VolumeDataMaintainer_SaveMuteStatusInternal_005.
* @tc.desc  : Test VolumeDataMaintainer API when GetMuteKeyForDataShare returns valid string and save succeeds.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainer_SaveMuteStatusInternal_005, TestSize.Level3)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainer = std::make_shared<VolumeDataMaintainer>();
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;
    AudioStreamType streamType = AudioStreamType::STREAM_VOICE_ASSISTANT;
    bool muteStatus = true;
    VolumeUtils::SetPCVolumeEnable(true);
    bool result = volumeDataMaintainer->SaveMuteStatusInternal(deviceType, streamType, muteStatus);
    EXPECT_TRUE(result);
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
 * @tc.name  : Test VolumeDataMaintainer GetDeviceVolumeInternal when volumeKey is empty.
 * @tc.number: GetDeviceVolumeInternal_001
 * @tc.desc  : Test VolumeDataMaintainer API when GetVolumeKeyForDataShare returns empty string.
 */
HWTEST(VolumeDataMaintainerUnitTest, GetDeviceVolumeInternal_001, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainer = std::make_shared<VolumeDataMaintainer>();
    DeviceType deviceType = DEVICE_TYPE_INVALID;
    AudioStreamType streamType = STREAM_MUSIC;
    int32_t volumeValue = volumeDataMaintainer->GetDeviceVolumeInternal(deviceType, streamType);
    EXPECT_EQ(volumeValue, 0);
}

/**
 * @tc.name  : Test VolumeDataMaintainer GetDeviceVolumeInternal when GetIntValue fails.
 * @tc.number: GetDeviceVolumeInternal_002
 * @tc.desc  : Test VolumeDataMaintainer API when GetIntValue returns failure.
 */
HWTEST(VolumeDataMaintainerUnitTest, GetDeviceVolumeInternal_002, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainer = std::make_shared<VolumeDataMaintainer>();
    DeviceType deviceType = DEVICE_TYPE_WIRED_HEADSET;
    AudioStreamType streamType = STREAM_MUSIC;
    int32_t volumeValue = volumeDataMaintainer->GetDeviceVolumeInternal(deviceType, streamType);
    EXPECT_EQ(volumeValue, 0);
}

/**
 * @tc.name  : Test VolumeDataMaintainer GetDeviceVolumeInternal when GetIntValue fails.
 * @tc.number: GetDeviceVolumeInternal_003
 * @tc.desc  : Test VolumeDataMaintainer API when GetIntValue returns failure.
 */
HWTEST(VolumeDataMaintainerUnitTest, GetDeviceVolumeInternal_003, TestSize.Level3)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainer = std::make_shared<VolumeDataMaintainer>();
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_SCO;
    AudioStreamType streamType = STREAM_MUSIC;
    VolumeUtils::SetPCVolumeEnable(true);
    int32_t volumeValue = volumeDataMaintainer->GetDeviceVolumeInternal(deviceType, streamType);
    EXPECT_EQ(volumeValue, 0);
}

/**
 * @tc.name  : Test VolumeDataMaintainer GetDeviceVolumeInternal when GetIntValue fails.
 * @tc.number: GetDeviceVolumeInternal_004
 * @tc.desc  : Test VolumeDataMaintainer API when GetIntValue returns failure.
 */
HWTEST(VolumeDataMaintainerUnitTest, GetDeviceVolumeInternal_004, TestSize.Level3)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainer = std::make_shared<VolumeDataMaintainer>();
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_SCO;
    AudioStreamType streamType = STREAM_VOICE_ASSISTANT;
    VolumeUtils::SetPCVolumeEnable(true);
    int32_t volumeValue = volumeDataMaintainer->GetDeviceVolumeInternal(deviceType, streamType);
    EXPECT_EQ(volumeValue, 0);
}

/**
 * @tc.name  : Test VolumeDataMaintainer GetDeviceVolumeInternal when GetIntValue fails.
 * @tc.number: GetDeviceVolumeInternal_005
 * @tc.desc  : Test VolumeDataMaintainer API when GetIntValue returns failure.
 */
HWTEST(VolumeDataMaintainerUnitTest, GetDeviceVolumeInternal_005, TestSize.Level3)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainer = std::make_shared<VolumeDataMaintainer>();
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;
    AudioStreamType streamType = STREAM_VOICE_ASSISTANT;
    VolumeUtils::SetPCVolumeEnable(true);
    int32_t volumeValue = volumeDataMaintainer->GetDeviceVolumeInternal(deviceType, streamType);
    EXPECT_EQ(volumeValue, 0);
}

/**
 * @tc.name  : Test VolumeDataMaintainer SaveMuteStatusInternal when muteKey is empty.
 * @tc.number: SaveMuteStatusInternal_001
 * @tc.desc  : Test VolumeDataMaintainer API when GetMuteKeyForDataShare returns empty string.
 */
HWTEST(VolumeDataMaintainerUnitTest, SaveMuteStatusInternal_001, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainer = std::make_shared<VolumeDataMaintainer>();
    std::lock_guard<ffrt::mutex> lock(volumeDataMaintainer->volumeMutex_);
    volumeDataMaintainer->volumeLevelMap_.clear();
    volumeDataMaintainer->appVolumeLevelMap_.clear();
    volumeDataMaintainer->appMuteStatusMap_.clear();

    DeviceType deviceType = DEVICE_TYPE_INVALID;
    AudioStreamType streamType = STREAM_MUSIC;
    bool muteStatus = true;
    bool result = volumeDataMaintainer->SaveMuteStatusInternal(deviceType, streamType, muteStatus);
    EXPECT_FALSE(result);
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
    volumeDataMaintainer->volumeLevelMap_.clear();
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
    volumeDataMaintainer->volumeLevelMap_.clear();
    volumeDataMaintainer->appVolumeLevelMap_.clear();
    volumeDataMaintainer->appMuteStatusMap_.clear();

    DeviceType deviceType = DEVICE_TYPE_USB_HEADSET;
    int32_t volume = 5;
    bool result = volumeDataMaintainer->SetRestoreVolumeLevel(deviceType, volume);
    EXPECT_FALSE(result);
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
    volumeDataMaintainer->volumeLevelMap_.clear();
    volumeDataMaintainer->appVolumeLevelMap_.clear();
    volumeDataMaintainer->appMuteStatusMap_.clear();

    DeviceType deviceType = DEVICE_TYPE_USB_ARM_HEADSET;
    int32_t volume = 5;
    bool result = volumeDataMaintainer->SetRestoreVolumeLevel(deviceType, volume);
    EXPECT_FALSE(result);
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
    volumeDataMaintainer->volumeLevelMap_.clear();
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
    volumeDataMaintainer->volumeLevelMap_.clear();
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
    volumeDataMaintainer->volumeLevelMap_.clear();
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
    EXPECT_EQ(ret, false);

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
 * @tc.number: SaveMuteStatusWithDatabaseVolumeName_001.
 * @tc.desc  : Test SaveMuteStatusWithDatabaseVolumeName API.
 */
HWTEST(VolumeDataMaintainerUnitTest, SaveMuteStatusWithDatabaseVolumeName_001, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    std::string databaseVolumeName = "TestName";
    AudioStreamType streamType = STREAM_MUSIC;
    bool mute = true;
    auto ret = volumeDataMaintainerRet->SaveMuteStatusWithDatabaseVolumeName(databaseVolumeName, streamType, mute);
    EXPECT_EQ(ret, false);

    mute = false;
    ret = volumeDataMaintainerRet->SaveMuteStatusWithDatabaseVolumeName(databaseVolumeName, streamType, mute);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test VolumeDataMaintainer.
 * @tc.number: SaveMuteStatusWithDatabaseVolumeName_002.
 * @tc.desc  : Test SaveMuteStatusWithDatabaseVolumeName API.
 */
HWTEST(VolumeDataMaintainerUnitTest, SaveMuteStatusWithDatabaseVolumeName_002, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    std::string databaseVolumeName = "TestName";
    AudioStreamType streamType = STREAM_RING;
    bool mute = true;
    auto ret = volumeDataMaintainerRet->SaveMuteStatusWithDatabaseVolumeName(databaseVolumeName, streamType, mute);
    EXPECT_EQ(ret, false);

    mute = false;
    ret = volumeDataMaintainerRet->SaveMuteStatusWithDatabaseVolumeName(databaseVolumeName, streamType, mute);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test VolumeDataMaintainer.
 * @tc.number: SaveMuteStatusWithDatabaseVolumeName_003.
 * @tc.desc  : Test SaveMuteStatusWithDatabaseVolumeName API.
 */
HWTEST(VolumeDataMaintainerUnitTest, SaveMuteStatusWithDatabaseVolumeName_003, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    std::string databaseVolumeName = "TestName";
    AudioStreamType streamType = STREAM_VOICE_CALL;
    bool mute = true;
    auto ret = volumeDataMaintainerRet->SaveMuteStatusWithDatabaseVolumeName(databaseVolumeName, streamType, mute);
    EXPECT_EQ(ret, false);

    mute = false;
    ret = volumeDataMaintainerRet->SaveMuteStatusWithDatabaseVolumeName(databaseVolumeName, streamType, mute);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test VolumeDataMaintainer.
 * @tc.number: SaveMuteStatusWithDatabaseVolumeName_004.
 * @tc.desc  : Test SaveMuteStatusWithDatabaseVolumeName API.
 */
HWTEST(VolumeDataMaintainerUnitTest, SaveMuteStatusWithDatabaseVolumeName_004, TestSize.Level4)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    std::string databaseVolumeName = "TestName";
    AudioStreamType streamType = STREAM_DEFAULT;
    bool mute = true;
    VolumeUtils::SetPCVolumeEnable(false);
    auto ret = volumeDataMaintainerRet->SaveMuteStatusWithDatabaseVolumeName(databaseVolumeName, streamType, mute);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test VolumeDataMaintainer.
 * @tc.number: GetMuteStatusWithDatabaseVolumeName_001.
 * @tc.desc  : Test GetMuteStatusWithDatabaseVolumeName API.
 */
HWTEST(VolumeDataMaintainerUnitTest, GetMuteStatusWithDatabaseVolumeName_001, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    std::string databaseVolumeName = "TestName";
    AudioStreamType streamType = STREAM_VOICE_ASSISTANT;
    bool mute = true;
    auto ret = volumeDataMaintainerRet->SaveMuteStatusWithDatabaseVolumeName(databaseVolumeName, streamType, mute);
    EXPECT_EQ(ret, false);

    ret = volumeDataMaintainerRet->GetMuteStatusWithDatabaseVolumeName(databaseVolumeName, streamType);
    EXPECT_EQ(ret, false);

    mute = false;
    ret = volumeDataMaintainerRet->SaveMuteStatusWithDatabaseVolumeName(databaseVolumeName, streamType, mute);
    EXPECT_EQ(ret, false);

    ret = volumeDataMaintainerRet->GetMuteStatusWithDatabaseVolumeName(databaseVolumeName, streamType);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test VolumeDataMaintainer.
 * @tc.number: SaveVolumeWithDatabaseVolumeName_001.
 * @tc.desc  : Test SaveVolumeWithDatabaseVolumeName API.
 */
HWTEST(VolumeDataMaintainerUnitTest, SaveVolumeWithDatabaseVolumeName_001, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    std::string databaseVolumeName = "TestName";
    AudioStreamType streamType = STREAM_MUSIC;
    int32_t volume = 1;
    auto ret = volumeDataMaintainerRet->SaveVolumeWithDatabaseVolumeName(databaseVolumeName, streamType, volume);
    EXPECT_EQ(ret, false);

    volume = 10;
    ret = volumeDataMaintainerRet->SaveVolumeWithDatabaseVolumeName(databaseVolumeName, streamType, volume);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test VolumeDataMaintainer.
 * @tc.number: SaveVolumeWithDatabaseVolumeName_002.
 * @tc.desc  : Test SaveVolumeWithDatabaseVolumeName API.
 */
HWTEST(VolumeDataMaintainerUnitTest, SaveVolumeWithDatabaseVolumeName_002, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    std::string databaseVolumeName = "TestName";
    AudioStreamType streamType = STREAM_RING;
    int32_t volume = 0;
    auto ret = volumeDataMaintainerRet->SaveVolumeWithDatabaseVolumeName(databaseVolumeName, streamType, volume);
    EXPECT_EQ(ret, false);

    volume = 15;
    ret = volumeDataMaintainerRet->SaveVolumeWithDatabaseVolumeName(databaseVolumeName, streamType, volume);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test VolumeDataMaintainer.
 * @tc.number: SaveVolumeWithDatabaseVolumeName_003.
 * @tc.desc  : Test SaveVolumeWithDatabaseVolumeName API.
 */
HWTEST(VolumeDataMaintainerUnitTest, SaveVolumeWithDatabaseVolumeName_003, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    std::string databaseVolumeName = "TestName";
    AudioStreamType streamType = STREAM_VOICE_CALL;
    int32_t volume = 0;
    auto ret = volumeDataMaintainerRet->SaveVolumeWithDatabaseVolumeName(databaseVolumeName, streamType, volume);
    EXPECT_EQ(ret, false);

    volume = 15;
    ret = volumeDataMaintainerRet->SaveVolumeWithDatabaseVolumeName(databaseVolumeName, streamType, volume);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test VolumeDataMaintainer.
 * @tc.number: GetVolumeWithDatabaseVolumeName_001.
 * @tc.desc  : Test GetVolumeWithDatabaseVolumeName API.
 */
HWTEST(VolumeDataMaintainerUnitTest, GetVolumeWithDatabaseVolumeName_001, TestSize.Level1)
{
    std::shared_ptr<VolumeDataMaintainer> volumeDataMaintainerRet = std::make_shared<VolumeDataMaintainer>();
    std::string databaseVolumeName = "TestName";
    AudioStreamType streamType = STREAM_VOICE_ASSISTANT;
    int32_t volume = 5;
    auto ret = volumeDataMaintainerRet->SaveVolumeWithDatabaseVolumeName(databaseVolumeName, streamType, volume);
    EXPECT_EQ(ret, false);

    ret = volumeDataMaintainerRet->GetVolumeWithDatabaseVolumeName(databaseVolumeName, streamType);
    EXPECT_EQ(ret, false);

    volume = 10;
    ret = volumeDataMaintainerRet->SaveVolumeWithDatabaseVolumeName(databaseVolumeName, streamType, volume);
    EXPECT_EQ(ret, false);

    ret = volumeDataMaintainerRet->GetVolumeWithDatabaseVolumeName(databaseVolumeName, streamType);
    EXPECT_EQ(ret, false);
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
} // AudioStandardnamespace
} // OHOSnamespace
