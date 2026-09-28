/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioHoaRenderNode"
#endif

#include "audio_suite_hoa_render_node.h"
#include "audio_utils.h"
#include "audio_suite_log.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

namespace {
static constexpr AudioChannelLayout HOA_ALGO_IN_CHANNEL_LAYOUT = CH_LAYOUT_HOA_ORDER1_ACN_N3D;
static constexpr AudioChannelLayout HOA_ALGO_OUT_CHANNEL_LAYOUT = CH_LAYOUT_STEREO;
static constexpr uint32_t ALGO_PROCESS_FRAME_LEN = 256;
static constexpr uint32_t HOA_CHANNELS_MAXNUM = 16;
}  // namespace

AudioSuiteHoaRenderNode::AudioSuiteHoaRenderNode()
    : AudioSuiteProcessNode(NODE_TYPE_HOA_RENDER)
{
}

AudioSuiteHoaRenderNode::~AudioSuiteHoaRenderNode()
{
    if (isInit_) {
        DeInit();
    }
}

int32_t AudioSuiteHoaRenderNode::Init()
{
    CHECK_AND_RETURN_RET_LOG(nodeParameter_.inSampleRate != 0, ERROR, "Invalid input SampleRate");
    CHECK_AND_RETURN_RET_LOG(nodeParameter_.outSampleRate != 0, ERROR, "Invalid output SampleRate");
    if (isInit_) {
        AUDIO_ERR_LOG("AudioSuiteHoaRenderNode::Init failed, already inited");
        return ERROR;
    }

    if (!isOutputStreamInit_) {
        CHECK_AND_RETURN_RET_LOG(InitOutputStream() == SUCCESS, ERROR, "Init OutPutStream error");
        isOutputStreamInit_ = true;
    }

    algoInterface_ = AudioSuiteAlgoInterface::CreateAlgoInterface(AlgoType::AUDIO_NODE_TYPE_HOA_RENDER,
        nodeParameter_);
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, ERROR, "algoInterface_ CreateAlgoInterface failed");

    int32_t ret = algoInterface_->Init();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "algoInterface_ Init failed");

    SetAudioNodeFormat(AudioFormat{{HOA_ALGO_IN_CHANNEL_LAYOUT, nodeParameter_.inChannels},
        static_cast<AudioSampleFormat>(nodeParameter_.inFormat),
        static_cast<AudioSamplingRate>(nodeParameter_.inSampleRate),
        {HOA_ALGO_OUT_CHANNEL_LAYOUT, nodeParameter_.outChannels},
        static_cast<AudioSampleFormat>(nodeParameter_.outFormat),
        static_cast<AudioSamplingRate>(nodeParameter_.outSampleRate)});

    nodeNeedDataDuration_ =
        static_cast<uint64_t>(nodeParameter_.frameLen) * MILLISECONDS_TO_MICROSECONDS / nodeParameter_.inSampleRate;
    isInit_ = true;
    upstreamFinished_ = false;
    AUDIO_INFO_LOG("AudioSuiteHoaRenderNode::Init end");
    return SUCCESS;
}

int32_t AudioSuiteHoaRenderNode::DeInit()
{
    if (algoInterface_ != nullptr) {
        int32_t ret = algoInterface_->Deinit();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "Deinit failed: %{public}d", ret);
        algoInterface_ = nullptr;
    }

    if (isInit_) {
        isInit_ = false;
        upstreamFinished_ = false;
        AUDIO_INFO_LOG("AudioSuiteHoaRenderNode::DeInit end");
        return SUCCESS;
    }

    return ERROR;
}

int32_t AudioSuiteHoaRenderNode::PrintNodeInfo(std::string &nodeInfo)
{
    nodeInfo += "    HOA: ambisonic-to-binaural rendering (no external params)\n";
    return SUCCESS;
}

int32_t AudioSuiteHoaRenderNode::InitHoaCacheLength(uint32_t needDataLength)
{
    uint32_t needFrames = nodeParameter_.frameLen;
    requestPreNodeDuration_ = nodeNeedDataDuration_;
    frameOutBytes_ = CalculationNeedBytes(requestPreNodeDuration_);
    algoOutPcmBuffer_.resize(resultNumber_);
    downStreamData_.resize(resultNumber_);
    // The buffer size is twice the maximum length of the data in the current node and the next node.
    uint32_t doubleLength = 2;

    int32_t ret = downStreamData_[0].ResizePcmBuffer(
        GetAudioNodeOutPcmFormat(), needDataLength);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "Target buffer allocation failed.");
    CHECK_AND_RETURN_RET_LOG(
        downStreamData_[0].GetPcmData() != nullptr, ERROR, "The target buffer pointer is null.");

    // init RingCache
    uint32_t needInByteLength = needFrames * HOA_CHANNELS_MAXNUM * sizeof(int32_t);
    ret = inputBuffer_.ResizeBuffer(needInByteLength * doubleLength);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "inputBuffer_ allocation failed.");
    
    uint32_t needOutByteLength =
        downStreamData_[0].GetDataSize() >= frameOutBytes_ ? downStreamData_[0].GetDataSize() : frameOutBytes_;
    ret = outputBuffer_.ResizeBuffer(needOutByteLength * doubleLength);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "outputBuffer_ allocation failed.");

    // init algoCache
    uint32_t chunkInSize = ALGO_PROCESS_FRAME_LEN * HOA_CHANNELS_MAXNUM * sizeof(int32_t);
    algoInputBuffer_.resize(chunkInSize);
    
    uint32_t chunkOutSize = ALGO_PROCESS_FRAME_LEN * nodeParameter_.outChannels * sizeof(int32_t);
    algoOutputBuffer_.resize(chunkOutSize);

    algoOutput_.clear();
    algoOutput_.emplace_back(algoOutputBuffer_.data());

    return SUCCESS;
}

int32_t AudioSuiteHoaRenderNode::DoProcess(uint32_t needDataLength)
{
    CHECK_AND_RETURN_RET_LOG(needDataLength <= maxRequestLength_, ERROR, "Request data length error.");
    if (GetAudioNodeDataFinishedFlag() && inputBuffer_.GetSize() == 0 && outputBuffer_.GetSize() == 0) {
        AUDIO_DEBUG_LOG("Current node type = %{public}d does not have more data to process.", GetNodeType());
        return ERROR;
    }

    if (nextNeedDataLength_ != needDataLength) {
        int32_t ret = InitHoaCacheLength(needDataLength);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "Failed to initialize buffer.");
        nextNeedDataLength_ = needDataLength;
    }

    requestPreNodeDuration_ = nodeNeedDataDuration_;
    while (downStreamData_[0].GetDataSize() > outputBuffer_.GetSize() && !GetAudioNodeDataFinishedFlag()) {
        ReadProcessNodePreOutputData();
        CHECK_AND_RETURN_RET_LOG(!preNodeResult_.empty(), ERROR,
            "node %{public}d can't get pcmbuffer from prenodes.", GetNodeType());

        int32_t ret = GetProcessedToBuffer();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "GetProcessedToBuffer failed.");

        ret = ProcessAlgoChunks();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "ProcessAlgoChunks failed.");
    }

    downStreamData_[0].Reset();
    uint32_t CopyDataLength = outputBuffer_.GetSize() <= downStreamData_[0].GetDataSize()
                                    ? outputBuffer_.GetSize()
                                    : downStreamData_[0].GetDataSize();
    int32_t ret = outputBuffer_.GetData(downStreamData_[0].GetPcmData(), CopyDataLength);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Get data from outputBuffer_ fail");
    downStreamData_[0].SetIsFinished(GetAudioNodeDataFinishedFlag() &&
        inputBuffer_.GetSize() == 0 && outputBuffer_.GetSize() == 0);
    WriteOutputData(&downStreamData_[0]);
    return SUCCESS;
}

int32_t AudioSuiteHoaRenderNode::GetProcessedToBuffer()
{
    for (auto* buffer : preNodeResult_) {
        if (buffer->GetIsFinished()) {
            upstreamFinished_ = true;
        }
        uint32_t inSize = buffer->GetDataSize();
        if (inSize > 0) {
            int32_t ret;
            if (GetNodeBypassStatus()) {
                AudioSuitePcmBuffer *convertData = convert_.Process(
                    buffer, const_cast<PcmBufferFormat &>(GetAudioNodeOutPcmFormat()));
                CHECK_AND_RETURN_RET_LOG(convertData != nullptr, ERROR, "Format conversion failed");
                ret = outputBuffer_.PushData(convertData->GetPcmData(), convertData->GetDataSize());
            } else {
                ret = inputBuffer_.PushData(buffer->GetPcmData(), inSize);
            }
            CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "PushData failed.");
        }
    }
    return SUCCESS;
}

int32_t AudioSuiteHoaRenderNode::ProcessAlgoChunks()
{
    uint32_t inChannels = GetAudioNodeFormat().audioChannelInfo.numChannels;
    uint32_t outChannels = nodeParameter_.outChannels;
    uint32_t chunkInSize = ALGO_PROCESS_FRAME_LEN * inChannels * sizeof(int32_t);
    uint32_t chunkOutSize = ALGO_PROCESS_FRAME_LEN * outChannels * sizeof(int32_t);

    while (inputBuffer_.GetSize() >= chunkInSize) {
        int32_t ret = inputBuffer_.GetData(algoInputBuffer_.data(), chunkInSize);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "GetData from inputBuffer failed.");

        algoInput_[0] = algoInputBuffer_.data();
        algoOutput_[0] = algoOutputBuffer_.data();

        // for dfx
        auto startTime = std::chrono::steady_clock::now();
        Trace trace("AudioSuiteHoaRenderNode::SignalProcess Start");

        ret = algoInterface_->Apply(algoInput_, algoOutput_);
        CHECK_AND_RETURN_RET_LOG(ret >= 0, ERROR, "Node SignalProcess Apply failed");

        trace.End();

        // for dfx
        auto endTime = std::chrono::steady_clock::now();
        auto processDuration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
        CheckEffectNodeProcessTime(ALGO_PROCESS_FRAME_LEN, static_cast<uint64_t>(processDuration));

        outputBuffer_.PushData(algoOutputBuffer_.data(), chunkOutSize);
    }

    if (upstreamFinished_ && inputBuffer_.GetSize() > 0 && inputBuffer_.GetSize() < chunkInSize) {
        uint32_t padSize = chunkInSize - inputBuffer_.GetSize();
        std::vector<uint8_t> zeros(padSize, 0);
        inputBuffer_.PushData(zeros.data(), padSize);

        int32_t ret = inputBuffer_.GetData(algoInputBuffer_.data(), chunkInSize);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "GetData failed.");
        
        algoInput_[0] = algoInputBuffer_.data();
        algoOutput_[0] = algoOutputBuffer_.data();
        
        ret = algoInterface_->Apply(algoInput_, algoOutput_);
        CHECK_AND_RETURN_RET_LOG(ret >= 0, ERROR, "Apply failed.");
        
        outputBuffer_.PushData(algoOutputBuffer_.data(), chunkOutSize);
    }

    return SUCCESS;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS