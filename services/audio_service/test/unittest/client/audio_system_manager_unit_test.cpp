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
#include "audio_workgroup_callback_impl.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

const int32_t TEST_RET_NUM = 0;
const int32_t TEST_RET_MAX_VOLUME = 15;
const StreamUsage ILLEGAL_STREAM_USAGE = static_cast<StreamUsage>(static_cast<int32_t>(STREAM_USAGE_MAX)+999);
const int32_t TEST_RET_ERROR_NOT_SUPPORTED = ERR_NOT_SUPPORTED;

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
    void OnMuteStateChange(const int32_t &uid, const uint32_t &sessionId, const bool &isMuted) override {}
};

class SystemVolumeChangeCallbackTest : public SystemVolumeChangeCallback {
public:
    void OnSystemVolumeChange(VolumeEvent volumeEvent) override {}
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
    EXPECT_GT(TEST_RET_NUM, result);
    result = AudioSystemManager::GetInstance()->GetMaxVolume(STREAM_ULTRASONIC);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetMaxVolume_001 result2:%{public}d", result);
    EXPECT_GT(TEST_RET_NUM, result);
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
    EXPECT_EQ(result, -1);
    result = AudioSystemManager::GetInstance()->GetMinVolume(STREAM_ULTRASONIC);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetMinVolume_001 result2:%{public}d", result);
    EXPECT_EQ(result, -1);
}

/**
 * @tc.name  : Test GetDeviceMaxVolume API
 * @tc.type  : FUNC
 * @tc.number: GetDeviceMaxVolume_001
 * @tc.desc  : Test GetDeviceMaxVolume interface.
 */
HWTEST(AudioSystemManagerUnitTest, GetDeviceMaxVolume_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetDeviceMaxVolume_001 start");
    int32_t result = AudioSystemManager::GetInstance()->GetDeviceMaxVolume(STREAM_ALL, DEVICE_TYPE_NONE);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetDeviceMaxVolume_001 result1:%{public}d", result);
    EXPECT_GT(TEST_RET_NUM, result);
    result = AudioSystemManager::GetInstance()->GetDeviceMaxVolume(STREAM_ULTRASONIC, DEVICE_TYPE_NONE);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetDeviceMaxVolume_001 result2:%{public}d", result);
    EXPECT_GT(TEST_RET_NUM, result);
    result = AudioSystemManager::GetInstance()->GetDeviceMaxVolume(STREAM_MUSIC, DEVICE_TYPE_SPEAKER);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetDeviceMaxVolume_001 result3:%{public}d", result);
    EXPECT_GT(TEST_RET_NUM, result);
    result = AudioSystemManager::GetInstance()->GetDeviceMaxVolume(STREAM_MUSIC, DEVICE_TYPE_BLUETOOTH_A2DP);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetDeviceMaxVolume_001 result4:%{public}d", result);
    EXPECT_GT(TEST_RET_NUM, result);
}

/**
 * @tc.name  : Test GetDeviceMaxVolume API
 * @tc.type  : FUNC
 * @tc.number: GetDeviceMaxVolume_002
 * @tc.desc  : Test GetDeviceMaxVolume interface.
 */
HWTEST(AudioSystemManagerUnitTest, GetDeviceMaxVolume_002, TestSize.Level1)
{
    int32_t result = AudioSystemManager::GetInstance()->GetDeviceMaxVolume(STREAM_ALL, DEVICE_TYPE_SPEAKER);
    EXPECT_NE(result, ERR_PERMISSION_DENIED);
    result = AudioSystemManager::GetInstance()->GetDeviceMaxVolume(STREAM_ULTRASONIC, DEVICE_TYPE_SPEAKER);
    EXPECT_NE(result, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name  : Test GetDeviceMinVolume API
 * @tc.type  : FUNC
 * @tc.number: GetDeviceMinVolume_001
 * @tc.desc  : Test GetDeviceMinVolume interface.
 */
HWTEST(AudioSystemManagerUnitTest, GetDeviceMinVolume_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetDeviceMinVolume_001 start");
    int32_t result = AudioSystemManager::GetInstance()->GetDeviceMinVolume(STREAM_ALL, DEVICE_TYPE_NONE);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetDeviceMinVolume_001 result1:%{public}d", result);
    EXPECT_EQ(result, -1);
    result = AudioSystemManager::GetInstance()->GetDeviceMinVolume(STREAM_ULTRASONIC, DEVICE_TYPE_NONE);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetDeviceMinVolume_001 result2:%{public}d", result);
    EXPECT_EQ(result, -1);
    result = AudioSystemManager::GetInstance()->GetDeviceMinVolume(STREAM_MUSIC, DEVICE_TYPE_SPEAKER);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetDeviceMinVolume_001 result3:%{public}d", result);
    EXPECT_EQ(result, -1);
    result = AudioSystemManager::GetInstance()->GetDeviceMinVolume(STREAM_MUSIC, DEVICE_TYPE_BLUETOOTH_A2DP);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetDeviceMinVolume_001 result4:%{public}d", result);
    EXPECT_EQ(result, -1);
}

/**
 * @tc.name  : Test GetDeviceMinVolume API
 * @tc.type  : FUNC
 * @tc.number: GetDeviceMinVolume_002
 * @tc.desc  : Test GetDeviceMinVolume interface.
 */
HWTEST(AudioSystemManagerUnitTest, GetDeviceMinVolume_002, TestSize.Level1)
{
    int32_t result = AudioSystemManager::GetInstance()->GetDeviceMinVolume(STREAM_ALL, DEVICE_TYPE_NONE);
    EXPECT_NE(result, ERR_PERMISSION_DENIED);
    result = AudioSystemManager::GetInstance()->GetDeviceMinVolume(STREAM_ULTRASONIC, DEVICE_TYPE_NONE);
    EXPECT_NE(result, ERR_PERMISSION_DENIED);
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
    EXPECT_EQ(result, false);

    result = AudioSystemManager::GetInstance()->IsStreamMute(STREAM_RING);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamMute_001 result2:%{public}d", result);
    EXPECT_EQ(result, false);

    result = AudioSystemManager::GetInstance()->IsStreamMute(STREAM_NOTIFICATION);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamMute_001 result3:%{public}d", result);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test IsStreamMute API
 * @tc.type  : FUNC
 * @tc.number: IsStreamMute_002
 * @tc.desc  : Test IsStreamMute interface.
 */
HWTEST(AudioSystemManagerUnitTest, IsStreamMute_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamMute_002 start");
    bool result = AudioSystemManager::GetInstance()->IsStreamMute(STREAM_ALL);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamMute_002 result1:%{public}d", result);
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
    EXPECT_EQ(result, -1);
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
    EXPECT_EQ(result, -1);
    result = AudioSystemManager::GetInstance()->GetSelfAppVolume(volume);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetSelfAppVolume_001 end result:%{public}d", result);
    EXPECT_EQ(result, -1);
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
 * @tc.name   : Test SetNearlinkDeviceVolume API
 * @tc.number : SetNearlinkDeviceVolume_001
 * @tc.desc   : Test SetNearlinkDeviceVolume interface createAudioWorkgroup
 */
HWTEST(AudioSystemManagerUnitTest, SetNearlinkDeviceVolume_001, TestSize.Level1)
{
    AudioSystemManager audioSystemManager;
    std::string macAddress = "LocalDevice";
    AudioVolumeType volumeType = STREAM_MUSIC;
    int32_t volume = 0;
    bool updateUi = true;

    int32_t result = audioSystemManager.SetNearlinkDeviceVolume(macAddress, volumeType, volume, updateUi);
    EXPECT_NE(result, -2);
}

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
    EXPECT_NE(result, TEST_RET_NUM);
    result = AudioSystemManager::GetInstance()->SetSelfAppVolumeCallback(callback);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSelfAppVolumeCallback2 end result:%{public}d", result);
    EXPECT_NE(result, TEST_RET_NUM);
    result = AudioSystemManager::GetInstance()->UnsetSelfAppVolumeCallback(callback);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest UnsetSelfAppVolumeCallback end result:%{public}d", result);
    EXPECT_NE(result, TEST_RET_NUM);
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
    EXPECT_NE(result, TEST_RET_NUM);
    result = AudioSystemManager::GetInstance()->SetSelfAppVolumeCallback(callback2);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest SetSelfAppVolumeCallback2 end result:%{public}d", result);
    EXPECT_NE(result, TEST_RET_NUM);
    result = AudioSystemManager::GetInstance()->UnsetSelfAppVolumeCallback(callback2);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest UnsetSelfAppVolumeCallback end result:%{public}d", result);
    EXPECT_NE(result, TEST_RET_NUM);
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
    EXPECT_NE(result, TEST_RET_NUM);
    result = AudioSystemManager::GetInstance()->UnsetSelfAppVolumeCallback(callback);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest UnsetSelfAppVolumeCallback end result:%{public}d", result);
    EXPECT_NE(result, TEST_RET_NUM);
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
    EXPECT_NE(result, TEST_RET_NUM);
    result = AudioSystemManager::GetInstance()->UnsetSelfAppVolumeCallback(nullptr);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest UnsetSelfAppVolumeCallback end result:%{public}d", result);
    EXPECT_NE(result, TEST_RET_NUM);
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

/**
 * @tc.name  : Test GetVolumeInUnitOfDb API
 * @tc.number: GetVolumeInUnitOfDb_001
 * @tc.tesc  : Test GetVolumeInUnitOfDb interface
 */
HWTEST(AudioSystemManagerUnitTest, GetVolumeInUnitOfDb_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetVolumeInUnitOfDb_001 start");
    AudioSystemManager manager;
    int32_t volLevel = 5;
    float result = manager.GetVolumeInUnitOfDb(AudioVolumeType::STREAM_MUSIC,
        volLevel,
        DeviceType::DEVICE_TYPE_SPEAKER);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetVolumeInUnitOfDb_001 result1:%{public}f", result);
    EXPECT_GE(TEST_RET_NUM, result);
}

/**
 * @tc.name  : Test GetMaxVolumeByUsage API
 * @tc.number: GetMaxVolumeByUsage_001
 * @tc.tesc  : Test GetMaxVolumeByUsage interface
 */
HWTEST(AudioSystemManagerUnitTest, GetMaxVolumeByUsage_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetMaxVolumeByUsage_001 start");
    AudioSystemManager manager;
    int32_t result = manager.GetMaxVolumeByUsage(StreamUsage::STREAM_USAGE_MUSIC);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetMaxVolumeByUsage_001 result1:%{public}d", result);
    EXPECT_GE(TEST_RET_NUM, result);
    EXPECT_LE(result, TEST_RET_MAX_VOLUME);
    result = manager.GetMaxVolumeByUsage(StreamUsage::STREAM_USAGE_ULTRASONIC);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetMaxVolumeByUsage_001 result2:%{public}d", result);
    EXPECT_GE(TEST_RET_NUM, result);
    EXPECT_LE(result, TEST_RET_MAX_VOLUME);
    result = manager.GetMaxVolumeByUsage(ILLEGAL_STREAM_USAGE);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetMaxVolumeByUsage_001 result3:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_ERROR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test GetMinVolumeByUsage API
 * @tc.number: GetMinVolumeByUsage_001
 * @tc.tesc  : Test GetMinVolumeByUsage interface
 */
HWTEST(AudioSystemManagerUnitTest, GetMinVolumeByUsage_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetMinVolumeByUsage_001 start");
    AudioSystemManager manager;
    int32_t result = manager.GetMinVolumeByUsage(StreamUsage::STREAM_USAGE_MUSIC);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetMinVolumeByUsage_001 result1:%{public}d", result);
    EXPECT_GE(TEST_RET_NUM, result);
    EXPECT_LE(result, TEST_RET_MAX_VOLUME);
    result = manager.GetMinVolumeByUsage(StreamUsage::STREAM_USAGE_ULTRASONIC);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetMinVolumeByUsage_001 result2:%{public}d", result);
    EXPECT_GE(TEST_RET_NUM, result);
    EXPECT_LE(result, TEST_RET_MAX_VOLUME);
    result = manager.GetMinVolumeByUsage(ILLEGAL_STREAM_USAGE);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetMinVolumeByUsage_001 result3:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_ERROR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test GetVolumeByUsage API
 * @tc.number: GetVolumeByUsage_001
 * @tc.tesc  : Test GetVolumeByUsage interface
 */
HWTEST(AudioSystemManagerUnitTest, GetVolumeByUsage_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetVolumeByUsage_001 start");
    AudioSystemManager manager;
    int32_t result = manager.GetVolumeByUsage(StreamUsage::STREAM_USAGE_MUSIC);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetVolumeByUsage_001 result1:%{public}d", result);
    EXPECT_GE(TEST_RET_NUM, result);
    EXPECT_LE(result, TEST_RET_MAX_VOLUME);
    result = manager.GetVolumeByUsage(StreamUsage::STREAM_USAGE_ULTRASONIC);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetVolumeByUsage_001 result2:%{public}d", result);
    EXPECT_GE(TEST_RET_NUM, result);
    EXPECT_LE(result, TEST_RET_MAX_VOLUME);
    result = manager.GetVolumeByUsage(ILLEGAL_STREAM_USAGE);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetVolumeByUsage_001 result3:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_ERROR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test IsStreamMuteByUsage API
 * @tc.number: IsStreamMuteByUsage_001
 * @tc.tesc  : Test IsStreamMuteByUsage interface
 */
HWTEST(AudioSystemManagerUnitTest, IsStreamMuteByUsage_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamMuteByUsage_001 start");
    AudioSystemManager manager;
    bool isMuted = false;
    int32_t result = manager.IsStreamMuteByUsage(StreamUsage::STREAM_USAGE_MUSIC, isMuted);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamMuteByUsage_001 result1:%{public}d", result);
    EXPECT_EQ(result, SUCCESS);
    result = manager.IsStreamMuteByUsage(StreamUsage::STREAM_USAGE_ULTRASONIC, isMuted);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamMuteByUsage_001 result2:%{public}d", result);
    EXPECT_EQ(result, SUCCESS);
    result = manager.IsStreamMuteByUsage(ILLEGAL_STREAM_USAGE, isMuted);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamMuteByUsage_001 result3:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_ERROR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test GetVolumeInDbByStream API
 * @tc.number: GetVolumeInDbByStream_001
 * @tc.tesc  : Test GetVolumeInDbByStream interface
 */
HWTEST(AudioSystemManagerUnitTest, GetVolumeInDbByStream_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetVolumeInDbByStream_001 start");
    AudioSystemManager manager;
    int32_t volLevel = 5;
    float result = manager.GetVolumeInDbByStream(StreamUsage::STREAM_USAGE_MUSIC,
        volLevel,
        DeviceType::DEVICE_TYPE_SPEAKER);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetVolumeInDbByStream_001 result1:%{public}f", result);
    EXPECT_GE(TEST_RET_NUM, result);
    result = manager.GetVolumeInDbByStream(StreamUsage::STREAM_USAGE_ULTRASONIC,
        volLevel,
        DeviceType::DEVICE_TYPE_SPEAKER);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetVolumeInDbByStream_001 result2:%{public}f", result);
    EXPECT_GE(TEST_RET_NUM, result);
    result = manager.GetVolumeInDbByStream(ILLEGAL_STREAM_USAGE,
        volLevel,
        DeviceType::DEVICE_TYPE_SPEAKER);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetVolumeInDbByStream_001 result3:%{public}f", result);
    EXPECT_EQ(result, TEST_RET_ERROR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test GetSupportedAudioVolumeTypes API
 * @tc.number: GetSupportedAudioVolumeTypes_001
 * @tc.tesc  : Test GetSupportedAudioVolumeTypes interface
 */
HWTEST(AudioSystemManagerUnitTest, GetSupportedAudioVolumeTypes_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetSupportedAudioVolumeTypes_001 start");
    AudioSystemManager manager;
    std::vector<AudioVolumeType> result = manager.GetSupportedAudioVolumeTypes();
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetSupportedAudioVolumeTypes_001 result size1:%{public}zu",
        result.size());
    EXPECT_GE(result.size(), TEST_RET_NUM);
}

/**
 * @tc.name  : Test GetAudioVolumeTypeByStreamUsage API
 * @tc.number: GetAudioVolumeTypeByStreamUsage_001
 * @tc.tesc  : Test GetAudioVolumeTypeByStreamUsage interface
 */
HWTEST(AudioSystemManagerUnitTest, GetAudioVolumeTypeByStreamUsage_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetAudioVolumeTypeByStreamUsage_001 start");
    AudioSystemManager manager;
    AudioVolumeType result = manager.GetAudioVolumeTypeByStreamUsage(StreamUsage::STREAM_USAGE_MUSIC);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetAudioVolumeTypeByStreamUsage_001 result1:%{public}d", result);
    EXPECT_GE(result, AudioVolumeType::STREAM_DEFAULT);
    EXPECT_LE(result, AudioVolumeType::STREAM_ALL);
}

/**
 * @tc.name  : Test GetStreamUsagesByVolumeType API
 * @tc.number: GetStreamUsagesByVolumeType_001
 * @tc.tesc  : Test GetStreamUsagesByVolumeType interface
 */
HWTEST(AudioSystemManagerUnitTest, GetStreamUsagesByVolumeType_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetStreamUsagesByVolumeType_001 start");
    AudioSystemManager manager;
    std::vector<StreamUsage> result = manager.GetStreamUsagesByVolumeType(AudioVolumeType::STREAM_MUSIC);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetStreamUsagesByVolumeType_001 result size1:%{public}zu",
        result.size());
    EXPECT_GE(result.size(), TEST_RET_NUM);
}

/**
 * @tc.name  : Test RegisterSystemVolumeChnageCallback API
 * @tc.number: RegisterSystemVolumeChnageCallback_001
 * @tc.tesc  : Test RegisterSystemVolumeChnageCallback interface
 */
HWTEST(AudioSystemManagerUnitTest, RegisterSystemVolumeChnageCallback_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest RegisterSystemVolumeChnageCallback_001 start");
    int32_t testClientId = 300300;
    std::shared_ptr<SystemVolumeChangeCallback> callback = std::make_shared<
            SystemVolumeChangeCallbackTest>();
    AudioSystemManager manager;
    int32_t result = manager.RegisterSystemVolumeChangeCallback(testClientId, callback);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest RegisterSystemVolumeChnageCallback_001 result1:%{public}d", result);
    EXPECT_NE(result, SUCCESS);
    result = manager.UnregisterSystemVolumeChangeCallback(testClientId, callback);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest RegisterSystemVolumeChnageCallback_001 result2:%{public}d", result);
    EXPECT_EQ(result, SUCCESS);
    result = manager.RegisterSystemVolumeChangeCallback(testClientId, nullptr);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest RegisterSystemVolumeChnageCallback_001 result3:%{public}d", result);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
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
    EXPECT_EQ(0, result);
}
/**
 * @tc.name   : Test GetVolumeInDbByStream API
 * @tc.number : GetVolumeInDbByStream
 * @tc.desc   : Test GetVolumeInDbByStream interface createAudioWorkgroup
 */
HWTEST(AudioSystemManagerUnitTest, GetVolumeInDbByStream_002, TestSize.Level1)
{
    StreamUsage streamUsage = STREAM_USAGE_MUSIC;
    int32_t volumeLevel = 50;
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;

    AudioSystemManager audioSystemManager;
    float result = audioSystemManager.GetVolumeInDbByStream(streamUsage, volumeLevel, deviceType);
    float errNotSupportedFloat = static_cast<float>(ERR_NOT_SUPPORTED);
    float errPermissionDeniedFloat = static_cast<float>(ERR_PERMISSION_DENIED);

    EXPECT_TRUE(result != errNotSupportedFloat && result != errPermissionDeniedFloat);

    streamUsage = static_cast<StreamUsage>(1000);
    result = audioSystemManager.GetVolumeInDbByStream(streamUsage, volumeLevel, deviceType);
    EXPECT_EQ(result, ERR_NOT_SUPPORTED);

    streamUsage = STREAM_USAGE_SYSTEM;
    result = audioSystemManager.GetVolumeInDbByStream(streamUsage, volumeLevel, deviceType);
    EXPECT_TRUE(result != errNotSupportedFloat && result != errPermissionDeniedFloat);

    streamUsage = STREAM_USAGE_DTMF;
    result = audioSystemManager.GetVolumeInDbByStream(streamUsage, volumeLevel, deviceType);
    EXPECT_TRUE(result != errNotSupportedFloat && result != errPermissionDeniedFloat);
}
/**
 * @tc.name   : Test IsValidToStartGroup API
 * @tc.number : IsValidToStartGroup_001
 * @tc.desc   : Test IsValidToStartGroup interface createAudioWorkgroup
 */
HWTEST(AudioSystemManagerUnitTest, IsValidToStartGroup_001, TestSize.Level1)
{
    int workgroupId = 1;

    AudioSystemManager audioSystemManager;
    bool result = audioSystemManager.IsValidToStartGroup(workgroupId);
    EXPECT_FALSE(result);

    workgroupId = -1111;
    result = audioSystemManager.IsValidToStartGroup(workgroupId);
    EXPECT_FALSE(result);

    workgroupId = 9999;
    result = audioSystemManager.IsValidToStartGroup(workgroupId);
    EXPECT_FALSE(result);
}

/**
 * @tc.name   : Test StopGroup API
 * @tc.number : StopGroupp_001
 * @tc.desc   : Test StopGroup interface createAudioWorkgroup
 */
HWTEST(AudioSystemManagerUnitTest, StopGroup_001, TestSize.Level1)
{
    int workgroupId = 1;

    AudioSystemManager audioSystemManager;
    bool result = audioSystemManager.StopGroup(workgroupId);
    EXPECT_TRUE(result);

    workgroupId = -111;
    result = audioSystemManager.StopGroup(workgroupId);
    EXPECT_TRUE(result);

    workgroupId = 9999;
    result = audioSystemManager.StopGroup(workgroupId);
    EXPECT_TRUE(result);
}

/**
 * @tc.name   : Test GetVolumeByUsage API
 * @tc.number : GetVolumeByUsage001
 * @tc.desc   : Test GetVolumeByUsage interface createAudioWorkgroup
 */
HWTEST(AudioSystemManagerUnitTest, GetVolumeByUsage_002, TestSize.Level1)
{
    StreamUsage streamUsage = STREAM_USAGE_SYSTEM;

    AudioSystemManager audioSystemManager;
    float result = audioSystemManager.GetVolumeByUsage(streamUsage);
    float errNotSupportedFloat = static_cast<float>(ERR_NOT_SUPPORTED);
    float errPermissionDeniedFloat = static_cast<float>(ERR_PERMISSION_DENIED);

    EXPECT_TRUE(result != errNotSupportedFloat && result != errPermissionDeniedFloat);

    streamUsage = static_cast<StreamUsage>(1000);
    result = audioSystemManager.GetVolumeByUsage(streamUsage);
    EXPECT_TRUE(result != -10);

    streamUsage = STREAM_USAGE_MUSIC;
    result = audioSystemManager.GetVolumeByUsage(streamUsage);
    EXPECT_TRUE(result != -10);

    streamUsage = STREAM_USAGE_DTMF;
    EXPECT_TRUE(result != errNotSupportedFloat && result != errPermissionDeniedFloat);
}

/**
 * @tc.name   : Test IsWhispering API
 * @tc.number : IsWhispering_001
 * @tc.desc   : Test IsWhispering interface createAudioWorkgroup
 */
HWTEST(AudioSystemManagerUnitTest, IsWhispering_001, TestSize.Level1)
{
    AudioSystemManager audioSystemManager;
    bool result = audioSystemManager.IsWhispering();
    EXPECT_FALSE(result);
}

/**
 * @tc.name   : Test IsWhispering API
 * @tc.number : IsWhispering_001
 * @tc.desc   : Test IsWhispering interface createAudioWorkgroup
 */
HWTEST(AudioSystemManagerUnitTest, SetVolumeWithDevice_001, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;
    AudioSystemManager audioSystemManager;
    EXPECT_NE(audioSystemManager.SetVolumeWithDevice(STREAM_MUSIC, 5, deviceType), 1);
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
    manager.workgroupPrioRecorderMap_[grpId] = recorder;
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

/**
 * @tc.name   : Test GetMaxVolumeByUsage API
 * @tc.number : GetMaxVolumeByUsage_002
 * @tc.desc   : Test GetMaxVolumeByUsage interface createAudioWorkgroup
 */
HWTEST(AudioSystemManagerUnitTest, GetMaxVolumeByUsage_002, TestSize.Level1)
{
    StreamUsage streamUsage = STREAM_USAGE_ULTRASONIC;
    AudioSystemManager audioSystemManager;
    EXPECT_NE(audioSystemManager.GetMaxVolumeByUsage(streamUsage), SUCCESS);
}

/**
 * @tc.name   : Test GetMaxVolumeByUsage API
 * @tc.number : GetMaxVolumeByUsage_003
 * @tc.desc   : Test GetMaxVolumeByUsage interface createAudioWorkgroup
 */
HWTEST(AudioSystemManagerUnitTest, GetMaxVolumeByUsage_003, TestSize.Level4)
{
    StreamUsage streamUsage = static_cast<StreamUsage>(1000);
    AudioSystemManager audioSystemManager;
    EXPECT_EQ(audioSystemManager.GetMaxVolumeByUsage(streamUsage), ERR_NOT_SUPPORTED);
}

/**
 * @tc.name   : Test GetMaxVolumeByUsage API
 * @tc.number : GetMinVolumeByUsage_002
 * @tc.desc   : Test GetMaxVolumeByUsage interface createAudioWorkgroup
 */
HWTEST(AudioSystemManagerUnitTest, GetMinVolumeByUsage_002, TestSize.Level1)
{
    StreamUsage  streamUsage = STREAM_USAGE_ULTRASONIC;
    AudioSystemManager audioSystemManager;
    EXPECT_NE(audioSystemManager.GetMinVolumeByUsage(streamUsage), ERR_NOT_SUPPORTED);
}

/**
 * @tc.name   : Test GetMaxVolumeByUsage API
 * @tc.number : GetMaxVolumeByUsage_003
 * @tc.desc   : Test GetMaxVolumeByUsage interface createAudioWorkgroup
 */
HWTEST(AudioSystemManagerUnitTest, GetMinVolumeByUsage_003, TestSize.Level4)
{
    StreamUsage  streamUsage = static_cast<StreamUsage>(1000);
    AudioSystemManager audioSystemManager;
    EXPECT_EQ(audioSystemManager.GetMinVolumeByUsage(streamUsage), ERR_NOT_SUPPORTED);
}

/**
 * @tc.name   : Test IsStreamMuteByUsage API
 * @tc.number : IsStreamMuteByUsage_002
 * @tc.desc   : Test IsStreamMuteByUsage interface createAudioWorkgroup
 */
HWTEST(AudioSystemManagerUnitTest, IsStreamMuteByUsage_002, TestSize.Level1)
{
    StreamUsage  streamUsage = STREAM_USAGE_ULTRASONIC;
    AudioSystemManager audioSystemManager;
    bool isMute = 0;
    EXPECT_NE(audioSystemManager.IsStreamMuteByUsage(streamUsage, isMute), ERR_NOT_SUPPORTED);
}

/**
 * @tc.name   : Test IsStreamMuteByUsage API
 * @tc.number : IsStreamMuteByUsage_003
 * @tc.desc   : Test IsStreamMuteByUsage interface createAudioWorkgroup
 */
HWTEST(AudioSystemManagerUnitTest, IsStreamMuteByUsage_003, TestSize.Level4)
{
    StreamUsage  streamUsage = static_cast<StreamUsage>(1000);
    AudioSystemManager audioSystemManager;
    bool isMute = 0;
    EXPECT_EQ(audioSystemManager.IsStreamMuteByUsage(streamUsage, isMute), ERR_NOT_SUPPORTED);
}

/**
 * @tc.name   : Test GetStreamType API
 * @tc.number : GetStreamType_001
 * @tc.desc   : Test GetStreamType interface
 */
HWTEST(AudioSystemManagerUnitTest, GetStreamType_001, TestSize.Level4)
{
    AudioSystemManager audioSystemManager;
    ContentType contentType = CONTENT_TYPE_MUSIC;
    StreamUsage streamUsage = STREAM_USAGE_MUSIC;
    EXPECT_EQ(audioSystemManager.GetStreamType(contentType, streamUsage), STREAM_MUSIC);
}

/**
 * @tc.name   : Test GetStreamType API
 * @tc.number : GetStreamType_002
 * @tc.desc   : Test GetStreamType interface
 */
HWTEST(AudioSystemManagerUnitTest, GetStreamType_002, TestSize.Level4)
{
    AudioSystemManager audioSystemManager;
    ContentType contentType = CONTENT_TYPE_MUSIC;
    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    EXPECT_EQ(audioSystemManager.GetStreamType(contentType, streamUsage), STREAM_MUSIC);
}

/**
 * @tc.name   : Test GetStreamType API
 * @tc.number : GetStreamType_003
 * @tc.desc   : Test GetStreamType interface
 */
HWTEST(AudioSystemManagerUnitTest, GetStreamType_003, TestSize.Level4)
{
    AudioSystemManager audioSystemManager;
    ContentType contentType = CONTENT_TYPE_MUSIC;
    StreamUsage streamUsage = STREAM_USAGE_AUDIOBOOK;
    EXPECT_EQ(audioSystemManager.GetStreamType(contentType, streamUsage), STREAM_MUSIC);
}

/**
 * @tc.name   : Test GetAudioScene API
 * @tc.number : GetAudioScene_001
 * @tc.desc   : Test GetAudioScene interface
 */
HWTEST(AudioSystemManagerUnitTest, GetAudioScene_001, TestSize.Level4)
{
    AudioSystemManager audioSystemManager;
    AudioSystemManager::GetInstance()->SetAudioScene(AUDIO_SCENE_DEFAULT);
    int result = audioSystemManager.GetAudioScene();
    EXPECT_EQ(result, AUDIO_SCENE_DEFAULT);
}

/**
 * @tc.name   : Test GetAudioScene API
 * @tc.number : GetAudioScene_002
 * @tc.desc   : Test GetAudioScene interface
 */
HWTEST(AudioSystemManagerUnitTest, GetAudioScene_002, TestSize.Level4)
{
    AudioSystemManager audioSystemManager;
    AudioSystemManager::GetInstance()->SetAudioScene(AUDIO_SCENE_VOICE_RINGING);
    int result = audioSystemManager.GetAudioScene();
    EXPECT_NE(result, AUDIO_SCENE_INVALID);
}

/**
 * @tc.name   : Test IsDeviceActive API
 * @tc.number : IsDeviceActive_001
 * @tc.desc   : Test IsDeviceActive interface
 */
HWTEST(AudioSystemManagerUnitTest, IsDeviceActive_001, TestSize.Level4)
{
    AudioSystemManager audioSystemManager;
    int result = audioSystemManager.IsDeviceActive(DeviceType::DEVICE_TYPE_INVALID);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name   : Test IsDeviceActive API
 * @tc.number : IsDeviceActive_002
 * @tc.desc   : Test IsDeviceActive interface
 */
HWTEST(AudioSystemManagerUnitTest, IsDeviceActive_002, TestSize.Level4)
{
    AudioSystemManager audioSystemManager;
    int result = audioSystemManager.IsDeviceActive(DeviceType::DEVICE_TYPE_MIC);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name   : Test GetVolume API
 * @tc.number : GetVolume_001
 * @tc.desc   : Test GetVolume interface
 */
HWTEST(AudioSystemManagerUnitTest, GetVolume_001, TestSize.Level4)
{
    AudioSystemManager audioSystemManager;
    AudioVolumeType volumeType = STREAM_MUSIC;
    EXPECT_NE(audioSystemManager.GetVolume(volumeType), ERR_NOT_SUPPORTED);
}

/**
 * @tc.name   : Test GetVolume API
 * @tc.number : GetVolume_002
 * @tc.desc   : Test GetVolume interface
 */
HWTEST(AudioSystemManagerUnitTest, GetVolume_002, TestSize.Level4)
{
    AudioSystemManager audioSystemManager;
    AudioVolumeType volumeType = STREAM_ULTRASONIC;
    EXPECT_NE(audioSystemManager.GetVolume(volumeType), ERR_PERMISSION_DENIED);
}

/**
 * @tc.name   : Test IsDeviceActive API
 * @tc.number : IsDeviceActive_003
 * @tc.desc   : Test IsDeviceActive interface
 */
HWTEST(AudioSystemManagerUnitTest, IsDeviceActive_003, TestSize.Level4)
{
    AudioSystemManager audioSystemManager;
    int result = audioSystemManager.IsDeviceActive(DeviceType::DEVICE_TYPE_NONE);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name   : Test GetVolume API
 * @tc.number : GetVolume_003
 * @tc.desc   : Test GetVolume interface
 */
HWTEST(AudioSystemManagerUnitTest, GetVolume_003, TestSize.Level4)
{
    AudioSystemManager audioSystemManager;
    AudioVolumeType volumeType = STREAM_ALL;
    EXPECT_NE(audioSystemManager.GetVolume(volumeType), ERR_PERMISSION_DENIED);
}

/**
 * @tc.name  : Test GetPinValueForPeripherals API
 * @tc.type  : FUNC
 * @tc.number: GetPinValueForPeripherals_001
 * @tc.desc  : Test GetPinValueForPeripherals interface.
 */
HWTEST(AudioSystemManagerUnitTest, GetPinValueForPeripherals_001, TestSize.Level4)
{
    AudioPin pinValue = AudioSystemManager::GetInstance()->GetPinValueForPeripherals(DEVICE_TYPE_FILE_SINK,
        OUTPUT_DEVICE, DM_DEVICE_TYPE_UWB);
    EXPECT_EQ(pinValue, AUDIO_PIN_NONE);
}

/**
 * @tc.name  : Test GetPinValueForPeripherals API
 * @tc.type  : FUNC
 * @tc.number: GetPinValueForPeripherals_002
 * @tc.desc  : Test GetPinValueForPeripherals interface.
 */
HWTEST(AudioSystemManagerUnitTest, GetPinValueForPeripherals_002, TestSize.Level4)
{
    AudioPin pinValue = AudioSystemManager::GetInstance()->GetPinValueForPeripherals(DEVICE_TYPE_ACCESSORY,
        OUTPUT_DEVICE, DM_DEVICE_TYPE_UWB);
    EXPECT_EQ(pinValue, AUDIO_PIN_NONE);
}

/**
 * @tc.name  : Test GetTypeValueFromPin API
 * @tc.type  : FUNC
 * @tc.number: GetTypeValueFromPin_001
 * @tc.desc  : Test GetTypeValueFromPin interface.
 */
HWTEST(AudioSystemManagerUnitTest, GetTypeValueFromPin_001, TestSize.Level4)
{
    DeviceType deviceValue = AudioSystemManager::GetInstance()->GetTypeValueFromPin(AUDIO_PIN_OUT_HEADSET);
    EXPECT_EQ(deviceValue, DEVICE_TYPE_NONE);
}

/**
 * @tc.name  : Test GetTypeValueFromPin API
 * @tc.type  : FUNC
 * @tc.number: GetTypeValueFromPin_002
 * @tc.desc  : Test GetTypeValueFromPin interface.
 */
HWTEST(AudioSystemManagerUnitTest, GetTypeValueFromPin_002, TestSize.Level4)
{
    DeviceType deviceValue = AudioSystemManager::GetInstance()->GetTypeValueFromPin(static_cast<AudioPin>(1000));
    EXPECT_EQ(deviceValue, DEVICE_TYPE_NONE);
}

/**
 * @tc.name   : Test IsValidToStartGroup API
 * @tc.number : IsValidToStartGroup_002
 * @tc.desc   : Test IsValidToStartGroup interface createAudioWorkgroup
 */
HWTEST(AudioSystemManagerUnitTest, IsValidToStartGroup_002, TestSize.Level4)
{
    int workgroupId = 1;

    AudioSystemManager audioSystemManager;
    audioSystemManager.hasSystemPermission_ = false;
    bool result = audioSystemManager.IsValidToStartGroup(workgroupId);
    EXPECT_FALSE(result);
}

/**
 * @tc.name   : Test OnWorkgroupChange API
 * @tc.number : OnWorkgroupChange_004
 * @tc.desc   : Test OnWorkgroupChange interface
 */
HWTEST(AudioSystemManagerUnitTest, OnWorkgroupChange_004, TestSize.Level4)
{
    AudioWorkgroupCallbackImpl audioWorkgroupCallbackImpl;
    AudioWorkgroupChangeInfoIpc info;
    audioWorkgroupCallbackImpl.workgroupCb_ = nullptr;
    EXPECT_EQ(audioWorkgroupCallbackImpl.OnWorkgroupChange(info), ERROR);
}

/**
 * @tc.name   : Test RemoveWorkgroupChangeCallback API
 * @tc.number : RemoveWorkgroupChangeCallback_001
 * @tc.desc   : Test RemoveWorkgroupChangeCallback interface
 */
HWTEST(AudioSystemManagerUnitTest, RemoveWorkgroupChangeCallback_001, TestSize.Level4)
{
    AudioWorkgroupCallbackImpl audioWorkgroupCallbackImpl;
    audioWorkgroupCallbackImpl.RemoveWorkgroupChangeCallback();
    EXPECT_EQ(nullptr, audioWorkgroupCallbackImpl.workgroupCb_);
}

} // namespace AudioStandard
} // namespace OHOS
