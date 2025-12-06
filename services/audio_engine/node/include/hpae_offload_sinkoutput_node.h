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

#ifndef HPAE_OFFLOAD_SINK_OUTPUT_NODE_H
#define HPAE_OFFLOAD_SINK_OUTPUT_NODE_H
#include <memory>
#include "hpae_backoff_controller.h"
#include "hpae_node.h"
#include "hpae_pcm_buffer.h"
#include "audio_info.h"
#include "sink/i_audio_render_sink.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#ifdef ENABLE_HOOK_PCM
#include "high_resolution_timer.h"
#include "hpae_pcm_dumper.h"
#endif
namespace OHOS {
namespace AudioStandard {
namespace HPAE {
typedef void(*AppCallbackFunc)(void* pHndl);
class HpaeOffloadSinkOutputNode : public InputNode<HpaePcmBuffer*> {
public:
    HpaeOffloadSinkOutputNode(HpaeNodeInfo& nodeInfo);
    virtual ~HpaeOffloadSinkOutputNode();
    virtual void DoProcess() override;
    virtual bool Reset() override;
    virtual bool ResetAll() override;
    void Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer*>>& preNode) override;
    void DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer*>>& preNode) override;
    int32_t GetRenderSinkInstance(const std::string &deviceClass, const std::string &deviceNetworkId);
    int32_t RenderSinkInit(IAudioSinkAttr& attr);
    int32_t RenderSinkDeInit();
    int32_t RenderSinkFlush();
    int32_t RenderSinkStart();
    int32_t RenderSinkStop();
    size_t GetPreOutNum();
    StreamManagerState GetSinkState(void);
    int32_t SetSinkState(StreamManagerState sinkState);
    const char* GetRenderFrameData(void);
    // need flush hdi cache and rewind
    void StopStream();
    // flush need clear sinkoutputjnode cache
    void FlushStream();
    // set offload policy state
    void SetPolicyState(int32_t policyState);
    // get offload latency for sinkinputnode, maybe extend to all node
    uint64_t GetLatency();
    // set timeout to suspend render and stop hdi
    int32_t SetTimeoutStopThd(uint32_t timeoutThdMs);
    // set offload render callback type in hdi
    int32_t SetOffloadRenderCallbackType(int32_t type);
    void SetSpeed(float speed);

    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid);
private:
    // lock/unlock running lock
    void RunningLock(bool isLock);
    // Set hdi buffer size, change after render frame success
    void SetBufferSizeWhileRenderFrame();
    void SetBufferSize();
    int32_t ProcessRenderFrame();
    // get presentation position from hdi, only trigger in offloadcallback
    int32_t UpdatePresentationPosition();
    // return hdi cache len in us, cal by hdiPos_
    uint64_t CalcOffloadCacheLenInHdi();
    // set hdi volume when first write
    void OffloadSetHdiVolume();
    // reset hdipos and firstWriteHdi
    void OffloadReset();
    // register callback to hdi
    void RegOffloadCallback();
    // offload callback reg to hdi
    void OffloadCallback(const RenderCallbackType type);
    // check when stop hdi, if need suspend
    bool CheckIfSuspend();
    // check renderFrame ret to decide whether need sleep
    void OffloadNeedSleep(int32_t retType);
    // renderFrame and set state
    int32_t WriteFrameToHdi();

    InputPort<HpaePcmBuffer*> inputStream_;
    std::vector<char> renderFrameData_;
    std::vector<char> renderFrameDataTemp_;
    std::shared_ptr<IAudioRenderSink> audioRendererSink_ = nullptr;
    uint32_t renderId_ = HDI_INVALID_ID;
    IAudioSinkAttr sinkOutAttr_;
    StreamManagerState state_ = STREAM_MANAGER_NEW;
#ifdef ENABLE_HOOK_PCM
    HighResolutionTimer intervalTimer_;
    std::unique_ptr<HpaePcmDumper> outputPcmDumper_ = nullptr;
#endif

    AudioOffloadType hdiPolicyState_ = OFFLOAD_ACTIVE_FOREGROUND;
    struct OffloadPolicyTask {
        bool flag = false; // indicate if task exsit
        AudioOffloadType state = OFFLOAD_DEFAULT;
        TimePoint time;
    } setPolicyStateTask_;

    bool firstWriteHdi_ = true;
    uint64_t writePos_ = 0;
    int32_t setHdiBufferSizeNum_ = 0;

    std::atomic<bool> isHdiFull_ = false;

    uint32_t frameLenMs_ = 0;
    uint32_t timeoutThdFrames_ = 0;
    // first stand for pos(in us), second stand for time
    std::pair<uint64_t, TimePoint> hdiPos_;
    uint32_t suspendCount_ = 0;
    float speed_ = 1.0f;
    uint64_t hdiRealPos_ = 0;

    HpaeBackoffController backoffController_;
};

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS

#endif