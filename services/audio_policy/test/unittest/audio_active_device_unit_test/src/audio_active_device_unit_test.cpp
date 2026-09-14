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

#include "audio_policy_utils.h"
#include "audio_active_device_unit_test.h"
#include "audio_router_select_strategy.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioActiveDeviceUnitTest::SetUpTestCase(void) {}
void AudioActiveDeviceUnitTest::TearDownTestCase(void) {}
void AudioActiveDeviceUnitTest::SetUp(void) {}
void AudioActiveDeviceUnitTest::TearDown(void) {}

constexpr int32_t TEST_DEVICE_ID = 1001;

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceUnitTest_001.
* @tc.desc  : Test GetActiveA2dpDeviceStreamInfo.
*/
HWTEST_F(AudioActiveDeviceUnitTest, AudioActiveDeviceUnitTest_001, TestSize.Level1)
{
    AudioStreamInfo streamInfo;
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    bool result = audioActiveDevice->GetActiveA2dpInDeviceStreamInfo(streamInfo);
    EXPECT_EQ(result, false);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceUnitTest_002.
* @tc.desc  : Test GetMaxAmplitude.
*/
HWTEST_F(AudioActiveDeviceUnitTest, AudioActiveDeviceUnitTest_002, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    auto desc = make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    desc->deviceId_ = TEST_DEVICE_ID;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentInputDevice(SYSTEM_UID, {desc});

    int32_t deviceId = AudioRouterSelectStrategy::GetInstance().Get1stCurrentInputDevice().deviceId_;
    AudioInterrupt audioInterrupt;
    float result = audioActiveDevice->GetMaxAmplitude(deviceId, audioInterrupt);
    EXPECT_FLOAT_EQ(result, 0.0f);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceUnitTest_011.
* @tc.desc  : Test IsDirectSupportedDevice.
*/
HWTEST_F(AudioActiveDeviceUnitTest, AudioActiveDeviceUnitTest_011, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    auto desc = make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_USB_HEADSET, OUTPUT_DEVICE);
    desc->deviceId_ = TEST_DEVICE_ID;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID, {desc});
    bool result = audioActiveDevice->IsDirectSupportedDevice();
    EXPECT_EQ(result, true);

    auto desc1 = make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE);
    desc1->deviceId_ = 1002;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc1);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID, {desc1});
    result = audioActiveDevice->IsDirectSupportedDevice();
    EXPECT_EQ(result, false);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: AudioActiveDeviceUnitTest_012.
* @tc.desc  : Test IsDeviceActive.
*/
HWTEST_F(AudioActiveDeviceUnitTest, AudioActiveDeviceUnitTest_012, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    auto desc = make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_USB_HEADSET, OUTPUT_DEVICE);
    desc->deviceId_ = TEST_DEVICE_ID;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID, {desc});
    bool result = audioActiveDevice->IsDeviceActive(DeviceType::DEVICE_TYPE_USB_HEADSET);
    EXPECT_EQ(result, true);

    result = audioActiveDevice->IsDeviceActive(DeviceType::DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(result, false);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: GetCurrentInputDeviceMacAddr_001.
* @tc.desc  : Test GetCurrentInputDeviceMacAddr.
*/
HWTEST_F(AudioActiveDeviceUnitTest, GetCurrentInputDeviceMacAddr_001, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    auto desc = make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_EARPIECE;
    desc->deviceId_ = TEST_DEVICE_ID;
    desc->macAddress_ = "00:11:22:33:44:55";
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentInputDevice(SYSTEM_UID, {desc});
    string ret = audioActiveDevice->GetCurrentInputDeviceMacAddr();
    EXPECT_EQ(ret, "00:11:22:33:44:55");
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: GetCurrentOutputDeviceCategory_001.
* @tc.desc  : Test GetCurrentOutputDeviceCategory.
*/
HWTEST_F(AudioActiveDeviceUnitTest, GetCurrentOutputDeviceCategory_001, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();

    DeviceCategory ret = audioActiveDevice->GetCurrentOutputDeviceCategory();
    EXPECT_EQ(ret, CATEGORY_DEFAULT);
}

/**
* @tc.name  : Test AudioActiveDevice.
* @tc.number: WriteOutputRouteChangeEvent_001.
* @tc.desc  : Test WriteOutputRouteChangeEvent.
*/
HWTEST_F(AudioActiveDeviceUnitTest, WriteOutputRouteChangeEvent_001, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    deviceDescriptor->deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    AudioStreamDeviceChangeReason reason = AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE;
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();

    audioActiveDevice->WriteOutputRouteChangeEvent(deviceDescriptor, reason);
    EXPECT_EQ(deviceDescriptor->deviceId_, 0);
}
/**
 * @tc.name  : GetDeviceForVolumeEnhanced_InvalidUid
 * @tc.number: GetDeviceForVolumeEnhanced_001
 * @tc.desc  : Test if(appUid==INVALID_UID) true branch, appUid replaced by callingUid
 */
HWTEST_F(AudioActiveDeviceUnitTest, GetDeviceForVolumeEnhanced_InvalidUid, TestSize.Level1)
{
    AudioActiveDevice &audioActiveDevice = AudioActiveDevice::GetInstance();
    auto result = audioActiveDevice.GetDeviceForVolumeEnhanced(INVALID_UID);
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name  : GetDeviceForVolumeEnhanced_ValidUid
 * @tc.number: GetDeviceForVolumeEnhanced_002
 * @tc.desc  : Test if(appUid==INVALID_UID) false branch, appUid stays as passed value
 */
HWTEST_F(AudioActiveDeviceUnitTest, GetDeviceForVolumeEnhanced_ValidUid, TestSize.Level1)
{
    AudioActiveDevice &audioActiveDevice = AudioActiveDevice::GetInstance();
    auto result = audioActiveDevice.GetDeviceForVolumeEnhanced(SYSTEM_UID);
    EXPECT_NE(result, nullptr);
}
/**
 * @tc.name  : GetUidVolumeDevicesForEnhancedRouting_UidNotFound
 * @tc.number: GetUidVolumeDevicesForEnhancedRouting_001
 * @tc.desc  : Test if condition true branch, uid not in uidVolumeDeviceMap_ returns empty vector
 */
HWTEST_F(AudioActiveDeviceUnitTest, GetUidVolumeDevicesForEnhancedRouting_UidNotFound, TestSize.Level1)
{
    AudioActiveDevice &audioActiveDevice = AudioActiveDevice::GetInstance();
    audioActiveDevice.uidVolumeDeviceMap_.clear();

    auto result = audioActiveDevice.GetUidVolumeDevicesForEnhancedRouting(9999);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name  : GetUidVolumeDevicesForEnhancedRouting_UidFound
 * @tc.number: GetUidVolumeDevicesForEnhancedRouting_002
 * @tc.desc  : Test if condition false branch, uid found in uidVolumeDeviceMap_ returns devices
 */
HWTEST_F(AudioActiveDeviceUnitTest, GetUidVolumeDevicesForEnhancedRouting_UidFound, TestSize.Level1)
{
    AudioActiveDevice &audioActiveDevice = AudioActiveDevice::GetInstance();
    audioActiveDevice.uidVolumeDeviceMap_.clear();
    int32_t uid = 1000;
    auto desc = std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_USB_HEADSET, OUTPUT_DEVICE);
    audioActiveDevice.uidVolumeDeviceMap_[uid] = {desc};

    auto result = audioActiveDevice.GetUidVolumeDevicesForEnhancedRouting(uid);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0]->deviceType_, DeviceType::DEVICE_TYPE_USB_HEADSET);

    audioActiveDevice.uidVolumeDeviceMap_.clear();
}

/**
 * @tc.name  : Test AudioActiveDevice.
 * @tc.number: AudioActiveDeviceUnitTest_003.
 * @tc.desc  : Test GetActiveA2dpDeviceStreamInfo with BLUETOOTH_A2DP_IN.
 */
HWTEST_F(AudioActiveDeviceUnitTest, AudioActiveDeviceUnitTest_003, TestSize.Level1)
{
    AudioStreamInfo streamInfo;
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    bool result = audioActiveDevice->GetActiveA2dpInDeviceStreamInfo(streamInfo);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test AudioActiveDevice.
 * @tc.number: AudioActiveDeviceUnitTest_004.
 * @tc.desc  : Test GetMaxAmplitude with BLUETOOTH_SCO device.
 */
HWTEST_F(AudioActiveDeviceUnitTest, AudioActiveDeviceUnitTest_004, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    auto desc = make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE);
    desc->deviceId_ = 2001;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID, {desc});

    int32_t deviceId = AudioRouterSelectStrategy::GetInstance().Get1stCurrentOutputDevice().deviceId_;
    AudioInterrupt audioInterrupt;
    float result = audioActiveDevice->GetMaxAmplitude(deviceId, audioInterrupt);
    EXPECT_FLOAT_EQ(result, 0.0f);
}

/**
 * @tc.name  : Test AudioActiveDevice.
 * @tc.number: AudioActiveDeviceUnitTest_005.
 * @tc.desc  : Test IsDirectSupportedDevice with BLUETOOTH_A2DP.
 */
HWTEST_F(AudioActiveDeviceUnitTest, AudioActiveDeviceUnitTest_005, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    auto desc = make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    desc->deviceId_ = 2002;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID, {desc});
    bool result = audioActiveDevice->IsDirectSupportedDevice();
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test AudioActiveDevice.
 * @tc.number: AudioActiveDeviceUnitTest_006.
 * @tc.desc  : Test IsDeviceActive with BLUETOOTH_A2DP.
 */
HWTEST_F(AudioActiveDeviceUnitTest, AudioActiveDeviceUnitTest_006, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    auto desc = make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    desc->deviceId_ = 2003;
    desc->networkId_ = LOCAL_NETWORK_ID;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID, {desc});
    bool result = audioActiveDevice->IsDeviceActive(DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name  : Test AudioActiveDevice.
 * @tc.number: GetCurrentInputDeviceMacAddr_002.
 * @tc.desc  : Test GetCurrentInputDeviceMacAddr with BLUETOOTH_SCO device.
 */
HWTEST_F(AudioActiveDeviceUnitTest, GetCurrentInputDeviceMacAddr_002, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    auto desc = make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE);
    desc->deviceId_ = 2004;
    desc->macAddress_ = "AA:BB:CC:DD:EE:FF";
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentInputDevice(SYSTEM_UID, {desc});
    string ret = audioActiveDevice->GetCurrentInputDeviceMacAddr();
    EXPECT_EQ(ret, "AA:BB:CC:DD:EE:FF");
}

/**
 * @tc.name  : Test AudioActiveDevice.
 * @tc.number: GetCurrentOutputDeviceCategory_002.
 * @tc.desc  : Test GetCurrentOutputDeviceCategory with BLUETOOTH_SCO device.
 */
HWTEST_F(AudioActiveDeviceUnitTest, GetCurrentOutputDeviceCategory_002, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    auto desc = make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE);
    desc->deviceId_ = 2005;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID, {desc});

    DeviceCategory ret = audioActiveDevice->GetCurrentOutputDeviceCategory();
    EXPECT_EQ(ret, CATEGORY_DEFAULT);
}

/**
 * @tc.name  : Test AudioActiveDevice.
 * @tc.number: WriteOutputRouteChangeEvent_002.
 * @tc.desc  : Test WriteOutputRouteChangeEvent with BLUETOOTH_SCO device.
 */
HWTEST_F(AudioActiveDeviceUnitTest, WriteOutputRouteChangeEvent_002, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    deviceDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    AudioStreamDeviceChangeReason reason = AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE;
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();

    audioActiveDevice->WriteOutputRouteChangeEvent(deviceDescriptor, reason);
    EXPECT_EQ(deviceDescriptor->deviceId_, 0);
}

/**
 * @tc.name  : GetDeviceForVolumeEnhanced_RegularUid
 * @tc.number: GetDeviceForVolumeEnhanced_003
 * @tc.desc  : Test GetDeviceForVolumeEnhanced with a regular non-system UID.
 */
HWTEST_F(AudioActiveDeviceUnitTest, GetDeviceForVolumeEnhanced_003, TestSize.Level1)
{
    AudioActiveDevice &audioActiveDevice = AudioActiveDevice::GetInstance();
    int32_t uid = 10042;
    auto result = audioActiveDevice.GetDeviceForVolumeEnhanced(uid);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name  : GetUidVolumeDevicesForEnhancedRouting_UidNotInMap
 * @tc.number: GetUidVolumeDevicesForEnhancedRouting_003
 * @tc.desc  : Test GetUidVolumeDevicesForEnhancedRouting with UID that has no entry in the map.
 */
HWTEST_F(AudioActiveDeviceUnitTest, GetUidVolumeDevicesForEnhancedRouting_003, TestSize.Level1)
{
    AudioActiveDevice &audioActiveDevice = AudioActiveDevice::GetInstance();
    audioActiveDevice.uidVolumeDeviceMap_.clear();
    int32_t uid = 10042;
    auto desc = std::make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    audioActiveDevice.uidVolumeDeviceMap_[uid] = {desc};

    auto result = audioActiveDevice.GetUidVolumeDevicesForEnhancedRouting(9999);
    EXPECT_TRUE(result.empty());

    result = audioActiveDevice.GetUidVolumeDevicesForEnhancedRouting(uid);
    EXPECT_EQ(result.size(), 1);

    audioActiveDevice.uidVolumeDeviceMap_.clear();
}

/**
* @tc.name  : NotifyUserDisSelectionEventToBt_001
* @tc.desc  : Test NotifyUserDisSelectionEventToBt with isSameDevice true
*/
HWTEST_F(AudioActiveDeviceUnitTest, NotifyUserDisSelectionEventToBt_001, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE);
    desc->macAddress_ = "00:11:22:33:44:55";
    audioActiveDevice->NotifyUserDisSelectionEventToBt(desc, true, STREAM_USAGE_MUSIC);
    EXPECT_TRUE(true);
}

/**
* @tc.name  : NotifyUserDisSelectionEventToBt_002
* @tc.desc  : Test NotifyUserDisSelectionEventToBt with nullptr desc
*/
HWTEST_F(AudioActiveDeviceUnitTest, NotifyUserDisSelectionEventToBt_002, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    audioActiveDevice->NotifyUserDisSelectionEventToBt(nullptr, false, STREAM_USAGE_MUSIC);
    EXPECT_TRUE(true);
}

/**
* @tc.name  : BuildWirelessDeviceNeedsBothDirections_001
* @tc.desc  : Test BuildWirelessDeviceNeedsBothDirections with BT devices
*/
HWTEST_F(AudioActiveDeviceUnitTest, BuildWirelessDeviceNeedsBothDirections_001, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    std::map<uint32_t, std::vector<std::shared_ptr<AudioDeviceDescriptor>>> outputPreviewResults;
    auto btDesc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE);
    outputPreviewResults[1] = {btDesc};

    std::map<uint32_t, std::shared_ptr<AudioDeviceDescriptor>> inputPreviewResults;
    auto result = audioActiveDevice->BuildWirelessDeviceNeedsBothDirections(outputPreviewResults, inputPreviewResults);
    EXPECT_EQ(result.size(), 1);
}

/**
* @tc.name  : BuildWirelessDeviceNeedsBothDirections_002
* @tc.desc  : Test BuildWirelessDeviceNeedsBothDirections with non-BT devices
*/
HWTEST_F(AudioActiveDeviceUnitTest, BuildWirelessDeviceNeedsBothDirections_002, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    std::map<uint32_t, std::vector<std::shared_ptr<AudioDeviceDescriptor>>> outputPreviewResults;
    auto speakerDesc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    outputPreviewResults[1] = {speakerDesc};

    std::map<uint32_t, std::shared_ptr<AudioDeviceDescriptor>> inputPreviewResults;
    auto result = audioActiveDevice->BuildWirelessDeviceNeedsBothDirections(outputPreviewResults, inputPreviewResults);
    EXPECT_EQ(result.size(), 0);
}

/**
* @tc.name  : BuildWirelessDeviceNeedsBothDirections_003
* @tc.desc  : Test BuildWirelessDeviceNeedsBothDirections with input BT device
*/
HWTEST_F(AudioActiveDeviceUnitTest, BuildWirelessDeviceNeedsBothDirections_003, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    std::map<uint32_t, std::vector<std::shared_ptr<AudioDeviceDescriptor>>> outputPreviewResults;

    std::map<uint32_t, std::shared_ptr<AudioDeviceDescriptor>> inputPreviewResults;
    auto btInputDesc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE);
    inputPreviewResults[1] = btInputDesc;

    auto result = audioActiveDevice->BuildWirelessDeviceNeedsBothDirections(outputPreviewResults, inputPreviewResults);
    EXPECT_EQ(result.size(), 1);
}

/**
* @tc.name  : IsDeviceInWirelessNeeds_001
* @tc.desc  : Test IsDeviceInWirelessNeeds with nullptr device
*/
HWTEST_F(AudioActiveDeviceUnitTest, IsDeviceInWirelessNeeds_001, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> needs;
    EXPECT_FALSE(audioActiveDevice->IsDeviceInWirelessNeeds(nullptr, needs));
}

/**
* @tc.name  : IsDeviceInWirelessNeeds_002
* @tc.desc  : Test IsDeviceInWirelessNeeds with matching BT device
*/
HWTEST_F(AudioActiveDeviceUnitTest, IsDeviceInWirelessNeeds_002, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    auto device = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE);
    device->macAddress_ = "00:11:22:33:44:55";
    auto need = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE);
    need->macAddress_ = "00:11:22:33:44:55";
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> needs = {need};
    EXPECT_TRUE(audioActiveDevice->IsDeviceInWirelessNeeds(device, needs));
}

/**
* @tc.name  : IsDeviceInWirelessNeeds_003
* @tc.desc  : Test IsDeviceInWirelessNeeds with non-matching device
*/
HWTEST_F(AudioActiveDeviceUnitTest, IsDeviceInWirelessNeeds_003, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    auto device = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE);
    device->macAddress_ = "AA:BB:CC:DD:EE:FF";
    auto need = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE);
    need->macAddress_ = "00:11:22:33:44:55";
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> needs = {need};
    EXPECT_FALSE(audioActiveDevice->IsDeviceInWirelessNeeds(device, needs));
}

/**
* @tc.name  : DecideWirelessDeviceActionBasedOnPreview_001
* @tc.desc  : Test DecideWirelessDeviceActionBasedOnPreview with nullptr new device
*/
HWTEST_F(AudioActiveDeviceUnitTest, DecideWirelessDeviceActionBasedOnPreview_001, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    std::map<uint32_t, std::vector<std::shared_ptr<AudioDeviceDescriptor>>> outputPreviewResults;
    std::map<uint32_t, std::shared_ptr<AudioDeviceDescriptor>> inputPreviewResults;
    audioActiveDevice->DecideWirelessDeviceActionBasedOnPreview(outputPreviewResults, inputPreviewResults,
        nullptr, STREAM_USAGE_MUSIC);
    EXPECT_TRUE(true);
}

/**
* @tc.name  : DecideActionForSingleWirelessOutputDevice_001
* @tc.desc  : Test DecideActionForSingleWirelessOutputDevice with device still needed
*/
HWTEST_F(AudioActiveDeviceUnitTest, DecideActionForSingleWirelessOutputDevice_001, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE);
    deviceDesc->macAddress_ = "00:11:22:33:44:55";
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> wirelessDeviceNeeds = {deviceDesc};
    audioActiveDevice->DecideActionForSingleWirelessOutputDevice(deviceDesc, STREAM_USAGE_MUSIC,
        wirelessDeviceNeeds);
    EXPECT_TRUE(true);
}

/**
* @tc.name  : NotifyNewDeviceSelected_001
* @tc.desc  : Test NotifyNewDeviceSelected with nullptr
*/
HWTEST_F(AudioActiveDeviceUnitTest, NotifyNewDeviceSelected_001, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    audioActiveDevice->NotifyNewDeviceSelected(nullptr, STREAM_USAGE_MUSIC);
    EXPECT_TRUE(true);
}

/**
* @tc.name  : DecideWirelessDeviceActionForInputBasedOnPreview_001
* @tc.desc  : Test DecideWirelessDeviceActionForInputBasedOnPreview with nullptr
*/
HWTEST_F(AudioActiveDeviceUnitTest, DecideWirelessDeviceActionForInputBasedOnPreview_001, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    std::map<uint32_t, std::vector<std::shared_ptr<AudioDeviceDescriptor>>> outputPreviewResults;
    std::map<uint32_t, std::shared_ptr<AudioDeviceDescriptor>> inputPreviewResults;
    audioActiveDevice->DecideWirelessDeviceActionForInputBasedOnPreview(outputPreviewResults, inputPreviewResults,
        nullptr, SOURCE_TYPE_MIC);
    EXPECT_TRUE(true);
}

/**
* @tc.name  : DecideActionForSingleWirelessInputDevice_001
* @tc.desc  : Test DecideActionForSingleWirelessInputDevice with device still needed
*/
HWTEST_F(AudioActiveDeviceUnitTest, DecideActionForSingleWirelessInputDevice_001, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_SCO, INPUT_DEVICE);
    deviceDesc->macAddress_ = "00:11:22:33:44:55";
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> wirelessDeviceNeeds = {deviceDesc};
    audioActiveDevice->DecideActionForSingleWirelessInputDevice(deviceDesc, SOURCE_TYPE_MIC,
        wirelessDeviceNeeds);
    EXPECT_TRUE(true);
}

/**
* @tc.name  : NotifyNewInputDeviceSelected_001
* @tc.desc  : Test NotifyNewInputDeviceSelected with nullptr
*/
HWTEST_F(AudioActiveDeviceUnitTest, NotifyNewInputDeviceSelected_001, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    audioActiveDevice->NotifyNewInputDeviceSelected(nullptr, SOURCE_TYPE_MIC);
    EXPECT_TRUE(true);
}

/**
* @tc.name  : ReleaseActiveDeviceRoute_001
* @tc.desc  : Test ReleaseActiveDeviceRoute
*/
HWTEST_F(AudioActiveDeviceUnitTest, ReleaseActiveDeviceRoute_001, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    audioActiveDevice->ReleaseActiveDeviceRoute(DEVICE_TYPE_BLUETOOTH_SCO,
        DeviceFlag::OUTPUT_DEVICES_FLAG, LOCAL_NETWORK_ID);
    EXPECT_TRUE(true);
}

/**
* @tc.name  : IsDeviceInVector_001
* @tc.desc  : Test IsDeviceInVector with nullptr
*/
HWTEST_F(AudioActiveDeviceUnitTest, IsDeviceInVector_001, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    EXPECT_FALSE(audioActiveDevice->IsDeviceInVector(nullptr, descs));
}

/**
* @tc.name  : IsDeviceInVector_002
* @tc.desc  : Test IsDeviceInVector with device found
*/
HWTEST_F(AudioActiveDeviceUnitTest, IsDeviceInVector_002, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    desc->deviceId_ = 1;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs = {desc};
    EXPECT_TRUE(audioActiveDevice->IsDeviceInVector(desc, descs));
}

/**
* @tc.name  : IsDeviceInVector_003
* @tc.desc  : Test IsDeviceInVector with device not found
*/
HWTEST_F(AudioActiveDeviceUnitTest, IsDeviceInVector_003, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    auto desc1 = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    desc1->deviceId_ = 1;
    auto desc2 = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE);
    desc2->deviceId_ = 2;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs = {desc1};
    EXPECT_FALSE(audioActiveDevice->IsDeviceInVector(desc2, descs));
}

/**
* @tc.name  : UpdateDeviceListWithStreamState_001
* @tc.desc  : Test UpdateDeviceListWithStreamState with nullptr
*/
HWTEST_F(AudioActiveDeviceUnitTest, UpdateDeviceListWithStreamState_001, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    audioActiveDevice->UpdateDeviceListWithStreamState(nullptr, descs);
    EXPECT_EQ(descs.size(), 0);
}

/**
* @tc.name  : UpdateDeviceListWithStreamState_002
* @tc.desc  : Test UpdateDeviceListWithStreamState with new device added
*/
HWTEST_F(AudioActiveDeviceUnitTest, UpdateDeviceListWithStreamState_002, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    desc->deviceId_ = 1;
    desc->hasStreamRunning_ = true;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    audioActiveDevice->UpdateDeviceListWithStreamState(desc, descs);
    EXPECT_EQ(descs.size(), 1);
}

/**
* @tc.name  : UpdateVolumeTypeDeviceMap_001
* @tc.desc  : Test UpdateVolumeTypeDeviceMap with nullptr
*/
HWTEST_F(AudioActiveDeviceUnitTest, UpdateVolumeTypeDeviceMap_001, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    audioActiveDevice->UpdateVolumeTypeDeviceMap(nullptr);
    EXPECT_TRUE(true);
}

/**
* @tc.name  : UpdateStreamUsageDeviceMap_001
* @tc.desc  : Test UpdateStreamUsageDeviceMap with nullptr
*/
HWTEST_F(AudioActiveDeviceUnitTest, UpdateStreamUsageDeviceMap_001, TestSize.Level1)
{
    auto audioActiveDevice = std::make_shared<AudioActiveDevice>();
    audioActiveDevice->UpdateStreamUsageDeviceMap(nullptr);
    EXPECT_TRUE(true);
}

/**
* @tc.name  : IsDeviceInActiveOutputDevices_001
* @tc.desc  : Test IsDeviceInActiveOutputDevices with shared_ptr
*/
HWTEST_F(AudioActiveDeviceUnitTest, IsDeviceInActiveOutputDevices_001, TestSize.Level1)
{
    AudioActiveDevice &audioActiveDevice = AudioActiveDevice::GetInstance();
    audioActiveDevice.activeOutputDevices_.clear();
    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    desc->deviceId_ = 1;
    EXPECT_FALSE(audioActiveDevice.IsDeviceInActiveOutputDevices(desc));

    audioActiveDevice.activeOutputDevices_.push_back(desc);
    EXPECT_TRUE(audioActiveDevice.IsDeviceInActiveOutputDevices(desc));
    audioActiveDevice.activeOutputDevices_.clear();
}

/**
* @tc.name  : GetActiveDeviceForVolume_001
* @tc.desc  : Test GetActiveDeviceForVolume
*/
HWTEST_F(AudioActiveDeviceUnitTest, GetActiveDeviceForVolume_001, TestSize.Level1)
{
    AudioActiveDevice &audioActiveDevice = AudioActiveDevice::GetInstance();
    auto result = audioActiveDevice.GetActiveDeviceForVolume(1000);
    EXPECT_EQ(result, nullptr);
}

/**
* @tc.name  : GetActiveOutputDevices_001
* @tc.desc  : Test GetActiveOutputDevices
*/
HWTEST_F(AudioActiveDeviceUnitTest, GetActiveOutputDevices_001, TestSize.Level1)
{
    AudioActiveDevice &audioActiveDevice = AudioActiveDevice::GetInstance();
    audioActiveDevice.activeOutputDevices_.clear();
    auto result = audioActiveDevice.GetActiveOutputDevices();
    EXPECT_EQ(result.size(), 0);

    auto desc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    audioActiveDevice.activeOutputDevices_.push_back(desc);
    result = audioActiveDevice.GetActiveOutputDevices();
    EXPECT_EQ(result.size(), 1);
    audioActiveDevice.activeOutputDevices_.clear();
}

} // namespace AudioStandard
} // namespace OHOS
