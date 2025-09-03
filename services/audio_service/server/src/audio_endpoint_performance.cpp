/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioEndpointInner"
#endif

#include "audio_endpoint.h"
#include "audio_endpoint_private.h"
#include "audio_performance_monitor.h"

#include <string>
#include <memory>

#include "audio_service_log.h"
#include "manager/hdi_adapter_manager.h"

namespace OHOS {
namespace AudioStandard {
namespace {
    static constexpr int64_t PLAYBACK_DELAY_STOP_HDI_TIME_NS = 3000000000; // 3s = 3 * 1000 * 1000 * 1000ns
    static constexpr int64_t RECORDER_DELAY_STOP_HDI_TIME_NS = 200000000; // 200ms = 200 * 1000 * 1000ns
    static constexpr int64_t DELAY_STOP_HDI_TIME_FOR_ZERO_VOLUME_NS = 4000000000; // 4s = 4 * 1000 * 1000 * 1000ns
}
void AudioEndpointInner::InitLatencyMeasurement()
{
    if (!AudioLatencyMeasurement::CheckIfEnabled()) {
        return;
    }
    signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    CHECK_AND_RETURN_LOG(signalDetectAgent_ != nullptr, "LatencyMeas signalDetectAgent_ is nullptr!");
    signalDetectAgent_->sampleFormat_ = SAMPLE_S16LE;
    signalDetectAgent_->formatByteSize_ = GetFormatByteSize(SAMPLE_S16LE);
    latencyMeasEnabled_ = true;
    signalDetected_ = false;
}

void AudioEndpointInner::DeinitLatencyMeasurement()
{
    signalDetectAgent_ = nullptr;
    latencyMeasEnabled_ = false;
}

void AudioEndpointInner::CheckPlaySignal(uint8_t *buffer, size_t bufferSize)
{
    if (!latencyMeasEnabled_) {
        return;
    }
    CHECK_AND_RETURN_LOG(signalDetectAgent_ != nullptr, "LatencyMeas signalDetectAgent_ is nullptr!");
    size_t byteSize = static_cast<size_t>(GetFormatByteSize(dstStreamInfo_.format));
    size_t newlyCheckedTime = bufferSize / (dstStreamInfo_.samplingRate /
        MILLISECOND_PER_SECOND) / (byteSize * sizeof(uint8_t) * dstStreamInfo_.channels);
    detectedTime_ += newlyCheckedTime;
    if (detectedTime_ >= MILLISECOND_PER_SECOND && signalDetectAgent_->signalDetected_ &&
        !signalDetectAgent_->dspTimestampGot_) {
            AudioParamKey key = NONE;
            std::string condition = "debug_audio_latency_measurement";
            std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(fastRenderId_);
            CHECK_AND_RETURN_LOG(sink != nullptr, "sink is nullptr!");
            std::string dspTime = sink->GetAudioParameter(key, condition);
            LatencyMonitor::GetInstance().UpdateDspTime(dspTime);
            LatencyMonitor::GetInstance().UpdateSinkOrSourceTime(true,
                signalDetectAgent_->lastPeakBufferTime_);
            AUDIO_INFO_LOG("LatencyMeas fastSink signal detected!");
            LatencyMonitor::GetInstance().ShowTimestamp(true);
            signalDetectAgent_->dspTimestampGot_ = true;
            signalDetectAgent_->signalDetected_ = false;
    }
    signalDetected_ = signalDetectAgent_->CheckAudioData(buffer, bufferSize);
    if (signalDetected_) {
        AUDIO_INFO_LOG("LatencyMeas fastSink signal detected!");
        detectedTime_ = 0;
    }
}

void AudioEndpointInner::CheckRecordSignal(uint8_t *buffer, size_t bufferSize)
{
    if (!latencyMeasEnabled_) {
        return;
    }
    CHECK_AND_RETURN_LOG(signalDetectAgent_ != nullptr, "LatencyMeas signalDetectAgent_ is nullptr!");
    signalDetected_ = signalDetectAgent_->CheckAudioData(buffer, bufferSize);
    if (signalDetected_) {
        AudioParamKey key = NONE;
        std::string condition = "debug_audio_latency_measurement";
        std::shared_ptr<IAudioCaptureSource> source = HdiAdapterManager::GetInstance().GetCaptureSource(fastCaptureId_);
        CHECK_AND_RETURN_LOG(source != nullptr, "source is nullptr!");
        std::string dspTime = source->GetAudioParameter(key, condition);
        LatencyMonitor::GetInstance().UpdateSinkOrSourceTime(false,
            signalDetectAgent_->lastPeakBufferTime_);
        LatencyMonitor::GetInstance().UpdateDspTime(dspTime);
        AUDIO_INFO_LOG("LatencyMeas fastSource signal detected");
        signalDetected_ = false;
    }
}

void AudioEndpointInner::ZeroVolumeCheck(const int32_t vol)
{
    if (fastSinkType_ == FAST_SINK_TYPE_BLUETOOTH) {
        return;
    }
    if (std::abs(vol - 0) <= std::numeric_limits<float>::epsilon()) {
        if (zeroVolumeState_ == INACTIVE) {
            zeroVolumeStartTime_ = ClockTime::GetCurNano();
            zeroVolumeState_ = IN_TIMING;
            AUDIO_INFO_LOG("zero volume, will stop fastSink in 4s.");
            return;
        }

        if (zeroVolumeState_ == IN_TIMING &&
            ClockTime::GetCurNano() - zeroVolumeStartTime_ > DELAY_STOP_HDI_TIME_FOR_ZERO_VOLUME_NS) {
            zeroVolumeState_ = ACTIVE;
            HandleZeroVolumeStopEvent();
            AudioPerformanceMonitor::GetInstance().DeleteOvertimeMonitor(adapterType_);
        }
    } else {
        if (zeroVolumeState_ == INACTIVE) {
            return;
        }
        if (zeroVolumeState_ == ACTIVE) {
            HandleZeroVolumeStartEvent();
        }
        ResetZeroVolumeState();
    }
}

void AudioEndpointInner::HandleZeroVolumeStartEvent()
{
    if (!isStarted_) {
        std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(fastRenderId_);
        if (sink == nullptr || sink->Start() != SUCCESS) {
            AUDIO_INFO_LOG("Volume from zero to none-zero, start fastSink failed.");
            isStarted_ = false;
        } else {
            AUDIO_INFO_LOG("Volume from zero to none-zero, start fastSink success.");
            isStarted_ = true;
            needReSyncPosition_ = true;
        }
    } else {
        AUDIO_INFO_LOG("fastSink already started");
    }
}

void AudioEndpointInner::HandleZeroVolumeStopEvent()
{
    if (isStarted_) {
        std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(fastRenderId_);
        if (sink != nullptr && sink->Stop() == SUCCESS) {
            AUDIO_INFO_LOG("Volume from none-zero to zero more than 4s, stop fastSink success.");
            isStarted_ = false;
        } else {
            AUDIO_INFO_LOG("Volume from none-zero to zero more than 4s, stop fastSink failed.");
            isStarted_ = true;
        }
    } else {
        AUDIO_INFO_LOG("fastSink already stopped");
    }
}

void AudioEndpointInner::ResetZeroVolumeState()
{
    zeroVolumeStartTime_ = INT64_MAX;
    zeroVolumeState_ = INACTIVE;
}

void AudioEndpointInner::CheckStandBy()
{
    if (endpointStatus_ == RUNNING) {
        endpointStatus_ = IsAnyProcessRunning() ? RUNNING : IDEL;
    }

    if (endpointStatus_ == RUNNING) {
        return;
    }

    AUDIO_INFO_LOG("endpoint status:%{public}s", GetStatusStr(endpointStatus_).c_str());
    if (endpointStatus_ == IDEL) {
        // delay call sink stop when no process running
        AUDIO_INFO_LOG("status is IDEL, need delay call stop");
        AudioMode audioMode = GetAudioMode();
        delayStopTime_ = ClockTime::GetCurNano() + ((audioMode == AUDIO_MODE_PLAYBACK)
            ? PLAYBACK_DELAY_STOP_HDI_TIME_NS : RECORDER_DELAY_STOP_HDI_TIME_NS);
    }
}

void AudioEndpointInner::Dump(std::string &dumpString)
{
    // dump endpoint stream info
    dumpString += "Endpoint stream info:\n";
    AppendFormat(dumpString, "  - samplingRate: %d\n", dstStreamInfo_.samplingRate);
    AppendFormat(dumpString, "  - channels: %u\n", dstStreamInfo_.channels);
    AppendFormat(dumpString, "  - format: %u\n", dstStreamInfo_.format);
    AppendFormat(dumpString, "  - sink type: %d\n", fastSinkType_);
    AppendFormat(dumpString, "  - source type: %d\n", fastSourceType_);

    // dump status info
    AppendFormat(dumpString, "  - Current endpoint status: %s\n", GetStatusStr(endpointStatus_).c_str());
    if (dstAudioBuffer_ != nullptr) {
        AppendFormat(dumpString, "  - Currend hdi read position: %u\n", dstAudioBuffer_->GetCurReadFrame());
        AppendFormat(dumpString, "  - Currend hdi write position: %u\n", dstAudioBuffer_->GetCurWriteFrame());
    }

    // dump linked process info
    std::lock_guard<std::mutex> lock(listLock_);
    AppendFormat(dumpString, "  - linked process:: %zu\n", processBufferList_.size());
    for (auto item : processBufferList_) {
        AppendFormat(dumpString, "  - process read position: %u\n", item->GetCurReadFrame());
        AppendFormat(dumpString, "  - process write position: %u\n", item->GetCurWriteFrame());
    }
    dumpString += "\n";
}

} // namespace AudioStandard
} // namespace OHOS
