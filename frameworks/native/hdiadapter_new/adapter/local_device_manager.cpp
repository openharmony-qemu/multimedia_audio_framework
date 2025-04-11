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
#define LOG_TAG "LocalDeviceManager"
#endif

#include "adapter/local_device_manager.h"
#include "audio_hdi_log.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace AudioStandard {
int32_t LocalDeviceManager::LoadAdapter(const std::string &adapterName)
{
    CHECK_AND_RETURN_RET_LOG(adapters_.count(adapterName) == 0 || adapters_[adapterName] == nullptr, SUCCESS,
        "adapter %{public}s already loaded", adapterName.c_str());

    if (audioManager_ == nullptr) {
        InitAudioManager();
    }
    CHECK_AND_RETURN_RET(audioManager_ != nullptr, ERR_INVALID_HANDLE);

    struct AudioAdapterDescriptor descs[MAX_AUDIO_ADAPTER_NUM];
    uint32_t size = MAX_AUDIO_ADAPTER_NUM;
    int32_t ret = audioManager_->GetAllAdapters(audioManager_, (struct AudioAdapterDescriptor *)&descs, &size);
    CHECK_AND_RETURN_RET_LOG(size <= MAX_AUDIO_ADAPTER_NUM && size != 0 && ret == SUCCESS, ERR_NOT_STARTED,
        "get adapters fail");
    int32_t index = SwitchAdapterDesc((struct AudioAdapterDescriptor *)&descs, adapterName, size);
    CHECK_AND_RETURN_RET(index >= 0, ERR_NOT_STARTED);

    struct IAudioAdapter *adapter = nullptr;
    ret = audioManager_->LoadAdapter(audioManager_, &(descs[index]), &adapter);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS && adapter != nullptr, ERR_NOT_STARTED, "load adapter fail");
    ret = adapter->InitAllPorts(adapter);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "init all ports fail");
    std::lock_guard<std::mutex> lock(adapterMtx_);
    adapters_[adapterName] = std::make_shared<LocalAdapterWrapper>();
    adapters_[adapterName]->adapterDesc_ = descs[index];
    adapters_[adapterName]->adapter_ = adapter;
    // LCOV_EXCL_START
    for (auto it = reSetParams_.begin(); it != reSetParams_.end();) {
        if (it->adapterName_ == adapterName) {
            SetAudioParameter(adapterName, it->key_, it->condition_, it->value_);
            it = reSetParams_.erase(it);
            continue;
        }
        ++it;
    }
    // LCOV_EXCL_STOP
    AUDIO_INFO_LOG("load adapter %{public}s success", adapterName.c_str());
    return SUCCESS;
}

void LocalDeviceManager::UnloadAdapter(const std::string &adapterName, bool force)
{
    CHECK_AND_RETURN_LOG(audioManager_ != nullptr, "audio manager is nullptr");

    std::shared_ptr<LocalAdapterWrapper> wrapper = GetAdapter(adapterName);
    CHECK_AND_RETURN_LOG(wrapper != nullptr && wrapper->adapter_ != nullptr, "adapter %{public}s is nullptr",
        adapterName.c_str());
    CHECK_AND_RETURN_LOG(force || (wrapper->hdiRenderIds_.size() == 0 && wrapper->hdiCaptureIds_.size() == 0),
        "adapter %{public}s has some ports busy, renderNum: %{public}zu, captureNum: %{public}zu", adapterName.c_str(),
        wrapper->hdiRenderIds_.size(), wrapper->hdiCaptureIds_.size());

    if (wrapper->routeHandle_ != -1) {
        wrapper->adapter_->ReleaseAudioRoute(wrapper->adapter_, wrapper->routeHandle_);
    }
    audioManager_->UnloadAdapter(audioManager_, wrapper->adapterDesc_.adapterName);
    std::lock_guard<std::mutex> lock(adapterMtx_);
    adapters_[adapterName].reset();
    adapters_.erase(adapterName);
    AUDIO_INFO_LOG("unload adapter %{public}s success", adapterName.c_str());
}

void LocalDeviceManager::AllAdapterSetMicMute(bool isMute)
{
    AUDIO_INFO_LOG("isMute: %{public}s", isMute ? "true" : "false");

    std::lock_guard<std::mutex> lock(adapterMtx_);
    for (auto &item : adapters_) {
        std::shared_ptr<LocalAdapterWrapper> wrapper = item.second;
        if (wrapper == nullptr || wrapper->adapter_ == nullptr) {
            continue;
        }
        int32_t ret = wrapper->adapter_->SetMicMute(wrapper->adapter_, isMute);
        if (ret != SUCCESS) {
            AUDIO_WARNING_LOG("set mute fail, adapterName: %{public}s", item.first.c_str());
        } else {
            AUDIO_INFO_LOG("set mute success, adapterName: %{public}s", item.first.c_str());
        }
    }
}

void LocalDeviceManager::SetAudioParameter(const std::string &adapterName, const AudioParamKey key,
    const std::string &condition, const std::string &value)
{
    AUDIO_INFO_LOG("key: %{public}d, condition: %{public}s, value: %{public}s", key, condition.c_str(), value.c_str());

    std::shared_ptr<LocalAdapterWrapper> wrapper = GetAdapter(adapterName);
    // LCOV_EXCL_START
    if (wrapper == nullptr || wrapper->adapter_ == nullptr) {
        AUDIO_ERR_LOG("adapter %{public}s is nullptr", adapterName.c_str());
        SaveSetParameter(adapterName, key, condition, value);
        return;
    }
    // LCOV_EXCL_STOP
    AudioExtParamKey hdiKey = AudioExtParamKey(key);
    int32_t ret = wrapper->adapter_->SetExtraParams(wrapper->adapter_, hdiKey, condition.c_str(), value.c_str());
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "set param fail, error code: %{public}d", ret);
}

std::string LocalDeviceManager::GetAudioParameter(const std::string &adapterName, const AudioParamKey key,
    const std::string &condition)
{
    AUDIO_INFO_LOG("key: %{public}d, condition: %{public}s", key, condition.c_str());

    std::shared_ptr<LocalAdapterWrapper> wrapper = GetAdapter(adapterName);
    CHECK_AND_RETURN_RET_LOG(wrapper != nullptr && wrapper->adapter_ != nullptr, "", "adapter %{public}s is nullptr",
        adapterName.c_str());
    AudioExtParamKey hdiKey = AudioExtParamKey(key);
    char value[DumpFileUtil::PARAM_VALUE_LENTH];
    int32_t ret = wrapper->adapter_->GetExtraParams(wrapper->adapter_, hdiKey, condition.c_str(), value,
        DumpFileUtil::PARAM_VALUE_LENTH);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, "", "get param fail, error code: %{public}d", ret);
    return value;
}

int32_t LocalDeviceManager::SetVoiceVolume(const std::string &adapterName, float volume)
{
    AUDIO_INFO_LOG("set modem call, volume: %{public}f", volume);

    Trace trace("LocalDeviceManager::SetVoiceVolume");
    std::shared_ptr<LocalAdapterWrapper> wrapper = GetAdapter(adapterName);
    CHECK_AND_RETURN_RET_LOG(wrapper != nullptr && wrapper->adapter_ != nullptr, ERR_INVALID_HANDLE,
        "adapter %{public}s is nullptr", adapterName.c_str());
    return wrapper->adapter_->SetVoiceVolume(wrapper->adapter_, volume);
}

int32_t LocalDeviceManager::SetOutputRoute(const std::string &adapterName, const std::vector<DeviceType> &devices,
    int32_t streamId)
{
    CHECK_AND_RETURN_RET_LOG(!devices.empty() && devices.size() <= AUDIO_CONCURRENT_ACTIVE_DEVICES_LIMIT,
        ERR_INVALID_PARAM, "invalid audio devices");

    Trace trace("LocalDeviceManager::SetOutputRoute device " + std::to_string(devices[0]));
    AudioRouteNode source = {
        .portId = 0,
        .role = AUDIO_PORT_SOURCE_ROLE,
        .type = AUDIO_PORT_MIX_TYPE,
        .ext.mix.moduleId = 0,
        .ext.mix.streamId = streamId,
        .ext.device.desc = (char *)"",
    };
    AudioRouteNode sinks[devices.size()];
    for (size_t i = 0; i < devices.size(); ++i) {
        sinks[i] = {};
        int32_t ret = SetOutputPortPin(devices[i], sinks[i]);
        CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
        AUDIO_INFO_LOG("output[%{public}zu], pin: 0x%{public}X", i, sinks[i].ext.device.type);
        sinks[i].portId = static_cast<int32_t>(GetPortId(adapterName, PORT_OUT));
        sinks[i].role = AUDIO_PORT_SINK_ROLE;
        sinks[i].type = AUDIO_PORT_DEVICE_TYPE;
        sinks[i].ext.device.moduleId = 0;
        sinks[i].ext.device.desc = (char *)"";
    }
    AudioRoute route = {
        .sources = &source,
        .sourcesLen = 1,
        .sinks = sinks,
        .sinksLen = devices.size(),
    };

    int64_t stamp = ClockTime::GetCurNano();
    std::shared_ptr<LocalAdapterWrapper> wrapper = GetAdapter(adapterName);
    CHECK_AND_RETURN_RET_LOG(wrapper != nullptr && wrapper->adapter_ != nullptr, ERR_INVALID_HANDLE,
        "adapter %{public}s is nullptr", adapterName.c_str());
    int32_t ret = wrapper->adapter_->UpdateAudioRoute(wrapper->adapter_, &route, &(wrapper->routeHandle_));
    stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
    AUDIO_INFO_LOG("update route, device: %{public}d, cost: [%{public}" PRId64 "]ms", devices[0], stamp);

    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "update route fail");
    return SUCCESS;
}

int32_t LocalDeviceManager::SetInputRoute(const std::string &adapterName, DeviceType device, int32_t streamId,
    int32_t inputType)
{
    AudioRouteNode source = {};
    int32_t ret = SetInputPortPin(device, source);
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
    AUDIO_INFO_LOG("input, pin: 0x%{public}X", source.ext.device.type);
    source.portId = static_cast<int32_t>(GetPortId(adapterName, PORT_IN));
    source.role = AUDIO_PORT_SOURCE_ROLE;
    source.type = AUDIO_PORT_DEVICE_TYPE;
    source.ext.mix.moduleId = 0;
    source.ext.device.desc = (char *)"";
    AudioRouteNode sink = {
        .portId = 0,
        .role = AUDIO_PORT_SINK_ROLE,
        .type = AUDIO_PORT_MIX_TYPE,
        .ext.mix.moduleId = 0,
        .ext.mix.streamId = streamId,
        .ext.mix.source = inputType,
        .ext.device.desc = (char *)"",
    };
    AudioRoute route = {
        .sources = &source,
        .sourcesLen = 1,
        .sinks = &sink,
        .sinksLen = 1,
    };

    int64_t stamp = ClockTime::GetCurNano();
    std::shared_ptr<LocalAdapterWrapper> wrapper = GetAdapter(adapterName);
    CHECK_AND_RETURN_RET_LOG(wrapper != nullptr && wrapper->adapter_ != nullptr, ERR_INVALID_HANDLE,
        "adapter %{public}s is nullptr", adapterName.c_str());
    ret = wrapper->adapter_->UpdateAudioRoute(wrapper->adapter_, &route, &(wrapper->routeHandle_));
    stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
    AUDIO_INFO_LOG("update route, device: %{public}d, cost: [%{public}" PRId64 "]ms", device, stamp);

    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "update route fail");
    return SUCCESS;
}

void LocalDeviceManager::SetMicMute(const std::string &adapterName, bool isMute)
{
    AUDIO_INFO_LOG("isMute: %{public}s", isMute ? "true" : "false");

    std::shared_ptr<LocalAdapterWrapper> wrapper = GetAdapter(adapterName);
    CHECK_AND_RETURN_LOG(wrapper != nullptr && wrapper->adapter_ != nullptr, "adapter %{public}s is nullptr",
        adapterName.c_str());
    int32_t ret = wrapper->adapter_->SetMicMute(wrapper->adapter_, isMute);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("set mute fail");
    } else {
        AUDIO_INFO_LOG("set mute success");
    }
}

void *LocalDeviceManager::CreateRender(const std::string &adapterName, void *param, void *deviceDesc,
    uint32_t &hdiRenderId)
{
    CHECK_AND_RETURN_RET_LOG(param != nullptr && deviceDesc != nullptr, nullptr, "param or deviceDesc is nullptr");
    struct AudioSampleAttributes *localParam = static_cast<struct AudioSampleAttributes *>(param);
    struct AudioDeviceDescriptor *localDeviceDesc = static_cast<struct AudioDeviceDescriptor *>(deviceDesc);

    std::shared_ptr<LocalAdapterWrapper> wrapper = GetAdapter(adapterName, true);
    CHECK_AND_RETURN_RET_LOG(wrapper != nullptr && wrapper->adapter_ != nullptr, nullptr,
        "adapter %{public}s is nullptr", adapterName.c_str());
    localDeviceDesc->portId = GetPortId(adapterName, PORT_OUT);

    struct IAudioRender *render = nullptr;
    int32_t ret = wrapper->adapter_->CreateRender(wrapper->adapter_, localDeviceDesc, localParam, &render,
        &hdiRenderId);
    if (ret != SUCCESS || render == nullptr) {
        AUDIO_ERR_LOG("create render fail");
        UnloadAdapter(adapterName);
        return nullptr;
    }
    AUDIO_INFO_LOG("create render success, hdiRenderId: %{public}u, desc: %{public}s", hdiRenderId,
        localDeviceDesc->desc);

    std::lock_guard<std::mutex> lock(wrapper->renderMtx_);
    wrapper->hdiRenderIds_.insert(hdiRenderId);
    return render;
}

void LocalDeviceManager::DestroyRender(const std::string &adapterName, uint32_t hdiRenderId)
{
    AUDIO_INFO_LOG("destroy render, hdiRenderId: %{public}u", hdiRenderId);

    std::shared_ptr<LocalAdapterWrapper> wrapper = GetAdapter(adapterName);
    CHECK_AND_RETURN_LOG(wrapper != nullptr && wrapper->adapter_ != nullptr, "adapter %{public}s is nullptr",
        adapterName.c_str());
    CHECK_AND_RETURN_LOG(wrapper->hdiRenderIds_.count(hdiRenderId) != 0, "render not exist");
    wrapper->adapter_->DestroyRender(wrapper->adapter_, hdiRenderId);

    std::lock_guard<std::mutex> lock(wrapper->renderMtx_);
    wrapper->hdiRenderIds_.erase(hdiRenderId);
}

void *LocalDeviceManager::CreateCapture(const std::string &adapterName, void *param, void *deviceDesc,
    uint32_t &hdiCaptureId)
{
    CHECK_AND_RETURN_RET_LOG(param != nullptr && deviceDesc != nullptr, nullptr, "param or deviceDesc is nullptr");
    struct AudioSampleAttributes *localParam = static_cast<struct AudioSampleAttributes *>(param);
    struct AudioDeviceDescriptor *localDeviceDesc = static_cast<struct AudioDeviceDescriptor *>(deviceDesc);

    std::shared_ptr<LocalAdapterWrapper> wrapper = GetAdapter(adapterName, true);
    CHECK_AND_RETURN_RET_LOG(wrapper != nullptr && wrapper->adapter_ != nullptr, nullptr,
        "adapter %{public}s is nullptr", adapterName.c_str());
    localDeviceDesc->portId = GetPortId(adapterName, PORT_IN);

    struct IAudioCapture *capture = nullptr;
    int32_t ret = wrapper->adapter_->CreateCapture(wrapper->adapter_, localDeviceDesc, localParam, &capture,
        &hdiCaptureId);
    if (ret != SUCCESS || capture == nullptr) {
        AUDIO_ERR_LOG("create capture fail");
        UnloadAdapter(adapterName);
        return nullptr;
    }
    AUDIO_INFO_LOG("create capture success, hdiCaptureId: %{public}u, desc: %{public}s", hdiCaptureId,
        localDeviceDesc->desc);

    std::lock_guard<std::mutex> lock(wrapper->captureMtx_);
    wrapper->hdiCaptureIds_.insert(hdiCaptureId);
    return capture;
}

void LocalDeviceManager::DestroyCapture(const std::string &adapterName, uint32_t hdiCaptureId)
{
    AUDIO_INFO_LOG("destroy capture, hdiCaptureId: %{public}u", hdiCaptureId);

    std::shared_ptr<LocalAdapterWrapper> wrapper = GetAdapter(adapterName);
    CHECK_AND_RETURN_LOG(wrapper != nullptr && wrapper->adapter_ != nullptr, "adapter %{public}s is nullptr",
        adapterName.c_str());
    CHECK_AND_RETURN_LOG(wrapper->hdiCaptureIds_.count(hdiCaptureId) != 0, "capture not exist");
    wrapper->adapter_->DestroyCapture(wrapper->adapter_, hdiCaptureId);

    std::lock_guard<std::mutex> lock(wrapper->captureMtx_);
    wrapper->hdiCaptureIds_.erase(hdiCaptureId);
}

void LocalDeviceManager::DumpInfo(std::string &dumpString)
{
    for (auto &item : adapters_) {
        uint32_t renderNum = item.second == nullptr ? 0 : item.second->hdiRenderIds_.size();
        uint32_t captureNum = item.second == nullptr ? 0 : item.second->hdiCaptureIds_.size();
        dumpString += "  - local/" + item.first + "\trenderNum: " + std::to_string(renderNum) + "\tcaptureNum: " +
            std::to_string(captureNum) + "\n";
    }
}

static void AudioHostOnRemoteDied(struct HdfDeathRecipient *recipent, struct HdfRemoteService *service)
{
    CHECK_AND_RETURN_LOG(recipent != nullptr && service != nullptr, "receive die message but params are nullptr");
    AUDIO_ERR_LOG("auto exit for audio host die");
    _Exit(0);
}

void LocalDeviceManager::InitAudioManager(void)
{
    CHECK_AND_RETURN_LOG(audioManager_ == nullptr, "audio manager already inited");
    AUDIO_INFO_LOG("init audio manager");
    audioManager_ = IAudioManagerGet(false);
    CHECK_AND_RETURN_LOG(audioManager_ != nullptr, "get audio manager fail");

    CHECK_AND_RETURN_LOG(hdfRemoteService_ == nullptr, "hdf remote service already inited");
    hdfRemoteService_ = audioManager_->AsObject(audioManager_);
    // Don't need to free, existing with process
    hdfDeathRecipient_ = (struct HdfDeathRecipient *)calloc(1, sizeof(*hdfDeathRecipient_));
    hdfDeathRecipient_->OnRemoteDied = AudioHostOnRemoteDied;
    HdfRemoteServiceAddDeathRecipient(hdfRemoteService_, hdfDeathRecipient_);

    AUDIO_INFO_LOG("init audio manager succ");
}

std::shared_ptr<LocalAdapterWrapper> LocalDeviceManager::GetAdapter(const std::string &adapterName, bool tryCreate)
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

int32_t LocalDeviceManager::SwitchAdapterDesc(struct AudioAdapterDescriptor *descs, const std::string &adapterName,
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

uint32_t LocalDeviceManager::GetPortId(const std::string &adapterName, enum AudioPortDirection portFlag)
{
    std::shared_ptr<LocalAdapterWrapper> wrapper = GetAdapter(adapterName);
    CHECK_AND_RETURN_RET_LOG(wrapper != nullptr && wrapper->adapter_ != nullptr, 0,
        "adapter %{public}s is nullptr", adapterName.c_str());
    struct AudioAdapterDescriptor &desc = wrapper->adapterDesc_;
    uint32_t portId = 0;
    for (uint32_t port = 0; port < desc.portsLen; ++port) {
        if (desc.ports[port].dir == portFlag) {
            portId = desc.ports[port].portId;
            break;
        }
    }
    AUDIO_DEBUG_LOG("portId: %{public}u", portId);
    return portId;
}

int32_t LocalDeviceManager::SetOutputPortPin(DeviceType outputDevice, AudioRouteNode &sink)
{
    int32_t ret = SUCCESS;

    switch (outputDevice) {
        case DEVICE_TYPE_EARPIECE:
            sink.ext.device.type = PIN_OUT_EARPIECE;
            sink.ext.device.desc = (char *)"pin_out_earpiece";
            break;
        case DEVICE_TYPE_SPEAKER:
            sink.ext.device.type = PIN_OUT_SPEAKER;
            sink.ext.device.desc = (char *)"pin_out_speaker";
            break;
        case DEVICE_TYPE_WIRED_HEADSET:
            sink.ext.device.type = PIN_OUT_HEADSET;
            sink.ext.device.desc = (char *)"pin_out_headset";
            break;
        case DEVICE_TYPE_USB_ARM_HEADSET:
            sink.ext.device.type = PIN_OUT_USB_HEADSET;
            sink.ext.device.desc = (char *)"pin_out_usb_headset";
            break;
        case DEVICE_TYPE_USB_HEADSET:
            sink.ext.device.type = PIN_OUT_USB_EXT;
            sink.ext.device.desc = (char *)"pin_out_usb_ext";
            break;
        case DEVICE_TYPE_BLUETOOTH_SCO:
            sink.ext.device.type = PIN_OUT_BLUETOOTH_SCO;
            sink.ext.device.desc = (char *)"pin_out_bluetooth_sco";
            break;
        case DEVICE_TYPE_BLUETOOTH_A2DP:
            sink.ext.device.type = PIN_OUT_BLUETOOTH_A2DP;
            sink.ext.device.desc = (char *)"pin_out_bluetooth_a2dp";
            break;
        case DEVICE_TYPE_NONE:
            sink.ext.device.type = PIN_NONE;
            sink.ext.device.desc = (char *)"pin_out_none";
            break;
        default:
            ret = ERR_NOT_SUPPORTED;
            break;
    }

    return ret;
}

int32_t LocalDeviceManager::SetInputPortPin(DeviceType inputDevice, AudioRouteNode &source)
{
    int32_t ret = SUCCESS;

    switch (inputDevice) {
        case DEVICE_TYPE_MIC:
        case DEVICE_TYPE_EARPIECE:
        case DEVICE_TYPE_SPEAKER:
        case DEVICE_TYPE_BLUETOOTH_A2DP_IN:
            source.ext.device.type = PIN_IN_MIC;
            source.ext.device.desc = (char *)"pin_in_mic";
            break;
        case DEVICE_TYPE_WIRED_HEADSET:
            source.ext.device.type = PIN_IN_HS_MIC;
            source.ext.device.desc = (char *)"pin_in_hs_mic";
            break;
        case DEVICE_TYPE_USB_ARM_HEADSET:
            source.ext.device.type = PIN_IN_USB_HEADSET;
            source.ext.device.desc = (char *)"pin_in_usb_headset";
            break;
        case DEVICE_TYPE_USB_HEADSET:
            source.ext.device.type = PIN_IN_USB_EXT;
            source.ext.device.desc = (char *)"pin_in_usb_ext";
            break;
        case DEVICE_TYPE_BLUETOOTH_SCO:
            source.ext.device.type = PIN_IN_BLUETOOTH_SCO_HEADSET;
            source.ext.device.desc = (char *)"pin_in_bluetooth_sco_headset";
            break;
        default:
            ret = ERR_NOT_SUPPORTED;
            break;
    }

    return ret;
}

void LocalDeviceManager::SaveSetParameter(const std::string &adapterName, const AudioParamKey key,
    const std::string &condition, const std::string &value)
{
    // save set param
    auto callerUid = IPCSkeleton::GetCallingUid();
    AUDIO_INFO_LOG("save param when adapter is nullptr, callerUid is %{public}u", callerUid);
    reSetParams_.push_back({ adapterName, key, condition, value });
}

} // namespace AudioStandard
} // namespace OHOS
