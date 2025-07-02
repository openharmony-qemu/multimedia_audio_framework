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

#ifndef I_CORE_SERVICE_PROVIDER_IPC_H
#define I_CORE_SERVICE_PROVIDER_IPC_H

#include "i_core_service_provider.h"

#include "ipc_types.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"


namespace OHOS {
namespace AudioStandard {
class ICoreServiceProviderIpc : public ICoreServiceProvider, public IRemoteBroker {
public:
    virtual ~ICoreServiceProviderIpc() = default;

    // IPC code.
    enum ICoreServiceProviderMsg : uint32_t {
        UPDATE_SESSION_OPERATION = 0,
        SET_DEFAULT_OUTPUT_DEVICE,
        GET_ADAPTER_NAME_BY_SESSION_ID,
        GET_PROCESS_DEVICE_INFO_BY_SESSION_ID,
        GENERATE_SESSION_ID,
        CORE_SERVICE_PROVIDER_MAX_MSG,
        RELOADCAPTURESESSION,
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"ICoreServiceProviderIpc");
};
} // namespace AudioStandard
} // namespace OHOS
#endif // I_CORE_SERVICE_PROVIDER_IPC_H
