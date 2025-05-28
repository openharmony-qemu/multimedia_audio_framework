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

#ifndef I_STANDARD_CLIENT_TRACKER_H
#define I_STANDARD_CLIENT_TRACKER_H

#include "ipc_types.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"

namespace OHOS {
namespace AudioStandard {
class IStandardClientTracker : public IRemoteBroker {
public:
    virtual ~IStandardClientTracker() = default;

    virtual void MuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) = 0;
    virtual void UnmuteStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) = 0;
    virtual void PausedStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) = 0;
    virtual void ResumeStreamImpl(const StreamSetStateEventInternal &streamSetStateEventInternal) = 0;
    virtual void SetLowPowerVolumeImpl(float volume) = 0;
    virtual void GetLowPowerVolumeImpl(float& volume) = 0;
    virtual void SetOffloadModeImpl(int32_t state, bool isAppBack) = 0;
    virtual void UnsetOffloadModeImpl() = 0;
    virtual void GetSingleStreamVolumeImpl(float& volume) = 0;

    enum AudioClientTrackerMsg {
        ON_ERROR = 0,
        ON_INIT,
        SETLOWPOWERVOL,
        GETLOWPOWERVOL,
        PAUSEDSTREAM,
        RESUMESTREAM,
        MUTESTREAM,
        UNMUTESTREAM,
        GETSINGLESTREAMVOL,
        SETOFFLOADMODE,
        UNSETOFFLOADMODE,
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"IStandardClientTracker");
};
} // namespace AudioStandard
} // namespace OHOS
#endif // I_STANDARD_CLIENT_TRACKER_H
