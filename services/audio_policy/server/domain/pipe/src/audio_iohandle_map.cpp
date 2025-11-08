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
#define LOG_TAG "AudioIOHandleMap"
#endif

#include "audio_iohandle_map.h"
#include "parameter.h"
#include "parameters.h"
#include "audio_policy_manager_factory.h"

#include "audio_server_proxy.h"
#include "audio_policy_async_action_handler.h"
#include "audio_pipe_manager.h"

namespace OHOS {
namespace AudioStandard {

class WaitActiveDeviceAction : public PolicyAsyncAction {
public:
    WaitActiveDeviceAction(int32_t muteDuration, const std::string &portName)
        : muteDuration_(muteDuration), portName_(portName)
    {}

    void Exec() override
    {
        AudioIOHandleMap::GetInstance().UnmutePortAfterMuteDuration(muteDuration_, portName_);
    }

private:
    int32_t muteDuration_;
    const std::string portName_;
};

class UnmutePortAction : public PolicyAsyncAction {
public:
    UnmutePortAction(int32_t muteDuration, const std::string &portName)
        : muteDuration_(muteDuration), portName_(portName)
    {}

    void Exec() override
    {
        AudioIOHandleMap::GetInstance().DoUnmutePort(muteDuration_, portName_);
    }

private:
    int32_t muteDuration_;
    const std::string portName_;
};

static const int64_t WAIT_SET_MUTE_LATENCY_TIME_US = 80000; // 80ms
static const int64_t OLD_DEVICE_UNAVALIABLE_MUTE_MS = 1000000; // 1s
static const int64_t WAIT_MOVE_DEVICE_MUTE_TIME_MAX_MS = 5000; // 5s
static const int64_t US_PER_MS = 1000;

std::map<std::string, std::string> AudioIOHandleMap::sinkPortStrToClassStrMap_ = {
    {PRIMARY_SPEAKER, PRIMARY_CLASS},
    {BLUETOOTH_SPEAKER, A2DP_CLASS},
    {USB_SPEAKER, USB_CLASS},
    {DP_SINK, DP_CLASS},
    {OFFLOAD_PRIMARY_SPEAKER, OFFLOAD_CLASS},
    {PRIMARY_DIRECT_VOIP, DIRECT_VOIP_CLASS},
    {PRIMARY_MMAP_VOIP, MMAP_VOIP_CLASS},
    {MCH_PRIMARY_SPEAKER, MCH_CLASS},
    {PRIMARY_MMAP, MMAP_CLASS},
    {BLUETOOTH_A2DP_FAST, A2DP_FAST_CLASS},
    {PRIMARY_DIRECT, DIRECT_CLASS}
};

void AudioIOHandleMap::DeInit()
{
    std::lock_guard<std::mutex> ioHandleLock(ioHandlesMutex_);
    IOHandles_.clear();
}

std::unordered_map<std::string, AudioIOHandle> AudioIOHandleMap::GetCopy()
{
    std::lock_guard<std::mutex> ioHandleLock(ioHandlesMutex_);
    return IOHandles_;
}

bool AudioIOHandleMap::GetModuleIdByKey(std::string moduleName, AudioIOHandle& moduleId)
{
    std::lock_guard<std::mutex> ioHandleLock(ioHandlesMutex_);
    if (IOHandles_.count(moduleName)) {
        moduleId = IOHandles_[moduleName];
        return true;
    }
    return false;
}

bool AudioIOHandleMap::CheckIOHandleExist(std::string moduleName)
{
    std::lock_guard<std::mutex> ioHandleLock(ioHandlesMutex_);
    return (IOHandles_.find(moduleName) != IOHandles_.end());
}

void AudioIOHandleMap::DelIOHandleInfo(std::string moduleName)
{
    std::lock_guard<std::mutex> ioHandleLock(ioHandlesMutex_);
    IOHandles_.erase(moduleName);
}

void AudioIOHandleMap::AddIOHandleInfo(std::string moduleName, const AudioIOHandle& moduleId)
{
    std::lock_guard<std::mutex> ioHandleLock(ioHandlesMutex_);
    IOHandles_[moduleName] = moduleId;
}

// private methods
AudioIOHandle AudioIOHandleMap::GetSinkIOHandle(DeviceType deviceType)
{
    std::lock_guard<std::mutex> ioHandleLock(ioHandlesMutex_);
    AudioIOHandle ioHandle;
    switch (deviceType) {
        case DeviceType::DEVICE_TYPE_WIRED_HEADSET:
        case DeviceType::DEVICE_TYPE_WIRED_HEADPHONES:
        case DeviceType::DEVICE_TYPE_USB_HEADSET:
        case DeviceType::DEVICE_TYPE_EARPIECE:
        case DeviceType::DEVICE_TYPE_SPEAKER:
        case DeviceType::DEVICE_TYPE_BLUETOOTH_SCO:
        case DeviceType::DEVICE_TYPE_HDMI:
            ioHandle = IOHandles_[PRIMARY_SPEAKER];
            break;
        case DeviceType::DEVICE_TYPE_USB_ARM_HEADSET:
            ioHandle = IOHandles_[USB_SPEAKER];
            break;
        case DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP:
            ioHandle = IOHandles_[BLUETOOTH_SPEAKER];
            break;
        case DeviceType::DEVICE_TYPE_FILE_SINK:
            ioHandle = IOHandles_[FILE_SINK];
            break;
        case DeviceType::DEVICE_TYPE_DP:
            ioHandle = IOHandles_[DP_SINK];
            break;
        default:
            ioHandle = IOHandles_[PRIMARY_SPEAKER];
            break;
    }
    return ioHandle;
}

AudioIOHandle AudioIOHandleMap::GetSourceIOHandle(DeviceType deviceType)
{
    std::lock_guard<std::mutex> ioHandleLock(ioHandlesMutex_);
    AudioIOHandle ioHandle;
    switch (deviceType) {
        case DeviceType::DEVICE_TYPE_USB_ARM_HEADSET:
            ioHandle = IOHandles_[USB_MIC];
            break;
        case DeviceType::DEVICE_TYPE_MIC:
            ioHandle = IOHandles_[PRIMARY_MIC];
            break;
        case DeviceType::DEVICE_TYPE_FILE_SOURCE:
            ioHandle = IOHandles_[FILE_SOURCE];
            break;
        case DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP_IN:
            ioHandle = IOHandles_[BLUETOOTH_MIC];
            break;
        case DeviceType::DEVICE_TYPE_ACCESSORY:
            ioHandle = IOHandles_[ACCESSORY_SOURCE];
            break;
        default:
            ioHandle = IOHandles_[PRIMARY_MIC];
            break;
    }
    return ioHandle;
}

int32_t AudioIOHandleMap::OpenPortAndInsertIOHandle(const std::string &moduleName,
    const AudioModuleInfo &moduleInfo)
{
    uint32_t paIndex = 0;
    AudioIOHandle ioHandle = AudioPolicyManagerFactory::GetAudioPolicyManager().OpenAudioPort(moduleInfo, paIndex);
    CHECK_AND_RETURN_RET_LOG(ioHandle != HDI_INVALID_ID, ERR_INVALID_HANDLE,
        "OpenAudioPort failed ioHandle[%{public}u]", ioHandle);
    CHECK_AND_RETURN_RET_LOG(paIndex != OPEN_PORT_FAILURE, ERR_OPERATION_FAILED,
        "OpenAudioPort failed paId[%{public}u]", paIndex);

    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->id_ = ioHandle;
    pipeInfo->paIndex_ = paIndex;
    pipeInfo->name_ = moduleName;
    if (moduleInfo.role == "sink") {
        pipeInfo->pipeRole_ = PIPE_ROLE_OUTPUT;
        pipeInfo->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    } else {
        pipeInfo->pipeRole_ = PIPE_ROLE_INPUT;
        pipeInfo->routeFlag_ = moduleInfo.sourceType == std::to_string(SourceType::SOURCE_TYPE_WAKEUP) ?
            AUDIO_INPUT_FLAG_WAKEUP : AUDIO_INPUT_FLAG_NORMAL;
    }
    pipeInfo->adapterName_ = moduleInfo.adapterName;
    pipeInfo->moduleInfo_ = moduleInfo;
    pipeInfo->pipeAction_ = PIPE_ACTION_DEFAULT;
    pipeInfo->InitAudioStreamInfo();
    AudioPipeManager::GetPipeManager()->AddAudioPipeInfo(pipeInfo);

    AddIOHandleInfo(moduleName, ioHandle);

    return SUCCESS;
}

int32_t AudioIOHandleMap::ClosePortAndEraseIOHandle(const std::string &moduleName)
{
    std::shared_ptr<AudioPipeManager> pipeManager = AudioPipeManager::GetPipeManager();
    auto pipeInfoInput = pipeManager->GetPipeinfoByNameAndFlag("primary", AUDIO_INPUT_FLAG_NORMAL);
    if (pipeInfoInput != nullptr && pipeInfoInput->softLinkFlag_) {
        pipeInfoInput->streamDescMap_.clear();
        pipeInfoInput->streamDescriptors_.clear();
        pipeManager->UpdateAudioPipeInfo(pipeInfoInput);
        return SUCCESS;
    }
    AudioIOHandle ioHandle;
    CHECK_AND_RETURN_RET_LOG(GetModuleIdByKey(moduleName, ioHandle), ERROR,
        "can not find %{public}s in io map", moduleName.c_str());
    DelIOHandleInfo(moduleName);

    uint32_t paIndex = pipeManager->GetPaIndexByIoHandle(ioHandle);
    pipeManager->RemoveAudioPipeInfo(ioHandle);

    int32_t result = AudioPolicyManagerFactory::GetAudioPolicyManager().CloseAudioPort(ioHandle, paIndex);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, result, "CloseAudioPort failed %{public}d", result);
    return SUCCESS;
}

void AudioIOHandleMap::MuteSinkPort(const std::string &portName, int32_t duration, bool isSync, bool isSleepEnabled)
{
    if (sinkPortStrToClassStrMap_.count(portName) > 0) {
        // Mute by render sink. (primary、a2dp、usb、dp、offload)
        AudioServerProxy::GetInstance().SetSinkMuteForSwitchDeviceProxy(sinkPortStrToClassStrMap_.at(portName),
            duration, true);
    } else {
        // Mute by pa.
        AudioPolicyManagerFactory::GetAudioPolicyManager().SetSinkMute(portName, true, isSync);
    }

    std::shared_ptr<WaitActiveDeviceAction> action = std::make_shared<WaitActiveDeviceAction>(duration, portName);
    CHECK_AND_RETURN_LOG(action != nullptr, "action is nullptr");
    AsyncActionDesc desc;
    desc.action = std::static_pointer_cast<PolicyAsyncAction>(action);
    DelayedSingleton<AudioPolicyAsyncActionHandler>::GetInstance()->PostAsyncAction(desc);

    if (isSleepEnabled) {
        usleep(WAIT_SET_MUTE_LATENCY_TIME_US); // sleep fix data cache pop.
    }
}

void AudioIOHandleMap::MuteDefaultSinkPort(std::string networkID, std::string sinkName)
{
    if (networkID != LOCAL_NETWORK_ID || sinkName != PRIMARY_SPEAKER) {
        // PA may move the sink to default when unloading module.
        MuteSinkPort(PRIMARY_SPEAKER, OLD_DEVICE_UNAVALIABLE_MUTE_MS, true);
    }
}

void AudioIOHandleMap::SetMoveFinish(bool flag)
{
    moveDeviceFinished_ = flag;
}

void AudioIOHandleMap::NotifyUnmutePort()
{
    std::unique_lock<std::mutex> lock(moveDeviceMutex_);
    moveDeviceFinished_ = true;
    moveDeviceCV_.notify_all();
}

void AudioIOHandleMap::UnmutePortAfterMuteDuration(int32_t muteDuration, const std::string &portName)
{
    Trace trace("UnmutePortAfterMuteDuration:" + portName + " for " + std::to_string(muteDuration) + "us");

    if (!moveDeviceFinished_.load()) {
        std::unique_lock<std::mutex> lock(moveDeviceMutex_);
        bool loadWaiting = moveDeviceCV_.wait_for(lock,
            std::chrono::milliseconds(WAIT_MOVE_DEVICE_MUTE_TIME_MAX_MS),
            [this] { return moveDeviceFinished_.load(); }
        );
        if (!loadWaiting) {
            AUDIO_ERR_LOG("move device time out");
        }
    }
    AUDIO_INFO_LOG("%{public}d us for device type[%{public}s]", muteDuration, portName.c_str());

    std::shared_ptr<UnmutePortAction> action = std::make_shared<UnmutePortAction>(muteDuration, portName);
    CHECK_AND_RETURN_LOG(action != nullptr, "action is nullptr");
    AsyncActionDesc desc;
    desc.delayTimeMs = muteDuration / US_PER_MS;
    desc.action = std::static_pointer_cast<PolicyAsyncAction>(action);
    DelayedSingleton<AudioPolicyAsyncActionHandler>::GetInstance()->PostAsyncAction(desc);
}

void AudioIOHandleMap::DoUnmutePort(int32_t muteDuration, const std::string &portName)
{
    if (sinkPortStrToClassStrMap_.count(portName) > 0) {
        AudioServerProxy::GetInstance().SetSinkMuteForSwitchDeviceProxy(sinkPortStrToClassStrMap_.at(portName),
            muteDuration, false);
    } else {
        AudioPolicyManagerFactory::GetAudioPolicyManager().SetSinkMute(portName, false);
    }
}

int32_t AudioIOHandleMap::ReloadPortAndUpdateIOHandle(std::shared_ptr<AudioPipeInfo> &pipeInfo,
    const AudioModuleInfo &moduleInfo, bool softLinkFlag)
{
    std::string oldModuleName = pipeInfo->moduleInfo_.name;
    AudioIOHandle ioHandle;
    CHECK_AND_RETURN_RET_LOG(GetModuleIdByKey(oldModuleName, ioHandle), ERROR,
        "can not find %{public}s in io map", oldModuleName.c_str());
    DelIOHandleInfo(oldModuleName);

    uint32_t paIndex = 0;
    ioHandle = AudioPolicyManagerFactory::GetAudioPolicyManager().ReloadA2dpAudioPort(moduleInfo, paIndex);
    AUDIO_INFO_LOG("[reload-module] %{public}s, id:%{public}d, paIndex: %{public}u, pipeName: %{public}s",
        moduleInfo.name.c_str(), ioHandle, paIndex, pipeInfo->name_.c_str());

    pipeInfo->id_ = ioHandle;
    pipeInfo->paIndex_ = paIndex;
    pipeInfo->adapterName_ = moduleInfo.adapterName;
    pipeInfo->moduleInfo_ = moduleInfo;
    pipeInfo->pipeAction_ = PIPE_ACTION_DEFAULT;
    pipeInfo->softLinkFlag_ = softLinkFlag;
    pipeInfo->InitAudioStreamInfo();

    AddIOHandleInfo(moduleInfo.name, ioHandle);
    return SUCCESS;
}
}
}
