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
#include "audio_suite_log.h"
#include "audio_suite_info.h"
#include "audio_suite_manager.h"
#include "audio_suite_pcm_buffer.h"
#include "audio_proresampler.h"
#include "hpae_format_convert.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
static constexpr uint32_t MIN_START_NODE_ID = 100;

struct PreResample {
    uint32_t preInResample;
    uint32_t preOutResample;
    uint32_t preChannels;
};

class AudioNode;

template <typename T>
class OutputPort;

class AudioNode : public std::enable_shared_from_this<AudioNode> {
public:
    AudioNode(AudioNodeType nodeType)
    {
        audioNodeInfo_.nodeId = GenerateAudioNodeId();
        audioNodeInfo_.nodeType = nodeType;
    }

    AudioNode(AudioNodeType nodeType, AudioFormat audioFormat)
    {
        audioNodeInfo_.nodeId = GenerateAudioNodeId();
        audioNodeInfo_.nodeType = nodeType;
        audioNodeInfo_.audioFormat = audioFormat;
    }

    virtual ~AudioNode() {};

    virtual int32_t Init()
    {
        return SUCCESS;
    }

    virtual int32_t DeInit()
    {
        return SUCCESS;
    }

    virtual int32_t DoProcess() = 0;
    // for Flush node
    virtual int32_t Flush() = 0;
    virtual int32_t InstallTap(AudioNodePortType portType, std::shared_ptr<SuiteNodeReadTapDataCallback> callback) = 0;
    virtual int32_t RemoveTap(AudioNodePortType portType) = 0;
    virtual int32_t Connect(const std::shared_ptr<AudioNode> &preNode,
    AudioNodePortType type) = 0;
    virtual int32_t Connect(const std::shared_ptr<AudioNode> &preNode) = 0;
    virtual int32_t DisConnect(const std::shared_ptr<AudioNode> &preNode) = 0;

    virtual std::shared_ptr<AudioNode> GetSharedInstance()
    {
        return shared_from_this();
    }

    virtual std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> GetOutputPort(AudioNodePortType type)
    {
        return nullptr;
    }

    virtual int32_t SetOnWriteDataCallback(std::shared_ptr<SuiteInputNodeWriteDataCallBack> callback)
    {
        AUDIO_ERR_LOG("SetOnWriteDataCallback failed, node type = %{public}d not support.", GetNodeType());
        return ERR_INVALID_OPERATION;
    }

    virtual bool IsSetReadDataCallback()
    {
        return false;
    }

    virtual int32_t SetOptions(std::string name, std::string value)
    {
        return ERROR;
    }

    virtual int32_t GetOptions(std::string name, std::string &value)
    {
        value = "";
        return ERROR;
    }

    virtual AudioNodeInfo& GetAudioNodeInfo()
    {
        return audioNodeInfo_;
    }

    virtual void SetAudioNodeInfo(AudioNodeInfo& audioNodeInfo)
    {
        audioNodeInfo_ = audioNodeInfo;
    }

    virtual void SetAudioNodeId(uint32_t nodeId)
    {
        audioNodeInfo_.nodeId = nodeId;
    }

    virtual void SetAudioNodeFormat(AudioFormat audioFormat)
    {
        audioNodeInfo_.audioFormat = audioFormat;
    }

    virtual void SetAudioNodeVolume(float volume)
    {
        audioNodeInfo_.volume = volume;
    }

    virtual void SetAudioNodeDataFinishedFlag(bool finishedFlag)
    {
        audioNodeInfo_.finishedFlag = finishedFlag;
    }

    virtual bool GetAudioNodeDataFinishedFlag()
    {
        return audioNodeInfo_.finishedFlag;
    }

    virtual AudioFormat GetAudioNodeFormat()
    {
        return audioNodeInfo_.audioFormat;
    }

    virtual uint32_t GetAudioNodeId()
    {
        return audioNodeInfo_.nodeId;
    }

    virtual float GetAudioNodeVolume()
    {
        return audioNodeInfo_.volume;
    }

    virtual AudioNodeType GetNodeType()
    {
        return audioNodeInfo_.nodeType;
    }

    virtual int32_t SetNodeEnableStatus(AudioNodeEnable enable)
    {
        audioNodeInfo_.enableProcess_ = enable;
        return SUCCESS;
    }

    virtual AudioNodeEnable GetNodeEnableStatus()
    {
        return audioNodeInfo_.enableProcess_;
    }

    virtual std::string GetEnvironmentType()
    {
        return "";
    }

    virtual std::string GetSoundFiledType()
    {
        return "";
    }

    virtual std::string GetEqualizerFrequencyBandGains()
    {
        return "";
    }

    virtual std::string GetVoiceBeautifierType()
    {
        return "";
    }
    
    void ConvertToFloat(AudioSampleFormat format, unsigned n, void *src, float *dst)
    {
        HPAE::ConvertToFloat(format, n, src, dst);
    }

    void ConvertFromFloat(AudioSampleFormat format, unsigned n, float *src, void *dst)
    {
        HPAE::ConvertFromFloat(format, n, src, dst);
    }

    int32_t SetUpResample(uint32_t inRate, uint32_t outRate, uint32_t channels, uint32_t quality)
    {
        if (proResampler_ == nullptr) {
            proResampler_ = std::make_unique<HPAE::ProResampler>(inRate, outRate, channels, quality);
                preResample_.preInResample = inRate;
                preResample_.preOutResample = outRate;
                preResample_.preChannels = channels;
                return RESAMPLER_ERR_SUCCESS;
        }
 
        bool noNeedUpdate = (inRate == preResample_.preInResample && outRate == preResample_.preOutResample &&
            channels == preResample_.preChannels);
        
        CHECK_AND_RETURN_RET(!noNeedUpdate, RESAMPLER_ERR_SUCCESS);

        preResample_.preInResample = inRate;
        preResample_.preOutResample = outRate;
        preResample_.preChannels = channels;

        proResampler_->Reset();
        int32_t ret = proResampler_->UpdateRates(inRate, outRate);
        CHECK_AND_RETURN_RET_LOG(ret == RESAMPLER_ERR_SUCCESS, ret,
            "ProResampler update rate failed with error code %{public}d", ret);
 
        ret = proResampler_->UpdateChannels(channels);
        CHECK_AND_RETURN_RET_LOG(ret == RESAMPLER_ERR_SUCCESS, ret,
            "ProResampler update Channels failed with error code %{public}d", ret);
 
        return ret;
    }
 
    int32_t DoResampleProcess(const float *inBuffer, uint32_t inFrameSize,
        float *outBuffer, uint32_t outFrameSize)
    {
        CHECK_AND_RETURN_RET_LOG(proResampler_ != nullptr, ERROR, "ProResampler_ is nullptr");
        return proResampler_->Process(inBuffer, inFrameSize, outBuffer, outFrameSize);
    }

private:
    static uint32_t GenerateAudioNodeId()
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

private:
    std::unique_ptr<HPAE::ProResampler> proResampler_ = nullptr;
    AudioNodeInfo audioNodeInfo_;
    inline static std::mutex nodeIdCounterMutex_;
    inline static uint32_t nodeIdCounter_ = MIN_START_NODE_ID;
    struct PreResample preResample_ = {0};
};

class Tap {
public:
    virtual std::shared_ptr<SuiteNodeReadTapDataCallback> GetOnReadTapDataCallback()
    {
        return callback_;
    }

    virtual AudioNodePortType GetAudioNodePortType()
    {
        return portType_;
    }

    void SetOnReadTapDataCallback(std::shared_ptr<SuiteNodeReadTapDataCallback> callback)
    {
        callback_ = callback;
    }

    void SetAudioNodePortType(AudioNodePortType type)
    {
        portType_ = type;
    }

private:
    std::shared_ptr<SuiteNodeReadTapDataCallback> callback_ = nullptr;
    AudioNodePortType portType_ = AUDIO_NODE_DEFAULT_OUTPORT_TYPE;
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif