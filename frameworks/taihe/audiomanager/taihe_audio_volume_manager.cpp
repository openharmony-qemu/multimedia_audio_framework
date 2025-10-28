/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioVolumeManagerImpl"
#endif

#include "taihe_audio_volume_manager.h"
#include "audio_log.h"
#include "audio_utils.h"
#include "taihe_audio_enum.h"
#include "taihe_audio_error.h"
#include "taihe_param_utils.h"
#include "taihe_appvolume_change_callback.h"
#include "taihe_active_volume_type_change_callback.h"

namespace ANI::Audio {
constexpr double VOLUME_DEFAULT_DOUBLE = 0.0;

AudioVolumeManagerImpl::AudioVolumeManagerImpl() : audioSystemMngr_(nullptr) {}

AudioVolumeManagerImpl::AudioVolumeManagerImpl(std::shared_ptr<AudioVolumeManagerImpl> obj)
    : audioSystemMngr_(nullptr)
{
    if (obj != nullptr) {
        audioSystemMngr_ = obj->audioSystemMngr_;
        cachedClientId_ = obj->cachedClientId_;
    }
}

AudioVolumeManagerImpl::~AudioVolumeManagerImpl()
{
    AUDIO_DEBUG_LOG("AudioVolumeManagerImpl::~AudioVolumeManagerImpl()");
}

array<VolumeGroupInfo> AudioVolumeManagerImpl::GetVolumeGroupInfosSync(string_view networkId)
{
    std::string innerNetworkId = std::string(networkId);
    std::vector<VolumeGroupInfo> emptyResult;
    if (audioSystemMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSystemMngr_ is nullptr");
        return array<VolumeGroupInfo>(emptyResult);
    }
    std::vector<OHOS::sptr<OHOS::AudioStandard::VolumeGroupInfo>> volumeGroupInfos;
    int32_t ret = audioSystemMngr_->GetVolumeGroups(innerNetworkId, volumeGroupInfos);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIO_OK, array<VolumeGroupInfo>(emptyResult), "GetVolumeGroups failure!");
    return TaiheParamUtils::SetVolumeGroupInfos(volumeGroupInfos);
}

int32_t AudioVolumeManagerImpl::GetAppVolumePercentageForUidSync(int32_t uid)
{
    int32_t appUid = uid;
    int32_t volLevel = 0;
    if (audioSystemMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSystemMngr_ is nullptr");
        return volLevel;
    }
    int32_t ret = audioSystemMngr_->GetAppVolume(appUid, volLevel);
    CHECK_AND_RETURN_RET(ret != OHOS::AudioStandard::SUCCESS, volLevel);
    if (ret == OHOS::AudioStandard::ERR_PERMISSION_DENIED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_PERMISSION);
        return volLevel;
    } else if (ret == OHOS::AudioStandard::ERR_SYSTEM_PERMISSION_DENIED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED);
        return volLevel;
    } else {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM);
        return volLevel;
    }
}

void AudioVolumeManagerImpl::SetAppVolumePercentageForUidSync(int32_t uid, int32_t volume)
{
    int32_t appUid = uid;
    int32_t volLevel = volume;
    if (audioSystemMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSystemMngr_ is nullptr");
        return;
    }
    int32_t intValue = audioSystemMngr_->SetAppVolume(appUid, volLevel);
    CHECK_AND_RETURN(intValue != OHOS::AudioStandard::SUCCESS);
    if (intValue == OHOS::AudioStandard::ERR_PERMISSION_DENIED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_PERMISSION);
        return;
    } else if (intValue == OHOS::AudioStandard::ERR_SYSTEM_PERMISSION_DENIED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED);
        return;
    } else if (intValue == OHOS::AudioStandard::ERR_INVALID_PARAM) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM);
        return;
    } else {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "System error. Set app volume fail.");
        return;
    }
}

bool AudioVolumeManagerImpl::IsAppVolumeMutedForUidSync(int32_t uid, bool owned)
{
    int32_t appUid = uid;
    bool isOwned = owned;
    bool isMute = false;
    if (audioSystemMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSystemMngr_ is nullptr");
        return isMute;
    }
    int32_t ret = audioSystemMngr_->IsAppVolumeMute(appUid, isOwned, isMute);
    CHECK_AND_RETURN_RET(ret != OHOS::AudioStandard::SUCCESS, isMute);
    if (ret == OHOS::AudioStandard::ERR_PERMISSION_DENIED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_PERMISSION);
        return isMute;
    } else if (ret == OHOS::AudioStandard::ERR_SYSTEM_PERMISSION_DENIED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED);
        return isMute;
    } else {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM);
        return isMute;
    }
}

void AudioVolumeManagerImpl::SetAppVolumeMutedForUidSync(int32_t uid, bool muted)
{
    int32_t appUid = uid;
    bool isMute = muted;
    if (audioSystemMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSystemMngr_ is nullptr");
        return;
    }
    int32_t intValue = audioSystemMngr_->SetAppVolumeMuted(appUid, isMute);
    CHECK_AND_RETURN(intValue != OHOS::AudioStandard::SUCCESS);
    if (intValue == OHOS::AudioStandard::ERR_PERMISSION_DENIED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_PERMISSION);
        return;
    } else if (intValue == OHOS::AudioStandard::ERR_SYSTEM_PERMISSION_DENIED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED);
        return;
    } else {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "System error. Set app volume fail.");
        return;
    }
}

int32_t AudioVolumeManagerImpl::GetAppVolumePercentageSync()
{
    int32_t volLevel = 0;
    if (audioSystemMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSystemMngr_ is nullptr");
        return volLevel;
    }
    int32_t ret = audioSystemMngr_->GetSelfAppVolume(volLevel);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM);
        return volLevel;
    } else {
        return volLevel;
    }
}

void AudioVolumeManagerImpl::SetAppVolumePercentageSync(int32_t volume)
{
    int32_t volLevel = volume;
    if (audioSystemMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSystemMngr_ is nullptr");
        return;
    }
    int32_t intValue = audioSystemMngr_->SetSelfAppVolume(volLevel);
    CHECK_AND_RETURN(intValue != OHOS::AudioStandard::SUCCESS);
    if (intValue == OHOS::AudioStandard::ERR_INVALID_PARAM) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM);
        return;
    } else {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "System error. Set app volume fail.");
        return;
    }
}

int32_t AudioVolumeManagerImpl::GetSystemVolume(AudioVolumeType volumeType)
{
    int32_t systemVolume = 0;
    int32_t volType = volumeType.get_value();
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return systemVolume;
    }
    if (audioSystemMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSystemMngr_ is nullptr");
        return systemVolume;
    }
    if (!TaiheAudioEnum::IsLegalInputArgumentVolType(volType)) {
        AUDIO_ERR_LOG("get volType failed: %{public}d", volType);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERROR_INVALID_PARAM,
            "parameter verification failed: The param of volType must be enum AudioVolumeType");
        return systemVolume;
    }
    systemVolume = audioSystemMngr_->GetVolume(TaiheAudioEnum::GetNativeAudioVolumeType(volType));
    return systemVolume;
}

int32_t AudioVolumeManagerImpl::GetMinSystemVolume(AudioVolumeType volumeType)
{
    int32_t minSystemVolume = 0;
    int32_t volType = volumeType.get_value();
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return minSystemVolume;
    }
    if (audioSystemMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSystemMngr_ is nullptr");
        return minSystemVolume;
    }
    if (!TaiheAudioEnum::IsLegalInputArgumentVolType(volType)) {
        AUDIO_ERR_LOG("get volType failed: %{public}d", volType);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERROR_INVALID_PARAM,
            "parameter verification failed: The param of volType must be enum AudioVolumeType");
        return minSystemVolume;
    }
    minSystemVolume = audioSystemMngr_->GetMinVolume(TaiheAudioEnum::GetNativeAudioVolumeType(volType));
    return minSystemVolume;
}

int32_t AudioVolumeManagerImpl::GetMaxSystemVolume(AudioVolumeType volumeType)
{
    int32_t maxSystemVolume = 0;
    int32_t volType = volumeType.get_value();
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return maxSystemVolume;
    }
    if (audioSystemMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSystemMngr_ is nullptr");
        return maxSystemVolume;
    }
    if (!TaiheAudioEnum::IsLegalInputArgumentVolType(volType)) {
        AUDIO_ERR_LOG("get volType failed: %{public}d", volType);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERROR_INVALID_PARAM,
            "parameter verification failed: The param of volType must be enum AudioVolumeType");
        return maxSystemVolume;
    }
    maxSystemVolume = audioSystemMngr_->GetMaxVolume(TaiheAudioEnum::GetNativeAudioVolumeType(volType));
    return maxSystemVolume;
}

int32_t AudioVolumeManagerImpl::GetVolumeByStream(StreamUsage streamUsage)
{
    int32_t volume = 0;
    int32_t streamUsageInt = streamUsage.get_value();
    if (audioSystemMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSystemMngr_ is nullptr");
        return volume;
    }
    if (!TaiheAudioEnum::IsLegalInputArgumentStreamUsage(streamUsageInt)) {
        AUDIO_ERR_LOG("get streamUsage failed: %{public}d", streamUsageInt);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERROR_INVALID_PARAM,
            "parameter verification failed: The param of streamUsage must be enum StreamUsage");
        return volume;
    }
    volume = audioSystemMngr_->GetVolumeByUsage(TaiheAudioEnum::GetNativeStreamUsage(streamUsageInt));
    return volume;
}

int32_t AudioVolumeManagerImpl::GetMinVolumeByStream(StreamUsage streamUsage)
{
    int32_t minVolume = 0;
    int32_t streamUsageInt = streamUsage.get_value();
    if (audioSystemMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSystemMngr_ is nullptr");
        return minVolume;
    }
    if (!TaiheAudioEnum::IsLegalInputArgumentStreamUsage(streamUsageInt)) {
        AUDIO_ERR_LOG("get streamUsage failed: %{public}d", streamUsageInt);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERROR_INVALID_PARAM,
            "parameter verification failed: The param of streamUsage must be enum StreamUsage");
        return minVolume;
    }
    minVolume = audioSystemMngr_->GetMinVolumeByUsage(TaiheAudioEnum::GetNativeStreamUsage(streamUsageInt));
    return minVolume;
}

int32_t AudioVolumeManagerImpl::GetMaxVolumeByStream(StreamUsage streamUsage)
{
    int32_t maxVolume = 0;
    int32_t streamUsageInt = streamUsage.get_value();
    if (audioSystemMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSystemMngr_ is nullptr");
        return maxVolume;
    }
    if (!TaiheAudioEnum::IsLegalInputArgumentStreamUsage(streamUsageInt)) {
        AUDIO_ERR_LOG("get streamUsage failed: %{public}d", streamUsageInt);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERROR_INVALID_PARAM,
            "parameter verification failed: The param of streamUsage must be enum StreamUsage");
        return maxVolume;
    }
    maxVolume = audioSystemMngr_->GetMaxVolumeByUsage(TaiheAudioEnum::GetNativeStreamUsage(streamUsageInt));
    return maxVolume;
}

double AudioVolumeManagerImpl::GetVolumeInUnitOfDb(AudioVolumeType volumeType, int32_t volumeLevel, DeviceType device)
{
    double volumeInDb = VOLUME_DEFAULT_DOUBLE;
    int32_t volType = volumeType.get_value();
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return volumeInDb;
    }
    if (!TaiheAudioEnum::IsLegalInputArgumentVolType(volType)) {
        AUDIO_ERR_LOG("get volumeType failed: %{public}d", volType);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: The param of volumeType must be enum AudioVolumeType");
        return volumeInDb;
    }
    int32_t volLevel = volumeLevel;
    int32_t deviceType = device.get_value();
    if (!TaiheAudioEnum::IsLegalInputArgumentDeviceType(deviceType)) {
        AUDIO_ERR_LOG("get deviceType failed: %{public}d", deviceType);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: The param of deviceType must be enum DeviceType");
        return volumeInDb;
    }
    if (audioSystemMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioGroupMngr_ is nullptr");
        return volumeInDb;
    }
    volumeInDb = audioSystemMngr_->GetVolumeInUnitOfDb(TaiheAudioEnum::GetNativeAudioVolumeType(volType), volLevel,
        static_cast<OHOS::AudioStandard::DeviceType>(deviceType));
    if (OHOS::AudioStandard::FLOAT_COMPARE_EQ(static_cast<float>(volumeInDb),
        static_cast<float>(OHOS::AudioStandard::ERR_INVALID_PARAM))) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "volumeInDb invalid");
        return volumeInDb;
    }
    return volumeInDb;
}

double AudioVolumeManagerImpl::GetVolumeInUnitOfDbByStream(StreamUsage streamUsage, int32_t volumeLevel,
    DeviceType device)
{
    double volumeInDb = VOLUME_DEFAULT_DOUBLE;
    int32_t streamUsageInt = streamUsage.get_value();
    if (!TaiheAudioEnum::IsLegalInputArgumentStreamUsage(streamUsageInt)) {
        AUDIO_ERR_LOG("get streamUsage failed: %{public}d", streamUsageInt);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: The param of streamUsage must be enum StreamUsage");
        return volumeInDb;
    }
    int32_t volLevel = volumeLevel;
    int32_t deviceType = device.get_value();
    if (!TaiheAudioEnum::IsLegalInputArgumentDeviceType(deviceType)) {
        AUDIO_ERR_LOG("get deviceType failed: %{public}d", deviceType);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: The param of deviceType must be enum DeviceType");
        return volumeInDb;
    }
    if (audioSystemMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioGroupMngr_ is nullptr");
        return volumeInDb;
    }
    volumeInDb = audioSystemMngr_->GetVolumeInDbByStream(TaiheAudioEnum::GetNativeStreamUsage(streamUsageInt), volLevel,
        static_cast<OHOS::AudioStandard::DeviceType>(deviceType));
    if (OHOS::AudioStandard::FLOAT_COMPARE_EQ(static_cast<float>(volumeInDb),
        static_cast<float>(OHOS::AudioStandard::ERR_PERMISSION_DENIED))) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return volumeInDb;
    }
    if (OHOS::AudioStandard::FLOAT_COMPARE_EQ(static_cast<float>(volumeInDb),
        static_cast<float>(OHOS::AudioStandard::ERR_NOT_SUPPORTED))) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "streamUsage not supported");
        return volumeInDb;
    }
    return volumeInDb;
}

array<AudioVolumeType> AudioVolumeManagerImpl::GetSupportedAudioVolumeTypes()
{
    std::vector<AudioVolumeType> volumeTypes;
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return array<AudioVolumeType>(volumeTypes);
    }
    if (audioSystemMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSystemMngr_ is nullptr");
        return array<AudioVolumeType>(volumeTypes);
    }
    std::vector<OHOS::AudioStandard::AudioVolumeType> volTypes = audioSystemMngr_->GetSupportedAudioVolumeTypes();
    return TaiheParamUtils::SetValueAudioVolumeTypeArray(volTypes);
}

AudioVolumeType AudioVolumeManagerImpl::GetAudioVolumeTypeByStreamUsage(StreamUsage streamUsage)
{
    OHOS::AudioStandard::AudioStreamType volType = OHOS::AudioStandard::AudioStreamType::STREAM_DEFAULT;
    int32_t streamUsageInt = streamUsage.get_value();
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return TaiheAudioEnum::GetJsAudioVolumeType(volType);
    }
    if (!TaiheAudioEnum::IsLegalInputArgumentStreamUsage(streamUsageInt)) {
        AUDIO_ERR_LOG("get streamUsage failed: %{public}d", streamUsageInt);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: The param of streamUsage must be enum StreamUsage");
        return TaiheAudioEnum::GetJsAudioVolumeType(volType);
    }
    if (audioSystemMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSystemMngr_ is nullptr");
        return TaiheAudioEnum::GetJsAudioVolumeType(volType);
    }
    volType = audioSystemMngr_->GetAudioVolumeTypeByStreamUsage(TaiheAudioEnum::GetNativeStreamUsage(streamUsageInt));
    if (volType == OHOS::AudioStandard::AudioStreamType::STREAM_DEFAULT) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "streamUsage not supported");
        return TaiheAudioEnum::GetJsAudioVolumeType(volType);
    }
    return TaiheAudioEnum::GetJsAudioVolumeType(volType);
}

array<StreamUsage> AudioVolumeManagerImpl::GetStreamUsagesByVolumeType(AudioVolumeType volumeType)
{
    std::vector<StreamUsage> streamUsages;
    int32_t volType = volumeType.get_value();
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return array<StreamUsage>(streamUsages);
    }
    if (!TaiheAudioEnum::IsLegalInputArgumentVolType(volType)) {
        AUDIO_ERR_LOG("get volumeType failed: %{public}d", volType);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: The param of volumeType must be enum AudioVolumeType");
        return array<StreamUsage>(streamUsages);
    }
    if (audioSystemMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSystemMngr_ is nullptr");
        return array<StreamUsage>(streamUsages);
    }
    std::vector<OHOS::AudioStandard::StreamUsage> stUsages = audioSystemMngr_->GetStreamUsagesByVolumeType(
        TaiheAudioEnum::GetNativeAudioVolumeType(volType));
    return TaiheParamUtils::SetValueStreamUsageArray(stUsages);
}

bool AudioVolumeManagerImpl::IsSystemMuted(AudioVolumeType volumeType)
{
    bool isMuted = false;
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return isMuted;
    }
    int32_t volType = volumeType.get_value();
    if (!TaiheAudioEnum::IsLegalInputArgumentVolType(volType)) {
        AUDIO_ERR_LOG("get volumeType failed: %{public}d", volType);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: The param of volumeType must be enum AudioVolumeType");
        return isMuted;
    }
    if (audioSystemMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSystemMngr_ is nullptr");
        return isMuted;
    }
    isMuted = audioSystemMngr_->IsStreamMute(TaiheAudioEnum::GetNativeAudioVolumeType(volType));
    return isMuted;
}

bool AudioVolumeManagerImpl::IsSystemMutedForStream(StreamUsage streamUsage)
{
    bool isMuted = false;
    int32_t streamUsageInt = streamUsage.get_value();
    if (!TaiheAudioEnum::IsLegalInputArgumentStreamUsage(streamUsageInt)) {
        AUDIO_ERR_LOG("get streamUsage failed: %{public}d", streamUsageInt);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: The param of streamUsage must be enum StreamUsage");
        return isMuted;
    }
    if (audioSystemMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSystemMngr_ is nullptr");
        return isMuted;
    }
    int32_t status = audioSystemMngr_->IsStreamMuteByUsage(
        TaiheAudioEnum::GetNativeStreamUsage(streamUsageInt), isMuted);
    if (status == OHOS::AudioStandard::ERR_PERMISSION_DENIED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return isMuted;
    }
    if (status == OHOS::AudioStandard::ERR_NOT_SUPPORTED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERROR_INVALID_PARAM, "No system permission");
        return isMuted;
    }
    return isMuted;
}

int32_t AudioVolumeManagerImpl::GetSystemVolumeByUid(AudioVolumeType volumeType, int32_t callingUid)
{
    int32_t volLevel = 0;
    int32_t volType = volumeType.get_value();
    if (!TaiheAudioEnum::IsLegalInputArgumentVolType(volType)) {
        AUDIO_ERR_LOG("get volType failed: %{public}d", volType);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERROR_INVALID_PARAM,
            "parameter verification failed: The param of volumeType must be enum AudioVolumeType");
        return volLevel;
    }
    if (callingUid < 0) {
        AUDIO_ERR_LOG("get uid failed: %{public}d", callingUid);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERROR_INVALID_PARAM,
            "parameter verification failed: The param of volumeType must be greater than zero");
        return volLevel;
    }
    if (audioSystemMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSystemMngr_ is nullptr");
        return volLevel;
    }
    volLevel = audioSystemMngr_->GetVolume(TaiheAudioEnum::GetNativeAudioVolumeType(volType), callingUid);
    return volLevel;
}

void AudioVolumeManagerImpl::SetSystemVolumeByUidSync(AudioVolumeType volumeType, int32_t volume, int32_t callingUid)
{
    int32_t volType = volumeType.get_value();
    if (!TaiheAudioEnum::IsLegalInputArgumentVolType(volType)) {
        AUDIO_ERR_LOG("get volumeType failed: %{public}d", volType);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERROR_INVALID_PARAM,
            "parameter verification failed: The param of volumeType must be enum AudioVolumeType");
        return;
    }
    if (audioSystemMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSystemMngr_ is nullptr");
        return;
    }
    int32_t ret = audioSystemMngr_->SetVolume(TaiheAudioEnum::GetNativeAudioVolumeType(volType), volume, callingUid);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        AUDIO_ERR_LOG("SetSystemVolumeByUid failed: %{public}d", ret);
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM);
        return;
    }
}

void AudioVolumeManagerImpl::ForceVolumeKeyControlType(AudioVolumeType volumeType, int32_t duration)
{
    int32_t volType = volumeType.get_value();
    if (audioSystemMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioSystemMngr_ is nullptr");
        return;
    }
    int32_t ret = audioSystemMngr_->ForceVolumeKeyControlType(
        TaiheAudioEnum::GetNativeAudioVolumeType(volType), duration);
    CHECK_AND_RETURN(ret != OHOS::AudioStandard::SUCCESS);
    if (ret == OHOS::AudioStandard::ERR_PERMISSION_DENIED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_PERMISSION);
        return;
    } else if (ret == OHOS::AudioStandard::ERR_SYSTEM_PERMISSION_DENIED) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED);
        return;
    } else if (ret == OHOS::AudioStandard::ERR_INVALID_PARAM) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM);
        return;
    } else {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM);
        return;
    }
}

AudioVolumeManager AudioVolumeManagerImpl::CreateVolumeManagerWrapper()
{
    std::shared_ptr<AudioVolumeManagerImpl> audioVolMngrImpl = std::make_shared<AudioVolumeManagerImpl>();
    if (audioVolMngrImpl != nullptr) {
        audioVolMngrImpl->audioSystemMngr_ = OHOS::AudioStandard::AudioSystemManager::GetInstance();
        audioVolMngrImpl->cachedClientId_ = getpid();
        return make_holder<AudioVolumeManagerImpl, AudioVolumeManager>(audioVolMngrImpl);
    }
    TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioVolMngrImpl is nullptr");
    return make_holder<AudioVolumeManagerImpl, AudioVolumeManager>(nullptr);
}

AudioVolumeGroupManager AudioVolumeManagerImpl::GetVolumeGroupManagerSync(int32_t groupId)
{
    return AudioVolumeGroupManagerImpl::CreateAudioVolumeGroupManagerWrapper(groupId);
}

void AudioVolumeManagerImpl::RegisterCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioVolumeManagerImpl *audioVolMngrImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioVolMngrImpl is nullptr");
    std::lock_guard<std::mutex> lock(audioVolMngrImpl->mutex_);
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl->audioSystemMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioSystemMngr_ is nullptr");
    if (audioVolMngrImpl->volumeKeyEventCallbackTaihe_ == nullptr) {
        audioVolMngrImpl->volumeKeyEventCallbackTaihe_ = std::make_shared<TaiheAudioVolumeKeyEvent>();
        int32_t ret =
            audioVolMngrImpl->audioSystemMngr_->RegisterVolumeKeyEventCallback(audioVolMngrImpl->cachedClientId_,
                audioVolMngrImpl->volumeKeyEventCallbackTaihe_);
        audioVolMngrImpl->volumeKeyEventCallbackTaiheList_.push_back(
            std::static_pointer_cast<TaiheAudioVolumeKeyEvent>(audioVolMngrImpl->volumeKeyEventCallbackTaihe_));
        if (ret) {
            AUDIO_ERR_LOG("RegisterVolumeKeyEventCallback Failed");
        }
    }
    std::shared_ptr<TaiheAudioVolumeKeyEvent> cb =
        std::static_pointer_cast<TaiheAudioVolumeKeyEvent>(audioVolMngrImpl->volumeKeyEventCallbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->SaveCallbackReference(cbName, callback);
}

void AudioVolumeManagerImpl::RegisterAppVolumeChangeForUidCallback(int32_t appUid, std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioVolumeManagerImpl *audioVolMngrImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioVolMngrImpl is nullptr");
    std::lock_guard<std::mutex> lock(audioVolMngrImpl->mutex_);
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl->audioSystemMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioSystemMngr_ is nullptr");
    if (audioVolMngrImpl->appVolumeChangeCallbackForUidTaihe_ == nullptr) {
        audioVolMngrImpl->appVolumeChangeCallbackForUidTaihe_ =
            std::make_shared<TaiheAudioManagerAppVolumeChangeCallback>();
    }
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl->appVolumeChangeCallbackForUidTaihe_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "System error"),
        "RegisterAppVolumeChangeForUidCallback: Memory Allocation Failed !");
    int32_t ret = audioVolMngrImpl->audioSystemMngr_->SetAppVolumeCallbackForUid(appUid,
        audioVolMngrImpl->appVolumeChangeCallbackForUidTaihe_);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        if (ret == OHOS::AudioStandard::ERROR_INVALID_PARAM) {
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
                "Parameter verification failed. Invalid callback.");
        } else if (ret == OHOS::AudioStandard::ERR_PERMISSION_DENIED) {
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "Permission denied");
        } else {
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "System error.");
        }
    }
    std::shared_ptr<TaiheAudioManagerAppVolumeChangeCallback> cb =
        std::static_pointer_cast<TaiheAudioManagerAppVolumeChangeCallback>(
        audioVolMngrImpl->appVolumeChangeCallbackForUidTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->SaveVolumeChangeCallbackForUidReference(cbName, callback, appUid);
}

void AudioVolumeManagerImpl::RegisterSelfAppVolumeChangeCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioVolumeManagerImpl *audioVolMngrImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioVolMngrImpl is nullptr");
    std::lock_guard<std::mutex> lock(audioVolMngrImpl->mutex_);
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl->audioSystemMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioSystemMngr_ is nullptr");
    if (audioVolMngrImpl->selfAppVolumeChangeCallbackTaihe_ == nullptr) {
        audioVolMngrImpl->selfAppVolumeChangeCallbackTaihe_ =
            std::make_shared<TaiheAudioManagerAppVolumeChangeCallback>();
    }
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl->selfAppVolumeChangeCallbackTaihe_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "System error"),
        "audioVolMngrImpl: Memory Allocation Failed !");
    int32_t ret = audioVolMngrImpl->audioSystemMngr_->SetSelfAppVolumeCallback(
        audioVolMngrImpl->selfAppVolumeChangeCallbackTaihe_);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        if (ret == OHOS::AudioStandard::ERROR_INVALID_PARAM) {
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
                "Parameter verification failed. Invalid callback.");
        } else {
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "System error.");
        }
    }
    std::shared_ptr<TaiheAudioManagerAppVolumeChangeCallback> cb =
        std::static_pointer_cast<TaiheAudioManagerAppVolumeChangeCallback>(
        audioVolMngrImpl->selfAppVolumeChangeCallbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->SaveSelfVolumdChangeCallbackReference(cbName, callback);
}

void AudioVolumeManagerImpl::RegisterActiveVolumeTypeChangeCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioVolumeManagerImpl *audioVolMngrImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioVolMngrImpl is nullptr");
    std::lock_guard<std::mutex> lock(audioVolMngrImpl->mutex_);
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl->audioSystemMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioSystemMngr_ is nullptr");
    if (audioVolMngrImpl->activeVolumeTypeChangeCallbackTaihe_ == nullptr) {
        audioVolMngrImpl->activeVolumeTypeChangeCallbackTaihe_ =
            std::make_shared<TaiheAudioManagerActiveVolumeTypeChangeCallback>();
    }
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl->activeVolumeTypeChangeCallbackTaihe_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "System error"),
        "RegisterActiveVolumeTypeChangeCallback: Memory Allocation Failed !");
    int32_t ret = audioVolMngrImpl->audioSystemMngr_->SetActiveVolumeTypeCallback(
        audioVolMngrImpl->activeVolumeTypeChangeCallbackTaihe_);
    if (ret != OHOS::AudioStandard::SUCCESS) {
        if (ret == OHOS::AudioStandard::ERROR_INVALID_PARAM) {
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "Invalid parameter");
        } else if (ret == OHOS::AudioStandard::ERR_PERMISSION_DENIED) {
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "Permission denied");
        } else {
            TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "System error.");
        }
    }
    std::shared_ptr<TaiheAudioManagerActiveVolumeTypeChangeCallback> cb =
        std::static_pointer_cast<TaiheAudioManagerActiveVolumeTypeChangeCallback>(
        audioVolMngrImpl->activeVolumeTypeChangeCallbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->SaveActiveVolumeTypeChangeCallbackReference(cbName, callback);
}

void AudioVolumeManagerImpl::UnregisterCallback(std::shared_ptr<uintptr_t> &callback,
    AudioVolumeManagerImpl *audioVolMngrImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioVolMngrImpl is nullptr");
    std::lock_guard<std::mutex> lock(audioVolMngrImpl->mutex_);
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl->audioSystemMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioSystemMngr_ is nullptr");
    if (callback != nullptr) {
        std::shared_ptr<TaiheAudioVolumeKeyEvent> cb =
            GetVolumeEventTaiheCallback(callback, audioVolMngrImpl);
        CHECK_AND_RETURN_LOG(cb != nullptr, "TaiheAudioVolumeKeyEvent is nullptr");
        int32_t ret = audioVolMngrImpl->audioSystemMngr_->UnregisterVolumeKeyEventCallback(
            audioVolMngrImpl->cachedClientId_, cb);
        CHECK_AND_RETURN_LOG(ret == OHOS::AudioStandard::SUCCESS, "Unset of VolumeKeyEventCallback failed");
        audioVolMngrImpl->volumeKeyEventCallbackTaiheList_.remove(cb);
        audioVolMngrImpl->volumeKeyEventCallbackTaihe_.reset();
        audioVolMngrImpl->volumeKeyEventCallbackTaihe_ = nullptr;
    } else {
        int32_t retValue = audioVolMngrImpl->audioSystemMngr_->UnregisterVolumeKeyEventCallback(
            audioVolMngrImpl->cachedClientId_);
        CHECK_AND_RETURN_LOG(retValue == OHOS::AudioStandard::SUCCESS, "Unset of VolumeKeyEventCallback failed");
        audioVolMngrImpl->volumeKeyEventCallbackTaiheList_.clear();
        audioVolMngrImpl->volumeKeyEventCallbackTaihe_.reset();
        audioVolMngrImpl->volumeKeyEventCallbackTaihe_ = nullptr;
    }
}

void AudioVolumeManagerImpl::UnregisterAppVolumeChangeForUidCallback(std::shared_ptr<uintptr_t> &callback,
    AudioVolumeManagerImpl *audioVolMngrImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioVolMngrImpl is nullptr");
    std::lock_guard<std::mutex> lock(audioVolMngrImpl->mutex_);
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl->audioSystemMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioSystemMngr_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl->appVolumeChangeCallbackForUidTaihe_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM), "UnregisterAppVolumeChangeForUidCallback failed");
    std::shared_ptr<TaiheAudioManagerAppVolumeChangeCallback> cb =
        std::static_pointer_cast<TaiheAudioManagerAppVolumeChangeCallback>(
        audioVolMngrImpl->appVolumeChangeCallbackForUidTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    if (callback != nullptr) {
        cb->RemoveAudioVolumeChangeForUidCbRef(callback);
    }

    if (callback == nullptr || cb->GetAppVolumeChangeForUidListSize() == 0) {
        audioVolMngrImpl->audioSystemMngr_->UnsetAppVolumeCallbackForUid();
        audioVolMngrImpl->appVolumeChangeCallbackForUidTaihe_.reset();
        audioVolMngrImpl->appVolumeChangeCallbackForUidTaihe_ = nullptr;
        cb->RemoveAllAudioVolumeChangeForUidCbRef();
    }
}

void AudioVolumeManagerImpl::UnregisterSelfAppVolumeChangeCallback(std::shared_ptr<uintptr_t> &callback,
    AudioVolumeManagerImpl *audioVolMngrImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioVolMngrImpl is nullptr");
    std::lock_guard<std::mutex> lock(audioVolMngrImpl->mutex_);
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl->audioSystemMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioSystemMngr_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl->selfAppVolumeChangeCallbackTaihe_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM), "UnregisterSelfAppVolumeChangeCallback failed");
    std::shared_ptr<TaiheAudioManagerAppVolumeChangeCallback> cb =
        std::static_pointer_cast<TaiheAudioManagerAppVolumeChangeCallback>(
        audioVolMngrImpl->selfAppVolumeChangeCallbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    if (callback != nullptr) {
        cb->RemoveSelfAudioVolumeChangeCbRef(callback);
    }
    if (callback == nullptr || cb->GetSelfAppVolumeChangeListSize() == 0) {
        audioVolMngrImpl->audioSystemMngr_->UnsetSelfAppVolumeCallback();
        audioVolMngrImpl->selfAppVolumeChangeCallbackTaihe_.reset();
        audioVolMngrImpl->selfAppVolumeChangeCallbackTaihe_ = nullptr;
        cb->RemoveAllSelfAudioVolumeChangeCbRef();
    }
}

void AudioVolumeManagerImpl::UnregisterActiveVolumeTypeChangeCallback(std::shared_ptr<uintptr_t> &callback,
    AudioVolumeManagerImpl *audioVolMngrImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioVolMngrImpl is nullptr");
    std::lock_guard<std::mutex> lock(audioVolMngrImpl->mutex_);
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl->audioSystemMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioSystemMngr_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl->activeVolumeTypeChangeCallbackTaihe_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM), "unregister activeVolumeType callback failed");
    std::shared_ptr<TaiheAudioManagerActiveVolumeTypeChangeCallback> cb =
        std::static_pointer_cast<TaiheAudioManagerActiveVolumeTypeChangeCallback>(
        audioVolMngrImpl->activeVolumeTypeChangeCallbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    if (callback != nullptr) {
        cb->RemoveSelfActiveVolumeTypeChangeCbRef(callback);
    }
    if (callback == nullptr || cb->GetActiveVolumeTypeChangeListSize() == 0) {
        audioVolMngrImpl->audioSystemMngr_->UnsetActiveVolumeTypeCallback(
            audioVolMngrImpl->activeVolumeTypeChangeCallbackTaihe_);
        audioVolMngrImpl->activeVolumeTypeChangeCallbackTaihe_.reset();
        audioVolMngrImpl->activeVolumeTypeChangeCallbackTaihe_ = nullptr;
        cb->RemoveAllActiveVolumeTypeChangeCbRef();
    }
}

void AudioVolumeManagerImpl::OnVolumeChange(callback_view<void(VolumeEvent const&)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterCallback(cacheCallback, VOLUME_KEY_EVENT_CALLBACK_NAME, this);
}

void AudioVolumeManagerImpl::OffVolumeChange(optional_view<callback<void(VolumeEvent const&)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterCallback(cacheCallback, this);
}

void AudioVolumeManagerImpl::OnAppVolumeChange(callback_view<void(VolumeEvent const&)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterSelfAppVolumeChangeCallback(cacheCallback, APP_VOLUME_CHANGE_CALLBACK_NAME, this);
}

void AudioVolumeManagerImpl::OffAppVolumeChange(optional_view<callback<void(VolumeEvent const&)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterSelfAppVolumeChangeCallback(cacheCallback, this);
}

void AudioVolumeManagerImpl::OnAppVolumeChangeForUid(int32_t uid, callback_view<void(VolumeEvent const&)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterAppVolumeChangeForUidCallback(uid, cacheCallback, APP_VOLUME_CHANGE_CALLBACK_NAME_FOR_UID, this);
}

void AudioVolumeManagerImpl::OffAppVolumeChangeForUid(optional_view<callback<void(VolumeEvent const&)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterAppVolumeChangeForUidCallback(cacheCallback, this);
}

void AudioVolumeManagerImpl::OnActiveVolumeTypeChange(callback_view<void(AudioVolumeType)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterActiveVolumeTypeChangeCallback(cacheCallback, ACTIVE_VOLUME_TYPE_CHANGE_CALLBACK_NAME, this);
}

void AudioVolumeManagerImpl::OffActiveVolumeTypeChange(optional_view<callback<void(AudioVolumeType)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterActiveVolumeTypeChangeCallback(cacheCallback, this);
}

void AudioVolumeManagerImpl::OnSystemVolumeChange(callback_view<void(VolumeEvent const&)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterSystemVolumeChangeCallback(cacheCallback, AUDIO_SYSTEM_VOLUME_CHANGE_CALLBACK_NAME, this);
}

void AudioVolumeManagerImpl::RegisterSystemVolumeChangeCallback(std::shared_ptr<uintptr_t> &callback,
    const std::string &cbName, AudioVolumeManagerImpl *audioVolMngrImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioVolMngrImpl is nullptr");
    std::lock_guard<std::mutex> lock(audioVolMngrImpl->mutex_);
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl->audioSystemMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioSystemMngr_ is nullptr");
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return;
    }
    if (audioVolMngrImpl->systemVolumeChangeCallbackTaihe_ == nullptr) {
        audioVolMngrImpl->systemVolumeChangeCallbackTaihe_ = std::make_shared<
            TaiheAudioSystemVolumeChangeCallback>();
        int32_t ret = audioVolMngrImpl->audioSystemMngr_->RegisterSystemVolumeChangeCallback(
            audioVolMngrImpl->cachedClientId_, audioVolMngrImpl->systemVolumeChangeCallbackTaihe_);
        audioVolMngrImpl->systemVolumeChangeCallbackTaiheList_.push_back(
            std::static_pointer_cast<TaiheAudioSystemVolumeChangeCallback>(
                audioVolMngrImpl->systemVolumeChangeCallbackTaihe_));
        if (ret) {
            AUDIO_ERR_LOG("RegisterSystemVolumeChangeCallback Failed");
        }
    }
    std::shared_ptr<TaiheAudioSystemVolumeChangeCallback> cb =
        std::static_pointer_cast<TaiheAudioSystemVolumeChangeCallback>(
            audioVolMngrImpl->systemVolumeChangeCallbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->SaveCallbackReference(cbName, callback);
}

void AudioVolumeManagerImpl::OffSystemVolumeChange(optional_view<callback<void(VolumeEvent const&)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterSystemVolumeChangeCallback(cacheCallback, this);
}

void AudioVolumeManagerImpl::UnregisterSystemVolumeChangeCallback(std::shared_ptr<uintptr_t> &callback,
    AudioVolumeManagerImpl *audioVolMngrImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioVolMngrImpl is nullptr");
    std::lock_guard<std::mutex> lock(audioVolMngrImpl->mutex_);
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl->audioSystemMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioSystemMngr_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl->systemVolumeChangeCallbackTaihe_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM), "UnregisterSystemVolumeChangeCallback failed");
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return;
    }
    if (callback != nullptr) {
        std::shared_ptr<TaiheAudioSystemVolumeChangeCallback> cb = GetSystemVolumeChangeTaiheCallback(
            callback, audioVolMngrImpl);
        if (cb == nullptr) {
            AUDIO_ERR_LOG("TaiheAudioSystemVolumeChangeCallback is nullptr");
            return;
        }
        int32_t ret = audioVolMngrImpl->audioSystemMngr_->UnregisterSystemVolumeChangeCallback(
            audioVolMngrImpl->cachedClientId_, cb);
        if (ret != OHOS::AudioStandard::SUCCESS) {
            AUDIO_ERR_LOG("UnregisterSystemVolumeChangeCallback failed");
            return;
        }
        audioVolMngrImpl->systemVolumeChangeCallbackTaiheList_.remove(cb);
        audioVolMngrImpl->systemVolumeChangeCallbackTaihe_.reset();
        audioVolMngrImpl->systemVolumeChangeCallbackTaihe_ = nullptr;
    } else {
        int32_t result = audioVolMngrImpl->audioSystemMngr_->UnregisterSystemVolumeChangeCallback(
            audioVolMngrImpl->cachedClientId_, nullptr);
        if (result != OHOS::AudioStandard::SUCCESS) {
            AUDIO_ERR_LOG("UnregisterSystemVolumeChangeCallback failed");
            return;
        }
        audioVolMngrImpl->systemVolumeChangeCallbackTaiheList_.clear();
        audioVolMngrImpl->systemVolumeChangeCallbackTaihe_.reset();
        audioVolMngrImpl->systemVolumeChangeCallbackTaihe_ = nullptr;
    }
}

std::shared_ptr<TaiheAudioSystemVolumeChangeCallback> AudioVolumeManagerImpl::GetSystemVolumeChangeTaiheCallback(
    std::shared_ptr<uintptr_t> callback, AudioVolumeManagerImpl *audioVolMngrImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl != nullptr, nullptr, "audioVolMngrImpl is nullptr");
    std::shared_ptr<TaiheAudioSystemVolumeChangeCallback> cb = nullptr;
    for (auto &iter : audioVolMngrImpl->systemVolumeChangeCallbackTaiheList_) {
        if (iter == nullptr) {
            AUDIO_ERR_LOG("iter is null");
            continue;
        }
        if (iter->ContainSameJsCallback(callback)) {
            cb = iter;
        }
    }
    return cb;
}

void AudioVolumeManagerImpl::OnStreamVolumeChange(StreamUsage streamUsage,
    callback_view<void(StreamVolumeEvent const&)> callback)
{
    auto cacheCallback = TaiheParamUtils::TypeCallback(callback);
    RegisterStreamVolumeChangeCallback(streamUsage, cacheCallback, AUDIO_STREAM_VOLUME_CHANGE_CALLBACK_NAME, this);
}

void AudioVolumeManagerImpl::RegisterStreamVolumeChangeCallback(StreamUsage streamUsage,
    std::shared_ptr<uintptr_t> &callback, const std::string &cbName, AudioVolumeManagerImpl *audioVolMngrImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioVolMngrImpl is nullptr");
    std::lock_guard<std::mutex> lock(audioVolMngrImpl->mutex_);
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl->audioSystemMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioSystemMngr_ is nullptr");

    int32_t streamUsageInt = streamUsage.get_value();
    if (audioVolMngrImpl->streamVolumeChangeCallbackTaihe_ == nullptr) {
        audioVolMngrImpl->streamVolumeChangeCallbackTaihe_ =
            std::make_shared<TaiheAudioStreamVolumeChangeCallback>();
        int32_t ret = audioVolMngrImpl->audioSystemMngr_->RegisterStreamVolumeChangeCallback(
            audioVolMngrImpl->cachedClientId_,
            { TaiheAudioEnum::GetNativeStreamUsage(streamUsageInt) },
            audioVolMngrImpl->streamVolumeChangeCallbackTaihe_);
        audioVolMngrImpl->streamVolumeChangeCallbackTaiheList_.push_back(
            std::static_pointer_cast<TaiheAudioStreamVolumeChangeCallback>(
                audioVolMngrImpl->streamVolumeChangeCallbackTaihe_));
        if (ret) {
            AUDIO_ERR_LOG("RegisterStreamVolumeChangeCallback Failed");
        }
    }
    std::shared_ptr<TaiheAudioStreamVolumeChangeCallback> cb =
        std::static_pointer_cast<TaiheAudioStreamVolumeChangeCallback>(
            audioVolMngrImpl->streamVolumeChangeCallbackTaihe_);
    CHECK_AND_RETURN_LOG(cb != nullptr, "cb is nullptr");
    cb->SaveCallbackReference(cbName, callback);
}

void AudioVolumeManagerImpl::OffStreamVolumeChange(optional_view<callback<void(StreamVolumeEvent const&)>> callback)
{
    std::shared_ptr<uintptr_t> cacheCallback = nullptr;
    if (callback.has_value()) {
        cacheCallback = TaiheParamUtils::TypeCallback(callback.value());
    }
    UnregisterStreamVolumeChangeCallback(cacheCallback, this);
}

void AudioVolumeManagerImpl::UnregisterStreamVolumeChangeCallback(std::shared_ptr<uintptr_t> &callback,
    AudioVolumeManagerImpl *audioVolMngrImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioVolMngrImpl is nullptr");
    std::lock_guard<std::mutex> lock(audioVolMngrImpl->mutex_);
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl->audioSystemMngr_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_NO_MEMORY), "audioSystemMngr_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl->streamVolumeChangeCallbackTaihe_ != nullptr,
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM), "UnregisterStreamVolumeChangeCallback failed");

    if (callback != nullptr) {
        std::shared_ptr<TaiheAudioStreamVolumeChangeCallback> cb = GetStreamVolumeChangeTaiheCallback(
            callback, audioVolMngrImpl);
        if (cb == nullptr) {
            AUDIO_ERR_LOG("TaiheAudioStreamVolumeChangeCallback is nullptr");
            return;
        }
        int32_t ret = audioVolMngrImpl->audioSystemMngr_->UnregisterStreamVolumeChangeCallback(
            audioVolMngrImpl->cachedClientId_, cb);
        if (ret != OHOS::AudioStandard::SUCCESS) {
            AUDIO_ERR_LOG("UnregisterStreamVolumeChangeCallback failed");
            return;
        }
        audioVolMngrImpl->streamVolumeChangeCallbackTaiheList_.remove(cb);
        audioVolMngrImpl->streamVolumeChangeCallbackTaihe_.reset();
        audioVolMngrImpl->streamVolumeChangeCallbackTaihe_ = nullptr;
    } else {
        int32_t result = audioVolMngrImpl->audioSystemMngr_->UnregisterStreamVolumeChangeCallback(
            audioVolMngrImpl->cachedClientId_, nullptr);
        if (result != OHOS::AudioStandard::SUCCESS) {
            AUDIO_ERR_LOG("UnregisterStreamVolumeChangeCallback failed");
            return;
        }
        audioVolMngrImpl->streamVolumeChangeCallbackTaiheList_.clear();
        audioVolMngrImpl->streamVolumeChangeCallbackTaihe_.reset();
        audioVolMngrImpl->streamVolumeChangeCallbackTaihe_ = nullptr;
    }
}

std::shared_ptr<TaiheAudioStreamVolumeChangeCallback> AudioVolumeManagerImpl::GetStreamVolumeChangeTaiheCallback(
    std::shared_ptr<uintptr_t> callback, AudioVolumeManagerImpl *audioVolMngrImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl != nullptr, nullptr, "audioVolMngrImpl is nullptr");
    std::shared_ptr<TaiheAudioStreamVolumeChangeCallback> cb = nullptr;
    for (auto &iter : audioVolMngrImpl->streamVolumeChangeCallbackTaiheList_) {
        if (iter == nullptr) {
            AUDIO_ERR_LOG("iter is null");
            continue;
        }
        if (iter->ContainSameJsCallback(callback)) {
            cb = iter;
        }
    }
    return cb;
}

std::shared_ptr<TaiheAudioVolumeKeyEvent> AudioVolumeManagerImpl::GetVolumeEventTaiheCallback(
    std::shared_ptr<uintptr_t> callback, AudioVolumeManagerImpl *audioVolMngrImpl)
{
    CHECK_AND_RETURN_RET_LOG(audioVolMngrImpl != nullptr, nullptr, "audioVolMngrImpl is nullptr");
    std::shared_ptr<TaiheAudioVolumeKeyEvent> cb = nullptr;
    for (auto &iter : audioVolMngrImpl->volumeKeyEventCallbackTaiheList_) {
        if (iter == nullptr) {
            AUDIO_ERR_LOG("iter is null");
            continue;
        }
        if (iter->ContainSameJsCallback(callback)) {
            cb = iter;
        }
    }
    return cb;
}
} // namespace ANI::Audio
