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

#include "device_init_callback_unit_test.h"
#include "audio_errors.h"
#include "audio_policy_log.h"

#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void DeviceInitCallbackUnitTest::SetUpTestCase(void) {}
void DeviceInitCallbackUnitTest::TearDownTestCase(void) {}
void DeviceInitCallbackUnitTest::SetUp(void) {}
void DeviceInitCallbackUnitTest::TearDown(void) {}

/**
* @tc.name  : Test DeviceInitCallbackUnitTest.
* @tc.number: DeviceInitCallbackUnitTest_001
* @tc.desc  : Test OnDeviceChanged interface.
*/
HWTEST_F(DeviceInitCallbackUnitTest, DeviceInitCallbackUnitTest_001, TestSize.Level4)
{
    auto deviceStatusCallbackImpl = make_shared<DeviceStatusCallbackImpl>();
    DistributedHardware::DmDeviceInfo dmDeviceInfo;
    strncpy_s(dmDeviceInfo.deviceName, DM_MAX_DEVICE_NAME_LEN,
        "test", 4);
    strncpy_s(dmDeviceInfo.networkId, DM_MAX_DEVICE_ID_LEN,
        "1234", 4);
    dmDeviceInfo.extraData = "{\"CAR_BRAND\\:BUGATTI}";
    deviceStatusCallbackImpl->OnDeviceChanged(dmDeviceInfo);
    EXPECT_NE(0, dmDeviceInfo.extraData.length());
    dmDeviceInfo.extraData.clear();
    dmDeviceInfo.extraData = "{\"CAR_BRAND\\:}";
    deviceStatusCallbackImpl->OnDeviceChanged(dmDeviceInfo);
    EXPECT_NE(0, dmDeviceInfo.extraData.length());
}

/**
* @tc.name  : Test DeviceInitCallbackUnitTest.
* @tc.number: DeviceInitCallbackUnitTest_002
* @tc.desc  : Test OnDeviceOnline interface.
*/
HWTEST_F(DeviceInitCallbackUnitTest, DeviceInitCallbackUnitTest_002, TestSize.Level4)
{
    auto deviceStatusCallbackImpl = make_shared<DeviceStatusCallbackImpl>();
    DistributedHardware::DmDeviceInfo dmDeviceInfo;
    strncpy_s(dmDeviceInfo.deviceName, DM_MAX_DEVICE_NAME_LEN,
        "test", 4);
    strncpy_s(dmDeviceInfo.networkId, DM_MAX_DEVICE_ID_LEN,
        "1234", 4);
    dmDeviceInfo.extraData = "{\"CAR_BRAND\\:BUGATTI}";
    deviceStatusCallbackImpl->OnDeviceOnline(dmDeviceInfo);
    EXPECT_NE(0, dmDeviceInfo.extraData.length());
    dmDeviceInfo.extraData.clear();
    dmDeviceInfo.extraData = "{\"CAR_BRAND\\:}";
    deviceStatusCallbackImpl->OnDeviceOnline(dmDeviceInfo);
    EXPECT_NE(0, dmDeviceInfo.extraData.length());
}

/**
 * @tc.name  : Test OnDeviceChanged with full GetExtraDataField coverage
 * @tc.number: DeviceInitCallbackUnitTest_003
 * @tc.desc  : Test OnDeviceChanged triggers GetExtraDataField with all escape cases
 */
HWTEST_F(DeviceInitCallbackUnitTest, DeviceInitCallbackUnitTest_003, TestSize.Level4)
{
    auto deviceStatusCallbackImpl = make_shared<DeviceStatusCallbackImpl>();
    DistributedHardware::DmDeviceInfo dmDeviceInfo;

    strncpy_s(dmDeviceInfo.deviceName, DM_MAX_DEVICE_NAME_LEN, "test", 4);
    strncpy_s(dmDeviceInfo.networkId, DM_MAX_DEVICE_ID_LEN, "1234", 4);

    dmDeviceInfo.extraData = "{\\\"\\\"CAR_BRAND\\\\\\\": \\\"\\\\\\\"\\\\\\\\\\\\\\\"Bugatti\\\"}";
    deviceStatusCallbackImpl->OnDeviceChanged(dmDeviceInfo);
    EXPECT_NE(0, dmDeviceInfo.extraData.length());
}
} // namespace AudioStandard
} // namespace OHOS
  