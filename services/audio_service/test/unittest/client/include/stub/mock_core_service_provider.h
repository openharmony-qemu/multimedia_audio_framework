/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef MOCK_CORE_SERVICE_PROVIDER_H
#define MOCK_CORE_SERVICE_PROVIDER_H

#include <gmock/gmock.h>
#include "i_core_service_provider.h"

namespace OHOS {
namespace AudioStandard {

class MockCoreServiceProvider : public ICoreServiceProvider {
public:
    MOCK_METHOD(int32_t, ReloadCaptureSession, (uint32_t sessionId, SessionOperation operation), (override));
    MOCK_METHOD(int32_t, UpdateSessionOperation, (uint32_t sessionId, SessionOperation operation,
        SessionOperationMsg opMsg, bool needRemoveFromMap), (override));
    MOCK_METHOD(uint32_t, GetPaIndexByPortName, (const std::string& portName), (override));
    MOCK_METHOD(int32_t, SetDefaultOutputDevice, (const DeviceType defaultOutputDevice,
        const uint32_t sessionID, const StreamUsage streamUsage, bool isRunning, bool skipForce), (override));
    MOCK_METHOD(std::string, GetAdapterNameBySessionId, (uint32_t sessionID), (override));
    MOCK_METHOD(std::string, GetModuleNameBySessionId, (uint32_t sessionID), (override));
    MOCK_METHOD(int32_t, GetProcessDeviceInfoBySessionId, (uint32_t sessionID, AudioDeviceDescriptor& deviceInfo,
        AudioStreamInfo& streamInfo, bool& isUltraFast, bool isReloadProcess), (override));
    MOCK_METHOD(uint32_t, GenerateSessionId, (), (override));
    MOCK_METHOD(int32_t, SetWakeUpAudioCapturerFromAudioServer, (const AudioProcessConfig& config), (override));
    MOCK_METHOD(int32_t, A2dpOffloadGetRenderPosition,
                    (uint32_t& delayValue, uint64_t& sendDataSize, uint32_t& timeStamp), (override));
    MOCK_METHOD(int32_t, SetRendererTarget,
                    (RenderTarget target, RenderTarget lastTarget, uint32_t sessionId), (override));
    MOCK_METHOD(int32_t, StartInjection, (uint32_t sessionId), (override));
    MOCK_METHOD(void, RemoveIdForInjector, (uint32_t streamId), (override));
    MOCK_METHOD(void, ReleaseCaptureInjector, (), (override));
    MOCK_METHOD(void, RebuildCaptureInjector, (uint32_t streamId), (override));
    MOCK_METHOD(void, OnCheckActiveMusicTime, (const std::string& reason), (override));
    MOCK_METHOD(int32_t, CaptureConcurrentCheck, (uint32_t sessionID), (override));
    MOCK_METHOD(int32_t, SetCapturerMuteHint, (uint32_t sessionId, bool mute), (override));
    MOCK_METHOD(int32_t, RecordBootStateTime, (int32_t state, int32_t uid, int64_t curTime), (override));
    MOCK_METHOD(void, NotifySessionStandbyStatus, (uint32_t sessionId, bool standbyStatus), (override));
    MOCK_METHOD(int32_t, NotifyStreamStandby, (int32_t pid, uint32_t streamId), (override));
    MOCK_METHOD(int32_t, NotifyStreamStandbyResume, (int32_t pid, uint32_t streamId), (override));
    MOCK_METHOD(bool, IsSupportIndependentPipe, (), (override));
    MOCK_METHOD(void, OnDeviceStateChange,
                    (const std::string& networkId, DeviceRole role, bool isAvailable), (override));
    MOCK_METHOD(void, NotifySilentStateChange, (uint32_t sessionId, bool isSilent), (override));
};

} // namespace AudioStandard
} // namespace OHOS

#endif // MOCK_CORE_SERVICE_PROVIDER_H
