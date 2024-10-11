/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef POLICY_HANDLER_H
#define POLICY_HANDLER_H

#include <sstream>
#include <map>
#include <mutex>
#include <vector>

#include "audio_info.h"
#include "i_policy_provider_ipc.h"

namespace OHOS {
namespace AudioStandard {
class PolicyHandler {
public:
    static PolicyHandler& GetInstance();

    ~PolicyHandler();

    void Dump(std::string &dumpString);

    // would be called only once
    bool ConfigPolicyProvider(const sptr<IPolicyProviderIpc> policyProvider);

    bool GetProcessDeviceInfo(const AudioProcessConfig &config, DeviceInfo &deviceInfo);

    bool InitVolumeMap();

    bool GetSharedVolume(AudioVolumeType streamType, DeviceType deviceType, Volume &vol);

    void SetActiveOutputDevice(DeviceType deviceType);

    uint32_t GenerateSessionId(int32_t uid);

    DeviceType GetActiveOutPutDevice();

    int32_t SetWakeUpAudioCapturerFromAudioServer(const AudioProcessConfig &config);

    int32_t NotifyCapturerAdded(AudioCapturerInfo capturerInfo, AudioStreamInfo streamInfo, uint32_t sessionId);

    int32_t NotifyWakeUpCapturerRemoved();

    bool IsAbsVolumeSupported();

    int32_t OffloadGetRenderPosition(uint32_t &delayValue, uint64_t &sendDataSize, uint32_t &timeStamp);

    bool GetHighResolutionExist();

    void SetHighResolutionExist(bool isHighResExist);

    int32_t GetAndSaveClientType(uint32_t uid, const std::string &bundleName);

private:
    PolicyHandler();
    sptr<IPolicyProviderIpc> iPolicyProvider_ = nullptr;

private:
    std::shared_ptr<AudioSharedMemory> policyVolumeMap_ = nullptr;
    volatile Volume *volumeVector_ = nullptr;
    DeviceType deviceType_ = DEVICE_TYPE_SPEAKER;
    bool isHighResolutionExist_ = false;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // POLICY_HANDLER_H
