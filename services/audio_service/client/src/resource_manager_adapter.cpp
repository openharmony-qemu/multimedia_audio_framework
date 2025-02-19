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
#ifndef LOG_TAG
#define LOG_TAG "ResourceManagerAdapter"
#endif

#include "resource_manager_adapter.h"

#include "locale_config.h"
#include "audio_service_log.h"

namespace OHOS {
namespace AudioStandard {
using namespace Global::Resource;

ResourceManagerAdapter::ResourceManagerAdapter()
{
    AUDIO_INFO_LOG("construct");
}

ResourceManagerAdapter::~ResourceManagerAdapter()
{
    AUDIO_INFO_LOG("destroy");
}

ResourceManagerAdapter *ResourceManagerAdapter::GetInstance()
{
    static ResourceManagerAdapter resourceManagerAdapter;
    return &resourceManagerAdapter;
}

/**
 * InitResourceManager must be called in resourceManagerMutex_ lock scope
 */
void ResourceManagerAdapter::InitResourceManager()
{
    if (!resourceManager_) {
        AUDIO_INFO_LOG("Init");
        resourceManager_ = Global::Resource::GetSystemResourceManagerNoSandBox();
    }
    if (!resConfig_) {
        resConfig_ = Global::Resource::CreateResConfig();
    }
    RefreshResConfig();
}

void ResourceManagerAdapter::RefreshResConfig()
{
    std::string language = Global::I18n::LocaleConfig::GetSystemLanguage();
    if (language.empty()) {
        AUDIO_ERR_LOG("Get system language failed, skip RefreshResConfig");
        return;
    }
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale locale = icu::Locale::forLanguageTag(language, status);
    if (status != U_ZERO_ERROR || locale == nullptr) {
        AUDIO_ERR_LOG("forLanguageTag failed, errCode:%{public}d", status);
        return;
    }
    if (!resConfig_) {
        AUDIO_ERR_LOG("resConfig_ is nullptr");
        return;
    }
    resConfig_->SetLocaleInfo(locale.getLanguage(), locale.getScript(), locale.getCountry());
    if (!resourceManager_) {
        AUDIO_ERR_LOG("resourceManager_ is nullptr");
        return;
    }
    resourceManager_->UpdateResConfig(*resConfig_);
    AUDIO_INFO_LOG("Refresh success");
}

void ResourceManagerAdapter::ReleaseSystemResourceManager()
{
    std::lock_guard<std::mutex> lock(resourceManagerMutex_);
    if (resourceManager_) {
        Global::Resource::ReleaseSystemResourceManager();
        resourceManager_ = nullptr;
    }
    
    if (resConfig_) {
        delete resConfig_;
        resConfig_ = nullptr;
    }
    AUDIO_INFO_LOG("Release success");
}

std::string ResourceManagerAdapter::GetSystemStringByName(std::string name)
{
    std::lock_guard<std::mutex> lock(resourceManagerMutex_);
    InitResourceManager();

    std::string result;
    if (!resourceManager_) {
        AUDIO_ERR_LOG("resourceManager_ is nullptr");
        return result;
    }

    Global::Resource::RState rstate = resourceManager_->GetStringByName(name.c_str(), result);
    AUDIO_INFO_LOG("name: %{public}s, rstate: %{public}d", name.c_str(), static_cast<int32_t>(rstate));
    return result;
}

Global::Resource::RState ResourceManagerAdapter::GetMediaDataByName(std::string name, size_t &len,
    std::unique_ptr<uint8_t[]> &outValue, uint32_t density)
{
    std::lock_guard<std::mutex> lock(resourceManagerMutex_);
    InitResourceManager();

    if (!resourceManager_) {
        AUDIO_ERR_LOG("resourceManager_ is nullptr");
        return Global::Resource::RState::ERROR;
    }

    Global::Resource::RState rstate = resourceManager_->GetMediaDataByName(name.c_str(), len, outValue);
    AUDIO_INFO_LOG("rstate: %{public}d", static_cast<int32_t>(rstate));
    return rstate;
}

} // namespace AudioStandard
} // namespace OHOS
