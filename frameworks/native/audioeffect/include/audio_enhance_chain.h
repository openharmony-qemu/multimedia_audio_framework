/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef AUDIO_ENHANCE_CHAIN_H
#define AUDIO_ENHANCE_CHAIN_H

#include <vector>
#include <mutex>
#include <map>
#include <memory>

#include "audio_effect.h"
#include "audio_effect_common.h"
#include "thread_handler.h"

namespace OHOS {
namespace AudioStandard {
struct EnhanceModulePara {
    std::string enhanceName;
    std::string enhanceProp;
    std::string libName;
    AudioEffectLibrary *libHandle { nullptr };
};

struct EnhanceModule {
    std::string enhanceName;
    AudioEffectHandle enhanceHandle { nullptr };
    AudioEffectLibrary *libHandle { nullptr };
};

struct EnhanceBuffer {
    std::vector<uint8_t> micBuffer;
    std::vector<uint8_t> ecBuffer;
    std::vector<uint8_t> micRefBuffer;
};

struct AlgoAttr {
    uint32_t bitDepth;
    uint32_t batchLen;
    uint32_t byteLenPerFrame;
};

struct AlgoCache {
    std::vector<uint8_t> input;
    std::vector<uint8_t> output;
};

struct AudioEnhanceParamAdapter {
    uint32_t muteInfo { 0 };
    uint32_t volumeInfo { 0 };
    uint32_t foldState { FOLD_STATE_MIDDLE };
    std::string preDevice;
    std::string postDevice;
    std::string sceneType;
    std::string preDeviceName;
};

class AudioEnhanceChain : public std::enable_shared_from_this<AudioEnhanceChain> {
public:
    AudioEnhanceChain(uint64_t chainId, const std::string &scene, ScenePriority scenePriority,
        const AudioEnhanceParamAdapter &algoParam, const AudioEnhanceDeviceAttr &deviceAttr);
    ~AudioEnhanceChain();

    bool IsEmptyEnhanceHandles();
    void GetAlgoConfig(AudioBufferConfig &micConfig, AudioBufferConfig &ecConfig, AudioBufferConfig &micRefConfig);
    uint64_t GetChainId(void) const;
    ScenePriority GetScenePriority(void) const;
    void ReleaseAllEnhanceModule(void);
    int32_t CreateAllEnhanceModule(const std::vector<EnhanceModulePara> &moduleParas);
    int32_t SetEnhanceProperty(const std::string &effect, const std::string &property);
    int32_t SetEnhanceParam(bool mute, uint32_t systemVol);
    int32_t SetInputDevice(const std::string &inputDevice, const std::string &deviceName);
    int32_t SetFoldState(uint32_t foldState);
    int32_t SetThreadHandler(const std::shared_ptr<ThreadHandler> &threadHandler);
    int32_t GetOutputDataFromChain(void *buf, size_t bufSize);
    int32_t ApplyEnhanceChain(const EnhanceTransBuffer &transBuf);
    int32_t InitCommand();

private:
    void InitAudioEnhanceChain();
    int32_t DeinterleaverData(uint8_t *src, uint32_t channel, uint8_t *dst, uint32_t offset);
    int32_t SetPropertyToHandle(AudioEffectHandle handle, const std::string &property);
    int32_t SetEnhanceParamToHandle(AudioEffectHandle handle);
    int32_t PrepareChainInputData(void);
    int32_t ProcessInitCommand(void);
    int32_t ProcessSetFoldState(uint32_t foldState);
    int32_t ProcessSetEnhanceParam(bool mute, uint32_t systemVol);
    int32_t ProcessCreateAllEnhanceModule(const std::vector<EnhanceModulePara> &moduleParas);
    int32_t ProcessSetInputDevice(const std::string &inputDevice, const std::string &deviceName);
    int32_t ProcessSetEnhanceProperty(const std::string &enhance, const std::string &property);
    int32_t ProcessApplyEnhanceChain(void);
    int32_t ProcessReleaseAllEnhanceModule(void);
    int32_t WriteChainOutputData(void *buf, size_t bufSize);
    int32_t CacheChainInputData(const EnhanceTransBuffer &transBuf);
    int32_t InitSingleEnhanceModule(AudioEffectHandle enhanceHandle, const std::string &enhanceProp);
    void ScheduleAudioTask(const ThreadHandler::Task &task);

    AlgoAttr algoAttr_;
    AlgoConfig algoSupportedConfig_;
    AlgoCache algoCache_;
    EnhanceBuffer enhanceBuf_;
    uint64_t chainId_ { 0 };
    std::string sceneType_;
    ScenePriority scenePriority_ { DEFAULT_SCENE };
    AudioEnhanceParamAdapter algoParam_;
    AudioEnhanceDeviceAttr deviceAttr_;
    bool needEcFlag_ { false };
    bool needMicRefFlag_ { false };
    bool hasTask_ { false };
    bool chainIsReady_ { false };
    std::vector<EnhanceModule> enhanceModules_;
    std::vector<uint8_t> outputCache_;
    std::shared_ptr<ThreadHandler> threadHandler_ { nullptr };
    std::mutex chainMutex_;
    FILE *dumpFileIn_ { nullptr };
    FILE *dumpFileOut_ { nullptr };
    std::string traceTagIn_;
    std::string traceTagOut_;
    AudioStreamInfo dfxStreamInfo_ {};
    mutable int64_t volumeDataCountIn_ { 0 };
    mutable int64_t volumeDataCountOut_ { 0 };
};

} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_ENHANCE_CHAIN_H