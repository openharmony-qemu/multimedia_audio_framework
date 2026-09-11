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

#include "audio_engine_client_manager.h"
#include "audio_errors.h"
#include "aiut_log.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
class TestAudioOutputPipeCallback : public AudioOutputPipeCallback {
public:
    void OnOutputPipeChange(AudioPipeChangeType changeType,
        const std::shared_ptr<AudioOutputPipeInfo> &changedPipeInfo) override
    {
        (void)changeType;
        (void)changedPipeInfo;
        called_ = true;
    }
    bool called_ = false;
};

class TestAudioInputPipeCallback : public AudioInputPipeCallback {
public:
    void OnInputPipeChange(AudioPipeChangeType changeType,
        const std::shared_ptr<AudioInputPipeInfo> &changedPipeInfo) override
    {
        called_ = true;
    }
    bool called_ = false;
};
} // namespace

class AudioEngineClientManagerAiut : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name  : Test CallbackHandle AddOutputPipeChangeCallback with nullptr
 * @tc.number: CallbackHandle_AddOutputPipeChangeCallback_WithNullptr
 * @tc.desc  : Test AddOutputPipeChangeCallback returns false when callback is nullptr.
 */
HWTEST_F(AudioEngineClientManagerAiut, CallbackHandle_AddOutputPipeChangeCallback_WithNullptr, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto handle = new AudioEngineClientManager::CallbackHandle();
    AIUT_ASSERT_NE_INFO_LOG(handle, nullptr, "CallbackHandle create failed");
    std::shared_ptr<AudioOutputPipeCallback> nullCb = nullptr;
    bool result = handle->AddOutputPipeChangeCallback(nullCb);
    AIUT_EXPECT_FALSE_INFO_LOG(result, "AddOutputPipeChangeCallback should return false for nullptr, ret=%d", result);
    delete handle;
}

/**
 * @tc.name  : Test CallbackHandle AddOutputPipeChangeCallback duplicate
 * @tc.number: CallbackHandle_AddOutputPipeChangeCallback_Duplicate
 * @tc.desc  : Test AddOutputPipeChangeCallback returns false for duplicate callback.
 */
HWTEST_F(AudioEngineClientManagerAiut, CallbackHandle_AddOutputPipeChangeCallback_Duplicate, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto handle = new AudioEngineClientManager::CallbackHandle();
    AIUT_ASSERT_NE_INFO_LOG(handle, nullptr, "CallbackHandle create failed");
    std::shared_ptr<AudioOutputPipeCallback> cb = std::make_shared<TestAudioOutputPipeCallback>();
    handle->AddOutputPipeChangeCallback(cb);
    bool result = handle->AddOutputPipeChangeCallback(cb);
    AIUT_EXPECT_FALSE_INFO_LOG(result, "AddOutputPipeChangeCallback should return false for duplicate, result=%d",
        result);
    delete handle;
}

/**
 * @tc.name  : Test CallbackHandle AddOutputPipeChangeCallback first callback
 * @tc.number: CallbackHandle_AddOutputPipeChangeCallback_FirstCallback
 * @tc.desc  : Test AddOutputPipeChangeCallback returns true for first callback (0 to 1 case).
 */
HWTEST_F(AudioEngineClientManagerAiut, CallbackHandle_AddOutputPipeChangeCallback_FirstCallback, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto handle = new AudioEngineClientManager::CallbackHandle();
    AIUT_ASSERT_NE_INFO_LOG(handle, nullptr, "CallbackHandle create failed");
    std::shared_ptr<AudioOutputPipeCallback> cb = std::make_shared<TestAudioOutputPipeCallback>();
    bool result = handle->AddOutputPipeChangeCallback(cb);
    AIUT_EXPECT_TRUE_INFO_LOG(result, "AddOutputPipeChangeCallback should return true for first, result=%d", result);
    delete handle;
}

/**
 * @tc.name  : Test CallbackHandle RemoveOutputPipeChangeCallback with nullptr
 * @tc.number: CallbackHandle_RemoveOutputPipeChangeCallback_WithNullptr
 * @tc.desc  : Test RemoveOutputPipeChangeCallback clears all and returns true for nullptr.
 */
HWTEST_F(AudioEngineClientManagerAiut, CallbackHandle_RemoveOutputPipeChangeCallback_WithNullptr, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto handle = new AudioEngineClientManager::CallbackHandle();
    AIUT_ASSERT_NE_INFO_LOG(handle, nullptr, "CallbackHandle create failed");
    std::shared_ptr<AudioOutputPipeCallback> cb1 = std::make_shared<TestAudioOutputPipeCallback>();
    std::shared_ptr<AudioOutputPipeCallback> cb2 = std::make_shared<TestAudioOutputPipeCallback>();
    handle->AddOutputPipeChangeCallback(cb1);
    handle->AddOutputPipeChangeCallback(cb2);
    std::shared_ptr<AudioOutputPipeCallback> nullCb = nullptr;
    bool result = handle->RemoveOutputPipeChangeCallback(nullCb);
    AIUT_EXPECT_TRUE_INFO_LOG(result, "RemoveOutputPipeChangeCallback should return true for nullptr, result=%d",
        result);
    delete handle;
}

/**
 * @tc.name  : Test CallbackHandle RemoveOutputPipeChangeCallback not found
 * @tc.number: CallbackHandle_RemoveOutputPipeChangeCallback_NotFound
 * @tc.desc  : Test RemoveOutputPipeChangeCallback returns false when callback not added.
 */
HWTEST_F(AudioEngineClientManagerAiut, CallbackHandle_RemoveOutputPipeChangeCallback_NotFound, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto handle = new AudioEngineClientManager::CallbackHandle();
    AIUT_ASSERT_NE_INFO_LOG(handle, nullptr, "CallbackHandle create failed");
    std::shared_ptr<AudioOutputPipeCallback> cb = std::make_shared<TestAudioOutputPipeCallback>();
    bool result = handle->RemoveOutputPipeChangeCallback(cb);
    AIUT_EXPECT_FALSE_INFO_LOG(result, "RemoveOutputPipeChangeCallback should return false for not added, result=%d",
        result);
    delete handle;
}

/**
 * @tc.name  : Test CallbackHandle RemoveOutputPipeChangeCallback last callback
 * @tc.number: CallbackHandle_RemoveOutputPipeChangeCallback_LastCallback
 * @tc.desc  : Test RemoveOutputPipeChangeCallback returns true for last callback (1 to 0 case).
 */
HWTEST_F(AudioEngineClientManagerAiut, CallbackHandle_RemoveOutputPipeChangeCallback_LastCallback, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto handle = new AudioEngineClientManager::CallbackHandle();
    AIUT_ASSERT_NE_INFO_LOG(handle, nullptr, "CallbackHandle create failed");
    std::shared_ptr<AudioOutputPipeCallback> cb = std::make_shared<TestAudioOutputPipeCallback>();
    handle->AddOutputPipeChangeCallback(cb);
    bool result = handle->RemoveOutputPipeChangeCallback(cb);
    AIUT_EXPECT_TRUE_INFO_LOG(result, "RemoveOutputPipeChangeCallback should return true for last, result=%d", result);
    delete handle;
}

/**
 * @tc.name  : Test CallbackHandle IsOutputPipeChangeEnable
 * @tc.number: CallbackHandle_IsOutputPipeChangeEnable_EmptyAndNonEmpty
 * @tc.desc  : Test IsOutputPipeChangeEnable returns false when empty and true when non-empty.
 */
HWTEST_F(AudioEngineClientManagerAiut, CallbackHandle_IsOutputPipeChangeEnable_EmptyAndNonEmpty, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto handle = new AudioEngineClientManager::CallbackHandle();
    AIUT_ASSERT_NE_INFO_LOG(handle, nullptr, "CallbackHandle create failed");
    bool result = handle->IsOutputPipeChangeEnable();
    AIUT_EXPECT_FALSE_INFO_LOG(result, "IsOutputPipeChangeEnable should be false when empty, result=%d", result);
    std::shared_ptr<AudioOutputPipeCallback> cb = std::make_shared<TestAudioOutputPipeCallback>();
    handle->AddOutputPipeChangeCallback(cb);
    result = handle->IsOutputPipeChangeEnable();
    AIUT_EXPECT_TRUE_INFO_LOG(result, "IsOutputPipeChangeEnable should be true when non-empty, result=%d", result);
    delete handle;
}

/**
 * @tc.name  : Test CallbackHandle OnOutputPipeChange
 * @tc.number: CallbackHandle_OnOutputPipeChange_NotifiesCallbacks
 * @tc.desc  : Test OnOutputPipeChange notifies registered callbacks.
 */
HWTEST_F(AudioEngineClientManagerAiut, CallbackHandle_OnOutputPipeChange_NotifiesCallbacks, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto handle = new AudioEngineClientManager::CallbackHandle();
    AIUT_ASSERT_NE_INFO_LOG(handle, nullptr, "CallbackHandle create failed");
    auto testCb = std::make_shared<TestAudioOutputPipeCallback>();
    std::shared_ptr<AudioOutputPipeCallback> cb = testCb;
    handle->AddOutputPipeChangeCallback(cb);
    auto pipeInfo = std::make_shared<AudioOutputPipeInfo>();
    int32_t ret = handle->OnOutputPipeChange(0, pipeInfo);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "OnOutputPipeChange should return SUCCESS, ret=%d", ret);
    AIUT_EXPECT_TRUE_INFO_LOG(testCb->called_, "OnOutputPipeChange callback should be called, called=%d",
        static_cast<int>(testCb->called_));
    delete handle;
}

/**
 * @tc.name  : Test CallbackHandle AddInputPipeChangeCallback with nullptr
 * @tc.number: CallbackHandle_AddInputPipeChangeCallback_WithNullptr
 * @tc.desc  : Test AddInputPipeChangeCallback returns false when callback is nullptr.
 */
HWTEST_F(AudioEngineClientManagerAiut, CallbackHandle_AddInputPipeChangeCallback_WithNullptr, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto handle = new AudioEngineClientManager::CallbackHandle();
    AIUT_ASSERT_NE_INFO_LOG(handle, nullptr, "CallbackHandle create failed");
    std::shared_ptr<AudioInputPipeCallback> nullCb = nullptr;
    bool result = handle->AddInputPipeChangeCallback(nullCb);
    AIUT_EXPECT_FALSE_INFO_LOG(result, "AddInputPipeChangeCallback should return false for nullptr, result=%d",
        result);
    delete handle;
}

/**
 * @tc.name  : Test CallbackHandle AddInputPipeChangeCallback duplicate
 * @tc.number: CallbackHandle_AddInputPipeChangeCallback_Duplicate
 * @tc.desc  : Test AddInputPipeChangeCallback returns false for duplicate callback.
 */
HWTEST_F(AudioEngineClientManagerAiut, CallbackHandle_AddInputPipeChangeCallback_Duplicate, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto handle = new AudioEngineClientManager::CallbackHandle();
    AIUT_ASSERT_NE_INFO_LOG(handle, nullptr, "CallbackHandle create failed");
    std::shared_ptr<AudioInputPipeCallback> cb = std::make_shared<TestAudioInputPipeCallback>();
    handle->AddInputPipeChangeCallback(cb);
    bool result = handle->AddInputPipeChangeCallback(cb);
    AIUT_EXPECT_FALSE_INFO_LOG(result, "AddInputPipeChangeCallback should return false for duplicate, result=%d",
        result);
    delete handle;
}

/**
 * @tc.name  : Test CallbackHandle AddInputPipeChangeCallback first callback
 * @tc.number: CallbackHandle_AddInputPipeChangeCallback_FirstCallback
 * @tc.desc  : Test AddInputPipeChangeCallback returns true for first callback (0 to 1 case).
 */
HWTEST_F(AudioEngineClientManagerAiut, CallbackHandle_AddInputPipeChangeCallback_FirstCallback, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto handle = new AudioEngineClientManager::CallbackHandle();
    AIUT_ASSERT_NE_INFO_LOG(handle, nullptr, "CallbackHandle create failed");
    std::shared_ptr<AudioInputPipeCallback> cb = std::make_shared<TestAudioInputPipeCallback>();
    bool result = handle->AddInputPipeChangeCallback(cb);
    AIUT_EXPECT_TRUE_INFO_LOG(result, "AddInputPipeChangeCallback should return true for first, result=%d", result);
    delete handle;
}

/**
 * @tc.name  : Test CallbackHandle RemoveInputPipeChangeCallback with nullptr
 * @tc.number: CallbackHandle_RemoveInputPipeChangeCallback_WithNullptr
 * @tc.desc  : Test RemoveInputPipeChangeCallback clears all and returns true for nullptr.
 */
HWTEST_F(AudioEngineClientManagerAiut, CallbackHandle_RemoveInputPipeChangeCallback_WithNullptr, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto handle = new AudioEngineClientManager::CallbackHandle();
    AIUT_ASSERT_NE_INFO_LOG(handle, nullptr, "CallbackHandle create failed");
    std::shared_ptr<AudioInputPipeCallback> cb1 = std::make_shared<TestAudioInputPipeCallback>();
    std::shared_ptr<AudioInputPipeCallback> cb2 = std::make_shared<TestAudioInputPipeCallback>();
    handle->AddInputPipeChangeCallback(cb1);
    handle->AddInputPipeChangeCallback(cb2);
    std::shared_ptr<AudioInputPipeCallback> nullCb = nullptr;
    bool result = handle->RemoveInputPipeChangeCallback(nullCb);
    AIUT_EXPECT_TRUE_INFO_LOG(result, "RemoveInputPipeChangeCallback should return true for nullptr, result=%d",
        result);
    delete handle;
}

/**
 * @tc.name  : Test CallbackHandle RemoveInputPipeChangeCallback not found
 * @tc.number: CallbackHandle_RemoveInputPipeChangeCallback_NotFound
 * @tc.desc  : Test RemoveInputPipeChangeCallback returns false when callback not added.
 */
HWTEST_F(AudioEngineClientManagerAiut, CallbackHandle_RemoveInputPipeChangeCallback_NotFound, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto handle = new AudioEngineClientManager::CallbackHandle();
    AIUT_ASSERT_NE_INFO_LOG(handle, nullptr, "CallbackHandle create failed");
    std::shared_ptr<AudioInputPipeCallback> cb = std::make_shared<TestAudioInputPipeCallback>();
    bool result = handle->RemoveInputPipeChangeCallback(cb);
    AIUT_EXPECT_FALSE_INFO_LOG(result, "RemoveInputPipeChangeCallback should return false for not added, result=%d",
        result);
    delete handle;
}

/**
 * @tc.name  : Test CallbackHandle RemoveInputPipeChangeCallback last callback
 * @tc.number: CallbackHandle_RemoveInputPipeChangeCallback_LastCallback
 * @tc.desc  : Test RemoveInputPipeChangeCallback returns true for last callback (1 to 0 case).
 */
HWTEST_F(AudioEngineClientManagerAiut, CallbackHandle_RemoveInputPipeChangeCallback_LastCallback, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto handle = new AudioEngineClientManager::CallbackHandle();
    AIUT_ASSERT_NE_INFO_LOG(handle, nullptr, "CallbackHandle create failed");
    std::shared_ptr<AudioInputPipeCallback> cb = std::make_shared<TestAudioInputPipeCallback>();
    handle->AddInputPipeChangeCallback(cb);
    bool result = handle->RemoveInputPipeChangeCallback(cb);
    AIUT_EXPECT_TRUE_INFO_LOG(result, "RemoveInputPipeChangeCallback should return true for last, result=%d", result);
    delete handle;
}

/**
 * @tc.name  : Test CallbackHandle IsInputPipeChangeEnable
 * @tc.number: CallbackHandle_IsInputPipeChangeEnable_EmptyAndNonEmpty
 * @tc.desc  : Test IsInputPipeChangeEnable returns false when empty and true when non-empty.
 */
HWTEST_F(AudioEngineClientManagerAiut, CallbackHandle_IsInputPipeChangeEnable_EmptyAndNonEmpty, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto handle = new AudioEngineClientManager::CallbackHandle();
    AIUT_ASSERT_NE_INFO_LOG(handle, nullptr, "CallbackHandle create failed");
    bool result = handle->IsInputPipeChangeEnable();
    AIUT_EXPECT_FALSE_INFO_LOG(result, "IsInputPipeChangeEnable should be false when empty, result=%d", result);
    std::shared_ptr<AudioInputPipeCallback> cb = std::make_shared<TestAudioInputPipeCallback>();
    handle->AddInputPipeChangeCallback(cb);
    result = handle->IsInputPipeChangeEnable();
    AIUT_EXPECT_TRUE_INFO_LOG(result, "IsInputPipeChangeEnable should be true when non-empty, result=%d", result);
    delete handle;
}

/**
 * @tc.name  : Test CallbackHandle OnInputPipeChange
 * @tc.number: CallbackHandle_OnInputPipeChange_NotifiesCallbacks
 * @tc.desc  : Test OnInputPipeChange notifies registered callbacks.
 */
HWTEST_F(AudioEngineClientManagerAiut, CallbackHandle_OnInputPipeChange_NotifiesCallbacks, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto handle = new AudioEngineClientManager::CallbackHandle();
    AIUT_ASSERT_NE_INFO_LOG(handle, nullptr, "CallbackHandle create failed");
    auto testCb = std::make_shared<TestAudioInputPipeCallback>();
    std::shared_ptr<AudioInputPipeCallback> cb = testCb;
    handle->AddInputPipeChangeCallback(cb);
    auto pipeInfo = std::make_shared<AudioInputPipeInfo>();
    int32_t ret = handle->OnInputPipeChange(0, pipeInfo);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "OnInputPipeChange should return SUCCESS, ret=%d", ret);
    AIUT_EXPECT_TRUE_INFO_LOG(testCb->called_, "OnInputPipeChange callback should be called, called=%d",
        static_cast<int>(testCb->called_));
    delete handle;
}

/**
 * @tc.name  : Test SetAuxiliarySinkEnable
 * @tc.number: AudioEngineClientManager_SetAuxiliarySinkEnable_EnableAndDisable
 * @tc.desc  : Test SetAuxiliarySinkEnable with true and false.
 */
HWTEST_F(AudioEngineClientManagerAiut, AudioEngineClientManager_SetAuxiliarySinkEnable_EnableAndDisable,
    TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto client = DelayedSingleton<AudioEngineClientManager>::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(client, nullptr, "GetInstance failed");
    int32_t ret1 = client->SetAuxiliarySinkEnable(true);
    int32_t ret2 = client->SetAuxiliarySinkEnable(false);
    AIUT_EXPECT_EQ_INFO_LOG(ret1, ret2,
        "SetAuxiliarySinkEnable should return consistent result, ret1=%d, ret2=%d", ret1, ret2);
}

/**
 * @tc.name  : Test CallbackHandle OnRemoteDied
 * @tc.number: CallbackHandle_OnRemoteDied_NullRemote
 * @tc.desc  : Test OnRemoteDied does not clear callback list.
 */
HWTEST_F(AudioEngineClientManagerAiut, CallbackHandle_OnRemoteDied_NullRemote, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto handle = new AudioEngineClientManager::CallbackHandle();
    AIUT_ASSERT_NE_INFO_LOG(handle, nullptr, "CallbackHandle create failed");
    std::shared_ptr<AudioOutputPipeCallback> cb = std::make_shared<TestAudioOutputPipeCallback>();
    handle->AddOutputPipeChangeCallback(cb);
    wptr<IRemoteObject> remote = nullptr;
    handle->OnRemoteDied(remote);
    bool result = handle->IsOutputPipeChangeEnable();
    AIUT_EXPECT_TRUE_INFO_LOG(result, "IsOutputPipeChangeEnable should remain true after OnRemoteDied, result=%d",
        result);
    delete handle;
}

} // namespace AudioStandard
} // namespace OHOS
