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

#ifndef AUDIO_ZONE_UNIT_TEST_BASE_H
#define AUDIO_ZONE_UNIT_TEST_BASE_H

#include <condition_variable>
#include <mutex>
#include "gtest/gtest.h"
#include "audio_zone.h"
#include "audio_zone_client_manager.h"
#include "audio_policy_server_handler.h"
#include "audio_zone_service.h"
#include "audio_interrupt_service.h"
#include "audio_session_service.h"
#include "i_audio_zone_event_dispatcher.h"

namespace OHOS {
namespace AudioStandard {
class AudioZoneUnitTestBase : public testing::Test {
public:
    void SetUp(void) override
    {
        std::shared_ptr<AudioPolicyServerHandler> handler = std::make_shared<AudioPolicyServerHandler>();
        EXPECT_NE(handler, nullptr);
        std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
        EXPECT_NE(interruptService, nullptr);
        AudioZoneService::GetInstance().Init(handler, interruptService);
    }

    void TearDown(void) override
    {
        AudioZoneService::GetInstance().DeInit();
    }
};

class AudioZoneUnitTestPreset : public AudioZoneUnitTestBase {
public:
    void SetUp() override
    {
        AudioZoneUnitTestBase::SetUp();
        AudioZoneContext context;
        zoneId1_ = AudioZoneService::GetInstance().CreateAudioZone("TestZone1", context, 0);
        EXPECT_NE(zoneId1_, 0);
        zoneId2_ = AudioZoneService::GetInstance().CreateAudioZone("TestZone2", context, 0);
        EXPECT_NE(zoneId2_, 0);
    }

    void TearDown() override
    {
        zoneId1_ = 0;
        zoneId2_ = 0;
        AudioZoneUnitTestBase::TearDown();
    }

    int32_t zoneId1_ = 0;
    int32_t zoneId2_ = 0;
};

std::shared_ptr<AudioDeviceDescriptor> CreateDevice(DeviceType type, DeviceRole role,
    const std::string &macAddress, const std::string &networkId)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(type, role);
    EXPECT_NE(desc, nullptr);
    desc->macAddress_ = macAddress;
    desc->networkId_ = networkId;
    return desc;
}
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_ZONE_UNIT_TEST_BASE_H