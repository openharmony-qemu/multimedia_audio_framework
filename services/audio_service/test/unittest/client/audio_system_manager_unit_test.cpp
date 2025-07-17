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

#include <gtest/gtest.h>

#include "audio_service_log.h"
#include "audio_errors.h"
#include "audio_system_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

const int32_t TEST_RET_NUM = 0;

class AudioSystemManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class AudioManagerAppVolumeChangeCallbackTest : public AudioManagerAppVolumeChangeCallback {
public:
    void OnAppVolumeChangedForUid(int32_t appUid, const VolumeEvent &event) override {}
    void OnSelfAppVolumeChanged(const VolumeEvent &event) override {}
};

class DataTransferStateChangeCallbackTest : public AudioRendererDataTransferStateChangeCallback {
public:
    void OnDataTransferStateChange(const AudioRendererDataTransferStateChangeInfo &info) override {}
};

/**
 * @tc.name  : Test GetMaxVolume API
 * @tc.type  : FUNC
 * @tc.number: GetMaxVolume_001
 * @tc.desc  : Test GetMaxVolume interface.
 */
HWTEST(AudioSystemManagerUnitTest, GetMaxVolume_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetMaxVolume_001 start");
    int32_t result = AudioSystemManager::GetInstance()->GetMaxVolume(STREAM_ALL);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetMaxVolume_001 result1:%{public}d", result);
    EXPECT_GT(result, TEST_RET_NUM);
    result = AudioSystemManager::GetInstance()->GetMaxVolume(STREAM_ULTRASONIC);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetMaxVolume_001 result2:%{public}d", result);
    EXPECT_GT(result, TEST_RET_NUM);
}

/**
 * @tc.name  : Test GetMinVolume API
 * @tc.type  : FUNC
 * @tc.number: GetMinVolume_001
 * @tc.desc  : Test GetMinVolume interface.
 */
HWTEST(AudioSystemManagerUnitTest, GetMinVolume_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetMinVolume_001 start");
    int32_t result = AudioSystemManager::GetInstance()->GetMinVolume(STREAM_ALL);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetMinVolume_001 result1:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
    result = AudioSystemManager::GetInstance()->GetMinVolume(STREAM_ULTRASONIC);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetMinVolume_001 result2:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
}

/**
 * @tc.name  : Test IsStreamMute API
 * @tc.type  : FUNC
 * @tc.number: IsStreamMute_001
 * @tc.desc  : Test IsStreamMute interface.
 */
HWTEST(AudioSystemManagerUnitTest, IsStreamMute_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamMute_001 start");
    bool result = AudioSystemManager::GetInstance()->IsStreamMute(STREAM_MUSIC);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamMute_001 result1:%{public}d", result);
    EXPECT_EQ(result, true);

    result = AudioSystemManager::GetInstance()->IsStreamMute(STREAM_RING);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamMute_001 result2:%{public}d", result);
    EXPECT_EQ(result, false);

    result = AudioSystemManager::GetInstance()->IsStreamMute(STREAM_NOTIFICATION);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamMute_001 result3:%{public}d", result);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test IsStreamActive API
 * @tc.type  : FUNC
 * @tc.number: IsStreamActive_002
 * @tc.desc  : Test IsStreamActive interface.
 */
HWTEST(AudioSystemManagerUnitTest, IsStreamActive_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamActive_002 start");
    bool result = AudioSystemManager::GetInstance()->IsStreamActive(STREAM_MUSIC);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamActive_002 result1:%{public}d", result);
    EXPECT_EQ(result, false);

    result = AudioSystemManager::GetInstance()->IsStreamActive(STREAM_ULTRASONIC);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamActive_002 result2:%{public}d", result);
    EXPECT_EQ(result, false);

    result = AudioSystemManager::GetInstance()->IsStreamActive(STREAM_ALL);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamActive_002 result3:%{public}d", result);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test GetSelfBundleName API
 * @tc.type  : FUNC
 * @tc.number: GetSelfBundleName_001
 * @tc.desc  : Test GetSelfBundleName interface.
 */
HWTEST(AudioSystemManagerUnitTest, GetSelfBundleName_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetSelfBundleName_001 start");
    std::string bundleName = AudioSystemManager::GetInstance()->GetSelfBundleName();
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetSelfBundleName_001 bundleName:%{public}s", bundleName.c_str());
    EXPECT_EQ(bundleName, "");
}

/**
 * @tc.name  : Test GetPinValueFromType API
 * @tc.type  : FUNC
 * @tc.number: GetPinValueFromType_001
 * @tc.desc  : Test GetPinValueFromType interface.
 */
HWTEST(AudioSystemManagerUnitTest, GetPinValueFromType_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetPinValueFromType_001 start");
    AudioPin pinValue = AudioSystemManager::GetInstance()->GetPinValueFromType(DEVICE_TYPE_DP, INPUT_DEVICE);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest ->GetPinValueFromType_001() pinValue:%{public}d", pinValue);
    EXPECT_NE(pinValue, AUDIO_PIN_NONE);
}

/**
 * @tc.name  : Test GetPinValueFromType API
 * @tc.type  : FUNC
 * @tc.number: GetPinValueFromType_002
 * @tc.desc  : Test GetPinValueFromType interface.
 */
HWTEST(AudioSystemManagerUnitTest, GetPinValueFromType_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetPinValueFromType_002 start");
    AudioPin pinValue = AudioSystemManager::GetInstance()->GetPinValueFromType(DEVICE_TYPE_HDMI, OUTPUT_DEVICE);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest ->GetPinValueFromType_002() pinValue:%{public}d", pinValue);
    EXPECT_NE(pinValue, AUDIO_PIN_NONE);
}

/**
 * @tc.name  : Test RegisterWakeupSourceCallback API
 * @tc.type  : FUNC
 * @tc.number: RegisterWakeupSourceCallback_001
 * @tc.desc  : Test RegisterWakeupSourceCallback interface.
 */
HWTEST(AudioSystemManagerUnitTest, RegisterWakeupSourceCallback_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest RegisterWakeupSourceCallback_001 start");
    int32_t result = AudioSystemManager::GetInstance()->RegisterWakeupSourceCallback();
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest ->RegisterWakeupSourceCallback_001() result:%{public}d", result);
    EXPECT_NE(result, ERROR);
}

/**
* @tc.name   : Test ConfigDistributedRoutingRole API
* @tc.number : ConfigDistributedRoutingRoleTest_001
* @tc.desc   : Test ConfigDistributedRoutingRole interface, when descriptor is nullptr.
*/
HWTEST(AudioSystemManagerUnitTest, ConfigDistributedRoutingRoleTest_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest ConfigDistributedRoutingRoleTest_001 start");
    CastType castType = CAST_TYPE_ALL;
    int32_t result = AudioSystemManager::GetInstance()->ConfigDistributedRoutingRole(nullptr, castType);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest ConfigDistributedRoutingRoleTest_001() result:%{public}d", result);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

#ifdef TEMP_DISABLE
/**
 * @tc.name   : Test ExcludeOutputDevices API
 * @tc.number : ExcludeOutputDevicesTest_001
 * @tc.desc   : Test ExcludeOutputDevices interface, when audioDeviceDescriptors is valid.
 */
HWTEST(AudioSystemManagerUnitTest, ExcludeOutputDevicesTest_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest ExcludeOutputDevicesTest_001 start");
    AudioDeviceUsage audioDevUsage = MEDIA_OUTPUT_DEVICES;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    std::shared_ptr<AudioDeviceDescriptor> audioDevDesc = std::make_shared<AudioDeviceDescriptor>();
    audioDevDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDevDesc->networkId_ = LOCAL_NETWORK_ID;
    audioDevDesc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDevDesc->macAddress_ = "00:00:00:00:00:00";
    audioDeviceDescriptors.push_back(audioDevDesc);
    int32_t result = AudioSystemManager::GetInstance()->ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest ExcludeOutputDevicesTest_001() result:%{public}d", result);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name   : Test ExcludeOutputDevices API
 * @tc.number : ExcludeOutputDevicesTest_002
 * @tc.desc   : Test ExcludeOutputDevices interface, when audioDeviceDescriptors is valid.
 */
HWTEST(AudioSystemManagerUnitTest, ExcludeOutputDevicesTest_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest ExcludeOutputDevicesTest_002 start");
    AudioDeviceUsage audioDevUsage = CALL_OUTPUT_DEVICES;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    std::shared_ptr<AudioDeviceDescriptor> audioDevDesc = std::make_shared<AudioDeviceDescriptor>();
    audioDevDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    audioDevDesc->networkId_ = LOCAL_NETWORK_ID;
    audioDevDesc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDevDesc->macAddress_ = "00:00:00:00:00:00";
    audioDeviceDescriptors.push_back(audioDevDesc);
    int32_t result = AudioSystemManager::GetInstance()->ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest ExcludeOutputDevicesTest_001() result:%{public}d", result);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name   : Test UnexcludeOutputDevices API
 * @tc.number : UnexcludeOutputDevicesTest_001
 * @tc.desc   : Test UnexcludeOutputDevices interface, when audioDeviceDescriptors is valid.
 */
HWTEST(AudioSystemManagerUnitTest, UnexcludeOutputDevicesTest_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest UnexcludeOutputDevicesTest_001 start");
    AudioDeviceUsage audioDevUsage = MEDIA_OUTPUT_DEVICES;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    std::shared_ptr<AudioDeviceDescriptor> audioDevDesc = std::make_shared<AudioDeviceDescriptor>();
    audioDevDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDevDesc->networkId_ = LOCAL_NETWORK_ID;
    audioDevDesc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDevDesc->macAddress_ = "00:00:00:00:00:00";
    audioDeviceDescriptors.push_back(audioDevDesc);
    AudioSystemManager::GetInstance()->ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
    int32_t result = AudioSystemManager::GetInstance()->UnexcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest UnexcludeOutputDevicesTest_001() result:%{public}d", result);
    EXPECT_EQ(result, SUCCESS);
}
#endif

/**
 * @tc.name   : Test UnexcludeOutputDevices API
 * @tc.number : UnexcludeOutputDevicesTest_002
 * @tc.desc   : Test UnexcludeOutputDevices interface, when audioDeviceDescriptors is empty.
 */
HWTEST(AudioSystemManagerUnitTest, UnexcludeOutputDevicesTest_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest UnexcludeOutputDevicesTest_002 start");
    AudioDeviceUsage audioDevUsage = CALL_OUTPUT_DEVICES;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    std::shared_ptr<AudioDeviceDescriptor> audioDevDesc = std::make_shared<AudioDeviceDescriptor>();
    audioDevDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    audioDevDesc->networkId_ = LOCAL_NETWORK_ID;
    audioDevDesc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDevDesc->macAddress_ = "00:00:00:00:00:00";
    audioDeviceDescriptors.push_back(audioDevDesc);
    AudioSystemManager::GetInstance()->ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
    int32_t result = AudioSystemManager::GetInstance()->UnexcludeOutputDevices(audioDevUsage);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest UnexcludeOutputDevicesTest_002() result:%{public}d", result);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name   : Test GetExcludedDevices API
 * @tc.number : GetExcludedDevicesTest_001
 * @tc.desc   : Test GetExcludedDevices interface.
 */
HWTEST(AudioSystemManagerUnitTest, GetExcludedDevicesTest_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetExcludedDevicesTest_001 start");
    AudioDeviceUsage audioDevUsage = MEDIA_OUTPUT_DEVICES;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors =
        AudioSystemManager::GetInstance()->GetExcludedDevices(audioDevUsage);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetExcludedDevicesTest_001() audioDeviceDescriptors.size:%{public}zu",
        audioDeviceDescriptors.size());
    EXPECT_EQ(audioDeviceDescriptors.size(), 0);
}

/**
 * @tc.name   : Test GetExcludedDevices API
 * @tc.number : GetExcludedDevicesTest_002
 * @tc.desc   : Test GetExcludedDevices interface.
 */
HWTEST(AudioSystemManagerUnitTest, GetExcludedDevicesTest_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetExcludedDevicesTest_002 start");
    AudioDeviceUsage audioDevUsage = CALL_OUTPUT_DEVICES;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors =
        AudioSystemManager::GetInstance()->GetExcludedDevices(audioDevUsage);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetExcludedDevicesTest_002() audioDeviceDescriptors.size:%{public}zu",
        audioDeviceDescriptors.size());
    EXPECT_EQ(audioDeviceDescriptors.size(), 0);
}

/**
* @tc.name   : Test SetSelfAppVolume API
* @tc.number : SetAppVolume_001
* @tc.desc   : Test SetSelfAppVolume interface
*/
HWTEST(AudioSystemManagerUnitTest, SetSelfAppVolume_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSelfAppVolume_001 start");
    int volume = 10;
    int32_t result = AudioSystemManager::GetInstance()->SetSelfAppVolume(volume);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSelfAppVolume_001 end result:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
}

/**
* @tc.name   : Test SetSelfAppVolume API
* @tc.number : SetAppVolume_002
* @tc.desc   : Test SetSelfAppVolume interface
*/
HWTEST(AudioSystemManagerUnitTest, SetSelfAppVolume_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSelfAppVolume_002 start");
    int volume = 1000;
    int32_t result = AudioSystemManager::GetInstance()->SetSelfAppVolume(volume);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSelfAppVolume_002 end result:%{public}d", result);
    EXPECT_NE(result, TEST_RET_NUM);
}

#ifdef TEMP_DISABLE
/**
* @tc.name   : Test SetAppVolume API
* @tc.number : SetAppVolume_001
* @tc.desc   : Test SetSelfAppVolume interface
*/
HWTEST(AudioSystemManagerUnitTest, SetAppVolume_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAppVolume_001 start");
    int32_t appUid = 30003000;
    int32_t volume = 10;
    int32_t result = AudioSystemManager::GetInstance()->SetAppVolume(appUid, volume);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAppVolume_001 end result:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
}

/**
* @tc.name   : Test SetAppVolume API
* @tc.number : SetAppVolume_002
* @tc.desc   : Test SetSelfAppVolume interface
*/
HWTEST(AudioSystemManagerUnitTest, SetAppVolume_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAppVolume_002 start");
    int32_t appUid = 30003000;
    int32_t volume = 1000;
    int32_t result = AudioSystemManager::GetInstance()->SetAppVolume(appUid, volume);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAppVolume_002 end result:%{public}d", result);
    EXPECT_NE(result, TEST_RET_NUM);
}
#endif

/**
* @tc.name   : Test GetSelfAppVolume API
* @tc.number : GetSelfAppVolume_001
* @tc.desc   : Test GetSelfAppVolume interface
*/
HWTEST(AudioSystemManagerUnitTest, GetSelfAppVolume_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetSelfAppVolume_001 start");
    int volume = 10;
    int32_t result = AudioSystemManager::GetInstance()->SetSelfAppVolume(volume);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSelfAppVolume end result:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
    result = AudioSystemManager::GetInstance()->GetSelfAppVolume(volume);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetSelfAppVolume_001 end result:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
}

#ifdef TEMP_DISABLE
/**
* @tc.name   : Test GetAppVolume API
* @tc.number : GetAppVolume_001
* @tc.desc   : Test GetAppVolume_001 interface
*/
HWTEST(AudioSystemManagerUnitTest, GetAppVolume_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetAppVolume_001 start");
    int32_t appUid = 30003000;
    int volume = 10;
    int result = AudioSystemManager::GetInstance()->SetAppVolume(appUid, volume);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAppVolume end result:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
    result = AudioSystemManager::GetInstance()->GetAppVolume(appUid, volume);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetAppVolume_001 end result:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
}

/**
* @tc.name   : Test GetAppVolume API
* @tc.number : GetAppVolume_002
* @tc.desc   : Test GetAppVolume_002 interface
*/
HWTEST(AudioSystemManagerUnitTest, GetAppVolume_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetAppVolume_002 start");
    int32_t appUid = 40004000;
    int volume = 0;
    int32_t result = AudioSystemManager::GetInstance()->GetAppVolume(appUid, volume);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetAppVolume_002 end result:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
}

/**
* @tc.name   : Test SetAppVolumeMuted API
* @tc.number : SetAppVolumeMuted_001
* @tc.desc   : Test SetAppVolumeMuted interface
*/
HWTEST(AudioSystemManagerUnitTest, SetAppVolumeMuted_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAppVolumeMuted_001 start");
    int appUid = 30003000;
    bool mute = true;
    int32_t result = AudioSystemManager::GetInstance()->SetAppVolumeMuted(appUid, mute);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAppVolumeMuted_001 end result:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
}

/**
* @tc.name   : Test SetAppVolumeMuted API
* @tc.number : SetAppVolumeMuted_002
* @tc.desc   : Test SetAppVolumeMuted interface
*/
HWTEST(AudioSystemManagerUnitTest, SetAppVolumeMuted_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAppVolumeMuted_002 start");
    int appUid = 30003000;
    bool mute = true;
    int32_t result = AudioSystemManager::GetInstance()->SetAppVolumeMuted(appUid, mute);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAppVolumeMuted_002 end result:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
    mute = false;
    result = AudioSystemManager::GetInstance()->SetAppVolumeMuted(appUid, mute);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAppVolumeMuted_002 end result2:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
}

/**
* @tc.name   : Test IsAppVolumeMuted API
* @tc.number : IsAppVolumeMuted_001
* @tc.desc   : Test IsAppVolumeMuted interface
*/
HWTEST(AudioSystemManagerUnitTest, IsAppVolumeMuted_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsAppVolumeMuted_001 start");
    int32_t appUid = 30003000;
    bool owned = true;
    bool mute = true;
    int32_t result = AudioSystemManager::GetInstance()->SetAppVolumeMuted(appUid, mute);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAppVolumeMuted end result:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
    result = AudioSystemManager::GetInstance()->IsAppVolumeMute(appUid, owned, mute);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsAppVolumeMuted_001 end result:%{public}d", result);

    result = AudioSystemManager::GetInstance()->IsAppVolumeMute(appUid + 1, owned, mute);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsAppVolumeMuted_001 end result2:%{public}d", result);
    EXPECT_EQ(mute, false);
}

/**
* @tc.name   : Test IsAppVolumeMuted API
* @tc.number : IsAppVolumeMuted_002
* @tc.desc   : Test IsAppVolumeMuted interface
*/
HWTEST(AudioSystemManagerUnitTest, IsAppVolumeMuted_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsAppVolumeMuted_002 start");
    int32_t appUid = 30003000;
    bool owned = false;
    bool mute = true;
    int32_t result = AudioSystemManager::GetInstance()->SetAppVolumeMuted(appUid, mute);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAppVolumeMuted end result:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
    result = AudioSystemManager::GetInstance()->IsAppVolumeMute(appUid, owned, mute);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsAppVolumeMuted_002 end result:%{public}d", result);

    result = AudioSystemManager::GetInstance()->IsAppVolumeMute(appUid + 1, owned, mute);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsAppVolumeMuted_002 end result2:%{public}d", result);
    EXPECT_EQ(mute, false);
}
#endif

/**
* @tc.name   : Test SetSelfAppVolumeCallback API
* @tc.number : SetSelfAppVolumeCallback_001
* @tc.desc   : Test SetSelfAppVolumeCallback interface
*/
HWTEST(AudioSystemManagerUnitTest, SetSelfAppVolumeCallback_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSelfAppVolumeCallback_001 start");
    std::shared_ptr<AudioManagerAppVolumeChangeCallback> callback = nullptr;
    int32_t result = AudioSystemManager::GetInstance()->SetSelfAppVolumeCallback(callback);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSelfAppVolumeCallback end result:%{public}d", result);
    EXPECT_NE(result, TEST_RET_NUM);
    result = AudioSystemManager::GetInstance()->UnsetSelfAppVolumeCallback(callback);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest UnsetAppVolumeCallback end result:%{public}d", result);
    EXPECT_NE(result, TEST_RET_NUM);
}

/**
* @tc.name   : Test SetSelfAppVolumeCallback API
* @tc.number : SetSelfAppVolumeCallback_002
* @tc.desc   : Test SetSelfAppVolumeCallback interface
*/
HWTEST(AudioSystemManagerUnitTest, SetSelfAppVolumeCallback_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSelfAppVolumeCallback_002 start");
    std::shared_ptr<AudioManagerAppVolumeChangeCallback> callback =
        std::make_shared<AudioManagerAppVolumeChangeCallbackTest>();
    int32_t result = AudioSystemManager::GetInstance()->SetSelfAppVolumeCallback(callback);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSelfAppVolumeCallback1 end result:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
    result = AudioSystemManager::GetInstance()->SetSelfAppVolumeCallback(callback);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSelfAppVolumeCallback2 end result:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
    result = AudioSystemManager::GetInstance()->UnsetSelfAppVolumeCallback(callback);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest UnsetSelfAppVolumeCallback end result:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
}

/**
* @tc.name   : Test SetSelfAppVolumeCallback API
* @tc.number : SetSelfAppVolumeCallback_003
* @tc.desc   : Test SetSelfAppVolumeCallback interface
*/
HWTEST(AudioSystemManagerUnitTest, SetSelfAppVolumeCallback_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSelfAppVolumeCallback_003 start");
    std::shared_ptr<AudioManagerAppVolumeChangeCallback> callback1 =
        std::make_shared<AudioManagerAppVolumeChangeCallbackTest>();
    std::shared_ptr<AudioManagerAppVolumeChangeCallback> callback2 =
        std::make_shared<AudioManagerAppVolumeChangeCallbackTest>();
    int32_t result = AudioSystemManager::GetInstance()->SetSelfAppVolumeCallback(callback1);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSelfAppVolumeCallback1 end result:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
    result = AudioSystemManager::GetInstance()->SetSelfAppVolumeCallback(callback2);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSelfAppVolumeCallback2 end result:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
    result = AudioSystemManager::GetInstance()->UnsetSelfAppVolumeCallback(callback2);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest UnsetSelfAppVolumeCallback end result:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
}

/**
* @tc.name   : Test SetSelfAppVolumeCallback API
* @tc.number : SetSelfAppVolumeCallback_004
* @tc.desc   : Test SetSelfAppVolumeCallback interface
*/
HWTEST(AudioSystemManagerUnitTest, SetSelfAppVolumeCallback_004, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSelfAppVolumeCallback_004 start");
    std::shared_ptr<AudioManagerAppVolumeChangeCallback> callback =
        std::make_shared<AudioManagerAppVolumeChangeCallbackTest>();
    int32_t result = AudioSystemManager::GetInstance()->SetSelfAppVolumeCallback(callback);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSelfAppVolumeCallback1 end result:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
    result = AudioSystemManager::GetInstance()->UnsetSelfAppVolumeCallback(callback);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest UnsetSelfAppVolumeCallback end result:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
}

/**
* @tc.name   : Test SetSelfAppVolumeCallback API
* @tc.number : SetSelfAppVolumeCallback_005
* @tc.desc   : Test SetSelfAppVolumeCallback interface
*/
HWTEST(AudioSystemManagerUnitTest, SetSelfAppVolumeCallback_005, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSelfAppVolumeCallback_005 start");
    std::shared_ptr<AudioManagerAppVolumeChangeCallback> callback =
        std::make_shared<AudioManagerAppVolumeChangeCallbackTest>();
    int32_t result = AudioSystemManager::GetInstance()->SetSelfAppVolumeCallback(callback);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSelfAppVolumeCallback1 end result:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
    result = AudioSystemManager::GetInstance()->UnsetSelfAppVolumeCallback(nullptr);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest UnsetSelfAppVolumeCallback end result:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
}

/**
* @tc.name   : Test SetAppVolumeCallbackForUid API
* @tc.number : SetAppVolumeCallbackForUid_001
* @tc.desc   : Test SetAppVolumeCallbackForUid interface
*/
HWTEST(AudioSystemManagerUnitTest, SetAppVolumeCallbackForUid_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAppVolumeCallbackForUid_001 start");
    int32_t appUid = 30003000;
    std::shared_ptr<AudioManagerAppVolumeChangeCallback> callback = nullptr;
    int32_t result = AudioSystemManager::GetInstance()->SetAppVolumeCallbackForUid(appUid, callback);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAppVolumeCallbackForUid_001 end result:%{public}d", result);
    EXPECT_NE(result, TEST_RET_NUM);
}
 
/**
 * @tc.name   : Test StartGroup API
 * @tc.number : StartGroup_001
 * @tc.desc   : Test StartGroup interface when startTime > endTime.
 */
HWTEST(AudioSystemManagerUnitTest, StartGroup_001, TestSize.Level1)
{
    AudioSystemManager manager;
    bool needUpdatePrio = true;
    int32_t testWorkgroupid = 1;
    int32_t startTimeMs = 1000;
    int32_t endTimeMs = 500;
    std::unordered_map<int32_t, bool> threads = {
        {101, true},
        {102, true}
    };
    int32_t result = manager.StartGroup(testWorkgroupid, startTimeMs, endTimeMs, threads, needUpdatePrio);
    EXPECT_EQ(result, AUDIO_ERR);
}

#ifdef TEMP_DISABLE
/**
* @tc.name   : Test SetAppVolumeCallbackForUid API
* @tc.number : SetAppVolumeCallbackForUid_002
* @tc.desc   : Test SetAppVolumeCallbackForUid interface
*/
HWTEST(AudioSystemManagerUnitTest, SetAppVolumeCallbackForUid_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAppVolumeCallbackForUid_002 start");
    int32_t appUid = 30003000;
    std::shared_ptr<AudioManagerAppVolumeChangeCallback> callback =
        std::make_shared<AudioManagerAppVolumeChangeCallbackTest>();
    int32_t result = AudioSystemManager::GetInstance()->SetAppVolumeCallbackForUid(appUid, callback);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAppVolumeCallbackForUid_002 end result:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
    result = AudioSystemManager::GetInstance()->SetAppVolumeCallbackForUid(appUid, callback);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAppVolumeCallbackForUid_002 end result2:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
    result = AudioSystemManager::GetInstance()->UnsetAppVolumeCallbackForUid(callback);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest UnsetAppVolumeCallbackForUid end result:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
}

/**
* @tc.name   : Test SetAppVolumeCallbackForUid API
* @tc.number : SetAppVolumeCallbackForUid_003
* @tc.desc   : Test SetAppVolumeCallbackForUid interface
*/
HWTEST(AudioSystemManagerUnitTest, SetAppVolumeCallbackForUid_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAppVolumeCallbackForUid_003 start");
    int32_t appUid = 30003000;
    std::shared_ptr<AudioManagerAppVolumeChangeCallback> callback1 =
        std::make_shared<AudioManagerAppVolumeChangeCallbackTest>();
    std::shared_ptr<AudioManagerAppVolumeChangeCallback> callback2 =
        std::make_shared<AudioManagerAppVolumeChangeCallbackTest>();
    int32_t result = AudioSystemManager::GetInstance()->SetAppVolumeCallbackForUid(appUid, callback1);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAppVolumeCallbackForUid_003 end result:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
    result = AudioSystemManager::GetInstance()->SetAppVolumeCallbackForUid(appUid, callback2);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAppVolumeCallbackForUid_003 end result2:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
    result = AudioSystemManager::GetInstance()->UnsetAppVolumeCallbackForUid(callback2);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest UnsetAppVolumeCallbackForUid end result:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
}

/**
* @tc.name   : Test SetAppVolumeCallbackForUid API
* @tc.number : SetAppVolumeCallbackForUid_004
* @tc.desc   : Test SetAppVolumeCallbackForUid interface
*/
HWTEST(AudioSystemManagerUnitTest, SetAppVolumeCallbackForUid_004, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAppVolumeCallbackForUid_004 start");
    int32_t appUid = 30003000;
    std::shared_ptr<AudioManagerAppVolumeChangeCallback> callback =
        std::make_shared<AudioManagerAppVolumeChangeCallbackTest>();
    int32_t result = AudioSystemManager::GetInstance()->SetAppVolumeCallbackForUid(appUid, callback);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAppVolumeCallbackForUid_004 end result:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
    result = AudioSystemManager::GetInstance()->UnsetAppVolumeCallbackForUid(callback);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest UnsetAppVolumeCallbackForUid end result:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
}

/**
* @tc.name   : Test SetAppVolumeCallbackForUid API
* @tc.number : SetAppVolumeCallbackForUid_005
* @tc.desc   : Test SetAppVolumeCallbackForUid interface
*/
HWTEST(AudioSystemManagerUnitTest, SetAppVolumeCallbackForUid_005, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAppVolumeCallbackForUid_005 start");
    int32_t appUid = 30003000;
    std::shared_ptr<AudioManagerAppVolumeChangeCallback> callback =
        std::make_shared<AudioManagerAppVolumeChangeCallbackTest>();
    int32_t result = AudioSystemManager::GetInstance()->SetAppVolumeCallbackForUid(appUid, callback);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetAppVolumeCallbackForUid_005 end result:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
    result = AudioSystemManager::GetInstance()->UnsetAppVolumeCallbackForUid(nullptr);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest UnsetAppVolumeCallbackForUid end result:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
}

/**
* @tc.name   : Test RegisterRendererDataTransfer API
* @tc.number : RegisterRendererDataTransfer_001
* @tc.desc   : Test RegisterRendererDataTransfer interface
*/
HWTEST(AudioSystemManagerUnitTest, RegisterRendererDataTransfer_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest RegisterRendererDataTransfer_001 start");
    std::shared_ptr<AudioRendererDataTransferStateChangeCallback> callback =
        std::make_shared<DataTransferStateChangeCallbackTest>();
    DataTransferMonitorParam param1;
    int32_t result = AudioSystemManager::GetInstance()->RegisterRendererDataTransferCallback(param1, callback);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest RegisterRendererDataTransfer_001 end result:%{public}d", result);
    EXPECT_EQ(result, SUCCESS);

    DataTransferMonitorParam param2;
    result = AudioSystemManager::GetInstance()->RegisterRendererDataTransferCallback(param2, callback);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest RegisterRendererDataTransfer_001 end result:%{public}d", result);
    EXPECT_EQ(result, SUCCESS);

    result = AudioSystemManager::GetInstance()->UnregisterRendererDataTransferCallback(callback);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest RegisterRendererDataTransfer_001 end result:%{public}d", result);
    EXPECT_EQ(result, SUCCESS);
}
#endif

/**
 * @tc.name   : Test CreateGroup API
 * @tc.number : CreateGroup_001
 * @tc.desc   : Test CreateGroup interface createAudioWorkgroup
 */
HWTEST(AudioSystemManagerUnitTest, CreateGroup_001, TestSize.Level1)
{
    AudioSystemManager audioSystemManager;

    int32_t result = audioSystemManager.CreateAudioWorkgroup();
    EXPECT_GT(result, 0);
}

/**
 * @tc.name   : Test WorkgroupPrioRecorder constructor
 * @tc.number : WorkgroupPrioRecorder_001
 * @tc.desc   : Test WorkgroupPrioRecorder constructor
 */
HWTEST(AudioSystemManagerUnitTest, WorkgroupPrioRecorder_001, TestSize.Level1)
{
    int32_t grpId = 1;
    AudioSystemManager::WorkgroupPrioRecorder recorder(grpId);
    EXPECT_EQ(recorder.grpId_, grpId);
    EXPECT_EQ(recorder.restoreByPermission_, false);
}
 
/**
 * @tc.name   : Test SetRestoreByPermission
 * @tc.number : SetRestoreByPermission_001
 * @tc.desc   : Test SetRestoreByPermission when isByPermission true
 */
HWTEST(AudioSystemManagerUnitTest, SetRestoreByPermission_001, TestSize.Level1)
{
    AudioSystemManager::WorkgroupPrioRecorder recorder(1);
    recorder.SetRestoreByPermission(true);
    EXPECT_TRUE(recorder.restoreByPermission_);
}
 
/**
 * @tc.name   : Test SetRestoreByPermission
 * @tc.number : SetRestoreByPermission_002
 * @tc.desc   : Test SetRestoreByPermission when isByPermission false
 */
HWTEST(AudioSystemManagerUnitTest, SetRestoreByPermission_002, TestSize.Level1)
{
    AudioSystemManager::WorkgroupPrioRecorder recorder(1);
    recorder.SetRestoreByPermission(false);
    EXPECT_FALSE(recorder.restoreByPermission_);
}
 
/**
 * @tc.name   : Test GetRestoreByPermission
 * @tc.number : GetRestoreByPermission_001
 * @tc.desc   : Test SetRestoreByPermission when permission is set
 */
HWTEST(AudioSystemManagerUnitTest, GetRestoreByPermission_001, TestSize.Level1)
{
    AudioSystemManager::WorkgroupPrioRecorder recorder(1);
    recorder.restoreByPermission_ = true;
    EXPECT_TRUE(recorder.GetRestoreByPermission());
}
 
/**
 * @tc.name   : Test GetRestoreByPermission
 * @tc.number : GetRestoreByPermission_002
 * @tc.desc   : Test SetRestoreByPermission when permission is not set
 */
HWTEST(AudioSystemManagerUnitTest, GetRestoreByPermission_002, TestSize.Level1)
{
    AudioSystemManager::WorkgroupPrioRecorder recorder(1);
    recorder.restoreByPermission_ = false;
    EXPECT_FALSE(recorder.GetRestoreByPermission());
}
 
/**
 * @tc.name   : Test RecordThreadPrio
 * @tc.number : RecordThreadPrio_001
 * @tc.desc   : Test RecordThreadPrio inteface
 */
HWTEST(AudioSystemManagerUnitTest, RecordThreadPrio_001, TestSize.Level1)
{
    AudioSystemManager::WorkgroupPrioRecorder recorder(1);
    int32_t tokenId = 1;
 
    // Add the tokenId to the threads_ map
    recorder.threads_[tokenId] = 2;
 
    // Call the method under test
    recorder.RecordThreadPrio(tokenId);
 
    // Verify the result
    auto it = recorder.threads_.find(tokenId);
    ASSERT_TRUE(it != recorder.threads_.end());
    EXPECT_EQ(it->second, 2);
}
 
/**
 * @tc.name   : Test RestoreGroupPrio
 * @tc.number : RestoreGroupPrio_001
 * @tc.desc   : Test RestoreGroupPrio set permission
 */
HWTEST(AudioSystemManagerUnitTest, RestoreGroupPrio_001, TestSize.Level1)
{
    AudioSystemManager::WorkgroupPrioRecorder recorder(1);
    int32_t result = recorder.RestoreGroupPrio(true);
    EXPECT_EQ(result, AUDIO_OK);
    EXPECT_TRUE(recorder.restoreByPermission_);
}
 
/**
 * @tc.name   : Test RestoreGroupPrio
 * @tc.number : RestoreGroupPrio_002
 * @tc.desc   : Test RestoreGroupPrio not set permission
 */
HWTEST(AudioSystemManagerUnitTest, RestoreGroupPrio_002, TestSize.Level1)
{
    AudioSystemManager::WorkgroupPrioRecorder recorder(1);
    int32_t result = recorder.RestoreGroupPrio(false);
    EXPECT_EQ(result, AUDIO_OK);
    EXPECT_TRUE(recorder.threads_.empty());
}
 
/**
 * @tc.name   : Test RestoreThreadPrio
 * @tc.number : RestoreThreadPrio_001
 * @tc.desc   : Test RestoreThreadPrio when tokenId not exist
 */
HWTEST(AudioSystemManagerUnitTest, RestoreThreadPrio_001, TestSize.Level1)
{
    AudioSystemManager::WorkgroupPrioRecorder recorder(1);
    int32_t tokenId = 1;
    recorder.threads_[tokenId] = 1;
    int32_t result = recorder.RestoreThreadPrio(tokenId + 1);
    EXPECT_EQ(result, AUDIO_OK);
}
 
/**
 * @tc.name   : Test RestoreThreadPrio
 * @tc.number : RestoreThreadPrio_002
 * @tc.desc   : Test RestoreThreadPrio when tokenId exist
 */
HWTEST(AudioSystemManagerUnitTest, RestoreThreadPrio_002, TestSize.Level1)
{
    AudioSystemManager::WorkgroupPrioRecorder recorder(1);
    int32_t tokenId = 1;
    recorder.threads_[tokenId] = 1;
    int32_t result = recorder.RestoreThreadPrio(tokenId);
    EXPECT_EQ(result, AUDIO_OK);
}
 
/**
 * @tc.name   : Test RestoreThreadPrio
 * @tc.number : RestoreThreadPrio_003
 * @tc.desc   : Test RestoreThreadPrio check tokenId
 */
HWTEST(AudioSystemManagerUnitTest, RestoreThreadPrio_003, TestSize.Level1)
{
    AudioSystemManager::WorkgroupPrioRecorder recorder(1);
    int32_t tokenId = 1;
    recorder.threads_[tokenId] = 1;
    int32_t result = recorder.RestoreThreadPrio(tokenId);
    EXPECT_EQ(result, AUDIO_OK);
    EXPECT_EQ(recorder.threads_.find(tokenId), recorder.threads_.end());
}
 
/**
 * @tc.name   : Test GetGrpId
 * @tc.number : GetGrpId_001
 * @tc.desc   : Test GetGrpId when call
 */
HWTEST(AudioSystemManagerUnitTest, GetGrpId_001, TestSize.Level1)
{
    AudioSystemManager::WorkgroupPrioRecorder recorder(1);
    recorder.grpId_ = 100;
    EXPECT_EQ(recorder.GetGrpId(), 100);
}
 
/**
 * @tc.name   : Test GetRecorderByGrpId
 * @tc.number : GetRecorderByGrpId_001
 * @tc.desc   : Test GetRecorderByGrpId when grpId exist
 */
HWTEST(AudioSystemManagerUnitTest, GetRecorderByGrpId_001, TestSize.Level1)
{
    AudioSystemManager manager;
    int32_t grpId = 1;
    auto recorder = std::make_shared<AudioSystemManager::WorkgroupPrioRecorder>(1);
    manager.workgroupPrioRecorderMap[grpId] = recorder;
    auto result = manager.GetRecorderByGrpId(grpId);
    EXPECT_EQ(result, recorder);
}
 
/**
 * @tc.name   : Test GetRecorderByGrpId
 * @tc.number : GetRecorderByGrpId_002
 * @tc.desc   : Test GetRecorderByGrpId when grpId not exist
 */
HWTEST(AudioSystemManagerUnitTest, GetRecorderByGrpId_002, TestSize.Level1)
{
    AudioSystemManager manager;
    int32_t grpId = 1;
    auto result = manager.GetRecorderByGrpId(grpId);
    EXPECT_EQ(result, nullptr);
}
 
/**
 * @tc.name   : Test OnWorkgroupChange
 * @tc.number : OnWorkgroupChange_001
 * @tc.desc   : Test OnWorkgroupChange when allowed is true
 */
HWTEST(AudioSystemManagerUnitTest, OnWorkgroupChange_001, TestSize.Level1)
{
    AudioSystemManager manager;
    AudioWorkgroupChangeInfo info;
    info.pid = 1;
    info.groupId = 1;
    info.startAllowed = true;
 
    manager.OnWorkgroupChange(info);
 
    // Check if the permission is set correctly
    EXPECT_EQ(manager.startGroupPermissionMap_[info.pid][info.groupId], info.startAllowed);
}
 
/**
 * @tc.name   : Test OnWorkgroupChange
 * @tc.number : OnWorkgroupChange_002
 * @tc.desc   : Test OnWorkgroupChange when allowed is false
 */
HWTEST(AudioSystemManagerUnitTest, OnWorkgroupChange_002, TestSize.Level1)
{
    AudioSystemManager manager;
    AudioWorkgroupChangeInfo info;
    info.pid = 1;
    info.groupId = 1;
    info.startAllowed = false;
 
    manager.OnWorkgroupChange(info);
 
    // Check if the permission is set correctly
    EXPECT_EQ(manager.startGroupPermissionMap_[info.pid][info.groupId], info.startAllowed);
}
 
/**
 * @tc.name   : Test OnWorkgroupChange
 * @tc.number : OnWorkgroupChange_003
 * @tc.desc   : Test OnWorkgroupChange when recorder is nullptr
 */
HWTEST(AudioSystemManagerUnitTest, OnWorkgroupChange_003, TestSize.Level1)
{
    AudioSystemManager manager;
    AudioWorkgroupChangeInfo info;
    info.pid = 1;
    info.groupId = 1;
    info.startAllowed = false;
 
    manager.OnWorkgroupChange(info);
 
    // Check if the permission is set correctly
    EXPECT_EQ(manager.startGroupPermissionMap_[info.pid][info.groupId], info.startAllowed);
    // Check if the recorder is nullptr
    EXPECT_EQ(manager.GetRecorderByGrpId(info.groupId), nullptr);
}
} // namespace AudioStandard
} // namespace OHOS
