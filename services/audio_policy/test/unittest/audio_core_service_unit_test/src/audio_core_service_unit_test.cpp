/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "audio_core_service_unit_test.h"
#include "get_server_util.h"

#include <thread>
#include <memory>
#include <vector>
#include "audio_info.h"
#include "audio_device_status.h"
#include "audio_core_config_manager.h"
#include "audio_select_interface_service.h"
#include "audio_router_infra.h"
#include "audio_router_select_strategy.h"
#include "i_hpae_manager.h"
#include "audio_volume.h"
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
bool g_hasPermission = false;
const uint32_t TEST_SESSION_ID = 100001;
const int32_t TEST_AUDIO_SELECT_UID = 10001;
const uint32_t TEST_AUDIO_SELECT_STREAM_ID = 100101;
static AudioPolicyServer* GetServerPtr()
{
    return GetServerUtil::GetServerPtr();
}

static void GetPermission()
{
    if (!g_hasPermission) {
        uint64_t tokenId;
        constexpr int perNum = 10;
        const char *perms[perNum] = {
            "ohos.permission.MICROPHONE",
            "ohos.permission.MANAGE_INTELLIGENT_VOICE",
            "ohos.permission.MANAGE_AUDIO_CONFIG",
            "ohos.permission.MICROPHONE_CONTROL",
            "ohos.permission.MODIFY_AUDIO_SETTINGS",
            "ohos.permission.ACCESS_NOTIFICATION_POLICY",
            "ohos.permission.USE_BLUETOOTH",
            "ohos.permission.CAPTURE_VOICE_DOWNLINK_AUDIO",
            "ohos.permission.RECORD_VOICE_CALL",
            "ohos.permission.MANAGE_SYSTEM_AUDIO_EFFECTS",
        };

        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = 10,
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .processName = "audio_core_service_unit_test",
            .aplStr = "system_basic",
        };
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
        g_hasPermission = true;
    }
}

static void GetNonSystemPermission()
{
    uint64_t tokenId;
    constexpr int perNum = 1;
    const char *perms[perNum] = {
        "ohos.permission.MICROPHONE",
    };

    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = perNum,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "audio_core_service_unit_test_non_system",
        .aplStr = "normal",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

static void ResetSystemPermission()
{
    uint64_t tokenId;
    constexpr int perNum = 10;
    const char *perms[perNum] = {
        "ohos.permission.MICROPHONE",
        "ohos.permission.MANAGE_INTELLIGENT_VOICE",
        "ohos.permission.MANAGE_AUDIO_CONFIG",
        "ohos.permission.MICROPHONE_CONTROL",
        "ohos.permission.MODIFY_AUDIO_SETTINGS",
        "ohos.permission.ACCESS_NOTIFICATION_POLICY",
        "ohos.permission.USE_BLUETOOTH",
        "ohos.permission.CAPTURE_VOICE_DOWNLINK_AUDIO",
        "ohos.permission.RECORD_VOICE_CALL",
        "ohos.permission.MANAGE_SYSTEM_AUDIO_EFFECTS",
    };

    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = perNum,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "audio_core_service_unit_test",
        .aplStr = "system_basic",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
    g_hasPermission = true;
}

static std::shared_ptr<PipeStreamPropInfo> CreatePipeStreamPropInfoForDevice(
    DeviceType deviceType, AudioChannel channels, uint32_t sampleRate = SAMPLE_RATE_48000,
    AudioSampleFormat format = SAMPLE_S16LE)
{
    auto streamPropInfo = std::make_shared<PipeStreamPropInfo>();
    streamPropInfo->format_ = format;
    streamPropInfo->sampleRate_ = sampleRate;
    streamPropInfo->channels_ = channels;
    streamPropInfo->channelLayout_ = channels == MONO ? CH_LAYOUT_MONO : CH_LAYOUT_STEREO;
    streamPropInfo->supportDeviceMap_.insert({deviceType, std::make_shared<AdapterDeviceInfo>()});
    return streamPropInfo;
}

static std::shared_ptr<AudioStreamDescriptor> CreateCapturerStreamDescriptorForVoipPrivacy(
    uint32_t sessionId, SourceType sourceType, AudioStreamStatus status = STREAM_STATUS_STARTED,
    int32_t callerPid = 0)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = sessionId;
    streamDesc->audioMode_ = AUDIO_MODE_RECORD;
    streamDesc->streamStatus_ = status;
    streamDesc->capturerInfo_.sourceType = sourceType;
    streamDesc->callerPid_ = callerPid;
    streamDesc->appInfo_.appPid = callerPid;
    return streamDesc;
}

static void AddCapturerStreamsToPipeManager(
    const std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs)
{
    auto pipeManager = AudioPipeManager::GetPipeManager();
    CHECK_AND_RETURN_LOG(pipeManager != nullptr, "pipeManager is nullptr");
    pipeManager->curPipeList_.clear();

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;
    pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    pipeInfo->streamDescriptors_ = streamDescs;
    pipeManager->AddAudioPipeInfo(pipeInfo);
}

static void ClearCapturerStreamsFromPipeManager()
{
    auto pipeManager = AudioPipeManager::GetPipeManager();
    CHECK_AND_RETURN_LOG(pipeManager != nullptr, "pipeManager is nullptr");
    pipeManager->curPipeList_.clear();
}

void AudioCoreServiceUnitTest::SetUpTestCase(void)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest::SetUpTestCase start-end");
    AudioPolicyServer* server = GetServerPtr();
    server->isNeedPublished_ = true;
    GetPermission();
    GetServerPtr()->eventEntry_->NotifyServiceReady();
}
void AudioCoreServiceUnitTest::TearDownTestCase(void)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest::TearDownTestCase start-end");
    AudioPolicyServer* server = GetServerPtr();
    server->isNeedPublished_ = false;
    server->coreService_ = nullptr;
}
void AudioCoreServiceUnitTest::SetUp(void)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest::SetUp start-end");
}
void AudioCoreServiceUnitTest::TearDown(void)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest::TearDown start-end");
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: CreateRenderClient_001
* @tc.desc  : Test CreateRenderClient - Create stream with (S32 48k STEREO) will be successful.
*/
HWTEST_F(AudioCoreServiceUnitTest, CreateRenderClient_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest CreateRenderClient_001 start");
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamInfo_.format = AudioSampleFormat::SAMPLE_S32LE;
    streamDesc->streamInfo_.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    streamDesc->streamInfo_.channels = AudioChannel::STEREO;
    streamDesc->streamInfo_.encoding = AudioEncodingType::ENCODING_PCM;
    streamDesc->streamInfo_.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MOVIE;

    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->createTimeStamp_ = ClockTime::GetCurNano();
    streamDesc->callerUid_ = getuid();
    uint32_t flag = AUDIO_OUTPUT_FLAG_NORMAL;
    uint32_t originalSessionId = 0;
    std::string networkId = LOCAL_NETWORK_ID;
    auto result = GetServerPtr()->eventEntry_->CreateRendererClient(streamDesc, flag, originalSessionId, networkId);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: CreateRenderClient_002
* @tc.desc  : Test CreateRenderClient - Create stream with (S32 96k STEREO) will be successful.
*/
HWTEST_F(AudioCoreServiceUnitTest, CreateRenderClient_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest CreateRenderClient_002 start");
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamInfo_.format = AudioSampleFormat::SAMPLE_S32LE;
    streamDesc->streamInfo_.samplingRate = AudioSamplingRate::SAMPLE_RATE_96000;
    streamDesc->streamInfo_.channels = AudioChannel::STEREO;
    streamDesc->streamInfo_.encoding = AudioEncodingType::ENCODING_PCM;
    streamDesc->streamInfo_.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_RINGTONE;

    streamDesc->callerUid_ = getuid();
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->createTimeStamp_ = ClockTime::GetCurNano();
    uint32_t originalSessionId = 0;
    uint32_t flag = AUDIO_OUTPUT_FLAG_NORMAL;
    std::string networkId = LOCAL_NETWORK_ID;
    auto result = GetServerPtr()->eventEntry_->CreateRendererClient(streamDesc, flag, originalSessionId, networkId);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: CreateRenderClient_003
* @tc.desc  : Test CreateRenderClient - Create stream with (S32 96k STEREO) will be successful.
*/
HWTEST_F(AudioCoreServiceUnitTest, CreateRenderClient_003, TestSize.Level1)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.toneFlag = false;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    uint32_t originalSessionId = 0;
    uint32_t flag = AUDIO_OUTPUT_FLAG_NORMAL;
    std::string networkId = LOCAL_NETWORK_ID;
    auto result = GetServerPtr()->eventEntry_->CreateRendererClient(streamDesc, flag, originalSessionId, networkId);

    streamDesc->rendererInfo_.toneFlag = false;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_VOICE_CALL_ASSISTANT;
    result = GetServerPtr()->eventEntry_->CreateRendererClient(streamDesc, flag, originalSessionId, networkId);

    streamDesc->rendererInfo_.toneFlag = true;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    result = GetServerPtr()->eventEntry_->CreateRendererClient(streamDesc, flag, originalSessionId, networkId);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: CreateRenderClient_004
* @tc.desc  : Test CreateRenderClient - active bluetooth a2dp.
*/
HWTEST_F(AudioCoreServiceUnitTest, CreateRenderClient_004, TestSize.Level1)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamInfo_.format = AudioSampleFormat::SAMPLE_S32LE;
    streamDesc->streamInfo_.samplingRate = AudioSamplingRate::SAMPLE_RATE_96000;
    streamDesc->streamInfo_.channels = AudioChannel::STEREO;
    streamDesc->streamInfo_.encoding = AudioEncodingType::ENCODING_PCM;
    streamDesc->streamInfo_.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_RINGTONE;

    streamDesc->callerUid_ = getuid();
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->createTimeStamp_ = ClockTime::GetCurNano();

    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    deviceDesc->networkId_ = LOCAL_NETWORK_ID;
    deviceDesc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    deviceDesc->macAddress_ = "00:00:00:00:00:00";
    streamDesc->newDeviceDescs_.clear();
    streamDesc->newDeviceDescs_.push_back(deviceDesc);

    uint32_t originalSessionId = 0;
    uint32_t flag = AUDIO_OUTPUT_FLAG_NORMAL;
    std::string networkId = LOCAL_NETWORK_ID;
    auto result = GetServerPtr()->eventEntry_->CreateRendererClient(streamDesc, flag, originalSessionId, networkId);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: CreateRenderClient_005
* @tc.desc  : Test CreateRenderClient - inactive bluetooth a2dp.
*/
HWTEST_F(AudioCoreServiceUnitTest, CreateRenderClient_005, TestSize.Level1)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamInfo_.format = AudioSampleFormat::SAMPLE_S32LE;
    streamDesc->streamInfo_.samplingRate = AudioSamplingRate::SAMPLE_RATE_96000;
    streamDesc->streamInfo_.channels = AudioChannel::STEREO;
    streamDesc->streamInfo_.encoding = AudioEncodingType::ENCODING_PCM;
    streamDesc->streamInfo_.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_RINGTONE;

    streamDesc->callerUid_ = getuid();
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->createTimeStamp_ = ClockTime::GetCurNano();

    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_MIC;
    deviceDesc->networkId_ = LOCAL_NETWORK_ID;
    deviceDesc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    deviceDesc->macAddress_ = "00:00:00:00:00:00";
    streamDesc->newDeviceDescs_.clear();
    streamDesc->newDeviceDescs_.push_back(deviceDesc);

    uint32_t originalSessionId = 0;
    uint32_t flag = AUDIO_OUTPUT_FLAG_NORMAL;
    std::string networkId = LOCAL_NETWORK_ID;
    auto result = GetServerPtr()->eventEntry_->CreateRendererClient(streamDesc, flag, originalSessionId, networkId);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: CreateCapturerClient_001
* @tc.desc  : Test CreateCapturerClient - Create stream with (S32 48k STEREO) will be successful..
*/
HWTEST_F(AudioCoreServiceUnitTest, CreateCapturerClient_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest CreateCapturerClient_001 start");
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamInfo_.format = AudioSampleFormat::SAMPLE_S32LE;
    streamDesc->streamInfo_.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    streamDesc->streamInfo_.channels = AudioChannel::STEREO;
    streamDesc->streamInfo_.encoding = AudioEncodingType::ENCODING_PCM;
    streamDesc->streamInfo_.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MOVIE;

    streamDesc->audioMode_ = AUDIO_MODE_RECORD;
    streamDesc->createTimeStamp_ = ClockTime::GetCurNano();
    streamDesc->callerUid_ = getuid();
    uint32_t flag = AUDIO_INPUT_FLAG_NORMAL;
    uint32_t originalSessionId = 0;
    auto result = GetServerPtr()->eventEntry_->CreateCapturerClient(streamDesc, flag, originalSessionId);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: CreateCapturerClient_002
* @tc.desc  : Test CreateCapturerClient - Create stream with (S32 48k STEREO) will be successful..
*/
HWTEST_F(AudioCoreServiceUnitTest, CreateCapturerClient_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest CreateCapturerClient_002 start");
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamInfo_.format = AudioSampleFormat::SAMPLE_S32LE;
    streamDesc->streamInfo_.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    streamDesc->streamInfo_.channels = AudioChannel::STEREO;
    streamDesc->streamInfo_.encoding = AudioEncodingType::ENCODING_PCM;
    streamDesc->streamInfo_.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MOVIE;

    streamDesc->audioMode_ = AUDIO_MODE_RECORD;
    streamDesc->createTimeStamp_ = ClockTime::GetCurNano();
    streamDesc->callerUid_ = getuid();
    uint32_t flag = AUDIO_INPUT_FLAG_NORMAL;
    uint32_t sessionId = 0;

    auto result = GetServerPtr()->eventEntry_->CreateCapturerClient(streamDesc, flag, sessionId);

    EXPECT_NE(sessionId, 0);
    EXPECT_EQ(result, SUCCESS);

    EXPECT_FALSE(streamDesc->newDeviceDescs_.empty());
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: CreateCapturerClient_003
* @tc.desc  : Test CreateCapturerClient - Create stream with (S32 48k STEREO) will be successful..
*/
HWTEST_F(AudioCoreServiceUnitTest, CreateCapturerClient_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest CreateCapturerClient_003 start");
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamInfo_.format = AudioSampleFormat::SAMPLE_S32LE;
    streamDesc->streamInfo_.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    streamDesc->streamInfo_.channels = AudioChannel::STEREO;
    streamDesc->streamInfo_.encoding = AudioEncodingType::ENCODING_PCM;
    streamDesc->streamInfo_.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MOVIE;

    streamDesc->audioMode_ = AUDIO_MODE_RECORD;
    streamDesc->createTimeStamp_ = ClockTime::GetCurNano();
    streamDesc->callerUid_ = getuid();
    uint32_t flag = AUDIO_INPUT_FLAG_NORMAL;
    uint32_t originalSessionId = 1;
    auto result = GetServerPtr()->eventEntry_->CreateCapturerClient(streamDesc, flag, originalSessionId);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: CreateCapturerClient_004
* @tc.desc  : Test CreateCapturerClient - voice recognition micIn/ec requires system permission.
*/
HWTEST_F(AudioCoreServiceUnitTest, CreateCapturerClient_004, TestSize.Level1)
{
    GetNonSystemPermission();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    streamDesc->micInStreamInfo_.channels = AudioChannel::CHANNEL_4;
    streamDesc->ecStreamInfo_.channels = AudioChannel::STEREO;

    AudioCoreService audioCoreService;
    uint32_t flag = AUDIO_INPUT_FLAG_NORMAL;
    uint32_t sessionId = 0;
    auto ret = audioCoreService.CreateCapturerClient(streamDesc, flag, sessionId);
    EXPECT_EQ(ret, SUCCESS);

    ResetSystemPermission();
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: CreateCapturerClient_005
* @tc.desc  : Test CreateCapturerClient - voice recognition micIn/ec needs independent route support.
*/
HWTEST_F(AudioCoreServiceUnitTest, CreateCapturerClient_005, TestSize.Level1)
{
    auto oldSourceStrategyMap = AudioSourceStrategyData::GetInstance().GetSourceStrategyMap();
    auto sourceStrategyMap = std::make_shared<std::map<SourceType, AudioSourceStrategyType>>();
    sourceStrategyMap->emplace(SOURCE_TYPE_VOICE_RECOGNITION,
        AudioSourceStrategyType {
        "AUDIO_INPUT_VOICE_RECOGNITION_TYPE", // hdiSource
        "primary", // adapterName
        "primary_input", // pipeName
        AUDIO_INPUT_FLAG_VOICE_RECOGNITION, // audioFlag
        1 // priority
        });
    AudioSourceStrategyData::GetInstance().SetSourceStrategyMap(sourceStrategyMap);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    streamDesc->micInStreamInfo_.channels = AudioChannel::CHANNEL_4;
    streamDesc->ecStreamInfo_.channels = AudioChannel::STEREO;

    AudioCoreService audioCoreService;
    uint32_t flag = AUDIO_INPUT_FLAG_NORMAL;
    uint32_t sessionId = 0;
    auto ret = audioCoreService.CreateCapturerClient(streamDesc, flag, sessionId);
    EXPECT_EQ(ret, SUCCESS);

    AudioSourceStrategyData::GetInstance().SetSourceStrategyMap(oldSourceStrategyMap);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: CreateCapturerClient_006
* @tc.desc  : Test CreateCapturerClient - voice recognition micIn/ec passes permission and route checks.
*/
HWTEST_F(AudioCoreServiceUnitTest, CreateCapturerClient_006, TestSize.Level1)
{
    ResetSystemPermission();
    auto oldSourceStrategyMap = AudioSourceStrategyData::GetInstance().GetSourceStrategyMap();
    auto sourceStrategyMap = std::make_shared<std::map<SourceType, AudioSourceStrategyType>>();
    sourceStrategyMap->emplace(SOURCE_TYPE_VOICE_RECOGNITION,
        AudioSourceStrategyType {
        "AUDIO_INPUT_VOICE_RECOGNITION_TYPE", // hdiSource
        "primary", // adapterName
        "primary_input_voice_recognition", // pipeName
        AUDIO_INPUT_FLAG_VOICE_RECOGNITION, // audioFlag
        1 // priority
        });
    AudioSourceStrategyData::GetInstance().SetSourceStrategyMap(sourceStrategyMap);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamInfo_.format = AudioSampleFormat::SAMPLE_S16LE;
    streamDesc->streamInfo_.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    streamDesc->streamInfo_.channels = AudioChannel::MONO;
    streamDesc->streamInfo_.encoding = AudioEncodingType::ENCODING_PCM;
    streamDesc->streamInfo_.channelLayout = AudioChannelLayout::CH_LAYOUT_MONO;
    streamDesc->audioMode_ = AUDIO_MODE_RECORD;
    streamDesc->createTimeStamp_ = ClockTime::GetCurNano();
    streamDesc->callerUid_ = getuid();
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    streamDesc->micInStreamInfo_.channels = AudioChannel::CHANNEL_4;
    streamDesc->ecStreamInfo_.channels = AudioChannel::STEREO;

    AudioCoreService audioCoreService;
    uint32_t flag = AUDIO_INPUT_FLAG_NORMAL;
    uint32_t sessionId = 0;
    auto ret = audioCoreService.CreateCapturerClient(streamDesc, flag, sessionId);
    EXPECT_NE(ret, ERR_PERMISSION_DENIED);
    EXPECT_NE(ret, ERR_NOT_SUPPORTED);

    AudioSourceStrategyData::GetInstance().SetSourceStrategyMap(oldSourceStrategyMap);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: CreateCapturerClient_007
* @tc.desc  : Test remote input stream bypasses voice recognition micIn/ec permission and route checks.
*/
HWTEST_F(AudioCoreServiceUnitTest, CreateCapturerClient_007, TestSize.Level1)
{
    GetNonSystemPermission();
    auto oldSourceStrategyMap = AudioSourceStrategyData::GetInstance().GetSourceStrategyMap();
    auto sourceStrategyMap = std::make_shared<std::map<SourceType, AudioSourceStrategyType>>();
    sourceStrategyMap->emplace(SOURCE_TYPE_VOICE_RECOGNITION,
        AudioSourceStrategyType {
        "AUDIO_INPUT_VOICE_RECOGNITION_TYPE", // hdiSource
        "primary", // adapterName
        "primary_input", // pipeName, not independent route
        AUDIO_INPUT_FLAG_VOICE_RECOGNITION, // audioFlag
        1 // priority
        });
    AudioSourceStrategyData::GetInstance().SetSourceStrategyMap(sourceStrategyMap);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    streamDesc->micInStreamInfo_.channels = AudioChannel::CHANNEL_4;
    streamDesc->ecStreamInfo_.channels = AudioChannel::STEREO;
    auto inputDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    inputDevice->networkId_ = "RemoteDevice";
    streamDesc->newDeviceDescs_.push_back(inputDevice);

    AudioCoreService audioCoreService;
    uint32_t flag = AUDIO_INPUT_FLAG_NORMAL;
    uint32_t sessionId = 0;
    auto ret = audioCoreService.CreateCapturerClient(streamDesc, flag, sessionId);
    EXPECT_NE(ret, ERR_PERMISSION_DENIED);
    EXPECT_NE(ret, ERR_NOT_SUPPORTED);

    AudioSourceStrategyData::GetInstance().SetSourceStrategyMap(oldSourceStrategyMap);
    ResetSystemPermission();
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: CreateCapturerClient_008
* @tc.desc  : Test CreateCapturerClient - camcorder micIn needs independent route support.
*/
HWTEST_F(AudioCoreServiceUnitTest, CreateCapturerClient_008, TestSize.Level1)
{
    auto oldSourceStrategyMap = AudioSourceStrategyData::GetInstance().GetSourceStrategyMap();
    auto sourceStrategyMap = std::make_shared<std::map<SourceType, AudioSourceStrategyType>>();
    sourceStrategyMap->emplace(SOURCE_TYPE_CAMCORDER,
        AudioSourceStrategyType {
        "AUDIO_INPUT_CAMCORDER_TYPE",
        "primary",
        "primary_input",
        AUDIO_INPUT_FLAG_CAMCORDER,
        1
        });
    AudioSourceStrategyData::GetInstance().SetSourceStrategyMap(sourceStrategyMap);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_CAMCORDER;
    streamDesc->micInStreamInfo_.channels = AudioChannel::CHANNEL_4;

    AudioCoreService audioCoreService;
    uint32_t flag = AUDIO_INPUT_FLAG_NORMAL;
    uint32_t sessionId = 0;
    auto ret = audioCoreService.CreateCapturerClient(streamDesc, flag, sessionId);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);

    AudioSourceStrategyData::GetInstance().SetSourceStrategyMap(oldSourceStrategyMap);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: CreateCapturerClient_009
* @tc.desc  : Test CreateCapturerClient - camcorder micIn passes independent route check.
*/
HWTEST_F(AudioCoreServiceUnitTest, CreateCapturerClient_009, TestSize.Level1)
{
    auto oldSourceStrategyMap = AudioSourceStrategyData::GetInstance().GetSourceStrategyMap();
    auto sourceStrategyMap = std::make_shared<std::map<SourceType, AudioSourceStrategyType>>();
    sourceStrategyMap->emplace(SOURCE_TYPE_CAMCORDER,
        AudioSourceStrategyType {
        "AUDIO_INPUT_CAMCORDER_TYPE",
        "primary",
        "primary_input_camcorder",
        AUDIO_INPUT_FLAG_CAMCORDER,
        1
        });
    AudioSourceStrategyData::GetInstance().SetSourceStrategyMap(sourceStrategyMap);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamInfo_.format = AudioSampleFormat::SAMPLE_S16LE;
    streamDesc->streamInfo_.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    streamDesc->streamInfo_.channels = AudioChannel::STEREO;
    streamDesc->streamInfo_.encoding = AudioEncodingType::ENCODING_PCM;
    streamDesc->streamInfo_.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    streamDesc->audioMode_ = AUDIO_MODE_RECORD;
    streamDesc->createTimeStamp_ = ClockTime::GetCurNano();
    streamDesc->callerUid_ = getuid();
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_CAMCORDER;
    streamDesc->micInStreamInfo_.channels = AudioChannel::CHANNEL_4;

    AudioCoreService audioCoreService;
    uint32_t flag = AUDIO_INPUT_FLAG_NORMAL;
    uint32_t sessionId = 0;
    auto ret = audioCoreService.CreateCapturerClient(streamDesc, flag, sessionId);
    EXPECT_NE(ret, ERR_NOT_SUPPORTED);

    AudioSourceStrategyData::GetInstance().SetSourceStrategyMap(oldSourceStrategyMap);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: SetPreferredInputDeviceIfValid_001
* @tc.desc  : Test CreateCapturerClient - Create stream with (S32 48k STEREO) will be successful..
*/
HWTEST_F(AudioCoreServiceUnitTest, SetPreferredInputDeviceIfValid_001, TestSize.Level1)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->preferredInputDevice.deviceType_ = DEVICE_TYPE_INVALID;
    streamDesc->sessionId_ = 1;
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;

    AudioCoreService audioCoreService;

    EXPECT_NO_THROW(audioCoreService.SetPreferredInputDeviceIfValid(streamDesc));
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: SetPreferredInputDeviceIfValid_002
* @tc.desc  : Test CreateCapturerClient - Create stream with (S32 48k STEREO) will be successful..
*/
HWTEST_F(AudioCoreServiceUnitTest, SetPreferredInputDeviceIfValid_002, TestSize.Level1)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->preferredInputDevice.deviceType_ = DEVICE_TYPE_BT_SPP;
    streamDesc->sessionId_ = 1;
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_INVALID;

    AudioCoreService audioCoreService;

    EXPECT_NO_THROW(audioCoreService.SetPreferredInputDeviceIfValid(streamDesc));
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: SetPreferredInputDeviceIfValid_003
* @tc.desc  : Test CreateCapturerClient - Create stream with (S32 48k STEREO) will be successful..
*/
HWTEST_F(AudioCoreServiceUnitTest, SetPreferredInputDeviceIfValid_003, TestSize.Level1)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->preferredInputDevice.deviceType_ = DEVICE_TYPE_INVALID;
    streamDesc->sessionId_ = 1;
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_INVALID;

    AudioCoreService audioCoreService;

    EXPECT_NO_THROW(audioCoreService.SetPreferredInputDeviceIfValid(streamDesc));
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: SetPreferredInputDeviceIfValid_004
* @tc.desc  : Test CreateCapturerClient - Create stream with (S32 48k STEREO) will be successful..
*/
HWTEST_F(AudioCoreServiceUnitTest, SetPreferredInputDeviceIfValid_004, TestSize.Level1)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->preferredInputDevice.deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->sessionId_ = 1;
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;

    AudioCoreService audioCoreService;

    EXPECT_NO_THROW(audioCoreService.SetPreferredInputDeviceIfValid(streamDesc));
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: SetPreferredInputDeviceIfValid_005
* @tc.desc  : Test CreateCapturerClient - Create stream with (S32 48k STEREO) will be successful..
*/
HWTEST_F(AudioCoreServiceUnitTest, SetPreferredInputDeviceIfValid_005, TestSize.Level1)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->preferredInputDevice.deviceType_ = DEVICE_TYPE_BT_SPP;
    streamDesc->sessionId_ = 1;
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;

    AudioCoreService audioCoreService;

    EXPECT_NO_THROW(audioCoreService.SetPreferredInputDeviceIfValid(streamDesc));
}

/**
* @tc.name  : Test AudioSelectInterfaceService.
* @tc.number: AudioSelectInterfaceService_SetInputDevice_001
* @tc.desc  : Test SetInputDevice updates selected input device for stream.
*/
HWTEST_F(AudioCoreServiceUnitTest, AudioSelectInterfaceService_SetInputDevice_001, TestSize.Level1)
{
    uint32_t sessionId = TEST_AUDIO_SELECT_STREAM_ID;
    auto &routerInfra = AudioRouterInfra::GetInstance();
    routerInfra.UpdateStreamSelectDevice(TEST_AUDIO_SELECT_UID, sessionId, nullptr);

    int32_t ret = AudioSelectInterfaceService::GetInstance().SetInputDevice(DEVICE_TYPE_MIC, sessionId,
        TEST_AUDIO_SELECT_UID);
    EXPECT_EQ(ret, SUCCESS);

    auto selectedDevice = routerInfra.GetStreamSelectDevice(TEST_AUDIO_SELECT_UID, sessionId);
    ASSERT_NE(selectedDevice, nullptr);
    EXPECT_EQ(selectedDevice->deviceType_, DEVICE_TYPE_MIC);

    routerInfra.UpdateStreamSelectDevice(TEST_AUDIO_SELECT_UID, sessionId, nullptr);
}

/**
* @tc.name  : Test AudioSelectInterfaceService.
* @tc.number: AudioSelectInterfaceService_SetPreferredInputDeviceIfValid_001
* @tc.desc  : Test SetPreferredInputDeviceIfValid handles nullptr and invalid preferred device.
*/
HWTEST_F(AudioCoreServiceUnitTest, AudioSelectInterfaceService_SetPreferredInputDeviceIfValid_001, TestSize.Level1)
{
    GetPermission();
    auto &routerInfra = AudioRouterInfra::GetInstance();

    uint32_t invalidSessionId = TEST_AUDIO_SELECT_STREAM_ID + 1;
    routerInfra.UpdateStreamSelectDevice(TEST_AUDIO_SELECT_UID, invalidSessionId, nullptr);
    std::shared_ptr<AudioStreamDescriptor> invalidStreamDesc = std::make_shared<AudioStreamDescriptor>();
    invalidStreamDesc->sessionId_ = invalidSessionId;
    invalidStreamDesc->appInfo_.appUid = TEST_AUDIO_SELECT_UID;
    invalidStreamDesc->preferredInputDevice.deviceType_ = DEVICE_TYPE_INVALID;
    EXPECT_NO_THROW(AudioSelectInterfaceService::GetInstance().SetPreferredInputDeviceIfValid(nullptr));
    EXPECT_NO_THROW(AudioSelectInterfaceService::GetInstance().SetPreferredInputDeviceIfValid(invalidStreamDesc));
    EXPECT_EQ(routerInfra.GetStreamSelectDevice(TEST_AUDIO_SELECT_UID, invalidSessionId), nullptr);
    routerInfra.UpdateStreamSelectDevice(TEST_AUDIO_SELECT_UID, invalidSessionId, nullptr);
}

/**
* @tc.name  : Test AudioSelectInterfaceService.
* @tc.number: AudioSelectInterfaceService_SetPreferredInputDeviceIfValid_002
* @tc.desc  : Test SetPreferredInputDeviceIfValid updates selected input device for recognition stream.
*/
HWTEST_F(AudioCoreServiceUnitTest, AudioSelectInterfaceService_SetPreferredInputDeviceIfValid_002, TestSize.Level1)
{
    GetPermission();
    auto &routerInfra = AudioRouterInfra::GetInstance();
    uint32_t recognitionSessionId = TEST_AUDIO_SELECT_STREAM_ID + 2;
    routerInfra.UpdateStreamSelectDevice(TEST_AUDIO_SELECT_UID, recognitionSessionId, nullptr);
    std::shared_ptr<AudioStreamDescriptor> recognitionStreamDesc = std::make_shared<AudioStreamDescriptor>();
    recognitionStreamDesc->sessionId_ = recognitionSessionId;
    recognitionStreamDesc->appInfo_.appUid = TEST_AUDIO_SELECT_UID;
    recognitionStreamDesc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    recognitionStreamDesc->preferredInputDevice.deviceType_ = DEVICE_TYPE_MIC;
    recognitionStreamDesc->preferredInputDevice.deviceRole_ = INPUT_DEVICE;
    recognitionStreamDesc->preferredInputDevice.networkId_ = LOCAL_NETWORK_ID;
    AudioSelectInterfaceService::GetInstance().SetPreferredInputDeviceIfValid(recognitionStreamDesc);
    auto selectedRecognitionDevice = routerInfra.GetStreamSelectDevice(TEST_AUDIO_SELECT_UID, recognitionSessionId);
    ASSERT_NE(selectedRecognitionDevice, nullptr);
    EXPECT_EQ(selectedRecognitionDevice->deviceType_, DEVICE_TYPE_MIC);
    routerInfra.UpdateStreamSelectDevice(TEST_AUDIO_SELECT_UID, recognitionSessionId, nullptr);
}

/**
* @tc.name  : Test AudioSelectInterfaceService.
* @tc.number: AudioSelectInterfaceService_SetPreferredInputDeviceIfValid_003
* @tc.desc  : Test SetPreferredInputDeviceIfValid accepts BT_SPP for non-recognition stream.
*/
HWTEST_F(AudioCoreServiceUnitTest, AudioSelectInterfaceService_SetPreferredInputDeviceIfValid_003, TestSize.Level1)
{
    GetPermission();
    auto &routerInfra = AudioRouterInfra::GetInstance();
    uint32_t btSppSessionId = TEST_AUDIO_SELECT_STREAM_ID + 3;
    routerInfra.UpdateStreamSelectDevice(TEST_AUDIO_SELECT_UID, btSppSessionId, nullptr);
    std::shared_ptr<AudioStreamDescriptor> btSppStreamDesc = std::make_shared<AudioStreamDescriptor>();
    btSppStreamDesc->sessionId_ = btSppSessionId;
    btSppStreamDesc->appInfo_.appUid = TEST_AUDIO_SELECT_UID;
    btSppStreamDesc->capturerInfo_.sourceType = SOURCE_TYPE_INVALID;
    btSppStreamDesc->preferredInputDevice.deviceType_ = DEVICE_TYPE_BT_SPP;
    btSppStreamDesc->preferredInputDevice.deviceRole_ = INPUT_DEVICE;
    btSppStreamDesc->preferredInputDevice.networkId_ = LOCAL_NETWORK_ID;
    AudioSelectInterfaceService::GetInstance().SetPreferredInputDeviceIfValid(btSppStreamDesc);
    auto selectedBtSppDevice = routerInfra.GetStreamSelectDevice(TEST_AUDIO_SELECT_UID, btSppSessionId);
    ASSERT_NE(selectedBtSppDevice, nullptr);
    EXPECT_EQ(selectedBtSppDevice->deviceType_, DEVICE_TYPE_BT_SPP);
    routerInfra.UpdateStreamSelectDevice(TEST_AUDIO_SELECT_UID, btSppSessionId, nullptr);
}

/**
* @tc.name  : Test AudioSelectInterfaceService.
* @tc.number: AudioSelectInterfaceService_SetPreferredInputDeviceIfValid_004
* @tc.desc  : Test SetPreferredInputDeviceIfValid updates selected input device for normal stream.
*/
HWTEST_F(AudioCoreServiceUnitTest, AudioSelectInterfaceService_SetPreferredInputDeviceIfValid_004, TestSize.Level1)
{
    GetPermission();
    auto &routerInfra = AudioRouterInfra::GetInstance();
    uint32_t normalSessionId = TEST_AUDIO_SELECT_STREAM_ID + 4;
    routerInfra.UpdateStreamSelectDevice(TEST_AUDIO_SELECT_UID, normalSessionId, nullptr);
    std::shared_ptr<AudioStreamDescriptor> normalStreamDesc = std::make_shared<AudioStreamDescriptor>();
    normalStreamDesc->sessionId_ = normalSessionId;
    normalStreamDesc->appInfo_.appUid = TEST_AUDIO_SELECT_UID;
    normalStreamDesc->capturerInfo_.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    normalStreamDesc->preferredInputDevice.deviceType_ = DEVICE_TYPE_MIC;
    normalStreamDesc->preferredInputDevice.deviceRole_ = INPUT_DEVICE;
    normalStreamDesc->preferredInputDevice.networkId_ = LOCAL_NETWORK_ID;
    AudioSelectInterfaceService::GetInstance().SetPreferredInputDeviceIfValid(normalStreamDesc);
    auto selectedNormalDevice = routerInfra.GetStreamSelectDevice(TEST_AUDIO_SELECT_UID, normalSessionId);
    ASSERT_NE(selectedNormalDevice, nullptr);
    EXPECT_EQ(selectedNormalDevice->deviceType_, DEVICE_TYPE_MIC);
    routerInfra.UpdateStreamSelectDevice(TEST_AUDIO_SELECT_UID, normalSessionId, nullptr);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: GetModuleNameBySessionId_001
* @tc.desc  : Test GetModuleNameBySessionId - invalid session id return "".
*/
HWTEST_F(AudioCoreServiceUnitTest, GetModuleNameBySessionId_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest GetModuleNameBySessionId_001 start");
    uint32_t sessionID = 0; // sessionId
    auto result = GetServerPtr()->eventEntry_->GetModuleNameBySessionId(sessionID);
    EXPECT_EQ(result, "");
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: GetProcessDeviceInfoBySessionId_001
* @tc.desc  : Test GetProcessDeviceInfoBySessionId - Get process device info by sessionId.
*/
HWTEST_F(AudioCoreServiceUnitTest, GetProcessDeviceInfoBySessionId_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest GetProcessDeviceInfoBySessionId_001 start");
    uint32_t sessionID = 100001; // sessionId
    AudioDeviceDescriptor deviceDesc;
    AudioStreamInfo info;
    bool isUltraFast = false;
    auto result =
        GetServerPtr()->eventEntry_->GetProcessDeviceInfoBySessionId(sessionID, deviceDesc, info, isUltraFast);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: GenerateSessionId_001
* @tc.desc  : Test GenerateSessionId in general scenarios.
*/
HWTEST_F(AudioCoreServiceUnitTest, GenerateSessionId_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest GenerateSessionId_001 start");
    auto result = GetServerPtr()->eventEntry_->GenerateSessionId();
    EXPECT_NE(result, 0);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: SetAudioScene_001
* @tc.desc  : Test SetAudioScene - AUDIO_SCENE_PHONE_CALL.
*/
HWTEST_F(AudioCoreServiceUnitTest, SetAudioScene_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest SetAudioScene_001 start");
    auto result = GetServerPtr()->eventEntry_->SetAudioScene(AUDIO_SCENE_PHONE_CALL);
    EXPECT_EQ(result, SUCCESS);
}

constexpr int32_t TEST_DEVICE_ID = 1001;
/**
* @tc.name  : Test AudioCoreService.
* @tc.number: SetAudioScene_002
* @tc.desc  : Test SetAudioScene - AUDIO_SCENE_DEFAULT.
*/
HWTEST_F(AudioCoreServiceUnitTest, SetAudioScene_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest SetAudioScene_002 start");
    auto result = GetServerPtr()->eventEntry_->SetAudioScene(AUDIO_SCENE_DEFAULT);
    EXPECT_EQ(result, SUCCESS);
}

static AudioDeviceDescriptor& CurrentActiveDevice()
{
    auto descs = AudioRouterSelectStrategy::GetInstance().GetCurrentOutputDevice(SYSTEM_UID);
    auto desc = descs.empty() || !descs.back() ? make_shared<AudioDeviceDescriptor>() : descs.back();
    if (desc->deviceId_ == 0) {
        desc->deviceId_ = TEST_DEVICE_ID;
        AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
        AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID, {desc});
    }
    desc = AudioDeviceManager::GetAudioDeviceManager().FindConnectedDeviceById(desc->deviceId_);
    return *desc;
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: IsSameScene_001
* @tc.desc  : Test IsSameScene.
*/
HWTEST_F(AudioCoreServiceUnitTest, IsSameScene_001, TestSize.Level1)
{
    CurrentActiveDevice().deviceType_ =
        DeviceType::DEVICE_TYPE_REMOTE_CAST;
    GetServerPtr()->eventEntry_->SetAudioScene(AUDIO_SCENE_RINGING);
    int32_t result = GetServerPtr()->eventEntry_->SetAudioScene(AUDIO_SCENE_DEFAULT);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: IsSameScene_002
* @tc.desc  : Test IsSameScene.
*/
HWTEST_F(AudioCoreServiceUnitTest, IsSameScene_002, TestSize.Level1)
{
    CurrentActiveDevice().deviceType_ =
        DeviceType::DEVICE_TYPE_SPEAKER;
    CurrentActiveDevice().networkId_ =
        REMOTE_NETWORK_ID;
    GetServerPtr()->eventEntry_->SetAudioScene(AUDIO_SCENE_RINGING);
    int32_t result = GetServerPtr()->eventEntry_->SetAudioScene(AUDIO_SCENE_DEFAULT);
    EXPECT_EQ(result, SUCCESS);

    CurrentActiveDevice().networkId_ =
        LOCAL_NETWORK_ID;
    GetServerPtr()->eventEntry_->SetAudioScene(AUDIO_SCENE_RINGING);
    result = GetServerPtr()->eventEntry_->SetAudioScene(AUDIO_SCENE_DEFAULT);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: EventEntry_GetDevices_001
* @tc.desc  : Test GetDevices - Get output devices.
*/
HWTEST_F(AudioCoreServiceUnitTest, EventEntry_GetDevices_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest GetDevices_001 start");
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> result =
        GetServerPtr()->eventEntry_->GetDevices(OUTPUT_DEVICES_FLAG);
    EXPECT_GT(result.size(), 0);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: SetDeviceActive_001
* @tc.desc  : Test SetDeviceActive - DEVICE_TYPE_SPEAKER.
*/
HWTEST_F(AudioCoreServiceUnitTest, SetDeviceActive_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest SetDeviceActive_001 start");
    auto result = GetServerPtr()->eventEntry_->SetDeviceActive(DEVICE_TYPE_SPEAKER, true, 0);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: RegisterTracker_001
* @tc.desc  : Test RegisterTracker - Register renderer with invalid params.
*/
HWTEST_F(AudioCoreServiceUnitTest, RegisterTracker_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest RegisterTracker_001 start");
    AudioMode mode = AUDIO_MODE_PLAYBACK;
    AudioStreamChangeInfo streamChangeInfo = {};
    int32_t apiVersion = 1;
    auto result = GetServerPtr()->eventEntry_->RegisterTracker(mode, streamChangeInfo, nullptr, apiVersion);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: RegisterTracker_002
* @tc.desc  : Test RegisterTracker - Register capturer with invalid params.
*/
HWTEST_F(AudioCoreServiceUnitTest, RegisterTracker_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest RegisterTracker_002 start");
    AudioMode mode = AUDIO_MODE_RECORD;
    AudioStreamChangeInfo streamChangeInfo = {};
    int32_t apiVersion = 1;
    auto result = GetServerPtr()->eventEntry_->RegisterTracker(mode, streamChangeInfo, nullptr, apiVersion);
    EXPECT_NE(result, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: UpdateTracker_001
* @tc.desc  : Test UpdateTracker - CAPTURER_NEW.
*/
HWTEST_F(AudioCoreServiceUnitTest, UpdateTracker_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest UpdateTracker_001 start");
    AudioMode mode = AUDIO_MODE_RECORD;
    AudioStreamChangeInfo streamChangeInfo = {};
    streamChangeInfo.audioCapturerChangeInfo.capturerState = CAPTURER_NEW;
    auto result = GetServerPtr()->eventEntry_->UpdateTracker(mode, streamChangeInfo);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: UpdateTracker_002
* @tc.desc  : Test UpdateTracker - CAPTURER_RELEASED.
*/
HWTEST_F(AudioCoreServiceUnitTest, UpdateTracker_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest UpdateTracker_002 start");
    AudioMode mode = AUDIO_MODE_RECORD;
    AudioStreamChangeInfo streamChangeInfo = {};
    streamChangeInfo.audioCapturerChangeInfo.capturerState = CAPTURER_RELEASED;
    auto result = GetServerPtr()->eventEntry_->UpdateTracker(mode, streamChangeInfo);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: UpdateTracker_003
* @tc.desc  : Test UpdateTracker - RENDERER_NEW.
*/
HWTEST_F(AudioCoreServiceUnitTest, UpdateTracker_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest UpdateTracker_003 start");
    AudioMode mode = AUDIO_MODE_PLAYBACK;
    AudioStreamChangeInfo streamChangeInfo = {};
    streamChangeInfo.audioRendererChangeInfo.rendererState = RENDERER_NEW;
    auto result = GetServerPtr()->eventEntry_->UpdateTracker(mode, streamChangeInfo);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: UpdateTracker_004
* @tc.desc  : Test UpdateTracker - RENDERER_RELEASED.
*/
HWTEST_F(AudioCoreServiceUnitTest, UpdateTracker_004, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest UpdateTracker_004 start");
    AudioMode mode = AUDIO_MODE_PLAYBACK;
    AudioStreamChangeInfo streamChangeInfo = {};
    streamChangeInfo.audioRendererChangeInfo.rendererState = RENDERER_RELEASED;
    auto result = GetServerPtr()->eventEntry_->UpdateTracker(mode, streamChangeInfo);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: UpdateTracker_005
* @tc.desc  : Test UpdateTracker - RENDERER_PAUSED.
*/
HWTEST_F(AudioCoreServiceUnitTest, UpdateTracker_005, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest UpdateTracker_005 start");
    AudioMode mode = AUDIO_MODE_PLAYBACK;
    AudioStreamChangeInfo streamChangeInfo = {};
    streamChangeInfo.audioRendererChangeInfo.rendererInfo.streamUsage = STREAM_USAGE_RINGTONE;
    streamChangeInfo.audioRendererChangeInfo.rendererState = RENDERER_PAUSED;
    auto result = GetServerPtr()->eventEntry_->UpdateTracker(mode, streamChangeInfo);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: UpdateTracker_006
* @tc.desc  : Test UpdateTracker - RENDERER_PREPARED.
*/
HWTEST_F(AudioCoreServiceUnitTest, UpdateTracker_006, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest UpdateTracker_006 start");
    AudioMode mode = AUDIO_MODE_PLAYBACK;
    AudioStreamChangeInfo streamChangeInfo = {};
    streamChangeInfo.audioRendererChangeInfo.rendererState = RENDERER_PREPARED;
    auto result = GetServerPtr()->eventEntry_->UpdateTracker(mode, streamChangeInfo);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: UpdateTracker_007
* @tc.desc  : Test UpdateTracker - RENDERER_INVALID.
*/
HWTEST_F(AudioCoreServiceUnitTest, UpdateTracker_007, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest UpdateTracker_007 start");
    AudioMode mode = AUDIO_MODE_PLAYBACK;
    AudioStreamChangeInfo streamChangeInfo = {};
    streamChangeInfo.audioRendererChangeInfo.rendererState = RENDERER_INVALID;
    auto result = GetServerPtr()->eventEntry_->UpdateTracker(mode, streamChangeInfo);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: UpdateTracker_008
 * @tc.desc  : Test UpdateTracker - PAUSE/STOP/RELEASE, AUDIO_SCENE_PHONE_CALL.
 */
HWTEST_F(AudioCoreServiceUnitTest, UpdateTracker_008, TestSize.Level1)
{
    AudioMode mode = AUDIO_MODE_PLAYBACK;
    AudioStreamChangeInfo streamChangeInfo = {};
    streamChangeInfo.audioRendererChangeInfo.rendererState = RENDERER_STOPPED;
    GetServerPtr()->eventEntry_->SetAudioScene(AUDIO_SCENE_PHONE_CALL, 1000, 1000);
    auto result = GetServerPtr()->eventEntry_->UpdateTracker(mode, streamChangeInfo);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: UpdateTracker_009
 * @tc.desc  : Test UpdateTracker - PAUSE/STOP/RELEASE, AUDIO_SCENE_PHONE_CHAT.
 */
HWTEST_F(AudioCoreServiceUnitTest, UpdateTracker_009, TestSize.Level1)
{
    AudioMode mode = AUDIO_MODE_PLAYBACK;
    AudioStreamChangeInfo streamChangeInfo = {};
    streamChangeInfo.audioRendererChangeInfo.rendererState = RENDERER_STOPPED;
    GetServerPtr()->eventEntry_->SetAudioScene(AUDIO_SCENE_PHONE_CHAT, 1000, 1000);
    auto result = GetServerPtr()->eventEntry_->UpdateTracker(mode, streamChangeInfo);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: ConnectServiceAdapter_001
* @tc.desc  : Test ConnectServiceAdapter - will return success.
*/
HWTEST_F(AudioCoreServiceUnitTest, ConnectServiceAdapter_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest ConnectServiceAdapter_001 start");
    auto result = GetServerPtr()->eventEntry_->ConnectServiceAdapter();
    EXPECT_EQ(result, true);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: SelectOutputDevice_001
* @tc.desc  : Test SelectOutputDevice - will return success.
*/
HWTEST_F(AudioCoreServiceUnitTest, SelectOutputDevice_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest SelectOutputDevice_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    ASSERT_NE(nullptr, audioRendererFilter) << "audioRendererFilter is nullptr.";
    audioRendererFilter->uid = getuid();
    audioRendererFilter->rendererInfo.rendererFlags = STREAM_FLAG_FAST;
    audioRendererFilter->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, audioDeviceDescriptor) << "audioDeviceDescriptor is nullptr.";
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDeviceDescriptor->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptorVector;
    deviceDescriptorVector.push_back(audioDeviceDescriptor);

    int32_t result = GetServerPtr()->eventEntry_->SelectOutputDevice(
        audioRendererFilter, deviceDescriptorVector);
    EXPECT_NE(SUCCESS, result);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: SelectOutputDevice_002
* @tc.desc  : Test SelectOutputDevice - will return success.
*/
HWTEST_F(AudioCoreServiceUnitTest, SelectOutputDevice_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest SelectOutputDevice_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    ASSERT_NE(nullptr, audioRendererFilter) << "audioRendererFilter is nullptr.";
    audioRendererFilter->uid = getuid();
    audioRendererFilter->rendererInfo.rendererFlags = STREAM_FLAG_FAST;
    audioRendererFilter->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;

    auto &devMan = AudioDeviceManager::GetAudioDeviceManager();
    shared_ptr<AudioDeviceDescriptor> devDesc;
    for (auto &item : devMan.connectedDevices_) {
        if (item->deviceRole_ == OUTPUT_DEVICE) {
            devDesc = item;
            break;
        }
    }
    CHECK_AND_RETURN(devDesc);
    auto selectedDev = make_shared<AudioDeviceDescriptor>(devDesc);
    devDesc->exceptionFlag_ = true;
    GetServerPtr()->eventEntry_->SelectOutputDevice(audioRendererFilter, {selectedDev});
    EXPECT_EQ(devDesc->exceptionFlag_, false);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: SelectInputDevice_001
* @tc.desc  : Test SelectInputDevice - will return success.
*/
HWTEST_F(AudioCoreServiceUnitTest, SelectInputDevice_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest SelectInputDevice_001 start");
    sptr<AudioCapturerFilter> audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();
    audioCapturerFilter->uid = -1;
    vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptorVector;
    auto audioDeviceDescriptors = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::INPUT_DEVICES_FLAG);
    auto inputDevice =  audioDeviceDescriptors[0];
    inputDevice->deviceRole_ = DeviceRole::INPUT_DEVICE;
    inputDevice->networkId_ = LOCAL_NETWORK_ID;
    deviceDescriptorVector.push_back(inputDevice);
    auto ret = GetServerPtr()->eventEntry_->SelectInputDevice(audioCapturerFilter, deviceDescriptorVector);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: SelectInputDevice_002
* @tc.desc  : Test SelectInputDevice - will return success.
*/
HWTEST_F(AudioCoreServiceUnitTest, SelectInputDevice_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest SelectInputDevice_002 start");
    sptr<AudioCapturerFilter> audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();
    vector<std::shared_ptr<AudioDeviceDescriptor>> devs;
    auto inputDevs = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::INPUT_DEVICES_FLAG);
    auto inputDevice =  inputDevs[0];
    inputDevice->deviceRole_ = DeviceRole::INPUT_DEVICE;
    inputDevice->networkId_ = LOCAL_NETWORK_ID;
    devs.push_back(inputDevice);

    constexpr int32_t BLUETOOTH_UID = 1002;
    audioCapturerFilter->uid = BLUETOOTH_UID;
    audioCapturerFilter->capturerInfo.sourceType == SOURCE_TYPE_VOICE_RECOGNITION;
    audioCapturerFilter->capturerInfo.capturerFlags == 0;
    AudioSceneManager::GetInstance().SetAudioScenePre(AUDIO_SCENE_DEFAULT);
    auto ret = AudioRecoveryDevice::GetInstance().SelectInputDevice(audioCapturerFilter, devs);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: NotifyRemoteRenderState_001
* @tc.desc  : Test NotifyRemoteRenderState - will return success.
*/
HWTEST_F(AudioCoreServiceUnitTest, NotifyRemoteRenderState_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest NotifyRemoteRenderState_001 start");
    std::string networkId = "LocalDevice";
    std::string condition = "";
    std::string value = "";
    GetServerPtr()->eventEntry_->NotifyRemoteRenderState(networkId, condition, value);
    EXPECT_NE(GetServerPtr(), nullptr);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: OnCapturerSessionAdded_001
* @tc.desc  : Test OnCapturerSessionAdded - will return success.
*/
HWTEST_F(AudioCoreServiceUnitTest, OnCapturerSessionAdded_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest OnCapturerSessionAdded_001 start");
    uint64_t sessionID = 100001; // sessionId for test
    SessionInfo sessionInfo = {SOURCE_TYPE_MIC, 48000, 2};
    AudioStreamInfo streamInfo;

    auto result = GetServerPtr()->eventEntry_->OnCapturerSessionAdded(sessionID, sessionInfo, streamInfo);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: OnCapturerSessionRemoved_001
* @tc.desc  : Test OnCapturerSessionRemoved - will return success.
*/
HWTEST_F(AudioCoreServiceUnitTest, OnCapturerSessionRemoved_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest OnCapturerSessionRemoved_001 start");
    uint64_t sessionID = 100001; // sessionId for test
    GetServerPtr()->eventEntry_->OnCapturerSessionRemoved(sessionID);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: SetDisplayName_001
* @tc.desc  : Test SetDisplayName - will return success.
*/
HWTEST_F(AudioCoreServiceUnitTest, SetDisplayName_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest SetDisplayName_001 start");
    ASSERT_NE(nullptr, GetServerPtr());

    // clear data
    GetServerPtr()->coreService_->audioConnectedDevice_.connectedDevices_.clear();

    // dummy data
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, audioDeviceDescriptor) << "audioDeviceDescriptor is nullptr.";
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    audioDeviceDescriptor->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDeviceDescriptor->displayName_ = "deviceA";
    audioDeviceDescriptor->networkId_ = LOCAL_NETWORK_ID;
    GetServerPtr()->coreService_->audioConnectedDevice_.connectedDevices_.push_back(audioDeviceDescriptor);

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor2 = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, audioDeviceDescriptor2) << "audioDeviceDescriptor is nullptr.";
    audioDeviceDescriptor2->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDeviceDescriptor2->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDeviceDescriptor2->displayName_ = "deviceB";
    audioDeviceDescriptor2->networkId_ = REMOTE_NETWORK_ID;
    GetServerPtr()->coreService_->audioConnectedDevice_.connectedDevices_.push_back(audioDeviceDescriptor2);

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor3 = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, audioDeviceDescriptor3) << "audioDeviceDescriptor is nullptr.";
    audioDeviceDescriptor3->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDeviceDescriptor3->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDeviceDescriptor3->displayName_ = "deviceC";
    audioDeviceDescriptor3->networkId_ = std::string(REMOTE_NETWORK_ID) + "xx";
    GetServerPtr()->coreService_->audioConnectedDevice_.connectedDevices_.push_back(audioDeviceDescriptor3);

    bool isLocalDevice = true;
    GetServerPtr()->coreService_->audioConnectedDevice_.SetDisplayName("deviceX", isLocalDevice);
    isLocalDevice = false;
    GetServerPtr()->coreService_->audioConnectedDevice_.SetDisplayName("deviceY", isLocalDevice);
    GetServerPtr()->coreService_->audioConnectedDevice_.SetDisplayName("deviceZ", isLocalDevice);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: TriggerFetchDevice_001
* @tc.desc  : Test TriggerFetchDevice - will return error because not init coreService.
*/
HWTEST_F(AudioCoreServiceUnitTest, TriggerFetchDevice_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest TriggerFetchDevice_001 start");
    int32_t systemAbilityId = 3009;
    bool runOnCreate = false;
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(server, nullptr);
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE;
    auto ret = server->TriggerFetchDevice(reason);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test AudioCoreServiceUnit
 * @tc.number: ExcludeOutputDevices_001
 * @tc.desc  : Test ExcludeOutputDevices interfaces - MEDIA_OUTPUT_DEVICES will return success.
 */
HWTEST_F(AudioCoreServiceUnitTest, ExcludeOutputDevices_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest ExcludeOutputDevices_001 start");
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);

    AudioDeviceUsage audioDevUsage = MEDIA_OUTPUT_DEVICES;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    std::shared_ptr<AudioDeviceDescriptor> audioDevDesc = std::make_shared<AudioDeviceDescriptor>();
    audioDevDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDevDesc->networkId_ = LOCAL_NETWORK_ID;
    audioDevDesc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDevDesc->macAddress_ = "00:00:00:00:00:00";
    audioDeviceDescriptors.push_back(audioDevDesc);

    int32_t ret = server->eventEntry_->ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test AudioCoreServiceUnit
 * @tc.number: ExcludeOutputDevices_002
 * @tc.desc  : Test ExcludeOutputDevices interfaces - CALL_OUTPUT_DEVICES wil return success.
 */
HWTEST_F(AudioCoreServiceUnitTest, ExcludeOutputDevices_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest ExcludeOutputDevices_002 start");
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);

    AudioDeviceUsage audioDevUsage = CALL_OUTPUT_DEVICES;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    std::shared_ptr<AudioDeviceDescriptor> audioDevDesc = std::make_shared<AudioDeviceDescriptor>();
    audioDevDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    audioDevDesc->networkId_ = LOCAL_NETWORK_ID;
    audioDevDesc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDevDesc->macAddress_ = "00:00:00:00:00:00";
    audioDeviceDescriptors.push_back(audioDevDesc);

    int32_t ret = server->eventEntry_->ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
    EXPECT_EQ(SUCCESS, ret);
}


/**
 * @tc.name   : Test AudioCoreServiceUnit
 * @tc.number : UnexcludeOutputDevicesTest_001
 * @tc.desc   : Test UnexcludeOutputDevices interface, when audioDeviceDescriptors is valid.
 */
HWTEST_F(AudioCoreServiceUnitTest, UnexcludeOutputDevicesTest_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest UnexcludeOutputDevicesTest_001 start");
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);
    AudioDeviceUsage audioDevUsage = MEDIA_OUTPUT_DEVICES;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    std::shared_ptr<AudioDeviceDescriptor> audioDevDesc = std::make_shared<AudioDeviceDescriptor>();
    audioDevDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDevDesc->networkId_ = LOCAL_NETWORK_ID;
    audioDevDesc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDevDesc->macAddress_ = "00:00:00:00:00:00";
    audioDeviceDescriptors.push_back(audioDevDesc);
    int32_t ret = server->eventEntry_->ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
    EXPECT_EQ(SUCCESS, ret);
    ret = server->eventEntry_->UnexcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test AudioCoreServiceUnit
 * @tc.number : UnexcludeOutputDevicesTest_002
 * @tc.desc   : Test UnexcludeOutputDevices interface, when audioDeviceDescriptors is empty.
 */
HWTEST_F(AudioCoreServiceUnitTest, UnexcludeOutputDevicesTest_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest UnexcludeOutputDevicesTest_002 start");
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);
    AudioDeviceUsage audioDevUsage = CALL_OUTPUT_DEVICES;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    std::shared_ptr<AudioDeviceDescriptor> audioDevDesc = std::make_shared<AudioDeviceDescriptor>();
    audioDevDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    audioDevDesc->networkId_ = LOCAL_NETWORK_ID;
    audioDevDesc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDevDesc->macAddress_ = "00:00:00:00:00:00";
    audioDeviceDescriptors.push_back(audioDevDesc);
    int32_t ret = server->eventEntry_->ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
    EXPECT_EQ(SUCCESS, ret);
    ret = server->eventEntry_->UnexcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreServiceUnit
* @tc.number: GetDevices_001
* @tc.desc  : Test AudioCoreService interfaces - Get all device flag.
*/
HWTEST_F(AudioCoreServiceUnitTest, GetDevices_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest GetDevices_001 start");
    ASSERT_NE(nullptr, GetServerPtr());

    // case nullptr
    DeviceFlag deviceFlag = OUTPUT_DEVICES_FLAG;
    std::shared_ptr<AudioDeviceDescriptor> ptr = nullptr;
    GetServerPtr()->coreService_->audioConnectedDevice_.connectedDevices_.push_back(ptr);
    GetServerPtr()->eventEntry_->GetDevices(deviceFlag);

    // case deviceType_ is DEVICE_TYPE_REMOTE_CAST
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, audioDeviceDescriptor) << "audioDeviceDescriptor is nullptr.";
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    std::vector<DeviceFlag> deviceFlagsTmp = {ALL_DEVICES_FLAG, OUTPUT_DEVICES_FLAG, INPUT_DEVICES_FLAG,
        ALL_DISTRIBUTED_DEVICES_FLAG, DISTRIBUTED_OUTPUT_DEVICES_FLAG, DISTRIBUTED_INPUT_DEVICES_FLAG};
    for (const auto& deviceFlag : deviceFlagsTmp) {
        std::vector<DeviceRole> deviceRolesTmp = {OUTPUT_DEVICE, INPUT_DEVICE};
        for (const auto& deviceRole : deviceRolesTmp) {
            audioDeviceDescriptor->deviceRole_ = deviceRole;
            audioDeviceDescriptor->networkId_ = LOCAL_NETWORK_ID;
            GetServerPtr()->coreService_->audioConnectedDevice_.connectedDevices_.push_back(
                audioDeviceDescriptor);
            GetServerPtr()->eventEntry_->GetDevices(deviceFlag);
            audioDeviceDescriptor->networkId_ = REMOTE_NETWORK_ID;
            GetServerPtr()->coreService_->audioConnectedDevice_.connectedDevices_.push_back(
                audioDeviceDescriptor);
            GetServerPtr()->eventEntry_->GetDevices(deviceFlag);
        }
    }

    // case deviceType_ is not DEVICE_TYPE_REMOTE_CAST
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    for (const auto& deviceFlag : deviceFlagsTmp) {
        std::vector<DeviceRole> deviceRolesTmp = {OUTPUT_DEVICE, INPUT_DEVICE};
        for (const auto& deviceRole : deviceRolesTmp) {
            audioDeviceDescriptor->deviceRole_ = deviceRole;
            audioDeviceDescriptor->networkId_ = LOCAL_NETWORK_ID;
            GetServerPtr()->coreService_->audioConnectedDevice_.connectedDevices_.push_back(
                audioDeviceDescriptor);
            GetServerPtr()->eventEntry_->GetDevices(deviceFlag);
            audioDeviceDescriptor->networkId_ = REMOTE_NETWORK_ID;
            GetServerPtr()->coreService_->audioConnectedDevice_.connectedDevices_.push_back(
                audioDeviceDescriptor);
            GetServerPtr()->eventEntry_->GetDevices(deviceFlag);
        }
    }
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: GetPreferredOutputDeviceDescriptors_001
* @tc.desc  : Test GetPreferredOutputDeviceDesc interface - should not throw errors.
*/
HWTEST_F(AudioCoreServiceUnitTest, GetPreferredOutputDeviceDescriptors_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);

    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest GetPreferredOutputDeviceDescriptors_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    EXPECT_NO_THROW(
        AudioRendererInfo rendererInfo;
        rendererInfo.streamUsage = STREAM_USAGE_INVALID;
        string networkId = REMOTE_NETWORK_ID;
        GetServerPtr()->eventEntry_->GetPreferredOutputDeviceDescriptors(rendererInfo, -1, networkId);

        rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
        GetServerPtr()->eventEntry_->GetPreferredOutputDeviceDescriptors(rendererInfo, -1, networkId);
    );
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: GetPreferredInputDeviceDescriptors_001
* @tc.desc  : Test GetPreferredInputDeviceDescriptors interface - should not throw errors.
*/
HWTEST_F(AudioCoreServiceUnitTest, GetPreferredInputDeviceDescriptors_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);

    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest GetPreferredInputDeviceDescriptors_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    EXPECT_NO_THROW(
        AudioCapturerInfo capturerInfo;
        capturerInfo.sourceType = SOURCE_TYPE_INVALID;
        string networkId = REMOTE_NETWORK_ID;
        GetServerPtr()->eventEntry_->GetPreferredInputDeviceDescriptors(capturerInfo, INVALID_UID, networkId);

        capturerInfo.sourceType = SOURCE_TYPE_MIC;
        GetServerPtr()->eventEntry_->GetPreferredInputDeviceDescriptors(capturerInfo, INVALID_UID, networkId);
    );
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: GetActiveBluetoothDevice_001
* @tc.desc  : Test GetActiveBluetoothDevice - return none when no a2dp device.
*/
HWTEST_F(AudioCoreServiceUnitTest, GetActiveBluetoothDevice_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    GetServerPtr()->coreService_->audioConnectedDevice_.connectedDevices_.clear();
    std::shared_ptr<AudioDeviceDescriptor> desc = GetServerPtr()->eventEntry_->GetActiveBluetoothDevice();
    EXPECT_EQ(desc->deviceType_, DEVICE_TYPE_NONE);
}

/**
 * @tc.name   : Test AudioCoreServiceUnit
 * @tc.number : GetAvailableMicrophones_001
 * @tc.desc   : Test GetAvailableMicrophones interface.
 */
HWTEST_F(AudioCoreServiceUnitTest, GetAvailableMicrophones_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    auto inputDeviceDescriptors = GetServerPtr()->coreService_->GetDevices(DeviceFlag::INPUT_DEVICES_FLAG);
    if (inputDeviceDescriptors.size() == 0) {
        return;
    }
    auto microphoneDescriptors = GetServerPtr()->coreService_->GetAvailableMicrophones();
    EXPECT_GT(microphoneDescriptors.size(), 0);
    for (auto inputDescriptor : inputDeviceDescriptors) {
        for (auto micDescriptor : microphoneDescriptors) {
            if (micDescriptor->deviceType_ == inputDescriptor->deviceType_) {
            }
        }
    }
}

/**
 * @tc.name   : Test AudioCoreServiceUnit
 * @tc.number : IsStreamSupportMultiChannel_001
 * @tc.desc   : Test IsStreamSupportMultiChannel interface - device type is not speaker/a2dp_offload, return false.
 */
HWTEST_F(AudioCoreServiceUnitTest, IsStreamSupportMultiChannel_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    deviceDesc->a2dpOffloadFlag_ = A2DP_NOT_OFFLOAD;
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    EXPECT_EQ(GetServerPtr()->coreService_->IsStreamSupportMultiChannel(streamDesc), false);
}

/**
 * @tc.name   : Test AudioCoreServiceUnit
 * @tc.number : IsStreamSupportMultiChannel_002
 * @tc.desc   : Test IsStreamSupportMultiChannel interface - channel count <= 2, return false.
 */
HWTEST_F(AudioCoreServiceUnitTest, IsStreamSupportMultiChannel_002, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    streamDesc->streamInfo_.channels = STEREO;
    EXPECT_EQ(GetServerPtr()->coreService_->IsStreamSupportMultiChannel(streamDesc), false);
}

/**
 * @tc.name   : Test AudioCoreServiceUnit
 * @tc.number : IsStreamSupportMultiChannel_003
 * @tc.desc   : Test IsStreamSupportMultiChannel interface
 */
HWTEST_F(AudioCoreServiceUnitTest, IsStreamSupportMultiChannel_003, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->newDeviceDescs_.push_back(std::make_shared<AudioDeviceDescriptor>());
    streamDesc->newDeviceDescs_.front()->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDeviceDescs_.front()->deviceRole_ = INPUT_DEVICE;
    streamDesc->newDeviceDescs_.front()->networkId_ = "LocalDevice";
    streamDesc->streamInfo_.format = AudioSampleFormat::SAMPLE_S16LE;
    streamDesc->streamInfo_.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    streamDesc->streamInfo_.encoding = ENCODING_AUDIOVIVID;
    streamDesc->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    EXPECT_EQ(GetServerPtr()->coreService_->IsStreamSupportMultiChannel(streamDesc), false);

    std::shared_ptr<AdapterDeviceInfo> deviceInfo = std::make_shared<AdapterDeviceInfo>();
    std::shared_ptr<AdapterPipeInfo> pipeInfo = std::make_shared<AdapterPipeInfo>();
    deviceInfo->supportPipeMap_.insert({AUDIO_OUTPUT_FLAG_MULTICHANNEL, pipeInfo});
    std::set<std::shared_ptr<AdapterDeviceInfo>> adapterDeviceInfoSet = {deviceInfo};
    auto deviceKey = std::make_pair<DeviceType, DeviceRole>(DEVICE_TYPE_SPEAKER, INPUT_DEVICE);
    AudioCoreConfigManager::GetInstance().GetAudioPolicyConfigData()
        .deviceInfoMap.insert({deviceKey, adapterDeviceInfoSet});
    EXPECT_EQ(GetServerPtr()->coreService_->IsStreamSupportMultiChannel(streamDesc), true);
}

/**
 * @tc.name: IsForcedNormal_001
 * @tc.number: IsForcedNormal_001
 * @tc.desc: Test IsForcedNormal interface - conditions that should return true and set audioFlag to NORMAL.
 */
HWTEST_F(AudioCoreServiceUnitTest, IsForcedNormal_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();

    streamDesc->rendererInfo_.originalFlag = AUDIO_FLAG_FORCED_NORMAL;
    bool result = GetServerPtr()->coreService_->IsForcedNormal(streamDesc);
    EXPECT_EQ(result, true);
    EXPECT_EQ(streamDesc->audioFlag_, AUDIO_OUTPUT_FLAG_NORMAL);

    streamDesc->rendererInfo_.originalFlag = AUDIO_FLAG_NORMAL;
    streamDesc->rendererInfo_.rendererFlags = AUDIO_FLAG_FORCED_NORMAL;
    result = GetServerPtr()->coreService_->IsForcedNormal(streamDesc);
    EXPECT_EQ(result, true);
    EXPECT_EQ(streamDesc->audioFlag_, AUDIO_OUTPUT_FLAG_NORMAL);
}

/**
 * @tc.name: IsForcedNormal_002
 * @tc.number: IsForcedNormal_002
 * @tc.desc: Test IsForcedNormal interface - conditions that should return false.
 */
HWTEST_F(AudioCoreServiceUnitTest, IsForcedNormal_002, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();

    streamDesc->rendererInfo_.originalFlag = AUDIO_FLAG_NONE;
    streamDesc->rendererInfo_.rendererFlags = AUDIO_FLAG_NONE;
    bool result = GetServerPtr()->coreService_->IsForcedNormal(streamDesc);
    EXPECT_EQ(result, false);
    EXPECT_EQ(streamDesc->audioFlag_, AUDIO_FLAG_NONE);

    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_VIDEO_COMMUNICATION;
    result = GetServerPtr()->coreService_->IsForcedNormal(streamDesc);
    EXPECT_EQ(result, false);
    EXPECT_EQ(streamDesc->audioFlag_, AUDIO_FLAG_NONE);

    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MEDIA;
    streamDesc->rendererInfo_.originalFlag = AUDIO_FLAG_NORMAL;
    streamDesc->rendererInfo_.rendererFlags = AUDIO_FLAG_NONE;
    result = GetServerPtr()->coreService_->IsForcedNormal(streamDesc);
    EXPECT_EQ(result, false);
    EXPECT_EQ(streamDesc->audioFlag_, AUDIO_FLAG_NONE);
}

/**
 * @tc.name   : Test AudioCoreServiceUnit
 * @tc.number : UpdatePlaybackStreamFlag_001
 * @tc.desc   : Test UpdatePlaybackStreamFlag interface - when streamDesc is null.
 */
HWTEST_F(AudioCoreServiceUnitTest, UpdatePlaybackStreamFlag_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    std::shared_ptr<AudioStreamDescriptor> streamDesc = nullptr;
    bool isCreateProcess = true;

    GetServerPtr()->coreService_->UpdatePlaybackStreamFlag(streamDesc, isCreateProcess);
    // Should return early without crash
    SUCCEED();
}

/**
 * @tc.name   : Test AudioCoreServiceUnit
 * @tc.number : UpdatePlaybackStreamFlag_002
 * @tc.desc   : Test UpdatePlaybackStreamFlag interface - when isCreateProcess and forceToNormal is true.
 */
HWTEST_F(AudioCoreServiceUnitTest, UpdatePlaybackStreamFlag_002, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.forceToNormal = true;
    streamDesc->rendererInfo_.originalFlag = AUDIO_FLAG_MMAP;
    streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_FAST;

    bool isCreateProcess = true;
    GetServerPtr()->coreService_->UpdatePlaybackStreamFlag(streamDesc, isCreateProcess);
    EXPECT_EQ(streamDesc->audioFlag_, AUDIO_OUTPUT_FLAG_NORMAL);
}

/**
 * @tc.name   : Test AudioCoreServiceUnit
 * @tc.number : UpdatePlaybackStreamFlag_003
 * @tc.desc   : Test UpdatePlaybackStreamFlag interface - when IsHWDecoding returns true.
 */
HWTEST_F(AudioCoreServiceUnitTest, UpdatePlaybackStreamFlag_003, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.forceToNormal = false;
    streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    streamDesc->streamInfo_.encoding = ENCODING_EAC3;

    bool isCreateProcess = true;
    GetServerPtr()->coreService_->UpdatePlaybackStreamFlag(streamDesc, isCreateProcess);
    // Should return early with HWDecoding check
    EXPECT_EQ(streamDesc->audioFlag_, AUDIO_OUTPUT_FLAG_HWDECODING);
}

/**
 * @tc.name   : Test AudioCoreServiceUnit
 * @tc.number : UpdatePlaybackStreamFlag_004
 * @tc.desc   : Test UpdatePlaybackStreamFlag interface - when IsForcedNormal returns true.
 */
HWTEST_F(AudioCoreServiceUnitTest, UpdatePlaybackStreamFlag_004, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.forceToNormal = true;
    streamDesc->rendererInfo_.rendererFlags = AUDIO_FLAG_FORCED_NORMAL;
    streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_FAST;

    bool isCreateProcess = true;
    GetServerPtr()->coreService_->UpdatePlaybackStreamFlag(streamDesc, isCreateProcess);
    // Should return early with forced normal check
    EXPECT_EQ(streamDesc->audioFlag_, AUDIO_OUTPUT_FLAG_NORMAL);
}

/**
 * @tc.name   : Test AudioCoreServiceUnit
 * @tc.number : UpdatePlaybackStreamFlag_005
 * @tc.desc   : Test UpdatePlaybackStreamFlag interface - when CheckStaticModeAndSelectFlag returns true.
 */
HWTEST_F(AudioCoreServiceUnitTest, UpdatePlaybackStreamFlag_005, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.forceToNormal = false;
    streamDesc->rendererInfo_.rendererFlags = AUDIO_OUTPUT_FLAG_NORMAL;
    streamDesc->audioFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    streamDesc->rendererInfo_.isStatic = true;

    // Add device description to avoid crash
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    deviceDesc->networkId_ = LOCAL_NETWORK_ID;
    streamDesc->newDeviceDescs_.push_back(deviceDesc);

    bool isCreateProcess = true;
    GetServerPtr()->coreService_->UpdatePlaybackStreamFlag(streamDesc, isCreateProcess);
    // Should return early with static mode check
    EXPECT_EQ(streamDesc->audioFlag_, AUDIO_OUTPUT_FLAG_NORMAL);
}

/**
 * @tc.name   : Test AudioCoreServiceUnit
 * @tc.number : UpdatePlaybackStreamFlag_006
 * @tc.desc   : Test UpdatePlaybackStreamFlag interface - when stream usage is voice communication.
 */
HWTEST_F(AudioCoreServiceUnitTest, UpdatePlaybackStreamFlag_006, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.forceToNormal = false;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    streamDesc->rendererInfo_.originalFlag = AUDIO_FLAG_NORMAL;

    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    deviceDesc->networkId_ = LOCAL_NETWORK_ID;
    streamDesc->newDeviceDescs_.push_back(deviceDesc);

    bool isCreateProcess = true;
    GetServerPtr()->coreService_->UpdatePlaybackStreamFlag(streamDesc, isCreateProcess);
    EXPECT_EQ(streamDesc->audioFlag_, AUDIO_OUTPUT_FLAG_VOIP);
}

/**
 * @tc.name   : Test AudioCoreServiceUnit
 * @tc.number : UpdatePlaybackStreamFlag_007
 * @tc.desc   : Test UpdatePlaybackStreamFlag interface - when stream usage is video communication.
 */
HWTEST_F(AudioCoreServiceUnitTest, UpdatePlaybackStreamFlag_007, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.forceToNormal = false;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_VIDEO_COMMUNICATION;
    streamDesc->rendererInfo_.originalFlag = AUDIO_FLAG_NORMAL;

    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    deviceDesc->networkId_ = LOCAL_NETWORK_ID;
    streamDesc->newDeviceDescs_.push_back(deviceDesc);

    bool isCreateProcess = true;
    GetServerPtr()->coreService_->UpdatePlaybackStreamFlag(streamDesc, isCreateProcess);
    EXPECT_EQ(streamDesc->audioFlag_, AUDIO_OUTPUT_FLAG_VOIP);
}

/**
 * @tc.name   : Test AudioCoreServiceUnit
 * @tc.number : UpdatePlaybackStreamFlag_008
 * @tc.desc   : Test UpdatePlaybackStreamFlag interface - when original flag is AUDIO_FLAG_MMAP.
 */
HWTEST_F(AudioCoreServiceUnitTest, UpdatePlaybackStreamFlag_008, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.forceToNormal = false;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MEDIA;
    streamDesc->rendererInfo_.originalFlag = AUDIO_FLAG_MMAP;

    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    deviceDesc->networkId_ = LOCAL_NETWORK_ID;
    streamDesc->newDeviceDescs_.push_back(deviceDesc);

    bool isCreateProcess = true;
    GetServerPtr()->coreService_->UpdatePlaybackStreamFlag(streamDesc, isCreateProcess);
    EXPECT_EQ(streamDesc->audioFlag_, AUDIO_OUTPUT_FLAG_FAST);
}

/**
 * @tc.name   : Test AudioCoreServiceUnit
 * @tc.number : UpdatePlaybackStreamFlag_009
 * @tc.desc   : Test UpdatePlaybackStreamFlag interface - when original flag is AUDIO_FLAG_VOIP_DIRECT.
 */
HWTEST_F(AudioCoreServiceUnitTest, UpdatePlaybackStreamFlag_009, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.forceToNormal = false;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MEDIA;
    streamDesc->rendererInfo_.originalFlag = AUDIO_FLAG_VOIP_DIRECT;

    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    deviceDesc->networkId_ = LOCAL_NETWORK_ID;
    streamDesc->newDeviceDescs_.push_back(deviceDesc);

    bool isCreateProcess = true;
    GetServerPtr()->coreService_->UpdatePlaybackStreamFlag(streamDesc, isCreateProcess);
    EXPECT_EQ(streamDesc->audioFlag_, AUDIO_OUTPUT_FLAG_VOIP);
}

/**
 * @tc.name   : Test AudioCoreServiceUnit
 * @tc.number : UpdatePlaybackStreamFlag_010
 * @tc.desc   : Test UpdatePlaybackStreamFlag interface - when original flag is AUDIO_FLAG_ULTRA_FAST and not supported.
 */
HWTEST_F(AudioCoreServiceUnitTest, UpdatePlaybackStreamFlag_010, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.forceToNormal = false;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MEDIA;
    streamDesc->rendererInfo_.originalFlag = AUDIO_FLAG_ULTRA_FAST;

    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    deviceDesc->networkId_ = LOCAL_NETWORK_ID;
    streamDesc->newDeviceDescs_.push_back(deviceDesc);

    bool isCreateProcess = true;
    GetServerPtr()->coreService_->UpdatePlaybackStreamFlag(streamDesc, isCreateProcess);
    EXPECT_EQ(streamDesc->IsUltraFastImplemented(), false);
}

/**
 * @tc.name   : Test AudioCoreServiceUnit
 * @tc.number : UpdatePlaybackStreamFlag_011
 * @tc.desc   : Test UpdatePlaybackStreamFlag interface - with empty newDeviceDescs_.
 */
HWTEST_F(AudioCoreServiceUnitTest, UpdatePlaybackStreamFlag_011, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.forceToNormal = false;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MEDIA;
    streamDesc->rendererInfo_.originalFlag = AUDIO_FLAG_MMAP;
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    streamDesc->newDeviceDescs_.push_back(deviceDesc);

    // streamDesc->newDeviceDescs_ is empty
    bool isCreateProcess = true;
    GetServerPtr()->coreService_->UpdatePlaybackStreamFlag(streamDesc, isCreateProcess);
    // Should handle empty vector gracefully
    SUCCEED();
}

/**
 * @tc.name   : Test AudioCoreServiceUnit
 * @tc.number : UpdatePlaybackStreamFlag_012
 * @tc.desc   : Test UpdatePlaybackStreamFlag interface - when streamDesc is null, return flag normal.
 */
HWTEST_F(AudioCoreServiceUnitTest, UpdatePlaybackStreamFlag_012, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.rendererFlags = AUDIO_FLAG_FORCED_NORMAL;
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    streamDesc->newDeviceDescs_.push_back(deviceDesc);

    bool isCreateProcess = true;
    GetServerPtr()->coreService_->UpdatePlaybackStreamFlag(streamDesc, isCreateProcess);
    EXPECT_EQ(streamDesc->audioFlag_, AUDIO_OUTPUT_FLAG_NORMAL);

    streamDesc->rendererInfo_.forceToNormal = true;
    GetServerPtr()->coreService_->UpdatePlaybackStreamFlag(streamDesc, isCreateProcess);
    EXPECT_EQ(streamDesc->audioFlag_, AUDIO_OUTPUT_FLAG_NORMAL);

    isCreateProcess = false;
    GetServerPtr()->coreService_->UpdatePlaybackStreamFlag(streamDesc, isCreateProcess);
    EXPECT_EQ(streamDesc->audioFlag_, AUDIO_OUTPUT_FLAG_NORMAL);

    streamDesc->rendererInfo_.forceToNormal = false;
    GetServerPtr()->coreService_->UpdatePlaybackStreamFlag(streamDesc, isCreateProcess);
    EXPECT_EQ(streamDesc->audioFlag_, AUDIO_OUTPUT_FLAG_NORMAL);
}

/**
 * @tc.name   : Test AudioCoreServiceUnit
 * @tc.number : SetFlagForSpecialStream_001
 * @tc.desc   : Test SetFlagForSpecialStream interface - when streamDesc is null, return flag normal.
 */
HWTEST_F(AudioCoreServiceUnitTest, SetFlagForSpecialStream_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    std::shared_ptr<AudioStreamDescriptor> streamDesc = nullptr;
    bool isCreateProcess = true;
    AudioFlag result = GetServerPtr()->coreService_->SetFlagForSpecialStream(streamDesc, isCreateProcess);
    EXPECT_EQ(result, AUDIO_OUTPUT_FLAG_NORMAL);
}

/**
* @tc.name  : Test AudioCoreServiceUnit
* @tc.number: AddAudioCapturerMicrophoneDescriptor_001
* @tc.desc  : Test AudioCoreService interfaces - mic desc should be added.
*/
HWTEST_F(AudioCoreServiceUnitTest, AddAudioCapturerMicrophoneDescriptor_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest AddAudioCapturerMicrophoneDescriptor_001 start");
    EXPECT_NE(nullptr, GetServerPtr());

    GetServerPtr()->eventEntry_->GetAudioCapturerMicrophoneDescriptors(TEST_SESSION_ID);
    // clear data
    GetServerPtr()->coreService_->audioMicrophoneDescriptor_.connectedMicrophones_.clear();

    // call when devType is DEVICE_TYPE_NONE
    GetServerPtr()->coreService_->audioMicrophoneDescriptor_.AddAudioCapturerMicrophoneDescriptor(
        TEST_SESSION_ID, DEVICE_TYPE_NONE);
    GetServerPtr()->eventEntry_->GetAudioCapturerMicrophoneDescriptors(TEST_SESSION_ID);

    // call when devType is DEVICE_TYPE_MIC and connectedMicrophones_ is empty
    GetServerPtr()->coreService_->audioMicrophoneDescriptor_.AddAudioCapturerMicrophoneDescriptor(
        TEST_SESSION_ID, DEVICE_TYPE_MIC);
    GetServerPtr()->eventEntry_->GetAudioCapturerMicrophoneDescriptors(TEST_SESSION_ID);

    // dummy data
    sptr<MicrophoneDescriptor> microphoneDescriptor = new(std::nothrow) MicrophoneDescriptor();
    ASSERT_NE(nullptr, microphoneDescriptor) << "microphoneDescriptor is nullptr.";
    microphoneDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    GetServerPtr()->coreService_->audioMicrophoneDescriptor_.connectedMicrophones_.push_back(
        microphoneDescriptor);

    // call when devType is DEVICE_TYPE_MIC but connectedMicrophones_ is DEVICE_TYPE_BLUETOOTH_A2DP
    GetServerPtr()->coreService_->audioMicrophoneDescriptor_.AddAudioCapturerMicrophoneDescriptor(
        TEST_SESSION_ID, DEVICE_TYPE_MIC);
    GetServerPtr()->eventEntry_->GetAudioCapturerMicrophoneDescriptors(TEST_SESSION_ID);

    // call when devType is DEVICE_TYPE_BLUETOOTH_A2DP and connectedMicrophones_ is also DEVICE_TYPE_BLUETOOTH_A2DP
    GetServerPtr()->coreService_->audioMicrophoneDescriptor_.AddAudioCapturerMicrophoneDescriptor(
        TEST_SESSION_ID, DEVICE_TYPE_BLUETOOTH_A2DP);
    std::vector<sptr<MicrophoneDescriptor>> micDescs =
        GetServerPtr()->eventEntry_->GetAudioCapturerMicrophoneDescriptors(TEST_SESSION_ID);
    EXPECT_GT(micDescs.size(), 0);
}

/**
* @tc.name  : Test AudioCoreServiceUnit
* @tc.number: GetCurrentRendererChangeInfos_001
* @tc.desc  : Test GetCurrentRendererChangeInfos interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, GetCurrentRendererChangeInfos_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, GetServerPtr());
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos =
        {std::make_shared<AudioRendererChangeInfo>()};
    bool hasBTPermission = true;
    bool hasSystemPermission = true;

    auto ret = GetServerPtr()->eventEntry_->GetCurrentRendererChangeInfos(audioRendererChangeInfos,
        hasBTPermission, hasSystemPermission);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreServiceUnit
 * @tc.number: GetCurrentCapturerChangeInfos_001
 * @tc.desc  : Test GetCurrentCapturerChangeInfos interface. Returns invalid.
 */
HWTEST_F(AudioCoreServiceUnitTest, GetCurrentCapturerChangeInfos_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, GetServerPtr());
    vector<shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    bool hasBTPermission = true;
    bool hasSystemPermission = true;
    auto ret = GetServerPtr()->eventEntry_->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos,
        hasBTPermission, hasSystemPermission);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name   : Test AudioCoreServiceUnit
 * @tc.number : GetExcludedDevicesTest_001
 * @tc.desc   : Test GetExcludedDevices interface - return 0 when no running stream.
 */
HWTEST_F(AudioCoreServiceUnitTest, GetExcludedDevicesTest_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest GetExcludedDevicesTest_001 start");
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);

    AudioDeviceUsage audioDevUsage = MEDIA_OUTPUT_DEVICES;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors =
        server->eventEntry_->GetExcludedDevices(audioDevUsage);
    EXPECT_EQ(audioDeviceDescriptors.size(), 0);
}

/**
 * @tc.name   : Test AudioCoreServiceUnit
 * @tc.number : GetExcludedDevicesTest_002
 * @tc.desc   : Test GetExcludedDevices interface - return 0 when no running stream.
 */
HWTEST_F(AudioCoreServiceUnitTest, GetExcludedDevicesTest_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest GetExcludedDevicesTest_002 start");
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);

    AudioDeviceUsage audioDevUsage = CALL_OUTPUT_DEVICES;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors =
        server->eventEntry_->GetExcludedDevices(audioDevUsage);
    EXPECT_EQ(audioDeviceDescriptors.size(), 0);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: EventEntry_GetVolumeGroupInfos_001
 * @tc.desc  : Test GetVolumeGroupInfos interface. Volume group info size will bigger than 0.
 */
HWTEST_F(AudioCoreServiceUnitTest, EventEntry_GetVolumeGroupInfos_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);
    std::vector<sptr<VolumeGroupInfo>> infos = server->eventEntry_->GetVolumeGroupInfos();
    EXPECT_GT(infos.size(), 0);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: NotifyDistributedOutputChange_001
 * @tc.desc  : Test NotifyDistributedOutputChange interface. Returns void.
 */
HWTEST_F(AudioCoreServiceUnitTest, NotifyDistributedOutputChange_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);
    AudioDeviceDescriptor deviceDesc;
    server->coreService_->NotifyDistributedOutputChange(deviceDesc);
    deviceDesc.deviceType_ = DEVICE_TYPE_SPEAKER;
    deviceDesc.deviceRole_ = OUTPUT_DEVICE;
    deviceDesc.networkId_ = "aaaaaaaa";
    server->coreService_->NotifyDistributedOutputChange(deviceDesc);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: GetDirectPlaybackSupport_001
 * @tc.desc  : Test GetDirectPlaybackSupport interfaces. Returns DIRECT_PLAYBACK_NOT_SUPPORTED when xml not supported.
 */
HWTEST_F(AudioCoreServiceUnitTest, GetDirectPlaybackSupport_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);

    AudioStreamInfo streamInfo;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.encoding = ENCODING_PCM;
    streamInfo.format = SAMPLE_S24LE;
    streamInfo.channels = STEREO;
    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    auto result = server->coreService_->GetDirectPlaybackSupport(streamInfo, streamUsage);
    EXPECT_EQ(result, DIRECT_PLAYBACK_NOT_SUPPORTED);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: GetDirectPlaybackSupport_002
 * @tc.desc  : Test GetDirectPlaybackSupport interfaces. Returns DIRECT_PLAYBACK_NOT_SUPPORTED when xml not supported.
 */
HWTEST_F(AudioCoreServiceUnitTest, GetDirectPlaybackSupport_002, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);

    AudioStreamInfo streamInfo;
    streamInfo.samplingRate = SAMPLE_RATE_24000;
    streamInfo.encoding = ENCODING_EAC3;
    streamInfo.format = SAMPLE_F32LE;
    streamInfo.channels = STEREO;
    StreamUsage streamUsage = STREAM_USAGE_MEDIA;
    auto result = server->coreService_->GetDirectPlaybackSupport(streamInfo, streamUsage);
    EXPECT_EQ(result, DIRECT_PLAYBACK_NOT_SUPPORTED);
}

/**
 * @tc.name  : RecordSelectDevice_001
 * @tc.number: RecordSelectDevice_001
 * @tc.desc  : Test RecordSelectDevice.
 */
HWTEST_F(AudioCoreServiceUnitTest, RecordSelectDevice_001, TestSize.Level1)
{
    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    audioCoreService->selectDeviceHistory_ = {};
    ASSERT_EQ(audioCoreService->selectDeviceHistory_.size(), 0);
    std::string history = "device1";
    audioCoreService->RecordSelectDevice(history);
    ASSERT_EQ(audioCoreService->selectDeviceHistory_.size(), 1);
    ASSERT_EQ(audioCoreService->selectDeviceHistory_.front(), history);
}

/**
 * @tc.name  : RecordSelectDevice_002
 * @tc.number: RecordSelectDevice_002
 * @tc.desc  : Test RecordSelectDevice.
 */
HWTEST_F(AudioCoreServiceUnitTest, RecordSelectDevice_002, TestSize.Level1)
{
    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    audioCoreService->selectDeviceHistory_ = {};
    std::string newhistory = "device2";
    size_t limit = 10; //SELECT_DEVICE_HISTORY_LIMIT
    while (audioCoreService->selectDeviceHistory_.size() < limit) {
        audioCoreService->RecordSelectDevice(newhistory);
    }
    ASSERT_EQ(audioCoreService->selectDeviceHistory_.size(), limit);
    ASSERT_EQ(audioCoreService->selectDeviceHistory_.front(), newhistory);
    audioCoreService->selectDeviceHistory_ = {};
}

/**
 * @tc.name  : RecordSelectDevice_003
 * @tc.number: RecordSelectDevice_003
 * @tc.desc  : Test RecordSelectDevice.
 */
HWTEST_F(AudioCoreServiceUnitTest, RecordSelectDevice_003, TestSize.Level1)
{
    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    audioCoreService->selectDeviceHistory_ = {};
    size_t limit = 10; //SELECT_DEVICE_HISTORY_LIMIT
    for (int i = 0; i < limit + 2; i++) {
        std::string history = "device" + std::to_string(i);
        audioCoreService->RecordSelectDevice(history);
    }
    ASSERT_EQ(audioCoreService->selectDeviceHistory_.back(), "device" + std::to_string(limit + 1));
}

/**
 * @tc.name  : DumpSelectHistory_001
 * @tc.number: DumpSelectHistory_001
 * @tc.desc  : Test DumpSelectHistory.
 */
HWTEST_F(AudioCoreServiceUnitTest, DumpSelectHistory_001, TestSize.Level1)
{
    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    audioCoreService->selectDeviceHistory_ = {};
    std::string dumpString;
    audioCoreService->DumpSelectHistory(dumpString);
    std::string expectedDump = "Select device history infos";
    EXPECT_TRUE(dumpString.find(expectedDump) != std::string::npos);
}

/**
 * @tc.name  : DumpSelectHistory_002
 * @tc.number: DumpSelectHistory_002
 * @tc.desc  : Test DumpSelectHistory.
 */
HWTEST_F(AudioCoreServiceUnitTest, DumpSelectHistory_002, TestSize.Level1)
{
    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    audioCoreService->selectDeviceHistory_.push_back("HistoryRecord1");
    audioCoreService->selectDeviceHistory_.push_back("HistoryRecord2");
    std::string dumpString;
    audioCoreService->DumpSelectHistory(dumpString);
    std::string expectedDump = "HistoryRecord2";
    EXPECT_TRUE(dumpString.find(expectedDump) != std::string::npos);
}

/**
* @tc.name  : Test CaptureConcurrentCheck.
* @tc.number: CaptureConcurrentCheck_001
* @tc.desc  : Test interface CaptureConcurrentCheck
*/
HWTEST_F(AudioCoreServiceUnitTest, CaptureConcurrentCheck_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest CaptureConcurrentCheck start");
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs = {
        std::make_shared<AudioStreamDescriptor>(),
        std::make_shared<AudioStreamDescriptor>()
    };
    uint32_t flag[2] = {AUDIO_INPUT_FLAG_NORMAL, AUDIO_INPUT_FLAG_FAST};
    uint32_t originalSessionId[2] = {0};
    for (int i = 0; i < 2; i++) {
        streamDescs[i]->streamInfo_.format = AudioSampleFormat::SAMPLE_S32LE;
        streamDescs[i]->streamInfo_.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
        streamDescs[i]->streamInfo_.channels = AudioChannel::STEREO;
        streamDescs[i]->streamInfo_.encoding = AudioEncodingType::ENCODING_PCM;
        streamDescs[i]->streamInfo_.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
        streamDescs[i]->rendererInfo_.streamUsage = STREAM_USAGE_MOVIE;

        streamDescs[i]->audioMode_ = AUDIO_MODE_RECORD;
        streamDescs[i]->createTimeStamp_ = ClockTime::GetCurNano();
        streamDescs[i]->stateStartTimeStamp_ = streamDescs[i]->createTimeStamp_ + 1;
        streamDescs[i]->callerUid_ = getuid();
        auto result = audioCoreService->CreateCapturerClient(streamDescs[i], flag[i], originalSessionId[i]);
        EXPECT_EQ(result, SUCCESS);
    }
    audioCoreService->CaptureConcurrentCheck(originalSessionId[1]);
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest CaptureConcurrentCheck end");
}

/**
* @tc.name  : Test AudioCoreService
* @tc.number: SetAudioScene_003
* @tc.desc  : Test scenario: switching from the AUDIO_SCENE_RINGING to another scene,
* with the app's STREAM_RING muted
*/
HWTEST_F(AudioCoreServiceUnitTest, SetAudioScene_003, TestSize.Level1)
{
    int32_t appUid = 123;
    int32_t sessionId = 10001;
    int32_t pid = 123;
    AudioStreamType streamType = STREAM_RING;
    StreamUsage streamUsage = STREAM_USAGE_RINGTONE;

    auto audioVolume = AudioVolume::GetInstance();
    ASSERT_NE(nullptr, audioVolume);
    audioVolume->streamVolume_.emplace(sessionId, std::make_shared<StreamVolume>(
        sessionId, streamType, streamUsage, appUid, pid, false, 1, false));

    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    ASSERT_NE(nullptr, audioCoreService);
    audioCoreService->audioVolumeManager_.SetAppRingMuted(appUid, true);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_RINGING;

    int32_t result = audioCoreService->SetAudioScene(AUDIO_SCENE_DEFAULT, appUid, pid);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(audioCoreService->audioVolumeManager_.IsAppRingMuted(appUid), false);
    audioCoreService->audioVolumeManager_.SetAppRingMuted(appUid, false);
    audioVolume->streamVolume_.clear();
}

/**
* @tc.name  : Test AudioCoreService
* @tc.number: SetAudioScene_004
* @tc.desc  : Test scenario: switching from the AUDIO_SCENE_RINGING to another scene,
* with the app's STREAM_RING not muted, another app's STREAM_RING muted
*/
HWTEST_F(AudioCoreServiceUnitTest, SetAudioScene_004, TestSize.Level1)
{
    int32_t appUid = 123;
    int32_t anotherAppUid = 456;
    int32_t sessionId = 10001;
    int32_t anotherSessionId = 10002;
    int32_t pid = 123;
    AudioStreamType streamType = STREAM_RING;
    StreamUsage streamUsage = STREAM_USAGE_RINGTONE;

    auto audioVolume = AudioVolume::GetInstance();
    ASSERT_NE(nullptr, audioVolume);
    audioVolume->streamVolume_.emplace(sessionId, std::make_shared<StreamVolume>(
        sessionId, streamType, streamUsage, appUid, pid, false, 1, false));
    audioVolume->streamVolume_.emplace(anotherSessionId, std::make_shared<StreamVolume>(
        anotherSessionId, streamType, streamUsage, anotherAppUid, pid, false, 1, false));

    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    ASSERT_NE(nullptr, audioCoreService);
    audioCoreService->audioVolumeManager_.SetAppRingMuted(appUid, true);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_RINGING;

    int32_t result = audioCoreService->SetAudioScene(AUDIO_SCENE_DEFAULT, appUid, pid);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(audioCoreService->audioVolumeManager_.IsAppRingMuted(appUid), false);
    audioCoreService->audioVolumeManager_.SetAppRingMuted(anotherAppUid, false);
    audioVolume->streamVolume_.clear();
}

/**
* @tc.name  : Test AudioCoreService
* @tc.number: SetAudioScene_005
* @tc.desc  : Test scenario: switching from the AUDIO_SCENE_RINGING to AUDIO_SCENE_RINGING scene
*/
HWTEST_F(AudioCoreServiceUnitTest, SetAudioScene_005, TestSize.Level1)
{
    int32_t appUid = 123;
    int32_t sessionId = 10001;
    int32_t pid = 123;
    AudioStreamType streamType = STREAM_RING;
    StreamUsage streamUsage = STREAM_USAGE_RINGTONE;

    auto audioVolume = AudioVolume::GetInstance();
    ASSERT_NE(nullptr, audioVolume);
    audioVolume->streamVolume_.emplace(sessionId, std::make_shared<StreamVolume>(
        sessionId, streamType, streamUsage, appUid, pid, false, 1, false));

    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    ASSERT_NE(nullptr, audioCoreService);
    audioCoreService->audioVolumeManager_.SetAppRingMuted(appUid, true);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_RINGING;

    int32_t result = audioCoreService->SetAudioScene(AUDIO_SCENE_RINGING, appUid, pid);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(audioCoreService->audioVolumeManager_.IsAppRingMuted(appUid), true);
    audioCoreService->audioVolumeManager_.SetAppRingMuted(appUid, false);
    audioVolume->streamVolume_.clear();
}

/**
* @tc.name  : Test AudioCoreService
* @tc.number: SetAudioScene_006
* @tc.desc  : Test scenario: switching from the AUDIO_SCENE_DEFAULT to AUDIO_SCENE_RINGING scene
*/
HWTEST_F(AudioCoreServiceUnitTest, SetAudioScene_006, TestSize.Level1)
{
    int32_t appUid = 123;
    int32_t sessionId = 10001;
    int32_t pid = 123;
    AudioStreamType streamType = STREAM_RING;
    StreamUsage streamUsage = STREAM_USAGE_RINGTONE;

    auto audioVolume = AudioVolume::GetInstance();
    ASSERT_NE(nullptr, audioVolume);
    audioVolume->streamVolume_.emplace(sessionId, std::make_shared<StreamVolume>(
        sessionId, streamType, streamUsage, appUid, pid, false, 1, false));

    std::shared_ptr<AudioCoreService> audioCoreService = AudioCoreService::GetCoreService();
    ASSERT_NE(nullptr, audioCoreService);
    audioCoreService->audioVolumeManager_.SetAppRingMuted(appUid, true);
    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_DEFAULT;

    int32_t result = audioCoreService->SetAudioScene(AUDIO_SCENE_RINGING, appUid, pid);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(audioCoreService->audioVolumeManager_.IsAppRingMuted(appUid), true);
    audioCoreService->audioVolumeManager_.SetAppRingMuted(appUid, false);
    audioVolume->streamVolume_.clear();
}

/**
* @tc.name  : Test AudioCoreService
* @tc.number: SetFlagForMmapStream_001
* @tc.desc  : Test GetFlagForMmapStream() when device type is DEVICE_TYPE_BLUETOOTH_A2DP
*/
HWTEST_F(AudioCoreServiceUnitTest, SetFlagForMmapStream_001, TestSize.Level4)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest CreateRenderClient_001 start");

    ASSERT_NE(nullptr, GetServerPtr());
    auto coreService_ = GetServerPtr()->coreService_;
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(nullptr, streamDesc);
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, deviceDesc);

    deviceDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    streamDesc->newDeviceDescs_.push_back(deviceDesc);

    auto ret = coreService_->GetFlagForMmapStream(streamDesc);
    EXPECT_EQ(AUDIO_OUTPUT_FLAG_FAST, ret);
}

/**
* @tc.name  : Test AudioCoreService
* @tc.number: UpdateRingerOrAlarmerDualDeviceOutputRouter_001
* @tc.desc  : Test UpdateRingerOrAlarmerDualDeviceOutputRouter() when device type is null
*/
HWTEST_F(AudioCoreServiceUnitTest, UpdateRingerOrAlarmerDualDeviceOutputRouter_001, TestSize.Level4)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest UpdateRingerOrAlarmerDualDeviceOutputRouter_001 start");

    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    audioCoreService->UpdateRingerOrAlarmerDualDeviceOutputRouter(nullptr);

    EXPECT_EQ(audioCoreService->shouldUpdateDeviceDueToDualTone_, false);

    AUDIO_INFO_LOG("AudioCoreServiceUnitTest UpdateRingerOrAlarmerDualDeviceOutputRouter_001 end");
}

/**
* @tc.name  : Test AudioCoreService
* @tc.number: UpdateRingerOrAlarmerDualDeviceOutputRouter_002
* @tc.desc  : Test UpdateRingerOrAlarmerDualDeviceOutputRouter() when device type is error
*/
HWTEST_F(AudioCoreServiceUnitTest, UpdateRingerOrAlarmerDualDeviceOutputRouter_002, TestSize.Level4)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest UpdateRingerOrAlarmerDualDeviceOutputRouter_002 start");

    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(streamDesc, nullptr);

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(audioDeviceDescriptor, nullptr);

    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_MIC;
    streamDesc->newDeviceDescs_.push_back(std::move(audioDeviceDescriptor));

    audioCoreService->UpdateRingerOrAlarmerDualDeviceOutputRouter(streamDesc);

    EXPECT_EQ(audioCoreService->shouldUpdateDeviceDueToDualTone_, true);
    EXPECT_EQ(audioCoreService->enableDualHalToneState_, false);

    AUDIO_INFO_LOG("AudioCoreServiceUnitTest UpdateRingerOrAlarmerDualDeviceOutputRouter_002 end");
}

/**
* @tc.name  : Test AudioCoreService
* @tc.number: UpdateRingerOrAlarmerDualDeviceOutputRouter_003
* @tc.desc  : Test UpdateRingerOrAlarmerDualDeviceOutputRouter() when device type is error
*/
HWTEST_F(AudioCoreServiceUnitTest, UpdateRingerOrAlarmerDualDeviceOutputRouter_003, TestSize.Level4)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest UpdateRingerOrAlarmerDualDeviceOutputRouter_003 start");

    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(streamDesc, nullptr);

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(audioDeviceDescriptor, nullptr);

    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDeviceDescs_.push_back(std::move(audioDeviceDescriptor));

    audioCoreService->UpdateRingerOrAlarmerDualDeviceOutputRouter(streamDesc);

    EXPECT_EQ(audioCoreService->audioVolumeManager_.IsRingerModeMute(), true);

    AUDIO_INFO_LOG("AudioCoreServiceUnitTest UpdateRingerOrAlarmerDualDeviceOutputRouter_003 end");
}

/**
* @tc.name  : Test AudioCoreService
* @tc.number: UpdateRingerOrAlarmerDualDeviceOutputRouter_005
* @tc.desc  : Test UpdateRingerOrAlarmerDualDeviceOutputRouter() when device type is error
*/
HWTEST_F(AudioCoreServiceUnitTest, UpdateRingerOrAlarmerDualDeviceOutputRouter_005, TestSize.Level4)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest UpdateRingerOrAlarmerDualDeviceOutputRouter_005 start");

    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(streamDesc, nullptr);

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(audioDeviceDescriptor, nullptr);

    audioCoreService->SetRingerMode(AudioRingerMode::RINGER_MODE_SILENT);

    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    streamDesc->newDeviceDescs_.push_back(std::move(audioDeviceDescriptor));
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_ALARM;

    audioCoreService->UpdateRingerOrAlarmerDualDeviceOutputRouter(streamDesc);

    EXPECT_EQ(audioCoreService->audioVolumeManager_.IsRingerModeMute(), false);

    AUDIO_INFO_LOG("AudioCoreServiceUnitTest UpdateRingerOrAlarmerDualDeviceOutputRouter_005 end");
}

/**
* @tc.name  : Test AudioCoreService SetRingerMode branch coverage
* @tc.number: SetRingerMode_Ringing_001
* @tc.desc  : Cover if branch when IsRingerAudioScene is true (enter if)
*/
HWTEST_F(AudioCoreServiceUnitTest, SetRingerMode_Ringing_001, TestSize.Level1)
{
    auto audioCoreService = GetServerPtr()->coreService_;
    ASSERT_NE(audioCoreService, nullptr);

    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_RINGING;

    int32_t result = audioCoreService->SetRingerMode(RINGER_MODE_NORMAL);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService SetRingerMode branch coverage
* @tc.number: SetRingerMode_Default_002
* @tc.desc  : Cover if branch when IsRingerAudioScene is false (skip if)
*/
HWTEST_F(AudioCoreServiceUnitTest, SetRingerMode_Default_002, TestSize.Level1)
{
    auto audioCoreService = GetServerPtr()->coreService_;
    ASSERT_NE(audioCoreService, nullptr);

    audioCoreService->audioSceneManager_.audioScene_ = AUDIO_SCENE_DEFAULT;

    int32_t result = audioCoreService->SetRingerMode(RINGER_MODE_NORMAL);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService
* @tc.number: UpdateDupDeviceOutputRoute_001
* @tc.desc  : Test UpdateDupDeviceOutputRoute() when device type is null
*/
HWTEST_F(AudioCoreServiceUnitTest, UpdateDupDeviceOutputRoute_001, TestSize.Level4)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest UpdateDupDeviceOutputRoute_003 start");

    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    audioCoreService->UpdateDupDeviceOutputRoute(nullptr);

    EXPECT_EQ(audioCoreService->shouldUpdateDeviceDueToDualTone_, false);

    AUDIO_INFO_LOG("AudioCoreServiceUnitTest UpdateDupDeviceOutputRoute_003 end");
}

/**
* @tc.name  : Test AudioCoreService
* @tc.number: UpdateDupDeviceOutputRoute_002
* @tc.desc  : Test UpdateDupDeviceOutputRoute() when device type is null
*/
HWTEST_F(AudioCoreServiceUnitTest, UpdateDupDeviceOutputRoute_002, TestSize.Level4)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest UpdateDupDeviceOutputRoute_002 start");

    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(streamDesc, nullptr);

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(audioDeviceDescriptor, nullptr);

    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDupDeviceDescs_.push_back(std::move(audioDeviceDescriptor));

    audioCoreService->UpdateDupDeviceOutputRoute(streamDesc);

    EXPECT_EQ(audioCoreService->shouldUpdateDeviceDueToDualTone_, true);

    AUDIO_INFO_LOG("AudioCoreServiceUnitTest UpdateDupDeviceOutputRoute_002 end");
}

/**
* @tc.name  : Test AudioCoreService
* @tc.number: UpdateDupDeviceOutputRoute_003
* @tc.desc  : Test UpdateDupDeviceOutputRoute() when device type is null
*/
HWTEST_F(AudioCoreServiceUnitTest, UpdateDupDeviceOutputRoute_003, TestSize.Level4)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest UpdateDupDeviceOutputRoute_003 start");

    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    ASSERT_NE(streamDesc, nullptr);

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(audioDeviceDescriptor, nullptr);

    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->oldDupDeviceDescs_.push_back(std::move(audioDeviceDescriptor));

    audioCoreService->UpdateDupDeviceOutputRoute(streamDesc);

    EXPECT_EQ(audioCoreService->shouldUpdateDeviceDueToDualTone_, false);

    AUDIO_INFO_LOG("AudioCoreServiceUnitTest UpdateDupDeviceOutputRoute_003 end");
}

/**
* @tc.name  : Test AudioCoreService
* @tc.number: SetSleVoiceStatusFlag_001
* @tc.desc  : Test SetSleVoiceStatusFlag
*/
HWTEST_F(AudioCoreServiceUnitTest, SetSleVoiceStatusFlag_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    auto desc = make_shared<AudioDeviceDescriptor>(DeviceType::DEVICE_TYPE_NEARLINK, DeviceRole::OUTPUT_DEVICE);
    desc->deviceId_ = TEST_DEVICE_ID;
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID, {desc});
    auto ret = audioCoreService->SetSleVoiceStatusFlag(AUDIO_SCENE_DEFAULT);
    EXPECT_EQ(ret, SUCCESS);
    ret = audioCoreService->SetSleVoiceStatusFlag(AUDIO_SCENE_PHONE_CALL);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: SetRendererTarget_001
* @tc.desc  : wzwzwz
*/
HWTEST_F(AudioCoreServiceUnitTest, SetRendererTarget_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    int32_t ret = ERROR;
    ret = audioCoreService->SetRendererTarget(NORMAL_PLAYBACK, INJECT_TO_VOICE_COMMUNICATION_CAPTURE, 1111);
    EXPECT_NE(ret, SUCCESS);
    ret = audioCoreService->SetRendererTarget(NORMAL_PLAYBACK, NORMAL_PLAYBACK, 1111);
    EXPECT_NE(ret, SUCCESS);
    ret = audioCoreService->SetRendererTarget(INJECT_TO_VOICE_COMMUNICATION_CAPTURE, NORMAL_PLAYBACK, 1111);
    EXPECT_EQ(ret, SUCCESS);
    ret = audioCoreService->SetRendererTarget(INJECT_TO_VOICE_COMMUNICATION_CAPTURE,
        INJECT_TO_VOICE_COMMUNICATION_CAPTURE, 1111);
    EXPECT_NE(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: StartInjection_001
* @tc.desc  : wzwzwz
*/
HWTEST_F(AudioCoreServiceUnitTest, StartInjection_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    int32_t ret = audioCoreService->StartInjection(1111);
    EXPECT_NE(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: PlayBackToInjection_001
* @tc.desc  : wzwzwz
*/
HWTEST_F(AudioCoreServiceUnitTest, PlayBackToInjection_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    int32_t ret = audioCoreService->PlayBackToInjection(1111);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: HandleMuteBeforeDeviceSwitch_001
 * @tc.desc  : Test AudioCoreService::HandleMuteBeforeDeviceSwitch()
 */
HWTEST_F(AudioCoreServiceUnitTest, HandleMuteBeforeDeviceSwitch_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    std::vector<AudioStreamStatus> streamStatusVec = {
        STREAM_STATUS_NEW,
        STREAM_STATUS_STARTED,
        STREAM_STATUS_PAUSED,
        STREAM_STATUS_STOPPED,
        STREAM_STATUS_RELEASED
    };
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs;
    for (auto status : streamStatusVec) {
        std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
        streamDesc->streamStatus_ = status;
        streamDescs.push_back(streamDesc);
    }

    AudioStreamDeviceChangeReasonExt::ExtEnum extReason = AudioStreamDeviceChangeReasonExt::ExtEnum::OVERRODE;
    AudioStreamDeviceChangeReasonExt reason(extReason);

    auto result = audioCoreService->HandleMuteBeforeDeviceSwitch(streamDescs, reason);
    EXPECT_TRUE(result);
}

/**
 * @tc.name  : Test A2dpOffloadGetRenderPosition.
 * @tc.number: A2dpOffloadGetRenderPosition_001
 * @tc.desc  : Test A2dpOffloadGetRenderPosition interfaces.
 */
HWTEST_F(AudioCoreServiceUnitTest, A2dpOffloadGetRenderPosition_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    uint32_t delayValue = 0;
    uint64_t sendDataSize = 0;
    uint32_t timeStamp = 0;

    CurrentActiveDevice().deviceType_ =
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP;
    CurrentActiveDevice().networkId_ = LOCAL_NETWORK_ID;
    int32_t ret = server->coreService_->A2dpOffloadGetRenderPosition(delayValue, sendDataSize, timeStamp);
    EXPECT_EQ(ret, SUCCESS);

    CurrentActiveDevice().deviceType_ =
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP;
    CurrentActiveDevice().networkId_ = REMOTE_NETWORK_ID;
    ret = server->coreService_->A2dpOffloadGetRenderPosition(delayValue, sendDataSize, timeStamp);
    EXPECT_EQ(ret, SUCCESS);

    CurrentActiveDevice().deviceType_ =
        DeviceType::DEVICE_TYPE_SPEAKER;
    CurrentActiveDevice().networkId_ = REMOTE_NETWORK_ID;
    ret = server->coreService_->A2dpOffloadGetRenderPosition(delayValue, sendDataSize, timeStamp);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test HandleDeviceConfigChanged.
 * @tc.number: HandleDeviceConfigChanged
 * @tc.desc  : Test HandleDeviceConfigChanged interfaces.
 */
HWTEST_F(AudioCoreServiceUnitTest, HandleDeviceConfigChanged_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_NEARLINK, DeviceRole::OUTPUT_DEVICE);
    desc->macAddress_ = "00:11:22:33:44:55";
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    ASSERT_NE(nullptr, desc) << "desc is nullptr.";
    std::string macAddress = "00:11:22:33:44:55";
    audioCoreService->HandleDeviceConfigChanged(desc);
    auto &deviceManager_ = AudioDeviceManager::GetAudioDeviceManager();
    EXPECT_TRUE(deviceManager_.ExistsByTypeAndAddress(DEVICE_TYPE_NEARLINK, macAddress));
}

/**
 * @tc.name  : Test HandleDeviceConfigChanged.
 * @tc.number: HandleDeviceConfigChanged
 * @tc.desc  : Test HandleDeviceConfigChanged interfaces.
 */
HWTEST_F(AudioCoreServiceUnitTest, HandleDeviceConfigChanged_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_NEARLINK, DeviceRole::OUTPUT_DEVICE);
    desc->macAddress_ = "00:00:22:33:44:55";
    AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
    ASSERT_NE(nullptr, desc) << "desc is nullptr.";
    std::string macAddress = "00:00:00:00:44:55";
    audioCoreService->HandleDeviceConfigChanged(desc);
    auto &deviceManager_ = AudioDeviceManager::GetAudioDeviceManager();
    EXPECT_FALSE(deviceManager_.ExistsByTypeAndAddress(DEVICE_TYPE_NEARLINK, macAddress));
}

/**
 * @tc.name   : Test AudioCoreServiceUnit
 * @tc.number : CheckStaticModeAndSelectFlag_001
 * @tc.desc   : Test CheckStaticModeAndSelectFlag interface - when rendererInfo_.isStatic = true
 */
HWTEST_F(AudioCoreServiceUnitTest, CheckStaticModeAndSelectFlag_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();

    streamDesc->rendererInfo_.isStatic = false;
    EXPECT_FALSE(audioCoreService->CheckStaticModeAndSelectFlag(streamDesc));

    streamDesc->rendererInfo_.isStatic = true;
    streamDesc->rendererInfo_.originalFlag = AUDIO_FLAG_MMAP;
    EXPECT_TRUE(audioCoreService->CheckStaticModeAndSelectFlag(streamDesc));

    streamDesc->rendererInfo_.originalFlag = AUDIO_FLAG_NORMAL;
    EXPECT_TRUE(audioCoreService->CheckStaticModeAndSelectFlag(streamDesc));
}

/**
 * @tc.name   : Test AudioCoreServiceUnit
 * @tc.number : CheckStaticModeAndSelectFlag_002
 * @tc.desc   : Test CheckStaticModeAndSelectFlag interface - when rendererInfo_.isStatic = true
 */
HWTEST_F(AudioCoreServiceUnitTest, CheckStaticModeAndSelectFlag_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();

    streamDesc->rendererInfo_.isStatic = false;
    EXPECT_FALSE(audioCoreService->CheckStaticModeAndSelectFlag(streamDesc));

    streamDesc->rendererInfo_.isStatic = true;
    streamDesc->rendererInfo_.originalFlag = AUDIO_FLAG_MMAP;
    EXPECT_TRUE(audioCoreService->CheckStaticModeAndSelectFlag(streamDesc));

    streamDesc->rendererInfo_.originalFlag = AUDIO_FLAG_NORMAL;
    EXPECT_TRUE(audioCoreService->CheckStaticModeAndSelectFlag(streamDesc));
}

/**
 * @tc.name   : Test AudioCoreService::HandleA2dpSuspendWhenLoad
 * @tc.number : HandleA2dpSuspendWhenLoad_001
 * @tc.desc   : Test HandleA2dpSuspendWhenLoad when a2dp need suspend
 */
HWTEST_F(AudioCoreServiceUnitTest, HandleA2dpSuspendWhenLoad_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    audioCoreService->a2dpNeedSuspend_.store(true);
    EXPECT_TRUE(audioCoreService->HandleA2dpSuspendWhenLoad());
}

/**
 * @tc.name   : Test AudioCoreService::HandleA2dpSuspendWhenLoad
 * @tc.number : HandleA2dpSuspendWhenLoad_002
 * @tc.desc   : Test HandleA2dpSuspendWhenLoad when a2dp needn't suspend
 */
HWTEST_F(AudioCoreServiceUnitTest, HandleA2dpSuspendWhenLoad_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    audioCoreService->a2dpNeedSuspend_.store(false);
    EXPECT_FALSE(audioCoreService->HandleA2dpSuspendWhenLoad());
}

/**
 * @tc.name   : Test AudioCoreService::HandleA2dpRestore
 * @tc.number : HandleA2dpRestore_001
 * @tc.desc   : Test HandleA2dpRestore, needn't restore
 */
HWTEST_F(AudioCoreServiceUnitTest, HandleA2dpRestore_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    audioCoreService->a2dpNeedSuspend_ = false;
    const uint32_t OLD_DEVICE_UNAVALIABLE_SUSPEND_MS = 1000; // 1s
    audioCoreService->a2dpSuspendUntil_ = std::chrono::steady_clock::now() +
        std::chrono::milliseconds(OLD_DEVICE_UNAVALIABLE_SUSPEND_MS);
    audioCoreService->HandleA2dpRestore();
    EXPECT_FALSE(audioCoreService->a2dpNeedSuspend_);
}

/**
 * @tc.name   : Test AudioCoreService::HandleA2dpRestore
 * @tc.number : HandleA2dpRestore_002
 * @tc.desc   : Test HandleA2dpRestore, call before a2dpSuspendUntil_
 */
HWTEST_F(AudioCoreServiceUnitTest, HandleA2dpRestore_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    audioCoreService->a2dpNeedSuspend_ = true;
    const uint32_t OLD_DEVICE_UNAVALIABLE_SUSPEND_MS = 1000; // 1s
    audioCoreService->a2dpSuspendUntil_ = std::chrono::steady_clock::now() +
        std::chrono::milliseconds(OLD_DEVICE_UNAVALIABLE_SUSPEND_MS);
    audioCoreService->HandleA2dpRestore();
    EXPECT_TRUE(audioCoreService->a2dpNeedSuspend_);
}

/**
 * @tc.name   : Test AudioCoreService::HandleA2dpRestore
 * @tc.number : HandleA2dpRestore_003
 * @tc.desc   : Test HandleA2dpRestore, call after a2dpSuspendUntil_
 */
HWTEST_F(AudioCoreServiceUnitTest, HandleA2dpRestore_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    audioCoreService->a2dpNeedSuspend_ = true;
    const uint32_t OLD_DEVICE_UNAVALIABLE_SUSPEND_MS = 1000; // 1s
    auto now = std::chrono::steady_clock::now();
    audioCoreService->a2dpSuspendUntil_ = now - std::chrono::milliseconds(OLD_DEVICE_UNAVALIABLE_SUSPEND_MS);
    auto afterSuspend = now + std::chrono::milliseconds(OLD_DEVICE_UNAVALIABLE_SUSPEND_MS);
    audioCoreService->HandleA2dpRestore();
    EXPECT_TRUE(std::chrono::steady_clock::now() < afterSuspend);
    EXPECT_FALSE(audioCoreService->a2dpNeedSuspend_);
}

/**
 * @tc.name   : Test AudioCoreService::HandleA2dpRestore
 * @tc.number : RecordIsForcedNormal_001
 * @tc.desc   : Test HandleA2dpRestore, call after a2dpSuspendUntil_
 */
HWTEST_F(AudioCoreServiceUnitTest, RecordIsForcedNormal_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->capturerInfo_.originalFlag = AUDIO_FLAG_FORCED_NORMAL;
    streamDesc->capturerInfo_.capturerFlags = AUDIO_FLAG_FORCED_NORMAL;
    EXPECT_EQ(audioCoreService->RecordIsForcedNormal(streamDesc), true);


    streamDesc->capturerInfo_.originalFlag = AUDIO_FLAG_MMAP;
    streamDesc->capturerInfo_.capturerFlags = AUDIO_FLAG_FORCED_NORMAL;
    EXPECT_EQ(audioCoreService->RecordIsForcedNormal(streamDesc), true);

    streamDesc->capturerInfo_.originalFlag = AUDIO_FLAG_FORCED_NORMAL;
    streamDesc->capturerInfo_.capturerFlags = AUDIO_FLAG_MMAP;
    EXPECT_EQ(audioCoreService->RecordIsForcedNormal(streamDesc), true);
}

/**
 * @tc.name   : Test AudioCoreService::HandleA2dpRestore
 * @tc.number : RecordIsForcedNormal_002
 * @tc.desc   : Test HandleA2dpRestore, call after a2dpSuspendUntil_
 */
HWTEST_F(AudioCoreServiceUnitTest, RecordIsForcedNormal_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->capturerInfo_.originalFlag = AUDIO_FLAG_MMAP;
    streamDesc->capturerInfo_.capturerFlags = AUDIO_FLAG_MMAP;

    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_REMOTE_CAST;
    EXPECT_EQ(audioCoreService->RecordIsForcedNormal(streamDesc), true);

    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_MIC;
    streamDesc->newDeviceDescs_ = {};
    EXPECT_EQ(audioCoreService->RecordIsForcedNormal(streamDesc), false);
}

/**
 * @tc.name   : Test AudioCoreService::HandleA2dpRestore
 * @tc.number : RecordIsForcedNormal_003
 * @tc.desc   : Test HandleA2dpRestore, call after a2dpSuspendUntil_
 */
HWTEST_F(AudioCoreServiceUnitTest, RecordIsForcedNormal_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->capturerInfo_.originalFlag = AUDIO_FLAG_MMAP;
    streamDesc->capturerInfo_.capturerFlags = AUDIO_FLAG_MMAP;
    streamDesc->capturerInfo_.sourceType == SOURCE_TYPE_REMOTE_CAST;

    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    streamDesc->newDeviceDescs_.push_back(deviceDesc);

    deviceDesc->deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    deviceDesc->SetDeviceSupportMmap(0);
    EXPECT_EQ(audioCoreService->RecordIsForcedNormal(streamDesc), true);
    deviceDesc->SetDeviceSupportMmap(1);
    EXPECT_EQ(audioCoreService->RecordIsForcedNormal(streamDesc), false);

    deviceDesc->deviceType_ = DEVICE_TYPE_MIC;
    deviceDesc->SetDeviceSupportMmap(0);
    EXPECT_EQ(audioCoreService->RecordIsForcedNormal(streamDesc), false);
    deviceDesc->SetDeviceSupportMmap(1);
    EXPECT_EQ(audioCoreService->RecordIsForcedNormal(streamDesc), false);
}

/**
 * @tc.name   : Test AudioCoreService::HandleA2dpRestore
 * @tc.number : IsForcedNormal_001
 * @tc.desc   : Test HandleA2dpRestore, call after a2dpSuspendUntil_
 */
HWTEST_F(AudioCoreServiceUnitTest, IsForcedNormal_010, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->rendererInfo_.originalFlag = AUDIO_FLAG_MMAP;
    streamDesc->rendererInfo_.rendererFlags = AUDIO_FLAG_MMAP;

    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    streamDesc->newDeviceDescs_.push_back(deviceDesc);

    deviceDesc->deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    deviceDesc->SetDeviceSupportMmap(0);
    EXPECT_EQ(audioCoreService->IsForcedNormal(streamDesc), true);
    deviceDesc->SetDeviceSupportMmap(1);
    EXPECT_EQ(audioCoreService->IsForcedNormal(streamDesc), false);

    deviceDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    deviceDesc->SetDeviceSupportMmap(0);
    EXPECT_EQ(audioCoreService->IsForcedNormal(streamDesc), false);
    deviceDesc->SetDeviceSupportMmap(1);
    EXPECT_EQ(audioCoreService->IsForcedNormal(streamDesc), false);
}

static void PutCurrentOutputDevice(AudioDeviceDescriptor &desc)
{
    if (desc.deviceId_ < 1) {
        desc.deviceId_ = TEST_DEVICE_ID;
    }
    auto dev = AudioDeviceManager::GetAudioDeviceManager().FindConnectedDeviceById(desc.deviceId_);
    if (!dev) {
        dev = make_shared<AudioDeviceDescriptor>(desc);
        AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(dev);
    }
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID, {dev});
}

/**
 * @tc.name: HandlePlaybackStoppingOperations_003
 * @tc.desc: Test condition2: Phone call scene should NOT fetch device (device unchanged).
 * @tc.type: FUNC
 */
HWTEST_F(AudioCoreServiceUnitTest, HandlePlaybackStoppingOperations_003, TestSize.Level1)
{
    auto audioCoreService = GetServerPtr()->coreService_;
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    AudioDeviceDescriptor curDesc(DeviceType::DEVICE_TYPE_NONE, DeviceRole::OUTPUT_DEVICE);
    PutCurrentOutputDevice(curDesc);
    DeviceType deviceBefore = audioCoreService->audioActiveDevice_.GetCurrentOutputDeviceType();
    EXPECT_EQ(deviceBefore, DEVICE_TYPE_NONE);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = TEST_SESSION_ID;
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->streamStatus_ = STREAM_STATUS_STOPPED;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;
    streamDesc->callerUid_ = 1000;

    audioCoreService->audioSceneManager_.SetAudioScenePre(AUDIO_SCENE_PHONE_CALL);
    audioCoreService->audioSceneManager_.SetPhoneCallOrChatSceneTriggerUid(1000);

    audioCoreService->HandlePlaybackStoppingOperations(streamDesc, "TestCaller");

    DeviceType deviceAfter = audioCoreService->audioActiveDevice_.GetCurrentOutputDeviceType();
    EXPECT_EQ(deviceAfter, deviceBefore);
}

/**
 * @tc.name: HandlePlaybackStoppingOperations_004
 * @tc.desc: Test condition2: NOT phone/chat scene should fetch device (device changed).
 * @tc.type: FUNC
 */
HWTEST_F(AudioCoreServiceUnitTest, HandlePlaybackStoppingOperations_004, TestSize.Level1)
{
    auto audioCoreService = GetServerPtr()->coreService_;
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    AudioDeviceDescriptor curDesc(DeviceType::DEVICE_TYPE_NONE, DeviceRole::OUTPUT_DEVICE);
    PutCurrentOutputDevice(curDesc);
    DeviceType deviceBefore = audioCoreService->audioActiveDevice_.GetCurrentOutputDeviceType();
    EXPECT_EQ(deviceBefore, DEVICE_TYPE_NONE);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = TEST_SESSION_ID;
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->streamStatus_ = STREAM_STATUS_STOPPED;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;

    audioCoreService->audioSceneManager_.SetAudioScenePre(AUDIO_SCENE_DEFAULT);

    audioCoreService->HandlePlaybackStoppingOperations(streamDesc, "TestCaller");

    DeviceType deviceAfter = audioCoreService->audioActiveDevice_.GetCurrentOutputDeviceType();
    EXPECT_EQ(deviceAfter, deviceBefore);
}

/**
 * @tc.name: HandlePlaybackStoppingOperations_005
 * @tc.desc: Test condition3: EnableDualHalTone true and sessionId matches should fetch device.
 * @tc.type: FUNC
 */
HWTEST_F(AudioCoreServiceUnitTest, HandlePlaybackStoppingOperations_005, TestSize.Level1)
{
    auto audioCoreService = GetServerPtr()->coreService_;
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    AudioDeviceDescriptor curDesc(DeviceType::DEVICE_TYPE_NONE, DeviceRole::OUTPUT_DEVICE);
    PutCurrentOutputDevice(curDesc);
    DeviceType deviceBefore = audioCoreService->audioActiveDevice_.GetCurrentOutputDeviceType();
    EXPECT_EQ(deviceBefore, DEVICE_TYPE_NONE);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = TEST_SESSION_ID;
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->streamStatus_ = STREAM_STATUS_STOPPED;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_RINGTONE;

    audioCoreService->enableDualHalToneState_ = true;
    audioCoreService->enableDualHalToneSessionId_ = TEST_SESSION_ID;

    audioCoreService->HandlePlaybackStoppingOperations(streamDesc, "TestCaller");

    DeviceType deviceAfter = audioCoreService->audioActiveDevice_.GetCurrentOutputDeviceType();
    EXPECT_EQ(deviceAfter, deviceBefore);
}

/**
 * @tc.name: HandlePlaybackStoppingOperations_006
 * @tc.desc: Test condition3: EnableDualHalTone true but sessionId NOT matches should NOT fetch device.
 * @tc.type: FUNC
 */
HWTEST_F(AudioCoreServiceUnitTest, HandlePlaybackStoppingOperations_006, TestSize.Level1)
{
    auto audioCoreService = GetServerPtr()->coreService_;
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    AudioDeviceDescriptor curDesc(DeviceType::DEVICE_TYPE_NONE, DeviceRole::OUTPUT_DEVICE);
    PutCurrentOutputDevice(curDesc);
    DeviceType deviceBefore = audioCoreService->audioActiveDevice_.GetCurrentOutputDeviceType();
    EXPECT_EQ(deviceBefore, DEVICE_TYPE_NONE);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = TEST_SESSION_ID;
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->streamStatus_ = STREAM_STATUS_STOPPED;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_RINGTONE;

    audioCoreService->enableDualHalToneState_ = true;
    audioCoreService->enableDualHalToneSessionId_ = TEST_SESSION_ID + 1;

    audioCoreService->HandlePlaybackStoppingOperations(streamDesc, "TestCaller");

    DeviceType deviceAfter = audioCoreService->audioActiveDevice_.GetCurrentOutputDeviceType();
    EXPECT_EQ(deviceAfter, deviceBefore);
}

/**
 * @tc.name: HandlePlaybackStoppingOperations_007
 * @tc.desc: Test condition3: EnableDualHalTone false even if sessionId matches should NOT fetch device.
 * @tc.type: FUNC
 */
HWTEST_F(AudioCoreServiceUnitTest, HandlePlaybackStoppingOperations_007, TestSize.Level1)
{
    auto audioCoreService = GetServerPtr()->coreService_;
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    AudioDeviceDescriptor curDesc(DeviceType::DEVICE_TYPE_NONE, DeviceRole::OUTPUT_DEVICE);
    PutCurrentOutputDevice(curDesc);
    DeviceType deviceBefore = audioCoreService->audioActiveDevice_.GetCurrentOutputDeviceType();
    EXPECT_EQ(deviceBefore, DEVICE_TYPE_NONE);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = TEST_SESSION_ID;
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->streamStatus_ = STREAM_STATUS_STOPPED;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_RINGTONE;

    audioCoreService->enableDualHalToneState_ = false;
    audioCoreService->enableDualHalToneSessionId_ = TEST_SESSION_ID;

    audioCoreService->HandlePlaybackStoppingOperations(streamDesc, "TestCaller");

    DeviceType deviceAfter = audioCoreService->audioActiveDevice_.GetCurrentOutputDeviceType();
    EXPECT_EQ(deviceAfter, deviceBefore);
}

/**
 * @tc.name: HandlePlaybackStoppingOperations_008
 * @tc.desc: Test condition3: EnableDualHalTone false and sessionId NOT matches should NOT fetch device.
 * @tc.type: FUNC
 */
HWTEST_F(AudioCoreServiceUnitTest, HandlePlaybackStoppingOperations_008, TestSize.Level1)
{
    auto audioCoreService = GetServerPtr()->coreService_;
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    AudioDeviceDescriptor curDesc(DeviceType::DEVICE_TYPE_NONE, DeviceRole::OUTPUT_DEVICE);
    PutCurrentOutputDevice(curDesc);
    DeviceType deviceBefore = audioCoreService->audioActiveDevice_.GetCurrentOutputDeviceType();
    EXPECT_EQ(deviceBefore, DEVICE_TYPE_NONE);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = TEST_SESSION_ID;
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->streamStatus_ = STREAM_STATUS_STOPPED;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_RINGTONE;

    audioCoreService->enableDualHalToneState_ = false;
    audioCoreService->enableDualHalToneSessionId_ = TEST_SESSION_ID + 1;

    audioCoreService->HandlePlaybackStoppingOperations(streamDesc, "TestCaller");

    DeviceType deviceAfter = audioCoreService->audioActiveDevice_.GetCurrentOutputDeviceType();
    EXPECT_EQ(deviceAfter, deviceBefore);
}

/**
 * @tc.name: HandlePlaybackStoppingOperations_009
 * @tc.desc: Test condition4: STOPPED + Ringer stream should call UpdateDualToneState (under condition3).
 * @tc.type: FUNC
 */
HWTEST_F(AudioCoreServiceUnitTest, HandlePlaybackStoppingOperations_009, TestSize.Level1)
{
    auto audioCoreService = GetServerPtr()->coreService_;
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = TEST_SESSION_ID;
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->streamStatus_ = STREAM_STATUS_STOPPED;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_RINGTONE;

    audioCoreService->enableDualHalToneState_ = true;
    audioCoreService->enableDualHalToneSessionId_ = TEST_SESSION_ID;

    audioCoreService->HandlePlaybackStoppingOperations(streamDesc, "TestCaller");

    EXPECT_FALSE(audioCoreService->enableDualHalToneState_);
}

/**
 * @tc.name: HandlePlaybackStoppingOperations_010
 * @tc.desc: Test condition4: STOPPED + Media stream should NOT call UpdateDualToneState (under condition3).
 * @tc.type: FUNC
 */
HWTEST_F(AudioCoreServiceUnitTest, HandlePlaybackStoppingOperations_010, TestSize.Level1)
{
    auto audioCoreService = GetServerPtr()->coreService_;
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = TEST_SESSION_ID;
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->streamStatus_ = STREAM_STATUS_STOPPED;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MEDIA;

    audioCoreService->enableDualHalToneState_ = true;
    audioCoreService->enableDualHalToneSessionId_ = TEST_SESSION_ID;

    audioCoreService->HandlePlaybackStoppingOperations(streamDesc, "TestCaller");

    EXPECT_TRUE(audioCoreService->enableDualHalToneState_);
}

/**
 * @tc.name: HandlePlaybackStoppingOperations_011
 * @tc.desc: Test condition4: RELEASED + Alarm stream should call UpdateDualToneState (under condition3).
 * @tc.type: FUNC
 */
HWTEST_F(AudioCoreServiceUnitTest, HandlePlaybackStoppingOperations_011, TestSize.Level1)
{
    auto audioCoreService = GetServerPtr()->coreService_;
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = TEST_SESSION_ID;
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->streamStatus_ = STREAM_STATUS_RELEASED;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_ALARM;

    audioCoreService->enableDualHalToneState_ = true;
    audioCoreService->enableDualHalToneSessionId_ = TEST_SESSION_ID;

    audioCoreService->HandlePlaybackStoppingOperations(streamDesc, "TestCaller");

    EXPECT_FALSE(audioCoreService->enableDualHalToneState_);
}

/**
 * @tc.name: HandlePlaybackStoppingOperations_012
 * @tc.desc: Test condition4: RELEASED + Media stream should NOT call UpdateDualToneState (under condition3).
 * @tc.type: FUNC
 */
HWTEST_F(AudioCoreServiceUnitTest, HandlePlaybackStoppingOperations_012, TestSize.Level1)
{
    auto audioCoreService = GetServerPtr()->coreService_;
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = TEST_SESSION_ID;
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->streamStatus_ = STREAM_STATUS_RELEASED;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MEDIA;

    audioCoreService->enableDualHalToneState_ = true;
    audioCoreService->enableDualHalToneSessionId_ = TEST_SESSION_ID;

    audioCoreService->HandlePlaybackStoppingOperations(streamDesc, "TestCaller");

    EXPECT_TRUE(audioCoreService->enableDualHalToneState_);
}

/**
 * @tc.name: HandlePlaybackStoppingOperations_013
 * @tc.desc: Test condition5&6: Cond5 true, cond6 true (not paused) - should fetch device.
 * @tc.type: FUNC
 */
HWTEST_F(AudioCoreServiceUnitTest, HandlePlaybackStoppingOperations_013, TestSize.Level1)
{
    auto audioCoreService = GetServerPtr()->coreService_;
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    AudioDeviceDescriptor curDesc(DeviceType::DEVICE_TYPE_NONE, DeviceRole::OUTPUT_DEVICE);
    PutCurrentOutputDevice(curDesc);
    DeviceType deviceBefore = audioCoreService->audioActiveDevice_.GetCurrentOutputDeviceType();
    EXPECT_EQ(deviceBefore, DEVICE_TYPE_NONE);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = TEST_SESSION_ID;
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->streamStatus_ = STREAM_STATUS_STOPPED;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_RINGTONE;

    audioCoreService->isRingDualToneOnPrimarySpeaker_ = true;

    audioCoreService->HandlePlaybackStoppingOperations(streamDesc, "TestCaller");

    DeviceType deviceAfter = audioCoreService->audioActiveDevice_.GetCurrentOutputDeviceType();
    EXPECT_EQ(deviceAfter, deviceBefore);
}

/**
 * @tc.name: HandlePlaybackStoppingOperations_014
 * @tc.desc: Test condition5&6: Cond5 true, cond6 true (paused not alarm) - should fetch device.
 * @tc.type: FUNC
 */
HWTEST_F(AudioCoreServiceUnitTest, HandlePlaybackStoppingOperations_014, TestSize.Level1)
{
    auto audioCoreService = GetServerPtr()->coreService_;
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    AudioDeviceDescriptor curDesc(DeviceType::DEVICE_TYPE_NONE, DeviceRole::OUTPUT_DEVICE);
    PutCurrentOutputDevice(curDesc);
    DeviceType deviceBefore = audioCoreService->audioActiveDevice_.GetCurrentOutputDeviceType();
    EXPECT_EQ(deviceBefore, DEVICE_TYPE_NONE);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = TEST_SESSION_ID;
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->streamStatus_ = STREAM_STATUS_PAUSED;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_RINGTONE;

    audioCoreService->audioSceneManager_.SetAudioScenePre(AUDIO_SCENE_DEFAULT);
    audioCoreService->isRingDualToneOnPrimarySpeaker_ = true;

    audioCoreService->HandlePlaybackStoppingOperations(streamDesc, "TestCaller");

    DeviceType deviceAfter = audioCoreService->audioActiveDevice_.GetCurrentOutputDeviceType();
    EXPECT_EQ(deviceAfter, deviceBefore);
}

/**
 * @tc.name: HandlePlaybackStoppingOperations_015
 * @tc.desc: Test condition5&6: Cond5 true, cond6 false (paused alarm) - should NOT fetch device.
 * @tc.type: FUNC
 */
HWTEST_F(AudioCoreServiceUnitTest, HandlePlaybackStoppingOperations_015, TestSize.Level1)
{
    auto audioCoreService = GetServerPtr()->coreService_;
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    AudioDeviceDescriptor curDesc(DeviceType::DEVICE_TYPE_NONE, DeviceRole::OUTPUT_DEVICE);
    PutCurrentOutputDevice(curDesc);
    DeviceType deviceBefore = audioCoreService->audioActiveDevice_.GetCurrentOutputDeviceType();
    EXPECT_EQ(deviceBefore, DEVICE_TYPE_NONE);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = TEST_SESSION_ID;
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->streamStatus_ = STREAM_STATUS_PAUSED;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_ALARM;

    audioCoreService->audioSceneManager_.SetAudioScenePre(AUDIO_SCENE_DEFAULT);
    audioCoreService->isRingDualToneOnPrimarySpeaker_ = true;

    audioCoreService->HandlePlaybackStoppingOperations(streamDesc, "TestCaller");

    DeviceType deviceAfter = audioCoreService->audioActiveDevice_.GetCurrentOutputDeviceType();
    EXPECT_EQ(deviceAfter, deviceBefore);
}

/**
 * @tc.name: HandlePlaybackStoppingOperations_016
 * @tc.desc: Test condition5&6: Cond5 true, cond6 false (paused in ring scene) - should NOT fetch device.
 * @tc.type: FUNC
 */
HWTEST_F(AudioCoreServiceUnitTest, HandlePlaybackStoppingOperations_016, TestSize.Level1)
{
    auto audioCoreService = GetServerPtr()->coreService_;
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    AudioDeviceDescriptor curDesc(DeviceType::DEVICE_TYPE_NONE, DeviceRole::OUTPUT_DEVICE);
    PutCurrentOutputDevice(curDesc);
    DeviceType deviceBefore = audioCoreService->audioActiveDevice_.GetCurrentOutputDeviceType();
    EXPECT_EQ(deviceBefore, DEVICE_TYPE_NONE);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = TEST_SESSION_ID;
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->streamStatus_ = STREAM_STATUS_PAUSED;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_RINGTONE;

    audioCoreService->audioSceneManager_.SetAudioScenePre(AUDIO_SCENE_VOICE_RINGING);
    audioCoreService->isRingDualToneOnPrimarySpeaker_ = true;

    audioCoreService->HandlePlaybackStoppingOperations(streamDesc, "TestCaller");

    DeviceType deviceAfter = audioCoreService->audioActiveDevice_.GetCurrentOutputDeviceType();
    EXPECT_EQ(deviceAfter, deviceBefore);
}

/**
 * @tc.name: HandlePlaybackStoppingOperations_017
 * @tc.desc: Test condition5&6: Cond5 false (not ringer) - should NOT fetch device.
 * @tc.type: FUNC
 */
HWTEST_F(AudioCoreServiceUnitTest, HandlePlaybackStoppingOperations_017, TestSize.Level1)
{
    auto audioCoreService = GetServerPtr()->coreService_;
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->Init();

    AudioDeviceDescriptor curDesc(DeviceType::DEVICE_TYPE_NONE, DeviceRole::OUTPUT_DEVICE);
    PutCurrentOutputDevice(curDesc);
    DeviceType deviceBefore = audioCoreService->audioActiveDevice_.GetCurrentOutputDeviceType();
    EXPECT_EQ(deviceBefore, DEVICE_TYPE_NONE);

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = TEST_SESSION_ID;
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->streamStatus_ = STREAM_STATUS_STOPPED;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MEDIA;

    audioCoreService->isRingDualToneOnPrimarySpeaker_ = true;

    audioCoreService->HandlePlaybackStoppingOperations(streamDesc, "TestCaller");

    DeviceType deviceAfter = audioCoreService->audioActiveDevice_.GetCurrentOutputDeviceType();
    EXPECT_EQ(deviceAfter, deviceBefore);
}

/**
 * @tc.name  : Test AudioCoreService with STREAM_USAGE_INTERPHONE
 * @tc.number: AudioCoreServiceUnitTest_Interphone_001
 * @tc.desc  : Test CreateRendererClient with STREAM_USAGE_INTERPHONE
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCoreServiceUnitTest_Interphone_001, TestSize.Level1)
{
    AudioStreamInfo audioStreamInfo = {};
    audioStreamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    audioStreamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    audioStreamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioStreamInfo.channels = STEREO;

    AudioRendererInfo rendererInfo = {};
    rendererInfo.streamUsage = STREAM_USAGE_INTERPHONE;
    rendererInfo.contentType = ContentType::CONTENT_TYPE_UNKNOWN;
    rendererInfo.rendererFlags = 0;

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamInfo_ = audioStreamInfo;
    streamDesc->rendererInfo_ = rendererInfo;
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->createTimeStamp_ = ClockTime::GetCurNano();
    streamDesc->callerUid_ = getuid();

    uint32_t flag = AUDIO_OUTPUT_FLAG_NORMAL;
    uint32_t originalSessionId = 0;
    std::string networkId = LOCAL_NETWORK_ID;

    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    deviceDesc->networkId_ = LOCAL_NETWORK_ID;
    deviceDesc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    streamDesc->newDeviceDescs_.clear();
    streamDesc->newDeviceDescs_.push_back(deviceDesc);

    auto result = GetServerPtr()->eventEntry_->CreateRendererClient(streamDesc, flag, originalSessionId, networkId);
    EXPECT_EQ(result, SUCCESS);

    result = GetServerPtr()->coreService_->ReleaseClient(originalSessionId, SESSION_OP_MSG_DEFAULT, false);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService with STREAM_USAGE_INTERPHONE
 * @tc.number: AudioCoreServiceUnitTest_Interphone_002
 * @tc.desc  : Test CreateCapturerClient with SOURCE_TYPE_INTERPHONE
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCoreServiceUnitTest_Interphone_002, TestSize.Level1)
{
    AudioStreamInfo audioStreamInfo = {};
    audioStreamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    audioStreamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    audioStreamInfo.channels = MONO;

    AudioCapturerInfo capturerInfo = {};
    capturerInfo.sourceType = SOURCE_TYPE_INTERPHONE;
    capturerInfo.capturerFlags = 0;

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamInfo_ = audioStreamInfo;
    streamDesc->capturerInfo_ = capturerInfo;
    streamDesc->audioMode_ = AUDIO_MODE_RECORD;
    streamDesc->createTimeStamp_ = ClockTime::GetCurNano();
    streamDesc->callerUid_ = getuid();

    uint32_t flag = AUDIO_INPUT_FLAG_NORMAL;
    uint32_t originalSessionId = 0;

    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_MIC;
    deviceDesc->networkId_ = LOCAL_NETWORK_ID;
    deviceDesc->deviceRole_ = DeviceRole::INPUT_DEVICE;
    streamDesc->newDeviceDescs_.clear();
    streamDesc->newDeviceDescs_.push_back(deviceDesc);

    auto result = GetServerPtr()->eventEntry_->CreateCapturerClient(streamDesc, flag, originalSessionId);
    EXPECT_EQ(result, SUCCESS);

    result = GetServerPtr()->coreService_->ReleaseClient(originalSessionId, SESSION_OP_MSG_DEFAULT, false);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name   : Test AudioCoreService::SetCallbackHandler
 * @tc.number : SetCallbackHandler_001
 * @tc.desc   : Test SetCallbackHandler with nullptr does not crash.
 */
HWTEST_F(AudioCoreServiceUnitTest, SetCallbackHandler_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    EXPECT_NO_THROW(audioCoreService->SetCallbackHandler(nullptr));
}

/**
 * @tc.name   : Test AudioCoreService::GetEventEntry
 * @tc.number : GetEventEntry_001
 * @tc.desc   : Test GetEventEntry returns nullptr before Init.
 */
HWTEST_F(AudioCoreServiceUnitTest, GetEventEntry_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    auto entry = audioCoreService->GetEventEntry();
    EXPECT_EQ(entry, nullptr);
}

/**
 * @tc.name   : Test AudioCoreService::DumpPipeManager
 * @tc.number : DumpPipeManager_001
 * @tc.desc   : Test DumpPipeManager can be called on a fresh instance without crash.
 */
HWTEST_F(AudioCoreServiceUnitTest, DumpPipeManager_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    std::string dumpString;
    EXPECT_NO_THROW(audioCoreService->DumpPipeManager(dumpString));
}

/**
 * @tc.name   : Test AudioCoreService::IsHWDecoding
 * @tc.number : IsHWDecoding_001
 * @tc.desc   : Test IsHWDecoding with nullptr returns false.
 */
HWTEST_F(AudioCoreServiceUnitTest, IsHWDecoding_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    bool ret = audioCoreService->IsHWDecoding(nullptr);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name   : Test AudioCoreService::IsHWDecoding
 * @tc.number : IsHWDecoding_002
 * @tc.desc   : Test IsHWDecoding with PCM encoding returns false.
 */
HWTEST_F(AudioCoreServiceUnitTest, IsHWDecoding_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamInfo_.encoding = ENCODING_PCM;
    bool ret = audioCoreService->IsHWDecoding(streamDesc);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name   : Test AudioCoreService::IsArmUsbDevice
 * @tc.number : IsArmUsbDevice_001
 * @tc.desc   : Test IsArmUsbDevice with non-USB device returns false.
 */
HWTEST_F(AudioCoreServiceUnitTest, IsArmUsbDevice_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    AudioDeviceDescriptor deviceDesc;
    deviceDesc.deviceType_ = DEVICE_TYPE_SPEAKER;
    bool result = audioCoreService->IsArmUsbDevice(deviceDesc);
    EXPECT_FALSE(result);
}

/**
 * @tc.name   : Test AudioCoreService::SetCallDeviceActive
 * @tc.number : SetCallDeviceActive_001
 * @tc.desc   : Test SetCallDeviceActive with DEVICE_TYPE_NONE returns ERR_DEVICE_NOT_SUPPORTED.
 */
HWTEST_F(AudioCoreServiceUnitTest, SetCallDeviceActive_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    int32_t ret = audioCoreService->SetCallDeviceActive(DEVICE_TYPE_NONE, true, "", -1);
    EXPECT_EQ(ret, ERR_DEVICE_NOT_SUPPORTED);
}

/**
 * @tc.name   : Test AudioCoreService::CreateRendererClient
 * @tc.number : CreateRendererClient_null_001
 * @tc.desc   : Test CreateRendererClient with nullptr streamDesc returns ERR_NULL_POINTER.
 */
HWTEST_F(AudioCoreServiceUnitTest, CreateRendererClient_null_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = nullptr;
    uint32_t audioFlag = 0;
    uint32_t sessionId = 0;
    std::string networkId;
    int32_t ret = audioCoreService->CreateRendererClient(streamDesc, audioFlag, sessionId, networkId);
    EXPECT_EQ(ret, ERR_NULL_POINTER);
}

/**
 * @tc.name   : Test AudioCoreService::CreateCapturerClient
 * @tc.number : CreateCapturerClient_null_001
 * @tc.desc   : Test CreateCapturerClient with nullptr streamDesc returns ERR_INVALID_PARAM.
 */
HWTEST_F(AudioCoreServiceUnitTest, CreateCapturerClient_null_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = nullptr;
    uint32_t audioFlag = 0;
    uint32_t sessionId = 0;
    int32_t ret = audioCoreService->CreateCapturerClient(streamDesc, audioFlag, sessionId);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name   : Test AudioCoreService::ClearSelectedInputDeviceByUid
 * @tc.number : ClearSelectedInputDeviceByUid_001
 * @tc.desc   : Test ClearSelectedInputDeviceByUid returns SUCCESS.
 */
HWTEST_F(AudioCoreServiceUnitTest, ClearSelectedInputDeviceByUid_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    int32_t ret = audioCoreService->ClearSelectedInputDeviceByUid(-1);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test AudioCoreService::CloseWakeUpAudioCapturer
 * @tc.number : CloseWakeUpAudioCapturer_001
 * @tc.desc   : Test CloseWakeUpAudioCapturer can be called without crash.
 */
HWTEST_F(AudioCoreServiceUnitTest, CloseWakeUpAudioCapturer_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    EXPECT_NO_THROW(audioCoreService->CloseWakeUpAudioCapturer());
}

/**
 * @tc.name   : Test AudioCoreService::OnDeviceInfoUpdated
 * @tc.number : OnDeviceInfoUpdated_001
 * @tc.desc   : Test OnDeviceInfoUpdated can be called with CATEGORY_UPDATE command.
 */
HWTEST_F(AudioCoreServiceUnitTest, OnDeviceInfoUpdated_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    AudioDeviceDescriptor desc;
    desc.deviceType_ = DEVICE_TYPE_SPEAKER;
    EXPECT_NO_THROW(audioCoreService->OnDeviceInfoUpdated(desc, DeviceInfoUpdateCommand::CATEGORY_UPDATE));
}

/**
 * @tc.name   : Test AudioCoreService::SetFirstScreenOn
 * @tc.number : SetFirstScreenOn_001
 * @tc.desc   : Test SetFirstScreenOn sets the flag.
 */
HWTEST_F(AudioCoreServiceUnitTest, SetFirstScreenOn_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    audioCoreService->SetFirstScreenOn();
    EXPECT_TRUE(audioCoreService->isFirstScreenOn_);
}

/**
 * @tc.name   : Test AudioCoreService::ClearStreamPropInfo
 * @tc.number : ClearStreamPropInfo_001
 * @tc.desc   : Test ClearStreamPropInfo can be called without crash.
 */
HWTEST_F(AudioCoreServiceUnitTest, ClearStreamPropInfo_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    EXPECT_NO_THROW(audioCoreService->ClearStreamPropInfo("primary", "primary_output"));
}

/**
 * @tc.name   : Test AudioCoreService::GetStreamPropInfoSize
 * @tc.number : GetStreamPropInfoSize_001
 * @tc.desc   : Test GetStreamPropInfoSize returns 0 for empty adapter.
 */
HWTEST_F(AudioCoreServiceUnitTest, GetStreamPropInfoSize_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    uint32_t size = audioCoreService->GetStreamPropInfoSize("nonexistent", "nonexistent_pipe");
    EXPECT_EQ(size, 0);
}

/**
 * @tc.name   : Test AudioCoreService::ParsePreferredInputDeviceHistory
 * @tc.number : ParsePreferredInputDeviceHistory_001
 * @tc.desc   : Test ParsePreferredInputDeviceHistory with nullptr returns empty string.
 */
HWTEST_F(AudioCoreServiceUnitTest, ParsePreferredInputDeviceHistory_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    std::string result = audioCoreService->ParsePreferredInputDeviceHistory(nullptr);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name   : Test AudioCoreService::ParsePreferredInputDeviceHistory
 * @tc.number : ParsePreferredInputDeviceHistory_002
 * @tc.desc   : Test ParsePreferredInputDeviceHistory with valid stream desc returns non-empty string.
 */
HWTEST_F(AudioCoreServiceUnitTest, ParsePreferredInputDeviceHistory_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 12345;
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_MIC;
    std::string result = audioCoreService->ParsePreferredInputDeviceHistory(streamDesc);
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("12345"), std::string::npos);
}

/**
 * @tc.name   : Test AudioCoreService::InVideoCommFastBlockList
 * @tc.number : InVideoCommFastBlockList_001
 * @tc.desc   : Test InVideoCommFastBlockList returns false when callback is null.
 */
HWTEST_F(AudioCoreServiceUnitTest, InVideoCommFastBlockList_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    bool ret = audioCoreService->InVideoCommFastBlockList("com.test.bundle");
    EXPECT_FALSE(ret);
}

/**
 * @tc.name   : Test AudioCoreService::IsDistributeServiceOnline
 * @tc.number : IsDistributeServiceOnline_001
 * @tc.desc   : Test IsDistributeServiceOnline returns false when deviceStatusListener_ is null.
 */
HWTEST_F(AudioCoreServiceUnitTest, IsDistributeServiceOnline_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);
    bool ret = audioCoreService->IsDistributeServiceOnline();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_010
 * @tc.desc  : Test udioCapturerSession::ReloadSourceForEffect()
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_010, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    AudioEffectPropertyArray oldPropertyArray;
    AudioEffectPropertyArray newPropertyArray;

    oldPropertyArray.property = {{"record", "PNR"}, {"voip_up", "PNR"}};
    newPropertyArray.property = {{"record", "PNR"}, {"voip_up", "PNR"}};

    audioCoreService->isMicRefFeatureEnable_ = true;
    audioCoreService->normalSourceOpened_ = SOURCE_TYPE_VOICE_COMMUNICATION;

    audioCoreService->ReloadSourceForEffect(oldPropertyArray, newPropertyArray);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_011
 * @tc.desc  : Test udioCapturerSession::ReloadSourceForEffect()
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_011, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    AudioEffectPropertyArray oldPropertyArray;
    AudioEffectPropertyArray newPropertyArray;

    oldPropertyArray.property = {{"record", "PNR"}, {"voip_up", "PNR"}};
    newPropertyArray.property = {{"record", "ABC"}, {"voip_up", "PNR"}};

    audioCoreService->isMicRefFeatureEnable_ = true;
    audioCoreService->normalSourceOpened_ = SOURCE_TYPE_VOICE_COMMUNICATION;

    audioCoreService->ReloadSourceForEffect(oldPropertyArray, newPropertyArray);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_012
 * @tc.desc  : Test udioCapturerSession::ReloadSourceForEffect()
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_012, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    AudioEffectPropertyArray oldPropertyArray;
    AudioEffectPropertyArray newPropertyArray;

    oldPropertyArray.property = {{"record", "PNR"}, {"voip_up", "PNR"}};
    newPropertyArray.property = {{"record", "ABC"}, {"voip_up", "ABC"}};

    audioCoreService->isMicRefFeatureEnable_ = true;
    audioCoreService->normalSourceOpened_ = SOURCE_TYPE_VOICE_COMMUNICATION;

    audioCoreService->ReloadSourceForEffect(oldPropertyArray, newPropertyArray);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_013
 * @tc.desc  : Test udioCapturerSession::ReloadSourceForDeviceChange()
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_013, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    AudioDeviceDescriptor inputDevice;
    AudioDeviceDescriptor outputDevice;
    std::string caller;

    audioCoreService->isEcFeatureEnable_ = true;
    audioCoreService->normalSourceOpened_ = SOURCE_TYPE_WAKEUP;

    audioCoreService->ReloadSourceForDeviceChange(inputDevice, outputDevice, caller);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_014
 * @tc.desc  : Test udioCapturerSession::ReloadSourceForDeviceChange()
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_014, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    AudioDeviceDescriptor inputDevice;
    AudioDeviceDescriptor outputDevice;
    std::string caller;

    audioCoreService->isEcFeatureEnable_ = true;
    audioCoreService->normalSourceOpened_ = SOURCE_TYPE_MIC;

    audioCoreService->ReloadSourceForDeviceChange(inputDevice, outputDevice, caller);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_015
 * @tc.desc  : Test udioCapturerSession::ReloadSourceForDeviceChange()
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_015, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    AudioDeviceDescriptor inputDevice;
    AudioDeviceDescriptor outputDevice;
    std::string caller;

    audioCoreService->isEcFeatureEnable_ = true;
    audioCoreService->normalSourceOpened_ = SOURCE_TYPE_VOICE_COMMUNICATION;

    audioCoreService->ReloadSourceForDeviceChange(inputDevice, outputDevice, caller);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_016
 * @tc.desc  : Test udioCapturerSession::ReloadSourceForDeviceChange()
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_016, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    AudioDeviceDescriptor inputDevice;
    inputDevice.deviceType_ = DEVICE_TYPE_DEFAULT;
    AudioDeviceDescriptor outputDevice;
    std::string caller;

    audioCoreService->isEcFeatureEnable_ = true;
    audioCoreService->normalSourceOpened_ = SOURCE_TYPE_VOICE_COMMUNICATION;

    audioCoreService->ReloadSourceForDeviceChange(inputDevice, outputDevice, caller);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_017
 * @tc.desc  : Test AudioCapturerSession::IsVoipDeviceChanged()
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_017, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    AudioDeviceDescriptor inputDevice;
    AudioDeviceDescriptor outputDevice;

    auto ret = audioCoreService->IsVoipDeviceChanged(inputDevice, outputDevice);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_018
 * @tc.desc  : Test AudioCapturerSession::FillWakeupStreamPropInfo()
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_018, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    AudioStreamInfo streamInfo;
    std::shared_ptr<AdapterPipeInfo> pipeInfo = nullptr;
    AudioModuleInfo audioModuleInfo;

    auto ret = audioCoreService->FillWakeupStreamPropInfo(streamInfo, pipeInfo, audioModuleInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_019
 * @tc.desc  : Test AudioCapturerSession::FillWakeupStreamPropInfo()
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_019, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    AudioStreamInfo streamInfo;
    std::shared_ptr<AdapterPipeInfo> pipeInfo = std::make_shared<AdapterPipeInfo>();
    EXPECT_NE(pipeInfo, nullptr);
    AudioModuleInfo audioModuleInfo;

    auto ret = audioCoreService->FillWakeupStreamPropInfo(streamInfo, pipeInfo, audioModuleInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_020
 * @tc.desc  : Test AudioCapturerSession::GetInstance()
 */

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_021
 * @tc.desc  : Test AudioCapturerSession::OnCapturerSessionAdded()
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_021, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    SessionInfo sessionInfo;
    sessionInfo.sourceType = SOURCE_TYPE_MIC;
    sessionInfo.rate = 44100;
    sessionInfo.channels = 2;
    AudioStreamInfo streamInfo;
    audioCoreService->SetConfigParserFlag();

    auto &audioVolumeManager = AudioVolumeManager::GetInstance();
    audioVolumeManager.SetDefaultDeviceLoadFlag(true);


    uint64_t sessionID = 1;
    audioCoreService->OnCapturerSessionRemoved(sessionID);

    auto ret = audioCoreService->OnCapturerSessionAdded(sessionID, sessionInfo, streamInfo);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_022
 * @tc.desc  : Test AudioCapturerSession::OnCapturerSessionAdded()
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_022, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    SessionInfo sessionInfo;
    AudioStreamInfo streamInfo;

    uint64_t sessionID = 1;

    auto ret = audioCoreService->OnCapturerSessionAdded(sessionID, sessionInfo, streamInfo);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_023
 * @tc.desc  : Test AudioCapturerSession::SetWakeUpAudioCapturerFromAudioServer()
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_023, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    AudioStreamInfo streamInfo;
    AudioProcessConfig config;
    config.streamInfo = streamInfo;

    auto ret = audioCoreService->SetWakeUpAudioCapturerFromAudioServer(config);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_025
 * @tc.desc  : Test ReloadSourceForDeviceChange() for valid source and device
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_025, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    AudioDeviceDescriptor inputDevice;
    inputDevice.deviceType_ = DEVICE_TYPE_MIC;
    AudioDeviceDescriptor outputDevice;
    std::string caller = "testCase";

    const uint64_t testSessionId = 99;
    audioCoreService->isEcFeatureEnable_ = true;
    audioCoreService->normalSourceOpened_ = SOURCE_TYPE_MIC;
    audioCoreService->sessionIdUsedToOpenSource_ = testSessionId;

    audioCoreService->ReloadSourceForDeviceChange(inputDevice, outputDevice, caller);
    EXPECT_EQ(audioCoreService->GetOpenedNormalSourceSessionId(), testSessionId);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_026
 * @tc.desc  : Test ReloadSourceForDeviceChange() for valid source and device
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_026, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    const uint64_t testSessionId = 12345;
    AudioStreamInfo streamInfo;
    SessionInfo sessionInfo;
    sessionInfo.sourceType = SOURCE_TYPE_MIC;
    audioCoreService->OnCapturerSessionAdded(testSessionId, sessionInfo, streamInfo);
    audioCoreService->normalSourceOpened_ = SOURCE_TYPE_MIC;

    sessionInfo.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    audioCoreService->OnCapturerSessionAdded(testSessionId + 1, sessionInfo, streamInfo);

    SessionOperation operation = SESSION_OPERATION_START;
    auto ret = audioCoreService->ReloadCaptureSession(testSessionId + 1, operation);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_027
 * @tc.desc  : Test ReloadSourceForDeviceChange() for valid source and device
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_027, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    const uint64_t testSessionId = 12345;
    AudioStreamInfo streamInfo;
    SessionInfo sessionInfo;
    sessionInfo.sourceType = SOURCE_TYPE_MIC;
    audioCoreService->OnCapturerSessionAdded(testSessionId, sessionInfo, streamInfo);

    sessionInfo.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    audioCoreService->OnCapturerSessionAdded(testSessionId + 1, sessionInfo, streamInfo);
    audioCoreService->normalSourceOpened_ = SOURCE_TYPE_VOICE_RECOGNITION;

    SessionOperation operation = SESSION_OPERATION_PAUSE;
    auto ret = audioCoreService->ReloadCaptureSession(testSessionId + 1, operation);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_028
 * @tc.desc  : Test ReloadSourceForDeviceChange() for valid source and device
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_028, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    const uint64_t testSessionId = 12345;
    AudioStreamInfo streamInfo;
    SessionInfo sessionInfo;
    sessionInfo.sourceType = SOURCE_TYPE_MIC;
    audioCoreService->OnCapturerSessionAdded(testSessionId, sessionInfo, streamInfo);

    sessionInfo.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    audioCoreService->OnCapturerSessionAdded(testSessionId + 1, sessionInfo, streamInfo);
    audioCoreService->normalSourceOpened_ = SOURCE_TYPE_VOICE_RECOGNITION;

    SessionOperation operation = SESSION_OPERATION_STOP;
    auto ret = audioCoreService->ReloadCaptureSession(testSessionId + 1, operation);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_029
 * @tc.desc  : Test ReloadSourceForDeviceChange() for valid source and device
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_029, TestSize.Level1)
{
    const uint64_t testSessionId = 12345;
    SessionOperation operation = SESSION_OPERATION_RELEASE;
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    int32_t result = audioCoreService->ReloadCaptureSession(testSessionId, operation);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_031
 * @tc.desc  : Test ReloadSourceForDeviceChange() for inputDeviceForReload default
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_030, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    AudioDeviceDescriptor inputDevice;
    inputDevice.deviceType_ = DEVICE_TYPE_MIC;
    AudioDeviceDescriptor outputDevice;
    std::string caller = "testCase";

    const uint64_t testSessionId = 99;
    audioCoreService->isEcFeatureEnable_ = true;
    audioCoreService->normalSourceOpened_ = SOURCE_TYPE_MIC;
    audioCoreService->sessionIdUsedToOpenSource_ = testSessionId;
    audioCoreService->inputDeviceForReload_.deviceType_ = DEVICE_TYPE_DEFAULT;

    audioCoreService->ReloadSourceForDeviceChange(inputDevice, outputDevice, caller);
    EXPECT_EQ(audioCoreService->inputDeviceForReload_.deviceType_, DEVICE_TYPE_MIC);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_032
 * @tc.desc  : Test ReloadSourceForDeviceChange() for inputDeviceForReload_ valid
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_031, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    AudioDeviceDescriptor inputDevice;
    inputDevice.deviceType_ = DEVICE_TYPE_MIC;
    AudioDeviceDescriptor outputDevice;
    std::string caller = "testCase";

    const uint64_t testSessionId = 99;
    audioCoreService->isEcFeatureEnable_ = true;
    audioCoreService->normalSourceOpened_ = SOURCE_TYPE_MIC;
    audioCoreService->sessionIdUsedToOpenSource_ = testSessionId;
    audioCoreService->inputDeviceForReload_.deviceType_ = DEVICE_TYPE_MIC;

    audioCoreService->ReloadSourceForDeviceChange(inputDevice, outputDevice, caller);
    EXPECT_EQ(audioCoreService->inputDeviceForReload_.deviceType_, DEVICE_TYPE_MIC);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_032
 * @tc.desc  : Test ReloadCaptureSessionSoftLink
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_032, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto pipeManager = AudioPipeManager::GetPipeManager();
    EXPECT_NE(pipeManager, nullptr);

    auto pipeInfoOne = std::make_shared<AudioPipeInfo>();
    pipeInfoOne->name_ = "test_output_one";
    pipeInfoOne->adapterName_ = "test_one";
    pipeManager->AddAudioPipeInfo(pipeInfoOne);

    auto pipeInfoTwo = std::make_shared<AudioPipeInfo>();
    pipeInfoTwo->name_ = "test_output_two";
    pipeInfoTwo->adapterName_ = "test_two";
    pipeInfoTwo->pipeRole_ = AudioPipeRole::PIPE_ROLE_OUTPUT;
    std::shared_ptr<AudioStreamDescriptor> streamDescriptor = std::make_shared<AudioStreamDescriptor>();
    streamDescriptor->sessionId_ = 0;
    pipeInfoTwo->streamDescriptors_.push_back(streamDescriptor);
    pipeManager->AddAudioPipeInfo(pipeInfoTwo);

    auto pipeInfoThree = std::make_shared<AudioPipeInfo>();
    pipeInfoThree->name_ = "test_input_three";
    pipeInfoThree->adapterName_ = "primary";
    pipeInfoThree->pipeRole_ = AudioPipeRole::PIPE_ROLE_INPUT;
    pipeInfoThree->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    std::shared_ptr<AudioStreamDescriptor> streamDescriptorOne = std::make_shared<AudioStreamDescriptor>();
    streamDescriptorOne->sessionId_ = 1;
    pipeInfoThree->streamDescriptors_.push_back(streamDescriptorOne);
    std::shared_ptr<AudioStreamDescriptor> streamDescriptorTwo = std::make_shared<AudioStreamDescriptor>();
    streamDescriptorTwo->sessionId_ = 2;
    pipeInfoThree->streamDescriptors_.push_back(streamDescriptorTwo);
    std::shared_ptr<AudioStreamDescriptor> streamDescriptorThree = std::make_shared<AudioStreamDescriptor>();
    streamDescriptorThree->sessionId_ = 3;
    pipeInfoThree->streamDescriptors_.push_back(streamDescriptorThree);
    pipeInfoThree->softLinkFlag_ = true;
    pipeManager->AddAudioPipeInfo(pipeInfoThree);

    auto ret = audioCoreService->ReloadCaptureSessionSoftLink();
    EXPECT_EQ(ret, SUCCESS);

    auto ioRet = AudioIOHandleMap::GetInstance().ClosePortAndEraseIOHandle("test");
    EXPECT_NE(ioRet, SUCCESS);

    auto pipeRet = pipeManager->GetUnusedRecordPipe();
    EXPECT_EQ(pipeRet.size(), 0);
    pipeManager->curPipeList_.clear();
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_033
 * @tc.desc  : Test AI pipe/pipe role is input
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_033, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> incommingPipe = std::make_shared<AudioPipeInfo>();
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;
    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipe->routeFlag_ = AUDIO_INPUT_FLAG_AI;
    pipeList.push_back(pipe);

    uint32_t sessionId = 1;
    AudioStreamDescriptor runningSessionInfo = {};
    bool hasSession = false;
    bool result = audioCoreService->HandleIndependentInputpipe(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_034
 * @tc.desc  : Test AI pipe/pipe role is not input
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_034, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> incommingPipe = std::make_shared<AudioPipeInfo>();
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;
    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipe->routeFlag_ = AUDIO_INPUT_FLAG_AI;
    pipeList.push_back(pipe);

    uint32_t sessionId = 1;
    AudioStreamDescriptor runningSessionInfo = {};
    bool hasSession = false;
    bool result = audioCoreService->HandleIndependentInputpipe(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_035
 * @tc.desc  : Test AI pipe is null
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_035, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> incommingPipe = std::make_shared<AudioPipeInfo>();
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;
    auto pipe = std::make_shared<AudioPipeInfo>();

    uint32_t sessionId = 1;
    AudioStreamDescriptor runningSessionInfo = {};
    bool hasSession = false;
    bool result = audioCoreService->HandleIndependentInputpipe(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_036
 * @tc.desc  : Test pipe list is null
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_036, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;
    uint32_t sessionId = 1;
    AudioStreamDescriptor runningSessionInfo = {};
    bool hasSession = false;
    bool result = audioCoreService->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
    EXPECT_EQ(hasSession, false);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_037
 * @tc.desc  : Test pipe is out or none
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_037, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;
    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipeList.push_back(pipe);

    uint32_t sessionId = 1;
    AudioStreamDescriptor runningSessionInfo = {};
    bool hasSession = false;
    bool result = audioCoreService->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
    EXPECT_EQ(hasSession, false);

    pipeList.clear();
    auto pipenew = std::make_shared<AudioPipeInfo>();
    pipenew->pipeRole_ = PIPE_ROLE_NONE;
    pipeList.push_back(pipenew);
    result = audioCoreService->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
    EXPECT_EQ(hasSession, false);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_038
 * @tc.desc  : Test routerflag is AI or Fast
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_038, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;
    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipe->routeFlag_ = AUDIO_INPUT_FLAG_AI;
    pipeList.push_back(pipe);

    uint32_t sessionId = 1;
    AudioStreamDescriptor runningSessionInfo = {};
    bool hasSession = false;
    bool result = audioCoreService->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
    EXPECT_EQ(hasSession, false);

    pipeList.clear();
    auto pipenew = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipe->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    pipeList.push_back(pipenew);
    result = audioCoreService->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
    EXPECT_EQ(hasSession, false);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_039
 * @tc.desc  : Test sessionid is same
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_039, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;
    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipe->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;

    uint32_t sessionId = 1;
    auto stream = std::make_shared<AudioStreamDescriptor>();
    stream->sessionId_ = sessionId;
    pipe->streamDescriptors_.push_back(stream);
    pipeList.push_back(pipe);

    // sessionId is same
    AudioStreamDescriptor runningSessionInfo = {};
    bool hasSession = false;
    bool result = audioCoreService->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
    EXPECT_EQ(hasSession, false);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_040
 * @tc.desc  : Test stream is null
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_040, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;
    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipe->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;

    uint32_t sessionId = 1;
    auto stream = std::make_shared<AudioStreamDescriptor>();
    stream->sessionId_ = 2;
    pipe->streamDescriptors_.push_back(stream);
    pipeList.push_back(pipe);

    // stream is null
    AudioStreamDescriptor runningSessionInfo = {};
    bool hasSession = false;
    bool result = audioCoreService->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
    EXPECT_EQ(hasSession, false);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_045
 * @tc.desc  : Test FindRunningNormalSession pipe is null
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_045, TestSize.Level1) {
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    AudioStreamDescriptor runningSessionInfo;
    bool result = audioCoreService->FindRunningNormalSession(1, runningSessionInfo);

    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_046
 * @tc.desc  : Test FindRunningNormalSession pipe is output
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_046, TestSize.Level1) {
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 1;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_VOICE_MESSAGE;
    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->id_ = 1;
    pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_AI;
    pipeInfo->name_ = "AIPipe";
    pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;

    pipeInfo->streamDescriptors_.push_back(streamDesc);

    auto pipeManager = AudioPipeManager::GetPipeManager();
    pipeManager->AddAudioPipeInfo(pipeInfo);

    AudioStreamDescriptor runningSessionInfo;
    bool result = audioCoreService->FindRunningNormalSession(1, runningSessionInfo);

    EXPECT_FALSE(result);
    pipeManager->curPipeList_.clear();
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_047
 * @tc.desc  : Test FindRunningNormalSession AI valid pipe
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_047, TestSize.Level1) {
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 1;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_VOICE_MESSAGE;
    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_TRANSCRIPTION;

    auto streamDescSecond = std::make_shared<AudioStreamDescriptor>();
    streamDescSecond->sessionId_ = 2;
    streamDescSecond->rendererInfo_.streamUsage = STREAM_USAGE_VOICE_MESSAGE;
    streamDescSecond->streamStatus_ = STREAM_STATUS_STARTED;
    streamDescSecond->capturerInfo_.sourceType = SOURCE_TYPE_UNPROCESSED;

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->id_ = 1;
    pipeInfo->routeFlag_ = (uint32_t)AUDIO_INPUT_FLAG_AI;
    pipeInfo->name_ = "AIPipe";
    pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;

    pipeInfo->streamDescriptors_.push_back(streamDesc);
    pipeInfo->streamDescriptors_.push_back(streamDescSecond);

    auto pipeManager = AudioPipeManager::GetPipeManager();
    pipeManager->AddAudioPipeInfo(pipeInfo);

    AudioStreamDescriptor runningSessionInfo;
    std::shared_ptr<std::map<SourceType, AudioSourceStrategyType>> sourceStrategyMap =
        std::make_shared<std::map<SourceType, AudioSourceStrategyType>>();
    sourceStrategyMap->emplace(SOURCE_TYPE_VOICE_TRANSCRIPTION,
        AudioSourceStrategyType {
        "AUDIO_INPUT_VOICE_TRANSCRIPTION",  // hdiSource
        "primary",               // adapterName
        "primary_input_AI",      // pipeName
        AUDIO_INPUT_FLAG_AI, // audioFlag
        1                       // priority
        });
    sourceStrategyMap->emplace(SOURCE_TYPE_UNPROCESSED,
        AudioSourceStrategyType {
        "AUDIO_INPUT_MIC_TYPE",  // hdiSource
        "primary",               // adapterName
        "primary_input_AI",     // pipeName
        AUDIO_INPUT_FLAG_AI, // audioFlag
        2                       // priority
        });
    AudioSourceStrategyData::GetInstance().SetSourceStrategyMap(sourceStrategyMap);
    bool result = audioCoreService->FindRunningNormalSession(1, runningSessionInfo);

    EXPECT_TRUE(result);
    pipeManager->curPipeList_.clear();
    AudioSourceStrategyData::GetInstance().GetSourceStrategyMap()->clear();
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_049
 * @tc.desc  : Test CompareIndependentxmlPriority sourcestrategy map is null
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_049, TestSize.Level1) {
    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->streamDescriptors_.push_back(std::make_shared<AudioStreamDescriptor>());
    bool hasSession = false;
    AudioStreamDescriptor runningSessionInfo;

    AudioSourceStrategyData::GetInstance().SetSourceStrategyMap(nullptr);
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    bool result = audioCoreService->CompareIndependentxmlPriority(pipe, 1, runningSessionInfo, hasSession);

    EXPECT_FALSE(result);
    EXPECT_FALSE(hasSession);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_052
 * @tc.desc  : Test CompareIndependentxmlPriority stream is null
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_052, TestSize.Level1) {
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    auto pipe = std::make_shared<AudioPipeInfo>();
    bool hasSession = false;
    AudioStreamDescriptor runningSessionInfo;
    bool result = audioCoreService->CompareIndependentxmlPriority(pipe, 1, runningSessionInfo, hasSession);

    EXPECT_FALSE(result);
    EXPECT_FALSE(hasSession);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_053
 * @tc.desc  : Test CompareIndependentxmlPriority sessionid is same
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_053, TestSize.Level1) {
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    auto pipe = std::make_shared<AudioPipeInfo>();
    auto stream = std::make_shared<AudioStreamDescriptor>();
    stream->sessionId_ = 1;
    pipe->streamDescriptors_.push_back(stream);
    bool hasSession = false;
    AudioStreamDescriptor runningSessionInfo;
    bool result = audioCoreService->CompareIndependentxmlPriority(pipe, 1, runningSessionInfo, hasSession);

    EXPECT_FALSE(result);
    EXPECT_FALSE(hasSession);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_054
 * @tc.desc  : Test CompareIndependentxmlPriority streamStatus_is not start
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_054, TestSize.Level1) {
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    auto pipe = std::make_shared<AudioPipeInfo>();
    auto firstStream = std::make_shared<AudioStreamDescriptor>();
    firstStream->streamStatus_ = STREAM_STATUS_STOPPED;
    firstStream->sessionId_ = 1;

    auto secondStream = std::make_shared<AudioStreamDescriptor>();
    secondStream->streamStatus_ = STREAM_STATUS_STOPPED;
    secondStream->sessionId_ = 2;

    pipe->streamDescriptors_.push_back(firstStream);
    pipe->streamDescriptors_.push_back(secondStream);

    bool hasSession = false;
    AudioStreamDescriptor runningSessionInfo;
    bool result = audioCoreService->CompareIndependentxmlPriority(pipe, 1, runningSessionInfo, hasSession);

    EXPECT_FALSE(result);
    EXPECT_FALSE(hasSession);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_055
 * @tc.desc  : Test CompareIndependentxmlPriority source strategy map can't find sourcetype
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_055, TestSize.Level1) {
    auto pipe = std::make_shared<AudioPipeInfo>();
    auto firstStream = std::make_shared<AudioStreamDescriptor>();
    firstStream->streamStatus_ = STREAM_STATUS_STARTED;
    firstStream->sessionId_ = 1;

    auto secondStream = std::make_shared<AudioStreamDescriptor>();
    secondStream->streamStatus_ = STREAM_STATUS_STARTED;
    secondStream->sessionId_ = 2;

    pipe->streamDescriptors_.push_back(firstStream);
    pipe->streamDescriptors_.push_back(secondStream);

    bool hasSession = false;
    AudioStreamDescriptor runningSessionInfo;

    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    bool result = audioCoreService->CompareIndependentxmlPriority(pipe, 2, runningSessionInfo, hasSession);

    EXPECT_FALSE(result);
    EXPECT_FALSE(hasSession);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_056
 * @tc.desc  : Test CompareIndependentxmlPriority enter check
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_056, TestSize.Level1) {
    auto pipe = std::make_shared<AudioPipeInfo>();
    auto stream1 = std::make_shared<AudioStreamDescriptor>();
    stream1->capturerInfo_.sourceType = SOURCE_TYPE_MIC;
    stream1->streamStatus_ = STREAM_STATUS_STARTED;
    stream1->sessionId_ = 1;
    pipe->streamDescriptors_.push_back(stream1);

    auto stream2 = std::make_shared<AudioStreamDescriptor>();
    stream2->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_CALL;
    stream2->streamStatus_ = STREAM_STATUS_STARTED;
    stream2->sessionId_ = 2;
    pipe->streamDescriptors_.push_back(stream2);

    bool hasSession = false;
    AudioStreamDescriptor runningSessionInfo;

    // create sourceStrategyMap, SOURCE_TYPE_VOICE_CALL has higher priority than SOURCE_TYPE_MIC
    auto sourceStrategyMap = std::make_shared<std::map<SourceType, AudioSourceStrategyType>>();
    sourceStrategyMap->emplace(SOURCE_TYPE_MIC,
        AudioSourceStrategyType {
        "AUDIO_INPUT_MIC_TYPE",  // hdiSource
        "primary",               // adapterName
        "primary_input",         // pipeName
        AUDIO_INPUT_FLAG_NORMAL, // audioFlag
        5                       // priority
        });
    sourceStrategyMap->emplace(SOURCE_TYPE_VOICE_CALL,
        AudioSourceStrategyType {
        "AUDIO_INPUT_MIC_TYPE",  // hdiSource
        "primary",               // adapterName
        "primary_input",         // pipeName
        AUDIO_INPUT_FLAG_NORMAL, // audioFlag
        6                       // priority
        });

    AudioSourceStrategyData::GetInstance().SetSourceStrategyMap(sourceStrategyMap);
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    bool result = audioCoreService->CompareIndependentxmlPriority(pipe, stream2->sessionId_, runningSessionInfo,
        hasSession);

    EXPECT_TRUE(result);
    EXPECT_TRUE(hasSession);
    EXPECT_EQ(runningSessionInfo.sessionId_, stream1->sessionId_);
    AudioSourceStrategyData::GetInstance().GetSourceStrategyMap()->clear();
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_062
 * @tc.desc  : Test ReloadSourceForDeviceChange() for valid source and device
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_062, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> incommingPipe = std::make_shared<AudioPipeInfo>();
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;

    pipeList.push_back(nullptr);

    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipe->routeFlag_ = AUDIO_INPUT_FLAG_AI;
    pipeList.push_back(pipe);

    uint32_t sessionId = 0;
    AudioStreamDescriptor runningSessionInfo;
    bool hasSession = false;
    bool result = audioCoreService->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);

    EXPECT_FALSE(result);
    EXPECT_FALSE(hasSession);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_063
 * @tc.desc  : Test ReloadSourceForDeviceChange() for valid source and device
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_063, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;

    pipeList.push_back(nullptr);

    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipe->routeFlag_ = AUDIO_INPUT_FLAG_AI;
    pipeList.push_back(pipe);

    uint32_t sessionId = 0;
    AudioStreamDescriptor runningSessionInfo;
    bool hasSession = false;
    bool result = audioCoreService->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);

    EXPECT_FALSE(result);
    EXPECT_FALSE(hasSession);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_064
 * @tc.desc  : Test ReloadSourceForDeviceChange() for valid source and device
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_064, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;

    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipe->adapterName_ = ADAPTER_TYPE_VA;
    pipeList.push_back(pipe);

    uint32_t sessionId = 0;
    AudioStreamDescriptor runningSessionInfo;
    bool hasSession = false;
    bool result = audioCoreService->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);

    EXPECT_FALSE(result);
    EXPECT_FALSE(hasSession);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_065
 * @tc.desc  : Test ReloadSourceForDeviceChange() for valid source and device
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_065, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;

    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipe->adapterName_ = ADAPTER_TYPE_VA;
    pipeList.push_back(pipe);

    uint32_t sessionId = 0;
    AudioStreamDescriptor runningSessionInfo;
    bool hasSession = false;
    bool result = audioCoreService->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);

    EXPECT_FALSE(result);
    EXPECT_FALSE(hasSession);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_066
 * @tc.desc  : Test ReloadSourceForDeviceChange() for valid source and device
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_066, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;

    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipe->adapterName_ = "test_one";
    pipeList.push_back(pipe);

    uint32_t sessionId = 0;
    AudioStreamDescriptor runningSessionInfo;
    bool hasSession = false;
    bool result = audioCoreService->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);

    EXPECT_FALSE(result);
    EXPECT_FALSE(hasSession);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_067
 * @tc.desc  : Test ReloadSourceForDeviceChange() for valid source and device
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_067, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;

    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipe->adapterName_ = "test_one";
    pipeList.push_back(pipe);

    uint32_t sessionId = 0;
    AudioStreamDescriptor runningSessionInfo;
    bool hasSession = false;
    bool result = audioCoreService->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);

    EXPECT_FALSE(result);
    EXPECT_FALSE(hasSession);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_068
 * @tc.desc  : Test ReloadCaptureSession()
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_068, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    audioCoreService->sessionWithInputPipeRouteFlag_[12345] = AUDIO_FLAG_NONE;
    int32_t ret = audioCoreService->ReloadCaptureSession(12345, SESSION_OPERATION_START);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_069
 * @tc.desc  : Test ReloadCaptureSession()
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_069, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    int32_t ret = audioCoreService->ReloadCaptureSession(12345, SESSION_OPERATION_START);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_070
 * @tc.desc  : Test ReloadSourceForDeviceChange() for valid source and device
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_070, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;

    pipeList.push_back(nullptr);

    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipe->routeFlag_ = AUDIO_INPUT_FLAG_UNPROCESS;
    pipeList.push_back(pipe);

    uint32_t sessionId = 0;
    AudioStreamDescriptor runningSessionInfo;
    bool hasSession = false;
    bool result = audioCoreService->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);

    EXPECT_FALSE(result);
    EXPECT_FALSE(hasSession);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_071
 * @tc.desc  : Test ReloadSourceForDeviceChange() for valid source and device
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_071, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;

    pipeList.push_back(nullptr);

    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipe->routeFlag_ = AUDIO_INPUT_FLAG_ULTRASONIC;
    pipeList.push_back(pipe);

    uint32_t sessionId = 0;
    AudioStreamDescriptor runningSessionInfo;
    bool hasSession = false;
    bool result = audioCoreService->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);

    EXPECT_FALSE(result);
    EXPECT_FALSE(hasSession);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_072
 * @tc.desc  : Test ReloadSourceForDeviceChange() for valid source and device
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_072, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;

    pipeList.push_back(nullptr);

    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipe->routeFlag_ = AUDIO_INPUT_FLAG_VOICE_RECOGNITION;
    pipeList.push_back(pipe);

    uint32_t sessionId = 0;
    AudioStreamDescriptor runningSessionInfo;
    bool hasSession = false;
    bool result = audioCoreService->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);

    EXPECT_FALSE(result);
    EXPECT_FALSE(hasSession);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_073
 * @tc.desc  : Test AUDIO_INPUT_FLAG_UNPROCESS pipe/pipe role is not input
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_073, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> incommingPipe = std::make_shared<AudioPipeInfo>();
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;
    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipe->routeFlag_ = AUDIO_INPUT_FLAG_UNPROCESS;
    pipeList.push_back(pipe);

    uint32_t sessionId = 1;
    AudioStreamDescriptor runningSessionInfo = {};
    bool hasSession = false;
    bool result = audioCoreService->HandleIndependentInputpipe(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_074
 * @tc.desc  : Test AUDIO_INPUT_FLAG_ULTRASONIC pipe/pipe role is not input
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_074, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> incommingPipe = std::make_shared<AudioPipeInfo>();
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;
    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipe->routeFlag_ = AUDIO_INPUT_FLAG_ULTRASONIC;
    pipeList.push_back(pipe);

    uint32_t sessionId = 1;
    AudioStreamDescriptor runningSessionInfo = {};
    bool hasSession = false;
    bool result = audioCoreService->HandleIndependentInputpipe(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_075
 * @tc.desc  : Test AUDIO_INPUT_FLAG_VOICE_RECOGNITION pipe/pipe role is not input
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_075, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> incommingPipe = std::make_shared<AudioPipeInfo>();
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;
    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipe->routeFlag_ = AUDIO_INPUT_FLAG_VOICE_RECOGNITION;
    pipeList.push_back(pipe);

    uint32_t sessionId = 1;
    AudioStreamDescriptor runningSessionInfo = {};
    bool hasSession = false;
    bool result = audioCoreService->HandleIndependentInputpipe(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_076
 * @tc.desc  : Test routerflag is AUDIO_INPUT_FLAG_UNPROCESS or Fast
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_076, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;
    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipe->routeFlag_ = AUDIO_INPUT_FLAG_UNPROCESS;
    pipeList.push_back(pipe);

    uint32_t sessionId = 1;
    AudioStreamDescriptor runningSessionInfo = {};
    bool hasSession = false;
    bool result = audioCoreService->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
    EXPECT_EQ(hasSession, false);

    pipeList.clear();
    auto pipenew = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipe->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    pipeList.push_back(pipenew);
    result = audioCoreService->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
    EXPECT_EQ(hasSession, false);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_077
 * @tc.desc  : Test routerflag is AUDIO_INPUT_FLAG_ULTRASONIC or Fast
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_077, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;
    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipe->routeFlag_ = AUDIO_INPUT_FLAG_ULTRASONIC;
    pipeList.push_back(pipe);

    uint32_t sessionId = 1;
    AudioStreamDescriptor runningSessionInfo = {};
    bool hasSession = false;
    bool result = audioCoreService->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
    EXPECT_EQ(hasSession, false);

    pipeList.clear();
    auto pipenew = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipe->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    pipeList.push_back(pipenew);
    result = audioCoreService->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
    EXPECT_EQ(hasSession, false);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_078
 * @tc.desc  : Test routerflag is AUDIO_INPUT_FLAG_VOICE_RECOGNITION or Fast
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_078, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;
    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipe->routeFlag_ = AUDIO_INPUT_FLAG_VOICE_RECOGNITION;
    pipeList.push_back(pipe);

    uint32_t sessionId = 1;
    AudioStreamDescriptor runningSessionInfo = {};
    bool hasSession = false;
    bool result = audioCoreService->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
    EXPECT_EQ(hasSession, false);

    pipeList.clear();
    auto pipenew = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipe->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    pipeList.push_back(pipenew);
    result = audioCoreService->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
    EXPECT_EQ(hasSession, false);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_079
 * @tc.desc  : Test ReloadSourceForDeviceChange() for valid source and device
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_079, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;

    pipeList.push_back(nullptr);

    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipe->routeFlag_ = AUDIO_INPUT_FLAG_RAW_AI;
    pipeList.push_back(pipe);

    uint32_t sessionId = 0;
    AudioStreamDescriptor runningSessionInfo;
    bool hasSession = false;
    bool result = audioCoreService->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);

    EXPECT_FALSE(result);
    EXPECT_FALSE(hasSession);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_080
 * @tc.desc  : Test AUDIO_INPUT_FLAG_RAW_AI pipe/pipe role is not input
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_080, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::shared_ptr<AudioPipeInfo> incommingPipe = std::make_shared<AudioPipeInfo>();
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;
    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipe->routeFlag_ = AUDIO_INPUT_FLAG_RAW_AI;
    pipeList.push_back(pipe);

    uint32_t sessionId = 1;
    AudioStreamDescriptor runningSessionInfo = {};
    bool hasSession = false;
    bool result = audioCoreService->HandleIndependentInputpipe(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_081
 * @tc.desc  : Test routerflag is AUDIO_INPUT_FLAG_RAW_AI or Fast
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_081, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;
    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipe->routeFlag_ = AUDIO_INPUT_FLAG_RAW_AI;
    pipeList.push_back(pipe);

    uint32_t sessionId = 1;
    AudioStreamDescriptor runningSessionInfo = {};
    bool hasSession = false;
    bool result = audioCoreService->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
    EXPECT_EQ(hasSession, false);

    pipeList.clear();
    auto pipenew = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipe->routeFlag_ = AUDIO_OUTPUT_FLAG_FAST;
    pipeList.push_back(pipenew);
    result = audioCoreService->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
    EXPECT_EQ(hasSession, false);
}

/**
 * @tc.name  : Test AudioCapturerSession.
 * @tc.number: AudioCapturerSession_082
 * @tc.desc  : Test routerflag is AUDIO_INPUT_FLAG_LIVE.
 */
HWTEST_F(AudioCoreServiceUnitTest, AudioCapturerSession_082, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList;
    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->pipeRole_ = PIPE_ROLE_INPUT;
    pipe->routeFlag_ = AUDIO_INPUT_FLAG_LIVE;
    pipeList.push_back(pipe);

    uint32_t sessionId = 1;
    AudioStreamDescriptor runningSessionInfo = {};
    bool hasSession = false;
    bool result = audioCoreService->HandleNormalInputPipes(pipeList, sessionId, runningSessionInfo, hasSession);
    EXPECT_EQ(result, false);
    EXPECT_EQ(hasSession, false);
}

/**
 * @tc.name  : Test UpdateArmModuleInfo API with valid parameters
 * @tc.type  : FUNC
 * @tc.number: UpdateArmModuleInfo_001
 * @tc.desc  : Test UpdateArmModuleInfo interface with valid device descriptor.
 */
HWTEST_F(AudioCoreServiceUnitTest, UpdateArmModuleInfo_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    // Create valid AudioDeviceDescriptor
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    DeviceStreamInfo streamInfo;
    streamInfo.samplingRate = {AudioSamplingRate::SAMPLE_RATE_48000};
    streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    std::list<DeviceStreamInfo> streamInfos = {streamInfo};
    deviceDesc->audioStreamInfo_ = streamInfos;

    AudioModuleInfo moduleInfo;
    moduleInfo.channels = "2";

    audioCoreService->UpdateArmModuleInfo(deviceDesc, moduleInfo);

    EXPECT_EQ(moduleInfo.rate, "48000");
    EXPECT_EQ(moduleInfo.format, "s16le");
    EXPECT_FALSE(moduleInfo.bufferSize.empty());
}

/**
 * @tc.name  : Test UpdateArmModuleInfo API with null device descriptor
 * @tc.type  : FUNC
 * @tc.number: UpdateArmModuleInfo_002
 * @tc.desc  : Test UpdateArmModuleInfo interface with null device descriptor.
 */
HWTEST_F(AudioCoreServiceUnitTest, UpdateArmModuleInfo_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();

    AudioModuleInfo moduleInfo;
    moduleInfo.rate = "44100";
    moduleInfo.format = "s16le";
    moduleInfo.channels = "2";

    // Save original values
    std::string originalRate = moduleInfo.rate;
    std::string originalFormat = moduleInfo.format;
    std::string originalChannels = moduleInfo.channels;

    // Pass null pointer, function should return directly without modifying moduleInfo
    audioCoreService->UpdateArmModuleInfo(nullptr, moduleInfo);

    EXPECT_EQ(moduleInfo.rate, originalRate);
    EXPECT_EQ(moduleInfo.format, originalFormat);
    EXPECT_EQ(moduleInfo.channels, originalChannels);
}

/**
 * @tc.name  : Test UpdateArmModuleInfo API with empty stream info
 * @tc.type  : FUNC
 * @tc.number: UpdateArmModuleInfo_003
 * @tc.desc  : Test UpdateArmModuleInfo interface with empty stream info.
 */
HWTEST_F(AudioCoreServiceUnitTest, UpdateArmModuleInfo_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();

    // Create AudioDeviceDescriptor without setting AudioStreamInfo
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();

    AudioModuleInfo moduleInfo;
    moduleInfo.rate = "44100";
    moduleInfo.format = "s16le";
    moduleInfo.channels = "2";

    // Save original values
    std::string originalRate = moduleInfo.rate;
    std::string originalFormat = moduleInfo.format;
    std::string originalChannels = moduleInfo.channels;

    audioCoreService->UpdateArmModuleInfo(deviceDesc, moduleInfo);

    // Since AudioStreamInfo is empty, function should return directly without modifying moduleInfo
    EXPECT_EQ(moduleInfo.rate, originalRate);
    EXPECT_EQ(moduleInfo.format, originalFormat);
    EXPECT_EQ(moduleInfo.channels, originalChannels);
}

/**
 * @tc.name  : Test UpdateArmModuleInfo API with empty sampling rate
 * @tc.type  : FUNC
 * @tc.number: UpdateArmModuleInfo_004
 * @tc.desc  : Test UpdateArmModuleInfo interface with empty sampling rate.
 */
HWTEST_F(AudioCoreServiceUnitTest, UpdateArmModuleInfo_004, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();

    // Create AudioDeviceDescriptor but set empty samplingRate
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    DeviceStreamInfo streamInfo;
    // Don't set samplingRate, keep it empty
    streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    std::list<DeviceStreamInfo> streamInfos = {streamInfo};
    deviceDesc->audioStreamInfo_ = streamInfos;

    AudioModuleInfo moduleInfo;
    moduleInfo.rate = "44100";
    moduleInfo.format = "s16le";
    moduleInfo.channels = "2";

    // Save original values
    std::string originalRate = moduleInfo.rate;
    std::string originalFormat = moduleInfo.format;
    std::string originalChannels = moduleInfo.channels;

    audioCoreService->UpdateArmModuleInfo(deviceDesc, moduleInfo);

    // Since samplingRate is empty, function should return directly without modifying moduleInfo
    EXPECT_EQ(moduleInfo.rate, originalRate);
    EXPECT_EQ(moduleInfo.format, originalFormat);
    EXPECT_EQ(moduleInfo.channels, originalChannels);
}

/**
 * @tc.name  : Test UpdateArmModuleInfo API with different sampling rates and formats
 * @tc.type  : FUNC
 * @tc.number: UpdateArmModuleInfo_005
 * @tc.desc  : Test UpdateArmModuleInfo interface with different sampling rates and formats.
 */
HWTEST_F(AudioCoreServiceUnitTest, UpdateArmModuleInfo_005, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();

    // Test with different sampling rate
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    DeviceStreamInfo streamInfo;
    streamInfo.samplingRate = {AudioSamplingRate::SAMPLE_RATE_44100};  // Different sampling rate
    streamInfo.format = AudioSampleFormat::SAMPLE_S24LE;
    std::list<DeviceStreamInfo> streamInfos = {streamInfo};
    deviceDesc->audioStreamInfo_ = streamInfos;

    AudioModuleInfo moduleInfo;
    moduleInfo.channels = "1";  // Different channel count

    audioCoreService->UpdateArmModuleInfo(deviceDesc, moduleInfo);

    EXPECT_EQ(moduleInfo.rate, "44100");
    EXPECT_EQ(moduleInfo.format, "s24le");
    EXPECT_FALSE(moduleInfo.bufferSize.empty());

    // Verify buffer size calculation is correct (44100 * 1 * 3 * 20 / 1000 = 2646)
    uint32_t expectedBufferSize = 44100 * 1 * 3 * 20 / 1000;
    EXPECT_EQ(moduleInfo.bufferSize, std::to_string(expectedBufferSize));
}

/**
 * @tc.name  : Test UpdateArmModuleInfo API with buffer size calculation
 * @tc.type  : FUNC
 * @tc.number: UpdateArmModuleInfo_006
 * @tc.desc  : Test UpdateArmModuleInfo interface buffer size calculation.
 */
HWTEST_F(AudioCoreServiceUnitTest, UpdateArmModuleInfo_006, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();

    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    DeviceStreamInfo streamInfo;
    streamInfo.samplingRate = {AudioSamplingRate::SAMPLE_RATE_96000};
    streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    std::list<DeviceStreamInfo> streamInfos = {streamInfo};
    deviceDesc->audioStreamInfo_ = streamInfos;

    AudioModuleInfo moduleInfo;
    moduleInfo.channels = "4";

    audioCoreService->UpdateArmModuleInfo(deviceDesc, moduleInfo);

    EXPECT_EQ(moduleInfo.rate, "96000");
    EXPECT_EQ(moduleInfo.format, "s16le");
    EXPECT_FALSE(moduleInfo.bufferSize.empty());
}

/**
 * @tc.name  : Test UpdateArmModuleInfo API with invalid original buffer size
 * @tc.type  : FUNC
 * @tc.number: UpdateArmModuleInfo_007
 * @tc.desc  : Verify function returns early when original bufferSize is invalid.
 */
HWTEST_F(AudioCoreServiceUnitTest, UpdateArmModuleInfo_007, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    DeviceStreamInfo streamInfo;
    streamInfo.samplingRate = {AudioSamplingRate::SAMPLE_RATE_48000};
    streamInfo.format = AudioSampleFormat::SAMPLE_S24LE;
    deviceDesc->audioStreamInfo_ = {streamInfo};

    AudioModuleInfo moduleInfo;
    moduleInfo.rate = "44100";
    moduleInfo.format = "s16le";
    moduleInfo.channels = "2";
    moduleInfo.bufferSize = "invalid_buffer_size";

    audioCoreService->UpdateArmModuleInfo(deviceDesc, moduleInfo);

    EXPECT_EQ(moduleInfo.rate, "44100");
    EXPECT_EQ(moduleInfo.format, "s16le");
    EXPECT_EQ(moduleInfo.bufferSize, "invalid_buffer_size");
}

/**
 * @tc.name  : Test UpdateArmModuleInfo API with valid original params
 * @tc.type  : FUNC
 * @tc.number: UpdateArmModuleInfo_008
 * @tc.desc  : Verify original module buffer duration is applied after HAL rate/format update.
 */
HWTEST_F(AudioCoreServiceUnitTest, UpdateArmModuleInfo_008, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    DeviceStreamInfo streamInfo;
    streamInfo.samplingRate = {AudioSamplingRate::SAMPLE_RATE_48000};
    streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    deviceDesc->audioStreamInfo_ = {streamInfo};

    AudioModuleInfo moduleInfo;
    moduleInfo.rate = "48000";
    moduleInfo.format = "s16le";
    moduleInfo.channels = "2";
    moduleInfo.bufferSize = "1920";

    audioCoreService->UpdateArmModuleInfo(deviceDesc, moduleInfo);

    EXPECT_EQ(moduleInfo.rate, "48000");
    EXPECT_EQ(moduleInfo.format, "s16le");
    EXPECT_EQ(moduleInfo.bufferSize, "1920");
}

/**
 * @tc.name  : Test UpdateArmModuleInfo API with incomplete original params
 * @tc.type  : FUNC
 * @tc.number: UpdateArmModuleInfo_009
 * @tc.desc  : Verify default 20ms is used when original module params are incomplete.
 */
HWTEST_F(AudioCoreServiceUnitTest, UpdateArmModuleInfo_009, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    DeviceStreamInfo streamInfo;
    streamInfo.samplingRate = {AudioSamplingRate::SAMPLE_RATE_48000};
    streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    deviceDesc->audioStreamInfo_ = {streamInfo};

    AudioModuleInfo moduleInfo;
    moduleInfo.channels = "2";

    audioCoreService->UpdateArmModuleInfo(deviceDesc, moduleInfo);

    EXPECT_EQ(moduleInfo.rate, "48000");
    EXPECT_EQ(moduleInfo.format, "s16le");
    EXPECT_EQ(moduleInfo.bufferSize, "3840");
}

/**
 * @tc.name  : Test UpdateArmModuleInfo API with empty original rate
 * @tc.type  : FUNC
 * @tc.number: UpdateArmModuleInfo_010
 * @tc.desc  : Verify empty original rate triggers default 20ms branch.
 */
HWTEST_F(AudioCoreServiceUnitTest, UpdateArmModuleInfo_010, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    DeviceStreamInfo streamInfo;
    streamInfo.samplingRate = {AudioSamplingRate::SAMPLE_RATE_48000};
    streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    deviceDesc->audioStreamInfo_ = {streamInfo};

    AudioModuleInfo moduleInfo;
    moduleInfo.format = "s16le";
    moduleInfo.channels = "2";
    moduleInfo.bufferSize = "960";

    audioCoreService->UpdateArmModuleInfo(deviceDesc, moduleInfo);

    EXPECT_EQ(moduleInfo.rate, "48000");
    EXPECT_EQ(moduleInfo.format, "s16le");
    EXPECT_EQ(moduleInfo.bufferSize, "3840");
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: SetEcSamplingRateByModule_001
* @tc.desc  : Test GetEcSamplingRate interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, SetEcSamplingRateByModule_001, TestSize.Level1)
{
    std::string halName = DP_CLASS;
    std::shared_ptr<PipeStreamPropInfo> outModuleInfo = std::make_shared<PipeStreamPropInfo>();
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::string sRet;

    outModuleInfo->sampleRate_ = 41000;
    sRet = audioCoreService->GetEcSamplingRate(halName, outModuleInfo);
    EXPECT_EQ(sRet, "41000");

    audioCoreService->dpSinkModuleInfo_.rate = "48000";
    sRet = audioCoreService->GetEcSamplingRate(halName, outModuleInfo);
    EXPECT_EQ(sRet, "48000");

    halName = USB_CLASS;
    sRet = audioCoreService->GetEcSamplingRate(halName, outModuleInfo);
    EXPECT_EQ(sRet, "41000");

    audioCoreService->usbSinkModuleInfo_.rate = "48000";
    sRet = audioCoreService->GetEcSamplingRate(halName, outModuleInfo);
    EXPECT_EQ(sRet, "48000");

    halName = "TEST";
    audioCoreService->primaryMicModuleInfo_.rate = "40000";
    sRet = audioCoreService->GetEcSamplingRate(halName, outModuleInfo);
    EXPECT_EQ(sRet, "40000");
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: SetEcSamplingChannelsByModule_001
* @tc.desc  : Test GetEcChannels interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, SetEcSamplingChannelsByModule_001, TestSize.Level1)
{
    std::string halName = DP_CLASS;
    std::shared_ptr<PipeStreamPropInfo> outModuleInfo = std::make_shared<PipeStreamPropInfo>();
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::string sRet;

    outModuleInfo->channelLayout_ = CH_LAYOUT_STEREO;
    audioCoreService->dpSinkModuleInfo_.channels = "";
    sRet = audioCoreService->GetEcChannels(halName, outModuleInfo);
    EXPECT_EQ(sRet, "0");

    audioCoreService->dpSinkModuleInfo_.channels = "3";
    sRet = audioCoreService->GetEcChannels(halName, outModuleInfo);
    EXPECT_EQ(sRet, "3");

    halName = USB_CLASS;
    audioCoreService->usbSinkModuleInfo_.channels = "";
    sRet = audioCoreService->GetEcChannels(halName, outModuleInfo);
    EXPECT_EQ(sRet, "0");

    audioCoreService->usbSinkModuleInfo_.channels = "5";
    sRet = audioCoreService->GetEcChannels(halName, outModuleInfo);
    EXPECT_EQ(sRet, "5");

    halName = "TEST";
    sRet = audioCoreService->GetEcChannels(halName, outModuleInfo);
    EXPECT_EQ(sRet, "2");
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: SetEcSamplingFormatByModule_001
* @tc.desc  : Test GetEcFormat interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, SetEcSamplingFormatByModule_001, TestSize.Level1)
{
    std::string halName = DP_CLASS;
    std::shared_ptr<PipeStreamPropInfo> outModuleInfo = std::make_shared<PipeStreamPropInfo>();
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::string sRet;

    outModuleInfo->format_ = SAMPLE_S32LE;
    audioCoreService->dpSinkModuleInfo_.format = "";
    sRet = audioCoreService->GetEcFormat(halName, outModuleInfo);
    EXPECT_EQ(sRet, "s32le");

    audioCoreService->dpSinkModuleInfo_.format = "4";
    sRet = audioCoreService->GetEcFormat(halName, outModuleInfo);
    EXPECT_EQ(sRet, "4");

    halName = USB_CLASS;
    audioCoreService->usbSinkModuleInfo_.format = "";
    sRet = audioCoreService->GetEcFormat(halName, outModuleInfo);
    EXPECT_EQ(sRet, "s32le");

    audioCoreService->usbSinkModuleInfo_.format = "5";
    sRet = audioCoreService->GetEcFormat(halName, outModuleInfo);
    EXPECT_EQ(sRet, "5");

    halName = "TEST";
    audioCoreService->primaryMicModuleInfo_.format = "2";
    sRet = audioCoreService->GetEcFormat(halName, outModuleInfo);
    EXPECT_EQ(sRet, "2");
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: GetPipeNameByDeviceForEc_001
* @tc.desc  : Test GetPipeNameByDeviceForEc interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, GetPipeNameByDeviceForEc_001, TestSize.Level1)
{
    std::string role;
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::string sRet;

    sRet = audioCoreService->GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(sRet, PIPE_PRIMARY_OUTPUT);

    deviceType = DEVICE_TYPE_WIRED_HEADSET;
    sRet = audioCoreService->GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(sRet, PIPE_PRIMARY_OUTPUT);

    deviceType = DEVICE_TYPE_USB_HEADSET;
    sRet = audioCoreService->GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(sRet, PIPE_PRIMARY_OUTPUT);

    deviceType = DEVICE_TYPE_BLUETOOTH_SCO;
    sRet = audioCoreService->GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(sRet, PIPE_PRIMARY_OUTPUT);

    deviceType = DEVICE_TYPE_NEARLINK;
    sRet = audioCoreService->GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(sRet, PIPE_PRIMARY_OUTPUT);

    role = ROLE_SOURCE;
    deviceType = DEVICE_TYPE_WIRED_HEADSET;
    sRet = audioCoreService->GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(sRet, PIPE_PRIMARY_INPUT);

    deviceType = DEVICE_TYPE_USB_HEADSET;
    sRet = audioCoreService->GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(sRet, PIPE_PRIMARY_INPUT);

    deviceType = DEVICE_TYPE_BLUETOOTH_SCO;
    sRet = audioCoreService->GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(sRet, PIPE_PRIMARY_INPUT);

    deviceType = DEVICE_TYPE_NEARLINK_IN;
    sRet = audioCoreService->GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(sRet, PIPE_PRIMARY_INPUT);

    deviceType = DEVICE_TYPE_MIC;
    sRet = audioCoreService->GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(sRet, PIPE_PRIMARY_INPUT);

    deviceType = DEVICE_TYPE_USB_ARM_HEADSET;
    sRet = audioCoreService->GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(sRet, PIPE_USB_ARM_INPUT);

    role = "TEST";
    sRet = audioCoreService->GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(sRet, PIPE_USB_ARM_OUTPUT);

    deviceType = DEVICE_TYPE_DP;
    sRet = audioCoreService->GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(sRet, PIPE_DP_OUTPUT);

    deviceType = DEVICE_TYPE_NONE;
    sRet = audioCoreService->GetPipeNameByDeviceForEc(role, deviceType);
    EXPECT_EQ(sRet, PIPE_PRIMARY_OUTPUT);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: GetPipeInfoByDeviceTypeForEc_001
* @tc.desc  : Test GetPipeInfoByDeviceTypeForEc interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, GetPipeInfoByDeviceTypeForEc_001, TestSize.Level1)
{
    std::string role = ROLE_SOURCE;
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;
    std::shared_ptr<AdapterPipeInfo> pipeInfo;
    auto audioCoreService = std::make_shared<AudioCoreService>();
    int32_t ret;

    ret = audioCoreService->GetPipeInfoByDeviceTypeForEc(role, deviceType, pipeInfo);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: GetEcType_001
* @tc.desc  : Test GetEcType interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, GetEcType_001, TestSize.Level1)
{
    DeviceType inputDevice;
    DeviceType outputDevice;
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EcType ecRet;

    inputDevice = DEVICE_TYPE_MIC;
    outputDevice = DEVICE_TYPE_SPEAKER;
    ecRet = audioCoreService->GetEcType(inputDevice, outputDevice);
    EXPECT_EQ(ecRet, EC_TYPE_SAME_ADAPTER);

    outputDevice = DEVICE_TYPE_MIC;
    ecRet = audioCoreService->GetEcType(inputDevice, outputDevice);
    EXPECT_EQ(ecRet, EC_TYPE_NONE);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: UpdateAudioEcInfo_001
* @tc.desc  : Test UpdateAudioEcInfo interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, UpdateAudioEcInfo_001, TestSize.Level1)
{
    AudioDeviceDescriptor inputDevice;
    AudioDeviceDescriptor outputDevice;
    auto audioCoreService = std::make_shared<AudioCoreService>();

    inputDevice.deviceType_ = DEVICE_TYPE_MIC;
    inputDevice.macAddress_ = "00:11:22:33:44:55";
    inputDevice.networkId_ = "1234567890";
    inputDevice.deviceRole_ = DEVICE_ROLE_NONE;
    outputDevice.deviceType_ = DEVICE_TYPE_MIC;
    outputDevice.macAddress_ = "00:11:22:33:44:55";
    outputDevice.networkId_ = "1234567890";
    outputDevice.deviceRole_ = DEVICE_ROLE_NONE;

    audioCoreService->audioEcInfo_.inputDevice.deviceType_ = DEVICE_TYPE_MIC;
    audioCoreService->audioEcInfo_.inputDevice.macAddress_ = "00:11:22:33:44:55";
    audioCoreService->audioEcInfo_.inputDevice.networkId_ = "1234567890";
    audioCoreService->audioEcInfo_.inputDevice.deviceRole_ = DEVICE_ROLE_NONE;
    audioCoreService->audioEcInfo_.outputDevice.deviceType_ = DEVICE_TYPE_MIC;
    audioCoreService->audioEcInfo_.outputDevice.macAddress_ = "00:11:22:33:44:55";
    audioCoreService->audioEcInfo_.outputDevice.networkId_ = "1234567890";
    audioCoreService->audioEcInfo_.outputDevice.deviceRole_ = DEVICE_ROLE_NONE;

    audioCoreService->isEcFeatureEnable_ = false;
    audioCoreService->UpdateAudioEcInfo(inputDevice, outputDevice);
    EXPECT_EQ(audioCoreService->isEcFeatureEnable_, false);

    audioCoreService->isEcFeatureEnable_ = true;
    audioCoreService->UpdateAudioEcInfo(inputDevice, outputDevice);
    EXPECT_EQ(audioCoreService->audioEcInfo_.inputDevice.IsSameDeviceDesc(inputDevice), true);

    inputDevice.networkId_ = "12345678";
    outputDevice.networkId_ = "12345678";
    audioCoreService->UpdateAudioEcInfo(inputDevice, outputDevice);
    EXPECT_EQ(audioCoreService->isEcFeatureEnable_, true);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: UpdateModuleInfoForEc_001
* @tc.desc  : Test UpdateModuleInfoForEc interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, UpdateModuleInfoForEc_001, TestSize.Level1)
{
    AudioModuleInfo moduleInfo;
    auto audioCoreService = std::make_shared<AudioCoreService>();

    audioCoreService->audioEcInfo_.channels = "5";
    audioCoreService->UpdateModuleInfoForEc(moduleInfo);
    EXPECT_EQ(moduleInfo.ecChannels, "5");
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: ShouldOpenMicRef_001
* @tc.desc  : Test ShouldOpenMicRef interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, ShouldOpenMicRef_001, TestSize.Level1)
{
    SourceType source = SOURCE_TYPE_VOICE_COMMUNICATION;
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::string sRet;

    audioCoreService->isMicRefFeatureEnable_ = false;
    sRet = audioCoreService->ShouldOpenMicRef(source);
    EXPECT_EQ(sRet, "0");

    audioCoreService->isMicRefFeatureEnable_ = true;
    sRet = audioCoreService->ShouldOpenMicRef(source);
    EXPECT_EQ(sRet, "0");
}

/**
 * @tc.name  : Test AudioEcManager.
 * @tc.number: UpdateModuleInfoForMicRef_002
 * @tc.desc  : Test UpdateModuleInfoForMicRef chooses stream prop matched by mic device.
 */
HWTEST_F(AudioCoreServiceUnitTest, UpdateModuleInfoForMicRef_002, TestSize.Level1)
{
    AudioModuleInfo moduleInfo;
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AdapterPipeInfo> pipeInfo = nullptr;
    ASSERT_EQ(audioCoreService->GetPipeInfoByDeviceTypeForEc(ROLE_SOURCE, DEVICE_TYPE_MIC, pipeInfo), SUCCESS);
    ASSERT_NE(pipeInfo, nullptr);

    const auto originStreamPropInfos = pipeInfo->streamPropInfos_;
    auto usbStreamPropInfo = CreatePipeStreamPropInfoForDevice(DEVICE_TYPE_USB_HEADSET, STEREO);
    auto micStreamPropInfo = CreatePipeStreamPropInfoForDevice(DEVICE_TYPE_MIC, CHANNEL_3);
    pipeInfo->streamPropInfos_ = {usbStreamPropInfo, micStreamPropInfo};

    audioCoreService->UpdateModuleInfoForMicRef(moduleInfo, SOURCE_TYPE_MIC);
    EXPECT_EQ(moduleInfo.micRefRate, "48000");
    EXPECT_EQ(moduleInfo.micRefFormat, "s16le");
    EXPECT_EQ(moduleInfo.micRefChannels, "3");

    pipeInfo->streamPropInfos_ = originStreamPropInfos;
}

/**
 * @tc.name  : Test AudioEcManager.
 * @tc.number: UpdateModuleInfoForMicRef_003
 * @tc.desc  : Test UpdateModuleInfoForMicRef fallback when mic stream prop is unavailable.
 */
HWTEST_F(AudioCoreServiceUnitTest, UpdateModuleInfoForMicRef_003, TestSize.Level1)
{
    AudioModuleInfo moduleInfo;
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AdapterPipeInfo> pipeInfo = nullptr;
    ASSERT_EQ(audioCoreService->GetPipeInfoByDeviceTypeForEc(ROLE_SOURCE, DEVICE_TYPE_MIC, pipeInfo), SUCCESS);
    ASSERT_NE(pipeInfo, nullptr);

    const auto originStreamPropInfos = pipeInfo->streamPropInfos_;
    pipeInfo->streamPropInfos_.clear();

    audioCoreService->UpdateModuleInfoForMicRef(moduleInfo, SOURCE_TYPE_MIC);
    EXPECT_EQ(moduleInfo.micRefRate, "48000");
    EXPECT_EQ(moduleInfo.micRefFormat, "s16le");
    EXPECT_EQ(moduleInfo.micRefChannels, "4");

    pipeInfo->streamPropInfos_ = originStreamPropInfos;
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: ResetAudioEcInfo_001
* @tc.desc  : Test GetAudioEcInfo & ResetAudioEcInfo interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, ResetAudioEcInfo_001, TestSize.Level1)
{
    AudioEcInfo ecInfo;
    auto audioCoreService = std::make_shared<AudioCoreService>();

    audioCoreService->audioEcInfo_.channels = "3";
    ecInfo = audioCoreService->GetAudioEcInfo();
    EXPECT_EQ(ecInfo.channels, "3");

    audioCoreService->ResetAudioEcInfo();
    ecInfo = audioCoreService->GetAudioEcInfo();
    EXPECT_EQ(ecInfo.inputDevice.deviceType_, DEVICE_TYPE_NONE);
    EXPECT_EQ(ecInfo.outputDevice.deviceType_, DEVICE_TYPE_NONE);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: ReloadSourceForSession_001
* @tc.desc  : Test ReloadSourceForSession interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, ReloadSourceForSession_001, TestSize.Level1)
{
    SessionInfo sessionInfo;
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_TRUE(audioCoreService != nullptr);

    sessionInfo.sourceType = SOURCE_TYPE_INVALID;
    audioCoreService->ReloadSourceForSession(sessionInfo, 100000);
}

static std::shared_ptr<AudioPipeInfo> CreateReloadSourceInputPipeInfo(const std::string &moduleName)
{
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->moduleInfo_.name = moduleName;
    pipeInfo->moduleInfo_.adapterName = "primary";
    pipeInfo->moduleInfo_.role = ROLE_SOURCE;
    pipeInfo->moduleInfo_.ecType = "";
    pipeInfo->moduleInfo_.ecSamplingRate = "";
    pipeInfo->moduleInfo_.ecChannels = "";
    pipeInfo->moduleInfo_.ecFormat = "";
    pipeInfo->moduleInfo_.micInRate = "";
    pipeInfo->moduleInfo_.micInChannels = "";
    pipeInfo->moduleInfo_.micInFormat = "";
    pipeInfo->moduleInfo_.micRefRate = "";
    pipeInfo->moduleInfo_.micRefChannels = "";
    pipeInfo->moduleInfo_.micRefFormat = "";
    return pipeInfo;
}

static std::shared_ptr<AudioStreamDescriptor> CreateVoiceRecognitionStreamDesc(uint32_t targetSessionId)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = targetSessionId;
    streamDesc->audioMode_ = AUDIO_MODE_RECORD;
    streamDesc->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    streamDesc->streamInfo_.format = SAMPLE_S16LE;
    streamDesc->streamInfo_.samplingRate = SAMPLE_RATE_48000;
    streamDesc->streamInfo_.channels = MONO;
    streamDesc->streamInfo_.encoding = ENCODING_PCM;
    streamDesc->streamInfo_.channelLayout = CH_LAYOUT_MONO;
    streamDesc->ecStreamInfo_.format = SAMPLE_S16LE;
    streamDesc->ecStreamInfo_.samplingRate = SAMPLE_RATE_48000;
    streamDesc->ecStreamInfo_.channels = MONO;
    streamDesc->micInStreamInfo_.format = SAMPLE_S16LE;
    streamDesc->micInStreamInfo_.samplingRate = SAMPLE_RATE_48000;
    streamDesc->micInStreamInfo_.channels = STEREO;
    auto inputDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    inputDevice->networkId_ = LOCAL_NETWORK_ID;
    streamDesc->newDeviceDescs_.push_back(inputDevice);
    return streamDesc;
}

static std::shared_ptr<AudioStreamDescriptor> CreateCamcorderStreamDesc(uint32_t targetSessionId)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = targetSessionId;
    streamDesc->audioMode_ = AUDIO_MODE_RECORD;
    streamDesc->routeFlag_ = AUDIO_INPUT_FLAG_CAMCORDER;
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_CAMCORDER;
    streamDesc->streamInfo_.format = SAMPLE_S16LE;
    streamDesc->streamInfo_.samplingRate = SAMPLE_RATE_48000;
    streamDesc->streamInfo_.channels = STEREO;
    streamDesc->streamInfo_.encoding = ENCODING_PCM;
    streamDesc->streamInfo_.channelLayout = CH_LAYOUT_STEREO;
    streamDesc->micInStreamInfo_.format = SAMPLE_S24LE;
    streamDesc->micInStreamInfo_.samplingRate = SAMPLE_RATE_48000;
    streamDesc->micInStreamInfo_.channels = CHANNEL_4;
    auto inputDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    inputDevice->networkId_ = LOCAL_NETWORK_ID;
    streamDesc->newDeviceDescs_.push_back(inputDevice);
    return streamDesc;
}

static void ExpectReloadSourceModuleInfo(const std::shared_ptr<AudioPipeInfo> &pipeInfo,
    const std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    EXPECT_EQ(pipeInfo->moduleInfo_.ecType, std::to_string(EC_TYPE_SAME_ADAPTER));
    EXPECT_EQ(pipeInfo->moduleInfo_.ecSamplingRate, std::to_string(streamDesc->ecStreamInfo_.samplingRate));
    EXPECT_EQ(pipeInfo->moduleInfo_.ecChannels, std::to_string(streamDesc->ecStreamInfo_.channels));
    EXPECT_EQ(pipeInfo->moduleInfo_.ecFormat, "s16");
    EXPECT_EQ(pipeInfo->moduleInfo_.micInRate, std::to_string(streamDesc->micInStreamInfo_.samplingRate));
    EXPECT_EQ(pipeInfo->moduleInfo_.micInChannels, std::to_string(streamDesc->micInStreamInfo_.channels));
    EXPECT_EQ(pipeInfo->moduleInfo_.micInFormat, "s16");
    EXPECT_EQ(pipeInfo->moduleInfo_.micRefRate, std::to_string(streamDesc->micInStreamInfo_.samplingRate));
    EXPECT_EQ(pipeInfo->moduleInfo_.micRefChannels, std::to_string(streamDesc->micInStreamInfo_.channels));
    EXPECT_EQ(pipeInfo->moduleInfo_.micRefFormat, "s16");
}

static void ExpectReloadSourceMicEcModuleInfoEmpty(const std::shared_ptr<AudioPipeInfo> &pipeInfo)
{
    EXPECT_EQ(pipeInfo->moduleInfo_.ecType, "");
    EXPECT_EQ(pipeInfo->moduleInfo_.ecSamplingRate, "");
    EXPECT_EQ(pipeInfo->moduleInfo_.ecChannels, "");
    EXPECT_EQ(pipeInfo->moduleInfo_.ecFormat, "");
    EXPECT_EQ(pipeInfo->moduleInfo_.micInRate, "");
    EXPECT_EQ(pipeInfo->moduleInfo_.micInChannels, "");
    EXPECT_EQ(pipeInfo->moduleInfo_.micInFormat, "");
    EXPECT_EQ(pipeInfo->moduleInfo_.micRefRate, "");
    EXPECT_EQ(pipeInfo->moduleInfo_.micRefChannels, "");
    EXPECT_EQ(pipeInfo->moduleInfo_.micRefFormat, "");
}

static void ExpectReloadSourceCamcorderModuleInfo(const std::shared_ptr<AudioPipeInfo> &pipeInfo,
    const std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    EXPECT_EQ(pipeInfo->moduleInfo_.ecType, "");
    EXPECT_EQ(pipeInfo->moduleInfo_.ecSamplingRate, "");
    EXPECT_EQ(pipeInfo->moduleInfo_.ecChannels, "");
    EXPECT_EQ(pipeInfo->moduleInfo_.ecFormat, "");
    EXPECT_EQ(pipeInfo->moduleInfo_.micInRate, std::to_string(streamDesc->micInStreamInfo_.samplingRate));
    EXPECT_EQ(pipeInfo->moduleInfo_.micInChannels, std::to_string(streamDesc->micInStreamInfo_.channels));
    EXPECT_EQ(pipeInfo->moduleInfo_.micInFormat, "s24");
    EXPECT_EQ(pipeInfo->moduleInfo_.micRefRate, std::to_string(streamDesc->micInStreamInfo_.samplingRate));
    EXPECT_EQ(pipeInfo->moduleInfo_.micRefChannels, std::to_string(streamDesc->micInStreamInfo_.channels));
    EXPECT_EQ(pipeInfo->moduleInfo_.micRefFormat, "s24");
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: ReloadSourceForInputPipe_001
* @tc.desc  : Test ReloadSourceForInputPipe updates voice recognition micIn/ec module info.
*/
HWTEST_F(AudioCoreServiceUnitTest, ReloadSourceForInputPipe_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_TRUE(audioCoreService != nullptr);
    auto &ioHandleMap = audioCoreService->audioIOHandleMap_;

    constexpr uint32_t targetSessionId = 123456;
    const std::string moduleName = "ut_voice_recognition_mic_ec_reload_pipe";
    AudioIOHandle oldIoHandle = 0;
    bool hadOldIoHandle = ioHandleMap.GetModuleIdByKey(moduleName, oldIoHandle);

    auto pipeInfo = CreateReloadSourceInputPipeInfo(moduleName);
    ASSERT_TRUE(pipeInfo != nullptr);
    auto streamDesc = CreateVoiceRecognitionStreamDesc(targetSessionId);
    ASSERT_TRUE(streamDesc != nullptr);
    ASSERT_FALSE(streamDesc->newDeviceDescs_.empty());

    pipeInfo->streamDescMap_[targetSessionId] = streamDesc;
    ioHandleMap.AddIOHandleInfo(moduleName, static_cast<AudioIOHandle>(targetSessionId));
    AudioIOHandle ioHandleInMap = 0;
    ASSERT_TRUE(ioHandleMap.GetModuleIdByKey(moduleName, ioHandleInMap));

    int32_t ret = audioCoreService->ReloadSourceForInputPipe(pipeInfo, targetSessionId);
    EXPECT_TRUE(ret == SUCCESS || ret == ERR_INVALID_HANDLE);
    if (ret == SUCCESS) {
        ExpectReloadSourceModuleInfo(pipeInfo, streamDesc);
    }

    if (hadOldIoHandle) {
        ioHandleMap.AddIOHandleInfo(moduleName, oldIoHandle);
    } else {
        ioHandleMap.DelIOHandleInfo(moduleName);
    }
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: ReloadSourceForInputPipe_002
* @tc.desc  : Test ReloadSourceForInputPipe keeps micIn/ec module info empty for remote input device.
*/
HWTEST_F(AudioCoreServiceUnitTest, ReloadSourceForInputPipe_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_TRUE(audioCoreService != nullptr);
    auto &ioHandleMap = audioCoreService->audioIOHandleMap_;

    constexpr uint32_t targetSessionId = 123457;
    const std::string moduleName = "ut_voice_recognition_remote_reload_pipe";
    AudioIOHandle oldIoHandle = 0;
    bool hadOldIoHandle = ioHandleMap.GetModuleIdByKey(moduleName, oldIoHandle);

    auto pipeInfo = CreateReloadSourceInputPipeInfo(moduleName);
    ASSERT_TRUE(pipeInfo != nullptr);
    auto streamDesc = CreateVoiceRecognitionStreamDesc(targetSessionId);
    ASSERT_TRUE(streamDesc != nullptr);
    ASSERT_FALSE(streamDesc->newDeviceDescs_.empty());
    streamDesc->newDeviceDescs_.front()->networkId_ = "RemoteDevice";

    pipeInfo->streamDescMap_[targetSessionId] = streamDesc;
    ioHandleMap.AddIOHandleInfo(moduleName, static_cast<AudioIOHandle>(targetSessionId));
    AudioIOHandle ioHandleInMap = 0;
    ASSERT_TRUE(ioHandleMap.GetModuleIdByKey(moduleName, ioHandleInMap));

    int32_t ret = audioCoreService->ReloadSourceForInputPipe(pipeInfo, targetSessionId);
    EXPECT_TRUE(ret == SUCCESS || ret == ERR_INVALID_HANDLE);
    if (ret == SUCCESS) {
        ExpectReloadSourceMicEcModuleInfoEmpty(pipeInfo);
    }

    if (hadOldIoHandle) {
        ioHandleMap.AddIOHandleInfo(moduleName, oldIoHandle);
    } else {
        ioHandleMap.DelIOHandleInfo(moduleName);
    }
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: ReloadSourceForInputPipe_003
* @tc.desc  : Test remote old input device also keeps micIn/ec module info empty.
*/
HWTEST_F(AudioCoreServiceUnitTest, ReloadSourceForInputPipe_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_TRUE(audioCoreService != nullptr);
    auto &ioHandleMap = audioCoreService->audioIOHandleMap_;

    constexpr uint32_t targetSessionId = 123458;
    const std::string moduleName = "ut_voice_recognition_old_remote_reload_pipe";
    AudioIOHandle oldIoHandle = 0;
    bool hadOldIoHandle = ioHandleMap.GetModuleIdByKey(moduleName, oldIoHandle);

    auto pipeInfo = CreateReloadSourceInputPipeInfo(moduleName);
    ASSERT_TRUE(pipeInfo != nullptr);
    auto streamDesc = CreateVoiceRecognitionStreamDesc(targetSessionId);
    ASSERT_TRUE(streamDesc != nullptr);
    ASSERT_FALSE(streamDesc->newDeviceDescs_.empty());
    streamDesc->newDeviceDescs_.front()->networkId_ = LOCAL_NETWORK_ID;
    auto oldRemoteInput = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    oldRemoteInput->networkId_ = "RemoteDevice";
    streamDesc->oldDeviceDescs_.push_back(oldRemoteInput);

    pipeInfo->streamDescMap_[targetSessionId] = streamDesc;
    ioHandleMap.AddIOHandleInfo(moduleName, static_cast<AudioIOHandle>(targetSessionId));
    AudioIOHandle ioHandleInMap = 0;
    ASSERT_TRUE(ioHandleMap.GetModuleIdByKey(moduleName, ioHandleInMap));

    int32_t ret = audioCoreService->ReloadSourceForInputPipe(pipeInfo, targetSessionId);
    EXPECT_TRUE(ret == SUCCESS || ret == ERR_INVALID_HANDLE);
    if (ret == SUCCESS) {
        ExpectReloadSourceMicEcModuleInfoEmpty(pipeInfo);
    }

    if (hadOldIoHandle) {
        ioHandleMap.AddIOHandleInfo(moduleName, oldIoHandle);
    } else {
        ioHandleMap.DelIOHandleInfo(moduleName);
    }
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: ReloadSourceForInputPipe_004
* @tc.desc  : Test ReloadSourceForInputPipe updates camcorder micIn module info.
*/
HWTEST_F(AudioCoreServiceUnitTest, ReloadSourceForInputPipe_004, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_TRUE(audioCoreService != nullptr);
    auto &ioHandleMap = audioCoreService->audioIOHandleMap_;

    constexpr uint32_t targetSessionId = 123459;
    const std::string moduleName = "ut_camcorder_mic_in_reload_pipe";
    AudioIOHandle oldIoHandle = 0;
    bool hadOldIoHandle = ioHandleMap.GetModuleIdByKey(moduleName, oldIoHandle);

    auto pipeInfo = CreateReloadSourceInputPipeInfo(moduleName);
    ASSERT_TRUE(pipeInfo != nullptr);
    auto streamDesc = CreateCamcorderStreamDesc(targetSessionId);
    ASSERT_TRUE(streamDesc != nullptr);
    ASSERT_FALSE(streamDesc->newDeviceDescs_.empty());

    pipeInfo->streamDescMap_[targetSessionId] = streamDesc;
    ioHandleMap.AddIOHandleInfo(moduleName, static_cast<AudioIOHandle>(targetSessionId));
    AudioIOHandle ioHandleInMap = 0;
    ASSERT_TRUE(ioHandleMap.GetModuleIdByKey(moduleName, ioHandleInMap));

    int32_t ret = audioCoreService->ReloadSourceForInputPipe(pipeInfo, targetSessionId);
    EXPECT_TRUE(ret == SUCCESS || ret == ERR_INVALID_HANDLE);
    if (ret == SUCCESS) {
        ExpectReloadSourceCamcorderModuleInfo(pipeInfo, streamDesc);
    }

    if (hadOldIoHandle) {
        ioHandleMap.AddIOHandleInfo(moduleName, oldIoHandle);
    } else {
        ioHandleMap.DelIOHandleInfo(moduleName);
    }
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: GetMicRefFeatureEnable_001
* @tc.desc  : Test GetMicRefFeatureEnable interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, GetMicRefFeatureEnable_001, TestSize.Level1)
{
    bool bRet;
    auto audioCoreService = std::make_shared<AudioCoreService>();

    audioCoreService->isMicRefFeatureEnable_ = true;
    bRet = audioCoreService->GetMicRefFeatureEnable();
    EXPECT_EQ(bRet, true);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: UpdateStreamEcAndMicRefInfo_001
* @tc.desc  : Test UpdateStreamEcAndMicRefInfo interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, UpdateStreamEcAndMicRefInfo_001, TestSize.Level1)
{
    AudioModuleInfo moduleInfo;
    SourceType sourceType = SOURCE_TYPE_INVALID;
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_TRUE(audioCoreService != nullptr);

    audioCoreService->UpdateStreamEcAndMicRefInfo(moduleInfo, sourceType);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: GetHalNameForDevice_001
* @tc.desc  : Test GetHalNameForDevice interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, GetHalNameForDevice_001, TestSize.Level1)
{
    std::string role;
    DeviceType deviceType;
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::string sRet;

    role = ROLE_SOURCE;
    deviceType = DEVICE_TYPE_MIC;
    sRet = audioCoreService->GetHalNameForDevice(role, deviceType);
    EXPECT_EQ(sRet, "primary");

    role = ROLE_SINK;
    sRet = audioCoreService->GetHalNameForDevice(role, deviceType);
    EXPECT_EQ(sRet, "primary");
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: GetEcFeatureEnableAndMicRefFeatureEnable_001
* @tc.desc  : Test Init & GetEcFeatureEnable & GetMicRefFeatureEnable interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, GetEcFeatureEnableAndMicRefFeatureEnable_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();

    audioCoreService->SetEcAndMicRefEnableState(0, 1);
    EXPECT_EQ(audioCoreService->GetEcFeatureEnable(), false);
    EXPECT_EQ(audioCoreService->GetMicRefFeatureEnable(), true);

    audioCoreService->SetEcAndMicRefEnableState(1, 0);
    EXPECT_EQ(audioCoreService->GetEcFeatureEnable(), true);
    EXPECT_EQ(audioCoreService->GetMicRefFeatureEnable(), false);

    audioCoreService->SetEcAndMicRefEnableState(1, 1);
    EXPECT_EQ(audioCoreService->GetEcFeatureEnable(), true);
    EXPECT_EQ(audioCoreService->GetMicRefFeatureEnable(), true);

    audioCoreService->SetEcAndMicRefEnableState(0, 0);
    EXPECT_EQ(audioCoreService->GetEcFeatureEnable(), false);
    EXPECT_EQ(audioCoreService->GetMicRefFeatureEnable(), false);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: PipeManagerPassThrough_001
* @tc.desc  : Test CoreService pass-through methods with PipeManager.
*/
HWTEST_F(AudioCoreServiceUnitTest, PipeManagerPassThrough_001, TestSize.Level1)
{
    auto audioCoreService = AudioCoreService::GetCoreService();
    ASSERT_TRUE(audioCoreService != nullptr);
    auto pipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_TRUE(pipeManager != nullptr);

    EXPECT_EQ(audioCoreService->GetFastFormat(), pipeManager->GetFastFormat());
    EXPECT_EQ(audioCoreService->IsSupportInnerCaptureOffload(), pipeManager->IsSupportInnerCaptureOffload());
    EXPECT_EQ(audioCoreService->GetMaxRendererInstances(), pipeManager->GetMaxRendererInstances());

    std::unordered_map<ClassType, std::list<AudioModuleInfo>> coreDeviceClassInfo = {};
    std::unordered_map<ClassType, std::list<AudioModuleInfo>> pipeDeviceClassInfo = {};
    audioCoreService->GetDeviceClassInfo(coreDeviceClassInfo);
    pipeManager->GetDeviceClassInfo(pipeDeviceClassInfo);
    EXPECT_EQ(coreDeviceClassInfo.size(), pipeDeviceClassInfo.size());
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: PipeManagerPassThrough_002
* @tc.desc  : Test SetEcEnableState pass-through.
*/
HWTEST_F(AudioCoreServiceUnitTest, PipeManagerPassThrough_002, TestSize.Level1)
{
    auto audioCoreService = AudioCoreService::GetCoreService();
    ASSERT_TRUE(audioCoreService != nullptr);

    audioCoreService->SetEcEnableState(false);
    audioCoreService->SetEcEnableState(true);

    EXPECT_TRUE(true);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: CloseNormalSourceAndGetSourceOpened_001

* @tc.desc  : Test CloseNormalSource & GetSourceOpened interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, CloseNormalSourceAndGetSourceOpened_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();

    audioCoreService->SetOpenedNormalSource(SOURCE_TYPE_MIC);
    EXPECT_EQ(audioCoreService->GetSourceOpened(), SOURCE_TYPE_MIC);

    audioCoreService->SetEcAndMicRefEnableState(1, 0);
    bool isEcFeatureEnable = audioCoreService->isEcFeatureEnable_;
    audioCoreService->isEcFeatureEnable_ = true;
    audioCoreService->CloseNormalSource();
    audioCoreService->isEcFeatureEnable_ = isEcFeatureEnable;
    EXPECT_EQ(audioCoreService->GetSourceOpened(), SOURCE_TYPE_INVALID);
    audioCoreService->SetEcAndMicRefEnableState(0, 0);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: EcGetTargetSourceTypeAndMatchingFlag_002
* @tc.desc  : Test GetTargetSourceTypeAndMatchingFlag interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, EcGetTargetSourceTypeAndMatchingFlag_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();

    SourceType targetSource;
    bool useMatchingPropInfo;

    audioCoreService->GetTargetSourceTypeAndMatchingFlag(
        SOURCE_TYPE_VOICE_RECOGNITION, targetSource, useMatchingPropInfo);
    EXPECT_EQ(targetSource, SOURCE_TYPE_VOICE_RECOGNITION);

    audioCoreService->GetTargetSourceTypeAndMatchingFlag(
        SOURCE_TYPE_VOICE_COMMUNICATION, targetSource, useMatchingPropInfo);
    EXPECT_EQ(targetSource, SOURCE_TYPE_VOICE_COMMUNICATION);

    audioCoreService->GetTargetSourceTypeAndMatchingFlag(SOURCE_TYPE_VOICE_CALL, targetSource, useMatchingPropInfo);
    EXPECT_EQ(targetSource, SOURCE_TYPE_VOICE_CALL);

    audioCoreService->GetTargetSourceTypeAndMatchingFlag(SOURCE_TYPE_UNPROCESSED, targetSource, useMatchingPropInfo);
    EXPECT_EQ(targetSource, SOURCE_TYPE_UNPROCESSED);

    audioCoreService->GetTargetSourceTypeAndMatchingFlag(SOURCE_TYPE_MIC, targetSource, useMatchingPropInfo);
    EXPECT_EQ(targetSource, SOURCE_TYPE_MIC);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: UpdateStreamEcInfo_001
* @tc.desc  : Test UpdateStreamEcInfo interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, UpdateStreamEcInfo_001, TestSize.Level4)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    AudioModuleInfo moduleInfo;
    SourceType sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    EXPECT_NO_THROW(audioCoreService->UpdateStreamEcInfo(moduleInfo, sourceType));
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: PresetArmIdleInput_001
* @tc.desc  : Test PresetArmIdleInput interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, PresetArmIdleInput_001, TestSize.Level4)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    audioCoreService->isEcFeatureEnable_ = false;
    audioCoreService->usbSourceModuleInfo_.role = "";
    audioCoreService->PresetArmIdleInput(deviceDesc);
    EXPECT_TRUE(audioCoreService->usbSourceModuleInfo_.role.empty());
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: CloseUsbArmDevice_001
* @tc.desc  : Test CloseUsbArmDevice interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, CloseUsbArmDevice_001, TestSize.Level4)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    AudioDeviceDescriptor device(DEVICE_TYPE_EARPIECE, INPUT_DEVICE);
    device.macAddress_ = "00:11:22:33:44:55";
    audioCoreService->activeArmInputAddr_ = device.macAddress_;
    EXPECT_NO_THROW(audioCoreService->CloseUsbArmDevice(device));

    device.deviceRole_ = OUTPUT_DEVICE;
    audioCoreService->activeArmOutputAddr_ = device.macAddress_;
    EXPECT_NO_THROW(audioCoreService->CloseUsbArmDevice(device));
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: EcGetTargetSourceTypeAndMatchingFlag_002
* @tc.desc  : Test GetTargetSourceTypeAndMatchingFlag interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, EcGetTargetSourceTypeAndMatchingFlag_002, TestSize.Level4)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    SourceType source = SOURCE_TYPE_LIVE;
    SourceType targetSource = SOURCE_TYPE_INVALID;
    bool useMatchingPropInfo = false;
    audioCoreService->GetTargetSourceTypeAndMatchingFlag(source, targetSource, useMatchingPropInfo);
    EXPECT_EQ(targetSource, SOURCE_TYPE_LIVE);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: UpdateStreamCommonInfo_001
* @tc.desc  : Test UpdateStreamCommonInfo interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, UpdateStreamCommonInfo_001, TestSize.Level4)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    bool originIsEcFeatureEnable = audioCoreService->isEcFeatureEnable_;
    audioCoreService->isEcFeatureEnable_ = false;
    AudioModuleInfo moduleInfo = {};
    PipeStreamPropInfo targetInfo = PipeStreamPropInfo();
    SourceType sourceType = SourceType::SOURCE_TYPE_MIC;
    audioCoreService->UpdateStreamCommonInfo(moduleInfo, targetInfo, sourceType);
    EXPECT_EQ(moduleInfo.sourceType, "0");

    audioCoreService->isEcFeatureEnable_ = true;
    audioCoreService->UpdateStreamCommonInfo(moduleInfo, targetInfo, sourceType);
    EXPECT_EQ(moduleInfo.sourceType, "0");

    audioCoreService->isEcFeatureEnable_ = originIsEcFeatureEnable;
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: UpdateEnhanceEffectState_001
* @tc.desc  : Test UpdateEnhanceEffectState interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, UpdateEnhanceEffectState_001, TestSize.Level4)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    SourceType sourceType = SourceType::SOURCE_TYPE_MIC;
    audioCoreService->UpdateEnhanceEffectState(sourceType);
    EXPECT_EQ(audioCoreService->isMicRefRecordOn_, false);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: UpdateStreamMicRefInfo_001
* @tc.desc  : Test UpdateStreamMicRefInfo interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, UpdateStreamMicRefInfo_001, TestSize.Level4)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    AudioModuleInfo moduleInfo = {};
    SourceType sourceType = SourceType::SOURCE_TYPE_MIC;
    EXPECT_NO_THROW(
        audioCoreService->UpdateStreamMicRefInfo(moduleInfo, sourceType);
    );

    sourceType = SourceType::SOURCE_TYPE_VOICE_COMMUNICATION;
    EXPECT_NO_THROW(
        audioCoreService->UpdateStreamMicRefInfo(moduleInfo, sourceType);
    );

    sourceType = SOURCE_TYPE_INVALID;
    EXPECT_NO_THROW(
        audioCoreService->UpdateStreamMicRefInfo(moduleInfo, sourceType);
    );
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: ReloadNormalSource_001
* @tc.desc  : Test ReloadNormalSource interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, ReloadNormalSource_001, TestSize.Level4)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    SessionInfo sessionInfo = {};
    PipeStreamPropInfo targetInfo = PipeStreamPropInfo();
    SourceType targetSource = SourceType::SOURCE_TYPE_MIC;
    int32_t ret = audioCoreService->ReloadNormalSource(sessionInfo, targetInfo, targetSource);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: GetOpenedNormalSourceSessionId_001
* @tc.desc  : Test GetOpenedNormalSourceSessionId interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, GetOpenedNormalSourceSessionId_001, TestSize.Level4)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    uint64_t ret = audioCoreService->GetOpenedNormalSourceSessionId();
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: SetOpenedNormalSourceSessionId_001
* @tc.desc  : Test SetOpenedNormalSourceSessionId interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, SetOpenedNormalSourceSessionId_001, TestSize.Level4)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    uint64_t originSessionId = audioCoreService->sessionIdUsedToOpenSource_;
    uint64_t sessionId = TEST_SESSION_ID;
    audioCoreService->SetOpenedNormalSourceSessionId(sessionId);
    EXPECT_EQ(audioCoreService->sessionIdUsedToOpenSource_, sessionId);

    audioCoreService->sessionIdUsedToOpenSource_ = originSessionId;
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: SetOpenedNormalSource_001
* @tc.desc  : Test SetOpenedNormalSource interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, SetOpenedNormalSource_001, TestSize.Level4)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    SourceType origin = audioCoreService->normalSourceOpened_;
    SourceType sourceType = SourceType::SOURCE_TYPE_MIC;
    audioCoreService->SetOpenedNormalSource(sourceType);
    EXPECT_EQ(sourceType, audioCoreService->normalSourceOpened_);

    audioCoreService->normalSourceOpened_ = origin;
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: SetPrimaryMicModuleInfo_001
* @tc.desc  : Test SetPrimaryMicModuleInfo interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, SetPrimaryMicModuleInfo_001, TestSize.Level4)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    AudioModuleInfo moduleInfo = {};
    moduleInfo.name = "test";
    audioCoreService->SetPrimaryMicModuleInfo(moduleInfo);
    EXPECT_EQ(audioCoreService->primaryMicModuleInfo_.name, moduleInfo.name);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: SetDpSinkModuleInfo_001
* @tc.desc  : Test SetDpSinkModuleInfo interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, SetDpSinkModuleInfo_001, TestSize.Level4)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    AudioModuleInfo moduleInfo = {};
    moduleInfo.className = "AudioCoreServiceUnitTest";
    audioCoreService->SetDpSinkModuleInfo(moduleInfo);
    EXPECT_EQ(audioCoreService->dpSinkModuleInfo_.className, moduleInfo.className);
}

/**
 * @tc.name  : Test AudioEcManager.
 * @tc.number: FetchTargetInfoForSessionAdd_001
 * @tc.desc  : Test FetchTargetInfoForSessionAdd interface.
 */
HWTEST_F(AudioCoreServiceUnitTest, FetchTargetInfoForSessionAdd_001, TestSize.Level4)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    SessionInfo sessionInfo = {};
    PipeStreamPropInfo targetInfo = PipeStreamPropInfo();
    SourceType targetSourceType = SourceType::SOURCE_TYPE_MIC;
    int32_t ret = audioCoreService->FetchTargetInfoForSessionAdd(sessionInfo, targetInfo, targetSourceType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioEcManager.
 * @tc.number: FetchTargetInfoForSessionAdd_002
 * @tc.desc  : Test FetchTargetInfoForSessionAdd interface.
 */
HWTEST_F(AudioCoreServiceUnitTest, FetchTargetInfoForSessionAdd_002, TestSize.Level4)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    SessionInfo sessionInfo = {};
    PipeStreamPropInfo targetInfo = PipeStreamPropInfo();
    SourceType targetSourceType = SourceType::SOURCE_TYPE_MIC;
    bool originEcFeatureEnable_ = audioCoreService->isEcFeatureEnable_;
    std::string originMicSpeaker = audioCoreService->primaryMicModuleInfo_.OpenMicSpeaker;

    audioCoreService->isEcFeatureEnable_ = false;
    audioCoreService->primaryMicModuleInfo_.OpenMicSpeaker = "0";
    int32_t ret = audioCoreService->FetchTargetInfoForSessionAdd(sessionInfo, targetInfo, targetSourceType);
    EXPECT_EQ(ret, SUCCESS);

    audioCoreService->isEcFeatureEnable_ = true;
    audioCoreService->primaryMicModuleInfo_.OpenMicSpeaker = "1";
    ret = audioCoreService->FetchTargetInfoForSessionAdd(sessionInfo, targetInfo, targetSourceType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioEcManager.
 * @tc.number: FetchTargetInfoForSessionAdd_003
 * @tc.desc  : Test target buffer duration calculation with a custom stream profile.
 */
HWTEST_F(AudioCoreServiceUnitTest, FetchTargetInfoForSessionAdd_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<PolicyAdapterInfo> adapterInfo = nullptr;
    ASSERT_TRUE(AudioCoreConfigManager::GetInstance().GetAdapterInfoByType(
        AudioAdapterType::TYPE_PRIMARY, adapterInfo));
    ASSERT_NE(adapterInfo, nullptr);
    std::shared_ptr<AdapterPipeInfo> pipeInfo = adapterInfo->GetPipeInfoByName(PIPE_PRIMARY_INPUT);
    ASSERT_NE(pipeInfo, nullptr);

    auto originalStreamPropInfos = pipeInfo->streamPropInfos_;
    auto customStreamProp = std::make_shared<PipeStreamPropInfo>();
    ASSERT_NE(customStreamProp, nullptr);
    customStreamProp->format_ = SAMPLE_S16LE;
    customStreamProp->sampleRate_ = static_cast<uint32_t>(AudioSamplingRate::SAMPLE_RATE_48000);
    customStreamProp->channels_ = AudioChannel::STEREO;
    customStreamProp->channelLayout_ = CH_LAYOUT_STEREO;
    customStreamProp->bufferSize_ = 1000;
    pipeInfo->streamPropInfos_.clear();
    pipeInfo->streamPropInfos_.push_back(customStreamProp);

    bool originEcFeatureEnable = audioCoreService->isEcFeatureEnable_;
    std::string originMicSpeaker = audioCoreService->primaryMicModuleInfo_.OpenMicSpeaker;
    audioCoreService->isEcFeatureEnable_ = false;
    audioCoreService->primaryMicModuleInfo_.OpenMicSpeaker = "1";

    SessionInfo sessionInfo = {};
    sessionInfo.sourceType = SOURCE_TYPE_MIC;
    PipeStreamPropInfo targetInfo = {};
    SourceType targetSourceType = SOURCE_TYPE_MIC;
    int32_t ret = audioCoreService->FetchTargetInfoForSessionAdd(sessionInfo, targetInfo, targetSourceType);
    EXPECT_EQ(ret, SUCCESS);

    constexpr uint64_t HALF_DIVISOR = 2ULL;
    constexpr uint32_t MS_PER_SECOND = 1000;
    constexpr uint32_t S16LE_BYTES_PER_SAMPLE = 2;
    const uint32_t channels = static_cast<uint32_t>(customStreamProp->channels_);
    const uint64_t denom = static_cast<uint64_t>(customStreamProp->sampleRate_) *
        static_cast<uint64_t>(channels) * static_cast<uint64_t>(S16LE_BYTES_PER_SAMPLE);
    const uint64_t number = static_cast<uint64_t>(customStreamProp->bufferSize_) * static_cast<uint64_t>(MS_PER_SECOND);
    const uint32_t expectedBufferMs = static_cast<uint32_t>((number + denom / HALF_DIVISOR) / denom);
    const uint32_t expectedBufferSize = expectedBufferMs * customStreamProp->sampleRate_ /
        MS_PER_SECOND * channels * S16LE_BYTES_PER_SAMPLE;
    EXPECT_EQ(targetInfo.bufferSize_, expectedBufferSize);

    audioCoreService->isEcFeatureEnable_ = originEcFeatureEnable;
    audioCoreService->primaryMicModuleInfo_.OpenMicSpeaker = originMicSpeaker;
    pipeInfo->streamPropInfos_ = originalStreamPropInfos;
}

/**
 * @tc.name  : Test AudioEcManager.
 * @tc.number: FetchTargetInfoForSessionAdd_004
 * @tc.desc  : Test default 20ms fallback when source profile bufferSize is invalid.
 */
HWTEST_F(AudioCoreServiceUnitTest, FetchTargetInfoForSessionAdd_004, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<PolicyAdapterInfo> adapterInfo = nullptr;
    ASSERT_TRUE(AudioCoreConfigManager::GetInstance().GetAdapterInfoByType(
        AudioAdapterType::TYPE_PRIMARY, adapterInfo));
    ASSERT_NE(adapterInfo, nullptr);
    std::shared_ptr<AdapterPipeInfo> pipeInfo = adapterInfo->GetPipeInfoByName(PIPE_PRIMARY_INPUT);
    ASSERT_NE(pipeInfo, nullptr);

    auto originalStreamPropInfos = pipeInfo->streamPropInfos_;
    auto customStreamProp = std::make_shared<PipeStreamPropInfo>();
    ASSERT_NE(customStreamProp, nullptr);
    customStreamProp->format_ = SAMPLE_S16LE;
    customStreamProp->sampleRate_ = static_cast<uint32_t>(AudioSamplingRate::SAMPLE_RATE_48000);
    customStreamProp->channels_ = AudioChannel::STEREO;
    customStreamProp->channelLayout_ = CH_LAYOUT_STEREO;
    customStreamProp->bufferSize_ = 0;
    pipeInfo->streamPropInfos_.clear();
    pipeInfo->streamPropInfos_.push_back(customStreamProp);

    bool originEcFeatureEnable = audioCoreService->isEcFeatureEnable_;
    std::string originMicSpeaker = audioCoreService->primaryMicModuleInfo_.OpenMicSpeaker;
    audioCoreService->isEcFeatureEnable_ = false;
    audioCoreService->primaryMicModuleInfo_.OpenMicSpeaker = "1";

    SessionInfo sessionInfo = {};
    sessionInfo.sourceType = SOURCE_TYPE_MIC;
    PipeStreamPropInfo targetInfo = {};
    SourceType targetSourceType = SOURCE_TYPE_MIC;
    int32_t ret = audioCoreService->FetchTargetInfoForSessionAdd(sessionInfo, targetInfo, targetSourceType);
    EXPECT_EQ(ret, SUCCESS);

    constexpr uint32_t DEFAULT_TARGET_BUFFER_MS = 20;
    constexpr uint32_t MS_PER_SECOND = 1000;
    constexpr uint32_t S16LE_BYTES_PER_SAMPLE = 2;
    const uint32_t channels = static_cast<uint32_t>(customStreamProp->channels_);
    const uint32_t expectedBufferSize = DEFAULT_TARGET_BUFFER_MS * customStreamProp->sampleRate_ /
        MS_PER_SECOND * channels * S16LE_BYTES_PER_SAMPLE;
    EXPECT_EQ(targetInfo.bufferSize_, expectedBufferSize);

    audioCoreService->isEcFeatureEnable_ = originEcFeatureEnable;
    audioCoreService->primaryMicModuleInfo_.OpenMicSpeaker = originMicSpeaker;
    pipeInfo->streamPropInfos_ = originalStreamPropInfos;
}

/**
 * @tc.name  : Test AudioEcManager.
 * @tc.number: FetchTargetInfoForSessionAdd_005
 * @tc.desc  : Test fallback branch when source profile sampleRate is 0.
 */
HWTEST_F(AudioCoreServiceUnitTest, FetchTargetInfoForSessionAdd_005, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<PolicyAdapterInfo> adapterInfo = nullptr;
    ASSERT_TRUE(AudioCoreConfigManager::GetInstance().GetAdapterInfoByType(
        AudioAdapterType::TYPE_PRIMARY, adapterInfo));
    ASSERT_NE(adapterInfo, nullptr);
    std::shared_ptr<AdapterPipeInfo> pipeInfo = adapterInfo->GetPipeInfoByName(PIPE_PRIMARY_INPUT);
    ASSERT_NE(pipeInfo, nullptr);

    auto originalStreamPropInfos = pipeInfo->streamPropInfos_;
    auto customStreamProp = std::make_shared<PipeStreamPropInfo>();
    ASSERT_NE(customStreamProp, nullptr);
    customStreamProp->format_ = SAMPLE_S16LE;
    customStreamProp->sampleRate_ = 0;
    customStreamProp->channels_ = AudioChannel::STEREO;
    customStreamProp->channelLayout_ = CH_LAYOUT_STEREO;
    customStreamProp->bufferSize_ = 1000;
    pipeInfo->streamPropInfos_.clear();
    pipeInfo->streamPropInfos_.push_back(customStreamProp);

    std::string originMicSpeaker = audioCoreService->primaryMicModuleInfo_.OpenMicSpeaker;
    audioCoreService->primaryMicModuleInfo_.OpenMicSpeaker = "0";

    SessionInfo sessionInfo = {};
    sessionInfo.sourceType = SOURCE_TYPE_MIC;
    PipeStreamPropInfo targetInfo = {};
    SourceType targetSourceType = SOURCE_TYPE_MIC;
    int32_t ret = audioCoreService->FetchTargetInfoForSessionAdd(sessionInfo, targetInfo, targetSourceType);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(targetInfo.sampleRate_, 0);

    audioCoreService->primaryMicModuleInfo_.OpenMicSpeaker = originMicSpeaker;
    pipeInfo->streamPropInfos_ = originalStreamPropInfos;
}

/**
 * @tc.name  : Test AudioEcManager.
 * @tc.number: FetchTargetInfoForSessionAdd_006
 * @tc.desc  : Test fallback branch when source profile format has zero bytesPerSample.
 */
HWTEST_F(AudioCoreServiceUnitTest, FetchTargetInfoForSessionAdd_006, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<PolicyAdapterInfo> adapterInfo = nullptr;
    ASSERT_TRUE(AudioCoreConfigManager::GetInstance().GetAdapterInfoByType(
        AudioAdapterType::TYPE_PRIMARY, adapterInfo));
    ASSERT_NE(adapterInfo, nullptr);
    std::shared_ptr<AdapterPipeInfo> pipeInfo = adapterInfo->GetPipeInfoByName(PIPE_PRIMARY_INPUT);
    ASSERT_NE(pipeInfo, nullptr);

    auto originalStreamPropInfos = pipeInfo->streamPropInfos_;
    auto customStreamProp = std::make_shared<PipeStreamPropInfo>();
    ASSERT_NE(customStreamProp, nullptr);
    customStreamProp->format_ = INVALID_WIDTH;
    customStreamProp->sampleRate_ = static_cast<uint32_t>(AudioSamplingRate::SAMPLE_RATE_48000);
    customStreamProp->channels_ = AudioChannel::STEREO;
    customStreamProp->channelLayout_ = CH_LAYOUT_STEREO;
    customStreamProp->bufferSize_ = 1000;
    pipeInfo->streamPropInfos_.clear();
    pipeInfo->streamPropInfos_.push_back(customStreamProp);

    std::string originMicSpeaker = audioCoreService->primaryMicModuleInfo_.OpenMicSpeaker;
    audioCoreService->primaryMicModuleInfo_.OpenMicSpeaker = "0";

    SessionInfo sessionInfo = {};
    sessionInfo.sourceType = SOURCE_TYPE_MIC;
    PipeStreamPropInfo targetInfo = {};
    SourceType targetSourceType = SOURCE_TYPE_MIC;
    int32_t ret = audioCoreService->FetchTargetInfoForSessionAdd(sessionInfo, targetInfo, targetSourceType);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(targetInfo.format_, INVALID_WIDTH);

    audioCoreService->primaryMicModuleInfo_.OpenMicSpeaker = originMicSpeaker;
    pipeInfo->streamPropInfos_ = originalStreamPropInfos;
}

/**
 * @tc.name  : Test AudioEcManager.
 * @tc.number: FetchTargetInfoForSessionAdd_008
 * @tc.desc  : Test FetchTargetInfoForSessionAdd returns error when selected stream prop is nullptr.
 */
HWTEST_F(AudioCoreServiceUnitTest, FetchTargetInfoForSessionAdd_008, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<PolicyAdapterInfo> adapterInfo = nullptr;
    ASSERT_TRUE(AudioCoreConfigManager::GetInstance().GetAdapterInfoByType(
        AudioAdapterType::TYPE_PRIMARY, adapterInfo));
    ASSERT_NE(adapterInfo, nullptr);
    std::shared_ptr<AdapterPipeInfo> pipeInfo = adapterInfo->GetPipeInfoByName(PIPE_PRIMARY_INPUT);
    ASSERT_NE(pipeInfo, nullptr);

    const auto originalStreamPropInfos = pipeInfo->streamPropInfos_;
    pipeInfo->streamPropInfos_ = {nullptr};

    SessionInfo sessionInfo = {};
    sessionInfo.sourceType = SOURCE_TYPE_MIC;
    PipeStreamPropInfo targetInfo = {};
    SourceType targetSourceType = SOURCE_TYPE_INVALID;
    int32_t ret = audioCoreService->FetchTargetInfoForSessionAdd(sessionInfo, targetInfo, targetSourceType);
    EXPECT_EQ(ret, ERROR);

    pipeInfo->streamPropInfos_ = originalStreamPropInfos;
}

/**
 * @tc.name  : Test AudioEcManager.
 * @tc.number: FetchTargetInfoForSessionAdd_009
 * @tc.desc  : Test FetchTargetInfoForSessionAdd prefers exact matching prop when matching is enabled.
 */
HWTEST_F(AudioCoreServiceUnitTest, FetchTargetInfoForSessionAdd_009, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<PolicyAdapterInfo> adapterInfo = nullptr;
    ASSERT_TRUE(AudioCoreConfigManager::GetInstance().GetAdapterInfoByType(
        AudioAdapterType::TYPE_PRIMARY, adapterInfo));
    ASSERT_NE(adapterInfo, nullptr);
    std::shared_ptr<AdapterPipeInfo> pipeInfo = adapterInfo->GetPipeInfoByName(PIPE_PRIMARY_INPUT);
    ASSERT_NE(pipeInfo, nullptr);

    const auto originalStreamPropInfos = pipeInfo->streamPropInfos_;
    auto defaultMicStreamPropInfo = CreatePipeStreamPropInfoForDevice(DEVICE_TYPE_MIC, MONO, SAMPLE_RATE_48000);
    auto exactMicStreamPropInfo = CreatePipeStreamPropInfoForDevice(DEVICE_TYPE_MIC, STEREO, SAMPLE_RATE_16000);
    pipeInfo->streamPropInfos_ = {defaultMicStreamPropInfo, exactMicStreamPropInfo};

    SessionInfo sessionInfo = {};
    sessionInfo.sourceType = SOURCE_TYPE_UNPROCESSED;
    sessionInfo.channels = STEREO;
    sessionInfo.rate = SAMPLE_RATE_16000;
    PipeStreamPropInfo targetInfo = {};
    SourceType targetSourceType = SOURCE_TYPE_INVALID;
    int32_t ret = audioCoreService->FetchTargetInfoForSessionAdd(sessionInfo, targetInfo, targetSourceType);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(targetSourceType, SOURCE_TYPE_UNPROCESSED);
    EXPECT_EQ(targetInfo.channels_, STEREO);
    EXPECT_EQ(targetInfo.sampleRate_, SAMPLE_RATE_16000);

    pipeInfo->streamPropInfos_ = originalStreamPropInfos;
}

/**
 * @tc.name  : Test AudioEcManager.
 * @tc.number: FetchTargetInfoForSessionAdd_010
 * @tc.desc  : Test FetchTargetInfoForSessionAdd returns error when stream prop list is empty.
 */
HWTEST_F(AudioCoreServiceUnitTest, FetchTargetInfoForSessionAdd_010, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    std::shared_ptr<PolicyAdapterInfo> adapterInfo = nullptr;
    ASSERT_TRUE(AudioCoreConfigManager::GetInstance().GetAdapterInfoByType(
        AudioAdapterType::TYPE_PRIMARY, adapterInfo));
    ASSERT_NE(adapterInfo, nullptr);
    std::shared_ptr<AdapterPipeInfo> pipeInfo = adapterInfo->GetPipeInfoByName(PIPE_PRIMARY_INPUT);
    ASSERT_NE(pipeInfo, nullptr);

    const auto originalStreamPropInfos = pipeInfo->streamPropInfos_;
    pipeInfo->streamPropInfos_.clear();

    SessionInfo sessionInfo = {};
    sessionInfo.sourceType = SOURCE_TYPE_MIC;
    PipeStreamPropInfo targetInfo = {};
    SourceType targetSourceType = SOURCE_TYPE_INVALID;
    int32_t ret = audioCoreService->FetchTargetInfoForSessionAdd(sessionInfo, targetInfo, targetSourceType);
    EXPECT_EQ(ret, ERROR);

    pipeInfo->streamPropInfos_ = originalStreamPropInfos;
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: UpdatePrimaryMicModuleInfo_007
* @tc.desc  : Test UpdatePrimaryMicModuleInfo interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, UpdatePrimaryMicModuleInfo_007, TestSize.Level4)
{
    SourceType sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    auto audioCoreService = std::make_shared<AudioCoreService>();

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    audioCoreService->isEcFeatureEnable_ = true;
    audioCoreService->UpdatePrimaryMicModuleInfo(pipeInfo, sourceType);
    EXPECT_EQ(audioCoreService->primaryMicModuleInfo_.channels, "");
    EXPECT_EQ(audioCoreService->primaryMicModuleInfo_.rate, "");
    EXPECT_EQ(audioCoreService->primaryMicModuleInfo_.format, "");
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: UpdatePrimaryMicModuleInfo_002
* @tc.desc  : Test UpdatePrimaryMicModuleInfo interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, UpdatePrimaryMicModuleInfo_002, TestSize.Level4) {
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "primary";
    audioCoreService->isEcFeatureEnable_ = true;

    auto inputDesc = std::make_shared<AudioDeviceDescriptor>();
    inputDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;

    audioCoreService->UpdatePrimaryMicModuleInfo(pipeInfo, SOURCE_TYPE_MIC);

    EXPECT_EQ(audioCoreService->primaryMicModuleInfo_.channels, "");
    EXPECT_EQ(audioCoreService->primaryMicModuleInfo_.rate, "");
    EXPECT_EQ(audioCoreService->primaryMicModuleInfo_.format, "");
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: UpdatePrimaryMicModuleInfo_003
* @tc.desc  : Test UpdatePrimaryMicModuleInfo interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, UpdatePrimaryMicModuleInfo_003, TestSize.Level4) {
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "not_primary";

    audioCoreService->UpdatePrimaryMicModuleInfo(pipeInfo, SOURCE_TYPE_MIC);

    EXPECT_EQ(audioCoreService->primaryMicModuleInfo_.channels, "");
    EXPECT_EQ(audioCoreService->primaryMicModuleInfo_.rate, "");
    EXPECT_EQ(audioCoreService->primaryMicModuleInfo_.format, "");
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: UpdatePrimaryMicModuleInfo_004
* @tc.desc  : Test UpdatePrimaryMicModuleInfo interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, UpdatePrimaryMicModuleInfo_004, TestSize.Level4) {
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "primary";
    audioCoreService->isEcFeatureEnable_ = false;

    audioCoreService->UpdatePrimaryMicModuleInfo(pipeInfo, SOURCE_TYPE_MIC);

    EXPECT_EQ(audioCoreService->primaryMicModuleInfo_.channels, "");
    EXPECT_EQ(audioCoreService->primaryMicModuleInfo_.rate, "");
    EXPECT_EQ(audioCoreService->primaryMicModuleInfo_.format, "");
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: UpdatePrimaryMicModuleInfo_005
* @tc.desc  : Test UpdatePrimaryMicModuleInfo interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, UpdatePrimaryMicModuleInfo_005, TestSize.Level4) {
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "primary";
    audioCoreService->isEcFeatureEnable_ = true;

    auto inputDesc = std::make_shared<AudioDeviceDescriptor>();
    inputDesc->deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;

    audioCoreService->UpdatePrimaryMicModuleInfo(pipeInfo, SOURCE_TYPE_MIC);

    EXPECT_EQ(audioCoreService->primaryMicModuleInfo_.channels, "");
    EXPECT_EQ(audioCoreService->primaryMicModuleInfo_.rate, "");
    EXPECT_EQ(audioCoreService->primaryMicModuleInfo_.format, "");
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: UpdatePrimaryMicModuleInfo_006
* @tc.desc  : Test UpdatePrimaryMicModuleInfo interface.
*/
HWTEST_F(AudioCoreServiceUnitTest, UpdatePrimaryMicModuleInfo_006, TestSize.Level4) {
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = "primary";
    audioCoreService->isEcFeatureEnable_ = true;

    auto inputDesc = std::make_shared<AudioDeviceDescriptor>();
    inputDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP_IN;

    audioCoreService->UpdatePrimaryMicModuleInfo(pipeInfo, SOURCE_TYPE_MIC);

    EXPECT_EQ(audioCoreService->primaryMicModuleInfo_.channels, "");
    EXPECT_EQ(audioCoreService->primaryMicModuleInfo_.rate, "");
    EXPECT_EQ(audioCoreService->primaryMicModuleInfo_.format, "");
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: ReloadNormalSource_002
* @tc.desc  : Test ReloadNormalSource interface with invalid sessionId.
*/
HWTEST_F(AudioCoreServiceUnitTest, ReloadNormalSource_002, TestSize.Level4)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    SessionInfo sessionInfo = {};
    PipeStreamPropInfo targetInfo = PipeStreamPropInfo();
    SourceType targetSource = SourceType::SOURCE_TYPE_VOICE_COMMUNICATION;


    AudioInjectorPolicy::GetInstance().SetCapturePortIdx(4321);
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = 4321;
    pipe1->pipeRole_ = PIPE_ROLE_INPUT;
    pipe1->moduleInfo_.name = "Built_in_mic";
    pipe1->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 1234;
    pipe1->streamDescriptors_.push_back(desc);
    AudioPipeManager::GetPipeManager()->AddAudioPipeInfo(pipe1);
    int32_t ret = audioCoreService->ReloadNormalSource(sessionInfo, targetInfo, targetSource);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: ReloadNormalSource_003
* @tc.desc  : Test ReloadNormalSource interface with invalid sessionId.
*/
HWTEST_F(AudioCoreServiceUnitTest, ReloadNormalSource_003, TestSize.Level4)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    SessionInfo sessionInfo = {};
    PipeStreamPropInfo targetInfo = PipeStreamPropInfo();
    SourceType targetSource = SourceType::SOURCE_TYPE_VOICE_CALL;
    uint32_t testSessionId = 1234;

    AudioInjectorPolicy::GetInstance().SetCapturePortIdx(4321);
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = 4321;
    pipe1->pipeRole_ = PIPE_ROLE_INPUT;
    pipe1->moduleInfo_.name = "Built_in_mic";
    pipe1->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 1234;
    pipe1->streamDescriptors_.push_back(desc);
    AudioPipeManager::GetPipeManager()->AddAudioPipeInfo(pipe1);
    int32_t ret = audioCoreService->ReloadNormalSource(sessionInfo, targetInfo, targetSource);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: ReloadNormalSource_004
* @tc.desc  : Test ReloadNormalSource interface with invalid sessionId.
*/
HWTEST_F(AudioCoreServiceUnitTest, ReloadNormalSource_004, TestSize.Level4)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    SessionInfo sessionInfo = {};
    PipeStreamPropInfo targetInfo = PipeStreamPropInfo();
    SourceType targetSource = SourceType::SOURCE_TYPE_VOICE_CALL;
    uint32_t testSessionId = 1234;

    AudioInjectorPolicy::GetInstance().SetCapturePortIdx(4321);
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = 4321;
    pipe1->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipe1->moduleInfo_.name = "Built_in_mic";
    pipe1->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 1234;
    pipe1->streamDescriptors_.push_back(desc);
    AudioPipeManager::GetPipeManager()->AddAudioPipeInfo(pipe1);
    int32_t ret = audioCoreService->ReloadNormalSource(sessionInfo, targetInfo, targetSource);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: ReloadNormalSource_005
* @tc.desc  : Test ReloadNormalSource interface with invalid sessionId.
*/
HWTEST_F(AudioCoreServiceUnitTest, ReloadNormalSource_005, TestSize.Level4)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    SessionInfo sessionInfo = {};
    PipeStreamPropInfo targetInfo = PipeStreamPropInfo();
    SourceType targetSource = SourceType::SOURCE_TYPE_VOICE_COMMUNICATION;
    uint32_t testSessionId = 1234;

    AudioInjectorPolicy::GetInstance().SetCapturePortIdx(4321);
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = 4321;
    pipe1->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipe1->moduleInfo_.name = "Built_in_mic";
    pipe1->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 1234;
    pipe1->streamDescriptors_.push_back(desc);
    AudioPipeManager::GetPipeManager()->AddAudioPipeInfo(pipe1);
    int32_t ret = audioCoreService->ReloadNormalSource(sessionInfo, targetInfo, targetSource);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: ReloadNormalSource_006
* @tc.desc  : Test ReloadNormalSource interface with invalid sessionId.
*/
HWTEST_F(AudioCoreServiceUnitTest, ReloadNormalSource_006, TestSize.Level4)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    SessionInfo sessionInfo = {};
    PipeStreamPropInfo targetInfo = PipeStreamPropInfo();
    SourceType targetSource = SourceType::SOURCE_TYPE_VOICE_COMMUNICATION;
    uint32_t testSessionId = 1234;

    AudioInjectorPolicy::GetInstance().SetCapturePortIdx(4321);
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = UINT32_INVALID_VALUE;
    pipe1->pipeRole_ = PIPE_ROLE_INPUT;
    pipe1->moduleInfo_.name = "Built_in_mic";
    pipe1->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 1234;
    pipe1->streamDescriptors_.push_back(desc);
    AudioPipeManager::GetPipeManager()->AddAudioPipeInfo(pipe1);
    int32_t ret = audioCoreService->ReloadNormalSource(sessionInfo, targetInfo, targetSource);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioEcManager.
* @tc.number: ReloadNormalSource_007
* @tc.desc  : Test ReloadNormalSource interface with invalid sessionId.
*/
HWTEST_F(AudioCoreServiceUnitTest, ReloadNormalSource_007, TestSize.Level4)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    SessionInfo sessionInfo = {};
    PipeStreamPropInfo targetInfo = PipeStreamPropInfo();
    SourceType targetSource = SourceType::SOURCE_TYPE_VOICE_COMMUNICATION;
    uint32_t testSessionId = 1234;

    AudioInjectorPolicy::GetInstance().SetCapturePortIdx(6789);
    std::shared_ptr<AudioPipeInfo> pipe1 = std::make_shared<AudioPipeInfo>();
    pipe1->paIndex_ = 4321;
    pipe1->pipeRole_ = PIPE_ROLE_INPUT;
    pipe1->moduleInfo_.name = "Built_in_mic";
    pipe1->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    desc->sessionId_ = 1234;
    pipe1->streamDescriptors_.push_back(desc);
    AudioPipeManager::GetPipeManager()->AddAudioPipeInfo(pipe1);
    int32_t ret = audioCoreService->ReloadNormalSource(sessionInfo, targetInfo, targetSource);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test ReleaseClient.
* @tc.number: ReleaseClient_001
* @tc.desc  : Test ReleaseClient - Verify switch stream behavior for both true/false needRemoveFromMap.
*/
HWTEST_F(AudioCoreServiceUnitTest, ReleaseClient_001, TestSize.Level1)
{
    auto pipeManager = AudioPipeManager::GetPipeManager();
    pipeManager->curPipeList_.clear();

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamInfo_.format = AudioSampleFormat::SAMPLE_S16LE;
    streamDesc->streamInfo_.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    streamDesc->streamInfo_.channels = AudioChannel::MONO;
    streamDesc->streamInfo_.encoding = AudioEncodingType::ENCODING_PCM;
    streamDesc->streamInfo_.channelLayout = AudioChannelLayout::CH_LAYOUT_MONO;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MEDIA;
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->callerUid_ = getuid();

    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    deviceDesc->networkId_ = LOCAL_NETWORK_ID;
    deviceDesc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    deviceDesc->macAddress_ = "00:00:00:00:00:00";
    streamDesc->newDeviceDescs_.clear();
    streamDesc->newDeviceDescs_.push_back(deviceDesc);

    uint32_t flag = AUDIO_OUTPUT_FLAG_NORMAL;
    uint32_t originalSessionId = 0;
    std::string networkId = LOCAL_NETWORK_ID;

    streamDesc->createTimeStamp_ = ClockTime::GetCurNano();
    auto result = GetServerPtr()->eventEntry_->CreateRendererClient(streamDesc, flag, originalSessionId, networkId);
    EXPECT_EQ(result, SUCCESS);
    pipeManager->AddSingleSwitchStream(originalSessionId, streamDesc);
    EXPECT_TRUE(pipeManager->IsSwitchStreamExist(originalSessionId));
    result = GetServerPtr()->coreService_->ReleaseClient(originalSessionId, SESSION_OP_MSG_DEFAULT, true);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_FALSE(pipeManager->IsSwitchStreamExist(originalSessionId));

    originalSessionId = 0;
    streamDesc->createTimeStamp_ = ClockTime::GetCurNano();
    result = GetServerPtr()->eventEntry_->CreateRendererClient(streamDesc, flag, originalSessionId, networkId);
    EXPECT_EQ(result, SUCCESS);
    pipeManager->AddSingleSwitchStream(originalSessionId, streamDesc);
    EXPECT_TRUE(pipeManager->IsSwitchStreamExist(originalSessionId));
    result = GetServerPtr()->coreService_->ReleaseClient(originalSessionId, SESSION_OP_MSG_DEFAULT, false);
    EXPECT_EQ(result, SUCCESS);
    EXPECT_TRUE(pipeManager->IsSwitchStreamExist(originalSessionId));
    pipeManager->RemoveSwitchStream(originalSessionId);
}

/**
* @tc.name  : Test HasNormalTypeCapturerSession.
* @tc.number: HasNormalTypeCapturerSession_001
* @tc.desc  : Test HasNormalTypeCapturerSession - empty pipeList returns false.
*/
HWTEST_F(AudioCoreServiceUnitTest, HasNormalTypeCapturerSession_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest HasNormalTypeCapturerSession_001 start");
    auto coreService = GetServerPtr()->coreService_;
    ASSERT_NE(coreService, nullptr);

    std::vector<std::shared_ptr<AudioPipeInfo>> emptyPipeList;
    bool result = coreService->HasNormalTypeCapturerSession(emptyPipeList);
    EXPECT_EQ(result, false);
}

/**
* @tc.name  : Test OnCapturerSessionRemoved.
* @tc.number: OnCapturerSessionRemoved_002
* @tc.desc  : Test OnCapturerSessionRemoved - sessionID in sessionWithInputPipeRouteFlag_ only.
*/
HWTEST_F(AudioCoreServiceUnitTest, OnCapturerSessionRemoved_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest OnCapturerSessionRemoved_002 start");
    auto coreService = GetServerPtr()->coreService_;
    ASSERT_NE(coreService, nullptr);

    uint64_t sessionId = 500001;
    coreService->sessionWithInputPipeRouteFlag_[sessionId] = AUDIO_INPUT_FLAG_NORMAL;
    coreService->OnCapturerSessionRemoved(sessionId);
    EXPECT_EQ(coreService->sessionWithInputPipeRouteFlag_.count(sessionId), 0);
    coreService->sessionSourceTypeMap_.clear();
    coreService->capturerSessionIdisRemovedSet_.clear();
    coreService->sessionWithInputPipeRouteFlag_.clear();
}

/**
* @tc.name  : Test OnCapturerSessionRemoved.
* @tc.number: OnCapturerSessionRemoved_003
* @tc.desc  : Test OnCapturerSessionRemoved - sessionID not in any map, goes to capturerSessionIdisRemovedSet_.
*/
HWTEST_F(AudioCoreServiceUnitTest, OnCapturerSessionRemoved_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest OnCapturerSessionRemoved_003 start");
    auto coreService = GetServerPtr()->coreService_;
    ASSERT_NE(coreService, nullptr);

    uint64_t sessionId = 500002;
    coreService->OnCapturerSessionRemoved(sessionId);
    EXPECT_EQ(coreService->capturerSessionIdisRemovedSet_.count(sessionId), 1);

    coreService->sessionSourceTypeMap_.clear();
    coreService->capturerSessionIdisRemovedSet_.clear();
    coreService->sessionWithInputPipeRouteFlag_.clear();
}

/**
* @tc.name  : Test OnCapturerSessionRemoved.
* @tc.number: OnCapturerSessionRemoved_004
* @tc.desc  : Test OnCapturerSessionRemoved - special source type, HandleRemoteCastDevice(false).
*/
HWTEST_F(AudioCoreServiceUnitTest, OnCapturerSessionRemoved_004, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest OnCapturerSessionRemoved_004 start");
    auto coreService = GetServerPtr()->coreService_;
    ASSERT_NE(coreService, nullptr);

    uint64_t sessionId = 500003;
    coreService->sessionSourceTypeMap_[sessionId] = SOURCE_TYPE_REMOTE_CAST;

    coreService->OnCapturerSessionRemoved(sessionId);
    EXPECT_EQ(coreService->sessionSourceTypeMap_.count(sessionId), 0);

    coreService->sessionSourceTypeMap_.clear();
    coreService->capturerSessionIdisRemovedSet_.clear();
    coreService->sessionWithInputPipeRouteFlag_.clear();
}

/**
* @tc.name  : Test OnCapturerSessionRemoved.
* @tc.number: OnCapturerSessionRemoved_005
* @tc.desc  : Test OnCapturerSessionRemoved - VOICE_COMMUNICATION, ResetAudioEcInfo and CloseNormalSource.
*/
HWTEST_F(AudioCoreServiceUnitTest, OnCapturerSessionRemoved_005, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest OnCapturerSessionRemoved_005 start");
    auto coreService = GetServerPtr()->coreService_;
    ASSERT_NE(coreService, nullptr);

    AudioPipeManager::GetPipeManager()->curPipeList_.clear();
    uint64_t sessionId = 500004;
    coreService->sessionSourceTypeMap_[sessionId] = SOURCE_TYPE_VOICE_COMMUNICATION;

    coreService->OnCapturerSessionRemoved(sessionId);
    EXPECT_EQ(coreService->sessionSourceTypeMap_.count(sessionId), 0);

    AudioPipeManager::GetPipeManager()->curPipeList_.clear();
    coreService->sessionSourceTypeMap_.clear();
    coreService->capturerSessionIdisRemovedSet_.clear();
    coreService->sessionWithInputPipeRouteFlag_.clear();
}

/**
* @tc.name  : Test OnCapturerSessionAdded.
* @tc.number: OnCapturerSessionAdded_002
* @tc.desc  : Test only normal source sessions are cached in sessionSourceTypeMap_.
*/
HWTEST_F(AudioCoreServiceUnitTest, OnCapturerSessionAdded_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest OnCapturerSessionAdded_002 start");
    auto coreService = GetServerPtr()->coreService_;
    ASSERT_NE(coreService, nullptr);

    AudioPipeManager::GetPipeManager()->curPipeList_.clear();
    coreService->SetConfigParserFlag();
    AudioVolumeManager::GetInstance().SetDefaultDeviceLoadFlag(true);
    coreService->sessionSourceTypeMap_.clear();
    coreService->capturerSessionIdisRemovedSet_.clear();
    coreService->normalSourceOpened_ = SOURCE_TYPE_MIC;

    AudioStreamInfo streamInfo;
    SessionInfo sessionInfo = {SOURCE_TYPE_MIC, 48000, 2};
    uint64_t normalSessionId = 610001;
    EXPECT_EQ(coreService->OnCapturerSessionAdded(normalSessionId, sessionInfo, streamInfo), SUCCESS);
    EXPECT_EQ(coreService->sessionSourceTypeMap_.count(normalSessionId), 1);

    sessionInfo.sourceType = SOURCE_TYPE_REMOTE_CAST;
    uint64_t remoteCastSessionId = 610002;
    EXPECT_EQ(coreService->OnCapturerSessionAdded(remoteCastSessionId, sessionInfo, streamInfo), SUCCESS);
    EXPECT_EQ(coreService->sessionSourceTypeMap_.count(remoteCastSessionId), 0);

    sessionInfo.sourceType = SOURCE_TYPE_WAKEUP;
    uint64_t wakeupSessionId = 610003;
    EXPECT_EQ(coreService->OnCapturerSessionAdded(wakeupSessionId, sessionInfo, streamInfo), SUCCESS);
    EXPECT_EQ(coreService->sessionSourceTypeMap_.count(wakeupSessionId), 0);

    coreService->normalSourceOpened_ = SOURCE_TYPE_INVALID;
    sessionInfo.sourceType = SOURCE_TYPE_MIC;
    uint64_t normalNotOpenedSessionId = 610004;
    EXPECT_EQ(coreService->OnCapturerSessionAdded(normalNotOpenedSessionId, sessionInfo, streamInfo), SUCCESS);
    EXPECT_EQ(coreService->sessionSourceTypeMap_.count(normalNotOpenedSessionId), 0);

    coreService->sessionSourceTypeMap_.clear();
    coreService->normalSourceOpened_ = SOURCE_TYPE_INVALID;
}

/**
* @tc.name  : Test HasNormalTypeCapturerSession.
* @tc.number: HasNormalTypeCapturerSession_002
* @tc.desc  : Test null source strategy map still detects normal recording sessions.
*/
HWTEST_F(AudioCoreServiceUnitTest, HasNormalTypeCapturerSession_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest HasNormalTypeCapturerSession_002 start");
    auto coreService = GetServerPtr()->coreService_;
    ASSERT_NE(coreService, nullptr);

    auto oldSourceStrategyMap = AudioSourceStrategyData::GetInstance().GetSourceStrategyMap();
    AudioSourceStrategyData::GetInstance().SetSourceStrategyMap(nullptr);

    auto pipe = std::make_shared<AudioPipeInfo>();
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->audioMode_ = AUDIO_MODE_RECORD;
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_MIC;
    pipe->streamDescriptors_.push_back(streamDesc);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList = {nullptr, pipe};
    EXPECT_TRUE(coreService->HasNormalTypeCapturerSession(pipeList));

    AudioSourceStrategyData::GetInstance().SetSourceStrategyMap(oldSourceStrategyMap);
}

/**
* @tc.name  : Test HasNormalTypeCapturerSession.
* @tc.number: HasNormalTypeCapturerSession_003
* @tc.desc  : Test non-recording, special source, null stream, and source strategy streams are ignored.
*/
HWTEST_F(AudioCoreServiceUnitTest, HasNormalTypeCapturerSession_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest HasNormalTypeCapturerSession_003 start");
    auto coreService = GetServerPtr()->coreService_;
    ASSERT_NE(coreService, nullptr);

    auto oldSourceStrategyMap = AudioSourceStrategyData::GetInstance().GetSourceStrategyMap();
    auto sourceStrategyMap = std::make_shared<std::map<SourceType, AudioSourceStrategyType>>();
    sourceStrategyMap->emplace(SOURCE_TYPE_MIC, AudioSourceStrategyType {
        "AUDIO_INPUT_MIC_TYPE",
        "primary",
        "primary_input",
        AUDIO_INPUT_FLAG_NORMAL,
        1
    });
    AudioSourceStrategyData::GetInstance().SetSourceStrategyMap(sourceStrategyMap);

    auto pipe = std::make_shared<AudioPipeInfo>();
    pipe->streamDescriptors_.push_back(nullptr);

    auto playbackStream = std::make_shared<AudioStreamDescriptor>();
    playbackStream->audioMode_ = AUDIO_MODE_PLAYBACK;
    playbackStream->capturerInfo_.sourceType = SOURCE_TYPE_MIC;
    pipe->streamDescriptors_.push_back(playbackStream);

    auto specialStream = std::make_shared<AudioStreamDescriptor>();
    specialStream->audioMode_ = AUDIO_MODE_RECORD;
    specialStream->capturerInfo_.sourceType = SOURCE_TYPE_REMOTE_CAST;
    pipe->streamDescriptors_.push_back(specialStream);

    auto strategyStream = std::make_shared<AudioStreamDescriptor>();
    strategyStream->audioMode_ = AUDIO_MODE_RECORD;
    strategyStream->capturerInfo_.sourceType = SOURCE_TYPE_MIC;
    pipe->streamDescriptors_.push_back(strategyStream);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList = {pipe};
    EXPECT_FALSE(coreService->HasNormalTypeCapturerSession(pipeList));

    AudioSourceStrategyData::GetInstance().SetSourceStrategyMap(oldSourceStrategyMap);
}

/**
 * @tc.name  : GetEnhancedRoutingSupported_001
 * @tc.number: GetEnhancedRoutingSupported_001
 * @tc.desc  : Verify AudioSelectInterfaceService forwards enhanced routing support from strategy layer.
 */
HWTEST_F(AudioCoreServiceUnitTest, GetEnhancedRoutingSupported_001, TestSize.Level1)
{
    bool supported = AudioSelectInterfaceService::GetInstance().GetEnhancedRoutingSupported();
    EXPECT_EQ(supported, AudioRouterSelectStrategy::GetInstance().GetEnhancedRoutingSupported());
    EXPECT_EQ(GetServerPtr()->coreService_->GetEnhancedRoutingSupported(), supported);
}

struct DirectTestParams {
    AudioSampleFormat format;
    AudioSamplingRate samplingRate;
    DeviceType deviceType;
    bool expectedResult;
    std::string description;
};

static std::shared_ptr<AudioStreamDescriptor> CreateDirectTestStream(const DirectTestParams& params)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamInfo_.format = params.format;
    streamDesc->streamInfo_.samplingRate = params.samplingRate;
    streamDesc->streamInfo_.channels = AudioChannel::STEREO;
    streamDesc->streamInfo_.encoding = AudioEncodingType::ENCODING_PCM;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_MUSIC;

    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = params.deviceType;
    deviceDesc->macAddress_ = "00:00:00:00:00:00";
    streamDesc->newDeviceDescs_.push_back(deviceDesc);

    return streamDesc;
}

HWTEST_F(AudioCoreServiceUnitTest, IsStreamSupportDirect_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest IsStreamSupportDirect_001 start");

    std::vector<DirectTestParams> testCases = {
        {SAMPLE_S24LE, SAMPLE_RATE_48000, DEVICE_TYPE_USB_HEADSET, true, "48k+24bit+USB"},
        {SAMPLE_S32LE, SAMPLE_RATE_48000, DEVICE_TYPE_USB_HEADSET, true, "48k+32bit+USB"},
        {SAMPLE_S24LE, SAMPLE_RATE_88200, DEVICE_TYPE_WIRED_HEADSET, true, "88.2k+24bit+WIRED"},
        {SAMPLE_S32LE, SAMPLE_RATE_88200, DEVICE_TYPE_WIRED_HEADSET, true, "88.2k+32bit+WIRED"},
        {SAMPLE_S24LE, SAMPLE_RATE_96000, DEVICE_TYPE_USB_HEADSET, true, "96k+24bit+USB"},
        {SAMPLE_S32LE, SAMPLE_RATE_96000, DEVICE_TYPE_USB_HEADSET, true, "96k+32bit+USB"},
        {SAMPLE_S24LE, SAMPLE_RATE_176400, DEVICE_TYPE_NEARLINK, true, "176.4k+24bit+NEARLINK"},
        {SAMPLE_S32LE, SAMPLE_RATE_176400, DEVICE_TYPE_NEARLINK, true, "176.4k+32bit+NEARLINK"},
        {SAMPLE_S24LE, SAMPLE_RATE_192000, DEVICE_TYPE_USB_HEADSET, true, "192k+24bit+USB"},
        {SAMPLE_S32LE, SAMPLE_RATE_192000, DEVICE_TYPE_USB_HEADSET, true, "192k+32bit+USB"},
    };

    for (const auto& params : testCases) {
        auto streamDesc = CreateDirectTestStream(params);
        bool result = GetServerPtr()->coreService_->IsStreamSupportDirect(streamDesc);
        EXPECT_EQ(result, params.expectedResult) << "Failed for: " << params.description;
    }
}

HWTEST_F(AudioCoreServiceUnitTest, IsStreamSupportDirect_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest IsStreamSupportDirect_002 start");

    std::vector<DirectTestParams> testCases = {
        {SAMPLE_S24LE, SAMPLE_RATE_44100, DEVICE_TYPE_USB_HEADSET, false, "44.1k unsupported rate"},
        {SAMPLE_S32LE, SAMPLE_RATE_64000, DEVICE_TYPE_USB_HEADSET, false, "64k unsupported rate"},
        {SAMPLE_S24LE, SAMPLE_RATE_384000, DEVICE_TYPE_USB_HEADSET, false, "384k unsupported rate"},
    };

    for (const auto& params : testCases) {
        auto streamDesc = CreateDirectTestStream(params);
        bool result = GetServerPtr()->coreService_->IsStreamSupportDirect(streamDesc);
        EXPECT_EQ(result, params.expectedResult) << "Failed for: " << params.description;
    }
}

HWTEST_F(AudioCoreServiceUnitTest, IsStreamSupportDirect_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest IsStreamSupportDirect_003 start");

    std::vector<DirectTestParams> testCases = {
        {SAMPLE_S16LE, SAMPLE_RATE_48000, DEVICE_TYPE_USB_HEADSET, false, "16bit unsupported format"},
        {SAMPLE_U8, SAMPLE_RATE_96000, DEVICE_TYPE_USB_HEADSET, false, "U8 unsupported format"},
        {SAMPLE_F32LE, SAMPLE_RATE_192000, DEVICE_TYPE_USB_HEADSET, false, "F32LE unsupported format"},
    };

    for (const auto& params : testCases) {
        auto streamDesc = CreateDirectTestStream(params);
        bool result = GetServerPtr()->coreService_->IsStreamSupportDirect(streamDesc);
        EXPECT_EQ(result, params.expectedResult) << "Failed for: " << params.description;
    }
}

HWTEST_F(AudioCoreServiceUnitTest, IsStreamSupportDirect_004, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest IsStreamSupportDirect_004 start");

    std::vector<DirectTestParams> testCases = {
        {SAMPLE_S24LE, SAMPLE_RATE_48000, DEVICE_TYPE_BLUETOOTH_A2DP, false, "BLUETOOTH_A2DP unsupported device"},
        {SAMPLE_S24LE, SAMPLE_RATE_48000, DEVICE_TYPE_SPEAKER, false, "SPEAKER unsupported device"},
    };

    for (const auto& params : testCases) {
        auto streamDesc = CreateDirectTestStream(params);
        bool result = GetServerPtr()->coreService_->IsStreamSupportDirect(streamDesc);
        EXPECT_EQ(result, params.expectedResult) << "Failed for: " << params.description;
    }
}

/**
* @tc.name  : Test NotifyStandbyStatus.
* @tc.number: NotifyStandbyStatus_001
* @tc.desc  : Test NotifyStandbyStatus with standbyStatus=true, verifies isStandby_ flag is set to true.
*/
HWTEST_F(AudioCoreServiceUnitTest, NotifyStandbyStatus_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest NotifyStandbyStatus_001 start");
    auto coreService = GetServerPtr()->coreService_;
    ASSERT_NE(coreService, nullptr);

    AudioPipeManager::GetPipeManager()->curPipeList_.clear();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->paIndex_ = 1000;
    pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipeInfo->moduleInfo_.name = "Speaker";

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 600001;
    streamDesc->isStandby_ = false;
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    pipeInfo->streamDescriptors_.push_back(streamDesc);
    AudioPipeManager::GetPipeManager()->AddAudioPipeInfo(pipeInfo);

    uint32_t sessionId = 600001;
    coreService->NotifyStandbyStatus(sessionId, true);

    auto pipeManager = AudioPipeManager::GetPipeManager();
    auto streamDescAfter = pipeManager->GetStreamDescById(sessionId);
    if (streamDescAfter != nullptr) {
        EXPECT_EQ(streamDescAfter->isStandby_, true);
    }

    AudioPipeManager::GetPipeManager()->curPipeList_.clear();
}

/**
* @tc.name  : Test NotifyStandbyStatus.
* @tc.number: NotifyStandbyStatus_002
* @tc.desc  : Test NotifyStandbyStatus with standbyStatus=true and invalid sessionId, verifies graceful handling.
*/
HWTEST_F(AudioCoreServiceUnitTest, NotifyStandbyStatus_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest NotifyStandbyStatus_002 start");
    auto coreService = GetServerPtr()->coreService_;
    ASSERT_NE(coreService, nullptr);

    AudioPipeManager::GetPipeManager()->curPipeList_.clear();
    uint32_t invalidSessionId = 999999;
    coreService->NotifyStandbyStatus(invalidSessionId, true);

    auto streamDesc = AudioPipeManager::GetPipeManager()->GetStreamDescById(invalidSessionId);
    EXPECT_EQ(streamDesc, nullptr);

    AudioPipeManager::GetPipeManager()->curPipeList_.clear();
}

/**
* @tc.name  : Test NotifyStandbyStatus.
* @tc.number: NotifyStandbyStatus_003
* @tc.desc  : Test NotifyStandbyStatus with standbyStatus=false, verifies isStandby_ flag is cleared to false.
*/
HWTEST_F(AudioCoreServiceUnitTest, NotifyStandbyStatus_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest NotifyStandbyStatus_003 start");
    auto coreService = GetServerPtr()->coreService_;
    ASSERT_NE(coreService, nullptr);

    AudioPipeManager::GetPipeManager()->curPipeList_.clear();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->paIndex_ = 1001;
    pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
    pipeInfo->moduleInfo_.name = "Speaker";

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 600002;
    streamDesc->isStandby_ = true;
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    pipeInfo->streamDescriptors_.push_back(streamDesc);
    AudioPipeManager::GetPipeManager()->AddAudioPipeInfo(pipeInfo);

    uint32_t sessionId = 600002;
    coreService->NotifyStandbyStatus(sessionId, false);

    auto pipeManager = AudioPipeManager::GetPipeManager();
    auto streamDescAfter = pipeManager->GetStreamDescById(sessionId);
    if (streamDescAfter != nullptr) {
        EXPECT_EQ(streamDescAfter->isStandby_, false);
    }

    AudioPipeManager::GetPipeManager()->curPipeList_.clear();
}

/**
* @tc.name  : Test NotifyStandbyStatus.
* @tc.number: NotifyStandbyStatus_004
* @tc.desc  : Test NotifyStandbyStatus with standbyStatus=false and invalid sessionId, verifies graceful handling.
*/
HWTEST_F(AudioCoreServiceUnitTest, NotifyStandbyStatus_004, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest NotifyStandbyStatus_004 start");
    auto coreService = GetServerPtr()->coreService_;
    ASSERT_NE(coreService, nullptr);

    AudioPipeManager::GetPipeManager()->curPipeList_.clear();
    uint32_t invalidSessionId = 999998;
    coreService->NotifyStandbyStatus(invalidSessionId, false);

    auto streamDesc = AudioPipeManager::GetPipeManager()->GetStreamDescById(invalidSessionId);
    EXPECT_EQ(streamDesc, nullptr);

    AudioPipeManager::GetPipeManager()->curPipeList_.clear();
}

/**
 * @tc.name  : NotifyRemoteRouteStateChange_001
 * @tc.number: NotifyRemoteRouteStateChange_001
 * @tc.desc  : Test NotifyRemoteRouteStateChange with enable=true sets CONNECTED state.
 */
HWTEST_F(AudioCoreServiceUnitTest, NotifyRemoteRouteStateChange_001, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> remoteDevice = std::make_shared<AudioDeviceDescriptor>();
    remoteDevice->deviceType_ = DEVICE_TYPE_SPEAKER;
    remoteDevice->networkId_ = "1234567890";
    remoteDevice->deviceRole_ = OUTPUT_DEVICE;
    remoteDevice->connectState_ = VIRTUAL_CONNECTED;
    remoteDevice->dmDeviceType_ = DM_DEVICE_TYPE_MUSIC_HOST;
    GetServerPtr()->coreService_->audioConnectedDevice_.AddConnectedDevice(remoteDevice);
    GetServerPtr()->coreService_->audioDeviceManager_.AddNewDevice(remoteDevice);

    AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_MEDIA_RENDER, remoteDevice);
    GetServerPtr()->eventEntry_->NotifyRemoteRouteStateChange("1234567890", DEVICE_TYPE_SPEAKER, true);
    auto currentPreferredDevice = AudioRouterSelectStrategy::GetInstance().GetMediaOutputDevice(
        INVALID_UID, INVALID_STREAM_ID);
    EXPECT_NE(currentPreferredDevice->dmDeviceType_, DM_DEVICE_TYPE_DEFAULT);

    GetServerPtr()->coreService_->audioConnectedDevice_.DelConnectedDevice("1234567890", DEVICE_TYPE_SPEAKER);
    AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_MEDIA_RENDER, std::make_shared<AudioDeviceDescriptor>());
}

/**
 * @tc.name  : NotifyRemoteRouteStateChange_002
 * @tc.number: NotifyRemoteRouteStateChange_002
 * @tc.desc  : Test NotifyRemoteRouteStateChange with enable=false and non-WIFI_SOUNDBOX device.
 */
HWTEST_F(AudioCoreServiceUnitTest, NotifyRemoteRouteStateChange_002, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> remoteDevice = std::make_shared<AudioDeviceDescriptor>();
    remoteDevice->deviceType_ = DEVICE_TYPE_SPEAKER;
    remoteDevice->networkId_ = "1234567891";
    remoteDevice->deviceRole_ = OUTPUT_DEVICE;
    remoteDevice->connectState_ = CONNECTED;
    remoteDevice->dmDeviceType_ = DM_DEVICE_TYPE_MUSIC_HOST;
    GetServerPtr()->coreService_->audioConnectedDevice_.AddConnectedDevice(remoteDevice);
    GetServerPtr()->coreService_->audioDeviceManager_.AddNewDevice(remoteDevice);

    AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_MEDIA_RENDER, remoteDevice);
    GetServerPtr()->eventEntry_->NotifyRemoteRouteStateChange("1234567891", DEVICE_TYPE_SPEAKER, false);
    auto currentPreferredDevice = AudioRouterSelectStrategy::GetInstance().GetMediaOutputDevice(
        INVALID_UID, INVALID_STREAM_ID);
    EXPECT_NE(currentPreferredDevice->dmDeviceType_, DM_DEVICE_TYPE_DEFAULT);

    GetServerPtr()->coreService_->audioConnectedDevice_.DelConnectedDevice("1234567891", DEVICE_TYPE_SPEAKER);
    AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_MEDIA_RENDER, std::make_shared<AudioDeviceDescriptor>());
}

/**
 * @tc.name  : NotifyRemoteRouteStateChange_003
 * @tc.number: NotifyRemoteRouteStateChange_003
 * @tc.desc  : Test NotifyRemoteRouteStateChange with WIFI_SOUNDBOX triggers SetPreferredDevice.
 */
HWTEST_F(AudioCoreServiceUnitTest, NotifyRemoteRouteStateChange_003, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> remoteDevice = std::make_shared<AudioDeviceDescriptor>();
    remoteDevice->deviceType_ = DEVICE_TYPE_SPEAKER;
    remoteDevice->networkId_ = "1234567892";
    remoteDevice->deviceRole_ = OUTPUT_DEVICE;
    remoteDevice->connectState_ = CONNECTED;
    remoteDevice->dmDeviceType_ = DM_DEVICE_TYPE_WIFI_SOUNDBOX;
    GetServerPtr()->coreService_->audioConnectedDevice_.AddConnectedDevice(remoteDevice);
    GetServerPtr()->coreService_->audioDeviceManager_.AddNewDevice(remoteDevice);

    AudioPolicyUtils::GetInstance().SetPreferredDevice(AUDIO_MEDIA_RENDER, remoteDevice);
    GetServerPtr()->eventEntry_->NotifyRemoteRouteStateChange("1234567892", DEVICE_TYPE_SPEAKER, false);
    auto currentPreferredDevice = AudioRouterSelectStrategy::GetInstance().GetMediaOutputDevice(
        INVALID_UID, INVALID_STREAM_ID);
    EXPECT_EQ(currentPreferredDevice->dmDeviceType_, DM_DEVICE_TYPE_DEFAULT);

    GetServerPtr()->coreService_->audioConnectedDevice_.DelConnectedDevice("1234567892", DEVICE_TYPE_SPEAKER);
}

/**
 * @tc.name  : GetActiveStreamsVolumeInfo_Empty
 * @tc.number: GetActiveStreamsVolumeInfo_001
 * @tc.desc  : Test GetActiveStreamsVolumeInfo returns empty when no active streams exist
 */
HWTEST_F(AudioCoreServiceUnitTest, GetActiveStreamsVolumeInfo_Empty, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto &streamCollector = audioCoreService->streamCollector_;
    streamCollector.audioRendererChangeInfos_.clear();

    std::vector<ActiveStreamVolumeInfo> activeStreamsVolumeInfo;
    auto result = audioCoreService->GetActiveStreamsVolumeInfo(activeStreamsVolumeInfo);
    EXPECT_EQ(SUCCESS, result);
    EXPECT_TRUE(activeStreamsVolumeInfo.empty());
}

/**
 * @tc.name  : GetActiveStreamsVolumeInfo_SingleStream
 * @tc.number: GetActiveStreamsVolumeInfo_002
 * @tc.desc  : Test GetActiveStreamsVolumeInfo returns single stream volume info
 */
HWTEST_F(AudioCoreServiceUnitTest, GetActiveStreamsVolumeInfo_SingleStream, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto &streamCollector = audioCoreService->streamCollector_;
    streamCollector.audioRendererChangeInfos_.clear();

    auto changeInfo = std::make_shared<AudioRendererChangeInfo>();
    changeInfo->clientUID = 1001;
    changeInfo->rendererState = RENDERER_RUNNING;
    changeInfo->rendererInfo.contentType = CONTENT_TYPE_MUSIC;
    changeInfo->rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    streamCollector.audioRendererChangeInfos_.push_back(std::move(changeInfo));

    std::vector<ActiveStreamVolumeInfo> activeStreamsVolumeInfo;
    auto result = audioCoreService->GetActiveStreamsVolumeInfo(activeStreamsVolumeInfo);
    EXPECT_EQ(SUCCESS, result);
    EXPECT_EQ(1, static_cast<int32_t>(activeStreamsVolumeInfo.size()));

    streamCollector.audioRendererChangeInfos_.clear();
}

/**
 * @tc.name  : GetActiveStreamsVolumeInfo_MultipleStreams
 * @tc.number: GetActiveStreamsVolumeInfo_003
 * @tc.desc  : Test GetActiveStreamsVolumeInfo returns multiple streams volume info
 */
HWTEST_F(AudioCoreServiceUnitTest, GetActiveStreamsVolumeInfo_MultipleStreams, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto &streamCollector = audioCoreService->streamCollector_;
    streamCollector.audioRendererChangeInfos_.clear();

    auto info1 = std::make_shared<AudioRendererChangeInfo>();
    info1->clientUID = 1001;
    info1->rendererState = RENDERER_RUNNING;
    info1->rendererInfo.contentType = CONTENT_TYPE_MUSIC;
    info1->rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    streamCollector.audioRendererChangeInfos_.push_back(std::move(info1));

    auto info2 = std::make_shared<AudioRendererChangeInfo>();
    info2->clientUID = 1002;
    info2->rendererState = RENDERER_RUNNING;
    info2->rendererInfo.contentType = CONTENT_TYPE_SPEECH;
    info2->rendererInfo.streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    streamCollector.audioRendererChangeInfos_.push_back(std::move(info2));

    std::vector<ActiveStreamVolumeInfo> activeStreamsVolumeInfo;
    auto result = audioCoreService->GetActiveStreamsVolumeInfo(activeStreamsVolumeInfo);
    EXPECT_EQ(SUCCESS, result);
    EXPECT_EQ(2, static_cast<int32_t>(activeStreamsVolumeInfo.size()));

    streamCollector.audioRendererChangeInfos_.clear();
}

/**
 * @tc.name  : GetActiveStreamsVolumeInfo_DuplicateStreams
 * @tc.number: GetActiveStreamsVolumeInfo_004
 * @tc.desc  : Test GetActiveStreamsVolumeInfo deduplicates same uid and streamType
 */
HWTEST_F(AudioCoreServiceUnitTest, GetActiveStreamsVolumeInfo_DuplicateStreams, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto &streamCollector = audioCoreService->streamCollector_;
    streamCollector.audioRendererChangeInfos_.clear();

    auto info1 = std::make_shared<AudioRendererChangeInfo>();
    info1->clientUID = 1001;
    info1->sessionId = 1;
    info1->rendererState = RENDERER_RUNNING;
    info1->rendererInfo.contentType = CONTENT_TYPE_MUSIC;
    info1->rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    streamCollector.audioRendererChangeInfos_.push_back(std::move(info1));

    auto info2 = std::make_shared<AudioRendererChangeInfo>();
    info2->clientUID = 1001;
    info2->sessionId = 2;
    info2->rendererState = RENDERER_RUNNING;
    info2->rendererInfo.contentType = CONTENT_TYPE_MUSIC;
    info2->rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    streamCollector.audioRendererChangeInfos_.push_back(std::move(info2));

    std::vector<ActiveStreamVolumeInfo> activeStreamsVolumeInfo;
    auto result = audioCoreService->GetActiveStreamsVolumeInfo(activeStreamsVolumeInfo);
    EXPECT_EQ(SUCCESS, result);
    EXPECT_EQ(1, static_cast<int32_t>(activeStreamsVolumeInfo.size()));

    streamCollector.audioRendererChangeInfos_.clear();
}

/**
 * @tc.name  : GetActiveStreamsVolumeInfo_NonRunningState
 * @tc.number: GetActiveStreamsVolumeInfo_005
 * @tc.desc  : Test GetActiveStreamsVolumeInfo excludes non-RUNNING streams
 */
HWTEST_F(AudioCoreServiceUnitTest, GetActiveStreamsVolumeInfo_NonRunningState, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto &streamCollector = audioCoreService->streamCollector_;
    streamCollector.audioRendererChangeInfos_.clear();

    auto info1 = std::make_shared<AudioRendererChangeInfo>();
    info1->clientUID = 1001;
    info1->rendererState = RENDERER_RUNNING;
    info1->rendererInfo.contentType = CONTENT_TYPE_MUSIC;
    info1->rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    streamCollector.audioRendererChangeInfos_.push_back(std::move(info1));

    auto info2 = std::make_shared<AudioRendererChangeInfo>();
    info2->clientUID = 1002;
    info2->rendererState = RENDERER_PAUSED;
    info2->rendererInfo.contentType = CONTENT_TYPE_MUSIC;
    info2->rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    streamCollector.audioRendererChangeInfos_.push_back(std::move(info2));

    auto info3 = std::make_shared<AudioRendererChangeInfo>();
    info3->clientUID = 1003;
    info3->rendererState = RENDERER_STOPPED;
    info3->rendererInfo.contentType = CONTENT_TYPE_MUSIC;
    info3->rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    streamCollector.audioRendererChangeInfos_.push_back(std::move(info3));

    std::vector<ActiveStreamVolumeInfo> activeStreamsVolumeInfo;
    auto result = audioCoreService->GetActiveStreamsVolumeInfo(activeStreamsVolumeInfo);
    EXPECT_EQ(SUCCESS, result);
    EXPECT_EQ(1, static_cast<int32_t>(activeStreamsVolumeInfo.size()));

    streamCollector.audioRendererChangeInfos_.clear();
}

/**
 * @tc.name  : HandleNormalInputPipes_VoipPriorityLowered_001
 * @tc.number: HandleNormalInputPipes_VoipPriorityLowered_001
 * @tc.desc  : Test lowered VoIP stream is skipped during priority comparison.
 */
HWTEST_F(AudioCoreServiceUnitTest, HandleNormalInputPipes_VoipPriorityLowered_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;
    pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;

    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 700001;
    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    streamDesc->voipPriorityLowered_.store(true);
    pipeInfo->streamDescriptors_.push_back(streamDesc);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList = { pipeInfo };
    AudioStreamDescriptor runningSessionInfo = {};
    runningSessionInfo.capturerInfo_.sourceType = SOURCE_TYPE_MIC;
    bool hasSession = false;

    bool result = audioCoreService->HandleNormalInputPipes(pipeList, TEST_SESSION_ID,
        runningSessionInfo, hasSession);

    EXPECT_FALSE(result);
    EXPECT_FALSE(hasSession);
    EXPECT_NE(runningSessionInfo.sessionId_, streamDesc->sessionId_);
}

/**
 * @tc.name  : HandleNormalInputPipes_VoipPriorityLowered_002
 * @tc.number: HandleNormalInputPipes_VoipPriorityLowered_002
 * @tc.desc  : Test non-lowered VoIP stream still participates in priority comparison.
 */
HWTEST_F(AudioCoreServiceUnitTest, HandleNormalInputPipes_VoipPriorityLowered_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;
    pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;

    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 700002;
    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    streamDesc->voipPriorityLowered_.store(false);
    pipeInfo->streamDescriptors_.push_back(streamDesc);

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeList = { pipeInfo };
    AudioStreamDescriptor runningSessionInfo = {};
    runningSessionInfo.capturerInfo_.sourceType = SOURCE_TYPE_MIC;
    bool hasSession = false;

    bool result = audioCoreService->HandleNormalInputPipes(pipeList, TEST_SESSION_ID,
        runningSessionInfo, hasSession);

    EXPECT_TRUE(result);
    EXPECT_TRUE(hasSession);
    EXPECT_EQ(runningSessionInfo.sessionId_, streamDesc->sessionId_);
    EXPECT_EQ(runningSessionInfo.capturerInfo_.sourceType, SOURCE_TYPE_VOICE_COMMUNICATION);
}

/**
 * @tc.name  : NotifyVoipPriorityLoweredByMute_001
 * @tc.number: NotifyVoipPriorityLoweredByMute_001
 * @tc.desc  : Test NotifyVoipPriorityLoweredByMute sets lowered flag to true.
 */
HWTEST_F(AudioCoreServiceUnitTest, NotifyVoipPriorityLoweredByMute_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    auto pipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(pipeManager, nullptr);
    pipeManager->curPipeList_.clear();

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;
    pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;

    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 700101;
    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    streamDesc->voipPriorityLowered_.store(false);
    pipeInfo->streamDescriptors_.push_back(streamDesc);
    pipeManager->AddAudioPipeInfo(pipeInfo);

    audioCoreService->NotifyVoipPriorityLoweredByMute(streamDesc->GetSessionId(), true);

    auto streamDescAfter = pipeManager->GetStreamDescById(streamDesc->GetSessionId());
    ASSERT_NE(streamDescAfter, nullptr);
    EXPECT_TRUE(streamDescAfter->voipPriorityLowered_.load());

    pipeManager->curPipeList_.clear();
}

/**
 * @tc.name  : NotifyVoipPriorityLoweredByMute_002
 * @tc.number: NotifyVoipPriorityLoweredByMute_002
 * @tc.desc  : Test NotifyVoipPriorityLoweredByMute clears lowered flag to false.
 */
HWTEST_F(AudioCoreServiceUnitTest, NotifyVoipPriorityLoweredByMute_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    auto pipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(pipeManager, nullptr);
    pipeManager->curPipeList_.clear();

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;
    pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;

    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 700102;
    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    streamDesc->voipPriorityLowered_.store(true);
    pipeInfo->streamDescriptors_.push_back(streamDesc);
    pipeManager->AddAudioPipeInfo(pipeInfo);

    audioCoreService->NotifyVoipPriorityLoweredByMute(streamDesc->GetSessionId(), false);

    auto streamDescAfter = pipeManager->GetStreamDescById(streamDesc->GetSessionId());
    ASSERT_NE(streamDescAfter, nullptr);
    EXPECT_FALSE(streamDescAfter->voipPriorityLowered_.load());

    pipeManager->curPipeList_.clear();
}

/**
 * @tc.name  : NotifyVoipPriorityLoweredByMute_003
 * @tc.number: NotifyVoipPriorityLoweredByMute_003
 * @tc.desc  : Test NotifyVoipPriorityLoweredByMute skips reload when stream is not started.
 */
HWTEST_F(AudioCoreServiceUnitTest, NotifyVoipPriorityLoweredByMute_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    auto pipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(pipeManager, nullptr);
    pipeManager->curPipeList_.clear();

    constexpr uint32_t sentinelSessionId = 700200;
    audioCoreService->SetOpenedNormalSourceSessionId(sentinelSessionId);

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;
    pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;

    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 700103;
    streamDesc->streamStatus_ = STREAM_STATUS_STOPPED;
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    streamDesc->voipPriorityLowered_.store(false);
    pipeInfo->streamDescriptors_.push_back(streamDesc);
    pipeManager->AddAudioPipeInfo(pipeInfo);

    audioCoreService->NotifyVoipPriorityLoweredByMute(streamDesc->GetSessionId(), true);

    auto streamDescAfter = pipeManager->GetStreamDescById(streamDesc->GetSessionId());
    ASSERT_NE(streamDescAfter, nullptr);
    EXPECT_TRUE(streamDescAfter->voipPriorityLowered_.load());
    EXPECT_EQ(audioCoreService->GetOpenedNormalSourceSessionId(), sentinelSessionId);

    pipeManager->curPipeList_.clear();
}

/**
 * @tc.name  : NotifyVoipPriorityLoweredByMute_004
 * @tc.number: NotifyVoipPriorityLoweredByMute_004
 * @tc.desc  : Test NotifyVoipPriorityLoweredByMute skips reload when stream is released.
 */
HWTEST_F(AudioCoreServiceUnitTest, NotifyVoipPriorityLoweredByMute_004, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    auto pipeManager = AudioPipeManager::GetPipeManager();
    ASSERT_NE(pipeManager, nullptr);
    pipeManager->curPipeList_.clear();

    constexpr uint32_t sentinelSessionId = 700201;
    audioCoreService->SetOpenedNormalSourceSessionId(sentinelSessionId);

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;
    pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;

    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->sessionId_ = 700104;
    streamDesc->streamStatus_ = STREAM_STATUS_RELEASED;
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    streamDesc->voipPriorityLowered_.store(false);
    pipeInfo->streamDescriptors_.push_back(streamDesc);
    pipeManager->AddAudioPipeInfo(pipeInfo);

    audioCoreService->NotifyVoipPriorityLoweredByMute(streamDesc->GetSessionId(), true);

    auto streamDescAfter = pipeManager->GetStreamDescById(streamDesc->GetSessionId());
    ASSERT_NE(streamDescAfter, nullptr);
    EXPECT_TRUE(streamDescAfter->voipPriorityLowered_.load());
    EXPECT_EQ(audioCoreService->GetOpenedNormalSourceSessionId(), sentinelSessionId);

    pipeManager->curPipeList_.clear();
}

/**
 * @tc.name  : IsVoipPrivacyBypassedByBehavior_001
 * @tc.number: IsVoipPrivacyBypassedByBehavior_001
 * @tc.desc  : Test descriptor-based voipNoPrivacyFlag check.
 */
HWTEST_F(AudioCoreServiceUnitTest, IsVoipPrivacyBypassedByBehavior_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    auto streamDesc = CreateCapturerStreamDescriptorForVoipPrivacy(710101,
        SOURCE_TYPE_VOICE_COMMUNICATION);
    ASSERT_NE(streamDesc, nullptr);

    EXPECT_FALSE(audioCoreService->IsVoipPrivacyBypassedByBehavior(nullptr));
    EXPECT_FALSE(audioCoreService->IsVoipPrivacyBypassedByBehavior(streamDesc));

    streamDesc->voipNoPrivacyFlag_.store(true);
    EXPECT_TRUE(audioCoreService->IsVoipPrivacyBypassedByBehavior(streamDesc));
}

/**
 * @tc.name  : UpdateVoipNoPrivacyFlagBySessionId_001
 * @tc.number: UpdateVoipNoPrivacyFlagBySessionId_001
 * @tc.desc  : Test UpdateVoipNoPrivacyFlagBySessionId updates descriptor flag.
 */
HWTEST_F(AudioCoreServiceUnitTest, UpdateVoipNoPrivacyFlagBySessionId_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    auto streamDesc = CreateCapturerStreamDescriptorForVoipPrivacy(710102,
        SOURCE_TYPE_VOICE_COMMUNICATION);
    AddCapturerStreamsToPipeManager({streamDesc});

    audioCoreService->UpdateVoipNoPrivacyFlagBySessionId(streamDesc->GetSessionId(), true);
    EXPECT_TRUE(streamDesc->voipNoPrivacyFlag_.load());

    audioCoreService->UpdateVoipNoPrivacyFlagBySessionId(streamDesc->GetSessionId(), false);
    EXPECT_FALSE(streamDesc->voipNoPrivacyFlag_.load());

    ClearCapturerStreamsFromPipeManager();
}

/**
 * @tc.name  : UpdateVoipNoPrivacyFlagByPid_001
 * @tc.number: UpdateVoipNoPrivacyFlagByPid_001
 * @tc.desc  : Test UpdateVoipNoPrivacyFlagByPid updates only matched descriptors.
 */
HWTEST_F(AudioCoreServiceUnitTest, UpdateVoipNoPrivacyFlagByPid_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    auto streamDescByAppPid = CreateCapturerStreamDescriptorForVoipPrivacy(710103,
        SOURCE_TYPE_VOICE_COMMUNICATION);
    streamDescByAppPid->callerPid_ = 0;
    streamDescByAppPid->appInfo_.appPid = 710201;

    auto streamDescByCallerPid = CreateCapturerStreamDescriptorForVoipPrivacy(710104,
        SOURCE_TYPE_MIC, STREAM_STATUS_STARTED, 710202);
    AddCapturerStreamsToPipeManager({streamDescByAppPid, streamDescByCallerPid});

    audioCoreService->UpdateVoipNoPrivacyFlagByPid(710201, true);
    EXPECT_TRUE(streamDescByAppPid->voipNoPrivacyFlag_.load());
    EXPECT_FALSE(streamDescByCallerPid->voipNoPrivacyFlag_.load());

    audioCoreService->UpdateVoipNoPrivacyFlagByPid(710202, true);
    EXPECT_TRUE(streamDescByCallerPid->voipNoPrivacyFlag_.load());

    ClearCapturerStreamsFromPipeManager();
}

/**
 * @tc.name  : EvaluateActivePrivacyVoipPresence_001
 * @tc.number: EvaluateActivePrivacyVoipPresence_001
 * @tc.desc  : Test active VOIP detection with muted and no-privacy exceptions.
 */
HWTEST_F(AudioCoreServiceUnitTest, EvaluateActivePrivacyVoipPresence_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    bool hasActivePrivacyVoipCapturer = false;
    auto activeVoip = CreateCapturerStreamDescriptorForVoipPrivacy(710105,
        SOURCE_TYPE_VOICE_COMMUNICATION);
    audioCoreService->EvaluateActivePrivacyVoipPresence({activeVoip},
        hasActivePrivacyVoipCapturer);
    EXPECT_TRUE(hasActivePrivacyVoipCapturer);

    hasActivePrivacyVoipCapturer = true;
    activeVoip->voipPriorityLowered_.store(true);
    audioCoreService->EvaluateActivePrivacyVoipPresence({activeVoip},
        hasActivePrivacyVoipCapturer);
    EXPECT_FALSE(hasActivePrivacyVoipCapturer);

    hasActivePrivacyVoipCapturer = false;
    activeVoip->voipPriorityLowered_.store(false);
    activeVoip->voipNoPrivacyFlag_.store(true);
    audioCoreService->EvaluateActivePrivacyVoipPresence({activeVoip},
        hasActivePrivacyVoipCapturer);
    EXPECT_FALSE(hasActivePrivacyVoipCapturer);
}

/**
 * @tc.name  : ShouldMuteCapturerByVoipPrivacy_001
 * @tc.number: ShouldMuteCapturerByVoipPrivacy_001
 * @tc.desc  : Test mute target, no-active-voip and ultrasonic bypass branches.
 */
HWTEST_F(AudioCoreServiceUnitTest, ShouldMuteCapturerByVoipPrivacy_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    auto micStream = CreateCapturerStreamDescriptorForVoipPrivacy(710106, SOURCE_TYPE_MIC);
    auto ultrasonicStream = CreateCapturerStreamDescriptorForVoipPrivacy(710107,
        SOURCE_TYPE_ULTRASONIC);
    auto voipStream = CreateCapturerStreamDescriptorForVoipPrivacy(710108,
        SOURCE_TYPE_VOICE_COMMUNICATION);

    EXPECT_TRUE(audioCoreService->ShouldMuteCapturerByVoipPrivacy(micStream, true));
    EXPECT_FALSE(audioCoreService->ShouldMuteCapturerByVoipPrivacy(micStream, false));
    EXPECT_FALSE(audioCoreService->ShouldMuteCapturerByVoipPrivacy(ultrasonicStream, true));
    EXPECT_FALSE(audioCoreService->ShouldMuteCapturerByVoipPrivacy(voipStream, true));
}

/**
 * @tc.name  : CollectSessionsToMuteByVoipPrivacy_001
 * @tc.number: CollectSessionsToMuteByVoipPrivacy_001
 * @tc.desc  : Test collecting mute target sessions under active VOIP.
 */
HWTEST_F(AudioCoreServiceUnitTest, CollectSessionsToMuteByVoipPrivacy_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    auto voipStream = CreateCapturerStreamDescriptorForVoipPrivacy(710109,
        SOURCE_TYPE_VOICE_COMMUNICATION);
    auto micStream = CreateCapturerStreamDescriptorForVoipPrivacy(710110, SOURCE_TYPE_MIC);
    auto ultrasonicStream = CreateCapturerStreamDescriptorForVoipPrivacy(710111,
        SOURCE_TYPE_ULTRASONIC);
    auto wakeupStream = CreateCapturerStreamDescriptorForVoipPrivacy(710112,
        SOURCE_TYPE_WAKEUP);

    std::unordered_set<uint32_t> targetMuteSessions;
    audioCoreService->CollectSessionsToMuteByVoipPrivacy(
        {voipStream, micStream, ultrasonicStream, wakeupStream}, true, targetMuteSessions);

    EXPECT_EQ(targetMuteSessions.count(voipStream->GetSessionId()), 0);
    EXPECT_EQ(targetMuteSessions.count(micStream->GetSessionId()), 1);
    EXPECT_EQ(targetMuteSessions.count(ultrasonicStream->GetSessionId()), 0);
    EXPECT_EQ(targetMuteSessions.count(wakeupStream->GetSessionId()), 1);
}

/**
 * @tc.name  : ReEvaluateVoipPrivacyMuteForCapturers_001
 * @tc.number: ReEvaluateVoipPrivacyMuteForCapturers_001
 * @tc.desc  : Test reevaluate main flow with descriptor-only state and no mute target.
 */
HWTEST_F(AudioCoreServiceUnitTest, ReEvaluateVoipPrivacyMuteForCapturers_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    ASSERT_NE(audioCoreService, nullptr);

    auto activeVoip = CreateCapturerStreamDescriptorForVoipPrivacy(710113,
        SOURCE_TYPE_VOICE_COMMUNICATION);
    auto pausedMic = CreateCapturerStreamDescriptorForVoipPrivacy(710114, SOURCE_TYPE_MIC,
        STREAM_STATUS_PAUSED);
    AddCapturerStreamsToPipeManager({activeVoip, pausedMic});

    audioCoreService->ReEvaluateVoipPrivacyMuteForCapturers(
        "AudioCoreServiceUnitTest::ReEvaluateVoipPrivacyMuteForCapturers_001");
    EXPECT_FALSE(activeVoip->voipPrivacyMuted_.load());
    EXPECT_FALSE(pausedMic->voipPrivacyMuted_.load());

    activeVoip->voipNoPrivacyFlag_.store(true);
    audioCoreService->ReEvaluateVoipPrivacyMuteForCapturers(
        "AudioCoreServiceUnitTest::ReEvaluateVoipPrivacyMuteForCapturers_001");
    EXPECT_FALSE(activeVoip->voipPrivacyMuted_.load());

    ClearCapturerStreamsFromPipeManager();
}
} // namespace AudioStandard
} // namespace OHOS
