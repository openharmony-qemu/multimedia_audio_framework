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

#ifndef CORE_SERVICE_PROVIDER_PROXY_H
#define CORE_SERVICE_PROVIDER_PROXY_H

#include "i_core_service_provider_ipc.h"

namespace OHOS {
namespace AudioStandard {
class CoreServiceProviderProxy : public IRemoteProxy<ICoreServiceProviderIpc> {
public:
    explicit CoreServiceProviderProxy(const sptr<IRemoteObject>& impl);
    virtual ~CoreServiceProviderProxy();

    int32_t UpdateSessionOperation(uint32_t sessionId, SessionOperation operation, SessionOperationMsg opMsg) override;
    int32_t SetDefaultOutputDevice(const DeviceType defaultOutputDevice,
        const uint32_t sessionID, const StreamUsage streamUsage, bool isRunning) override;
    std::string GetAdapterNameBySessionId(uint32_t sessionId) override;
    int32_t GetProcessDeviceInfoBySessionId(uint32_t sessionId, AudioDeviceDescriptor &deviceInfo) override;
    uint32_t GenerateSessionId() override;

private:
    static inline BrokerDelegator<CoreServiceProviderProxy> delegator_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // CORE_SERVICE_PROVIDER_PROXY_H
