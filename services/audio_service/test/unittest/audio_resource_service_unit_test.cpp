/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include "audio_resource_service.h"
#include "audio_workgroup.h"
#include "audio_errors.h"
#include "iipc_stream.h"
#include "message_parcel.h"
#include "parcel.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {
class AudioResourceServiceUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioResourceServiceUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void AudioResourceServiceUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void AudioResourceServiceUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioResourceServiceUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

AudioResourceService::AudioWorkgroupDeathRecipient deathRecipient;
AudioResourceService audioResourceService;
const int32_t testRtgId = 2;
static constexpr int32_t AUDIO_MAX_PROCESS = 2;
static constexpr int32_t AUDIO_MAX_GRP_PER_PROCESS = 4;

class RemoteObjectTestStub : public IRemoteObject {
public:
    RemoteObjectTestStub() : IRemoteObject(u"IRemoteObject") {}
    int32_t GetObjectRefCount() { return 0; };
    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) { return 0; };
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; };
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; };
    int Dump(int fd, const std::vector<std::u16string> &args) { return 0; };
    DECLARE_INTERFACE_DESCRIPTOR(u"RemoteObjectTestStub");
};

class DummyAudioWorkgroup : public AudioWorkgroup {
public:
    explicit DummyAudioWorkgroup(int32_t groupId) : AudioWorkgroup(groupId) {}
};

/**
 * @tc.name  : Test deathRecipient
 * @tc.type  : FUNC
 * @tc.number: DeathRecipient_001
 * @tc.desc  : Test OnRemoteDied when param is nullptr
 */
HWTEST(AudioResourceServiceUnitTest, DeathRecipient_001, TestSize.Level0)
{
    std::function<void()> diedCb = []() {
    };

    deathRecipient.diedCb_ = diedCb;
    deathRecipient.OnRemoteDied(nullptr);
    EXPECT_TRUE(deathRecipient.diedCb_ != nullptr);
}

/**
 * @tc.name  : Test deathRecipient
 * @tc.type  : FUNC
 * @tc.number: DeathRecipient_002
 * @tc.desc  : Test OnRemoteDied SetNotifyCb called
 */
HWTEST(AudioResourceServiceUnitTest, DeathRecipient_002, TestSize.Level0)
{
    std::function<void()> func = []() {
    };

    deathRecipient.diedCb_ = func;
    deathRecipient.SetNotifyCb(func);
    EXPECT_TRUE(deathRecipient.diedCb_);
}

/**
 * @tc.name  : Test deathRecipient
 * @tc.type  : FUNC
 * @tc.number: DeathRecipient_003
 * @tc.desc  : Test OnWorkgroupRemoteDied when called
 */
HWTEST(AudioResourceServiceUnitTest, DeathRecipient_003, TestSize.Level0)
{
    std::shared_ptr<AudioWorkgroup> workgroup = std::make_shared<AudioWorkgroup>(testRtgId);
    sptr<IRemoteObject> remoteObj = nullptr;

    audioResourceService.audioWorkgroupMap_[1].groups[testRtgId] = {workgroup};
    audioResourceService.OnWorkgroupRemoteDied(workgroup, remoteObj);
    EXPECT_EQ(audioResourceService.audioWorkgroupMap_[1].groups.size(), 0);
    EXPECT_EQ(audioResourceService.audioWorkgroupMap_[1].groups[testRtgId], nullptr);
}

/**
 * @tc.name  : Test deathRecipient
 * @tc.type  : FUNC
 * @tc.number: DeathRecipient_004
 * @tc.desc  : Test OnWorkgroupRemoteDied when audioWorkgroupMap_ zero
 */
HWTEST(AudioResourceServiceUnitTest, DeathRecipient_004, TestSize.Level0)
{
    std::shared_ptr<AudioWorkgroup> workgroup = std::make_shared<AudioWorkgroup>(testRtgId);
    sptr<IRemoteObject> remoteObj = nullptr;

    audioResourceService.audioWorkgroupMap_[1].groups[testRtgId] = {workgroup};
    audioResourceService.OnWorkgroupRemoteDied(workgroup, remoteObj);
    EXPECT_EQ(audioResourceService.audioWorkgroupMap_.count(1), 0);
}

/**
 * @tc.name  : Test deathRecipient
 * @tc.type  : FUNC
 * @tc.number: DeathRecipient_005
 * @tc.desc  : Test OnWorkgroupRemoteDied when audioWorkgroupMap_ empty
 */
HWTEST(AudioResourceServiceUnitTest, DeathRecipient_005, TestSize.Level0)
{
    std::shared_ptr<AudioWorkgroup> workgroup = std::make_shared<AudioWorkgroup>(testRtgId);
    sptr<IRemoteObject> remoteObj = nullptr;

    audioResourceService.OnWorkgroupRemoteDied(workgroup, remoteObj);
    EXPECT_TRUE(audioResourceService.audioWorkgroupMap_.empty());
}

/**
 * @tc.name  : Test deathRecipient OnWorkgroupRemoteDied
 * @tc.type  : FUNC
 * @tc.number: DeathRecipient_006
 * @tc.desc  : Test OnWorkgroupRemoteDied find audioWorkgroupMap_
 */
HWTEST(AudioResourceServiceUnitTest, DeathRecipient_006, TestSize.Level0)
{
    std::shared_ptr<AudioWorkgroup> workgroup = std::make_shared<AudioWorkgroup>(testRtgId);
    sptr<IRemoteObject> remoteObj = new RemoteObjectTestStub();

    audioResourceService.ReleaseWorkgroupDeathRecipient(workgroup, remoteObj);
    EXPECT_EQ(audioResourceService.deathRecipientMap_.find(workgroup), audioResourceService.deathRecipientMap_.end());
}

/**
 * @tc.name  : Test deathRecipient
 * @tc.type  : FUNC
 * @tc.number: DeathRecipient_007
 * @tc.desc  : Test ReleaseWorkgroupDeathRecipient when different remote object
 */
HWTEST(AudioResourceServiceUnitTest, DeathRecipient_007, TestSize.Level0)
{
    std::shared_ptr<AudioWorkgroup> workgroup = std::make_shared<AudioWorkgroup>(testRtgId);
    sptr<IRemoteObject> remoteObj1 = new RemoteObjectTestStub();
    sptr<IRemoteObject> remoteObj2 = new RemoteObjectTestStub();

    audioResourceService.deathRecipientMap_[workgroup] = std::make_pair(remoteObj1, nullptr);
    audioResourceService.ReleaseWorkgroupDeathRecipient(workgroup, remoteObj2);
    EXPECT_EQ(audioResourceService.deathRecipientMap_[workgroup].first, remoteObj1);
}

/**
 * @tc.name  : Test deathRecipient
 * @tc.type  : FUNC
 * @tc.number: DeathRecipient_008
 * @tc.desc  : Test ReleaseWorkgroupDeathRecipient when find workgroup
 */
HWTEST(AudioResourceServiceUnitTest, DeathRecipient_008, TestSize.Level0)
{
    std::shared_ptr<AudioWorkgroup> workgroup = std::make_shared<AudioWorkgroup>(testRtgId);
    sptr<IRemoteObject> remoteObj = new RemoteObjectTestStub();

    audioResourceService.deathRecipientMap_[workgroup] = std::make_pair(remoteObj, nullptr);
    audioResourceService.ReleaseWorkgroupDeathRecipient(workgroup, remoteObj);
    EXPECT_EQ(audioResourceService.deathRecipientMap_.find(workgroup), audioResourceService.deathRecipientMap_.end());
}

/**
 * @tc.name  : Test ImproveAudioWorkgroupPrio
 * @tc.type  : FUNC
 * @tc.number: ImproveAudioWorkgroupPrio_001
 * @tc.desc  : Test ImproveAudioWorkgroupPrio when threads map is not empty
 */
HWTEST(AudioResourceServiceUnitTest, ImproveAudioWorkgroupPrio_001, TestSize.Level0)
{
    AudioResourceService audioResourceService;
    pid_t pid = 1234;
    std::unordered_map<int32_t, bool> threads = {{1, true}, {2, false}};
    int32_t result = audioResourceService.ImproveAudioWorkgroupPrio(pid, threads);
    EXPECT_EQ(result, AUDIO_OK);
}
 
/**
 * @tc.name  : Test ImproveAudioWorkgroupPrio
 * @tc.type  : FUNC
 * @tc.number: ImproveAudioWorkgroupPrio_002
 * @tc.desc  : Test ImproveAudioWorkgroupPrio when threads map is empty
 */
HWTEST(AudioResourceServiceUnitTest, ImproveAudioWorkgroupPrio_002, TestSize.Level0)
{
    AudioResourceService audioResourceService;
    pid_t pid = 1234;
    std::unordered_map<int32_t, bool> threads = {};
    int32_t result = audioResourceService.ImproveAudioWorkgroupPrio(pid, threads);
    EXPECT_EQ(result, AUDIO_OK);
}
 
/**
 * @tc.name  : Test RestoreAudioWorkgroupPrio
 * @tc.type  : FUNC
 * @tc.number: RestoreAudioWorkgroupPrio_001
 * @tc.desc  : Test RestoreAudioWorkgroupPrio when threads map is empty
 */
HWTEST(AudioResourceServiceUnitTest, RestoreAudioWorkgroupPrio_001, TestSize.Level0)
{
    AudioResourceService audioResourceService;
    pid_t pid = 1234;
    std::unordered_map<int32_t, int32_t> threads = {{1, 2}, {3, 4}};
    int32_t result = audioResourceService.RestoreAudioWorkgroupPrio(pid, threads);
    EXPECT_EQ(result, AUDIO_OK);
}
 
/**
 * @tc.name  : Test RestoreAudioWorkgroupPrio
 * @tc.type  : FUNC
 * @tc.number: RestoreAudioWorkgroupPrio_002
 * @tc.desc  : Test RestoreAudioWorkgroupPrio when threads map is empty
 */
HWTEST(AudioResourceServiceUnitTest, RestoreAudioWorkgroupPrio_002, TestSize.Level0)
{
    AudioResourceService audioResourceService;
    pid_t pid = 1234;
    std::unordered_map<int32_t, int32_t> threads = {};
    int32_t result = audioResourceService.RestoreAudioWorkgroupPrio(pid, threads);
    EXPECT_EQ(result, AUDIO_OK);
}

/**
 * @tc.name  : Test AudioWorkgroupCheck
 * @tc.type  : FUNC
 * @tc.number: AudioWorkgroupCheck
 * @tc.desc  : Test AudioWorkgroupCheck method with invalid and valid pid
 */
HWTEST(AudioResourceServiceUnitTest, AudioWorkgroupCheck_001, TestSize.Level0)
{
    int32_t pid = 123;
    int32_t result = audioResourceService.AudioWorkgroupCheck(pid);
    EXPECT_EQ(result, SUCCESS);

    pid = -111;
    result = audioResourceService.AudioWorkgroupCheck(pid);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test ReleaseAudioWorkgroup
 * @tc.type  : FUNC
 * @tc.number: ReleaseAudioWorkgroup
 * @tc.desc  : Test ReleaseAudioWorkgroup method with invalid and valid pid
 */
HWTEST(AudioResourceServiceUnitTest, ReleaseAudioWorkgroup_001, TestSize.Level0)
{
    int32_t invalidPid = -1;
    int32_t workgroupId = 1;
    EXPECT_EQ(audioResourceService.ReleaseAudioWorkgroup(invalidPid, workgroupId), ERR_OPERATION_FAILED);

    int32_t pid = 123;
    int32_t nonExistentWorkgroupId = 999;
    EXPECT_EQ(audioResourceService.ReleaseAudioWorkgroup(pid, nonExistentWorkgroupId), ERR_INVALID_PARAM);

    std::shared_ptr<AudioWorkgroup> workgroup = std::make_shared<AudioWorkgroup>(testRtgId);
    audioResourceService.audioWorkgroupMap_[1].groups[testRtgId] = {workgroup};
    EXPECT_NE(audioResourceService.ReleaseAudioWorkgroup(1, 1), ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test GetThreadsNumPerProcess
 * @tc.type  : FUNC
 * @tc.number: GetThreadsNumPerProcess
 * @tc.desc  : Test GetThreadsNumPerProcess when find workgroup
 */
HWTEST(AudioResourceServiceUnitTest, GetThreadsNumPerProcess_001, TestSize.Level0)
{
    int32_t nonExistPid = 9999;
    EXPECT_EQ(audioResourceService.GetThreadsNumPerProcess(nonExistPid), 0);

    int32_t pid = 1234;
    audioResourceService.audioWorkgroupMap_[pid].groups.clear();
    EXPECT_EQ(audioResourceService.GetThreadsNumPerProcess(pid), 0);

    int32_t ExistPid = 1;
    EXPECT_EQ(audioResourceService.GetThreadsNumPerProcess(ExistPid), 0);
}

/**
 * @tc.name  : Test RegisterAudioWorkgroupMonitor
 * @tc.type  : FUNC
 * @tc.number: RegisterAudioWorkgroupMonitor
 * @tc.desc  : Test RegisterAudioWorkgroupMonitor when find workgroup
 */
HWTEST(AudioResourceServiceUnitTest, RegisterAudioWorkgroupMonitor_001, TestSize.Level0)
{
    int32_t pid = 123;
    int32_t groupId = 1;
    sptr<IRemoteObject> remoteObj = new RemoteObjectTestStub();
    int32_t ret = audioResourceService.RegisterAudioWorkgroupMonitor(pid, groupId, remoteObj);
    EXPECT_NE(ret, 0);

    groupId = -1;
    ret = audioResourceService.RegisterAudioWorkgroupMonitor(pid, groupId, remoteObj);
    EXPECT_NE(ret, 0);

    pid = 1;
    ret = audioResourceService.RegisterAudioWorkgroupMonitor(pid, groupId, remoteObj);

    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test CreateAudioWorkgroup
 * @tc.type  : FUNC
 * @tc.number: CreateAudioWorkgroup
 * @tc.desc  : Test ReleaseWorkgroupDeathRecipient when find workgroup
 */
HWTEST(AudioResourceServiceUnitTest, CreateAudioWorkgroup_001, TestSize.Level0)
{
    sptr<IRemoteObject> remoteObj = nullptr;
    EXPECT_EQ(audioResourceService.CreateAudioWorkgroup(-1, remoteObj), ERR_INVALID_PARAM);

    EXPECT_EQ(audioResourceService.CreateAudioWorkgroup(1, nullptr), ERR_OPERATION_FAILED);

    remoteObj = new RemoteObjectTestStub();
    EXPECT_NE(audioResourceService.CreateAudioWorkgroup(1, remoteObj), ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test AddThreadToGroup
 * @tc.type  : FUNC
 * @tc.number: AddThreadToGroup
 * @tc.desc  : Test AddThreadToGroup when find workgroup
 */
HWTEST(AudioResourceServiceUnitTest, AddThreadToGroup_001, TestSize.Level0)
{
    int32_t pid = 123;
    int32_t workgroupId = 1;
    int32_t tokenId = pid;

    int32_t ret = audioResourceService.AddThreadToGroup(pid, workgroupId, tokenId);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);

    tokenId = 3;
    ret = audioResourceService.AddThreadToGroup(pid, workgroupId, tokenId);
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name  : Test WorkgroupRendererMonitor
 * @tc.type  : FUNC
 * @tc.number: WorkgroupRendererMonitor
 * @tc.desc  : Test WorkgroupRendererMonitor when find workgroup
 */
HWTEST(AudioResourceServiceUnitTest, WorkgroupRendererMonitor_001, TestSize.Level0)
{
    int32_t testPid = -111;
    audioResourceService.WorkgroupRendererMonitor(testPid, true);
    EXPECT_FALSE(audioResourceService.audioWorkgroupMap_[testPid].permission);

    testPid = 123;
    audioResourceService.audioWorkgroupMap_[testPid].permission = true;
    audioResourceService.WorkgroupRendererMonitor(testPid, true);

    EXPECT_TRUE(audioResourceService.audioWorkgroupMap_[testPid].permission);
}

/**
 * @tc.name  : Test DumpAudioWorkgroupMap
 * @tc.type  : FUNC
 * @tc.number: DumpAudioWorkgroupMap
 * @tc.desc  : Test DumpAudioWorkgroupMap when find workgroup
 */
HWTEST(AudioResourceServiceUnitTest, DumpAudioWorkgroupMap_001, TestSize.Level0)
{
    sptr<IRemoteObject> remoteObj = nullptr;
    audioResourceService.DumpAudioWorkgroupMap();
    EXPECT_TRUE(remoteObj == nullptr);
}

/**
 * @tc.name  : Test StopGroup
 * @tc.type  : FUNC
 * @tc.number: StopGroup
 * @tc.desc  : Test StopGroup when find workgroup
 */
HWTEST(AudioResourceServiceUnitTest, StopGroup_001, TestSize.Level0)
{
    int32_t ret = audioResourceService.StopGroup(123, 456);
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name  : StopGroup
 * @tc.type  : FUNC
 * @tc.number: StopGroup_002
 * @tc.desc  : Test that StopGroup executes Stop when group exists.
 */
HWTEST(AudioResourceServiceUnitTest, StopGroup_002, TestSize.Level1)
{
    AudioResourceService* service = AudioResourceService::GetInstance();
    int32_t pid = 55;
    int32_t groupId = 66;
 
    std::shared_ptr<AudioWorkgroup> workgroupPtr = std::make_shared<AudioWorkgroup>(groupId);
    service->audioWorkgroupMap_[pid].groups[groupId] = workgroupPtr;
 
    int32_t ret = service->StopGroup(pid, groupId);
 
    EXPECT_EQ(ret, AUDIO_ERR);
}

/**
 * @tc.name  : Test IsProcessHasSystemPermission
 * @tc.type  : FUNC
 * @tc.number: IsProcessHasSystemPermission_001
 * @tc.desc  : Test IsProcessHasSystemPermission when find workgroup
 */
HWTEST(AudioResourceServiceUnitTest, IsProcessHasSystemPermission_001, TestSize.Level0)
{
    int32_t pid = 123;
    audioResourceService.audioWorkgroupMap_[pid].hasSystemPermission = true;
    EXPECT_TRUE(audioResourceService.IsProcessHasSystemPermission(pid));
}

/**
 * @tc.name  : Test IsProcessHasSystemPermission
 * @tc.type  : FUNC
 * @tc.number: IsProcessHasSystemPermission_002
 * @tc.desc  : Test IsProcessHasSystemPermission when find workgroup
 */
HWTEST(AudioResourceServiceUnitTest, IsProcessHasSystemPermission_002, TestSize.Level0)
{
    int32_t pid = 123;
    audioResourceService.audioWorkgroupMap_[pid].hasSystemPermission = false;
    EXPECT_FALSE(audioResourceService.IsProcessHasSystemPermission(pid));
}

/**
 * @tc.name  : Test RegisterAudioWorkgroupMonitor
 * @tc.type  : FUNC
 * @tc.number: RegisterAudioWorkgroupMonitor_002
 * @tc.desc  : Test RegisterAudioWorkgroupMonitor when find workgroup
 */
HWTEST(AudioResourceServiceUnitTest, RegisterAudioWorkgroupMonitor_002, TestSize.Level0)
{
    int32_t pid = 1;
    int32_t groupId = 1;
    sptr<IRemoteObject> remoteObj = new RemoteObjectTestStub();
    EXPECT_NE(audioResourceService.RegisterAudioWorkgroupMonitor(pid, groupId, remoteObj), SUCCESS);
}

/**
 * @tc.name  : Test RegisterAudioWorkgroupMonitor
 * @tc.type  : FUNC
 * @tc.number: RegisterAudioWorkgroupMonitor_003
 * @tc.desc  : Test RegisterAudioWorkgroupMonitor when find workgroup
 */
HWTEST(AudioResourceServiceUnitTest, RegisterAudioWorkgroupMonitor_003, TestSize.Level0)
{
    int32_t pid = 1;
    int32_t groupId = 1;
    sptr<IRemoteObject> object = nullptr;
    EXPECT_EQ(audioResourceService.RegisterAudioWorkgroupMonitor(pid, groupId, object), ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test RegisterAudioWorkgroupMonitor
 * @tc.type  : FUNC
 * @tc.number: RegisterAudioWorkgroupMonitor_004
 * @tc.desc  : Test RegisterAudioWorkgroupMonitor when find workgroup
 */
HWTEST(AudioResourceServiceUnitTest, RegisterAudioWorkgroupMonitor_004, TestSize.Level0)
{
    int32_t pid = 1;
    int32_t groupId = 1;
    sptr<IRemoteObject> remoteObj = new RemoteObjectTestStub();
    audioResourceService.audioWorkgroupMap_[pid].hasSystemPermission = false;
    EXPECT_EQ(audioResourceService.RegisterAudioWorkgroupMonitor(pid, groupId, remoteObj), ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test RegisterAudioWorkgroupMonitor
 * @tc.type  : FUNC
 * @tc.number: RegisterAudioWorkgroupMonitor_005
 * @tc.desc  : Test RegisterAudioWorkgroupMonitor when find workgroup
 */
HWTEST(AudioResourceServiceUnitTest, RegisterAudioWorkgroupMonitor_005, TestSize.Level0)
{
    int32_t pid = 1;
    int32_t groupId = 1;
    sptr<IRemoteObject> remoteObj = new RemoteObjectTestStub();
    std::shared_ptr<AudioWorkgroup> workgroup = std::make_shared<AudioWorkgroup>(testRtgId);
    audioResourceService.audioWorkgroupMap_[pid].hasSystemPermission = false;
    audioResourceService.audioWorkgroupMap_[1].groups[testRtgId] = {workgroup};
    EXPECT_NE(audioResourceService.RegisterAudioWorkgroupMonitor(pid, groupId, remoteObj), SUCCESS);
}

/**
 * @tc.name  : Test StartGroup
 * @tc.type  : FUNC
 * @tc.number: StartGroup_001
 * @tc.desc  : Test StartGroup when find workgroup
 */
HWTEST(AudioResourceServiceUnitTest, StartGroup_001, TestSize.Level0)
{
    int32_t pid = 1;
    int32_t workgroupId = 1;
    uint64_t startTime = 1000;
    uint64_t deadlineTime = 2000;

    int32_t ret = audioResourceService.StartGroup(pid, workgroupId, startTime, deadlineTime);

    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : StartGroup
 * @tc.type  : FUNC
 * @tc.number: StartGroup_002
 * @tc.desc  : Test that StartGroup executes Start when group exists.
 */
HWTEST(AudioResourceServiceUnitTest, StartGroup_002, TestSize.Level1)
{
    AudioResourceService* service = AudioResourceService::GetInstance();
    int32_t pid = 101;
    int32_t groupId = 202;
    uint64_t startTime = 1000;
    uint64_t deadlineTime = 2000;
 
    std::shared_ptr<AudioWorkgroup> workgroupPtr = std::make_shared<AudioWorkgroup>(groupId);
    service->audioWorkgroupMap_[pid].groups[groupId] = workgroupPtr;
 
    int32_t ret = service->StartGroup(pid, groupId, startTime, deadlineTime);
 
    EXPECT_EQ(ret, AUDIO_ERR);
}

/**
 * @tc.name  : Test RemoveThreadFromGroup
 * @tc.type  : FUNC
 * @tc.number: RemoveThreadFromGroup_001
 * @tc.desc  : Test RemoveThreadFromGroup when find workgroup
 */
HWTEST(AudioResourceServiceUnitTest, RemoveThreadFromGroup_001, TestSize.Level0)
{
    int32_t pid = 1;
    int32_t workgroupId = 1;
    int32_t tokenId = 1;

    int32_t ret = audioResourceService.RemoveThreadFromGroup(pid, workgroupId, tokenId);

    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : RemoveThreadFromGroup
 * @tc.type  : FUNC
 * @tc.number: RemoveThreadFromGroup_002
 * @tc.desc  : Test that RemoveThreadFromGroup executes RemoveThread when group exists.
 */
HWTEST(AudioResourceServiceUnitTest, RemoveThreadFromGroup_002, TestSize.Level1)
{
    AudioResourceService* service = AudioResourceService::GetInstance();
    int32_t pid = 1111;
    int32_t groupId = 2222;
    int32_t tokenId = 3333;
 
    // Insert a real AudioWorkgroup into map
    std::shared_ptr<AudioWorkgroup> workgroupPtr = std::make_shared<AudioWorkgroup>(groupId);
    service->audioWorkgroupMap_[pid].groups[groupId] = workgroupPtr;
 
    int32_t ret = service->RemoveThreadFromGroup(pid, groupId, tokenId);
 
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name  : Test AddThreadToGroup
 * @tc.type  : FUNC
 * @tc.number: AddThreadToGroup_002
 * @tc.desc  : Test AddThreadToGroup when find workgroup
 */
HWTEST(AudioResourceServiceUnitTest, AddThreadToGroup_002, TestSize.Level0)
{
    int32_t pid = 1;
    int32_t workgroupId = 1;
    int32_t tokenId = 1;
    int32_t ret = audioResourceService.AddThreadToGroup(pid, workgroupId, tokenId);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test AddThreadToGroup
 * @tc.type  : FUNC
 * @tc.number: AddThreadToGroup_003
 * @tc.desc  : Test AddThreadToGroup when find workgroup
 */
HWTEST(AudioResourceServiceUnitTest, AddThreadToGroup_003, TestSize.Level0)
{
    int32_t pid = 2;
    int32_t workgroupId = 2;
    int32_t tokenId = 2;
    int32_t ret = audioResourceService.AddThreadToGroup(pid, workgroupId, tokenId);
    EXPECT_NE(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test AddThreadToGroup
 * @tc.type  : FUNC
 * @tc.number: AddThreadToGroup_004
 * @tc.desc  : Test AddThreadToGroup when find workgroup
 */
HWTEST(AudioResourceServiceUnitTest, AddThreadToGroup_004, TestSize.Level0)
{
    int32_t pid = 3;
    int32_t workgroupId = 3;
    int32_t tokenId = 3;
    int32_t ret = audioResourceService.AddThreadToGroup(pid, workgroupId, tokenId);
    EXPECT_NE(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test AddThreadToGroup
 * @tc.type  : FUNC
 * @tc.number: AddThreadToGroup_005
 * @tc.desc  : Test AddThreadToGroup when find workgroup
 */
HWTEST(AudioResourceServiceUnitTest, AddThreadToGroup_005, TestSize.Level0)
{
    int32_t pid = 4;
    int32_t workgroupId = 4;
    int32_t tokenId = 4;
    int32_t ret = audioResourceService.AddThreadToGroup(pid, workgroupId, tokenId);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AddThreadToGroup
 * @tc.type  : FUNC
 * @tc.number: AddThreadToGroup_006
 * @tc.desc  : Test when the number of threads per process reaches AUDIO_MAX_RT_THREADS,
 *             AddThreadToGroup should return ERR_NOT_SUPPORTED.
 */
HWTEST(AudioResourceServiceUnitTest, AddThreadToGroup_006, TestSize.Level1)
{
    AudioResourceService* service = AudioResourceService::GetInstance();
    int32_t pid = 12345;
    int32_t groupId = 67890;
    int32_t tokenId = 12346;
 
    std::shared_ptr<DummyAudioWorkgroup> workgroupPtr = std::make_shared<DummyAudioWorkgroup>(groupId);
    service->audioWorkgroupMap_[pid].groups[groupId] = workgroupPtr;
 
    for (int i = 0; i < 4; ++i) {
        workgroupPtr->AddThread(tokenId + i + 100);
    }
    int32_t ret = service->AddThreadToGroup(pid, groupId, tokenId);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : ReleaseAudioWorkgroup
 * @tc.type  : FUNC
 * @tc.number: ReleaseAudioWorkgroup_006
 * @tc.desc  : Test successful release of an existing audio workgroup. Covers the normal branch where reply.paramA == 0.
 */
HWTEST(AudioResourceServiceUnitTest, ReleaseAudioWorkgroup_005, TestSize.Level1)
{
    AudioResourceService* service = AudioResourceService::GetInstance();
    int32_t pid = 12345;
    int32_t groupId = 67890;
 
    std::shared_ptr<DummyAudioWorkgroup> workgroupPtr = std::make_shared<DummyAudioWorkgroup>(groupId);
    service->audioWorkgroupMap_[pid].groups[groupId] = workgroupPtr;
 
    int32_t ret = service->ReleaseAudioWorkgroup(pid, groupId);
 
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(service->audioWorkgroupMap_.count(pid), 0);
}

/**
 * @tc.name  : Test AudioWorkgroupCheck
 * @tc.type  : FUNC
 * @tc.number: AudioWorkgroupCheck_003
 * @tc.desc  : Test AudioWorkgroupCheck when find workgroup
 */
HWTEST(AudioResourceServiceUnitTest, AudioWorkgroupCheck_003, TestSize.Level0)
{
    int32_t pid = 1234;
    for (int i = 0; i < AUDIO_MAX_PROCESS; i++) {
        audioResourceService.audioWorkgroupMap_[AUDIO_MAX_PROCESS].hasSystemPermission = false;
    }
    EXPECT_NE(audioResourceService.AudioWorkgroupCheck(pid), ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test AudioWorkgroupCheck
 * @tc.type  : FUNC
 * @tc.number: AudioWorkgroupCheck_004
 * @tc.desc  : Test AudioWorkgroupCheck when find workgroup
 */
HWTEST(AudioResourceServiceUnitTest, AudioWorkgroupCheck_004, TestSize.Level0)
{
    int32_t pid = 1234;
    EXPECT_EQ(audioResourceService.AudioWorkgroupCheck(pid), SUCCESS);
}

/**
 * @tc.name  : Test WorkgroupRendererMonitor
 * @tc.type  : FUNC
 * @tc.number: WorkgroupRendererMonitor
 * @tc.desc  : Test WorkgroupRendererMonitor when find workgroup
 */
HWTEST(AudioResourceServiceUnitTest, WorkgroupRendererMonitor_002, TestSize.Level0)
{
    int32_t testPid = 321;
    audioResourceService.audioWorkgroupMap_[testPid].permission = false;
    audioResourceService.WorkgroupRendererMonitor(testPid, true);

    EXPECT_TRUE(audioResourceService.audioWorkgroupMap_[testPid].permission);
}

/**
 * @tc.name  : Test deathRecipient
 * @tc.type  : FUNC
 * @tc.number: OnWorkgroupRemoteDied_001
 * @tc.desc  : Test OnWorkgroupRemoteDied when called
 */
HWTEST(AudioResourceServiceUnitTest, OnWorkgroupRemoteDied_001, TestSize.Level0)
{
    std::shared_ptr<AudioWorkgroup> workgroup = std::make_shared<AudioWorkgroup>(testRtgId);
    std::shared_ptr<AudioWorkgroup> workGroup = std::make_shared<AudioWorkgroup>(testRtgId);
    sptr<IRemoteObject> remoteObj = nullptr;

    audioResourceService.audioWorkgroupMap_[10].groups[testRtgId] = {workGroup};
    audioResourceService.OnWorkgroupRemoteDied(workgroup, remoteObj);
    EXPECT_EQ(audioResourceService.audioWorkgroupMap_[10].groups.count(testRtgId), 1);
}

/**
 * @tc.name  : Test AudioWorkgroupCheck
 * @tc.type  : FUNC
 * @tc.number: AudioWorkgroupCheck
 * @tc.desc  : Test ReleaseWorkgroupDeathRecipient when find workgroup
 */
HWTEST(AudioResourceServiceUnitTest, AudioWorkgroupCheck_008, TestSize.Level0)
{
    int32_t pid = 4321;
    for (int i = 0; i <= AUDIO_MAX_GRP_PER_PROCESS; i++) {
        audioResourceService.audioWorkgroupMap_[pid].groups[i] = nullptr;
    }
    EXPECT_TRUE(audioResourceService.IsProcessInWorkgroup(pid));
    EXPECT_EQ(audioResourceService.AudioWorkgroupCheck(pid), ERR_NOT_SUPPORTED);

    pid = 532;
    audioResourceService.audioWorkgroupMap_[pid].groups[pid] = nullptr;
    audioResourceService.audioWorkgroupMap_[pid].hasSystemPermission = false;

    for (int i = 0; i <= AUDIO_MAX_PROCESS; i++) {
        audioResourceService.audioWorkgroupMap_[i].groups[i] = nullptr;
    }
    EXPECT_FALSE(audioResourceService.IsProcessInWorkgroup(pid + 1));
    EXPECT_EQ(audioResourceService.AudioWorkgroupCheck(pid + 1), ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : AudioWorkgroupCheck
 * @tc.type  : FUNC
 * @tc.number: AudioWorkgroupCheck_005
 * @tc.desc  : Should return ERR_NOT_SUPPORTED if a process already has >= AUDIO_MAX_GRP_PER_PROCESS groups.
 */
HWTEST(AudioResourceServiceUnitTest, AudioWorkgroupCheck_005, TestSize.Level1)
{
    auto* service = AudioResourceService::GetInstance();
    int32_t pid = 1001;
    service->audioWorkgroupMap_.clear();
    service->audioWorkgroupMap_[pid].groups.clear();
    for (int i = 0; i < 4; ++i) {
        service->audioWorkgroupMap_[pid].groups[i] = std::make_shared<AudioWorkgroup>(i);
    }
    EXPECT_EQ(service->AudioWorkgroupCheck(pid), ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : AudioWorkgroupCheck
 * @tc.type  : FUNC
 * @tc.number: AudioWorkgroupCheck_006
 * @tc.desc  : Should enter the for-loop over processes when not in any group.
 */
HWTEST(AudioResourceServiceUnitTest, AudioWorkgroupCheck_006, TestSize.Level1)
{
    auto* service = AudioResourceService::GetInstance();
    service->audioWorkgroupMap_.clear();
    // Fill a few process entries, all without system permission
    service->audioWorkgroupMap_[2001].hasSystemPermission = false;
    int32_t pid = 9999;
    EXPECT_EQ(service->AudioWorkgroupCheck(pid), SUCCESS);
}

/**
 * @tc.name  : AudioWorkgroupCheck - Max Process Limit
 * @tc.type  : FUNC
 * @tc.number: AudioWorkgroupCheck_007
 * @tc.desc  : Should return ERR_NOT_SUPPORTED if normal process count reaches AUDIO_MAX_PROCESS.
 */
HWTEST(AudioResourceServiceUnitTest, AudioWorkgroupCheck_007, TestSize.Level1)
{
    auto* service = AudioResourceService::GetInstance();
    service->audioWorkgroupMap_.clear();
    for (int i = 0; i < 2; ++i) {
        service->audioWorkgroupMap_[3000 + i].hasSystemPermission = false;
    }
    int32_t pid = 8888;
    EXPECT_EQ(service->AudioWorkgroupCheck(pid), ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test FillAudioWorkgroupCgroupLimit
 * @tc.type  : FUNC
 * @tc.number: FillAudioWorkgroupCgroupLimit_001
 * @tc.desc  : Test FillAudioWorkgroupCgroupLimit when no used group id
 */
HWTEST(AudioResourceServiceUnitTest, FillAudioWorkgroupCgroupLimit_NoUsedGroupId, TestSize.Level0)
{
    AudioResourceService service;
    int32_t pid = 2;
    std::shared_ptr<AudioWorkgroup> workgroup = std::make_shared<AudioWorkgroup>(123);
    service.FillAudioWorkgroupCgroupLimit(pid, workgroup);
    EXPECT_EQ(workgroup->cgroupLimit.globalCgroupId, 0);
    EXPECT_EQ(workgroup->cgroupLimit.clientPid, pid);
}
 
/**
 * @tc.name  : Test FillAudioWorkgroupCgroupLimit
 * @tc.type  : FUNC
 * @tc.number: FillAudioWorkgroupCgroupLimit_002
 * @tc.desc  : Test FillAudioWorkgroupCgroupLimit when all group ids are used
 */
HWTEST(AudioResourceServiceUnitTest, FillAudioWorkgroupCgroupLimit_AllGroupIdsUsed, TestSize.Level0)
{
    AudioResourceService service;
    int32_t pid = 4;
    for (int i = 0; i < 4; ++i) {
        auto g = std::make_shared<AudioWorkgroup>(300+i);
        g->SetCgroupLimitParams(pid, i);
        service.audioWorkgroupMap_[pid].groups[i] = g;
    }
    std::shared_ptr<AudioWorkgroup> workgroup = std::make_shared<AudioWorkgroup>(888);
    service.FillAudioWorkgroupCgroupLimit(pid, workgroup);
    EXPECT_EQ(workgroup->cgroupLimit.globalCgroupId, -1);
    EXPECT_EQ(workgroup->cgroupLimit.clientPid, pid);
}
 
/**
 * @tc.name  : Test GetCgroupLimitId
 * @tc.type  : FUNC
 * @tc.number: GetCgroupLimitId_001
 * @tc.desc  : Test GetCgroupLimitId returns default value
 */
HWTEST(AudioWorkgroupUnitTest, GetCgroupLimitId_DefaultValue, TestSize.Level0)
{
    AudioWorkgroup workgroup(1);
    EXPECT_EQ(workgroup.GetCgroupLimitId(), -1);
}
 
/**
 * @tc.name  : Test SetCgroupLimitParams and GetCgroupLimitId
 * @tc.type  : FUNC
 * @tc.number: SetCgroupLimitParams_001
 * @tc.desc  : Test SetCgroupLimitParams sets globalCgroupId and can be retrieved by GetCgroupLimitId
 */
HWTEST(AudioWorkgroupUnitTest, SetCgroupLimitParams_SetAndGet, TestSize.Level0)
{
    AudioWorkgroup workgroup(2);
    int32_t testPid = 100;
    int32_t testCgroupId = 7;
    workgroup.SetCgroupLimitParams(testPid, testCgroupId);
    EXPECT_EQ(workgroup.GetCgroupLimitId(), testCgroupId);
    EXPECT_EQ(workgroup.cgroupLimit.clientPid, testPid);
}
 
/**
 * @tc.name  : Test SetCgroupLimitParams overwrite
 * @tc.type  : FUNC
 * @tc.number: SetCgroupLimitParams_002
 * @tc.desc  : Test SetCgroupLimitParams can overwrite previous values
 */
HWTEST(AudioWorkgroupUnitTest, SetCgroupLimitParams_Overwrite, TestSize.Level0)
{
    AudioWorkgroup workgroup(3);
    workgroup.SetCgroupLimitParams(200, 5);
    workgroup.SetCgroupLimitParams(201, 9);
    EXPECT_EQ(workgroup.GetCgroupLimitId(), 9);
    EXPECT_EQ(workgroup.cgroupLimit.clientPid, 201);
}
} // namespace AudioStandard
} // namespace OHOS