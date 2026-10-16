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

#include "audio_stream_client_manager.h"
#include "audio_errors.h"
#include "aiut_log.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
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

class AudioStreamClientManagerAiut : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name  : Test GetVolumeBySessionId
 * @tc.number: AudioStreamClientManager_GetVolumeBySessionId_ValidSessionId
 * @tc.desc  : Test GetVolumeBySessionId with valid sessionId.
 */
HWTEST_F(AudioStreamClientManagerAiut, AudioStreamClientManager_GetVolumeBySessionId_ValidSessionId, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &manager = AudioStreamClientManager::GetInstance();
    uint32_t sessionId = 0;
    float volume = 0.0f;
    int32_t ret = manager.GetVolumeBySessionId(sessionId, volume);
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS,
        "GetVolumeBySessionId should not return SUCCESS when proxy is null, ret=%d", ret);
}

/**
 * @tc.name  : Test GetVolumeBySessionId with non-zero sessionId
 * @tc.number: AudioStreamClientManager_GetVolumeBySessionId_NonZeroSessionId
 * @tc.desc  : Test GetVolumeBySessionId with non-zero sessionId.
 */
HWTEST_F(AudioStreamClientManagerAiut, AudioStreamClientManager_GetVolumeBySessionId_NonZeroSessionId, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &manager = AudioStreamClientManager::GetInstance();
    uint32_t sessionId = 100;
    float volume = 0.0f;
    int32_t ret = manager.GetVolumeBySessionId(sessionId, volume);
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS,
        "GetVolumeBySessionId(100) should not return SUCCESS when proxy is null, ret=%d", ret);
}

/**
 * @tc.name  : Test RegisterRendererDataTransferCallback with nullptr
 * @tc.number: AudioStreamClientManager_RegisterRendererDataTransferCallback_WithNullptr
 * @tc.desc  : Test RegisterRendererDataTransferCallback returns error for nullptr callback.
 */
HWTEST_F(AudioStreamClientManagerAiut,
    AudioStreamClientManager_RegisterRendererDataTransferCallback_WithNullptr, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &manager = AudioStreamClientManager::GetInstance();
    DataTransferMonitorParam param;
    std::shared_ptr<AudioRendererDataTransferStateChangeCallback> nullCb = nullptr;
    int32_t ret = manager.RegisterRendererDataTransferCallback(param, nullCb);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERROR_INVALID_PARAM, "RegisterRendererDataTransferCallback should return "
        "ERROR_INVALID_PARAM for nullptr, ret=%d", ret);
}

/**
 * @tc.name  : Test RegisterRendererDataTransferCallback with valid callback
 * @tc.number: AudioStreamClientManager_RegisterRendererDataTransferCallback_ValidCallback
 * @tc.desc  : Test RegisterRendererDataTransferCallback with valid callback.
 */
HWTEST_F(AudioStreamClientManagerAiut,
    AudioStreamClientManager_RegisterRendererDataTransferCallback_ValidCallback, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &manager = AudioStreamClientManager::GetInstance();
    DataTransferMonitorParam param;
    auto cb = std::make_shared<TestRendererDataTransferStateChangeCallback>();
    int32_t ret = manager.RegisterRendererDataTransferCallback(param, cb);
    // Proxy is available in test environment, registration succeeds and returns SUCCESS.
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS,
        "RegisterRendererDataTransferCallback should return SUCCESS for valid callback, ret=%d", ret);
}

/**
 * @tc.name  : Test UnregisterRendererDataTransferCallback with nullptr
 * @tc.number: AudioStreamClientManager_UnregisterRendererDataTransferCallback_WithNullptr
 * @tc.desc  : Test UnregisterRendererDataTransferCallback returns error for nullptr callback.
 */
HWTEST_F(AudioStreamClientManagerAiut,
    AudioStreamClientManager_UnregisterRendererDataTransferCallback_WithNullptr, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &manager = AudioStreamClientManager::GetInstance();
    std::shared_ptr<AudioRendererDataTransferStateChangeCallback> nullCb = nullptr;
    int32_t ret = manager.UnregisterRendererDataTransferCallback(nullCb);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERROR_INVALID_PARAM, "UnregisterRendererDataTransferCallback should return "
        "ERROR_INVALID_PARAM for nullptr, ret=%d", ret);
}

/**
 * @tc.name  : Test UnregisterRendererDataTransferCallback with valid callback
 * @tc.number: AudioStreamClientManager_UnregisterRendererDataTransferCallback_AfterRegister
 * @tc.desc  : Test UnregisterRendererDataTransferCallback with valid callback after register.
 */
HWTEST_F(AudioStreamClientManagerAiut,
    AudioStreamClientManager_UnregisterRendererDataTransferCallback_AfterRegister, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &manager = AudioStreamClientManager::GetInstance();
    DataTransferMonitorParam param;
    auto cb = std::make_shared<TestRendererDataTransferStateChangeCallback>();
    manager.RegisterRendererDataTransferCallback(param, cb);
    int32_t ret = manager.UnregisterRendererDataTransferCallback(cb);
    // Proxy is available in test environment, unregistration succeeds and returns SUCCESS.
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS,
        "UnregisterRendererDataTransferCallback should return SUCCESS after register, ret=%d", ret);
}

/**
 * @tc.name  : Test UnregisterRendererDataTransferCallback without prior register
 * @tc.number: AudioStreamClientManager_UnregisterRendererDataTransferCallback_NoPriorRegister
 * @tc.desc  : Test UnregisterRendererDataTransferCallback when listener is null.
 */
HWTEST_F(AudioStreamClientManagerAiut,
    AudioStreamClientManager_UnregisterRendererDataTransferCallback_NoPriorRegister, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &manager = AudioStreamClientManager::GetInstance();
    auto cb = std::make_shared<TestRendererDataTransferStateChangeCallback>();
    int32_t ret = manager.UnregisterRendererDataTransferCallback(cb);
    // Proxy is available in test environment, returns SUCCESS even without prior register.
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS,
        "UnregisterRendererDataTransferCallback (no prior register) should return SUCCESS, ret=%d", ret);
}

} // namespace AudioStandard
} // namespace OHOS
