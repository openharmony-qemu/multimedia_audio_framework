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
#include <iostream>
#include "gtest/gtest.h"
#include <gmock/gmock.h>
#include "audio_errors.h"
#include "va_device_broker_wrapper_impl_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void VADeviceBrokerWrapperImplTest::SetUpTestCase(void) {}
void VADeviceBrokerWrapperImplTest::TearDownTestCase(void) {}
void VADeviceBrokerWrapperImplTest::SetUp(void) {}
void VADeviceBrokerWrapperImplTest::TearDown(void) {}

/**
 * @tc.name  : Test VADeviceBrokerWrapperImpl.
 * @tc.number: VADeviceBrokerWrapperImpl_001
 * @tc.desc  : Test VADeviceBrokerWrapperImpl OnInterrupt interface.
 */
HWTEST_F(VADeviceBrokerWrapperImplTest, VADeviceBrokerWrapperImpl_001, TestSize.Level3)
{
    VADeviceBrokerWrapperImpl* wrapperImpl = new VADeviceBrokerWrapperImpl();
    VADevice device;
    std::shared_ptr<VADeviceControllerCallback> nullptrCallback = nullptr;

    int32_t result = wrapperImpl->OnDevicesConnected(device, nullptrCallback);

    EXPECT_NE(result, SUCCESS);
}

/**
 * @tc.name  : Test VADeviceBrokerWrapperImpl.
 * @tc.number: VADeviceBrokerWrapperImpl_002
 * @tc.desc  : Test VADeviceBrokerWrapperImpl OnInterrupt interface.
 */
HWTEST_F(VADeviceBrokerWrapperImplTest, VADeviceBrokerWrapperImpl_002, TestSize.Level3)
{
    VADeviceBrokerWrapperImpl wrapperImpl;
    VADevice device;
    std::weak_ptr<VADeviceControllerCallback> vaDeviceControllerCallback_;
    std::shared_ptr<VADeviceControllerCallback> controllerCallback = vaDeviceControllerCallback_.lock();
    int32_t result = wrapperImpl.OnDevicesConnected(device, controllerCallback);

    EXPECT_NE(result, SUCCESS);
}

/**
 * @tc.name  : Test VADeviceBrokerWrapperImpl.
 * @tc.number: VADeviceBrokerWrapperImpl_005
 * @tc.desc  : Test VADeviceBrokerWrapperImpl OnDevicesDisconnected interface with null device.
 */
HWTEST_F(VADeviceBrokerWrapperImplTest, VADeviceBrokerWrapperImpl_005, TestSize.Level3)
{
    auto iAudioPolicy = VADeviceBrokerWrapperImpl::GetAudioPolicyProxyFromSamgr(false);
    EXPECT_NE(iAudioPolicy, nullptr);
}
} // namespace AudioStandard
} // namespace OHOS