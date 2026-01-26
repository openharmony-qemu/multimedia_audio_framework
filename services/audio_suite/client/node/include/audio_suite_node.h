/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef AUDIO_NODE_H
#define AUDIO_NODE_H
#include <memory>
#include <stdint.h>
#include <unordered_map>
#include <vector>
#include <set>
#include <sstream>
#include "audio_errors.h"
#include "audio_suite_manager.h"
#include "audio_suite_pcm_buffer.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
static constexpr uint32_t MIN_START_NODE_ID = 100;

struct AudioNodeInfo {
    AudioNodeType nodeType;
    uint32_t nodeId;
    float volume = 1.0;
    bool finishedFlag = false;
    bool bypassStatus = false;
    AudioFormat audioFormat;
    PcmBufferFormat inPcmFormat;
    PipelineWorkMode workMode = PIPELINE_EDIT_MODE;
};

class AudioNode;

template <typename T>
class OutputPort;

class AudioNode : public std::enable_shared_from_this<AudioNode> {
public:
    AudioNode(AudioNodeType nodeType);
    AudioNode(AudioNodeType nodeType, AudioFormat audioFormat);
    virtual ~AudioNode() {};

    virtual int32_t Init();
    virtual int32_t DeInit();
    virtual int32_t DoProcess(uint32_t needDataLength) = 0;
    // for Flush node
    virtual int32_t Flush() = 0;
    virtual int32_t Connect(const std::shared_ptr<AudioNode> &preNode) = 0;
    virtual int32_t DisConnect(const std::shared_ptr<AudioNode> &preNode) = 0;
    virtual std::shared_ptr<AudioNode> GetSharedInstance();
    virtual OutputPort<AudioSuitePcmBuffer*>* GetOutputPort();
    virtual int32_t SetRequestDataCallback(std::shared_ptr<InputNodeRequestDataCallBack> callback);
    virtual bool IsSetReadDataCallback();
    virtual int32_t SetOptions(std::string name, std::string value);
    virtual int32_t GetOptions(std::string name, std::string &value);
    virtual AudioNodeInfo& GetAudioNodeInfo();
    virtual void SetAudioNodeInfo(const AudioNodeInfo& audioNodeInfo);
    virtual void SetAudioNodeId(uint32_t nodeId);
    virtual void SetAudioNodeFormat(AudioFormat audioFormat);
    virtual void SetAudioNodeVolume(float volume);
    virtual void SetAudioNodeDataFinishedFlag(bool finishedFlag);
    virtual bool GetAudioNodeDataFinishedFlag();
    virtual AudioFormat GetAudioNodeFormat();
    virtual const PcmBufferFormat &GetAudioNodeInPcmFormat();
    virtual uint32_t GetAudioNodeId();
    virtual float GetAudioNodeVolume();
    virtual AudioNodeType GetNodeType();
    virtual std::string GetNodeTypeString();
    virtual int32_t SetBypassEffectNode(bool bypass);
    virtual bool GetNodeBypassStatus();
    virtual std::string GetEnvironmentType();
    virtual std::string GetSoundFieldType();
    virtual std::string GetEqualizerFrequencyBandGains();
    virtual std::string GetVoiceBeautifierType();
    virtual void SetAudioNodeWorkMode(PipelineWorkMode workMode);
    virtual PipelineWorkMode GetAudioNodeWorkMode();

private:
    static uint32_t GenerateAudioNodeId();

private:
    AudioNodeInfo audioNodeInfo_;
    inline static std::mutex nodeIdCounterMutex_;
    inline static uint32_t nodeIdCounter_ = MIN_START_NODE_ID;
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif