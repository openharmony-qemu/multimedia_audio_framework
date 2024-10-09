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

#include "i_policy_provider_ipc.h"

namespace OHOS {
namespace AudioStandard {
class PolicyProviderStub : public IRemoteStub<IPolicyProviderIpc> {
public:
    virtual ~PolicyProviderStub() = default;
    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
private:
    static bool CheckInterfaceToken(MessageParcel &data);

    int32_t HandleGetProcessDeviceInfo(MessageParcel &data, MessageParcel &reply);
    int32_t HandleInitSharedVolume(MessageParcel &data, MessageParcel &reply);
    int32_t HandleSetWakeupCapturer(MessageParcel &data, MessageParcel &reply);
    int32_t HandleSetCapturer(MessageParcel &data, MessageParcel &reply);
    int32_t HandleWakeupCapturerRemoved(MessageParcel &data, MessageParcel &reply);
    int32_t HandleIsAbsVolumeSupported(MessageParcel &data, MessageParcel &reply);
    int32_t HandleOffloadGetRenderPosition(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetAndSaveClientType(MessageParcel &data, MessageParcel &reply);
};

class PolicyProviderWrapper : public PolicyProviderStub {
public:
    ~PolicyProviderWrapper();
    PolicyProviderWrapper(IPolicyProvider *policyWorker);

    int32_t GetProcessDeviceInfo(const AudioProcessConfig &config, DeviceInfo &deviceInfo) override;
    int32_t InitSharedVolume(std::shared_ptr<AudioSharedMemory> &buffer) override;
    int32_t SetWakeUpAudioCapturerFromAudioServer(const AudioProcessConfig &config) override;
    int32_t NotifyCapturerAdded(AudioCapturerInfo capturerInfo, AudioStreamInfo streamInfo,
        uint32_t sessionId) override;
    int32_t NotifyWakeUpCapturerRemoved() override;
    bool IsAbsVolumeSupported() override;
    int32_t OffloadGetRenderPosition(uint32_t &delayValue, uint64_t &sendDataSize, uint32_t &timeStamp) override;
    int32_t GetAndSaveClientType(uint32_t uid, const std::string &bundleName) override;
private:
    IPolicyProvider *policyWorker_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // POLICY_PROVIDER_STUB_H
