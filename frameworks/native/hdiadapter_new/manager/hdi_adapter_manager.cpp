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
#define LOG_TAG "HdiAdapterManager"
#endif

#include "manager/hdi_adapter_manager.h"
#include "audio_hdi_log.h"
#include "audio_errors.h"
#include "util/id_handler.h"
#include "manager/hdi_adapter_factory.h"

namespace OHOS {
namespace AudioStandard {
HdiAdapterManager::~HdiAdapterManager()
{
    renderSinkMtx_.lock();
    renderSinks_.clear();
    renderSinkMtx_.unlock();

    captureSourceMtx_.lock();
    captureSources_.clear();
    captureSourceMtx_.unlock();

    std::lock_guard<std::mutex> lock(deviceManagerMtx_);
    for (uint32_t i = 0; i < HDI_DEVICE_MANAGER_TYPE_NUM; ++i) {
        if (deviceManagers_[i] == nullptr) {
            continue;
        }
        deviceManagers_[i].reset();
    }
}

HdiAdapterManager &HdiAdapterManager::GetInstance(void)
{
    static HdiAdapterManager instance;
    return instance;
}

std::shared_ptr<IDeviceManager> HdiAdapterManager::GetDeviceManager(HdiDeviceManagerType type)
{
    CHECK_AND_RETURN_RET_LOG(type < HDI_DEVICE_MANAGER_TYPE_NUM, nullptr, "invalid type");

    std::lock_guard<std::mutex> lock(deviceManagerMtx_);
    if (deviceManagers_[type] == nullptr) {
        HdiAdapterFactory &fac = HdiAdapterFactory::GetInstance();
        deviceManagers_[type] = fac.CreateDeviceManager(type);
    }
    return deviceManagers_[type];
}

void HdiAdapterManager::ReleaseDeviceManager(HdiDeviceManagerType type)
{
    CHECK_AND_RETURN_LOG(type < HDI_DEVICE_MANAGER_TYPE_NUM, "invalid type");

    std::lock_guard<std::mutex> lock(deviceManagerMtx_);
    if (deviceManagers_[type] == nullptr) {
        return;
    }
    deviceManagers_[type].reset();
}

uint32_t HdiAdapterManager::GetId(HdiIdBase base, HdiIdType type, const std::string &info, bool isResident)
{
    uint32_t id = IdHandler::GetInstance().GetId(base, type, info);
    CHECK_AND_RETURN_RET(id != HDI_INVALID_ID, HDI_INVALID_ID);
    CHECK_AND_RETURN_RET(isResident, id);
    std::unique_lock<std::mutex> renderLock(renderSinkMtx_);
    std::unique_lock<std::mutex> captureLock(captureSourceMtx_);
    if (renderSinks_.count(id) == 0 && captureSources_.count(id) == 0) {
        IdHandler::GetInstance().IncInfoIdUseCount(id);
    }
    AUDIO_INFO_LOG("base: %{public}u, type: %{public}u, info: %{public}s, id: %{public}u", base, type, info.c_str(),
        id);
    IncRefCount(id);
    return id;
}

uint32_t HdiAdapterManager::GetRenderIdByDeviceClass(const std::string &deviceClass, const std::string &info,
    bool isResident)
{
    uint32_t id = IdHandler::GetInstance().GetRenderIdByDeviceClass(deviceClass, info);
    AUDIO_INFO_LOG("Device class: %{public}s, info: %{public}s, id: %{public}u",
        deviceClass.c_str(), info.c_str(), id);
    CHECK_AND_RETURN_RET(id != HDI_INVALID_ID, HDI_INVALID_ID);
    CHECK_AND_RETURN_RET(isResident, id);
    std::unique_lock<std::mutex> renderLock(renderSinkMtx_);
    std::unique_lock<std::mutex> captureLock(captureSourceMtx_);
    if (renderSinks_.count(id) == 0 && captureSources_.count(id) == 0) {
        IdHandler::GetInstance().IncInfoIdUseCount(id);
    }
    IncRefCount(id);
    return id;
}

uint32_t HdiAdapterManager::GetCaptureIdByDeviceClass(const std::string &deviceClass, const SourceType sourceType,
    const std::string &info, bool isResident)
{
    uint32_t id = IdHandler::GetInstance().GetCaptureIdByDeviceClass(deviceClass, sourceType, info);
    AUDIO_INFO_LOG("Device class: %{public}s, sourceType: %{public}d, info: %{public}s, id: %{public}u",
        deviceClass.c_str(), sourceType, info.c_str(), id);
    CHECK_AND_RETURN_RET(id != HDI_INVALID_ID, HDI_INVALID_ID);
    CHECK_AND_RETURN_RET(isResident, id);
    std::unique_lock<std::mutex> renderLock(renderSinkMtx_);
    std::unique_lock<std::mutex> captureLock(captureSourceMtx_);
    if (renderSinks_.count(id) == 0 && captureSources_.count(id) == 0) {
        IdHandler::GetInstance().IncInfoIdUseCount(id);
    }
    IncRefCount(id);
    return id;
}

void HdiAdapterManager::ReleaseId(uint32_t &id)
{
    uint32_t tempId = id;
    id = HDI_INVALID_ID;
    std::unique_lock<std::mutex> renderLock(renderSinkMtx_);
    std::unique_lock<std::mutex> captureLock(captureSourceMtx_);
    CHECK_AND_RETURN(tempId != HDI_INVALID_ID && (renderSinks_.count(tempId) || captureSources_.count(tempId)));
    DecRefCount(tempId);
}

std::shared_ptr<IAudioRenderSink> HdiAdapterManager::GetRenderSink(uint32_t renderId, bool tryCreate)
{
    CHECK_AND_RETURN_RET(IdHandler::GetInstance().CheckId(renderId, HDI_ID_BASE_RENDER), nullptr);

    std::lock_guard<std::mutex> lock(renderSinkMtx_);
    if (renderSinks_.count(renderId) != 0 && renderSinks_[renderId].sink_ != nullptr) {
        return renderSinks_[renderId].sink_;
    }
    if (!tryCreate) {
        AUDIO_ERR_LOG("no available sink, renderId: %{public}u", renderId);
        return nullptr;
    }
    AUDIO_INFO_LOG("create sink, renderId: %{public}u", renderId);
    HdiAdapterFactory &fac = HdiAdapterFactory::GetInstance();
    std::shared_ptr<IAudioRenderSink> renderSink = fac.CreateRenderSink(renderId);
    if (renderSink == nullptr) {
        AUDIO_ERR_LOG("create sink fail, renderId: %{public}u", renderId);
        return nullptr;
    }
    DoRegistSinkCallback(renderId, renderSink);
    DoSetSinkPrestoreInfo(renderSink);
    renderSinks_[renderId].sink_ = renderSink;
    return renderSinks_[renderId].sink_;
}

std::shared_ptr<IAudioCaptureSource> HdiAdapterManager::GetCaptureSource(uint32_t captureId, bool tryCreate)
{
    CHECK_AND_RETURN_RET(IdHandler::GetInstance().CheckId(captureId, HDI_ID_BASE_CAPTURE), nullptr);

    std::lock_guard<std::mutex> lock(captureSourceMtx_);
    if (captureSources_.count(captureId) != 0 && captureSources_[captureId].source_ != nullptr) {
        return captureSources_[captureId].source_;
    }
    if (!tryCreate) {
        AUDIO_ERR_LOG("no available source, captureId: %{public}u", captureId);
        return nullptr;
    }
    AUDIO_INFO_LOG("create source, captureId: %{public}u", captureId);
    HdiAdapterFactory &fac = HdiAdapterFactory::GetInstance();
    std::shared_ptr<IAudioCaptureSource> captureSource = fac.CreateCaptureSource(captureId);
    if (captureSource == nullptr) {
        AUDIO_ERR_LOG("create source fail, captureId: %{public}u", captureId);
        return nullptr;
    }
    DoRegistSourceCallback(captureId, captureSource);
    captureSources_[captureId].source_ = captureSource;
    return captureSources_[captureId].source_;
}

int32_t HdiAdapterManager::LoadAdapter(HdiDeviceManagerType type, const std::string &adapterName)
{
    std::shared_ptr<IDeviceManager> deviceManager = GetDeviceManager(type);
    CHECK_AND_RETURN_RET(deviceManager != nullptr, ERR_INVALID_HANDLE);
    return deviceManager->LoadAdapter(adapterName);
}

void HdiAdapterManager::UnloadAdapter(HdiDeviceManagerType type, const std::string &adapterName, bool force)
{
    std::shared_ptr<IDeviceManager> deviceManager = GetDeviceManager(type);
    CHECK_AND_RETURN(deviceManager != nullptr);
    deviceManager->UnloadAdapter(adapterName, force);
}

int32_t HdiAdapterManager::ProcessSink(const std::function<int32_t(uint32_t,
    std::shared_ptr<IAudioRenderSink>)> &processFunc)
{
    int32_t ret = SUCCESS;
    auto func = [&ret, &processFunc](const std::pair<const uint32_t, RenderSinkInfo> &item) -> void {
        uint32_t renderId = item.first;
        if (processFunc(renderId, item.second.sink_) != SUCCESS) {
            AUDIO_ERR_LOG("process render sink fail, renderId: %{public}u", renderId);
            ret = ERR_OPERATION_FAILED;
        }
    };
    std::lock_guard<std::mutex> lock(renderSinkMtx_);
    std::for_each(renderSinks_.begin(), renderSinks_.end(), func);
    return ret;
}

int32_t HdiAdapterManager::ProcessSource(const std::function<int32_t(uint32_t,
    std::shared_ptr<IAudioCaptureSource>)> &processFunc)
{
    int32_t ret = SUCCESS;
    auto func = [&ret, &processFunc](const std::pair<const uint32_t, CaptureSourceInfo> &item) -> void {
        uint32_t captureId = item.first;
        if (processFunc(captureId, item.second.source_) != SUCCESS) {
            AUDIO_ERR_LOG("process capture source fail, captureId: %{public}u", captureId);
            ret = ERR_OPERATION_FAILED;
        }
    };
    std::lock_guard<std::mutex> lock(captureSourceMtx_);
    std::for_each(captureSources_.begin(), captureSources_.end(), func);
    return ret;
}

void HdiAdapterManager::RegistSinkCallback(HdiAdapterCallbackType type, std::shared_ptr<IAudioSinkCallback> cb,
    const std::function<bool(uint32_t)> &limitFunc)
{
    CHECK_AND_RETURN_LOG(cb != nullptr, "callback of type %{public}u is nullptr", type);

    sinkCbs_.RegistCallback(type, cb);
    cbLimitFunc_[HDI_ID_BASE_RENDER][type] = limitFunc;
    AUDIO_INFO_LOG("regist sink callback succ, type: %{public}u", type);
}

void HdiAdapterManager::RegistSinkCallback(HdiAdapterCallbackType type, IAudioSinkCallback *cb,
    const std::function<bool(uint32_t)> &limitFunc)
{
    CHECK_AND_RETURN_LOG(cb != nullptr, "callback of type %{public}u is nullptr", type);

    sinkCbs_.RegistCallback(type, cb);
    cbLimitFunc_[HDI_ID_BASE_RENDER][type] = limitFunc;
    AUDIO_INFO_LOG("regist sink callback succ, type: %{public}u", type);
}

void HdiAdapterManager::RegistSinkCallbackGenerator(HdiAdapterCallbackType type,
    const std::function<std::shared_ptr<IAudioSinkCallback>(uint32_t)> cbGenerator,
    const std::function<bool(uint32_t)> &limitFunc)
{
    CHECK_AND_RETURN_LOG(cbGenerator, "callback generator of type %{public}u is nullptr", type);

    sinkCbs_.RegistCallbackGenerator(type, cbGenerator);
    cbLimitFunc_[HDI_ID_BASE_RENDER][type] = limitFunc;
    AUDIO_INFO_LOG("regist sink callback generator succ, type: %{public}u", type);
}

void HdiAdapterManager::RegistSourceCallback(HdiAdapterCallbackType type, std::shared_ptr<IAudioSourceCallback> cb,
    const std::function<bool(uint32_t)> &limitFunc)
{
    CHECK_AND_RETURN_LOG(cb != nullptr, "callback of type %{public}u is nullptr", type);

    sourceCbs_.RegistCallback(type, cb);
    cbLimitFunc_[HDI_ID_BASE_CAPTURE][type] = limitFunc;
    AUDIO_INFO_LOG("regist source callback succ, type: %{public}u", type);
}

void HdiAdapterManager::RegistSourceCallback(HdiAdapterCallbackType type, IAudioSourceCallback *cb,
    const std::function<bool(uint32_t)> &limitFunc)
{
    CHECK_AND_RETURN_LOG(cb != nullptr, "callback of type %{public}u is nullptr", type);

    sourceCbs_.RegistCallback(type, cb);
    cbLimitFunc_[HDI_ID_BASE_CAPTURE][type] = limitFunc;
    AUDIO_INFO_LOG("regist source callback succ, type: %{public}u", type);
}

void HdiAdapterManager::RegistSourceCallbackGenerator(HdiAdapterCallbackType type,
    const std::function<std::shared_ptr<IAudioSourceCallback>(uint32_t)> cbGenerator,
    const std::function<bool(uint32_t)> &limitFunc)
{
    CHECK_AND_RETURN_LOG(cbGenerator, "callback generator of type %{public}u is nullptr", type);

    sourceCbs_.RegistCallbackGenerator(type, cbGenerator);
    cbLimitFunc_[HDI_ID_BASE_CAPTURE][type] = limitFunc;
    AUDIO_INFO_LOG("regist source callback generator succ, type: %{public}u", type);
}

void HdiAdapterManager::DumpInfo(std::string &dumpString)
{
    dumpString += "- adapter\n";
    deviceManagerMtx_.lock();
    for (auto &item : deviceManagers_) {
        if (item != nullptr) {
            item->DumpInfo(dumpString);
        }
    }
    deviceManagerMtx_.unlock();

    if (!renderSinks_.empty()) {
        dumpString += "\n- render\n";
        renderSinkMtx_.lock();
        for (auto &item : renderSinks_) {
            if (item.second.sink_ == nullptr ||  !item.second.sink_->IsInited()) {
                continue;
            }
            dumpString += "  - id: " + std::to_string(item.first) + "\trefCount: " +
                std::to_string(item.second.refCount_.load()) + "\t";
            item.second.sink_->DumpInfo(dumpString);
        }
        renderSinkMtx_.unlock();
    }

    if (!captureSources_.empty()) {
        dumpString += "\n- capture\n";
        captureSourceMtx_.lock();
        for (auto &item : captureSources_) {
            if (item.second.source_ == nullptr || !item.second.source_->IsInited()) {
                continue;
            }
            dumpString += "  - id: " + std::to_string(item.first) + "\trefCount: " +
                std::to_string(item.second.refCount_.load()) + "\t";
            item.second.source_->DumpInfo(dumpString);
        }
        captureSourceMtx_.unlock();
    }
}

void HdiAdapterManager::IncRefCount(uint32_t id)
{
    uint32_t base = IdHandler::GetInstance().ParseBase(id);
    if (base == HDI_ID_BASE_RENDER) {
        renderSinks_[id].refCount_++;
    } else {
        captureSources_[id].refCount_++;
    }
}

void HdiAdapterManager::DecRefCount(uint32_t id)
{
    uint32_t base = IdHandler::GetInstance().ParseBase(id);
    if (base == HDI_ID_BASE_RENDER) {
        if (renderSinks_[id].refCount_.load() > 0) {
            renderSinks_[id].refCount_--;
            if (renderSinks_[id].refCount_.load() > 0) {
                return;
            }
        }
        AUDIO_INFO_LOG("no reference of id %{public}u, try remove the sink", id);
        renderSinks_[id].sink_.reset();
        renderSinks_.erase(id);
        IdHandler::GetInstance().DecInfoIdUseCount(id);
    } else {
        if (captureSources_[id].refCount_.load() > 0) {
            captureSources_[id].refCount_--;
            if (captureSources_[id].refCount_.load() > 0) {
                return;
            }
        }
        AUDIO_INFO_LOG("no reference of id %{public}u, try remove the source", id);
        captureSources_[id].source_.reset();
        captureSources_.erase(id);
        IdHandler::GetInstance().DecInfoIdUseCount(id);
    }
}

void HdiAdapterManager::DoRegistSinkCallback(uint32_t id, std::shared_ptr<IAudioRenderSink> sink)
{
    CHECK_AND_RETURN_LOG(sink != nullptr, "sink is nullptr");

    for (uint32_t type = 0; type < HDI_CB_TYPE_NUM; ++type) {
        auto cb = sinkCbs_.GetCallback(type, id);
        auto rawCb = sinkCbs_.GetRawCallback(type);
        if (cbLimitFunc_[HDI_ID_BASE_RENDER][type] == nullptr || !cbLimitFunc_[HDI_ID_BASE_RENDER][type](id)) {
            continue;
        }
        if (cb != nullptr) {
            sink->RegistCallback(type, cb);
        } else if (rawCb != nullptr) {
            sink->RegistCallback(type, rawCb);
        } else {
            AUDIO_ERR_LOG("callback is nullptr, callback type: %{public}u", type);
        }
    }
}

void HdiAdapterManager::DoRegistSourceCallback(uint32_t id, std::shared_ptr<IAudioCaptureSource> source)
{
    CHECK_AND_RETURN_LOG(source != nullptr, "source is nullptr");

    for (uint32_t type = 0; type < HDI_CB_TYPE_NUM; ++type) {
        auto cb = sourceCbs_.GetCallback(type, id);
        auto rawCb = sourceCbs_.GetRawCallback(type);
        if (cbLimitFunc_[HDI_ID_BASE_CAPTURE][type] == nullptr || !cbLimitFunc_[HDI_ID_BASE_CAPTURE][type](id)) {
            continue;
        }
        if (cb != nullptr) {
            source->RegistCallback(type, cb);
        } else if (rawCb != nullptr) {
            source->RegistCallback(type, rawCb);
        } else {
            AUDIO_ERR_LOG("callback is nullptr, callback type: %{public}u", type);
        }
    }
}

void HdiAdapterManager::DoSetSinkPrestoreInfo(std::shared_ptr<IAudioRenderSink> sink)
{
    float audioBalance = 0.0;
    int32_t ret = sinkPrestoreInfo_.Get(PRESTORE_INFO_AUDIO_BALANCE, audioBalance);
    if (ret == SUCCESS) {
        sink->SetAudioBalanceValue(audioBalance);
    } else {
        AUDIO_WARNING_LOG("get %s fail", PRESTORE_INFO_AUDIO_BALANCE);
    }

    bool audioMono = false;
    ret = sinkPrestoreInfo_.Get(PRESTORE_INFO_AUDIO_MONO, audioMono);
    if (ret == SUCCESS) {
        sink->SetAudioMonoState(audioMono);
    } else {
        AUDIO_WARNING_LOG("get %s fail", PRESTORE_INFO_AUDIO_MONO);
    }
}

} // namespace AudioStandard
} // namespace OHOS
