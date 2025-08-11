/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef I_ST_AUDIO_MANAGER_BASE_H
#define I_ST_AUDIO_MANAGER_BASE_H

#include "iremote_object.h"
#include "audio_stutter.h"
#include "audio_workgroup_ipc.h"

namespace OHOS {
namespace AudioStandard {
class AudioDeviceDescriptor;
class DataTransferStateChangeCallbackInner {
public:
    virtual ~DataTransferStateChangeCallbackInner() = default;
    
    virtual void OnDataTransferStateChange(const int32_t &callbackId,
        const AudioRendererDataTransferStateChangeInfo &info) = 0;

    virtual void OnMuteStateChange(const int32_t &callbackId, const int32_t &uid,
        const uint32_t &sessionId, const bool &isMuted) = 0;
};

} // namespace AudioStandard
} // namespace OHO
#endif // I_ST_AUDIO_MANAGER_BASE_H
