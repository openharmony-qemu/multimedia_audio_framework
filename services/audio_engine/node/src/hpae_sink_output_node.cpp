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
#define LOG_TAG "HpaeSinkOutputNode"
#endif

#include <hpae_sink_output_node.h>
#include "audio_errors.h"
#include <iostream>
#include "hpae_format_convert.h"
#include "audio_engine_log.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
namespace {
constexpr uint32_t SLEEP_TIME_IN_US = 2000;
}

HpaeSinkOutputNode::HpaeSinkOutputNode(HpaeNodeInfo &nodeInfo)
    : HpaeNode(nodeInfo),
      renderFrameData_(nodeInfo.frameLen * nodeInfo.channels * GetSizeFromFormat(nodeInfo.format)),
      interleveData_(nodeInfo.frameLen * nodeInfo.channels)
{
#ifdef ENABLE_HOOK_PCM
    outputPcmDumper_ = std::make_unique<HpaePcmDumper>("HpaeSinkOutputNode_Out_bit_" + std::to_string(GetBitWidth()) +
                                                       "_ch_" + std::to_string(GetChannelCount()) + "_rate_" +
                                                       std::to_string(GetSampleRate()) + ".pcm");
    AUDIO_INFO_LOG("HpaeSinkOutputNode name is %{public}s", sinkOutAttr_.adapterName.c_str());
#endif
}

void HpaeSinkOutputNode::HandleRemoteTiming()
{
    remoteTimer_.Stop();
    uint64_t remoteElapsed = remoteTimer_.Elapsed();
    auto now = std::chrono::high_resolution_clock::now();
    remoteTimePoint_ += std::chrono::milliseconds(20);  // 20ms frameLen, need optimize
    std::this_thread::sleep_for(remoteSleepTime_);
    if (remoteTimePoint_ > now + std::chrono::milliseconds(remoteElapsed)) {
        remoteSleepTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(remoteTimePoint_ - now) -
                           std::chrono::milliseconds(remoteElapsed);
    } else {
        remoteSleepTime_ = std::chrono::milliseconds(0);
    }
    remoteTimer_.Start();
}

void HpaeSinkOutputNode::DoProcess()
{
    auto rate = "rate[" + std::to_string(GetSampleRate()) + "]_";
    auto ch = "ch[" + std::to_string(GetChannelCount()) + "]_";
    auto len = "len[" + std::to_string(GetFrameLen()) + "]_";
    auto format = "bit[" + std::to_string(GetBitWidth()) + "]";
    Trace trace("HpaeSinkOutputNode::DoProcess " + rate + ch + len + format);
    if (audioRendererSink_ == nullptr) {
        AUDIO_WARNING_LOG("audioRendererSink_ is nullptr sessionId: %{public}u", GetSessionId());
        return;
    }
    std::vector<HpaePcmBuffer *> &outputVec = inputStream_.ReadPreOutputData();
    if (outputVec.empty()) {
        return;
    }
    HpaePcmBuffer *outputData = outputVec.front();
    ConvertFromFloat(
        GetBitWidth(), GetChannelCount() * GetFrameLen(), outputData->GetPcmDataBuffer(), renderFrameData_.data());
    uint64_t writeLen = 0;
    char *renderFrameData = (char *)renderFrameData_.data();

#ifdef ENABLE_HOOK_PCM
    HighResolutionTimer timer;
    timer.Start();
    intervalTimer_.Stop();
    outputPcmDumper_->CheckAndReopenHandlde();
    if (outputPcmDumper_) {
        outputPcmDumper_->Dump((int8_t *)renderFrameData, renderFrameData_.size());
    }
#endif
    if (GetDeviceClass() == "remote") {
        HandleRemoteTiming();
    }
    auto ret = audioRendererSink_->RenderFrame(*renderFrameData, renderFrameData_.size(), writeLen);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("HpaeSinkOutputNode: RenderFrame failed");
        usleep(SLEEP_TIME_IN_US);
        return;
    }
#ifdef ENABLE_HOOK_PCM
    timer.Stop();
    uint64_t elapsed = timer.Elapsed();
    AUDIO_DEBUG_LOG("HpaeSinkOutputNode :name %{public}s, RenderFrame elapsed time: %{public}" PRIu64 " ms",
        sinkOutAttr_.adapterName.c_str(),
        elapsed);
    intervalTimer_.Start();
#endif
    return;
}

const char *HpaeSinkOutputNode::GetRenderFrameData(void)
{
    return renderFrameData_.data();
}

bool HpaeSinkOutputNode::Reset()
{
    const auto preOutputMap = inputStream_.GetPreOuputMap();
    for (const auto &preOutput : preOutputMap) {
        OutputPort<HpaePcmBuffer *> *output = preOutput.first;
        inputStream_.DisConnect(output);
    }
    return true;
}

bool HpaeSinkOutputNode::ResetAll()
{
    const auto preOutputMap = inputStream_.GetPreOuputMap();
    for (const auto &preOutput : preOutputMap) {
        OutputPort<HpaePcmBuffer *> *output = preOutput.first;
        std::shared_ptr<HpaeNode> hpaeNode = preOutput.second;
        if (hpaeNode->ResetAll()) {
            inputStream_.DisConnect(output);
        }
    }
    return true;
}

void HpaeSinkOutputNode::Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    inputStream_.Connect(preNode->GetSharedInstance(), preNode->GetOutputPort());
}

void HpaeSinkOutputNode::DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    inputStream_.DisConnect(preNode->GetOutputPort());
}

int32_t HpaeSinkOutputNode::GetRenderSinkInstance(std::string deviceClass, std::string deviceNetId)
{
    if (deviceNetId.empty()) {
        renderId_ = HdiAdapterManager::GetInstance().GetRenderIdByDeviceClass(deviceClass, HDI_ID_INFO_DEFAULT, true);
    } else {
        renderId_ = HdiAdapterManager::GetInstance().GetRenderIdByDeviceClass(deviceClass, deviceNetId, true);
    }
    audioRendererSink_ = HdiAdapterManager::GetInstance().GetRenderSink(renderId_, true);
    if (audioRendererSink_ == nullptr) {
        AUDIO_ERR_LOG("get sink fail, deviceClass: %{public}s, deviceNetId: %{public}s, renderId_: %{public}u",
            deviceClass.c_str(),
            deviceNetId.c_str(),
            renderId_);
        HdiAdapterManager::GetInstance().ReleaseId(renderId_);
        return ERROR;
    }
    return SUCCESS;
}

int32_t HpaeSinkOutputNode::RenderSinkInit(IAudioSinkAttr &attr)
{
    if (audioRendererSink_ == nullptr) {
        return ERROR;
    }

    sinkOutAttr_ = attr;
    state_ = RENDERER_PREPARED;
#ifdef ENABLE_HOOK_PCM
    HighResolutionTimer timer;
    timer.Start();
#endif
    int32_t ret = audioRendererSink_->Init(attr);
#ifdef ENABLE_HOOK_PCM
    timer.Stop();
    uint64_t interval = timer.Elapsed();
    AUDIO_INFO_LOG("HpaeSinkOutputNode: name %{public}s, RenderSinkInit Elapsed: %{public}" PRIu64
                   " ms ret: %{public}d",
        sinkOutAttr_.adapterName.c_str(),
        interval,
        ret);
    std::string adapterName = sinkOutAttr_.adapterName;
    outputPcmDumper_ = std::make_unique<HpaePcmDumper>(
        "HpaeSinkOutputNode_" + adapterName + "_bit_" + std::to_string(GetBitWidth()) + "_ch_" +
        std::to_string(GetChannelCount()) + "_rate_" + std::to_string(GetSampleRate()) + ".pcm");
#endif
    return ret;
}

int32_t HpaeSinkOutputNode::RenderSinkDeInit(void)
{
    if (audioRendererSink_ == nullptr) {
        return ERROR;
    }
    state_ = RENDERER_INVALID;
#ifdef ENABLE_HOOK_PCM
    HighResolutionTimer timer;
    timer.Start();
#endif
    audioRendererSink_->DeInit();
    audioRendererSink_ = nullptr;
    HdiAdapterManager::GetInstance().ReleaseId(renderId_);
#ifdef ENABLE_HOOK_PCM
    timer.Stop();
    uint64_t interval = timer.Elapsed();
    AUDIO_INFO_LOG("HpaeSinkOutputNode: name %{public}s, RenderSinkDeInit Elapsed: %{public}" PRIu64 " ms",
        sinkOutAttr_.adapterName.c_str(),
        interval);
#endif
    return SUCCESS;
}

int32_t HpaeSinkOutputNode::RenderSinkFlush(void)
{
    if (audioRendererSink_ == nullptr) {
        return ERROR;
    }
    return audioRendererSink_->Flush();
}

int32_t HpaeSinkOutputNode::RenderSinkPause(void)
{
    if (audioRendererSink_ == nullptr) {
        return ERROR;
    }
    audioRendererSink_->Pause();
    state_ = RENDERER_PAUSED;
    return SUCCESS;
}

int32_t HpaeSinkOutputNode::RenderSinkReset(void)
{
    if (audioRendererSink_ == nullptr) {
        return ERROR;
    }
    return audioRendererSink_->Reset();
}

int32_t HpaeSinkOutputNode::RenderSinkResume(void)
{
    if (audioRendererSink_ == nullptr) {
        return ERROR;
    }
    int32_t ret = audioRendererSink_->Resume();
    if (ret != SUCCESS) {
        return ret;
    }
    state_ = RENDERER_RUNNING;
    return SUCCESS;
}

int32_t HpaeSinkOutputNode::RenderSinkStart(void)
{
    if (audioRendererSink_ == nullptr) {
        return ERROR;
    }

    int32_t ret;
#ifdef ENABLE_HOOK_PCM
    HighResolutionTimer timer;
    timer.Start();
#endif
    ret = audioRendererSink_->Start();
    if (ret != SUCCESS) {
        return ERROR;
    }
#ifdef ENABLE_HOOK_PCM
    timer.Stop();
    uint64_t interval = timer.Elapsed();
    AUDIO_INFO_LOG("HpaeSinkOutputNode: name %{public}s, RenderSinkStart Elapsed: %{public}" PRIu64 " ms",
        sinkOutAttr_.adapterName.c_str(),
        interval);
#endif
    state_ = RENDERER_RUNNING;
    if (GetDeviceClass() == "remote") {
        remoteTimePoint_ = std::chrono::high_resolution_clock::now();
    }
    return SUCCESS;
}

int32_t HpaeSinkOutputNode::RenderSinkStop(void)
{
    if (audioRendererSink_ == nullptr) {
        return ERROR;
    }
    int32_t ret;
#ifdef ENABLE_HOOK_PCM
    HighResolutionTimer timer;
    timer.Start();
#endif
    ret = audioRendererSink_->Stop();
    if (ret != SUCCESS) {
        return ret;
    }
#ifdef ENABLE_HOOK_PCM
    timer.Stop();
    uint64_t interval = timer.Elapsed();
    AUDIO_INFO_LOG("HpaeSinkOutputNode: name %{public}s, RenderSinkStop Elapsed: %{public}" PRIu64 " ms",
        sinkOutAttr_.adapterName.c_str(), interval);
#endif
    state_ = RENDERER_STOPPED;
    return SUCCESS;
}

RendererState HpaeSinkOutputNode::GetSinkState(void)
{
    return state_;
}

size_t HpaeSinkOutputNode::GetPreOutNum()
{
    return inputStream_.GetPreOutputNum();
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
