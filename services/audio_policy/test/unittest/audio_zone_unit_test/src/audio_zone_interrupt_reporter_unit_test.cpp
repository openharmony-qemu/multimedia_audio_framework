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

#include "audio_zone_interrupt_reporter_unit_test.h"
#include "audio_errors.h"
#include "audio_policy_log.h"
#include "audio_pipe_manager.h"

#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioZoneInterruptReporterUnitTest::SetUpTestCase(void) {}
void AudioZoneInterruptReporterUnitTest::TearDownTestCase(void) {}
void AudioZoneInterruptReporterUnitTest::SetUp(void) {}
void AudioZoneInterruptReporterUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test AudioZoneInterruptReporterUnitTest.
 * @tc.number: AudioZoneInterruptReporter_001
 * @tc.desc  : Test AddAudioInterruptCallback interface.
 */
HWTEST_F(AudioZoneInterruptReporterUnitTest, AudioZoneInterruptReporter_001, TestSize.Level1)
{
    EXPECT_EQ(reporter_.EnableInterruptReport(0, 0, "", true), SUCCESS);
    EXPECT_EQ(reporter_.EnableInterruptReport(0, 0, "", false), SUCCESS);
}

/**
 * @tc.name  : Test AudioZoneInterruptReporterUnitTest.
 * @tc.number: AudioZoneInterruptReporter_002
 * @tc.desc  : Test AddAudioInterruptCallback interface.
 */
HWTEST_F(AudioZoneInterruptReporterUnitTest, AudioZoneInterruptReporter_002, TestSize.Level1)
{
    EXPECT_EQ(reporter_.RegisterInterruptReport(0, 0, ""), SUCCESS);
    EXPECT_EQ(reporter_.RegisterInterruptReport(0, 0, ""), SUCCESS);
}

/**
 * @tc.name  : Test AudioZoneInterruptReporterUnitTest.
 * @tc.number: GetFocusList_001
 * @tc.desc  : Test GetFocusList interface.
 */
HWTEST_F(AudioZoneInterruptReporterUnitTest, GetFocusList_001, TestSize.Level1)
{
    auto interruptZone = std::make_shared<AudioInterruptZone>();
    AudioInterrupt a1;
    a1.streamId = 100;
    interruptZone->audioFocusInfoList.push_back({a1, AudioFocuState::ACTIVE});

    reporter_.interruptService_ = std::make_shared<AudioInterruptService>();
    reporter_.interruptService_->zonesMap_.clear();
    reporter_.interruptService_->zonesMap_[1] = interruptZone;

    std::list<std::pair<AudioInterrupt, AudioFocuState>> focusList = {};
    focusList = reporter_.GetFocusList();
    EXPECT_EQ(focusList.size(), 0);

    auto &pipeList = AudioPipeManager::GetPipeManager()->curPipeList_;
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs = {};
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> newDeviceDescs = {};
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    deviceDesc->networkId_ = "LocalDevice";
    newDeviceDescs.emplace_back(deviceDesc);
    streamDesc->sessionId_ = 100;
    streamDesc->newDeviceDescs_ = newDeviceDescs;
    streamDescs.emplace_back(streamDesc);
    pipeInfo->streamDescriptors_ = streamDescs;
    pipeList.emplace_back(pipeInfo);

    focusList = reporter_.GetFocusList();
    pipeList.pop_back();
    EXPECT_EQ(focusList.size(), 0);
}

/**
 * @tc.name  : Test AudioZoneInterruptReporterUnitTest.
 * @tc.number: GetFocusList_002
 * @tc.desc  : Test GetFocusList interface.
 */
HWTEST_F(AudioZoneInterruptReporterUnitTest, GetFocusList_002, TestSize.Level1)
{
    auto interruptZone = std::make_shared<AudioInterruptZone>();
    AudioInterrupt a1;
    a1.streamId = 100;
    interruptZone->audioFocusInfoList.push_back({a1, AudioFocuState::ACTIVE});

    reporter_.interruptService_ = std::make_shared<AudioInterruptService>();
    reporter_.interruptService_->zonesMap_.clear();
    reporter_.interruptService_->zonesMap_[1] = interruptZone;

    std::list<std::pair<AudioInterrupt, AudioFocuState>> focusList = {};
    focusList = reporter_.GetFocusList();
    EXPECT_EQ(focusList.size(), 0);

    auto &pipeList = AudioPipeManager::GetPipeManager()->curPipeList_;
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs = {};
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> newDeviceDescs = {};
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    deviceDesc->networkId_ = "RemoteDevice";
    newDeviceDescs.emplace_back(deviceDesc);
    streamDesc->sessionId_ = 100;
    streamDesc->newDeviceDescs_ = newDeviceDescs;
    streamDescs.emplace_back(streamDesc);
    pipeInfo->streamDescriptors_ = streamDescs;
    pipeList.emplace_back(pipeInfo);

    focusList = reporter_.GetFocusList();
    pipeList.pop_back();
    EXPECT_EQ(focusList.size(), 0);
}
} // namespace AudioStandard
} // namespace OHOS
 