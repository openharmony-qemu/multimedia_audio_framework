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

#ifndef I_DEVICE_MANAGER_H
#define I_DEVICE_MANAGER_H

#include <iostream>
#include <cstring>
#include <cinttypes>
#include "audio_info.h"
#include "audio_errors.h"
#include "sink/i_audio_render_sink.h"
#include "source/i_audio_capture_source.h"

namespace OHOS {
namespace AudioStandard {
class IDeviceManagerCallback {
public:
    virtual void OnAudioParamChange(const std::string &adapterName, const AudioParamKey key,
        const std::string &condition, const std::string &value) {}
};

class IDeviceManager {
public:
    IDeviceManager() = default;
    virtual ~IDeviceManager() = default;

    virtual int32_t LoadAdapter(const std::string &adapterName) = 0;
    virtual void UnloadAdapter(const std::string &adapterName, bool force = false) = 0;

    virtual void AllAdapterSetMicMute(bool isMute) = 0;

    virtual void SetAudioParameter(const std::string &adapterName, const AudioParamKey key,
        const std::string &condition, const std::string &value) = 0;
    virtual std::string GetAudioParameter(const std::string &adapterName, const AudioParamKey key,
        const std::string &condition) = 0;
    virtual int32_t SetVoiceVolume(const std::string &adapterName, float volume) = 0;
    virtual int32_t SetOutputRoute(const std::string &adapterName, const std::vector<DeviceType> &devices,
        int32_t streamId) = 0;
    virtual int32_t SetInputRoute(const std::string &adapterName, DeviceType device, int32_t streamId,
        int32_t inputType) = 0;
    virtual void SetMicMute(const std::string &adapterName, bool isMute) = 0;
    virtual int32_t HandleEvent(const std::string &adapterName, const AudioParamKey key, const char *condition,
        const char *value, void *reserved) { return ERR_NOT_SUPPORTED; }
    virtual void RegistRenderSinkCallback(const std::string &adapterName, uint32_t hdiRenderId,
        IDeviceManagerCallback *callback) {}
    virtual void RegistCaptureSourceCallback(const std::string &adapterName, uint32_t hdiCaptureId,
        IDeviceManagerCallback *callback) {}
    virtual void UnRegistRenderSinkCallback(const std::string &adapterName, uint32_t hdiRenderId) {}
    virtual void UnRegistCaptureSourceCallback(const std::string &adapterName, uint32_t hdiCaptureId) {}

    virtual void *CreateRender(const std::string &adapterName, void *param, void *deviceDesc,
        uint32_t &hdiRenderId) = 0;
    virtual void DestroyRender(const std::string &adapterName, uint32_t hdiRenderId) = 0;
    virtual void *CreateCapture(const std::string &adapterName, void *param, void *deviceDesc,
        uint32_t &hdiCaptureId) = 0;
    virtual void DestroyCapture(const std::string &adapterName, uint32_t hdiCaptureId) = 0;

    virtual void DumpInfo(std::string &dumpString) = 0;

    virtual void SetDmDeviceType(uint16_t dmDeviceType, DeviceType deviceType) = 0;

    virtual void SetAudioScene(const AudioScene scene) = 0;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // I_DEVICE_MANAGER_H
