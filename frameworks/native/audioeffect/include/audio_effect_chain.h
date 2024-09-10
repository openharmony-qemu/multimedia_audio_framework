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

#ifndef AUDIO_EFFECT_CHAIN_H
#define AUDIO_EFFECT_CHAIN_H

#include <atomic>

#include "audio_effect.h"
#include "audio_utils.h"

#ifdef SENSOR_ENABLE
#include "audio_head_tracker.h"
#endif
#include "audio_effect_hdi_param.h"
#ifdef WINDOW_MANAGER_ENABLE
#include "audio_effect_rotation.h"
#endif
#include "audio_effect_volume.h"

namespace OHOS {
namespace AudioStandard {
struct AudioEffectProcInfo {
    bool headTrackingEnabled;
    bool btOffloadEnabled;
};

class AudioEffectChain {
public:
#ifdef SENSOR_ENABLE
    AudioEffectChain(std::string scene, std::shared_ptr<HeadTracker> headTracker);
#else
    AudioEffectChain(std::string scene);
#endif
    ~AudioEffectChain();
    void SetEffectMode(const std::string &mode);
    void SetExtraSceneType(const std::string &extraSceneType);
    void SetEffectCurrSceneType(AudioEffectScene currSceneType);
    void AddEffectHandle(AudioEffectHandle effectHandle, AudioEffectLibrary *libHandle, AudioEffectScene currSceneType);
    void ApplyEffectChain(float *bufIn, float *bufOut, uint32_t frameLen, AudioEffectProcInfo procInfo);
    bool IsEmptyEffectHandles();
    void Dump();
    int32_t UpdateMultichannelIoBufferConfig(const uint32_t &channels, const uint64_t &channelLayout);
    void StoreOldEffectChainInfo(std::string &sceneMode, AudioEffectConfig &ioBufferConfig);
    void InitEffectChain();
    void SetHeadTrackingDisabled();
    uint32_t GetLatency();
    int32_t UpdateEffectParam();
    void ResetIoBufferConfig();
    void SetSpatialDeviceType(AudioSpatialDeviceType spatialDeviceType);

private:
    AudioEffectConfig GetIoBufferConfig();
    void ReleaseEffectChain();
    int32_t SetEffectParamToHandle(AudioEffectHandle handle, int32_t &replyData);
    void DumpEffectProcessData(std::string fileName, void *buffer, size_t len);

    std::mutex reloadMutex_;
    std::string sceneType_ = "";
    std::string effectMode_ = "";
    uint32_t latency_ = 0;
    uint32_t extraEffectChainType_ = 0;
    AudioEffectScene currSceneType_ = SCENE_MUSIC;
    std::vector<AudioEffectHandle> standByEffectHandles_;
    std::vector<AudioEffectLibrary *> libHandles_;
    AudioEffectConfig ioBufferConfig_ = {};
    AudioBuffer audioBufIn_ = {};
    AudioBuffer audioBufOut_ = {};
    FILE *dumpFileInput_ = nullptr;
    FILE *dumpFileOutput_ = nullptr;
    std::string dumpNameIn_ = "";
    std::string dumpNameOut_ = "";
    AudioSpatialDeviceType spatialDeviceType_{ EARPHONE_TYPE_OTHERS };

#ifdef SENSOR_ENABLE
    std::shared_ptr<HeadTracker> headTracker_;
#endif
};

} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_EFFECT_CHAIN_H