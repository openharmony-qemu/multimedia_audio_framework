/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <atomic>
#include <thread>
#include "audio_policy_server.h"
#include "audio_device_info.h"
#include "message_parcel.h"
#include "accesstoken_kit.h"
#include "audio_routing_manager.h"
#include "audio_stream_manager.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
#include "audio_policy_utils.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
bool g_hasPermission = false;
const int32_t MOD_NUM_TWO = 2;
const int32_t SYSTEM_ABILITY_ID = 3009;
const bool RUN_ON_CREATE = false;
const uint32_t CHANNELS = 2;
const uint32_t RATE = 4;
const uint64_t SESSIONID = 123456;
constexpr int32_t DEFAULT_STREAM_ID = 10;
bool g_hasServerInit = false;
const int64_t ACTIVEBTTIME = 60 * 1140 * 2;
const uint32_t ENUM_NUM = 4;
const int32_t A2DP_PLAYING = 2;
const int32_t A2DP_STOPPED = 1;
const std::string SPLITARGS = "splitArgs";
const std::string NETWORKID = "networkId";
const int32_t SESSIONID_32 = 123456;
static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;

/*
* describe: get data from outside untrusted data(RAW_DATA) which size is according to sizeof(T)
* tips: only support basic type
*/
template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (RAW_DATA == nullptr || objectSize > g_dataSize - g_pos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, RAW_DATA + g_pos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_pos += objectSize;
    return object;
}

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

AudioPolicyServer *GetServerPtr()
{
    static AudioPolicyServer server(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    if (!g_hasServerInit) {
        server.OnStart();
        server.OnAddSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID, "");
#ifdef FEATURE_MULTIMODALINPUT_INPUT
        server.OnAddSystemAbility(MULTIMODAL_INPUT_SERVICE_ID, "");
#endif
        server.OnAddSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID, "");
        server.OnAddSystemAbility(POWER_MANAGER_SERVICE_ID, "");
        server.OnAddSystemAbility(SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN, "");
        server.audioPolicyService_.SetDefaultDeviceLoadFlag(true);
        g_hasServerInit = true;
    }
    return &server;
}

static AudioProcessConfig InitProcessConfig()
{
    AudioProcessConfig config;
    config.appInfo.appUid = DEFAULT_STREAM_ID;
    config.appInfo.appPid = DEFAULT_STREAM_ID;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_RECORD;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    return config;
}

void AudioFuzzTestGetPermission()
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
            .processName = "audiofuzztest",
            .aplStr = "system_basic",
        };
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
        g_hasPermission = true;
    }
}

void InitGetServerService(DeviceRole deviceRole, AudioStreamInfo audioStreamInfo_2)
{
    AudioStreamInfo audioStreamInfo = {};
    audioStreamInfo.samplingRate = GetData<AudioSamplingRate>();
    audioStreamInfo.encoding = GetData<AudioEncodingType>();
    audioStreamInfo.format = GetData<AudioSampleFormat>();
    audioStreamInfo.channels = GetData<AudioChannel>();

    GetServerPtr()->audioPolicyService_.audioActiveDevice_.activeBTDevice_ = "activeBTDevice";
    A2dpDeviceConfigInfo configInfo = {audioStreamInfo, false};
    GetServerPtr()->audioPolicyService_.audioA2dpDevice_.connectedA2dpDeviceMap_.insert({"activeBTDevice",
        configInfo});
    GetServerPtr()->audioPolicyService_.audioA2dpDevice_.connectedA2dpDeviceMap_.insert({"A2dpDeviceCommon", {}});
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.networkId_ = LOCAL_NETWORK_ID;
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_
        = DEVICE_TYPE_BLUETOOTH_A2DP;
}

void ThreadFunctionTest()
{
    GetServerPtr()->audioPolicyService_.audioConfigManager_.isAdapterInfoMap_.store(true);
}

void AudioPolicyServiceSecondTest(AudioStreamInfo audioStreamInfo,
    std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor)
{
    bool isConnected = GetData<bool>();
    GetServerPtr()->audioPolicyService_.audioCapturerSession_.HandleRemoteCastDevice(isConnected, audioStreamInfo);
    GetServerPtr()->audioPolicyService_.audioConfigManager_.OnVoipConfigParsed(isConnected);
    GetServerPtr()->audioPolicyService_.audioConfigManager_.GetVoipConfig();
    pid_t clientPid = GetData<pid_t>();
    GetServerPtr()->audioPolicyService_.ReduceAudioPolicyClientProxyMap(clientPid);
    AudioStreamChangeInfo streamChangeInfo;
    int32_t clientUID = GetData<int32_t>();
    int32_t sessionId = GetData<int32_t>();
    int32_t clientPid1 = GetData<int32_t>();
    streamChangeInfo.audioRendererChangeInfo.clientUID = clientUID;
    streamChangeInfo.audioRendererChangeInfo.sessionId = sessionId;
    streamChangeInfo.audioRendererChangeInfo.clientPid = clientPid1;
    streamChangeInfo.audioRendererChangeInfo.rendererState = RENDERER_NEW;
    streamChangeInfo.audioRendererChangeInfo.rendererInfo = {};

    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    std::vector<AudioMode> audioMode = { AUDIO_MODE_RECORD, AUDIO_MODE_PLAYBACK };
    uint32_t modeInt = GetData<uint32_t>() % audioMode.size();
    AudioMode mode = audioMode[modeInt];
    GetServerPtr()->audioPolicyService_.RegisterTracker(mode, streamChangeInfo, object, sessionId);
    std::vector<DeviceType> DeviceTypeVec = {
        DEVICE_TYPE_BLUETOOTH_SCO,
        DEVICE_TYPE_USB_ARM_HEADSET,
        DEVICE_TYPE_BLUETOOTH_A2DP,
        DEVICE_TYPE_FILE_SINK,
        DEVICE_TYPE_DP,
    };
    uint32_t deviceTypeInt = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeInt];
    GetServerPtr()->audioPolicyService_.audioIOHandleMap_.GetSinkIOHandle(deviceType);
    std::vector<DeviceType> DeviceTypeSourceVec = {
        DEVICE_TYPE_USB_ARM_HEADSET,
        DEVICE_TYPE_MIC,
        DEVICE_TYPE_FILE_SOURCE,
        DEVICE_TYPE_DP,
    };
    uint32_t deviceTypeSouInt = GetData<uint32_t>() % DeviceTypeSourceVec.size();
    DeviceType deviceTypeSou = DeviceTypeSourceVec[deviceTypeSouInt];
    GetServerPtr()->audioPolicyService_.audioIOHandleMap_.GetSourceIOHandle(deviceTypeSou);
    SinkInput sinkInput = {};
    SourceOutput sourceOutput = {};
    GetServerPtr()->audioPolicyService_.audioDeviceStatus_.WriteOutputDeviceChangedSysEvents(remoteDeviceDescriptor,
        sinkInput);
    GetServerPtr()->audioPolicyService_.audioDeviceStatus_.WriteInputDeviceChangedSysEvents(remoteDeviceDescriptor,
        sourceOutput);
}

void AudioPolicyServiceThirdTest()
{
    AudioProcessConfig config = InitProcessConfig();
    std::thread t1(ThreadFunctionTest);
    t1.join();
    StreamPropInfo streamPropInfo;
    AudioAdapterInfo adapterInfo = {};
    adapterInfo.adapterName_ = "wakeup_input";
    adapterInfo.adaptersupportScene_ = "supportScene";
    std::list<PipeInfo> pipeInfos_;
    PipeInfo pipeInfo = {};
    pipeInfo.name_ = "wakeup_input";
    pipeInfo.streamPropInfos_.push_back(streamPropInfo);
    pipeInfos_.push_back(pipeInfo);
    adapterInfo.pipeInfos_ = pipeInfos_;
    GetServerPtr()->audioPolicyService_.audioConfigManager_.adapterInfoMap_ = {};
    GetServerPtr()->audioPolicyService_.audioConfigManager_.adapterInfoMap_.
        insert({AdaptersType::TYPE_PRIMARY, adapterInfo});
    GetServerPtr()->audioPolicyService_.SetWakeUpAudioCapturerFromAudioServer(config);

    vector<shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    AudioStreamManager::GetInstance()->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    GetServerPtr()->audioPolicyService_.RegisterRemoteDevStatusCallback();
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_
        = DEVICE_TYPE_BLUETOOTH_A2DP;
    uint32_t channelcount = GetData<uint32_t>();
    GetServerPtr()->audioPolicyService_.ReconfigureAudioChannel(channelcount, DEVICE_TYPE_FILE_SINK);
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_FILE_SINK;
    GetServerPtr()->audioPolicyService_.ReconfigureAudioChannel(channelcount, DEVICE_TYPE_FILE_SINK);
    GetServerPtr()->audioPolicyService_.ReconfigureAudioChannel(channelcount, DEVICE_TYPE_FILE_SOURCE);
    std::vector<DeviceType> DeviceTypeVec = {
        DEVICE_TYPE_BLUETOOTH_A2DP,
        DEVICE_TYPE_BLUETOOTH_SCO,
    };
    uint32_t deviceTypeInt = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeInt];
    GetServerPtr()->audioPolicyService_.audioVolumeManager_.IsBlueTooth(deviceType);
    GetServerPtr()->audioPolicyService_.audioVolumeManager_.activeSafeTimeBt_ = ACTIVEBTTIME;
    GetServerPtr()->audioPolicyService_.audioVolumeManager_.CheckBlueToothActiveMusicTime(1);
    GetServerPtr()->audioPolicyService_.audioVolumeManager_.CheckWiredActiveMusicTime(1);
    int32_t safeVolume = GetData<int32_t>();
    GetServerPtr()->audioPolicyService_.audioVolumeManager_.CheckBlueToothActiveMusicTime(safeVolume);
    GetServerPtr()->audioPolicyService_.audioVolumeManager_.CheckWiredActiveMusicTime(safeVolume);
}

void MakeAdapterInfoMap()
{
    AudioAdapterInfo adapterInfo = {};
    adapterInfo.adapterName_ = "wakeup_input";
    adapterInfo.adaptersupportScene_ = "supportScene";
    std::list<PipeInfo> pipeInfos_;
    PipeInfo pipeInfo = {};
    pipeInfo.name_ = "primary_input";
    StreamPropInfo streamPropInfo;
    pipeInfo.streamPropInfos_.push_back(streamPropInfo);
    pipeInfos_.push_back(pipeInfo);
    adapterInfo.pipeInfos_ = pipeInfos_;
    GetServerPtr()->audioPolicyService_.audioConfigManager_.adapterInfoMap_.
        insert({AdaptersType::TYPE_PRIMARY, adapterInfo});
}

void AudioPolicyServiceTest()
{
    AudioStreamInfo streamInfo;
    streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    streamInfo.channels = AudioChannel::STEREO;
    streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    SessionInfo sessionInfo;
    sessionInfo.sourceType = SOURCE_TYPE_VOICE_CALL;
    sessionInfo.rate = RATE;
    sessionInfo.channels = CHANNELS;
    MakeAdapterInfoMap();
    GetServerPtr()->audioPolicyService_.OnCapturerSessionAdded(SESSIONID, sessionInfo, streamInfo);

    uint32_t deviceRoleInt = GetData<uint32_t>();
    deviceRoleInt = (deviceRoleInt % ENUM_NUM) - 1;
    DeviceRole deviceRole = static_cast<DeviceRole>(deviceRoleInt);
    std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();

    AudioStreamInfo audioStreamInfo = {};
    audioStreamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    audioStreamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    audioStreamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioStreamInfo.channels = AudioChannel::STEREO;
    InitGetServerService(deviceRole, audioStreamInfo);
    GetServerPtr()->audioPolicyService_.OnDeviceConfigurationChanged(DEVICE_TYPE_BLUETOOTH_A2DP,
        GetServerPtr()->audioPolicyService_.audioActiveDevice_.activeBTDevice_, "DeviceName", audioStreamInfo);

    shared_ptr<AudioDeviceDescriptor> dis = make_shared<AudioDeviceDescriptor>();
    dis->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    dis->macAddress_ = GetServerPtr()->audioPolicyService_.audioActiveDevice_.activeBTDevice_;
    dis->deviceRole_ = OUTPUT_DEVICE;
    GetServerPtr()->audioPolicyService_.audioDeviceManager_.connectedDevices_.push_back(dis);

    std::vector<DeviceType> DeviceTypeVec = {
        DEVICE_TYPE_BLUETOOTH_A2DP,
        DEVICE_TYPE_BLUETOOTH_SCO,
    };
    uint32_t deviceTypeInt = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeInt];
    GetServerPtr()-> audioPolicyService_.OnForcedDeviceSelected(deviceType,
        GetServerPtr()->audioPolicyService_.audioActiveDevice_.activeBTDevice_);
    AudioPolicyServiceSecondTest(audioStreamInfo, remoteDeviceDescriptor);
    AudioPolicyServiceThirdTest();
}

void AudioPolicyServiceTestII()
{
    int32_t volumeLevel = GetData<int32_t>();
    bool isA2dpDevice = GetData<bool>();
    GetServerPtr()->audioPolicyService_.audioVolumeManager_.DealWithSafeVolume(volumeLevel, isA2dpDevice);
    AudioStreamInfo audioStreamInfo = {};
    audioStreamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    audioStreamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    audioStreamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioStreamInfo.channels = AudioChannel::STEREO;
    A2dpDeviceConfigInfo configInfo = {audioStreamInfo, true};
    volumeLevel = GetServerPtr()->audioPolicyService_.audioPolicyManager_.GetSafeVolumeLevel() + MOD_NUM_TWO;
    GetServerPtr()->audioPolicyService_.audioA2dpDevice_.connectedA2dpDeviceMap_.insert({"activeBTDevice_1",
        configInfo});
    GetServerPtr()->audioPolicyService_.SetA2dpDeviceVolume("activeBTDevice_1", volumeLevel, isA2dpDevice);
    DeviceType devType = GetData<DeviceType>();
    DeviceBlockStatus status = GetData<DeviceBlockStatus>();
    GetServerPtr()->audioPolicyService_.OnMicrophoneBlockedUpdate(devType, status);
    vector<shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    AudioStreamManager::GetInstance()->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    for (auto &capturerChangeInfo : audioCapturerChangeInfos) {
        capturerChangeInfo->capturerInfo.sourceType = SOURCE_TYPE_VIRTUAL_CAPTURE;
        capturerChangeInfo->capturerState = CAPTURER_PREPARED;
    }
    GetServerPtr()->audioPolicyService_.OnReceiveBluetoothEvent(
        GetServerPtr()->audioPolicyService_.audioActiveDevice_.activeBTDevice_, "deviceName");
    GetServerPtr()->audioPolicyService_.GetAudioEffectOffloadFlag();
    GetServerPtr()->audioPolicyService_.audioOffloadStream_.CheckSpatializationAndEffectState();
    int32_t sessionId = GetData<int32_t>();
    GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->IsA2dpOffloadConnecting(sessionId);
    std::vector<int32_t> playingStateVec = {
        A2DP_STOPPED,
        A2DP_PLAYING,
    };
    int32_t playingStateInt = GetData<int32_t>() % playingStateVec.size();
    int32_t playingState = playingStateVec[playingStateInt];
    GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->a2dpOffloadDeviceAddress_ = "A2dpMacAddress";
    GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->audioA2dpOffloadFlag_.currentOffloadConnectionState_
        = CONNECTION_STATUS_CONNECTED;
    GetServerPtr()->
        audioPolicyService_.audioA2dpOffloadManager_->OnA2dpPlayingStateChanged("A2dpMacAddressS", playingState);
    GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->audioA2dpOffloadFlag_.currentOffloadConnectionState_
        = CONNECTION_STATUS_CONNECTING;
    GetServerPtr()->
        audioPolicyService_.audioA2dpOffloadManager_->OnA2dpPlayingStateChanged("A2dpMacAddress", playingState);
    GetServerPtr()->audioPolicyService_.LoadSplitModule("", NETWORKID);
    GetServerPtr()->audioPolicyService_.LoadSplitModule(SPLITARGS, NETWORKID);
}

void AudioPolicyServiceTestIII()
{
    uint32_t rotate = GetData<uint32_t>();
    GetServerPtr()->audioPolicyService_.SetRotationToEffect(rotate);
    AudioRendererChangeInfo audioRendererChangeInfo;
    int32_t clientUID = GetData<int32_t>();
    int32_t sessionId = GetData<int32_t>();
    int32_t clientPid1 = GetData<int32_t>();
    audioRendererChangeInfo.clientUID = clientUID;
    audioRendererChangeInfo.sessionId = sessionId;
    audioRendererChangeInfo.clientPid = clientPid1;
    audioRendererChangeInfo.rendererState = RENDERER_NEW;
    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    audioRendererChangeInfo.rendererInfo = rendererInfo;
    GetServerPtr()->audioPolicyService_.audioScene_ = AUDIO_SCENE_PHONE_CALL;
    GetServerPtr()->audioPolicyService_.streamCollector_.audioRendererChangeInfos_.
        push_back(make_shared<AudioRendererChangeInfo>(audioRendererChangeInfo));
    GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->WaitForConnectionCompleted();

    std::string dumpString = "";
    GetServerPtr()->audioPolicyDump_.AudioStreamDump(dumpString);
    GetServerPtr()->audioPolicyService_.audioVolumeManager_.ringerModeMute_ = true;
    GetServerPtr()->audioPolicyService_.ResetRingerModeMute();

    AudioStreamInfo audioStreamInfo = {};
    audioStreamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    audioStreamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    audioStreamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioStreamInfo.channels = AudioChannel::STEREO;
    DeviceType deviceType = GetData<DeviceType>();
    GetServerPtr()->audioPolicyService_.audioDeviceStatus_.ReloadA2dpOffloadOnDeviceChanged(deviceType,
        GetServerPtr()->audioPolicyService_.audioActiveDevice_.activeBTDevice_, "DeviceName", audioStreamInfo);

    int32_t sessionId = GetData<int32_t>();
    RestoreInfo restoreInfo;
    restoreInfo.restoreReason = static_cast<RestoreReason>(GetData<int32_t>());
    restoreInfo.targetStreamFlag = GetData<int32_t>();
    GetServerPtr()->audioPolicyService_.RestoreSession(sessionId, restoreInfo);
}

void AudioPolicyServiceTestIV()
{
    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    if (audioRendererFilter == nullptr) {return;}
    audioRendererFilter->uid = getuid();
    audioRendererFilter->rendererInfo.rendererFlags = STREAM_FLAG_FAST;
    audioRendererFilter->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    AudioRoutingManager::GetInstance()->
        GetPreferredOutputDeviceForRendererInfo(audioRendererFilter->rendererInfo, desc);
    AudioModuleInfo audioModuleInfo;
    AudioIOHandle ioHandle = GetServerPtr()->audioPolicyService_.audioPolicyManager_.OpenAudioPort(audioModuleInfo);
    std::string moduleName = "moduleName";
    GetServerPtr()->audioPolicyService_.audioIOHandleMap_.AddIOHandleInfo(moduleName, ioHandle);
    GetServerPtr()->audioPolicyService_.CloseWakeUpAudioCapturer();
    AudioDeviceDescriptor newDeviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    newDeviceInfo.networkId_ = LOCAL_NETWORK_ID;
    newDeviceInfo.macAddress_ = GetServerPtr()->audioPolicyService_.audioActiveDevice_.activeBTDevice_;
    int32_t sessionId = GetData<int32_t>();
    AudioPolicyUtils::GetInstance().GetSinkName(newDeviceInfo, sessionId);
    AudioDeviceDescriptor ads;
    ads.networkId_ = LOCAL_NETWORK_ID;
    ads.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    AudioPolicyUtils::GetInstance().GetSinkName(ads, sessionId);
}

typedef void (*TestFuncs[4])();

TestFuncs g_testFuncs = {
    AudioPolicyServiceTest,
    AudioPolicyServiceTestII,
    AudioPolicyServiceTestIII,
    AudioPolicyServiceTestIV,
};

bool FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return false;
    }

    // initialize data
    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;

    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testFuncs);
    if (len > 0) {
        g_testFuncs[code % len]();
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }

    return true;
}
} // namespace AudioStandard
} // namesapce OHOS

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    OHOS::AudioStandard::AudioFuzzTestGetPermission();
    return 0;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
