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

#ifndef LOG_TAG
#define LOG_TAG "AudioSuiteNode"
#endif

#include "audio_suite_node.h"
#include "audio_suite_log.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

AudioNode::AudioNode(AudioNodeType nodeType)
{
    audioNodeInfo_.nodeId = GenerateAudioNodeId();
    audioNodeInfo_.nodeType = nodeType;
}

AudioNode::AudioNode(AudioNodeType nodeType, AudioFormat audioFormat)
{
    audioNodeInfo_.nodeId = GenerateAudioNodeId();
    audioNodeInfo_.nodeType = nodeType;
    audioNodeInfo_.audioFormat = audioFormat;
    audioNodeInfo_.inPcmFormat = PcmBufferFormat(audioFormat.rate, audioFormat.audioChannelInfo.numChannels,
        audioFormat.audioChannelInfo.channelLayout, audioFormat.format);
}

int32_t AudioNode::Init()
{
    return SUCCESS;
}

int32_t AudioNode::DeInit()
{
    return SUCCESS;
}

std::shared_ptr<AudioNode> AudioNode::GetSharedInstance()
{
    return shared_from_this();
}

OutputPort<AudioSuitePcmBuffer*>* AudioNode::GetOutputPort()
{
    return nullptr;
}

int32_t AudioNode::SetRequestDataCallback(std::shared_ptr<InputNodeRequestDataCallBack> callback)
{
    AUDIO_ERR_LOG("SetRequestDataCallback failed, node type = %{public}d not support.", GetNodeType());
    return ERR_INVALID_OPERATION;
}

bool AudioNode::IsSetReadDataCallback()
{
    return false;
}

int32_t AudioNode::SetOptions(std::string name, std::string value)
{
    return ERROR;
}

int32_t AudioNode::GetOptions(std::string name, std::string &value)
{
    value = "";
    return ERROR;
}

AudioNodeInfo& AudioNode::GetAudioNodeInfo()
{
    return audioNodeInfo_;
}

void AudioNode::SetAudioNodeInfo(const AudioNodeInfo& audioNodeInfo)
{
    audioNodeInfo_ = audioNodeInfo;
}

void AudioNode::SetAudioNodeId(uint32_t nodeId)
{
    audioNodeInfo_.nodeId = nodeId;
}

void AudioNode::SetAudioNodeFormat(AudioFormat audioFormat)
{
    audioNodeInfo_.audioFormat = audioFormat;
    audioNodeInfo_.inPcmFormat = PcmBufferFormat(audioFormat.rate, audioFormat.audioChannelInfo.numChannels,
        audioFormat.audioChannelInfo.channelLayout, audioFormat.format);
}

void AudioNode::SetAudioNodeVolume(float volume)
{
    audioNodeInfo_.volume = volume;
}

void AudioNode::SetAudioNodeDataFinishedFlag(bool finishedFlag)
{
    audioNodeInfo_.finishedFlag = finishedFlag;
}

bool AudioNode::GetAudioNodeDataFinishedFlag()
{
    return audioNodeInfo_.finishedFlag;
}

AudioFormat AudioNode::GetAudioNodeFormat()
{
    return audioNodeInfo_.audioFormat;
}

const PcmBufferFormat &AudioNode::GetAudioNodeInPcmFormat()
{
    return audioNodeInfo_.inPcmFormat;
}

uint32_t AudioNode::GetAudioNodeId()
{
    return audioNodeInfo_.nodeId;
}

float AudioNode::GetAudioNodeVolume()
{
    return audioNodeInfo_.volume;
}

AudioNodeType AudioNode::GetNodeType()
{
    return audioNodeInfo_.nodeType;
}

std::string AudioNode::GetNodeTypeString()
{
    auto it = NODETYPE_TOSTRING_MAP.find(audioNodeInfo_.nodeType);
    return it != NODETYPE_TOSTRING_MAP.end() ? it->second : "NODE_TYPE_UNKNOWN";
}

int32_t AudioNode::SetBypassEffectNode(bool bypass)
{
    audioNodeInfo_.bypassStatus = bypass;
    return SUCCESS;
}

bool AudioNode::GetNodeBypassStatus()
{
    return audioNodeInfo_.bypassStatus;
}

std::string AudioNode::GetEnvironmentType()
{
    return "";
}

std::string AudioNode::GetSoundFieldType()
{
    return "";
}

std::string AudioNode::GetEqualizerFrequencyBandGains()
{
    return "";
}

std::string AudioNode::GetVoiceBeautifierType()
{
    return "";
}

void AudioNode::SetAudioNodeWorkMode(PipelineWorkMode workMode)
{
    audioNodeInfo_.workMode = workMode;
}

PipelineWorkMode AudioNode::GetAudioNodeWorkMode()
{
    return audioNodeInfo_.workMode;
}

uint32_t AudioNode::GenerateAudioNodeId()
{
    std::lock_guard<std::mutex> lock(nodeIdCounterMutex_);
    if (nodeIdCounter_ == std::numeric_limits<uint32_t>::max() - MIN_START_NODE_ID) {
        nodeIdCounter_ = MIN_START_NODE_ID;
        AUDIO_WARNING_LOG("AudioNode NodeId approach the boundary value");
    } else {
        ++nodeIdCounter_;
    }
    return nodeIdCounter_;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS