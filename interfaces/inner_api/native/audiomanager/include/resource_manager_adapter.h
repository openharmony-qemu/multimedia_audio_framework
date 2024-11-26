/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef RESOURCE_MANAGER_ADAPTER_H
#define RESOURCE_MANAGER_ADAPTER_H

#include <mutex>
#include "resource_manager.h"

namespace OHOS {
namespace AudioStandard {

class ResourceManagerAdapter {
public:
    static ResourceManagerAdapter *GetInstance();

    ResourceManagerAdapter();
    ~ResourceManagerAdapter();

    void ReleaseSystemResourceManager();
    std::string GetSystemStringByName(std::string name);
    Global::Resource::RState GetMediaDataByName(std::string name, size_t &len, std::unique_ptr<uint8_t[]> &outValue,
        uint32_t density = 0);
private:
    void InitResourceManager();
    void RefreshResConfig();
    /**
     * Ensure that operations on resourceManager_ are not executed concurrently
     */
    std::mutex resourceManagerMutex_;
    Global::Resource::ResourceManager *resourceManager_ = nullptr;
    Global::Resource::ResConfig *resConfig_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // RESOURCE_MANAGER_ADAPTER_H
