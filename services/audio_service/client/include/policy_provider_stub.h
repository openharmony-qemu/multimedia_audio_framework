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

#ifndef POLICY_PROVIDER_STUB_H
#define POLICY_PROVIDER_STUB_H

#include "i_policy_provider.h"
#include "policy_provider_ipc_stub.h"
#include "audio_process_config.h"

namespace OHOS {
namespace AudioStandard {
class PolicyProviderWrapper : public PolicyProviderIpcStub {
public:
    ~PolicyProviderWrapper();
    PolicyProviderWrapper(IPolicyProvider *policyWorker);

    int32_t GetProcessDeviceInfo(const AudioProcessConfig &config, bool lockFlag,
        AudioDeviceDescriptor &deviceInfo) override;
    int32_t InitSharedVolume(std::shared_ptr<AudioSharedMemory> &buffer) override;
    int32_t NotifyCapturerAdded(const AudioCapturerInfo &capturerInfo, const AudioStreamInfo &streamInfo,
        uint32_t sessionId) override;
    int32_t NotifyWakeUpCapturerRemoved() override;
    int32_t IsAbsVolumeSupported(bool &isSupported) override;
    int32_t OffloadGetRenderPosition(uint32_t &delayValue, uint64_t &sendDataSize, uint32_t &timeStamp) override;
    int32_t NearlinkGetRenderPosition(uint32_t &delayValue) override;
    int32_t GetAndSaveClientType(uint32_t uid, const std::string &bundleName) override;
    int32_t GetMaxRendererInstances(int32_t &maxInstances) override;
    int32_t NotifyCapturerRemoved(uint64_t sessionId) override;
// #ifdef HAS_FEATURE_INNERCAPTURER
    int32_t LoadModernInnerCapSink(int32_t innerCapId) override;
    int32_t UnloadModernInnerCapSink(int32_t innerCapId) override;
// #endif
    int32_t ClearAudioFocusBySessionID(int32_t sessionID) override;
    int32_t CaptureConcurrentCheck(uint32_t sessionID) override;
    
private:
    IPolicyProvider *policyWorker_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // POLICY_PROVIDER_STUB_H
