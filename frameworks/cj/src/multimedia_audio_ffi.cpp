/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

#include "multimedia_audio_ffi.h"

#include "audio_capturer_log.h"
#include "multimedia_audio_capturer_callback.h"
#include "multimedia_audio_capturer_impl.h"
#include "multimedia_audio_common.h"
#include "multimedia_audio_error.h"
#include "multimedia_audio_manager_impl.h"
#include "multimedia_audio_renderer_callback.h"
#include "multimedia_audio_renderer_impl.h"
#include "multimedia_audio_routing_manager_callback.h"
#include "multimedia_audio_routing_manager_impl.h"
#include "multimedia_audio_session_manager_impl.h"
#include "multimedia_audio_stream_manager_callback.h"
#include "multimedia_audio_stream_manager_impl.h"
#include "multimedia_audio_volume_group_manager_impl.h"
#include "multimedia_audio_volume_manager_impl.h"

using namespace OHOS::FFI;

namespace OHOS {
namespace AudioStandard {
extern "C" {
// Audio Capturer
FFI_EXPORT int64_t FfiMMACreateAudioCapturer(CAudioCapturerOptions options, int32_t* errorCode)
{
    auto capturer = FFIData::Create<MMAAudioCapturerImpl>();
    if (!capturer) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Create MMAAudioCapturerImpl error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    auto ret = capturer->CreateAudioCapturer(options);
    if (ret != SUCCESS_CODE) {
        FFIData::Release(capturer->GetID());
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("CreateAudioCapturer error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return capturer->GetID();
}

FFI_EXPORT int32_t FfiMMAAudioCapturerGetState(int64_t id, int32_t* errorCode)
{
    auto capturer = FFIData::GetData<MMAAudioCapturerImpl>(id);
    if (!capturer) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("get state failed, invalid id of AudioCapturer");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return capturer->GetState();
}

FFI_EXPORT uint32_t FfiMMAAudioCapturerGetStreamId(int64_t id, int32_t* errorCode)
{
    auto capturer = FFIData::GetData<MMAAudioCapturerImpl>(id);
    if (!capturer) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("get stream id failed, invalid id of AudioCapturer");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return capturer->GetStreamId(errorCode);
}

FFI_EXPORT int64_t FfiMMAAudioCapturerGetAudioTime(int64_t id, int32_t* errorCode)
{
    auto capturer = FFIData::GetData<MMAAudioCapturerImpl>(id);
    if (!capturer) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("get audio time failed, invalid id of AudioCapturer");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return capturer->GetAudioTime(errorCode);
}

FFI_EXPORT uint32_t FfiMMAAudioCapturerGetBufferSize(int64_t id, int32_t* errorCode)
{
    auto capturer = FFIData::GetData<MMAAudioCapturerImpl>(id);
    if (!capturer) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("get buffer size failed, invalid id of AudioCapturer");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return capturer->GetBufferSize(errorCode);
}

FFI_EXPORT uint32_t FfiMMAAudioCapturerGetOverflowCount(int64_t id, int32_t* errorCode)
{
    auto capturer = FFIData::GetData<MMAAudioCapturerImpl>(id);
    if (!capturer) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("get OverflowCount failed, invalid id of AudioCapturer");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return capturer->GetOverflowCount();
}

FFI_EXPORT void FfiMMAAudioCapturerStart(int64_t id, int32_t* errorCode)
{
    auto capturer = FFIData::GetData<MMAAudioCapturerImpl>(id);
    if (!capturer) {
        AUDIO_ERR_LOG("AudioCapturer start failed, invalid id of AudioCapturer");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    auto isSuccess = capturer->Start();
    if (isSuccess != SUCCESS_CODE) {
        *errorCode = isSuccess;
    }
}

FFI_EXPORT void FfiMMAAudioCapturerStop(int64_t id, int32_t* errorCode)
{
    auto capturer = FFIData::GetData<MMAAudioCapturerImpl>(id);
    if (!capturer) {
        AUDIO_ERR_LOG("AudioCapturer stop failed, invalid id of AudioCapturer");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    auto isSuccess = capturer->Stop();
    if (isSuccess != SUCCESS_CODE) {
        *errorCode = isSuccess;
    }
}

FFI_EXPORT void FfiMMAAudioCapturerRelease(int64_t id, int32_t* errorCode)
{
    auto capturer = FFIData::GetData<MMAAudioCapturerImpl>(id);
    if (!capturer) {
        AUDIO_ERR_LOG("AudioCapturer release failed, invalid id of AudioCapturer");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    auto isSuccess = capturer->Release();
    if (isSuccess != SUCCESS_CODE) {
        *errorCode = isSuccess;
    }
}

FFI_EXPORT CAudioCapturerChangeInfo FfiMMAAudioCapturerGetAudioCapturerChangeInfo(int64_t id, int32_t* errorCode)
{
    auto capturer = FFIData::GetData<MMAAudioCapturerImpl>(id);
    if (!capturer) {
        AUDIO_ERR_LOG("Get AudioCapturerChangeInfo failed, invalid id of AudioCapturer");
        *errorCode = CJ_ERR_SYSTEM;
        return CAudioCapturerChangeInfo();
    }
    auto info = capturer->GetAudioCapturerChangeInfo(errorCode);
    return info;
}

FFI_EXPORT CArrDeviceDescriptor FfiMMAAudioCapturerGetInputDevices(int64_t id, int32_t* errorCode)
{
    auto capturer = FFIData::GetData<MMAAudioCapturerImpl>(id);
    if (!capturer) {
        AUDIO_ERR_LOG("Get InputDevices failed, invalid id of AudioCapturer");
        *errorCode = CJ_ERR_SYSTEM;
        return CArrDeviceDescriptor();
    }
    auto devices = capturer->GetInputDevices(errorCode);
    return devices;
}

FFI_EXPORT CAudioCapturerInfo FfiMMAAudioCapturerGetCapturerInfo(int64_t id, int32_t* errorCode)
{
    auto capturer = FFIData::GetData<MMAAudioCapturerImpl>(id);
    if (!capturer) {
        AUDIO_ERR_LOG("Get AudioCapturerInfo failed, invalid id of AudioCapturer");
        *errorCode = CJ_ERR_SYSTEM;
        return CAudioCapturerInfo();
    }
    auto info = capturer->GetCurrentCapturerInfo(errorCode);
    return info;
}

FFI_EXPORT CAudioStreamInfo FfiMMAAudioCapturerGetStreamInfo(int64_t id, int32_t* errorCode)
{
    auto capturer = FFIData::GetData<MMAAudioCapturerImpl>(id);
    if (!capturer) {
        AUDIO_ERR_LOG("Get StreamInfo failed, invalid id of AudioCapturer");
        *errorCode = CJ_ERR_SYSTEM;
        return CAudioStreamInfo();
    }
    auto info = capturer->GetStreamInfo(errorCode);
    return info;
}

FFI_EXPORT void FfiMMAAudioCapturerOn(int64_t id, int32_t callbackType, void (*callback)(), int32_t* errorCode)
{
    auto capturer = FFIData::GetData<MMAAudioCapturerImpl>(id);
    if (!capturer) {
        AUDIO_ERR_LOG("register failed, invalid id of AudioCapturer");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    capturer->RegisterCallback(callbackType, callback, errorCode);
}

FFI_EXPORT void FfiMMAAudioCapturerOnWithFrame(
    int64_t id, int32_t callbackType, void (*callback)(), int64_t frame, int32_t* errorCode)
{
    auto capturer = FFIData::GetData<MMAAudioCapturerImpl>(id);
    if (!capturer) {
        AUDIO_ERR_LOG("register failed, invalid id of AudioCapturer");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    capturer->RegisterCallbackWithFrame(callbackType, callback, frame, errorCode);
}

// Audio Manager
FFI_EXPORT int64_t FfiMMACreateAudioManager(int32_t* errorCode)
{
    auto mgr = FFIData::Create<MMAAudioManagerImpl>();
    if (!mgr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Create AudioManager error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return mgr->GetID();
}

FFI_EXPORT int64_t FfiMMAAudioManagerGetRoutingManager(int64_t id, int32_t* errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("Get RoutingManager failed, invalid id of AudioManager");
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return mgr->GetRoutingManager(errorCode);
}

FFI_EXPORT int64_t FfiMMAAudioManagerGetStreamManager(int64_t id, int32_t* errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("Get StreamManager failed, invalid id of AudioManager");
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return mgr->GetStreamManager(errorCode);
}

FFI_EXPORT int64_t FfiMMAAudioManagerGetSessionManager(int64_t id, int32_t* errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("Get SessionManager failed, invalid id of AudioManager");
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return mgr->GetSessionManager(errorCode);
}

FFI_EXPORT int32_t FfiMMAAudioManagerGetAudioScene(int64_t id, int32_t* errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("Get AudioScene failed, invalid id of AudioManager");
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return mgr->GetAudioScene();
}

FFI_EXPORT int64_t FfiMMAAudioManagerGetVolumeManager(int64_t id, int32_t* errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioManagerImpl>(id);
    if (mgr == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("FfiMMAAudioManagerGetVolumeManager failed.");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return mgr->GetVolumeManager(errorCode);
}

// Audio Stream Manager
FFI_EXPORT bool FfiMMAASMIsActive(int64_t id, int32_t volumeType, int32_t* errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioStreamManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("IsActive failed, invalid id of AudioStreamManager");
        *errorCode = CJ_ERR_SYSTEM;
        return false;
    }
    return mgr->IsActive(volumeType);
}

FFI_EXPORT CArrAudioRendererChangeInfo FfiMMAASMGetCurrentAudioRendererInfoArray(int64_t id, int32_t* errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioStreamManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("Get CurrentAudioRendererInfoArray failed, invalid id of AudioStreamManager");
        *errorCode = CJ_ERR_SYSTEM;
        return CArrAudioRendererChangeInfo();
    }
    return mgr->GetCurrentRendererChangeInfos(errorCode);
}

FFI_EXPORT CArrAudioCapturerChangeInfo FfiMMAASMGetCurrentAudioCapturerInfoArray(int64_t id, int32_t* errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioStreamManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("Get CurrentAudioCapturerInfoArray failed, invalid id of AudioStreamManager");
        *errorCode = CJ_ERR_SYSTEM;
        return CArrAudioCapturerChangeInfo();
    }
    return mgr->GetAudioCapturerInfoArray(errorCode);
}

FFI_EXPORT CArrI32 FfiMMAASMGetAudioEffectInfoArray(int64_t id, int32_t usage, int32_t* errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioStreamManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("Get AudioEffectInfoArray failed, invalid id of AudioStreamManager");
        *errorCode = CJ_ERR_SYSTEM;
        return CArrI32();
    }
    return mgr->GetAudioEffectInfoArray(usage, errorCode);
}

FFI_EXPORT void FfiMMAASMOn(int64_t id, int32_t callbackType, void (*callback)(), int32_t* errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioStreamManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("register failed, invalid id of AudioStreamManager");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    mgr->RegisterCallback(callbackType, callback, errorCode);
}

// Audio Routing Manager
FFI_EXPORT void FfiMMAARMSetCommunicationDevice(int64_t id, int32_t deviceType, bool active, int32_t* errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioRoutingManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("SetCommunicationDevice failed, invalid id of AudioRoutingManager");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    *errorCode = mgr->SetCommunicationDevice(deviceType, active);
}

FFI_EXPORT bool FfiMMAARMIsCommunicationDeviceActive(int64_t id, int32_t deviceType, int32_t* errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioRoutingManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("IsCommunicationDeviceActive failed, invalid id of AudioRoutingManager");
        *errorCode = CJ_ERR_SYSTEM;
        return false;
    }
    return mgr->IsCommunicationDeviceActive(deviceType);
}

FFI_EXPORT CArrDeviceDescriptor FfiMMAARMGetDevices(int64_t id, int32_t deviceFlag, int32_t* errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioRoutingManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("GetDevices failed, invalid id of AudioRoutingManager");
        *errorCode = CJ_ERR_SYSTEM;
        return CArrDeviceDescriptor();
    }
    return mgr->GetDevices(deviceFlag, errorCode);
}

FFI_EXPORT CArrDeviceDescriptor FfiMMAARMGetAvailableDevices(int64_t id, uint32_t deviceUsage, int32_t* errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioRoutingManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("GetDevices failed, invalid id of AudioRoutingManager");
        *errorCode = CJ_ERR_SYSTEM;
        return CArrDeviceDescriptor();
    }
    return mgr->GetAvailableDevices(deviceUsage, errorCode);
}

FFI_EXPORT void FfiMMAARMFreeCArrDeviceDescriptor(CArrDeviceDescriptor deviceDescriptors)
{
    FreeCArrDeviceDescriptor(deviceDescriptors);
}

FFI_EXPORT CArrDeviceDescriptor FfiMMAARMGetPreferredInputDeviceForCapturerInfo(
    int64_t id, CAudioCapturerInfo capturerInfo, int32_t* errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioRoutingManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("GetPreferredInputDeviceForCapturerInfo failed, invalid id of AudioRoutingManager");
        *errorCode = CJ_ERR_SYSTEM;
        return CArrDeviceDescriptor();
    }
    return mgr->GetPreferredInputDeviceForCapturerInfo(capturerInfo, errorCode);
}

FFI_EXPORT CArrDeviceDescriptor FfiMMAARMGetPreferredOutputDeviceForRendererInfo(
    int64_t id, CAudioRendererInfo rendererInfo, int32_t* errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioRoutingManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("GetPreferredOutputDeviceForRendererInfo failed, invalid id of AudioRoutingManager");
        *errorCode = CJ_ERR_SYSTEM;
        return CArrDeviceDescriptor();
    }
    return mgr->GetPreferredOutputDeviceForRendererInfo(rendererInfo, errorCode);
}

FFI_EXPORT void FfiMMAARMOn(int64_t id, int32_t callbackType, uint32_t deviceUsage,
    void (*callback)(), int32_t* errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioRoutingManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("register failed, invalid id of AudioRoutingManager");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    mgr->RegisterCallback(callbackType, deviceUsage, callback, errorCode);
}

FFI_EXPORT void FfiMMAARMOnWithFlags(int64_t id, int32_t callbackType, void (*callback)(),
    int32_t flags, int32_t* errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioRoutingManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("register failed, invalid id of AudioRoutingManager");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    mgr->RegisterDeviceChangeCallback(callbackType, callback, flags, errorCode);
}

FFI_EXPORT void FfiMMAARMOnWithCapturerInfo(
    int64_t id, int32_t callbackType, void (*callback)(), CAudioCapturerInfo capturerInfo, int32_t* errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioRoutingManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("register failed, invalid id of AudioRoutingManager");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    mgr->RegisterPreferredInputDeviceChangeCallback(callbackType, callback, capturerInfo, errorCode);
}

FFI_EXPORT void FfiMMAARMOnWithRendererInfo(
    int64_t id, int32_t callbackType, void (*callback)(), CAudioRendererInfo rendererInfo, int32_t* errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioRoutingManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("register failed, invalid id of AudioRoutingManager");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    mgr->RegisterPreferredOutputDeviceChangeCallback(callbackType, callback, rendererInfo, errorCode);
}

/* Audio Session Manager */
FFI_EXPORT void FfiMMAASeMActivateAudioSession(int64_t id, CAudioSessionStrategy strategy, int32_t* errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioSessionManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("register failed, invalid id of AudioSessionManager");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    mgr->ActivateAudioSession(strategy, errorCode);
}

FFI_EXPORT void FfiMMAASeMDeactivateAudioSession(int64_t id, int32_t* errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioSessionManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("register failed, invalid id of AudioSessionManager");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    mgr->DeactivateAudioSession(errorCode);
}

FFI_EXPORT bool FfiMMAASeMIsAudioSessionActivated(int64_t id, int32_t* errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioSessionManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("register failed, invalid id of AudioSessionManager");
        *errorCode = CJ_ERR_SYSTEM;
        return false;
    }
    return mgr->IsAudioSessionActivated();
}

FFI_EXPORT void FfiMMAASeMOn(int64_t id, const char* type, int64_t callback, int32_t* errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioSessionManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("register failed, invalid id of AudioSessionManager");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    mgr->On(type, callback, errorCode);
}

/* Audio Volume Manager */
FFI_EXPORT int64_t FfiMMAAVMGetVolumeGroupManager(int64_t id, int32_t groupId, int32_t* errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioVolumeManagerImpl>(id);
    if (mgr == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("FfiMMAAVMGetVolumeGroupManager failed.");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return mgr->GetVolumeGroupManager(groupId, errorCode);
}

FFI_EXPORT void FfiMMAAVMOn(int64_t id, int32_t callbackType, void (*callback)(), int32_t* errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioVolumeManagerImpl>(id);
    if (mgr == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("FfiMMAAVMOn failed.");
        return;
    }
    return mgr->RegisterCallback(callbackType, callback, errorCode);
}

/* Audio Volumne Group Manager */
FFI_EXPORT int32_t FfiMMAAVGMGetMaxVolume(int64_t id, int32_t volumeType, int32_t* errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("FfiMMAAVGMGetMaxVolume error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->GetMaxVolume(volumeType);
}

FFI_EXPORT int32_t FfiMMAAVGMGetMinVolume(int64_t id, int32_t volumeType, int32_t* errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("FfiMMAAVGMGetMinVolume error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->GetMinVolume(volumeType);
}

FFI_EXPORT int32_t FfiMMAAVGMGetRingerMode(int64_t id, int32_t* errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("FfiMMAAVGMGetRingerMode error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->GetRingerMode();
}

FFI_EXPORT float FfiMMAAVGMGetSystemVolumeInDb(
    int64_t id, int32_t volumeType, int32_t volumeLevel, int32_t device, int32_t* errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("FfiMMAAVGMGetSystemVolumeInDb error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->GetSystemVolumeInDb(volumeType, volumeLevel, device);
}

FFI_EXPORT int32_t FfiMMAAVGMGetVolume(int64_t id, int32_t volumeType, int32_t* errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("FfiMMAAVGMGetVolume error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->GetVolume(volumeType);
}

FFI_EXPORT bool FfiMMAAVGMIsMicrophoneMute(int64_t id, int32_t* errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Get FfiMMAAVGMIsMicrophoneMute error");
        return false;
    }
    *errorCode = SUCCESS_CODE;
    return inst->IsMicrophoneMute();
}

FFI_EXPORT bool FfiMMAAVGMIsMute(int64_t id, int32_t volumeType, int32_t* errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Get FfiMMAAVGMIsMute error");
        return false;
    }
    *errorCode = SUCCESS_CODE;
    return inst->IsMute(volumeType);
}

FFI_EXPORT bool FfiMMAAVGMIsVolumeUnadjustable(int64_t id, int32_t* errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Get FfiMMAAVGMIsVolumeUnadjustable error");
        return false;
    }
    *errorCode = SUCCESS_CODE;
    return inst->IsVolumeUnadjustable();
}

/* CAPABILITY FOR LAST CANGJIE VERSION AND WILL BE REMOVED */
/* Audio Manager */
FFI_EXPORT int64_t FfiMMAGetVolumeManager(int64_t id, int32_t* errorCode)
{
    auto mgr = FFIData::Create<MMAAudioVolumeManagerImpl>();
    if (mgr == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("FfiMMACreateAudioManager failed.");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return mgr->GetID();
}

/* Audio Volume Manager */
FFI_EXPORT int64_t FfiMMAGetVolumeGroupManager(int64_t id, int32_t* errorCode)
{
    auto mgr = FFIData::Create<MMAAudioVolumeManagerImpl>();
    if (mgr == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("FfiMMACreateAudioManager failed.");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return mgr->GetID();
}

/* Audio Volumne Group Manager */
FFI_EXPORT int32_t FfiMMAGetMaxVolume(int64_t id, int32_t volumeType, int32_t* errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Get MMAAudioVolumeGroupManagerImpl error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->GetMaxVolume(volumeType);
}

FFI_EXPORT int32_t FfiMMAGetMinVolume(int64_t id, int32_t volumeType, int32_t* errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Get MMAAudioVolumeGroupManagerImpl error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->GetMinVolume(volumeType);
}

FFI_EXPORT int32_t FfiMMAGetRingerMode(int64_t id, int32_t* errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Get MMAAudioVolumeGroupManagerImpl error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->GetRingerMode();
}

FFI_EXPORT float FfiMMAGetSystemVolumeInDb(int64_t id, int32_t volumeType, int32_t volumeLevel,
    int32_t device, int32_t* errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Get MMAAudioVolumeGroupManagerImpl error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->GetSystemVolumeInDb(volumeType, volumeLevel, device);
}

FFI_EXPORT int32_t FfiMMAGetVolume(int64_t id, int32_t volumeType, int32_t* errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Get MMAAudioVolumeGroupManagerImpl error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->GetVolume(volumeType);
}

FFI_EXPORT bool FfiMMAIsMicrophoneMute(int64_t id, int32_t* errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Get MMAAudioVolumeGroupManagerImpl error");
        return false;
    }
    *errorCode = SUCCESS_CODE;
    return inst->IsMicrophoneMute();
}

FFI_EXPORT bool FfiMMAIsMute(int64_t id, int32_t volumeType, int32_t* errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Get MMAAudioVolumeGroupManagerImpl error");
        return false;
    }
    *errorCode = SUCCESS_CODE;
    return inst->IsMute(volumeType);
}

FFI_EXPORT bool FfiMMAIsVolumeUnadjustable(int64_t id, int32_t* errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Get MMAAudioVolumeGroupManagerImpl error");
        return false;
    }
    *errorCode = SUCCESS_CODE;
    return inst->IsVolumeUnadjustable();
}

FFI_EXPORT float FfiMMAAVGMGetMaxAmplitudeForOutputDevice(int64_t id, CDeviceDescriptor desc, int32_t* errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Get MMAAudioVolumeGroupManagerImpl error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->GetMaxAmplitudeForOutputDevice(desc.id);
}

FFI_EXPORT float FfiMMAAVGMGetMaxAmplitudeForInputDevice(int64_t id, CDeviceDescriptor desc, int32_t* errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Get MMAAudioVolumeGroupManagerImpl error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->GetMaxAmplitudeForInputDevice(desc.id);
}

FFI_EXPORT void FfiMMAAVGMOn(int64_t id, int32_t callbackType, void (*callback)(), int32_t* errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Get MMAAudioVolumeGroupManagerImpl error");
        return;
    }
    *errorCode = SUCCESS_CODE;
    inst->RegisterCallback(callbackType, callback, errorCode);
}

/* Audio Renderer */
FFI_EXPORT int64_t FfiMMACreateAudioRenderer(CAudioRendererOptions options, int32_t* errorCode)
{
    auto renderer = FFIData::Create<MMAAudioRendererImpl>();
    if (renderer == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Create MMAAudioRendererImpl error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    auto ret = renderer->CreateAudioRenderer(options);
    if (ret != SUCCESS_CODE) {
        FFIData::Release(renderer->GetID());
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("CreateAudioRenderer error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return renderer->GetID();
}

FFI_EXPORT int32_t FfiMMAARGetState(int64_t id, int32_t* errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("GetState failed, invalid id of AudioRenderer");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return renderer->GetState();
}

FFI_EXPORT int64_t FfiMMAARGetAudioTime(int64_t id, int32_t* errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("GetAudioTime failed, invalid id of AudioRenderer");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return renderer->GetAudioTime(errorCode);
}

FFI_EXPORT uint32_t FfiMMAARGetBufferSize(int64_t id, int32_t* errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("GetBufferSize failed, invalid id of AudioRenderer");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return renderer->GetBufferSize(errorCode);
}

FFI_EXPORT void FfiMMAARFlush(int64_t id, int32_t* errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Flush failed, invalid id of AudioRenderer");
        return;
    }
    auto isSuccess = renderer->Flush();
    if (isSuccess != SUCCESS_CODE) {
        *errorCode = isSuccess;
    }
}

FFI_EXPORT void FfiMMAARPause(int64_t id, int32_t* errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Pause failed, invalid id of AudioRenderer");
        return;
    }
    auto isSuccess = renderer->Pause();
    if (isSuccess != SUCCESS_CODE) {
        *errorCode = isSuccess;
    }
}

FFI_EXPORT void FfiMMAARDrain(int64_t id, int32_t* errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Drain failed, invalid id of AudioRenderer");
        return;
    }
    auto isSuccess = renderer->Drain();
    if (isSuccess != SUCCESS_CODE) {
        *errorCode = isSuccess;
    }
}

FFI_EXPORT CArrDeviceDescriptor FfiMMAARGetCurrentOutputDevices(int64_t id, int32_t* errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        AUDIO_ERR_LOG("GetCurrentOutputDevices failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return CArrDeviceDescriptor();
    }
    return renderer->GetCurrentOutputDevices(errorCode);
}

FFI_EXPORT double FfiMMAARGetSpeed(int64_t id, int32_t* errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        AUDIO_ERR_LOG("GetSpeed failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_DOUBLE_VALUE;
    }
    return renderer->GetSpeed(errorCode);
}

FFI_EXPORT bool FfiMMAARGetSilentModeAndMixWithOthers(int64_t id, int32_t* errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        AUDIO_ERR_LOG("GetSilentModeAndMixWithOthers failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return false;
    }
    return renderer->GetSilentModeAndMixWithOthers(errorCode);
}

FFI_EXPORT double FfiMMAARGetVolume(int64_t id, int32_t* errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        AUDIO_ERR_LOG("GetVolume failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_DOUBLE_VALUE;
    }
    return renderer->GetVolume(errorCode);
}

FFI_EXPORT uint32_t FfiMMAARGetUnderflowCount(int64_t id, int32_t* errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        AUDIO_ERR_LOG("GetUnderflowCount failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return renderer->GetUnderflowCount(errorCode);
}

FFI_EXPORT void FfiMMAARSetVolumeWithRamp(int64_t id, double volume, int32_t duration, int32_t* errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        AUDIO_ERR_LOG("SetVolumeWithRamp failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    renderer->SetVolumeWithRamp(volume, duration, errorCode);
}

FFI_EXPORT void FfiMMAARSetSpeed(int64_t id, double speed, int32_t* errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        AUDIO_ERR_LOG("SetSpeed failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    renderer->SetSpeed(speed, errorCode);
}

FFI_EXPORT void FfiMMAARSetVolume(int64_t id, double volume, int32_t* errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        AUDIO_ERR_LOG("SetVolume failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    renderer->SetVolume(volume, errorCode);
}

FFI_EXPORT void FfiMMAARSetSilentModeAndMixWithOthers(int64_t id, bool on, int32_t* errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        AUDIO_ERR_LOG("SetSilentModeAndMixWithOthers failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    renderer->SetSilentModeAndMixWithOthers(on, errorCode);
}

FFI_EXPORT void FfiMMAARSetInterruptMode(int64_t id, int32_t mode, int32_t* errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        AUDIO_ERR_LOG("SetInterruptMode failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    renderer->SetInterruptMode(mode, errorCode);
}

FFI_EXPORT void FfiMMAARSetChannelBlendMode(int64_t id, int32_t mode, int32_t* errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        AUDIO_ERR_LOG("SetChannelBlendMode failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    renderer->SetChannelBlendMode(mode, errorCode);
}

FFI_EXPORT void FfiMMAARSetDefaultOutputDevice(int64_t id, int32_t deviceType, int32_t* errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        AUDIO_ERR_LOG("SetDefaultOutputDevice failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    renderer->SetDefaultOutputDevice(deviceType, errorCode);
}

FFI_EXPORT void FfiMMAAROnWithFrame(int64_t id, int32_t callbackType, void (*callback)(),
    int64_t frame, int32_t* errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        AUDIO_ERR_LOG("register failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    renderer->RegisterCallbackWithFrame(callbackType, callback, frame, errorCode);
}

FFI_EXPORT int32_t FfiMMAARGetAudioEffectMode(int64_t id, int32_t* errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (!renderer) {
        AUDIO_ERR_LOG("GetAudioEffectMode failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return renderer->GetAudioEffectMode(errorCode);
}

FFI_EXPORT void FfiMMAARSetAudioEffectMode(int64_t id, int32_t mode, int32_t* errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (!renderer) {
        AUDIO_ERR_LOG("SetAudioEffectMode failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    return renderer->SetAudioEffectMode(mode, errorCode);
}

FFI_EXPORT double FfiMMAARGetMinStreamVolume(int64_t id, int32_t* errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (!renderer) {
        AUDIO_ERR_LOG("GetMinStreamVolume failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_DOUBLE_VALUE;
    }
    return renderer->GetMinStreamVolume(errorCode);
}

FFI_EXPORT double FfiMMAARGetMaxStreamVolume(int64_t id, int32_t* errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (!renderer) {
        AUDIO_ERR_LOG("GetMaxStreamVolume failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_DOUBLE_VALUE;
    }
    return renderer->GetMaxStreamVolume(errorCode);
}

FFI_EXPORT void FfiMMAARRelease(int64_t id, int32_t* errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (!renderer) {
        AUDIO_ERR_LOG("Release failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    return renderer->Release(errorCode);
}

FFI_EXPORT uint32_t FfiMMAARGetStreamId(int64_t id, int32_t* errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (!renderer) {
        AUDIO_ERR_LOG("GetStreamId failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return renderer->GetStreamId(errorCode);
}

FFI_EXPORT void FfiMMAARStop(int64_t id, int32_t* errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (!renderer) {
        AUDIO_ERR_LOG("Stop failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    return renderer->Stop(errorCode);
}

FFI_EXPORT void FfiMMAARStart(int64_t id, int32_t* errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (!renderer) {
        AUDIO_ERR_LOG("Start failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    return renderer->Start(errorCode);
}

FFI_EXPORT CAudioStreamInfo FfiMMAARGetStreamInfo(int64_t id, int32_t* errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (!renderer) {
        AUDIO_ERR_LOG("GetStreamInfo failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return CAudioStreamInfo();
    }
    return renderer->GetStreamInfo(errorCode);
}

FFI_EXPORT CAudioRendererInfo FfiMMAARGetRendererInfo(int64_t id, int32_t* errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (!renderer) {
        AUDIO_ERR_LOG("GetRendererInfo failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return CAudioRendererInfo();
    }
    return renderer->GetRendererInfo(errorCode);
}

FFI_EXPORT void FfiMMAAROn(int64_t id, int32_t callbackType, void (*callback)(), int32_t* errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        AUDIO_ERR_LOG("register failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    renderer->RegisterCallback(callbackType, callback, errorCode);
}
}
} // namespace AudioStandard
} // namespace OHOS
