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
#include "multimedia_audio_error.h"
#include "multimedia_audio_manager_impl.h"
#include "multimedia_audio_renderer_callback.h"
#include "multimedia_audio_renderer_impl.h"
#include "multimedia_audio_routing_manager_callback.h"
#include "multimedia_audio_routing_manager_impl.h"
#include "multimedia_audio_stream_manager_callback.h"
#include "multimedia_audio_stream_manager_impl.h"
#include "multimedia_audio_volume_group_manager_impl.h"
#include "multimedia_audio_volume_manager_impl.h"

using namespace OHOS::FFI;

namespace OHOS {
namespace AudioStandard {
extern "C" {
// Audio Capturer
int64_t FfiMMACreateAudioCapturer(CAudioCapturerOptions options, int32_t *errorCode)
{
    auto capturer = FFIData::Create<MMAAudioCapturerImpl>();
    if (!capturer) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Create MMAAudioCapturerImpl error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    auto ret = capturer->CreateAudioCapturer(options);
    if (ret != SUCCESS_CODE) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("CreateAudioCapturer error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return capturer->GetID();
}

int32_t FfiMMAAudioCapturerGetState(int64_t id, int32_t *errorCode)
{
    auto capturer = FFIData::GetData<MMAAudioCapturerImpl>(id);
    if (!capturer) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("get state failed, invalid id of AudioCapturer");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return capturer->GetState();
}

uint32_t FfiMMAAudioCapturerGetStreamId(int64_t id, int32_t *errorCode)
{
    auto capturer = FFIData::GetData<MMAAudioCapturerImpl>(id);
    if (!capturer) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("get stream id failed, invalid id of AudioCapturer");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return capturer->GetStreamId(errorCode);
}

int64_t FfiMMAAudioCapturerGetAudioTime(int64_t id, int32_t *errorCode)
{
    auto capturer = FFIData::GetData<MMAAudioCapturerImpl>(id);
    if (!capturer) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("get audio time failed, invalid id of AudioCapturer");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return capturer->GetAudioTime(errorCode);
}

uint32_t FfiMMAAudioCapturerGetBufferSize(int64_t id, int32_t *errorCode)
{
    auto capturer = FFIData::GetData<MMAAudioCapturerImpl>(id);
    if (!capturer) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("get buffer size failed, invalid id of AudioCapturer");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return capturer->GetBufferSize(errorCode);
}

uint32_t FfiMMAAudioCapturerGetOverflowCount(int64_t id, int32_t *errorCode)
{
    auto capturer = FFIData::GetData<MMAAudioCapturerImpl>(id);
    if (!capturer) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("get OverflowCount failed, invalid id of AudioCapturer");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return capturer->GetOverflowCount();
}

void FfiMMAAudioCapturerStart(int64_t id, int32_t *errorCode)
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

void FfiMMAAudioCapturerStop(int64_t id, int32_t *errorCode)
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

void FfiMMAAudioCapturerRelease(int64_t id, int32_t *errorCode)
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

CAudioCapturerChangeInfo FfiMMAAudioCapturerGetAudioCapturerChangeInfo(int64_t id, int32_t *errorCode)
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

CArrDeviceDescriptor FfiMMAAudioCapturerGetInputDevices(int64_t id, int32_t *errorCode)
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

CAudioCapturerInfo FfiMMAAudioCapturerGetCapturerInfo(int64_t id, int32_t *errorCode)
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

CAudioStreamInfo FfiMMAAudioCapturerGetStreamInfo(int64_t id, int32_t *errorCode)
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

void FfiMMAAudioCapturerOn(int64_t id, int32_t callbackType, void (*callback)(), int32_t *errorCode)
{
    auto capturer = FFIData::GetData<MMAAudioCapturerImpl>(id);
    if (!capturer) {
        AUDIO_ERR_LOG("register failed, invalid id of AudioCapturer");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    capturer->RegisterCallback(callbackType, callback, errorCode);
}

void FfiMMAAudioCapturerOnWithFrame(int64_t id, int32_t callbackType, void (*callback)(), int64_t frame,
    int32_t *errorCode)
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
int64_t FfiMMACreateAudioManager(int32_t *errorCode)
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

int64_t FfiMMAAudioManagerGetRoutingManager(int64_t id, int32_t *errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("Get RoutingManager failed, invalid id of AudioManager");
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return mgr->GetRoutingManager(errorCode);
}

int64_t FfiMMAAudioManagerGetStreamManager(int64_t id, int32_t *errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("Get StreamManager failed, invalid id of AudioManager");
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return mgr->GetStreamManger(errorCode);
}

int32_t FfiMMAAudioManagerGetAudioScene(int64_t id, int32_t *errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("Get AudioScene failed, invalid id of AudioManager");
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return mgr->GetAudioScene();
}

int64_t FfiMMAAudioManagerGetVolumeManager(int64_t id, int32_t *errorCode)
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
bool FfiMMAASMIsActive(int64_t id, int32_t volumeType, int32_t *errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioStreamManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("IsActive failed, invalid id of AudioStreamManager");
        *errorCode = CJ_ERR_SYSTEM;
        return false;
    }
    return mgr->IsActive(volumeType);
}

CArrAudioRendererChangeInfo FfiMMAASMGetCurrentAudioRendererInfoArray(int64_t id, int32_t *errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioStreamManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("Get CurrentAudioRendererInfoArray failed, invalid id of AudioStreamManager");
        *errorCode = CJ_ERR_SYSTEM;
        return CArrAudioRendererChangeInfo();
    }
    return mgr->GetCurrentRendererChangeInfos(errorCode);
}

CArrAudioCapturerChangeInfo FfiMMAASMGetCurrentAudioCapturerInfoArray(int64_t id, int32_t *errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioStreamManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("Get CurrentAudioCapturerInfoArray failed, invalid id of AudioStreamManager");
        *errorCode = CJ_ERR_SYSTEM;
        return CArrAudioCapturerChangeInfo();
    }
    return mgr->GetAudioCapturerInfoArray(errorCode);
}

CArrI32 FfiMMAASMGetAudioEffectInfoArray(int64_t id, int32_t usage, int32_t *errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioStreamManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("Get AudioEffectInfoArray failed, invalid id of AudioStreamManager");
        *errorCode = CJ_ERR_SYSTEM;
        return CArrI32();
    }
    return mgr->GetAudioEffectInfoArray(usage, errorCode);
}

void FfiMMAASMOn(int64_t id, int32_t callbackType, void (*callback)(), int32_t *errorCode)
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
void FfiMMAARMSetCommunicationDevice(int64_t id, int32_t deviceType, bool active, int32_t *errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioRoutingManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("SetCommunicationDevice failed, invalid id of AudioRoutingManager");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    *errorCode = mgr->SetCommunicationDevice(deviceType, active);
}

bool FfiMMAARMIsCommunicationDeviceActive(int64_t id, int32_t deviceType, int32_t *errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioRoutingManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("IsCommunicationDeviceActive failed, invalid id of AudioRoutingManager");
        *errorCode = CJ_ERR_SYSTEM;
        return false;
    }
    return mgr->IsCommunicationDeviceActive(deviceType);
}

CArrDeviceDescriptor FfiMMAARMGetDevices(int64_t id, int32_t deviceFlag, int32_t *errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioRoutingManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("GetDevices failed, invalid id of AudioRoutingManager");
        *errorCode = CJ_ERR_SYSTEM;
        return CArrDeviceDescriptor();
    }
    return mgr->GetDevices(deviceFlag, errorCode);
}

CArrDeviceDescriptor FfiMMAARMGetPreferredInputDeviceForCapturerInfo(int64_t id, CAudioCapturerInfo capturerInfo,
    int32_t *errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioRoutingManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("GetPreferredInputDeviceForCapturerInfo failed, invalid id of AudioRoutingManager");
        *errorCode = CJ_ERR_SYSTEM;
        return CArrDeviceDescriptor();
    }
    return mgr->GetPreferredInputDeviceForCapturerInfo(capturerInfo, errorCode);
}

CArrDeviceDescriptor FfiMMAARMGetPreferredOutputDeviceForRendererInfo(int64_t id,
    CAudioRendererInfo rendererInfo, int32_t *errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioRoutingManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("GetPreferredOutputDeviceForRendererInfo failed, invalid id of AudioRoutingManager");
        *errorCode = CJ_ERR_SYSTEM;
        return CArrDeviceDescriptor();
    }
    return mgr->GetPreferredOutputDeviceForRendererInfo(rendererInfo, errorCode);
}

void FfiMMAARMOn(int64_t id, int32_t callbackType, uint32_t deviceUsage, void (*callback)(), int32_t *errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioRoutingManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("register failed, invalid id of AudioRoutingManager");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    mgr->RegisterCallback(callbackType, deviceUsage, callback, errorCode);
}

void FfiMMAARMOnWithFlags(int64_t id, int32_t callbackType, void (*callback)(), int32_t flags, int32_t *errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioRoutingManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("register failed, invalid id of AudioRoutingManager");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    mgr->RegisterDeviceChangeCallback(callbackType, callback, flags, errorCode);
}

void FfiMMAARMOnWithCapturerInfo(int64_t id, int32_t callbackType, void (*callback)(), CAudioCapturerInfo capturerInfo,
    int32_t *errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioRoutingManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("register failed, invalid id of AudioRoutingManager");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    mgr->RegisterPreferredInputDeviceChangeCallback(callbackType, callback, capturerInfo, errorCode);
}

void FfiMMAARMOnWithRendererInfo(int64_t id, int32_t callbackType, void (*callback)(), CAudioRendererInfo rendererInfo,
    int32_t *errorCode)
{
    auto mgr = FFIData::GetData<MMAAudioRoutingManagerImpl>(id);
    if (!mgr) {
        AUDIO_ERR_LOG("register failed, invalid id of AudioRoutingManager");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    mgr->RegisterPreferredOutputDeviceChangeCallback(callbackType, callback, rendererInfo, errorCode);
}

/* Audio Volume Manager */
int64_t FfiMMAAVMGetVolumeGroupManager(int64_t id, int32_t groupId, int32_t *errorCode)
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

void FfiMMAAVMOn(int64_t id, int32_t callbackType, void (*callback)(), int32_t *errorCode)
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
int32_t FfiMMAAVGMGetMaxVolume(int64_t id, int32_t volumeType, int32_t *errorCode)
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

int32_t FfiMMAAVGMGetMinVolume(int64_t id, int32_t volumeType, int32_t *errorCode)
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

int32_t FfiMMAAVGMGetRingerMode(int64_t id, int32_t *errorCode)
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

float FfiMMAAVGMGetSystemVolumeInDb(int64_t id, int32_t volumeType, int32_t volumeLevel, int32_t device,
    int32_t *errorCode)
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

int32_t FfiMMAAVGMGetVolume(int64_t id, int32_t volumeType, int32_t *errorCode)
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

bool FfiMMAAVGMIsMicrophoneMute(int64_t id, int32_t *errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Get FfiMMAAVGMIsMicrophoneMute error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->IsMicrophoneMute();
}

bool FfiMMAAVGMIsMute(int64_t id, int32_t volumeType, int32_t *errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Get FfiMMAAVGMIsMute error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->IsMute(volumeType);
}

bool FfiMMAAVGMIsVolumeUnadjustable(int64_t id, int32_t *errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Get FfiMMAAVGMIsVolumeUnadjustable error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->IsVolumeUnadjustable();
}

/* CAPABILITY FOR LAST CANGJIE VERSION AND WILL BE REMOVED */
/* Audio Manager */
int64_t FfiMMAGetVolumeManager(int64_t id, int32_t *errorCode)
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
int64_t FfiMMAGetVolumeGroupManager(int64_t id, int32_t *errorCode)
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
int32_t FfiMMAGetMaxVolume(int64_t id, int32_t volumeType, int32_t *errorCode)
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

int32_t FfiMMAGetMinVolume(int64_t id, int32_t volumeType, int32_t *errorCode)
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

int32_t FfiMMAGetRingerMode(int64_t id, int32_t *errorCode)
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

float FfiMMAGetSystemVolumeInDb(int64_t id, int32_t volumeType, int32_t volumeLevel, int32_t device, int32_t *errorCode)
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

int32_t FfiMMAGetVolume(int64_t id, int32_t volumeType, int32_t *errorCode)
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

bool FfiMMAIsMicrophoneMute(int64_t id, int32_t *errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Get MMAAudioVolumeGroupManagerImpl error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->IsMicrophoneMute();
}

bool FfiMMAIsMute(int64_t id, int32_t volumeType, int32_t *errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Get MMAAudioVolumeGroupManagerImpl error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->IsMute(volumeType);
}

bool FfiMMAIsVolumeUnadjustable(int64_t id, int32_t *errorCode)
{
    auto inst = FFIData::GetData<MMAAudioVolumeGroupManagerImpl>(id);
    if (inst == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Get MMAAudioVolumeGroupManagerImpl error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return inst->IsVolumeUnadjustable();
}

float FfiMMAAVGMGetMaxAmplitudeForOutputDevice(int64_t id, CDeviceDescriptor desc, int32_t *errorCode)
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

float FfiMMAAVGMGetMaxAmplitudeForInputDevice(int64_t id, CDeviceDescriptor desc, int32_t *errorCode)
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

void FfiMMAAVGMOn(int64_t id, int32_t callbackType, void (*callback)(), int32_t *errorCode)
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
int64_t FfiMMACreateAudioRenderer(CAudioRendererOptions options, int32_t *errorCode)
{
    auto renderer = FFIData::Create<MMAAudioRendererImpl>();
    if (renderer == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("Create MMAAudioRendererImpl error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    auto ret = renderer->CreateAudioRenderer(options);
    if (ret != SUCCESS_CODE) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("CreateAudioRenderer error");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    *errorCode = SUCCESS_CODE;
    return renderer->GetID();
}

int32_t FfiMMAARGetState(int64_t id, int32_t *errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("GetState failed, invalid id of AudioRenderer");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return renderer->GetState();
}

int64_t FfiMMAARGetAudioTime(int64_t id, int32_t *errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("GetAudioTime failed, invalid id of AudioRenderer");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return renderer->GetAudioTime(errorCode);
}

uint32_t FfiMMAARGetBufferSize(int64_t id, int32_t *errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        *errorCode = CJ_ERR_SYSTEM;
        AUDIO_ERR_LOG("GetBufferSize failed, invalid id of AudioRenderer");
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return renderer->GetBufferSize(errorCode);
}

void FfiMMAARFlush(int64_t id, int32_t *errorCode)
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

void FfiMMAARPause(int64_t id, int32_t *errorCode)
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

void FfiMMAARDrain(int64_t id, int32_t *errorCode)
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

CArrDeviceDescriptor FfiMMAARGetCurrentOutputDevices(int64_t id, int32_t *errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        AUDIO_ERR_LOG("GetCurrentOutputDevices failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return CArrDeviceDescriptor();
    }
    return renderer->GetCurrentOutputDevices(errorCode);
}

double FfiMMAARGetSpeed(int64_t id, int32_t *errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        AUDIO_ERR_LOG("GetSpeed failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_DOUBLE_VALUE;
    }
    return renderer->GetSpeed(errorCode);
}

bool FfiMMAARGetSilentModeAndMixWithOthers(int64_t id, int32_t *errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        AUDIO_ERR_LOG("GetSilentModeAndMixWithOthers failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return false;
    }
    return renderer->GetSilentModeAndMixWithOthers(errorCode);
}

double FfiMMAARGetVolume(int64_t id, int32_t *errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        AUDIO_ERR_LOG("GetVolume failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_DOUBLE_VALUE;
    }
    return renderer->GetVolume(errorCode);
}

uint32_t FfiMMAARGetUnderflowCount(int64_t id, int32_t *errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        AUDIO_ERR_LOG("GetUnderflowCount failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return renderer->GetUnderflowCount(errorCode);
}

void FfiMMAARSetVolumeWithRamp(int64_t id, double volume, int32_t duration, int32_t *errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        AUDIO_ERR_LOG("SetVolumeWithRamp failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    renderer->SetVolumeWithRamp(volume, duration, errorCode);
}

void FfiMMAARSetSpeed(int64_t id, double speed, int32_t *errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        AUDIO_ERR_LOG("SetSpeed failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    renderer->SetSpeed(speed, errorCode);
}

void FfiMMAARSetVolume(int64_t id, double volume, int32_t *errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        AUDIO_ERR_LOG("SetVolume failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    renderer->SetVolume(volume, errorCode);
}

void FfiMMAARSetSilentModeAndMixWithOthers(int64_t id, bool on, int32_t *errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        AUDIO_ERR_LOG("SetSilentModeAndMixWithOthers failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    renderer->SetSilentModeAndMixWithOthers(on, errorCode);
}

void FfiMMAARSetInterruptMode(int64_t id, int32_t mode, int32_t *errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        AUDIO_ERR_LOG("SetInterruptMode failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    renderer->SetInterruptMode(mode, errorCode);
}

void FfiMMAARSetChannelBlendMode(int64_t id, int32_t mode, int32_t *errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        AUDIO_ERR_LOG("SetInterruptMode failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    renderer->SetChannelBlendMode(mode, errorCode);
}

void FfiMMAAROnWithFrame(int64_t id, int32_t callbackType, void (*callback)(), int64_t frame, int32_t *errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (renderer == nullptr) {
        AUDIO_ERR_LOG("register failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    renderer->RegisterCallbackWithFrame(callbackType, callback, frame, errorCode);
}

int32_t FfiMMAARGetAudioEffectMode(int64_t id, int32_t *errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (!renderer) {
        AUDIO_ERR_LOG("GetAudioEffectMode failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return renderer->GetAudioEffectMode(errorCode);
}

void FfiMMAARSetAudioEffectMode(int64_t id, int32_t mode, int32_t *errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (!renderer) {
        AUDIO_ERR_LOG("SetAudioEffectMode failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    return renderer->SetAudioEffectMode(mode, errorCode);
}

double FfiMMAARGetMinStreamVolume(int64_t id, int32_t *errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (!renderer) {
        AUDIO_ERR_LOG("GetMinStreamVolume failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_DOUBLE_VALUE;
    }
    return renderer->GetMinStreamVolume(errorCode);
}

double FfiMMAARGetMaxStreamVolume(int64_t id, int32_t *errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (!renderer) {
        AUDIO_ERR_LOG("GetMaxStreamVolume failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_DOUBLE_VALUE;
    }
    return renderer->GetMaxStreamVolume(errorCode);
}

void FfiMMAARRelease(int64_t id, int32_t *errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (!renderer) {
        AUDIO_ERR_LOG("Release failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    return renderer->Release(errorCode);
}

uint32_t FfiMMAARGetStreamId(int64_t id, int32_t *errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (!renderer) {
        AUDIO_ERR_LOG("GetStreamId failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return CJ_ERR_INVALID_RETURN_VALUE;
    }
    return renderer->GetStreamId(errorCode);
}

void FfiMMAARStop(int64_t id, int32_t *errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (!renderer) {
        AUDIO_ERR_LOG("Stop failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    return renderer->Stop(errorCode);
}

void FfiMMAARStart(int64_t id, int32_t *errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (!renderer) {
        AUDIO_ERR_LOG("Start failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return;
    }
    return renderer->Start(errorCode);
}

CAudioStreamInfo FfiMMAARGetStreamInfo(int64_t id, int32_t *errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (!renderer) {
        AUDIO_ERR_LOG("GetStreamInfo failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return CAudioStreamInfo();
    }
    return renderer->GetStreamInfo(errorCode);
}

CAudioRendererInfo FfiMMAARGetRendererInfo(int64_t id, int32_t *errorCode)
{
    auto renderer = FFIData::GetData<MMAAudioRendererImpl>(id);
    if (!renderer) {
        AUDIO_ERR_LOG("GetRendererInfo failed, invalid id of AudioRenderer");
        *errorCode = CJ_ERR_SYSTEM;
        return CAudioRendererInfo();
    }
    return renderer->GetRendererInfo(errorCode);
}

void FfiMMAAROn(int64_t id, int32_t callbackType, void (*callback)(), int32_t *errorCode)
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
