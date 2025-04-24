/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioSafeVolumeNotificationImpl"
#endif

#include "audio_safe_volume_notification_impl.h"

#include <map>

#include "want_agent_helper.h"
#include "want_agent_info.h"
#include "notification_helper.h"
#include "notification_request.h"
#include "notification_constant.h"
#include "notification_bundle_option.h"
#include "rstate.h"
#include "ipc_skeleton.h"
#include "audio_policy_log.h"
#include "os_account_manager.h"
#include "locale_config.h"
#include "resource_manager_adapter.h"

namespace OHOS {
namespace AudioStandard {
std::string AudioSafeVolumeNotificationImpl::GetSystemStringByName(std::string name)
{
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    std::string result = ResourceManagerAdapter::GetInstance()->GetSystemStringByName(name);
    IPCSkeleton::SetCallingIdentity(identity);
    return result;
}

bool AudioSafeVolumeNotificationImpl::SetTitleAndText(int32_t notificationId,
    std::shared_ptr<Notification::NotificationNormalContent> content)
{
    if (content == nullptr) {
        AUDIO_ERR_LOG("notification normal content nullptr");
        return false;
    }

    switch (notificationId) {
        case RESTORE_VOLUME_NOTIFICATION_ID:
            content->SetTitle(GetSystemStringByName(SAFE_VOLUME_MUSIC_TIMER_TITTLE_ID));
            content->SetText(GetSystemStringByName(SAFE_VOLUME_MUSIC_TIMER_TEXT_ID));
            break;
        case INCREASE_VOLUME_NOTIFICATION_ID:
            content->SetTitle(GetSystemStringByName(SAFE_VOLUME_INCREASE_VOLUME_TITTLE_ID));
            content->SetText(GetSystemStringByName(SAFE_VOLUME_INCREASE_VOLUME_TEXT_ID));
            break;
        default:
            AUDIO_ERR_LOG("error notificationId");
            return false;
    }
    return true;
}

std::string AudioSafeVolumeNotificationImpl::GetButtonName(uint32_t notificationId)
{
    std::string buttonName;

    switch (notificationId) {
        case RESTORE_VOLUME_NOTIFICATION_ID:
            AUDIO_INFO_LOG("GetButtonName RESTORE_VOLUME_NOTIFICATION_ID.");
            buttonName = GetSystemStringByName(SAFE_VOLUME_RESTORE_VOL_BUTTON_ID);
            break;
        case INCREASE_VOLUME_NOTIFICATION_ID:
            AUDIO_INFO_LOG("GetButtonName INCREASE_VOLUME_NOTIFICATION_ID.");
            buttonName = GetSystemStringByName(SAFE_VOLUME_INCREASE_VOL_BUTTON_ID);
            break;
        default:
            AUDIO_ERR_LOG("resource name is error.");
            return buttonName;
    }
    return buttonName;
}

static void SetActionButton(int32_t notificationId, const std::string& buttonName,
    Notification::NotificationRequest& request)
{
    auto want = std::make_shared<AAFwk::Want>();
    if (notificationId == RESTORE_VOLUME_NOTIFICATION_ID) {
        AUDIO_INFO_LOG("SetActionButton AUDIO_RESTORE_VOLUME_EVENT.");
        want->SetAction(AUDIO_RESTORE_VOLUME_EVENT);
    } else {
        AUDIO_INFO_LOG("SetActionButton AUDIO_INCREASE_VOLUME_EVENT.");
        want->SetAction(AUDIO_INCREASE_VOLUME_EVENT);
    }
    std::vector<std::shared_ptr<AAFwk::Want>> wants;
    wants.push_back(want);
    std::vector<AbilityRuntime::WantAgent::WantAgentConstant::Flags> flags;
    flags.push_back(AbilityRuntime::WantAgent::WantAgentConstant::Flags::CONSTANT_FLAG);
    AbilityRuntime::WantAgent::WantAgentInfo wantAgentInfo(
        0, AbilityRuntime::WantAgent::WantAgentConstant::OperationType::SEND_COMMON_EVENT,
        flags, wants, nullptr
    );
    auto wantAgentDeal = AbilityRuntime::WantAgent::WantAgentHelper::GetWantAgent(wantAgentInfo);
    std::shared_ptr<Notification::NotificationActionButton> actionButtonDeal =
        Notification::NotificationActionButton::Create(nullptr, buttonName, wantAgentDeal);
    if (actionButtonDeal == nullptr) {
        AUDIO_ERR_LOG("get notification actionButton nullptr");
        return;
    }
    AUDIO_INFO_LOG("SetActionButton AddActionButton.");
    request.AddActionButton(actionButtonDeal);
}

static void SetExtraParam(int32_t notificationId, Notification::NotificationRequest& request)
{
    std::shared_ptr<AAFwk::WantParams> wantParams = std::make_shared<AAFwk::WantParams>();
    if (notificationId == RESTORE_VOLUME_NOTIFICATION_ID) {
        AUDIO_INFO_LOG("SetExtraParam AUDIO_RESTORE_VOLUME_EVENT.");
        wantParams->SetParam(SAVE_VOLUME_SYS_ABILITY_NAME, AAFwk::String::Box(AUDIO_RESTORE_VOLUME_EVENT));
    } else {
        AUDIO_INFO_LOG("SetExtraParam AUDIO_INCREASE_VOLUME_EVENT.");
        wantParams->SetParam(SAVE_VOLUME_SYS_ABILITY_NAME, AAFwk::String::Box(AUDIO_INCREASE_VOLUME_EVENT));
    }

    request.SetAdditionalData(wantParams);
}

bool AudioSafeVolumeNotificationImpl::GetPixelMap()
{
    if (iconPixelMap_ != nullptr) {
        AUDIO_ERR_LOG("icon pixel map already exists.");
        return false;
    }

    std::unique_ptr<uint8_t[]> resourceData;
    size_t resourceDataLength = 0;
    auto ret = GetMediaDataByName(SAFE_VOLUME_ICON_ID, resourceDataLength, resourceData);
    if (ret != Global::Resource::RState::SUCCESS) {
        AUDIO_ERR_LOG("get (%{public}s) failed, errorCode:%{public}d", SAFE_VOLUME_ICON_ID.c_str(),
            static_cast<int32_t>(ret));
        return false;
    }

    Media::SourceOptions opts;
    uint32_t errorCode = 0;
    std::unique_ptr<Media::ImageSource> imageSource =
        Media::ImageSource::CreateImageSource(resourceData.get(), resourceDataLength, opts, errorCode);
    Media::DecodeOptions decodeOpts;
    decodeOpts.desiredSize = {ICON_WIDTH, ICON_HEIGHT};
    decodeOpts.desiredPixelFormat = Media::PixelFormat::BGRA_8888;
    if (imageSource) {
        AUDIO_INFO_LOG("GetPixelMap SUCCESS.");
        auto pixelMapPtr = imageSource->CreatePixelMap(decodeOpts, errorCode);
        iconPixelMap_ = std::shared_ptr<Media::PixelMap>(pixelMapPtr.release());
    }
    if (errorCode != 0 || (iconPixelMap_ == nullptr)) {
        AUDIO_ERR_LOG("get badge failed, errorCode:%{public}u", errorCode);
        return false;
    }
    return true;
}

static void SetBasicOption(int32_t notificationId, Notification::NotificationRequest &request)
{
    request.SetCreatorUid(SAVE_VOLUME_SYS_ABILITY_ID);
    Notification::NotificationBundleOption bundle(SAVE_VOLUME_SYS_ABILITY_NAME, SAVE_VOLUME_SYS_ABILITY_ID);
    Notification::NotificationHelper::SetNotificationSlotFlagsAsBundle(bundle, NOTIFICATION_BANNER_FLAG);
    int32_t userId;
    AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(SAVE_VOLUME_SYS_ABILITY_ID, userId);
    request.SetCreatorUserId(userId);
    request.SetCreatorPid(getpid());
    request.SetAutoDeletedTime(OHOS::Notification::NotificationConstant::INVALID_AUTO_DELETE_TIME);
    request.SetTapDismissed(false);
    request.SetSlotType(OHOS::Notification::NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    if (notificationId == RESTORE_VOLUME_NOTIFICATION_ID) {
        request.SetNotificationControlFlags(NOTIFICATION_BANNER_FLAG);
    } else {
        request.SetNotificationControlFlags(NOTIFICATION_BANNER_FLAG | NOTIFICATION_CLOSE_SOUND_FLAG);
    }
}

void AudioSafeVolumeNotificationImpl::PublishSafeVolumeNotification(int32_t notificationId)
{
    std::shared_ptr<Notification::NotificationNormalContent> normalContent =
        std::make_shared<Notification::NotificationNormalContent>();
    if (normalContent == nullptr) {
        AUDIO_ERR_LOG("get notification normal content nullptr");
        return;
    }

    if (!SetTitleAndText(notificationId, normalContent)) {
        AUDIO_ERR_LOG("error setting title and text");
        return;
    }

    std::shared_ptr<Notification::NotificationContent> content =
        std::make_shared<Notification::NotificationContent>(normalContent);

    if (content == nullptr) {
        AUDIO_ERR_LOG("get notification content nullptr");
        return;
    }

    Notification::NotificationRequest request;
    SetBasicOption(notificationId, request);
    request.SetContent(content);
    request.SetNotificationId(notificationId);
    request.SetIsDoNotDisturbByPassed(true);
    GetPixelMap();
    if (iconPixelMap_ != nullptr) {
        request.SetLittleIcon(iconPixelMap_);
        request.SetBadgeIconStyle(Notification::NotificationRequest::BadgeStyle::LITTLE);
    }

    std::string buttonName = GetButtonName(notificationId);
    if (!buttonName.empty()) {
        SetActionButton(notificationId, buttonName, request);
    }

    SetExtraParam(notificationId, request);

    auto ret = Notification::NotificationHelper::PublishNotification(request);
    AUDIO_INFO_LOG("safe volume service publish notification result = %{public}d", ret);
}

void AudioSafeVolumeNotificationImpl::CancelSafeVolumeNotification(int32_t notificationId)
{
    auto ret = Notification::NotificationHelper::CancelNotification(notificationId);
    AUDIO_INFO_LOG("safe volume service cancel notification result = %{public}d", ret);

    std::string identity = IPCSkeleton::ResetCallingIdentity();
    ResourceManagerAdapter::GetInstance()->ReleaseSystemResourceManager();
    IPCSkeleton::SetCallingIdentity(identity);
}

Global::Resource::RState AudioSafeVolumeNotificationImpl::GetMediaDataByName(const std::string& name, size_t& len,
    std::unique_ptr<uint8_t[]>& outValue, uint32_t density)
{
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    Global::Resource::RState rstate =
        ResourceManagerAdapter::GetInstance()->GetMediaDataByName(name, len, outValue, density);
    IPCSkeleton::SetCallingIdentity(identity);
    return rstate;
}
}  // namespace AudioStandard
}  // namespace OHOS
