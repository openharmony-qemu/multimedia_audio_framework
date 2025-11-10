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
#include "policy_provider_stub.h"
#include "audio_service.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class MockPolicyProvider : public IPolicyProvider {
public:
    MockPolicyProvider() {};
    ~MockPolicyProvider() {};

    int32_t GetProcessDeviceInfo(const AudioProcessConfig &config, bool lockFlag,
        AudioDeviceDescriptor &deviceInfo) override;

    int32_t InitSharedVolume(std::shared_ptr<AudioSharedMemory> &buffer) override;

    int32_t NotifyCapturerAdded(AudioCapturerInfo capturerInfo, AudioStreamInfo streamInfo,
        uint32_t sessionId) override;

    int32_t NotifyWakeUpCapturerRemoved() override;

    bool IsAbsVolumeSupported() override;

    int32_t OffloadGetRenderPosition(uint32_t &delayValue, uint64_t &sendDataSize, uint32_t &timeStamp) override;

    int32_t NearlinkGetRenderPosition(uint32_t &delayValue) override;

    int32_t GetAndSaveClientType(uint32_t uid, const std::string &bundleName) override;

    int32_t GetMaxRendererInstances() override;

    int32_t NotifyCapturerRemoved(uint64_t sessionId) override;

    int32_t LoadModernInnerCapSink(int32_t innerCapId) override;

    int32_t UnloadModernInnerCapSink(int32_t innerCapId) override;

    int32_t ClearAudioFocusBySessionID(const int32_t &sessionID) override;

    int32_t CaptureConcurrentCheck(const uint32_t &sessionID) override;

    std::shared_ptr<AudioSharedMemory> policyVolumeMap_ = nullptr;
};

int32_t MockPolicyProvider::GetProcessDeviceInfo(const AudioProcessConfig &config, bool lockFlag,
    AudioDeviceDescriptor &deviceInfo)
{
    return SUCCESS;
}

int32_t MockPolicyProvider::InitSharedVolume(std::shared_ptr<AudioSharedMemory> &buffer)
{
    return SUCCESS;
}

int32_t MockPolicyProvider::NotifyCapturerAdded(AudioCapturerInfo capturerInfo, AudioStreamInfo streamInfo,
    uint32_t sessionId)
{
    return SUCCESS;
}

int32_t MockPolicyProvider::NotifyWakeUpCapturerRemoved()
{
    return SUCCESS;
}

bool MockPolicyProvider::IsAbsVolumeSupported()
{
    return SUCCESS;
}

int32_t MockPolicyProvider::OffloadGetRenderPosition(uint32_t &delayValue, uint64_t &sendDataSize, uint32_t &timeStamp)
{
    return SUCCESS;
}

int32_t MockPolicyProvider::NearlinkGetRenderPosition(uint32_t &delayValue)
{
    return SUCCESS;
}

int32_t MockPolicyProvider::GetAndSaveClientType(uint32_t uid, const std::string &bundleName)
{
    return SUCCESS;
}

int32_t MockPolicyProvider::GetMaxRendererInstances()
{
    return SUCCESS;
}

int32_t MockPolicyProvider::NotifyCapturerRemoved(uint64_t sessionId)
{
    return SUCCESS;
}

int32_t MockPolicyProvider::LoadModernInnerCapSink(int32_t innerCapId)
{
    return SUCCESS;
}

int32_t MockPolicyProvider::UnloadModernInnerCapSink(int32_t innerCapId)
{
    return SUCCESS;
}

int32_t MockPolicyProvider::ClearAudioFocusBySessionID(const int32_t &sessionID)
{
    return SUCCESS;
}

int32_t MockPolicyProvider::CaptureConcurrentCheck(const uint32_t &sessionID)
{
    return SUCCESS;
}

class PolicyProviderStubUnitTest : public ::testing::Test {
public:
    void SetUp();
    void TearDown();
};

void PolicyProviderStubUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void PolicyProviderStubUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name  : Test PolicyProviderStub.
 * @tc.type  : FUNC
 * @tc.number: PolicyProviderStub_001
 * @tc.desc  : Test PolicyProviderStub::OnRemoteRequest().
 */
HWTEST_F(PolicyProviderStubUnitTest, PolicyProviderStub_001, TestSize.Level1)
{
    MockPolicyProvider mockProvider;
    auto policyProviderStub = std::make_shared<PolicyProviderWrapper>(&mockProvider);
    EXPECT_NE(policyProviderStub, nullptr);

    uint32_t code = IPolicyProviderIpc::IPolicyProviderMsg::GET_DEVICE_INFO;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IPolicyProviderIpc::GetDescriptor());

    bool ret = policyProviderStub->CheckInterfaceToken(data);

    EXPECT_EQ(ret, true);

    ret = policyProviderStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test PolicyProviderStub.
 * @tc.type  : FUNC
 * @tc.number: PolicyProviderStub_002
 * @tc.desc  : Test PolicyProviderStub::OnRemoteRequest().
 */
HWTEST_F(PolicyProviderStubUnitTest, PolicyProviderStub_002, TestSize.Level1)
{
    MockPolicyProvider mockProvider;
    auto policyProviderStub = std::make_shared<PolicyProviderWrapper>(&mockProvider);
    EXPECT_NE(policyProviderStub, nullptr);

    uint32_t code = IPolicyProviderIpc::IPolicyProviderMsg::INIT_VOLUME_MAP;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IPolicyProviderIpc::GetDescriptor());

    bool ret = policyProviderStub->CheckInterfaceToken(data);

    EXPECT_EQ(ret, true);

    ret = policyProviderStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test PolicyProviderStub.
 * @tc.type  : FUNC
 * @tc.number: PolicyProviderStub_003
 * @tc.desc  : Test PolicyProviderStub::OnRemoteRequest().
 */
HWTEST_F(PolicyProviderStubUnitTest, PolicyProviderStub_003, TestSize.Level1)
{
    MockPolicyProvider mockProvider;
    auto policyProviderStub = std::make_shared<PolicyProviderWrapper>(&mockProvider);
    EXPECT_NE(policyProviderStub, nullptr);

    uint32_t code = IPolicyProviderIpc::IPolicyProviderMsg::SET_WAKEUP_ADUIO_CAPTURER;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IPolicyProviderIpc::GetDescriptor());

    bool ret = policyProviderStub->CheckInterfaceToken(data);

    EXPECT_EQ(ret, true);

    ret = policyProviderStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test PolicyProviderStub.
 * @tc.type  : FUNC
 * @tc.number: PolicyProviderStub_004
 * @tc.desc  : Test PolicyProviderStub::OnRemoteRequest().
 */
HWTEST_F(PolicyProviderStubUnitTest, PolicyProviderStub_004, TestSize.Level1)
{
    MockPolicyProvider mockProvider;
    auto policyProviderStub = std::make_shared<PolicyProviderWrapper>(&mockProvider);
    EXPECT_NE(policyProviderStub, nullptr);

    uint32_t code = IPolicyProviderIpc::IPolicyProviderMsg::SET_AUDIO_CAPTURER;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IPolicyProviderIpc::GetDescriptor());

    bool ret = policyProviderStub->CheckInterfaceToken(data);

    EXPECT_EQ(ret, true);

    ret = policyProviderStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test PolicyProviderStub.
 * @tc.type  : FUNC
 * @tc.number: PolicyProviderStub_005
 * @tc.desc  : Test PolicyProviderStub::OnRemoteRequest().
 */
HWTEST_F(PolicyProviderStubUnitTest, PolicyProviderStub_005, TestSize.Level1)
{
    MockPolicyProvider mockProvider;
    auto policyProviderStub = std::make_shared<PolicyProviderWrapper>(&mockProvider);
    EXPECT_NE(policyProviderStub, nullptr);

    uint32_t code = IPolicyProviderIpc::IPolicyProviderMsg::REMOVE_WAKEUP_CAPUTER;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IPolicyProviderIpc::GetDescriptor());

    bool ret = policyProviderStub->CheckInterfaceToken(data);

    EXPECT_EQ(ret, true);

    ret = policyProviderStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test PolicyProviderStub.
 * @tc.type  : FUNC
 * @tc.number: PolicyProviderStub_006
 * @tc.desc  : Test PolicyProviderStub::OnRemoteRequest().
 */
HWTEST_F(PolicyProviderStubUnitTest, PolicyProviderStub_006, TestSize.Level1)
{
    MockPolicyProvider mockProvider;
    auto policyProviderStub = std::make_shared<PolicyProviderWrapper>(&mockProvider);
    EXPECT_NE(policyProviderStub, nullptr);

    uint32_t code = IPolicyProviderIpc::IPolicyProviderMsg::IS_ABS_VOLUME_SUPPORTED;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IPolicyProviderIpc::GetDescriptor());

    bool ret = policyProviderStub->CheckInterfaceToken(data);

    EXPECT_EQ(ret, true);

    ret = policyProviderStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test PolicyProviderStub.
 * @tc.type  : FUNC
 * @tc.number: PolicyProviderStub_007
 * @tc.desc  : Test PolicyProviderStub::OnRemoteRequest().
 */
HWTEST_F(PolicyProviderStubUnitTest, PolicyProviderStub_007, TestSize.Level1)
{
    MockPolicyProvider mockProvider;
    auto policyProviderStub = std::make_shared<PolicyProviderWrapper>(&mockProvider);
    EXPECT_NE(policyProviderStub, nullptr);

    uint32_t code = IPolicyProviderIpc::IPolicyProviderMsg::OFFLOAD_GET_RENDER_POSITION;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IPolicyProviderIpc::GetDescriptor());

    bool ret = policyProviderStub->CheckInterfaceToken(data);

    EXPECT_EQ(ret, true);

    ret = policyProviderStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test PolicyProviderStub.
 * @tc.type  : FUNC
 * @tc.number: PolicyProviderStub_008
 * @tc.desc  : Test PolicyProviderStub::OnRemoteRequest().
 */
HWTEST_F(PolicyProviderStubUnitTest, PolicyProviderStub_008, TestSize.Level1)
{
    MockPolicyProvider mockProvider;
    auto policyProviderStub = std::make_shared<PolicyProviderWrapper>(&mockProvider);
    EXPECT_NE(policyProviderStub, nullptr);

    uint32_t code = IPolicyProviderIpc::IPolicyProviderMsg::GET_AND_SAVE_CLIENT_TYPE;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IPolicyProviderIpc::GetDescriptor());

    bool ret = policyProviderStub->CheckInterfaceToken(data);

    EXPECT_EQ(ret, true);

    ret = policyProviderStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test PolicyProviderStub.
 * @tc.type  : FUNC
 * @tc.number: PolicyProviderStub_009
 * @tc.desc  : Test PolicyProviderStub::OnRemoteRequest().
 */
HWTEST_F(PolicyProviderStubUnitTest, PolicyProviderStub_009, TestSize.Level1)
{
    MockPolicyProvider mockProvider;
    auto policyProviderStub = std::make_shared<PolicyProviderWrapper>(&mockProvider);
    EXPECT_NE(policyProviderStub, nullptr);

    uint32_t code = IPolicyProviderIpc::IPolicyProviderMsg::GET_MAX_RENDERER_INSTANCES;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IPolicyProviderIpc::GetDescriptor());

    bool ret = policyProviderStub->CheckInterfaceToken(data);

    EXPECT_EQ(ret, true);

    ret = policyProviderStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test PolicyProviderStub.
 * @tc.type  : FUNC
 * @tc.number: PolicyProviderStub_010
 * @tc.desc  : Test PolicyProviderStub::OnRemoteRequest().
 */
HWTEST_F(PolicyProviderStubUnitTest, PolicyProviderStub_010, TestSize.Level1)
{
    MockPolicyProvider mockProvider;
    auto policyProviderStub = std::make_shared<PolicyProviderWrapper>(&mockProvider);
    EXPECT_NE(policyProviderStub, nullptr);

    uint32_t code = IPolicyProviderIpc::IPolicyProviderMsg::ACTIVATE_CONCURRENCY_FROM_SERVER;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IPolicyProviderIpc::GetDescriptor());

    bool ret = policyProviderStub->CheckInterfaceToken(data);

    EXPECT_EQ(ret, true);

    ret = policyProviderStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test PolicyProviderStub.
 * @tc.type  : FUNC
 * @tc.number: PolicyProviderStub_011
 * @tc.desc  : Test PolicyProviderStub::OnRemoteRequest().
 */
HWTEST_F(PolicyProviderStubUnitTest, PolicyProviderStub_011, TestSize.Level1)
{
    MockPolicyProvider mockProvider;
    auto policyProviderStub = std::make_shared<PolicyProviderWrapper>(&mockProvider);
    EXPECT_NE(policyProviderStub, nullptr);

    uint32_t code = IPolicyProviderIpc::IPolicyProviderMsg::REMOVE_AUDIO_CAPTURER;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IPolicyProviderIpc::GetDescriptor());

    bool ret = policyProviderStub->CheckInterfaceToken(data);

    EXPECT_EQ(ret, true);

    ret = policyProviderStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test PolicyProviderStub.
 * @tc.type  : FUNC
 * @tc.number: PolicyProviderStub_012
 * @tc.desc  : Test PolicyProviderStub::OnRemoteRequest().
 */
HWTEST_F(PolicyProviderStubUnitTest, PolicyProviderStub_012, TestSize.Level1)
{
    MockPolicyProvider mockProvider;
    auto policyProviderStub = std::make_shared<PolicyProviderWrapper>(&mockProvider);
    EXPECT_NE(policyProviderStub, nullptr);

    uint32_t code = IPolicyProviderIpc::IPolicyProviderMsg::SET_DEFAULT_OUTPUT_DEVICE;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IPolicyProviderIpc::GetDescriptor());

    bool ret = policyProviderStub->CheckInterfaceToken(data);

    EXPECT_EQ(ret, true);

    ret = policyProviderStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test PolicyProviderStub.
 * @tc.type  : FUNC
 * @tc.number: PolicyProviderStub_013
 * @tc.desc  : Test PolicyProviderStub::OnRemoteRequest().
 */
HWTEST_F(PolicyProviderStubUnitTest, PolicyProviderStub_013, TestSize.Level1)
{
    MockPolicyProvider mockProvider;
    auto policyProviderStub = std::make_shared<PolicyProviderWrapper>(&mockProvider);
    EXPECT_NE(policyProviderStub, nullptr);

    uint32_t code = IPolicyProviderIpc::IPolicyProviderMsg::LOAD_MODERN_INNER_CAPTURE_SINK;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IPolicyProviderIpc::GetDescriptor());

    bool ret = policyProviderStub->CheckInterfaceToken(data);

    EXPECT_EQ(ret, true);

    ret = policyProviderStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test PolicyProviderStub.
 * @tc.type  : FUNC
 * @tc.number: PolicyProviderStub_014
 * @tc.desc  : Test PolicyProviderStub::OnRemoteRequest().
 */
HWTEST_F(PolicyProviderStubUnitTest, PolicyProviderStub_014, TestSize.Level1)
{
    MockPolicyProvider mockProvider;
    auto policyProviderStub = std::make_shared<PolicyProviderWrapper>(&mockProvider);
    EXPECT_NE(policyProviderStub, nullptr);

    uint32_t code = IPolicyProviderIpc::IPolicyProviderMsg::UNLOAD_MODERN_INNER_CAPTURE_SINK;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IPolicyProviderIpc::GetDescriptor());

    bool ret = policyProviderStub->CheckInterfaceToken(data);

    EXPECT_EQ(ret, true);

    ret = policyProviderStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test PolicyProviderStub.
 * @tc.type  : FUNC
 * @tc.number: PolicyProviderStub_015
 * @tc.desc  : Test PolicyProviderStub::OnRemoteRequest().
 */
HWTEST_F(PolicyProviderStubUnitTest, PolicyProviderStub_015, TestSize.Level1)
{
    MockPolicyProvider mockProvider;
    auto policyProviderStub = std::make_shared<PolicyProviderWrapper>(&mockProvider);
    EXPECT_NE(policyProviderStub, nullptr);

    uint32_t code = IPolicyProviderIpc::IPolicyProviderMsg::POLICY_PROVIDER_MAX_MSG;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IPolicyProviderIpc::GetDescriptor());

    bool ret = policyProviderStub->CheckInterfaceToken(data);

    EXPECT_EQ(ret, true);

    ret = policyProviderStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test PolicyProviderStub.
 * @tc.type  : FUNC
 * @tc.number: PolicyProviderStub_016
 * @tc.desc  : Test PolicyProviderStub::OnRemoteRequest().
 */
HWTEST_F(PolicyProviderStubUnitTest, PolicyProviderStub_016, TestSize.Level1)
{
    MockPolicyProvider mockProvider;
    auto policyProviderStub = std::make_shared<PolicyProviderWrapper>(&mockProvider);
    EXPECT_NE(policyProviderStub, nullptr);

    uint32_t code = (IPolicyProviderIpc::IPolicyProviderMsg)20;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IPolicyProviderIpc::GetDescriptor());

    bool ret = policyProviderStub->CheckInterfaceToken(data);

    EXPECT_EQ(ret, true);

    ret = policyProviderStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test PolicyProviderStub.
 * @tc.type  : FUNC
 * @tc.number: PolicyProviderStub_017
 * @tc.desc  : Test PolicyProviderStub::HandleInitSharedVolume().
 */
HWTEST_F(PolicyProviderStubUnitTest, PolicyProviderStub_017, TestSize.Level1)
{
    MockPolicyProvider mockProvider;
    auto policyProviderStub = std::make_shared<PolicyProviderWrapper>(&mockProvider);
    EXPECT_NE(policyProviderStub, nullptr);

    MessageParcel data;
    MessageParcel reply;

    data.WriteInterfaceToken(IPolicyProviderIpc::GetDescriptor());

    bool ret = policyProviderStub->CheckInterfaceToken(data);
    EXPECT_EQ(ret, true);

    ret = policyProviderStub->HandleInitSharedVolume(data, reply);
    EXPECT_EQ(ret, true);
}
}
}