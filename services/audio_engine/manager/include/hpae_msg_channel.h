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
#ifndef HPAE_MSG_CHANNEL_H
#define HPAE_MSG_CHANNEL_H

#include <any>
#include "i_stream.h"
#include "hpae_info.h"
#include "hpae_pcm_buffer.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
enum HpaeMsgCode {
    UPDATE_STATUS,
    INIT_DEVICE_RESULT,
    MOVE_SINK_INPUT,
    MOVE_ALL_SINK_INPUT,
    MOVE_SOURCE_OUTPUT,
    MOVE_ALL_SOURCE_OUTPUT,
    DUMP_SINK_INFO,
    DUMP_SOURCE_INFO,
    MOVE_SESSION_FAILED,
    RELOAD_AUDIO_SINK_RESULT,
    CONNECT_CO_BUFFER_NODE,
    DISCONNECT_CO_BUFFER_NODE,
    INIT_SOURCE_RESULT,
};

enum NodeOperation { UNDERFLOW, FADED, DRAINED };

class ISendMsgCallback {
public:
    virtual void Invoke(HpaeMsgCode cmdID, const std::any &args) = 0;
    virtual void InvokeSync(HpaeMsgCode cmdID, const std::any &args) = 0;
};

class CallbackSender {
protected:
    std::weak_ptr<ISendMsgCallback> weakCallback_;

public:
    void RegisterSendMsgCallback(std::weak_ptr<ISendMsgCallback> cb)
    {
        weakCallback_ = cb;
    }

    template <typename... Args>
    void TriggerCallback(HpaeMsgCode cmdID, Args &&...args)
    {
        if (auto callback = weakCallback_.lock()) {
            // pack the arguments into a tuple
            auto packed = std::make_tuple(std::forward<Args>(args)...);
            callback->Invoke(cmdID, packed);
        }
    }

    template <typename... Args>
    void TriggerSyncCallback(HpaeMsgCode cmdID, Args &&...args)
    {
        if (auto callback = weakCallback_.lock()) {
            // pack the arguments into a tuple
            auto packed = std::make_tuple(std::forward<Args>(args)...);
            callback->InvokeSync(cmdID, packed);
        }
    }
};

enum HpaeProcessorType {
    HPAE_SCENE_UNCONNECTED = -1,
    HPAE_SCENE_DEFAULT = 0,
    HPAE_SCENE_MUSIC = 1,
    HPAE_SCENE_GAME = 2,
    HPAE_SCENE_MOVIE = 3,
    HPAE_SCENE_SPEECH = 4,
    HPAE_SCENE_RING = 5,
    HPAE_SCENE_VOIP_DOWN = 6,
    HPAE_SCENE_OTHERS = 7,
    HPAE_SCENE_EFFECT_NONE = 8,
    HPAE_SCENE_EFFECT_OUT = 9,

    // special scene for split
    HPAE_SCENE_SPLIT_MEDIA = 10,
    HPAE_SCENE_SPLIT_NAVIGATION = 11,
    HPAE_SCENE_SPLIT_COMMUNICATION = 12,

    // up processor scene
    HPAE_SCENE_VOIP_UP = 20,
    HPAE_SCENE_RECORD = 21,
    HPAE_SCENE_PRE_ENHANCE = 22,
    HPAE_SCENE_ASR = 23,
    HPAE_SCENE_VOICE_MESSAGE = 24,

    // scene for collaboration
    HPAE_SCENE_COLLABORATIVE = 25,
    HPAE_SCENE_RECOGNITION = 26,
};

// mark sourceInputNode(cluster)
enum HpaeSourceInputNodeType {
    HPAE_SOURCE_DEFAULT,
    HPAE_SOURCE_MIC,
    HPAE_SOURCE_MIC_EC,
    HPAE_SOURCE_EC,
    HPAE_SOURCE_MICREF,
    HPAE_SOURCE_OFFLOAD,
};

struct HpaeDfxNodeInfo {
    uint32_t nodeId;
    uint32_t sessionId;
    uint32_t frameLen;
    size_t historyFrameCount;
    AudioSamplingRate samplingRate;
    uint32_t customSampleRate = 0;
    AudioSampleFormat format = AudioSampleFormat::SAMPLE_F32LE;
    AudioChannel channels;
    AudioChannelLayout channelLayout = AudioChannelLayout::CH_LAYOUT_UNKNOWN;
    FadeType fadeType = NONE_FADE;
    AudioStreamType streamType = STREAM_DEFAULT;
    HpaeProcessorType sceneType;
    std::string deviceClass;
    std::string deviceNetId;
    std::string deviceName;
    std::string nodeName;
    SourceType sourceType;
};

class INodeCallback {
public:
    virtual void OnNodeStatusUpdate(uint32_t sessionId, IOperation operation){};
    virtual void OnFadeDone(uint32_t sessionId){};
    virtual void OnRequestLatency(uint32_t sessionId, uint64_t &latency){};
    virtual void OnRewindAndFlush(uint64_t rewindTime, uint64_t hdiFramePosition = 0){};
    virtual void OnNotifyQueue(){};
    virtual void OnDisConnectProcessCluster(HpaeProcessorType sceneType){};
    virtual void OnNotifyDfxNodeAdmin(bool isAdd, const HpaeDfxNodeInfo &nodeInfo){};
    virtual void OnNotifyDfxNodeInfo(bool isConnect, uint32_t parentId, uint32_t childId){};
    virtual void OnNotifyDfxNodeInfoChanged(uint32_t NodeId, const HpaeDfxNodeInfo &nodeInfo){};
};

struct HpaeNodeInfo : HpaeDfxNodeInfo {
    HpaeEffectInfo effectInfo;
    std::weak_ptr<INodeCallback> statusCallback;
    HpaeSourceBufferType sourceBufferType = HpaeSourceBufferType::HPAE_SOURCE_BUFFER_TYPE_DEFAULT;
    HpaeSourceInputNodeType sourceInputNodeType = HpaeSourceInputNodeType::HPAE_SOURCE_DEFAULT;

    SplitStreamType GetSplitStreamType() const
    {
        static const auto splitTypeMap = [] {
            std::unordered_map<HpaeProcessorType, SplitStreamType> map;
            map[HPAE_SCENE_SPLIT_NAVIGATION] = STREAM_TYPE_NAVIGATION;
            map[HPAE_SCENE_SPLIT_COMMUNICATION] = STREAM_TYPE_COMMUNICATION;
            map[HPAE_SCENE_SPLIT_MEDIA] = STREAM_TYPE_MEDIA;
            return map;
        } ();
        auto it = splitTypeMap.find(sceneType);
        return (it != splitTypeMap.end()) ? it->second : STREAM_TYPE_DEFAULT;
    }
};

class INodeFormatInfoCallback {
public:
    virtual int32_t GetNodeInputFormatInfo(uint32_t sessionId, AudioBasicFormat &basicFormat) = 0;
};

} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS

#endif