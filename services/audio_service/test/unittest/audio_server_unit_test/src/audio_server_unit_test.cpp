/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "audio_server_unit_test.h"

#include "accesstoken_kit.h"
#include "audio_device_info.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_process_config.h"
#include "audio_server.h"
#include "audio_service.h"
#include "audio_stream_info.h"
#include "policy_handler.h"
#include "system_ability_definition.h"
#include "iservice_registry.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
const int32_t SYSTEM_ABILITY_ID = 3001;
const bool RUN_ON_CREATE = false;

static std::shared_ptr<AudioServer> audioServer;

void AudioServerUnitTest::SetUpTestCase(void)
{
    audioServer = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    audioServer->OnDump();
}

void AudioServerUnitTest::TearDownTestCase(void)
{
    audioServer->OnStop();
}

void AudioServerUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioServerUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name  : Test OnAddSystemAbility API
 * @tc.type  : FUNC
 * @tc.number: AudioServerOnAddSystemAbility_001
 * @tc.desc  : Test OnAddSystemAbility interface. Set systemAbilityId is -1, deviceId is "".
 */
HWTEST_F(AudioServerUnitTest, AudioServerOnAddSystemAbility_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    int32_t id = -1;
    audioServer->OnAddSystemAbility(id, "");
    audioServer->OnAddSystemAbility(AUDIO_POLICY_SERVICE_ID, "");
    audioServer->OnAddSystemAbility(RES_SCHED_SYS_ABILITY_ID, "");
    audioServer->OnAddSystemAbility(MEMORY_MANAGER_SA_ID, "");
    audioServer->RecognizeAudioEffectType("", "", "");
    EXPECT_GT(0, id);
}

/**
 * @tc.name  : Test SetExtraParameters API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetExtraParameters_001
 * @tc.desc  : Test SetExtraParameters interface. Set key is "", kvpairs is empty.
 */
HWTEST_F(AudioServerUnitTest, AudioServerSetExtraParameters_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    std::vector<std::pair<std::string, std::string>> kvpairs;
    int32_t ret = audioServer->SetExtraParameters("PCM_DUMP", kvpairs);
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetAsrAecMode API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetAsrAecModer_001
 * @tc.desc  : Test SetAsrAecMode interface. Set asrAecMode is BYPASS, value and asrAecMode is STANDARD.
 */
HWTEST_F(AudioServerUnitTest, AudioServerSetAsrAecModer_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    int32_t ret = audioServer->SetAsrAecMode(AsrAecMode::BYPASS);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioServer->SetAsrAecMode(AsrAecMode::STANDARD);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SuspendRenderSink API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSuspendRenderSink_001
 * @tc.desc  : Test SuspendRenderSink interface. Set SinkName is "primary".
 */
HWTEST_F(AudioServerUnitTest, AudioServerSuspendRenderSink_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    int32_t ret = audioServer->SuspendRenderSink("primary");
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test SuspendRenderSink API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSuspendRenderSink_001
 * @tc.desc  : Test SuspendRenderSink interface. Set SinkName is "primary".
 */
HWTEST_F(AudioServerUnitTest, AudioServerGetAsrNoiseSuppressionMode_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    AsrNoiseSuppressionMode asrNoiseSuppressionMode;
    int32_t ret = audioServer->GetAsrNoiseSuppressionMode(asrNoiseSuppressionMode);
    EXPECT_NE(SUCCESS, ret);

    ret = audioServer->SetAsrNoiseSuppressionMode(AsrNoiseSuppressionMode::BYPASS);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioServer->SetAsrNoiseSuppressionMode(AsrNoiseSuppressionMode::FAR_FIELD);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioServer->GetAsrNoiseSuppressionMode(asrNoiseSuppressionMode);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioServer->SetAsrNoiseSuppressionMode(static_cast<AsrNoiseSuppressionMode>(4));
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetAsrWhisperDetectionMode API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetAsrWhisperDetectionMode_001
 * @tc.desc  : Test SetAsrWhisperDetectionMode interface. Set AsrWhisperDetectionMode is BYPASS
 *          and Set AsrWhisperDetectionMode is 4.
 */
HWTEST_F(AudioServerUnitTest, AudioServerSetAsrWhisperDetectionMode_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    AsrWhisperDetectionMode asrWhisperDetectionMode;
    int32_t ret = audioServer->GetAsrWhisperDetectionMode(asrWhisperDetectionMode);
    EXPECT_NE(SUCCESS, ret);

    ret = audioServer->SetAsrWhisperDetectionMode(AsrWhisperDetectionMode::BYPASS);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioServer->SetAsrWhisperDetectionMode(static_cast<AsrWhisperDetectionMode>(4));
    EXPECT_NE(SUCCESS, ret);

    ret = audioServer->GetAsrWhisperDetectionMode(asrWhisperDetectionMode);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetAsrVoiceControlMode API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetAsrVoiceControlMode_001
 * @tc.desc  : Test SetAsrVoiceControlMode interface. Set AsrVoiceControlMode is AUDIO_2_VOICETX, on is true and
 *          set AsrVoiceControlMode is AUDIO_2_VOICETX, on is false, and set AsrVoiceControlMode is 4, on is true,
 *          and set AsrVoiceControlMode is 4, on is false.
 */
HWTEST_F(AudioServerUnitTest, AudioServerSetAsrVoiceControlMode_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    int32_t ret = audioServer->SetAsrVoiceControlMode(AsrVoiceControlMode::AUDIO_2_VOICETX, true);

    EXPECT_EQ(SUCCESS, ret);
    ret = audioServer->SetAsrVoiceControlMode(AsrVoiceControlMode::AUDIO_2_VOICETX, false);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioServer->SetAsrVoiceControlMode(static_cast<AsrVoiceControlMode>(4), true);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioServer->SetAsrVoiceControlMode(static_cast<AsrVoiceControlMode>(4), false);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetSetAsrVoiceMuteMode API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetAsrVoiceMuteMode_001
 * @tc.desc  : Test SetAsrVoiceMuteMode interface. Set AsrVoiceMuteMode is OUTPUT_MUTE, on is true and
 *          set AsrVoiceMuteMode is OUTPUT_MUTE, on is false, and set AsrVoiceMuteMode is 5, on is true,
 *          and set AsrVoiceMuteMode is 5, on is false.
 */
HWTEST_F(AudioServerUnitTest, AudioServerSetAsrVoiceMuteMode_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    int32_t ret = audioServer->SetAsrVoiceMuteMode(AsrVoiceMuteMode::OUTPUT_MUTE, true);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioServer->SetAsrVoiceMuteMode(AsrVoiceMuteMode::OUTPUT_MUTE, false);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioServer->SetAsrVoiceMuteMode(static_cast<AsrVoiceMuteMode>(5), true);
    EXPECT_NE(SUCCESS, ret);

    ret = audioServer->SetAsrVoiceMuteMode(static_cast<AsrVoiceMuteMode>(5), false);
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test IsWhispering API
 * @tc.type  : FUNC
 * @tc.number: AudioServerIsWhispering_001
 * @tc.desc  : Test IsWhispering interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerIsWhispering_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    int32_t ret = audioServer->IsWhispering();
    EXPECT_EQ(0, ret);
}

/**
 * @tc.name  : Test GetExtraParameters API
 * @tc.type  : FUNC
 * @tc.number: AudioServerGetExtraParameters_001
 * @tc.desc  : Test GetExtraParameters interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerGetExtraParameters_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    std::vector<std::pair<std::string, std::string>> result;
    std::vector<std::string> subKeys;
    int32_t ret = audioServer->GetExtraParameters("", subKeys, result);
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test GetExtraParameters API
 * @tc.type  : FUNC
 * @tc.number: AudioServerGetExtraParameters_002
 * @tc.desc  : Test GetExtraParameters interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerGetExtraParameters_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    std::vector<std::pair<std::string, std::string>> result;
    std::vector<std::string> subKeys;
    std::string mainKey = "PCM_DUMP";
    int32_t ret = audioServer->GetExtraParameters(mainKey, subKeys, result);
    EXPECT_EQ(ERROR, ret);
}

/**
 * @tc.name  : Test GetExtraParameters API
 * @tc.type  : FUNC
 * @tc.number: AudioServerGetExtraParameters_003
 * @tc.desc  : Test GetExtraParameters interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerGetExtraParameters_003, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    std::vector<std::pair<std::string, std::string>> result;
    std::vector<std::string> subKeys;
    std::string mainKey = "test_003";
    AudioServer::audioParameterKeys = {
        {
            "Category1", {
                {"Key1", {"Value1", "Value2"}}
            }
        }
    };
    int32_t ret = audioServer->GetExtraParameters(mainKey, subKeys, result);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    auto it = AudioServer::audioParameterKeys.find(mainKey);
    if (it != AudioServer::audioParameterKeys.end()) {
        AudioServer::audioParameterKeys.erase(mainKey);
    }
}

/**
 * @tc.name  : Test GetExtraParameters API
 * @tc.type  : FUNC
 * @tc.number: AudioServerGetExtraParameters_004
 * @tc.desc  : Test GetExtraParameters interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerGetExtraParameters_004, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    std::vector<std::pair<std::string, std::string>> result;
    std::vector<std::string> subKeys;
    std::string mainKey = "test_004";
    AudioServer::audioParameterKeys = {
        {
            "test_004", {
                {"Key1", {"Value1", "Value2"}}
            }
        }
    };
    int32_t ret = audioServer->GetExtraParameters(mainKey, subKeys, result);
    EXPECT_EQ(SUCCESS, ret);
    auto it = AudioServer::audioParameterKeys.find(mainKey);
    if (it != AudioServer::audioParameterKeys.end()) {
        AudioServer::audioParameterKeys.erase(mainKey);
    }
}

/**
 * @tc.name  : Test GetExtraParameters API
 * @tc.type  : FUNC
 * @tc.number: AudioServerGetExtraParameters_005
 * @tc.desc  : Test GetExtraParameters interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerGetExtraParameters_005, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    std::vector<std::pair<std::string, std::string>> result;
    std::vector<std::string> subKeys = {"test_005"};
    std::string mainKey = "test_005";
    AudioServer::audioParameterKeys = {
        {
            "test_005", {
                {"test_005", {"Value1", "Value2"}}
            }
        }
    };
    int32_t ret = audioServer->GetExtraParameters(mainKey, subKeys, result);
    EXPECT_EQ(SUCCESS, ret);
    auto it = AudioServer::audioParameterKeys.find(mainKey);
    if (it != AudioServer::audioParameterKeys.end()) {
        AudioServer::audioParameterKeys.erase(mainKey);
    }
}

/**
 * @tc.name  : Test GetExtraParameters API
 * @tc.type  : FUNC
 * @tc.number: AudioServerGetExtraParameters_006
 * @tc.desc  : Test GetExtraParameters interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerGetExtraParameters_006, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    std::vector<std::pair<std::string, std::string>> result;
    std::vector<std::string> subKeys = {""};
    std::string mainKey = "test_006";
    AudioServer::audioParameterKeys = {
        {
            "test_006", {
                {"test_006", {"Value1", "Value2"}}
            }
        }
    };
    int32_t ret = audioServer->GetExtraParameters(mainKey, subKeys, result);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    auto it = AudioServer::audioParameterKeys.find(mainKey);
    if (it != AudioServer::audioParameterKeys.end()) {
        AudioServer::audioParameterKeys.erase(mainKey);
    }
}

/**
 * @tc.name  : Test CheckAndPrintStacktrace API
 * @tc.type  : FUNC
 * @tc.number: AudioServerGetExtraParameters_001
 * @tc.desc  : Test CheckAndPrintStacktrace interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerCheckAndPrintStacktrace, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    EXPECT_TRUE(audioServer->CheckAndPrintStacktrace("dump_pulseaudio_stacktrace"));

    EXPECT_TRUE(audioServer->CheckAndPrintStacktrace("recovery_audio_server"));

    EXPECT_TRUE(audioServer->CheckAndPrintStacktrace("dump_pa_stacktrace_and_kill"));

    EXPECT_FALSE(audioServer->CheckAndPrintStacktrace(""));
}

/**
 * @tc.name  : Test GetAudioParameter API
 * @tc.type  : FUNC
 * @tc.number: AudioServerGetAudioParameter_001
 * @tc.desc  : Test GetAudioParameter interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerGetAudioParameter_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    audioServer->SetAudioParameter("A2dpSuspended", "");
    audioServer->SetAudioParameter("AUDIO_EXT_PARAM_KEY_LOWPOWER", "");
    audioServer->SetAudioParameter("bt_headset_nrec", "");
    audioServer->SetAudioParameter("bt_wbs", "");
    audioServer->SetAudioParameter("AUDIO_EXT_PARAM_KEY_A2DP_OFFLOAD_CONFIG", "");
    audioServer->SetAudioParameter("mmi", "");
    audioServer->SetAudioParameter("perf_info", "");
    audioServer->GetAudioParameter("");
    audioServer->GetAudioParameter("AUDIO_EXT_PARAM_KEY_LOWPOWER");
    audioServer->GetAudioParameter("perf_info");
    audioServer->GetAudioParameter("getSmartPAPOWER");
    audioServer->GetAudioParameter("Is_Fast_Blocked_For_AppName#");
    auto result = audioServer->GetUsbParameter("address=card2;device=0 role=1");
    audioServer->GetAudioParameter(LOCAL_NETWORK_ID, AudioParamKey::USB_DEVICE, "address=card=2;device=0 role=0");
    audioServer->GetAudioParameter(LOCAL_NETWORK_ID, AudioParamKey::USB_DEVICE, "address=card=2;device=0 role=1");
    audioServer->GetAudioParameter(LOCAL_NETWORK_ID, AudioParamKey::USB_DEVICE, "address=card=2;device=0 role=2");
    audioServer->GetAudioParameter(LOCAL_NETWORK_ID, AudioParamKey::GET_DP_DEVICE_INFO, "");
    audioServer->GetAudioParameter("", AudioParamKey::GET_DP_DEVICE_INFO, "");
}

/**
 * @tc.name  : Test GetTransactionId API
 * @tc.type  : FUNC
 * @tc.number: AudioServerGetTransactionId_001
 * @tc.desc  : Test GetTransactionId interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerGetTransactionId_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    std::vector<std::pair<std::string, std::string>> result;
    uint64_t ret = audioServer->GetTransactionId(DeviceType::DEVICE_TYPE_USB_ARM_HEADSET, DeviceRole::DEVICE_ROLE_MAX);
    EXPECT_NE(0, ret);

    ret = audioServer->GetTransactionId(DeviceType::DEVICE_TYPE_USB_ARM_HEADSET, DeviceRole::INPUT_DEVICE);
    EXPECT_EQ(0, ret);

    ret = audioServer->GetTransactionId(DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE);
    EXPECT_EQ(0, ret);

    ret = audioServer->GetTransactionId(DeviceType::DEVICE_TYPE_USB_ARM_HEADSET, DeviceRole::OUTPUT_DEVICE);
    EXPECT_EQ(0, ret);
}

/**
 * @tc.name  : Test SetAudioScene API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetAudioScene_001
 * @tc.desc  : Test SetAudioScene interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerSetAudioScene_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    std::vector<DeviceType> activeOutputDevices;
    activeOutputDevices.push_back(DEVICE_TYPE_USB_ARM_HEADSET);
    int32_t ret = audioServer->SetAudioScene(AUDIO_SCENE_INVALID, activeOutputDevices, DEVICE_TYPE_USB_ARM_HEADSET,
        NO_A2DP_DEVICE);
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetIORoutes API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetIORoutes_001
 * @tc.desc  : Test SetIORoutes interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerSetIORoutes_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    std::vector<DeviceType> deviceTypes;
    std::vector<DeviceType> activeOutputDevices;

    activeOutputDevices.push_back(DEVICE_TYPE_USB_ARM_HEADSET);
    int32_t ret = audioServer->SetAudioScene(AUDIO_SCENE_DEFAULT, activeOutputDevices, DEVICE_TYPE_USB_ARM_HEADSET,
        A2DP_OFFLOAD);

    ret = audioServer->SetIORoutes(DEVICE_TYPE_USB_ARM_HEADSET, DeviceFlag::ALL_DEVICES_FLAG, deviceTypes,
        A2DP_OFFLOAD);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioServer->SetAudioScene(AUDIO_SCENE_INVALID, activeOutputDevices, DEVICE_TYPE_USB_ARM_HEADSET,
        A2DP_OFFLOAD);
    activeOutputDevices.clear();
    activeOutputDevices.push_back(DEVICE_TYPE_BLUETOOTH_A2DP);
    ret = audioServer->SetIORoutes(DEVICE_TYPE_USB_ARM_HEADSET, DeviceFlag::ALL_DEVICES_FLAG, deviceTypes,
        A2DP_OFFLOAD);
    EXPECT_EQ(SUCCESS, ret);

    deviceTypes.push_back(DEVICE_TYPE_BLUETOOTH_A2DP);
    ret = audioServer->SetIORoutes(DEVICE_TYPE_BLUETOOTH_A2DP, DeviceFlag::OUTPUT_DEVICES_FLAG, deviceTypes,
        A2DP_NOT_OFFLOAD);
    EXPECT_EQ(SUCCESS, ret);

    deviceTypes.clear();
    deviceTypes.push_back(DEVICE_TYPE_WIRED_HEADPHONES);
    ret = audioServer->SetIORoutes(DEVICE_TYPE_BLUETOOTH_A2DP, DeviceFlag::OUTPUT_DEVICES_FLAG, deviceTypes,
        A2DP_NOT_OFFLOAD);
    EXPECT_EQ(SUCCESS, ret);
}


/**
 * @tc.name  : Test CheckStreamInfoFormat API
 * @tc.type  : FUNC
 * @tc.number: AudioServerCheckStreamInfoFormat_001
 * @tc.desc  : Test CheckStreamInfoFormat interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerCheckStreamInfoFormat_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    audioServer->NotifyDeviceInfo(LOCAL_NETWORK_ID, true);
    audioServer->NotifyDeviceInfo(LOCAL_NETWORK_ID, false);
    audioServer->NotifyDeviceInfo("", true);
    audioServer->NotifyDeviceInfo("", false);

    AudioProcessConfig config = {};
    config.callerUid = AudioServer::MEDIA_SERVICE_UID;
    config.capturerInfo.sourceType = SourceType::SOURCE_TYPE_WAKEUP;
    audioServer->ResetRecordConfig(config);

    config.audioMode = AUDIO_MODE_RECORD;
    config.streamInfo.channels = CHANNEL_11;
    config.streamInfo.channelLayout = CH_LAYOUT_MONO;
    config.streamInfo.encoding = ENCODING_PCM;
    config.streamInfo.format = SAMPLE_U8;
    config.streamInfo.samplingRate = SAMPLE_RATE_8000;
    bool ret = audioServer->CheckStreamInfoFormat(config);
    EXPECT_FALSE(ret);

    config.audioMode = AUDIO_MODE_PLAYBACK;
    ret = audioServer->CheckStreamInfoFormat(config);
    EXPECT_FALSE(ret);

    config.streamInfo.channelLayout = static_cast<AudioChannelLayout>(-1);
    ret = audioServer->CheckStreamInfoFormat(config);
    EXPECT_FALSE(ret);

    config.streamInfo.encoding = ENCODING_INVALID;
    ret = audioServer->CheckStreamInfoFormat(config);
    EXPECT_FALSE(ret);

    config.streamInfo.format = INVALID_WIDTH;
    ret = audioServer->CheckStreamInfoFormat(config);
    EXPECT_FALSE(ret);

    config.streamInfo.samplingRate = static_cast<AudioSamplingRate>(-1);
    ret = audioServer->CheckStreamInfoFormat(config);
    EXPECT_FALSE(ret);

    config.rendererInfo.streamUsage = STREAM_USAGE_INVALID;
    ret = audioServer->CheckRendererFormat(config);
    EXPECT_FALSE(ret);

    config.rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    ret = audioServer->CheckRendererFormat(config);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test CheckConfigFormat API
 * @tc.type  : FUNC
 * @tc.number: AudioServerCheckConfigFormat_001
 * @tc.desc  : Test CheckConfigFormat interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerCheckConfigFormat_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    AudioProcessConfig config = {};
    config.rendererInfo.streamUsage = STREAM_USAGE_INVALID;
    config.audioMode = static_cast<AudioMode>(-1);
    config.streamInfo.channels = MONO;
    config.streamInfo.channelLayout = CH_LAYOUT_MONO;
    config.streamInfo.encoding = ENCODING_PCM;
    config.streamInfo.format = SAMPLE_U8;
    config.streamInfo.samplingRate = SAMPLE_RATE_8000;
    bool ret = audioServer->CheckConfigFormat(config);
    EXPECT_FALSE(ret);

    config.streamInfo.format = INVALID_WIDTH;
    ret = audioServer->CheckConfigFormat(config);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test CheckConfigFormat API
 * @tc.type  : FUNC
 * @tc.number: AudioServerCheckConfigFormat_002
 * @tc.desc  : Test CheckConfigFormat interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerCheckConfigFormat_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    AudioProcessConfig config = {};
    config.rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    config.audioMode = AUDIO_MODE_PLAYBACK;
    config.streamInfo.channels = MONO;
    config.streamInfo.channelLayout = CH_LAYOUT_MONO;
    config.streamInfo.encoding = ENCODING_PCM;
    config.streamInfo.format = SAMPLE_U8;
    config.streamInfo.samplingRate = SAMPLE_RATE_8000;
    bool ret = audioServer->CheckConfigFormat(config);
    EXPECT_TRUE(ret);

    config.audioMode = AUDIO_MODE_RECORD;
    config.capturerInfo.sourceType = SOURCE_TYPE_MIC_REF;
    ret = audioServer->CheckConfigFormat(config);
    EXPECT_FALSE(ret);
    config.capturerInfo.sourceType = SOURCE_TYPE_INVALID;
    config.capturerInfo.capturerFlags = AUDIO_FLAG_MMAP;
    ret = audioServer->CheckConfigFormat(config);
    EXPECT_FALSE(ret);
    config.capturerInfo.capturerFlags = AUDIO_FLAG_NORMAL;
    ret = audioServer->CheckConfigFormat(config);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test CheckRemoteDeviceState API
 * @tc.type  : FUNC
 * @tc.number: AudioServerCheckRemoteDeviceState_001
 * @tc.desc  : Test CheckRemoteDeviceState interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerCheckRemoteDeviceState_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    audioServer->CheckRemoteDeviceState(LOCAL_NETWORK_ID, DeviceRole::OUTPUT_DEVICE, true);
    audioServer->CheckRemoteDeviceState(LOCAL_NETWORK_ID, DeviceRole::INPUT_DEVICE, true);
    int32_t ret = audioServer->CheckRemoteDeviceState(LOCAL_NETWORK_ID, DeviceRole::DEVICE_ROLE_MAX, true);
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test PermissionChecker API
 * @tc.type  : FUNC
 * @tc.number: AudioServerPermissionChecker_001
 * @tc.desc  : Test PermissionChecker interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerPermissionChecker_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    AudioProcessConfig config = {};
    config.audioMode = static_cast<AudioMode>(-1);
    bool ret = audioServer->PermissionChecker(config);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test PermissionChecker API
 * @tc.type  : FUNC
 * @tc.number: AudioServerPermissionChecker_002
 * @tc.desc  : Test PermissionChecker interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerPermissionChecker_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    AudioProcessConfig config = {};
    config.audioMode = AUDIO_MODE_PLAYBACK;
    config.rendererInfo.streamUsage = STREAM_USAGE_SYSTEM;
    bool ret = audioServer->PermissionChecker(config);
    EXPECT_TRUE(ret);
    config.rendererInfo.streamUsage = STREAM_USAGE_UNKNOWN;
    ret = audioServer->PermissionChecker(config);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test PermissionChecker API
 * @tc.type  : FUNC
 * @tc.number: AudioServerPermissionChecker_003
 * @tc.desc  : Test PermissionChecker interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerPermissionChecker_003, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    AudioProcessConfig config = {};
    config.audioMode = AUDIO_MODE_RECORD;
    bool ret = audioServer->PermissionChecker(config);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test CheckRecorderPermission API
 * @tc.type  : FUNC
 * @tc.number: AudioServerCheckRecorderPermission_001
 * @tc.desc  : Test CheckRecorderPermission interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerCheckRecorderPermission_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    AudioProcessConfig config = {};
    config.audioMode = static_cast<AudioMode>(-1);
    config.appInfo.appUid = INVALID_UID;
    config.capturerInfo.sourceType = SOURCE_TYPE_MIC;
    bool ret = audioServer->CheckRecorderPermission(config);
    EXPECT_FALSE(ret);

    config.capturerInfo.sourceType = SOURCE_TYPE_WAKEUP;
    ret = audioServer->CheckRecorderPermission(config);
    EXPECT_TRUE(ret);

    config.capturerInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    config.innerCapMode = MODERN_INNER_CAP;
    ret = audioServer->CheckRecorderPermission(config);
    EXPECT_TRUE(ret);

    config.innerCapMode = INVALID_CAP_MODE;
    ret = audioServer->CheckRecorderPermission(config);
    EXPECT_TRUE(ret);

    config.capturerInfo.sourceType = SOURCE_TYPE_REMOTE_CAST;
    ret = audioServer->CheckRecorderPermission(config);
    EXPECT_TRUE(ret);

    config.capturerInfo.sourceType = SOURCE_TYPE_VOICE_CALL;
    ret = audioServer->CheckRecorderPermission(config);
    EXPECT_TRUE(ret);

    config.appInfo.appUid = 0;
    ret = audioServer->CheckRecorderPermission(config);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test CreatePlaybackCapturerManager API
 * @tc.type  : FUNC
 * @tc.number: AudioServeCreatePlaybackCapturerManager_001
 * @tc.desc  : Test CreatePlaybackCapturerManager interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServeCreatePlaybackCapturerManager_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    bool ret = audioServer->CreatePlaybackCapturerManager();
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test GetMaxAmplitude API
 * @tc.type  : FUNC
 * @tc.number: AudioServerGetMaxAmplitude_001
 * @tc.desc  : Test GetMaxAmplitude interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerGetMaxAmplitude_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    std::string timestamp = "";
    audioServer->UpdateLatencyTimestamp(timestamp, true);
    audioServer->UpdateLatencyTimestamp(timestamp, false);
    float ret = audioServer->GetMaxAmplitude(false, "usb", SOURCE_TYPE_MIC);
    EXPECT_EQ(0, ret);

    ret = audioServer->GetMaxAmplitude(false, "a2dp", SOURCE_TYPE_MIC);
    EXPECT_EQ(0, ret);

    ret = audioServer->GetMaxAmplitude(true, "usb", SOURCE_TYPE_INVALID);
    EXPECT_EQ(0, ret);

    ret = audioServer->GetMaxAmplitude(true, "a2dp", SOURCE_TYPE_INVALID);
    EXPECT_EQ(0, ret);
}

/**
 * @tc.name  : Test UpdateDualToneState API
 * @tc.type  : FUNC
 * @tc.number: AudioServerUpdateDualToneState_001
 * @tc.desc  : Test UpdateDualToneState interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerUpdateDualToneState_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    int32_t ret = audioServer->UpdateDualToneState(false, 123);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioServer->UpdateDualToneState(true, 123);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetSinkRenderEmpty API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetSinkRenderEmpty_001
 * @tc.desc  : Test SetSinkRenderEmpty interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerSetSinkRenderEmpty_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    int32_t ret = audioServer->SetSinkRenderEmpty("primary", 0);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioServer->SetSinkRenderEmpty("primary", 1);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetSinkMuteForSwitchDevice API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetSinkMuteForSwitchDevice_001
 * @tc.desc  : Test SetSinkMuteForSwitchDevice interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerSetSinkMuteForSwitchDevice_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    int32_t ret = audioServer->SetSinkMuteForSwitchDevice("primary", 1, false);
    EXPECT_NE(SUCCESS, ret);

    ret = audioServer->SetSinkMuteForSwitchDevice("primary", 0, false);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test RestoreSession API
 * @tc.type  : FUNC
 * @tc.number: AudioServerRestoreSession_001
 * @tc.desc  : Test RestoreSession interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerRestoreSession_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    RestoreInfo restoreInfo;
    audioServer->RestoreSession(-1, restoreInfo);
    audioServer->RestoreSession(-1, restoreInfo);
}

/**
 * @tc.name  : Test RegiestPolicyProvider API
 * @tc.type  : FUNC
 * @tc.number: AudioServerRegiestPolicyProvider_001
 * @tc.desc  : Test RegiestPolicyProvider interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerRegiestPolicyProvider_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(nullptr, samgr);
    sptr<IRemoteObject> object = nullptr;
    int32_t ret = audioServer->RegiestPolicyProvider(object);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
    object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    EXPECT_NE(nullptr, object);
    ret = audioServer->RegiestPolicyProvider(object);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : Test CreateAudioProcess API
 * @tc.type  : FUNC
 * @tc.number: AudioServerCreateAudioProcess_001
 * @tc.desc  : Test CreateAudioProcess interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerCreateAudioProcess_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    AudioProcessConfig config;
    config.audioMode = AUDIO_MODE_RECORD;
    int32_t errorCode = 0;
    audioServer->CreateAudioProcess(config, errorCode);
    EXPECT_EQ(errorCode, 0);
}

/**
 * @tc.name  : Test CreateAudioProcess API
 * @tc.type  : FUNC
 * @tc.number: AudioServerCreateAudioProcess_002
 * @tc.desc  : Test CreateAudioProcess interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerCreateAudioProcess_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    AudioProcessConfig config;
    int32_t errorCode = 0;
    audioServer->waitCreateStreamInServerCount_ = 6;
    audioServer->CreateAudioProcess(config, errorCode);
    EXPECT_EQ(errorCode, ERR_RETRY_IN_CLIENT);
}

/**
 * @tc.name  : Test ResetRecordConfig API
 * @tc.type  : FUNC
 * @tc.number: AudioServerResetRecordConfig_001
 * @tc.desc  : Test ResetRecordConfig interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerResetRecordConfig_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    AudioProcessConfig config;
    config.capturerInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    audioServer->ResetRecordConfig(config);
    config.capturerInfo.sourceType = SOURCE_TYPE_WAKEUP;
    audioServer->ResetRecordConfig(config);
}

/**
 * @tc.name  : Test CreateAudioStream API
 * @tc.type  : FUNC
 * @tc.number: AudioServerCreateAudioStream_001
 * @tc.desc  : Test CreateAudioStream interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerCreateAudioStream_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    AudioProcessConfig config;
    sptr<IRemoteObject> remoteObject = nullptr;
    remoteObject = audioServer->CreateAudioStream(config, AudioServer::VASSISTANT_UID);
    remoteObject = audioServer->CreateAudioStream(config, AudioServer::MEDIA_SERVICE_UID);
    EXPECT_EQ(nullptr, remoteObject);
    config.audioMode = AUDIO_MODE_RECORD;
    remoteObject = audioServer->CreateAudioStream(config, AudioServer::MEDIA_SERVICE_UID);
    EXPECT_EQ(nullptr, remoteObject);
    bool ret = audioServer->IsFastBlocked(1, PLAYER_TYPE_DEFAULT);
    EXPECT_EQ(false, ret);
    config.audioMode = static_cast<AudioMode>(-1);
    ret = audioServer->IsNormalIpcStream(config);
    EXPECT_EQ(false, ret);
    AudioParamKey key = NONE;
    audioServer->OnRenderSinkParamChange("", key, "", "");
    audioServer->OnCaptureSourceParamChange("", key, "", "");
    audioServer->OnWakeupClose();
    audioServer->OnCapturerState(true, 1);
    audioServer->OnCapturerState(false, 1);
    int32_t res = audioServer->SetParameterCallback(remoteObject);
    EXPECT_EQ(res, ERR_INVALID_PARAM);
    res = audioServer->SetWakeupSourceCallback(remoteObject);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name  : Test SetAudioEffectProperty API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetAudioEffectProperty_001
 * @tc.desc  : Test SetAudioEffectProperty interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerSetAudioEffectProperty_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    AudioEffectPropertyV3  audioEffectPropertyV31 = {
        .name = "testName1",
        .category = "testCategory1",
        .flag = RENDER_EFFECT_FLAG,
    };

    AudioEffectPropertyV3  audioEffectPropertyV32 = {
        .name = "testName2",
        .category = "testCategory2",
        .flag = RENDER_EFFECT_FLAG,
    };

    AudioEffectPropertyArrayV3 audioEffectPropertyArrayV3 = {};
    audioEffectPropertyArrayV3.property.push_back(audioEffectPropertyV31);
    audioEffectPropertyArrayV3.property.push_back(audioEffectPropertyV32);

    int32_t ret = audioServer->SetAudioEffectProperty(audioEffectPropertyArrayV3);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetAudioEffectProperty API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetAudioEffectProperty_002
 * @tc.desc  : Test SetAudioEffectProperty interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerSetAudioEffectProperty_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    AudioEffectPropertyV3  audioEffectPropertyV31 = {
        .name = "testName1",
        .category = "testCategory1",
        .flag = CAPTURE_EFFECT_FLAG,
    };

    AudioEffectPropertyV3  audioEffectPropertyV32 = {
        .name = "testName2",
        .category = "testCategory2",
        .flag = CAPTURE_EFFECT_FLAG,
    };

    AudioEffectPropertyArrayV3 audioEffectPropertyArrayV3 = {};
    audioEffectPropertyArrayV3.property.push_back(audioEffectPropertyV31);
    audioEffectPropertyArrayV3.property.push_back(audioEffectPropertyV32);

    int32_t ret = audioServer->SetAudioEffectProperty(audioEffectPropertyArrayV3);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetAudioEffectProperty API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetAudioEffectProperty_003
 * @tc.desc  : Test SetAudioEffectProperty interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerSetAudioEffectProperty_003, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    AudioEffectPropertyV3  audioEffectPropertyV31 = {
        .name = "testName1",
        .category = "testCategory1",
        .flag = RENDER_EFFECT_FLAG,
    };

    AudioEffectPropertyV3  audioEffectPropertyV32 = {
        .name = "testName2",
        .category = "testCategory2",
        .flag = CAPTURE_EFFECT_FLAG,
    };

    AudioEffectPropertyArrayV3 audioEffectPropertyArrayV3 = {};
    audioEffectPropertyArrayV3.property.push_back(audioEffectPropertyV31);
    audioEffectPropertyArrayV3.property.push_back(audioEffectPropertyV32);

    int32_t ret = audioServer->SetAudioEffectProperty(audioEffectPropertyArrayV3);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test GetAudioEffectProperty API
 * @tc.type  : FUNC
 * @tc.number: AudioServerGetAudioEffectProperty_001
 * @tc.desc  : Test GetAudioEffectProperty interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerGetAudioEffectProperty_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    AudioEffectPropertyArrayV3 audioEffectPropertyArrayV3 = {};

    int32_t ret = audioServer->GetAudioEffectProperty(audioEffectPropertyArrayV3);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test GetAudioEffectProperty API
 * @tc.type  : FUNC
 * @tc.number: AudioServerGetAudioEffectProperty_002
 * @tc.desc  : Test GetAudioEnhancePropertyArray interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerGetAudioEffectProperty_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    AudioEffectPropertyArrayV3 audioEffectPropertyArrayV3 = {};

    int32_t ret = audioServer->GetAudioEnhancePropertyArray(audioEffectPropertyArrayV3, DEVICE_TYPE_EARPIECE);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test GetAudioEffectProperty API
 * @tc.type  : FUNC
 * @tc.number: AudioServerGetAudioEffectProperty_003
 * @tc.desc  : Test GetAudioEffectPropertyArray interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerGetAudioEffectProperty_003, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    AudioEffectPropertyArrayV3 audioEffectPropertyArrayV3 = {};

    int32_t ret = audioServer->GetAudioEffectPropertyArray(audioEffectPropertyArrayV3);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test LoadAudioEffectLibraries API
 * @tc.type  : FUNC
 * @tc.number: AudioServerLoadAudioEffectLibraries_001
 * @tc.desc  : Test LoadAudioEffectLibraries interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerLoadAudioEffectLibraries_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    std::vector<Library> libraries;
    std::vector<Effect> effects;
    std::vector<Effect> successEffectList;
    int32_t ret = audioServer->LoadAudioEffectLibraries(libraries, effects, successEffectList);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test CheckParam API
 * @tc.type  : FUNC
 * @tc.number: CheckParam_001
 * @tc.desc  : Test CheckParam interface.
 */
HWTEST_F(AudioServerUnitTest, CheckParam_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    AudioProcessConfig config;
    config.rendererInfo.contentType = static_cast<ContentType>(-1);
    int32_t ret = audioServer->CheckParam(config);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
    config.rendererInfo.contentType = static_cast<ContentType>(100);
    ret = audioServer->CheckParam(config);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    config.rendererInfo.contentType = CONTENT_TYPE_ULTRASONIC;
    config.rendererInfo.streamUsage = static_cast<StreamUsage>(-1);
    ret = audioServer->CheckParam(config);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
    config.rendererInfo.streamUsage = static_cast<StreamUsage>(100);
    ret = audioServer->CheckParam(config);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
    config.rendererInfo.streamUsage = STREAM_USAGE_SYSTEM;
    ret = audioServer->CheckParam(config);
    EXPECT_EQ(ret, SUCCESS);
    config.rendererInfo.contentType = CONTENT_TYPE_MUSIC;
    ret = audioServer->CheckParam(config);
    EXPECT_EQ(ret, SUCCESS);
    config.rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    ret = audioServer->CheckParam(config);
    EXPECT_EQ(ret, SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS
