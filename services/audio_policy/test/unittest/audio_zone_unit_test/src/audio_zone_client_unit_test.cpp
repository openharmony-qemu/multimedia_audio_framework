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

#include "audio_zone_client_unit_test.h"
#include "audio_errors.h"
#include "audio_policy_log.h"

#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioZoneClientUnitTest::SetUpTestCase(void) {}
void AudioZoneClientUnitTest::TearDownTestCase(void) {}
void AudioZoneClientUnitTest::SetUp(void)
{
    client_ = new(std::nothrow) AudioZoneClient();
}
void AudioZoneClientUnitTest::TearDown(void)
{
    client_ = nullptr;
}

/**
 * @tc.name  : Test AudioZoneClientUnitTest.
 * @tc.number: AudioZoneClient_001
 * @tc.desc  : Test OnRemoteRequest interface.
 */
HWTEST_F(AudioZoneClientUnitTest, AudioZoneClient_001, TestSize.Level1)
{
    EXPECT_EQ(client_->OnRemoteRequest(static_cast<uint32_t>(AudioZoneClientCode::ON_AUDIO_ZONE_ADD),
        data, reply, option), SUCCESS);
    EXPECT_EQ(client_->OnRemoteRequest(static_cast<uint32_t>(AudioZoneClientCode::ON_AUDIO_ZONE_REMOVE),
        data, reply, option), SUCCESS);
    EXPECT_EQ(client_->OnRemoteRequest(static_cast<uint32_t>(AudioZoneClientCode::ON_AUDIO_ZONE_CHANGE),
        data, reply, option), SUCCESS);
    EXPECT_EQ(client_->OnRemoteRequest(static_cast<uint32_t>(AudioZoneClientCode::ON_AUDIO_ZONE_INTERRUPT),
        data, reply, option), SUCCESS);
    EXPECT_EQ(client_->OnRemoteRequest(static_cast<uint32_t>(AudioZoneClientCode::ON_AUDIO_ZONE_DEVICE_INTERRUPT),
        data, reply, option), SUCCESS);
    EXPECT_EQ(client_->OnRemoteRequest(static_cast<uint32_t>(AudioZoneClientCode::ON_AUDIO_ZONE_SYSTEM_VOLUME_SET),
        data, reply, option), SUCCESS);
    EXPECT_EQ(client_->OnRemoteRequest(static_cast<uint32_t>(AudioZoneClientCode::ON_AUDIO_ZONE_SYSTEM_VOLUME_GET),
        data, reply, option), SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS