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

#include "audio_errors.h"
#include "audio_capturer_log.h"
#include "audio_service.h"
#include "audio_schedule.h"
#include "audio_utils.h"
#include "media_monitor_manager.h"
#include "audio_dump_pcm.h"
#include "audio_performance_monitor.h"
#ifdef RESSCHE_ENABLE
#include "res_type.h"
#include "res_sched_client.h"
#endif

namespace OHOS {
namespace AudioStandard {
namespace {
static constexpr int32_t VOLUME_SHIFT_NUMBER = 16; // 1 >> 16 = 65536, max volume
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

    const auto [samplingRate, encoding, format, channels, channelLayout] = processConfig.streamInfo;
    // eg: 100005_dump_process_server_audio_48000_2_1.pcm
    dumpFileName_ = std::to_string(sessionId_) + '_' + "_dump_process_server_audio_" +
        std::to_string(samplingRate) + '_' + std::to_string(channels) + '_' + std::to_string(format) +
        ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileName_, &dumpFile_);
    playerDfx_ = std::make_unique<PlayerDfxWriter>(processConfig_.appInfo, sessionId_);
    recorderDfx_ = std::make_unique<RecorderDfxWriter>(processConfig_.appInfo, sessionId_);
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
        AUDIO_ERR_LOG("GetStandbyStatus failed, buffer is nullptr.");
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
    CHECK_AND_RETURN_LOG(processBuffer_ != nullptr && processBuffer_->GetStreamStatus() != nullptr, "failed: nullptr");
    processBuffer_->GetStreamStatus()->store(StreamStatus::STREAM_STAND_BY);
    enterStandbyTime_ = ClockTime::GetCurNano();

    WriterRenderStreamStandbySysEvent(sessionId_, 1);
}

int32_t AudioProcessInServer::ResolveBuffer(std::shared_ptr<OHAudioBuffer> &buffer)
{
    AUDIO_INFO_LOG("ResolveBuffer start");
    CHECK_AND_RETURN_RET_LOG(isBufferConfiged_, ERR_ILLEGAL_STATE,
        "ResolveBuffer failed, buffer is not configed.");

    if (processBuffer_ == nullptr) {
        AUDIO_ERR_LOG("ResolveBuffer failed, buffer is nullptr.");
    }
    buffer = processBuffer_;
    CHECK_AND_RETURN_RET_LOG(buffer != nullptr, ERR_ILLEGAL_STATE, "ResolveBuffer failed, processBuffer_ is null.");

    return SUCCESS;
}

int32_t AudioProcessInServer::RequestHandleInfo(bool isAsync)
{
    CHECK_AND_RETURN_RET_LOG(isInited_, ERR_ILLEGAL_STATE, "not inited!");
    CHECK_AND_RETURN_RET_LOG(processBuffer_ != nullptr, ERR_ILLEGAL_STATE, "buffer not inited!");

    for (size_t i = 0; i < listenerList_.size(); i++) {
        listenerList_[i]->OnUpdateHandleInfo(this);
    }
    return SUCCESS;
}

bool AudioProcessInServer::TurnOnMicIndicator(CapturerState capturerState)
{
    uint32_t tokenId = processConfig_.appInfo.appTokenId;
    uint64_t fullTokenId = processConfig_.appInfo.appFullTokenId;
    SwitchStreamInfo info = {
        sessionId_,
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
            "No need to call NotifyPrivacyStart!", sessionId_);
    } else {
        CHECK_AND_RETURN_RET_LOG(PermissionUtil::NotifyPrivacyStart(tokenId, sessionId_),
            false, "NotifyPrivacyStart failed!");
        AUDIO_INFO_LOG("Turn on micIndicator of stream:%{public}d from off"
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
        lastWriteFrame_ = processBuffer_->GetCurReadFrame();
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

    for (size_t i = 0; i < listenerList_.size(); i++) {
        listenerList_[i]->OnStart(this);
    }
    if (streamStatus_->load() == STREAM_STAND_BY) {
        AUDIO_INFO_LOG("Call start while in stand-by, session %{public}u", sessionId_);
        WriterRenderStreamStandbySysEvent(sessionId_, 0);
        streamStatus_->store(STREAM_STARTING);
        enterStandbyTime_ = 0;
    }

    processBuffer_->SetLastWrittenTime(ClockTime::GetCurNano());
    AudioPerformanceMonitor::GetInstance().ClearSilenceMonitor(sessionId_);
    AUDIO_INFO_LOG("Start in server success!");
    return SUCCESS;
}

int32_t AudioProcessInServer::Pause(bool isFlush)
{
    CHECK_AND_RETURN_RET_LOG(isInited_, ERR_ILLEGAL_STATE, "not inited!");

    (void)isFlush;
    std::lock_guard<std::mutex> lock(statusLock_);
    CHECK_AND_RETURN_RET_LOG(streamStatus_->load() == STREAM_PAUSING,
        ERR_ILLEGAL_STATE, "Pause failed, invalid status.");

    if (processConfig_.audioMode == AUDIO_MODE_RECORD && needCheckBackground_) {
        TurnOffMicIndicator(CAPTURER_PAUSED);
    }

    for (size_t i = 0; i < listenerList_.size(); i++) {
        listenerList_[i]->OnPause(this);
    }

    if (playerDfx_ && processConfig_.audioMode == AUDIO_MODE_PLAYBACK) {
        playerDfx_->WriteDfxActionMsg(sessionId_, RENDERER_STAGE_PAUSE_OK);
    } else if (recorderDfx_ && processConfig_.audioMode == AUDIO_MODE_RECORD) {
        recorderDfx_->WriteDfxActionMsg(sessionId_, CAPTURER_STAGE_PAUSE_OK);
    }

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
    AudioPerformanceMonitor::GetInstance().ClearSilenceMonitor(sessionId_);
    processBuffer_->SetLastWrittenTime(ClockTime::GetCurNano());
    AUDIO_PRERELEASE_LOGI("Resume in server success!");
    return SUCCESS;
}

int32_t AudioProcessInServer::Stop()
{
    CHECK_AND_RETURN_RET_LOG(isInited_, ERR_ILLEGAL_STATE, "not inited!");

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
        lastWriteFrame_ = processBuffer_->GetCurReadFrame() - lastWriteFrame_;
    }
    if (playerDfx_ && processConfig_.audioMode == AUDIO_MODE_PLAYBACK) {
        playerDfx_->WriteDfxStopMsg(sessionId_, RENDERER_STAGE_STOP_OK,
            {lastWriteFrame_, lastWriteMuteFrame_, GetLastAudioDuration(), underrunCount_}, processConfig_);
    } else if (recorderDfx_ && processConfig_.audioMode == AUDIO_MODE_RECORD) {
        recorderDfx_->WriteDfxStopMsg(sessionId_, CAPTURER_STAGE_STOP_OK,
            GetLastAudioDuration(), processConfig_);
    }

    AUDIO_INFO_LOG("Stop in server success!");
    return SUCCESS;
}

int32_t AudioProcessInServer::Release(bool isSwitchStream)
{
    CHECK_AND_RETURN_RET_LOG(isInited_, ERR_ILLEGAL_STATE, "not inited or already released");
    UnscheduleReportData(processConfig_.appInfo.appPid, clientTid_, clientBundleName_.c_str());
    clientThreadPriorityRequested_ = false;
    isInited_ = false;
    std::lock_guard<std::mutex> lock(statusLock_);
    CHECK_AND_RETURN_RET_LOG(releaseCallback_ != nullptr, ERR_OPERATION_FAILED, "Failed: no service to notify.");

    if (processConfig_.audioMode == AUDIO_MODE_RECORD && needCheckBackground_) {
        TurnOffMicIndicator(CAPTURER_RELEASED);
    }

    int32_t ret = releaseCallback_->OnProcessRelease(this, isSwitchStream);
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

int32_t AudioProcessInServer::RegisterProcessCb(sptr<IRemoteObject> object)
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
    innerCapStates_[innerCapId] = isInnerCapped;
}

bool AudioProcessInServer::GetInnerCapState(int32_t innerCapId)
{
    if (innerCapStates_.count(innerCapId) && innerCapStates_[innerCapId]) {
        return true;
    }
    return false;
}

std::unordered_map<int32_t, bool> AudioProcessInServer::GetInnerCapState()
{
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

std::shared_ptr<OHAudioBuffer> AudioProcessInServer::GetStreamBuffer()
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

    uint32_t spanCount = processBuffer_->GetSpanCount();
    for (uint32_t i = 0; i < spanCount; i++) {
        SpanInfo *spanInfo = processBuffer_->GetSpanInfoByIndex(i);
        CHECK_AND_RETURN_RET_LOG(spanInfo != nullptr, ERR_ILLEGAL_STATE,
            "InitBufferStatus failed, null spaninfo");
        spanInfo->spanStatus = SPAN_READ_DONE;
        spanInfo->offsetInFrame = 0;

        spanInfo->readStartTime = 0;
        spanInfo->readDoneTime = 0;

        spanInfo->writeStartTime = 0;
        spanInfo->writeDoneTime = 0;

        spanInfo->volumeStart = 1 << VOLUME_SHIFT_NUMBER; // 65536 for initialize
        spanInfo->volumeEnd = 1 << VOLUME_SHIFT_NUMBER; // 65536 for initialize
        spanInfo->isMute = false;
    }
    processBuffer_->SetLastWrittenTime(ClockTime::GetCurNano());
    return SUCCESS;
}

int32_t AudioProcessInServer::ConfigProcessBuffer(uint32_t &totalSizeInframe,
    uint32_t &spanSizeInframe, DeviceStreamInfo &serverStreamInfo, const std::shared_ptr<OHAudioBuffer> &buffer)
{
    if (processBuffer_ != nullptr) {
        AUDIO_INFO_LOG("ConfigProcessBuffer: process buffer already configed!");
        return SUCCESS;
    }
    // check
    CHECK_AND_RETURN_RET_LOG(totalSizeInframe != 0 && spanSizeInframe != 0 && totalSizeInframe % spanSizeInframe == 0,
        ERR_INVALID_PARAM, "ConfigProcessBuffer failed: ERR_INVALID_PARAM");
    CHECK_AND_RETURN_RET_LOG(serverStreamInfo.samplingRate.size() > 0 && serverStreamInfo.channels.size() > 0,
        ERR_INVALID_PARAM, "Invalid stream info in server");
    uint32_t spanTime = spanSizeInframe * AUDIO_MS_PER_SECOND / *serverStreamInfo.samplingRate.rbegin();
    spanSizeInframe_ = spanTime * processConfig_.streamInfo.samplingRate / AUDIO_MS_PER_SECOND;
    totalSizeInframe_ = totalSizeInframe / spanSizeInframe * spanSizeInframe_;

    uint32_t channel = processConfig_.streamInfo.channels;
    uint32_t formatbyte = PcmFormatToBits(processConfig_.streamInfo.format);
    byteSizePerFrame_ = channel * formatbyte;
    if (*serverStreamInfo.channels.rbegin() != processConfig_.streamInfo.channels ||
        serverStreamInfo.format != processConfig_.streamInfo.format) {
        size_t spanSizeInByte = 0;
        if (processConfig_.audioMode == AUDIO_MODE_PLAYBACK) {
            uint32_t serverByteSize = *serverStreamInfo.channels.rbegin() * PcmFormatToBits(serverStreamInfo.format);
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
        processBuffer_ = OHAudioBuffer::CreateFromLocal(totalSizeInframe_, spanSizeInframe_, byteSizePerFrame_);
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

int32_t AudioProcessInServer::RegisterThreadPriority(uint32_t tid, const std::string &bundleName)
{
    if (!clientThreadPriorityRequested_) {
        clientTid_ = tid;
        clientBundleName_ = bundleName;
        ScheduleReportData(processConfig_.appInfo.appPid, tid, bundleName.c_str());
        return SUCCESS;
    } else {
        AUDIO_ERR_LOG("client thread priority requested");
        return ERR_OPERATION_FAILED;
    }
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

int32_t AudioProcessInServer::SetDefaultOutputDevice(const DeviceType defaultOutputDevice)
{
    return PolicyHandler::GetInstance().SetDefaultOutputDevice(defaultOutputDevice, sessionId_,
        processConfig_.rendererInfo.streamUsage, streamStatus_->load() == STREAM_RUNNING);
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

int64_t AudioProcessInServer::GetLastAudioDuration()
{
    auto ret = lastStopTime_ - lastStartTime_;
    return ret < 0 ? -1 : ret;
}

RestoreStatus AudioProcessInServer::RestoreSession(RestoreInfo restoreInfo)
{
    RestoreStatus restoreStatus = processBuffer_->SetRestoreStatus(NEED_RESTORE);
    if (restoreStatus == NEED_RESTORE) {
        processBuffer_->SetRestoreInfo(restoreInfo);
    }
    return restoreStatus;
}

} // namespace AudioStandard
} // namespace OHOS
