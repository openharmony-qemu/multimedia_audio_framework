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
#define LOG_TAG "AudioHearingAidDeviceBehavior"
#endif

#include "audio_errors.h"
#include "audio_log.h"
#include "audio_a2dp_device.h"
#include "audio_active_device.h"
#include "audio_iohandle_map.h"
#include "audio_pipe_manager.h"
#include "audio_core_config_manager.h"
#include "audio_policy_manager_factory.h"
#include "audio_policy_utils.h"
#include "audio_server_proxy.h"


#include "audio_hearing_aid_device_behavior.h"

namespace OHOS {
namespace AudioStandard {
int32_t AudioHearingAidDeviceBehavior::ActivateDevice(const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    const AudioStreamDeviceChangeReasonExt reason)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, ERR_NULL_POINTER, "streamDesc is nullptr");
    CHECK_AND_RETURN_RET_LOG(!streamDesc->newDeviceDescs_.empty(), ERR_INVALID_PARAM, "newDeviceDescs_ is empty");
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = streamDesc->newDeviceDescs_.front();
    CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERR_NULL_POINTER, "deviceDesc is nullptr");

    CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr &&
        AudioA2dpDevice::GetInstance().CheckHearingAidDeviceExist(deviceDesc->macAddress_),
        ERR_INVALID_PARAM, "Target HearingAid device doesn't exist.");
    int32_t result = ERROR;
#ifdef BLUETOOTH_ENABLE
    if (AudioIOHandleMap::GetInstance().CheckIOHandleExist(HEARING_AID_SPEAKER)) {
        AUDIO_WARNING_LOG("HearingAid device [%{public}s] [%{public}s] is already active",
            AudioPolicyUtils::GetInstance().GetEncryptAddr(deviceDesc->macAddress_).c_str(),
            deviceDesc->deviceName_.c_str());
        return SUCCESS;
    }

    AudioStreamInfo audioStreamInfo = {};
    DeviceStreamInfo hearingAidStreamInfo = deviceDesc->GetDeviceStreamInfo();
    audioStreamInfo.samplingRate = hearingAidStreamInfo.samplingRate.empty() ? AudioSamplingRate::SAMPLE_RATE_16000 :
        *hearingAidStreamInfo.samplingRate.rbegin();
    audioStreamInfo.encoding = hearingAidStreamInfo.encoding;
    audioStreamInfo.format = hearingAidStreamInfo.format;
    audioStreamInfo.channelLayout = hearingAidStreamInfo.channelLayout.empty() ? AudioChannelLayout::CH_LAYOUT_UNKNOWN :
        *hearingAidStreamInfo.channelLayout.rbegin();
    audioStreamInfo.channels = hearingAidStreamInfo.GetChannels().empty() ? AudioChannel::CHANNEL_UNKNOW :
        *hearingAidStreamInfo.GetChannels().rbegin();

    std::string networkId = AudioActiveDevice::GetInstance().GetCurrentOutputDeviceNetworkId();
    std::string sinkName = AudioPolicyUtils::GetInstance().GetSinkPortName(DEVICE_TYPE_HEARING_AID);
    result = LoadHearingAidModule(deviceDesc, audioStreamInfo, networkId, sinkName, SOURCE_TYPE_INVALID);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED, "LoadHearingAidModule failed %{public}d", result);
#endif
    return result;
}

int32_t AudioHearingAidDeviceBehavior::LoadHearingAidModule(std::shared_ptr<AudioDeviceDescriptor> deviceDesc,
    const AudioStreamInfo &audioStreamInfo, std::string networkId, std::string sinkName, SourceType sourceType)
{
    std::list<AudioModuleInfo> moduleInfoList;
    bool ret = AudioCoreConfigManager::GetInstance().GetModuleListByType(ClassType::TYPE_HEARING_AID, moduleInfoList);
    CHECK_AND_RETURN_RET_LOG(ret, ERR_OPERATION_FAILED, "HearingAid module is not exist in the configuration file");

    int32_t loadRet = AudioServerProxy::GetInstance().LoadHdiAdapterProxy(HDI_DEVICE_MANAGER_TYPE_BLUETOOTH,
        "bt_hearing_aid");
    if (loadRet) {
        AUDIO_ERR_LOG("load adapter failed");
    }
    for (auto &moduleInfo : moduleInfoList) {
        if (moduleInfo.role != "sink") {
            AUDIO_INFO_LOG("Load hearingAid module [%{public}s], role[%{public}s]",
                moduleInfo.name.c_str(), moduleInfo.role.c_str());
            continue;
        }
        DeviceRole configRole = OUTPUT_DEVICE;
        DeviceRole deviceRole = deviceDesc->deviceType_ == DEVICE_TYPE_HEARING_AID ? OUTPUT_DEVICE : INPUT_DEVICE;
        AUDIO_INFO_LOG("Load hearingAid module [%{public}s], role[%{public}d], config role[%{public}d]",
            moduleInfo.name.c_str(), deviceRole, configRole);
        if (configRole != deviceRole) {continue;}
        if (AudioIOHandleMap::GetInstance().CheckIOHandleExist(moduleInfo.name) == false) {
            AUDIO_INFO_LOG("hearingAid device connects for the first time");
            // HearingAid device connects for the first time
            AudioA2dpDevice::GetInstance().GetA2dpModuleInfo(moduleInfo, audioStreamInfo);
            uint32_t paIndex = 0;
            auto ioHandle = AudioPolicyManagerFactory::GetAudioPolicyManager().OpenAudioPort(moduleInfo, paIndex);
            CHECK_AND_CALL_FUNC_RETURN_RET(ioHandle != HDI_INVALID_ID, ERR_INVALID_HANDLE,
                HILOG_COMM_ERROR("[LoadHearingAidModule]OpenAudioPort failed ioHandle[%{public}u]", ioHandle));
            CHECK_AND_CALL_FUNC_RETURN_RET(paIndex != OPEN_PORT_FAILURE, ERR_OPERATION_FAILED,
                HILOG_COMM_ERROR("[LoadHearingAidModule]OpenAudioPort failed paId[%{public}u]", paIndex));
            AudioIOHandleMap::GetInstance().AddIOHandleInfo(moduleInfo.name, ioHandle);

            std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
            pipeInfo->id_ = ioHandle;
            pipeInfo->paIndex_ = paIndex;
            pipeInfo->name_ = "hearing_aid_output";
            pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
            pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
            pipeInfo->adapterName_ = "hearing_aid";
            pipeInfo->moduleInfo_ = moduleInfo;
            pipeInfo->pipeAction_ = PIPE_ACTION_DEFAULT;
            pipeInfo->InitAudioStreamInfo();
            auto pipeManager = AudioPipeManager::GetPipeManager();
            CHECK_AND_RETURN_RET_LOG(pipeManager != nullptr, ERR_OPERATION_FAILED, "pipeManager is null");
            pipeManager->AddAudioPipeInfo(pipeInfo);
            AUDIO_INFO_LOG("Add PipeInfo %{public}u in load hearingAid.", pipeInfo->id_);
            PipeDeviceVolumeInfo info(pipeInfo->id_, deviceDesc, {STREAM_VOICE_CALL});
            AudioAdapterManager::GetInstance().UpdateAudioPipeVolume({info});
        }
    }

    return SUCCESS;
}

int32_t AudioHearingAidDeviceBehavior::DeactivateDevice()
{
    return SUCCESS;
}

int32_t AudioHearingAidDeviceBehavior::DeactivateDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc)
{
    return SUCCESS;
}

} // namespace AudioStandard
} // namespace OHOS
