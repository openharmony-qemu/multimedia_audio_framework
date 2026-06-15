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
#ifndef MOCK_I_DEVICE_MANAGER_H
#define MOCK_I_DEVICE_MANAGER_H

#include "gmock/gmock.h"
#include "adapter/i_device_manager.h"

namespace OHOS {
namespace AudioStandard {

class MockIDeviceManager : public IDeviceManager {
public:
    MOCK_METHOD(int32_t, LoadAdapter, (const std::string &adapterName), (override));
    MOCK_METHOD(void, UnloadAdapter, (const std::string &adapterName, bool force), (override));
    MOCK_METHOD(void, AllAdapterSetMicMute, (bool isMute), (override));
    MOCK_METHOD(int32_t, SetAudioParameter,
        (const std::string &adapterName, const AudioParamKey key,
         const std::string &condition, const std::string &value), (override));
    MOCK_METHOD(std::string, GetAudioParameter,
        (const std::string &adapterName, const AudioParamKey key,
         const std::string &condition), (override));
    MOCK_METHOD(int32_t, SetVoiceVolume, (const std::string &adapterName, float volume), (override));
    MOCK_METHOD(int32_t, SetOutputRoute,
        (const std::string &adapterName, const std::vector<DeviceType> &devices, int32_t streamId),
        (override));
    MOCK_METHOD(int32_t, SetInputRoute,
        (const std::string &adapterName, DeviceType device, int32_t streamId, int32_t inputType),
        (override));
    MOCK_METHOD(void, ReleaseOutputRoute, (const std::string &adapterName), (override));
    MOCK_METHOD(void, SetMicMute, (const std::string &adapterName, bool isMute), (override));
    MOCK_METHOD(int32_t, HandleEvent,
        (const std::string &adapterName, const AudioParamKey key, const char *condition,
         const char *value, void *reserved), (override));
    MOCK_METHOD(void, RegistRenderSinkCallback,
        (const std::string &adapterName, uint32_t hdiRenderId,
         std::shared_ptr<IDeviceManagerCallback> callback), (override));
    MOCK_METHOD(void, RegistCaptureSourceCallback,
        (const std::string &adapterName, uint32_t hdiCaptureId,
         std::shared_ptr<IDeviceManagerCallback> callback), (override));
    MOCK_METHOD(void, RegistAdapterManagerCallback,
        (const std::string &adapterName, IAudioSinkCallback *callback), (override));
    MOCK_METHOD(void, UnRegistRenderSinkCallback,
        (const std::string &adapterName, uint32_t hdiRenderId), (override));
    MOCK_METHOD(void, UnRegistCaptureSourceCallback,
        (const std::string &adapterName, uint32_t hdiCaptureId), (override));
    MOCK_METHOD(void, RegistCallback, (uint32_t type, IAudioSinkCallback *callback), (override));
    MOCK_METHOD(void, UnRegistAdapterManagerCallback, (const std::string &adapterName), (override));
    MOCK_METHOD(void*, CreateRender,
        (const std::string &adapterName, void *param, void *deviceDesc, uint32_t &hdiRenderId), (override));
    MOCK_METHOD(void, DestroyRender, (const std::string &adapterName, uint32_t hdiRenderId), (override));
    MOCK_METHOD(void*, CreateCapture,
        (const std::string &adapterName, void *param, void *deviceDesc, uint32_t &hdiCaptureId), (override));
    MOCK_METHOD(void, DestroyCapture, (const std::string &adapterName, uint32_t hdiCaptureId), (override));
    MOCK_METHOD(void, DumpInfo, (std::string &dumpString), (override));
    MOCK_METHOD(void, SetDmDeviceType, (uint16_t dmDeviceType, DeviceType deviceType), (override));
    MOCK_METHOD(void, SetAudioScene, (AudioScene scene), (override));
    MOCK_METHOD(int32_t, CreateCognitionStream,
        (const std::string &adapterName, void *param, int32_t &sinkId, void *buffer), (override));
    MOCK_METHOD(int32_t, DestroyCognitionStream,
        (const std::string &adapterName, const int32_t &sinkId), (override));
    MOCK_METHOD(int32_t, NotifyCognitionData,
        (const std::string &adapterName, const int32_t &sinkId, uint32_t size, uint32_t offset), (override));
};

} // namespace AudioStandard
} // namespace OHOS

#endif // MOCK_I_DEVICE_MANAGER_H
