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

#include "audio_state_manager_unit_test.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_policy_log.h"

#include <thread>
#include <memory>
#include <vector>

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

const int32_t ANCO_SERVICE_BROKER_UID = 5557;

void AudioStateManagerUnitTest::SetUpTestCase(void) {}
void AudioStateManagerUnitTest::TearDownTestCase(void) {}
void AudioStateManagerUnitTest::SetUp(void) {}
void AudioStateManagerUnitTest::TearDown(void) {}

class IStandardAudioPolicyManagerListenerStub : public IStandardAudioPolicyManagerListener {
public:
    sptr<IRemoteObject> AsObject() override { return nullptr; }

    ~IStandardAudioPolicyManagerListenerStub() {}

    ErrCode OnInterrupt(const InterruptEventInternal& interruptEvent) override { return SUCCESS; }

    ErrCode OnRouteUpdate(uint32_t routeFlag, const std::string& networkId) override { return SUCCESS; }

    ErrCode OnAvailableDeviceChange(uint32_t usage, const DeviceChangeAction& deviceChangeAction) override
    {
        return SUCCESS;
    }

    ErrCode OnQueryClientType(const std::string& bundleName, uint32_t uid, bool& ret) override
    {
        return SUCCESS;
    }

    ErrCode OnCheckClientInfo(const std::string& bundleName, int32_t& uid, int32_t pid, bool& ret) override
    {
        return SUCCESS;
    }

    ErrCode OnCheckMediaControllerBundle(const std::string& bundleName, bool& ret) override
    {
        return SUCCESS;
    }

    ErrCode OnQueryIsForceGetZoneDevice(const std::string& bundleName, bool& ret) override
    {
        return SUCCESS;
    }

    ErrCode OnCheckVKBInfo(const std::string& bundleName, bool& isValid) override
    {
        return SUCCESS;
    }

    ErrCode OnQueryAllowedPlayback(int32_t uid, int32_t pid, bool& ret) override
    {
        return SUCCESS;
    }

    ErrCode OnBackgroundMute(int32_t uid) override
    {
        return SUCCESS;
    }

    ErrCode OnQueryBundleNameIsInList(const std::string& bundleName, const std::string& listType, bool& ret) override
    {
        ret = true;
        return SUCCESS;
    }

    ErrCode OnQueryDeviceVolumeBehavior(VolumeBehavior &volumeBehavior) override
    {
        volumeBehavior.isReady = false;
        volumeBehavior.isVolumeControlDisabled = false;
        volumeBehavior.databaseVolumeName = "";
        return SUCCESS;
    }

    ErrCode OnQueryIsForceGetDevByVolumeType(const std::string &bundleName, bool &ret) override
    {
        return SUCCESS;
    }
};

/**
* @tc.name  : Test AudioStateManager.
* @tc.number: AudioStateManagerUnitTest_001
* @tc.desc  : Test SetPreferredMediaRenderDevice interface.
*/
HWTEST_F(AudioStateManagerUnitTest, AudioStateManagerUnitTest_001, TestSize.Level1)
{
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioStateManager::GetAudioStateManager().SetPreferredMediaRenderDevice(desc);
    EXPECT_NE(AudioStateManager::GetAudioStateManager().GetPreferredMediaRenderDevice(), nullptr);
}

/**
* @tc.name  : Test AudioStateManager.
* @tc.number: AudioStateManagerUnitTest_002
* @tc.desc  : Test SetAndGetPreferredCallRenderDevice interface.
*/
HWTEST_F(AudioStateManagerUnitTest, AudioStateManagerUnitTest_002, TestSize.Level1)
{
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    shared_ptr<AudioDeviceDescriptor> speaker = std::make_shared<AudioDeviceDescriptor>();
    speaker->deviceType_ = DEVICE_TYPE_SPEAKER;
    AudioStateManager::GetAudioStateManager().SetPreferredCallRenderDevice(desc, 0);
    AudioStateManager::GetAudioStateManager().SetPreferredCallRenderDevice(desc, -1);
    AudioStateManager::GetAudioStateManager().SetPreferredCallRenderDevice(desc, 1);
    AudioStateManager::GetAudioStateManager().SetPreferredCallRenderDevice(speaker, -1);
    AudioStateManager::GetAudioStateManager().SetPreferredCallRenderDevice(speaker, 1);
    AudioStateManager::GetAudioStateManager().SetPreferredCallRenderDevice(speaker, 789);
    shared_ptr<AudioDeviceDescriptor> deviceDesc =
        AudioStateManager::GetAudioStateManager().GetPreferredCallRenderDevice();
    EXPECT_NE(deviceDesc, nullptr);
    AudioStateManager::GetAudioStateManager().SetAudioSceneOwnerUid(789);
    deviceDesc = AudioStateManager::GetAudioStateManager().GetPreferredCallRenderDevice();
    EXPECT_NE(deviceDesc, nullptr);
    AudioStateManager::GetAudioStateManager().SetAudioSceneOwnerUid(790);
    deviceDesc = AudioStateManager::GetAudioStateManager().GetPreferredCallRenderDevice();
    EXPECT_NE(deviceDesc, nullptr);
    sptr<IStandardAudioPolicyManagerListener> callback = new IStandardAudioPolicyManagerListenerStub();
    AudioStateManager::GetAudioStateManager().SetAudioClientInfoMgrCallback(callback);
    AudioStateManager::GetAudioStateManager().SetPreferredCallRenderDevice(speaker, 1);
    AudioStateManager::GetAudioStateManager().SetPreferredCallRenderDevice(desc, 790);
    deviceDesc = AudioStateManager::GetAudioStateManager().GetPreferredCallRenderDevice();
    EXPECT_NE(deviceDesc, speaker);
}

/**
* @tc.name  : Test AudioStateManager.
* @tc.number: AudioStateManagerUnitTest_003
* @tc.desc  : Test SetPreferredCallCaptureDevice interface.
*/
HWTEST_F(AudioStateManagerUnitTest, AudioStateManagerUnitTest_003, TestSize.Level1)
{
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioStateManager::GetAudioStateManager().SetPreferredCallCaptureDevice(desc);
    EXPECT_NE(AudioStateManager::GetAudioStateManager().GetPreferredCallCaptureDevice(), nullptr);
}

/**
* @tc.name  : Test AudioStateManager.
* @tc.number: AudioStateManagerUnitTest_004
* @tc.desc  : Test SetPreferredCallCaptureDevice interface.
*/
HWTEST_F(AudioStateManagerUnitTest, AudioStateManagerUnitTest_004, TestSize.Level1)
{
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioStateManager::GetAudioStateManager().SetPreferredRingRenderDevice(desc);
    EXPECT_NE(AudioStateManager::GetAudioStateManager().GetPreferredRingRenderDevice(), nullptr);
}

/**
* @tc.name  : Test AudioStateManager.
* @tc.number: AudioStateManagerUnitTest_005
* @tc.desc  : Test SetPreferredRecordCaptureDevice interface.
*/
HWTEST_F(AudioStateManagerUnitTest, AudioStateManagerUnitTest_005, TestSize.Level1)
{
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioStateManager::GetAudioStateManager().SetPreferredRecordCaptureDevice(desc);
    EXPECT_NE(AudioStateManager::GetAudioStateManager().GetPreferredRecordCaptureDevice(), nullptr);
}

/**
* @tc.name  : Test AudioStateManager.
* @tc.number: AudioStateManagerUnitTest_006
* @tc.desc  : Test SetPreferredToneRenderDevice interface.
*/
HWTEST_F(AudioStateManagerUnitTest, AudioStateManagerUnitTest_006, TestSize.Level1)
{
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioStateManager::GetAudioStateManager().SetPreferredToneRenderDevice(desc);
    EXPECT_NE(AudioStateManager::GetAudioStateManager().GetPreferredToneRenderDevice(), nullptr);
}

/**
* @tc.name  : Test AudioStateManager.
* @tc.number: AudioStateManagerUnitTest_007
* @tc.desc  : Test UpdatePreferredMediaRenderDeviceConnectState interface.
*/
HWTEST_F(AudioStateManagerUnitTest, AudioStateManagerUnitTest_007, TestSize.Level1)
{
    AudioStateManager::GetAudioStateManager().UpdatePreferredMediaRenderDeviceConnectState(
        ConnectState::CONNECTED);
    EXPECT_NE(AudioStateManager::GetAudioStateManager().GetPreferredMediaRenderDevice()->connectState_,
        ConnectState::CONNECTED);
}

/**
* @tc.name  : Test AudioStateManager.
* @tc.number: AudioStateManagerUnitTest_008
* @tc.desc  : Test UpdatePreferredCallRenderDeviceConnectState interface.
*/
HWTEST_F(AudioStateManagerUnitTest, AudioStateManagerUnitTest_008, TestSize.Level1)
{
    AudioStateManager::GetAudioStateManager().UpdatePreferredCallRenderDeviceConnectState(
        ConnectState::CONNECTED);
    EXPECT_NE(AudioStateManager::GetAudioStateManager().GetPreferredCallRenderDevice()->connectState_,
        ConnectState::CONNECTED);
}

/**
* @tc.name  : Test AudioStateManager.
* @tc.number: AudioStateManagerUnitTest_009
* @tc.desc  : Test UpdatePreferredCallCaptureDeviceConnectState interface.
*/
HWTEST_F(AudioStateManagerUnitTest, AudioStateManagerUnitTest_009, TestSize.Level1)
{
    AudioStateManager::GetAudioStateManager().UpdatePreferredCallCaptureDeviceConnectState(
        ConnectState::CONNECTED);
    EXPECT_NE(AudioStateManager::GetAudioStateManager().GetPreferredCallCaptureDevice()->connectState_,
        ConnectState::CONNECTED);
}

/**
* @tc.name  : Test AudioStateManager.
* @tc.number: AudioStateManagerUnitTest_010
* @tc.desc  : Test UpdatePreferredRecordCaptureDeviceConnectState interface.
*/
HWTEST_F(AudioStateManagerUnitTest, AudioStateManagerUnitTest_010, TestSize.Level1)
{
    AudioStateManager::GetAudioStateManager().UpdatePreferredRecordCaptureDeviceConnectState(
        ConnectState::CONNECTED);
    EXPECT_NE(AudioStateManager::GetAudioStateManager().GetPreferredRecordCaptureDevice()->connectState_,
        ConnectState::CONNECTED);
}

/**
* @tc.name  : Test AudioStateManager.
* @tc.number: AudioStateManagerUnitTest_011
* @tc.desc  : Test SetPreferredRecognitionCaptureDevice interface.
*/
HWTEST_F(AudioStateManagerUnitTest, SetPreferredRecognitionCaptureDevice_1, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioStateManager::GetAudioStateManager().SetPreferredRecognitionCaptureDevice(desc);
    EXPECT_NE(AudioStateManager::GetAudioStateManager().GetPreferredRecognitionCaptureDevice(), nullptr);
}

/**
* @tc.name  : Test AudioStateManager.
* @tc.number: AudioStateManagerUnitTest_011
* @tc.desc  : Test ExcludeOutputDevices interface.
*/
HWTEST_F(AudioStateManagerUnitTest, AudioStateManagerUnitTest_011, TestSize.Level1)
{
    AudioDeviceUsage audioDevUsage = MEDIA_OUTPUT_DEVICES;
    auto descriptor = std::make_shared<AudioDeviceDescriptor>();
    vector<shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    audioDeviceDescriptors.push_back(descriptor);
    AudioStateManager::GetAudioStateManager().ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
    AudioStateManager::GetAudioStateManager().UnexcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
    EXPECT_NE(0, audioDeviceDescriptors.size());

    audioDevUsage = CALL_OUTPUT_DEVICES;
    AudioStateManager::GetAudioStateManager().ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
    AudioStateManager::GetAudioStateManager().UnexcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
    EXPECT_NE(0, audioDeviceDescriptors.size());

    audioDevUsage = CALL_INPUT_DEVICES;
    AudioStateManager::GetAudioStateManager().ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
    AudioStateManager::GetAudioStateManager().UnexcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
    EXPECT_NE(0, audioDeviceDescriptors.size());
}

/**
* @tc.name  : Test AudioStateManager.
* @tc.number: AudioStateManagerUnitTest_012
* @tc.desc  : Test SetAudioSceneOwnerUid interface.
*/
HWTEST_F(AudioStateManagerUnitTest, AudioStateManagerUnitTest_012, TestSize.Level1)
{
    int32_t uid = 1041;
    AudioStateManager::GetAudioStateManager().SetAudioSceneOwnerUid(uid);
    EXPECT_EQ(ANCO_SERVICE_BROKER_UID, AudioStateManager::GetAudioStateManager().ownerUid_);
}

/**
* @tc.name  : Test AudioStateManager.
* @tc.number: AudioStateManagerUnitTest_013
* @tc.desc  : Test GetExcludedDevices interface.
*/
HWTEST_F(AudioStateManagerUnitTest, AudioStateManagerUnitTest_013, TestSize.Level1)
{
    AudioDeviceUsage usage = CALL_INPUT_DEVICES;
    auto ret = AudioStateManager::GetAudioStateManager().GetExcludedDevices(usage);
    EXPECT_EQ(0, ret.size());
}

/**
* @tc.name  : Test AudioStateManager.
* @tc.number: FindExcludedDevices_001
* @tc.desc  : Test FindExcludedDevices interface.
*/
HWTEST_F(AudioStateManagerUnitTest, FindExcludedDevices_001, TestSize.Level1)
{
    auto &man = AudioStateManager::GetAudioStateManager();
    ExcludedDevices *devs;
    devs = man.FindExcludedDevices(MEDIA_OUTPUT_DEVICES);
    EXPECT_NE(devs, nullptr);
    devs = man.FindExcludedDevices(MEDIA_INPUT_DEVICES);
    EXPECT_NE(devs, nullptr);
    devs = man.FindExcludedDevices(CALL_OUTPUT_DEVICES);
    EXPECT_NE(devs, nullptr);
    devs = man.FindExcludedDevices(CALL_INPUT_DEVICES);
    EXPECT_NE(devs, nullptr);
    devs = man.FindExcludedDevices(D_ALL_DEVICES);
    EXPECT_EQ(devs, nullptr);
}

/**
* @tc.name  : Test AudioStateManager.
* @tc.number: ExcludeDevices_001
* @tc.desc  : Test ExcludeDevices interface.
*/
HWTEST_F(AudioStateManagerUnitTest, ExcludeDevices_001, TestSize.Level1)
{
    auto &man = AudioStateManager::GetAudioStateManager();
    auto dev = make_shared<AudioDeviceDescriptor>();
    dev->deviceId_ = 1000;
    dev->deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    dev->deviceName_ = "--";
    dev->deviceRole_ = OUTPUT_DEVICE;
    dev->macAddress_ = "card=2;device=0";
    man.ExcludeDevices(MEDIA_OUTPUT_DEVICES, {dev});
    EXPECT_TRUE(man.IsExcludedDevice(MEDIA_OUTPUT_DEVICES, dev));
}

/**
* @tc.name  : Test AudioStateManager.
* @tc.number: UnexcludeDevices_001
* @tc.desc  : Test UnexcludeDevices interface.
*/
HWTEST_F(AudioStateManagerUnitTest, UnexcludeDevices_001, TestSize.Level1)
{
    auto &man = AudioStateManager::GetAudioStateManager();
    auto dev = make_shared<AudioDeviceDescriptor>();
    dev->deviceId_ = 1000;
    dev->deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    dev->deviceName_ = "--";
    dev->deviceRole_ = OUTPUT_DEVICE;
    dev->macAddress_ = "card=2;device=0";
    man.UnexcludeDevices(MEDIA_OUTPUT_DEVICES, {dev});
    EXPECT_FALSE(man.IsExcludedDevice(MEDIA_OUTPUT_DEVICES, dev));
}

/**
* @tc.name  : Test AudioStateManager.
* @tc.number: AudioStateManagerUnitTest_014
* @tc.desc  : Test SetAndGetPreferredCallRenderDeviceForUid interface.
*/
HWTEST_F(AudioStateManagerUnitTest, AudioStateManagerUnitTest_014, TestSize.Level1)
{
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    shared_ptr<AudioDeviceDescriptor> speaker = std::make_shared<AudioDeviceDescriptor>();
    speaker->deviceType_ = DEVICE_TYPE_SPEAKER;
    AudioStateManager::GetAudioStateManager().SetPreferredCallRenderDevice(desc, 0);
    AudioStateManager::GetAudioStateManager().SetAudioSceneOwnerUid(299);
    AudioStateManager::GetAudioStateManager().SetPreferredCallRenderDevice(speaker, 299);
    shared_ptr<AudioDeviceDescriptor> deviceDesc =
        AudioStateManager::GetAudioStateManager().GetPreferredCallRenderDeviceForUid(299);
    EXPECT_EQ(deviceDesc->deviceType_, DEVICE_TYPE_SPEAKER);
    AudioStateManager::GetAudioStateManager().SetPreferredCallRenderDevice(speaker, 1);
    deviceDesc =
        AudioStateManager::GetAudioStateManager().GetPreferredCallRenderDeviceForUid(1);
    EXPECT_EQ(deviceDesc->deviceType_, DEVICE_TYPE_SPEAKER);
    deviceDesc =
        AudioStateManager::GetAudioStateManager().GetPreferredCallRenderDeviceForUid(456);
    EXPECT_EQ(deviceDesc->deviceType_, DEVICE_TYPE_SPEAKER);
    AudioStateManager::GetAudioStateManager().SetAudioSceneOwnerUid(0);
    AudioStateManager::GetAudioStateManager().SetPreferredCallRenderDevice(speaker, 1);
    deviceDesc = AudioStateManager::GetAudioStateManager().GetPreferredCallRenderDevice();
    EXPECT_EQ(deviceDesc->deviceType_, DEVICE_TYPE_SPEAKER);
    AudioStateManager::GetAudioStateManager().SetPreferredCallRenderDevice(desc, 0);
}

/**
* @tc.name  : Test IsRepeatedPreferredCallRenderer.
* @tc.number: IsRepeatedPreferredCallRenderer_001
* @tc.desc  : Test IsRepeatedPreferredCallRenderer clear by CLEAR_UID.
*/
HWTEST_F(AudioStateManagerUnitTest, IsRepeatedPreferredCallRenderer_001, TestSize.Level1)
{
    auto &stateManager = AudioStateManager::GetAudioStateManager();
    const auto preferred = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_NONE, OUTPUT_DEVICE);
    const int32_t callerUid = CLEAR_UID;
    const int32_t ownerUid = 123;
    std::list<std::map<int32_t, std::shared_ptr<AudioDeviceDescriptor>>> forcedList = {
        {
            { ownerUid, std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE) }
        }
    };

    EXPECT_FALSE(stateManager.IsRepeatedPreferredCallRenderer(preferred, callerUid, ownerUid, forcedList));

    forcedList = {
    };
    EXPECT_TRUE(stateManager.IsRepeatedPreferredCallRenderer(preferred, callerUid, ownerUid, forcedList));
}

/**
* @tc.name  : Test IsRepeatedPreferredCallRenderer.
* @tc.number: IsRepeatedPreferredCallRenderer_002
* @tc.desc  : Test IsRepeatedPreferredCallRenderer clear by SYSTEM_UID or ownerUid.
*/
HWTEST_F(AudioStateManagerUnitTest, IsRepeatedPreferredCallRenderer_002, TestSize.Level1)
{
    auto &stateManager = AudioStateManager::GetAudioStateManager();
    const auto preferred = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_NONE, OUTPUT_DEVICE);
    int32_t callerUid = SYSTEM_UID;
    const int32_t ownerUid = 123;
    std::list<std::map<int32_t, std::shared_ptr<AudioDeviceDescriptor>>> forcedList = {
        {
            { 456, std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE) }
        }
    };

    EXPECT_TRUE(stateManager.IsRepeatedPreferredCallRenderer(preferred, callerUid, ownerUid, forcedList));

    callerUid = ownerUid;
    EXPECT_TRUE(stateManager.IsRepeatedPreferredCallRenderer(preferred, callerUid, ownerUid, forcedList));

    forcedList = {
        {
            { SYSTEM_UID, std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE) }
        }
    };
    EXPECT_FALSE(stateManager.IsRepeatedPreferredCallRenderer(preferred, callerUid, ownerUid, forcedList));

    callerUid = SYSTEM_UID;
    EXPECT_FALSE(stateManager.IsRepeatedPreferredCallRenderer(preferred, callerUid, ownerUid, forcedList));
}

/**
* @tc.name  : Test IsRepeatedPreferredCallRenderer.
* @tc.number: IsRepeatedPreferredCallRenderer_003
* @tc.desc  : Test IsRepeatedPreferredCallRenderer clear by callerUid.
*/
HWTEST_F(AudioStateManagerUnitTest, IsRepeatedPreferredCallRenderer_003, TestSize.Level1)
{
    auto &stateManager = AudioStateManager::GetAudioStateManager();
    const auto preferred = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_NONE, OUTPUT_DEVICE);
    const int32_t callerUid = 123;
    const int32_t ownerUid = 456;
    std::list<std::map<int32_t, std::shared_ptr<AudioDeviceDescriptor>>> forcedList = {
        {
            { ownerUid, std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE) }
        }
    };

    EXPECT_TRUE(stateManager.IsRepeatedPreferredCallRenderer(preferred, callerUid, ownerUid, forcedList));

    forcedList = {
        {
            { callerUid, std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE) }
        }
    };
    EXPECT_FALSE(stateManager.IsRepeatedPreferredCallRenderer(preferred, callerUid, ownerUid, forcedList));
}

/**
* @tc.name  : Test IsRepeatedPreferredCallRenderer.
* @tc.number: IsRepeatedPreferredCallRenderer_004
* @tc.desc  : Test IsRepeatedPreferredCallRenderer first set.
*/
HWTEST_F(AudioStateManagerUnitTest, IsRepeatedPreferredCallRenderer_004, TestSize.Level1)
{
    auto &stateManager = AudioStateManager::GetAudioStateManager();
    const auto preferred = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    const int32_t callerUid = 123;
    const int32_t ownerUid = 456;
    std::list<std::map<int32_t, std::shared_ptr<AudioDeviceDescriptor>>> forcedList = {
    };

    EXPECT_FALSE(stateManager.IsRepeatedPreferredCallRenderer(preferred, callerUid, ownerUid, forcedList));
}

/**
* @tc.name  : Test IsRepeatedPreferredCallRenderer.
* @tc.number: IsRepeatedPreferredCallRenderer_005
* @tc.desc  : Test IsRepeatedPreferredCallRenderer not first set.
*/
HWTEST_F(AudioStateManagerUnitTest, IsRepeatedPreferredCallRenderer_005, TestSize.Level1)
{
    auto &stateManager = AudioStateManager::GetAudioStateManager();
    const auto preferred = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    const int32_t callerUid = 123;
    const int32_t ownerUid = 456;
    std::list<std::map<int32_t, std::shared_ptr<AudioDeviceDescriptor>>> forcedList = {
        {
            { ownerUid, std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE) }
        }
    };

    EXPECT_FALSE(stateManager.IsRepeatedPreferredCallRenderer(preferred, callerUid, ownerUid, forcedList));

    forcedList = {
        {
            { callerUid, std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE) }
        }
    };
    EXPECT_TRUE(stateManager.IsRepeatedPreferredCallRenderer(preferred, callerUid, ownerUid, forcedList));
}

/**
* @tc.name  : Test IsSamePreferred.
* @tc.number: IsSamePreferred_001
* @tc.desc  : Test IsSamePreferred uid.
*/
HWTEST_F(AudioStateManagerUnitTest, IsSamePreferred_001, TestSize.Level1)
{
    auto &stateManager = AudioStateManager::GetAudioStateManager();
    const int32_t uid = 123;
    const auto preferred = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    const std::map<int32_t, std::shared_ptr<AudioDeviceDescriptor>> recordMap = {
            { 456, std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE) }
    };

    EXPECT_FALSE(stateManager.IsSamePreferred(uid, preferred, recordMap));
}

/**
* @tc.name  : Test IsSamePreferred.
* @tc.number: IsSamePreferred_002
* @tc.desc  : Test IsSamePreferred device.
*/
HWTEST_F(AudioStateManagerUnitTest, IsSamePreferred_002, TestSize.Level1)
{
    auto &stateManager = AudioStateManager::GetAudioStateManager();
    const int32_t uid = 123;
    const auto preferred = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    const std::map<int32_t, std::shared_ptr<AudioDeviceDescriptor>> recordMap = {
        { uid, std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE) }
    };

    EXPECT_FALSE(stateManager.IsSamePreferred(uid, preferred, recordMap));
}

/**
* @tc.name  : Test IsSamePreferred.
* @tc.number: IsSamePreferred_003
* @tc.desc  : Test IsSamePreferred uid and device.
*/
HWTEST_F(AudioStateManagerUnitTest, IsSamePreferred_003, TestSize.Level1)
{
    auto &stateManager = AudioStateManager::GetAudioStateManager();
    const int32_t uid = 123;
    const auto preferred = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    const std::map<int32_t, std::shared_ptr<AudioDeviceDescriptor>> recordMap = {
        { uid, std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE) }
    };

    EXPECT_TRUE(stateManager.IsSamePreferred(uid, preferred, recordMap));
}
} // namespace AudioStandard
} // namespace OHOS
 