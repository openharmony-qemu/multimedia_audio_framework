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

#include "va_device_controller_registry.h"

#include "audio_log.h"

namespace OHOS {
namespace AudioStandard {

VADeviceControllerRegistry &VADeviceControllerRegistry::GetInstance()
{
    static VADeviceControllerRegistry registry;
    return registry;
}

void VADeviceControllerRegistry::RegisterController(const std::string &address,
    const sptr<IVADeviceController> &controller)
{
    if (address.empty() || controller == nullptr) {
        AUDIO_WARNING_LOG("register controller ignored, addressEmpty=%{public}d, controllerNull=%{public}d",
            address.empty(), controller == nullptr);
        return;
    }
    sptr<IRemoteObject> object = controller->AsObject();
    {
        std::lock_guard<std::mutex> lock(mutex_);
        controllerMap_[address] = controller;
    }
    if (object == nullptr) {
        AUDIO_WARNING_LOG("controller object is null, addressLen=%{public}zu", address.size());
        return;
    }
    sptr<VADeviceControllerDeathRecipient> recipient = new (std::nothrow) VADeviceControllerDeathRecipient(
        [address]() { VADeviceControllerRegistry::GetInstance().UnregisterController(address); });
    if (recipient == nullptr) {
        AUDIO_WARNING_LOG("create death recipient failed, addressLen=%{public}zu", address.size());
        return;
    }
    if (!object->AddDeathRecipient(recipient)) {
        AUDIO_WARNING_LOG("AddDeathRecipient failed, addressLen=%{public}zu", address.size());
    }
}

void VADeviceControllerRegistry::UnregisterController(const std::string &address)
{
    if (address.empty()) {
        AUDIO_WARNING_LOG("unregister controller ignored, address is empty");
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    controllerMap_.erase(address);
    hasProcessInfo_ = false;
}

sptr<IVADeviceController> VADeviceControllerRegistry::GetController(const std::string &address)
{
    if (address.empty()) {
        AUDIO_WARNING_LOG("get controller failed, address is empty");
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    auto iter = controllerMap_.find(address);
    if (iter == controllerMap_.end()) {
        AUDIO_INFO_LOG("get controller miss, addressLen=%{public}zu, controllerCount=%{public}zu",
            address.size(), controllerMap_.size());
        return nullptr;
    }
    return iter->second;
}

void VADeviceControllerRegistry::SetProcessInfo(pid_t pid, uid_t uid, const std::string &bundleName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    processPid_ = pid;
    processUid_ = uid;
    processBundleName_ = bundleName;
    hasProcessInfo_ = true;
}

bool VADeviceControllerRegistry::GetProcessInfo(pid_t &pid, uid_t &uid, std::string &bundleName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!hasProcessInfo_) {
        AUDIO_INFO_LOG("get process info miss, no process info stored");
        return false;
    }
    pid = processPid_;
    uid = processUid_;
    bundleName = processBundleName_;
    return true;
}

void VADeviceControllerRegistry::RemoveProcessInfo()
{
    std::lock_guard<std::mutex> lock(mutex_);
    hasProcessInfo_ = false;
}

} // namespace AudioStandard
} // namespace OHOS
