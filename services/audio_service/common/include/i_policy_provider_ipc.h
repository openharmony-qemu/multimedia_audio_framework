/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef I_POLICY_PROVIDER_IPC_H
#define I_POLICY_PROVIDER_IPC_H

#include "i_policy_provider.h"

#include "ipc_types.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"

#include "audio_process_config.h"

namespace OHOS {
namespace AudioStandard {
class IPolicyProviderIpc : public IPolicyProvider, public IRemoteBroker {
public:
    virtual ~IPolicyProviderIpc() = default;

    // IPC code.
    enum IPolicyProviderMsg : uint32_t {
        GET_DEVICE_INFO = 0,
        INIT_VOLUME_MAP,
        SET_WAKEUP_ADUIO_CAPTURER,
        SET_AUDIO_CAPTURER,
        REMOVE_WAKEUP_CAPUTER,
        IS_ABS_VOLUME_SUPPORTED,
        OFFLOAD_GET_RENDER_POSITION,
        GET_AND_SAVE_CLIENT_TYPE,
        POLICY_PROVIDER_MAX_MSG,
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"IPolicyProviderIpc");
};
} // namespace AudioStandard
} // namespace OHOS
#endif // I_POLICY_PROVIDER_IPC_H
