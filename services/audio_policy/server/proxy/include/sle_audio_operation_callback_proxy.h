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

#ifndef SLE_AUDIO_OPERATION_CALLBACK_PROXY_H
#define SLE_AUDIO_OPERATION_CALLBACK_PROXY_H

#include "audio_general_manager.h"
#include "i_standard_sle_audio_operation_callback.h"

namespace OHOS {
namespace AudioStandard {
class SleAudioOperationCallbackProxy : public IRemoteProxy<IStandardSleAudioOperationCallback> {
public:
    explicit SleAudioOperationCallbackProxy(const sptr<IRemoteObject> &impl);
    virtual ~SleAudioOperationCallbackProxy();
    DISALLOW_COPY_AND_MOVE(SleAudioOperationCallbackProxy);

    void GetSleAudioDeviceList(std::vector<AudioDeviceDescriptor> &devices) override;
    void GetSleVirtualAudioDeviceList(std::vector<AudioDeviceDescriptor> &devices) override;
    bool IsInBandRingOpen(const std::string &device) override;
    uint32_t GetSupportStreamType(const std::string &device) override;
    int32_t SetActiveSinkDevice(const std::string &device, uint32_t streamType) override;
    int32_t StartPlaying(const std::string &device, uint32_t streamType) override;
    int32_t StopPlaying(const std::string &device, uint32_t streamType) override;
    int32_t ConnectAllowedProfiles(const std::string &remoteAddr) override;
    int32_t SetDeviceAbsVolume(const std::string &remoteAddr, uint32_t volume, uint32_t streamType) override;
    int32_t SendUserSelection(const std::string &device, uint32_t streamType) override;
private:
    static inline BrokerDelegator<SleAudioOperationCallbackProxy> delegator_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // SLE_AUDIO_OPERATION_CALLBACK_PROXY_H
