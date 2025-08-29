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
#include "audio_utils.h"
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
#include "audio_service_types.h"
#include "audio_server_hpae_dump.h"

using namespace testing::ext;
using OHOS::AudioStandard::SetSysPara;
namespace OHOS {
namespace AudioStandard {
const int32_t SYSTEM_ABILITY_ID = 3001;
const bool RUN_ON_CREATE = false;
constexpr int32_t INVALID_VALUE = -1;

static std::shared_ptr<AudioServer> audioServer;

void AudioServerUnitTest::SetUpTestCase(void)
{
    audioServer = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    EXPECT_NE(nullptr, audioServer);
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

enum PermissionStatus {
    PERMISSION_GRANTED = 0,
    PERMISSION_DENIED = 1,
    PERMISSION_UNKNOWN = 2,
};

class DataTransferStateChangeCallbackInnerTest : public DataTransferStateChangeCallbackInner {
public:
    void OnDataTransferStateChange(const int32_t &callbackId,
        const AudioRendererDataTransferStateChangeInfo &info) override {}

    void OnMuteStateChange(const int32_t &callbackId, const int32_t &uid,
        const uint32_t &sessionId, const bool &isMuted) override {}
};

class WakeUpSourceCallbackTest : public WakeUpSourceCallback {
public:
    void OnCapturerState(bool isActive) override {}
    void OnWakeupClose() override {}
};

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

    std::vector<StringPair> kvpairs;
    int32_t ret = audioServer->SetExtraParameters("PCM_DUMP", kvpairs);
    EXPECT_NE(SUCCESS, ret);
}

#ifdef TEMP_DISABLE
/**
 * @tc.name  : Test SetAsrAecMode API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetAsrAecModer_001
 * @tc.desc  : Test SetAsrAecMode interface. Set asrAecMode is BYPASS, value and asrAecMode is STANDARD.
 */
HWTEST_F(AudioServerUnitTest, AudioServerSetAsrAecModer_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    int32_t ret = audioServer->SetAsrAecMode(static_cast<int32_t>(AsrAecMode::BYPASS));
    EXPECT_EQ(SUCCESS, ret);

    ret = audioServer->SetAsrAecMode(static_cast<int32_t>(AsrAecMode::STANDARD));
    EXPECT_EQ(SUCCESS, ret);
}
#endif

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

#ifdef TEMP_DISABLE
/**
 * @tc.name  : Test SuspendRenderSink API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSuspendRenderSink_001
 * @tc.desc  : Test SuspendRenderSink interface. Set SinkName is "primary".
 */
HWTEST_F(AudioServerUnitTest, AudioServerGetAsrNoiseSuppressionMode_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    int32_t asrNoiseSuppressionMode = 0;
    int32_t ret = audioServer->GetAsrNoiseSuppressionMode(asrNoiseSuppressionMode);
    EXPECT_NE(SUCCESS, ret);

    ret = audioServer->SetAsrNoiseSuppressionMode(static_cast<int32_t>(AsrNoiseSuppressionMode::BYPASS));
    EXPECT_EQ(SUCCESS, ret);

    ret = audioServer->SetAsrNoiseSuppressionMode(static_cast<int32_t>(AsrNoiseSuppressionMode::FAR_FIELD));
    EXPECT_EQ(SUCCESS, ret);

    ret = audioServer->GetAsrNoiseSuppressionMode(asrNoiseSuppressionMode);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioServer->SetAsrNoiseSuppressionMode(static_cast<int32_t>(4));
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

    int32_t asrWhisperDetectionMode;
    int32_t ret = audioServer->GetAsrWhisperDetectionMode(asrWhisperDetectionMode);
    EXPECT_NE(SUCCESS, ret);

    ret = audioServer->SetAsrWhisperDetectionMode(static_cast<int32_t>(AsrWhisperDetectionMode::BYPASS));
    EXPECT_EQ(SUCCESS, ret);

    ret = audioServer->SetAsrWhisperDetectionMode(static_cast<int32_t>(4));
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
    int32_t ret = audioServer->SetAsrVoiceControlMode(
        static_cast<int32_t>(AsrVoiceControlMode::AUDIO_2_VOICETX), true);

    EXPECT_EQ(SUCCESS, ret);
    ret = audioServer->SetAsrVoiceControlMode(static_cast<int32_t>(AsrVoiceControlMode::AUDIO_2_VOICETX), false);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioServer->SetAsrVoiceControlMode(4, true);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioServer->SetAsrVoiceControlMode(4, false);
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

    int32_t ret = audioServer->SetAsrVoiceMuteMode(static_cast<int32_t>(AsrVoiceMuteMode::OUTPUT_MUTE), true);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioServer->SetAsrVoiceMuteMode(static_cast<int32_t>(AsrVoiceMuteMode::OUTPUT_MUTE), false);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioServer->SetAsrVoiceMuteMode(5, true);
    EXPECT_NE(SUCCESS, ret);

    ret = audioServer->SetAsrVoiceMuteMode(5, false);
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

    int32_t ret = 0;
    audioServer->IsWhispering(ret);
    EXPECT_EQ(0, ret);
}
#endif

/**
 * @tc.name  : Test GetExtraParameters API
 * @tc.type  : FUNC
 * @tc.number: AudioServerGetExtraParameters_001
 * @tc.desc  : Test GetExtraParameters interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerGetExtraParameters_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    std::vector<StringPair> result;
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

    std::vector<StringPair> result;
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

    std::vector<StringPair> result;
    std::vector<std::string> subKeys;
    std::string mainKey = "test_003";
    AudioServer::audioParameterKeys = {
        {
            "Category1", {
                {"Key1", {"Value1", "Value2"}}
            }
        }
    };
    audioServer->isAudioParameterParsed_.store(true);
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

    std::vector<StringPair> result;
    std::vector<std::string> subKeys;
    std::string mainKey = "test_004";
    AudioServer::audioParameterKeys = {
        {
            "test_004", {
                {"Key1", {"Value1", "Value2"}}
            }
        }
    };
    audioServer->isAudioParameterParsed_.store(true);
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

    std::vector<StringPair> result;
    std::vector<std::string> subKeys = {"test_005"};
    std::string mainKey = "test_005";
    AudioServer::audioParameterKeys = {
        {
            "test_005", {
                {"test_005", {"Value1", "Value2"}}
            }
        }
    };
    audioServer->isAudioParameterParsed_.store(true);
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

    std::vector<StringPair> result;
    std::vector<std::string> subKeys = {""};
    std::string mainKey = "test_006";
    AudioServer::audioParameterKeys = {
        {
            "test_006", {
                {"test_006", {"Value1", "Value2"}}
            }
        }
    };
    audioServer->isAudioParameterParsed_.store(true);
    int32_t ret = audioServer->GetExtraParameters(mainKey, subKeys, result);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
    auto it = AudioServer::audioParameterKeys.find(mainKey);
    if (it != AudioServer::audioParameterKeys.end()) {
        AudioServer::audioParameterKeys.erase(mainKey);
    }
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

    string str = "test";
    audioServer->SetAudioParameter("A2dpSuspended", "");
    audioServer->SetAudioParameter("AUDIO_EXT_PARAM_KEY_LOWPOWER", "");
    audioServer->SetAudioParameter("bt_headset_nrec", "");
    audioServer->SetAudioParameter("bt_wbs", "");
    audioServer->SetAudioParameter("AUDIO_EXT_PARAM_KEY_A2DP_OFFLOAD_CONFIG", "");
    audioServer->SetAudioParameter("mmi", "");
    audioServer->SetAudioParameter("perf_info", "");
    audioServer->GetAudioParameter("", str);
    audioServer->GetAudioParameter("AUDIO_EXT_PARAM_KEY_LOWPOWER", str);
    audioServer->GetAudioParameter("perf_info", str);
    audioServer->GetAudioParameter("getSmartPAPOWER", str);
    audioServer->GetAudioParameter("Is_Fast_Blocked_For_AppName#", str);
    auto result = audioServer->GetUsbParameter("address=card2;device=0 role=1");
    audioServer->GetAudioParameter(LOCAL_NETWORK_ID, AudioParamKey::USB_DEVICE,
        "address=card=2;device=0 role=0", str);
    audioServer->GetAudioParameter(LOCAL_NETWORK_ID, AudioParamKey::USB_DEVICE,
        "address=card=2;device=0 role=1", str);
    audioServer->GetAudioParameter(LOCAL_NETWORK_ID, AudioParamKey::USB_DEVICE,
        "address=card=2;device=0 role=2", str);
    audioServer->GetAudioParameter(LOCAL_NETWORK_ID, AudioParamKey::GET_DP_DEVICE_INFO, "", str);
    audioServer->GetAudioParameter("", AudioParamKey::GET_DP_DEVICE_INFO, "", str);
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
    uint64_t ret;
    audioServer->GetTransactionId(DeviceType::DEVICE_TYPE_USB_ARM_HEADSET, DeviceRole::DEVICE_ROLE_MAX, ret);
    EXPECT_NE(0, ret);

    audioServer->GetTransactionId(DeviceType::DEVICE_TYPE_USB_ARM_HEADSET, DeviceRole::INPUT_DEVICE, ret);
    EXPECT_EQ(0, ret);

    audioServer->GetTransactionId(DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE, ret);
    EXPECT_EQ(0, ret);

    audioServer->GetTransactionId(DeviceType::DEVICE_TYPE_USB_ARM_HEADSET, DeviceRole::OUTPUT_DEVICE, ret);
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

    bool scoExcludeFlag = false;
    int32_t ret = audioServer->SetAudioScene(AUDIO_SCENE_INVALID, NO_A2DP_DEVICE, scoExcludeFlag);
    EXPECT_EQ(SUCCESS, ret);
}

#ifdef TEMP_DISABLE

/**
 * @tc.name  : Test SetForegroundList API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetForegroundList_001
 * @tc.desc  : Test SetForegroundList interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerSetForegroundList_001, TestSize.Level2)
{
    EXPECT_NE(nullptr, audioServer);

    std::vector<std::string> list;
    list.push_back("com.test");
    int32_t ret = audioServer->SetForegroundList(list);
    EXPECT_EQ(SUCCESS, ret);
}
#endif


/**
 * @tc.name  : Test SetIORoutes API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetIORoutes_001
 * @tc.desc  : Test SetIORoutes interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerSetIORoutes_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    bool scoExcludeFlag = false;
    std::string deviceName;
    std::vector<DeviceType> deviceTypes;
    std::vector<int32_t> activeOutputDevices;

    activeOutputDevices.push_back(DEVICE_TYPE_USB_ARM_HEADSET);
    int32_t ret = audioServer->SetAudioScene(AUDIO_SCENE_DEFAULT, A2DP_OFFLOAD, scoExcludeFlag);

    ret = audioServer->SetIORoutes(DEVICE_TYPE_USB_ARM_HEADSET, DeviceFlag::ALL_DEVICES_FLAG, deviceTypes,
        A2DP_OFFLOAD, deviceName);
    EXPECT_EQ(SUCCESS, ret);

    ret = audioServer->SetAudioScene(AUDIO_SCENE_INVALID, A2DP_OFFLOAD, scoExcludeFlag);
    activeOutputDevices.clear();
    activeOutputDevices.push_back(DEVICE_TYPE_BLUETOOTH_A2DP);
    ret = audioServer->SetIORoutes(DEVICE_TYPE_USB_ARM_HEADSET, DeviceFlag::ALL_DEVICES_FLAG, deviceTypes,
        A2DP_OFFLOAD, deviceName);
    EXPECT_EQ(SUCCESS, ret);

    deviceTypes.push_back(DEVICE_TYPE_BLUETOOTH_A2DP);
    ret = audioServer->SetIORoutes(DEVICE_TYPE_BLUETOOTH_A2DP, DeviceFlag::OUTPUT_DEVICES_FLAG, deviceTypes,
        A2DP_NOT_OFFLOAD, deviceName);
    EXPECT_EQ(SUCCESS, ret);

    deviceTypes.clear();
    deviceTypes.push_back(DEVICE_TYPE_WIRED_HEADPHONES);
    ret = audioServer->SetIORoutes(DEVICE_TYPE_BLUETOOTH_A2DP, DeviceFlag::OUTPUT_DEVICES_FLAG, deviceTypes,
        A2DP_NOT_OFFLOAD, deviceName);
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

#ifdef TEMP_DISABLE
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
#endif

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

#ifdef TEMP_DISABLE
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
    bool ret = false;
    audioServer->CreatePlaybackCapturerManager(ret);
    EXPECT_TRUE(ret);
}
#endif

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
    float ret;
    audioServer->GetMaxAmplitude(false, "usb", SOURCE_TYPE_MIC, ret);
    EXPECT_EQ(0, ret);

    audioServer->GetMaxAmplitude(false, "a2dp", SOURCE_TYPE_MIC, ret);
    EXPECT_EQ(0, ret);

    audioServer->GetMaxAmplitude(true, "usb", SOURCE_TYPE_INVALID, ret);
    EXPECT_EQ(0, ret);

    audioServer->GetMaxAmplitude(true, "a2dp", SOURCE_TYPE_INVALID, ret);
    EXPECT_EQ(0, ret);
}

#ifdef TEMP_DISABLE
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
#endif

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

#ifdef TEMP_DISABLE
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
#endif

/**
 * @tc.name  : Test RestoreSession API
 * @tc.type  : FUNC
 * @tc.number: AudioServerRestoreSession_001
 * @tc.desc  : Test RestoreSession interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerRestoreSession_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    RestoreInfoIpc restoreInfo;
    audioServer->RestoreSession(-1, restoreInfo);
    audioServer->RestoreSession(-1, restoreInfo);
}

#ifdef TEMP_DISABLE
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
#endif

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
    AudioPlaybackCaptureConfig filterConfig = AudioPlaybackCaptureConfig();
    sptr<IRemoteObject> ret = nullptr;
    int32_t errorCode = 0;
    audioServer->CreateAudioProcess(config, errorCode, filterConfig, ret);
    EXPECT_EQ(ret, nullptr);
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
    AudioPlaybackCaptureConfig filterConfig = AudioPlaybackCaptureConfig();
    sptr<IRemoteObject> ret = nullptr;
    int32_t errorCode = 0;
    audioServer->waitCreateStreamInServerCount_ = 6;
    audioServer->CreateAudioProcess(config, errorCode, filterConfig, ret);
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
 * @tc.name  : Test CheckMaxLoopbackInstances API
 * @tc.type  : FUNC
 * @tc.number: CheckMaxLoopbackInstances_001
 * @tc.desc  : Test CheckMaxLoopbackInstances interface.
 */
HWTEST_F(AudioServerUnitTest, CheckMaxLoopbackInstances_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    int32_t ret = audioServer->CheckMaxLoopbackInstances(AUDIO_MODE_RECORD);
    EXPECT_EQ(ret, SUCCESS);
}

#ifdef TEMP_DISABLE
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
    std::shared_ptr<PipeInfoGuard> pipeinfoGuard = std::make_shared<PipeInfoGuard>(0);
    remoteObject = audioServer->CreateAudioStream(config, AudioServer::VASSISTANT_UID, pipeinfoGuard);
    remoteObject = audioServer->CreateAudioStream(config, AudioServer::MEDIA_SERVICE_UID, pipeinfoGuard);
    EXPECT_EQ(nullptr, remoteObject);
    config.audioMode = AUDIO_MODE_RECORD;
    remoteObject = audioServer->CreateAudioStream(config, AudioServer::MEDIA_SERVICE_UID, pipeinfoGuard);
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
    audioServer->OnCapturerState(true, 0, 1);
    audioServer->OnCapturerState(false, 1, 0);
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

    int32_t ret = 0;
    AudioEffectPropertyArrayV3 audioEffectPropertyArrayV3 = {};
    audioEffectPropertyArrayV3.property.push_back(audioEffectPropertyV31);
    audioEffectPropertyArrayV3.property.push_back(audioEffectPropertyV32);

    audioServer->SetAudioEffectProperty(audioEffectPropertyArrayV3, ret);
    EXPECT_EQ(SUCCESS, ret);
}
#endif
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

    int32_t ret = 0;
    audioServer->SetAudioEffectProperty(audioEffectPropertyArrayV3, ret);
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

    int32_t ret = 0;
    audioServer->SetAudioEffectProperty(audioEffectPropertyArrayV3, ret);
    EXPECT_EQ(SUCCESS, ret);
}

#ifdef TEMP_DISABLE
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

    int32_t ret = 0;
    audioServer->GetAudioEffectProperty(audioEffectPropertyArrayV3, ret);
    EXPECT_EQ(SUCCESS, ret);
}
#endif

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
    bool ret = false;
    audioServer->LoadAudioEffectLibraries(libraries, effects, successEffectList, ret);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name  : Test IsSatellite API
 * @tc.type  : FUNC
 * @tc.number: IsSatellite_001
 * @tc.desc  : Test IsSatellite interface.
 */
HWTEST_F(AudioServerUnitTest, IsSatellite_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    AudioProcessConfig config;
    config.rendererInfo.streamUsage = STREAM_USAGE_UNKNOWN;
    int32_t callerUid = 0;
    bool result = audioServer->IsSatellite(config, callerUid);
    EXPECT_EQ(result, false) << "streamUsage is not MODEM_COMMUNICATION, should be false";

    config.rendererInfo.streamUsage = STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    result = audioServer->IsSatellite(config, callerUid);
    EXPECT_EQ(result, false) << "callerUid is 0, should be false";

    callerUid = 5523; // foundation
    result = audioServer->IsSatellite(config, callerUid);
    EXPECT_EQ(result, false) << "isSatellite is false, should be false";

    config.rendererInfo.isSatellite = true;
    result = audioServer->IsSatellite(config, callerUid);
    EXPECT_EQ(result, true) << "all meet, should be true";
}

/**
 * @tc.name  : Test SetVoiceVolume API
 * @tc.type  : FUNC
 * @tc.number: SetVoiceVolume_001
 * @tc.desc  : Test SetVoiceVolume interface.
 */
HWTEST_F(AudioServerUnitTest, SetVoiceVolume_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    int32_t ret = audioServer->SetVoiceVolume(0.5f);
    EXPECT_EQ(ret, -62980105);
}

/**
 * @tc.name  : Test OffloadSetVolume API
 * @tc.type  : FUNC
 * @tc.number: OffloadSetVolume_001
 * @tc.desc  : Test OffloadSetVolume interface.
 */
HWTEST_F(AudioServerUnitTest, OffloadSetVolume_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    int32_t ret = audioServer->OffloadSetVolume(0.5f, "offload", "default");
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test ResetRouteForDisconnect API
 * @tc.type  : FUNC
 * @tc.number: ResetRouteForDisconnect_001
 * @tc.desc  : Test ResetRouteForDisconnect interface.
 */
HWTEST_F(AudioServerUnitTest, ResetRouteForDisconnect_001, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_USB_ARM_HEADSET;

    EXPECT_NE(nullptr, audioServer);
    auto ret = audioServer->ResetRouteForDisconnect(deviceType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test CheckRecorderFormat API
 * @tc.type  : FUNC
 * @tc.number: CheckRecorderFormat_001
 * @tc.desc  : Test CheckRecorderFormat interface.
 */
HWTEST_F(AudioServerUnitTest, CheckRecorderFormat_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    AudioProcessConfig config;
    config.audioMode = AUDIO_MODE_RECORD;
    config.capturerInfo.sourceType = SOURCE_TYPE_MIC;
    config.streamInfo.channels = MONO;
    config.streamInfo.channelLayout = CH_LAYOUT_MONO;
    config.streamInfo.encoding = ENCODING_PCM;
    config.streamInfo.format = SAMPLE_U8;
    config.streamInfo.samplingRate = SAMPLE_RATE_8000;
    bool ret = audioServer->CheckRecorderFormat(config);
    EXPECT_EQ(ret, true);

    config.capturerInfo.sourceType = SOURCE_TYPE_MIC_REF;
    ret = audioServer->CheckRecorderFormat(config);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test CheckAndWaitAudioPolicyReady API
 * @tc.type  : FUNC
 * @tc.number: CheckAndWaitAudioPolicyReady_001
 * @tc.desc  : Test CheckAndWaitAudioPolicyReady interface.
 */
HWTEST_F(AudioServerUnitTest, CheckAndWaitAudioPolicyReady_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    int32_t ret = audioServer->CheckAndWaitAudioPolicyReady();
    EXPECT_EQ(ret, -62980132);
}

/**
 * @tc.name  : Test NotifyStreamVolumeChanged API
 * @tc.type  : FUNC
 * @tc.number: NotifyStreamVolumeChanged_001
 * @tc.desc  : Test NotifyStreamVolumeChanged interface.
 */
HWTEST_F(AudioServerUnitTest, NotifyStreamVolumeChanged_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    AudioStreamType streamType = STREAM_MUSIC;
    float volume = 0.5f;
    int32_t ret = audioServer->NotifyStreamVolumeChanged(streamType, volume);
    EXPECT_EQ(ret, SUCCESS);

    streamType = static_cast<AudioStreamType>(-1);
    ret = audioServer->NotifyStreamVolumeChanged(streamType, volume);
    EXPECT_EQ(ret, SUCCESS);

    streamType = STREAM_MUSIC;
    volume = -1.0f;
    ret = audioServer->NotifyStreamVolumeChanged(streamType, volume);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetVolumeDataCount API
 * @tc.type  : FUNC
 * @tc.number: GetVolumeDataCount_001
 * @tc.desc  : Test GetVolumeDataCount interface.
 */
HWTEST_F(AudioServerUnitTest, GetVolumeDataCount_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    std::string testName = "testSink";
    int64_t volueDataCount = INVALID_VALUE;
    audioServer->GetVolumeDataCount(testName, volueDataCount);
    EXPECT_EQ(volueDataCount, 0);
}

/**
 * @tc.name  : Test GetVolumeDataCount API
 * @tc.type  : FUNC
 * @tc.number: GetVolumeDataCount_002
 * @tc.desc  : Test GetVolumeDataCount interface.
 */
HWTEST_F(AudioServerUnitTest, GetVolumeDataCount_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    std::string testName = "primary";
    int64_t volueDataCount = INVALID_VALUE;
    audioServer->GetVolumeDataCount(testName, volueDataCount);
    EXPECT_EQ(volueDataCount, 0);
}

/**
 * @tc.name  : Test SetDefaultAdapterEnable API
 * @tc.type  : FUNC
 * @tc.number: SetDefaultAdapterEnable_001
 * @tc.desc  : Test SetDefaultAdapterEnable interface.
 */
HWTEST_F(AudioServerUnitTest, SetDefaultAdapterEnable_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    bool isEnable = false;
    audioServer->SetDefaultAdapterEnable(isEnable);
    EXPECT_NE(nullptr, audioServer);
}

/**
 * @tc.name  : Test RendererDataTransferCallback API
 * @tc.type  : FUNC
 * @tc.number: RendererDataTransferCallback_001
 * @tc.desc  : Test RendererDataTransferCallback interface.
 */
HWTEST_F(AudioServerUnitTest, RendererDataTransferCallback_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    audioServer->RemoveRendererDataTransferCallback(0);

    std::shared_ptr<DataTransferStateChangeCallbackInnerImpl> callback =
        std::make_shared<DataTransferStateChangeCallbackInnerImpl>();
    int32_t pid = IPCSkeleton::GetCallingPid();
    audioServer->audioDataTransferCbMap_[pid] = callback;
    AudioRendererDataTransferStateChangeInfo info = {};
    int callbackId = 0;
    audioServer->OnDataTransferStateChange(pid, callbackId, info);
    audioServer->RemoveRendererDataTransferCallback(pid);
    EXPECT_EQ(audioServer->audioDataTransferCbMap_.size(), 0);
}

/**
 * @tc.name  : Test RendereataTransferStateChangeCallback API
 * @tc.type  : FUNC
 * @tc.number: RendereataTransferStateChangeCallback_001
 * @tc.desc  : Test RendereataTransferStateChangeCallback interface.
 */
HWTEST_F(AudioServerUnitTest, RendereataTransferStateChangeCallback_001, TestSize.Level1)
{
    std::shared_ptr<DataTransferStateChangeCallbackInnerImpl> callback =
        std::make_shared<DataTransferStateChangeCallbackInnerImpl>();
    EXPECT_NE(nullptr, callback);
    int32_t pid = IPCSkeleton::GetCallingPid();
    AudioRendererDataTransferStateChangeInfo info;
    info.stateChangeType = DATA_TRANS_RESUME;
    info.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    callback->OnDataTransferStateChange(pid, info);
}

/**
 * @tc.name  : Test RendereataTransferStateChangeCallback API
 * @tc.type  : FUNC
 * @tc.number: RendereataTransferStateChangeCallback_002
 * @tc.desc  : Test RendereataTransferStateChangeCallback interface.
 */
HWTEST_F(AudioServerUnitTest, RendereataTransferStateChangeCallback_002, TestSize.Level1)
{
    std::shared_ptr<DataTransferStateChangeCallbackInnerImpl> callback =
        std::make_shared<DataTransferStateChangeCallbackInnerImpl>();
    EXPECT_NE(nullptr, callback);
    int32_t pid = IPCSkeleton::GetCallingPid();
    AudioRendererDataTransferStateChangeInfo info;
    info.stateChangeType = DATA_TRANS_STOP;
    info.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    callback->OnDataTransferStateChange(pid, info);
}

/**
 * @tc.name  : Test RendereataTransferStateChangeCallback API
 * @tc.type  : FUNC
 * @tc.number: RendereataTransferStateChangeCallback_003
 * @tc.desc  : Test RendereataTransferStateChangeCallback interface.
 */
HWTEST_F(AudioServerUnitTest, RendereataTransferStateChangeCallback_003, TestSize.Level1)
{
    std::shared_ptr<DataTransferStateChangeCallbackInnerImpl> callback =
        std::make_shared<DataTransferStateChangeCallbackInnerImpl>();
    EXPECT_NE(nullptr, callback);
    int32_t pid = IPCSkeleton::GetCallingPid();
    AudioRendererDataTransferStateChangeInfo info;
    info.stateChangeType = DATA_TRANS_STOP;
    info.streamUsage = STREAM_USAGE_MUSIC;
    callback->OnDataTransferStateChange(pid, info);
}

/**
 * @tc.name  : Test RendereataTransferStateChangeCallback API
 * @tc.type  : FUNC
 * @tc.number: RendereataTransferStateChangeCallback_004
 * @tc.desc  : Test RendereataTransferStateChangeCallback interface.
 */
HWTEST_F(AudioServerUnitTest, RendereataTransferStateChangeCallback_004, TestSize.Level1)
{
    std::shared_ptr<DataTransferStateChangeCallbackInnerImpl> callback =
        std::make_shared<DataTransferStateChangeCallbackInnerImpl>();
    EXPECT_NE(nullptr, callback);
    int32_t pid = IPCSkeleton::GetCallingPid();
    AudioRendererDataTransferStateChangeInfo info;
    info.stateChangeType = DATA_TRANS_STOP;
    info.streamUsage = STREAM_USAGE_VIDEO_COMMUNICATION;
    callback->OnDataTransferStateChange(pid, info);
}

/**
 * @tc.name  : Test RendereataTransferStateChangeCallback API
 * @tc.type  : FUNC
 * @tc.number: RendereataTransferStateChangeCallback_005
 * @tc.desc  : Test RendereataTransferStateChangeCallback interface.
 */
HWTEST_F(AudioServerUnitTest, RendereataTransferStateChangeCallback_005, TestSize.Level1)
{
    std::shared_ptr<DataTransferStateChangeCallbackInnerImpl> callback =
        std::make_shared<DataTransferStateChangeCallbackInnerImpl>();
    EXPECT_NE(nullptr, callback);
    int32_t pid = IPCSkeleton::GetCallingPid();
    AudioRendererDataTransferStateChangeInfo info;
    info.stateChangeType = DATA_TRANS_STOP;
    info.streamUsage = STREAM_USAGE_VIDEO_COMMUNICATION;
    info.isBackground = true;
    callback->OnDataTransferStateChange(pid, info);
}

/*
 * @tc.name  : Test CreateAudioWorkgroup API
 * @tc.type  : FUNC
 * @tc.number: CreateAudioWorkgroup_001
 * @tc.desc  : Test CreateAudioWorkgroup interface when null object
 */
HWTEST_F(AudioServerUnitTest, CreateAudioWorkgroup_001, TestSize.Level1)
{
    int32_t pid = 123;
    sptr<IRemoteObject> object = nullptr;
    int32_t result = -1;
    audioServer->CreateAudioWorkgroup(pid, object, result);
    EXPECT_NE(result, 0);
}

/**
 * @tc.name  : Test SetReleaseFlag API
 * @tc.type  : FUNC
 * @tc.number: SetReleaseFlag_001
 * @tc.desc  : Test SetReleaseFlag interface.
 */
HWTEST_F(AudioServerUnitTest, SetReleaseFlag_001, TestSize.Level1)
{
    auto pipeInfoGuard_ = std::make_shared<PipeInfoGuard>(0);
    EXPECT_NE(nullptr, pipeInfoGuard_);

    pipeInfoGuard_->SetReleaseFlag(true);
    EXPECT_EQ(pipeInfoGuard_->releaseFlag_, true);
}

/**
 * @tc.name  : Test SetReleaseFlag API
 * @tc.type  : FUNC
 * @tc.number: SetReleaseFlag_002
 * @tc.desc  : Test SetReleaseFlag interface.
 */
HWTEST_F(AudioServerUnitTest, SetReleaseFlag_002, TestSize.Level1)
{
    auto pipeInfoGuard_ = std::make_shared<PipeInfoGuard>(0);
    EXPECT_NE(nullptr, pipeInfoGuard_);

    pipeInfoGuard_->SetReleaseFlag(false);
    EXPECT_EQ(pipeInfoGuard_->releaseFlag_, false);
}

/**
 * @tc.name  : Test Dump API
 * @tc.type  : FUNC
 * @tc.number: Dump_002
 * @tc.desc  : Test Dump interface.
 */
HWTEST_F(AudioServerUnitTest, Dump_002, TestSize.Level1)
{
    std::vector<std::u16string> args;
    args.push_back(u"-dfl");

    auto ret = audioServer->Dump(0, args);
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name  : Test SetEffectLiveParameter API
 * @tc.type  : FUNC
 * @tc.number: SetEffectLiveParameter_001
 * @tc.desc  : Test SetEffectLiveParameter interface.
 */
HWTEST_F(AudioServerUnitTest, SetEffectLiveParameter_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    SetSysPara("const.multimedia.audio.proaudioEnable", 0);

    std::vector<std::pair<std::string, std::string>> params;
    params.push_back({"key1", "value1"});

    bool ret = audioServer->SetEffectLiveParameter(params);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test GetExtraParameters API
 * @tc.type  : FUNC
 * @tc.number: GetExtraParameters_001
 * @tc.desc  : Test GetExtraParameters interface.
 */
HWTEST_F(AudioServerUnitTest, GetExtraParameters_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    std::string mainKey = "hpae_effect";
    std::vector<std::string> subKeys;
    std::vector<StringPair> result_;
    result_.push_back({"key1", "value1"});

    int32_t ret = audioServer->GetExtraParameters(mainKey, subKeys, result_);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test SetIORoutes API
 * @tc.type  : FUNC
 * @tc.number: SetIORoutes_001
 * @tc.desc  : Test SetIORoutes interface.
 */
HWTEST_F(AudioServerUnitTest, SetIORoutes_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    std::vector<DeviceType> deviceTypes;
    deviceTypes.push_back(DeviceType::DEVICE_TYPE_SPEAKER);
    BluetoothOffloadState a2dpOffloadFlag = NO_A2DP_DEVICE;
    std::string deviceName = "test";
    int32_t ret = SUCCESS;

    std::vector<std::pair<DeviceType, DeviceFlag>> activeDevices;
    activeDevices.push_back({DEVICE_TYPE_ACCESSORY, OUTPUT_DEVICES_FLAG});
    ret = audioServer->SetIORoutes(activeDevices, a2dpOffloadFlag, deviceName);
    EXPECT_EQ(ret, SUCCESS);

    DeviceType type = DEVICE_TYPE_ACCESSORY;
    DeviceFlag flag = OUTPUT_DEVICES_FLAG;
    ret = audioServer->SetIORoutes(type, flag, deviceTypes, a2dpOffloadFlag, deviceName);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetIORoutes API
 * @tc.type  : FUNC
 * @tc.number: SetIORoutes_002
 * @tc.desc  : Test SetIORoutes interface.
 */
HWTEST_F(AudioServerUnitTest, SetIORoutes_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    std::vector<DeviceType> deviceTypes;
    deviceTypes.push_back(DeviceType::DEVICE_TYPE_SPEAKER);
    BluetoothOffloadState a2dpOffloadFlag = A2DP_OFFLOAD;
    std::string deviceName = "test";
    int32_t ret = SUCCESS;
    DeviceType type = DEVICE_TYPE_ACCESSORY;
    DeviceFlag flag = OUTPUT_DEVICES_FLAG;

    ret = audioServer->SetIORoutes(type, flag, deviceTypes, a2dpOffloadFlag, deviceName);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetIORoutes API
 * @tc.type  : FUNC
 * @tc.number: SetIORoutes_003
 * @tc.desc  : Test SetIORoutes interface.
 */
HWTEST_F(AudioServerUnitTest, SetIORoutes_003, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    std::vector<DeviceType> deviceTypes;
    deviceTypes.push_back(DeviceType::DEVICE_TYPE_SPEAKER);
    BluetoothOffloadState a2dpOffloadFlag = A2DP_OFFLOAD;
    std::string deviceName = "test";
    int32_t ret = SUCCESS;
    DeviceType type = DEVICE_TYPE_FILE_SOURCE;
    DeviceFlag flag = OUTPUT_DEVICES_FLAG;

    ret = audioServer->SetIORoutes(type, flag, deviceTypes, a2dpOffloadFlag, deviceName);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetIORoutes API
 * @tc.type  : FUNC
 * @tc.number: SetIORoutes_004
 * @tc.desc  : Test SetIORoutes interface.
 */
HWTEST_F(AudioServerUnitTest, SetIORoutes_004, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    std::vector<DeviceType> deviceTypes;
    deviceTypes.push_back(DeviceType::DEVICE_TYPE_SPEAKER);
    BluetoothOffloadState a2dpOffloadFlag = A2DP_NOT_OFFLOAD;
    std::string deviceName = "test";
    int32_t ret = SUCCESS;
    DeviceType type = DEVICE_TYPE_BLUETOOTH_A2DP;
    DeviceFlag flag = OUTPUT_DEVICES_FLAG;

    ret = audioServer->SetIORoutes(type, flag, deviceTypes, a2dpOffloadFlag, deviceName);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetIORoutes API
 * @tc.type  : FUNC
 * @tc.number: SetIORoutes_005
 * @tc.desc  : Test SetIORoutes interface.
 */
HWTEST_F(AudioServerUnitTest, SetIORoutes_005, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    std::vector<DeviceType> deviceTypes;
    deviceTypes.push_back(DeviceType::DEVICE_TYPE_SPEAKER);
    BluetoothOffloadState a2dpOffloadFlag = A2DP_NOT_OFFLOAD;
    std::string deviceName = "test";
    int32_t ret = SUCCESS;
    DeviceType type = DEVICE_TYPE_USB_ARM_HEADSET;
    DeviceFlag flag = OUTPUT_DEVICES_FLAG;

    audioServer->audioScene_ = AUDIO_SCENE_DEFAULT;
    ret = audioServer->SetIORoutes(type, flag, deviceTypes, a2dpOffloadFlag, deviceName);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetIORoutes API
 * @tc.type  : FUNC
 * @tc.number: SetIORoutes_006
 * @tc.desc  : Test SetIORoutes interface.
 */
HWTEST_F(AudioServerUnitTest, SetIORoutes_006, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    std::vector<DeviceType> deviceTypes;
    deviceTypes.push_back(DeviceType::DEVICE_TYPE_SPEAKER);
    BluetoothOffloadState a2dpOffloadFlag = A2DP_NOT_OFFLOAD;
    std::string deviceName = "test";
    int32_t ret = SUCCESS;
    DeviceType type = DEVICE_TYPE_USB_ARM_HEADSET;
    DeviceFlag flag = OUTPUT_DEVICES_FLAG;

    audioServer->audioScene_ = AUDIO_SCENE_RINGING;
    ret = audioServer->SetIORoutes(type, flag, deviceTypes, a2dpOffloadFlag, deviceName);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetIORoutes API
 * @tc.type  : FUNC
 * @tc.number: SetIORoutes_007
 * @tc.desc  : Test SetIORoutes interface.
 */
HWTEST_F(AudioServerUnitTest, SetIORoutes_007, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    std::vector<DeviceType> deviceTypes;
    deviceTypes.push_back(DeviceType::DEVICE_TYPE_SPEAKER);
    BluetoothOffloadState a2dpOffloadFlag = A2DP_NOT_OFFLOAD;
    std::string deviceName = "test";
    int32_t ret = SUCCESS;
    DeviceType type = DEVICE_TYPE_USB_ARM_HEADSET;
    DeviceFlag flag = INPUT_DEVICES_FLAG;

    audioServer->audioScene_ = AUDIO_SCENE_DEFAULT;
    ret = audioServer->SetIORoutes(type, flag, deviceTypes, a2dpOffloadFlag, deviceName);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetIORoutes API
 * @tc.type  : FUNC
 * @tc.number: SetIORoutes_008
 * @tc.desc  : Test SetIORoutes interface.
 */
HWTEST_F(AudioServerUnitTest, SetIORoutes_008, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    std::vector<DeviceType> deviceTypes;
    deviceTypes.push_back(DeviceType::DEVICE_TYPE_SPEAKER);
    BluetoothOffloadState a2dpOffloadFlag = A2DP_NOT_OFFLOAD;
    std::string deviceName = "test";
    int32_t ret = SUCCESS;
    DeviceType type = DEVICE_TYPE_USB_ARM_HEADSET;
    DeviceFlag flag = INPUT_DEVICES_FLAG;

    audioServer->audioScene_ = AUDIO_SCENE_RINGING;
    ret = audioServer->SetIORoutes(type, flag, deviceTypes, a2dpOffloadFlag, deviceName);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetIORoutes API
 * @tc.type  : FUNC
 * @tc.number: SetIORoutes_009
 * @tc.desc  : Test SetIORoutes interface.
 */
HWTEST_F(AudioServerUnitTest, SetIORoutes_009, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    std::vector<DeviceType> deviceTypes;
    deviceTypes.push_back(DeviceType::DEVICE_TYPE_SPEAKER);
    BluetoothOffloadState a2dpOffloadFlag = A2DP_NOT_OFFLOAD;
    std::string deviceName = "test";
    int32_t ret = SUCCESS;
    DeviceType type = DEVICE_TYPE_USB_ARM_HEADSET;
    DeviceFlag flag = ALL_DEVICES_FLAG;

    audioServer->audioScene_ = AUDIO_SCENE_DEFAULT;
    ret = audioServer->SetIORoutes(type, flag, deviceTypes, a2dpOffloadFlag, deviceName);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetIORoutes API
 * @tc.type  : FUNC
 * @tc.number: SetIORoutes_010
 * @tc.desc  : Test SetIORoutes interface.
 */
HWTEST_F(AudioServerUnitTest, SetIORoutes_010, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    std::vector<DeviceType> deviceTypes;
    deviceTypes.push_back(DeviceType::DEVICE_TYPE_SPEAKER);
    BluetoothOffloadState a2dpOffloadFlag = A2DP_NOT_OFFLOAD;
    std::string deviceName = "test";
    int32_t ret = SUCCESS;
    DeviceType type = DEVICE_TYPE_USB_ARM_HEADSET;
    DeviceFlag flag = ALL_DEVICES_FLAG;

    audioServer->audioScene_ = AUDIO_SCENE_RINGING;
    ret = audioServer->SetIORoutes(type, flag, deviceTypes, a2dpOffloadFlag, deviceName);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetIORoutes API
 * @tc.type  : FUNC
 * @tc.number: SetIORoutes_011
 * @tc.desc  : Test SetIORoutes interface.
 */
HWTEST_F(AudioServerUnitTest, SetIORoutes_011, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    std::vector<DeviceType> deviceTypes;
    deviceTypes.push_back(DeviceType::DEVICE_TYPE_SPEAKER);
    BluetoothOffloadState a2dpOffloadFlag = A2DP_NOT_OFFLOAD;
    std::string deviceName = "test";
    int32_t ret = SUCCESS;
    DeviceType type = DEVICE_TYPE_USB_ARM_HEADSET;
    DeviceFlag flag = ALL_DISTRIBUTED_DEVICES_FLAG;

    ret = audioServer->SetIORoutes(type, flag, deviceTypes, a2dpOffloadFlag, deviceName);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test ResetRecordConfig API
 * @tc.type  : FUNC
 * @tc.number: ResetRecordConfig_001
 * @tc.desc  : Test ResetRecordConfig interface.
 */
HWTEST_F(AudioServerUnitTest, ResetRecordConfig_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    AudioProcessConfig config;
    config.capturerInfo.sourceType = SourceType::SOURCE_TYPE_LIVE;

    audioServer->ResetRecordConfig(config);
    EXPECT_FALSE(config.isInnerCapturer);
    EXPECT_FALSE(config.isWakeupCapturer);
}

/**
 * @tc.name  : Test ResetRecordConfig API
 * @tc.type  : FUNC
 * @tc.number: ResetRecordConfig_002
 * @tc.desc  : Test ResetRecordConfig interface.
 */
HWTEST_F(AudioServerUnitTest, ResetRecordConfig_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    AudioProcessConfig config;
    config.capturerInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    audioServer->ResetRecordConfig(config);
    EXPECT_EQ(config.isInnerCapturer, true);
}

/**
 * @tc.name  : Test IsFastBlocked API
 * @tc.type  : FUNC
 * @tc.number: IsFastBlocked_001
 * @tc.desc  : Test IsFastBlocked interface.
 */
HWTEST_F(AudioServerUnitTest, IsFastBlocked_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    int32_t callingUid = IPCSkeleton::GetCallingUid();
    PlayerType playerType = PLAYER_TYPE_SOUND_POOL;
    bool ret = audioServer->IsFastBlocked(callingUid, playerType);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test IsFastBlocked API
 * @tc.type  : FUNC
 * @tc.number: IsFastBlocked_002
 * @tc.desc  : Test IsFastBlocked interface.
 */
HWTEST_F(AudioServerUnitTest, IsFastBlocked_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    int32_t callingUid = IPCSkeleton::GetCallingUid();
    PlayerType playerType = PLAYER_TYPE_SOUND_POOL;

    playerType = PLAYER_TYPE_AV_PLAYER;
    bool ret = audioServer->IsFastBlocked(callingUid, playerType);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test CheckMaxLoopbackInstances API
 * @tc.type  : FUNC
 * @tc.number: CheckMaxLoopbackInstances_002
 * @tc.desc  : Test CheckMaxLoopbackInstances interface.
 */
HWTEST_F(AudioServerUnitTest, CheckMaxLoopbackInstances_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);

    AudioService::GetInstance()->SetIncMaxLoopbackStreamCnt(AUDIO_MODE_RECORD);
    int32_t ret = audioServer->CheckMaxLoopbackInstances(AUDIO_MODE_RECORD);
    EXPECT_EQ(ret, ERR_EXCEED_MAX_STREAM_CNT);
}

/**
 * @tc.name  : Test Dump API
 * @tc.type  : FUNC
 * @tc.number: Dump_001
 * @tc.desc  : Test Dump interface.
 */
HWTEST_F(AudioServerUnitTest, Dump_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    std::vector<std::u16string> args;
    args.push_back(u"-fb");
    args.push_back(u"test");

    auto ret = audioServer->Dump(0, args);
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name  : Test Dump API
 * @tc.type  : FUNC
 * @tc.number: Dump_003
 * @tc.desc  : Test Dump interface.
 */
HWTEST_F(AudioServerUnitTest, Dump_003, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    std::vector<std::u16string> args;
    args.push_back(u"-other");
    args.push_back(u"test");

    auto ret = audioServer->Dump(0, args);
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name  : Test OnDataTransferStateChange API
 * @tc.type  : FUNC
 * @tc.number: OnDataTransferStateChange_002
 * @tc.desc  : Test OnDataTransferStateChange interface.
 */
HWTEST_F(AudioServerUnitTest, OnDataTransferStateChange_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    int32_t callbackId = 1;
    AudioRendererDataTransferStateChangeInfo info;
    info.clientUID = 1000;
    info.sessionId = 1;
    info.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    info.stateChangeType = DataTransferStateChangeType::AUDIO_STREAM_START;
    info.isBackground = false;
    info.badDataRatio[0] = 0;
    info.badDataRatio[1] = 0;

    std::shared_ptr<DataTransferStateChangeCallbackInnerImpl> callback =
        std::make_shared<DataTransferStateChangeCallbackInnerImpl>();
    int32_t pid = IPCSkeleton::GetCallingPid();
    info.clientPid = pid;
    audioServer->audioDataTransferCbMap_[pid] = callback;
    audioServer->OnDataTransferStateChange(pid, callbackId, info);
}

/**
 * @tc.name  : Test ProcessKeyValuePairs API
 * @tc.type  : FUNC
 * @tc.number: ProcessKeyValuePairs_001
 * @tc.desc  : Test ProcessKeyValuePairs interface.
 */
HWTEST_F(AudioServerUnitTest, ProcessKeyValuePairs_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    std::string key = "test_key";
    std::vector<std::pair<std::string, std::string>> kvpairs = {{"sub_key1", "value1"}, {"sub_key2", "value2"}};
    std::unordered_map<std::string, std::set<std::string>> subKeyMap = {
        {"sub_key1", {"effect"}},
        {"sub_key2", {"effect"}}
    };
    std::string value;

    EXPECT_TRUE(audioServer->ProcessKeyValuePairs(key, kvpairs, subKeyMap, value));
}

/**
 * @tc.name  : Test ProcessKeyValuePairs API
 * @tc.type  : FUNC
 * @tc.number: ProcessKeyValuePairs_002
 * @tc.desc  : Test ProcessKeyValuePairs interface.
 */
HWTEST_F(AudioServerUnitTest, ProcessKeyValuePairs_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    std::string key = "test_key";
    std::vector<std::pair<std::string, std::string>> kvpairs = {{"sub_key1", "value1"}, {"invalid_key2", "value2"}};
    std::unordered_map<std::string, std::set<std::string>> subKeyMap = {
        {"sub_key1", {"effect"}},
        {"sub_key2", {"effect"}}
    };
    std::string value;

    EXPECT_FALSE(audioServer->ProcessKeyValuePairs(key, kvpairs, subKeyMap, value));
}

/**
 * @tc.name  : Test CreateAudioStream API
 * @tc.type  : FUNC
 * @tc.number: AudioServerCreateAudioStream_002
 * @tc.desc  : Test CreateAudioStream interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerCreateAudioStream_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    AudioProcessConfig config;
    config.streamInfo.format = SAMPLE_S16LE;
    config.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    config.audioMode = AUDIO_MODE_PLAYBACK;
    config.rendererInfo.rendererFlags = AUDIO_FLAG_NORMAL;
    config.capturerInfo.capturerFlags = AUDIO_FLAG_NORMAL;
    sptr<IRemoteObject> remoteObject = nullptr;
    std::shared_ptr<PipeInfoGuard> pipeinfoGuard = std::make_shared<PipeInfoGuard>(0);
    remoteObject = audioServer->CreateAudioStream(config, AudioServer::VASSISTANT_UID, pipeinfoGuard);
    EXPECT_EQ(nullptr, remoteObject);

    config.capturerInfo.capturerFlags = AUDIO_FLAG_DIRECT;
    config.audioMode = AUDIO_MODE_RECORD;
    config.capturerInfo.isLoopback = true;
    remoteObject = audioServer->CreateAudioStream(config, AudioServer::MEDIA_SERVICE_UID, pipeinfoGuard);
    EXPECT_EQ(nullptr, remoteObject);

    config.capturerInfo.isLoopback = false;
    remoteObject = audioServer->CreateAudioStream(config, AudioServer::MEDIA_SERVICE_UID, pipeinfoGuard);
    EXPECT_EQ(nullptr, remoteObject);
}

/**
 * @tc.name  : Test NotifyProcessStatus API
 * @tc.type  : FUNC
 * @tc.number: NotifyProcessStatus_001
 * @tc.desc  : Test NotifyProcessStatus interface.
 */
HWTEST_F(AudioServerUnitTest, NotifyProcessStatus_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    audioServer->NotifyProcessStatus();
    EXPECT_NE(dlopen("libmemmgrclient.z.so", RTLD_NOW), nullptr);
}

#ifdef HAS_FEATURE_INNERCAPTURER
/**
 * @tc.name  : Test HandleCheckCaptureLimit API
 * @tc.type  : FUNC
 * @tc.number: HandleCheckCaptureLimit_001
 * @tc.desc  : Test HandleCheckCaptureLimit interface.
 */
HWTEST_F(AudioServerUnitTest, HandleCheckCaptureLimit_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    AudioProcessConfig resetConfig;
    AudioPlaybackCaptureConfig filterConfig;
    resetConfig.capturerInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    EXPECT_EQ(audioServer->HandleCheckCaptureLimit(resetConfig, filterConfig), true);
    resetConfig.capturerInfo.sourceType = SOURCE_TYPE_MIC;
    EXPECT_EQ(audioServer->HandleCheckCaptureLimit(resetConfig, filterConfig), false);
}
#endif

/**
 * @tc.name  : Test IsNormalIpcStream API
 * @tc.type  : FUNC
 * @tc.number: IsNormalIpcStream_001
 * @tc.desc  : Test IsNormalIpcStream interface.
 */
HWTEST_F(AudioServerUnitTest, IsNormalIpcStream_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    AudioProcessConfig config;
    config.audioMode = AUDIO_MODE_PLAYBACK;
    config.rendererInfo.rendererFlags = AUDIO_FLAG_NORMAL;
    EXPECT_TRUE(audioServer->IsNormalIpcStream(config));

    config.audioMode = AUDIO_MODE_PLAYBACK;
    config.rendererInfo.rendererFlags = AUDIO_FLAG_VOIP_DIRECT;
    EXPECT_TRUE(audioServer->IsNormalIpcStream(config));

    config.audioMode = AUDIO_MODE_PLAYBACK;
    config.rendererInfo.rendererFlags = AUDIO_FLAG_DIRECT;
    EXPECT_TRUE(audioServer->IsNormalIpcStream(config));

    config.capturerInfo.capturerFlags = AUDIO_FLAG_NORMAL;
    config.audioMode = AUDIO_MODE_RECORD;
    EXPECT_TRUE(audioServer->IsNormalIpcStream(config));
}

/**
 * @tc.name  : Test OnCapturerState API
 * @tc.type  : FUNC
 * @tc.number: OnCapturerState_001
 * @tc.desc  : Test OnCapturerState interface.
 */
HWTEST_F(AudioServerUnitTest, OnCapturerState_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    std::shared_ptr<WakeUpSourceCallback> callback = std::make_shared<WakeUpSourceCallbackTest>();
    audioServer->wakeupCallback_ = callback;
    audioServer->OnCapturerState(true, 1, 1);
    audioServer->OnCapturerState(true, 0, 1);
    EXPECT_NE(audioServer->wakeupCallback_, nullptr);
}

/**
 * @tc.name  : Test CheckInnerRecorderPermission API
 * @tc.type  : FUNC
 * @tc.number: CheckInnerRecorderPermission_001
 * @tc.desc  : Test CheckInnerRecorderPermission interface.
 */
HWTEST_F(AudioServerUnitTest, CheckInnerRecorderPermission_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    AudioProcessConfig config;
    config.audioMode = AUDIO_MODE_RECORD;
    config.capturerInfo.sourceType = SOURCE_TYPE_MIC;
    auto ret = audioServer->CheckInnerRecorderPermission(config);
    EXPECT_EQ(ret, 2);

    config.capturerInfo.sourceType = SOURCE_TYPE_MIC_REF;
    ret = audioServer->CheckInnerRecorderPermission(config);
    EXPECT_EQ(ret, 2);
}

/**
 * @tc.name  : Test CheckInnerRecorderPermission API
 * @tc.type  : FUNC
 * @tc.number: CheckInnerRecorderPermission_002
 * @tc.desc  : Test CheckInnerRecorderPermission interface.
 */
HWTEST_F(AudioServerUnitTest, CheckInnerRecorderPermission_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    AudioProcessConfig config;
    config.appInfo.appTokenId = 12345;
    config.capturerInfo.sourceType = SOURCE_TYPE_REMOTE_CAST;
    EXPECT_EQ(audioServer->CheckInnerRecorderPermission(config), PERMISSION_GRANTED);

    config.innerCapMode = MODERN_INNER_CAP;
    config.capturerInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    EXPECT_EQ(audioServer->CheckInnerRecorderPermission(config), PERMISSION_GRANTED);

    config.innerCapMode = INVALID_CAP_MODE;
    config.capturerInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    EXPECT_EQ(audioServer->CheckInnerRecorderPermission(config), PERMISSION_UNKNOWN);

    config.capturerInfo.sourceType = SOURCE_TYPE_INVALID;
    EXPECT_EQ(audioServer->CheckInnerRecorderPermission(config), PERMISSION_UNKNOWN);
}

/**
 * @tc.name  : Test CheckPlaybackPermission API
 * @tc.type  : FUNC
 * @tc.number: CheckPlaybackPermission_001
 * @tc.desc  : Test CheckPlaybackPermission interface.
 */
HWTEST_F(AudioServerUnitTest, CheckPlaybackPermission_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    AudioProcessConfig config;
    config.audioMode = AUDIO_MODE_PLAYBACK;
    config.rendererInfo.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    config.capturerInfo.sourceType = SOURCE_TYPE_MIC;
    bool ret = audioServer->CheckPlaybackPermission(config);
    EXPECT_EQ(ret, true);

    config.rendererInfo.streamUsage = STREAM_USAGE_UNKNOWN;
    ret = audioServer->CheckPlaybackPermission(config);
    EXPECT_EQ(ret, true);

    config.rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    ret = audioServer->CheckPlaybackPermission(config);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test CheckPlaybackPermission API
 * @tc.type  : FUNC
 * @tc.number: CheckPlaybackPermission_002
 * @tc.desc  : Test CheckPlaybackPermission interface.
 */
HWTEST_F(AudioServerUnitTest, CheckPlaybackPermission_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    AudioProcessConfig config;
    config.audioMode = AUDIO_MODE_PLAYBACK;
    config.rendererInfo.streamUsage = STREAM_USAGE_SYSTEM;
    bool ret;
    ret = audioServer->CheckPlaybackPermission(config);
    EXPECT_EQ(ret, true);

    config.rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    ret = audioServer->CheckPlaybackPermission(config);
    EXPECT_EQ(ret, true);

    config.callerUid = 0;
    config.rendererInfo.streamUsage = STREAM_USAGE_ULTRASONIC;
    ret = audioServer->CheckPlaybackPermission(config);
    EXPECT_EQ(ret, false);

    config.callerUid = 6699; // msdp
    ret = audioServer->CheckPlaybackPermission(config);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test HandleCheckRecorderBackgroundCapture API
 * @tc.type  : FUNC
 * @tc.number: HandleCheckRecorderBackgroundCapture_001
 * @tc.desc  : Test HandleCheckRecorderBackgroundCapture interface.
 */
HWTEST_F(AudioServerUnitTest, HandleCheckRecorderBackgroundCapture_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    AudioProcessConfig config;
    config.audioMode = AUDIO_MODE_RECORD;
    config.capturerInfo.sourceType = SOURCE_TYPE_MIC;
    config.capturerInfo.capturerFlags = AUDIO_FLAG_MMAP;
    int32_t ret = audioServer->HandleCheckRecorderBackgroundCapture(config);
    EXPECT_EQ(ret, 0);

    config.capturerInfo.capturerFlags = AUDIO_FLAG_NORMAL;
    ret = audioServer->HandleCheckRecorderBackgroundCapture(config);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test HandleCheckRecorderBackgroundCapture API
 * @tc.type  : FUNC
 * @tc.number: HandleCheckRecorderBackgroundCapture_002
 * @tc.desc  : Test HandleCheckRecorderBackgroundCapture interface.
 */
HWTEST_F(AudioServerUnitTest, HandleCheckRecorderBackgroundCapture_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    AudioProcessConfig config;
    config.callerUid = 1001;
    config.capturerInfo.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    EXPECT_TRUE(audioServer->HandleCheckRecorderBackgroundCapture(config));

    config.callerUid = 1000;
    EXPECT_FALSE(audioServer->HandleCheckRecorderBackgroundCapture(config));
}

/**
 * @tc.name  : Test CreateHdiSinkPort API
 * @tc.type  : FUNC
 * @tc.number: CreateHdiSinkPort_001
 * @tc.desc  : Test CreateHdiSinkPort interface.
 */
HWTEST_F(AudioServerUnitTest, CreateHdiSinkPort_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    uint32_t renderId = 0;
    int32_t result = audioServer->CreateHdiSinkPort("deviceClass", "idInfo", IAudioSinkAttr(), renderId);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name  : Test CreateSinkPort API
 * @tc.type  : FUNC
 * @tc.number: CreateSinkPort_001
 * @tc.desc  : Test CreateSinkPort interface.
 */
HWTEST_F(AudioServerUnitTest, CreateSinkPort_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    HdiIdBase idBase = HDI_ID_BASE_RENDER;
    HdiIdType idType = HDI_ID_TYPE_PRIMARY;
    std::string idInfo = "test";
    IAudioSinkAttr attr;
    uint32_t renderId = 0;
    int32_t result = audioServer->CreateSinkPort(idBase, idType, idInfo, attr, renderId);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name  : Test CreateSinkPort API
 * @tc.type  : FUNC
 * @tc.number: CreateSinkPort_002
 * @tc.desc  : Test CreateSinkPort interface.
 */
HWTEST_F(AudioServerUnitTest, CreateSinkPort_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    HdiIdBase idBase = HDI_ID_BASE_RENDER;
    HdiIdType idType = HDI_ID_TYPE_FAST;
    std::string idInfo = "test";
    IAudioSinkAttr attr;
    uint32_t renderId = 0;
    int32_t result = audioServer->CreateSinkPort(idBase, idType, idInfo, attr, renderId);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name  : Test CreateSourcePort API
 * @tc.type  : FUNC
 * @tc.number: CreateSourcePort_001
 * @tc.desc  : Test CreateSourcePort interface.
 */
HWTEST_F(AudioServerUnitTest, CreateSourcePort_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    HdiIdBase idBase = HDI_ID_BASE_CAPTURE;
    HdiIdType idType = HDI_ID_TYPE_FAST;
    std::string idInfo = "test";
    IAudioSourceAttr attr;
    attr.sourceType = 1;
    uint32_t captureId = 0;
    int32_t result = audioServer->CreateSourcePort(idBase, idType, idInfo, attr, captureId);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name  : Test CreateSourcePort API
 * @tc.type  : FUNC
 * @tc.number: CreateSourcePort_002
 * @tc.desc  : Test CreateSourcePort interface.
 */
HWTEST_F(AudioServerUnitTest, CreateSourcePort_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    HdiIdBase idBase = HDI_ID_BASE_CAPTURE;
    HdiIdType idType = HDI_ID_TYPE_PRIMARY;
    std::string idInfo = "test";
    uint32_t captureId = 0;
    IAudioSourceAttr attr;
    attr.sourceType = 100;
    int32_t result = audioServer->CreateSourcePort(idBase, idType, idInfo, attr, captureId);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name  : Test CreateHdiSourcePort API
 * @tc.type  : FUNC
 * @tc.number: CreateHdiSourcePort_001
 * @tc.desc  : Test CreateHdiSourcePort interface.
 */
HWTEST_F(AudioServerUnitTest, CreateHdiSourcePort_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    uint32_t captureId = 0;
    int32_t result = audioServer->CreateHdiSourcePort("deviceClass", "idInfo", IAudioSourceAttr(), captureId);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name  : Test SetActiveOutputDevice API
 * @tc.type  : FUNC
 * @tc.number: SetActiveOutputDevice_001
 * @tc.desc  : Test SetActiveOutputDevice interface.
 */
HWTEST_F(AudioServerUnitTest, SetActiveOutputDevice_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    int32_t result = audioServer->SetActiveOutputDevice(DEVICE_TYPE_NONE);
    EXPECT_EQ(result, SUCCESS);
    result = audioServer->SetActiveOutputDevice(DEVICE_TYPE_INVALID);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test ImproveAudioWorkgroupPrio API
 * @tc.type  : FUNC
 * @tc.number: ImproveAudioWorkgroupPrio_001
 * @tc.desc  : Test ImproveAudioWorkgroupPrio when valid pid and threads
 */
HWTEST_F(AudioServerUnitTest, ImproveAudioWorkgroupPrio_001, TestSize.Level1)
{
    pid_t pid = 1234;
    std::unordered_map<int32_t, bool> threads = {{1, true}, {2, false}};
    int32_t result = audioServer->ImproveAudioWorkgroupPrio(pid, threads);
    EXPECT_EQ(result, 0);
}
 
/**
 * @tc.name  : Test ImproveAudioWorkgroupPrio API
 * @tc.type  : FUNC
 * @tc.number: ImproveAudioWorkgroupPrio_002
 * @tc.desc  : Test ImproveAudioWorkgroupPrio when valid pid
 */
HWTEST_F(AudioServerUnitTest, ImproveAudioWorkgroupPrio_002, TestSize.Level1)
{
    pid_t pid = -1;
    std::unordered_map<int32_t, bool> threads = {{1, true}, {2, false}};
    int32_t result = audioServer->ImproveAudioWorkgroupPrio(pid, threads);
    EXPECT_EQ(result, 0);
}
 
/**
 * @tc.name  : Test ImproveAudioWorkgroupPrio API
 * @tc.type  : FUNC
 * @tc.number: ImproveAudioWorkgroupPrio_003
 * @tc.desc  : Test ImproveAudioWorkgroupPrio when empty threads
 */
HWTEST_F(AudioServerUnitTest, ImproveAudioWorkgroupPrio_003, TestSize.Level1)
{
    pid_t pid = 1234;
    std::unordered_map<int32_t, bool> threads = {};
    int32_t result = audioServer->ImproveAudioWorkgroupPrio(pid, threads);
    EXPECT_EQ(result, 0);
}
 
/**
 * @tc.name  : Test RestoreAudioWorkgroupPrio API
 * @tc.type  : FUNC
 * @tc.number: RestoreAudioWorkgroupPrio_001
 * @tc.desc  : Test RestoreAudioWorkgroupPrio when called
 */
HWTEST_F(AudioServerUnitTest, RestoreAudioWorkgroupPrio_001, TestSize.Level1)
{
    pid_t pid = 1234;
    std::unordered_map<int32_t, int32_t> threads = {{1, 10}, {2, 20}, {3, 30}};
    int32_t result = audioServer->RestoreAudioWorkgroupPrio(pid, threads);
    EXPECT_EQ(result, 0);
}
 
/**
 * @tc.name  : Test RestoreAudioWorkgroupPrio API
 * @tc.type  : FUNC
 * @tc.number: RestoreAudioWorkgroupPrio_002
 * @tc.desc  : Test RestoreAudioWorkgroupPrio when invalid pid
 */
HWTEST_F(AudioServerUnitTest, RestoreAudioWorkgroupPrio_002, TestSize.Level1)
{
    pid_t pid = -1;
    std::unordered_map<int32_t, int32_t> threads = {{1, 10}, {2, 20}, {3, 30}};
    int32_t result = audioServer->RestoreAudioWorkgroupPrio(pid, threads);
    EXPECT_EQ(result, 0);
}
 
/**
 * @tc.name  : Test RestoreAudioWorkgroupPrio API
 * @tc.type  : FUNC
 * @tc.number: RestoreAudioWorkgroupPrio_003
 * @tc.desc  : Test RestoreAudioWorkgroupPrio when empty threads
 */
HWTEST_F(AudioServerUnitTest, RestoreAudioWorkgroupPrio_003, TestSize.Level1)
{
    pid_t pid = 1234;
    std::unordered_map<int32_t, int32_t> threads = {};
    int32_t result = audioServer->RestoreAudioWorkgroupPrio(pid, threads);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name  : Test SetRenderWhitelist API
 * @tc.type  : FUNC
 * @tc.number: AudioServerSetRenderWhitelist_001
 * @tc.desc  : Test SetRenderWhitelist interface.
 */
HWTEST_F(AudioServerUnitTest, AudioServerSetRenderWhitelist_001, TestSize.Level2)
{
    EXPECT_NE(nullptr, audioServer);

    std::vector<std::string> list;
    list.push_back("com.test");
    int32_t ret = audioServer->SetRenderWhitelist(list);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test GenerateSessionId API
 * @tc.type  : FUNC
 * @tc.number: GenerateSessionId_001
 * @tc.desc  : Test GenerateSessionId interface.
 */
HWTEST_F(AudioServerUnitTest, GenerateSessionId_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    uint32_t sessionId = 0;
    int32_t ret = audioServer->GenerateSessionId(sessionId);
    EXPECT_EQ(ERROR, ret);
}

/**
 * @tc.name  : Test SetAsrVoiceMuteMode API
 * @tc.type  : FUNC
 * @tc.number: SetAsrVoiceMuteMode_001
 * @tc.desc  : Test SetAsrVoiceMuteMode interface.
 */
HWTEST_F(AudioServerUnitTest, SetAsrVoiceMuteMode_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    int32_t asrVoiceMuteMode = 0;
    bool on = true;
    EXPECT_EQ(audioServer->SetAsrVoiceMuteMode(asrVoiceMuteMode, on), 0);
}

/**
 * @tc.name  : Test OnMuteStateChange API
 * @tc.type  : FUNC
 * @tc.number: OnMuteStateChange_001
 * @tc.desc  : Test OnMuteStateChange interface.
 */
HWTEST_F(AudioServerUnitTest, OnMuteStateChange_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    int32_t uid = 0;
    uint32_t sessionId = 0;
    bool isMuted = true;

    audioServer->audioDataTransferCbMap_.clear();
    audioServer->OnMuteStateChange(0, 0, uid, sessionId, isMuted);
    EXPECT_EQ(audioServer->audioDataTransferCbMap_.size(), 0);
}

/**
 * @tc.name  : Test OnMuteStateChange API
 * @tc.type  : FUNC
 * @tc.number: OnMuteStateChange_002
 * @tc.desc  : Test OnMuteStateChange interface.
 */
HWTEST_F(AudioServerUnitTest, OnMuteStateChange_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, audioServer);
    int32_t callbackId = 1;
    int32_t uid = 0;
    uint32_t sessionId = 0;
    bool isMuted = true;

    audioServer->audioDataTransferCbMap_.clear();
    std::shared_ptr<DataTransferStateChangeCallbackInner> callback =
        std::make_shared<DataTransferStateChangeCallbackInnerTest>();
    int32_t pid = IPCSkeleton::GetCallingPid();
    audioServer->audioDataTransferCbMap_[pid] = callback;
    audioServer->OnMuteStateChange(pid, callbackId, uid, sessionId, isMuted);
    audioServer->audioDataTransferCbMap_.clear();
    EXPECT_EQ(audioServer->audioDataTransferCbMap_.size(), 0);
}

/**
 * @tc.name  : Test ArgDataDump API
 * @tc.type  : FUNC
 * @tc.number: ArgDataDump_001
 * @tc.desc  : Test ArgDataDump interface.
 */
HWTEST_F(AudioServerUnitTest, ArgDataDump_001, TestSize.Level1)
{
    AudioServerHpaeDump audioServerHpaeDump;
    std::string dumpString;
    std::queue<std::u16string> argQue;

    audioServerHpaeDump.ArgDataDump(dumpString, argQue);

    EXPECT_NE(dumpString, "Hpae AudioServer Data Dump:\n\n");
}
} // namespace AudioStandard
} // namespace OHOS
