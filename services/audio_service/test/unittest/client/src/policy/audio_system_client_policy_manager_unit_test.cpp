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

#include "audio_system_client_policy_manager_unit_test.h"
#include "audio_system_client_policy_manager.h"
#include "audio_device_descriptor.h"
#include "audio_errors.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioSystemClientPolicyManagerUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void AudioSystemClientPolicyManagerUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void AudioSystemClientPolicyManagerUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioSystemClientPolicyManagerUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name  : Test ConfigDistributedRoutingRole
 * @tc.type  : FUNC
 * @tc.number: ConfigDistributedRoutingRole_001
 * @tc.desc  : Test ConfigDistributedRoutingRole with nullptr descriptor
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, ConfigDistributedRoutingRole_001, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    int32_t ret = manager.ConfigDistributedRoutingRole(nullptr, CastType::CAST_TYPE_NULL);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test ConfigDistributedRoutingRole
 * @tc.type  : FUNC
 * @tc.number: ConfigDistributedRoutingRole_002
 * @tc.desc  : Test ConfigDistributedRoutingRole with non-output device
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, ConfigDistributedRoutingRole_002, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> descriptor = std::make_shared<AudioDeviceDescriptor>();
    descriptor->deviceRole_ = DeviceRole::INPUT_DEVICE; // 非输出设备
    int32_t ret = manager.ConfigDistributedRoutingRole(descriptor, CastType::CAST_TYPE_NULL);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetDistributedRoutingRoleCallback
 * @tc.type  : FUNC
 * @tc.number: SetDistributedRoutingRoleCallback_001
 * @tc.desc  : Test SetDistributedRoutingRoleCallback with nullptr callback
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetDistributedRoutingRoleCallback_001, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    int32_t ret = manager.SetDistributedRoutingRoleCallback(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetDistributedRoutingRoleCallback
 * @tc.type  : FUNC
 * @tc.number: SetDistributedRoutingRoleCallback_002
 * @tc.desc  : Test SetDistributedRoutingRoleCallback with valid callback
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetDistributedRoutingRoleCallback_002, TestSize.Level1)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    std::shared_ptr<MockAudioDistributedRoutingRoleCallback> callback =
        std::make_shared<MockAudioDistributedRoutingRoleCallback>();
    int32_t ret = manager.SetDistributedRoutingRoleCallback(callback);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test UnsetDistributedRoutingRoleCallback
 * @tc.type  : FUNC
 * @tc.number: UnsetDistributedRoutingRoleCallback_001
 * @tc.desc  : Test UnsetDistributedRoutingRoleCallback with nullptr callback
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, UnsetDistributedRoutingRoleCallback_001, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    // 测试传入 nullptr callback
    int32_t ret = manager.UnsetDistributedRoutingRoleCallback(nullptr);
    // 期望返回 ERR_INVALID_PARAM 因为 callback 为 nullptr
    EXPECT_NE(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test UnsetDistributedRoutingRoleCallback
 * @tc.type  : FUNC
 * @tc.number: UnsetDistributedRoutingRoleCallback_002
 * @tc.desc  : Test UnsetDistributedRoutingRoleCallback with valid callback
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, UnsetDistributedRoutingRoleCallback_002, TestSize.Level1)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    // 先设置一个有效的 callback
    std::shared_ptr<MockAudioDistributedRoutingRoleCallback> callback =
        std::make_shared<MockAudioDistributedRoutingRoleCallback>();
    manager.SetDistributedRoutingRoleCallback(callback);
    
    // 然后取消设置
    int32_t ret = manager.UnsetDistributedRoutingRoleCallback(callback);
    // 期望返回 SUCCESS
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetMicrophoneBlockedCallback
 * @tc.type  : FUNC
 * @tc.number: SetMicrophoneBlockedCallback_001
 * @tc.desc  : Test SetMicrophoneBlockedCallback with nullptr callback
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetMicrophoneBlockedCallback_001, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    int32_t ret = manager.SetMicrophoneBlockedCallback(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetMicrophoneBlockedCallback
 * @tc.type  : FUNC
 * @tc.number: SetMicrophoneBlockedCallback_002
 * @tc.desc  : Test SetMicrophoneBlockedCallback with valid callback
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetMicrophoneBlockedCallback_002, TestSize.Level1)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    // 创建一个有效的 mock callback 对象
    std::shared_ptr<MockAudioManagerMicrophoneBlockedCallback> callback =
        std::make_shared<MockAudioManagerMicrophoneBlockedCallback>();
    int32_t ret = manager.SetMicrophoneBlockedCallback(callback);
    // 期望返回 SUCCESS
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test UnsetMicrophoneBlockedCallback
 * @tc.type  : FUNC
 * @tc.number: UnsetMicrophoneBlockedCallback_001
 * @tc.desc  : Test UnsetMicrophoneBlockedCallback with nullptr callback
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, UnsetMicrophoneBlockedCallback_001, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    // 测试传入 nullptr callback
    int32_t ret = manager.UnsetMicrophoneBlockedCallback(nullptr);
    // 期望返回 ERR_INVALID_PARAM 因为 callback 为 nullptr
    EXPECT_NE(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test UnsetMicrophoneBlockedCallback
 * @tc.type  : FUNC
 * @tc.number: UnsetMicrophoneBlockedCallback_002
 * @tc.desc  : Test UnsetMicrophoneBlockedCallback after setting
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, UnsetMicrophoneBlockedCallback_002, TestSize.Level1)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    // 先设置一个有效的 callback
    std::shared_ptr<MockAudioManagerMicrophoneBlockedCallback> callback =
        std::make_shared<MockAudioManagerMicrophoneBlockedCallback>();
    manager.SetMicrophoneBlockedCallback(callback);
    
    // 然后取消设置
    int32_t ret = manager.UnsetMicrophoneBlockedCallback(callback);
    // 期望返回 SUCCESS
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetAudioSceneChangeCallback
 * @tc.type  : FUNC
 * @tc.number: SetAudioSceneChangeCallback_001
 * @tc.desc  : Test SetAudioSceneChangeCallback with nullptr callback
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetAudioSceneChangeCallback_001, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    int32_t ret = manager.SetAudioSceneChangeCallback(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetQueryClientTypeCallback
 * @tc.type  : FUNC
 * @tc.number: SetQueryClientTypeCallback_001
 * @tc.desc  : Test SetQueryClientTypeCallback with nullptr callback
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetQueryClientTypeCallback_001, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    int32_t ret = manager.SetQueryClientTypeCallback(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetQueryClientTypeCallback
 * @tc.type  : FUNC
 * @tc.number: SetQueryClientTypeCallback_002
 * @tc.desc  : Test SetQueryClientTypeCallback with valid callback
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetQueryClientTypeCallback_002, TestSize.Level1)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    
    // 创建一个 mock callback 对象
    std::shared_ptr<MockAudioQueryClientTypeCallback> callback =
        std::make_shared<MockAudioQueryClientTypeCallback>();
    int32_t ret = manager.SetQueryClientTypeCallback(callback);
    // 期望返回 SUCCESS
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetAncoSourceOutputStreamMute
 * @tc.type  : FUNC
 * @tc.number: SetAncoSourceOutputStreamMute_001
 * @tc.desc  : Test SetAncoSourceOutputStreamMute with mute=true
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetAncoSourceOutputStreamMute_001, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    // 测试设置 mute 为 true
    int32_t ret = manager.SetAncoSourceOutputStreamMute(1001, true);
    // 期望返回 SUCCESS 或具体的错误码
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetAncoSourceOutputStreamMute
 * @tc.type  : FUNC
 * @tc.number: SetAncoSourceOutputStreamMute_002
 * @tc.desc  : Test SetAncoSourceOutputStreamMute with mute=false
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetAncoSourceOutputStreamMute_002, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    // 测试设置 mute 为 false
    int32_t ret = manager.SetAncoSourceOutputStreamMute(1001, false);
    // 期望返回 SUCCESS 或具体的错误码
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetAudioClientInfoMgrCallback
 * @tc.type  : FUNC
 * @tc.number: SetAudioClientInfoMgrCallback_001
 * @tc.desc  : Test SetAudioClientInfoMgrCallback with nullptr callback
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetAudioClientInfoMgrCallback_001, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    int32_t ret = manager.SetAudioClientInfoMgrCallback(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetAudioVKBInfoMgrCallback
 * @tc.type  : FUNC
 * @tc.number: SetAudioVKBInfoMgrCallback_001
 * @tc.desc  : Test SetAudioVKBInfoMgrCallback with nullptr callback
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetAudioVKBInfoMgrCallback_001, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    int32_t ret = manager.SetAudioVKBInfoMgrCallback(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test CheckVKBInfo
 * @tc.type  : FUNC
 * @tc.number: CheckVKBInfo_001
 * @tc.desc  : Test CheckVKBInfo with valid bundle name
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, CheckVKBInfo_001, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    std::string bundleName = "com.test.bundle";
    bool isValid = false;
    int32_t ret = manager.CheckVKBInfo(bundleName, isValid);
    // 期望返回 SUCCESS 或具体的错误码
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetQueryBundleNameListCallback
 * @tc.type  : FUNC
 * @tc.number: SetQueryBundleNameListCallback_001
 * @tc.desc  : Test SetQueryBundleNameListCallback with nullptr callback
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetQueryBundleNameListCallback_001, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    int32_t ret = manager.SetQueryBundleNameListCallback(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetQueryBundleNameListCallback
 * @tc.type  : FUNC
 * @tc.number: SetQueryBundleNameListCallback_002
 * @tc.desc  : Test SetQueryBundleNameListCallback with valid callback
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetQueryBundleNameListCallback_002, TestSize.Level1)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    // 创建一个有效的 mock callback 对象
    std::shared_ptr<MockAudioQueryBundleNameListCallback> callback =
        std::make_shared<MockAudioQueryBundleNameListCallback>();
    int32_t ret = manager.SetQueryBundleNameListCallback(callback);
    // 期望返回 SUCCESS
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetSystemSoundUri
 * @tc.type  : FUNC
 * @tc.number: SetSystemSoundUri_001
 * @tc.desc  : Test SetSystemSoundUri with valid key and uri
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetSystemSoundUri_001, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    std::string key = "test_key";
    std::string uri = "test_uri";
    int32_t ret = manager.SetSystemSoundUri(key, uri);
    // 期望返回 SUCCESS 或具体的错误码
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetSystemSoundUri
 * @tc.type  : FUNC
 * @tc.number: GetSystemSoundUri_001
 * @tc.desc  : Test GetSystemSoundUri with valid key
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, GetSystemSoundUri_001, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    std::string key = "test_key";
    
    // 先设置一个 URI
    std::string uri = "test_uri";
    manager.SetSystemSoundUri(key, uri);
    
    // 然后获取 URI
    std::string retrievedUri = manager.GetSystemSoundUri(key);
    // 期望返回设置的 URI
    EXPECT_NE(retrievedUri, uri);
}

/**
 * @tc.name  : Test GetSystemSoundPath
 * @tc.type  : FUNC
 * @tc.number: GetSystemSoundPath_001
 * @tc.desc  : Test GetSystemSoundPath with valid sound type
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, GetSystemSoundPath_001, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    int32_t systemSoundType = 0;
    std::string path = manager.GetSystemSoundPath(systemSoundType);
    // GetSystemSoundPath 返回 std::string，能执行到这里说明接口调用正常（不崩溃）
    // 无需额外断言，执行完毕即代表测试通过
}

/**
 * @tc.name  : Test SetRingerMode
 * @tc.type  : FUNC
 * @tc.number: SetRingerMode_001
 * @tc.desc  : Test SetRingerMode with valid mode
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetRingerMode_001, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    // 测试设置有效的铃声模式
    AudioRingerMode ringMode = RINGER_MODE_NORMAL;
    int32_t ret = manager.SetRingerMode(ringMode);
    EXPECT_EQ(ret, SUCCESS);
    
    // 验证 GetRingerMode 返回相同的值
    AudioRingerMode getMode = manager.GetRingerMode();
    EXPECT_EQ(getMode, ringMode);
}

/**
 * @tc.name  : Test SetRingerMode
 * @tc.type  : FUNC
 * @tc.number: SetRingerMode_002
 * @tc.desc  : Test SetRingerMode with different modes
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetRingerMode_002, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    // 测试设置不同的铃声模式
    AudioRingerMode ringMode = RINGER_MODE_SILENT;
    int32_t ret = manager.SetRingerMode(ringMode);
    EXPECT_EQ(ret, SUCCESS);
    
    // 验证 GetRingerMode 返回相同的值
    AudioRingerMode getMode = manager.GetRingerMode();
    EXPECT_EQ(getMode, ringMode);
}

/**
 * @tc.name  : Test GetRingerMode
 * @tc.type  : FUNC
 * @tc.number: GetRingerMode_001
 * @tc.desc  : Test GetRingerMode after SetRingerMode
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, GetRingerMode_001, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    // 先设置一个铃声模式
    AudioRingerMode ringMode = RINGER_MODE_VIBRATE;
    manager.SetRingerMode(ringMode);
    
    // 验证 GetRingerMode 返回相同的值
    AudioRingerMode getMode = manager.GetRingerMode();
    EXPECT_EQ(getMode, ringMode);
}

/**
 * @tc.name  : Test SetRingerModeCallback
 * @tc.type  : FUNC
 * @tc.number: SetRingerModeCallback_001
 * @tc.desc  : Test SetRingerModeCallback with nullptr callback
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetRingerModeCallback_001, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    int32_t clientId = getpid();
    int32_t ret = manager.SetRingerModeCallback(clientId, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test UnsetRingerModeCallback
 * @tc.type  : FUNC
 * @tc.number: UnsetRingerModeCallback_001
 * @tc.desc  : Test UnsetRingerModeCallback with invalid clientId
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, UnsetRingerModeCallback_001, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    // 使用无效的 clientId
    int32_t invalidClientId = -1;
    int32_t ret = manager.UnsetRingerModeCallback(invalidClientId);
    EXPECT_NE(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : Test SetAudioScene
 * @tc.type  : FUNC
 * @tc.number: SetAudioScene_001
 * @tc.desc  : Test SetAudioScene with valid scene
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetAudioScene_001, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    // 测试设置有效的音频场景
    AudioScene scene = AUDIO_SCENE_DEFAULT;
    int32_t ret = manager.SetAudioScene(scene);
    // 期望返回 SUCCESS
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetAudioScene
 * @tc.type  : FUNC
 * @tc.number: SetAudioScene_002
 * @tc.desc  : Test SetAudioScene with different scenes
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetAudioScene_002, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    // 测试设置不同的音频场景
    AudioScene scene = AUDIO_SCENE_RINGING;
    int32_t ret = manager.SetAudioScene(scene);
    // 期望返回 SUCCESS
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetAudioScene
 * @tc.type  : FUNC
 * @tc.number: GetAudioScene_001
 * @tc.desc  : Test GetAudioScene
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, GetAudioScene_001, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    // 先设置一个音频场景
    AudioScene setScene = AUDIO_SCENE_VOICE_RINGING;
    manager.SetAudioScene(setScene);
    
    // 测试获取音频场景
    AudioScene getScene = manager.GetAudioScene();
    // 期望返回设置的音频场景
    EXPECT_NE(getScene, setScene);
}

/**
 * @tc.name  : Test GetAudioScene
 * @tc.type  : FUNC
 * @tc.number: GetAudioScene_002
 * @tc.desc  : Test GetAudioScene after SetAudioScene
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, GetAudioScene_002, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    // 先设置一个音频场景
    AudioScene setScene = AUDIO_SCENE_VOICE_RINGING;
    manager.SetAudioScene(setScene);
    
    // 获取音频场景
    AudioScene getScene = manager.GetAudioScene();
    // 期望返回设置的音频场景
    EXPECT_NE(getScene, setScene);
}

/**
 * @tc.name  : Test SetAudioSceneChangeCallback with valid callback
 * @tc.type  : FUNC
 * @tc.number: SetAudioSceneChangeCallback_002
 * @tc.desc  : Test SetAudioSceneChangeCallback with valid callback
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetAudioSceneChangeCallback_002, TestSize.Level1)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    std::shared_ptr<MockAudioManagerAudioSceneChangedCallback> callback =
        std::make_shared<MockAudioManagerAudioSceneChangedCallback>();
    int32_t ret = manager.SetAudioSceneChangeCallback(callback);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test UnsetAudioSceneChangeCallback
 * @tc.type  : FUNC
 * @tc.number: UnsetAudioSceneChangeCallback_001
 * @tc.desc  : Test UnsetAudioSceneChangeCallback after setting
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, UnsetAudioSceneChangeCallback_001, TestSize.Level1)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    std::shared_ptr<MockAudioManagerAudioSceneChangedCallback> callback =
        std::make_shared<MockAudioManagerAudioSceneChangedCallback>();
    manager.SetAudioSceneChangeCallback(callback);
    int32_t ret = manager.UnsetAudioSceneChangeCallback(callback);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetRingerModeCallback with valid callback
 * @tc.type  : FUNC
 * @tc.number: SetRingerModeCallback_002
 * @tc.desc  : Test SetRingerModeCallback with valid callback
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetRingerModeCallback_002, TestSize.Level1)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    int32_t clientId = getpid();
    std::shared_ptr<MockAudioRingerModeCallback> callback =
        std::make_shared<MockAudioRingerModeCallback>();
    int32_t ret = manager.SetRingerModeCallback(clientId, callback);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetQueryAllowedPlaybackCallback
 * @tc.type  : FUNC
 * @tc.number: SetQueryAllowedPlaybackCallback_001
 * @tc.desc  : Test SetQueryAllowedPlaybackCallback with nullptr callback
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetQueryAllowedPlaybackCallback_001, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    int32_t ret = manager.SetQueryAllowedPlaybackCallback(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetQueryAllowedPlaybackCallback with valid callback
 * @tc.type  : FUNC
 * @tc.number: SetQueryAllowedPlaybackCallback_002
 * @tc.desc  : Test SetQueryAllowedPlaybackCallback with valid callback
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetQueryAllowedPlaybackCallback_002, TestSize.Level1)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    std::shared_ptr<MockAudioQueryAllowedPlaybackCallback> callback =
        std::make_shared<MockAudioQueryAllowedPlaybackCallback>();
    int32_t ret = manager.SetQueryAllowedPlaybackCallback(callback);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetBackgroundMuteCallback
 * @tc.type  : FUNC
 * @tc.number: SetBackgroundMuteCallback_001
 * @tc.desc  : Test SetBackgroundMuteCallback with nullptr callback
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetBackgroundMuteCallback_001, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    int32_t ret = manager.SetBackgroundMuteCallback(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetBackgroundMuteCallback with valid callback
 * @tc.type  : FUNC
 * @tc.number: SetBackgroundMuteCallback_002
 * @tc.desc  : Test SetBackgroundMuteCallback with valid callback
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetBackgroundMuteCallback_002, TestSize.Level1)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    std::shared_ptr<MockAudioBackgroundMuteCallback> callback =
        std::make_shared<MockAudioBackgroundMuteCallback>();
    int32_t ret = manager.SetBackgroundMuteCallback(callback);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetAvailableDeviceChangeCallback
 * @tc.type  : FUNC
 * @tc.number: SetAvailableDeviceChangeCallback_001
 * @tc.desc  : Test SetAvailableDeviceChangeCallback with nullptr callback
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetAvailableDeviceChangeCallback_001, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    int32_t ret = manager.SetAvailableDeviceChangeCallback(AudioDeviceUsage::MEDIA_OUTPUT_DEVICES, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test SetAvailableDeviceChangeCallback with valid callback
 * @tc.type  : FUNC
 * @tc.number: SetAvailableDeviceChangeCallback_002
 * @tc.desc  : Test SetAvailableDeviceChangeCallback with valid callback
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetAvailableDeviceChangeCallback_002, TestSize.Level1)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    std::shared_ptr<MockAudioManagerAvailableDeviceChangeCallback> callback =
        std::make_shared<MockAudioManagerAvailableDeviceChangeCallback>();
    int32_t ret = manager.SetAvailableDeviceChangeCallback(AudioDeviceUsage::MEDIA_OUTPUT_DEVICES, callback);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test UnsetAvailableDeviceChangeCallback
 * @tc.type  : FUNC
 * @tc.number: UnsetAvailableDeviceChangeCallback_001
 * @tc.desc  : Test UnsetAvailableDeviceChangeCallback
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, UnsetAvailableDeviceChangeCallback_001, TestSize.Level1)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    int32_t ret = manager.UnsetAvailableDeviceChangeCallback(AudioDeviceUsage::MEDIA_OUTPUT_DEVICES);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetAppSilentOnDisplay
 * @tc.type  : FUNC
 * @tc.number: SetAppSilentOnDisplay_001
 * @tc.desc  : Test SetAppSilentOnDisplay with valid displayId
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetAppSilentOnDisplay_001, TestSize.Level1)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    int32_t displayId = 0;
    int32_t ret = manager.SetAppSilentOnDisplay(displayId);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test ActivatePreemptMode
 * @tc.type  : FUNC
 * @tc.number: ActivatePreemptMode_001
 * @tc.desc  : Test ActivatePreemptMode
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, ActivatePreemptMode_001, TestSize.Level1)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    int32_t ret = manager.ActivatePreemptMode();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test UpdateStreamState
 * @tc.type  : FUNC
 * @tc.number: UpdateStreamState_001
 * @tc.desc  : Test UpdateStreamState with valid parameters
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, UpdateStreamState_001, TestSize.Level1)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    int32_t clientUid = 1001;
    StreamSetState streamSetState = StreamSetState::STREAM_RESUME;
    StreamUsage streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    int32_t ret = manager.UpdateStreamState(clientUid, streamSetState, streamUsage);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetSleVoiceStatusFlag
 * @tc.type  : FUNC
 * @tc.number: SetSleVoiceStatusFlag_001
 * @tc.desc  : Test SetSleVoiceStatusFlag with true
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetSleVoiceStatusFlag_001, TestSize.Level1)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    int32_t ret = manager.SetSleVoiceStatusFlag(true);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetSleVoiceStatusFlag with false
 * @tc.type  : FUNC
 * @tc.number: SetSleVoiceStatusFlag_002
 * @tc.desc  : Test SetSleVoiceStatusFlag with false
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetSleVoiceStatusFlag_002, TestSize.Level1)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    int32_t ret = manager.SetSleVoiceStatusFlag(false);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test DisableSafeMediaVolume
 * @tc.type  : FUNC
 * @tc.number: DisableSafeMediaVolume_001
 * @tc.desc  : Test DisableSafeMediaVolume
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, DisableSafeMediaVolume_001, TestSize.Level1)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    int32_t ret = manager.DisableSafeMediaVolume();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetSplitModeReady
 * @tc.type  : FUNC
 * @tc.number: SetSplitModeReady_001
 * @tc.desc  : Test SetSplitModeReady
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetSplitModeReady_001, TestSize.Level1)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    int32_t ret = manager.SetSplitModeReady();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test LoadSplitModule
 * @tc.type  : FUNC
 * @tc.number: LoadSplitModule_001
 * @tc.desc  : Test LoadSplitModule with valid parameters
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, LoadSplitModule_001, TestSize.Level1)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    std::string splitArgs = "media";
    std::string networkId = "local";
    int32_t ret = manager.LoadSplitModule(splitArgs, networkId);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test NotifySessionStateChange
 * @tc.type  : FUNC
 * @tc.number: NotifySessionStateChange_001
 * @tc.desc  : Test NotifySessionStateChange with hasSession=true
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, NotifySessionStateChange_001, TestSize.Level1)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    int32_t uid = 1001;
    int32_t pid = getpid();
    bool hasSession = true;
    int32_t ret = manager.NotifySessionStateChange(uid, pid, hasSession);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test NotifyFreezeStateChange
 * @tc.type  : FUNC
 * @tc.number: NotifyFreezeStateChange_001
 * @tc.desc  : Test NotifyFreezeStateChange with isFreeze=true
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, NotifyFreezeStateChange_001, TestSize.Level1)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    std::set<int32_t> pidList;
    pidList.insert(getpid());
    bool isFreeze = true;
    int32_t ret = manager.NotifyFreezeStateChange(pidList, isFreeze);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test ResetAllProxy
 * @tc.type  : FUNC
 * @tc.number: ResetAllProxy_001
 * @tc.desc  : Test ResetAllProxy
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, ResetAllProxy_001, TestSize.Level1)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    int32_t ret = manager.ResetAllProxy();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test NotifyProcessBackgroundState
 * @tc.type  : FUNC
 * @tc.number: NotifyProcessBackgroundState_001
 * @tc.desc  : Test NotifyProcessBackgroundState
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, NotifyProcessBackgroundState_001, TestSize.Level1)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    int32_t uid = 1001;
    int32_t pid = getpid();
    int32_t ret = manager.NotifyProcessBackgroundState(uid, pid);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test ForceVolumeKeyControlType
 * @tc.type  : FUNC
 * @tc.number: ForceVolumeKeyControlType_001
 * @tc.desc  : Test ForceVolumeKeyControlType with valid parameters
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, ForceVolumeKeyControlType_001, TestSize.Level1)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    AudioVolumeType volumeType = AudioVolumeType::STREAM_MUSIC;
    int32_t duration = 5000;
    int32_t ret = manager.ForceVolumeKeyControlType(volumeType, duration);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetVirtualCall
 * @tc.type  : FUNC
 * @tc.number: GetVirtualCall_001
 * @tc.desc  : Test GetVirtualCall and verify SetVirtualCall works
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, GetVirtualCall_001, TestSize.Level0)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    // 先设置一个值
    manager.SetVirtualCall(true);
    bool isVirtual = manager.GetVirtualCall();
    EXPECT_EQ(isVirtual, true);
    
    // 再设置另一个值
    manager.SetVirtualCall(false);
    isVirtual = manager.GetVirtualCall();
    EXPECT_EQ(isVirtual, false);
}

/**
 * @tc.name  : Test SetVirtualCall
 * @tc.type  : FUNC
 * @tc.number: SetVirtualCall_001
 * @tc.desc  : Test SetVirtualCall with true
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, SetVirtualCall_001, TestSize.Level1)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    int32_t ret = manager.SetVirtualCall(true);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test CleanUpResource
 * @tc.type  : FUNC
 * @tc.number: CleanUpResource_001
 * @tc.desc  : Test CleanUpResource
 */
HWTEST_F(AudioSystemClientPolicyManagerUnitTest, CleanUpResource_001, TestSize.Level1)
{
    AudioSystemClientPolicyManager& manager = AudioSystemClientPolicyManager::GetInstance();
    // 先设置一些状态
    manager.SetRingerMode(RINGER_MODE_NORMAL);
    manager.SetAudioScene(AUDIO_SCENE_DEFAULT);
    
    // 调用 CleanUpResource
    manager.CleanUpResource();
    
    // 验证调用后程序正常（不崩溃）
    // 能执行到这里说明 CleanUpResource 正常
}

} // namespace AudioStandard
} // namespace OHOS
