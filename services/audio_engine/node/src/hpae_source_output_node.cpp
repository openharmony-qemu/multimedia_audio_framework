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
#define LOG_TAG "HpaeSourceOutputNode"
#endif

#include "hpae_source_output_node.h"
#include "hpae_format_convert.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "hpae_node_common.h"
#include "audio_engine_log.h"
#include "parameters.h"
#include "securec.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
static constexpr uint64_t AUDIO_NS_PER_S = 1000000000;
static constexpr size_t EXPECTED_RET_SAMPLES = 3840;
int32_t PC_ENABLE_STATE = system::GetBoolParameter("const.multimedia.audio.fwk_ec.enable", 0);

HpaeSourceOutputNode::HpaeSourceOutputNode(HpaeNodeInfo &nodeInfo)
    : HpaeNode(nodeInfo),
      sourceOutputData_(nodeInfo.frameLen * nodeInfo.channels * GetSizeFromFormat(nodeInfo.format)),
      interleveData_(nodeInfo.frameLen * nodeInfo.channels),
      framesRead_(0), totalFrames_(0), isMute_(false), highpassFilter_(nullptr), filterChannelCount_(0),
      isSupportHighpassFilter_(false),
      pcmBufferInfo_(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate, nodeInfo.channelLayout),
      mixedOutput_(pcmBufferInfo_), tmpOutput_(pcmBufferInfo_), silenceData_(pcmBufferInfo_)
{
    AUDIO_INFO_LOG("sourceType: %{public}d, PC_ENABLE_STATE: %{public}d", GetSourceType(), PC_ENABLE_STATE);
    if (GetSourceType() == SOURCE_TYPE_ULTRASONIC && PC_ENABLE_STATE != 0) {
        isSupportHighpassFilter_ = true;
        filterChannelCount_ = static_cast<int32_t>(GetChannelCount());
        CHECK_AND_RETURN_LOG(filterChannelCount_ > 0, "invalid channelCount: %{public}d", filterChannelCount_);
        highpassFilter_ = std::make_unique<AudioHighPassFilter>();
        if (highpassFilter_) {
            int32_t ret = highpassFilter_->InitFilter(filterChannelCount_);
            if (ret != SUCCESS) {
                AUDIO_ERR_LOG("filter init failed: %{public}d, disable highpass filter", ret);
                highpassFilter_.reset();
                isSupportHighpassFilter_ = false;
                filterChannelCount_ = 0;
            } else {
                AUDIO_INFO_LOG("Init HighpassFilter succ");
            }
        } else {
            AUDIO_ERR_LOG("create highpass filter failed");
            isSupportHighpassFilter_ = false;
            filterChannelCount_ = 0;
        }
    }
#ifdef ENABLE_HIDUMP_DFX
    SetNodeName("hpaeSourceOutputNode");
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeAdmin(true, GetNodeInfo());
    }
#endif
    RebuildTraceNames();
}

void HpaeSourceOutputNode::OnTraceInfoChanged()
{
    RebuildTraceNames();
}

void HpaeSourceOutputNode::RebuildTraceNames()
{
    auto traceBase = "[" + std::to_string(GetSessionId()) + "]HpaeSourceOutputNode::DoProcess "
        + GetTraceInfo();
    muteTraceName_ = traceBase + "_[Mute]";
    unmuteTraceName_ = traceBase + "_[unMute]";
}

HpaeSourceOutputNode::~HpaeSourceOutputNode()
{
#ifdef ENABLE_HIDUMP_DFX
    AUDIO_INFO_LOG("NodeId: %{public}u NodeName: %{public}s destructed.",
        GetNodeId(), GetNodeName().c_str());
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeAdmin(false, GetNodeInfo());
    }
#endif
}

void HpaeSourceOutputNode::ApplyHighpassFilter(HpaePcmBuffer *inputPcmBuffer, uint32_t safeFrameCount)
{
    CHECK_AND_RETURN_LOG(inputPcmBuffer && inputPcmBuffer->GetPcmDataBuffer(), "invalid inputBuffer");
    float *pcmData = inputPcmBuffer->GetPcmDataBuffer();
    CHECK_AND_RETURN_LOG(pcmData != nullptr, "pcmData buffer is nullptr");

    // safeFrameCount is already clamped to the upstream buffer capacity by the caller
    int32_t frameCount = static_cast<int32_t>(safeFrameCount);
    CHECK_AND_RETURN_LOG(frameCount > 0, "invalid frameCount: %{public}d", frameCount);
    int32_t nbSamples = filterChannelCount_ * frameCount;
    std::vector<float> inputSamples(nbSamples);
    size_t samplesPerVec = static_cast<size_t>(nbSamples) * sizeof(float);
    int32_t ret = memcpy_s(inputSamples.data(), samplesPerVec, pcmData, samplesPerVec);
    CHECK_AND_RETURN_LOG(ret == 0, "copy pcmData to filter input failed: %{public}d", ret);

    std::vector<float> filteredSamples(nbSamples);
    CHECK_AND_RETURN_LOG(highpassFilter_ != nullptr, "highpassFilter_ is nullptr");
    ret = highpassFilter_->ApplyFilter(inputSamples, filteredSamples);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "filter apply failed: %{public}d", ret);
    CHECK_AND_RETURN_LOG(filteredSamples.size() == EXPECTED_RET_SAMPLES,
        "filteredSamples size:%{public}zu, expect 3840", filteredSamples.size());
    filteredSamples = std::vector<float>(filteredSamples.end() - static_cast<size_t>(nbSamples),
        filteredSamples.end());
    ret = memcpy_s(inputPcmBuffer->GetPcmDataBuffer(), samplesPerVec, filteredSamples.data(), samplesPerVec);
    CHECK_AND_RETURN_LOG(ret == 0, "copy filtered samples failed: %{public}d", ret);
}

static void CopyRightToLeft(uint8_t *data, size_t size, const AudioSampleFormat &format)
{
    CHECK_AND_RETURN_LOG(data != nullptr && size > 0, "error param");
    const size_t bytesPerSample = static_cast<size_t>(GetSizeFromFormat(format));
    const size_t frameSize = bytesPerSample * 2;
    CHECK_AND_RETURN_LOG(size > frameSize, "invalid data size");
    uint8_t *left = nullptr;
    uint8_t *right = nullptr;
    for (size_t i = 0; i < size - frameSize + 1; i += frameSize) {
        left = data + i;
        right = left + bytesPerSample;
        CHECK_AND_RETURN_LOG(memcpy_s(left, bytesPerSample, right, bytesPerSample) == 0, "memcpy_s failed");
    }
}

HpaePcmBuffer *HpaeSourceOutputNode::GetAndValidateOutput()
{
    std::vector<HpaePcmBuffer *> &outputVec = inputStream_.ReadPreOutputData();
    if (outputVec.empty()) {
        AUDIO_WARNING_LOG("sessionId %{public}u DoProcess(), data read is empty", GetSessionId());
        return nullptr;
    }
    HpaePcmBuffer *outputData = SelectOutputData(outputVec);
    if (outputData == nullptr) {
        return nullptr;
    }
    if (!outputData->IsValid() && GetNodeInfo().sourceType != SOURCE_TYPE_PLAYBACK_CAPTURE) {
        return nullptr;
    }
    return outputData;
}

HpaePcmBuffer *HpaeSourceOutputNode::SelectOutputData(const std::vector<HpaePcmBuffer *> &outputVec)
{
    if (!isInjector_) {
        return outputVec.front();
    }
    // mix mode: sum all upstream buffers (mic + injector) with limiter
    if (!isMicMute_) {
        return MixInputs(outputVec);
    }
    // solo mode: only the INJECTOR buffer (ignore mic), fall back to silence if absent
    for (auto *buffer : outputVec) {
        if (buffer != nullptr && buffer->GetSourceBufferType() == HPAE_SOURCE_BUFFER_TYPE_INJECTOR) {
            return buffer;
        }
    }
    return &silenceData_;
}

void HpaeSourceOutputNode::SetInjectorState(bool isInjector)
{
    isInjector_ = isInjector;
}

void HpaeSourceOutputNode::SetMicMute(bool isMute)
{
    isMicMute_ = isMute;
}

int32_t HpaeSourceOutputNode::SetupAudioLimiter()
{
    if (limiter_ != nullptr) {
        AUDIO_INFO_LOG("sessionId %{public}u limiter already setup", GetSessionId());
        return ERROR;
    }
    limiter_ = std::make_unique<AudioLimiter>(GetNodeId());
    int32_t ret = limiter_->SetConfig(GetFrameLen() * GetChannelCount() * sizeof(float), sizeof(float),
        GetSampleRate(), GetChannelCount());
    if (ret == SUCCESS) {
        AUDIO_INFO_LOG("sessionId %{public}u limiter init success", GetSessionId());
    } else {
        limiter_ = nullptr;
        AUDIO_ERR_LOG("sessionId %{public}u limiter init fail ret=%{public}d", GetSessionId(), ret);
    }
    return ret;
}

void HpaeSourceOutputNode::ResetAudioLimiter()
{
    limiter_.reset();
}

HpaePcmBuffer *HpaeSourceOutputNode::MixInputs(const std::vector<HpaePcmBuffer *> &inputs)
{
    mixedOutput_.Reset();
    uint32_t bufferState = PCM_BUFFER_STATE_INVALID | PCM_BUFFER_STATE_SILENCE;
    if (limiter_ == nullptr) {
        for (auto *input : inputs) {
            mixedOutput_ += *input;
            bufferState &= input->GetBufferState();
        }
    } else {
        tmpOutput_.Reset();
        for (auto *input : inputs) {
            tmpOutput_ += *input;
            bufferState &= input->GetBufferState();
        }
        limiter_->Process(GetFrameLen() * GetChannelCount(),
            tmpOutput_.GetPcmDataBuffer(), mixedOutput_.GetPcmDataBuffer());
    }
    mixedOutput_.SetBufferState(bufferState);
    return &mixedOutput_;
}

void HpaeSourceOutputNode::ProcessAudioData(HpaePcmBuffer *outputData, bool isMute)
{
    int32_t ret = ERROR;
    if (isMute) {
        ret = memset_s(sourceOutputData_.data(), sourceOutputData_.size(), 0, sourceOutputData_.size());
        CHECK_AND_RETURN_LOG(ret == EOK, "memset_s failed with error:%{public}d", ret);
    } else {
        // The upstream buffer may have been built at a different frameLen than this node
        // (e.g. a cached format converter frozen at another session's span); clamp reads
        // to its real capacity.
        size_t requestSamples = static_cast<size_t>(GetChannelCount()) * GetFrameLen();
        size_t upstreamSamples = static_cast<size_t>(outputData->GetChannelCount())
            * outputData->GetFrameLen();
        size_t safeSamples = std::min(requestSamples, upstreamSamples);
        if (GetChannelCount() > 0) {
            safeSamples -= safeSamples % static_cast<size_t>(GetChannelCount());
        }
        if (safeSamples < requestSamples) {
            AUDIO_ERR_LOG("sessionId %{public}u clamp convert: upstream %{public}u*%{public}u,"
                " request %{public}u*%{public}u", GetSessionId(), outputData->GetChannelCount(),
                outputData->GetFrameLen(), static_cast<uint32_t>(GetChannelCount()),
                static_cast<uint32_t>(GetFrameLen()));
            // zero the buffer first so the unconverted tail is silence instead of stale data
            ret = memset_s(sourceOutputData_.data(), sourceOutputData_.size(), 0, sourceOutputData_.size());
            CHECK_AND_RETURN_LOG(ret == EOK, "memset_s failed with error:%{public}d", ret);
        }
        if (isSupportHighpassFilter_ && safeSamples > 0) {
            ApplyHighpassFilter(outputData, static_cast<uint32_t>(safeSamples / GetChannelCount()));
        }
        ConvertFromFloat(GetBitWidth(), static_cast<unsigned int>(safeSamples),
            outputData->GetPcmDataBuffer(), sourceOutputData_.data());
    }
    auto nodeCallback = GetNodeStatusCallback().lock();
    if (nodeCallback) {
        nodeCallback->OnRequestLatency(GetSessionId(), streamInfo_.latency);
    }
    if (GetNodeInfo().sourceType == SOURCE_TYPE_VOICE_DOWNLINK) {
        AUDIO_WARNING_LOG("Use downlink call");
        CopyRightToLeft((uint8_t *)sourceOutputData_.data(), sourceOutputData_.size(), GetNodeInfo().format);
    }
}

void HpaeSourceOutputNode::WriteDataAndUpdateFrames()
{
    int32_t ret = ERROR;
    streamInfo_ = {
        .framesRead = framesRead_.load(),
        .timestamp = GetTimestamp(),
        .outputData = (int8_t *)sourceOutputData_.data(),
        .requestDataLen = sourceOutputData_.size(),
        .routeFlag = GetNodeInfo().routeFlag,
        .deviceType = GetNodeInfo().deviceType,
        .adapterType = GetNodeInfo().captureAdapterType,
    };
    if (auto readCallback = readCallback_.lock()) {
        ret = readCallback->OnStreamData(streamInfo_);
    } else {
        AUDIO_ERR_LOG("sessionId %{public}u, readCallback_ is nullptr", GetSessionId());
        return;
    }
    if (ret == ERR_WRITE_FAILED) {
        AUDIO_DEBUG_LOG("sessionId %{public}u, readCallback_ write read data overflow", GetSessionId());
        return;
    }
    CHECK_AND_RETURN_LOG(ret == 0, "sessionId %{public}u, readCallback_ write read data error", GetSessionId());
    totalFrames_ += GetFrameLen();
    framesRead_.store(totalFrames_);
    return;
}

void HpaeSourceOutputNode::DoProcess()
{
    bool isMute = isMute_ || (!isInjector_ && isMicMute_);
    Trace trace(isMute ? muteTraceName_ : unmuteTraceName_);
    HpaePcmBuffer *outputData = GetAndValidateOutput();
    if (outputData == nullptr) {
        return;
    }
    ProcessAudioData(outputData, isMute);
    WriteDataAndUpdateFrames();
}

uint64_t HpaeSourceOutputNode::GetTimestamp()
{
    timespec tm{};
    clock_gettime(CLOCK_MONOTONIC, &tm);
    return static_cast<uint64_t>(tm.tv_sec) * AUDIO_NS_PER_S + static_cast<uint64_t>(tm.tv_nsec);
}

bool HpaeSourceOutputNode::Reset()
{
    const auto preOutputMap = inputStream_.GetPreOutputMap();
    for (const auto &preOutput : preOutputMap) {
        OutputPort<HpaePcmBuffer *> *output = preOutput.first;
        inputStream_.DisConnect(output);
    }
    return true;
}

bool HpaeSourceOutputNode::ResetAll()
{
    const auto preOutputMap = inputStream_.GetPreOutputMap();
    for (const auto &preOutput : preOutputMap) {
        OutputPort<HpaePcmBuffer *> *output = preOutput.first;
        std::shared_ptr<HpaeNode> hpaeNode = preOutput.second;
        if (hpaeNode->ResetAll()) {
            inputStream_.DisConnect(output);
        }
    }
    return true;
}

bool HpaeSourceOutputNode::RegisterReadCallback(const std::weak_ptr<ICapturerStreamCallback> &callback)
{
    if (callback.lock() == nullptr) {
        return false;
    }
    readCallback_ = callback;
    return true;
}

void HpaeSourceOutputNode::Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    inputStream_.Connect(preNode->GetSharedInstance(), preNode->GetOutputPort());
#ifdef ENABLE_HIDUMP_DFX
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeInfo(true, preNode->GetSharedInstance()->GetNodeId(), GetNodeId());
    }
#endif
}

void HpaeSourceOutputNode::ConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode,
    HpaeNodeInfo &nodeInfo)
{
    std::shared_ptr<HpaeNode> realPreNode = preNode->GetSharedInstance(nodeInfo);
    inputStream_.Connect(realPreNode, preNode->GetOutputPort(nodeInfo));
#ifdef ENABLE_HIDUMP_DFX
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeInfo(true, realPreNode->GetNodeId(), GetNodeId());
    }
#endif
}

void HpaeSourceOutputNode::DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    CHECK_AND_RETURN_LOG(preNode != nullptr, "preNode is nullptr");
    inputStream_.DisConnect(preNode->GetOutputPort());
#ifdef ENABLE_HIDUMP_DFX
    CHECK_AND_RETURN_LOG(preNode->GetOutputPort() != nullptr, "port is nullptr");
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeInfo(false, preNode->GetOutputPort()->GetNodeId(), GetNodeId());
    }
#endif
}

void HpaeSourceOutputNode::DisConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode,
    HpaeNodeInfo &nodeInfo)
{
    CHECK_AND_RETURN_LOG(!inputStream_.CheckIfDisConnected(preNode->GetOutputPort(nodeInfo)),
        "%{public}u has disconnected with preNode", GetSessionId());
    const auto port = preNode->GetOutputPort(nodeInfo, true);
    inputStream_.DisConnect(port);
#ifdef ENABLE_HIDUMP_DFX
    CHECK_AND_RETURN_LOG(port != nullptr, "port is nullptr");
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeInfo(false, port->GetNodeId(), GetNodeId());
    }
#endif
}

int32_t HpaeSourceOutputNode::SetState(HpaeSessionState captureState)
{
    HILOG_COMM_INFO("[SetState]Capturer[%{public}s]->Session[%{public}u - %{public}d] "
        "state change:[%{public}s]-->[%{public}s]", GetDeviceClass().c_str(), GetSessionId(), GetStreamType(),
        ConvertSessionState2Str(state_).c_str(), ConvertSessionState2Str(captureState).c_str());
    state_ = captureState;
    return SUCCESS;
}

HpaeSessionState HpaeSourceOutputNode::GetState()
{
    return state_;
}

void HpaeSourceOutputNode::SetAppUid(int32_t appUid)
{
    appUid_ = appUid;
}

int32_t HpaeSourceOutputNode::GetAppUid()
{
    return appUid_;
}

void HpaeSourceOutputNode::SetTokenId(uint32_t tokenId)
{
    tokenId_ = tokenId;
}

uint32_t HpaeSourceOutputNode::GetTokenId()
{
    return tokenId_;
}

void HpaeSourceOutputNode::SetMute(bool isMute)
{
    if (isMute_ != isMute) {
        isMute_ = isMute;
        AUDIO_INFO_LOG("SetMute:%{public}d", isMute);
    }
}

bool HpaeSourceOutputNode::GetMute()
{
    return isMute_;
}

int32_t HpaeSourceOutputNode::GetPcEnableState()
{
    return PC_ENABLE_STATE;
}

void HpaeSourceOutputNode::SetPcEnableState(int32_t val)
{
    PC_ENABLE_STATE = val;
}

void HpaeSourceOutputNode::SetNodeInfo(HpaeNodeInfo &nodeInfo)
{
    // Channels-only change (same frameLen) also changes the sample count per frame;
    // resize on either difference or later ConvertFromFloat overruns the stale buffer.
    if (nodeInfo.frameLen != GetNodeInfo().frameLen || nodeInfo.channels != GetNodeInfo().channels) {
        sourceOutputData_.resize(nodeInfo.frameLen * nodeInfo.channels * GetSizeFromFormat(nodeInfo.format));
        interleveData_.resize(nodeInfo.frameLen * nodeInfo.channels);
        // injector-path buffers were built from ctor geometry; rebuild them with the new
        // geometry, otherwise MixInputs()/SelectOutputData() keep serving ctor-sized buffers
        pcmBufferInfo_ = PcmBufferInfo(nodeInfo.channels, nodeInfo.frameLen,
            nodeInfo.samplingRate, nodeInfo.channelLayout);
        mixedOutput_.ReConfig(pcmBufferInfo_);
        tmpOutput_.ReConfig(pcmBufferInfo_);
        silenceData_.ReConfig(pcmBufferInfo_);
    }
    HpaeNode::SetNodeInfo(nodeInfo);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
