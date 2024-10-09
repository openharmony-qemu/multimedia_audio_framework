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

#ifndef CLIENT_TYPE_MANAGER_H
#define CLIENT_TYPE_MANAGER_H

#include <mutex>
#include <string>
#include <unordered_map>

#include "client_type_manager.h"
#include "client_type_manager_handler.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

class ClientTypeManager : public ClientTypeListener {
public:
    static ClientTypeManager *GetInstance();
    void GetAndSaveClientType(uint32_t uid, const std::string &bundleName);
    ClientType GetClientTypeByUid(uint32_t uid);

    void OnClientTypeQueryCompleted(uint32_t uid, ClientType clientType) override;

    void SetQueryClientTypeCallback(const sptr<IStandardAudioPolicyManagerListener> &callback);

private:
    std::mutex clientTypeMapMutex_;
    std::unordered_map<uint32_t, ClientType> clientTypeMap_;

    std::mutex handlerMutex_;
    std::shared_ptr<ClientTypeManagerHandler> clientTypeManagerHandler_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // CLIENT_TYPE_MANAGER_H
