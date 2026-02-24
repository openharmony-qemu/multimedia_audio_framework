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

#include <list>
#include <set>
#include "va_device_manager_test.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "standard_client_tracker_stub.h"
#include "audio_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void VADeviceManagerTest::SetUpTestCase(void) {}
void VADeviceManagerTest::TearDownTestCase(void) {}
void VADeviceManagerTest::SetUp(void) {}
void VADeviceManagerTest::TearDown(void) {}

/**
 * @tc.name  : Test VADeviceManager.
 * @tc.number: VADeviceManagerTest_001
 * @tc.desc  : Test OnDevicesConnected.
 */
HWTEST_F(VADeviceManagerTest, VADeviceManagerTest_001, TestSize.Level1)
{
    auto vADeviceManager = std::make_shared<VADeviceManager>();
    ASSERT_NE(vADeviceManager, nullptr);

    std::shared_ptr<VADevice> vaDevice = std::make_shared<VADevice>();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_NE(samgr, nullptr);

    static int32_t systemAbilityId = 3009;
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(systemAbilityId);
    ASSERT_NE(remoteObject, nullptr);

    sptr<IVADeviceController> controller = iface_cast<IVADeviceController>(remoteObject);
    ASSERT_NE(controller, nullptr);

    auto coreService = std::make_shared<AudioCoreService>();
    AudioCoreService::GetCoreService()->eventEntry_ = std::make_shared<AudioCoreService::EventEntry>(coreService);

    vADeviceManager->OnDevicesConnected(vaDevice, controller);
    EXPECT_NE(vaDevice, nullptr);
}

/**
 * @tc.name  : Test VADeviceManager.
 * @tc.number: VADeviceManagerTest_002
 * @tc.desc  : Test OnDevicesDisconnected.
 */
HWTEST_F(VADeviceManagerTest, VADeviceManagerTest_002, TestSize.Level1)
{
    auto vADeviceManager = std::make_shared<VADeviceManager>();
    ASSERT_NE(vADeviceManager, nullptr);
    std::shared_ptr<VADevice> vaDevice = std::make_shared<VADevice>();

    vADeviceManager->connectedVADeviceMap_.clear();

    auto coreService = std::make_shared<AudioCoreService>();
    AudioCoreService::GetCoreService()->eventEntry_ = std::make_shared<AudioCoreService::EventEntry>(coreService);

    vADeviceManager->OnDevicesDisconnected(vaDevice);
    EXPECT_NE(vaDevice, nullptr);
}


/**
 * @tc.name  : Test VADeviceManager.
 * @tc.number: VADeviceManagerTest_003
 * @tc.desc  : Test OnDevicesDisconnected.
 */
HWTEST_F(VADeviceManagerTest, VADeviceManagerTest_003, TestSize.Level1)
{
    auto vADeviceManager = std::make_shared<VADeviceManager>();
    ASSERT_NE(vADeviceManager, nullptr);
    std::shared_ptr<VADevice> vaDevice = std::make_shared<VADevice>();

    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_NE(samgr, nullptr);

    static int32_t systemAbilityId = 3009;
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(systemAbilityId);
    ASSERT_NE(remoteObject, nullptr);

    sptr<IVADeviceController> controller = iface_cast<IVADeviceController>(remoteObject);
    ASSERT_NE(controller, nullptr);

    vADeviceManager->connectedVADeviceMap_.insert(std::make_pair("device1", controller));
    vADeviceManager->connectedVADeviceMap_.insert(std::make_pair("device2", nullptr));

    auto coreService = std::make_shared<AudioCoreService>();
    AudioCoreService::GetCoreService()->eventEntry_ = std::make_shared<AudioCoreService::EventEntry>(coreService);

    vADeviceManager->OnDevicesDisconnected(vaDevice);
    EXPECT_NE(vaDevice, nullptr);
}

/**
 * @tc.name  : Test VADeviceManager.
 * @tc.number: VADeviceManagerTest_004
 * @tc.desc  : Test GetDeviceController.
 */
HWTEST_F(VADeviceManagerTest, VADeviceManagerTest_004, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("audio_server");
    MockNative::Mock();
    auto vADeviceManager = std::make_shared<VADeviceManager>();
    ASSERT_NE(vADeviceManager, nullptr);

    std::string macAddr = "test";
    sptr<IRemoteObject> controller;
    vADeviceManager->connectedVADeviceMap_.clear();
    vADeviceManager->GetDeviceController(macAddr, controller);

    vADeviceManager->connectedVADeviceMap_.insert(std::make_pair("test", nullptr));
    vADeviceManager->GetDeviceController(macAddr, controller);
    EXPECT_EQ(controller, nullptr);
    MockNative::Resume();
}

/**
 * @tc.name  : Test VADeviceManager.
 * @tc.number: VADeviceManagerTest_005
 * @tc.desc  : Test GetDeviceController.
 */
HWTEST_F(VADeviceManagerTest, VADeviceManagerTest_005, TestSize.Level1)
{
    MockNative::GenerateNativeTokenID("audio_server");
    MockNative::Mock();
    auto vADeviceManager = std::make_shared<VADeviceManager>();
    ASSERT_NE(vADeviceManager, nullptr);

    std::string macAddr = "test";
    sptr<IRemoteObject> controller;
    vADeviceManager->connectedVADeviceMap_.clear();

    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_NE(samgr, nullptr);

    static int32_t systemAbilityId = 3009;
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(systemAbilityId);
    ASSERT_NE(remoteObject, nullptr);

    sptr<IVADeviceController> deviceController = iface_cast<IVADeviceController>(remoteObject);
    ASSERT_NE(deviceController, nullptr);

    vADeviceManager->connectedVADeviceMap_.insert(std::make_pair("test", deviceController));
    vADeviceManager->GetDeviceController(macAddr, controller);
    EXPECT_NE(controller, nullptr);
    MockNative::Resume();
}

/**
 * @tc.name  : Test VADeviceManager.
 * @tc.number: VADeviceManagerTest_006
 * @tc.desc  : Test GetDeviceController.
 */
HWTEST_F(VADeviceManagerTest, VADeviceManagerTest_006, TestSize.Level1)
{
    auto vADeviceManager = std::make_shared<VADeviceManager>();
    ASSERT_NE(vADeviceManager, nullptr);

    VAAudioStreamProperty vaStreamProperty;
    vaStreamProperty.samplesPerCycle_ = AudioSamplingRate::SAMPLE_RATE_192000;
    auto ret = vADeviceManager->CalculateBufferSize(vaStreamProperty);
    EXPECT_EQ(ret, 0);
}
} // namespace AudioStandard
} // namespace OHOS
