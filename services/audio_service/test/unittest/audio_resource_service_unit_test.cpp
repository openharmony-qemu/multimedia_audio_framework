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
#include "ipc_stream_stub.h"
#include "ipc_stream.h"
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

    audioResourceService.audioWorkgroupMap[1][testRtgId] = {workgroup};
    audioResourceService.OnWorkgroupRemoteDied(workgroup, remoteObj);
    EXPECT_EQ(audioResourceService.audioWorkgroupMap[1].size(), 0);
    EXPECT_EQ(audioResourceService.audioWorkgroupMap[1][testRtgId], nullptr);
}

/**
 * @tc.name  : Test deathRecipient
 * @tc.type  : FUNC
 * @tc.number: DeathRecipient_004
 * @tc.desc  : Test OnWorkgroupRemoteDied when audioWorkgroupMap zero
 */
HWTEST(AudioResourceServiceUnitTest, DeathRecipient_004, TestSize.Level0)
{
    std::shared_ptr<AudioWorkgroup> workgroup = std::make_shared<AudioWorkgroup>(testRtgId);
    sptr<IRemoteObject> remoteObj = nullptr;

    audioResourceService.audioWorkgroupMap[1][testRtgId] = {workgroup};
    audioResourceService.OnWorkgroupRemoteDied(workgroup, remoteObj);
    EXPECT_EQ(audioResourceService.audioWorkgroupMap.count(1), 0);
}

/**
 * @tc.name  : Test deathRecipient
 * @tc.type  : FUNC
 * @tc.number: DeathRecipient_005
 * @tc.desc  : Test OnWorkgroupRemoteDied when audioWorkgroupMap empty
 */
HWTEST(AudioResourceServiceUnitTest, DeathRecipient_005, TestSize.Level0)
{
    std::shared_ptr<AudioWorkgroup> workgroup = std::make_shared<AudioWorkgroup>(testRtgId);
    sptr<IRemoteObject> remoteObj = nullptr;

    audioResourceService.OnWorkgroupRemoteDied(workgroup, remoteObj);
    EXPECT_TRUE(audioResourceService.audioWorkgroupMap.empty());
}

/**
 * @tc.name  : Test deathRecipient OnWorkgroupRemoteDied
 * @tc.type  : FUNC
 * @tc.number: DeathRecipient_006
 * @tc.desc  : Test OnWorkgroupRemoteDied find audioWorkgroupMap 
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
} // namespace AudioStandard
} // namespace OHOS