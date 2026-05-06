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
#define LOG_TAG "HpaeDirectSinkOutputNode"
#endif

#include "hpae_direct_sinkoutput_node.h"
#include "audio_errors.h"
#include <iostream>
#include <cinttypes>
#include "hpae_format_convert.h"
#include "hpae_node_common.h"
#include "audio_utils.h"
#include "audio_engine_log.h"
#include "audio_mute_factor_manager.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
namespace {
constexpr uint32_t SLEEP_TIME_IN_US = 20000;
static constexpr uint32_t AUDIO_MS_PER_S = 1000;
static constexpr int32_t AUDIO_US_PER_MS = 1000;
static constexpr int32_t AUDIO_FRAME_WORK_LATENCY_US = 40000;
static constexpr int32_t AUDIO_DEFAULT_LATENCY_US = 160000;
}

HpaeDirectSinkOutputNode::HpaeDirectSinkOutputNode(HpaeNodeInfo &nodeInfo)
    : HpaeNode(nodeInfo),
      renderFrameData_(nodeInfo.frameLen * static_cast<uint32_t>(nodeInfo.channels) *
          GetSizeFromFormat(nodeInfo.format))
{
    renderSize_ = renderFrameData_.size();
    outputSize_ = renderSize_;
    currentSize_ = 0;
    AUDIO_INFO_LOG("renderSize = %{public}zu", renderSize_);
#ifdef ENABLE_HIDUMP_DFX
    SetNodeName("hpaeDirectSinkOutputNode");
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeAdmin(true, GetNodeInfo());
    }
#endif
}

HpaeDirectSinkOutputNode::~HpaeDirectSinkOutputNode()
{
#ifdef ENABLE_HIDUMP_DFX
    AUDIO_INFO_LOG("NodeId: %{public}u NodeName: %{public}s destructed.",
        GetNodeId(), GetNodeName().c_str());
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeAdmin(false, GetNodeInfo());
    }
#endif
}

bool HpaeDirectSinkOutputNode::ReadDataAndConvertFormat()
{
    while (currentSize_ < renderSize_) {
        std::vector<HpaePcmBuffer *> &outputVec = inputStream_.ReadPreOutputData();
        CHECK_AND_RETURN_RET(!outputVec.empty(), false);
        HpaePcmBuffer *outputData = outputVec.front();
        CHECK_AND_RETURN_RET_LOG(outputData, false, "outputData is nullptr");
        if (!outputData->IsValid()) {
            periodTimer_.Stop();
            uint64_t usedTimeUs = static_cast<uint64_t>(periodTimer_.Elapsed<std::chrono::microseconds>());
            usleep(SLEEP_TIME_IN_US > usedTimeUs ? SLEEP_TIME_IN_US - usedTimeUs : 0);
            periodTimer_.Start();
            return false;
        }
        uint32_t frameLen = outputData->GetFrameLen();
        uint32_t channels = outputData->GetChannelCount();
        uint32_t inDurationMs = frameLen * AUDIO_MS_PER_S / outputData->GetSampleRate();
        uint32_t outDurationMs = GetFrameLen() * AUDIO_MS_PER_S / GetSampleRate();
        if (renderFrameData_.size() == renderSize_ && inDurationMs != outDurationMs) {
            outputSize_ = frameLen * channels * static_cast<size_t>(GetSizeFromFormat(GetBitWidth()));
            AUDIO_INFO_LOG("Update outputSize to %{public}zu", outputSize_);
            renderFrameData_.resize(outputSize_ + renderSize_);
        }
        ConvertFromFloat(
            GetBitWidth(), channels * frameLen, outputData->GetPcmDataBuffer(),
            renderFrameData_.data() + currentSize_);
        currentSize_ += outputSize_;
    }
    return true;
}

void HpaeDirectSinkOutputNode::DoProcess()
{
    Trace trace("HpaeDirectSinkOutputNode::DoProcess " + GetTraceInfo());
    if (audioRendererSink_ == nullptr) {
        AUDIO_WARNING_LOG("audioRendererSink_ is nullptr sessionId: %{public}u", GetSessionId());
        return;
    }

    CHECK_AND_RETURN(ReadDataAndConvertFormat());
    uint64_t writeLen = 0;
    char *renderFrameData = reinterpret_cast<char *>(renderFrameData_.data());

#ifdef ENABLE_HOOK_PCM
    HighResolutionTimer timer;
    timer.Start();
    intervalTimer_.Stop();
#endif
    int32_t ret = audioRendererSink_->RenderFrame(*renderFrameData, renderSize_, writeLen);
    if (ret != SUCCESS || writeLen != renderSize_) {
        AUDIO_ERR_LOG("RenderFrame failed, write len:%{public}" PRIu64 "", writeLen);
        periodTimer_.Stop();
        uint64_t usedTimeUs = static_cast<uint64_t>(periodTimer_.Elapsed<std::chrono::microseconds>());
        usleep(SLEEP_TIME_IN_US > usedTimeUs ? SLEEP_TIME_IN_US - usedTimeUs : 0);
    }
    periodTimer_.Start();

    std::move(renderFrameData_.begin() + renderSize_, renderFrameData_.begin() + currentSize_,
        renderFrameData_.begin());
    currentSize_ -= renderSize_;
#ifdef ENABLE_HOOK_PCM
    timer.Stop();
    int64_t elapsed = timer.Elapsed();
    AUDIO_DEBUG_LOG("name %{public}s, RenderFrame elapsed time: %{public}" PRId64 " ms",
        sinkOutAttr_.adapterName.c_str(), elapsed);
    intervalTimer_.Start();
#endif
}

bool HpaeDirectSinkOutputNode::Reset()
{
    const auto preOutputMap = inputStream_.GetPreOutputMap();
    for (const auto &preOutput : preOutputMap) {
        OutputPort<HpaePcmBuffer *> *output = preOutput.first;
        inputStream_.DisConnect(output);
    }
    return true;
}

bool HpaeDirectSinkOutputNode::ResetAll()
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

void HpaeDirectSinkOutputNode::Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    CHECK_AND_RETURN_LOG(audioRendererSink_, "audioRendererSink_ is nullptr sessionId: %{public}u", GetSessionId());
    CHECK_AND_RETURN_LOG(audioRendererSink_->IsInited(), "audioRendererSink_ is not init");
    inputStream_.Connect(preNode->GetSharedInstance(), preNode->GetOutputPort());
#ifdef ENABLE_HIDUMP_DFX
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeInfo(true, GetNodeId(), preNode->GetSharedInstance()->GetNodeId());
    }
#endif
}

void HpaeDirectSinkOutputNode::DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    inputStream_.DisConnect(preNode->GetOutputPort());
#ifdef ENABLE_HIDUMP_DFX
    if (auto callback = GetNodeStatusCallback().lock()) {
        auto preNodeReal = preNode->GetSharedInstance();
        callback->OnNotifyDfxNodeInfo(false, GetNodeId(), preNodeReal->GetNodeId());
    }
#endif
}

int32_t HpaeDirectSinkOutputNode::GetRenderSinkInstance(const std::string &deviceClass,
    const std::string &deviceNetworkId)
{
    renderId_ = HdiAdapterManager::GetInstance().GetRenderIdByDeviceClass(deviceClass, "", true,
        true, GetNodeInfo().routeFlag);
    audioRendererSink_ = HdiAdapterManager::GetInstance().GetRenderSink(renderId_, true);
    if (audioRendererSink_ == nullptr) {
        AUDIO_ERR_LOG("get direct sink fail, deviceClass: %{public}s, renderId_: %{public}u",
            deviceClass.c_str(), renderId_);
        HdiAdapterManager::GetInstance().ReleaseId(renderId_);
        return ERROR;
    }
    return SUCCESS;
}

int32_t HpaeDirectSinkOutputNode::RenderSinkInit(IAudioSinkAttr &attr)
{
    CHECK_AND_RETURN_RET_LOG(audioRendererSink_, ERR_ILLEGAL_STATE,
        "audioRendererSink_ is nullptr sessionId: %{public}u", GetSessionId());
    auto mdmMute = AudioMuteFactorManager::GetInstance().GetMdmMuteStatus();
    float volume = mdmMute ? 0.0f : 1.0f;
    sinkOutAttr_ = attr;
    if (audioRendererSink_->IsInited()) {
        SetSinkState(STREAM_MANAGER_IDLE);
        audioRendererSink_->SetVolume(volume, volume);
        AUDIO_WARNING_LOG("audioRenderSink already inited");
        return SUCCESS;
    }
#ifdef ENABLE_HOOK_PCM
    HighResolutionTimer timer;
    timer.Start();
#endif
    int32_t ret = audioRendererSink_->Init(attr);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
        "audioRendererSink_ init failed, errCode is %{public}d", ret);
    ret = audioRendererSink_->SetVolume(volume, volume);
    SetSinkState(STREAM_MANAGER_IDLE);
#ifdef ENABLE_HOOK_PCM
    timer.Stop();
    int64_t interval = timer.Elapsed();
    AUDIO_INFO_LOG("name %{public}s, RenderSinkInit Elapsed: %{public}" PRId64 " ms",
        sinkOutAttr_.adapterName.c_str(), interval);
#endif
    return ret;
}

int32_t HpaeDirectSinkOutputNode::RenderSinkDeInit()
{
    CHECK_AND_RETURN_RET_LOG(audioRendererSink_, ERR_ILLEGAL_STATE,
        "audioRendererSink_ is nullptr sessionId: %{public}u", GetSessionId());
    SetSinkState(STREAM_MANAGER_RELEASED);
#ifdef ENABLE_HOOK_PCM
    HighResolutionTimer timer;
    timer.Start();
#endif
    audioRendererSink_->DeInit();
    audioRendererSink_ = nullptr;
    HdiAdapterManager::GetInstance().ReleaseId(renderId_);
#ifdef ENABLE_HOOK_PCM
    timer.Stop();
    int64_t interval = timer.Elapsed();
    AUDIO_INFO_LOG("name %{public}s, RenderSinkDeInit Elapsed: %{public}" PRId64 " ms",
        sinkOutAttr_.adapterName.c_str(), interval);
#endif
    return SUCCESS;
}

int32_t HpaeDirectSinkOutputNode::RenderSinkFlush(void)
{
    CHECK_AND_RETURN_RET_LOG(audioRendererSink_, ERR_ILLEGAL_STATE,
        "audioRendererSink_ is nullptr sessionId: %{public}u", GetSessionId());
    return audioRendererSink_->Flush();
}

int32_t HpaeDirectSinkOutputNode::RenderSinkStart(void)
{
    CHECK_AND_RETURN_RET_LOG(audioRendererSink_, ERR_ILLEGAL_STATE,
        "audioRendererSink_ is nullptr sessionId: %{public}u", GetSessionId());

    int32_t ret;
#ifdef ENABLE_HOOK_PCM
    HighResolutionTimer timer;
    timer.Start();
#endif
    ret = audioRendererSink_->Start();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
        "audioRendererSink_ start failed, errCode is %{public}d", ret);
#ifdef ENABLE_HOOK_PCM
    timer.Stop();
    int64_t interval = timer.Elapsed();
    AUDIO_INFO_LOG("name %{public}s, RenderSinkStart Elapsed: %{public}" PRId64 " ms",
        sinkOutAttr_.adapterName.c_str(), interval);
#endif
    SetSinkState(STREAM_MANAGER_RUNNING);
    periodTimer_.Start();
    return SUCCESS;
}

int32_t HpaeDirectSinkOutputNode::RenderSinkStop(void)
{
    CHECK_AND_RETURN_RET_LOG(audioRendererSink_, ERR_ILLEGAL_STATE,
        "audioRendererSink_ is nullptr sessionId: %{public}u", GetSessionId());
    SetSinkState(STREAM_MANAGER_SUSPENDED);
    int32_t ret;
#ifdef ENABLE_HOOK_PCM
    HighResolutionTimer timer;
    timer.Start();
#endif
    ret = audioRendererSink_->Stop();
    currentSize_ = 0;
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
        "audioRendererSink_ stop failed, errCode is %{public}d", ret);
#ifdef ENABLE_HOOK_PCM
    timer.Stop();
    int64_t interval = timer.Elapsed();
    AUDIO_INFO_LOG("name %{public}s, RenderSinkStop Elapsed: %{public}" PRId64 " ms",
        sinkOutAttr_.adapterName.c_str(), interval);
#endif
    return SUCCESS;
}

StreamManagerState HpaeDirectSinkOutputNode::GetSinkState(void)
{
    return state_;
}

int32_t HpaeDirectSinkOutputNode::SetSinkState(StreamManagerState sinkState)
{
    HILOG_COMM_INFO("[SetSinkState]Sink state change:[%{public}s]-->[%{public}s]",
        ConvertStreamManagerState2Str(state_).c_str(), ConvertStreamManagerState2Str(sinkState).c_str());
    state_ = sinkState;
    return SUCCESS;
}

const char *HpaeDirectSinkOutputNode::GetRenderFrameData(void)
{
    return renderFrameData_.data();
}

uint64_t HpaeDirectSinkOutputNode::GetLatency()
{
    CHECK_AND_RETURN_RET_LOG(audioRendererSink_ != nullptr, 0, "audioRendererSink_ is nullptr");
    if (latency_ > 0) {
        return latency_;
    }
    uint32_t latency = 0;
    if (audioRendererSink_->GetLatency(latency) == SUCCESS) {
        latency_ = latency * AUDIO_US_PER_MS + AUDIO_FRAME_WORK_LATENCY_US;
    } else {
        AUDIO_INFO_LOG("get latency failed, use default");
        latency_ = AUDIO_DEFAULT_LATENCY_US;
    }
    return latency_;
}

int32_t HpaeDirectSinkOutputNode::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
    CHECK_AND_RETURN_RET_LOG(audioRendererSink_ != nullptr, ERROR, "audioRendererSink_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(audioRendererSink_->IsInited(), ERR_ILLEGAL_STATE, "audioRendererSink_ not init");
    return audioRendererSink_->UpdateAppsUid(appsUid);
}

void HpaeDirectSinkOutputNode::NotifyStreamChangeToSink(StreamChangeType change,
    uint32_t sessionId, StreamUsage usage, RendererState state, uint32_t appUid)
{
    CHECK_AND_RETURN_LOG(audioRendererSink_ != nullptr, "audioRendererSink_ is nullptr");
    CHECK_AND_RETURN_LOG(audioRendererSink_->IsInited(), "audioRendererSink_ not init");
    audioRendererSink_->NotifyStreamChangeToSink(change, sessionId, usage, state, appUid);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
