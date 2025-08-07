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

#include "audio_zone_client_manager_unit_test.h"
#include "audio_errors.h"
#include "audio_policy_log.h"

#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioZoneClientManagerUnitTest::SetUpTestCase(void) {}
void AudioZoneClientManagerUnitTest::TearDownTestCase(void) {}
void AudioZoneClientManagerUnitTest::SetUp(void) {}
void AudioZoneClientManagerUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test AudioZoneClientManagerUnitTest.
 * @tc.number: AudioZoneClientManager_001
 * @tc.desc  : Test AddAudioInterruptCallback interface.
 */
HWTEST_F(AudioZoneClientManagerUnitTest, AudioZoneClientManager_001, TestSize.Level1)
{
    std::shared_ptr<AudioZoneEvent> event = std::make_shared<AudioZoneEvent>();
    event->type = AudioZoneEventType::AUDIO_ZONE_ADD_EVENT;
    AudioZoneClientManager::GetInstance().DispatchEvent(event);
    event->type = AudioZoneEventType::AUDIO_ZONE_REMOVE_EVENT;
    AudioZoneClientManager::GetInstance().DispatchEvent(event);
    event->type = AudioZoneEventType::AUDIO_ZONE_CHANGE_EVENT;
    AudioZoneClientManager::GetInstance().DispatchEvent(event);
    event->type = AudioZoneEventType::AUDIO_ZONE_INTERRUPT_EVENT;
    AudioZoneClientManager::GetInstance().DispatchEvent(event);
    event->deviceTag = "1";
    event->type = AudioZoneEventType::AUDIO_ZONE_INTERRUPT_EVENT;
    AudioZoneClientManager::GetInstance().DispatchEvent(event);
    event->descriptor = std::make_shared<AudioZoneDescriptor>();
    AudioZoneClientManager::GetInstance().DispatchEvent(event);
    EXPECT_NE(event, nullptr);
}

/**
 * @tc.name  : Test UnRegisterAudioZoneClientUnitTest.
 * @tc.number: UnRegisterAudioZoneClient_001
 * @tc.desc  : Test UnRegisterAudioZoneClient interface.
 */
HWTEST_F(AudioZoneClientManagerUnitTest, UnRegisterAudioZoneClient_001, TestSize.Level1)
{
    pid_t clientPid = 1000;
    AudioZoneClientManager::GetInstance().UnRegisterAudioZoneClient(clientPid);
    EXPECT_TRUE(AudioZoneClientManager::GetInstance().clients_.find(clientPid) ==
        AudioZoneClientManager::GetInstance().clients_.end());
}
} // namespace AudioStandard
} // namespace OHOS
 