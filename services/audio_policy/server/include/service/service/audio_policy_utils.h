/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#ifndef ST_AUDIO_POLICY_UTILS_H
#define ST_AUDIO_POLICY_UTILS_H

#include <bitset>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include "singleton.h"
#include "audio_group_handle.h"
#include "audio_info.h"
#include "audio_manager_base.h"
#include "audio_module_info.h"
#include "audio_ec_info.h"
#include "datashare_helper.h"
#include "audio_utils.h"
#include "audio_errors.h"
#include "audio_state_manager.h"
#include "audio_device_manager.h"
#include "audio_stream_collector.h"

#include "audio_a2dp_offload_flag.h"

namespace OHOS {
namespace AudioStandard {

const int64_t SET_BT_ABS_SCENE_DELAY_MS = 120000; // 120ms

class AudioPolicyUtils {
public:
    static AudioPolicyUtils& GetInstance()
    {
        static AudioPolicyUtils instance;
        return instance;
    }
    void WriteServiceStartupError(std::string reason);
    std::string GetRemoteModuleName(std::string networkId, DeviceRole role);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetAvailableDevicesInner(AudioDeviceUsage usage);
    void SetBtConnecting(bool flag);
    int32_t SetPreferredDevice(const PreferredType preferredType, const std::shared_ptr<AudioDeviceDescriptor> &desc);
    void ClearScoDeviceSuspendState(std::string macAddress = "");
    int64_t GetCurrentTimeMS();
    std::string GetSinkPortName(DeviceType deviceType, AudioPipeType pipeType = PIPE_TYPE_UNKNOWN);
    string ConvertToHDIAudioFormat(AudioSampleFormat sampleFormat);
    std::string GetSinkName(const AudioDeviceDescriptor& desc, int32_t sessionId);
    uint32_t PcmFormatToBytes(AudioSampleFormat format);
private:
    AudioPolicyUtils() : streamCollector_(AudioStreamCollector::GetAudioStreamCollector()),
        audioStateManager_(AudioStateManager::GetAudioStateManager()),
        audioDeviceManager_(AudioDeviceManager::GetAudioDeviceManager()),
        audioA2dpOffloadFlag_(AudioA2dpOffloadFlag::GetInstance()) {}
    ~AudioPolicyUtils() {}
    int32_t ErasePreferredDeviceByType(const PreferredType preferredType);
private:
    bool isBTReconnecting_ = false;
    static std::map<std::string, AudioSampleFormat> formatStrToEnum;

    AudioStreamCollector& streamCollector_;
    AudioStateManager &audioStateManager_;
    AudioDeviceManager &audioDeviceManager_;
    AudioA2dpOffloadFlag& audioA2dpOffloadFlag_;
};

}
}

#endif