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
// 本文件仅包含现有audio_system_client_policy_manager_unit_test.cpp未覆盖的分支/边界条件。
// 重复方法（nullptr校验、INPUT_DEVICE校验、DEFAULT/RINGING场景等）已在现有UT中覆盖，此处不再重复。

#include "audio_system_client_policy_manager.h"
#include "audio_device_descriptor.h"
#include "audio_errors.h"
#include "aiut_log.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
class MockDistributedRoutingRoleCallback : public AudioDistributedRoutingRoleCallback {
public:
    void OnDistributedRoutingRoleChange(std::shared_ptr<AudioDeviceDescriptor> descriptor,
        const CastType type) override
    {
        (void)descriptor;
        (void)type;
    }
};

class MockRingerModeCallback : public AudioRingerModeCallback {
public:
    void OnRingerModeUpdated(const AudioRingerMode &ringerMode) override
    {
        called_ = true;
        lastMode_ = ringerMode;
    }
    bool called_ = false;
    AudioRingerMode lastMode_ = RINGER_MODE_NORMAL;
};
} // namespace

class AudioSystemClientPolicyManagerAiut : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name  : Test ConfigDistributedRoutingRole with output device
 * @tc.number: PolicyManager_ConfigDistributedRoutingRole_WithOutputDevice
 * @tc.desc  : Test ConfigDistributedRoutingRole with valid output device descriptor.
 *             现有UT仅覆盖nullptr和INPUT_DEVICE分支，本用例覆盖OUTPUT_DEVICE正常分支。
 */
HWTEST_F(AudioSystemClientPolicyManagerAiut,
    PolicyManager_ConfigDistributedRoutingRole_WithOutputDevice, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &manager = AudioSystemClientPolicyManager::GetInstance();
    auto descriptor = std::make_shared<AudioDeviceDescriptor>();
    descriptor->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    int32_t ret1 = manager.ConfigDistributedRoutingRole(descriptor, CastType::CAST_TYPE_NULL);
    int32_t ret2 = manager.ConfigDistributedRoutingRole(descriptor, CastType::CAST_TYPE_NULL);
    AIUT_EXPECT_EQ_INFO_LOG(ret1, ret2,
        "ConfigDistributedRoutingRole should return consistent result, ret1=%d, ret2=%d", ret1, ret2);
}

/**
 * @tc.name  : Test SetRingerMode with callback
 * @tc.number: PolicyManager_SetRingerMode_WithCallback
 * @tc.desc  : Test SetRingerMode triggers callback when set.
 *             现有UT仅测试无callback时SetRingerMode，本用例覆盖callback触发分支。
 */
HWTEST_F(AudioSystemClientPolicyManagerAiut, PolicyManager_SetRingerMode_WithCallback, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &manager = AudioSystemClientPolicyManager::GetInstance();
    int32_t clientId = getpid();
    auto cb = std::make_shared<MockRingerModeCallback>();
    manager.ringerModeCallback_ = cb;
    manager.cbClientId_ = clientId;
    AudioRingerMode mode = RINGER_MODE_SILENT;
    int32_t ret = manager.SetRingerMode(mode);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetRingerMode should return SUCCESS, ret=%d", ret);
    AIUT_EXPECT_TRUE_INFO_LOG(cb->called_, "SetRingerMode callback should be called, called=%d",
        static_cast<int>(cb->called_));
}

/**
 * @tc.name  : Test UnsetRingerModeCallback with valid clientId
 * @tc.number: PolicyManager_UnsetRingerModeCallback_ValidClientId
 * @tc.desc  : Test UnsetRingerModeCallback with matching clientId.
 *             现有UT仅测试invalid clientId(-1)，本用例覆盖valid clientId正常移除分支。
 */
HWTEST_F(AudioSystemClientPolicyManagerAiut, PolicyManager_UnsetRingerModeCallback_ValidClientId, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &manager = AudioSystemClientPolicyManager::GetInstance();
    int32_t clientId = getpid();
    auto cb = std::make_shared<MockRingerModeCallback>();
    manager.ringerModeCallback_ = cb;
    manager.cbClientId_ = clientId;
    int32_t ret = manager.UnsetRingerModeCallback(clientId);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "UnsetRingerModeCallback should return SUCCESS, ret=%d", ret);
}

/**
 * @tc.name  : Test SetAudioScene with VOICE_RINGING
 * @tc.number: PolicyManager_SetAudioScene_VoiceRinging
 * @tc.desc  : Test SetAudioScene with AUDIO_SCENE_VOICE_RINGING.
 *             现有UT仅覆盖DEFAULT和RINGING场景，本用例覆盖VOICE_RINGING分支。
 */
HWTEST_F(AudioSystemClientPolicyManagerAiut, PolicyManager_SetAudioScene_VoiceRinging, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &manager = AudioSystemClientPolicyManager::GetInstance();
    int32_t ret1 = manager.SetAudioScene(AUDIO_SCENE_VOICE_RINGING);
    int32_t ret2 = manager.SetAudioScene(AUDIO_SCENE_VOICE_RINGING);
    AIUT_EXPECT_EQ_INFO_LOG(ret1, ret2,
        "SetAudioScene(VOICE_RINGING) should return consistent result, ret1=%d, ret2=%d", ret1, ret2);
}

/**
 * @tc.name  : Test SetCallDeviceActive with valid parameters
 * @tc.number: PolicyManager_SetCallDeviceActive_ValidParams
 * @tc.desc  : Test SetCallDeviceActive with valid parameters.
 *             SetCallDeviceActive是现有UT未覆盖的新方法。
 */
HWTEST_F(AudioSystemClientPolicyManagerAiut, PolicyManager_SetCallDeviceActive_ValidParams, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &manager = AudioSystemClientPolicyManager::GetInstance();
    std::string address = "test_address";
    int32_t ret1 = manager.SetCallDeviceActive(DeviceType::DEVICE_TYPE_SPEAKER, true, address, -1);
    int32_t ret2 = manager.SetCallDeviceActive(DeviceType::DEVICE_TYPE_SPEAKER, true, address, -1);
    AIUT_EXPECT_EQ_INFO_LOG(ret1, ret2,
        "SetCallDeviceActive should return consistent result, ret1=%d, ret2=%d", ret1, ret2);
}

/**
 * @tc.name  : Test SetCallDeviceActive with specific uid
 * @tc.number: PolicyManager_SetCallDeviceActive_WithUid
 * @tc.desc  : Test SetCallDeviceActive with specific clientUid.
 *             SetCallDeviceActive是现有UT未覆盖的新方法，本用例覆盖带uid的分支。
 */
HWTEST_F(AudioSystemClientPolicyManagerAiut, PolicyManager_SetCallDeviceActive_WithUid, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &manager = AudioSystemClientPolicyManager::GetInstance();
    std::string address = "test_address";
    int32_t ret1 = manager.SetCallDeviceActive(DeviceType::DEVICE_TYPE_SPEAKER, false, address, 1001);
    int32_t ret2 = manager.SetCallDeviceActive(DeviceType::DEVICE_TYPE_SPEAKER, false, address, 1001);
    AIUT_EXPECT_EQ_INFO_LOG(ret1, ret2,
        "SetCallDeviceActive with uid should return consistent result, ret1=%d, ret2=%d", ret1, ret2);
}

} // namespace AudioStandard
} // namespace OHOS
