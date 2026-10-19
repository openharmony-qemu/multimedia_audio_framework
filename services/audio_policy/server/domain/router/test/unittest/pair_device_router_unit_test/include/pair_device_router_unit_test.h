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

#ifndef PAIR_DEVICE_ROUTER_UNIT_TEST_H
#define PAIR_DEVICE_ROUTER_UNIT_TEST_H

#include <gtest/gtest.h>
#include "audio_bluetooth_manager.h"
#include "audio_device_info.h"
#include "audio_policy_service.h"
#include "audio_system_manager.h"
#include "pair_device_router.h"

namespace OHOS {
namespace AudioStandard {

class PairDeviceRouterUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp(void);
    void TearDown(void);

protected:
    static constexpr int32_t UID = 100;
    static constexpr uint32_t SESSION = 200;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> systemOutputs_;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> connectedDevices_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // PAIR_DEVICE_ROUTER_UNIT_TEST_H