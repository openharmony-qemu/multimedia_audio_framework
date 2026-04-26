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
* @tc.number: AudioStateManagerUnitTest_004
* @tc.desc  : Test SetPreferredRingRenderDevice interface.
*/
HWTEST_F(AudioStateManagerUnitTest, AudioStateManagerUnitTest_004, TestSize.Level1)
{
    shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioStateManager::GetAudioStateManager().SetPreferredRingRenderDevice(desc);
    EXPECT_NE(AudioStateManager::GetAudioStateManager().GetPreferredRingRenderDevice(), nullptr);
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
* @tc.number: AudioStateManagerUnitTest_011
* @tc.desc  : Test SetPreferredRecognitionCaptureDevice interface.
*/
HWTEST_F(AudioStateManagerUnitTest, SetPreferredRecognitionCaptureDevice_1, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    AudioStateManager::GetAudioStateManager().SetPreferredRecognitionCaptureDevice(desc);
    EXPECT_NE(AudioStateManager::GetAudioStateManager().GetPreferredRecognitionCaptureDevice(), nullptr);
}
} // namespace AudioStandard
} // namespace OHOS
 