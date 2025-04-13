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
#define LOG_TAG "CapturerInServer"
#endif

#include "capturer_in_server.h"
#include <cinttypes>
#include "securec.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "audio_capturer_log.h"
#include "audio_service.h"
#include "audio_process_config.h"
#include "i_stream_manager.h"
#ifdef HAS_FEATURE_INNERCAPTURER
#include "playback_capturer_manager.h"
#endif
#include "policy_handler.h"
#include "media_monitor_manager.h"
#include "audio_dump_pcm.h"
#include "volume_tools.h"

namespace OHOS {
namespace AudioStandard {
namespace {
    static constexpr int32_t VOLUME_SHIFT_NUMBER = 16; // 1 >> 16 = 65536, max volume
    static const size_t CAPTURER_BUFFER_DEFAULT_NUM = 4;
    static const size_t CAPTURER_BUFFER_WAKE_UP_NUM = 100;
    static const uint32_t OVERFLOW_LOG_LOOP_COUNT = 100;
}

CapturerInServer::CapturerInServer(AudioProcessConfig processConfig, std::weak_ptr<IStreamListener> streamListener)
{
    processConfig_ = processConfig;
    streamListener_ = streamListener;
    innerCapId_ = processConfig.innerCapId;
}

CapturerInServer::~CapturerInServer()
{
    if (status_ != I_STATUS_RELEASED) {
        Release();
    }
    DumpFileUtil::CloseDumpFile(&dumpS2C_);
    if (needCheckBackground_) {
        TurnOffMicIndicator(CAPTURER_INVALID);
    }
}

int32_t CapturerInServer::ConfigServerBuffer()
{
    if (audioServerBuffer_ != nullptr) {
        AUDIO_INFO_LOG("ConfigProcessBuffer: process buffer already configed!");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(stream_ != nullptr, ERR_OPERATION_FAILED, "ConfigServerBuffer failed, stream_ is null");
    stream_->GetSpanSizePerFrame(spanSizeInFrame_);
    const size_t bufferNum = ((processConfig_.capturerInfo.sourceType == SOURCE_TYPE_WAKEUP)
        ? CAPTURER_BUFFER_WAKE_UP_NUM : CAPTURER_BUFFER_DEFAULT_NUM);
    totalSizeInFrame_ = spanSizeInFrame_ * bufferNum;
    stream_->GetByteSizePerFrame(byteSizePerFrame_);
    spanSizeInBytes_ = byteSizePerFrame_ * spanSizeInFrame_;
    AUDIO_INFO_LOG("ConfigProcessBuffer: totalSizeInFrame_: %{public}zu, spanSizeInFrame_: %{public}zu,"
        "byteSizePerFrame_: %{public}zu, spanSizeInBytes_ %{public}zu", totalSizeInFrame_, spanSizeInFrame_,
        byteSizePerFrame_, spanSizeInBytes_);
    if (totalSizeInFrame_ == 0 || spanSizeInFrame_ == 0 || totalSizeInFrame_ % spanSizeInFrame_ != 0) {
        AUDIO_ERR_LOG("ConfigProcessBuffer: ERR_INVALID_PARAM");
        return ERR_INVALID_PARAM;
    }

    int32_t ret = InitCacheBuffer(2 * spanSizeInBytes_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "InitCacheBuffer failed %{public}d", ret);

    // create OHAudioBuffer in server
    audioServerBuffer_ = OHAudioBuffer::CreateFromLocal(totalSizeInFrame_, spanSizeInFrame_, byteSizePerFrame_);
    CHECK_AND_RETURN_RET_LOG(audioServerBuffer_ != nullptr, ERR_OPERATION_FAILED, "Create oh audio buffer failed");

    // we need to clear data buffer to avoid dirty data.
    memset_s(audioServerBuffer_->GetDataBase(), audioServerBuffer_->GetDataSize(), 0,
        audioServerBuffer_->GetDataSize());
    ret = InitBufferStatus();
    AUDIO_DEBUG_LOG("Clear data buffer, ret:%{public}d", ret);
    isBufferConfiged_ = true;
    isInited_ = true;
    return SUCCESS;
}

int32_t CapturerInServer::InitBufferStatus()
{
    if (audioServerBuffer_ == nullptr) {
        AUDIO_ERR_LOG("InitBufferStatus failed, null buffer.");
        return ERR_ILLEGAL_STATE;
    }

    uint32_t spanCount = audioServerBuffer_->GetSpanCount();
    AUDIO_INFO_LOG("InitBufferStatus: spanCount %{public}u", spanCount);
    for (uint32_t i = 0; i < spanCount; i++) {
        SpanInfo *spanInfo = audioServerBuffer_->GetSpanInfoByIndex(i);
        if (spanInfo == nullptr) {
            AUDIO_ERR_LOG("InitBufferStatus failed, null spaninfo");
            return ERR_ILLEGAL_STATE;
        }
        spanInfo->spanStatus = SPAN_READ_DONE;
        spanInfo->offsetInFrame = 0;

        spanInfo->readStartTime = 0;
        spanInfo->readDoneTime = 0;

        spanInfo->readStartTime = 0;
        spanInfo->readDoneTime = 0;

        spanInfo->volumeStart = 1 << VOLUME_SHIFT_NUMBER; // 65536 for initialize
        spanInfo->volumeEnd = 1 << VOLUME_SHIFT_NUMBER; // 65536 for initialize
        spanInfo->isMute = false;
    }
    return SUCCESS;
}

int32_t CapturerInServer::Init()
{
    int32_t ret = IStreamManager::GetRecorderManager().CreateCapturer(processConfig_, stream_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS && stream_ != nullptr, ERR_OPERATION_FAILED,
        "Construct CapturerInServer failed: %{public}d", ret);
    streamIndex_ = stream_->GetStreamIndex();
    ret = ConfigServerBuffer();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "ConfigServerBuffer failed: %{public}d", ret);
    stream_->RegisterStatusCallback(shared_from_this());
    stream_->RegisterReadCallback(shared_from_this());

    traceTag_ = "[" + std::to_string(streamIndex_) + "]CapturerInServer"; // [100001]CapturerInServer
    // eg: /data/data/.pulse_dir/10000_100009_capturer_server_out_48000_2_1.pcm
    AudioStreamInfo tempInfo = processConfig_.streamInfo;
    dumpFileName_ = std::to_string(processConfig_.appInfo.appPid) + "_" + std::to_string(streamIndex_)
        + "_capturer_server_out_" + std::to_string(tempInfo.samplingRate) + "_"
        + std::to_string(tempInfo.channels) + "_" + std::to_string(tempInfo.format) + ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileName_, &dumpS2C_);
    recorderDfx_ = std::make_unique<RecorderDfxWriter>(processConfig_.appInfo, streamIndex_);

    return SUCCESS;
}

void CapturerInServer::OnStatusUpdate(IOperation operation)
{
    AUDIO_INFO_LOG("CapturerInServer::OnStatusUpdate operation: %{public}d", operation);
    operation_ = operation;
    if (status_ == I_STATUS_RELEASED) {
        AUDIO_WARNING_LOG("Stream already released");
        return;
    }
    std::shared_ptr<IStreamListener> stateListener = streamListener_.lock();
    CHECK_AND_RETURN_LOG((stateListener != nullptr && recorderDfx_ != nullptr), "IStreamListener is nullptr");
    switch (operation) {
        case OPERATION_UNDERFLOW:
            underflowCount += 1;
            AUDIO_INFO_LOG("Underflow!! underflow count %{public}d", underflowCount);
            stateListener->OnOperationHandled(BUFFER_OVERFLOW, underflowCount);
            break;
        case OPERATION_STARTED:
            status_ = I_STATUS_STARTED;
            lastStartTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            stateListener->OnOperationHandled(START_STREAM, 0);
            break;
        case OPERATION_PAUSED:
            status_ = I_STATUS_PAUSED;
            stateListener->OnOperationHandled(PAUSE_STREAM, 0);
            recorderDfx_->WriteDfxActionMsg(streamIndex_, CAPTURER_STAGE_PAUSE_OK);
            break;
        case OPERATION_STOPPED:
            status_ = I_STATUS_STOPPED;
            stateListener->OnOperationHandled(STOP_STREAM, 0);
            lastStopTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            recorderDfx_->WriteDfxStopMsg(streamIndex_, CAPTURER_STAGE_STOP_OK,
                GetLastAudioDuration(), processConfig_);
            break;
        case OPERATION_FLUSHED:
            HandleOperationFlushed();
            stateListener->OnOperationHandled(FLUSH_STREAM, 0);
            break;
        default:
            AUDIO_INFO_LOG("Invalid operation %{public}u", operation);
            status_ = I_STATUS_INVALID;
    }
}

void CapturerInServer::HandleOperationFlushed()
{
    if (status_ == I_STATUS_FLUSHING_WHEN_STARTED) {
        status_ = I_STATUS_STARTED;
    } else if (status_ == I_STATUS_FLUSHING_WHEN_PAUSED) {
        status_ = I_STATUS_PAUSED;
    } else if (status_ == I_STATUS_FLUSHING_WHEN_STOPPED) {
        status_ = I_STATUS_STOPPED;
    } else {
        AUDIO_WARNING_LOG("Invalid status before flusing");
    }
}

BufferDesc CapturerInServer::DequeueBuffer(size_t length)
{
    return stream_->DequeueBuffer(length);
}

bool CapturerInServer::IsReadDataOverFlow(size_t length, uint64_t currentWriteFrame,
    std::shared_ptr<IStreamListener> stateListener)
{
    if (audioServerBuffer_->GetAvailableDataFrames() <= static_cast<int32_t>(spanSizeInFrame_)) {
        if (overFlowLogFlag_ == 0) {
            AUDIO_INFO_LOG("OverFlow!!!");
        } else if (overFlowLogFlag_ == OVERFLOW_LOG_LOOP_COUNT) {
            overFlowLogFlag_ = 0;
        }
        overFlowLogFlag_++;
        BufferDesc dstBuffer = stream_->DequeueBuffer(length);
        stream_->EnqueueBuffer(dstBuffer);
        stateListener->OnOperationHandled(UPDATE_STREAM, currentWriteFrame);
        return true;
    }
    return false;
}

void CapturerInServer::ReadData(size_t length)
{
    CHECK_AND_RETURN_LOG(length >= spanSizeInBytes_,
        "Length %{public}zu is less than spanSizeInBytes %{public}zu", length, spanSizeInBytes_);
    std::shared_ptr<IStreamListener> stateListener = streamListener_.lock();
    CHECK_AND_RETURN_LOG(stateListener != nullptr, "IStreamListener is nullptr");
    CHECK_AND_RETURN_LOG(stream_ != nullptr, "ReadData failed, stream_ is null");

    uint64_t currentWriteFrame = audioServerBuffer_->GetCurWriteFrame();
    if (IsReadDataOverFlow(length, currentWriteFrame, stateListener)) {
        return;
    }
    Trace trace(traceTag_ + "::ReadData:" + std::to_string(currentWriteFrame));
    OptResult result = ringCache_->GetWritableSize();
    CHECK_AND_RETURN_LOG(result.ret == OPERATION_SUCCESS, "RingCache write invalid size %{public}zu", result.size);
    BufferDesc srcBuffer = stream_->DequeueBuffer(result.size);
    ringCache_->Enqueue({srcBuffer.buffer, srcBuffer.bufLength});
    result = ringCache_->GetReadableSize();
    if (result.ret != OPERATION_SUCCESS || result.size < spanSizeInBytes_) {
        stream_->EnqueueBuffer(srcBuffer);
        return;
    }

    BufferDesc dstBuffer = {nullptr, 0, 0};
    uint64_t curWritePos = audioServerBuffer_->GetCurWriteFrame();
    if (audioServerBuffer_->GetWriteBuffer(curWritePos, dstBuffer) < 0) {
        return;
    }
    if ((processConfig_.capturerInfo.sourceType == SOURCE_TYPE_PLAYBACK_CAPTURE && processConfig_.innerCapMode ==
        LEGACY_MUTE_CAP) || muteFlag_) {
        dstBuffer.buffer = dischargeBuffer_.get(); // discharge valid data.
    }
    if (muteFlag_) {
        memset_s(static_cast<void *>(dstBuffer.buffer), dstBuffer.bufLength, 0, dstBuffer.bufLength);
    }
    ringCache_->Dequeue({dstBuffer.buffer, dstBuffer.bufLength});
    VolumeTools::DfxOperation(dstBuffer, processConfig_.streamInfo, traceTag_, volumeDataCount_);
    if (AudioDump::GetInstance().GetVersionType() == DumpFileUtil::BETA_VERSION) {
        DumpFileUtil::WriteDumpFile(dumpS2C_, static_cast<void *>(dstBuffer.buffer), dstBuffer.bufLength);
        AudioCacheMgr::GetInstance().CacheData(dumpFileName_,
            static_cast<void *>(dstBuffer.buffer), dstBuffer.bufLength);
    }

    uint64_t nextWriteFrame = currentWriteFrame + spanSizeInFrame_;
    audioServerBuffer_->SetCurWriteFrame(nextWriteFrame);
    audioServerBuffer_->SetHandleInfo(currentWriteFrame, ClockTime::GetCurNano());

    stream_->EnqueueBuffer(srcBuffer);
    stateListener->OnOperationHandled(UPDATE_STREAM, currentWriteFrame);
}

int32_t CapturerInServer::OnReadData(size_t length)
{
    Trace trace(traceTag_ + "::OnReadData:" + std::to_string(length));
    ReadData(length);
    return SUCCESS;
}

int32_t CapturerInServer::UpdateReadIndex()
{
    AUDIO_DEBUG_LOG("audioServerBuffer_->GetAvailableDataFrames(): %{public}d, needStart: %{public}d",
        audioServerBuffer_->GetAvailableDataFrames(), needStart);
    return SUCCESS;
}

int32_t CapturerInServer::ResolveBuffer(std::shared_ptr<OHAudioBuffer> &buffer)
{
    buffer = audioServerBuffer_;
    return SUCCESS;
}

int32_t CapturerInServer::GetSessionId(uint32_t &sessionId)
{
    CHECK_AND_RETURN_RET_LOG(stream_ != nullptr, ERR_OPERATION_FAILED, "GetSessionId failed, stream_ is null");
    sessionId = streamIndex_;
    CHECK_AND_RETURN_RET_LOG(sessionId < INT32_MAX, ERR_OPERATION_FAILED, "GetSessionId failed, sessionId:%{public}d",
        sessionId);

    return SUCCESS;
}

bool CapturerInServer::TurnOnMicIndicator(CapturerState capturerState)
{
    uint32_t tokenId = processConfig_.appInfo.appTokenId;
    uint64_t fullTokenId = processConfig_.appInfo.appFullTokenId;
    SwitchStreamInfo info = {
        streamIndex_,
        processConfig_.callerUid,
        processConfig_.appInfo.appUid,
        processConfig_.appInfo.appPid,
        tokenId,
        capturerState,
    };
    if (!SwitchStreamUtil::IsSwitchStreamSwitching(info, SWITCH_STATE_STARTED)) {
        CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyBackgroundCapture(tokenId, fullTokenId),
            false, "VerifyBackgroundCapture failed!");
    }
    SwitchStreamUtil::UpdateSwitchStreamRecord(info, SWITCH_STATE_STARTED);

    if (isMicIndicatorOn_) {
        AUDIO_WARNING_LOG("MicIndicator of stream:%{public}d is already on."
            "No need to call NotifyPrivacyStart!", streamIndex_);
    } else {
        CHECK_AND_RETURN_RET_LOG(PermissionUtil::NotifyPrivacyStart(tokenId, streamIndex_),
            false, "NotifyPrivacyStart failed!");
        AUDIO_INFO_LOG("Turn on micIndicator of stream:%{public}d from off"
            "after NotifyPrivacyStart success!", streamIndex_);
        isMicIndicatorOn_ = true;
    }
    return true;
}

bool CapturerInServer::TurnOffMicIndicator(CapturerState capturerState)
{
    uint32_t tokenId = processConfig_.appInfo.appTokenId;
    SwitchStreamInfo info = {
        streamIndex_,
        processConfig_.callerUid,
        processConfig_.appInfo.appUid,
        processConfig_.appInfo.appPid,
        tokenId,
        capturerState,
    };
    SwitchStreamUtil::UpdateSwitchStreamRecord(info, SWITCH_STATE_FINISHED);

    if (isMicIndicatorOn_) {
        PermissionUtil::NotifyPrivacyStop(tokenId, streamIndex_);
        AUDIO_INFO_LOG("Turn off micIndicator of stream:%{public}d from on after NotifyPrivacyStop!", streamIndex_);
        isMicIndicatorOn_ = false;
    } else {
        AUDIO_WARNING_LOG("MicIndicator of stream:%{public}d is already off."
            "No need to call NotifyPrivacyStop!", streamIndex_);
    }
    return true;
}

int32_t CapturerInServer::Start()
{
    int32_t ret = StartInner();
    CapturerStage stage = ret == SUCCESS ? CAPTURER_STAGE_START_OK : CAPTURER_STAGE_START_FAIL;
    if (recorderDfx_) {
        recorderDfx_->WriteDfxStartMsg(streamIndex_, stage, processConfig_);
    }
    return ret;
}

int32_t CapturerInServer::StartInner()
{
    needStart = 0;
    std::unique_lock<std::mutex> lock(statusLock_);

    if (status_ != I_STATUS_IDLE && status_ != I_STATUS_PAUSED && status_ != I_STATUS_STOPPED) {
        AUDIO_ERR_LOG("CapturerInServer::Start failed, Illegal state: %{public}u", status_);
        return ERR_ILLEGAL_STATE;
    }

    if (!needCheckBackground_ && PermissionUtil::NeedVerifyBackgroundCapture(processConfig_.callerUid,
        processConfig_.capturerInfo.sourceType)) {
        AUDIO_INFO_LOG("set needCheckBackground_: true");
        needCheckBackground_ = true;
    }
    if (needCheckBackground_) {
        CHECK_AND_RETURN_RET_LOG(TurnOnMicIndicator(CAPTURER_RUNNING), ERR_PERMISSION_DENIED,
            "Turn on micIndicator failed or check backgroud capture failed for stream:%{public}d!", streamIndex_);
    }

    AudioService::GetInstance()->UpdateSourceType(processConfig_.capturerInfo.sourceType);

    status_ = I_STATUS_STARTING;
    int ret = stream_->Start();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Start stream failed, reason: %{public}d", ret);
    resetTime_ = true;
    return SUCCESS;
}

int32_t CapturerInServer::Pause()
{
    std::unique_lock<std::mutex> lock(statusLock_);
    if (status_ != I_STATUS_STARTED) {
        AUDIO_ERR_LOG("CapturerInServer::Pause failed, Illegal state: %{public}u", status_);
        return ERR_ILLEGAL_STATE;
    }
    if (needCheckBackground_) {
        TurnOffMicIndicator(CAPTURER_PAUSED);
    }
    status_ = I_STATUS_PAUSING;
    int ret = stream_->Pause();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Pause stream failed, reason: %{public}d", ret);
    return SUCCESS;
}

int32_t CapturerInServer::Flush()
{
    std::unique_lock<std::mutex> lock(statusLock_);
    if (status_ == I_STATUS_STARTED) {
        status_ = I_STATUS_FLUSHING_WHEN_STARTED;
    } else if (status_ == I_STATUS_PAUSED) {
        status_ = I_STATUS_FLUSHING_WHEN_PAUSED;
    } else if (status_ == I_STATUS_STOPPED) {
        status_ = I_STATUS_FLUSHING_WHEN_STOPPED;
    } else {
        AUDIO_ERR_LOG("CapturerInServer::Flush failed, Illegal state: %{public}u", status_);
        return ERR_ILLEGAL_STATE;
    }

    // Flush buffer of audio server
    uint64_t writeFrame = audioServerBuffer_->GetCurWriteFrame();
    uint64_t readFrame = audioServerBuffer_->GetCurReadFrame();

    while (readFrame < writeFrame) {
        BufferDesc bufferDesc = {nullptr, 0, 0};
        int32_t readResult = audioServerBuffer_->GetReadbuffer(readFrame, bufferDesc);
        if (readResult != 0) {
            return ERR_OPERATION_FAILED;
        }
        memset_s(bufferDesc.buffer, bufferDesc.bufLength, 0, bufferDesc.bufLength);
        readFrame += spanSizeInFrame_;
        AUDIO_INFO_LOG("On flush, write frame: %{public}" PRIu64 ", nextReadFrame: %{public}zu,"
            "readFrame: %{public}" PRIu64 "", writeFrame, spanSizeInFrame_, readFrame);
        audioServerBuffer_->SetCurReadFrame(readFrame);
    }

    int ret = stream_->Flush();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Flush stream failed, reason: %{public}d", ret);
    return SUCCESS;
}

int32_t CapturerInServer::DrainAudioBuffer()
{
    return SUCCESS;
}

int32_t CapturerInServer::Stop()
{
    std::unique_lock<std::mutex> lock(statusLock_);
    if (status_ != I_STATUS_STARTED && status_ != I_STATUS_PAUSED) {
        AUDIO_ERR_LOG("CapturerInServer::Stop failed, Illegal state: %{public}u", status_);
        return ERR_ILLEGAL_STATE;
    }
    status_ = I_STATUS_STOPPING;

    if (needCheckBackground_) {
        TurnOffMicIndicator(CAPTURER_STOPPED);
    }

    int ret = stream_->Stop();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Stop stream failed, reason: %{public}d", ret);
    return SUCCESS;
}

int32_t CapturerInServer::Release()
{
    AudioService::GetInstance()->RemoveCapturer(streamIndex_);
    {
        std::unique_lock<std::mutex> lock(statusLock_);
        if (status_ == I_STATUS_RELEASED) {
            AUDIO_INFO_LOG("Already released");
            return SUCCESS;
        }
    }
    AUDIO_INFO_LOG("Start release capturer");
    int32_t ret = IStreamManager::GetRecorderManager().ReleaseCapturer(streamIndex_);
    if (ret < 0) {
        AUDIO_ERR_LOG("Release stream failed, reason: %{public}d", ret);
        status_ = I_STATUS_INVALID;
        return ret;
    }
    status_ = I_STATUS_RELEASED;
#ifdef HAS_FEATURE_INNERCAPTURER
    if (processConfig_.capturerInfo.sourceType == SOURCE_TYPE_PLAYBACK_CAPTURE) {
        AUDIO_INFO_LOG("Disable inner capturer for %{public}u， innerCapId :%{public}d, innerCapMode:%{public}d",
            streamIndex_, innerCapId_, processConfig_.innerCapMode);
        if (processConfig_.innerCapMode == MODERN_INNER_CAP) {
            PlaybackCapturerManager::GetInstance()->RemovePlaybackCapturerFilterInfo(streamIndex_, innerCapId_);
        } else {
            PlaybackCapturerManager::GetInstance()->SetInnerCapturerState(false);
        }
        if (PlaybackCapturerManager::GetInstance()->CheckReleaseUnloadModernInnerCapSink(innerCapId_)) {
            AudioService::GetInstance()->UnloadModernInnerCapSink(innerCapId_);
        }
        innerCapId_ = 0;
    }
#endif
    if (needCheckBackground_) {
        TurnOffMicIndicator(CAPTURER_RELEASED);
    }
    return SUCCESS;
}

#ifdef HAS_FEATURE_INNERCAPTURER
int32_t CapturerInServer::UpdatePlaybackCaptureConfigInLegacy(const AudioPlaybackCaptureConfig &config)
{
    Trace trace("UpdatePlaybackCaptureConfigInLegacy");
    // Legacy mode, only usage filter works.
    AUDIO_INFO_LOG("Update config in legacy mode with %{public}zu usage", config.filterOptions.usages.size());

    std::vector<int32_t> usage;
    for (size_t i = 0; i < config.filterOptions.usages.size(); i++) {
        usage.push_back(config.filterOptions.usages[i]);
    }

    PlaybackCapturerManager::GetInstance()->SetSupportStreamUsage(usage);
    PlaybackCapturerManager::GetInstance()->SetInnerCapturerState(true);
    return SUCCESS;
}

int32_t CapturerInServer::UpdatePlaybackCaptureConfig(const AudioPlaybackCaptureConfig &config)
{
    Trace trace("UpdatePlaybackCaptureConfig:" + ProcessConfig::DumpInnerCapConfig(config));
    CHECK_AND_RETURN_RET_LOG(processConfig_.capturerInfo.sourceType == SOURCE_TYPE_PLAYBACK_CAPTURE,
        ERR_INVALID_OPERATION, "This not a inner-cap source!");

    AUDIO_INFO_LOG("Client using config: %{public}s", ProcessConfig::DumpInnerCapConfig(config).c_str());

    for (auto &usg : config.filterOptions.usages) {
        if (usg != STREAM_USAGE_VOICE_COMMUNICATION) {
            continue;
        }

        if (!PermissionUtil::VerifyPermission(CAPTURER_VOICE_DOWNLINK_PERMISSION, processConfig_.appInfo.appTokenId)) {
            AUDIO_ERR_LOG("downlink capturer permission check failed");
            return ERR_PERMISSION_DENIED;
        }
    }
    filterConfig_ = config;

    if (filterConfig_.filterOptions.usages.size() == 0) {
        std::vector<StreamUsage> defalutUsages = PlaybackCapturerManager::GetInstance()->GetDefaultUsages();
        for (size_t i = 0; i < defalutUsages.size(); i++) {
            filterConfig_.filterOptions.usages.push_back(defalutUsages[i]);
        }
        AUDIO_INFO_LOG("Reset config to %{public}s", ProcessConfig::DumpInnerCapConfig(filterConfig_).c_str());
    }

    if (processConfig_.innerCapMode != MODERN_INNER_CAP) {
        return UpdatePlaybackCaptureConfigInLegacy(filterConfig_);
    }

    // in plan: add more check and print config
    PlaybackCapturerManager::GetInstance()->SetPlaybackCapturerFilterInfo(streamIndex_, filterConfig_, innerCapId_);
    return SUCCESS;
}
#endif

int32_t CapturerInServer::GetAudioTime(uint64_t &framePos, uint64_t &timestamp)
{
    if (status_ == I_STATUS_STOPPED) {
        AUDIO_WARNING_LOG("Current status is stopped");
        return ERR_ILLEGAL_STATE;
    }

    CHECK_AND_RETURN_RET_LOG(stream_ != nullptr, ERR_OPERATION_FAILED, "GetAudioTime failed, stream_ is null");
    stream_->GetStreamFramesRead(framePos);
    stream_->GetCurrentTimeStamp(timestamp);
    if (resetTime_) {
        resetTime_ = false;
        resetTimestamp_ = timestamp;
    }
    return SUCCESS;
}

int32_t CapturerInServer::GetLatency(uint64_t &latency)
{
    CHECK_AND_RETURN_RET_LOG(stream_ != nullptr, ERR_OPERATION_FAILED, "GetLatency failed, stream_ is null");
    return stream_->GetLatency(latency);
}

int32_t CapturerInServer::InitCacheBuffer(size_t targetSize)
{
    CHECK_AND_RETURN_RET_LOG(spanSizeInBytes_ != 0, ERR_OPERATION_FAILED, "spanSizeInByte_ invalid");

    AUDIO_INFO_LOG("old size:%{public}zu, new size:%{public}zu", cacheSizeInBytes_, targetSize);
    cacheSizeInBytes_ = targetSize;

    if (ringCache_ == nullptr) {
        ringCache_ = AudioRingCache::Create(cacheSizeInBytes_);
    } else {
        OptResult result = ringCache_->ReConfig(cacheSizeInBytes_, false); // false --> clear buffer
        if (result.ret != OPERATION_SUCCESS) {
            AUDIO_ERR_LOG("ReConfig AudioRingCache to size %{public}u failed:ret%{public}zu", result.ret, targetSize);
            return ERR_OPERATION_FAILED;
        }
    }

    if (processConfig_.capturerInfo.sourceType == SOURCE_TYPE_PLAYBACK_CAPTURE && processConfig_.innerCapMode ==
        LEGACY_MUTE_CAP) {
        dischargeBuffer_ = std::make_unique<uint8_t []>(cacheSizeInBytes_);
    }

    return SUCCESS;
}

void CapturerInServer::SetNonInterruptMute(const bool muteFlag)
{
    AUDIO_INFO_LOG("muteFlag: %{public}d", muteFlag);
    muteFlag_ = muteFlag;
    AudioService::GetInstance()->UpdateMuteControlSet(streamIndex_, muteFlag);
}

RestoreStatus CapturerInServer::RestoreSession(RestoreInfo restoreInfo)
{
    RestoreStatus restoreStatus = audioServerBuffer_->SetRestoreStatus(NEED_RESTORE);
    if (restoreStatus == NEED_RESTORE) {
        audioServerBuffer_->SetRestoreInfo(restoreInfo);
    }
    return restoreStatus;
}

int64_t CapturerInServer::GetLastAudioDuration()
{
    auto ret = lastStopTime_ - lastStartTime_;
    return ret < 0 ? -1 : ret;
}

} // namespace AudioStandard
} // namespace OHOS
