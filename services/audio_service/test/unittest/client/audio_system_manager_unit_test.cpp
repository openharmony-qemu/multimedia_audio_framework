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
 * @tc.name   : Test GetVolumeEvent API
 * @tc.number : GetVolumeEvent_001
 * @tc.desc   : Test GetVolumeEvent interface when VolumeEvent is not empty.
 */
HWTEST(AudioSystemManagerUnitTest, GetVolumeEvent_001, TestSize.Level1)
{
    std::unordered_map<AudioStreamType, VolumeEvent> expectedVolumeEventMap;
    expectedVolumeEventMap[AudioStreamType::STREAM_MUSIC] = VolumeEvent();
    expectedVolumeEventMap[AudioStreamType::STREAM_RING] = VolumeEvent();
 
    AudioSystemManager::GetInstance()->volumeEventMap_ = expectedVolumeEventMap;
 
    auto actualVolumeEventMap = AudioSystemManager::GetInstance()->GetVolumeEvent();
 
    EXPECT_EQ(actualVolumeEventMap[AudioStreamType::STREAM_MUSIC].volumeType, expectedVolumeEventMap[AudioStreamType::STREAM_MUSIC].volumeType);
    EXPECT_EQ(actualVolumeEventMap[AudioStreamType::STREAM_MUSIC].volume, expectedVolumeEventMap[AudioStreamType::STREAM_MUSIC].volume);
    EXPECT_EQ(actualVolumeEventMap[AudioStreamType::STREAM_MUSIC].updateUi, expectedVolumeEventMap[AudioStreamType::STREAM_MUSIC].updateUi);
    EXPECT_EQ(actualVolumeEventMap[AudioStreamType::STREAM_RING].volumeType, expectedVolumeEventMap[AudioStreamType::STREAM_RING].volumeType);
    EXPECT_EQ(actualVolumeEventMap[AudioStreamType::STREAM_RING].volume, expectedVolumeEventMap[AudioStreamType::STREAM_RING].volume);
    EXPECT_EQ(actualVolumeEventMap[AudioStreamType::STREAM_RING].updateUi, expectedVolumeEventMap[AudioStreamType::STREAM_RING].updateUi);
}
 
/**
 * @tc.name   : Test GetVolumeEvent API
 * @tc.number : GetVolumeEvent_002
 * @tc.desc   : Test GetVolumeEvent interface when VolumeEvent is empty.
 */
HWTEST(AudioSystemManagerUnitTest, GetVolumeEvent_002, TestSize.Level1)
{
    std::unordered_map<AudioStreamType, VolumeEvent> expectedVolumeEventMap;
 
    AudioSystemManager::GetInstance()->volumeEventMap_ = expectedVolumeEventMap;
 
    auto actualVolumeEventMap = AudioSystemManager::GetInstance()->GetVolumeEvent();
 
    EXPECT_TRUE(actualVolumeEventMap.empty());
}
 
/**
 * @tc.name   : Test SetVolumeEvent API
 * @tc.number : SetVolumeEvent_001
 * @tc.desc   : Test SetVolumeEvent interface when AudioStreamType is MUSIC.
 */
HWTEST(AudioSystemManagerUnitTest, SetVolumeEvent_001, TestSize.Level1)
{
    AudioSystemManager manager;
    VolumeEvent event = VolumeEvent();
 
    manager.SetVolumeEvent(AudioStreamType::STREAM_MUSIC, event);
 
    EXPECT_EQ(manager.volumeEventMap_[AudioStreamType::STREAM_MUSIC].volumeType, event.volumeType);
}
 
/**
 * @tc.name   : Test SetVolumeEvent API
 * @tc.number : SetVolumeEvent_002
 * @tc.desc   : Test SetVolumeEvent interface when AudioStreamType is VOICE_COMMUNICATION.
 */
HWTEST(AudioSystemManagerUnitTest, SetVolumeEvent_002, TestSize.Level1)
{
    AudioSystemManager manager;
    VolumeEvent event = VolumeEvent();
 
    manager.SetVolumeEvent(AudioStreamType::STREAM_VOICE_COMMUNICATION, event);
 
    EXPECT_EQ(manager.volumeEventMap_[AudioStreamType::STREAM_VOICE_COMMUNICATION].volumeType, event.volumeType);
}
 
 
/**
 * @tc.name   : Test GetAudioRendererChangeInfo API
 * @tc.number : GetAudioRendererChangeInfo_001
 * @tc.desc   : Test GetAudioRendererChangeInfo interface
 */
HWTEST(AudioSystemManagerUnitTest, GetAudioRendererChangeInfo_001, TestSize.Level1)
{
    AudioSystemManager manager;
 
    std::unordered_map<AudioStreamType, std::shared_ptr<AudioRendererChangeInfo>> expectedMap;
    expectedMap[AudioStreamType::STREAM_MUSIC] = std::make_shared<AudioRendererChangeInfo>();
    expectedMap[AudioStreamType::STREAM_ALARM] = std::make_shared<AudioRendererChangeInfo>();
 
    manager.audioRendererChangeInfoMap_ = expectedMap;
 
    auto actualMap = manager.GetAudioRendererChangeInfo();
    
    EXPECT_EQ(actualMap, expectedMap);
}
 
/**
 * @tc.name   : Test SetAudioRendererChangeInfo API
 * @tc.number : SetAudioRendererChangeInfo_001
 * @tc.desc   : Test SetAudioRendererChangeInfo interface when set once.
 */
HWTEST(AudioSystemManagerUnitTest, SetAudioRendererChangeInfo_001, TestSize.Level1)
{
    AudioStreamType type = AudioStreamType::STREAM_MUSIC;
    auto info = std::make_shared<AudioRendererChangeInfo>();
 
    AudioSystemManager::GetInstance()->SetAudioRendererChangeInfo(type, info);
 
    auto storedInfo = AudioSystemManager::GetInstance()->audioRendererChangeInfoMap_[type];
    EXPECT_EQ(storedInfo, info);
}
 
/**
 * @tc.name   : Test SetAudioRendererChangeInfo API
 * @tc.number : SetAudioRendererChangeInfo_002
 * @tc.desc   : Test SetAudioRendererChangeInfo interface when set twice.
 */
HWTEST(AudioSystemManagerUnitTest, SetAudioRendererChangeInfo_002, TestSize.Level1)
{
    AudioStreamType type = AudioStreamType::STREAM_MUSIC;
    auto info1 = std::make_shared<AudioRendererChangeInfo>();
    auto info2 = std::make_shared<AudioRendererChangeInfo>();
 
    AudioSystemManager::GetInstance()->SetAudioRendererChangeInfo(type, info1);
 
    AudioSystemManager::GetInstance()->SetAudioRendererChangeInfo(type, info2);
 
    auto storedInfo = AudioSystemManager::GetInstance()->audioRendererChangeInfoMap_[type];
    EXPECT_EQ(storedInfo, info2);
}
 
/**
 * @tc.name   : Test IsValidStreamType API
 * @tc.number : IsValidStreamType_001
 * @tc.desc   : Test IsValidStreamType interface
 */
HWTEST(AudioSystemManagerUnitTest, IsValidStreamType_001, TestSize.Level1)
{
    AudioSystemManager manager;
    EXPECT_FALSE(manager.IsValidStreamType(AudioStreamType::STREAM_ALARM));
    EXPECT_TRUE(manager.IsValidStreamType(AudioStreamType::STREAM_MUSIC));
    EXPECT_TRUE(manager.IsValidStreamType(AudioStreamType::STREAM_VOICE_COMMUNICATION));
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
    int32_t startTime_ms = 1000;
    int32_t endTime_ms = 500;
    std::unordered_map<int32_t, bool> threads = {
        {101, true}, 
        {102, true}
    };
    int32_t result = manager.StartGroup(testWorkgroupid, startTime_ms, endTime_ms, threads, needUpdatePrio);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}
 
/**
 * @tc.name   : Test StartGroup API
 * @tc.number : StartGroup_002
 * @tc.desc   : Test StartGroup interface when startTime < endTime.
 */
HWTEST(AudioSystemManagerUnitTest, StartGroup_002, TestSize.Level1)
{
    AudioSystemManager manager;
    bool needUpdatePrio = true;
    int32_t testWorkgroupid = 1;
    int32_t startTime_ms = 1000;
    int32_t endTime_ms = 2000;
    std::unordered_map<int32_t, bool> threads = {
        {101, true}, 
        {102, true}
    };
    int32_t result = manager.StartGroup(1, startTime_ms, endTime_ms, threads, needUpdatePrio);
    EXPECT_EQ(result, AUDIO_OK);
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
} // namespace AudioStandard
} // namespace OHOS