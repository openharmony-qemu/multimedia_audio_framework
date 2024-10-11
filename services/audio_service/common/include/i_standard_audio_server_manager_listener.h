/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef I_STANDARD_AUDIO_SERVER_MANAGER_LISTENER_H
#define I_STANDARD_AUDIO_SERVER_MANAGER_LISTENER_H

#include "ipc_types.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"

namespace OHOS {
namespace AudioStandard {
class IStandardAudioServerManagerListener : public IRemoteBroker {
public:
    virtual ~IStandardAudioServerManagerListener() = default;
    virtual void OnAudioParameterChange(const std::string networkId, const AudioParamKey key,
        const std::string& condition, const std::string& value) = 0;
    virtual void OnCapturerState(bool isActive) = 0;
    virtual void OnWakeupClose() = 0;

    enum AudioServerManagerListenerMsg {
        ON_ERROR = 0,
        ON_PARAMETER_CHANGED,
        ON_CAPTURER_STATE,
        ON_WAKEUP_CLOSE
    };
    DECLARE_INTERFACE_DESCRIPTOR(u"IStandardAudioServerManagerListener");
};
} // namespace AudioStandard
} // namespace OHOS
#endif // I_STANDARD_AUDIO_SERVER_MANAGER_LISTENER_H
