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
#ifndef AUDIO_A2DP_DEVICE_UNIT_TEST_H
#define AUDIO_A2DP_DEVICE_UNIT_TEST_H
#include "gtest/gtest.h"
#include "audio_a2dp_device.h"
#include "audio_a2dp_device_behavior.h"
#include "audio_active_device.h"
#include "audio_iohandle_map.h"

namespace OHOS {
namespace AudioStandard {

class AudioA2dpDeviceUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp(void);
    void TearDown(void);

    AudioA2dpDeviceBehavior a2dpDeviceBehavior_;
    std::set<std::string> savedActiveBtMac_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif //AUDIO_A2DP_DEVICE_UNIT_TEST_H