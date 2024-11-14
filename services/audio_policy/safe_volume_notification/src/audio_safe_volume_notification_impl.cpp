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
#include "audio_log.h"
#include "os_account_manager.h"
#include "locale_config.h"

namespace OHOS {
namespace AudioStandard {
std::string AudioSafeVolumeNotificationImpl::GetStringByName(const char *name)
{
    std::string resourceContext;
    if (resourceManager_ == nullptr) {
        AUDIO_ERR_LOG("resourceManager_ is null.");
        return resourceContext;
    }

    auto ret = OHOS::Global::Resource::RState::SUCCESS;
    if (name == SAFE_VOLUME_MUSIC_TIMER_TITTLE_ID) {
        ret = resourceManager_->GetStringByName(SAFE_VOLUME_MUSIC_TIMER_TITTLE_ID, resourceContext);
        if (ret != OHOS::Global::Resource::RState::SUCCESS) {
            AUDIO_ERR_LOG("get SAFE_VOLUME_MUSIC_TIMER_TITTLE_ID failed.");
        }
    } else if (name == SAFE_VOLUME_INCREASE_VOLUME_TITTLE_ID) {
        ret = resourceManager_->GetStringByName(SAFE_VOLUME_INCREASE_VOLUME_TITTLE_ID, resourceContext);
        if (ret != OHOS::Global::Resource::RState::SUCCESS) {
            AUDIO_ERR_LOG("get SAFE_VOLUME_INCREASE_VOLUME_TITTLE_ID failed.");
        }
    } else if (name == SAFE_VOLUME_MUSIC_TIMER_TEXT_ID) {
        ret = resourceManager_->GetStringByName(SAFE_VOLUME_MUSIC_TIMER_TEXT_ID, resourceContext);
        if (ret != OHOS::Global::Resource::RState::SUCCESS) {
            AUDIO_ERR_LOG("get SAFE_VOLUME_MUSIC_TIMER_TEXT_ID failed.");
        }
    } else if (name == SAFE_VOLUME_INCREASE_VOLUME_TEXT_ID) {
        ret = resourceManager_->GetStringByName(SAFE_VOLUME_INCREASE_VOLUME_TEXT_ID, resourceContext);
        if (ret != OHOS::Global::Resource::RState::SUCCESS) {
            AUDIO_ERR_LOG("get SAFE_VOLUME_INCREASE_VOLUME_TEXT_ID failed.");
        }
    } else {
        AUDIO_ERR_LOG("resource name is error.");
        return resourceContext;
    }
    return resourceContext;
}

bool AudioSafeVolumeNotificationImpl::SetTitleAndText(int32_t notificationId,
    std::shared_ptr<Notification::NotificationNormalContent> content)
{
    if (content == nullptr) {
        AUDIO_ERR_LOG("notification normal content nullptr");
        return false;
    }

    if (resourceManager_ == nullptr) {
        AUDIO_ERR_LOG("resourceManager_ is null.");
        return false;
    }

    switch (notificationId) {
        case RESTORE_VOLUME_NOTIFICATION_ID:
            content->SetTitle(GetStringByName(SAFE_VOLUME_MUSIC_TIMER_TITTLE_ID));
            content->SetText(GetStringByName(SAFE_VOLUME_MUSIC_TIMER_TEXT_ID));
            break;
        case INCREASE_VOLUME_NOTIFICATION_ID:
            content->SetTitle(GetStringByName(SAFE_VOLUME_INCREASE_VOLUME_TITTLE_ID));
            content->SetText(GetStringByName(SAFE_VOLUME_INCREASE_VOLUME_TEXT_ID));
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
    if (resourceManager_ == nullptr) {
        AUDIO_ERR_LOG("resourceManager_ is null.");
        return buttonName;
    }

    auto ret = OHOS::Global::Resource::RState::SUCCESS;
    switch (notificationId) {
        case RESTORE_VOLUME_NOTIFICATION_ID:
            AUDIO_INFO_LOG("GetButtonName RESTORE_VOLUME_NOTIFICATION_ID.");
            ret = resourceManager_->GetStringByName(SAFE_VOLUME_RESTORE_VOL_BUTTON_ID, buttonName);
            if (ret != OHOS::Global::Resource::RState::SUCCESS) {
                AUDIO_ERR_LOG("get SAFE_VOLUME_RESTORE_VOL_BUTTON_ID failed.");
            }
            break;
        case INCREASE_VOLUME_NOTIFICATION_ID:
            AUDIO_INFO_LOG("GetButtonName INCREASE_VOLUME_NOTIFICATION_ID.");
            ret = resourceManager_->GetStringByName(SAFE_VOLUME_INCREASE_VOL_BUTTON_ID, buttonName);
            if (ret != OHOS::Global::Resource::RState::SUCCESS) {
                AUDIO_ERR_LOG("get SAFE_VOLUME_INCREASE_VOL_BUTTON_ID failed.");
            }
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

void AudioSafeVolumeNotificationImpl::RefreshResConfig()
{
    std::string language = Global::I18n::LocaleConfig::GetSystemLanguage();
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale locale = icu::Locale::forLanguageTag(language, status);
    if (status != U_ZERO_ERROR) {
        AUDIO_INFO_LOG("forLanguageTag failed, errCode:%{public}d", status);
    }
    if (resConfig_) {
        resConfig_->SetLocaleInfo(locale.getLanguage(), locale.getScript(), locale.getCountry());
    }
    if (resourceManager_) {
        resourceManager_->UpdateResConfig(*resConfig_);
    }
}

void AudioSafeVolumeNotificationImpl::Init()
{
    if (resourceManager_ == nullptr) {
        resourceManager_ = Global::Resource::GetSystemResourceManagerNoSandBox();
    }
    if (resConfig_ == nullptr) {
        resConfig_ = Global::Resource::CreateResConfig();
    }
    RefreshResConfig();
}

AudioSafeVolumeNotificationImpl::AudioSafeVolumeNotificationImpl()
{
    AUDIO_INFO_LOG("AudioSafeVolumeNotificationImpl enter.");
    Init();
}

bool AudioSafeVolumeNotificationImpl::GetPixelMap()
{
    if (iconPixelMap_ != nullptr) {
        AUDIO_ERR_LOG("icon pixel map already exists.");
        return false;
    }

    if (resourceManager_ == nullptr) {
        AUDIO_ERR_LOG("resourceManager_ is null.");
        return false;
    }

    std::unique_ptr<uint8_t[]> resourceData;
    size_t resourceDataLength = 0;
    auto ret = resourceManager_->GetMediaDataByName(SAFE_VOLUME_ICON_ID, resourceDataLength, resourceData);
    if (ret != Global::Resource::RState::SUCCESS) {
        AUDIO_ERR_LOG("get (%{public}s) failed, errorCode:%{public}d", SAFE_VOLUME_ICON_ID, static_cast<int32_t>(ret));
        return false;
    }

    Media::SourceOptions opts;
    uint32_t errorCode = 0;
    std::unique_ptr<Media::ImageSource> imageSource =
        Media::ImageSource::CreateImageSource(resourceData.get(), resourceDataLength, opts, errorCode);
    Media::DecodeOptions decodeOpts;
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

void AudioSafeVolumeNotificationImpl::PublishSafeVolumeNotification(int32_t notificationId)
{
    RefreshResConfig();

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
    int32_t AUDIO_UID = IPCSkeleton::GetCallingUid();
    Notification::NotificationRequest request;
    request.SetCreatorUid(AUDIO_UID);
    request.SetCreatorPid(getpid());

    int32_t userId;
    AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(AUDIO_UID, userId);

    request.SetCreatorUserId(userId);
    request.SetAutoDeletedTime(OHOS::Notification::NotificationConstant::INVALID_AUTO_DELETE_TIME);
    request.SetTapDismissed(false);
    request.SetSlotType(OHOS::Notification::NotificationConstant::SlotType::SOCIAL_COMMUNICATION);
    request.SetNotificationId(notificationId);
    request.SetNotificationControlFlags(NOTIFICATION_BANNER_FLAG);
    request.SetContent(content);

    GetPixelMap();
    if (iconPixelMap_ != nullptr) {
        request.SetLittleIcon(iconPixelMap_);
        request.SetBadgeIconStyle(Notification::NotificationRequest::BadgeStyle::LITTLE);
    }

    std::string buttonName = GetButtonName(notificationId);
    if (!buttonName.empty()) {
        SetActionButton(notificationId, buttonName, request);
    }

    auto ret = Notification::NotificationHelper::PublishNotification(request);
    AUDIO_INFO_LOG("safe volume service publish notification result = %{public}d", ret);
}

void AudioSafeVolumeNotificationImpl::CancelSafeVolumeNotification(int32_t notificationId)
{
    auto ret = Notification::NotificationHelper::CancelNotification(notificationId);
    AUDIO_INFO_LOG("safe volume service cancel notification result = %{public}d", ret);
}
}  // namespace AudioStandard
}  // namespace OHOS
