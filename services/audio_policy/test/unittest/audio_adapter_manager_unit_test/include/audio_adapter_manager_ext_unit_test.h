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

#ifndef AUDIO_ADAPTER_MANAGER_EXT_UNIT_TEST_H
#define AUDIO_ADAPTER_MANAGER_EXT_UNIT_TEST_H

#include "gtest/gtest.h"
#include "audio_adapter_manager.h"
#include "audio_interrupt_service.h"
#include "audio_policy_server_handler.h"
#include "audio_zone_service.h"
#include "audio_volume.h"

namespace OHOS {
namespace AudioStandard {

class AudioAdapterManagerExtUnitTest : public testing::Test {
public:
    // SetUpTestCase: Called before all test cases
    static void SetUpTestCase(void);
    // TearDownTestCase: Called after all test case
    static void TearDownTestCase(void);

    void SetUp() override
    {
        std::shared_ptr<AudioPolicyServerHandler> handler = std::make_shared<AudioPolicyServerHandler>();
        std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
        AudioZoneService::GetInstance().Init(handler, interruptService);
        AudioZoneContext context;
        zoneId1_ = AudioZoneService::GetInstance().CreateAudioZone("TestZone1", context, 0);
        zoneId2_ = AudioZoneService::GetInstance().CreateAudioZone("TestZone2", context, 0);
    }

    void TearDown() override
    {
        zoneId1_ = 0;
        zoneId2_ = 0;
        AudioZoneService::GetInstance().DeInit();
    }

    int32_t zoneId1_ = 0;
    int32_t zoneId2_ = 0;
};
} // namespace AudioStandard
} // namespace OHOS
#endif //AUDIO_ADAPTER_MANAGER_EXT_UNIT_TEST_H