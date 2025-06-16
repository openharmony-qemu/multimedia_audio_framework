/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MULTIMEDIA_AUDIO_ROUTING_MANAGER_IMPL_H
#define MULTIMEDIA_AUDIO_ROUTING_MANAGER_IMPL_H
#include "audio_routing_manager.h"
#include "audio_system_manager.h"
#include "cj_common_ffi.h"
#include "multimedia_audio_ffi.h"
#include "multimedia_audio_routing_manager_callback.h"
#include "native/ffi_remote_data.h"

namespace OHOS {
namespace AudioStandard {
class MMAAudioRoutingManagerImpl : public OHOS::FFI::FFIData {
    DECL_TYPE(MMAAudioRoutingManagerImpl, OHOS::FFI::FFIData)
public:
    MMAAudioRoutingManagerImpl();
    ~MMAAudioRoutingManagerImpl();
    bool IsCommunicationDeviceActive(int32_t deviceType);

    int32_t SetCommunicationDevice(int32_t deviceType, bool active);

    CArrDeviceDescriptor GetDevices(int32_t flags, int32_t* errorCode);

    CArrDeviceDescriptor GetAvailableDevices(uint32_t deviceUsage, int32_t* errorCode);

    CArrDeviceDescriptor GetPreferredInputDeviceForCapturerInfo(CAudioCapturerInfo cInfo, int32_t* errorCode);

    CArrDeviceDescriptor GetPreferredOutputDeviceForRendererInfo(CAudioRendererInfo cInfo, int32_t* errorCode);

    void RegisterCallback(int32_t callbackType, uint32_t deviceUsage, void (*callback)(), int32_t* errorCode);

    void RegisterPreferredInputDeviceChangeCallback(
        int32_t callbackType, void (*callback)(), CAudioCapturerInfo info, int32_t* errorCode);

    void RegisterPreferredOutputDeviceChangeCallback(
        int32_t callbackType, void (*callback)(), CAudioRendererInfo info, int32_t* errorCode);

    void RegisterDeviceChangeCallback(int32_t callbackType, void (*callback)(), int32_t flags, int32_t* errorCode);

private:
    AudioSystemManager* audioMgr_ {};
    AudioRoutingManager* routingMgr_ {};
    std::shared_ptr<CjAudioManagerAvailableDeviceChangeCallback> deviceUsageCallback_ {};
    std::shared_ptr<CjAudioPreferredInputDeviceChangeCallback> preferredInputDeviceChangeCallBack_ {};
    std::shared_ptr<CjAudioPreferredOutputDeviceChangeCallback> preferredOutputDeviceChangeCallBack_ {};
    std::shared_ptr<CjAudioManagerDeviceChangeCallback> deviceChangeCallBack_ {};
};
} // namespace AudioStandard
} // namespace OHOS
#endif // MULTIMEDIA_AUDIO_ROUTING_MANAGER_IMPL_H
