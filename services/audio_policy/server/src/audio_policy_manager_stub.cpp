/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "AudioPolicyManagerStub"
#endif

#include "audio_policy_manager_stub.h"

#include "audio_errors.h"
#include "audio_policy_log.h"
#include "audio_utils.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace AudioStandard {
namespace {
constexpr int MAX_PID_COUNT = 1000;
const unsigned int ON_REMOTE_REQUEST_TIMEOUT_SEC = 20;
const char *g_audioPolicyCodeStrs[] = {
    "GET_MAX_VOLUMELEVEL",
    "GET_MIN_VOLUMELEVEL",
    "SET_SYSTEM_VOLUMELEVEL_LEGACY",
    "SET_SYSTEM_VOLUMELEVEL",
    "SET_APP_VOLUMELEVEL",
    "SET_APP_VOLUME_MUTED",
    "IS_APP_MUTE",
    "SET_SELF_APP_VOLUMELEVEL",
    "SET_SYSTEM_VOLUMELEVEL_WITH_DEVICE",
    "GET_SYSTEM_VOLUMELEVEL",
    "GET_APP_VOLUMELEVEL",
    "GET_SELF_APP_VOLUME_LEVEL",
    "SET_STREAM_MUTE_LEGACY",
    "SET_STREAM_MUTE",
    "GET_STREAM_MUTE",
    "IS_STREAM_ACTIVE",
    "SET_DEVICE_ACTIVE",
    "IS_DEVICE_ACTIVE",
    "GET_ACTIVE_OUTPUT_DEVICE",
    "GET_ACTIVE_INPUT_DEVICE",
    "SET_RINGER_MODE_LEGACY",
    "SET_RINGER_MODE",
    "GET_RINGER_MODE",
    "SET_AUDIO_SCENE",
    "GET_AUDIO_SCENE",
    "SET_MICROPHONE_MUTE",
    "SET_MICROPHONE_MUTE_AUDIO_CONFIG",
    "IS_MICROPHONE_MUTE_LEGACY",
    "IS_MICROPHONE_MUTE",
    "SET_CALLBACK",
    "UNSET_CALLBACK",
    "SET_QUERY_CLIENT_TYPE_CALLBACK",
    "SET_CLIENT_INFO_MGR_CALLBACK",
    "ACTIVATE_INTERRUPT",
    "DEACTIVATE_INTERRUPT",
    "SET_INTERRUPT_CALLBACK",
    "UNSET_INTERRUPT_CALLBACK",
    "REQUEST_AUDIO_FOCUS",
    "ABANDON_AUDIO_FOCUS",
    "GET_STREAM_IN_FOCUS",
    "GET_SESSION_INFO_IN_FOCUS",
    "GET_DEVICES",
    "SELECT_OUTPUT_DEVICE",
    "GET_SELECTED_DEVICE_INFO",
    "SELECT_INPUT_DEVICE",
    "RECONFIGURE_CHANNEL",
    "GET_AUDIO_LATENCY",
    "GET_SINK_LATENCY",
    "GET_PREFERRED_OUTPUT_STREAM_TYPE",
    "GET_PREFERRED_INPUT_STREAM_TYPE",
    "REGISTER_TRACKER",
    "UPDATE_TRACKER",
    "GET_RENDERER_CHANGE_INFOS",
    "GET_CAPTURER_CHANGE_INFOS",
    "SET_LOW_POWER_STREM_VOLUME",
    "GET_LOW_POWRR_STREM_VOLUME",
    "UPDATE_STREAM_STATE",
    "GET_SINGLE_STREAM_VOLUME",
    "GET_VOLUME_GROUP_INFO",
    "GET_NETWORKID_BY_GROUP_ID",
#ifdef FEATURE_DTMF_TONE
    "GET_TONEINFO",
    "GET_SUPPORTED_TONES",
#endif
    "IS_AUDIO_RENDER_LOW_LATENCY_SUPPORTED",
    "GET_ACTIVE_OUTPUT_DEVICE_DESCRIPTORS",
    "GET_PREFERRED_INTPUT_DEVICE_DESCRIPTORS",
    "SET_CALLBACKS_ENABLE",
    "GET_AUDIO_FOCUS_INFO_LIST",
    "SET_SYSTEM_SOUND_URI",
    "GET_SYSTEM_SOUND_URI",
    "GET_MIN_VOLUME_STREAM",
    "GET_MAX_VOLUME_STREAM",
    "GET_MAX_RENDERER_INSTANCES",
    "IS_VOLUME_UNADJUSTABLE",
    "ADJUST_VOLUME_BY_STEP",
    "ADJUST_SYSTEM_VOLUME_BY_STEP",
    "GET_SYSTEM_VOLUME_IN_DB",
    "QUERY_EFFECT_SCENEMODE",
    "GET_HARDWARE_OUTPUT_SAMPLING_RATE",
    "GET_AUDIO_CAPTURER_MICROPHONE_DESCRIPTORS",
    "GET_AVAILABLE_MICROPHONE_DESCRIPTORS",
    "SET_DEVICE_ABSOLUTE_VOLUME_SUPPORTED",
    "GET_ABS_VOLUME_SCENE",
    "SET_A2DP_DEVICE_VOLUME",
    "GET_AVAILABLE_DESCRIPTORS",
    "SET_AVAILABLE_DEVICE_CHANGE_CALLBACK",
    "UNSET_AVAILABLE_DEVICE_CHANGE_CALLBACK",
    "IS_SPATIALIZATION_ENABLED",
    "IS_SPATIALIZATION_ENABLED_FOR_DEVICE",
    "SET_SPATIALIZATION_ENABLED",
    "SET_SPATIALIZATION_ENABLED_FOR_DEVICE",
    "IS_HEAD_TRACKING_ENABLED",
    "IS_HEAD_TRACKING_ENABLED_FOR_DEVICE",
    "SET_HEAD_TRACKING_ENABLED",
    "SET_HEAD_TRACKING_ENABLED_FOR_DEVICE",
    "GET_SPATIALIZATION_STATE",
    "IS_SPATIALIZATION_SUPPORTED",
    "IS_SPATIALIZATION_SUPPORTED_FOR_DEVICE",
    "IS_HEAD_TRACKING_SUPPORTED",
    "IS_HEAD_TRACKING_SUPPORTED_FOR_DEVICE",
    "UPDATE_SPATIAL_DEVICE_STATE",
    "REGISTER_SPATIALIZATION_STATE_EVENT",
    "CONFIG_DISTRIBUTED_ROUTING_ROLE",
    "SET_DISTRIBUTED_ROUTING_ROLE_CALLBACK",
    "UNSET_DISTRIBUTED_ROUTING_ROLE_CALLBACK",
    "UNREGISTER_SPATIALIZATION_STATE_EVENT",
    "REGISTER_POLICY_CALLBACK_CLIENT",
    "CREATE_AUDIO_INTERRUPT_ZONE",
    "ADD_AUDIO_INTERRUPT_ZONE_PIDS",
    "REMOVE_AUDIO_INTERRUPT_ZONE_PIDS",
    "RELEASE_AUDIO_INTERRUPT_ZONE",
    "SET_CALL_DEVICE_ACTIVE",
    "GET_AUDIO_CONVERTER_CONFIG",
    "GET_ACTIVE_BLUETOOTH_DESCRIPTOR",
    "FETCH_OUTPUT_DEVICE_FOR_TRACK",
    "FETCH_INPUT_DEVICE_FOR_TRACK",
    "IS_HIGH_RESOLUTION_EXIST",
    "SET_HIGH_RESOLUTION_EXIST",
    "GET_SPATIALIZATION_SCENE_TYPE",
    "SET_SPATIALIZATION_SCENE_TYPE",
    "GET_MAX_AMPLITUDE",
    "IS_HEAD_TRACKING_DATA_REQUESTED",
    "SET_AUDIO_DEVICE_REFINER_CALLBACK",
    "UNSET_AUDIO_DEVICE_REFINER_CALLBACK",
    "TRIGGER_FETCH_DEVICE",
    "MOVE_TO_NEW_PIPE",
    "DISABLE_SAFE_MEDIA_VOLUME",
    "GET_DEVICES_INNER",
    "SET_AUDIO_CONCURRENCY_CALLBACK",
    "UNSET_AUDIO_CONCURRENCY_CALLBACK",
    "ACTIVATE_AUDIO_CONCURRENCY",
    "SET_MICROPHONE_MUTE_PERSISTENT",
    "GET_MICROPHONE_MUTE_PERSISTENT",
    "GET_SUPPORT_AUDIO_EFFECT_PROPERTY_V3",
    "GET_AUDIO_EFFECT_PROPERTY_V3",
    "SET_AUDIO_EFFECT_PROPERTY_V3",
    "GET_SUPPORT_AUDIO_ENHANCE_PROPERTY",
    "GET_SUPPORT_AUDIO_EFFECT_PROPERTY",
    "GET_AUDIO_ENHANCE_PROPERTY",
    "GET_AUDIO_EFFECT_PROPERTY",
    "SET_AUDIO_ENHANCE_PROPERTY",
    "SET_AUDIO_EFFECT_PROPERTY",
    "INJECT_INTERRUPTION",
    "ACTIVATE_AUDIO_SESSION",
    "DEACTIVATE_AUDIO_SESSION",
    "IS_AUDIO_SESSION_ACTIVATED",
    "LOAD_SPLIT_MODULE",
    "SET_DEFAULT_OUTPUT_DEVICE",
    "GET_SYSTEM_ACTIVEVOLUME_TYPE",
    "GET_OUTPUT_DEVICE",
    "GET_INPUT_DEVICE",
    "SET_AUDIO_DEVICE_ANAHS_CALLBACK",
    "UNSET_AUDIO_DEVICE_ANAHS_CALLBACK",
    "IS_ALLOWED_PLAYBACK",
    "SET_VOICE_RINGTONE_MUTE",
    "SET_CALLBACK_RENDERER_INFO",
    "SET_CALLBACK_CAPTURER_INFO",
    "GET_STREAM_IN_FOCUS_BY_UID",
    "SET_PREFERRED_DEVICE",
    "SAVE_REMOTE_INFO",
    "SET_VIRTUAL_CALL",
    "SET_DEVICE_CONNECTION_STATUS",
    "EXCLUDE_OUTPUT_DEVICES",
    "UNEXCLUDE_OUTPUT_DEVICES",
    "GET_EXCLUDED_OUTPUT_DEVICES",
    "IS_SPATIALIZATION_ENABLED_FOR_CURRENT_DEVICE",
    "SET_QUERY_ALLOWED_PLAYBACK_CALLBACK",
};

constexpr size_t codeNums = sizeof(g_audioPolicyCodeStrs) / sizeof(const char *);
static_assert(codeNums == (static_cast<size_t> (AudioPolicyInterfaceCode::AUDIO_POLICY_MANAGER_CODE_MAX) + 1),
    "keep same with AudioPolicyInterfaceCode");
}
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

void AudioPolicyManagerStub::SetSystemVolumeLevelLegacyInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioVolumeType volumeType = static_cast<AudioVolumeType>(data.ReadInt32());
    int32_t volumeLevel = data.ReadInt32();
    int result = SetSystemVolumeLevelLegacy(volumeType, volumeLevel);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::SetSystemVolumeLevelInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioVolumeType volumeType = static_cast<AudioVolumeType>(data.ReadInt32());
    int32_t volumeLevel = data.ReadInt32();
    int32_t volumeFlag = data.ReadInt32();
    int result = SetSystemVolumeLevel(volumeType, volumeLevel, volumeFlag);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::SetSystemVolumeLevelWithDeviceInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioVolumeType volumeType = static_cast<AudioVolumeType>(data.ReadInt32());
    int32_t volumeLevel = data.ReadInt32();
    DeviceType deviceType = static_cast<DeviceType>(data.ReadInt32());
    int32_t volumeFlag = data.ReadInt32();
    int result = SetSystemVolumeLevelWithDevice(volumeType, volumeLevel, deviceType, volumeFlag);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::SetSelfAppVolumeLevelInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t volumeLevel = data.ReadInt32();
    int32_t volumeFlag = data.ReadInt32();
    int result = SetSelfAppVolumeLevel(volumeLevel, volumeFlag);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::SetAppVolumeMutedInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t appUid = data.ReadInt32();
    bool muted = data.ReadBool();
    int32_t volumeFlag = data.ReadInt32();
    int result = SetAppVolumeMuted(appUid, muted, volumeFlag);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::GetAppVolumeIsMuteInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t appUid = data.ReadInt32();
    bool owned = data.ReadBool();
    int result = IsAppVolumeMute(appUid, owned);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::SetAppVolumeLevelInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t appUid = data.ReadInt32();
    int32_t volumeLevel = data.ReadInt32();
    int32_t volumeFlag = data.ReadInt32();
    int result = SetAppVolumeLevel(appUid, volumeLevel, volumeFlag);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::SetRingerModeLegacyInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioRingerMode rMode = static_cast<AudioRingerMode>(data.ReadInt32());
    int32_t result = SetRingerModeLegacy(rMode);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::SetRingerModeInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioRingerMode rMode = static_cast<AudioRingerMode>(data.ReadInt32());
    int32_t result = SetRingerMode(rMode);
    reply.WriteInt32(result);
}

#ifdef FEATURE_DTMF_TONE
void AudioPolicyManagerStub::GetToneInfoInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t ltonetype = data.ReadInt32();
    std::string countryCode = data.ReadString();
    std::shared_ptr<ToneInfo> ltoneInfo = GetToneConfig(ltonetype, countryCode);
    CHECK_AND_RETURN_LOG(ltoneInfo != nullptr, "obj is null");
    ltoneInfo->Marshalling(reply);
}

void AudioPolicyManagerStub::GetSupportedTonesInternal(MessageParcel &data, MessageParcel &reply)
{
    std::string countryCode = data.ReadString();
    int32_t lToneListSize = 0;
    std::vector<int32_t> lToneList = GetSupportedTones(countryCode);
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

void AudioPolicyManagerStub::IsMicrophoneMuteLegacyInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = IsMicrophoneMuteLegacy();
    reply.WriteBool(result);
}

void AudioPolicyManagerStub::IsMicrophoneMuteInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = IsMicrophoneMute();
    reply.WriteBool(result);
}

void AudioPolicyManagerStub::GetAudioSceneInternal(MessageParcel & /* data */, MessageParcel &reply)
{
    AudioScene audioScene = GetAudioScene();
    reply.WriteInt32(static_cast<int>(audioScene));
}

void AudioPolicyManagerStub::GetSystemActiveVolumeTypeInternal(MessageParcel& data, MessageParcel& reply)
{
    int32_t clientUid = data.ReadInt32();
    AudioStreamType volumeType = GetSystemActiveVolumeType(clientUid);
    reply.WriteInt32(volumeType);
}

void AudioPolicyManagerStub::GetSystemVolumeLevelInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioStreamType streamType = static_cast<AudioStreamType>(data.ReadInt32());
    int32_t volumeLevel = GetSystemVolumeLevel(streamType);
    reply.WriteInt32(volumeLevel);
}

void AudioPolicyManagerStub::GetAppVolumeLevelInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t appUid = data.ReadInt32();
    int32_t volumeLevel = GetAppVolumeLevel(appUid);
    reply.WriteInt32(volumeLevel);
}

void AudioPolicyManagerStub::GetSelfAppVolumeLevelInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t volumeLevel = GetSelfAppVolumeLevel();
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

void AudioPolicyManagerStub::SetStreamMuteLegacyInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioVolumeType volumeType = static_cast<AudioVolumeType>(data.ReadInt32());
    bool mute = data.ReadBool();
    DeviceType deviceType = static_cast<DeviceType>(data.ReadInt32());
    int result = SetStreamMuteLegacy(volumeType, mute);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::SetStreamMuteInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioVolumeType volumeType = static_cast<AudioVolumeType>(data.ReadInt32());
    bool mute = data.ReadBool();
    DeviceType deviceType = static_cast<DeviceType>(data.ReadInt32());
    int result = SetStreamMute(volumeType, mute, deviceType);
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

void AudioPolicyManagerStub::SetClientCallbacksEnableInternal(MessageParcel &data, MessageParcel &reply)
{
    CallbackChange callbackchange = static_cast<CallbackChange>(data.ReadInt32());
    bool enable = data.ReadBool();
    int32_t result = SetClientCallbacksEnable(callbackchange, enable);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::SetCallbackRendererInfoInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioRendererInfo rendererInfo;
    rendererInfo.Unmarshalling(data);
    int32_t result = SetCallbackRendererInfo(rendererInfo);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::SetCallbackCapturerInfoInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioCapturerInfo capturerInfo;
    capturerInfo.Unmarshalling(data);
    int32_t result = SetCallbackCapturerInfo(capturerInfo);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::WriteAudioFocusInfo(MessageParcel &reply,
    const std::pair<AudioInterrupt, AudioFocuState> &focusInfo)
{
    AudioInterrupt::Marshalling(reply, focusInfo.first);
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

void AudioPolicyManagerStub::SetInterruptCallbackInternal(MessageParcel &data, MessageParcel &reply)
{
    uint32_t sessionID = data.ReadUint32();
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    uint32_t zoneID = data.ReadUint32();
    uint32_t clientUid = data.ReadUint32();
    CHECK_AND_RETURN_LOG(object != nullptr, "AudioPolicyManagerStub: AudioInterruptCallback obj is null");
    int32_t result = SetAudioInterruptCallback(sessionID, object, clientUid, zoneID);
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
    bool isUpdatedAudioStrategy = data.ReadBool();
    AudioInterrupt audioInterrupt = {};
    AudioInterrupt::Unmarshalling(data, audioInterrupt);
    int32_t result = ActivateAudioInterrupt(audioInterrupt, zoneID, isUpdatedAudioStrategy);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::DeactivateInterruptInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t zoneID = data.ReadInt32();
    AudioInterrupt audioInterrupt = {};
    AudioInterrupt::Unmarshalling(data, audioInterrupt);
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
    AudioInterrupt::Unmarshalling(data, audioInterrupt);
    int32_t result = RequestAudioFocus(clientId, audioInterrupt);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::AbandonAudioFocusInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioInterrupt audioInterrupt = {};
    int32_t clientId = data.ReadInt32();
    AudioInterrupt::Unmarshalling(data, audioInterrupt);
    int32_t result = AbandonAudioFocus(clientId, audioInterrupt);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::GetStreamInFocusInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t zoneID = data.ReadInt32();
    AudioStreamType streamInFocus = GetStreamInFocus(zoneID);
    reply.WriteInt32(static_cast<int32_t>(streamInFocus));
}

void AudioPolicyManagerStub::GetStreamInFocusByUidInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t zoneID = data.ReadInt32();
    int32_t uid = data.ReadInt32();
    AudioStreamType streamInFocus = GetStreamInFocusByUid(uid, zoneID);
    reply.WriteInt32(static_cast<int32_t>(streamInFocus));
}

void AudioPolicyManagerStub::GetSessionInfoInFocusInternal(MessageParcel &data, MessageParcel &reply)
{
    uint32_t invalidSessionID = static_cast<uint32_t>(-1);
    AudioInterrupt audioInterrupt {STREAM_USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN,
        {AudioStreamType::STREAM_DEFAULT, SourceType::SOURCE_TYPE_INVALID, true}, invalidSessionID};
    int32_t zoneID = data.ReadInt32();
    int32_t ret = GetSessionInfoInFocus(audioInterrupt, zoneID);
    AudioInterrupt::Marshalling(reply, audioInterrupt);
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::GetPreferredOutputStreamTypeInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioRendererInfo rendererInfo;
    rendererInfo.Unmarshalling(data);
    int32_t result = GetPreferredOutputStreamType(rendererInfo);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::GetPreferredInputStreamTypeInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioCapturerInfo capturerInfo;
    capturerInfo.Unmarshalling(data);
    int32_t result = GetPreferredInputStreamType(capturerInfo);
    reply.WriteInt32(result);
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
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    int ret = GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("AudioPolicyManagerStub:GetRendererChangeInfos Error!!");
        reply.WriteInt32(size);
        return;
    }

    size = audioRendererChangeInfos.size();
    reply.WriteInt32(size);
    for (const std::shared_ptr<AudioRendererChangeInfo> &rendererChangeInfo: audioRendererChangeInfos) {
        CHECK_AND_CONTINUE_LOG(rendererChangeInfo != nullptr,
            "AudioPolicyManagerStub:Renderer change info null, something wrong!!");
        rendererChangeInfo->Marshalling(reply);
    }
}

void AudioPolicyManagerStub::GetCapturerChangeInfosInternal(MessageParcel &data, MessageParcel &reply)
{
    size_t size = 0;
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    int32_t ret = GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("AudioPolicyManagerStub:GetCapturerChangeInfos Error!!");
        reply.WriteInt32(size);
        return;
    }

    size = audioCapturerChangeInfos.size();
    reply.WriteInt32(size);
    for (const std::shared_ptr<AudioCapturerChangeInfo> &capturerChangeInfo: audioCapturerChangeInfos) {
        CHECK_AND_CONTINUE_LOG(capturerChangeInfo != nullptr,
            "AudioPolicyManagerStub:Capturer change info null, something wrong!!");
        capturerChangeInfo->Marshalling(reply);
    }
}

void AudioPolicyManagerStub::UpdateStreamStateInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t clientUid = data.ReadInt32();
    StreamSetState streamSetState = static_cast<StreamSetState>(data.ReadInt32());
    StreamUsage streamUsage = static_cast<StreamUsage>(data.ReadInt32());

    int32_t result = UpdateStreamState(clientUid, streamSetState, streamUsage);
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
    reply.WriteInt32(GetMaxRendererInstances());
}

static void PreprocessMode(SupportedEffectConfig &supportedEffectConfig, MessageParcel &reply, int32_t i, int32_t j)
{
    reply.WriteString(supportedEffectConfig.preProcessNew.stream[i].streamEffectMode[j].mode);
    uint32_t countDev = supportedEffectConfig.preProcessNew.stream[i].streamEffectMode[j].devicePort.size();
    reply.WriteInt32(countDev);
    if (countDev > 0) {
        for (uint32_t k = 0; k < countDev; k++) {
            reply.WriteString(supportedEffectConfig.preProcessNew.stream[i].streamEffectMode[j].devicePort[k].type);
            reply.WriteString(supportedEffectConfig.preProcessNew.stream[i].streamEffectMode[j].devicePort[k].chain);
        }
    }
}
static void PreprocessProcess(SupportedEffectConfig &supportedEffectConfig, MessageParcel &reply, int32_t i)
{
    reply.WriteString(supportedEffectConfig.preProcessNew.stream[i].scene);
    uint32_t countMode = supportedEffectConfig.preProcessNew.stream[i].streamEffectMode.size();
    reply.WriteInt32(countMode);
    if (countMode > 0) {
        for (uint32_t j = 0; j < countMode; j++) {
            PreprocessMode(supportedEffectConfig, reply, i, j);
        }
    }
}
static void PostprocessMode(SupportedEffectConfig &supportedEffectConfig, MessageParcel &reply, int32_t i, int32_t j)
{
    reply.WriteString(supportedEffectConfig.postProcessNew.stream[i].streamEffectMode[j].mode);
    uint32_t countDev = supportedEffectConfig.postProcessNew.stream[i].streamEffectMode[j].devicePort.size();
    reply.WriteInt32(countDev);
    if (countDev > 0) {
        for (uint32_t k = 0; k < countDev; k++) {
            reply.WriteString(supportedEffectConfig.postProcessNew.stream[i].streamEffectMode[j].devicePort[k].type);
            reply.WriteString(supportedEffectConfig.postProcessNew.stream[i].streamEffectMode[j].devicePort[k].chain);
        }
    }
}
static void PostprocessProcess(SupportedEffectConfig &supportedEffectConfig, MessageParcel &reply, int32_t i)
{
    // i th stream
    reply.WriteString(supportedEffectConfig.postProcessNew.stream[i].scene);
    uint32_t countMode = supportedEffectConfig.postProcessNew.stream[i].streamEffectMode.size();
    reply.WriteInt32(countMode);
    if (countMode > 0) {
        for (uint32_t j = 0; j < countMode; j++) {
            PostprocessMode(supportedEffectConfig, reply, i, j);
        }
    }
}

void AudioPolicyManagerStub::QueryEffectSceneModeInternal(MessageParcel &data, MessageParcel &reply)
{
    uint32_t i;
    SupportedEffectConfig supportedEffectConfig;
    int32_t ret = QueryEffectSceneMode(supportedEffectConfig); // audio_policy_server.cpp
    CHECK_AND_RETURN_LOG(ret != -1, "default mode is unavailable !");

    uint32_t countPre = supportedEffectConfig.preProcessNew.stream.size();
    uint32_t countPost = supportedEffectConfig.postProcessNew.stream.size();
    uint32_t countPostMap = supportedEffectConfig.postProcessSceneMap.size();
    reply.WriteUint32(countPre);
    reply.WriteUint32(countPost);
    reply.WriteUint32(countPostMap);
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
    if (countPostMap > 0) {
        for (i = 0; i < countPostMap; i++) {
            reply.WriteString(supportedEffectConfig.postProcessSceneMap[i].name);
            reply.WriteString(supportedEffectConfig.postProcessSceneMap[i].sceneType);
        }
    }
}

void AudioPolicyManagerStub::GetHardwareOutputSamplingRateInternal(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = AudioDeviceDescriptor::UnmarshallingPtr(data);
    CHECK_AND_RETURN_LOG(audioDeviceDescriptor != nullptr, "Unmarshalling fail.");
    MapExternalToInternalDeviceType(*audioDeviceDescriptor);
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
    reply.WriteBool(IsAbsVolumeScene());
}

void AudioPolicyManagerStub::SetA2dpDeviceVolumeInternal(MessageParcel &data, MessageParcel &reply)
{
    std::string macAddress = data.ReadString();
    int32_t volume = data.ReadInt32();
    bool updateUi = data.ReadBool();
    int32_t result = SetA2dpDeviceVolume(macAddress, volume, updateUi);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::ConfigDistributedRoutingRoleInternal(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<AudioDeviceDescriptor> descriptor = AudioDeviceDescriptor::UnmarshallingPtr(data);
    MapExternalToInternalDeviceType(*descriptor);
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
    reply.WriteInt32(UnsetDistributedRoutingRoleCallback());
}

void AudioPolicyManagerStub::IsSpatializationEnabledInternal(MessageParcel &data, MessageParcel &reply)
{
    bool result = IsSpatializationEnabled();
    reply.WriteBool(result);
}

void AudioPolicyManagerStub::IsSpatializationEnabledForDeviceInternal(MessageParcel &data, MessageParcel &reply)
{
    std::string address = data.ReadString();
    bool result = IsSpatializationEnabled(address);
    reply.WriteBool(result);
}

void AudioPolicyManagerStub::IsSpatializationEnabledForCurrentDeviceInternal(MessageParcel &data, MessageParcel &reply)
{
    bool result = IsSpatializationEnabledForCurrentDevice();
    reply.WriteBool(result);
}

void AudioPolicyManagerStub::SetSpatializationEnabledInternal(MessageParcel &data, MessageParcel &reply)
{
    bool enable = data.ReadBool();
    int32_t result = SetSpatializationEnabled(enable);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::SetSpatializationEnabledForDeviceInternal(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = AudioDeviceDescriptor::UnmarshallingPtr(data);
    CHECK_AND_RETURN_LOG(audioDeviceDescriptor != nullptr, "Unmarshalling fail.");
    MapExternalToInternalDeviceType(*audioDeviceDescriptor);
    bool enable = data.ReadBool();
    int32_t result = SetSpatializationEnabled(audioDeviceDescriptor, enable);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::IsHeadTrackingEnabledInternal(MessageParcel &data, MessageParcel &reply)
{
    bool result = IsHeadTrackingEnabled();
    reply.WriteBool(result);
}

void AudioPolicyManagerStub::IsHeadTrackingEnabledForDeviceInternal(MessageParcel &data, MessageParcel &reply)
{
    std::string address = data.ReadString();
    bool result = IsHeadTrackingEnabled(address);
    reply.WriteBool(result);
}

void AudioPolicyManagerStub::SetHeadTrackingEnabledInternal(MessageParcel &data, MessageParcel &reply)
{
    bool enable = data.ReadBool();
    int32_t result = SetHeadTrackingEnabled(enable);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::SetHeadTrackingEnabledForDeviceInternal(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = AudioDeviceDescriptor::UnmarshallingPtr(data);
    CHECK_AND_RETURN_LOG(audioDeviceDescriptor != nullptr, "Unmarshalling fail.");
    MapExternalToInternalDeviceType(*audioDeviceDescriptor);
    bool enable = data.ReadBool();
    int32_t result = SetHeadTrackingEnabled(audioDeviceDescriptor, enable);
    reply.WriteInt32(result);
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

void AudioPolicyManagerStub::ActivateAudioSessionInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioSessionStrategy strategy;
    strategy.concurrencyMode = static_cast<AudioConcurrencyMode>(data.ReadInt32());
    int32_t result = ActivateAudioSession(strategy);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::DeactivateAudioSessionInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = DeactivateAudioSession();
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::IsAudioSessionActivatedInternal(MessageParcel &data, MessageParcel &reply)
{
    bool result = IsAudioSessionActivated();
    reply.WriteBool(result);
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

void AudioPolicyManagerStub::SetQueryClientTypeCallbackInternal(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    CHECK_AND_RETURN_LOG(object != nullptr, "SetQueryClientTypeCallback is null");
    int32_t result = SetQueryClientTypeCallback(object);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::SetAudioClientInfoMgrCallbackInternal(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    CHECK_AND_RETURN_LOG(object != nullptr, "AudioClientInfoMgrCallback is null");
    int32_t result = SetAudioClientInfoMgrCallback(object);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::OnMiddleTenRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    switch (code) {
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_CLIENT_INFO_MGR_CALLBACK):
            SetAudioClientInfoMgrCallbackInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_AUDIO_DEVICE_ANAHS_CALLBACK):
            SetAudioDeviceAnahsCallbackInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::UNSET_AUDIO_DEVICE_ANAHS_CALLBACK):
            UnsetAudioDeviceAnahsCallbackInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_VOICE_RINGTONE_MUTE):
            SetVoiceRingtoneMuteInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SUPPORT_AUDIO_EFFECT_PROPERTY_V3):
            GetSupportedAudioEffectPropertyV3Internal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AUDIO_EFFECT_PROPERTY_V3):
            GetAudioEffectPropertyV3Internal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_AUDIO_EFFECT_PROPERTY_V3):
            SetAudioEffectPropertyV3Internal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_PREFERRED_DEVICE):
            SetPreferredDeviceInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SAVE_REMOTE_INFO):
            SaveRemoteInfoInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_DEVICE_CONNECTION_STATUS):
            SetDeviceConnectionStatusInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::EXCLUDE_OUTPUT_DEVICES):
            ExcludeOutputDevicesInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::UNEXCLUDE_OUTPUT_DEVICES):
            UnexcludeOutputDevicesInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_EXCLUDED_OUTPUT_DEVICES):
            GetExcludedDevicesInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_QUERY_ALLOWED_PLAYBACK_CALLBACK):
            SetQueryAllowedPlaybackCallbackInternal(data, reply);
            break;
        default:
            AUDIO_ERR_LOG("default case, need check AudioPolicyManagerStub");
            IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
    }
}

void AudioPolicyManagerStub::OnMiddleNinRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    switch (code) {
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_OUTPUT_DEVICE):
            GetOutputDeviceInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_INPUT_DEVICE):
            GetInputDeviceInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_SPATIALIZATION_ENABLED_FOR_DEVICE):
            IsSpatializationEnabledForDeviceInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_SPATIALIZATION_ENABLED_FOR_DEVICE):
            SetSpatializationEnabledForDeviceInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_HEAD_TRACKING_ENABLED_FOR_DEVICE):
            IsHeadTrackingEnabledForDeviceInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_HEAD_TRACKING_ENABLED_FOR_DEVICE):
            SetHeadTrackingEnabledForDeviceInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_SPATIALIZATION_ENABLED_FOR_CURRENT_DEVICE):
            IsSpatializationEnabledForCurrentDeviceInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_QUERY_CLIENT_TYPE_CALLBACK):
            SetQueryClientTypeCallbackInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SUPPORT_AUDIO_ENHANCE_PROPERTY):
            GetSupportedAudioEnhancePropertyInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SUPPORT_AUDIO_EFFECT_PROPERTY):
            GetSupportedAudioEffectPropertyInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AUDIO_ENHANCE_PROPERTY):
            GetAudioEnhancePropertyInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AUDIO_EFFECT_PROPERTY):
            GetAudioEffectPropertyInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_AUDIO_ENHANCE_PROPERTY):
            SetAudioEnhancePropertyInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_AUDIO_EFFECT_PROPERTY):
            SetAudioEffectPropertyInternal(data, reply);
            break;
        default:
            OnMiddleTenRemoteRequest(code, data, reply, option);
            break;
    }
}
void AudioPolicyManagerStub::OnMiddleEigRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    switch (code) {
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::TRIGGER_FETCH_DEVICE):
            TriggerFetchDeviceInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::MOVE_TO_NEW_PIPE):
            MoveToNewTypeInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::DISABLE_SAFE_MEDIA_VOLUME):
            DisableSafeMediaVolumeInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_DEVICES_INNER):
            GetDevicesInnerInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_AUDIO_CONCURRENCY_CALLBACK):
            SetConcurrencyCallbackInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::UNSET_AUDIO_CONCURRENCY_CALLBACK):
            UnsetConcurrencyCallbackInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::ACTIVATE_AUDIO_CONCURRENCY):
            ActivateAudioConcurrencyInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_MICROPHONE_MUTE_PERSISTENT):
            SetMicrophoneMutePersistentInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_MICROPHONE_MUTE_PERSISTENT):
            GetMicrophoneMutePersistentInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::INJECT_INTERRUPTION):
            InjectInterruptionInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::ACTIVATE_AUDIO_SESSION):
            ActivateAudioSessionInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::DEACTIVATE_AUDIO_SESSION):
            DeactivateAudioSessionInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_AUDIO_SESSION_ACTIVATED):
            IsAudioSessionActivatedInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_VIRTUAL_CALL):
            SetVirtualCallInternal(data, reply);
            break;
        default:
            OnMiddleNinRemoteRequest(code, data, reply, option);
            break;
    }
}

void AudioPolicyManagerStub::OnMiddleSevRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    switch (code) {
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::RELEASE_AUDIO_INTERRUPT_ZONE):
            ReleaseAudioInterruptZoneInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_CALL_DEVICE_ACTIVE):
            SetCallDeviceActiveInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AUDIO_CONVERTER_CONFIG):
            GetConverterConfigInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_ACTIVE_BLUETOOTH_DESCRIPTOR):
            GetActiveBluetoothDeviceInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::FETCH_OUTPUT_DEVICE_FOR_TRACK):
            FetchOutputDeviceForTrackInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::FETCH_INPUT_DEVICE_FOR_TRACK):
            FetchInputDeviceForTrackInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_HIGH_RESOLUTION_EXIST):
            IsHighResolutionExistInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_HIGH_RESOLUTION_EXIST):
            SetHighResolutionExistInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SPATIALIZATION_SCENE_TYPE):
            GetSpatializationSceneTypeInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_SPATIALIZATION_SCENE_TYPE):
            SetSpatializationSceneTypeInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_MAX_AMPLITUDE):
            GetMaxAmplitudeInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_HEAD_TRACKING_DATA_REQUESTED):
            IsHeadTrackingDataRequestedInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_AUDIO_DEVICE_REFINER_CALLBACK):
            SetAudioDeviceRefinerCallbackInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::UNSET_AUDIO_DEVICE_REFINER_CALLBACK):
            UnsetAudioDeviceRefinerCallbackInternal(data, reply);
            break;
        default:
            OnMiddleEigRemoteRequest(code, data, reply, option);
            break;
    }
}

void AudioPolicyManagerStub::OnMiddleSixRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    switch (code) {
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_SPATIALIZATION_SUPPORTED_FOR_DEVICE):
            IsSpatializationSupportedForDeviceInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_HEAD_TRACKING_SUPPORTED):
            IsHeadTrackingSupportedInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_HEAD_TRACKING_SUPPORTED_FOR_DEVICE):
            IsHeadTrackingSupportedForDeviceInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::UPDATE_SPATIAL_DEVICE_STATE):
            UpdateSpatialDeviceStateInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::REGISTER_SPATIALIZATION_STATE_EVENT):
            RegisterSpatializationStateEventListenerInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::CONFIG_DISTRIBUTED_ROUTING_ROLE):
            ConfigDistributedRoutingRoleInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_DISTRIBUTED_ROUTING_ROLE_CALLBACK):
            SetDistributedRoutingRoleCallbackInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::UNSET_DISTRIBUTED_ROUTING_ROLE_CALLBACK):
            UnsetDistributedRoutingRoleCallbackInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::UNREGISTER_SPATIALIZATION_STATE_EVENT):
            UnregisterSpatializationStateEventListenerInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::REGISTER_POLICY_CALLBACK_CLIENT):
            RegisterPolicyCallbackClientInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::CREATE_AUDIO_INTERRUPT_ZONE):
            CreateAudioInterruptZoneInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::ADD_AUDIO_INTERRUPT_ZONE_PIDS):
            AddAudioInterruptZonePidsInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::REMOVE_AUDIO_INTERRUPT_ZONE_PIDS):
            RemoveAudioInterruptZonePidsInternal(data, reply);
            break;
        default:
            OnMiddleSevRemoteRequest(code, data, reply, option);
            break;
    }
}

void AudioPolicyManagerStub::OnMiddleFifRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    switch (code) {
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AVAILABLE_MICROPHONE_DESCRIPTORS):
            GetAvailableMicrophonesInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_DEVICE_ABSOLUTE_VOLUME_SUPPORTED):
            SetDeviceAbsVolumeSupportedInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_ABS_VOLUME_SCENE):
            IsAbsVolumeSceneInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_A2DP_DEVICE_VOLUME):
            SetA2dpDeviceVolumeInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AVAILABLE_DESCRIPTORS):
            GetAvailableDevicesInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_AVAILABLE_DEVICE_CHANGE_CALLBACK):
            SetAvailableDeviceChangeCallbackInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::UNSET_AVAILABLE_DEVICE_CHANGE_CALLBACK):
            UnsetAvailableDeviceChangeCallbackInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_SPATIALIZATION_ENABLED):
            IsSpatializationEnabledInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_SPATIALIZATION_ENABLED):
            SetSpatializationEnabledInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_HEAD_TRACKING_ENABLED):
            IsHeadTrackingEnabledInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_HEAD_TRACKING_ENABLED):
            SetHeadTrackingEnabledInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SPATIALIZATION_STATE):
            GetSpatializationStateInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_SPATIALIZATION_SUPPORTED):
            IsSpatializationSupportedInternal(data, reply);
            break;
        default:
            OnMiddleSixRemoteRequest(code, data, reply, option);
            break;
    }
}

void AudioPolicyManagerStub::OnMiddleFouRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    switch (code) {
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SYSTEM_SOUND_URI):
            GetSystemSoundUriInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_MIN_VOLUME_STREAM):
            GetMinStreamVolumeInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_MAX_VOLUME_STREAM):
            GetMaxStreamVolumeInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_MAX_RENDERER_INSTANCES):
            GetMaxRendererInstancesInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_VOLUME_UNADJUSTABLE):
            IsVolumeUnadjustableInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::ADJUST_VOLUME_BY_STEP):
            AdjustVolumeByStepInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::ADJUST_SYSTEM_VOLUME_BY_STEP):
            AdjustSystemVolumeByStepInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SYSTEM_VOLUME_IN_DB):
            GetSystemVolumeInDbInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::QUERY_EFFECT_SCENEMODE):
            QueryEffectSceneModeInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_HARDWARE_OUTPUT_SAMPLING_RATE):
            GetHardwareOutputSamplingRateInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AUDIO_CAPTURER_MICROPHONE_DESCRIPTORS):
            GetAudioCapturerMicrophoneDescriptorsInternal(data, reply);
            break;
        default:
            OnMiddleFifRemoteRequest(code, data, reply, option);
            break;
    }
}

void AudioPolicyManagerStub::OnMiddleTirRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    switch (code) {
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SINGLE_STREAM_VOLUME):
            GetSingleStreamVolumeInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_VOLUME_GROUP_INFO):
            GetVolumeGroupInfoInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_NETWORKID_BY_GROUP_ID):
            GetNetworkIdByGroupIdInternal(data, reply);
            break;
#ifdef FEATURE_DTMF_TONE
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_TONEINFO):
            GetToneInfoInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SUPPORTED_TONES):
            GetSupportedTonesInternal(data, reply);
            break;
#endif
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_ACTIVE_OUTPUT_DEVICE_DESCRIPTORS):
            GetPreferredOutputDeviceDescriptorsInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_PREFERRED_INTPUT_DEVICE_DESCRIPTORS):
            GetPreferredInputDeviceDescriptorsInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_CALLBACKS_ENABLE):
            SetClientCallbacksEnableInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_CALLBACK_RENDERER_INFO):
            SetCallbackRendererInfoInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AUDIO_FOCUS_INFO_LIST):
            GetAudioFocusInfoListInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_SYSTEM_SOUND_URI):
            SetSystemSoundUriInternal(data, reply);
            break;
        default:
            OnMiddleFouRemoteRequest(code, data, reply, option);
            break;
    }
}

void AudioPolicyManagerStub::OnMiddleSecRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    switch (code) {
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SELECT_INPUT_DEVICE):
            SelectInputDeviceInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::RECONFIGURE_CHANNEL):
            ReconfigureAudioChannelInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_PREFERRED_OUTPUT_STREAM_TYPE):
            GetPreferredOutputStreamTypeInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_PREFERRED_INPUT_STREAM_TYPE):
            GetPreferredInputStreamTypeInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::REGISTER_TRACKER):
            RegisterTrackerInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::UPDATE_TRACKER):
            UpdateTrackerInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_RENDERER_CHANGE_INFOS):
            GetRendererChangeInfosInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_CAPTURER_CHANGE_INFOS):
            GetCapturerChangeInfosInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_LOW_POWER_STREM_VOLUME):
            SetLowPowerVolumeInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_LOW_POWRR_STREM_VOLUME):
            GetLowPowerVolumeInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::UPDATE_STREAM_STATE):
            UpdateStreamStateInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_CALLBACK_CAPTURER_INFO):
            SetCallbackCapturerInfoInternal(data, reply);
            break;
        default:
            OnMiddleTirRemoteRequest(code, data, reply, option);
            break;
    }
}

void AudioPolicyManagerStub::OnMiddleFirRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    switch (code) {
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::ACTIVATE_INTERRUPT):
            ActivateInterruptInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::DEACTIVATE_INTERRUPT):
            DeactivateInterruptInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_INTERRUPT_CALLBACK):
            SetAudioManagerInterruptCbInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::UNSET_INTERRUPT_CALLBACK):
            UnsetAudioManagerInterruptCbInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::REQUEST_AUDIO_FOCUS):
            RequestAudioFocusInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::ABANDON_AUDIO_FOCUS):
            AbandonAudioFocusInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_STREAM_IN_FOCUS):
            GetStreamInFocusInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SESSION_INFO_IN_FOCUS):
            GetSessionInfoInFocusInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_DEVICES):
            GetDevicesInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SELECT_OUTPUT_DEVICE):
            SelectOutputDeviceInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SELECTED_DEVICE_INFO):
            GetSelectedDeviceInfoInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_STREAM_IN_FOCUS_BY_UID):
            GetStreamInFocusByUidInternal(data, reply);
            break;
        default:
            OnMiddleSecRemoteRequest(code, data, reply, option);
            break;
    }
}

void AudioPolicyManagerStub::OnMiddlesRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    switch (code) {
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_ACTIVE_OUTPUT_DEVICE):
            GetActiveOutputDeviceInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_ACTIVE_INPUT_DEVICE):
            GetActiveInputDeviceInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_RINGER_MODE_LEGACY):
            SetRingerModeLegacyInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_RINGER_MODE):
            SetRingerModeInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_RINGER_MODE):
            GetRingerModeInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_AUDIO_SCENE):
            SetAudioSceneInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_AUDIO_SCENE):
            GetAudioSceneInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_MICROPHONE_MUTE):
            SetMicrophoneMuteInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_MICROPHONE_MUTE_AUDIO_CONFIG):
            SetMicrophoneMuteAudioConfigInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_MICROPHONE_MUTE_LEGACY):
            IsMicrophoneMuteLegacyInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_MICROPHONE_MUTE):
            IsMicrophoneMuteInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_CALLBACK):
            SetInterruptCallbackInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::UNSET_CALLBACK):
            UnsetInterruptCallbackInternal(data, reply);
            break;
        default:
            OnMiddleFirRemoteRequest(code, data, reply, option);
            break;
    }
}

void AudioPolicyManagerStub::OnMidRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    switch (code) {
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SYSTEM_VOLUMELEVEL):
            GetSystemVolumeLevelInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_APP_VOLUMELEVEL):
            GetAppVolumeLevelInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SELF_APP_VOLUME_LEVEL):
            GetSelfAppVolumeLevelInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_STREAM_MUTE_LEGACY):
            SetStreamMuteLegacyInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_STREAM_MUTE):
            SetStreamMuteInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_STREAM_MUTE):
            GetStreamMuteInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_STREAM_ACTIVE):
            IsStreamActiveInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_DEVICE_ACTIVE):
            SetDeviceActiveInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_DEVICE_ACTIVE):
            IsDeviceActiveInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::LOAD_SPLIT_MODULE):
            LoadSplitModuleInternal(data, reply);
            break;
        case static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_ALLOWED_PLAYBACK):
            IsAllowedPlaybackInternal(data, reply);
            break;
        default:
            OnMiddlesRemoteRequest(code, data, reply, option);
            break;
    }
}

int AudioPolicyManagerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    CHECK_AND_RETURN_RET_LOG(data.ReadInterfaceToken() == GetDescriptor(), -1, "ReadInterfaceToken failed");
    Trace trace(code >= codeNums ? "invalid audio policy code" : g_audioPolicyCodeStrs[code]);
    AudioXCollie audioXCollie("AudioPolicy::ProcessIPC", ON_REMOTE_REQUEST_TIMEOUT_SEC, nullptr, nullptr,
        (AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY));
    if (code <= static_cast<uint32_t>(AudioPolicyInterfaceCode::AUDIO_POLICY_MANAGER_CODE_MAX)) {
        switch (code) {
            case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_MAX_VOLUMELEVEL):
                GetMaxVolumeLevelInternal(data, reply);
                break;
            case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_MIN_VOLUMELEVEL):
                GetMinVolumeLevelInternal(data, reply);
                break;
            case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_SYSTEM_VOLUMELEVEL_LEGACY):
                SetSystemVolumeLevelLegacyInternal(data, reply);
                break;
            case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_SYSTEM_VOLUMELEVEL):
                SetSystemVolumeLevelInternal(data, reply);
                break;
            case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_SYSTEM_VOLUMELEVEL_WITH_DEVICE):
                SetSystemVolumeLevelWithDeviceInternal(data, reply);
                break;
            case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_APP_VOLUMELEVEL):
                SetAppVolumeLevelInternal(data, reply);
                break;
            case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_SELF_APP_VOLUMELEVEL):
                SetSelfAppVolumeLevelInternal(data, reply);
                break;
            case static_cast<uint32_t>(AudioPolicyInterfaceCode::SET_APP_VOLUME_MUTED):
                SetAppVolumeMutedInternal(data, reply);
                break;
            case static_cast<uint32_t>(AudioPolicyInterfaceCode::IS_APP_MUTE):
                GetAppVolumeIsMuteInternal(data, reply);
                break;
            case static_cast<uint32_t>(AudioPolicyInterfaceCode::GET_SYSTEM_ACTIVEVOLUME_TYPE):
                GetSystemActiveVolumeTypeInternal(data, reply);
                break;
            default:
                OnMidRemoteRequest(code, data, reply, option);
                break;
        }
        return AUDIO_OK;
    }
    AUDIO_ERR_LOG("default case, need check AudioPolicyManagerStub");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

void AudioPolicyManagerStub::GetConverterConfigInternal(MessageParcel &data, MessageParcel &reply)
{
    ConverterConfig result = GetConverterConfig();
    reply.WriteString(result.library.name);
    reply.WriteString(result.library.path);
    reply.WriteUint64(result.outChannelLayout);
}

void AudioPolicyManagerStub::IsHighResolutionExistInternal(MessageParcel &data, MessageParcel &reply)
{
    bool ret = IsHighResolutionExist();
    reply.WriteBool(ret);
}

void AudioPolicyManagerStub::SetHighResolutionExistInternal(MessageParcel &data, MessageParcel &reply)
{
    bool highResExist = data.ReadBool();
    SetHighResolutionExist(highResExist);
}

void AudioPolicyManagerStub::GetSpatializationSceneTypeInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioSpatializationSceneType spatializationSceneType = GetSpatializationSceneType();
    reply.WriteInt32(static_cast<int32_t>(spatializationSceneType));
}

void AudioPolicyManagerStub::SetSpatializationSceneTypeInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioSpatializationSceneType spatializationSceneType = static_cast<AudioSpatializationSceneType>(data.ReadInt32());
    int32_t ret = SetSpatializationSceneType(spatializationSceneType);
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::GetMaxAmplitudeInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t deviceId = data.ReadInt32();
    float result = GetMaxAmplitude(deviceId);
    reply.WriteFloat(result);
}

void AudioPolicyManagerStub::IsHeadTrackingDataRequestedInternal(MessageParcel &data, MessageParcel &reply)
{
    std::string macAddress = data.ReadString();
    bool result = IsHeadTrackingDataRequested(macAddress);
    reply.WriteBool(result);
}

void AudioPolicyManagerStub::SetAudioDeviceRefinerCallbackInternal(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    int32_t result = SetAudioDeviceRefinerCallback(object);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::UnsetAudioDeviceRefinerCallbackInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = UnsetAudioDeviceRefinerCallback();
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::TriggerFetchDeviceInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioStreamDeviceChangeReasonExt reason(static_cast<AudioStreamDeviceChangeReasonExt::ExtEnum>(data.ReadInt32()));
    int32_t result = TriggerFetchDevice(reason);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::SetPreferredDeviceInternal(MessageParcel &data, MessageParcel &reply)
{
    PreferredType preferredType = static_cast<PreferredType>(data.ReadInt32());
    int32_t uid = static_cast<int32_t>(data.ReadInt32());
    std::shared_ptr<AudioDeviceDescriptor> desc = AudioDeviceDescriptor::UnmarshallingPtr(data);
    int32_t result = SetPreferredDevice(preferredType, desc, uid);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::SaveRemoteInfoInternal(MessageParcel &data, MessageParcel &reply)
{
    std::string networkId = data.ReadString();
    DeviceType deviceType = static_cast<DeviceType>(data.ReadInt32());
    SaveRemoteInfo(networkId, deviceType);
}

void AudioPolicyManagerStub::SetAudioDeviceAnahsCallbackInternal(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    int32_t result = SetAudioDeviceAnahsCallback(object);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::UnsetAudioDeviceAnahsCallbackInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = UnsetAudioDeviceAnahsCallback();
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::MoveToNewTypeInternal(MessageParcel &data, MessageParcel &reply)
{
    uint32_t sessionId = data.ReadUint32();
    AudioPipeType pipeType = static_cast<AudioPipeType>(data.ReadInt32());
    int32_t result = MoveToNewPipe(sessionId, pipeType);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::DisableSafeMediaVolumeInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = DisableSafeMediaVolume();
    reply.WriteInt32(ret);
}

void AudioPolicyManagerStub::SetConcurrencyCallbackInternal(MessageParcel &data, MessageParcel &reply)
{
    uint32_t sessionID = data.ReadUint32();
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    CHECK_AND_RETURN_LOG(object != nullptr, "AudioPolicyManagerStub: AudioInterruptCallback obj is null");
    int32_t result = SetAudioConcurrencyCallback(sessionID, object);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::UnsetConcurrencyCallbackInternal(MessageParcel &data, MessageParcel &reply)
{
    uint32_t sessionID = data.ReadUint32();
    int32_t result = UnsetAudioConcurrencyCallback(sessionID);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::ActivateAudioConcurrencyInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioPipeType pipeType = static_cast<AudioPipeType>(data.ReadInt32());
    int32_t result = ActivateAudioConcurrency(pipeType);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::SetMicrophoneMutePersistentInternal(MessageParcel &data, MessageParcel &reply)
{
    bool isMute = data.ReadBool();
    PolicyType type = static_cast<PolicyType>(data.ReadInt32());
    int32_t result = SetMicrophoneMutePersistent(isMute, type);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::GetMicrophoneMutePersistentInternal(MessageParcel &data, MessageParcel &reply)
{
    bool result = GetPersistentMicMuteState();
    reply.WriteBool(result);
}

void AudioPolicyManagerStub::GetSupportedAudioEffectPropertyV3Internal(MessageParcel &data, MessageParcel &reply)
{
    AudioEffectPropertyArrayV3 propertyArray = {};
    int32_t result = GetSupportedAudioEffectProperty(propertyArray);
    reply.WriteInt32(result);
    int32_t size = static_cast<int32_t>(propertyArray.property.size());
    CHECK_AND_RETURN_LOG(size >= 0 && size <= AUDIO_EFFECT_COUNT_UPPER_LIMIT,
        "get supported audio effect property size invalid.");
    reply.WriteInt32(size);
    for (int32_t i = 0; i < size; i++) {
        propertyArray.property[i].Marshalling(reply);
    }
    return;
}

void AudioPolicyManagerStub::SetAudioEffectPropertyV3Internal(MessageParcel &data, MessageParcel &reply)
{
    int32_t size = data.ReadInt32();
    CHECK_AND_RETURN_LOG(size > 0 && size <= AUDIO_EFFECT_COUNT_UPPER_LIMIT,
        "set audio effect property size invalid.");
    AudioEffectPropertyArrayV3 propertyArray = {};
    for (int32_t i = 0; i < size; i++) {
        AudioEffectPropertyV3 prop = {};
        prop.Unmarshalling(data);
        propertyArray.property.push_back(prop);
    }
    int32_t result = SetAudioEffectProperty(propertyArray);
    reply.WriteInt32(result);
    return;
}

void AudioPolicyManagerStub::GetAudioEffectPropertyV3Internal(MessageParcel &data, MessageParcel &reply)
{
    AudioEffectPropertyArrayV3 propertyArray = {};
    int32_t result = GetAudioEffectProperty(propertyArray);
    reply.WriteInt32(result);
    int32_t size = static_cast<int32_t>(propertyArray.property.size());
    CHECK_AND_RETURN_LOG(size >= 0 && size <= AUDIO_EFFECT_COUNT_UPPER_LIMIT,
        "get audio effect property size invalid.");
    reply.WriteInt32(size);
    for (int32_t i = 0; i < size; i++) {
        propertyArray.property[i].Marshalling(reply);
    }
    return;
}

void AudioPolicyManagerStub::GetSupportedAudioEnhancePropertyInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioEnhancePropertyArray propertyArray = {};
    int32_t result = GetSupportedAudioEnhanceProperty(propertyArray);
    int32_t size = propertyArray.property.size();
    CHECK_AND_RETURN_LOG(size >= 0 && size <= AUDIO_EFFECT_COUNT_UPPER_LIMIT,
        "get supported audio effect property size invalid.");
    reply.WriteInt32(size);
    for (int i = 0; i < size; i++) {
        propertyArray.property[i].Marshalling(reply);
    }
    reply.WriteInt32(result);
    return;
}

void AudioPolicyManagerStub::GetSupportedAudioEffectPropertyInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioEffectPropertyArray propertyArray = {};
    int32_t result = GetSupportedAudioEffectProperty(propertyArray);
    int32_t size = propertyArray.property.size();
    CHECK_AND_RETURN_LOG(size >= 0 && size <= AUDIO_EFFECT_COUNT_UPPER_LIMIT,
        "get supported audio effect property size invalid.");
    reply.WriteInt32(size);
    for (int i = 0; i < size; i++) {
        propertyArray.property[i].Marshalling(reply);
    }
    reply.WriteInt32(result);
    return;
}

void AudioPolicyManagerStub::SetAudioEffectPropertyInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t size = data.ReadInt32();
    CHECK_AND_RETURN_LOG(size > 0 && size <= AUDIO_EFFECT_COUNT_UPPER_LIMIT,
        "set audio effect property size upper limit.");
    AudioEffectPropertyArray propertyArray = {};
    for (int i = 0; i < size; i++) {
        AudioEffectProperty prop = {};
        prop.Unmarshalling(data);
        propertyArray.property.push_back(prop);
    }
    int32_t result = SetAudioEffectProperty(propertyArray);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::GetAudioEffectPropertyInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioEffectPropertyArray propertyArray = {};
    int32_t result = GetAudioEffectProperty(propertyArray);
    int32_t size = propertyArray.property.size();
    CHECK_AND_RETURN_LOG(size >= 0 && size <= AUDIO_EFFECT_COUNT_UPPER_LIMIT,
        "get audio effect property size invalid.");
    reply.WriteInt32(size);
    for (int i = 0; i < size; i++) {
        propertyArray.property[i].Marshalling(reply);
    }
    reply.WriteInt32(result);
    return;
}

void AudioPolicyManagerStub::SetAudioEnhancePropertyInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t size = data.ReadInt32();
    CHECK_AND_RETURN_LOG(size > 0 && size <= AUDIO_EFFECT_COUNT_UPPER_LIMIT,
        "set audio enhance property size upper limit.");
    AudioEnhancePropertyArray propertyArray = {};
    for (int i = 0; i < size; i++) {
        AudioEnhanceProperty prop = {};
        prop.Unmarshalling(data);
        propertyArray.property.push_back(prop);
    }
    int32_t result = SetAudioEnhanceProperty(propertyArray);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::GetAudioEnhancePropertyInternal(MessageParcel &data, MessageParcel &reply)
{
    AudioEnhancePropertyArray propertyArray = {};
    int32_t result = GetAudioEnhanceProperty(propertyArray);
    int32_t size = propertyArray.property.size();
    reply.WriteInt32(size);
    for (int i = 0; i < size; i++) {
        propertyArray.property[i].Marshalling(reply);
    }
    reply.WriteInt32(result);
    return;
}

void AudioPolicyManagerStub::InjectInterruptionInternal(MessageParcel &data, MessageParcel &reply)
{
    std::string networkId = data.ReadString();
    InterruptEvent event;
    event.eventType = static_cast<InterruptType>(data.ReadInt32());
    event.forceType = static_cast<InterruptForceType>(data.ReadInt32());
    event.hintType = static_cast<InterruptHint>(data.ReadInt32());

    int32_t result = InjectInterruption(networkId, event);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::LoadSplitModuleInternal(MessageParcel &data, MessageParcel &reply)
{
    std::string splitArgs = data.ReadString();
    std::string netWorkId = data.ReadString();
    int32_t result = LoadSplitModule(splitArgs, netWorkId);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::IsAllowedPlaybackInternal(MessageParcel &data, MessageParcel &reply)
{
    int32_t uid = data.ReadInt32();
    int32_t pid = data.ReadInt32();
    bool result = IsAllowedPlayback(uid, pid);
    reply.WriteBool(result);
}

void AudioPolicyManagerStub::SetVoiceRingtoneMuteInternal(MessageParcel &data, MessageParcel &reply)
{
    bool isMute = data.ReadBool();
    int32_t result = SetVoiceRingtoneMute(isMute);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::SetVirtualCallInternal(MessageParcel &data, MessageParcel &reply)
{
    bool isVirtual = data.ReadBool();
    int32_t result = SetVirtualCall(isVirtual);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::SetDeviceConnectionStatusInternal(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = AudioDeviceDescriptor::UnmarshallingPtr(data);
    bool isConnected = data.ReadBool();
    int32_t result = SetDeviceConnectionStatus(desc, isConnected);
    reply.WriteInt32(result);
}

void AudioPolicyManagerStub::SetQueryAllowedPlaybackCallbackInternal(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    CHECK_AND_RETURN_LOG(object != nullptr, "SetQueryAllowedPlaybackCallback is null");
    int32_t result = SetQueryAllowedPlaybackCallback(object);
    reply.WriteInt32(result);
}
} // namespace audio_policy
} // namespace OHOS
