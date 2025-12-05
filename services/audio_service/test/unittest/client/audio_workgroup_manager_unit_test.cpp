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

#include <gtest/gtest.h>

#include <memory>
#include <unordered_map>

#include "audio_errors.h"
#include "audio_log.h"
#include "audio_workgroup_client_manager.h"
#include "audio_workgroup_callback_impl.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
class WorkgroupManagerStateGuard {
public:
    explicit WorkgroupManagerStateGuard(WorkgroupPrioRecorderManager &manager)
        : workGroupManager_(manager),
          hasSystemPermissionBackup_(manager.hasSystemPermission_),
          workgroupPrioRecorderMapBackup_(manager.workgroupPrioRecorderMap_),
          startGroupPermissionMapBackup_(manager.startGroupPermissionMap_) {}

    ~WorkgroupManagerStateGuard()
    {
        workGroupManager_.hasSystemPermission_ = hasSystemPermissionBackup_;
        workGroupManager_.workgroupPrioRecorderMap_ = workgroupPrioRecorderMapBackup_;
        workGroupManager_.startGroupPermissionMap_ = startGroupPermissionMapBackup_;
    }

private:
    WorkgroupPrioRecorderManager &workGroupManager_;
    bool hasSystemPermissionBackup_;
    std::unordered_map<int32_t, std::shared_ptr<WorkgroupPrioRecorder>> workgroupPrioRecorderMapBackup_;
    std::unordered_map<uint32_t, std::unordered_map<uint32_t, bool>> startGroupPermissionMapBackup_;
};
} // namespace

class AudioWorkgroupManagerUnitTest : public testing::Test {
public:
    void SetUp() override
    {
        manager_ = &WorkgroupPrioRecorderManager::GetInstance();
        stateGuard_ = std::make_unique<WorkgroupManagerStateGuard>(*manager_);
        manager_->hasSystemPermission_ = false;
        manager_->workgroupPrioRecorderMap_.clear();
        manager_->startGroupPermissionMap_.clear();
    }

    void TearDown() override
    {
        stateGuard_.reset();
    }

protected:
    WorkgroupPrioRecorderManager *manager_ = nullptr;
    std::unique_ptr<WorkgroupManagerStateGuard> stateGuard_;
};

/**
 * @tc.name   : WorkgroupPrioRecorder_DefaultInit
 * @tc.number : WorkgroupPrioRecorder_DefaultInit_001
 * @tc.desc   : Verify recorder default state after construction.
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, WorkgroupPrioRecorder_DefaultInit_001, TestSize.Level1)
{
    int32_t grpId = 10;
    WorkgroupPrioRecorder recorder(grpId);

    EXPECT_EQ(recorder.grpId_, grpId);
    EXPECT_FALSE(recorder.restoreByPermission_);
    EXPECT_TRUE(recorder.threads_.empty());
}

/**
 * @tc.name   : WorkgroupPrioRecorder_SetRestoreFlag
 * @tc.number : WorkgroupPrioRecorder_SetRestoreFlag_001
 * @tc.desc   : Verify SetRestoreByPermission updates the flag.
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, WorkgroupPrioRecorder_SetRestoreFlag_001, TestSize.Level1)
{
    WorkgroupPrioRecorder recorder(1);
    recorder.SetRestoreByPermission(true);
    EXPECT_TRUE(recorder.GetRestoreByPermission());

    recorder.SetRestoreByPermission(false);
    EXPECT_FALSE(recorder.GetRestoreByPermission());
}

/**
 * @tc.name   : WorkgroupPrioRecorder_RecordThreadPrio
 * @tc.number : WorkgroupPrioRecorder_RecordThreadPrio_001
 * @tc.desc   : Verify RecordThreadPrio caches thread priority.
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, WorkgroupPrioRecorder_RecordThreadPrio_001, TestSize.Level1)
{
    WorkgroupPrioRecorder recorder(1);
    int32_t tokenId = 123;

    recorder.RecordThreadPrio(tokenId);
    auto it = recorder.threads_.find(tokenId);
    ASSERT_NE(it, recorder.threads_.end());
    EXPECT_GE(it->second, -1);
}

/**
 * @tc.name   : WorkgroupPrioRecorderManager_GetRecorderByGrpId
 * @tc.number : WorkgroupPrioRecorderManager_GetRecorderByGrpId_001
 * @tc.desc   : Verify GetRecorderByGrpId returns existing recorder.
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, WorkgroupPrioRecorderManager_GetRecorderByGrpId_001, TestSize.Level1)
{
    int32_t grpId = 7;
    auto recorder = std::make_shared<WorkgroupPrioRecorder>(grpId);
    manager_->workgroupPrioRecorderMap_[grpId] = recorder;

    auto result = manager_->GetRecorderByGrpId(grpId);
    EXPECT_EQ(result, recorder);

    grpId = 999;
    EXPECT_EQ(manager_->GetRecorderByGrpId(grpId), nullptr);
}

/**
 * @tc.name   : WorkgroupPrioRecorderManager_IsValidToStartGroup
 * @tc.number : WorkgroupPrioRecorderManager_IsValidToStartGroup_001
 * @tc.desc   : Verify IsValidToStartGroup returns true with system permission.
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, WorkgroupPrioRecorderManager_IsValidToStartGroup_001, TestSize.Level1)
{
    manager_->hasSystemPermission_ = true;
    EXPECT_TRUE(manager_->IsValidToStartGroup(1));

    manager_->hasSystemPermission_ = false;
    uint32_t pid = 4321;
    uint32_t grpId = 8;
    manager_->startGroupPermissionMap_[pid][grpId] = false;

    EXPECT_FALSE(manager_->IsValidToStartGroup(grpId));
}

/**
 * @tc.name   : WorkgroupPrioRecorderManager_IsValidToStartGroup
 * @tc.number : WorkgroupPrioRecorderManager_IsValidToStartGroup_002
 * @tc.desc   : Verify IsValidToStartGroup checks permission map when system permission missing.
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, WorkgroupPrioRecorderManager_IsValidToStartGroup_002, TestSize.Level1)
{
    manager_->hasSystemPermission_ = false;
    int32_t pid = getpid();
    uint32_t grpId = 5;
    manager_->startGroupPermissionMap_[pid][grpId] = true;

    EXPECT_TRUE(manager_->IsValidToStartGroup(grpId));
}

/**
 * @tc.name   : WorkgroupPrioRecorderManager_OnWorkgroupChange
 * @tc.number : WorkgroupPrioRecorderManager_OnWorkgroupChange_001
 * @tc.desc   : Verify OnWorkgroupChange updates permission to true.
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, WorkgroupPrioRecorderManager_OnWorkgroupChange_001, TestSize.Level1)
{
    AudioWorkgroupChangeInfo info;
    info.pid = 111;
    info.groupId = 3;
    info.startAllowed = true;

    manager_->OnWorkgroupChange(info);

    EXPECT_TRUE(manager_->startGroupPermissionMap_[info.pid][info.groupId]);
}

/**
 * @tc.name   : WorkgroupPrioRecorderManager_OnWorkgroupChange
 * @tc.number : WorkgroupPrioRecorderManager_OnWorkgroupChange_002
 * @tc.desc   : Verify OnWorkgroupChange updates permission to false.
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, WorkgroupPrioRecorderManager_OnWorkgroupChange_002, TestSize.Level1)
{
    AudioWorkgroupChangeInfo info;
    info.pid = 222;
    info.groupId = 4;
    info.startAllowed = false;

    manager_->OnWorkgroupChange(info);

    EXPECT_FALSE(manager_->startGroupPermissionMap_[info.pid][info.groupId]);
}

/**
 * @tc.name   : WorkgroupPrioRecorderManager_ExecuteAudioWorkgroupPrioImprove
 * @tc.number : WorkgroupPrioRecorderManager_ExecuteAudioWorkgroupPrioImprove_001
 * @tc.desc   : Verify ExecuteAudioWorkgroupPrioImprove returns error when recorder missing.
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, WorkgroupPrioRecorderManager_ExecuteAudioWorkgroupPrioImprove_001,
    TestSize.Level1)
{
    bool needUpdatePrio = true;
    std::unordered_map<int32_t, bool> threads;

    int32_t result = manager_->ExecuteAudioWorkgroupPrioImprove(9999, threads, needUpdatePrio);
    EXPECT_EQ(result, AUDIO_ERR);
    EXPECT_TRUE(needUpdatePrio);
}

/**
 * @tc.name   : Test CreateGroup API
 * @tc.number : CreateGroup_001
 * @tc.desc   : Test CreateGroup interface createAudioWorkgroup
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, CreateGroup_001, TestSize.Level1)
{
    int32_t result = manager_->CreateAudioWorkgroup();
    EXPECT_GT(result, 0);
}

/**
 * @tc.name   : Test IsValidToStartGroup API
 * @tc.number : IsValidToStartGroup_001
 * @tc.desc   : Test IsValidToStartGroup interface createAudioWorkgroup
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, IsValidToStartGroup_001, TestSize.Level1)
{
    int workgroupId = 1;

    bool result = manager_->IsValidToStartGroup(workgroupId);
    EXPECT_FALSE(result);

    workgroupId = -1111;
    result = manager_->IsValidToStartGroup(workgroupId);
    EXPECT_FALSE(result);

    workgroupId = 9999;
    result = manager_->IsValidToStartGroup(workgroupId);
    EXPECT_FALSE(result);
}

/**
 * @tc.name   : Test IsValidToStartGroup API
 * @tc.number : IsValidToStartGroup_002
 * @tc.desc   : Test IsValidToStartGroup interface createAudioWorkgroup
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, IsValidToStartGroup_002, TestSize.Level4)
{
    int workgroupId = 1;

    manager_->hasSystemPermission_ = false;
    bool result = manager_->IsValidToStartGroup(workgroupId);
    EXPECT_FALSE(result);
}

/**
 * @tc.name   : Test RemoveWorkgroupChangeCallback API
 * @tc.number : RemoveWorkgroupChangeCallback_001
 * @tc.desc   : Test RemoveWorkgroupChangeCallback interface
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, RemoveWorkgroupChangeCallback_001, TestSize.Level4)
{
    AudioWorkgroupCallbackImpl audioWorkgroupCallbackImpl;
    audioWorkgroupCallbackImpl.RemoveWorkgroupChangeCallback();
    EXPECT_EQ(nullptr, audioWorkgroupCallbackImpl.workgroupCb_);
}

/**
 * @tc.name   : Test StartGroup API
 * @tc.number : StartGroup_001
 * @tc.desc   : Test StartGroup interface when startTime > endTime.
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, StartGroup_001, TestSize.Level1)
{
    bool needUpdatePrio = true;
    int32_t testWorkgroupid = 1;
    int32_t startTimeMs = 1000;
    int32_t endTimeMs = 500;
    std::unordered_map<int32_t, bool> threads = {
        {101, true},
        {102, true}
    };
    int32_t result = manager_->StartGroup(testWorkgroupid, startTimeMs, endTimeMs, threads, needUpdatePrio);
    EXPECT_EQ(result, AUDIO_ERR);
}

/**
 * @tc.name   : Test StartGroup API
 * @tc.number : StartGroup_002
 * @tc.desc   : Test StartGroup interface
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, StartGroup_002, TestSize.Level4)
{
    bool needUpdatePrio = true;
    int32_t testWorkgroupid = 1;
    int32_t startTimeMs = 500;
    int32_t endTimeMs = 1000;
    std::unordered_map<int32_t, bool> threads = {
        {101, true},
        {102, true}
    };
    int32_t result = manager_->StartGroup(testWorkgroupid, startTimeMs, endTimeMs, threads, needUpdatePrio);
    EXPECT_EQ(result, AUDIO_ERR);
}

/**
 * @tc.name   : Test StartGroup API
 * @tc.number : StartGroup_003
 * @tc.desc   : Test StartGroup interface.
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, StartGroup_003, TestSize.Level4)
{
    bool needUpdatePrio = false;
    int32_t testWorkgroupid = 1;
    int32_t startTimeMs = 1000;
    int32_t endTimeMs = 500;
    std::unordered_map<int32_t, bool> threads = {
        {101, true},
        {102, true}
    };
    int32_t result = manager_->StartGroup(testWorkgroupid, startTimeMs, endTimeMs, threads, needUpdatePrio);
    EXPECT_EQ(result, AUDIO_ERR);
}

/**
 * @tc.name   : Test StopGroup API
 * @tc.number : StopGroupp_001
 * @tc.desc   : Test StopGroup interface createAudioWorkgroup
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, StopGroup_001, TestSize.Level1)
{
    int workgroupId = 1;

    bool result = manager_->StopGroup(workgroupId);
    EXPECT_TRUE(result);

    workgroupId = -111;
    result = manager_->StopGroup(workgroupId);
    EXPECT_TRUE(result);

    workgroupId = 9999;
    result = manager_->StopGroup(workgroupId);
    EXPECT_TRUE(result);
}

/**
 * @tc.name   : Test SetRestoreByPermission
 * @tc.number : SetRestoreByPermission_001
 * @tc.desc   : Test SetRestoreByPermission when isByPermission true
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, SetRestoreByPermission_001, TestSize.Level1)
{
    WorkgroupPrioRecorder recorder(1);
    recorder.SetRestoreByPermission(true);
    EXPECT_TRUE(recorder.restoreByPermission_);
}

/**
 * @tc.name   : Test SetRestoreByPermission
 * @tc.number : SetRestoreByPermission_002
 * @tc.desc   : Test SetRestoreByPermission when isByPermission false
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, SetRestoreByPermission_002, TestSize.Level1)
{
    WorkgroupPrioRecorder recorder(1);
    recorder.SetRestoreByPermission(false);
    EXPECT_FALSE(recorder.restoreByPermission_);
}
 
/**
 * @tc.name   : Test GetRestoreByPermission
 * @tc.number : GetRestoreByPermission_001
 * @tc.desc   : Test SetRestoreByPermission when permission is set
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, GetRestoreByPermission_001, TestSize.Level1)
{
    WorkgroupPrioRecorder recorder(1);
    recorder.restoreByPermission_ = true;
    EXPECT_TRUE(recorder.GetRestoreByPermission());
}
 
/**
 * @tc.name   : Test GetRestoreByPermission
 * @tc.number : GetRestoreByPermission_002
 * @tc.desc   : Test SetRestoreByPermission when permission is not set
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, GetRestoreByPermission_002, TestSize.Level1)
{
    WorkgroupPrioRecorder recorder(1);
    recorder.restoreByPermission_ = false;
    EXPECT_FALSE(recorder.GetRestoreByPermission());
}
 
/**
 * @tc.name   : Test RecordThreadPrio
 * @tc.number : RecordThreadPrio_001
 * @tc.desc   : Test RecordThreadPrio inteface
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, RecordThreadPrio_001, TestSize.Level1)
{
    WorkgroupPrioRecorder recorder(1);
    int32_t tokenId = 1;
 
    // Add the tokenId to the threads_ map
    recorder.threads_[tokenId] = 2;
 
    // Call the method under test
    recorder.RecordThreadPrio(tokenId);
 
    // Verify the result
    auto it = recorder.threads_.find(tokenId);
    ASSERT_TRUE(it != recorder.threads_.end());
    EXPECT_EQ(it->second, 2);
}
 
/**
 * @tc.name   : Test RestoreGroupPrio
 * @tc.number : RestoreGroupPrio_001
 * @tc.desc   : Test RestoreGroupPrio set permission
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, RestoreGroupPrio_001, TestSize.Level1)
{
    WorkgroupPrioRecorder recorder(1);
    int32_t result = recorder.RestoreGroupPrio(true);
    EXPECT_NE(result, AUDIO_OK);
    EXPECT_FALSE(recorder.restoreByPermission_);
}
 
/**
 * @tc.name   : Test RestoreGroupPrio
 * @tc.number : RestoreGroupPrio_002
 * @tc.desc   : Test RestoreGroupPrio not set permission
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, RestoreGroupPrio_002, TestSize.Level1)
{
    WorkgroupPrioRecorder recorder(1);
    int32_t result = recorder.RestoreGroupPrio(false);
    EXPECT_NE(result, AUDIO_OK);
    EXPECT_TRUE(recorder.threads_.empty());
}

/**
 * @tc.name   : Test RestoreGroupPrio
 * @tc.number : RestoreGroupPrio_003
 * @tc.desc   : Test RestoreGroupPrio with threads
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, RestoreGroupPrio_003, TestSize.Level1)
{
    WorkgroupPrioRecorder recorder(1);
    recorder.threads_.emplace(1, 1);
    int32_t result = recorder.RestoreGroupPrio(true);
    EXPECT_EQ(result, AUDIO_OK);
    EXPECT_TRUE(recorder.restoreByPermission_);
}

/**
 * @tc.name   : Test RestoreThreadPrio
 * @tc.number : RestoreThreadPrio_001
 * @tc.desc   : Test RestoreThreadPrio when tokenId not exist
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, RestoreThreadPrio_001, TestSize.Level1)
{
    WorkgroupPrioRecorder recorder(1);
    int32_t tokenId = 1;
    recorder.threads_[tokenId] = 1;
    int32_t result = recorder.RestoreThreadPrio(tokenId + 1);
    EXPECT_EQ(result, AUDIO_OK);
}
 
/**
 * @tc.name   : Test RestoreThreadPrio
 * @tc.number : RestoreThreadPrio_002
 * @tc.desc   : Test RestoreThreadPrio when tokenId exist
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, RestoreThreadPrio_002, TestSize.Level1)
{
    WorkgroupPrioRecorder recorder(1);
    int32_t tokenId = 1;
    recorder.threads_[tokenId] = 1;
    int32_t result = recorder.RestoreThreadPrio(tokenId);
    EXPECT_EQ(result, AUDIO_OK);
}
 
/**
 * @tc.name   : Test RestoreThreadPrio
 * @tc.number : RestoreThreadPrio_003
 * @tc.desc   : Test RestoreThreadPrio check tokenId
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, RestoreThreadPrio_003, TestSize.Level1)
{
    WorkgroupPrioRecorder recorder(1);
    int32_t tokenId = 1;
    recorder.threads_[tokenId] = 1;
    int32_t result = recorder.RestoreThreadPrio(tokenId);
    EXPECT_EQ(result, AUDIO_OK);
    EXPECT_EQ(recorder.threads_.find(tokenId), recorder.threads_.end());
}

/**
 * @tc.name   : Test WorkgroupPrioRecorder constructor
 * @tc.number : WorkgroupPrioRecorder_001
 * @tc.desc   : Test WorkgroupPrioRecorder constructor
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, WorkgroupPrioRecorder_001, TestSize.Level1)
{
    int32_t grpId = 1;
    WorkgroupPrioRecorder recorder(grpId);
    EXPECT_EQ(recorder.grpId_, grpId);
    EXPECT_EQ(recorder.restoreByPermission_, false);
}
 
/**
 * @tc.name   : Test GetGrpId
 * @tc.number : GetGrpId_001
 * @tc.desc   : Test GetGrpId when call
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, GetGrpId_001, TestSize.Level1)
{
    WorkgroupPrioRecorder recorder(1);
    recorder.grpId_ = 100;
    EXPECT_EQ(recorder.GetGrpId(), 100);
}
 
/**
 * @tc.name   : Test GetRecorderByGrpId
 * @tc.number : GetRecorderByGrpId_001
 * @tc.desc   : Test GetRecorderByGrpId when grpId exist
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, GetRecorderByGrpId_001, TestSize.Level1)
{
    int32_t grpId = 1;
    auto recorder = std::make_shared<WorkgroupPrioRecorder>(1);
    manager_->workgroupPrioRecorderMap_[grpId] = recorder;
    auto result = manager_->GetRecorderByGrpId(grpId);
    EXPECT_EQ(result, recorder);
}
 
/**
 * @tc.name   : Test GetRecorderByGrpId
 * @tc.number : GetRecorderByGrpId_002
 * @tc.desc   : Test GetRecorderByGrpId when grpId not exist
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, GetRecorderByGrpId_002, TestSize.Level1)
{
    int32_t grpId = 1;
    auto result = manager_->GetRecorderByGrpId(grpId);
    EXPECT_EQ(result, nullptr);
}
 
/**
 * @tc.name   : Test OnWorkgroupChange
 * @tc.number : OnWorkgroupChange_001
 * @tc.desc   : Test OnWorkgroupChange when allowed is true
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, OnWorkgroupChange_001, TestSize.Level1)
{
    AudioWorkgroupChangeInfo info;
    info.pid = 1;
    info.groupId = 1;
    info.startAllowed = true;
 
    manager_->OnWorkgroupChange(info);
 
    // Check if the permission is set correctly
    EXPECT_EQ(manager_->startGroupPermissionMap_[info.pid][info.groupId], info.startAllowed);
}
 
/**
 * @tc.name   : Test OnWorkgroupChange
 * @tc.number : OnWorkgroupChange_002
 * @tc.desc   : Test OnWorkgroupChange when allowed is false
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, OnWorkgroupChange_002, TestSize.Level1)
{
    AudioWorkgroupChangeInfo info;
    info.pid = 1;
    info.groupId = 1;
    info.startAllowed = false;
 
    manager_->OnWorkgroupChange(info);
 
    // Check if the permission is set correctly
    EXPECT_EQ(manager_->startGroupPermissionMap_[info.pid][info.groupId], info.startAllowed);
}
 
/**
 * @tc.name   : Test OnWorkgroupChange
 * @tc.number : OnWorkgroupChange_003
 * @tc.desc   : Test OnWorkgroupChange when recorder is nullptr
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, OnWorkgroupChange_003, TestSize.Level1)
{
    AudioWorkgroupChangeInfo info;
    info.pid = 1;
    info.groupId = 1;
    info.startAllowed = false;
 
    manager_->OnWorkgroupChange(info);
 
    // Check if the permission is set correctly
    EXPECT_EQ(manager_->startGroupPermissionMap_[info.pid][info.groupId], info.startAllowed);
    // Check if the recorder is nullptr
    EXPECT_EQ(manager_->GetRecorderByGrpId(info.groupId), nullptr);
}

/**
 * @tc.name   : Test OnWorkgroupChange API
 * @tc.number : OnWorkgroupChange_004
 * @tc.desc   : Test OnWorkgroupChange interface
 */
HWTEST_F(AudioWorkgroupManagerUnitTest, OnWorkgroupChange_004, TestSize.Level4)
{
    AudioWorkgroupCallbackImpl audioWorkgroupCallbackImpl;
    AudioWorkgroupChangeInfoIpc info;
    audioWorkgroupCallbackImpl.workgroupCb_ = nullptr;
    EXPECT_EQ(audioWorkgroupCallbackImpl.OnWorkgroupChange(info), ERROR);
}
} // namespace AudioStandard
} // namespace OHOS
