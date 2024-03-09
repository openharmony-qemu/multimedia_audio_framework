/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "audio_policy_manager_stub.h"

#include "audio_errors.h"
#include "audio_log.h"
#include "audio_policy_ipc_interface_code.h"

namespace {
constexpr int MAX_PID_COUNT = 1000;
}

namespace OHOS {
namespace AudioStandard {
using namespace std;

void AudioPolicyManagerStub::ReadStreamChangeInfo(MessageParcel &data, const AudioMode &mode,
    AudioStreamChangeInfo &streamChangeInfo)
{
    if (mode == AUDIO_MODE_PLAYBACK) {
        streamChangeInfo.audioRendererChangeInfo.Unmarshalling(data);
        return;
    } else {
        // mode == AUDIO_MODE_RECORDING
        streamChangeInfo.audioCapturerChangeInfo.Unmarshalling(data);
    }
}

void AudioPolicyManagerStub::GetMaxVolumeLevelInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioVolumeType volumeType = static_cast<AudioVolumeType>(data.ReadInt32());
    int32_t maxLevel = GetMaxVolumeLevel(volumeType);
    reply.WriteInt32(maxLevel);
}

void AudioPolicyManagerStub::GetMinVolumeLevelInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioVolumeType volumeType = static_cast<AudioVolumeType>(data.ReadInt32());
    int32_t minLevel = GetMinVolumeLevel(volumeType);
    reply.WriteInt32(minLevel);
}

void AudioPolicyManagerStub::SetSystemVolumeLevelInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioVolumeType volumeType = static_cast<AudioVolumeType>(data.ReadInt32());
    int32_t volumeLevel = data.ReadInt32();
    API_VERSION api_v = static_cast<API_VERSION>(data.ReadInt32());
    int result = SetSystemVolumeLevel(volumeType, volumeLevel, api_v);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::SetRingerModeInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioRingerMode rMode = static_cast<AudioRingerMode>(data.ReadInt32());
    API_VERSION api_v = static_cast<API_VERSION>(data.ReadInt32());
    int32_t result = SetRingerMode(rMode, api_v);
    reply.WriteInt32(result);
}

#ifdef FEATURE_DTMF_TONE
void AudioPolicyManagerStub::GetToneInfoInternal(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<ToneInfo> ltoneInfo = GetToneConfig(data.ReadInt32());
    CHECK_AND_RETURN_LOG(ltoneInfo != nullptr, "obj is null");
    ltoneInfo->Marshalling(reply);
}

void AudioPolicyManagerStub::GetSupportedTonesInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t lToneListSize = 0;
    std::vector<int32_t> lToneList = GetSupportedTones();
    lToneListSize = static_cast<int32_t>(lToneList.size());
    reply.WriteInt32(lToneListSize);
    for (int i = 0; i < lToneListSize; i++) {
        reply.WriteInt32(lToneList[i]);
    }
}
#endif

void AudioPolicyManagerStub::GetRingerModeInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioRingerMode rMode = GetRingerMode();
    reply.WriteInt32(static_cast<int>(rMode));
}

void AudioPolicyManagerStub::SetAudioSceneInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioScene audioScene = static_cast<AudioScene>(data.ReadInt32());
    int32_t result = SetAudioScene(audioScene);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::SetMicrophoneMuteInternal(MessageParcel &data, MessageParcel &reply)
{
    bool isMute = data.ReadBool();
    int32_t result = SetMicrophoneMute(isMute);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::SetMicrophoneMuteAudioConfigInternal(MessageParcel &data, MessageParcel &reply)
{
    bool isMute = data.ReadBool();
    int32_t result = SetMicrophoneMuteAudioConfig(isMute);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::IsMicrophoneMuteInternal(MessageParcel &data, MessageParcel &reply)
{
    API_VERSION api_v = static_cast<API_VERSION>(data.ReadInt32());
    int32_t result = IsMicrophoneMute(api_v);
    reply.WriteBool(result);
}

void AudioPolicyManagerStub::GetAudioSceneInternal(MessageParcel & /* data */, MessageParcel &reply)
{
    AudioScene audioScene = GetAudioScene();
    reply.WriteInt32(static_cast<int>(audioScene));
}

void AudioPolicyManagerStub::GetSystemVolumeLevelInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioStreamType streamType = static_cast<AudioStreamType>(data.ReadInt32());
    int32_t volumeLevel = GetSystemVolumeLevel(streamType);
    reply.WriteInt32(volumeLevel);
}

void AudioPolicyManagerStub::SetLowPowerVolumeInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t streamId = data.ReadInt32();
    float volume = data.ReadFloat();
    int result = SetLowPowerVolume(streamId, volume);
    if (result == SUCCESS)
        reply.WriteInt32(AUDIO_OK);
    else
        reply.WriteInt32(AUDIO_ERR);
}

void AudioPolicyManagerStub::GetLowPowerVolumeInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t streamId = data.ReadInt32();
    float volume = GetLowPowerVolume(streamId);
    reply.WriteFloat(volume);
}

void AudioPolicyManagerStub::GetSingleStreamVolumeInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t streamId = data.ReadInt32();
    float volume = GetSingleStreamVolume(streamId);
    reply.WriteFloat(volume);
}

void AudioPolicyManagerStub::SetStreamMuteInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioVolumeType volumeType = static_cast<AudioVolumeType>(data.ReadInt32());
    bool mute = data.ReadBool();
    API_VERSION api_v = static_cast<API_VERSION>(data.ReadInt32());
    int result = SetStreamMute(volumeType, mute, api_v);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::GetStreamMuteInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioVolumeType volumeType = static_cast<AudioVolumeType>(data.ReadInt32());
    bool mute = GetStreamMute(volumeType);
    reply.WriteBool(mute);
}

void AudioPolicyManagerStub::IsStreamActiveInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioVolumeType volumeType = static_cast<AudioVolumeType>(data.ReadInt32());
    bool isActive = IsStreamActive(volumeType);
    reply.WriteBool(isActive);
}

void AudioPolicyManagerStub::AdjustVolumeByStepInternal(MessageParcel &data, MessageParcel &reply)
{
    VolumeAdjustType adjustType = static_cast<VolumeAdjustType>(data.ReadInt32());
    int32_t result = AdjustVolumeByStep(adjustType);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::GetSystemVolumeInDbInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioVolumeType volumeType = static_cast<AudioVolumeType>(data.ReadInt32());
    int32_t volumeLevel = data.ReadInt32();
    DeviceType deviceType = static_cast<DeviceType>(data.ReadInt32());
    float result = GetSystemVolumeInDb(volumeType, volumeLevel, deviceType);
    reply.WriteFloat(result);
}

void AudioPolicyManagerStub::IsVolumeUnadjustableInternal(MessageParcel &data, MessageParcel &reply)
{
    bool isVolumeUnadjustable = IsVolumeUnadjustable();
    reply.WriteBool(isVolumeUnadjustable);
}

void AudioPolicyManagerStub::AdjustSystemVolumeByStepInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioVolumeType volumeType = static_cast<AudioVolumeType>(data.ReadInt32());
    VolumeAdjustType adjustType = static_cast<VolumeAdjustType>(data.ReadInt32());
    int32_t result = AdjustSystemVolumeByStep(volumeType, adjustType);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::GetDevicesInternal(MessageParcel &data, MessageParcel &reply)
{
    int deviceFlag = data.ReadInt32();
    DeviceFlag deviceFlagConfig = static_cast<DeviceFlag>(deviceFlag);
    std::vector<sptr<AudioDeviceDescriptor>> devices = GetDevices(deviceFlagConfig);
    int32_t size = static_cast<int32_t>(devices.size());
    reply.WriteInt32(size);
    for (int i = 0; i < size; i++) {
        devices[i]->Marshalling(reply);
    }
}

void AudioPolicyManagerStub::NotifyCapturerAddedInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioCapturerInfo capturerInfo;
    AudioStreamInfo streamInfo;
    uint32_t sessionId;

    capturerInfo.Unmarshalling(data);
    streamInfo.Unmarshalling(data);
    data.ReadUint32(sessionId);

    int32_t result = NotifyCapturerAdded(capturerInfo, streamInfo, sessionId);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::GetPreferredOutputDeviceDescriptorsInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioRendererInfo rendererInfo;
    rendererInfo.Unmarshalling(data);
    std::vector<sptr<AudioDeviceDescriptor>> devices = GetPreferredOutputDeviceDescriptors(rendererInfo);
    int32_t size = static_cast<int32_t>(devices.size());
    reply.WriteInt32(size);
    for (int i = 0; i < size; i++) {
        devices[i]->Marshalling(reply);
    }
}

void AudioPolicyManagerStub::GetPreferredInputDeviceDescriptorsInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioCapturerInfo captureInfo;
    captureInfo.Unmarshalling(data);
    std::vector<sptr<AudioDeviceDescriptor>> devices = GetPreferredInputDeviceDescriptors(captureInfo);
    size_t size = static_cast<int32_t>(devices.size());
    reply.WriteInt32(size);
    for (size_t i = 0; i < size; i++) {
        devices[i]->Marshalling(reply);
    }
}

void AudioPolicyManagerStub::SetDeviceActiveInternal(MessageParcel &data, MessageParcel &reply)
{
    InternalDeviceType deviceType = static_cast<InternalDeviceType>(data.ReadInt32());
    bool active = data.ReadBool();
    int32_t result = SetDeviceActive(deviceType, active);
    if (result == SUCCESS)
        reply.WriteInt32(AUDIO_OK);
    else
        reply.WriteInt32(AUDIO_ERR);
}

void AudioPolicyManagerStub::IsDeviceActiveInternal(MessageParcel &data, MessageParcel &reply)
{
    InternalDeviceType deviceType = static_cast<InternalDeviceType>(data.ReadInt32());
    bool result = IsDeviceActive(deviceType);
    reply.WriteBool(result);
}

void AudioPolicyManagerStub::GetActiveOutputDeviceInternal(MessageParcel &data, MessageParcel &reply)
{
    InternalDeviceType deviceType = GetActiveOutputDevice();
    reply.WriteInt32(static_cast<int>(deviceType));
}

void AudioPolicyManagerStub::GetActiveInputDeviceInternal(MessageParcel &data, MessageParcel &reply)
{
    InternalDeviceType deviceType = GetActiveInputDevice();
    reply.WriteInt32(static_cast<int>(deviceType));
}

void AudioPolicyManagerStub::WriteAudioFocusInfo(MessageParcel &reply,
    const std::pair<AudioInterrupt, AudioFocuState> &focusInfo)
{
    focusInfo.first.Marshalling(reply);
    reply.WriteInt32(focusInfo.second);
}

void AudioPolicyManagerStub::GetAudioFocusInfoListInternal(MessageParcel &data, MessageParcel &reply)
{
    std::list<std::pair<AudioInterrupt, AudioFocuState>> focusInfoList;
    int32_t zoneID = data.ReadInt32();
    int32_t result = GetAudioFocusInfoList(focusInfoList, zoneID);
    int32_t size = static_cast<int32_t>(focusInfoList.size());
    reply.WriteInt32(result);
    reply.WriteInt32(size);
    if (result == SUCCESS) {
        for (std::pair<AudioInterrupt, AudioFocuState> focusInfo : focusInfoList) {
            WriteAudioFocusInfo(reply, focusInfo);
        }
    }
}

void AudioPolicyManagerStub::SelectOutputDeviceInternal(MessageParcel &data, MessageParcel &reply)
{
    sptr<AudioRendererFilter> audioRendererFilter = AudioRendererFilter::Unmarshalling(data);
    CHECK_AND_RETURN_LOG(audioRendererFilter != nullptr, "AudioRendererFilter unmarshall fail.");

    int validSize = 20; // Use 20 as limit.
    int size = data.ReadInt32();
    if (size <= 0 || size > validSize) {
        AUDIO_ERR_LOG("SelectOutputDevice get invalid device size.");
        return;
    }
    std::vector<sptr<AudioDeviceDescriptor>> targetOutputDevice;
    for (int i = 0; i < size; i++) {
        sptr<AudioDeviceDescriptor> audioDeviceDescriptor = AudioDeviceDescriptor::Unmarshalling(data);
        CHECK_AND_RETURN_LOG(audioDeviceDescriptor != nullptr, "Unmarshalling fail.");
        targetOutputDevice.push_back(audioDeviceDescriptor);
    }

    int32_t ret = SelectOutputDevice(audioRendererFilter, targetOutputDevice);
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::GetSelectedDeviceInfoInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t uid = data.ReadInt32();
    int32_t pid = data.ReadInt32();
    AudioStreamType streamType =  static_cast<AudioStreamType>(data.ReadInt32());

    std::string deviceName = GetSelectedDeviceInfo(uid, pid, streamType);
    reply.WriteString(deviceName);
}

void AudioPolicyManagerStub::SelectInputDeviceInternal(MessageParcel &data, MessageParcel &reply)
{
    sptr<AudioCapturerFilter> audioCapturerFilter = AudioCapturerFilter::Unmarshalling(data);
    CHECK_AND_RETURN_LOG(audioCapturerFilter != nullptr, "AudioCapturerFilter unmarshall fail.");

    int validSize = 10; // Use 10 as limit.
    int size = data.ReadInt32();
    CHECK_AND_RETURN_LOG(size > 0 && size <= validSize, "SelectInputDevice get invalid device size.");
    std::vector<sptr<AudioDeviceDescriptor>> targetInputDevice;
    for (int i = 0; i < size; i++) {
        sptr<AudioDeviceDescriptor> audioDeviceDescriptor = AudioDeviceDescriptor::Unmarshalling(data);
        CHECK_AND_RETURN_LOG(audioDeviceDescriptor != nullptr, "Unmarshalling fail.");
        targetInputDevice.push_back(audioDeviceDescriptor);
    }

    int32_t ret = SelectInputDevice(audioCapturerFilter, targetInputDevice);
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::SetInterruptCallbackInternal(MessageParcel &data, MessageParcel &reply)
{
    uint32_t sessionID = data.ReadUint32();
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    uint32_t zoneID = data.ReadUint32();
    CHECK_AND_RETURN_LOG(object != nullptr, "AudioPolicyManagerStub: AudioInterruptCallback obj is null");
    int32_t result = SetAudioInterruptCallback(sessionID, object, zoneID);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::UnsetInterruptCallbackInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t sessionID = data.ReadInt32();
    int32_t zoneID = data.ReadInt32();
    int32_t result = UnsetAudioInterruptCallback(sessionID, zoneID);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::ActivateInterruptInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t zoneID = data.ReadInt32();
    AudioInterrupt audioInterrupt = {};
    audioInterrupt.Unmarshalling(data);
    int32_t result = ActivateAudioInterrupt(audioInterrupt, zoneID);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::DeactivateInterruptInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t zoneID = data.ReadInt32();
    AudioInterrupt audioInterrupt = {};
    audioInterrupt.Unmarshalling(data);
    int32_t result = DeactivateAudioInterrupt(audioInterrupt, zoneID);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::SetAudioManagerInterruptCbInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t clientId = data.ReadInt32();
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    CHECK_AND_RETURN_LOG(object != nullptr, "AudioPolicyManagerStub: AudioInterruptCallback obj is null");
    int32_t result = SetAudioManagerInterruptCallback(clientId, object);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::UnsetAudioManagerInterruptCbInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t clientId = data.ReadInt32();
    int32_t result = UnsetAudioManagerInterruptCallback(clientId);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::RequestAudioFocusInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioInterrupt audioInterrupt = {};
    int32_t clientId = data.ReadInt32();
    audioInterrupt.Unmarshalling(data);
    int32_t result = RequestAudioFocus(clientId, audioInterrupt);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::AbandonAudioFocusInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioInterrupt audioInterrupt = {};
    int32_t clientId = data.ReadInt32();
    audioInterrupt.Unmarshalling(data);
    int32_t result = AbandonAudioFocus(clientId, audioInterrupt);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::GetStreamInFocusInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t zoneID = data.ReadInt32();
    AudioStreamType streamInFocus = GetStreamInFocus(zoneID);
    reply.WriteInt32(static_cast<int32_t>(streamInFocus));
}

void AudioPolicyManagerStub::GetSessionInfoInFocusInternal(MessageParcel &data, MessageParcel &reply)
{
    uint32_t invalidSessionID = static_cast<uint32_t>(-1);
    AudioInterrupt audioInterrupt {STREAM_USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN,
        {AudioStreamType::STREAM_DEFAULT, SourceType::SOURCE_TYPE_INVALID, true}, invalidSessionID};
    int32_t zoneID = data.ReadInt32();
    int32_t ret = GetSessionInfoInFocus(audioInterrupt, zoneID);
    audioInterrupt.Marshalling(reply);
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::CheckRecordingCreateInternal(MessageParcel &data, MessageParcel &reply)
{
    uint32_t appTokenId = data.ReadUint32();
    uint64_t appFullTokenId = data.ReadUint64();
    uint32_t appUid = data.ReadInt32();
    SourceType sourceType = static_cast<SourceType> (data.ReadInt32());
    bool ret = CheckRecordingCreate(appTokenId, appFullTokenId, appUid, sourceType);
    reply.WriteBool(ret);
}

void AudioPolicyManagerStub::CheckRecordingStateChangeInternal(MessageParcel &data, MessageParcel &reply)
{
    uint32_t appTokenId = data.ReadUint32();
    uint64_t appFullTokenId = data.ReadUint64();
    int32_t appUid = data.ReadInt32();
    AudioPermissionState state = static_cast<AudioPermissionState>(data.ReadInt32());
    bool ret = CheckRecordingStateChange(appTokenId, appFullTokenId, appUid, state);
    reply.WriteBool(ret);
}

void AudioPolicyManagerStub::GetAudioLatencyFromXmlInternal(MessageParcel &data, MessageParcel &reply)
{
    int ret = GetAudioLatencyFromXml();
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::GetSinkLatencyFromXmlInternal(MessageParcel &data, MessageParcel &reply)
{
    uint32_t ret = GetSinkLatencyFromXml();
    reply.WriteUint32(ret);
}

void AudioPolicyManagerStub::ReconfigureAudioChannelInternal(MessageParcel &data, MessageParcel &reply)
{
    uint32_t count = data.ReadUint32();
    DeviceType deviceType = static_cast<DeviceType>(data.ReadInt32());
    int32_t ret = ReconfigureAudioChannel(count, deviceType);
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::RegisterTrackerInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioStreamChangeInfo streamChangeInfo = {};
    AudioMode mode = static_cast<AudioMode> (data.ReadInt32());
    ReadStreamChangeInfo(data, mode, streamChangeInfo);
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    CHECK_AND_RETURN_LOG(remoteObject != nullptr, "Client Tracker obj is null");

    int ret = RegisterTracker(mode, streamChangeInfo, remoteObject);
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::UpdateTrackerInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioStreamChangeInfo streamChangeInfo = {};
    AudioMode mode = static_cast<AudioMode> (data.ReadInt32());
    ReadStreamChangeInfo(data, mode, streamChangeInfo);
    int ret = UpdateTracker(mode, streamChangeInfo);
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::GetRendererChangeInfosInternal(MessageParcel &data, MessageParcel &reply)
{
    size_t size = 0;
    vector<unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    int ret = GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("AudioPolicyManagerStub:GetRendererChangeInfos Error!!");
        reply.WriteInt32(size);
        return;
    }

    size = audioRendererChangeInfos.size();
    reply.WriteInt32(size);
    for (const unique_ptr<AudioRendererChangeInfo> &rendererChangeInfo: audioRendererChangeInfos) {
        if (!rendererChangeInfo) {
            AUDIO_ERR_LOG("AudioPolicyManagerStub:Renderer change info null, something wrong!!");
            continue;
        }
        rendererChangeInfo->Marshalling(reply);
    }
}

void AudioPolicyManagerStub::GetCapturerChangeInfosInternal(MessageParcel &data, MessageParcel &reply)
{
    size_t size = 0;
    vector<unique_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    int32_t ret = GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("AudioPolicyManagerStub:GetCapturerChangeInfos Error!!");
        reply.WriteInt32(size);
        return;
    }

    size = audioCapturerChangeInfos.size();
    reply.WriteInt32(size);
    for (const unique_ptr<AudioCapturerChangeInfo> &capturerChangeInfo: audioCapturerChangeInfos) {
        if (!capturerChangeInfo) {
            AUDIO_ERR_LOG("AudioPolicyManagerStub:Capturer change info null, something wrong!!");
            continue;
        }
        capturerChangeInfo->Marshalling(reply);
    }
}

void AudioPolicyManagerStub::UpdateStreamStateInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t clientUid = data.ReadInt32();
    StreamSetState streamSetState = static_cast<StreamSetState>(data.ReadInt32());
    AudioStreamType streamType = static_cast<AudioStreamType>(data.ReadInt32());

    int32_t result = UpdateStreamState(clientUid, streamSetState, streamType);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::GetVolumeGroupInfoInternal(MessageParcel& data, MessageParcel& reply)
{
    std::string networkId = data.ReadString();
    std::vector<sptr<VolumeGroupInfo>> groupInfos;
    int32_t ret = GetVolumeGroupInfos(networkId, groupInfos);
    int32_t size = static_cast<int32_t>(groupInfos.size());
    if (ret == SUCCESS && size > 0) {
        reply.WriteInt32(size);
        for (int i = 0; i < size; i++) {
            groupInfos[i]->Marshalling(reply);
        }
    } else {
        reply.WriteInt32(ret);
    }
}

void AudioPolicyManagerStub::GetNetworkIdByGroupIdInternal(MessageParcel& data, MessageParcel& reply)
{
    int32_t groupId = data.ReadInt32();
    std::string networkId;
    int32_t ret = GetNetworkIdByGroupId(groupId, networkId);

    reply.WriteString(networkId);
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::IsAudioRendererLowLatencySupportedInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioStreamInfo audioStreamInfo = {};
    audioStreamInfo.samplingRate = static_cast<AudioSamplingRate>(data.ReadInt32());
    audioStreamInfo.channels = static_cast<AudioChannel>(data.ReadInt32());
    audioStreamInfo.format = static_cast<OHOS::AudioStandard::AudioSampleFormat>(data.ReadInt32());
    audioStreamInfo.encoding = static_cast<AudioEncodingType>(data.ReadInt32());
    bool isSupported = IsAudioRendererLowLatencySupported(audioStreamInfo);
    reply.WriteBool(isSupported);
}

void AudioPolicyManagerStub::SetSystemSoundUriInternal(MessageParcel &data, MessageParcel &reply)
{
    std::string key = data.ReadString();
    std::string value = data.ReadString();
    int32_t result =  SetSystemSoundUri(key, value);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::GetSystemSoundUriInternal(MessageParcel &data, MessageParcel &reply)
{
    std::string key = data.ReadString();
    std::string result = GetSystemSoundUri(key);
    reply.WriteString(result);
}

void AudioPolicyManagerStub::GetMinStreamVolumeInternal(MessageParcel &data, MessageParcel &reply)
{
    float volume = GetMinStreamVolume();
    reply.WriteFloat(volume);
}

void AudioPolicyManagerStub::GetMaxStreamVolumeInternal(MessageParcel &data, MessageParcel &reply)
{
    float volume = GetMaxStreamVolume();
    reply.WriteFloat(volume);
}

void AudioPolicyManagerStub::GetMaxRendererInstancesInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t result =  GetMaxRendererInstances();
    reply.WriteInt32(result);
}

static void PreprocessMode(SupportedEffectConfig &supportedEffectConfig, MessageParcel &reply, int32_t i, int32_t j)
{
    reply.WriteString(supportedEffectConfig.preProcessNew.stream[i].streamEffectMode[j].mode);
    int32_t countDev = supportedEffectConfig.preProcessNew.stream[i].streamEffectMode[j].devicePort.size();
    reply.WriteInt32(countDev);
    if (countDev > 0) {
        for (int32_t k = 0; k < countDev; k++) {
            reply.WriteString(supportedEffectConfig.preProcessNew.stream[i].streamEffectMode[j].devicePort[k].type);
            reply.WriteString(supportedEffectConfig.preProcessNew.stream[i].streamEffectMode[j].devicePort[k].chain);
        }
    }
}
static void PreprocessProcess(SupportedEffectConfig &supportedEffectConfig, MessageParcel &reply, int32_t i)
{
    reply.WriteString(supportedEffectConfig.preProcessNew.stream[i].scene);
    int32_t countMode = supportedEffectConfig.preProcessNew.stream[i].streamEffectMode.size();
    reply.WriteInt32(countMode);
    if (countMode > 0) {
        for (int32_t j = 0; j < countMode; j++) {
            PreprocessMode(supportedEffectConfig, reply, i, j);
        }
    }
}
static void PostprocessMode(SupportedEffectConfig &supportedEffectConfig, MessageParcel &reply, int32_t i, int32_t j)
{
    reply.WriteString(supportedEffectConfig.postProcessNew.stream[i].streamEffectMode[j].mode);
    int32_t countDev = supportedEffectConfig.postProcessNew.stream[i].streamEffectMode[j].devicePort.size();
    reply.WriteInt32(countDev);
    if (countDev > 0) {
        for (int32_t k = 0; k < countDev; k++) {
            reply.WriteString(supportedEffectConfig.postProcessNew.stream[i].streamEffectMode[j].devicePort[k].type);
            reply.WriteString(supportedEffectConfig.postProcessNew.stream[i].streamEffectMode[j].devicePort[k].chain);
        }
    }
}
static void PostprocessProcess(SupportedEffectConfig &supportedEffectConfig, MessageParcel &reply, int32_t i)
{
    // i th stream
    reply.WriteString(supportedEffectConfig.postProcessNew.stream[i].scene);
    int countMode = supportedEffectConfig.postProcessNew.stream[i].streamEffectMode.size();
    reply.WriteInt32(countMode);
    if (countMode > 0) {
        for (int32_t j = 0; j < countMode; j++) {
            PostprocessMode(supportedEffectConfig, reply, i, j);
        }
    }
}

void AudioPolicyManagerStub::QueryEffectSceneModeInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t i;
    SupportedEffectConfig supportedEffectConfig;
    int32_t ret = QueryEffectSceneMode(supportedEffectConfig); // audio_policy_server.cpp
    CHECK_AND_RETURN_LOG(ret != -1, "default mode is unavailable !");

    int32_t countPre = supportedEffectConfig.preProcessNew.stream.size();
    int32_t countPost = supportedEffectConfig.postProcessNew.stream.size();
    reply.WriteInt32(countPre);
    reply.WriteInt32(countPost);

    if (countPre > 0) {
        for (i = 0; i < countPre; i++) {
            PreprocessProcess(supportedEffectConfig, reply, i);
        }
    }
    if (countPost > 0) {
        for (i = 0; i < countPost; i++) {
            PostprocessProcess(supportedEffectConfig, reply, i);
        }
    }
}

void AudioPolicyManagerStub::SetPlaybackCapturerFilterInfosInternal(MessageParcel &data, MessageParcel &reply)
{
    uint32_t maxUsageNum = 30;
    AudioPlaybackCaptureConfig config;
    int32_t flag = data.ReadInt32();
    if (flag == 1) {
        config.silentCapture = true;
    }
    uint32_t ss = data.ReadUint32();
    if (ss >= maxUsageNum) {
        reply.WriteInt32(ERROR);
        return;
    }
    for (uint32_t i = 0; i < ss; i++) {
        int32_t tmp_usage = data.ReadInt32();
        if (std::find(AUDIO_SUPPORTED_STREAM_USAGES.begin(), AUDIO_SUPPORTED_STREAM_USAGES.end(), tmp_usage) ==
            AUDIO_SUPPORTED_STREAM_USAGES.end()) {
            continue;
        }
        config.filterOptions.usages.push_back(static_cast<StreamUsage>(tmp_usage));
    }
    uint32_t appTokenId = data.ReadUint32();

    int32_t ret = SetPlaybackCapturerFilterInfos(config, appTokenId);
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::SetCaptureSilentStateInternal(MessageParcel &data, MessageParcel &reply)
{
    bool flag = data.ReadBool();

    int32_t ret = SetCaptureSilentState(flag);
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::GetHardwareOutputSamplingRateInternal(MessageParcel &data, MessageParcel &reply)
{
    sptr<AudioDeviceDescriptor> audioDeviceDescriptor = AudioDeviceDescriptor::Unmarshalling(data);
    CHECK_AND_RETURN_LOG(audioDeviceDescriptor != nullptr, "Unmarshalling fail.");

    int32_t result =  GetHardwareOutputSamplingRate(audioDeviceDescriptor);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::GetAudioCapturerMicrophoneDescriptorsInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t sessionId = data.ReadInt32();
    std::vector<sptr<MicrophoneDescriptor>> descs = GetAudioCapturerMicrophoneDescriptors(sessionId);
    int32_t size = static_cast<int32_t>(descs.size());
    reply.WriteInt32(size);
    for (int i = 0; i < size; i++) {
        descs[i]->Marshalling(reply);
    }
}

void AudioPolicyManagerStub::GetAvailableMicrophonesInternal(MessageParcel &data, MessageParcel &reply)
{
    std::vector<sptr<MicrophoneDescriptor>> descs = GetAvailableMicrophones();
    int32_t size = static_cast<int32_t>(descs.size());
    reply.WriteInt32(size);
    for (int i = 0; i < size; i++) {
        descs[i]->Marshalling(reply);
    }
}

void AudioPolicyManagerStub::SetDeviceAbsVolumeSupportedInternal(MessageParcel &data, MessageParcel &reply)
{
    std::string macAddress = data.ReadString();
    bool support = data.ReadBool();
    int32_t result = SetDeviceAbsVolumeSupported(macAddress, support);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::IsAbsVolumeSceneInternal(MessageParcel &data, MessageParcel &reply)
{
    bool result = IsAbsVolumeScene();
    reply.WriteBool(result);
}

void AudioPolicyManagerStub::SetA2dpDeviceVolumeInternal(MessageParcel &data, MessageParcel &reply)
{
    std::string macAddress = data.ReadString();
    int32_t volume = data.ReadInt32();
    bool updateUi = data.ReadBool();
    int32_t result = SetA2dpDeviceVolume(macAddress, volume, updateUi);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::GetAvailableDevicesInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioDeviceUsage usage  = static_cast<AudioDeviceUsage>(data.ReadInt32());
    std::vector<std::unique_ptr<AudioDeviceDescriptor>> descs = GetAvailableDevices(usage);
    int32_t size = static_cast<int32_t>(descs.size());
    reply.WriteInt32(size);
    for (int32_t i = 0; i < size; i++) {
        descs[i]->Marshalling(reply);
    }
}

void AudioPolicyManagerStub::SetAvailableDeviceChangeCallbackInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t clientId = data.ReadInt32();
    AudioDeviceUsage usage = static_cast<AudioDeviceUsage>(data.ReadInt32());
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    CHECK_AND_RETURN_LOG(object != nullptr, "AudioInterruptCallback obj is null");
    int32_t result = SetAvailableDeviceChangeCallback(clientId, usage, object);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::UnsetAvailableDeviceChangeCallbackInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t clientId = data.ReadInt32();
    AudioDeviceUsage usage = static_cast<AudioDeviceUsage>(data.ReadInt32());
    int32_t result = UnsetAvailableDeviceChangeCallback(clientId, usage);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::ConfigDistributedRoutingRoleInternal(MessageParcel &data, MessageParcel &reply)
{
    sptr<AudioDeviceDescriptor> descriptor = AudioDeviceDescriptor::Unmarshalling(data);
    CastType type = static_cast<CastType>(data.ReadInt32());
    int32_t result = ConfigDistributedRoutingRole(descriptor, type);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::SetDistributedRoutingRoleCallbackInternal(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    CHECK_AND_RETURN_LOG(object != nullptr, "SetDistributedRoutingRoleCallback obj is null");
    int32_t result = SetDistributedRoutingRoleCallback(object);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::UnsetDistributedRoutingRoleCallbackInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = UnsetDistributedRoutingRoleCallback();
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::IsSpatializationEnabledInternal(MessageParcel &data, MessageParcel &reply)
{
    bool result = IsSpatializationEnabled();
    reply.WriteBool(result);
}

void AudioPolicyManagerStub::SetSpatializationEnabledInternal(MessageParcel &data, MessageParcel &reply)
{
    bool enable = data.ReadBool();
    int32_t result = SetSpatializationEnabled(enable);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::IsHeadTrackingEnabledInternal(MessageParcel &data, MessageParcel &reply)
{
    bool result = IsHeadTrackingEnabled();
    reply.WriteBool(result);
}

void AudioPolicyManagerStub::SetHeadTrackingEnabledInternal(MessageParcel &data, MessageParcel &reply)
{
    bool enable = data.ReadBool();
    int32_t result = SetHeadTrackingEnabled(enable);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::RegisterSpatializationEnabledEventListenerInternal(MessageParcel &data,
    MessageParcel &reply)
{
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    CHECK_AND_RETURN_LOG(remoteObject != nullptr, "AudioSpatializationEnabledChangeCallback obj is null");
    int32_t ret = RegisterSpatializationEnabledEventListener(remoteObject);
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::RegisterHeadTrackingEnabledEventListenerInternal(MessageParcel &data,
    MessageParcel &reply)
{
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    CHECK_AND_RETURN_LOG(remoteObject != nullptr, "AudioHeadTrackingEnabledChangeCallback obj is null");
    int32_t ret = RegisterHeadTrackingEnabledEventListener(remoteObject);
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::UnregisterSpatializationEnabledEventListenerInternal(MessageParcel &data,
    MessageParcel &reply)
{
    int32_t ret = UnregisterSpatializationEnabledEventListener();
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::UnregisterHeadTrackingEnabledEventListenerInternal(MessageParcel &data,
    MessageParcel &reply)
{
    int32_t ret = UnregisterHeadTrackingEnabledEventListener();
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::GetSpatializationStateInternal(MessageParcel &data, MessageParcel &reply)
{
    StreamUsage streamUsage = static_cast<StreamUsage>(data.ReadInt32());
    AudioSpatializationState spatializationState = GetSpatializationState(streamUsage);
    reply.WriteBool(spatializationState.spatializationEnabled);
    reply.WriteBool(spatializationState.headTrackingEnabled);
}

void AudioPolicyManagerStub::IsSpatializationSupportedInternal(MessageParcel &data, MessageParcel &reply)
{
    bool isSupported = IsSpatializationSupported();
    reply.WriteBool(isSupported);
}

void AudioPolicyManagerStub::IsSpatializationSupportedForDeviceInternal(MessageParcel &data, MessageParcel &reply)
{
    std::string address = data.ReadString();
    bool result = IsSpatializationSupportedForDevice(address);
    reply.WriteBool(result);
}

void AudioPolicyManagerStub::IsHeadTrackingSupportedInternal(MessageParcel &data, MessageParcel &reply)
{
    bool isSupported = IsHeadTrackingSupported();
    reply.WriteBool(isSupported);
}

void AudioPolicyManagerStub::IsHeadTrackingSupportedForDeviceInternal(MessageParcel &data, MessageParcel &reply)
{
    std::string address = data.ReadString();
    bool result = IsHeadTrackingSupportedForDevice(address);
    reply.WriteBool(result);
}

void AudioPolicyManagerStub::UpdateSpatialDeviceStateInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioSpatialDeviceState audioSpatialDeviceState;
    audioSpatialDeviceState.address = data.ReadString();
    audioSpatialDeviceState.isSpatializationSupported = data.ReadBool();
    audioSpatialDeviceState.isHeadTrackingSupported = data.ReadBool();
    audioSpatialDeviceState.spatialDeviceType = static_cast<AudioSpatialDeviceType>(data.ReadInt32());
    int32_t result = UpdateSpatialDeviceState(audioSpatialDeviceState);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::RegisterSpatializationStateEventListenerInternal(MessageParcel &data,
    MessageParcel &reply)
{
    uint32_t sessionID = static_cast<uint32_t>(data.ReadInt32());
    StreamUsage streamUsage = static_cast<StreamUsage>(data.ReadInt32());
    sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
    CHECK_AND_RETURN_LOG(remoteObject != nullptr, "AudioSpatializationStateChangeCallback obj is null");
    int32_t ret = RegisterSpatializationStateEventListener(sessionID, streamUsage, remoteObject);
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::UnregisterSpatializationStateEventListenerInternal(MessageParcel &data,
    MessageParcel &reply)
{
    uint32_t sessionID = static_cast<uint32_t>(data.ReadInt32());
    int32_t ret = UnregisterSpatializationStateEventListener(sessionID);
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::RegisterPolicyCallbackClientInternal(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    int32_t zoneID = data.ReadInt32();
    CHECK_AND_RETURN_LOG(object != nullptr, "RegisterPolicyCallbackClientInternal obj is null");
    int32_t result = RegisterPolicyCallbackClient(object, zoneID);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::CreateAudioInterruptZoneInternal(MessageParcel &data, MessageParcel &reply)
{
    std::set<int32_t> pids;
    int32_t zoneID = data.ReadInt32();
    int32_t pidsSize = data.ReadInt32();
    pidsSize = pidsSize > MAX_PID_COUNT ? MAX_PID_COUNT : pidsSize;
    if (pidsSize > 0) {
        for (int32_t i = 0; i < pidsSize; i ++) {
            pids.insert(data.ReadInt32());
        }
    }
    int32_t result = CreateAudioInterruptZone(pids, zoneID);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::AddAudioInterruptZonePidsInternal(MessageParcel &data, MessageParcel &reply)
{
    std::set<int32_t> pids;
    int32_t zoneID = data.ReadInt32();
    int32_t pidsSize = data.ReadInt32();
    pidsSize = pidsSize > MAX_PID_COUNT ? MAX_PID_COUNT : pidsSize;
    if (pidsSize > 0) {
        for (int32_t i = 0; i < pidsSize; i ++) {
            pids.insert(data.ReadInt32());
        }
    }
    int32_t result = AddAudioInterruptZonePids(pids, zoneID);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::RemoveAudioInterruptZonePidsInternal(MessageParcel &data, MessageParcel &reply)
{
    std::set<int32_t> pids;
    int32_t zoneID = data.ReadInt32();
    int32_t pidsSize = data.ReadInt32();
    pidsSize = pidsSize > MAX_PID_COUNT ? MAX_PID_COUNT : pidsSize;
    if (pidsSize > 0) {
        for (int32_t i = 0; i < pidsSize; i ++) {
            pids.insert(data.ReadInt32());
        }
    }
    int32_t result = RemoveAudioInterruptZonePids(pids, zoneID);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::ReleaseAudioInterruptZoneInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t zoneID = data.ReadInt32();
    int32_t result = ReleaseAudioInterruptZone(zoneID);
    reply.WriteInt32(result);
}

int AudioPolicyManagerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    CHECK_AND_RETURN_RET_LOG(data.ReadInterfaceToken() == GetDescriptor(), -1, "ReadInterfaceToken failed");
    if (code <= static_cast<uint32_t>(AudioPolicyInterfaceCode::AUDIO_POLICY_MANAGER_CODE_MAX)) {
        (this->*handlers[code])(data, reply);
        return AUDIO_OK;
    }
    AUDIO_ERR_LOG("default case, need check AudioPolicyManagerStub");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

void AudioPolicyManagerStub::SetCallDeviceActiveInternal(MessageParcel &data, MessageParcel &reply)
{
    InternalDeviceType deviceType = static_cast<InternalDeviceType>(data.ReadInt32());
    bool active = data.ReadBool();
    std::string address = data.ReadString();
    int32_t result = SetCallDeviceActive(deviceType, active, address);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::GetActiveBluetoothDeviceInternal(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<AudioDeviceDescriptor> desc = GetActiveBluetoothDevice();
    desc->Marshalling(reply);
}
} // namespace audio_policy
} // namespace OHOS
