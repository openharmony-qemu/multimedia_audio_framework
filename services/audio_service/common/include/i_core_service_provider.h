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

#ifndef I_CORE_SERVICE_PROVIDER_H
#define I_CORE_SERVICE_PROVIDER_H

#include <cstdint>
#include "audio_service_enum.h"
#include "audio_device_info.h"
#include "audio_device_descriptor.h"


namespace OHOS {
namespace AudioStandard {
class ICoreServiceProvider {
public:
    virtual int32_t ReloadCaptureSession(uint32_t sessionId, SessionOperation operation) = 0;
    virtual int32_t UpdateSessionOperation(uint32_t sessionId, SessionOperation operation,
        SessionOperationMsg opMsg) = 0;
    virtual int32_t SetDefaultOutputDevice(const DeviceType defaultOutputDevice,
        const uint32_t sessionID, const StreamUsage streamUsage, bool isRunning, bool skipForce = false) = 0;
    virtual std::string GetAdapterNameBySessionId(uint32_t sessionID) = 0;
    virtual int32_t GetProcessDeviceInfoBySessionId(uint32_t sessionID, AudioDeviceDescriptor &deviceInfo,
        AudioStreamInfo &streamInfo, bool isReloadProcess) = 0;
    virtual uint32_t GenerateSessionId() = 0;
    virtual int32_t SetWakeUpAudioCapturerFromAudioServer(const AudioProcessConfig &config) = 0;

    virtual ~ICoreServiceProvider() = default;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // I_CORE_SERVICE_PROVIDER_H
