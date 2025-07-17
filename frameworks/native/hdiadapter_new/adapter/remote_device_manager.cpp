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
#define LOG_TAG "RemoteDeviceManager"
#endif

#include "adapter/remote_device_manager.h"
#include "manager/hdi_monitor.h"
#include "audio_hdi_log.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "manager/hdi_adapter_manager.h"

using namespace OHOS::HDI::DistributedAudio::Audio::V1_0;

namespace OHOS {
namespace AudioStandard {
RemoteAdapterHdiCallback::RemoteAdapterHdiCallback(const std::string &adapterName)
    : adapterName_(adapterName)
{
}

int32_t RemoteAdapterHdiCallback::RenderCallback(AudioCallbackType type, int8_t &reserved, int8_t &cookie)
{
    (void)type;
    (void)reserved;
    (void)cookie;
    return SUCCESS;
}

int32_t RemoteAdapterHdiCallback::ParamCallback(AudioExtParamKey key, const std::string &condition,
    const std::string &value, int8_t &reserved, int8_t cookie)
{
    (void)cookie;
    AUDIO_INFO_LOG("key: %{public}d, condition: %{public}s, value: %{public}s", key, condition.c_str(), value.c_str());
    AudioParamKey audioKey = AudioParamKey(key);
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_REMOTE);
    CHECK_AND_RETURN_RET(deviceManager != nullptr, ERR_INVALID_HANDLE);
    deviceManager->HandleEvent(adapterName_, audioKey, condition.c_str(), value.c_str(),
        static_cast<void *>(&reserved));
    return SUCCESS;
}

int32_t RemoteDeviceManager::LoadAdapter(const std::string &adapterName)
{
    CHECK_AND_RETURN_RET_LOG(adapters_.count(adapterName) == 0 || adapters_[adapterName] == nullptr, SUCCESS,
        "adapter %{public}s already loaded", adapterName.c_str());

    std::lock_guard<std::mutex> mgrLock(managerMtx_);
    if (audioManager_ == nullptr || adapters_.size() == 0) {
        audioManager_ = nullptr;
        InitAudioManager();
    }
    CHECK_AND_RETURN_RET(audioManager_ != nullptr, ERR_INVALID_HANDLE);

    std::vector<AudioAdapterDescriptor> descs;
    int32_t ret = audioManager_->GetAllAdapters(descs);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS && descs.data() != nullptr, ERR_NOT_STARTED, "get adapters fail");
    int32_t index = SwitchAdapterDesc(descs, adapterName);
    CHECK_AND_RETURN_RET(index >= 0, ERR_NOT_STARTED);

    sptr<IAudioAdapter> adapter = nullptr;
    AudioAdapterDescriptor desc = {
        .adapterName = descs[index].adapterName,
    };
    ret = audioManager_->LoadAdapter(desc, adapter);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS && adapter != nullptr, ERR_NOT_STARTED, "load adapter fail");
    ret = adapter->InitAllPorts();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "init all ports fail");
    std::lock_guard<std::mutex> lock(adapterMtx_);
    adapters_[adapterName] = std::make_shared<RemoteAdapterWrapper>(adapterName);
    adapters_[adapterName]->adapterDesc_ = descs[index];
    adapters_[adapterName]->adapter_ = adapter;
    AUDIO_INFO_LOG("load adapter %{public}s success", adapterName.c_str());
#ifdef FEATURE_DISTRIBUTE_AUDIO
    adapters_[adapterName]->hdiCallback_ = new RemoteAdapterHdiCallback(adapterName);
    ret = adapter->RegExtraParamObserver(adapters_[adapterName]->hdiCallback_, 0);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "regist extra param observer fail, ret: %{public}d", ret);
#endif
    return SUCCESS;
}

void RemoteDeviceManager::UnloadAdapter(const std::string &adapterName, bool force)
{
    std::lock_guard<std::mutex> mgrLock(managerMtx_);
    CHECK_AND_RETURN_LOG(audioManager_ != nullptr, "audio manager is nullptr");

    std::shared_ptr<RemoteAdapterWrapper> wrapper = GetAdapter(adapterName);
    CHECK_AND_RETURN_LOG(wrapper != nullptr, "adapter %{public}s is nullptr", adapterName.c_str());
    std::unique_lock<std::mutex> innerLock(wrapper->adapterMtx_);
    CHECK_AND_RETURN_LOG(wrapper->adapter_ != nullptr, "adapter %{public}s is nullptr", adapterName.c_str());
    CHECK_AND_RETURN_LOG(force || (wrapper->hdiRenderIds_.size() == 0 && wrapper->hdiCaptureIds_.size() == 0),
        "adapter %{public}s has some ports busy, renderNum: %{public}zu, captureNum: %{public}zu", adapterName.c_str(),
        wrapper->hdiRenderIds_.size(), wrapper->hdiCaptureIds_.size());

    if (wrapper->routeHandle_ != -1) {
        wrapper->adapter_->ReleaseAudioRoute(wrapper->routeHandle_);
    }
    audioManager_->UnloadAdapter(wrapper->adapterDesc_.adapterName);
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

void RemoteDeviceManager::AllAdapterSetMicMute(bool isMute)
{
    AUDIO_INFO_LOG("not support");
}

void RemoteDeviceManager::SetAudioParameter(const std::string &adapterName, const AudioParamKey key,
    const std::string &condition, const std::string &value)
{
#ifdef FEATURE_DISTRIBUTE_AUDIO
    AUDIO_INFO_LOG("key: %{public}d, condition: %{public}s, value: %{public}s", key, condition.c_str(), value.c_str());

    std::shared_ptr<RemoteAdapterWrapper> wrapper = GetAdapter(adapterName);
    CHECK_AND_RETURN_LOG(wrapper != nullptr && wrapper->adapter_ != nullptr, "adapter %{public}s is nullptr",
        adapterName.c_str());
    AudioExtParamKey hdiKey = AudioExtParamKey(key);
    int32_t ret = wrapper->adapter_->SetExtraParams(hdiKey, condition, value);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "set param fail, error code: %{public}d", ret);
#else
    AUDIO_INFO_LOG("not support");
#endif
}

std::string RemoteDeviceManager::GetAudioParameter(const std::string &adapterName, const AudioParamKey key,
    const std::string &condition)
{
#ifdef FEATURE_DISTRIBUTE_AUDIO
    AUDIO_INFO_LOG("key: %{public}d, condition: %{public}s", key, condition.c_str());

    std::shared_ptr<RemoteAdapterWrapper> wrapper = GetAdapter(adapterName);
    CHECK_AND_RETURN_RET_LOG(wrapper != nullptr && wrapper->adapter_ != nullptr, "", "adapter %{public}s is nullptr",
        adapterName.c_str());
    AudioExtParamKey hdiKey = AudioExtParamKey(key);
    std::string value;
    int32_t ret = wrapper->adapter_->GetExtraParams(hdiKey, condition.c_str(), value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, "", "get param fail, error code: %{public}d", ret);
    return value;
#else
    AUDIO_INFO_LOG("not support");
    return "";
#endif
}

int32_t RemoteDeviceManager::SetVoiceVolume(const std::string &adapterName, float volume)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t RemoteDeviceManager::SetOutputRoute(const std::string &adapterName, const std::vector<DeviceType> &devices,
    int32_t streamId)
{
    CHECK_AND_RETURN_RET_LOG(!devices.empty(), ERR_INVALID_PARAM, "invalid audio devices");
    DeviceType device = devices[0];

    AudioRouteNode source = {
        .portId = 0,
        .role = AudioPortRole::AUDIO_PORT_SOURCE_ROLE,
        .type = AudioPortType::AUDIO_PORT_MIX_TYPE,
        .ext.mix.moduleId = 0,
        .ext.mix.streamId = streamId,
    };
    AudioRouteNode sink = {};
    int32_t ret = SetOutputPortPin(device, sink);
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
    AUDIO_INFO_LOG("output, pin: 0x%{public}X", sink.ext.device.type);
    sink.portId = static_cast<int32_t>(GetPortId(PORT_OUT));
    sink.role = AudioPortRole::AUDIO_PORT_SINK_ROLE;
    sink.type = AudioPortType::AUDIO_PORT_DEVICE_TYPE;
    sink.ext.device.moduleId = 0;
    AudioRoute route;
    route.sources.push_back(source);
    route.sinks.push_back(sink);

    std::shared_ptr<RemoteAdapterWrapper> wrapper = GetAdapter(adapterName);
    CHECK_AND_RETURN_RET_LOG(wrapper != nullptr && wrapper->adapter_ != nullptr, ERR_INVALID_HANDLE,
        "adapter %{public}s is nullptr", adapterName.c_str());
    ret = wrapper->adapter_->UpdateAudioRoute(route, wrapper->routeHandle_);

    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "update route fail");
    return SUCCESS;
}

int32_t RemoteDeviceManager::SetInputRoute(const std::string &adapterName, DeviceType device, int32_t streamId,
    int32_t inputType)
{
    (void)inputType;

    AudioRouteNode source = {};
    int32_t ret = SetInputPortPin(device, source);
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
    AUDIO_INFO_LOG("input, pin: 0x%{public}X", source.ext.device.type);
    source.portId = static_cast<int32_t>(GetPortId(PORT_IN));
    source.role = AudioPortRole::AUDIO_PORT_SOURCE_ROLE;
    source.type = AudioPortType::AUDIO_PORT_DEVICE_TYPE;
    source.ext.device.moduleId = 0;
    AudioRouteNode sink = {
        .portId = 0,
        .role = AudioPortRole::AUDIO_PORT_SINK_ROLE,
        .type = AudioPortType::AUDIO_PORT_MIX_TYPE,
        .ext.mix.moduleId = 0,
        .ext.mix.streamId = streamId,
    };
    AudioRoute route;
    route.sources.push_back(source);
    route.sinks.push_back(sink);

    std::shared_ptr<RemoteAdapterWrapper> wrapper = GetAdapter(adapterName);
    CHECK_AND_RETURN_RET_LOG(wrapper != nullptr && wrapper->adapter_ != nullptr, ERR_INVALID_HANDLE,
        "adapter %{public}s is nullptr", adapterName.c_str());
    ret = wrapper->adapter_->UpdateAudioRoute(route, wrapper->routeHandle_);

    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "update route fail");
    return SUCCESS;
}

void RemoteDeviceManager::SetMicMute(const std::string &adapterName, bool isMute)
{
    AUDIO_INFO_LOG("not support");
}

int32_t RemoteDeviceManager::HandleEvent(const std::string &adapterName, const AudioParamKey key, const char *condition,
    const char *value, void *reserved)
{
    AUDIO_INFO_LOG("key: %{public}d, condition: %{public}s, value: %{public}s", key, condition, value);
    int32_t ret = SUCCESS;
    switch (key) {
        case AudioParamKey::PARAM_KEY_STATE:
            ret = HandleStateChangeEvent(adapterName, key, condition, value);
            break;
        case AudioParamKey::VOLUME:
        case AudioParamKey::INTERRUPT:
            ret = HandleRenderParamEvent(adapterName, key, condition, value);
            break;
        default:
            AUDIO_ERR_LOG("not support, key: %{public}d", key);
            return ERR_NOT_SUPPORTED;
    }
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "handle fail, key: %{public}d, error code: %{public}d", key, ret);
    return SUCCESS;
}

void RemoteDeviceManager::RegistRenderSinkCallback(const std::string &adapterName, uint32_t hdiRenderId,
    IDeviceManagerCallback *callback)
{
    std::shared_ptr<RemoteAdapterWrapper> wrapper = GetAdapter(adapterName, true);
    CHECK_AND_RETURN_LOG(wrapper != nullptr, "adapter %{public}s is nullptr", adapterName.c_str());
    std::lock_guard<std::mutex> lock(wrapper->renderCallbackMtx_);
    CHECK_AND_RETURN_LOG(wrapper->renderCallbacks_.count(hdiRenderId) == 0,
        "callback already existed, hdiRenderId: %{public}u", hdiRenderId);
    wrapper->renderCallbacks_[hdiRenderId] = callback;
}

void RemoteDeviceManager::RegistCaptureSourceCallback(const std::string &adapterName, uint32_t hdiCaptureId,
    IDeviceManagerCallback *callback)
{
    std::shared_ptr<RemoteAdapterWrapper> wrapper = GetAdapter(adapterName, true);
    CHECK_AND_RETURN_LOG(wrapper != nullptr, "adapter %{public}s is nullptr", adapterName.c_str());
    std::lock_guard<std::mutex> lock(wrapper->captureCallbackMtx_);
    CHECK_AND_RETURN_LOG(wrapper->captureCallbacks_.count(hdiCaptureId) == 0,
        "callback already existed, hdiCaptureId: %{public}u", hdiCaptureId);
    wrapper->captureCallbacks_[hdiCaptureId] = callback;
}

void RemoteDeviceManager::UnRegistRenderSinkCallback(const std::string &adapterName, uint32_t hdiRenderId)
{
    std::shared_ptr<RemoteAdapterWrapper> wrapper = GetAdapter(adapterName);
    CHECK_AND_RETURN_LOG(wrapper != nullptr, "adapter %{public}s is nullptr", adapterName.c_str());
    std::lock_guard<std::mutex> lock(wrapper->renderCallbackMtx_);
    CHECK_AND_RETURN_LOG(wrapper->renderCallbacks_.count(hdiRenderId) != 0,
        "callback not exist, hdiRenderId: %{public}u", hdiRenderId);
    wrapper->renderCallbacks_.erase(hdiRenderId);
}

void RemoteDeviceManager::UnRegistCaptureSourceCallback(const std::string &adapterName, uint32_t hdiCaptureId)
{
    std::shared_ptr<RemoteAdapterWrapper> wrapper = GetAdapter(adapterName);
    CHECK_AND_RETURN_LOG(wrapper != nullptr, "adapter %{public}s is nullptr", adapterName.c_str());
    std::lock_guard<std::mutex> lock(wrapper->captureCallbackMtx_);
    CHECK_AND_RETURN_LOG(wrapper->captureCallbacks_.count(hdiCaptureId) != 0,
        "callback not exist, hdiCaptureId: %{public}u", hdiCaptureId);
    wrapper->captureCallbacks_.erase(hdiCaptureId);
}

void *RemoteDeviceManager::CreateRender(const std::string &adapterName, void *param, void *deviceDesc,
    uint32_t &hdiRenderId)
{
    CHECK_AND_RETURN_RET_LOG(param != nullptr && deviceDesc != nullptr, nullptr, "param or deviceDesc is nullptr");
    AudioSampleAttributes &remoteParam = *(static_cast<struct AudioSampleAttributes *>(param));
    AudioDeviceDescriptor &remoteDeviceDesc = *(static_cast<struct AudioDeviceDescriptor *>(deviceDesc));

    std::shared_ptr<RemoteAdapterWrapper> wrapper = GetAdapter(adapterName, true);
    CHECK_AND_RETURN_RET_LOG(wrapper != nullptr && wrapper->adapter_ != nullptr, nullptr,
        "adapter %{public}s is nullptr", adapterName.c_str());
    remoteDeviceDesc.portId = GetPortId(PORT_OUT);

    sptr<IAudioRender> render = nullptr;
    int32_t ret = wrapper->adapter_->CreateRender(remoteDeviceDesc, remoteParam, render, hdiRenderId);
    if (ret != SUCCESS || render == nullptr) {
        AUDIO_ERR_LOG("create render fail");
        wrapper->isValid_ = false;
        HdiMonitor::ReportHdiException(HdiType::REMOTE, ErrorCase::CALL_HDI_FAILED, ret, (adapterName +
            " create render fail, id:" + std::to_string(hdiRenderId)));
        return nullptr;
    }
    IAudioRender *rawRender = render.GetRefPtr();
    render.ForceSetRefPtr(nullptr);
    AUDIO_INFO_LOG("create render success, hdiRenderId: %{public}u, desc: %{public}s", hdiRenderId,
        remoteDeviceDesc.desc.c_str());

    std::lock_guard<std::mutex> lock(wrapper->renderMtx_);
    wrapper->hdiRenderIds_.insert(hdiRenderId);
    return rawRender;
}

void RemoteDeviceManager::DestroyRender(const std::string &adapterName, uint32_t hdiRenderId)
{
    AUDIO_INFO_LOG("destroy render, hdiRenderId: %{public}u", hdiRenderId);

    std::shared_ptr<RemoteAdapterWrapper> wrapper = GetAdapter(adapterName);
    CHECK_AND_RETURN_LOG(wrapper != nullptr && wrapper->adapter_ != nullptr, "adapter %{public}s is nullptr",
        adapterName.c_str());
    if (wrapper->hdiRenderIds_.count(hdiRenderId) == 0) {
        AUDIO_ERR_LOG("render not exist");
        if (!wrapper->isValid_) {
            UnloadAdapter(adapterName);
        }
        return;
    }
    wrapper->adapter_->DestroyRender(hdiRenderId);

    std::lock_guard<std::mutex> lock(wrapper->renderMtx_);
    wrapper->hdiRenderIds_.erase(hdiRenderId);
    UnloadAdapter(adapterName);
}

void *RemoteDeviceManager::CreateCapture(const std::string &adapterName, void *param, void *deviceDesc,
    uint32_t &hdiCaptureId)
{
    CHECK_AND_RETURN_RET_LOG(param != nullptr && deviceDesc != nullptr, nullptr, "param or deviceDesc is nullptr");
    AudioSampleAttributes &remoteParam = *(static_cast<struct AudioSampleAttributes *>(param));
    AudioDeviceDescriptor &remoteDeviceDesc = *(static_cast<struct AudioDeviceDescriptor *>(deviceDesc));

    std::shared_ptr<RemoteAdapterWrapper> wrapper = GetAdapter(adapterName, true);
    CHECK_AND_RETURN_RET_LOG(wrapper != nullptr && wrapper->adapter_ != nullptr, nullptr,
        "adapter %{public}s is nullptr", adapterName.c_str());
    remoteDeviceDesc.portId = GetPortId(PORT_IN);

    sptr<IAudioCapture> capture = nullptr;
    int32_t ret = wrapper->adapter_->CreateCapture(remoteDeviceDesc, remoteParam, capture, hdiCaptureId);
    if (ret != SUCCESS || capture == nullptr) {
        AUDIO_ERR_LOG("create capture fail");
        wrapper->isValid_ = false;
        HdiMonitor::ReportHdiException(HdiType::REMOTE, ErrorCase::CALL_HDI_FAILED, ret, (adapterName +
            " create capture fail, id:" + std::to_string(hdiCaptureId)));
        return nullptr;
    }
    IAudioCapture *rawCapture = capture.GetRefPtr();
    capture.ForceSetRefPtr(nullptr);
    AUDIO_INFO_LOG("create capture success, hdiCaptureId: %{public}u, desc: %{public}s", hdiCaptureId,
        remoteDeviceDesc.desc.c_str());

    std::lock_guard<std::mutex> lock(wrapper->captureMtx_);
    wrapper->hdiCaptureIds_.insert(hdiCaptureId);
    return rawCapture;
}

void RemoteDeviceManager::DestroyCapture(const std::string &adapterName, uint32_t hdiCaptureId)
{
    AUDIO_INFO_LOG("destroy capture, hdiCaptureId: %{public}u", hdiCaptureId);

    std::shared_ptr<RemoteAdapterWrapper> wrapper = GetAdapter(adapterName);
    CHECK_AND_RETURN_LOG(wrapper != nullptr && wrapper->adapter_ != nullptr, "adapter %{public}s is nullptr",
        adapterName.c_str());
    if (wrapper->hdiCaptureIds_.count(hdiCaptureId) == 0) {
        AUDIO_ERR_LOG("capture not exist");
        if (!wrapper->isValid_) {
            UnloadAdapter(adapterName);
        }
        return;
    }
    wrapper->adapter_->DestroyCapture(hdiCaptureId);

    std::lock_guard<std::mutex> lock(wrapper->captureMtx_);
    wrapper->hdiCaptureIds_.erase(hdiCaptureId);
    UnloadAdapter(adapterName);
}

void RemoteDeviceManager::DumpInfo(std::string &dumpString)
{
    for (auto &item : adapters_) {
        uint32_t renderNum = item.second == nullptr ? 0 : item.second->hdiRenderIds_.size();
        uint32_t captureNum = item.second == nullptr ? 0 : item.second->hdiCaptureIds_.size();
        dumpString += "  - remote/" + item.first + "\trenderNum: " + std::to_string(renderNum) + "\tcaptureNum: " +
            std::to_string(captureNum) + "\n";
    }
}

void RemoteDeviceManager::InitAudioManager(void)
{
    CHECK_AND_RETURN_LOG(audioManager_ == nullptr, "audio manager already inited");
#ifdef FEATURE_DISTRIBUTE_AUDIO
    AUDIO_INFO_LOG("init audio manager");
    audioManager_ = IAudioManager::Get("daudio_primary_service", false);
    if (audioManager_ == nullptr) {
        HdiMonitor::ReportHdiException(HdiType::REMOTE, ErrorCase::CALL_HDI_FAILED, 0,
            "get hdi manager fail");
    }
    CHECK_AND_RETURN_LOG(audioManager_ != nullptr, "get audio manager fail");

    AUDIO_INFO_LOG("init audio manager succ");
#endif
}

std::shared_ptr<RemoteAdapterWrapper> RemoteDeviceManager::GetAdapter(const std::string &adapterName, bool tryCreate)
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

int32_t RemoteDeviceManager::SwitchAdapterDesc(const std::vector<AudioAdapterDescriptor> &descs,
    const std::string &adapterName)
{
    for (uint32_t index = 0; index < descs.size(); ++index) {
        const AudioAdapterDescriptor &desc = descs[index];
        if (desc.adapterName.c_str() == nullptr) {
            continue;
        }
        AUDIO_DEBUG_LOG("index: %{public}u, adapterName: %{public}s", index, desc.adapterName.c_str());
        if (!adapterName.compare(desc.adapterName)) {
            AUDIO_INFO_LOG("match adapter %{public}s", desc.adapterName.c_str());
            return index;
        }
    }
    AUDIO_ERR_LOG("switch adapter fail, adapterName: %{public}s", adapterName.c_str());
    return ERR_INVALID_INDEX;
}

uint32_t RemoteDeviceManager::GetPortId(enum AudioPortDirection portFlag)
{
    uint32_t portId = 0;
    if (portFlag == PORT_OUT) {
        portId = AudioPortPin::PIN_OUT_SPEAKER;
    } else if (portFlag == PORT_IN) {
        portId = AudioPortPin::PIN_IN_MIC;
    }
    AUDIO_DEBUG_LOG("portId: %{public}u", portId);
    return portId;
}

int32_t RemoteDeviceManager::HandleStateChangeEvent(const std::string &adapterName, const AudioParamKey key,
    const char *condition, const char *value)
{
    char eventDes[EVENT_DES_SIZE];
    char contentDes[ADAPTER_STATE_CONTENT_DES_SIZE];
    CHECK_AND_RETURN_RET_LOG(sscanf_s(condition, "%[^;];%s", eventDes, EVENT_DES_SIZE, contentDes,
        ADAPTER_STATE_CONTENT_DES_SIZE) == PARAMS_STATE_NUM, ERR_INVALID_PARAM, "parse condition fail");
    CHECK_AND_RETURN_RET_LOG(strcmp(eventDes, "ERR_EVENT") == 0, ERR_NOT_SUPPORTED, "not support event %{public}s",
        eventDes);

    std::string devTypeKey = "DEVICE_TYPE=";
    std::string contentDesStr = std::string(contentDes);
    size_t devTypeKeyPos = contentDesStr.find(devTypeKey);
    CHECK_AND_RETURN_RET_LOG(devTypeKeyPos != std::string::npos, ERR_INVALID_PARAM,
        "not find daudio device type info, contentDes: %{public}s", contentDesStr.c_str());
    size_t devTypeValPos = devTypeKeyPos + devTypeKey.length();
    CHECK_AND_RETURN_RET_LOG(devTypeValPos < contentDesStr.length(), ERR_INVALID_PARAM,
        "not find daudio device type value, contentDes: %{public}s", contentDesStr.c_str());

    int32_t ret = SUCCESS;
    if (contentDesStr[devTypeValPos] == DAUDIO_DEV_TYPE_SPK) {
        AUDIO_INFO_LOG("ERR_EVENT is DAUDIO_DEV_TYPE_SPK");
        ret = HandleRenderParamEvent(adapterName, key, condition, value);
    } else if (contentDesStr[devTypeValPos] == DAUDIO_DEV_TYPE_MIC) {
        AUDIO_INFO_LOG("ERR_EVENT is DAUDIO_DEV_TYPE_MIC");
        ret = HandleCaptureParamEvent(adapterName, key, condition, value);
    } else {
        AUDIO_ERR_LOG("not support device type, contentDes: %{public}s", contentDesStr.c_str());
        return ERR_NOT_SUPPORTED;
    }
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "handle event %{public}s fail", contentDesStr.c_str());
    return SUCCESS;
}

int32_t RemoteDeviceManager::HandleRenderParamEvent(const std::string &adapterName, const AudioParamKey key,
    const char *condition, const char *value)
{
    std::shared_ptr<RemoteAdapterWrapper> wrapper = GetAdapter(adapterName);
    CHECK_AND_RETURN_RET_LOG(wrapper != nullptr, ERR_INVALID_HANDLE, "adapter %{public}s is nullptr",
        adapterName.c_str());
    IDeviceManagerCallback *renderCallback = nullptr;
    {
        std::lock_guard<std::mutex> lock(wrapper->renderCallbackMtx_);
        if (wrapper->renderCallbacks_.size() != 1) {
            AUDIO_WARNING_LOG("exist %{public}zu renders port in adapter", wrapper->renderCallbacks_.size());
        }
        for (auto &cb : wrapper->renderCallbacks_) {
            if (cb.second != nullptr) {
                renderCallback = cb.second;
                break;
            }
        }
    }
    CHECK_AND_RETURN_RET_LOG(renderCallback != nullptr, ERR_INVALID_HANDLE, "not find render port in adapter");
    renderCallback->OnAudioParamChange(adapterName, key, std::string(condition), std::string(value));
    return SUCCESS;
}

int32_t RemoteDeviceManager::HandleCaptureParamEvent(const std::string &adapterName, const AudioParamKey key,
    const char *condition, const char *value)
{
    std::shared_ptr<RemoteAdapterWrapper> wrapper = GetAdapter(adapterName);
    CHECK_AND_RETURN_RET_LOG(wrapper != nullptr, ERR_INVALID_HANDLE, "adapter %{public}s is nullptr",
        adapterName.c_str());
    IDeviceManagerCallback *captureCallback = nullptr;
    {
        std::lock_guard<std::mutex> lock(wrapper->captureCallbackMtx_);
        if (wrapper->captureCallbacks_.size() != 1) {
            AUDIO_WARNING_LOG("exist %{public}zu captures port in adapter", wrapper->captureCallbacks_.size());
        }
        for (auto &cb : wrapper->captureCallbacks_) {
            if (cb.second != nullptr) {
                captureCallback = cb.second;
                break;
            }
        }
    }
    CHECK_AND_RETURN_RET_LOG(captureCallback != nullptr, ERR_INVALID_HANDLE, "not find capture port in adapter");
    captureCallback->OnAudioParamChange(adapterName, key, std::string(condition), std::string(value));
    return SUCCESS;
}

int32_t RemoteDeviceManager::SetOutputPortPin(DeviceType outputDevice, AudioRouteNode &sink)
{
    int32_t ret = SUCCESS;

    switch (outputDevice) {
        case DEVICE_TYPE_SPEAKER:
            sink.ext.device.type = AudioPortPin::PIN_OUT_SPEAKER;
            sink.ext.device.desc = "pin_out_speaker";
            break;
        case DEVICE_TYPE_WIRED_HEADSET:
            sink.ext.device.type = AudioPortPin::PIN_OUT_HEADSET;
            sink.ext.device.desc = "pin_out_headset";
            break;
        case DEVICE_TYPE_USB_HEADSET:
            sink.ext.device.type = AudioPortPin::PIN_OUT_USB_EXT;
            sink.ext.device.desc = "pin_out_usb_ext";
            break;
        default:
            ret = ERR_NOT_SUPPORTED;
            break;
    }

    return ret;
}

int32_t RemoteDeviceManager::SetInputPortPin(DeviceType inputDevice, AudioRouteNode &source)
{
    int32_t ret = SUCCESS;

    switch (inputDevice) {
        case DEVICE_TYPE_MIC:
            source.ext.device.type = AudioPortPin::PIN_IN_MIC;
            source.ext.device.desc = "pin_in_mic";
            break;
        case DEVICE_TYPE_WIRED_HEADSET:
            source.ext.device.type = AudioPortPin::PIN_IN_HS_MIC;
            source.ext.device.desc = "pin_in_hs_mic";
            break;
        case DEVICE_TYPE_USB_HEADSET:
            source.ext.device.type = AudioPortPin::PIN_IN_USB_EXT;
            source.ext.device.desc = "pin_in_usb_ext";
            break;
        default:
            ret = ERR_NOT_SUPPORTED;
            break;
    }

    return ret;
}

void RemoteDeviceManager::SetDmDeviceType(uint16_t dmDeviceType, DeviceType deviceType)
{
    AUDIO_INFO_LOG("not support");
}

void RemoteDeviceManager::SetAudioScene(const AudioScene scene)
{
    AUDIO_INFO_LOG("not support");
}
} // namespace AudioStandard
} // namespace OHOS
