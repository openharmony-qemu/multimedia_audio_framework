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

#include "audio_policy_manager_zone_stub_unit_test.h"
#include "audio_errors.h"
#include "audio_policy_log.h"
#include "audio_zone_service.h"
#include "audio_policy_server.h"

#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioPolicyManagerZoneStubUnitTest::SetUpTestCase(void) {}
void AudioPolicyManagerZoneStubUnitTest::TearDownTestCase(void) {}
void AudioPolicyManagerZoneStubUnitTest::SetUp(void)
{
    stub_ = std::make_shared<AudioPolicyServer>(1);
}
void AudioPolicyManagerZoneStubUnitTest::TearDown(void)
{
    stub_ = nullptr;
}

/**
 * @tc.name  : Test AudioPolicyManagerZoneStubUnitTest.
 * @tc.number: AudioPolicyManagerZoneStub_001
 * @tc.desc  : Test OnAudioZoneRemoteRequest interface.
 */
HWTEST_F(AudioPolicyManagerZoneStubUnitTest, AudioPolicyManagerZoneStub_001, TestSize.Level1)
{
    stub_->OnAudioZoneRemoteRequest(static_cast<uint32_t>(
        AudioPolicyInterfaceCode::REGISTER_AUDIO_ZONE_CLIENT),
        data, reply, option);
    stub_->OnAudioZoneRemoteRequest(static_cast<uint32_t>(
        AudioPolicyInterfaceCode::CREATE_AUDIO_ZONE),
        data, reply, option);
    stub_->OnAudioZoneRemoteRequest(static_cast<uint32_t>(
        AudioPolicyInterfaceCode::RELEASE_AUDIO_ZONE),
        data, reply, option);
    stub_->OnAudioZoneRemoteRequest(static_cast<uint32_t>(
        AudioPolicyInterfaceCode::GET_ALL_AUDIO_ZONE),
        data, reply, option);
    stub_->OnAudioZoneRemoteRequest(static_cast<uint32_t>(
        AudioPolicyInterfaceCode::GET_AUDIO_ZONE_BY_ID),
        data, reply, option);
    stub_->OnAudioZoneRemoteRequest(static_cast<uint32_t>(
        AudioPolicyInterfaceCode::BIND_AUDIO_ZONE_DEVICE),
        data, reply, option);
    stub_->OnAudioZoneRemoteRequest(static_cast<uint32_t>(
        AudioPolicyInterfaceCode::UNBIND_AUDIO_ZONE_DEVICE),
        data, reply, option);
    stub_->OnAudioZoneRemoteRequest(static_cast<uint32_t>(
        AudioPolicyInterfaceCode::ENABLE_AUDIO_ZONE_REPORT),
        data, reply, option);
    stub_->OnAudioZoneRemoteRequest(static_cast<uint32_t>(
        AudioPolicyInterfaceCode::ENABLE_AUDIO_ZONE_CHANGE_REPORT),
        data, reply, option);
    stub_->OnAudioZoneRemoteRequest(static_cast<uint32_t>(
        AudioPolicyInterfaceCode::ADD_UID_TO_AUDIO_ZONE),
        data, reply, option);
    stub_->OnAudioZoneRemoteRequest(static_cast<uint32_t>(
        AudioPolicyInterfaceCode::REMOVE_UID_FROM_AUDIO_ZONE),
        data, reply, option);
    stub_->OnAudioZoneRemoteRequest(static_cast<uint32_t>(
        AudioPolicyInterfaceCode::ENABLE_SYSTEM_VOLUME_PROXY),
        data, reply, option);
    stub_->OnAudioZoneRemoteRequest(static_cast<uint32_t>(
        AudioPolicyInterfaceCode::GET_AUDIO_INTERRUPT_FOR_ZONE),
        data, reply, option);
    EXPECT_NE(stub_, nullptr);
}

/**
 * @tc.name  : Test AudioPolicyManagerZoneStubUnitTest.
 * @tc.number: AudioPolicyManagerZoneStub_002
 * @tc.desc  : Test OnAudioZoneRemoteRequestExt interface.
 */
HWTEST_F(AudioPolicyManagerZoneStubUnitTest, AudioPolicyManagerZoneStub_002, TestSize.Level1)
{
    stub_->OnAudioZoneRemoteRequestExt(static_cast<uint32_t>(
        AudioPolicyInterfaceCode::GET_AUDIO_INTERRUPT_OF_DEVICE_FOR_ZONE),
        data, reply, option);
    stub_->OnAudioZoneRemoteRequestExt(static_cast<uint32_t>(
        AudioPolicyInterfaceCode::ENABLE_AUDIO_ZONE_INTERRUPT_REPORT),
        data, reply, option);
    stub_->OnAudioZoneRemoteRequestExt(static_cast<uint32_t>(
        AudioPolicyInterfaceCode::INJECT_INTERRUPT_TO_AUDIO_ZONE),
        data, reply, option);
    stub_->OnAudioZoneRemoteRequestExt(static_cast<uint32_t>(
        AudioPolicyInterfaceCode::INJECT_INTERRUPT_OF_DEVICE_TO_AUDIO_ZONE),
        data, reply, option);
    EXPECT_NE(stub_, nullptr);
}

/**
 * @tc.name  : Test AudioPolicyManagerZoneStubUnitTest.
 * @tc.number: AudioPolicyManagerZoneStub_003
 * @tc.desc  : Test HandleGetAudioZone interface.
 */
HWTEST_F(AudioPolicyManagerZoneStubUnitTest, AudioPolicyManagerZoneStub_003, TestSize.Level1)
{
    MessageParcel data1;
    MessageParcel data2;
    data1.WriteInt32(0);
    stub_->HandleGetAudioZone(data1, reply);
    AudioZoneContext context;
    int32_t zoneId = AudioZoneService::GetInstance().CreateAudioZone("zone1", context);
    data2.WriteInt32(zoneId);
    stub_->HandleGetAudioZone(data2, reply);
    EXPECT_NE(stub_, nullptr);
}
} // namespace AudioStandard
} // namespace OHOS
 