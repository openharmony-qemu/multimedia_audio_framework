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

#ifndef HPAE_FAST_SOURCE_INPUT_NODE_H
#define HPAE_FAST_SOURCE_INPUT_NODE_H

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "audio_dump_pcm.h"
#include "capturer_clock_manager.h"
#include "common/hdi_adapter_info.h"
#include "hpae_node.h"
#include "hpae_pcm_buffer.h"
#include "linear_pos_time_model.h"
#include "manager/hdi_adapter_manager.h"
#include "oh_audio_buffer.h"
#include "source/i_audio_capture_source.h"

namespace OHOS {
namespace AudioStandard {
struct SignalDetectAgent;
class OHAudioBuffer;
namespace HPAE {

class HpaeFastSourceInputNode : public OutputNode<HpaePcmBuffer *> {
public:
    explicit HpaeFastSourceInputNode(HpaeNodeInfo &nodeInfo);
    ~HpaeFastSourceInputNode() override;

    void DoProcess() override;
    bool Reset() override;
    bool ResetAll() override;
    std::shared_ptr<HpaeNode> GetSharedInstance() final;
    OutputPort<HpaePcmBuffer*> *GetOutputPort() final;
    OutputPort<HpaePcmBuffer*> *GetOutputPort(HpaeNodeInfo &nodeInfo, bool isDisConnect = false) final;
    HpaeSourceBufferType GetOutputPortBufferType(HpaeNodeInfo &nodeInfo) final;

    int32_t GetCapturerSourceInstance(const std::string &deviceClass, const std::string &deviceNetId,
        const SourceType &sourceType, const std::string &sourceName, const std::string &busAddress = "");
    int32_t CapturerSourceInit(IAudioSourceAttr &attr);
    int32_t CapturerSourceDeInit();
    int32_t CapturerSourceFlush(void);
    int32_t CapturerSourcePause(void);
    int32_t CapturerSourceReset(void);
    int32_t CapturerSourceResume(void);
    int32_t CapturerSourceStart(void);
    int32_t CapturerSourceStop(void);
    StreamManagerState GetSourceState(void);
    int32_t SetSourceState(StreamManagerState sourceState);
    size_t GetOutputPortNum();
    uint32_t GetCaptureId() const;
    void UpdateAppsUidAndSessionId(std::vector<int32_t> &appsUid, std::vector<int32_t> &sessionsId);
    void NotifyStreamChangeToSource(StreamChangeType change,
        uint32_t sessionId, SourceType source, CapturerState state, uint32_t appUid = INVALID_UID, bool mute = false);
    bool PrepareNextLoop(int64_t &wakeUpTime);

private:
    static constexpr int64_t ONE_MILLISECOND_DURATION_NS = 1000000;
    static constexpr int64_t RELATIVE_SLEEP_TIME_NS = 5000000;
    static constexpr int64_t MAX_WAKEUP_TIME_NS = 2000000000;
    static constexpr int64_t RECORD_DELAY_TIME_NS = 4000000;
    static constexpr int64_t RECORD_VOIP_DELAY_TIME_NS = 20000000;

    int32_t GetCapturerSourceAdapter(
        const std::string &deviceClass, const SourceType &sourceType, const std::string &info);
    int32_t PrepareDeviceBuffer();
    int32_t GetAdapterBufferInfo();
    void InitAudiobuffer(bool resetReadWritePos);
    void AsyncGetPosTime();
    void StopUpdateThread();
    bool ResetReadPosition();
    bool GetDeviceHandleInfo(uint64_t &frames, int64_t &nanoTime);
    int64_t GetPredictNextWriteTime(uint64_t posInFrame);
    void CheckWakeUpTime(int64_t &wakeUpTime);
    void RecordCheckSyncInfo(uint64_t curReadPos);
    bool IsVoipFast();
    bool TryReadOneSpan(uint64_t handlePos);
    AudioStreamInfo GetDfxStreamInfo();
    void InitDumpFile();
    void InitLatencyMeasurement();
    void DeinitLatencyMeasurement();
    void CheckRecordSignal(uint8_t *buffer, size_t bufferSize);

    OutputPort<HpaePcmBuffer *> outputStream_;
    PcmBufferInfo pcmBufferInfo_;
    HpaePcmBuffer inputAudioBuffer_;
    std::shared_ptr<IAudioCaptureSource> audioCapturerSource_ = nullptr;
    uint32_t captureId_ = HDI_INVALID_ID;
    IAudioSourceAttr audioSourceAttr_ = {};
    StreamManagerState state_ = STREAM_MANAGER_NEW;

    std::shared_ptr<OHAudioBuffer> srcAudioBuffer_ = nullptr;
    int srcBufferFd_ = -1;
    uint32_t srcTotalSizeInframe_ = 0;
    uint32_t srcSpanSizeInframe_ = 0;
    uint32_t srcByteSizePerFrame_ = 0;
    uint32_t syncInfoSize_ = 0;
    LinearPosTimeModel writeTimeModel_;
    int64_t spanDuration_ = 0;
    uint64_t curReadPos_ = 0;
    bool needReSyncPosition_ = true;
    std::thread updatePosTimeThread_;
    std::mutex updateThreadLock_;
    std::condition_variable updateThreadCV_;
    std::atomic<bool> stopUpdateThread_ = false;
    std::atomic<uint64_t> posInFrame_ = 0;
    std::atomic<int64_t> timeInNano_ = 0;

    mutable int64_t volumeDataCount_ = 0;
    std::string logUtilsTag_ = "";
    FILE *dumpHdi_ = nullptr;
    std::string dumpHdiName_ = "";
    bool latencyMeasEnabled_ = false;
    bool signalDetected_ = false;
    std::shared_ptr<SignalDetectAgent> signalDetectAgent_ = nullptr;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS

#endif
