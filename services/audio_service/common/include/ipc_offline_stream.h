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

#ifndef IPC_OFFLINE_STREAM_H
#define IPC_OFFLINE_STREAM_H

#include <memory>

#include "ipc_types.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"

#include "audio_info.h"
#include "oh_audio_buffer.h"

namespace OHOS {
namespace AudioStandard {
class IpcOfflineStream : public IRemoteBroker {
public:
    virtual ~IpcOfflineStream() = default;

    virtual int32_t CreateOfflineEffectChain(const std::string &chainName) = 0;

    virtual int32_t ConfigureOfflineEffectChain(const AudioStreamInfo &inInfo, const AudioStreamInfo &outInfo) = 0;

    virtual int32_t PrepareOfflineEffectChain(std::shared_ptr<AudioSharedMemory> &inBuffer,
        std::shared_ptr<AudioSharedMemory> &outBuffer) = 0;

    virtual int32_t ProcessOfflineEffectChain(uint32_t inSize, uint32_t outSize) = 0;

    virtual void ReleaseOfflineEffectChain() = 0;

    // IPC code.
    enum IpcOfflineStreamMsg : uint32_t {
        CREATE_OFFLINE_EFFECT_CHAIN,
        CONFIGURE_OFFLINE_EFFECT_CHAIN,
        PREPARE_OFFLINE_EFFECT_CHAIN,
        PROCESS_OFFLINE_EFFECT_CHAIN,
        RELEASE_OFFLINE_EFFECT_CHAIN,
        IPC_OFFLINE_STREAM_MAX_MSG
    };
    DECLARE_INTERFACE_DESCRIPTOR(u"IpcOfflineStream");
};
} // namespace AudioStandard
} // namespace OHOS
#endif // IPC_OFFLINE_STREAM_H
