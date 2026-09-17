/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioPolicyManagerZone"
#endif

#include "audio_policy_manager.h"
#include "audio_policy_proxy.h"
#include "audio_errors.h"
#include "audio_server_death_recipient.h"
#include "audio_policy_log.h"
#include "audio_utils.h"
#include "audio_zone_client.h"

namespace OHOS {
namespace AudioStandard {
int32_t AudioPolicyManager::RegisterAudioZoneClient(const sptr<IRemoteObject> &object)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);

    return gsp->RegisterAudioZoneClient(object);
}

int32_t AudioPolicyManager::CreateAudioZone(const std::string &name, const AudioZoneContext &context)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);

    int32_t zoneId = ERROR;
    gsp->CreateAudioZone(name, context, zoneId, getpid());
    return zoneId;
}

void AudioPolicyManager::ReleaseAudioZone(int32_t zoneId)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "audio policy manager proxy is NULL.");

    gsp->ReleaseAudioZone(zoneId);
}

void AudioPolicyManager::UpdateContextForAudioZone(int32_t zoneId, const AudioZoneContext &context)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "audio policy manager proxy is NULL.");

    gsp->UpdateContextForAudioZone(zoneId, context);
}

int32_t AudioPolicyManager::GetAudioZoneForApp(const std::vector<int32_t> &userIds, std::vector<int32_t> &retUserIds)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);

    return gsp->GetAudioZoneForApp(userIds, retUserIds);
}

const std::vector<std::shared_ptr<AudioZoneDescriptor>> AudioPolicyManager::GetAllAudioZone()
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    std::vector<std::shared_ptr<AudioZoneDescriptor>> zoneDescriptors;
    CHECK_AND_RETURN_RET(gsp != nullptr, zoneDescriptors);

    std::vector<std::shared_ptr<AudioZoneDescriptor>> descs;
    gsp->GetAllAudioZone(descs);
    return descs;
}

const std::shared_ptr<AudioZoneDescriptor> AudioPolicyManager::GetAudioZone(int32_t zoneId)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, nullptr);

    std::shared_ptr<AudioZoneDescriptor> desc;
    gsp->GetAudioZone(zoneId, desc);
    return desc;
}

int32_t AudioPolicyManager::GetAudioZoneByName(std::string name)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);

    int32_t zoneId = ERROR;
    gsp->GetAudioZoneByName(name, zoneId);
    return zoneId;
}

int32_t AudioPolicyManager::BindDeviceToAudioZone(int32_t zoneId,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);

    return gsp->BindDeviceToAudioZone(zoneId, devices);
}

int32_t AudioPolicyManager::UnBindDeviceToAudioZone(int32_t zoneId,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);

    return gsp->UnBindDeviceToAudioZone(zoneId, devices);
}

int32_t AudioPolicyManager::EnableAudioZoneReport(bool enable)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);

    return gsp->EnableAudioZoneReport(enable);
}

int32_t AudioPolicyManager::EnableAudioZoneChangeReport(int32_t zoneId, bool enable)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);

    return gsp->EnableAudioZoneChangeReport(zoneId, enable);
}

int32_t AudioPolicyManager::AddUidToAudioZone(int32_t zoneId, int32_t uid)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);

    return gsp->AddUidToAudioZone(zoneId, uid);
}

int32_t AudioPolicyManager::RemoveUidFromAudioZone(int32_t zoneId, int32_t uid)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);

    return gsp->RemoveUidFromAudioZone(zoneId, uid);
}

int32_t AudioPolicyManager::AddUidUsagesToAudioZone(int32_t zoneId, int32_t uid, const std::set<StreamUsage> &usages)
{
    std::set<int32_t> usageSet;
    for (const auto &usage : usages) {
        CHECK_AND_RETURN_RET_LOG(usage > STREAM_USAGE_UNKNOWN && usage <= STREAM_USAGE_MAX, ERR_INVALID_PARAM,
            "usage is invalid");
        usageSet.insert(static_cast<int32_t>(usage));
    }

    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);

    return gsp->AddUidUsagesToAudioZone(zoneId, uid, usageSet);
}

int32_t AudioPolicyManager::RemoveUidUsagesFromAudioZone(
    int32_t zoneId, int32_t uid, const std::set<StreamUsage> &usages)
{
    std::set<int32_t> usageSet;
    for (const auto &usage : usages) {
        CHECK_AND_RETURN_RET_LOG(usage > STREAM_USAGE_UNKNOWN && usage <= STREAM_USAGE_MAX, ERR_INVALID_PARAM,
            "usage is invalid");
        usageSet.insert(static_cast<int32_t>(usage));
    }

    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);

    return gsp->RemoveUidUsagesFromAudioZone(zoneId, uid, usageSet);
}

int32_t AudioPolicyManager::AddStreamToAudioZone(int32_t zoneId, AudioZoneStream stream)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, -1);
    return gsp->AddStreamToAudioZone(zoneId, stream);
}

int32_t AudioPolicyManager::AddStreamsToAudioZone(int32_t zoneId, std::vector<AudioZoneStream> streams)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, -1);
    return gsp->AddStreamsToAudioZone(zoneId, streams);
}

int32_t AudioPolicyManager::AddStreamIdToAudioZone(int32_t zoneId, uint32_t streamId)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);
    return gsp->AddStreamIdToAudioZone(zoneId, streamId);
}

int32_t AudioPolicyManager::RemoveStreamIdFromAudioZone(int32_t zoneId, uint32_t streamId)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);
    return gsp->RemoveStreamIdFromAudioZone(zoneId, streamId);
}

int32_t AudioPolicyManager::PauseAudioFocusForExtAudioZone(const std::vector<int32_t> &zoneIds)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);
    return gsp->PauseAudioFocusForExtAudioZone(zoneIds);
}

int32_t AudioPolicyManager::ResumeAudioFocusForExtAudioZone(const std::vector<int32_t> &zoneIds)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);
    return gsp->ResumeAudioFocusForExtAudioZone(zoneIds);
}

int32_t AudioPolicyManager::ExcludeUidForExtZone(int32_t uid)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);
    return gsp->ExcludeUidForExtZone(uid);
}

int32_t AudioPolicyManager::UnexcludeUidForExtZone(int32_t uid)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);
    return gsp->UnexcludeUidForExtZone(uid);
}

void AudioPolicyManager::SetZoneDeviceVisible(bool visible)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "audio policy manager proxy is NULL.");
    gsp->SetZoneDeviceVisible(visible);
}

int32_t AudioPolicyManager::RemoveStreamFromAudioZone(int32_t zoneId, AudioZoneStream stream)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, -1);
    return gsp->RemoveStreamFromAudioZone(zoneId, stream);
}

int32_t AudioPolicyManager::RemoveStreamsFromAudioZone(int32_t zoneId, std::vector<AudioZoneStream> streams)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, -1);
    return gsp->RemoveStreamsFromAudioZone(zoneId, streams);
}

int32_t AudioPolicyManager::EnableSystemVolumeProxy(int32_t zoneId, DeviceType deviceType, bool enable)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);

    return gsp->EnableSystemVolumeProxy(zoneId, deviceType, enable);
}

std::list<std::pair<AudioInterrupt, AudioFocuState>> AudioPolicyManager::GetAudioInterruptForZone(
    int32_t zoneId, bool filterRemote)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, {});

    std::vector<std::map<AudioInterrupt, int32_t>> retList;
    gsp->GetAudioInterruptForZone(zoneId, retList, filterRemote);
    auto focusInfoList = FromIpcInterrupts(retList);
    return focusInfoList;
}

int32_t AudioPolicyManager::GetCurrentRendererChangeInfosForZone(int32_t zoneId,
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);

    return gsp->GetCurrentRendererChangeInfosForZone(zoneId, audioRendererChangeInfos);
}

std::list<std::pair<AudioInterrupt, AudioFocuState>> AudioPolicyManager::GetAudioInterruptForZone(
    int32_t zoneId, const std::string &deviceTag)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, {});

    std::vector<std::map<AudioInterrupt, int32_t>> retList;
    gsp->GetAudioInterruptForZone(zoneId, deviceTag, retList);
    auto focusInfoList = FromIpcInterrupts(retList);
    return focusInfoList;
}

int32_t AudioPolicyManager::EnableAudioZoneInterruptReport(int32_t zoneId, const std::string &deviceTag, bool enable)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);

    return gsp->EnableAudioZoneInterruptReport(zoneId, deviceTag, enable);
}

int32_t AudioPolicyManager::InjectInterruptToAudioZone(int32_t zoneId,
    const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);

    auto ipcInterrupts = ToIpcInterrupts(interrupts);
    return gsp->InjectInterruptToAudioZone(zoneId, ipcInterrupts);
}

int32_t AudioPolicyManager::InjectInterruptToAudioZone(int32_t zoneId, const std::string &deviceTag,
    const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts)
{
    const sptr<IAudioPolicy> gsp = GetAudioPolicyManagerProxy();
    CHECK_AND_RETURN_RET(gsp != nullptr, ERROR);

    auto ipcInterrupts = ToIpcInterrupts(interrupts);
    return gsp->InjectInterruptToAudioZone(zoneId, deviceTag, ipcInterrupts);
}
} // namespace AudioStandard
} // namespace OHOS
