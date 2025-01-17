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

#include "../include/volume_data_maintainer_unit_test.h"
#include "audio_errors.h"

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
    VolumeDataMaintainer &volumeDataMaintainerRet = VolumeDataMaintainer::GetVolumeDataMaintainer();
    DeviceType typeRet = DEVICE_TYPE_NONE;
    AudioStreamType streamTypeRet = STREAM_DEFAULT;
    int32_t volumeLevelRet = 0;
    auto ret = volumeDataMaintainerRet.SaveVolume(typeRet, streamTypeRet, volumeLevelRet);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_002.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_002, TestSize.Level1)
{
    VolumeDataMaintainer &volumeDataMaintainerRet = VolumeDataMaintainer::GetVolumeDataMaintainer();
    DeviceType typeRet = DEVICE_TYPE_DP;
    AudioStreamType streamTypeRet = STREAM_MUSIC;
    int32_t volumeLevelRet = 0;
    auto ret = volumeDataMaintainerRet.SaveVolume(typeRet, streamTypeRet, volumeLevelRet);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_003.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_003, TestSize.Level1)
{
    VolumeDataMaintainer &volumeDataMaintainerRet = VolumeDataMaintainer::GetVolumeDataMaintainer();
    DeviceType deviceTypeRet = DEVICE_TYPE_NONE;
    AudioStreamType streamTypeRet = STREAM_DEFAULT;
    auto ret = volumeDataMaintainerRet.GetVolume(deviceTypeRet, streamTypeRet);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_004.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_004, TestSize.Level1)
{
    VolumeDataMaintainer &volumeDataMaintainerRet = VolumeDataMaintainer::GetVolumeDataMaintainer();
    DeviceType deviceTypeRet = DEVICE_TYPE_NONE;
    AudioStreamType streamTypeRet = STREAM_RING;
    bool muteStatusRet = false;
    auto ret = volumeDataMaintainerRet.SaveMuteStatus(deviceTypeRet, streamTypeRet, muteStatusRet);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_005.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_005, TestSize.Level1)
{
    VolumeDataMaintainer &volumeDataMaintainerRet = VolumeDataMaintainer::GetVolumeDataMaintainer();
    DeviceType deviceTypeRet = DEVICE_TYPE_DP;
    AudioStreamType streamTypeRet = STREAM_MUSIC;
    bool muteStatusRet = false;
    auto ret = volumeDataMaintainerRet.SaveMuteStatus(deviceTypeRet, streamTypeRet, muteStatusRet);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_006.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_006, TestSize.Level1)
{
    VolumeDataMaintainer &volumeDataMaintainerRet = VolumeDataMaintainer::GetVolumeDataMaintainer();
    DeviceType deviceTypeRet = DEVICE_TYPE_DP;
    AudioStreamType streamTypeRet = STREAM_DEFAULT;
    auto ret = volumeDataMaintainerRet.GetMuteStatusInternal(deviceTypeRet, streamTypeRet);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_007.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_007, TestSize.Level1)
{
    VolumeDataMaintainer &volumeDataMaintainerRet = VolumeDataMaintainer::GetVolumeDataMaintainer();
    DeviceType deviceTypeRet = DEVICE_TYPE_DP;
    AudioStreamType streamTypeRet = STREAM_MUSIC;
    auto ret = volumeDataMaintainerRet.GetMuteStatusInternal(deviceTypeRet, streamTypeRet);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_008.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_008, TestSize.Level1)
{
    VolumeDataMaintainer &volumeDataMaintainerRet = VolumeDataMaintainer::GetVolumeDataMaintainer();
    int32_t affectedRet;
    bool statusRet;
    auto ret = volumeDataMaintainerRet.GetMuteAffected(affectedRet);
    EXPECT_EQ(ret, false);

    ret = volumeDataMaintainerRet.GetMuteTransferStatus(statusRet);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_009.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_009, TestSize.Level1)
{
    VolumeDataMaintainer &volumeDataMaintainerRet = VolumeDataMaintainer::GetVolumeDataMaintainer();
    int32_t affectedRet = 0;
    bool statusRet = false;
    AudioRingerMode ringerModeRet = RINGER_MODE_SILENT;
    auto ret = volumeDataMaintainerRet.SetMuteAffectedToMuteStatusDataBase(affectedRet);
    EXPECT_EQ(ret, true);

    ret = volumeDataMaintainerRet.SaveMuteTransferStatus(statusRet);
    EXPECT_EQ(ret, false);

    ret = volumeDataMaintainerRet.SaveRingerMode(ringerModeRet);
    EXPECT_EQ(ret, false);

    ret = volumeDataMaintainerRet.GetRingerMode(ringerModeRet);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_010.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_010, TestSize.Level1)
{
    VolumeDataMaintainer &volumeDataMaintainerRet = VolumeDataMaintainer::GetVolumeDataMaintainer();
    DeviceType deviceTypeRet = DEVICE_TYPE_BLUETOOTH_SCO;
    SafeStatus safeStatusRet = SAFE_UNKNOWN;
    auto ret = volumeDataMaintainerRet.SaveSafeStatus(deviceTypeRet, safeStatusRet);
    EXPECT_EQ(ret, false);

    deviceTypeRet = DEVICE_TYPE_USB_ARM_HEADSET;
    ret = volumeDataMaintainerRet.SaveSafeStatus(deviceTypeRet, safeStatusRet);
    EXPECT_EQ(ret, false);

    deviceTypeRet = DEVICE_TYPE_NONE;
    ret = volumeDataMaintainerRet.SaveSafeStatus(deviceTypeRet, safeStatusRet);
    EXPECT_EQ(ret, false);

    deviceTypeRet = DEVICE_TYPE_BLUETOOTH_A2DP;
    ret = volumeDataMaintainerRet.SaveSafeStatus(deviceTypeRet, safeStatusRet);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_011.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_011, TestSize.Level1)
{
    VolumeDataMaintainer &volumeDataMaintainerRet = VolumeDataMaintainer::GetVolumeDataMaintainer();
    DeviceType deviceTypeRet = DEVICE_TYPE_NONE;
    SafeStatus safeStatusRet = SAFE_UNKNOWN;
    auto ret = volumeDataMaintainerRet.GetSafeStatus(deviceTypeRet, safeStatusRet);
    EXPECT_EQ(ret, false);

    deviceTypeRet = DEVICE_TYPE_USB_ARM_HEADSET;
    ret = volumeDataMaintainerRet.GetSafeStatus(deviceTypeRet, safeStatusRet);
    EXPECT_EQ(ret, false);

    deviceTypeRet = DEVICE_TYPE_BLUETOOTH_A2DP;
    ret = volumeDataMaintainerRet.GetSafeStatus(deviceTypeRet, safeStatusRet);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_012.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_012, TestSize.Level1)
{
    VolumeDataMaintainer &volumeDataMaintainerRet = VolumeDataMaintainer::GetVolumeDataMaintainer();
    DeviceType deviceTypeRet = DEVICE_TYPE_BLUETOOTH_SCO;
    int64_t timeRet = 0;
    auto ret = volumeDataMaintainerRet.SaveSafeVolumeTime(deviceTypeRet, timeRet);
    EXPECT_EQ(ret, false);

    deviceTypeRet = DEVICE_TYPE_USB_ARM_HEADSET;
    ret = volumeDataMaintainerRet.SaveSafeVolumeTime(deviceTypeRet, timeRet);
    EXPECT_EQ(ret, false);

    deviceTypeRet = DEVICE_TYPE_NONE;
    ret = volumeDataMaintainerRet.SaveSafeVolumeTime(deviceTypeRet, timeRet);
    EXPECT_EQ(ret, false);

    deviceTypeRet = DEVICE_TYPE_BLUETOOTH_A2DP;
    ret = volumeDataMaintainerRet.SaveSafeVolumeTime(deviceTypeRet, timeRet);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_013.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_013, TestSize.Level1)
{
    VolumeDataMaintainer &volumeDataMaintainerRet = VolumeDataMaintainer::GetVolumeDataMaintainer();
    DeviceType deviceTypeRet = DEVICE_TYPE_NONE;
    int64_t timeRet = 0;
    auto ret = volumeDataMaintainerRet.GetSafeVolumeTime(deviceTypeRet, timeRet);
    EXPECT_EQ(ret, false);

    deviceTypeRet = DEVICE_TYPE_BLUETOOTH_SCO;
    ret = volumeDataMaintainerRet.GetSafeVolumeTime(deviceTypeRet, timeRet);
    EXPECT_EQ(ret, false);

    deviceTypeRet = DEVICE_TYPE_USB_HEADSET;
    ret = volumeDataMaintainerRet.GetSafeVolumeTime(deviceTypeRet, timeRet);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_014.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_014, TestSize.Level1)
{
    VolumeDataMaintainer &volumeDataMaintainerRet = VolumeDataMaintainer::GetVolumeDataMaintainer();
    std::string keyRet1;
    std::string valueRet1;
    auto ret = volumeDataMaintainerRet.SaveSystemSoundUrl(keyRet1, valueRet1);
    EXPECT_EQ(ret, false);

    std::string keyRet2;
    std::string valueRet2;
    ret = volumeDataMaintainerRet.GetSystemSoundUrl(keyRet2, valueRet2);
    EXPECT_EQ(ret, false);
    volumeDataMaintainerRet.RegisterCloned();
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_015.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_015, TestSize.Level1)
{
    VolumeDataMaintainer &volumeDataMaintainerRet = VolumeDataMaintainer::GetVolumeDataMaintainer();
    bool isMuteRet1 = false;
    auto ret = volumeDataMaintainerRet.SaveMicMuteState(isMuteRet1);
    EXPECT_EQ(ret, false);

    bool isMuteRet2;
    ret = volumeDataMaintainerRet.GetMicMuteState(isMuteRet2);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_016.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_016, TestSize.Level1)
{
    VolumeDataMaintainer &volumeDataMaintainerRet = VolumeDataMaintainer::GetVolumeDataMaintainer();
    DeviceType deviceTypeRet = DEVICE_TYPE_BLUETOOTH_SCO;

    auto ret = volumeDataMaintainerRet.GetDeviceTypeName(deviceTypeRet);
    std::string typeRet = "_wireless";
    EXPECT_EQ(ret, typeRet);

    deviceTypeRet = DEVICE_TYPE_USB_ARM_HEADSET;
    ret = volumeDataMaintainerRet.GetDeviceTypeName(deviceTypeRet);
    typeRet = "_wired";
    EXPECT_EQ(ret, typeRet);

    deviceTypeRet = DEVICE_TYPE_REMOTE_CAST;
    ret = volumeDataMaintainerRet.GetDeviceTypeName(deviceTypeRet);
    typeRet = "_remote_cast";
    EXPECT_EQ(ret, typeRet);

    deviceTypeRet = DEVICE_TYPE_NONE;
    ret = volumeDataMaintainerRet.GetDeviceTypeName(deviceTypeRet);
    typeRet = "";
    EXPECT_EQ(ret, typeRet);
}

/**
* @tc.name  : Test VolumeDataMaintainer.
* @tc.number: VolumeDataMaintainerUnitTest_017.
* @tc.desc  : Test VolumeDataMaintainer API.
*/
HWTEST(VolumeDataMaintainerUnitTest, VolumeDataMaintainerUnitTest_017, TestSize.Level1)
{
    VolumeDataMaintainer &volumeDataMaintainerRet = VolumeDataMaintainer::GetVolumeDataMaintainer();
    DeviceType deviceTypeRet = DEVICE_TYPE_BLUETOOTH_SCO;
    AudioStreamType streamTypeRet = STREAM_DEFAULT;

    auto ret = volumeDataMaintainerRet.GetVolumeKeyForDataShare(deviceTypeRet, streamTypeRet);
    std::string typeRet = "";
    EXPECT_EQ(ret, typeRet);
}
} // AudioStandardnamespace
} // OHOSnamespace
