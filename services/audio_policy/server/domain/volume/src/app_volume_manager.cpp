/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "AppVolumeManager"
#endif

#include "app_volume_manager.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_policy_log.h"
#include "audio_inner_call.h"
#include "audio_volume_utils.h"
#include "audio_policy_server_handler.h"
#include "audio_policy_manager_factory.h"

namespace OHOS {
namespace AudioStandard {

class FlushSystemAppVolumeAction : public AsyncActionHandler::AsyncAction {
public:
    explicit FlushSystemAppVolumeAction(AppVolumeManager& appVolumeManager) : appVolumeManager_(appVolumeManager)
    {}

    void Exec() override
    {
        appVolumeManager_.FlushSystemAppVolume();
    }

private:
    AppVolumeManager& appVolumeManager_;
};

constexpr uint32_t SYSTEM_APP_VOLUME_PERSIST_DELAY_MS = 1000; // 1s
constexpr int32_t VOLUME_PERCENTAGE_UNSET = -1;

AppVolumeManager::AppVolumeManager()
    : audioPolicyManager_(AudioPolicyManagerFactory::GetAudioPolicyManager()),
      audioPolicyServerHandler_(DelayedSingleton<AudioPolicyServerHandler>::GetInstance()),
      asyncHandler_(nullptr)
{
}

AppVolumeManager::~AppVolumeManager()
{
}

void AppVolumeManager::SetAsyncActionHandler(std::shared_ptr<AsyncActionHandler> asyncHandler)
{
    asyncHandler_ = asyncHandler;
}

int32_t AppVolumeManager::GetAppVolumeLevel(int32_t appUid, int32_t &volumeLevel)
{
    return audioPolicyManager_.GetAppVolumeLevel(appUid, volumeLevel);
}

int32_t AppVolumeManager::SetAppVolumeLevel(int32_t appUid, int32_t volumeLevel)
{
    AUDIO_INFO_LOG("enter AppVolumeManager::SetAppVolumeLevel");
    // audioPolicyManager_ : AudioAdapterManager
    return audioPolicyManager_.SetAppVolumeLevel(appUid, volumeLevel);
}

int32_t AppVolumeManager::SetSelfAppVolumeLevelWithCallback(int32_t appUid, int32_t volumeLevel, bool isUpdateUi)
{
    AUDIO_INFO_LOG("appUid: %{public}d, volumeLevel: %{public}d, updateUi: %{public}d",
        appUid, volumeLevel, isUpdateUi);
    int32_t ret = audioPolicyManager_.SetAppVolumeLevel(appUid, volumeLevel);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Fail to set App Volume level");

    VolumeEvent volumeEvent;
    volumeEvent.volumeType = STREAM_APP;
    volumeEvent.volume = volumeLevel;
    volumeEvent.updateUi = isUpdateUi;
    volumeEvent.volumeGroupId = 0;
    volumeEvent.networkId = LOCAL_NETWORK_ID;
    volumeEvent.volumeMode = AUDIOSTREAM_VOLUMEMODE_APP_INDIVIDUAL;
    CHECK_AND_RETURN_RET_LOG(audioPolicyServerHandler_ != nullptr, ERROR, "audioPolicyServerHandler_ is null");
    audioPolicyServerHandler_->SendSelfAppVolumeChangeCallback(appUid, volumeEvent);
    return ret;
}

int32_t AppVolumeManager::SetAppVolumeMuted(int32_t appUid, bool muted)
{
    AUDIO_INFO_LOG("enter AppVolumeManager::SetAppVolumeMuted");
    return audioPolicyManager_.SetAppVolumeMuted(appUid, muted);
}

int32_t AppVolumeManager::IsAppVolumeMute(int32_t appUid, bool owned, bool &isMute)
{
    AUDIO_INFO_LOG("enter AppVolumeManager::IsAppVolumeMute");
    return audioPolicyManager_.IsAppVolumeMute(appUid, owned, isMute);
}

int32_t AppVolumeManager::SetSystemAppVolumePercentage(int32_t appUid, int32_t volumePercentage)
{
    // For system-controlled app volume, the volumeLevel range is 0-100.
    int32_t volumeLevel = volumePercentage;
    int32_t ret = audioPolicyManager_.SetSystemAppVolumeLevel(appUid, volumeLevel);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Set system app volume level fail");
    systemAppVolumeDirty_ = true;
    ScheduleSystemAppVolumePersist();
    return SUCCESS;
}

void AppVolumeManager::ScheduleSystemAppVolumePersist()
{
    if (systemAppVolumeFlushScheduled_.exchange(true)) {
        return;
    }

    auto action = std::make_shared<FlushSystemAppVolumeAction>(*this);
    AsyncActionHandler::AsyncActionDesc desc;
    desc.action = std::static_pointer_cast<AsyncActionHandler::AsyncAction>(action);
    desc.delayTimeMs = SYSTEM_APP_VOLUME_PERSIST_DELAY_MS;
    CHECK_AND_CALL_FUNC_RETURN_LOG(asyncHandler_ != nullptr,
        systemAppVolumeFlushScheduled_ = false, "asyncHandler_ is nullptr");
    asyncHandler_->PostAsyncAction(desc);
}

void AppVolumeManager::FlushSystemAppVolume()
{
    if (systemAppVolumeDirty_) {
        audioPolicyManager_.SaveSystemAppVolumeLevelToDb();
        audioPolicyManager_.SaveSystemAppVolumeMuteStatusToDb();
        systemAppVolumeDirty_ = false;
    }
    systemAppVolumeFlushScheduled_ = false;
}

int32_t AppVolumeManager::GetSystemAppVolumePercentage(int32_t appUid, int32_t &volumePercentage)
{
    int32_t volumeLevel = 0;
    int32_t ret = audioPolicyManager_.GetSystemAppVolumeLevel(appUid, volumeLevel);
    // For system-controlled app volume, the volumeLevel range is 0-100.
    volumePercentage = volumeLevel;
    return ret;
}

int32_t AppVolumeManager::SetSystemAppVolumeMuted(int32_t appUid, bool muted)
{
    int32_t ret = audioPolicyManager_.SetSystemAppVolumeMuted(appUid, muted);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Set system app mute status fail");
    audioPolicyManager_.SaveSystemAppVolumeMuteStatusToDb(true);
    return ret;
}

int32_t AppVolumeManager::IsSystemAppVolumeMuted(int32_t appUid, bool &isMute)
{
    return audioPolicyManager_.IsSystemAppVolumeMuted(appUid, isMute);
}

int32_t AppVolumeManager::SetSystemAppVolumeMutedForUid(int32_t appUid, bool muted)
{
    bool appliedSystemAppMuteStatus = false;
    IsSystemAppVolumeMuted(appUid, appliedSystemAppMuteStatus);
    AUDIO_INFO_LOG("appUid: %{public}d, new mute: %{public}d, old mute: %{public}d",
        appUid, muted, appliedSystemAppMuteStatus);
    // Skip setting if mute status is already up-to-date
    CHECK_AND_RETURN_RET(appliedSystemAppMuteStatus != muted, SUCCESS);

    int32_t ret = SetSystemAppVolumeMuted(appUid, muted);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Fail to set systemAppVolume mute");

    VolumeEvent volumeEvent;
    volumeEvent.volumeType = VolumeUtils::IsPCVolumeEnable() ? STREAM_ALL : STREAM_MUSIC;
    int32_t volumePercentage = 0;
    GetSystemAppVolumePercentage(appUid, volumePercentage);
    volumeEvent.volume = muted ? 0 : volumePercentage;
    volumeEvent.updateUi = true;
    volumeEvent.volumeGroupId = 0;
    volumeEvent.networkId = LOCAL_NETWORK_ID;
    volumeEvent.volumeMode = AUDIOSTREAM_VOLUMEMODE_APP_INDIVIDUAL;
    volumeEvent.appUid = appUid;
    CHECK_AND_RETURN_RET_LOG(audioPolicyServerHandler_ != nullptr, ERROR, "audioPolicyServerHandler_ is null");
    audioPolicyServerHandler_->SendSystemAppVolumeChangeCallback(appUid, volumeEvent);
    return ret;
}

int32_t AppVolumeManager::SetSystemAppVolumePercentageWithCallback(int32_t appUid, int32_t volumePercentage)
{
    int32_t appliedVolumePercentage = VOLUME_PERCENTAGE_UNSET;
    GetSystemAppVolumePercentage(appUid, appliedVolumePercentage);
    AUDIO_INFO_LOG("appUid: %{public}d, new percentage: %{public}d, old percentage: %{public}d",
        appUid, volumePercentage, appliedVolumePercentage);
    // Skip setting if volume percentage is already up-to-date
    CHECK_AND_RETURN_RET(appliedVolumePercentage != volumePercentage, SUCCESS);

    int32_t ret = SetSystemAppVolumePercentage(appUid, volumePercentage);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Fail to set system app volume percentage");

    VolumeEvent volumeEvent;
    volumeEvent.volumeType = VolumeUtils::IsPCVolumeEnable() ? STREAM_ALL : STREAM_MUSIC;
    volumeEvent.volume = volumePercentage;
    volumeEvent.updateUi = true;
    volumeEvent.volumeGroupId = 0;
    volumeEvent.networkId = LOCAL_NETWORK_ID;
    volumeEvent.volumeMode = AUDIOSTREAM_VOLUMEMODE_APP_INDIVIDUAL;
    volumeEvent.appUid = appUid;
    CHECK_AND_RETURN_RET_LOG(audioPolicyServerHandler_ != nullptr, ERROR, "audioPolicyServerHandler_ is null");
    audioPolicyServerHandler_->SendSystemAppVolumeChangeCallback(appUid, volumeEvent);
    return ret;
}

int32_t AppVolumeManager::SetAppRingMuted(int32_t appUid, bool muted)
{
    AUDIO_INFO_LOG("enter AppVolumeManager::SetAppRingMuted");
    return audioPolicyManager_.SetAppRingMuted(appUid, muted);
}

bool AppVolumeManager::IsAppRingMuted(int32_t appUid)
{
    AUDIO_INFO_LOG("enter AppVolumeManager::IsAppRingMuted");
    return audioPolicyManager_.IsAppRingMuted(appUid);
}

}
}