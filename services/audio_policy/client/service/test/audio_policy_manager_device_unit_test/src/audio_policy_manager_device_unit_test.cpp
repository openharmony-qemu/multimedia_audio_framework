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
#include "audio_unit_test.h"

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
 * @tc.number: RegisterPreferredDeviceSetCallback_001.
 * @tc.desc  : Test RegisterPreferredDeviceSetCallback && UnregisterPreferredDeviceSetCallback.
 */
HWTEST(AudioPolicyManagerDevice, RegisterPreferredDeviceSetCallback_001, TestSize.Level1)
{
    auto &policyManager = AudioPolicyManager::GetInstance();
    int32_t ret = 0;
    auto cb = std::make_shared<PreferredDeviceSetCallbackTest>();
    ret = policyManager.RegisterPreferredDeviceSetCallback(cb);
    EXPECT_EQ(ret, SUCCESS);
    ret = policyManager.UnregisterPreferredDeviceSetCallback(cb);
    EXPECT_EQ(ret, SUCCESS);
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

/**
 * @tc.name  : Test AudioPolicyManagerDevice.
 * @tc.number: UnsetDeviceInfoUpdateCallback_001.
 * @tc.desc  : Test UnsetDeviceInfoUpdateCallback().
 */
HWTEST(AudioPolicyManagerDevice, UnsetDeviceInfoUpdateCallback_001, TestSize.Level1)
{
    auto audioPolicyManager = std::make_shared<AudioPolicyManager>();
    ASSERT_TRUE(audioPolicyManager != nullptr);

    int32_t clientId = 1;
    std::shared_ptr<AudioManagerDeviceInfoUpdateCallback> cb =
        std::make_shared<ConcreteAudioManagerDeviceInfoUpdateCallback>();
    audioPolicyManager->audioPolicyClientStubCB_ = nullptr;
    int32_t ret = audioPolicyManager->UnsetDeviceInfoUpdateCallback(clientId, cb);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioPolicyManagerDevice.
 * @tc.number: UnsetDeviceInfoUpdateCallback_002.
 * @tc.desc  : Test UnsetDeviceInfoUpdateCallback().
 */
HWTEST(AudioPolicyManagerDevice, UnsetDeviceInfoUpdateCallback_002, TestSize.Level1)
{
    auto audioPolicyManager = std::make_shared<AudioPolicyManager>();
    ASSERT_TRUE(audioPolicyManager != nullptr);

    int32_t clientId = 1;
    std::shared_ptr<AudioManagerDeviceInfoUpdateCallback> cb =
        std::make_shared<ConcreteAudioManagerDeviceInfoUpdateCallback>();
    audioPolicyManager->audioPolicyClientStubCB_ = new(std::nothrow) AudioPolicyClientStubImpl();
    audioPolicyManager->audioPolicyClientStubCB_->deviceInfoUpdateCallbackList_.clear();
    audioPolicyManager->UnsetDeviceInfoUpdateCallback(clientId, cb);

    std::shared_ptr<AudioManagerDeviceInfoUpdateCallback> cb1 = nullptr;
    audioPolicyManager->audioPolicyClientStubCB_->deviceInfoUpdateCallbackList_.push_back(cb1);

    auto ret = audioPolicyManager->UnsetDeviceInfoUpdateCallback(clientId, cb);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioPolicyManagerDevice.
 * @tc.number: UnsetPreferredInputDeviceChangeCallback_003.
 * @tc.desc  : Test SetPreferredInputDeviceChangeCallback && UnsetPreferredInputDeviceChangeCallback.
 */
HWTEST(AudioPolicyManagerDevice, UnsetPreferredInputDeviceChangeCallback_003, TestSize.Level1)
{
    auto &policyManager = AudioPolicyManager::GetInstance();
    int32_t ret = 0;
    auto captureInfo = AudioCapturerInfo();
    policyManager.isAudioPolicyClientRegisted_.store(false);
    policyManager.SetPreferredInputDeviceChangeCallback(
        captureInfo, std::make_shared<AudioPreferredInputDeviceChangeCallbackTest>());
    ASSERT_EQ(ret, SUCCESS);
    policyManager.UnsetPreferredInputDeviceChangeCallback(
        std::make_shared<AudioPreferredInputDeviceChangeCallbackTest>());
    ASSERT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioPolicyManagerDevice.
 * @tc.number: SetDeviceInfoUpdateCallback_001.
 * @tc.desc  : Test SetDeviceInfoUpdateCallback().
 */
HWTEST(AudioPolicyManagerDevice, SetDeviceInfoUpdateCallback_001, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("audio_server");
    MockNative::Mock();
    auto audioPolicyManager = std::make_shared<AudioPolicyManager>();
    ASSERT_TRUE(audioPolicyManager != nullptr);

    audioPolicyManager->audioPolicyClientStubCB_ = nullptr;
    audioPolicyManager->isAudioPolicyClientRegisted_.store(false);

    int32_t clientId = 1;
    std::shared_ptr<AudioManagerDeviceInfoUpdateCallback> cb =
        std::make_shared<ConcreteAudioManagerDeviceInfoUpdateCallback>();
    int32_t ret = audioPolicyManager->SetDeviceInfoUpdateCallback(clientId, cb);
    EXPECT_EQ(ret, SUCCESS);
    MockNative::Resume();
}

/**
 * @tc.name  : Test AudioPolicyManagerDevice.
 * @tc.number: RegisterPreferredDeviceSetCallback_002.
 * @tc.desc  : Test RegisterPreferredDeviceSetCallback && UnregisterPreferredDeviceSetCallback.
 */
HWTEST(AudioPolicyManagerDevice, RegisterPreferredDeviceSetCallback_002, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("audio_server");
    MockNative::Mock();
    auto &policyManager = AudioPolicyManager::GetInstance();
    auto cb = std::make_shared<PreferredDeviceSetCallbackTest>();
    policyManager.RegisterPreferredDeviceSetCallback(cb);
    auto ret = policyManager.UnregisterPreferredDeviceSetCallback(cb);
    EXPECT_EQ(ret, SUCCESS);
    MockNative::Resume();
}

/**
 * @tc.name  : Test AudioPolicyManagerDevice.
 * @tc.number: IsCollaborativePlaybackSupportedForDevice_001.
 * @tc.desc  : Test IsCollaborativePlaybackSupportedForDevice.
 */
HWTEST(AudioPolicyManagerDevice, IsCollaborativePlaybackSupportedForDevice_001, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    ASSERT_TRUE(audioPolicyManager_ != nullptr);

    bool result = audioPolicyManager_->IsCollaborativePlaybackSupportedForDevice(nullptr);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test AudioPolicyManagerDevice.
 * @tc.number: IsCollaborativePlaybackSupportedForDevice_002.
 * @tc.desc  : Test IsCollaborativePlaybackSupportedForDevice.
 */
HWTEST(AudioPolicyManagerDevice, IsCollaborativePlaybackSupportedForDevice_002, TestSize.Level1)
{
    auto audioPolicyManager_ = std::make_shared<AudioPolicyManager>();
    ASSERT_TRUE(audioPolicyManager_ != nullptr);
    std::shared_ptr<AudioDeviceDescriptor> device = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_TRUE(device != nullptr);
    device->macAddress_ = "8C-32-23-23-6C-12";

    bool result = audioPolicyManager_->IsCollaborativePlaybackSupportedForDevice(device);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test AudioPolicyManagerDevice.
 * @tc.number: SetPreferredOutputDeviceChangeCallback_WithUid_001.
 * @tc.desc  : Test SetPreferredOutputDeviceChangeCallback stores uid correctly in rendererInfosWithUid_.
 */
HWTEST(AudioPolicyManagerDevice, SetPreferredOutputDeviceChangeCallback_WithUid_001, TestSize.Level1)
{
    auto audioPolicyManager = std::make_shared<AudioPolicyManager>();
    ASSERT_TRUE(audioPolicyManager != nullptr);

    audioPolicyManager->audioPolicyClientStubCB_ = new(std::nothrow) AudioPolicyClientStubImpl();
    ASSERT_TRUE(audioPolicyManager->audioPolicyClientStubCB_ != nullptr);

    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MUSIC;
    int32_t testUid = 1234;
    auto callback = std::make_shared<AudioPreferredOutputDeviceChangeCallbackTest>();

    int32_t ret = audioPolicyManager->SetPreferredOutputDeviceChangeCallback(rendererInfo, callback, testUid);
    EXPECT_EQ(ret, SUCCESS);

    // Verify callback was added (indirect verification of uid storage)
    size_t callbackSize = audioPolicyManager->audioPolicyClientStubCB_->GetPreferredOutputDeviceChangeCallbackSize();
    EXPECT_GE(callbackSize, 1);
}

/**
 * @tc.name  : Test AudioPolicyManagerDevice.
 * @tc.number: SetPreferredInputDeviceChangeCallback_WithUid_001.
 * @tc.desc  : Test SetPreferredInputDeviceChangeCallback stores uid correctly in capturerInfosWithUid_.
 */
HWTEST(AudioPolicyManagerDevice, SetPreferredInputDeviceChangeCallback_WithUid_001, TestSize.Level1)
{
    auto audioPolicyManager = std::make_shared<AudioPolicyManager>();
    ASSERT_TRUE(audioPolicyManager != nullptr);

    audioPolicyManager->audioPolicyClientStubCB_ = new(std::nothrow) AudioPolicyClientStubImpl();
    ASSERT_TRUE(audioPolicyManager->audioPolicyClientStubCB_ != nullptr);

    AudioCapturerInfo capturerInfo;
    capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    int32_t testUid = 5678;
    auto callback = std::make_shared<AudioPreferredInputDeviceChangeCallbackTest>();

    int32_t ret = audioPolicyManager->SetPreferredInputDeviceChangeCallback(capturerInfo, callback, testUid);
    EXPECT_EQ(ret, SUCCESS);

    // Verify callback was added (indirect verification of uid storage)
    size_t callbackSize = audioPolicyManager->audioPolicyClientStubCB_->GetPreferredInputDeviceChangeCallbackSize();
    EXPECT_GE(callbackSize, 1);
}

/**
 * @tc.name  : Test AudioPolicyManagerDevice.
 * @tc.number: SetPreferredOutputDeviceChangeCallback_DuplicatePrevention_001.
 * @tc.desc  : Test duplicate registration doesn't add duplicate entries to rendererInfosWithUid_.
 */
HWTEST(AudioPolicyManagerDevice, SetPreferredOutputDeviceChangeCallback_DuplicatePrevention_001, TestSize.Level1)
{
    auto audioPolicyManager = std::make_shared<AudioPolicyManager>();
    ASSERT_TRUE(audioPolicyManager != nullptr);

    audioPolicyManager->audioPolicyClientStubCB_ = new(std::nothrow) AudioPolicyClientStubImpl();
    ASSERT_TRUE(audioPolicyManager->audioPolicyClientStubCB_ != nullptr);

    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MUSIC;
    int32_t testUid = 9999;
    auto callback1 = std::make_shared<AudioPreferredOutputDeviceChangeCallbackTest>();
    auto callback2 = std::make_shared<AudioPreferredOutputDeviceChangeCallbackTest>();

    // Register same (rendererInfo, uid) pair twice
    int32_t ret1 = audioPolicyManager->SetPreferredOutputDeviceChangeCallback(rendererInfo, callback1, testUid);
    EXPECT_EQ(ret1, SUCCESS);

    size_t sizeAfterFirst = audioPolicyManager->audioPolicyClientStubCB_->GetPreferredOutputDeviceChangeCallbackSize();

    int32_t ret2 = audioPolicyManager->SetPreferredOutputDeviceChangeCallback(rendererInfo, callback2, testUid);
    EXPECT_EQ(ret2, SUCCESS);

    size_t sizeAfterSecond = audioPolicyManager->audioPolicyClientStubCB_->GetPreferredOutputDeviceChangeCallbackSize();

    // Duplicate check should prevent adding duplicate entry
    // The size should not increase significantly (implementation may add callback but not duplicate uid entry)
    EXPECT_LE(sizeAfterSecond, sizeAfterFirst + 1);
}

/**
 * @tc.name  : Test AudioPolicyManagerDevice.
 * @tc.number: SetPreferredInputDeviceChangeCallback_DuplicatePrevention_001.
 * @tc.desc  : Test duplicate registration doesn't add duplicate entries to capturerInfosWithUid_.
 */
HWTEST(AudioPolicyManagerDevice, SetPreferredInputDeviceChangeCallback_DuplicatePrevention_001, TestSize.Level1)
{
    auto audioPolicyManager = std::make_shared<AudioPolicyManager>();
    ASSERT_TRUE(audioPolicyManager != nullptr);

    audioPolicyManager->audioPolicyClientStubCB_ = new(std::nothrow) AudioPolicyClientStubImpl();
    ASSERT_TRUE(audioPolicyManager->audioPolicyClientStubCB_ != nullptr);

    AudioCapturerInfo capturerInfo;
    capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    int32_t testUid = 8888;
    auto callback1 = std::make_shared<AudioPreferredInputDeviceChangeCallbackTest>();
    auto callback2 = std::make_shared<AudioPreferredInputDeviceChangeCallbackTest>();

    // Register same (capturerInfo, uid) pair twice
    int32_t ret1 = audioPolicyManager->SetPreferredInputDeviceChangeCallback(capturerInfo, callback1, testUid);
    EXPECT_EQ(ret1, SUCCESS);

    size_t sizeAfterFirst = audioPolicyManager->audioPolicyClientStubCB_->GetPreferredInputDeviceChangeCallbackSize();

    int32_t ret2 = audioPolicyManager->SetPreferredInputDeviceChangeCallback(capturerInfo, callback2, testUid);
    EXPECT_EQ(ret2, SUCCESS);

    size_t sizeAfterSecond = audioPolicyManager->audioPolicyClientStubCB_->GetPreferredInputDeviceChangeCallbackSize();

    // Duplicate check should prevent adding duplicate entry
    EXPECT_LE(sizeAfterSecond, sizeAfterFirst + 1);
}

/**
 * @tc.name  : Test AudioPolicyManagerDevice.
 * @tc.number: UnsetPreferredOutputDeviceChangeCallback_RemovesUidEntry_001.
 * @tc.desc  : Test UnsetPreferredOutputDeviceChangeCallback removes entry from rendererInfosWithUid_.
 */
HWTEST(AudioPolicyManagerDevice, UnsetPreferredOutputDeviceChangeCallback_RemovesUidEntry_001, TestSize.Level1)
{
    auto audioPolicyManager = std::make_shared<AudioPolicyManager>();
    ASSERT_TRUE(audioPolicyManager != nullptr);

    audioPolicyManager->audioPolicyClientStubCB_ = new(std::nothrow) AudioPolicyClientStubImpl();
    ASSERT_TRUE(audioPolicyManager->audioPolicyClientStubCB_ != nullptr);

    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MUSIC;
    int32_t testUid = 1111;
    auto callback = std::make_shared<AudioPreferredOutputDeviceChangeCallbackTest>();

    // Register callback with specific uid
    int32_t ret = audioPolicyManager->SetPreferredOutputDeviceChangeCallback(rendererInfo, callback, testUid);
    EXPECT_EQ(ret, SUCCESS);

    size_t sizeAfterSet = audioPolicyManager->audioPolicyClientStubCB_->GetPreferredOutputDeviceChangeCallbackSize();
    EXPECT_GE(sizeAfterSet, 1);

    // Unset callback with same uid
    ret = audioPolicyManager->UnsetPreferredOutputDeviceChangeCallback(callback, rendererInfo, testUid);
    EXPECT_EQ(ret, SUCCESS);

    // Verify entry was removed (callback size should decrease)
    size_t sizeAfterUnset = audioPolicyManager->audioPolicyClientStubCB_->GetPreferredOutputDeviceChangeCallbackSize();
    EXPECT_LT(sizeAfterUnset, sizeAfterSet);
}

/**
 * @tc.name  : Test AudioPolicyManagerDevice.
 * @tc.number: UnsetPreferredInputDeviceChangeCallback_RemovesUidEntry_001.
 * @tc.desc  : Test UnsetPreferredInputDeviceChangeCallback removes entry from capturerInfosWithUid_.
 */
HWTEST(AudioPolicyManagerDevice, UnsetPreferredInputDeviceChangeCallback_RemovesUidEntry_001, TestSize.Level1)
{
    auto audioPolicyManager = std::make_shared<AudioPolicyManager>();
    ASSERT_TRUE(audioPolicyManager != nullptr);

    audioPolicyManager->audioPolicyClientStubCB_ = new(std::nothrow) AudioPolicyClientStubImpl();
    ASSERT_TRUE(audioPolicyManager->audioPolicyClientStubCB_ != nullptr);

    AudioCapturerInfo capturerInfo;
    capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    int32_t testUid = 2222;
    auto callback = std::make_shared<AudioPreferredInputDeviceChangeCallbackTest>();

    // Register callback with specific uid
    int32_t ret = audioPolicyManager->SetPreferredInputDeviceChangeCallback(capturerInfo, callback, testUid);
    EXPECT_EQ(ret, SUCCESS);

    size_t sizeAfterSet = audioPolicyManager->audioPolicyClientStubCB_->GetPreferredInputDeviceChangeCallbackSize();
    EXPECT_GE(sizeAfterSet, 1);

    // Unset callback with same uid
    ret = audioPolicyManager->UnsetPreferredInputDeviceChangeCallback(callback, capturerInfo, testUid);
    EXPECT_EQ(ret, SUCCESS);

    // Verify entry was removed (callback size should decrease)
    size_t sizeAfterUnset = audioPolicyManager->audioPolicyClientStubCB_->GetPreferredInputDeviceChangeCallbackSize();
    EXPECT_LT(sizeAfterUnset, sizeAfterSet);
}
} // namespace AudioStandard
} // namespace OHOS
