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
#include "i_hpae_manager.h"
#include "audio_volume.h"
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
bool g_hasPermission = false;
const uint32_t TEST_SESSION_ID = 100001;
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

void AudioCoreServiceUnitTest::SetUpTestCase(void)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest::SetUpTestCase start-end");
    AudioPolicyServer* server = GetServerPtr();
    server->isUT_ = true;
    GetPermission();
    GetServerPtr()->eventEntry_->NotifyServiceReady();
}
void AudioCoreServiceUnitTest::TearDownTestCase(void)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest::TearDownTestCase start-end");
    AudioPolicyServer* server = GetServerPtr();
    server->isUT_ = false;
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
* @tc.name  : Test AudioCoreService.
* @tc.number: SetDefaultOutputDevice_001
* @tc.desc  : Test SetDefaultOutputDevice - Set DEVICE_TYPE_SPEAKER as default device to nonexistent session.
*/
HWTEST_F(AudioCoreServiceUnitTest, SetDefaultOutputDevice_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest SetDefaultOutputDevice_001 start");
    uint32_t sessionID = 99999; // nonexistent sessionId
    auto result = GetServerPtr()->eventEntry_->SetDefaultOutputDevice(DEVICE_TYPE_SPEAKER,
        sessionID, STREAM_USAGE_MEDIA, false);

    auto desc = GetServerPtr()->eventEntry_->coreService_->pipeManager_->GetStreamDescById(sessionID);
    if (desc == nullptr) {
        EXPECT_EQ(result, ERR_NOT_SUPPORTED);
    } else {
        EXPECT_EQ(result, SUCCESS);
    }
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: SetDefaultOutputDevice_002
* @tc.desc  : Test SetDefaultOutputDevice - Set DEVICE_TYPE_SPEAKER as default device.
*/
HWTEST_F(AudioCoreServiceUnitTest, SetDefaultOutputDevice_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest SetDefaultOutputDevice_002 start");
    uint32_t sessionID = 0; // sessionId

    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamInfo_.format = AudioSampleFormat::SAMPLE_S32LE;
    streamDesc->streamInfo_.samplingRate = AudioSamplingRate::SAMPLE_RATE_96000;
    streamDesc->streamInfo_.channels = AudioChannel::STEREO;
    streamDesc->streamInfo_.encoding = AudioEncodingType::ENCODING_PCM;
    streamDesc->streamInfo_.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    streamDesc->rendererInfo_.streamUsage = STREAM_USAGE_RINGTONE;

    streamDesc->callerUid_ = getuid();
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    streamDesc->startTimeStamp_ = ClockTime::GetCurNano();
    uint32_t flag = AUDIO_OUTPUT_FLAG_NORMAL;
    std::string networkId = LOCAL_NETWORK_ID;
    auto createResult = GetServerPtr()->eventEntry_->CreateRendererClient(streamDesc, flag, sessionID, networkId);
    EXPECT_EQ(createResult, SUCCESS);

    auto result = GetServerPtr()->eventEntry_->SetDefaultOutputDevice(DEVICE_TYPE_SPEAKER,
        sessionID, STREAM_USAGE_MEDIA, false);
    auto desc = GetServerPtr()->eventEntry_->coreService_->pipeManager_->GetStreamDescById(sessionID);
    if (desc == nullptr) {
        EXPECT_EQ(result, ERR_NOT_SUPPORTED);
    } else {
        EXPECT_NE(result, SUCCESS);
    }
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

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: IsSameScene_001
* @tc.desc  : Test IsSameScene.
*/
HWTEST_F(AudioCoreServiceUnitTest, IsSameScene_001, TestSize.Level1)
{
    GetServerPtr()->coreService_->audioActiveDevice_.currentActiveDevice_.deviceType_ =
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
    GetServerPtr()->coreService_->audioActiveDevice_.currentActiveDevice_.deviceType_ =
        DeviceType::DEVICE_TYPE_SPEAKER;
    GetServerPtr()->coreService_->audioActiveDevice_.currentActiveDevice_.networkId_ =
        REMOTE_NETWORK_ID;
    GetServerPtr()->eventEntry_->SetAudioScene(AUDIO_SCENE_RINGING);
    int32_t result = GetServerPtr()->eventEntry_->SetAudioScene(AUDIO_SCENE_DEFAULT);
    EXPECT_EQ(result, SUCCESS);

    GetServerPtr()->coreService_->audioActiveDevice_.currentActiveDevice_.networkId_ =
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
    EXPECT_EQ(result, ERR_INVALID_PARAM);
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
* @tc.number: HandleNearlinkErrResult_001
* @tc.desc  : Test HandleNearlinkErrResult
*/
HWTEST_F(AudioCoreServiceUnitTest, HandleNearlinkErrResult_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest HandleNearlinkErrResult_001 start");
    auto coreSvc = AudioCoreService::GetCoreService();
    int32_t result = 200;
    auto devDesc = make_shared<AudioDeviceDescriptor>();
    coreSvc->HandleNearlinkErrResult(result, devDesc, true);
    result = 404;
    coreSvc->HandleNearlinkErrResult(result, devDesc, true);
    EXPECT_NE(devDesc, nullptr);
    result = 201;
    coreSvc->HandleNearlinkErrResult(result, devDesc, true);
    EXPECT_EQ(devDesc->deviceUsage_, static_cast<DeviceUsage>(static_cast<uint32_t>(devDesc->deviceUsage_) &
        ~static_cast<uint32_t>(DeviceUsage::VOICE)));
    coreSvc->HandleNearlinkErrResult(result, devDesc, false);
    EXPECT_EQ(devDesc->deviceUsage_, static_cast<DeviceUsage>(static_cast<uint32_t>(devDesc->deviceUsage_) &
        ~static_cast<uint32_t>(DeviceUsage::MEDIA)));
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
        GetServerPtr()->eventEntry_->GetPreferredOutputDeviceDescriptors(rendererInfo, networkId);

        rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
        GetServerPtr()->eventEntry_->GetPreferredOutputDeviceDescriptors(rendererInfo, networkId);
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
    GetServerPtr()->coreService_->policyConfigMananger_.audioPolicyConfig_
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
    EXPECT_EQ(streamDesc->GetUltraFastFlag(), false);
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
        streamDescs[i]->startTimeStamp_ = streamDescs[i]->createTimeStamp_ + 1;
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
    StreamVolume streamVolume(sessionId, streamType, streamUsage, appUid, pid, false, 1, false);
    audioVolume->streamVolume_.emplace(sessionId, streamVolume);

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
    StreamVolume streamVolume1(sessionId, streamType, streamUsage, appUid, pid, false, 1, false);
    StreamVolume streamVolume2(anotherSessionId, streamType, streamUsage, anotherAppUid, pid, false, 1, false);
    audioVolume->streamVolume_.emplace(sessionId, streamVolume1);
    audioVolume->streamVolume_.emplace(anotherSessionId, streamVolume2);

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
    StreamVolume streamVolume(sessionId, streamType, streamUsage, appUid, pid, false, 1, false);
    audioVolume->streamVolume_.emplace(sessionId, streamVolume);

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
    StreamVolume streamVolume(sessionId, streamType, streamUsage, appUid, pid, false, 1, false);
    audioVolume->streamVolume_.emplace(sessionId, streamVolume);

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

    EXPECT_EQ(audioCoreService->audioVolumeManager_.IsRingerModeMute(), true);

    AUDIO_INFO_LOG("AudioCoreServiceUnitTest UpdateRingerOrAlarmerDualDeviceOutputRouter_005 end");
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
    AudioDeviceDescriptor curDesc(DeviceType::DEVICE_TYPE_NEARLINK, DeviceRole::OUTPUT_DEVICE);
    audioCoreService->audioActiveDevice_.SetCurrentOutputDevice(curDesc);
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

    server->coreService_->audioActiveDevice_.currentActiveDevice_.deviceType_ =
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP;
    server->coreService_->audioActiveDevice_.currentActiveDevice_.networkId_ = LOCAL_NETWORK_ID;
    int32_t ret = server->coreService_->A2dpOffloadGetRenderPosition(delayValue, sendDataSize, timeStamp);
    EXPECT_EQ(ret, SUCCESS);

    server->coreService_->audioActiveDevice_.currentActiveDevice_.deviceType_ =
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP;
    server->coreService_->audioActiveDevice_.currentActiveDevice_.networkId_ = REMOTE_NETWORK_ID;
    ret = server->coreService_->A2dpOffloadGetRenderPosition(delayValue, sendDataSize, timeStamp);
    EXPECT_EQ(ret, SUCCESS);

    server->coreService_->audioActiveDevice_.currentActiveDevice_.deviceType_ =
        DeviceType::DEVICE_TYPE_SPEAKER;
    server->coreService_->audioActiveDevice_.currentActiveDevice_.networkId_ = REMOTE_NETWORK_ID;
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
    deviceDesc->SetDeviceSupportMmap(0);
    EXPECT_EQ(audioCoreService->RecordIsForcedNormal(streamDesc), true);
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
    deviceDesc->SetDeviceSupportMmap(0);
    EXPECT_EQ(audioCoreService->IsForcedNormal(streamDesc), true);
    deviceDesc->SetDeviceSupportMmap(1);
    EXPECT_EQ(audioCoreService->IsForcedNormal(streamDesc), false);
}
} // namespace AudioStandard
} // namespace OHOS
