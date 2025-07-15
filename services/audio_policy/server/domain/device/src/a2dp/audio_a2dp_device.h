/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#ifndef ST_AUDIO_A2DP_SERVICE_H
#define ST_AUDIO_A2DP_SERVICE_H

#include <bitset>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include "audio_module_info.h"
#include "audio_device_info.h"

namespace OHOS {
namespace AudioStandard {
class AudioA2dpDevice {
public:
    static AudioA2dpDevice& GetInstance()
    {
        static AudioA2dpDevice instance;
        return instance;
    }
    bool GetA2dpDeviceInfo(const std::string& device, A2dpDeviceConfigInfo& info);
    bool GetA2dpInDeviceInfo(const std::string& device, A2dpDeviceConfigInfo& info);
    bool GetA2dpDeviceVolumeLevel(const std::string& device, int32_t& volumeLevel);
    bool CheckA2dpDeviceExist(const std::string& device);
    bool SetA2dpDeviceMute(const std::string& device, bool mute);
    void SetA2dpDeviceStreamInfo(const std::string& device, const DeviceStreamInfo& streamInfo);
    void AddA2dpDevice(const std::string& device, const A2dpDeviceConfigInfo& config);
    bool SetA2dpDeviceAbsVolumeSupport(const std::string& device, const bool support, int32_t volume, bool mute);
    bool SetA2dpDeviceVolumeLevel(const std::string& device, const int32_t volumeLevel);
    size_t DelA2dpDevice(const std::string& device);

    void AddA2dpInDevice(const std::string& device, const A2dpDeviceConfigInfo& config);
    size_t DelA2dpInDevice(const std::string& device);
    bool GetA2dpDeviceMute(const std::string& device, bool& isMute);
    void AddHearingAidDevice(const std::string& device, const A2dpDeviceConfigInfo& config);
    size_t DelHearingAidDevice(const std::string& device);
    bool CheckHearingAidDeviceExist(const std::string& device);
private:
    AudioA2dpDevice() {}
    ~AudioA2dpDevice() {}
private:
    mutable std::mutex a2dpDeviceMapMutex_;
    std::unordered_map<std::string, A2dpDeviceConfigInfo> connectedA2dpDeviceMap_;
    mutable std::mutex a2dpInDeviceMapMutex_;
    std::unordered_map<std::string, A2dpDeviceConfigInfo> connectedA2dpInDeviceMap_;
    mutable std::mutex hearingAidDeviceMapMutex_;
    std::unordered_map<std::string, A2dpDeviceConfigInfo> connectedHearingAidDeviceMap_;
};
}
}

#endif