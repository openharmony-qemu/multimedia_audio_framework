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
#include "device_status_listener_unit_test.h"
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void DeviceStatusListenerUnitTest::SetUpTestCase(void) {}
void DeviceStatusListenerUnitTest::TearDownTestCase(void) {}
void DeviceStatusListenerUnitTest::SetUp(void) {}
void DeviceStatusListenerUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test DeviceStatusListener.
 * @tc.number: DeviceStatusListener_001
 * @tc.desc  : Test GetInternalDeviceType().
 */
HWTEST(DeviceStatusListenerUnitTest, DeviceStatusListener_001, TestSize.Level1)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());

    const std::string info = "EVENT_TYPE=1;DEVICE_TYPE=4;";

    deviceStatusListenerPtr->OnMicrophoneBlocked(info);
    EXPECT_NE(deviceStatusListenerPtr, nullptr);
}

/**
 * @tc.name  : Test DeviceStatusListener.
 * @tc.number: DeviceStatusListener_002
 * @tc.desc  : Test GetInternalDeviceType().
 */
HWTEST(DeviceStatusListenerUnitTest, DeviceStatusListener_002, TestSize.Level1)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());

    const std::string info = "EVENT_TYPE=1;DEVICE_TYPE=2;";

    deviceStatusListenerPtr->OnMicrophoneBlocked(info);
    EXPECT_NE(deviceStatusListenerPtr, nullptr);
}

/**
 * @tc.name  : Test DeviceStatusListener.
 * @tc.number: DeviceStatusListener_003
 * @tc.desc  : Test GetInternalDeviceType().
 */
HWTEST(DeviceStatusListenerUnitTest, DeviceStatusListener_003, TestSize.Level1)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());

    const std::string info = "EVENT_TYPE=1;DEVICE_TYPE=8;";

    deviceStatusListenerPtr->OnMicrophoneBlocked(info);
    EXPECT_NE(deviceStatusListenerPtr, nullptr);
}

/**
 * @tc.name  : Test DeviceStatusListener.
 * @tc.number: DeviceStatusListener_004
 * @tc.desc  : Test GetInternalDeviceType().
 */
HWTEST(DeviceStatusListenerUnitTest, DeviceStatusListener_004, TestSize.Level1)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());

    const std::string info = "EVENT_TYPE=1;DEVICE_TYPE=2048;";

    deviceStatusListenerPtr->OnMicrophoneBlocked(info);
    EXPECT_NE(deviceStatusListenerPtr, nullptr);
}

/**
 * @tc.name  : Test DeviceStatusListener.
 * @tc.number: DeviceStatusListener_005
 * @tc.desc  : Test GetInternalDeviceType().
 */
HWTEST(DeviceStatusListenerUnitTest, DeviceStatusListener_005, TestSize.Level1)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());

    const std::string info = "EVENT_TYPE=1;DEVICE_TYPE=4096;";

    deviceStatusListenerPtr->OnMicrophoneBlocked(info);
    EXPECT_NE(deviceStatusListenerPtr, nullptr);
}

/**
 * @tc.name  : Test DeviceStatusListener.
 * @tc.number: DeviceStatusListener_006
 * @tc.desc  : Test GetInternalDeviceType().
 */
HWTEST(DeviceStatusListenerUnitTest, DeviceStatusListener_006, TestSize.Level1)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());

    const std::string info = "EVENT_TYPE=1;DEVICE_TYPE=8192;";

    deviceStatusListenerPtr->OnMicrophoneBlocked(info);
    EXPECT_NE(deviceStatusListenerPtr, nullptr);
}

/**
 * @tc.name  : Test DeviceStatusListener.
 * @tc.number: DeviceStatusListener_007
 * @tc.desc  : Test GetInternalDeviceType().
 */
HWTEST(DeviceStatusListenerUnitTest, DeviceStatusListener_007, TestSize.Level1)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());

    const std::string info = "EVENT_TYPE=1;DEVICE_TYPE=1;";

    deviceStatusListenerPtr->OnMicrophoneBlocked(info);
    EXPECT_NE(deviceStatusListenerPtr, nullptr);
}

/**
 * @tc.name  : Test DeviceStatusListener.
 * @tc.number: DeviceStatusListener_008
 * @tc.desc  : Test GetInternalDeviceType().
 */
HWTEST(DeviceStatusListenerUnitTest, DeviceStatusListener_008, TestSize.Level1)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());

    const std::string info = "abc";

    deviceStatusListenerPtr->OnMicrophoneBlocked(info);
    EXPECT_NE(deviceStatusListenerPtr, nullptr);
}

/**
 * @tc.name  : Test DeviceStatusListener.
 * @tc.number: DeviceStatusListener_014
 * @tc.desc  : Test DeviceStatusListener::UnRegisterDeviceStatusListener().
 */
HWTEST(DeviceStatusListenerUnitTest, DeviceStatusListener_014, TestSize.Level1)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());
    EXPECT_NE(deviceStatusListenerPtr, nullptr);

    HDIServiceManager hdiServiceManager;
    ServiceStatusListener listener;

    deviceStatusListenerPtr->hdiServiceManager_ = HDIServiceManagerGet();
    deviceStatusListenerPtr->listener_ = HdiServiceStatusListenerNewInstance();
    deviceStatusListenerPtr->audioPnpServer_ = &AudioPnpServer::GetAudioPnpServer();

    EXPECT_NE(deviceStatusListenerPtr->hdiServiceManager_, nullptr);
    EXPECT_NE(deviceStatusListenerPtr->listener_, nullptr);
    EXPECT_NE(deviceStatusListenerPtr->audioPnpServer_, nullptr);

    auto ret = deviceStatusListenerPtr->UnRegisterDeviceStatusListener();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test DeviceStatusListener.
 * @tc.number: DeviceStatusListener_015
 * @tc.desc  : Test DeviceStatusListener::UnRegisterDeviceStatusListener().
 */
HWTEST(DeviceStatusListenerUnitTest, DeviceStatusListener_015, TestSize.Level1)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());
    EXPECT_NE(deviceStatusListenerPtr, nullptr);

    HDIServiceManager hdiServiceManager;
    ServiceStatusListener listener;

    deviceStatusListenerPtr->hdiServiceManager_ = nullptr;
    deviceStatusListenerPtr->listener_ = HdiServiceStatusListenerNewInstance();
    deviceStatusListenerPtr->audioPnpServer_ = &AudioPnpServer::GetAudioPnpServer();

    EXPECT_NE(deviceStatusListenerPtr->listener_, nullptr);
    EXPECT_NE(deviceStatusListenerPtr->audioPnpServer_, nullptr);

    auto ret = deviceStatusListenerPtr->UnRegisterDeviceStatusListener();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test DeviceStatusListener.
 * @tc.number: DeviceStatusListener_016
 * @tc.desc  : Test DeviceStatusListener::UnRegisterDeviceStatusListener().
 */
HWTEST(DeviceStatusListenerUnitTest, DeviceStatusListener_016, TestSize.Level1)
{
    auto deviceStatusListenerPtr = std::make_shared<DeviceStatusListener>(AudioPolicyService::GetAudioPolicyService());
    EXPECT_NE(deviceStatusListenerPtr, nullptr);

    HDIServiceManager hdiServiceManager;
    ServiceStatusListener listener;

    deviceStatusListenerPtr->hdiServiceManager_ = nullptr;
    deviceStatusListenerPtr->listener_ = nullptr;
    deviceStatusListenerPtr->audioPnpServer_ = &AudioPnpServer::GetAudioPnpServer();

    EXPECT_NE(deviceStatusListenerPtr->audioPnpServer_, nullptr);

    auto ret = deviceStatusListenerPtr->UnRegisterDeviceStatusListener();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}
} // namespace AudioStandard
} // namespace OHOS
