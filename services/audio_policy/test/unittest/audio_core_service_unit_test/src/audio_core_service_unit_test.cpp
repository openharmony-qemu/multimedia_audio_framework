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
    GetPermission();
    HPAE::IHpaeManager::GetHpaeManager().Init();
    GetServerPtr()->coreService_->OnServiceConnected(HDI_SERVICE_INDEX);
}
void AudioCoreServiceUnitTest::TearDownTestCase(void)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest::TearDownTestCase start-end");
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
* @tc.number: SetDefaultOutputDevice_001
* @tc.desc  : Test SetDefaultOutputDevice - Set DEVICE_TYPE_SPEAKER as default device.
*/
HWTEST_F(AudioCoreServiceUnitTest, SetDefaultOutputDevice_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest SetDefaultOutputDevice_001 start");
    uint32_t sessionID = 100001; // sessionId
    auto result = GetServerPtr()->eventEntry_->SetDefaultOutputDevice(DEVICE_TYPE_SPEAKER,
        sessionID, STREAM_USAGE_MEDIA, false);
    EXPECT_EQ(result, ERR_NOT_SUPPORTED);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: SetDefaultOutputDevice_002
* @tc.desc  : Test SetDefaultOutputDevice - Set DEVICE_TYPE_BLUETOOTH_A2DP as default device.
*/
HWTEST_F(AudioCoreServiceUnitTest, SetDefaultOutputDevice_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest SetDefaultOutputDevice_002 start");
    uint32_t sessionID = 100001; // sessionId
    auto result = GetServerPtr()->eventEntry_->SetDefaultOutputDevice(DEVICE_TYPE_BLUETOOTH_A2DP,
        sessionID, STREAM_USAGE_MEDIA, false);
    EXPECT_EQ(result, ERR_NOT_SUPPORTED);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: GetAdapterNameBySessionId_001
* @tc.desc  : Test GetAdapterNameBySessionId - invalid session id return "".
*/
HWTEST_F(AudioCoreServiceUnitTest, GetAdapterNameBySessionId_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioCoreServiceUnitTest GetAdapterNameBySessionId_001 start");
    uint32_t sessionID = 100001; // sessionId
    auto result = GetServerPtr()->eventEntry_->GetAdapterNameBySessionId(sessionID);
    EXPECT_EQ(result, "Speaker");
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
    auto result = GetServerPtr()->eventEntry_->GetProcessDeviceInfoBySessionId(sessionID, deviceDesc, info);
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
    EXPECT_EQ(SUCCESS, result);
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
        GetServerPtr()->eventEntry_->GetPreferredInputDeviceDescriptors(capturerInfo, networkId);

        capturerInfo.sourceType = SOURCE_TYPE_MIC;
        GetServerPtr()->eventEntry_->GetPreferredInputDeviceDescriptors(capturerInfo, networkId);
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
* @tc.number: GetPreferredOutputStreamType_001
* @tc.desc  : Test interface GetPreferredOutputStreamType - will return SUCCESS.
*/
HWTEST_F(AudioCoreServiceUnitTest, GetPreferredOutputStreamType_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);

    AudioRendererInfo rendererInfo;
    int32_t ret = 0;
    server->GetPreferredOutputStreamType(rendererInfo, ret);
    EXPECT_EQ(ret, 0);
    server->coreService_->isFastControlled_ = true;
    server->GetPreferredOutputStreamType(rendererInfo, ret);
    EXPECT_EQ(ret, 0);
    rendererInfo.rendererFlags = AUDIO_FLAG_MMAP;
    server->GetPreferredOutputStreamType(rendererInfo, ret);
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name  : Test AudioCoreService.
* @tc.number: GetPreferredInputStreamType_001
* @tc.desc  : Test interface GetPreferredInputStreamType - will return SUCCESS.
*/
HWTEST_F(AudioCoreServiceUnitTest, GetPreferredInputStreamType_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    EXPECT_NE(nullptr, server);

    AudioCapturerInfo capturerInfo;
    int32_t ret = 0;
    server->GetPreferredInputStreamType(capturerInfo, ret);
    EXPECT_EQ(ret, 0);
    server->coreService_->isFastControlled_ = true;
    server->GetPreferredInputStreamType(capturerInfo, ret);
    EXPECT_EQ(ret, 0);
    capturerInfo.capturerFlags = AUDIO_FLAG_MMAP;
    server->GetPreferredInputStreamType(capturerInfo, ret);
    EXPECT_EQ(ret, 0);
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
    std::string expectedDump = "Select device history infos\n  - TotalPipeNums: 0\n\n\n";
    EXPECT_EQ(dumpString, expectedDump);
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
    std::string expectedDump = "Select device history infos\n  - TotalPipeNums: 2\n\nHistoryRecord1\n"
                               "HistoryRecord2\n\n";
    EXPECT_EQ(dumpString, expectedDump);
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
} // namespace AudioStandard
} // namespace OHOS
