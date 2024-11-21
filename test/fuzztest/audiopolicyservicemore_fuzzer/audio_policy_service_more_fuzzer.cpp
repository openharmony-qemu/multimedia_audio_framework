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
constexpr int32_t OFFSET = 4;
const int32_t MOD_NUM_TWO = 2;
const int32_t CONNECTING_NUMBER = 10;
const std::u16string FORMMGR_INTERFACE_TOKEN = u"IAudioPolicy";
const int32_t SYSTEM_ABILITY_ID = 3009;
const bool RUN_ON_CREATE = false;
const int32_t LIMITSIZE = 4;
const int32_t SHIFT_LEFT_8 = 8;
const int32_t SHIFT_LEFT_16 = 16;
const int32_t SHIFT_LEFT_24 = 24;
const uint32_t LIMIT_ONE = 0;
const uint32_t LIMIT_TWO = 30;
const uint32_t LIMIT_THREE = 60;
const uint32_t LIMIT_FOUR = static_cast<uint32_t>(AudioPolicyInterfaceCode::AUDIO_POLICY_MANAGER_CODE_MAX);
const uint32_t CHANNELS = 2;
const uint32_t RATE = 4;
const uint64_t SESSIONID = 123456;
constexpr int32_t DEFAULT_STREAM_ID = 10;
bool g_hasServerInit = false;
const int64_t ACTIVEBTTIME = 60*1140*2;
const uint32_t ENUM_NUM = 4;
const int32_t A2DP_PLAYING = 2;
const int32_t A2DP_STOPPED = 1;
const std::string SPLITARGS = "splitArgs";
const std::string NETWORKID = "networkId";
const int32_t SESSIONID_32 = 123456;

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

void InitGetServerService(const uint8_t *rawData, size_t size, DeviceRole deviceRole,
    AudioStreamInfo audioStreamInfo_2)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioStreamInfo audioStreamInfo = {};
    audioStreamInfo.samplingRate = *reinterpret_cast<const AudioSamplingRate *>(rawData);
    audioStreamInfo.encoding = *reinterpret_cast<const AudioEncodingType *>(rawData);
    audioStreamInfo.format = *reinterpret_cast<const AudioSampleFormat *>(rawData);
    audioStreamInfo.channels = *reinterpret_cast<const AudioChannel *>(rawData);

    GetServerPtr()->audioPolicyService_.audioActiveDevice_.activeBTDevice_ = "activeBTDevice";
    A2dpDeviceConfigInfo configInfo = {audioStreamInfo, false};
    GetServerPtr()->audioPolicyService_.audioA2dpDevice_.connectedA2dpDeviceMap_.insert({"activeBTDevice",
        configInfo});
    GetServerPtr()->audioPolicyService_.audioA2dpDevice_.connectedA2dpDeviceMap_.insert({"A2dpDeviceCommon", {}});
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.networkId_ = LOCAL_NETWORK_ID;
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_
        = DEVICE_TYPE_BLUETOOTH_A2DP;

    AudioModuleInfo audioModuleInfo = AudioPolicyUtils::GetInstance().ConstructRemoteAudioModuleInfo(LOCAL_NETWORK_ID,
        deviceRole, DEVICE_TYPE_BLUETOOTH_A2DP);
    GetServerPtr()->audioPolicyService_.audioConfigManager_.deviceClassInfo_.insert({ClassType::TYPE_A2DP,
        {audioModuleInfo}});

    AudioIOHandle ioHandle = GetServerPtr()->audioPolicyService_.audioPolicyManager_.OpenAudioPort(audioModuleInfo);

    GetServerPtr()->audioPolicyService_.audioIOHandleMap_.AddIOHandleInfo(audioModuleInfo.name, ioHandle);
}

void ThreadFunctionTest()
{
    GetServerPtr()->audioPolicyService_.audioConfigManager_.isAdapterInfoMap_.store(true);
}

void AudioPolicyServiceSecondTest(const uint8_t* rawData, size_t size, AudioStreamInfo audioStreamInfo,
    std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    GetServerPtr()->audioPolicyService_.LoadSinksForCapturer();
    GetServerPtr()->audioPolicyService_.HandleRemoteCastDevice(true, audioStreamInfo);
    GetServerPtr()->audioPolicyService_.HandleRemoteCastDevice(false, audioStreamInfo);
    GetServerPtr()->audioPolicyService_.audioConfigManager_.OnVoipConfigParsed(false);
    GetServerPtr()->audioPolicyService_.audioConfigManager_.GetVoipConfig();
    pid_t clientPid = *reinterpret_cast<const pid_t*>(rawData);
    GetServerPtr()->audioPolicyService_.ReduceAudioPolicyClientProxyMap(clientPid);
    AudioStreamChangeInfo streamChangeInfo;
    int32_t clientUID = *reinterpret_cast<const int32_t*>(rawData);
    int32_t sessionId = *reinterpret_cast<const int32_t*>(rawData);
    int32_t clientPid_1 = *reinterpret_cast<const int32_t*>(rawData);
    streamChangeInfo.audioRendererChangeInfo.clientUID = clientUID;
    streamChangeInfo.audioRendererChangeInfo.sessionId = sessionId;
    streamChangeInfo.audioRendererChangeInfo.clientPid = clientPid_1;
    streamChangeInfo.audioRendererChangeInfo.rendererState = RENDERER_NEW;
    streamChangeInfo.audioRendererChangeInfo.rendererInfo = {};

    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    AudioMode mode = AUDIO_MODE_RECORD;
    GetServerPtr()->audioPolicyService_.RegisterTracker(mode, streamChangeInfo, object, sessionId);
    mode = AUDIO_MODE_PLAYBACK;
    GetServerPtr()->audioPolicyService_.RegisterTracker(mode, streamChangeInfo, object, sessionId);
    GetServerPtr()->audioPolicyService_.audioIOHandleMap_.GetSinkIOHandle(DEVICE_TYPE_BLUETOOTH_SCO);
    GetServerPtr()->audioPolicyService_.audioIOHandleMap_.GetSinkIOHandle(DEVICE_TYPE_USB_ARM_HEADSET);
    GetServerPtr()->audioPolicyService_.audioIOHandleMap_.GetSinkIOHandle(DEVICE_TYPE_BLUETOOTH_A2DP);
    GetServerPtr()->audioPolicyService_.audioIOHandleMap_.GetSinkIOHandle(DEVICE_TYPE_FILE_SINK);
    GetServerPtr()->audioPolicyService_.audioIOHandleMap_.GetSinkIOHandle(DEVICE_TYPE_DP);
    GetServerPtr()->audioPolicyService_.audioIOHandleMap_.GetSourceIOHandle(DEVICE_TYPE_USB_ARM_HEADSET);
    GetServerPtr()->audioPolicyService_.audioIOHandleMap_.GetSourceIOHandle(DEVICE_TYPE_MIC);
    GetServerPtr()->audioPolicyService_.audioIOHandleMap_.GetSourceIOHandle(DEVICE_TYPE_FILE_SOURCE);
    GetServerPtr()->audioPolicyService_.audioIOHandleMap_.GetSourceIOHandle(DEVICE_TYPE_DP);
    SinkInput sinkInput = {};
    SourceOutput sourceOutput = {};
    GetServerPtr()->audioPolicyService_.WriteOutputDeviceChangedSysEvents(remoteDeviceDescriptor, sinkInput);
    GetServerPtr()->audioPolicyService_.WriteInputDeviceChangedSysEvents(remoteDeviceDescriptor, sourceOutput);
}

void AudioPolicyServiceThirdTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

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
    shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor = make_shared<AudioDeviceDescriptor>();
    GetServerPtr()->audioPolicyService_.audioDeviceCommon_.MoveToNewInputDevice(*audioCapturerChangeInfos.begin(),
        remoteDeviceDescriptor);
    remoteDeviceDescriptor->networkId_ = REMOTE_NETWORK_ID;
    GetServerPtr()->audioPolicyService_.audioDeviceCommon_.MoveToNewInputDevice(*audioCapturerChangeInfos.begin(),
        remoteDeviceDescriptor);
    GetServerPtr()->audioPolicyService_.RegisterRemoteDevStatusCallback();
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_
        = DEVICE_TYPE_BLUETOOTH_A2DP;
    uint32_t channelcount = *reinterpret_cast<const uint32_t*>(rawData);
    GetServerPtr()->audioPolicyService_.ReconfigureAudioChannel(channelcount, DEVICE_TYPE_FILE_SINK);
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_FILE_SINK;
    GetServerPtr()->audioPolicyService_.ReconfigureAudioChannel(channelcount, DEVICE_TYPE_FILE_SINK);
    GetServerPtr()->audioPolicyService_.ReconfigureAudioChannel(channelcount, DEVICE_TYPE_FILE_SOURCE);
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_SCO;
    GetServerPtr()->audioPolicyService_.audioVolumeManager_.IsBlueTooth(deviceType);
    GetServerPtr()->audioPolicyService_.audioVolumeManager_.activeSafeTimeBt_ = ACTIVEBTTIME;
    GetServerPtr()->audioPolicyService_.audioVolumeManager_.CheckBlueToothActiveMusicTime(1);
    GetServerPtr()->audioPolicyService_.audioVolumeManager_.CheckWiredActiveMusicTime(1);
    int32_t safeVolume = *reinterpret_cast<const int32_t*>(rawData);
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

void AudioPolicyServiceTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

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
    uint32_t deviceRole_int = *reinterpret_cast<const uint32_t*>(rawData);
    deviceRole_int = (deviceRole_int % ENUM_NUM) - 1;
    DeviceRole deviceRole = static_cast<DeviceRole>(deviceRole_int);
    std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    GetServerPtr()->audioPolicyService_.audioDeviceCommon_.OpenRemoteAudioDevice(REMOTE_NETWORK_ID,
        deviceRole, DEVICE_TYPE_EARPIECE, remoteDeviceDescriptor);
    AudioStreamInfo audioStreamInfo = {};
    audioStreamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    audioStreamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    audioStreamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioStreamInfo.channels = AudioChannel::STEREO;
    InitGetServerService(rawData, size, deviceRole, audioStreamInfo);
    GetServerPtr()->audioPolicyService_.OnDeviceConfigurationChanged(DEVICE_TYPE_BLUETOOTH_A2DP,
        GetServerPtr()->audioPolicyService_.audioActiveDevice_.activeBTDevice_, "DeviceName", audioStreamInfo);
    shared_ptr<AudioDeviceDescriptor> dis = make_shared<AudioDeviceDescriptor>();
    dis->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    dis->macAddress_ = GetServerPtr()->audioPolicyService_.audioActiveDevice_.activeBTDevice_;
    dis->deviceRole_ = OUTPUT_DEVICE;
    GetServerPtr()->audioPolicyService_.audioDeviceManager_.connectedDevices_.push_back(dis);
    GetServerPtr()->
        audioPolicyService_.OnForcedDeviceSelected(DEVICE_TYPE_BLUETOOTH_A2DP,
        GetServerPtr()->audioPolicyService_.audioActiveDevice_.activeBTDevice_);
    GetServerPtr()->
        audioPolicyService_.OnForcedDeviceSelected(DEVICE_TYPE_BLUETOOTH_SCO,
        GetServerPtr()->audioPolicyService_.audioActiveDevice_.activeBTDevice_);
    AudioPolicyServiceSecondTest(rawData, size, audioStreamInfo, remoteDeviceDescriptor);
    AudioPolicyServiceThirdTest(rawData, size);
}

void AudioPolicyServiceTestII(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    int32_t volumeLevel = *reinterpret_cast<const int32_t*>(rawData);
    GetServerPtr()->audioPolicyService_.audioVolumeManager_.DealWithSafeVolume(volumeLevel, true);
    GetServerPtr()->audioPolicyService_.audioVolumeManager_.DealWithSafeVolume(volumeLevel, false);
    AudioStreamInfo audioStreamInfo = {};
    audioStreamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    audioStreamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    audioStreamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioStreamInfo.channels = AudioChannel::STEREO;
    A2dpDeviceConfigInfo configInfo = {audioStreamInfo, true};
    volumeLevel = GetServerPtr()->audioPolicyService_.audioPolicyManager_.GetSafeVolumeLevel() + MOD_NUM_TWO;
    GetServerPtr()->audioPolicyService_.audioA2dpDevice_.connectedA2dpDeviceMap_.insert({"activeBTDevice_1",
        configInfo});
    GetServerPtr()->audioPolicyService_.SetA2dpDeviceVolume("activeBTDevice_1", volumeLevel, true);
    GetServerPtr()->audioPolicyService_.SetA2dpDeviceVolume("activeBTDevice_1", volumeLevel, false);
    GetServerPtr()->audioPolicyService_.OnMicrophoneBlockedUpdate(DEVICE_TYPE_BLUETOOTH_SCO, DEVICE_UNBLOCKED);
    vector<shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    AudioStreamManager::GetInstance()->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    for (auto &capturerChangeInfo : audioCapturerChangeInfos) {
        capturerChangeInfo->capturerInfo.sourceType = SOURCE_TYPE_VIRTUAL_CAPTURE;
        capturerChangeInfo->capturerState = CAPTURER_PREPARED;
    }
    GetServerPtr()->
        audioPolicyService_.audioDeviceCommon_.FetchInputDevice(audioCapturerChangeInfos,
        AudioStreamDeviceChangeReason::UNKNOWN);
    GetServerPtr()->audioPolicyService_.OnReceiveBluetoothEvent(
        GetServerPtr()->audioPolicyService_.audioActiveDevice_.activeBTDevice_, "deviceName");
    GetServerPtr()->audioPolicyService_.GetAudioEffectOffloadFlag();
    GetServerPtr()->audioPolicyService_.audioOffloadStream_.CheckSpatializationAndEffectState();
    GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->IsA2dpOffloadConnecting(MOD_NUM_TWO);
    GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->a2dpOffloadDeviceAddress_ = "A2dpMacAddress";
    GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->audioA2dpOffloadFlag_.currentOffloadConnectionState_
        = CONNECTION_STATUS_CONNECTED;
    GetServerPtr()->
        audioPolicyService_.audioA2dpOffloadManager_->OnA2dpPlayingStateChanged("A2dpMacAddressS", A2DP_STOPPED);
    GetServerPtr()->
        audioPolicyService_.audioA2dpOffloadManager_->OnA2dpPlayingStateChanged("A2dpMacAddressS", A2DP_PLAYING);
    GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->audioA2dpOffloadFlag_.currentOffloadConnectionState_
        = CONNECTION_STATUS_CONNECTING;
    GetServerPtr()->
        audioPolicyService_.audioA2dpOffloadManager_->OnA2dpPlayingStateChanged("A2dpMacAddress", A2DP_PLAYING);
    GetServerPtr()->
        audioPolicyService_.audioA2dpOffloadManager_->OnA2dpPlayingStateChanged("A2dpMacAddress", A2DP_STOPPED);
    GetServerPtr()->audioPolicyService_.LoadSplitModule("", NETWORKID);
    GetServerPtr()->audioPolicyService_.LoadSplitModule(SPLITARGS, NETWORKID);
    GetServerPtr()->audioPolicyService_.SetRotationToEffect(ENUM_NUM);
}

void AudioPolicyServiceTestIII(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    AudioRendererChangeInfo audioRendererChangeInfo;
    int32_t clientUID = *reinterpret_cast<const int32_t*>(rawData);
    int32_t sessionId = *reinterpret_cast<const int32_t*>(rawData);
    int32_t clientPid_1 = *reinterpret_cast<const int32_t*>(rawData);
    audioRendererChangeInfo.clientUID = clientUID;
    audioRendererChangeInfo.sessionId = sessionId;
    audioRendererChangeInfo.clientPid = clientPid_1;
    audioRendererChangeInfo.rendererState = RENDERER_NEW;
    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = STREAM_USAGE_VOICE_MODEM_COMMUNICATION;
    audioRendererChangeInfo.rendererInfo = rendererInfo;
    GetServerPtr()->audioPolicyService_.audioScene_ = AUDIO_SCENE_PHONE_CALL;
    GetServerPtr()->audioPolicyService_.streamCollector_.audioRendererChangeInfos_.
        push_back(make_shared<AudioRendererChangeInfo>(audioRendererChangeInfo));
    GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->WaitForConnectionCompleted();
    std::string dumpString = "";
    GetServerPtr()->audioPolicyService_.AudioStreamDump(dumpString);
    GetServerPtr()->audioPolicyService_.audioDeviceCommon_.ScoInputDeviceFetchedForRecongnition(true, NETWORKID,
        SUSPEND_CONNECTED);
    GetServerPtr()->audioPolicyService_.audioDeviceCommon_.ScoInputDeviceFetchedForRecongnition(false, NETWORKID,
        SUSPEND_CONNECTED);
    GetServerPtr()->audioPolicyService_.audioVolumeManager_.ringerModeMute_ = true;
    GetServerPtr()->audioPolicyService_.ResetRingerModeMute();
    InternalDeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    GetServerPtr()->audioPolicyService_.audioDeviceCommon_.IsRingerOrAlarmerDualDevicesRange(deviceType);
    AudioStreamInfo audioStreamInfo = {};
    audioStreamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    audioStreamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    audioStreamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioStreamInfo.channels = AudioChannel::STEREO;
    GetServerPtr()->audioPolicyService_.ReloadA2dpOffloadOnDeviceChanged(DEVICE_TYPE_BLUETOOTH_A2DP,
        GetServerPtr()->audioPolicyService_.audioActiveDevice_.activeBTDevice_, "DeviceName", audioStreamInfo);
    std::shared_ptr<AudioDeviceDescriptor> dis = std::make_shared<AudioDeviceDescriptor>();
    dis->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    dis->macAddress_ = GetServerPtr()->audioPolicyService_.audioActiveDevice_.activeBTDevice_;
    dis->deviceRole_ = OUTPUT_DEVICE;
    GetServerPtr()->audioPolicyService_.audioRecoveryDevice_.ConnectVirtualDevice(dis);
    GetServerPtr()->audioPolicyService_.audioRecoveryDevice_.
        HandleRecoveryPreferredDevices(A2DP_PLAYING, LIMITSIZE, LIMITSIZE);
    GetServerPtr()->audioPolicyService_.RestoreSession(SESSIONID_32, true);
}

void AudioPolicyServiceTestIV(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {return;}
    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    if (audioRendererFilter == nullptr) {return;}
    audioRendererFilter->uid = getuid();
    audioRendererFilter->rendererInfo.rendererFlags = STREAM_FLAG_FAST;
    audioRendererFilter->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    AudioRoutingManager::GetInstance()->
        GetPreferredOutputDeviceForRendererInfo(audioRendererFilter->rendererInfo, desc);
    GetServerPtr()->audioPolicyService_.audioRecoveryDevice_.SelectOutputDeviceByFilterInner(audioRendererFilter, desc);
    vector<SinkInput> sinkInputs;
    std::shared_ptr<AudioDeviceDescriptor> dis = std::make_shared<AudioDeviceDescriptor>();
    dis->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    dis->macAddress_ = GetServerPtr()->audioPolicyService_.audioActiveDevice_.activeBTDevice_;
    dis->deviceRole_ = OUTPUT_DEVICE;
    dis->networkId_ = "RemoteDevice";
    std::string moduleName = dis->networkId_ + (dis->deviceRole_ == DeviceRole::OUTPUT_DEVICE ? "_out" : "_in");
    AudioModuleInfo audioModuleInfo;
    AudioIOHandle ioHandle = GetServerPtr()->audioPolicyService_.audioPolicyManager_.OpenAudioPort(audioModuleInfo);
    GetServerPtr()->audioPolicyService_.audioIOHandleMap_.AddIOHandleInfo(moduleName, ioHandle);
    GetServerPtr()->audioPolicyService_.audioDeviceCommon_.MoveToRemoteOutputDevice(sinkInputs, dis);
    sptr<AudioCapturerFilter> audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();
    audioCapturerFilter->uid = SYSTEM_ABILITY_ID;
    GetServerPtr()->audioPolicyService_.audioRecoveryDevice_.SelectFastInputDevice(audioCapturerFilter, dis);
    GetServerPtr()->audioPolicyService_.audioRecoveryDevice_.SetCaptureDeviceForUsage(AUDIO_SCENE_PHONE_CALL,
        SOURCE_TYPE_VOICE_CALL, dis);
    GetServerPtr()->audioPolicyService_.CloseWakeUpAudioCapturer();
    AudioDeviceDescriptor newDeviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    newDeviceInfo.networkId_ = LOCAL_NETWORK_ID;
    newDeviceInfo.macAddress_ = GetServerPtr()->audioPolicyService_.audioActiveDevice_.activeBTDevice_;
    AudioPolicyUtils::GetInstance().GetSinkName(newDeviceInfo, SESSIONID_32);
    AudioDeviceDescriptor ads;
    ads.networkId_ = LOCAL_NETWORK_ID;
    ads.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    AudioPolicyUtils::GetInstance().GetSinkName(ads, SESSIONID_32);
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->sessionId = SESSIONID_32;
    rendererChangeInfo->outputDeviceInfo = newDeviceInfo;
    vector<std::shared_ptr<AudioDeviceDescriptor>> outputDevices =
        GetServerPtr()->audioPolicyService_.audioRouterCenter_.FetchOutputDevices(STREAM_USAGE_MEDIA, -1);
    GetServerPtr()->audioPolicyService_.audioDeviceCommon_.
        MoveToNewOutputDevice(rendererChangeInfo, outputDevices, sinkInputs,
        AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN);
    std::shared_ptr<AudioDeviceDescriptor> adc = std::make_shared<AudioDeviceDescriptor>();
    vector<shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    AudioStreamManager::GetInstance()->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    GetServerPtr()->audioPolicyService_.audioDeviceCommon_.
        ActivateA2dpDevice(adc, audioRendererChangeInfos,  AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN);
}

} // namespace AudioStandard
} // namesapce OHOS

extern "C" int LLVMFuzzerInitialize(const uint8_t *data, size_t size)
{
    OHOS::AudioStandard::AudioFuzzTestGetPermission();
    return 0;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::AudioPolicyServiceTest(data, size);
    OHOS::AudioStandard::AudioPolicyServiceTestII(data, size);
    OHOS::AudioStandard::AudioPolicyServiceTestIII(data, size);
    OHOS::AudioStandard::AudioPolicyServiceTestIV(data, size);
    return 0;
}
