/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OFFLINE_AUDIO_EFFECT_SERVER_CHAIN_H
#define OFFLINE_AUDIO_EFFECT_SERVER_CHAIN_H

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <mutex>

#include "v1_0/effect_types.h"
#include "v1_0/ieffect_control.h"
#include "v1_0/ieffect_model.h"

#include "audio_device_info.h"
#include "audio_stream_info.h"
#include "audio_source_type.h"
#include "audio_shared_memory.h"
#include "oh_audio_buffer.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

struct OfflineEffectConfig {
    uint32_t samplingRate;
    uint32_t channels;
    uint32_t format;
};

struct OfflineEffectIOConfig {
    OfflineEffectConfig inputCfg;
    OfflineEffectConfig outputCfg;
};

class OfflineAudioEffectServerChain {
public:
    OfflineAudioEffectServerChain(const string &chainName);
    ~OfflineAudioEffectServerChain();

    static int32_t GetOfflineAudioEffectChains(vector<string> &chainNamesVector);

    int32_t Create();
    int32_t SetConfig(AudioStreamInfo inInfo, AudioStreamInfo outInfo);
    int32_t SetParam(const std::vector<uint8_t> &param);
    int32_t GetEffectBufferSize(uint32_t &inBufferSize, uint32_t &outBufferSize);
    int32_t Prepare(const shared_ptr<AudioSharedMemory> &bufferIn, const shared_ptr<AudioSharedMemory> &bufferOut);
    int32_t Process(uint32_t inBufferSize, uint32_t outBufferSize);
    int32_t Release();

private:
    void InitDump();

    struct IEffectControl *controller_ = nullptr;
    struct ControllerId controllerId_ = {};
    shared_ptr<AudioSharedMemory> serverBufferIn_ = nullptr;
    shared_ptr<AudioSharedMemory> serverBufferOut_ = nullptr;
    uint32_t inBufferSize_ = 0;
    uint32_t outBufferSize_ = 0;
    string chainName_;
    OfflineEffectIOConfig offlineConfig_ = {};
    FILE *dumpFileIn_ = nullptr;
    FILE *dumpFileOut_ = nullptr;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // OFFLINE_AUDIO_EFFECT_SERVER_CHAIN_H
