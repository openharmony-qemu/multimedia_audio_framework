/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

// AIUT补充用例说明：
// 本文件仅包含现有audio_manager_listener_stub_unit_test.cpp未覆盖的分支/边界条件。
// 现有UT已覆盖：OnCapturerState(无callback)、OnWakeupClose(无callback)、
// OnAudioParameterChange(无callback)、OnDataTransferStateChange(无效callbackId)。
// 本文件补充：callback已注册时的触发分支、OnHdiRouteStateChange、OnMuteStateChange、
// Add/RemoveDataTransferStateChangeCallback的边界条件。

#include "audio_manager_listener_stub_impl.h"
#include "audio_errors.h"
#include "aiut_log.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
class TestAudioParameterCallback : public AudioParameterCallback {
public:
    void OnAudioParameterChange(const std::string networkId, const AudioParamKey key,
        const std::string &condition, const std::string &value) override
    {
        (void)networkId;
        (void)key;
        (void)condition;
        (void)value;
        called_ = true;
    }
    void OnHdiRouteStateChange(const std::string &networkId, bool enable) override
    {
        hdiCalled_ = true;
    }
    bool called_ = false;
    bool hdiCalled_ = false;
};

class TestWakeUpSourceCallback : public WakeUpSourceCallback {
public:
    void OnCapturerState(bool isActive) override
    {
        (void)isActive;
        capturerCalled_ = true;
    }
    void OnWakeupClose() override
    {
        wakeupCalled_ = true;
    }
    bool capturerCalled_ = false;
    bool wakeupCalled_ = false;
};

class TestRendererDataTransferStateChangeCallback : public AudioRendererDataTransferStateChangeCallback {
public:
    void OnDataTransferStateChange(const AudioRendererDataTransferStateChangeInfo &info) override
    {
        (void)info;
        called_ = true;
    }
    void OnMuteStateChange(const int32_t &uid, const uint32_t &sessionId, const bool &isMuted) override
    {
        (void)uid;
        (void)sessionId;
        (void)isMuted;
        muteCalled_ = true;
    }
    bool called_ = false;
    bool muteCalled_ = false;
};
} // namespace

class AudioManagerListenerStubImplAiut : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name  : Test OnAudioParameterChange with callback set
 * @tc.number: AudioManagerListenerStubImpl_OnAudioParameterChange_WithCallback
 * @tc.desc  : Test OnAudioParameterChange when callback is set.
 *             现有UT仅测试无callback时返回SUCCESS，本用例覆盖callback已注册时的触发分支。
 */
HWTEST_F(AudioManagerListenerStubImplAiut,
    AudioManagerListenerStubImpl_OnAudioParameterChange_WithCallback, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto listener = std::make_unique<AudioManagerListenerStubImpl>();
    AIUT_ASSERT_NE_INFO_LOG(listener, nullptr, "listener create failed");
    auto cb = std::make_shared<TestAudioParameterCallback>();
    listener->SetParameterCallback(cb);
    int32_t ret = listener->OnAudioParameterChange("netId", 0, "cond", "val");
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "OnAudioParameterChange should return SUCCESS, ret=%d", ret);
    AIUT_EXPECT_TRUE_INFO_LOG(cb->called_, "OnAudioParameterChange callback should be called, called=%d",
        static_cast<int>(cb->called_));
}

/**
 * @tc.name  : Test OnHdiRouteStateChange with callback
 * @tc.number: AudioManagerListenerStubImpl_OnHdiRouteStateChange_WithCallback
 * @tc.desc  : Test OnHdiRouteStateChange when callback is set.
 *             OnHdiRouteStateChange是现有UT完全未覆盖的方法。
 */
HWTEST_F(AudioManagerListenerStubImplAiut,
    AudioManagerListenerStubImpl_OnHdiRouteStateChange_WithCallback, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto listener = std::make_unique<AudioManagerListenerStubImpl>();
    AIUT_ASSERT_NE_INFO_LOG(listener, nullptr, "listener create failed");
    auto cb = std::make_shared<TestAudioParameterCallback>();
    listener->SetParameterCallback(cb);
    int32_t ret = listener->OnHdiRouteStateChange("netId", true);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "OnHdiRouteStateChange should return SUCCESS, ret=%d", ret);
    AIUT_EXPECT_TRUE_INFO_LOG(cb->hdiCalled_, "OnHdiRouteStateChange callback should be called, called=%d",
        static_cast<int>(cb->hdiCalled_));
}

/**
 * @tc.name  : Test OnHdiRouteStateChange without callback
 * @tc.number: AudioManagerListenerStubImpl_OnHdiRouteStateChange_WithoutCallback
 * @tc.desc  : Test OnHdiRouteStateChange when callback is not set.
 *             覆盖callback为nullptr时的分支（现有UT未覆盖此方法）。
 */
HWTEST_F(AudioManagerListenerStubImplAiut,
    AudioManagerListenerStubImpl_OnHdiRouteStateChange_WithoutCallback, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto listener = std::make_unique<AudioManagerListenerStubImpl>();
    AIUT_ASSERT_NE_INFO_LOG(listener, nullptr, "listener create failed");
    int32_t ret = listener->OnHdiRouteStateChange("netId", false);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "OnHdiRouteStateChange should return SUCCESS without callback, ret=%d", ret);
}

/**
 * @tc.name  : Test OnCapturerState with callback
 * @tc.number: AudioManagerListenerStubImpl_OnCapturerState_WithCallback
 * @tc.desc  : Test OnCapturerState when callback is set.
 *             现有UT仅测试无callback时返回SUCCESS，本用例覆盖callback已注册时的触发分支。
 */
HWTEST_F(AudioManagerListenerStubImplAiut,
    AudioManagerListenerStubImpl_OnCapturerState_WithCallback, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto listener = std::make_unique<AudioManagerListenerStubImpl>();
    AIUT_ASSERT_NE_INFO_LOG(listener, nullptr, "listener create failed");
    auto cb = std::make_shared<TestWakeUpSourceCallback>();
    listener->SetWakeupSourceCallback(cb);
    int32_t ret = listener->OnCapturerState(true);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "OnCapturerState should return SUCCESS, ret=%d", ret);
    AIUT_EXPECT_TRUE_INFO_LOG(cb->capturerCalled_, "OnCapturerState callback should be called, called=%d",
        static_cast<int>(cb->capturerCalled_));
}

/**
 * @tc.name  : Test OnWakeupClose with callback
 * @tc.number: AudioManagerListenerStubImpl_OnWakeupClose_WithCallback
 * @tc.desc  : Test OnWakeupClose when callback is set.
 *             现有UT仅测试无callback时返回SUCCESS，本用例覆盖callback已注册时的触发分支。
 */
HWTEST_F(AudioManagerListenerStubImplAiut,
    AudioManagerListenerStubImpl_OnWakeupClose_WithCallback, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto listener = std::make_unique<AudioManagerListenerStubImpl>();
    AIUT_ASSERT_NE_INFO_LOG(listener, nullptr, "listener create failed");
    auto cb = std::make_shared<TestWakeUpSourceCallback>();
    listener->SetWakeupSourceCallback(cb);
    int32_t ret = listener->OnWakeupClose();
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "OnWakeupClose should return SUCCESS, ret=%d", ret);
    AIUT_EXPECT_TRUE_INFO_LOG(cb->wakeupCalled_, "OnWakeupClose callback should be called, called=%d",
        static_cast<int>(cb->wakeupCalled_));
}

/**
 * @tc.name  : Test OnDataTransferStateChange with registered callback
 * @tc.number: AudioManagerListenerStubImpl_OnDataTransferStateChange_WithRegisteredCallback
 * @tc.desc  : Test OnDataTransferStateChange when callback is registered.
 *             现有UT仅测试无效callbackId(123)，本用例覆盖已注册callback的触发分支。
 */
HWTEST_F(AudioManagerListenerStubImplAiut,
    AudioManagerListenerStubImpl_OnDataTransferStateChange_WithRegisteredCallback, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto listener = std::make_unique<AudioManagerListenerStubImpl>();
    AIUT_ASSERT_NE_INFO_LOG(listener, nullptr, "listener create failed");
    DataTransferMonitorParam param;
    auto cb = std::make_shared<TestRendererDataTransferStateChangeCallback>();
    int32_t callbackId = listener->AddDataTransferStateChangeCallback(param, cb);
    AIUT_INFO_LOG("AddDataTransferStateChangeCallback callbackId=%d", callbackId);
    AudioRendererDataTransferStateChangeInfo info;
    int32_t ret = listener->OnDataTransferStateChange(callbackId, info);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "OnDataTransferStateChange should return SUCCESS, ret=%d", ret);
    AIUT_EXPECT_TRUE_INFO_LOG(cb->called_, "OnDataTransferStateChange callback should be called, called=%d",
        static_cast<int>(cb->called_));
}

/**
 * @tc.name  : Test OnMuteStateChange with registered callback
 * @tc.number: AudioManagerListenerStubImpl_OnMuteStateChange_WithRegisteredCallback
 * @tc.desc  : Test OnMuteStateChange when callback is registered.
 *             OnMuteStateChange是现有UT完全未覆盖的方法。
 */
HWTEST_F(AudioManagerListenerStubImplAiut,
    AudioManagerListenerStubImpl_OnMuteStateChange_WithRegisteredCallback, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto listener = std::make_unique<AudioManagerListenerStubImpl>();
    AIUT_ASSERT_NE_INFO_LOG(listener, nullptr, "listener create failed");
    DataTransferMonitorParam param;
    auto cb = std::make_shared<TestRendererDataTransferStateChangeCallback>();
    int32_t callbackId = listener->AddDataTransferStateChangeCallback(param, cb);
    AIUT_INFO_LOG("AddDataTransferStateChangeCallback callbackId=%d", callbackId);
    int32_t ret = listener->OnMuteStateChange(callbackId, 100, 1, true);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "OnMuteStateChange should return SUCCESS, ret=%d", ret);
    AIUT_EXPECT_TRUE_INFO_LOG(cb->muteCalled_, "OnMuteStateChange callback should be called, called=%d",
        static_cast<int>(cb->muteCalled_));
}

/**
 * @tc.name  : Test OnMuteStateChange with invalid callbackId
 * @tc.number: AudioManagerListenerStubImpl_OnMuteStateChange_InvalidCallbackId
 * @tc.desc  : Test OnMuteStateChange when callbackId is not registered.
 *             覆盖callbackId未注册时的分支（现有UT未覆盖此方法）。
 */
HWTEST_F(AudioManagerListenerStubImplAiut,
    AudioManagerListenerStubImpl_OnMuteStateChange_InvalidCallbackId, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto listener = std::make_unique<AudioManagerListenerStubImpl>();
    AIUT_ASSERT_NE_INFO_LOG(listener, nullptr, "listener create failed");
    int32_t ret = listener->OnMuteStateChange(999, 100, 1, true);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "OnMuteStateChange should return SUCCESS for invalid id, ret=%d", ret);
}

/**
 * @tc.name  : Test AddDataTransferStateChangeCallback max count
 * @tc.number: AudioManagerListenerStubImpl_AddDataTransferStateChangeCallback_MaxCountExceeded
 * @tc.desc  : Test AddDataTransferStateChangeCallback returns -1 when max count exceeded.
 *             AddDataTransferStateChangeCallback是现有UT完全未覆盖的方法。
 */
HWTEST_F(AudioManagerListenerStubImplAiut,
    AudioManagerListenerStubImpl_AddDataTransferStateChangeCallback_MaxCountExceeded, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto listener = std::make_unique<AudioManagerListenerStubImpl>();
    AIUT_ASSERT_NE_INFO_LOG(listener, nullptr, "listener create failed");
    DataTransferMonitorParam param;
    for (int i = 0; i < 10; ++i) {
        auto cb = std::make_shared<TestRendererDataTransferStateChangeCallback>();
        listener->AddDataTransferStateChangeCallback(param, cb);
    }
    auto cb11 = std::make_shared<TestRendererDataTransferStateChangeCallback>();
    int32_t callbackId = listener->AddDataTransferStateChangeCallback(param, cb11);
    AIUT_EXPECT_EQ_INFO_LOG(callbackId, -1, "AddDataTransferStateChangeCallback should return -1 when max exceeded, "
        "callbackId=%d", callbackId);
}

/**
 * @tc.name  : Test AddDataTransferStateChangeCallback duplicate
 * @tc.number: AudioManagerListenerStubImpl_AddDataTransferStateChangeCallback_Duplicate
 * @tc.desc  : Test AddDataTransferStateChangeCallback returns same id for duplicate.
 *             覆盖重复注册时返回相同id的分支。
 */
HWTEST_F(AudioManagerListenerStubImplAiut,
    AudioManagerListenerStubImpl_AddDataTransferStateChangeCallback_Duplicate, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto listener = std::make_unique<AudioManagerListenerStubImpl>();
    AIUT_ASSERT_NE_INFO_LOG(listener, nullptr, "listener create failed");
    DataTransferMonitorParam param;
    auto cb = std::make_shared<TestRendererDataTransferStateChangeCallback>();
    int32_t id1 = listener->AddDataTransferStateChangeCallback(param, cb);
    int32_t id2 = listener->AddDataTransferStateChangeCallback(param, cb);
    AIUT_EXPECT_EQ_INFO_LOG(id1, id2, "AddDataTransferStateChangeCallback should return same id for duplicate, "
        "id1=%d, id2=%d", id1, id2);
}

/**
 * @tc.name  : Test RemoveDataTransferStateChangeCallback
 * @tc.number: AudioManagerListenerStubImpl_RemoveDataTransferStateChangeCallback_Registered
 * @tc.desc  : Test RemoveDataTransferStateChangeCallback removes registered callback.
 *             RemoveDataTransferStateChangeCallback是现有UT完全未覆盖的方法。
 */
HWTEST_F(AudioManagerListenerStubImplAiut,
    AudioManagerListenerStubImpl_RemoveDataTransferStateChangeCallback_Registered, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto listener = std::make_unique<AudioManagerListenerStubImpl>();
    AIUT_ASSERT_NE_INFO_LOG(listener, nullptr, "listener create failed");
    DataTransferMonitorParam param;
    auto cb = std::make_shared<TestRendererDataTransferStateChangeCallback>();
    listener->AddDataTransferStateChangeCallback(param, cb);
    auto ids = listener->RemoveDataTransferStateChangeCallback(cb);
    AIUT_EXPECT_FALSE_INFO_LOG(ids.empty(), "RemoveDataTransferStateChangeCallback should return non-empty ids, "
        "size=%zu", ids.size());
}

/**
 * @tc.name  : Test RemoveDataTransferStateChangeCallback not registered
 * @tc.number: AudioManagerListenerStubImpl_RemoveDataTransferStateChangeCallback_NotRegistered
 * @tc.desc  : Test RemoveDataTransferStateChangeCallback returns empty for not registered callback.
 *             覆盖未注册callback移除时返回空列表的分支。
 */
HWTEST_F(AudioManagerListenerStubImplAiut,
    AudioManagerListenerStubImpl_RemoveDataTransferStateChangeCallback_NotRegistered, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto listener = std::make_unique<AudioManagerListenerStubImpl>();
    AIUT_ASSERT_NE_INFO_LOG(listener, nullptr, "listener create failed");
    auto cb = std::make_shared<TestRendererDataTransferStateChangeCallback>();
    auto ids = listener->RemoveDataTransferStateChangeCallback(cb);
    AIUT_EXPECT_TRUE_INFO_LOG(ids.empty(), "RemoveDataTransferStateChangeCallback should return empty for not "
        "registered, size=%zu", ids.size());
}

} // namespace AudioStandard
} // namespace OHOS
