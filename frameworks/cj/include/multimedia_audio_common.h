/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef MULTIMEDIA_AUDIO_COMMON_H
#define MULTIMEDIA_AUDIO_COMMON_H
#include "audio_info.h"
#include "audio_system_manager.h"
#include "cj_common_ffi.h"
#include "multimedia_audio_ffi.h"
#include "securec.h"

namespace OHOS {
namespace AudioStandard {
const size_t MAX_MEM_MALLOC_SIZE = 50 * 1024 * 1024;
AudioVolumeType GetNativeAudioVolumeType(int32_t volumeType);
void Convert2AudioCapturerOptions(AudioCapturerOptions& opions, const CAudioCapturerOptions& cOptions);
char* MallocCString(const std::string& origin);
void Convert2CAudioCapturerInfo(CAudioCapturerInfo& cInfo, const AudioCapturerInfo& capturerInfo);
void Convert2CAudioStreamInfo(CAudioStreamInfo& cInfo, const AudioStreamInfo& streamInfo);
void Convert2CAudioCapturerChangeInfo(
    CAudioCapturerChangeInfo& cInfo, const AudioCapturerChangeInfo& changeInfo, int32_t* errorCode);
void Convert2CArrDeviceDescriptor(CArrDeviceDescriptor& devices,
    const std::vector<std::shared_ptr<AudioDeviceDescriptor>>& deviceDescriptors, int32_t* errorCode);
void Convert2CDeviceDescriptor(CDeviceDescriptor* device, const AudioDeviceDescriptor& deviceInfo, int32_t* errorCode);
void Convert2CArrDeviceDescriptorByDeviceInfo(
    CArrDeviceDescriptor& devices, const AudioDeviceDescriptor& deviceInfo, int32_t* errorCode);
void ConvertAudioDeviceDescriptor2DeviceInfo(
    AudioDeviceDescriptor& deviceInfo, std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor);
void FreeCDeviceDescriptor(CDeviceDescriptor& device);
void FreeCArrDeviceDescriptor(CArrDeviceDescriptor& devices);
void FreeCArrAudioCapturerChangeInfo(CArrAudioCapturerChangeInfo& infos);
void FreeCArrAudioRendererChangeInfo(CArrAudioRendererChangeInfo& infos);
void Convert2AudioRendererOptions(AudioRendererOptions& opions, const CAudioRendererOptions& cOptions);
void Convert2AudioRendererInfo(CAudioRendererInfo& cInfo, const AudioRendererInfo& rendererInfo);
void Convert2CAudioRendererChangeInfo(
    CAudioRendererChangeInfo& cInfo, const AudioRendererChangeInfo& changeInfo, int32_t* errorCode);
} // namespace AudioStandard
} // namespace OHOS
#endif // MULTIMEDIA_AUDIO_COMMON_H
