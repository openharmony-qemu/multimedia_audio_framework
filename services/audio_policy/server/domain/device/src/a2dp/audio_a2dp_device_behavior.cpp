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
#define LOG_TAG "AudioA2dpDeviceBehavior"
#endif

#include "audio_a2dp_device.h"
#include "audio_active_device.h"
#include "audio_collaborative_service.h"
#include "audio_core_service.h"
#include "audio_device_info.h"
#include "audio_errors.h"
#include "audio_iohandle_map.h"
#include "audio_log.h"
#include "audio_core_config_manager.h"
#include "audio_policy_manager_factory.h"
#include "audio_policy_utils.h"
#include "audio_pipe_manager.h"
#include "audio_server_proxy.h"

#include "audio_a2dp_device_behavior.h"

namespace OHOS {
namespace AudioStandard {
namespace {
static const int32_t BLUETOOTH_FETCH_RESULT_ERROR = 2;
static const std::string EMPTY_ADDRESS = "00:00:00:00:00:00";
static const std::string NULL_ADDRESS = "";
} // namespace
int32_t AudioA2dpDeviceBehavior::ActivateDevice(const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    const AudioStreamDeviceChangeReasonExt reason)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, ERR_NULL_POINTER, "streamDesc is nullptr");
    CHECK_AND_RETURN_RET_LOG(!streamDesc->newDeviceDescs_.empty(), ERR_INVALID_PARAM, "newDeviceDescs_ is empty");
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = streamDesc->newDeviceDescs_.front();
    CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERR_NULL_POINTER, "deviceDesc is nullptr");

    int32_t ret = ActivateA2dpDeviceWhenDescEnabled(deviceDesc, reason);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Activate a2dp [%{public}s] failed",
            AudioPolicyUtils::GetInstance().GetEncryptAddr(deviceDesc->macAddress_).c_str());
        return BLUETOOTH_FETCH_RESULT_ERROR;
    }
    return SUCCESS;
}

int32_t AudioA2dpDeviceBehavior::ActivateA2dpDeviceWhenDescEnabled(std::shared_ptr<AudioDeviceDescriptor> deviceDesc,
    const AudioStreamDeviceChangeReasonExt reason)
{
    CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERR_NULL_POINTER, "deviceDesc is nullptr");
    if (deviceDesc->isEnable_) {
        return ActivateA2dpDevice(deviceDesc, reason);
    }
    return SUCCESS;
}

int32_t AudioA2dpDeviceBehavior::ActivateA2dpDevice(std::shared_ptr<AudioDeviceDescriptor> deviceDesc,
    const AudioStreamDeviceChangeReasonExt reason)
{
    Trace trace("AudioA2dpDeviceBehavior::ActiveA2dpDevice");
    int32_t ret = SwitchActiveA2dpDevice(deviceDesc);
    return ret;
}

std::string AudioA2dpDeviceBehavior::GetActivatedBtDeviceMac()
{
    auto activedBts = AudioActiveDevice::GetInstance().GetActiveBtDeviceMac();
    std::string lastActiveA2dpDevice = activedBts.empty() ? "" : *activedBts.begin();
    return lastActiveA2dpDevice;
}

bool AudioA2dpDeviceBehavior::IsBtDeviceActive(std::shared_ptr<AudioDeviceDescriptor> deviceDesc)
{
    auto isActivated = Bluetooth::AudioA2dpManager::GetActiveA2dpDevice() == deviceDesc->macAddress_ &&
    Bluetooth::AudioA2dpManager::GetActiveA2dpDeviceLocal() == deviceDesc->macAddress_;
// if bluetooth used for audio zone, devices will be actived by default, and the interface will return empty.
#ifdef FEATURE_AUDIO_ZONE_VOLUME
    isActivated = true;
#endif
    return isActivated;
}

int32_t AudioA2dpDeviceBehavior::SwitchActiveA2dpDevice(std::shared_ptr<AudioDeviceDescriptor> deviceDesc)
{
    CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr &&
        AudioA2dpDevice::GetInstance().CheckA2dpDeviceExist(deviceDesc->macAddress_),
        ERR_INVALID_PARAM, "Target A2DP device doesn't exist.");

    int32_t result = ERROR;
#ifdef BLUETOOTH_ENABLE
    std::string lastActiveA2dpDevice = GetActivatedBtDeviceMac();
    bool isActivated = IsBtDeviceActive(deviceDesc);
    if (isActivated) {
        AUDIO_INFO_LOG("[%{public}s] id:%{public}d already active",
            AudioPolicyUtils::GetInstance().GetEncryptAddr(deviceDesc->macAddress_).c_str(),
            deviceDesc->deviceId_);
    } else {
        Bluetooth::BluetoothRemoteDevice device = Bluetooth::BluetoothRemoteDevice(deviceDesc->macAddress_);
        std::string productId;
        device.GetDeviceProductId(productId);
        result = AudioCollaborativeService::GetAudioCollaborativeService().UpdateCollaborativeProductId(productId);
        if (result != SUCCESS) {
            AUDIO_INFO_LOG("productId: %{public}s", productId.c_str());
        }

        result = Bluetooth::AudioA2dpManager::SetActiveA2dpDevice(deviceDesc->GetMacAddress());
        if (result != SUCCESS) {
            AUDIO_ERR_LOG("Active [%{public}s] failed, active devices unchanged",
                AudioPolicyUtils::GetInstance().GetEncryptAddr(deviceDesc->GetMacAddress()).c_str());
            return result;
        }
    }
#ifndef FEATURE_AUDIO_ZONE
    AudioActiveDevice::GetInstance().RemoveActiveBtDevice(lastActiveA2dpDevice);
#endif
    AudioActiveDevice::GetInstance().AddActiveBtDevice(deviceDesc->GetMacAddress());

    std::string moduleName = AudioPolicyUtils::GetInstance().GetA2dpModuleName(deviceDesc->macAddress_, OUTPUT_DEVICE);
    CHECK_AND_RETURN_RET(!isActivated || !AudioIOHandleMap::GetInstance().CheckIOHandleExist(moduleName), SUCCESS);
    AudioStreamInfo audioStreamInfo = {};
    AudioActiveDevice::GetInstance().GetActiveA2dpDeviceStreamInfo(deviceDesc->GetMacAddress(), audioStreamInfo);
    std::string networkId  = AudioActiveDevice::GetInstance().GetCurrentOutputDeviceNetworkId();
    std::string sinkName   = AudioPolicyUtils::GetInstance().GetSinkPortName(
        AudioActiveDevice::GetInstance().GetCurrentOutputDeviceType(), PIPE_TYPE_UNKNOWN, deviceDesc->macAddress_);

    A2dpReloadContext context = {DEVICE_TYPE_BLUETOOTH_A2DP, audioStreamInfo, networkId, sinkName,
        SOURCE_TYPE_INVALID, deviceDesc->macAddress_};
    result = LoadA2dpModule(context);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED, "LoadA2dpModule failed %{public}d", result);
    auto coreService = AudioCoreService::GetCoreService();
    CHECK_AND_RETURN_RET_LOG(coreService != nullptr, ERR_NULL_POINTER, "coreService is nullptr");
    coreService->HandleA2dpSuspendWhenLoad(deviceDesc->macAddress_);
#endif
    return result;
}

int32_t AudioA2dpDeviceBehavior::LoadA2dpModule(const A2dpReloadContext &context)
{
    std::list<AudioModuleInfo> moduleInfoList;
    bool ret = AudioCoreConfigManager::GetInstance().GetModuleListByType(ClassType::TYPE_A2DP, moduleInfoList);
    CHECK_AND_RETURN_RET_LOG(ret, ERR_OPERATION_FAILED, "A2dp module is not exist in the configuration file");

    // not load bt_a2dp_fast and bt_hdap, maybe need fix
    int32_t loadRet = AudioServerProxy::GetInstance().LoadHdiAdapterProxy(HDI_DEVICE_MANAGER_TYPE_BLUETOOTH, "bt_a2dp");

    for (auto &moduleInfo : moduleInfoList) {
        DeviceRole configRole = (moduleInfo.role == "source") ? INPUT_DEVICE : OUTPUT_DEVICE;
        DeviceRole deviceRole = (context.deviceType == DEVICE_TYPE_BLUETOOTH_A2DP) ? OUTPUT_DEVICE : INPUT_DEVICE;
        AUDIO_INFO_LOG("Load a2dp module [%{public}s], role[%{public}d], config role[%{public}d]",
            moduleInfo.name.c_str(), deviceRole, configRole);
        if (configRole != deviceRole) { continue; }
        moduleInfo.name = AudioPolicyUtils::GetInstance().GetA2dpModuleName(context.macAddress, deviceRole);
        if (!AudioIOHandleMap::GetInstance().CheckIOHandleExist(moduleInfo.name)) {
            AudioA2dpDevice::GetInstance().GetA2dpModuleInfo(moduleInfo, context.audioStreamInfo);

            uint32_t paIndex = 0;
            moduleInfo.deviceType = std::to_string(static_cast<int32_t>(context.deviceType));
            moduleInfo.macAddress = context.macAddress;
            AudioIOHandle ioHandle = AudioPolicyManagerFactory::GetAudioPolicyManager().OpenAudioPort(moduleInfo,
                paIndex);
            CHECK_AND_CALL_FUNC_RETURN_RET(ioHandle != HDI_INVALID_ID, ERR_INVALID_HANDLE,
                HILOG_COMM_ERROR("[LoadA2dpModule]OpenAudioPort failed ioHandle[%{public}u]", ioHandle));
            CHECK_AND_CALL_FUNC_RETURN_RET(paIndex != OPEN_PORT_FAILURE, ERR_OPERATION_FAILED,
                HILOG_COMM_ERROR("[LoadA2dpModule]OpenAudioPort failed paId[%{public}u]", paIndex));

            AudioIOHandleMap::GetInstance().AddIOHandleInfo(moduleInfo.name, ioHandle);

            int32_t result = LoadA2dpModuleInner(moduleInfo, ioHandle, paIndex);
            CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "LoadA2dpModuleInner failed %{public}d", result);
        } else {
            // At least one a2dp device is already connected. A new a2dp device is connecting.
            // Need to reload a2dp module when switching to a2dp device.
            int32_t result = ReloadA2dpAudioPort(moduleInfo, context);
            CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "ReloadA2dpAudioPort failed %{public}d", result);
        }
    }
    return SUCCESS;
}

int32_t AudioA2dpDeviceBehavior::LoadA2dpModuleInner(const AudioModuleInfo &moduleInfo, AudioIOHandle ioHandle,
    uint32_t paIndex)
{
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->id_ = ioHandle;
    pipeInfo->paIndex_ = paIndex;
    if (moduleInfo.role == "sink") {
        pipeInfo->name_ = AudioPolicyUtils::GetInstance().GetA2dpPipeName(moduleInfo.macAddress, OUTPUT_DEVICE);
        pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
        pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    } else {
        pipeInfo->name_ = "a2dp_input";
        pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;
        pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    }
    pipeInfo->adapterName_ = "a2dp";
    pipeInfo->moduleInfo_ = moduleInfo;
    pipeInfo->pipeAction_ = PIPE_ACTION_DEFAULT;
    pipeInfo->InitAudioStreamInfo();
    auto pipeManager = AudioPipeManager::GetPipeManager();
    CHECK_AND_RETURN_RET_LOG(pipeManager != nullptr, ERR_NULL_POINTER, "pipeManager is nullptr");
    pipeManager->AddAudioPipeInfo(pipeInfo);
    AUDIO_INFO_LOG("Add PipeInfo %{public}u in load a2dp.", pipeInfo->id_);
    return SUCCESS;
}

int32_t AudioA2dpDeviceBehavior::ReloadA2dpAudioPort(AudioModuleInfo &moduleInfo, const A2dpReloadContext &context)
{
    AudioIOHandleMap::GetInstance().MuteDefaultSinkPort(context.networkId, context.sinkName);

    std::string portName = (context.deviceType == DEVICE_TYPE_BLUETOOTH_A2DP_IN) ?
        BLUETOOTH_MIC : moduleInfo.name;

    AudioIOHandle activateDeviceIOHandle;
    AudioIOHandleMap::GetInstance().GetModuleIdByKey(portName, activateDeviceIOHandle);

    auto pipeManager = AudioPipeManager::GetPipeManager();
    CHECK_AND_RETURN_RET_LOG(pipeManager != nullptr, ERR_NULL_POINTER, "pipeManager is nullptr");
    uint32_t curPaIndex = pipeManager->GetPaIndexByIoHandle(activateDeviceIOHandle);
    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescs =
        pipeManager->GetStreamDescsByIoHandle(activateDeviceIOHandle);
    AUDIO_INFO_LOG("INFO: %{public}u,%{public}u,%{public}zu", activateDeviceIOHandle, curPaIndex, streamDescs.size());

    int32_t engineFlag = GetEngineFlag();
    if (engineFlag != 1) {
        int32_t result = AudioPolicyManagerFactory::GetAudioPolicyManager().CloseAudioPort(
            activateDeviceIOHandle, curPaIndex);
        CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "CloseAudioPort failed %{public}d", result);
    }
    pipeManager->RemoveAudioPipeInfo(activateDeviceIOHandle);
    AudioAdapterManager::GetInstance().RemoveAudioPipeVolume(activateDeviceIOHandle);

    AudioA2dpDevice::GetInstance().GetA2dpModuleInfo(moduleInfo, context.audioStreamInfo);
    uint32_t paIndex = 0;
    moduleInfo.deviceType = std::to_string(static_cast<int32_t>(context.deviceType));
    moduleInfo.macAddress = context.macAddress;
    AudioIOHandle ioHandle = ReloadOrOpenAudioPort(engineFlag, moduleInfo, paIndex);
    AudioIOHandleMap::GetInstance().AddIOHandleInfo(moduleInfo.name, ioHandle);

    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->id_ = ioHandle;
    pipeInfo->paIndex_ = paIndex;
    if (moduleInfo.role == "sink") {
        pipeInfo->name_ = AudioPolicyUtils::GetInstance().GetA2dpPipeName(moduleInfo.macAddress, OUTPUT_DEVICE);
        pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
        pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    } else {
        pipeInfo->name_ = "a2dp_input";
        pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;
        pipeInfo->routeFlag_ = AUDIO_INPUT_FLAG_NORMAL;
    }
    pipeInfo->adapterName_ = "a2dp";
    pipeInfo->moduleInfo_ = moduleInfo;
    pipeInfo->pipeAction_ = PIPE_ACTION_DEFAULT;
    pipeInfo->InitAudioStreamInfo();
    pipeInfo->streamDescriptors_.insert(pipeInfo->streamDescriptors_.end(),
        streamDescs.begin(), streamDescs.end());
    pipeManager->AddAudioPipeInfo(pipeInfo);
    auto pipeDeviceVolumeGroup = AudioPipeManager::GetPipeManager()->GetPipeDeviceVolumeInfoForPipe(pipeInfo);
    AudioAdapterManager::GetInstance().UpdateAudioPipeVolume(pipeDeviceVolumeGroup);

    AUDIO_INFO_LOG("reload a2dp module, Close paIndex: %{public}u, open paIndex: %{public}u", curPaIndex, paIndex);
    return SUCCESS;
}

AudioIOHandle AudioA2dpDeviceBehavior::ReloadOrOpenAudioPort(int32_t engineFlag, AudioModuleInfo &moduleInfo,
    uint32_t &paIndex)
{
    AudioIOHandle ioHandle;
    if (engineFlag == 1) {
        ioHandle = AudioPolicyManagerFactory::GetAudioPolicyManager().ReloadA2dpAudioPort(moduleInfo, paIndex);
        CHECK_AND_CALL_FUNC_RETURN_RET(ioHandle != HDI_INVALID_ID, ERR_INVALID_HANDLE,
            HILOG_COMM_ERROR("[ReloadOrOpenAudioPort]ReloadAudioPort failed ioHandle[%{public}u]", ioHandle));
        CHECK_AND_CALL_FUNC_RETURN_RET(paIndex != OPEN_PORT_FAILURE, ERR_OPERATION_FAILED,
            HILOG_COMM_ERROR("[ReloadOrOpenAudioPort]ReloadAudioPort failed paId[%{public}u]", paIndex));
    } else {
        ioHandle = AudioPolicyManagerFactory::GetAudioPolicyManager().OpenAudioPort(moduleInfo, paIndex);
        CHECK_AND_CALL_FUNC_RETURN_RET(ioHandle != HDI_INVALID_ID, ERR_INVALID_HANDLE,
            HILOG_COMM_ERROR("[ReloadOrOpenAudioPort]OpenAudioPort failed ioHandle[%{public}u]", ioHandle));
        CHECK_AND_CALL_FUNC_RETURN_RET(paIndex != OPEN_PORT_FAILURE, ERR_OPERATION_FAILED,
            HILOG_COMM_ERROR("[ReloadOrOpenAudioPort]OpenAudioPort failed paId[%{public}u]", paIndex));
    }
    return ioHandle;
}

int32_t AudioA2dpDeviceBehavior::DeactivateDevice()
{
    if (Bluetooth::AudioA2dpManager::GetActiveA2dpDeviceLocal() != EMPTY_ADDRESS) {
        AudioPolicyManagerFactory::GetAudioPolicyManager().StopAudioPort(BLUETOOTH_SPEAKER);
        Bluetooth::AudioA2dpManager::SetActiveA2dpDevice(NULL_ADDRESS);
    }
    return SUCCESS;
}

int32_t AudioA2dpDeviceBehavior::DeactivateDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc)
{
    return SUCCESS;
}

} // namespace AudioStandard
} // namespace OHOS