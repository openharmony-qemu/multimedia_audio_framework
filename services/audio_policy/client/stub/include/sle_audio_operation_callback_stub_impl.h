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

#ifndef SLE_AUDIO_OPERATION_CALLBACK_STUB_H
#define SLE_AUDIO_OPERATION_CALLBACK_STUB_H

#include "audio_policy_interface.h"
#include "standard_sle_audio_operation_callback_stub.h"

namespace OHOS {
namespace AudioStandard {
class SleAudioOperationCallbackStubImpl : public StandardSleAudioOperationCallbackStub {
public:
    SleAudioOperationCallbackStubImpl();
    virtual ~SleAudioOperationCallbackStubImpl();

    int32_t SetSleAudioOperationCallback(const std::weak_ptr<SleAudioOperationCallback> &callback);

    int32_t GetSleAudioDeviceList(std::vector<AudioDeviceDescriptor> &devices) override;
    int32_t GetSleVirtualAudioDeviceList(std::vector<AudioDeviceDescriptor> &devices) override;
    int32_t IsInBandRingOpen(const std::string &device, bool& ret) override;
    int32_t GetSupportStreamType(const std::string &device, uint32_t& retType) override;
    int32_t SetActiveSinkDevice(const std::string &device, uint32_t streamType, int32_t& ret) override;
    int32_t StartPlaying(const std::string &device, uint32_t streamType, int32_t timeoutMs, int32_t& ret) override;
    int32_t StopPlaying(const std::string &device, uint32_t streamType, int32_t& ret) override;
    int32_t ConnectAllowedProfiles(const std::string &remoteAddr, int32_t& ret) override;
    int32_t SetDeviceAbsVolume(const std::string &remoteAddr, uint32_t volume, uint32_t streamType,
        int32_t& ret) override;
    int32_t SendUserSelection(const std::string &device,
        uint32_t sleStreamType, int32_t eventType, int32_t& ret) override;
    int32_t GetRenderPosition(const std::string &device, uint32_t &delayValue) override;
private:
    std::mutex sleAudioOperationCallbackMutex_;
    std::weak_ptr<SleAudioOperationCallback> sleAudioOperationCallback_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // SLE_AUDIO_OPERATION_CALLBACK_STUB_H
