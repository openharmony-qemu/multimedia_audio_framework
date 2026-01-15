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
#define LOG_TAG "AudioBusSelector"
#endif

#include "audio_bus_selector.h"
#include "audio_errors.h"
#include "audio_module_info.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002B84

namespace OHOS {
namespace AudioStandard {

namespace {
constexpr std::string_view PRIMARY_ZONE_NAME = "primary";
constexpr std::string_view PRIMARY_DEFAULT_BUS = "bus0_media_out";
}
AudioBusSelector &AudioBusSelector::GetBusSelector() noexcept
{
    static AudioBusSelector instance(AudioZoneService::GetInstance(), AudioPipeManager::GetPipeManager(),
                                    AudioPolicyConfigManager::GetInstance());
    return instance;
}

int32_t AudioBusSelector::SetCustomAudioMix(const std::string &zoneName, const std::vector<AudioMix> &audioMixes)
{
    std::lock_guard<std::mutex> lock(audioMixMutex_);
    AUDIO_INFO_LOG("Entered %{public}s, zoneName: %{public}s, audioMix size: %{public}zu", __func__, zoneName.c_str(),
                   audioMixes.size());
    CHECK_AND_RETURN_RET_LOG(zoneName != "" && audioMixes.size() > 0, ERR_INVALID_PARAM,
                             "zoneName is empty or audioMix is empty.");
    audioMixMap_[zoneName] = audioMixes;
    return SUCCESS;
}

std::vector<std::string> AudioBusSelector::GetBusAddressesByStreamDesc(
    const std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    if (!streamDesc) {
        AUDIO_ERR_LOG("streamDesc is null, cannot retrieve bus addresses.");
        return {};
    }

    std::string zoneName = audioZoneService_.FindAudioZoneNameByUid(streamDesc->callerUid_);
    if (zoneName.empty()) {
        zoneName = std::string(PRIMARY_ZONE_NAME);
    }

    std::lock_guard<std::mutex> lock(audioMixMutex_);
    auto iter = audioMixMap_.find(zoneName);
    CHECK_AND_RETURN_RET(iter != audioMixMap_.end(), {});

    std::vector<std::string> busAddresses;
    for (const auto &audioMix : iter->second) {
        if (audioMix.encodingType == streamDesc->rendererInfo_.encodingType &&
            std::any_of(
                audioMix.streamUsages.begin(), audioMix.streamUsages.end(),
                [&streamDesc](const StreamUsage &usage) { return usage == streamDesc->rendererInfo_.streamUsage; })) {
            busAddresses.push_back(audioMix.busAddress);
        }
    }
    if (busAddresses.empty()) {
        std::string defaultBus = GetDefaultBusByConfig();
        if (defaultBus.empty()) {
            defaultBus = std::string(PRIMARY_DEFAULT_BUS);
        }
        AUDIO_WARNING_LOG("Not match any bus, use default bus%{public}s, for stream usage: %{public}d",
                          defaultBus.c_str(), streamDesc->rendererInfo_.streamUsage);
        return {defaultBus};
    }
    std::string busAddressesStr =
        std::reduce(busAddresses.begin(), busAddresses.end(), std::string{},
                    [](const std::string &a, const std::string &b) { return a.empty() ? b : a + ", " + b; });
    AUDIO_INFO_LOG("Bus Addresses: %{public}s", busAddressesStr.c_str());
    return busAddresses;
}

std::string AudioBusSelector::GetDefaultBusByConfig()
{
    PolicyGlobalConfigs globalConfigs;
    audioConfigManager_.GetGlobalConfigs(globalConfigs);
    if (globalConfigs.adapter_.empty() || globalConfigs.pipe_.empty()) {
        return "";
    }

    AudioAdapterType adpaterType = PolicyAdapterInfo::GetAdapterType(globalConfigs.adapter_);
    std::shared_ptr<PolicyAdapterInfo> adapterInfo{nullptr};
    bool ret = audioConfigManager_.GetAdapterInfoByType(adapterType, adapterInfo);
    if (!ret || !adapterInfo) {
        AUDIO_ERR_LOG("Can not find adapter info for default bus.");
        return "";
    }

    std::shared_ptr<AdapterPipeInfo> pipeInfo = adapterInfo->GetPipeInfoByName(globalConfigs.pipe_);
    if (pipeInfo == nullptr) {
        AUDIO_ERR_LOG("Can not find pipe info for default bus.");
        return "";
    }
    return pipeInfo->paProp_.busAddress_;
}

std::string AudioBusSelector::GetSinkNameByStreamId(int32_t streamId)
{
    const auto &pipeInfoList = audioPipeManager_->GetPipeList();
    for (const auto &pipeInfo : pipeInfoList) {
        bool matchStream =
            std::any_of(pipeInfo->streamDescriptors_.begin(), pipeInfo->streamDescriptors_.end(),
                        [streamId](const auto &desc) { return static_cast<int32_t>(desc->sessionId_) == streamId; });
        CHECK_AND_RETURN_RET(!matchStream, pipeInfo->moduleInfo_.name);
    }
    return PORT_NONE;
}

}
}