/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioDeviceManagerImpl"
#endif

#include "audio_device_manager_impl.h"

#include <dlfcn.h>

#include "audio_device_adapter_impl.h"
#include "audio_errors.h"
#include "audio_hdi_log.h"

namespace OHOS {
namespace AudioStandard {
AudioDeviceManagerFactory &AudioDeviceManagerFactory::GetInstance()
{
    static AudioDeviceManagerFactory devMgr;
    return devMgr;
}

int32_t AudioDeviceManagerFactory::DestoryDeviceManager(const AudioDeviceManagerType audioMgrType)
{
    std::lock_guard<std::mutex> lock(devMgrFactoryMtx_);
    if (allHdiDevMgr_.find(audioMgrType) == allHdiDevMgr_.end()) {
        AUDIO_INFO_LOG("Audio manager is already destoried, audioMgrType %{public}d.", audioMgrType);
        return SUCCESS;
    }

    int32_t ret = allHdiDevMgr_[audioMgrType]->Release();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Audio manager is busy, audioMgrType %{public}d.", audioMgrType);

    allHdiDevMgr_.erase(audioMgrType);
    return SUCCESS;
}

std::shared_ptr<IAudioDeviceManager> AudioDeviceManagerFactory::CreatDeviceManager(
    const AudioDeviceManagerType audioMgrType)
{
    std::lock_guard<std::mutex> lock(devMgrFactoryMtx_);
    if (allHdiDevMgr_.find(audioMgrType) != allHdiDevMgr_.end()) {
        return allHdiDevMgr_[audioMgrType];
    }
    std::shared_ptr<IAudioDeviceManager> audioDevMgr = nullptr;
    switch (audioMgrType) {
        case LOCAL_DEV_MGR:
            audioDevMgr = InitLocalAudioMgr();
            break;
        case REMOTE_DEV_MGR:
            audioDevMgr = InitRemoteAudioMgr();
            break;
        case BLUETOOTH_DEV_MGR:
            audioDevMgr = InitBluetoothAudioMgr();
            break;
        default:
            AUDIO_ERR_LOG("Get audio manager of audioMgrType %{public}d is not supported.", audioMgrType);
            return nullptr;
    }

    CHECK_AND_RETURN_RET_LOG(audioDevMgr != nullptr, nullptr,
        "Get audio manager of audioMgrType %{public}d fail.", audioMgrType);
    allHdiDevMgr_[audioMgrType] = audioDevMgr;
    return allHdiDevMgr_[audioMgrType];
}

std::shared_ptr<IAudioDeviceManager> AudioDeviceManagerFactory::InitLocalAudioMgr()
{
    AUDIO_ERR_LOG("Get local audio manager is not supported.");
    return nullptr;
}

std::shared_ptr<IAudioDeviceManager> AudioDeviceManagerFactory::InitRemoteAudioMgr()
{
    AUDIO_INFO_LOG("AudioDeviceManagerFactory: Init remote audio manager proxy.");
#ifdef FEATURE_DISTRIBUTE_AUDIO
    sptr<IAudioManager> audioMgr = IAudioManager::Get("daudio_primary_service", false);
#else
    sptr<IAudioManager> audioMgr = nullptr;
#endif // FEATURE_DISTRIBUTE_AUDIO

    CHECK_AND_RETURN_RET_LOG((audioMgr != nullptr), nullptr, "Init remote audio manager proxy fail.");
    AUDIO_DEBUG_LOG("Init remote hdi manager proxy success.");
    return std::make_shared<AudioDeviceManagerImpl>(REMOTE_DEV_MGR, audioMgr);
}

std::shared_ptr<IAudioDeviceManager> AudioDeviceManagerFactory::InitBluetoothAudioMgr()
{
    AUDIO_ERR_LOG("Get local audio manager is not supported.");
    return nullptr;
}

int32_t AudioDeviceManagerImpl::GetAllAdapters()
{
    CHECK_AND_RETURN_RET_LOG((audioMgr_ != nullptr), ERR_INVALID_HANDLE,
        "Audio manager is null, audioMgrType %{public}d.", audioMgrType_);
    int32_t ret = audioMgr_->GetAllAdapters(descriptors_);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_NOT_STARTED, "Get adapters failed");
    CHECK_AND_RETURN_RET_LOG(descriptors_.data() != nullptr, ERR_OPERATION_FAILED,
        "Get target adapters descriptor fail.");
    return SUCCESS;
}

struct AudioAdapterDescriptor *AudioDeviceManagerImpl::GetTargetAdapterDesc(const std::string &adapterName,
    bool isMmap)
{
    (void) isMmap;
    int32_t ret = GetAllAdapters();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, nullptr,
        "Get all adapters fail, audioMgrType %{public}d, ret %{public}d.", audioMgrType_, ret);

    int32_t targetIdx = INVALID_INDEX;
    for (uint32_t index = 0; index < descriptors_.size(); index++) {
        struct AudioAdapterDescriptor desc = descriptors_[index];
        if (desc.adapterName.c_str() == nullptr) {
            continue;
        }
        if (adapterName.compare(desc.adapterName)) {
            AUDIO_INFO_LOG("[%{public}d] is not target adapter", index);
            continue;
        }
        targetIdx = static_cast<int32_t>(index);
    }
    CHECK_AND_RETURN_RET_LOG((targetIdx >= 0), nullptr, "can not find target adapter.");
    return &descriptors_[targetIdx];
}

std::shared_ptr<IAudioDeviceAdapter> AudioDeviceManagerImpl::LoadAdapters(const std::string &adapterName, bool isMmap)
{
    struct AudioAdapterDescriptor *desc = GetTargetAdapterDesc(adapterName, isMmap);
    CHECK_AND_RETURN_RET_LOG(desc != nullptr, nullptr, "can not find target adapter.");

    std::lock_guard<std::mutex> lock(mtx_);
    if (enableAdapters_.find(std::string(desc->adapterName)) != enableAdapters_.end()) {
        AUDIO_INFO_LOG("LoadAdapters: Audio adapter already inited.");
        return enableAdapters_[std::string(desc->adapterName)].devAdp;
    }

    CHECK_AND_RETURN_RET_LOG((audioMgr_ != nullptr), nullptr,
        "LoadAdapters: Audio manager is null, audioMgrType %{public}d.", audioMgrType_);
    sptr<IAudioAdapter> audioAdapter = nullptr;
    AudioAdapterDescriptor descriptor = {
        .adapterName = desc->adapterName,
    };
    int32_t ret = audioMgr_->LoadAdapter(descriptor, audioAdapter);
    CHECK_AND_RETURN_RET_LOG(ret == 0 && audioAdapter != nullptr,
        nullptr, "Load audio adapter fail, audioMgrType %{public}d, ret %{public}d.", audioMgrType_, ret);
    auto audioDevAdp = std::make_shared<AudioDeviceAdapterImpl>(std::string(desc->adapterName), audioAdapter);
    audioDevAdp->SetParamCallback(audioDevAdp);
    ret = audioDevAdp->Init();
    CHECK_AND_RETURN_RET_LOG((ret == SUCCESS), nullptr, "LoadAdapters: Init all ports fail, ret %{public}d.", ret);

    DeviceAdapterInfo adpInfo = {audioDevAdp, audioAdapter};
    enableAdapters_[std::string(desc->adapterName)] = adpInfo;
    AUDIO_DEBUG_LOG("Load adapter end.");
    return audioDevAdp;
}

int32_t AudioDeviceManagerImpl::UnloadAdapter(const std::string &adapterName)
{
    AUDIO_INFO_LOG("Unload adapter, audioMgrType %{public}d.", audioMgrType_);
    std::lock_guard<std::mutex> lock(mtx_);
    if (enableAdapters_.find(adapterName) == enableAdapters_.end()) {
        AUDIO_INFO_LOG("Adapter is already unloaded.");
        return SUCCESS;
    }

    auto adpInfo = enableAdapters_[adapterName];
    CHECK_AND_RETURN_RET_LOG((audioMgr_ != nullptr && adpInfo.audioAdapter != nullptr), ERR_INVALID_HANDLE,
        "UnloadAdapter: Audio manager or audio adapter is null, audioMgrType %{public}d.", audioMgrType_);
    if (adpInfo.devAdp != nullptr) {
        int32_t ret = adpInfo.devAdp->Release();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Adapter release fail, ret %{public}d.", ret);
        AUDIO_DEBUG_LOG("Device adapter release OK, audioMgrType %{public}d.", audioMgrType_);
    }

    audioMgr_->UnloadAdapter(adapterName);
    enableAdapters_.erase(adapterName);
    return SUCCESS;
}

int32_t AudioDeviceManagerImpl::Release()
{
    AUDIO_INFO_LOG("Release enter.");
    std::lock_guard<std::mutex> lock(mtx_);
    CHECK_AND_RETURN_RET_LOG(enableAdapters_.empty(), ERR_ILLEGAL_STATE,
        "Audio manager has some adapters busy, audioMgrType %{public}d.", audioMgrType_);

    audioMgr_ = nullptr;
    return SUCCESS;
}
}  // namespace AudioStandard
}  // namespace OHOS