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
#ifndef LOG_TAG
#define LOG_TAG "AudioSelectInterfaceService"
#endif

#include "audio_select_interface_service.h"

#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
int32_t AudioSelectInterfaceService::SetDeviceActive(DeviceType deviceType, bool active,
    const std::string &address, const int32_t uid)
{
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc, const int32_t audioDeviceSelectMode,
    const bool isNeedNotifyBt)
{
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc)
{
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::SelectInputDeviceByUid(const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc,
    int32_t uid)
{
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::ExcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs)
{
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs)
{
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::SetSessionDefaultOutputDevice(const int32_t callerPid,
    const DeviceType &deviceType)
{
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::SetDefaultOutputDevice(
    const DeviceType deviceType, const uint32_t sessionID, const StreamUsage streamUsage, bool isRunning,
    bool skipForce)
{
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::SetInputDevice(const DeviceType deviceType, const uint32_t sessionID,
    const SourceType sourceType, bool isRunning)
{
    return SUCCESS;
}

int32_t AudioSelectInterfaceService::PreferBluetoothAndNearlinkRecordByUid(int32_t uid,
    BluetoothAndNearlinkPreferredRecordCategory category)
{
    return SUCCESS;
}

void AudioSelectInterfaceService::SetPreferredInputDeviceIfValid(std::shared_ptr<AudioStreamDescriptor> streamDesc)
{
    return;
}

void AudioSelectInterfaceService::OnForcedDeviceSelected(DeviceType devType, const std::string &macAddress,
    sptr<AudioRendererFilter> filter, const std::string &caller)
{
    return;
}

void AudioSelectInterfaceService::OnPrivacyDeviceSelected(DeviceType devType, const std::string &macAddress,
    const std::string &caller)
{
    return;
}

} // namespace AudioStandard
} // namespace OHOS
