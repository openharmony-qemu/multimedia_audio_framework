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
        std::to_string(samplingRate) + '_' + std::to_string(channels) + '_' + std::to_string(format) +
        ".pcm";
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
    if (processConfig_.audioMode == AUDIO_MODE_RECORD && needCheckBackground_) {
        TurnOffMicIndicator(CAPTURER_INVALID);
    }
    AudioStreamMonitor::GetInstance().DeleteCheckForMonitor(processConfig_.originalSessionId);
}

static CapturerState HandleStreamStatusToCapturerState(const StreamStatus &status)
{
    switch (status) {
        case STREAM_IDEL:
        case STREAM_STAND_BY:
            return CAPTURER_PREPARED;
        case STREAM_STARTING:
        case STREAM_RUNNING:
            return CAPTURER_RUNNING;
        case STREAM_PAUSING:
        case STREAM_PAUSED:
            return CAPTURER_PAUSED;
        case STREAM_STOPPING:
        case STREAM_STOPPED:
            return CAPTURER_STOPPED;
        case STREAM_RELEASED:
            return CAPTURER_RELEASED;
        default:
            return CAPTURER_INVALID;
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

    for (size_t i = 0; i < listenerList_.size(); i++) {
        listenerList_[i]->OnUpdateHandleInfo(this);
    }
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

    CHECK_AND_RETURN_RET_LOG(processConfig_.capturerInfo.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION &&
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
        AUDIO_WARNING_LOG("MicIndicator of stream:%{public}d is already on."
            "No need to call NotifyPrivacyStart!", sessionId_);
    } else {
        CHECK_AND_RETURN_RET_LOG(PermissionUtil::NotifyPrivacyStart(tokenId, sessionId_),
            false, "NotifyPrivacyStart failed!");
        AUDIO_INFO_LOG("Turn on micIndicator of stream:%{public}d from off "
            "after NotifyPrivacyStart success!", sessionId_);
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
        AUDIO_INFO_LOG("Turn off micIndicator of stream:%{public}d from on after NotifyPrivacyStop!", sessionId_);
        isMicIndicatorOn_ = false;
    } else {
        AUDIO_WARNING_LOG("MicIndicator of stream:%{public}d is already off."
            "No need to call NotifyPrivacyStop!", sessionId_);
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

    processBuffer_->SetLastWrittenTime(ClockTime::GetCurNano());
    AudioPerformanceMonitor::GetInstance().StartSilenceMonitor(sessionId_, processConfig_.appInfo.appTokenId);
    AUDIO_INFO_LOG("Start in server success!");
    return SUCCESS;
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
    AUDIO_PRERELEASE_LOGI("Pause in server success!");
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
    AUDIO_PRERELEASE_LOGI("Resume in server success!");
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
    AUDIO_INFO_LOG("Stop in server success!");
    return SUCCESS;
}

int32_t AudioProcessInServer::Release(bool isSwitchStream)
{
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
    AUDIO_INFO_LOG("notify service release result: %{public}d", ret);
    return SUCCESS;
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
        serverStreamInfo.format != processConfig_.streamInfo.format) {
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
                HandleStreamStatusToCapturerState(streamStatus_->load())
            };
            AUDIO_INFO_LOG("Insert fast record stream:%{public}u uid:%{public}d tokenId:%{public}u "
                "into switchStreamRecord because restoreStatus:NEED_RESTORE",
                sessionId_, info.callerUid, info.appTokenId);
            SwitchStreamUtil::UpdateSwitchStreamRecord(info, SWITCH_STATE_WAITING);
        }

        processBuffer_->SetRestoreInfo(restoreInfo);
        processBuffer_->WakeFutex();
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
} // namespace AudioStandard
} // namespace OHOS
