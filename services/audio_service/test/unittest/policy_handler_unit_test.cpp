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
#include "audio_errors.h"
#include "policy_handler.h"
#include "policy_provider_ipc_proxy.h"

using namespace testing::ext;
class PolicyProviderIpcProxy;

namespace OHOS {
namespace AudioStandard {
class PolicyHandlerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class AudioSharedMemoryTest : public AudioSharedMemory {
public:
    uint8_t *GetBase() override { return nullptr; };
    size_t GetSize() override { return 0; };
    int GetFd() override { return 0; };
    std::string GetName() override { return "abc"; };
    bool Marshalling(Parcel &parcel) const override { return true; };
};

void PolicyHandlerUnitTest::SetUpTestCase(void) {}

void PolicyHandlerUnitTest::TearDownTestCase(void) {}

void PolicyHandlerUnitTest::SetUp(void) {}

void PolicyHandlerUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test PolicyHandler API
 * @tc.type  : FUNC
 * @tc.number: PolicyHandler_001
 * @tc.desc  : Test PolicyHandler interface.
 */
HWTEST_F(PolicyHandlerUnitTest, PolicyHandler_001, TestSize.Level1)
{
    auto policyHandler = std::make_shared<PolicyHandler>();
    EXPECT_NE(policyHandler, nullptr);

    std::string dumpString = "";
    policyHandler->iPolicyProvider_ = nullptr;
    policyHandler->policyVolumeMap_ = nullptr;
    policyHandler->volumeVector_ = nullptr;

    policyHandler->Dump(dumpString);
    EXPECT_EQ(dumpString, "PolicyHandler is null...\n");
}

/**
 * @tc.name  : Test PolicyHandler API
 * @tc.type  : FUNC
 * @tc.number: PolicyHandler_002
 * @tc.desc  : Test PolicyHandler interface.
 */
HWTEST_F(PolicyHandlerUnitTest, PolicyHandler_002, TestSize.Level1)
{
    auto policyHandler = std::make_shared<PolicyHandler>();
    EXPECT_NE(policyHandler, nullptr);

    std::string dumpString = "";
    sptr<IRemoteObject> impl = nullptr;
    policyHandler->iPolicyProvider_ = new PolicyProviderIpcProxy(impl);
    EXPECT_NE(policyHandler->iPolicyProvider_, nullptr);
    policyHandler->policyVolumeMap_ = nullptr;
    policyHandler->volumeVector_ = nullptr;

    policyHandler->Dump(dumpString);
    EXPECT_EQ(dumpString, "PolicyHandler is null...\n");
}

/**
 * @tc.name  : Test PolicyHandler API
 * @tc.type  : FUNC
 * @tc.number: PolicyHandler_003
 * @tc.desc  : Test PolicyHandler interface.
 */
HWTEST_F(PolicyHandlerUnitTest, PolicyHandler_003, TestSize.Level1)
{
    auto policyHandler = std::make_shared<PolicyHandler>();
    EXPECT_NE(policyHandler, nullptr);

    std::string dumpString = "";
    sptr<IRemoteObject> impl = nullptr;
    policyHandler->iPolicyProvider_ = new PolicyProviderIpcProxy(impl);
    EXPECT_NE(policyHandler->iPolicyProvider_, nullptr);

    policyHandler->policyVolumeMap_ = std::make_shared<AudioSharedMemoryTest>();
    EXPECT_NE(policyHandler->policyVolumeMap_, nullptr);

    policyHandler->volumeVector_ = nullptr;

    policyHandler->Dump(dumpString);
    EXPECT_EQ(dumpString, "PolicyHandler is null...\n");
}

/**
 * @tc.name  : Test PolicyHandler API
 * @tc.type  : FUNC
 * @tc.number: PolicyHandler_004
 * @tc.desc  : Test PolicyHandler interface.
 */
HWTEST_F(PolicyHandlerUnitTest, PolicyHandler_004, TestSize.Level1)
{
    auto policyHandler = std::make_shared<PolicyHandler>();
    EXPECT_NE(policyHandler, nullptr);

    std::string dumpString = "";
    sptr<IRemoteObject> impl = nullptr;
    policyHandler->iPolicyProvider_ = new PolicyProviderIpcProxy(impl);
    EXPECT_NE(policyHandler->iPolicyProvider_, nullptr);

    policyHandler->policyVolumeMap_ = std::make_shared<AudioSharedMemoryTest>();
    EXPECT_NE(policyHandler->policyVolumeMap_, nullptr);

    Volume volume[IPolicyProvider::GetVolumeVectorSize()];
    policyHandler->volumeVector_ = volume;

    bool sharedAbsVolumeScene = true;
    policyHandler->sharedAbsVolumeScene_ = &sharedAbsVolumeScene;
    policyHandler->Dump(dumpString);
}

/**
 * @tc.name  : Test PolicyHandler API
 * @tc.type  : FUNC
 * @tc.number: PolicyHandler_005
 * @tc.desc  : Test PolicyHandler interface.
 */
HWTEST_F(PolicyHandlerUnitTest, PolicyHandler_005, TestSize.Level1)
{
    auto policyHandler = std::make_shared<PolicyHandler>();
    EXPECT_NE(policyHandler, nullptr);

    std::string dumpString = "";
    sptr<IRemoteObject> impl = nullptr;
    policyHandler->iPolicyProvider_ = new PolicyProviderIpcProxy(impl);
    EXPECT_NE(policyHandler->iPolicyProvider_, nullptr);

    policyHandler->policyVolumeMap_ = std::make_shared<AudioSharedMemoryTest>();
    EXPECT_NE(policyHandler->policyVolumeMap_, nullptr);

    Volume volume[IPolicyProvider::GetVolumeVectorSize()];
    policyHandler->volumeVector_ = volume;
    policyHandler->sharedAbsVolumeScene_ = nullptr;
    policyHandler->Dump(dumpString);
}

/**
 * @tc.name  : Test PolicyHandler API
 * @tc.type  : FUNC
 * @tc.number: PolicyHandler_006
 * @tc.desc  : Test PolicyHandler interface.
 */
HWTEST_F(PolicyHandlerUnitTest, PolicyHandler_006, TestSize.Level1)
{
    auto policyHandler = std::make_shared<PolicyHandler>();
    EXPECT_NE(policyHandler, nullptr);

    sptr<IRemoteObject> impl = nullptr;
    policyHandler->iPolicyProvider_ = new PolicyProviderIpcProxy(impl);
    EXPECT_NE(policyHandler->iPolicyProvider_, nullptr);

    sptr<IPolicyProviderIpc> policyProvider = new PolicyProviderIpcProxy(impl);
    EXPECT_NE(policyProvider, nullptr);

    auto ret = policyHandler->ConfigPolicyProvider(policyProvider);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test PolicyHandler API
 * @tc.type  : FUNC
 * @tc.number: PolicyHandler_007
 * @tc.desc  : Test PolicyHandler interface.
 */
HWTEST_F(PolicyHandlerUnitTest, PolicyHandler_007, TestSize.Level1)
{
    auto policyHandler = std::make_shared<PolicyHandler>();
    EXPECT_NE(policyHandler, nullptr);

    sptr<IRemoteObject> impl = nullptr;
    policyHandler->iPolicyProvider_ = new PolicyProviderIpcProxy(impl);
    EXPECT_NE(policyHandler->iPolicyProvider_, nullptr);

    Volume volume[IPolicyProvider::GetVolumeVectorSize()];
    policyHandler->volumeVector_ = volume;

    AudioVolumeType streamType = AudioStreamType::STREAM_VOICE_CALL;
    DeviceType deviceType = DeviceType::DEVICE_TYPE_INVALID;
    Volume vol;

    auto ret = policyHandler->GetSharedVolume(streamType, deviceType, vol);
    EXPECT_EQ(ret, false);
}
}  // namespace OHOS::AudioStandard
}  // namespace OHOS