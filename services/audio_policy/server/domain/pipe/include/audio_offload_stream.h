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
#ifndef ST_AUDIO_OFFLOAD_STREAM_H
#define ST_AUDIO_OFFLOAD_STREAM_H

#include <string>
#include <vector>
#include <mutex>

#include "power_mgr_client.h"

#include "audio_info.h"
#include "audio_policy_manager_factory.h"
#include "audio_stream_collector.h"

namespace OHOS {
namespace AudioStandard {

enum OffloadAction : uint32_t {
    OFFLOAD_NEW = 0,
    OFFLOAD_MOVE_IN,
    OFFLOAD_MOVE_OUT
};

constexpr uint32_t NO_OFFLOAD_STREAM_SESSIONID = 0;

class AudioOffloadStream {
public:
    static AudioOffloadStream &GetInstance()
    {
        static AudioOffloadStream instance;
        return instance;
    }

    uint32_t GetOffloadSessionId(OffloadAdapter offloadAdapter);
    void SetOffloadStatus(OffloadAdapter offloadAdapter, uint32_t sessionId);
    void UnsetOffloadStatus(uint32_t sessionId);
    void HandlePowerStateChanged(PowerMgr::PowerState state);
    void UpdateOffloadStatusFromUpdateTracker(uint32_t sessionId, RendererState state);
    void Dump(std::string &dumpString);

    // not offload related
    std::vector<SinkInput> FilterSinkInputs(int32_t sessionId, std::vector<SinkInput> sinkInputs);

private:
    AudioOffloadStream()
        : audioPolicyManager_(AudioPolicyManagerFactory::GetAudioPolicyManager()),
        streamCollector_(AudioStreamCollector::GetAudioStreamCollector())
    {
        for (uint32_t i = 0; i < OFFLOAD_IN_ADAPTER_SIZE; ++i) {
            offloadSessionIdMap_[static_cast<OffloadAdapter>(i)] = NO_OFFLOAD_STREAM_SESSIONID;
        }
    }
    ~AudioOffloadStream() {}

    void SetOffloadStatusInternal(uint32_t sessionId, OffloadAdapter offloadAdapter);
    void UnsetOffloadStatusInternal(uint32_t sessionId, OffloadAdapter offloadAdapter);

private:
    IAudioPolicyInterface &audioPolicyManager_;
    AudioStreamCollector &streamCollector_;

    std::mutex offloadMutex_;
    std::map<OffloadAdapter, uint32_t> offloadSessionIdMap_ ;
    PowerMgr::PowerState currentPowerState_ = PowerMgr::PowerState::AWAKE;
};

} // namespace AudioStandard
} // namespace OHOS

#endif