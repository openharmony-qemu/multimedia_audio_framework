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

#include "audio_errors.h"
#include "audio_policy_manager_device_unit_test.h"
#include "audio_utils.h"
#include "audio_device_status.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioPolicyManagerDeviceUnitTest::SetUpTestCase(void) {}
void AudioPolicyManagerDeviceUnitTest::TearDownTestCase(void) {}
void AudioPolicyManagerDeviceUnitTest::SetUp(void) {}
void AudioPolicyManagerDeviceUnitTest::TearDown(void) {}

/**
* @tc.name  : Test AudioPolicyManagerDevice.
* @tc.number: SelectOutputDevice_001.
* @tc.desc  : Test SelectOutputDevice.
*/
HWTEST(AudioPolicyManagerDevice, SelectOutputDevice_001, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descriptors;
    for (int i = 0; i < 21; ++i) {
        descriptors.push_back(std::make_shared<AudioDeviceDescriptor>());
    }
    int32_t ret = audioPolicyManager_->SelectOutputDevice(audioRendererFilter, descriptors);
    EXPECT_EQ(ret, -1);
    descriptors.clear();
}

/**
* @tc.name  : Test AudioPolicyManagerDevice.
* @tc.number: SelectOutputDevice_002.
* @tc.desc  : Test SelectOutputDevice.
*/
HWTEST(AudioPolicyManagerDevice, SelectOutputDevice_002, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descriptors;
    int32_t ret = audioPolicyManager_->SelectOutputDevice(audioRendererFilter, descriptors);
    EXPECT_EQ(ret, -1);
}

/**
* @tc.name  : Test AudioPolicyManagerDevice.
* @tc.number: GetSelectedDeviceInfo_001.
* @tc.desc  : Test GetSelectedDeviceInfo.
*/
HWTEST(AudioPolicyManagerDevice, GetSelectedDeviceInfo_001, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    EXPECT_NE(audioPolicyManager_, nullptr);
    auto ret = audioPolicyManager_->GetSelectedDeviceInfo(0, 0, AudioStreamType::STREAM_VOICE_CALL);
    EXPECT_NE(ret, "test");
}

/**
* @tc.name  : Test AudioPolicyManagerDevice.
* @tc.number: SelectInputDevice_001.
* @tc.desc  : Test SelectInputDevice.
*/
HWTEST(AudioPolicyManagerDevice, SelectInputDevice_001, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    EXPECT_NE(audioPolicyManager_, nullptr);
    sptr<AudioCapturerFilter> audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descriptors;
    for (int i = 0; i < 5; ++i) {
        descriptors.push_back(std::make_shared<AudioDeviceDescriptor>());
    }
    int32_t ret = audioPolicyManager_->SelectInputDevice(audioCapturerFilter, descriptors);
    EXPECT_NE(ret, SUCCESS);
    descriptors.clear();
}

/**
* @tc.name  : Test AudioPolicyManagerDevice.
* @tc.number: GetExcludedDevices_001.
* @tc.desc  : Test GetExcludedDevices.
*/
HWTEST(AudioPolicyManagerDevice, GetExcludedDevices_001, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    EXPECT_NE(audioPolicyManager_, nullptr);
    auto ret = audioPolicyManager_->GetExcludedDevices(AudioDeviceUsage::MEDIA_OUTPUT_DEVICES);
    EXPECT_NE(ret.size(), 5);
}

/**
* @tc.name  : Test AudioPolicyManagerDevice.
* @tc.number: GetDevicesInner_001.
* @tc.desc  : Test GetDevicesInner.
*/
HWTEST(AudioPolicyManagerDevice, GetDevicesInner_001, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    EXPECT_NE(audioPolicyManager_, nullptr);
    auto ret = audioPolicyManager_->GetDevicesInner(DeviceFlag::OUTPUT_DEVICES_FLAG);
    EXPECT_NE(ret.size(), 5);
}

/**
* @tc.name  : Test AudioPolicyManagerDevice.
* @tc.number: TriggerFetchDevice_001.
* @tc.desc  : Test TriggerFetchDevice.
*/
HWTEST(AudioPolicyManagerDevice, TriggerFetchDevice_001, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    EXPECT_NE(audioPolicyManager_, nullptr);
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    int32_t ret = audioPolicyManager_->TriggerFetchDevice(reason);
    EXPECT_NE(ret,  SUCCESS);
}

/**
* @tc.name  : Test AudioPolicyManagerDevice.
* @tc.number: GetDmDeviceType_001.
* @tc.desc  : Test GetDmDeviceType.
*/
HWTEST(AudioPolicyManagerDevice, GetDmDeviceType_001, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    auto& deviceStatus = AudioDeviceStatus::GetInstance();
    deviceStatus.dmDeviceType_ = DEVICE_TYPE_INVALID;
    int32_t result = audioPolicyManager_->GetDmDeviceType();
    EXPECT_EQ(result,  DEVICE_TYPE_INVALID);
}

/**
 * @tc.name  : Test AudioPolicyManagerDevice.
 * @tc.number: UnsetDeviceChangeCallback_001.
 * @tc.desc  : Test SetDeviceChangeCallback && UnsetDeviceChangeCallback.
 */
HWTEST(AudioPolicyManagerDevice, UnsetDeviceChangeCallback_001, TestSize.Level1)
{
    auto &policyManager = AudioPolicyManager::GetInstance();
    int32_t ret = 0;
    auto cb = std::make_shared<AudioManagerDeviceChangeCallbackTest>();
    ASSERT_NE(cb, nullptr);
    auto basecb = static_cast<std::shared_ptr<AudioManagerDeviceChangeCallback>>(cb);
    policyManager.SetDeviceChangeCallback(
        42, INPUT_DEVICES_FLAG, std::make_shared<AudioManagerDeviceChangeCallbackTest>());
    policyManager.SetDeviceChangeCallback(
        42, OUTPUT_DEVICES_FLAG, std::make_shared<AudioManagerDeviceChangeCallbackTest>());
    policyManager.UnsetDeviceChangeCallback(42, INPUT_DEVICES_FLAG, basecb);
    ASSERT_EQ(ret, SUCCESS);
    policyManager.UnsetDeviceChangeCallback(42, OUTPUT_DEVICES_FLAG, basecb);
    ASSERT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioPolicyManagerDevice.
 * @tc.number: UnsetPreferredOutputDeviceChangeCallback_001.
 * @tc.desc  : Test SetPreferredOutputDeviceChangeCallback && UnsetPreferredOutputDeviceChangeCallback.
 */
HWTEST(AudioPolicyManagerDevice, UnsetPreferredOutputDeviceChangeCallback_001, TestSize.Level1)
{
    auto &policyManager = AudioPolicyManager::GetInstance();
    int32_t ret = 0;
    auto rendererInfo = AudioRendererInfo();
    policyManager.SetPreferredOutputDeviceChangeCallback(
        rendererInfo, std::make_shared<AudioPreferredOutputDeviceChangeCallbackTest>());
    ASSERT_EQ(ret, SUCCESS);
    auto cb = std::make_shared<AudioPreferredOutputDeviceChangeCallbackTest>();
    policyManager.UnsetPreferredOutputDeviceChangeCallback(cb);
    ASSERT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioPolicyManagerDevice.
 * @tc.number: UnsetPreferredInputDeviceChangeCallback_002.
 * @tc.desc  : Test SetPreferredInputDeviceChangeCallback && UnsetPreferredInputDeviceChangeCallback.
 */
HWTEST(AudioPolicyManagerDevice, UnsetPreferredInputDeviceChangeCallback_002, TestSize.Level1)
{
    auto &policyManager = AudioPolicyManager::GetInstance();
    int32_t ret = 0;
    auto captureInfo = AudioCapturerInfo();
    policyManager.SetPreferredInputDeviceChangeCallback(
        captureInfo, std::make_shared<AudioPreferredInputDeviceChangeCallbackTest>());
    ASSERT_EQ(ret, SUCCESS);
    policyManager.UnsetPreferredInputDeviceChangeCallback(
        std::make_shared<AudioPreferredInputDeviceChangeCallbackTest>());
    ASSERT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioPolicyManagerDevice.
 * @tc.number: UnregisterDeviceChangeWithInfoCallback_001.
 * @tc.desc  : Test RegisterDeviceChangeWithInfoCallback && UnregisterDeviceChangeWithInfoCallback.
 */
HWTEST(AudioPolicyManagerDevice, UnregisterDeviceChangeWithInfoCallback_001, TestSize.Level1)
{
    auto &policyManager = AudioPolicyManager::GetInstance();
    int32_t ret = 0;
    auto cb = std::make_shared<ConcreteDeviceChangeWithInfoCallback>();
    std::weak_ptr<DeviceChangeWithInfoCallback> deviceChangeWithInfoCallback(cb);
    policyManager.RegisterDeviceChangeWithInfoCallback(42, deviceChangeWithInfoCallback);
    ASSERT_EQ(ret, SUCCESS);
    policyManager.UnregisterDeviceChangeWithInfoCallback(42);
    ASSERT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioPolicyManagerDevice.
 * @tc.number: GetAvailableDevices_001.
 * @tc.desc  : Test GetAvailableDevices().
 */
HWTEST(AudioPolicyManagerDevice, GetAvailableDevices_001, TestSize.Level1)
{
    auto &policyManager = AudioPolicyManager::GetInstance();
    ASSERT_EQ((policyManager.GetAvailableDevices(MEDIA_INPUT_DEVICES)).size(), 1);
}
} // namespace AudioStandard
} // namespace OHOS
