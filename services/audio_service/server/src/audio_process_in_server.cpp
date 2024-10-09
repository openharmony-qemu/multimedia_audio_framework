/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "audio_service_log.h"
#include "audio_service.h"
#include "audio_schedule.h"
#include "audio_utils.h"
#include "media_monitor_manager.h"

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
    if (processConfig.originalSessionId < MIN_SESSIONID || processConfig.originalSessionId > MAX_SESSIONID) {
        sessionId_ = PolicyHandler::GetInstance().GenerateSessionId(processConfig_.appInfo.appUid);
    } else {
        sessionId_ = processConfig.originalSessionId;
    }
}

AudioProcessInServer::~AudioProcessInServer()
{
    AUDIO_INFO_LOG("~AudioProcessInServer()");
    if (convertedBuffer_.buffer != nullptr) {
        delete [] convertedBuffer_.buffer;
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

bool AudioProcessInServer::GetMuteFlag()
{
    return muteFlag_;
}

uint32_t AudioProcessInServer::GetSessionId()
{
    return sessionId_;
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

int32_t AudioProcessInServer::Start()
{
    CHECK_AND_RETURN_RET_LOG(isInited_, ERR_ILLEGAL_STATE, "not inited!");

    std::lock_guard<std::mutex> lock(statusLock_);
    CHECK_AND_RETURN_RET_LOG(streamStatus_->load() == STREAM_STARTING || streamStatus_->load() == STREAM_STAND_BY,
        ERR_ILLEGAL_STATE, "Start failed, invalid status.");

    if (processConfig_.audioMode != AUDIO_MODE_PLAYBACK && !needCheckBackground_ &&
        PermissionUtil::NeedVerifyBackgroundCapture(processConfig_.callerUid, processConfig_.capturerInfo.sourceType)) {
        AUDIO_INFO_LOG("set needCheckBackground_: true");
        needCheckBackground_ = true;
    }
    if (processConfig_.audioMode != AUDIO_MODE_PLAYBACK && needCheckBackground_) {
        CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyBackgroundCapture(processConfig_.appInfo.appTokenId,
            processConfig_.appInfo.appFullTokenId), ERR_OPERATION_FAILED, "VerifyBackgroundCapture failed!");
        CHECK_AND_RETURN_RET_LOG(PermissionUtil::NotifyPrivacy(processConfig_.appInfo.appTokenId,
            AUDIO_PERMISSION_START), ERR_PERMISSION_DENIED, "NotifyPrivacy failed!");
    }

    for (size_t i = 0; i < listenerList_.size(); i++) {
        listenerList_[i]->OnStart(this);
    }

    if (streamStatus_->load() == STREAM_STAND_BY) {
        AUDIO_INFO_LOG("Call start while in stand-by, session %{public}u", sessionId_);
        WriterRenderStreamStandbySysEvent(sessionId_, 0);
        streamStatus_->store(STREAM_STARTING);
    }
    processBuffer_->SetLastWrittenTime(ClockTime::GetCurNano());

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
    if (processConfig_.audioMode != AUDIO_MODE_PLAYBACK && needCheckBackground_) {
        uint32_t tokenId = processConfig_.appInfo.appTokenId;
        PermissionUtil::NotifyPrivacy(tokenId, AUDIO_PERMISSION_STOP);
    }
    for (size_t i = 0; i < listenerList_.size(); i++) {
        listenerList_[i]->OnPause(this);
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
    if (processConfig_.audioMode != AUDIO_MODE_PLAYBACK && !needCheckBackground_ &&
        PermissionUtil::NeedVerifyBackgroundCapture(processConfig_.callerUid, processConfig_.capturerInfo.sourceType)) {
        AUDIO_INFO_LOG("set needCheckBackground_: true");
        needCheckBackground_ = true;
    }
    if (processConfig_.audioMode != AUDIO_MODE_PLAYBACK && needCheckBackground_) {
        uint32_t tokenId = processConfig_.appInfo.appTokenId;
        uint64_t fullTokenId = processConfig_.appInfo.appFullTokenId;
        CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyBackgroundCapture(tokenId, fullTokenId), ERR_OPERATION_FAILED,
            "VerifyBackgroundCapture failed!");
        CHECK_AND_RETURN_RET_LOG(PermissionUtil::NotifyPrivacy(tokenId, AUDIO_PERMISSION_START), ERR_PERMISSION_DENIED,
            "NotifyPrivacy failed!");
    }

    for (size_t i = 0; i < listenerList_.size(); i++) {
        listenerList_[i]->OnStart(this);
    }

    AUDIO_PRERELEASE_LOGI("Resume in server success!");
    return SUCCESS;
}

int32_t AudioProcessInServer::Stop()
{
    CHECK_AND_RETURN_RET_LOG(isInited_, ERR_ILLEGAL_STATE, "not inited!");

    std::lock_guard<std::mutex> lock(statusLock_);
    CHECK_AND_RETURN_RET_LOG(streamStatus_->load() == STREAM_STOPPING,
        ERR_ILLEGAL_STATE, "Stop failed, invalid status.");
    if (processConfig_.audioMode != AUDIO_MODE_PLAYBACK && needCheckBackground_) {
        uint32_t tokenId = processConfig_.appInfo.appTokenId;
        PermissionUtil::NotifyPrivacy(tokenId, AUDIO_PERMISSION_STOP);
    }
    for (size_t i = 0; i < listenerList_.size(); i++) {
        listenerList_[i]->OnPause(this); // notify endpoint?
    }

    AUDIO_INFO_LOG("Stop in server success!");
    return SUCCESS;
}

int32_t AudioProcessInServer::Release()
{
    CHECK_AND_RETURN_RET_LOG(isInited_, ERR_ILLEGAL_STATE, "not inited or already released");
    UnscheduleReportData(processConfig_.appInfo.appPid, clientTid_, clientBundleName_.c_str());
    clientThreadPriorityRequested_ = false;
    isInited_ = false;
    std::lock_guard<std::mutex> lock(statusLock_);
    CHECK_AND_RETURN_RET_LOG(releaseCallback_ != nullptr, ERR_OPERATION_FAILED, "Failed: no service to notify.");

    if (processConfig_.audioMode != AUDIO_MODE_PLAYBACK && needCheckBackground_) {
        uint32_t tokenId = processConfig_.appInfo.appTokenId;
        PermissionUtil::NotifyPrivacy(tokenId, AUDIO_PERMISSION_STOP);
    }
    int32_t ret = releaseCallback_->OnProcessRelease(this);
    AUDIO_INFO_LOG("notify service release result: %{public}d", ret);
    return SUCCESS;
}

ProcessDeathRecipient::ProcessDeathRecipient(AudioProcessInServer *processInServer,
    ProcessReleaseCallback *processHolder)
{
    processInServer_ = processInServer;
    processHolder_ = processHolder;
}

void ProcessDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    CHECK_AND_RETURN_LOG(processHolder_ != nullptr, "processHolder_ is null.");
    int32_t ret = processHolder_->OnProcessRelease(processInServer_);
    AUDIO_INFO_LOG("OnRemoteDied, call release ret: %{public}d", ret);
}

int32_t AudioProcessInServer::RegisterProcessCb(sptr<IRemoteObject> object)
{
    sptr<IProcessCb> processCb = iface_cast<IProcessCb>(object);
    CHECK_AND_RETURN_RET_LOG(processCb != nullptr, ERR_INVALID_PARAM, "RegisterProcessCb obj cast failed");
    bool result = object->AddDeathRecipient(new ProcessDeathRecipient(this, releaseCallback_));
    CHECK_AND_RETURN_RET_LOG(result, ERR_OPERATION_FAILED, "AddDeathRecipient failed.");

    return SUCCESS;
}

void AudioProcessInServer::SetInnerCapState(bool isInnerCapped)
{
    AUDIO_INFO_LOG("process[%{public}u] innercapped: %{public}s", sessionId_, isInnerCapped ? "true" : "false");
    isInnerCapped_ = isInnerCapped;
}

bool AudioProcessInServer::GetInnerCapState()
{
    return isInnerCapped_;
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
    if (*serverStreamInfo.channels.rbegin() != processConfig_.streamInfo.channels) {
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
}
} // namespace AudioStandard
} // namespace OHOS
