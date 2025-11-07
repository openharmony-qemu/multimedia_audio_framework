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
#define LOG_TAG "AudioProcessInServer"
#endif

#include "audio_process_in_server.h"
#include "policy_handler.h"

#include "securec.h"
#include "xperf_adapter.h"
#include "iprocess_cb.h"
#include "audio_errors.h"
#include "audio_capturer_log.h"
#include "audio_service.h"
#include "audio_schedule.h"
#include "audio_utils.h"
#include "media_monitor_manager.h"
#include "audio_dump_pcm.h"
#include "audio_performance_monitor.h"
#include "core_service_handler.h"
#include "stream_dfx_manager.h"
#include "audio_stream_concurrency_detector.h"
#include "format_converter.h"
#ifdef RESSCHE_ENABLE
#include "res_type.h"
#include "res_sched_client.h"
#endif

namespace OHOS {
namespace AudioStandard {
namespace {
}

sptr<AudioProcessInServer> AudioProcessInServer::Create(const AudioProcessConfig &processConfig,
    ProcessReleaseCallback *releaseCallback)
{
    sptr<AudioProcessInServer> process = new(std::nothrow) AudioProcessInServer(processConfig, releaseCallback);
    return process;
}

void AudioProcessInServer::UpdateStreamInfo()
{
    CHECK_AND_RETURN_LOG(checkCount_ <= audioCheckFreq_, "the stream had been already checked");

    if ((audioCheckFreq_ == checkCount_) || (checkCount_ == 0)) {
        AudioStreamConcurrencyDetector::GetInstance().UpdateWriteTime(processConfig_, sessionId_);
    }

    checkCount_++;
}

void AudioProcessInServer::RemoveStreamInfo()
{
    AudioStreamConcurrencyDetector::GetInstance().RemoveStream(processConfig_, sessionId_);
    checkCount_ = 0;
}

AudioProcessInServer::AudioProcessInServer(const AudioProcessConfig &processConfig,
    ProcessReleaseCallback *releaseCallback) : processConfig_(processConfig), releaseCallback_(releaseCallback)
{
    if (processConfig.originalSessionId < MIN_STREAMID || processConfig.originalSessionId > MAX_STREAMID) {
        sessionId_ = PolicyHandler::GetInstance().GenerateSessionId(processConfig_.appInfo.appUid);
    } else {
        sessionId_ = processConfig.originalSessionId;
    }

    AudioSamplingRate samplingRate = processConfig_.streamInfo.samplingRate;
    AudioSampleFormat format = processConfig_.streamInfo.format;
    AudioChannel channels = processConfig_.streamInfo.channels;
    // eg: 100005_dump_process_server_audio_48000_2_1.pcm
    dumpFileName_ = std::to_string(sessionId_) + '_' + "_dump_process_server_audio_" +
        std::to_string(samplingRate) + '_' + std::to_string(channels) + '_' + std::to_string(format) + ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileName_, &dumpFile_);
    playerDfx_ = std::make_unique<PlayerDfxWriter>(processConfig_.appInfo, sessionId_);
    recorderDfx_ = std::make_unique<RecorderDfxWriter>(processConfig_.appInfo, sessionId_);
    if (processConfig_.audioMode == AUDIO_MODE_RECORD) {
        AudioService::GetInstance()->RegisterMuteStateChangeCallback(sessionId_, [this](bool flag) {
            AUDIO_INFO_LOG("recv mute state change flag %{public}d", flag ? 1 : 0);
            muteFlag_ = flag;
        });
    }
    audioStreamChecker_ = std::make_shared<AudioStreamChecker>(processConfig);
    AudioStreamMonitor::GetInstance().AddCheckForMonitor(processConfig.originalSessionId, audioStreamChecker_);
    streamStatusInServer_ = STREAM_IDEL;

    dumpFACName_ = std::to_string(sessionId_) + '_' + "_dump_fac_audio_" +
        std::to_string(samplingRate) + '_' + std::to_string(channels) + '_' + std::to_string(format) + ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFACName_, &dumpFAC_);
}

AudioProcessInServer::~AudioProcessInServer()
{
    AUDIO_INFO_LOG("~AudioProcessInServer()");
    if (object_ != nullptr) {
        bool res = object_->RemoveDeathRecipient(deathRecipient_);
        AUDIO_INFO_LOG("RemoveDeathRecipient ret: %{public}d", res);
    }

    if (convertedBuffer_.buffer != nullptr) {
        delete [] convertedBuffer_.buffer;
    }
    DumpFileUtil::CloseDumpFile(&dumpFile_);
    DumpFileUtil::CloseDumpFile(&dumpFAC_);
    if (processConfig_.audioMode == AUDIO_MODE_RECORD && needCheckBackground_) {
        TurnOffMicIndicator(CAPTURER_INVALID);
    }

    NotifyXperfOnPlayback(processConfig_.audioMode, XPERF_EVENT_RELEASE);
    AudioStreamMonitor::GetInstance().DeleteCheckForMonitor(processConfig_.originalSessionId);
}

bool AudioProcessInServer::PrepareRingBuffer(uint64_t curRead, RingBufferWrapper& ringBuffer)
{
    int32_t ret = processBuffer_->GetAllReadableBufferFromPosFrame(curRead, ringBuffer);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS && ringBuffer.dataLength > 0, false,
        "getBuffer failed ret: %{public}d lenth: %{public}zu",
        ret, ringBuffer.dataLength);

    auto byteSizePerFrame = GetByteSizePerFrame();
    CHECK_AND_RETURN_RET_LOG(byteSizePerFrame != 0, false, "byteSizePerFrame is 0");

    size_t spanSizeInByte = GetSpanSizeInFrame() * byteSizePerFrame;
    if (ringBuffer.dataLength > spanSizeInByte) {
        ringBuffer.dataLength = spanSizeInByte;
    }

    UpdateStreamInfo();
    return true;
}

bool AudioProcessInServer::NeedUseTempBuffer(const RingBufferWrapper &ringBuffer, size_t spanSizeInByte)
{
    if (ringBuffer.dataLength > ringBuffer.basicBufferDescs[0].bufLength) {
        return true;
    }

    if (ringBuffer.dataLength < spanSizeInByte) {
        return true;
    }

    return false;
}

void AudioProcessInServer::PrepareStreamDataBuffer(size_t spanSizeInByte,
    RingBufferWrapper &ringBuffer, AudioStreamData &streamData)
{
    if (NeedUseTempBuffer(ringBuffer, spanSizeInByte)) {
        processTmpBuffer_.resize(0);
        processTmpBuffer_.resize(spanSizeInByte);
        RingBufferWrapper ringBufferDescForCotinueData;
        ringBufferDescForCotinueData.dataLength = ringBuffer.dataLength;
        ringBufferDescForCotinueData.basicBufferDescs[0].buffer = processTmpBuffer_.data();
        ringBufferDescForCotinueData.basicBufferDescs[0].bufLength = ringBuffer.dataLength;
        ringBufferDescForCotinueData.CopyInputBufferValueToCurBuffer(ringBuffer);
        streamData.bufferDesc.buffer = processTmpBuffer_.data();
        streamData.bufferDesc.bufLength = spanSizeInByte;
        streamData.bufferDesc.dataLength = spanSizeInByte;
    } else {
        streamData.bufferDesc.buffer = ringBuffer.basicBufferDescs[0].buffer;
        streamData.bufferDesc.bufLength = ringBuffer.dataLength;
        streamData.bufferDesc.dataLength = ringBuffer.dataLength;
    }
}

int32_t AudioProcessInServer::GetSessionId(uint32_t &sessionId)
{
    sessionId = sessionId_;
    return SUCCESS;
}

void AudioProcessInServer::SetNonInterruptMute(const bool muteFlag)
{
    muteFlag_ = muteFlag;
    AUDIO_INFO_LOG("muteFlag_: %{public}d", muteFlag);
    AudioService::GetInstance()->UpdateMuteControlSet(sessionId_, muteFlag);
}

bool AudioProcessInServer::GetMuteState()
{
    return muteFlag_ || silentModeAndMixWithOthers_;
}

uint32_t AudioProcessInServer::GetSessionId()
{
    return sessionId_;
}

int32_t AudioProcessInServer::GetStandbyStatus(bool &isStandby, int64_t &enterStandbyTime)
{
    if (processBuffer_ == nullptr || processBuffer_->GetStreamStatus() == nullptr) {
        AUDIO_ERR_LOG("GetStandbyStatus failed, buffer is nullptr!");
        return ERR_OPERATION_FAILED;
    }
    isStandby = processBuffer_->GetStreamStatus()->load() == STREAM_STAND_BY;
    if (isStandby) {
        enterStandbyTime = enterStandbyTime_;
    } else {
        enterStandbyTime = 0;
    }

    return SUCCESS;
}

void AudioProcessInServer::EnableStandby()
{
    CHECK_AND_RETURN_LOG(processBuffer_ != nullptr && processBuffer_->GetStreamStatus() != nullptr, "failed: nullptr!");
    processBuffer_->GetStreamStatus()->store(StreamStatus::STREAM_STAND_BY);
    enterStandbyTime_ = ClockTime::GetCurNano();
    audioStreamChecker_->RecordStandbyTime(true);
    WriterRenderStreamStandbySysEvent(sessionId_, 1);
}

int32_t AudioProcessInServer::ResolveBufferBaseAndGetServerSpanSize(std::shared_ptr<OHAudioBufferBase> &buffer,
    uint32_t &spanSizeInFrame)
{
    AUDIO_INFO_LOG("ResolveBuffer start");
    CHECK_AND_RETURN_RET_LOG(isBufferConfiged_, ERR_ILLEGAL_STATE,
        "ResolveBuffer failed, buffer is not configed!");

    if (processBuffer_ == nullptr) {
        AUDIO_ERR_LOG("ResolveBuffer failed, buffer is nullptr!");
    }
    buffer = processBuffer_;
    spanSizeInFrame = spanSizeInframe_;
    CHECK_AND_RETURN_RET_LOG(buffer != nullptr, ERR_ILLEGAL_STATE, "ResolveBuffer failed, processBuffer_ is nullptr!");

    return SUCCESS;
}

int32_t AudioProcessInServer::RequestHandleInfo()
{
    CHECK_AND_RETURN_RET_LOG(isInited_, ERR_ILLEGAL_STATE, "not inited!");
    CHECK_AND_RETURN_RET_LOG(processBuffer_ != nullptr, ERR_ILLEGAL_STATE, "buffer not inited!");

    // Check update handle info usage later
    return SUCCESS;
}

int32_t AudioProcessInServer::RequestHandleInfoAsync()
{
    return RequestHandleInfo();
}


bool AudioProcessInServer::CheckBGCapturer()
{
    uint32_t tokenId = processConfig_.appInfo.appTokenId;
    uint64_t fullTokenId = processConfig_.appInfo.appFullTokenId;

    if (PermissionUtil::VerifyBackgroundCapture(tokenId, fullTokenId)) {
        return true;
    }

    CHECK_AND_RETURN_RET_LOG(Util::IsBackgroundSourceType(processConfig_.capturerInfo.sourceType) &&
        AudioService::GetInstance()->InForegroundList(processConfig_.appInfo.appUid), false, "Verify failed");

    AudioService::GetInstance()->UpdateForegroundState(tokenId, true);
    bool res = PermissionUtil::VerifyBackgroundCapture(tokenId, fullTokenId);
    AUDIO_INFO_LOG("Retry result:%{public}s", (res ? "success" : "fail"));
    AudioService::GetInstance()->UpdateForegroundState(tokenId, false);

    return res;
}

bool AudioProcessInServer::TurnOnMicIndicator(CapturerState capturerState)
{
    uint32_t tokenId = processConfig_.appInfo.appTokenId;
    SwitchStreamInfo info = {
        sessionId_,
        processConfig_.callerUid,
        processConfig_.appInfo.appUid,
        processConfig_.appInfo.appPid,
        tokenId,
        capturerState,
    };
    if (!SwitchStreamUtil::IsSwitchStreamSwitching(info, SWITCH_STATE_STARTED)) {
        CHECK_AND_RETURN_RET_LOG(CheckBGCapturer(), false, "Verify failed");
    }
    SwitchStreamUtil::UpdateSwitchStreamRecord(info, SWITCH_STATE_STARTED);

    if (isMicIndicatorOn_) {
        AUDIO_WARNING_LOG("MicIndicator:already on, Stream:%{public}u.", sessionId_);
    } else {
        CHECK_AND_RETURN_RET_LOG(PermissionUtil::NotifyPrivacyStart(tokenId, sessionId_),
            false, "NotifyPrivacyStart failed!");
        AUDIO_INFO_LOG("MicIndicator:turn on,Stream:%{public}u", sessionId_);
        isMicIndicatorOn_ = true;
    }
    return true;
}

bool AudioProcessInServer::TurnOffMicIndicator(CapturerState capturerState)
{
    uint32_t tokenId = processConfig_.appInfo.appTokenId;
    SwitchStreamInfo info = {
        sessionId_,
        processConfig_.callerUid,
        processConfig_.appInfo.appUid,
        processConfig_.appInfo.appPid,
        tokenId,
        capturerState,
    };
    SwitchStreamUtil::UpdateSwitchStreamRecord(info, SWITCH_STATE_FINISHED);
    if (isMicIndicatorOn_) {
        PermissionUtil::NotifyPrivacyStop(tokenId, sessionId_);
        AUDIO_INFO_LOG("MicIndicator:turn off, Stream:%{public}u", sessionId_);
        isMicIndicatorOn_ = false;
    } else {
        AUDIO_WARNING_LOG("MicIndicator:already off, Stream:%{public}u", sessionId_);
    }
    return true;
}

int32_t AudioProcessInServer::Start()
{
    int32_t ret = StartInner();
    if (playerDfx_ && processConfig_.audioMode == AUDIO_MODE_PLAYBACK) {
        RendererStage stage = ret == SUCCESS ? RENDERER_STAGE_START_OK : RENDERER_STAGE_START_FAIL;
        playerDfx_->WriteDfxStartMsg(sessionId_, stage, sourceDuration_, processConfig_);
    } else if (recorderDfx_ && processConfig_.audioMode == AUDIO_MODE_RECORD) {
        CapturerStage stage = ret == SUCCESS ? CAPTURER_STAGE_START_OK : CAPTURER_STAGE_START_FAIL;
        recorderDfx_->WriteDfxStartMsg(sessionId_, stage, processConfig_);
    }

    lastStartTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    if (processBuffer_ != nullptr) {
        lastWriteFrame_ = static_cast<int64_t>(processBuffer_->GetCurReadFrame());
    }
    lastWriteMuteFrame_ = 0;
    streamStatusInServer_ = STREAM_RUNNING;
    return ret;
}

int32_t AudioProcessInServer::StartInner()
{
    CHECK_AND_RETURN_RET_LOG(isInited_, ERR_ILLEGAL_STATE, "not inited!");

    std::lock_guard<std::mutex> lock(statusLock_);
    CHECK_AND_RETURN_RET_LOG(streamStatus_->load() == STREAM_STARTING || streamStatus_->load() == STREAM_STAND_BY,
        ERR_ILLEGAL_STATE, "Start failed, invalid status.");

    if (processConfig_.audioMode == AUDIO_MODE_RECORD && !needCheckBackground_ &&
        PermissionUtil::NeedVerifyBackgroundCapture(processConfig_.callerUid, processConfig_.capturerInfo.sourceType)) {
        AUDIO_INFO_LOG("set needCheckBackground_: true");
        needCheckBackground_ = true;
    }

    if (processConfig_.audioMode == AUDIO_MODE_RECORD && needCheckBackground_) {
        CHECK_AND_RETURN_RET_LOG(TurnOnMicIndicator(CAPTURER_RUNNING), ERR_PERMISSION_DENIED,
            "Turn on micIndicator failed or check backgroud capture failed for stream:%{public}d!", sessionId_);
    }

    int32_t ret = CoreServiceHandler::GetInstance().UpdateSessionOperation(sessionId_, SESSION_OPERATION_START);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Policy start client failed, reason: %{public}d", ret);
    StreamDfxManager::GetInstance().CheckStreamOccupancy(sessionId_, processConfig_, true);
    for (size_t i = 0; i < listenerList_.size(); i++) {
        listenerList_[i]->OnStart(this);
    }
    if (streamStatus_->load() == STREAM_STAND_BY) {
        AUDIO_INFO_LOG("Call start while in stand-by, session %{public}u", sessionId_);
        WriterRenderStreamStandbySysEvent(sessionId_, 0);
        streamStatus_->store(STREAM_STARTING);
        enterStandbyTime_ = 0;
        audioStreamChecker_->RecordStandbyTime(false);
    } else {
        audioStreamChecker_->MonitorOnAllCallback(AUDIO_STREAM_START, false);
    }

    RebuildCaptureInjector();
    processBuffer_->SetLastWrittenTime(ClockTime::GetCurNano());
    AudioPerformanceMonitor::GetInstance().StartSilenceMonitor(sessionId_, processConfig_.appInfo.appTokenId);
    NotifyXperfOnPlayback(processConfig_.audioMode, XPERF_EVENT_START);
    HILOG_COMM_INFO("Start in server success!");
    return SUCCESS;
}

void AudioProcessInServer::RebuildCaptureInjector()
{
    CHECK_AND_RETURN_LOG(rebuildFlag_, "no need to rebuild");
    if (processConfig_.audioMode == AUDIO_MODE_RECORD &&
        processConfig_.capturerInfo.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION) {
        CoreServiceHandler::GetInstance().RebuildCaptureInjector(sessionId_);
    }
    rebuildFlag_ = false;
}

int32_t AudioProcessInServer::Pause(bool isFlush)
{
    CHECK_AND_RETURN_RET_LOG(isInited_, ERR_ILLEGAL_STATE, "not inited!");

    (void)isFlush;

    {
        std::lock_guard lock(scheduleGuardsMutex_);
        scheduleGuards_[METHOD_START] = nullptr;
    }

    std::lock_guard<std::mutex> lock(statusLock_);
    CHECK_AND_RETURN_RET_LOG(streamStatus_->load() == STREAM_PAUSING,
        ERR_ILLEGAL_STATE, "Pause failed, invalid status.");
        
    if (processConfig_.audioMode == AUDIO_MODE_RECORD && needCheckBackground_) {
        TurnOffMicIndicator(CAPTURER_PAUSED);
    }

    for (size_t i = 0; i < listenerList_.size(); i++) {
        listenerList_[i]->OnPause(this);
    }
    audioStreamChecker_->MonitorOnAllCallback(AUDIO_STREAM_PAUSE, false);
    if (playerDfx_ && processConfig_.audioMode == AUDIO_MODE_PLAYBACK) {
        playerDfx_->WriteDfxActionMsg(sessionId_, RENDERER_STAGE_PAUSE_OK);
    } else if (recorderDfx_ && processConfig_.audioMode == AUDIO_MODE_RECORD) {
        lastStopTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        recorderDfx_->WriteDfxStopMsg(sessionId_, CAPTURER_STAGE_PAUSE_OK,
            GetLastAudioDuration(), processConfig_);
    }
    CoreServiceHandler::GetInstance().UpdateSessionOperation(sessionId_, SESSION_OPERATION_PAUSE);
    StreamDfxManager::GetInstance().CheckStreamOccupancy(sessionId_, processConfig_, false);
    AudioPerformanceMonitor::GetInstance().PauseSilenceMonitor(sessionId_);
    NotifyXperfOnPlayback(processConfig_.audioMode, XPERF_EVENT_STOP);
    HILOG_COMM_INFO("Pause in server success!");
    streamStatusInServer_ = STREAM_PAUSED;
    RemoveStreamInfo();
    return SUCCESS;
}

int32_t AudioProcessInServer::Resume()
{
    CHECK_AND_RETURN_RET_LOG(isInited_, ERR_ILLEGAL_STATE, "not inited!");
    std::lock_guard<std::mutex> lock(statusLock_);
    CHECK_AND_RETURN_RET_LOG(streamStatus_->load() == STREAM_STARTING,
        ERR_ILLEGAL_STATE, "Resume failed, invalid status.");
    if (processConfig_.audioMode == AUDIO_MODE_RECORD && !needCheckBackground_ &&
        PermissionUtil::NeedVerifyBackgroundCapture(processConfig_.callerUid, processConfig_.capturerInfo.sourceType)) {
        AUDIO_INFO_LOG("set needCheckBackground_: true");
        needCheckBackground_ = true;
    }
    if (processConfig_.audioMode == AUDIO_MODE_RECORD && needCheckBackground_) {
        CHECK_AND_RETURN_RET_LOG(TurnOnMicIndicator(CAPTURER_RUNNING), ERR_PERMISSION_DENIED,
            "Turn on micIndicator failed or check backgroud capture failed for stream:%{public}d!", sessionId_);
    }

    for (size_t i = 0; i < listenerList_.size(); i++) {
        listenerList_[i]->OnStart(this);
    }
    AudioPerformanceMonitor::GetInstance().StartSilenceMonitor(sessionId_, processConfig_.appInfo.appTokenId);
    processBuffer_->SetLastWrittenTime(ClockTime::GetCurNano());
    CoreServiceHandler::GetInstance().UpdateSessionOperation(sessionId_, SESSION_OPERATION_START);
    audioStreamChecker_->MonitorOnAllCallback(AUDIO_STREAM_START, false);
    NotifyXperfOnPlayback(processConfig_.audioMode, XPERF_EVENT_START);
    HILOG_COMM_INFO("Resume in server success!");
    streamStatusInServer_ = STREAM_RUNNING;
    return SUCCESS;
}

int32_t AudioProcessInServer::Stop(int32_t stage)
{
    CHECK_AND_RETURN_RET_LOG(isInited_, ERR_ILLEGAL_STATE, "not inited!");

    {
        std::lock_guard lock(scheduleGuardsMutex_);
        scheduleGuards_[METHOD_START] = nullptr;
    }

    std::lock_guard<std::mutex> lock(statusLock_);
    CHECK_AND_RETURN_RET_LOG(streamStatus_->load() == STREAM_STOPPING,
        ERR_ILLEGAL_STATE, "Stop failed, invalid status.");
    if (processConfig_.audioMode == AUDIO_MODE_RECORD && needCheckBackground_) {
        TurnOffMicIndicator(CAPTURER_STOPPED);
    }
    for (size_t i = 0; i < listenerList_.size(); i++) {
        listenerList_[i]->OnPause(this); // notify endpoint?
    }

    lastStopTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    if (processBuffer_ != nullptr) {
        lastWriteFrame_ = static_cast<int64_t>(processBuffer_->GetCurReadFrame()) - lastWriteFrame_;
    }
    audioStreamChecker_->MonitorOnAllCallback(AUDIO_STREAM_STOP, false);
    if (playerDfx_ && processConfig_.audioMode == AUDIO_MODE_PLAYBACK) {
        RendererStage rendererStage = stage == AUDIO_PROC_STAGE_STOP_BY_RELEASE ?
            RENDERER_STAGE_STOP_BY_RELEASE : RENDERER_STAGE_STOP_OK;
        playerDfx_->WriteDfxStopMsg(sessionId_, rendererStage,
            {lastWriteFrame_, lastWriteMuteFrame_, GetLastAudioDuration(), underrunCount_}, processConfig_);
    } else if (recorderDfx_ && processConfig_.audioMode == AUDIO_MODE_RECORD) {
        CapturerStage capturerStage = stage == AUDIO_PROC_STAGE_STOP_BY_RELEASE ?
            CAPTURER_STAGE_STOP_BY_RELEASE : CAPTURER_STAGE_STOP_OK;
        recorderDfx_->WriteDfxStopMsg(sessionId_, capturerStage,
            GetLastAudioDuration(), processConfig_);
    }
    CoreServiceHandler::GetInstance().UpdateSessionOperation(sessionId_, SESSION_OPERATION_STOP);
    StreamDfxManager::GetInstance().CheckStreamOccupancy(sessionId_, processConfig_, false);
    AudioPerformanceMonitor::GetInstance().PauseSilenceMonitor(sessionId_);
    NotifyXperfOnPlayback(processConfig_.audioMode, XPERF_EVENT_STOP);
    HILOG_COMM_INFO("Stop in server success!");
    streamStatusInServer_ = STREAM_STOPPED;
    RemoveStreamInfo();
    return SUCCESS;
}

int32_t AudioProcessInServer::Release(bool isSwitchStream)
{
    AudioStreamMonitor::GetInstance().DeleteCheckForMonitor(processConfig_.originalSessionId);
    CHECK_AND_RETURN_RET_LOG(isInited_, ERR_ILLEGAL_STATE, "not inited or already released");
    {
        std::lock_guard lock(scheduleGuardsMutex_);
        scheduleGuards_[METHOD_WRITE_OR_READ] = nullptr;
        scheduleGuards_[METHOD_START] = nullptr;
    }
    isInited_ = false;
    std::lock_guard<std::mutex> lock(statusLock_);
    CHECK_AND_RETURN_RET_LOG(releaseCallback_ != nullptr, ERR_OPERATION_FAILED, "Failed: no service to notify.");

    if (processConfig_.audioMode == AUDIO_MODE_RECORD && needCheckBackground_) {
        TurnOffMicIndicator(CAPTURER_RELEASED);
    }
    int32_t ret = CoreServiceHandler::GetInstance().UpdateSessionOperation(sessionId_, SESSION_OPERATION_RELEASE);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Policy remove client failed, reason: %{public}d", ret);
    StreamDfxManager::GetInstance().CheckStreamOccupancy(sessionId_, processConfig_, false);
    ret = releaseCallback_->OnProcessRelease(this, isSwitchStream);
    NotifyXperfOnPlayback(processConfig_.audioMode, XPERF_EVENT_RELEASE);
    HILOG_COMM_INFO("notify service release result: %{public}d", ret);
    ReleaseCaptureInjector();
    streamStatusInServer_ = STREAM_RELEASED;
    RemoveStreamInfo();
    return SUCCESS;
}

void AudioProcessInServer::ReleaseCaptureInjector()
{
    if (processConfig_.audioMode == AUDIO_MODE_RECORD &&
        processConfig_.capturerInfo.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION) {
        CoreServiceHandler::GetInstance().ReleaseCaptureInjector();
    }
}

ProcessDeathRecipient::ProcessDeathRecipient(AudioProcessInServer *processInServer,
    ProcessReleaseCallback *processHolder)
{
    processInServer_ = processInServer;
    processHolder_ = processHolder;
    createTime_ = ClockTime::GetCurNano();
    AUDIO_INFO_LOG("OnRemoteDied create time: %{public}" PRId64 "", createTime_);
}

void ProcessDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    CHECK_AND_RETURN_LOG(processHolder_ != nullptr, "processHolder_ is null.");
    int32_t ret = processHolder_->OnProcessRelease(processInServer_);
    AUDIO_INFO_LOG("OnRemoteDied ret: %{public}d %{public}" PRId64 "", ret, createTime_);
}

int32_t AudioProcessInServer::RegisterProcessCb(const sptr<IRemoteObject>& object)
{
    std::lock_guard<std::mutex> lock(registerProcessCbLock_);
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_INVALID_PARAM, "RegisterProcessCb obj is null");
    CHECK_AND_RETURN_RET_LOG(object_ == nullptr, SUCCESS, "Has registerProcessCb obj");
    sptr<IProcessCb> processCb = iface_cast<IProcessCb>(object);
    CHECK_AND_RETURN_RET_LOG(processCb != nullptr, ERR_INVALID_PARAM, "RegisterProcessCb obj cast failed");
    deathRecipient_ = new ProcessDeathRecipient(this, releaseCallback_);
    bool result = object->AddDeathRecipient(deathRecipient_);
    CHECK_AND_RETURN_RET_LOG(result, ERR_OPERATION_FAILED, "AddDeathRecipient failed.");
    object_= object;
    return SUCCESS;
}

void AudioProcessInServer::SetInnerCapState(bool isInnerCapped, int32_t innerCapId)
{
    AUDIO_INFO_LOG("process[%{public}u] innercapped: %{public}s, innerCapId:%{public}d",
        sessionId_, isInnerCapped ? "true" : "false", innerCapId);
    std::lock_guard<std::mutex> lock(innerCapStateMutex_);
    innerCapStates_[innerCapId] = isInnerCapped;
}

bool AudioProcessInServer::GetInnerCapState(int32_t innerCapId)
{
    std::lock_guard<std::mutex> lock(innerCapStateMutex_);
    if (innerCapStates_.count(innerCapId) && innerCapStates_[innerCapId]) {
        return true;
    }
    return false;
}

std::unordered_map<int32_t, bool> AudioProcessInServer::GetInnerCapState()
{
    std::lock_guard<std::mutex> lock(innerCapStateMutex_);
    return  innerCapStates_;
}

AppInfo AudioProcessInServer::GetAppInfo()
{
    return processConfig_.appInfo;
}

BufferDesc &AudioProcessInServer::GetConvertedBuffer()
{
    return convertedBuffer_;
}

int AudioProcessInServer::Dump(int fd, const std::vector<std::u16string> &args)
{
    return SUCCESS;
}

void AudioProcessInServer::Dump(std::string &dumpString)
{
    AppendFormat(dumpString, "\n  - uid: %d\n", processConfig_.appInfo.appUid);
    AppendFormat(dumpString, "- pid: %d\n", processConfig_.appInfo.appUid);
    dumpString += "process info:\n";
    dumpString += "stream info:\n";
    AppendFormat(dumpString, "  - samplingRate: %d\n", processConfig_.streamInfo.samplingRate);
    AppendFormat(dumpString, "  - channels: %d\n", processConfig_.streamInfo.channels);
    AppendFormat(dumpString, "  - format: %d\n", processConfig_.streamInfo.format);
    AppendFormat(dumpString, "  - encoding: %d\n", processConfig_.streamInfo.encoding);
    if (streamStatus_ != nullptr) {
        AppendFormat(dumpString, "  - Status: %d\n", streamStatus_->load());
    }
    dumpString += "\n";
}

std::shared_ptr<OHAudioBufferBase> AudioProcessInServer::GetStreamBuffer()
{
    CHECK_AND_RETURN_RET_LOG(isBufferConfiged_ && processBuffer_ != nullptr,
        nullptr, "GetStreamBuffer failed:process buffer not config.");
    return processBuffer_;
}

AudioStreamInfo AudioProcessInServer::GetStreamInfo()
{
    return processConfig_.streamInfo;
}

uint32_t AudioProcessInServer::GetAudioSessionId()
{
    return sessionId_;
}

AudioStreamType AudioProcessInServer::GetAudioStreamType()
{
    return processConfig_.streamType;
}

AudioProcessConfig AudioProcessInServer::GetAudioProcessConfig()
{
    return processConfig_;
}

inline uint32_t PcmFormatToBits(AudioSampleFormat format)
{
    switch (format) {
        case SAMPLE_U8:
            return 1; // 1 byte
        case SAMPLE_S16LE:
            return 2; // 2 byte
        case SAMPLE_S24LE:
            return 3; // 3 byte
        case SAMPLE_S32LE:
            return 4; // 4 byte
        case SAMPLE_F32LE:
            return 4; // 4 byte
        default:
            return 2; // 2 byte
    }
}

int32_t AudioProcessInServer::InitBufferStatus()
{
    CHECK_AND_RETURN_RET_LOG(processBuffer_ != nullptr, ERR_ILLEGAL_STATE,
        "InitBufferStatus failed, null buffer.");

    processBuffer_->SetLastWrittenTime(ClockTime::GetCurNano());
    return SUCCESS;
}

bool AudioProcessInServer::IsNeedRecordResampleConv(AudioSamplingRate srcSamplingRate)
{
    return ((processConfig_.audioMode == AUDIO_MODE_RECORD) &&
        (processConfig_.streamInfo.samplingRate != srcSamplingRate) &&
        (processConfig_.streamInfo.format != SAMPLE_F32LE));    // resample already conv f32
}

int32_t AudioProcessInServer::ConfigProcessBuffer(uint32_t &totalSizeInframe,
    uint32_t &spanSizeInframe, AudioStreamInfo &serverStreamInfo, const std::shared_ptr<OHAudioBufferBase> &buffer)
{
    if (processBuffer_ != nullptr) {
        AUDIO_INFO_LOG("ConfigProcessBuffer: process buffer already configed!");
        return SUCCESS;
    }
    // check
    CHECK_AND_RETURN_RET_LOG(totalSizeInframe != 0 && spanSizeInframe != 0 && totalSizeInframe % spanSizeInframe == 0,
        ERR_INVALID_PARAM, "ConfigProcessBuffer failed: ERR_INVALID_PARAM");

    uint32_t spanTime = spanSizeInframe * AUDIO_MS_PER_SECOND / serverStreamInfo.samplingRate;
    spanSizeInframe_ = spanTime * processConfig_.streamInfo.samplingRate / AUDIO_MS_PER_SECOND;
    totalSizeInframe_ = totalSizeInframe / spanSizeInframe * spanSizeInframe_;

    uint32_t channel = processConfig_.streamInfo.channels;
    uint32_t formatbyte = PcmFormatToBits(processConfig_.streamInfo.format);
    byteSizePerFrame_ = channel * formatbyte;
    if (serverStreamInfo.channels != processConfig_.streamInfo.channels ||
        serverStreamInfo.format != processConfig_.streamInfo.format ||
        IsNeedRecordResampleConv(serverStreamInfo.samplingRate)) {
        size_t spanSizeInByte = 0;
        if (processConfig_.audioMode == AUDIO_MODE_PLAYBACK) {
            uint32_t serverByteSize = serverStreamInfo.channels * PcmFormatToBits(serverStreamInfo.format);
            spanSizeInByte = static_cast<size_t>(spanSizeInframe * serverByteSize);
        } else {
            spanSizeInByte = static_cast<size_t>(spanSizeInframe_ * byteSizePerFrame_);
        }
        convertedBuffer_.buffer = new uint8_t[spanSizeInByte];
        convertedBuffer_.bufLength = spanSizeInByte;
        convertedBuffer_.dataLength = spanSizeInByte;
    }

    if (buffer == nullptr) {
        // create OHAudioBuffer in server.
        processBuffer_ = OHAudioBufferBase::CreateFromLocal(totalSizeInframe_, byteSizePerFrame_);
        CHECK_AND_RETURN_RET_LOG(processBuffer_ != nullptr, ERR_OPERATION_FAILED, "Create process buffer failed.");

        CHECK_AND_RETURN_RET_LOG(processBuffer_->GetBufferHolder() == AudioBufferHolder::AUDIO_SERVER_SHARED,
            ERR_ILLEGAL_STATE, "CreateFormLocal in server failed.");
        AUDIO_INFO_LOG("Config: totalSizeInframe:%{public}d spanSizeInframe:%{public}d byteSizePerFrame:%{public}d",
            totalSizeInframe_, spanSizeInframe_, byteSizePerFrame_);

        // we need to clear data buffer to avoid dirty data.
        memset_s(processBuffer_->GetDataBase(), processBuffer_->GetDataSize(), 0, processBuffer_->GetDataSize());
        int32_t ret = InitBufferStatus();
        AUDIO_DEBUG_LOG("clear data buffer, ret:%{public}d", ret);
    } else {
        processBuffer_ = buffer;
        AUDIO_INFO_LOG("ConfigBuffer in server separate, base: %{public}d", *processBuffer_->GetDataBase());
    }

    streamStatus_ = processBuffer_->GetStreamStatus();
    CHECK_AND_RETURN_RET_LOG(streamStatus_ != nullptr, ERR_OPERATION_FAILED, "Create process buffer failed.");
    isBufferConfiged_ = true;
    isInited_ = true;
    audioCheckFreq_ = threshold * AUDIO_MS_PER_SECOND / spanTime;
    return SUCCESS;
}

int32_t AudioProcessInServer::AddProcessStatusListener(std::shared_ptr<IProcessStatusListener> listener)
{
    std::lock_guard<std::mutex> lock(listenerListLock_);
    listenerList_.push_back(listener);
    return SUCCESS;
}

int32_t AudioProcessInServer::RemoveProcessStatusListener(std::shared_ptr<IProcessStatusListener> listener)
{
    std::lock_guard<std::mutex> lock(listenerListLock_);
    std::vector<std::shared_ptr<IProcessStatusListener>>::iterator it = listenerList_.begin();
    bool isFind = false;
    while (it != listenerList_.end()) {
        if (*it == listener) {
            listenerList_.erase(it);
            isFind = true;
            break;
        } else {
            it++;
        }
    }

    AUDIO_INFO_LOG("%{public}s the endpoint.", (isFind ? "find and remove" : "not find"));
    return SUCCESS;
}

int32_t AudioProcessInServer::RegisterThreadPriority(int32_t tid, const std::string &bundleName, uint32_t method)
{
    pid_t pid = IPCSkeleton::GetCallingPid();
    CHECK_AND_RETURN_RET_LOG(method < METHOD_MAX, ERR_INVALID_PARAM, "err param %{public}u", method);
    auto sharedGuard = SharedAudioScheduleGuard::Create(pid, static_cast<pid_t>(tid), bundleName);
    std::lock_guard lock(scheduleGuardsMutex_);
    scheduleGuards_[method].swap(sharedGuard);
    return SUCCESS;
}

int32_t AudioProcessInServer::SetAudioHapticsSyncId(int32_t audioHapticsSyncId)
{
    AUDIO_INFO_LOG("AudioProcessInServer::SetAudioHapticsSyncId %{public}d", audioHapticsSyncId);
    audioHapticsSyncId_.store(audioHapticsSyncId);
    return SUCCESS;
}

void AudioProcessInServer::WriterRenderStreamStandbySysEvent(uint32_t sessionId, int32_t standby)
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::STREAM_STANDBY,
        Media::MediaMonitor::BEHAVIOR_EVENT);
    bean->Add("STREAMID", static_cast<int32_t>(sessionId));
    bean->Add("STANDBY", standby);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);

    std::unordered_map<std::string, std::string> payload;
    payload["uid"] = std::to_string(processConfig_.appInfo.appUid);
    payload["sessionId"] = std::to_string(sessionId);
    payload["isStandby"] = std::to_string(standby);
    ReportDataToResSched(payload, ResourceSchedule::ResType::RES_TYPE_AUDIO_RENDERER_STANDBY);

    if (playerDfx_ && processConfig_.audioMode == AUDIO_MODE_PLAYBACK) {
        playerDfx_->WriteDfxActionMsg(sessionId_, standby == 0 ?
            RENDERER_STAGE_STANDBY_END : RENDERER_STAGE_STANDBY_BEGIN);
    }
}

void AudioProcessInServer::ReportDataToResSched(std::unordered_map<std::string, std::string> payload, uint32_t type)
{
#ifdef RESSCHE_ENABLE
    AUDIO_INFO_LOG("report event to ResSched ,event type : %{public}d", type);
    ResourceSchedule::ResSchedClient::GetInstance().ReportData(type, 0, payload);
#endif
}

void AudioProcessInServer::WriteDumpFile(void *buffer, size_t bufferSize)
{
    if (AudioDump::GetInstance().GetVersionType() == DumpFileUtil::BETA_VERSION) {
        DumpFileUtil::WriteDumpFile(dumpFile_, buffer, bufferSize);
        AudioCacheMgr::GetInstance().CacheData(dumpFileName_, buffer, bufferSize);
    }
}

int32_t AudioProcessInServer::SetDefaultOutputDevice(int32_t defaultOutputDevice, bool skipForce)
{
    CHECK_AND_RETURN_RET_LOG(streamStatus_ != nullptr, ERROR, "streamStatus_ is nullptr");
    return CoreServiceHandler::GetInstance().SetDefaultOutputDevice(static_cast<DeviceType>(defaultOutputDevice),
        sessionId_, processConfig_.rendererInfo.streamUsage, streamStatus_->load() == STREAM_RUNNING, skipForce);
}

int32_t AudioProcessInServer::SetSilentModeAndMixWithOthers(bool on)
{
    silentModeAndMixWithOthers_ = on;
    AUDIO_INFO_LOG("%{public}d", on);
    return SUCCESS;
}

std::time_t AudioProcessInServer::GetStartMuteTime()
{
    return startMuteTime_;
}
 
void AudioProcessInServer::SetStartMuteTime(std::time_t time)
{
    startMuteTime_ = time;
}
 
bool AudioProcessInServer::GetSilentState()
{
    return isInSilentState_;
}
 
void AudioProcessInServer::SetSilentState(bool state)
{
    isInSilentState_ = state;
}
int32_t AudioProcessInServer::SetSourceDuration(int64_t duration)
{
    sourceDuration_ = duration;
    return SUCCESS;
}

int32_t AudioProcessInServer::SetUnderrunCount(uint32_t underrunCnt)
{
    underrunCount_ = underrunCnt;
    return SUCCESS;
}

void AudioProcessInServer::AddMuteWriteFrameCnt(int64_t muteFrameCnt)
{
    lastWriteMuteFrame_ += muteFrameCnt;
}

void AudioProcessInServer::AddMuteFrameSize(int64_t muteFrameCnt)
{
    if (muteFrameCnt < 0) {
        audioStreamChecker_->RecordMuteFrame();
    }
}

void AudioProcessInServer::AddNoDataFrameSize()
{
    audioStreamChecker_->RecordNodataFrame();
}

void AudioProcessInServer::AddNormalFrameSize()
{
    audioStreamChecker_->RecordNormalFrame();
}

StreamStatus AudioProcessInServer::GetStreamStatus()
{
    return streamStatus_->load();
}

int32_t AudioProcessInServer::GetAudioHapticsSyncId()
{
    return audioHapticsSyncId_.load();
}

int64_t AudioProcessInServer::GetLastAudioDuration()
{
    auto ret = lastStopTime_ - lastStartTime_;
    return ret < 0 ? -1 : ret;
}

RestoreStatus AudioProcessInServer::RestoreSession(RestoreInfo restoreInfo)
{
    RestoreStatus restoreStatus = processBuffer_->SetRestoreStatus(NEED_RESTORE);
    if (restoreStatus == NEED_RESTORE) {
        if (processConfig_.audioMode == AUDIO_MODE_RECORD) {
            SwitchStreamInfo info = {
                sessionId_,
                processConfig_.callerUid,
                processConfig_.appInfo.appUid,
                processConfig_.appInfo.appPid,
                processConfig_.appInfo.appTokenId,
                streamStatusInServer_ == STREAM_RUNNING ? CAPTURER_RUNNING : CAPTURER_PREPARED
            };
            AUDIO_INFO_LOG("Insert switchStream:%{public}u uid:%{public}d tokenId:%{public}u "
                "Reason:NEED_RESTORE", sessionId_, info.callerUid, info.appTokenId);
            SwitchStreamUtil::UpdateSwitchStreamRecord(info, SWITCH_STATE_WAITING);
        }

        processBuffer_->SetRestoreInfo(restoreInfo);
        processBuffer_->WakeFutex();

        std::lock_guard<std::mutex> lock(listenerListLock_);
        std::vector<std::shared_ptr<IProcessStatusListener>>::iterator it = listenerList_.begin();
        while (it != listenerList_.end()) {
            (*it)->StopByRestore(restoreInfo);
            it++;
        }
    }
    return restoreStatus;
}

int32_t AudioProcessInServer::SaveAdjustStreamVolumeInfo(float volume, uint32_t sessionId,
    const std::string& adjustTime, uint32_t code)
{
    AudioService::GetInstance()->SaveAdjustStreamVolumeInfo(volume, sessionId, adjustTime, code);
    return SUCCESS;
}

int32_t AudioProcessInServer::StopSession()
{
    CHECK_AND_RETURN_RET_LOG(processBuffer_ != nullptr, ERR_INVALID_PARAM, "processBuffer_ is nullptr");
    processBuffer_->SetStopFlag(true);
    return SUCCESS;
}

uint32_t AudioProcessInServer::GetSpanSizeInFrame()
{
    return spanSizeInframe_;
}

uint32_t AudioProcessInServer::GetByteSizePerFrame()
{
    return byteSizePerFrame_;
}

void AudioProcessInServer::NotifyXperfOnPlayback(AudioMode audioMode, XperfEventId eventId)
{
    CHECK_AND_RETURN(audioMode == AUDIO_MODE_PLAYBACK);
    XperfAdapter::GetInstance().ReportStateChangeEventIfNeed(eventId,
        processConfig_.rendererInfo.streamUsage, sessionId_, processConfig_.appInfo.appPid,
        processConfig_.appInfo.appUid);
}

StreamStatus AudioProcessInServer::GetStreamInServerStatus()
{
    return streamStatusInServer_;
}

int32_t AudioProcessInServer::WriteToRingBuffer(RingBufferWrapper &writeBuf, const BufferDesc &buffer)
{
    CHECK_AND_RETURN_RET_LOG(buffer.buffer != nullptr && buffer.bufLength > 0, ERR_WRITE_FAILED, "failed");
    return writeBuf.CopyInputBufferValueToCurBuffer(RingBufferWrapper{
        .basicBufferDescs = {{
            {.buffer = buffer.buffer, .bufLength = buffer.bufLength},
            {.buffer = nullptr, .bufLength = 0}}},
        .dataLength = buffer.bufLength
    });
}

void AudioProcessInServer::SetCaptureStreamInfo(AudioStreamInfo &srcInfo, AudioCaptureDataProcParams &procParams)
{
    srcInfo.channels = STEREO;
    srcInfo.format = procParams.isConvertReadFormat_ ? SAMPLE_F32LE : SAMPLE_S16LE;
    srcInfo.samplingRate = procParams.srcSamplingRate;
}

int32_t AudioProcessInServer::CaptureDataResampleProcess(const size_t bufLen,
                                                         BufferDesc &outBuf,
                                                         AudioStreamInfo &srcInfo,
                                                         AudioCaptureDataProcParams &procParams)
{
    uint32_t srcRate = static_cast<uint32_t>(srcInfo.samplingRate);
    uint32_t dstRate = static_cast<uint32_t>(processConfig_.streamInfo.samplingRate);

    /* no need resample */
    if (srcRate == dstRate) {
        /* if already convert, data is not readbuf, need update to new buff */
        if (procParams.isConvertReadFormat_) {
            outBuf.buffer = procParams.captureConvBuffer_.data();
            outBuf.bufLength = bufLen;
        }
        return SUCCESS;
    }

    AUDIO_INFO_LOG("Audio capture resample, srcRate:%{public}u, dstRate:%{public}u", srcRate, dstRate);
    int32_t ret;
    float *resampleInBuff = nullptr;
    if (srcInfo.format != SAMPLE_F32LE) {
        BufferDesc convBufTmp = {ReallocVectorBufferAndClear(procParams.captureConvBuffer_, bufLen), bufLen};
        ret = FormatConverter::S16StereoToF32Stereo(outBuf, convBufTmp);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Convert s16 stereo to f32 stereo failed");
        srcInfo.format = SAMPLE_F32LE;
        resampleInBuff = reinterpret_cast<float*>(convBufTmp.buffer);
    } else {
        resampleInBuff = reinterpret_cast<float*>(procParams.captureConvBuffer_.data());
    }

    if (resampler_ == nullptr) {
        resampler_ = std::make_unique<HPAE::ProResampler>(srcRate, dstRate, srcInfo.channels, 1);
    }

    uint32_t formatByte = PcmFormatToBits(srcInfo.format);
    uint32_t channels = static_cast<uint32_t>(srcInfo.channels);
    uint32_t byteSizePerFrame = channels * formatByte;
    uint32_t resampleInBuffSize = bufLen / byteSizePerFrame;
    uint32_t resampleOutBuffSize = spanSizeInframe_;
    uint32_t outBuffLen = spanSizeInframe_ * byteSizePerFrame; // here need use src byteSize
    float *resampleOutBuff =
        reinterpret_cast<float*>(ReallocVectorBufferAndClear(procParams.rendererConvBuffer_, outBuffLen));
    ret = resampler_->Process(resampleInBuff, resampleInBuffSize, resampleOutBuff, resampleOutBuffSize);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Capture data resample failed");

    outBuf.buffer = reinterpret_cast<uint8_t*>(resampleOutBuff);
    outBuf.bufLength = outBuffLen;

    return SUCCESS;
}

int32_t AudioProcessInServer::CapturerDataFormatAndChnConv(RingBufferWrapper &writeBuf,
                                                           BufferDesc &resampleOutBuf,
                                                           const AudioStreamInfo &srcInfo,
                                                           const AudioStreamInfo &dstInfo)
{
    AudioChannel srcChn = srcInfo.channels;
    AudioChannel dstChn = dstInfo.channels;
    AudioSampleFormat srcFormat = srcInfo.format;
    AudioSampleFormat dstFormat = dstInfo.format;
    FormatKey key{srcChn, srcFormat, dstChn, dstFormat};
    FormatHandlerMap formatHanlders = FormatConverter::GetFormatHandlers();

    auto it = formatHanlders.find(key);
    if (it != formatHanlders.end()) {
        bool isDoConvert = false;
        int32_t ret = it->second(resampleOutBuf, convertedBuffer_, isDoConvert);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_WRITE_FAILED, "Convert format failed");

        if (isDoConvert) {
            ret = WriteToRingBuffer(writeBuf, convertedBuffer_);
            CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_WRITE_FAILED, "write to client buffer failed");
            ret = memset_s(static_cast<void *>(convertedBuffer_.buffer), convertedBuffer_.bufLength, 0,
                           convertedBuffer_.bufLength);
            CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_WRITE_FAILED, "memset converted buffer to 0 failed");
            return SUCCESS;
        } else {
            return WriteToRingBuffer(writeBuf, resampleOutBuf);
        }
    }

    return ERR_NOT_SUPPORTED;
}

int32_t AudioProcessInServer::HandleCapturerDataParams(RingBufferWrapper &writeBuf,
                                                       AudioCaptureDataProcParams &procParams)
{
    AudioStreamInfo srcInfo = {};
    SetCaptureStreamInfo(srcInfo, procParams);

    size_t bufLen = procParams.readBuf_.bufLength * 2; // unit of byte, 2 is int16_t to float
    BufferDesc resampleOutBuf = procParams.readBuf_;
    int32_t ret = CaptureDataResampleProcess(bufLen, resampleOutBuf, srcInfo, procParams);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_WRITE_FAILED, "capture data resample failed");

    ret = CapturerDataFormatAndChnConv(writeBuf, resampleOutBuf, srcInfo, processConfig_.streamInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "capture data convert failed");
    DumpFileUtil::WriteDumpFile(dumpFAC_, static_cast<void *>(writeBuf.basicBufferDescs[0].buffer),
        writeBuf.dataLength);

    return SUCCESS;
}

int32_t AudioProcessInServer::SetRebuildFlag()
{
    rebuildFlag_ = true;
    return SUCCESS;
}

int32_t AudioProcessInServer::WriteToSpecialProcBuf(AudioCaptureDataProcParams &procParams)
{
    CHECK_AND_RETURN_RET_LOG(processBuffer_ != nullptr, ERR_INVALID_HANDLE, "process buffer is null.");
    uint64_t curWritePos = processBuffer_->GetCurWriteFrame();
    Trace trace("WriteProcessData-<" + std::to_string(curWritePos));

    int32_t writeAbleSize = processBuffer_->GetWritableDataFrames();
    uint32_t dstSpanSizeInframe = spanSizeInframe_;
    if (writeAbleSize <= 0 || static_cast<uint32_t>(writeAbleSize) <= dstSpanSizeInframe) {
        AUDIO_WARNING_LOG("client read too slow: curWritePos:%{public}" PRIu64" writeAbleSize:%{public}d",
            curWritePos, writeAbleSize);
        return ERR_OPERATION_FAILED;
    }

    RingBufferWrapper ringBuffer;
    int32_t ret = processBuffer_->GetAllWritableBufferFromPosFrame(curWritePos, ringBuffer);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "get write buffer fail, ret %{public}d.", ret);

    uint32_t totalSizeInFrame;
    uint32_t byteSizePerFrame;
    processBuffer_->GetSizeParameter(totalSizeInFrame, byteSizePerFrame);
    CHECK_AND_RETURN_RET_LOG(byteSizePerFrame > 0, ERR_OPERATION_FAILED, "byteSizePerFrame is 0");
    uint32_t writeableSizeInFrame = ringBuffer.dataLength / byteSizePerFrame;
    if (writeableSizeInFrame > dstSpanSizeInframe) {
        ringBuffer.dataLength = dstSpanSizeInframe * byteSizePerFrame;
    }

    if (GetMuteState()) {
        ringBuffer.SetBuffersValueWithSpecifyDataLen(0);
    } else {
        ret = HandleCapturerDataParams(ringBuffer, procParams);
    }

    CHECK_AND_RETURN_RET_LOG(ret == EOK, ERR_WRITE_FAILED, "memcpy data to process buffer fail, "
        "curWritePos %{public}" PRIu64", ret %{public}d.", curWritePos, ret);

    processBuffer_->SetHandleInfo(curWritePos, ClockTime::GetCurNano());
    ret = processBuffer_->SetCurWriteFrame(curWritePos + dstSpanSizeInframe);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("set processBuffer_ next write frame fail, ret %{public}d.", ret);
        return ERR_OPERATION_FAILED;
    }
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
