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
void AudioZoneClientManagerUnitTest::SetUp(void)
{
    AudioZoneClientManager::GetInstance().clients_.clear();
}
void AudioZoneClientManagerUnitTest::TearDown(void)
{
    AudioZoneClientManager::GetInstance().clients_.clear();
}

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

/**
 * @tc.name  : Test AudioZoneClientManagerUnitTest.
 * @tc.number: AudioZoneClientManager_002
 * @tc.desc  : Test RegisterAudioZoneClient interface.
 */
HWTEST_F(AudioZoneClientManagerUnitTest, AudioZoneClientManager_002, TestSize.Level4)
{
    AudioZoneClientManager manager(nullptr);

    pid_t pid = 101;
    int32_t ret = manager.RegisterAudioZoneClient(pid, nullptr);

    EXPECT_EQ(ret, ERROR);
    EXPECT_FALSE(manager.IsRegisterAudioZoneClient(pid));
}

/**
 * @tc.name  : Test AudioZoneClientManagerUnitTest.
 * @tc.number: AudioZoneClientManager_003
 * @tc.desc  : Test RegisterAudioZoneClient interface.
 */
HWTEST_F(AudioZoneClientManagerUnitTest, AudioZoneClientManager_003, TestSize.Level4)
{
    pid_t pid = 102;
    sptr<IStandardAudioZoneClient> client = new IStandardAudioZoneClientUnitTest();
    ASSERT_NE(client, nullptr);

    int32_t ret = AudioZoneClientManager::GetInstance().RegisterAudioZoneClient(pid, client);
    EXPECT_EQ(ret, SUCCESS);

    auto& mapRef = AudioZoneClientManager::GetInstance().clients_;
    auto it = mapRef.find(pid);
    EXPECT_TRUE(it != mapRef.end());
    EXPECT_EQ(it->second, client);
}

/**
 * @tc.name  : Test AudioZoneClientManagerUnitTest.
 * @tc.number: AudioZoneClientManager_004
 * @tc.desc  : Test RegisterAudioZoneClient interface.
 */
HWTEST_F(AudioZoneClientManagerUnitTest, AudioZoneClientManager_004, TestSize.Level4)
{
    pid_t pid = 103;
    sptr<IStandardAudioZoneClient> client1 = new IStandardAudioZoneClientUnitTest();
    sptr<IStandardAudioZoneClient> client2 = new IStandardAudioZoneClientUnitTest();
    ASSERT_NE(client1, nullptr);
    ASSERT_NE(client2, nullptr);

    EXPECT_EQ(AudioZoneClientManager::GetInstance().RegisterAudioZoneClient(pid, client1), SUCCESS);
    EXPECT_EQ(AudioZoneClientManager::GetInstance().RegisterAudioZoneClient(pid, client2), SUCCESS);

    auto& mapRef = AudioZoneClientManager::GetInstance().clients_;
    EXPECT_EQ(mapRef[pid], client2);
}

/**
 * @tc.name  : Test AudioZoneClientManagerUnitTest.
 * @tc.number: AudioZoneClientManager_005
 * @tc.desc  : Test UnRegisterAudioZoneClient interface.
 */
HWTEST_F(AudioZoneClientManagerUnitTest, AudioZoneClientManager_005, TestSize.Level4)
{
    pid_t pid = 101;
    auto& mapRef = AudioZoneClientManager::GetInstance().clients_;
    EXPECT_TRUE(mapRef.find(pid) == mapRef.end());

    AudioZoneClientManager::GetInstance().UnRegisterAudioZoneClient(pid);
    EXPECT_TRUE(mapRef.empty());
}

/**
 * @tc.name  : Test AudioZoneClientManagerUnitTest.
 * @tc.number: AudioZoneClientManager_006
 * @tc.desc  : Test UnRegisterAudioZoneClient interface.
 */
HWTEST_F(AudioZoneClientManagerUnitTest, AudioZoneClientManager_006, TestSize.Level4)
{
    pid_t pid = 102;
    sptr<IStandardAudioZoneClient> client = new IStandardAudioZoneClientUnitTest();
    ASSERT_NE(client, nullptr);

    auto& mapRef = AudioZoneClientManager::GetInstance().clients_;
    mapRef[pid] = client;
    EXPECT_TRUE(mapRef.find(pid) != mapRef.end());

    AudioZoneClientManager::GetInstance().UnRegisterAudioZoneClient(pid);
    EXPECT_TRUE(mapRef.find(pid) == mapRef.end());
}

/**
 * @tc.name  : Test AudioZoneClientManagerUnitTest.
 * @tc.number: AudioZoneClientManager_007
 * @tc.desc  : Test DispatchEvent interface.
 */
HWTEST_F(AudioZoneClientManagerUnitTest, AudioZoneClientManager_007, TestSize.Level4)
{
    auto& manager = AudioZoneClientManager::GetInstance();

    pid_t pid = 101;
    sptr<IStandardAudioZoneClient> client = new IStandardAudioZoneClientUnitTest();
    ASSERT_NE(client, nullptr);
    manager.clients_[pid] = client;

    auto event = std::make_shared<AudioZoneEvent>();
    event->type = AudioZoneEventType::AUDIO_ZONE_ADD_EVENT;
    event->clientPid = pid;
    event->descriptor = std::make_shared<AudioZoneDescriptor>();
    ASSERT_NE(event->descriptor, nullptr);
    event->descriptor->zoneId_ = 201;

    manager.DispatchEvent(event);

    auto testClient = static_cast<IStandardAudioZoneClientUnitTest*>(client.GetRefPtr());
    testClient->Wait();
    EXPECT_EQ(testClient->recvEvent_.type, AudioZoneEventType::AUDIO_ZONE_ADD_EVENT);
}

/**
 * @tc.name  : Test AudioZoneClientManagerUnitTest.
 * @tc.number: AudioZoneClientManager_008
 * @tc.desc  : Test DispatchEvent interface.
 */
HWTEST_F(AudioZoneClientManagerUnitTest, AudioZoneClientManager_008, TestSize.Level4)
{
    auto& manager = AudioZoneClientManager::GetInstance();

    pid_t pid = 102;
    sptr<IStandardAudioZoneClient> client = new IStandardAudioZoneClientUnitTest();
    ASSERT_NE(client, nullptr);
    manager.clients_[pid] = client;

    auto event = std::make_shared<AudioZoneEvent>();
    event->type = AudioZoneEventType::AUDIO_ZONE_REMOVE_EVENT;
    event->clientPid = pid;
    event->zoneId = 202;

    manager.DispatchEvent(event);

    auto testClient = static_cast<IStandardAudioZoneClientUnitTest*>(client.GetRefPtr());
    testClient->Wait();
    EXPECT_EQ(testClient->recvEvent_.type, AudioZoneEventType::AUDIO_ZONE_REMOVE_EVENT);
    EXPECT_EQ(testClient->recvEvent_.zoneId, 202);
}

/**
 * @tc.name  : Test AudioZoneClientManagerUnitTest.
 * @tc.number: AudioZoneClientManager_009
 * @tc.desc  : Test DispatchEvent interface.
 */
HWTEST_F(AudioZoneClientManagerUnitTest, AudioZoneClientManager_009, TestSize.Level4)
{
    auto& manager = AudioZoneClientManager::GetInstance();

    pid_t pid = 103;
    sptr<IStandardAudioZoneClient> client = new IStandardAudioZoneClientUnitTest();
    ASSERT_NE(client, nullptr);
    manager.clients_[pid] = client;

    auto event = std::make_shared<AudioZoneEvent>();
    event->type = AudioZoneEventType::AUDIO_ZONE_CHANGE_EVENT;
    event->clientPid = pid;
    event->zoneId = 203;
    event->descriptor = std::make_shared<AudioZoneDescriptor>();
    ASSERT_NE(event->descriptor, nullptr);
    event->descriptor->zoneId_ = 203;
    event->zoneChangeReason = static_cast<AudioZoneChangeReason>(3);

    manager.DispatchEvent(event);

    auto testClient = static_cast<IStandardAudioZoneClientUnitTest*>(client.GetRefPtr());
    testClient->Wait();
    EXPECT_EQ(testClient->recvEvent_.type, AudioZoneEventType::AUDIO_ZONE_CHANGE_EVENT);
    EXPECT_EQ(testClient->recvEvent_.zoneId, 203);
}

/**
 * @tc.name  : Test AudioZoneClientManagerUnitTest.
 * @tc.number: AudioZoneClientManager_010
 * @tc.desc  : Test DispatchEvent interface.
 */
HWTEST_F(AudioZoneClientManagerUnitTest, AudioZoneClientManager_010, TestSize.Level4)
{
    auto& manager = AudioZoneClientManager::GetInstance();

    pid_t pid = 104;
    sptr<IStandardAudioZoneClient> client = new IStandardAudioZoneClientUnitTest();
    ASSERT_NE(client, nullptr);
    manager.clients_[pid] = client;

    auto event = std::make_shared<AudioZoneEvent>();
    event->type = AudioZoneEventType::AUDIO_ZONE_INTERRUPT_EVENT;
    event->clientPid = pid;
    event->zoneId = 204;
    event->deviceTag = "";
    event->zoneInterruptReason = static_cast<AudioZoneInterruptReason>(2);
    event->interrupts.clear();

    manager.DispatchEvent(event);

    auto testClient = static_cast<IStandardAudioZoneClientUnitTest*>(client.GetRefPtr());
    testClient->Wait();
    EXPECT_EQ(testClient->recvEvent_.type, AudioZoneEventType::AUDIO_ZONE_INTERRUPT_EVENT);
    EXPECT_EQ(testClient->recvEvent_.zoneId, 204);
    EXPECT_TRUE(testClient->recvEvent_.deviceTag.empty());
}

/**
 * @tc.name  : Test AudioZoneClientManagerUnitTest.
 * @tc.number: AudioZoneClientManager_011
 * @tc.desc  : Test DispatchEvent interface.
 */
HWTEST_F(AudioZoneClientManagerUnitTest, AudioZoneClientManager_011, TestSize.Level4)
{
    auto& manager = AudioZoneClientManager::GetInstance();

    pid_t pid = 105;
    sptr<IStandardAudioZoneClient> client = new IStandardAudioZoneClientUnitTest();
    ASSERT_NE(client, nullptr);
    manager.clients_[pid] = client;

    auto event = std::make_shared<AudioZoneEvent>();
    event->type = AudioZoneEventType::AUDIO_ZONE_INTERRUPT_EVENT;
    event->clientPid = pid;
    event->zoneId = 205;
    event->deviceTag = "BT_SPK";
    event->zoneInterruptReason = static_cast<AudioZoneInterruptReason>(5);
    event->interrupts.clear();

    manager.DispatchEvent(event);

    auto testClient = static_cast<IStandardAudioZoneClientUnitTest*>(client.GetRefPtr());
    testClient->Wait();
    EXPECT_EQ(testClient->recvEvent_.type, AudioZoneEventType::AUDIO_ZONE_INTERRUPT_EVENT);
    EXPECT_EQ(testClient->recvEvent_.zoneId, 205);
    EXPECT_EQ(testClient->recvEvent_.deviceTag, "BT_SPK");
}

/**
 * @tc.name  : Test AudioZoneClientManagerUnitTest.
 * @tc.number: AudioZoneClientManager_012
 * @tc.desc  : Test DispatchEvent interface.
 */
HWTEST_F(AudioZoneClientManagerUnitTest, AudioZoneClientManager_012, TestSize.Level4)
{
    auto& manager = AudioZoneClientManager::GetInstance();

    pid_t pid = 106;
    sptr<IStandardAudioZoneClient> client = new IStandardAudioZoneClientUnitTest();
    ASSERT_NE(client, nullptr);
    manager.clients_[pid] = client;

    auto event = std::make_shared<AudioZoneEvent>();
    event->type = static_cast<AudioZoneEventType>(999);
    event->clientPid = pid;
    event->zoneId = 206;

    auto testClient = static_cast<IStandardAudioZoneClientUnitTest*>(client.GetRefPtr());
    testClient->recvEvent_.type = AudioZoneEventType::AUDIO_ZONE_REMOVE_EVENT;
    testClient->waitStatus_ = 0;

    manager.DispatchEvent(event);

    EXPECT_EQ(testClient->recvEvent_.type, AudioZoneEventType::AUDIO_ZONE_REMOVE_EVENT);
    EXPECT_EQ(testClient->waitStatus_, 0);
}

 /**
 * @tc.name  : Test AudioZoneClientManagerUnitTest.
 * @tc.number: AudioZoneClientManager_013
 * @tc.desc  : Cover Set/Get SystemVolumeDegree and SetSystemVolumeLevel branches:
 *             1. calls without registered client -> expect non-zero/error
 *             2. calls with registered client -> expect success and values routed to client
 */
HWTEST_F(AudioZoneClientManagerUnitTest, AudioZoneClientManager_013, TestSize.Level4)
{
    auto& manager = AudioZoneClientManager::GetInstance();

    // ensure clean state
    manager.clients_.clear();

    AudioVolumeType volumeType = STREAM_MUSIC;
    int32_t volumeDegree = 11;
    // construct VolumeScale(level, degree) — use same numeric values for test
    VolumeScale volume{volumeDegree, volumeDegree};
    int32_t zoneId = 0;

    // device parameter was missing — use a concrete device for tests
    DeviceType device = DEVICE_TYPE_SPEAKER;

    pid_t missingPid = 9999;
    // Case A: no client registered -> operations should not succeed (expect non-zero)
    EXPECT_NE(manager.GetSystemVolumeDegree(missingPid, zoneId, device, volumeType), 0);
    EXPECT_NE(manager.SetSystemVolumeLevel(missingPid, zoneId, device, volumeType, volume, 0), 0);
    EXPECT_NE(manager.GetSystemVolumeLevel(missingPid, zoneId, device, volumeType), 0);

    // Case B: register client and verify calls reach it
    pid_t pid = 300;
    sptr<IStandardAudioZoneClient> client = new IStandardAudioZoneClientUnitTest();
    ASSERT_NE(client, nullptr);
    {
        std::lock_guard<std::mutex> lock(manager.clientMutex_);
        manager.clients_[pid] = client;
    }

    // call SetSystemVolumeLevel -> should succeed and client notified
    EXPECT_EQ(manager.SetSystemVolumeLevel(pid, zoneId, device, volumeType, volume, 0), 0);

    auto testClient = static_cast<IStandardAudioZoneClientUnitTest*>(client.GetRefPtr());
    testClient->Wait();
    EXPECT_EQ(testClient->volumeLevel_, volume.VolumeLevel());
    EXPECT_EQ(testClient->volumeDegree_, volume.VolumeDegree());

    // GetSystemVolumeDegree should return the degree set in stub
    EXPECT_EQ(manager.GetSystemVolumeDegree(pid, zoneId, device, volumeType), volumeDegree);

    // cleanup
    {
        std::lock_guard<std::mutex> lock(manager.clientMutex_);
        manager.clients_.erase(pid);
    }
}
} // namespace AudioStandard
} // namespace OHOS
 