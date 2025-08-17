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

#ifndef CORE_SERVICE_HANDLER_H
#define CORE_SERVICE_HANDLER_H

#include <sstream>
#include <map>
#include <mutex>
#include <vector>

#include "icore_service_provider_ipc.h"
#include "audio_service_enum.h"

namespace OHOS {
namespace AudioStandard {
class CoreServiceHandler {
public:
    static CoreServiceHandler& GetInstance();

    ~CoreServiceHandler();

    // would be called only once
    int32_t ConfigCoreServiceProvider(const sptr<ICoreServiceProviderIpc> policyProvider);

    int32_t ReloadCaptureSession(uint32_t sessionId, SessionOperation operation);
    int32_t UpdateSessionOperation(uint32_t sessionId, SessionOperation operation,
        SessionOperationMsg opMsg = SESSION_OP_MSG_DEFAULT);
    int32_t SetDefaultOutputDevice(
        const DeviceType defaultOutputDevice, const uint32_t sessionID, const StreamUsage streamUsage, bool isRunning,
        bool skipForce = false);
    std::string GetAdapterNameBySessionId(uint32_t sessionId);
    int32_t GetProcessDeviceInfoBySessionId(uint32_t sessionId, AudioDeviceDescriptor &deviceInfo,
        AudioStreamInfo &streamInfo, bool isReloadProcess);
    uint32_t GenerateSessionId();
    int32_t SetWakeUpAudioCapturerFromAudioServer(const AudioProcessConfig &config);
private:
    CoreServiceHandler();
    sptr<ICoreServiceProviderIpc> iCoreServiceProvider_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // CORE_SERVICE_HANDLER_H
