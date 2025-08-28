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

#ifndef CORE_SERVICE_PROVIDER_STUB_H
#define CORE_SERVICE_PROVIDER_STUB_H

#include "core_service_provider_ipc_stub.h"
#include "i_core_service_provider.h"

namespace OHOS {
namespace AudioStandard {
class CoreServiceProviderWrapper : public CoreServiceProviderIpcStub {
public:
    ~CoreServiceProviderWrapper();
    CoreServiceProviderWrapper(ICoreServiceProvider *coreServiceWorker);

    int32_t UpdateSessionOperation(uint32_t sessionId, uint32_t operation, uint32_t opMsg) override;
    int32_t ReloadCaptureSession(uint32_t sessionId, uint32_t operation) override;
    int32_t SetDefaultOutputDevice(int32_t defaultOutputDevice, uint32_t sessionID, int32_t streamUsage,
        bool isRunning, bool skipForce = false) override;
    int32_t GetAdapterNameBySessionId(uint32_t sessionId, std::string& name) override;
    int32_t GetProcessDeviceInfoBySessionId(uint32_t sessionId, AudioDeviceDescriptor& deviceInfo,
        AudioStreamInfo &streamInfo, bool isReloadProcess) override;
    int32_t GenerateSessionId(uint32_t &sessionId) override;

    int32_t SetWakeUpAudioCapturerFromAudioServer(const AudioProcessConfig &config, int32_t &ret) override;
private:
    ICoreServiceProvider *coreServiceWorker_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // CORE_SERVICE_PROVIDER_STUB_H
