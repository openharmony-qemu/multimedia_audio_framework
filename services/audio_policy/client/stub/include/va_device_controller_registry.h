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

#ifndef VA_DEVICE_CONTROLLER_REGISTRY_H
#define VA_DEVICE_CONTROLLER_REGISTRY_H

#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <sys/types.h>

#include "iremote_object.h"
#include "iv_a_device_controller.h"

namespace OHOS {
namespace AudioStandard {

class VADeviceControllerDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    explicit VADeviceControllerDeathRecipient(std::function<void()> callback) : callback_(std::move(callback)) {}
    ~VADeviceControllerDeathRecipient() override = default;
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override
    {
        if (callback_) {
            callback_();
        }
    }
private:
    std::function<void()> callback_;
};

class VADeviceControllerRegistry {
public:
    static VADeviceControllerRegistry &GetInstance();

    void RegisterController(const std::string &address,
        const sptr<IVADeviceController> &controller);
    void UnregisterController(const std::string &address);
    sptr<IVADeviceController> GetController(const std::string &address);
    void SetProcessInfo(pid_t pid, uid_t uid, const std::string &bundleName);
    bool GetProcessInfo(pid_t &pid, uid_t &uid, std::string &bundleName);
    void RemoveProcessInfo();

private:
    VADeviceControllerRegistry() = default;
    ~VADeviceControllerRegistry() = default;

    std::mutex mutex_;
    std::unordered_map<std::string, sptr<IVADeviceController>> controllerMap_;
    pid_t processPid_ = 0;
    uid_t processUid_ = 0;
    std::string processBundleName_;
    bool hasProcessInfo_ = false;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // VA_DEVICE_CONTROLLER_REGISTRY_H
