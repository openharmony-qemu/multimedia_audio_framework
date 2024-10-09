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

#ifndef I_AUDIO_PROCESS_H
#define I_AUDIO_PROCESS_H

#include <memory>

#include "ipc_types.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"

#include "audio_info.h"
#include "audio_process_config.h"
#include "oh_audio_buffer.h"

namespace OHOS {
namespace AudioStandard {
class AudioProcess {
public:
    virtual int32_t ResolveBuffer(std::shared_ptr<OHAudioBuffer> &buffer) = 0;

    virtual int32_t GetSessionId(uint32_t &sessionId) = 0;

    virtual int32_t Start() = 0;

    virtual int32_t Pause(bool isFlush) = 0;

    virtual int32_t Resume() = 0;

    virtual int32_t Stop() = 0;

    virtual int32_t RequestHandleInfo(bool isAync = true) = 0;

    virtual int32_t Release() = 0;

    virtual ~AudioProcess() = default;
};

class IProcessCb : public IRemoteBroker {
public:
    virtual ~IProcessCb() = default;

    virtual int32_t OnEndpointChange(int32_t status) = 0;

    // IPC code.
    enum IProcessCbMsg : uint32_t {
        ON_ENDPOINT_CHANGE = 0,
        PROCESS_CB_MAX_MSG
    };
    DECLARE_INTERFACE_DESCRIPTOR(u"IProcessCb");
};

class IAudioProcess : public AudioProcess, public IRemoteBroker {
public:
    virtual ~IAudioProcess() = default;

    virtual int32_t RegisterProcessCb(sptr<IRemoteObject> object) = 0;
    virtual int32_t RegisterThreadPriority(uint32_t tid, const std::string &bundleName) = 0;

    // IPC code.
    enum IAudioProcessMsg : uint32_t {
        ON_RESOLVE_BUFFER = 0,
        OH_GET_SESSIONID,
        ON_START,
        ON_PAUSE,
        ON_RESUME,
        ON_STOP,
        ON_REQUEST_HANDLE_INFO,
        ON_RELEASE,
        ON_REGISTER_PROCESS_CB,
        ON_REGISTER_THREAD_PRIORITY,
        PROCESS_MAX_MSG
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"IAudioProcess");
};
} // namespace AudioStandard
} // namespace OHOS
#endif // I_AUDIO_PROCESS_H
