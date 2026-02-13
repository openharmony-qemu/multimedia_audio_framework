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
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
class AudioSystemManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class DataTransferStateChangeCallbackTest : public AudioRendererDataTransferStateChangeCallback {
public:
    void OnDataTransferStateChange(const AudioRendererDataTransferStateChangeInfo &info) override {}
    void OnMuteStateChange(const int32_t &uid, const uint32_t &sessionId, const bool &isMuted) override {}
};

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
* @tc.name   : Test ConfigDistributedRoutingRole API
* @tc.number : ConfigDistributedRoutingRoleTest_002
* @tc.desc   : Test ConfigDistributedRoutingRole interface.
*/
HWTEST(AudioSystemManagerUnitTest, ConfigDistributedRoutingRoleTest_002, TestSize.Level4)
{
    CastType castType = CAST_TYPE_ALL;
    std::shared_ptr<AudioDeviceDescriptor> audioDevDesc = std::make_shared<AudioDeviceDescriptor>();;
    audioDevDesc->networkId_ = LOCAL_NETWORK_ID;
    int32_t result = AudioSystemManager::GetInstance()->ConfigDistributedRoutingRole(audioDevDesc, castType);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test GetTypeValueFromPin API
 * @tc.type  : FUNC
 * @tc.number: GetTypeValueFromPin_003
 * @tc.desc  : Test GetTypeValueFromPin interface.
 */
HWTEST(AudioSystemManagerUnitTest, GetTypeValueFromPin_003, TestSize.Level4)
{
    DeviceType deviceValue = AudioSystemManager::GetInstance()->GetTypeValueFromPin(AUDIO_PIN_IN_UWB);
    EXPECT_EQ(deviceValue, DEVICE_TYPE_ACCESSORY);
}

/**
* @tc.name   : Test ConfigDistributedRoutingRole API
* @tc.number : ConfigDistributedRoutingRoleTest_003
* @tc.desc   : Test ConfigDistributedRoutingRole interface.
*/
HWTEST(AudioSystemManagerUnitTest, ConfigDistributedRoutingRoleTest_003, TestSize.Level4)
{
    CastType castType = CAST_TYPE_ALL;
    std::shared_ptr<AudioDeviceDescriptor> audioDevDesc = std::make_shared<AudioDeviceDescriptor>();
    constexpr size_t VALID_REMOTE_NETWORK_ID_LENGTH = 64;
    constexpr char invalidLocalNetworkId[] = "123456";
    audioDevDesc->networkId_ = invalidLocalNetworkId;
    int32_t result = AudioSystemManager::GetInstance()->ConfigDistributedRoutingRole(audioDevDesc, castType);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    audioDevDesc->networkId_ = LOCAL_NETWORK_ID;
    audioDevDesc->networkId_.resize(VALID_REMOTE_NETWORK_ID_LENGTH);
    result = AudioSystemManager::GetInstance()->ConfigDistributedRoutingRole(audioDevDesc, castType);
    EXPECT_EQ(result, ERR_INVALID_PARAM);

    constexpr size_t INVALID_REMOTE_NETWORK_ID_LENGTH = 100;
    audioDevDesc->networkId_.resize(INVALID_REMOTE_NETWORK_ID_LENGTH);
    result = AudioSystemManager::GetInstance()->ConfigDistributedRoutingRole(audioDevDesc, castType);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test UpdateSafeVolumeByStrMode API
 * @tc.type  : FUNC
 * @tc.number: UpdateSafeVolumeByStrMode_001
 * @tc.desc  : Test UpdateSafeVolumeByStrMode interface.
 */
HWTEST(AudioSystemManagerUnitTest, UpdateSafeVolumeByStrMode_001, TestSize.Level4)
{
    int32_t result = AudioSystemManager::GetInstance()->UpdateSafeVolumeByStrMode();
    EXPECT_EQ(result, SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS
