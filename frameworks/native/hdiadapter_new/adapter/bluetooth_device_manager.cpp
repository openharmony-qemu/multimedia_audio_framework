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
#define LOG_TAG "BluetoothDeviceManager"
#endif

#include "adapter/bluetooth_device_manager.h"
#include "manager/hdi_monitor.h"
#include <dlfcn.h>
#include "audio_hdi_log.h"
#include "audio_errors.h"
#include "audio_utils.h"

using namespace OHOS::HDI::Audio_Bluetooth;

namespace OHOS {
namespace AudioStandard {
BluetoothDeviceManager::~BluetoothDeviceManager()
{
    if (handle_ != nullptr) {
#ifndef TEST_COVERAGE
        dlclose(handle_);
#endif
        handle_ = nullptr;
    }
}

int32_t BluetoothDeviceManager::LoadAdapter(const std::string &adapterName)
{
    CHECK_AND_RETURN_RET_LOG(adapters_.count(adapterName) == 0 || adapters_[adapterName] == nullptr, SUCCESS,
        "adapter %{public}s already loaded", adapterName.c_str());
    if (audioManager_ == nullptr || adapters_.size() == 0) {
        audioManager_ = nullptr;
        InitAudioManager();
    }
    CHECK_AND_RETURN_RET(audioManager_ != nullptr, ERR_INVALID_HANDLE);

    struct AudioAdapterDescriptor *descs = nullptr;
    int32_t size = 0;
    int32_t ret = audioManager_->GetAllAdapters(audioManager_, &descs, &size);
    CHECK_AND_RETURN_RET_LOG(size <= (int32_t)MAX_AUDIO_ADAPTER_NUM && size != 0 && ret == SUCCESS && descs != nullptr,
        ERR_NOT_STARTED, "get adapters fail");
    int32_t index = SwitchAdapterDesc(descs, adapterName, size);
    CHECK_AND_RETURN_RET(index >= 0, ERR_NOT_STARTED);

    struct AudioAdapter *adapter = nullptr;
    ret = audioManager_->LoadAdapter(audioManager_, &(descs[index]), &adapter);
    if (ret != SUCCESS) {
        HdiMonitor::ReportHdiException(HdiType::A2DP, ErrorCase::CALL_HDI_FAILED, ret, (adapterName +
            " load adapter fail, ret: " + std::to_string(ret)));
    }
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS && adapter != nullptr, ERR_NOT_STARTED, "load adapter fail");
    ret = adapter->InitAllPorts(adapter);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "init all ports fail");
    std::lock_guard<std::mutex> lock(adapterMtx_);
    adapters_[adapterName] = std::make_shared<BluetoothAdapterWrapper>();
    adapters_[adapterName]->adapterDesc_ = descs[index];
    adapters_[adapterName]->adapter_ = adapter;
    AUDIO_INFO_LOG("load adapter %{public}s success", adapterName.c_str());
    return SUCCESS;
}

void BluetoothDeviceManager::UnloadAdapter(const std::string &adapterName, bool force)
{
    CHECK_AND_RETURN_LOG(audioManager_ != nullptr, "audio manager is nullptr");

    std::shared_ptr<BluetoothAdapterWrapper> wrapper = GetAdapter(adapterName);
    CHECK_AND_RETURN_LOG(wrapper != nullptr, "adapter %{public}s is nullptr", adapterName.c_str());
    std::unique_lock<std::mutex> innerLock(wrapper->adapterMtx_);
    CHECK_AND_RETURN_LOG(wrapper->adapter_ != nullptr, "adapter %{public}s is nullptr", adapterName.c_str());
    CHECK_AND_RETURN_LOG(force || (wrapper->renders_.size() == 0 && wrapper->captures_.size() == 0),
        "adapter %{public}s has some ports busy, renderNum: %{public}zu, captureNum: %{public}zu", adapterName.c_str(),
        wrapper->renders_.size(), wrapper->captures_.size());

    audioManager_->UnloadAdapter(audioManager_, wrapper->adapter_);
    wrapper->adapter_ = nullptr;
    innerLock.unlock();
    std::lock_guard<std::mutex> lock(adapterMtx_);
    adapters_[adapterName].reset();
    adapters_.erase(adapterName);
    if (adapters_.size() == 0) {
        audioManager_ = nullptr;
    }
    AUDIO_INFO_LOG("unload adapter %{public}s success", adapterName.c_str());
}

void BluetoothDeviceManager::AllAdapterSetMicMute(bool isMute)
{
    AUDIO_INFO_LOG("not support");
}

void BluetoothDeviceManager::SetAudioParameter(const std::string &adapterName, const AudioParamKey key,
    const std::string &condition, const std::string &value)
{
    AUDIO_INFO_LOG("not support");
}

std::string BluetoothDeviceManager::GetAudioParameter(const std::string &adapterName, const AudioParamKey key,
    const std::string &condition)
{
    AUDIO_INFO_LOG("key: %{public}d, condition: %{public}s", key, condition.c_str());

    std::shared_ptr<BluetoothAdapterWrapper> wrapper = GetAdapter(adapterName);
    CHECK_AND_RETURN_RET_LOG(wrapper != nullptr && wrapper->adapter_ != nullptr, "", "adapter %{public}s is nullptr",
        adapterName.c_str());
    AudioExtParamKey hdiKey = AudioExtParamKey(key);
    char value[DumpFileUtil::PARAM_VALUE_LENTH];
    int32_t ret = wrapper->adapter_->GetExtraParams(wrapper->adapter_, hdiKey, condition.c_str(), value,
        DumpFileUtil::PARAM_VALUE_LENTH);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, "", "get param fail, error code: %{public}d", ret);
    return value;
}

int32_t BluetoothDeviceManager::SetVoiceVolume(const std::string &adapterName, float volume)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t BluetoothDeviceManager::SetOutputRoute(const std::string &adapterName, const std::vector<DeviceType> &devices,
    int32_t streamId)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t BluetoothDeviceManager::SetInputRoute(const std::string &adapterName, DeviceType device, int32_t streamId,
    int32_t inputType)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

void BluetoothDeviceManager::SetMicMute(const std::string &adapterName, bool isMute)
{
    AUDIO_INFO_LOG("not support");
}

void *BluetoothDeviceManager::CreateRender(const std::string &adapterName, void *param, void *deviceDesc,
    uint32_t &hdiRenderId)
{
    CHECK_AND_RETURN_RET_LOG(param != nullptr && deviceDesc != nullptr, nullptr, "param or deviceDesc is nullptr");
    struct AudioSampleAttributes *bluetoothParam = static_cast<struct AudioSampleAttributes *>(param);
    struct AudioDeviceDescriptor *bluetoothDeviceDesc = static_cast<struct AudioDeviceDescriptor *>(deviceDesc);

    std::shared_ptr<BluetoothAdapterWrapper> wrapper = GetAdapter(adapterName, true);
    CHECK_AND_RETURN_RET_LOG(wrapper != nullptr && wrapper->adapter_ != nullptr, nullptr,
        "adapter %{public}s is nullptr", adapterName.c_str());
    bluetoothDeviceDesc->portId = GetPortId(adapterName, PORT_OUT);

    struct AudioRender *render = nullptr;
    int32_t ret = wrapper->adapter_->CreateRender(wrapper->adapter_, bluetoothDeviceDesc, bluetoothParam, &render);
    if (ret != SUCCESS || render == nullptr) {
        AUDIO_ERR_LOG("create render fail");
        HdiMonitor::ReportHdiException(HdiType::A2DP, ErrorCase::CALL_HDI_FAILED, ret, (adapterName +
            " create render fail, id:" + std::to_string(hdiRenderId)));
        UnloadAdapter(adapterName);
        return nullptr;
    }
    AUDIO_INFO_LOG("create render success, desc: %{public}s", bluetoothDeviceDesc->desc);

    std::lock_guard<std::mutex> lock(wrapper->renderMtx_);
    hdiRenderId = GetHdiRenderId(adapterName);
    wrapper->renders_[hdiRenderId] = render;
    return render;
}

void BluetoothDeviceManager::DestroyRender(const std::string &adapterName, uint32_t hdiRenderId)
{
    AUDIO_INFO_LOG("destroy render, hdiRenderId: %{public}u", hdiRenderId);

    std::shared_ptr<BluetoothAdapterWrapper> wrapper = GetAdapter(adapterName);
    CHECK_AND_RETURN_LOG(wrapper != nullptr && wrapper->adapter_ != nullptr, "adapter %{public}s is nullptr",
        adapterName.c_str());
    CHECK_AND_RETURN_LOG(wrapper->renders_.count(hdiRenderId) != 0, "render not exist");
    std::lock_guard<std::mutex> lock(wrapper->renderMtx_);
    wrapper->adapter_->DestroyRender(wrapper->adapter_, wrapper->renders_[hdiRenderId]);
    wrapper->renders_.erase(hdiRenderId);
    wrapper->freeHdiRenderIdSet_.insert(hdiRenderId);
    UnloadAdapter(adapterName);
}

void *BluetoothDeviceManager::CreateCapture(const std::string &adapterName, void *param, void *deviceDesc,
    uint32_t &hdiCaptureId)
{
    CHECK_AND_RETURN_RET_LOG(param != nullptr && deviceDesc != nullptr, nullptr, "param or deviceDesc is nullptr");
    struct AudioSampleAttributes *bluetoothParam = static_cast<struct AudioSampleAttributes *>(param);
    struct AudioDeviceDescriptor *bluetoothDeviceDesc = static_cast<struct AudioDeviceDescriptor *>(deviceDesc);

    std::shared_ptr<BluetoothAdapterWrapper> wrapper = GetAdapter(adapterName, true);
    CHECK_AND_RETURN_RET_LOG(wrapper != nullptr && wrapper->adapter_ != nullptr, nullptr,
        "adapter %{public}s is nullptr", adapterName.c_str());
    bluetoothDeviceDesc->portId = GetPortId(adapterName, PORT_IN);

    struct AudioCapture *capture = nullptr;
    int32_t ret = wrapper->adapter_->CreateCapture(wrapper->adapter_, bluetoothDeviceDesc, bluetoothParam, &capture);
    if (ret != SUCCESS || capture == nullptr) {
        AUDIO_ERR_LOG("create capture fail");
        HdiMonitor::ReportHdiException(HdiType::A2DP, ErrorCase::CALL_HDI_FAILED, ret, (adapterName +
            " create capture fail, id:" + std::to_string(hdiCaptureId)));
        UnloadAdapter(adapterName);
        return nullptr;
    }
    AUDIO_INFO_LOG("create capture success, desc: %{public}s", bluetoothDeviceDesc->desc);

    std::lock_guard<std::mutex> lock(wrapper->captureMtx_);
    hdiCaptureId = GetHdiCaptureId(adapterName);
    wrapper->captures_[hdiCaptureId] = capture;
    return capture;
}

void BluetoothDeviceManager::DestroyCapture(const std::string &adapterName, uint32_t hdiCaptureId)
{
    AUDIO_INFO_LOG("destroy capture, hdiCaptureId: %{public}u", hdiCaptureId);

    std::shared_ptr<BluetoothAdapterWrapper> wrapper = GetAdapter(adapterName);
    CHECK_AND_RETURN_LOG(wrapper != nullptr && wrapper->adapter_ != nullptr, "adapter %{public}s is nullptr",
        adapterName.c_str());
    CHECK_AND_RETURN_LOG(wrapper->captures_.count(hdiCaptureId) != 0, "capture not exist");
    std::lock_guard<std::mutex> lock(wrapper->captureMtx_);
    wrapper->adapter_->DestroyCapture(wrapper->adapter_, wrapper->captures_[hdiCaptureId]);
    wrapper->captures_.erase(hdiCaptureId);
    wrapper->freeHdiCaptureIdSet_.insert(hdiCaptureId);
    UnloadAdapter(adapterName);
}

void BluetoothDeviceManager::DumpInfo(std::string &dumpString)
{
    for (auto &item : adapters_) {
        uint32_t renderNum = item.second == nullptr ? 0 : item.second->renders_.size();
        uint32_t captureNum = item.second == nullptr ? 0 : item.second->captures_.size();
        dumpString += "  - bt/" + item.first + "\trenderNum: " + std::to_string(renderNum) + "\tcaptureNum: " +
            std::to_string(captureNum) + "\n";
    }
}

void BluetoothDeviceManager::InitAudioManager(void)
{
    CHECK_AND_RETURN_LOG(audioManager_ == nullptr, "audio manager already inited");
    AUDIO_INFO_LOG("init audio manager");

    char resolvedPath[] = "libaudio_bluetooth_hdi_proxy_server.z.so";
    handle_ = dlopen(resolvedPath, RTLD_LAZY);
    CHECK_AND_RETURN_LOG(handle_ != nullptr, "dlopen %{public}s fail", resolvedPath);
    struct AudioProxyManager *(*getAudioManager)() = nullptr;
    getAudioManager = (struct AudioProxyManager *(*)())(dlsym(handle_, "GetAudioProxyManagerFuncs"));
    CHECK_AND_RETURN_LOG(getAudioManager != nullptr, "dlsym fail");
    audioManager_ = getAudioManager();
    if (audioManager_ == nullptr) {
        HdiMonitor::ReportHdiException(HdiType::A2DP, ErrorCase::CALL_HDI_FAILED, 0,
            "get hdi manager fail");
    }
    CHECK_AND_RETURN_LOG(audioManager_ != nullptr, "get audio manager fail");
    AUDIO_INFO_LOG("init audio manager succ");
}

std::shared_ptr<BluetoothAdapterWrapper> BluetoothDeviceManager::GetAdapter(const std::string &adapterName,
    bool tryCreate)
{
    {
        std::lock_guard<std::mutex> lock(adapterMtx_);
        if (adapters_.count(adapterName) != 0 && adapters_[adapterName] != nullptr) {
            return adapters_[adapterName];
        }
    }
    if (!tryCreate) {
        return nullptr;
    }
    LoadAdapter(adapterName);
    std::lock_guard<std::mutex> lock(adapterMtx_);
    return adapters_.count(adapterName) == 0 ? nullptr : adapters_[adapterName];
}

int32_t BluetoothDeviceManager::SwitchAdapterDesc(struct AudioAdapterDescriptor *descs, const std::string &adapterName,
    uint32_t size)
{
    CHECK_AND_RETURN_RET(descs != nullptr, ERROR);

    for (uint32_t index = 0; index < size; ++index) {
        struct AudioAdapterDescriptor *desc = &descs[index];
        if (desc == nullptr || desc->adapterName == nullptr) {
            continue;
        }
        AUDIO_DEBUG_LOG("index: %{public}u, adapterName: %{public}s", index, desc->adapterName);
        if (!strcmp(desc->adapterName, adapterName.c_str())) {
            AUDIO_INFO_LOG("match adapter %{public}s", desc->adapterName);
            return index;
        }
    }
    AUDIO_ERR_LOG("switch adapter fail, adapterName: %{public}s", adapterName.c_str());
    return ERR_INVALID_INDEX;
}

uint32_t BluetoothDeviceManager::GetPortId(const std::string &adapterName, enum AudioPortDirection portFlag)
{
    std::shared_ptr<BluetoothAdapterWrapper> wrapper = GetAdapter(adapterName);
    CHECK_AND_RETURN_RET_LOG(wrapper != nullptr && wrapper->adapter_ != nullptr, 0, "adapter %{public}s is nullptr",
        adapterName.c_str());
    struct AudioAdapterDescriptor &desc = wrapper->adapterDesc_;
    uint32_t portId = 0;
    for (uint32_t port = 0; port < desc.portNum; ++port) {
        if (desc.ports[port].dir == portFlag) {
            portId = desc.ports[port].portId;
            break;
        }
    }
    AUDIO_DEBUG_LOG("portId: %{public}u", portId);
    return portId;
}

uint32_t BluetoothDeviceManager::GetHdiRenderId(const std::string &adapterName)
{
    std::shared_ptr<BluetoothAdapterWrapper> wrapper = GetAdapter(adapterName);
    CHECK_AND_RETURN_RET_LOG(wrapper != nullptr, 0, "adapter %{public}s is nullptr", adapterName.c_str());

    if (wrapper->freeHdiRenderIdSet_.empty()) {
        return wrapper->renders_.size();
    }
    uint32_t hdiRenderId = *(wrapper->freeHdiRenderIdSet_.begin());
    wrapper->freeHdiRenderIdSet_.erase(hdiRenderId);
    return hdiRenderId;
}

uint32_t BluetoothDeviceManager::GetHdiCaptureId(const std::string &adapterName)
{
    std::shared_ptr<BluetoothAdapterWrapper> wrapper = GetAdapter(adapterName);
    CHECK_AND_RETURN_RET_LOG(wrapper != nullptr, 0, "adapter %{public}s is nullptr", adapterName.c_str());

    if (wrapper->freeHdiCaptureIdSet_.empty()) {
        return wrapper->captures_.size();
    }
    uint32_t hdiCaptureId = *(wrapper->freeHdiCaptureIdSet_.begin());
    wrapper->freeHdiCaptureIdSet_.erase(hdiCaptureId);
    return hdiCaptureId;
}

void BluetoothDeviceManager::SetDmDeviceType(uint16_t dmDeviceType, DeviceType deviceType)
{
    AUDIO_INFO_LOG("not support");
}

void BluetoothDeviceManager::SetAudioScene(const AudioScene scene)
{
    AUDIO_INFO_LOG("not support");
}
} // namespace AudioStandard
} // namespace OHOS
