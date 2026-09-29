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
#include <unistd.h>
#include <algorithm>
#include <cinttypes>
#include "securec.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "audio_info.h"
#include "audio_capturer_log.h"
#include "audio_service.h"
#include "audio_process_config.h"
#include "i_stream_manager.h"
#include "audio_policy_manager.h"
#ifdef HAS_FEATURE_INNERCAPTURER
#include "playback_capturer_manager.h"
#endif
#include "policy_handler.h"
#include "media_monitor_manager.h"
#include "audio_dump_pcm.h"
#include "volume_tools.h"
#include "core_service_handler.h"
#include "record_privacy_manager.h"
#include "stream_dfx_manager.h"
#include "audio_general_manager.h"
#ifdef FEATURE_CALL_MANAGER
#include "call_manager_client.h"
#endif
#include "audio_privacy_ui_ext_connection.h"
#include "app_mgr_client.h"
#include "extension_manager_client.h"
#include "want.h"
#include "parameters.h"
#include "audio_privacy_manager_in_server.h"

namespace OHOS {
namespace AudioStandard {
const char* CAPTURE_SERVER_PLAYBACK_PERMISSION = "ohos.permission.CAPTURE_PLAYBACK";
const char* CAPTURE_SERVER_PLAYBACK_DOWNLINK_PERMISSION = "ohos.permission.CAPTURE_PLAYBACK_DOWNLINK";
namespace {
    static constexpr int32_t UID_DISTRIBUTED_AUDIO_SA = 3055;
    static constexpr int32_t VOLUME_SHIFT_NUMBER = 16; // 1 >> 16 = 65536, max volume
    static constexpr int32_t UID_AUDIO = 1041;
    static const size_t CAPTURER_BUFFER_DEFAULT_NUM = 4;
    static const size_t CAPTURER_BUFFER_WAKE_UP_NUM = 100;
    static const uint32_t OVERFLOW_LOG_LOOP_COUNT = 100;
    constexpr int32_t RELEASE_TIMEOUT_IN_SEC = 10; // 10S
    constexpr int32_t TOTAL_SIZE_5MS_TO_20MS = 4;
    static const uint32_t OVERFLOW_REPORT_INTERVAL_MS = 60000;
#ifdef FEATURE_CALL_MANAGER
static const int32_t TELEPHONY_CALL_MANAGER_SYS_ABILITY_ID = 4005;
#endif
enum CallbackHandlerEvent : uint32_t {
    NOTIFY_VOIP_START = 0,
};

static constexpr int32_t MAX_AUDIO_CHANNELS = 32;

bool IsCapturerConfigValid(const AudioProcessConfig &config)
{
    if (config.appInfo.appUid < 0) {
        AUDIO_ERR_LOG("Invalid appUid in capturer config: %{public}d", config.appInfo.appUid);
        return false;
    }
    if (config.appInfo.appPid < 0) {
        AUDIO_ERR_LOG("Invalid appPid in capturer config: %{public}d", config.appInfo.appPid);
        return false;
    }
    if (config.streamInfo.samplingRate <= 0) {
        AUDIO_ERR_LOG("Invalid samplingRate in capturer config: %{public}d", config.streamInfo.samplingRate);
        return false;
    }
    if (config.streamInfo.channels <= 0 || config.streamInfo.channels > MAX_AUDIO_CHANNELS) {
        AUDIO_ERR_LOG("Invalid channels in capturer config: %{public}d", config.streamInfo.channels);
        return false;
    }
    return true;
}

void ReportServerWakeupEvent(const AudioProcessConfig &config, AudioWakeupStageCode stage,
    AudioWakeupErrorCode errorCode)
{
    SourceType sourceType = config.capturerInfo.sourceType;
    if (sourceType != SOURCE_TYPE_WAKEUP && sourceType != SOURCE_TYPE_VOICE_RECOGNITION &&
        sourceType != SOURCE_TYPE_UNPROCESSED_VOICE_ASSISTANT) {
        return;
    }
    AudioWakeupTrackInfo info {};
    info.stage = stage;
    info.result = WAKEUP_RESULT_FAIL;
    info.errCode = errorCode;
    (void)AudioService::GetInstance()->ReportWakeupEvent(info);
}
}

CapturerInServer::CapturerInServer(AudioProcessConfig processConfig, std::weak_ptr<IStreamListener> streamListener)
{
    processConfig_ = processConfig;
    streamListener_ = streamListener;
    innerCapId_ = processConfig.innerCapId;
    if (!IsCapturerConfigValid(processConfig)) {
        AUDIO_ERR_LOG("Invalid processConfig for AudioStreamChecker in capturer, using default");
        AudioProcessConfig defaultConfig;
        audioStreamChecker_ = std::make_shared<AudioStreamChecker>(defaultConfig);
    } else {
        audioStreamChecker_ = std::make_shared<AudioStreamChecker>(processConfig);
    }
    deviceId_ = processConfig.appInfo.deviceId;
    AudioStreamMonitor::GetInstance().AddCheckForMonitor(processConfig.originalSessionId, audioStreamChecker_);
}

CapturerInServer::~CapturerInServer()
{
    if (status_ != I_STATUS_RELEASED) {
        Release();
    }
    DumpFileUtil::CloseDumpFile(&dumpS2C_);
    if (isMicIndicatorOn_) {
        TurnOffMicIndicator(CAPTURER_INVALID);
    }
    TurnOffRemoteMicIndicator();
    if (processConfig_.capturerInfo.loopBackEffectEnabled) {
        AudioPolicyManager::GetInstance().SetKaraokeParameters(DEVICE_TYPE_SPEAKER,
            KaraokeParamType::KARAOKE_PARAM_CAPTURE_REVERB, 0);
    }
}

int32_t CapturerInServer::ConfigServerBuffer()
{
    if (audioServerBuffer_ != nullptr) {
        AUDIO_INFO_LOG("ConfigProcessBuffer: process buffer already configed!");
        return SUCCESS;
    }
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(stream_ != nullptr, ERR_OPERATION_FAILED,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_ILLEGAL_STATE, "ConfigServerBuffer failed, stream_ is null", false),
        "ConfigServerBuffer failed, stream_ is null!");
    stream_->GetSpanSizePerFrame(spanSizeInFrame_);
    const size_t bufferNum = ((processConfig_.capturerInfo.sourceType == SOURCE_TYPE_WAKEUP)
        ? CAPTURER_BUFFER_WAKE_UP_NUM : CAPTURER_BUFFER_DEFAULT_NUM);
    totalSizeInFrame_ = spanSizeInFrame_ * bufferNum;
    size_t totalSizeInFrameToCreate = processConfig_.capturerInfo.capturerFlags == AUDIO_FLAG_MMAP ?
        totalSizeInFrame_ * TOTAL_SIZE_5MS_TO_20MS : totalSizeInFrame_;
    stream_->GetByteSizePerFrame(byteSizePerFrame_);
    spanSizeInBytes_ = byteSizePerFrame_ * spanSizeInFrame_;
    AUDIO_INFO_LOG("ConfigProcessBuffer: totalSizeInFrame_: %{public}zu, spanSizeInFrame_: %{public}zu,"
        "byteSizePerFrame_: %{public}zu, spanSizeInBytes_ %{public}zu", totalSizeInFrameToCreate, spanSizeInFrame_,
        byteSizePerFrame_, spanSizeInBytes_);
    if (totalSizeInFrame_ == 0 || spanSizeInFrame_ == 0 || totalSizeInFrame_ % spanSizeInFrame_ != 0) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_INVALID_PARAM, "ConfigProcessBuffer: ERR_INVALID_PARAM", false);
        AUDIO_ERR_LOG("ConfigProcessBuffer: ERR_INVALID_PARAM");
        return ERR_INVALID_PARAM;
    }

    int32_t ret = InitCacheBuffer(2 * spanSizeInBytes_);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_OPERATION_FAILED, "InitCacheBuffer failed", false),
        "InitCacheBuffer failed %{public}d", ret);

    // create OHAudioBuffer in server
    audioServerBuffer_ = OHAudioBuffer::CreateFromLocal(totalSizeInFrameToCreate, spanSizeInFrame_, byteSizePerFrame_);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioServerBuffer_ != nullptr, ERR_OPERATION_FAILED,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_MEMORY_ALLOC_FAILED, "Create oh audio buffer failed", false),
        "Create oh audio buffer failed");

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
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_ILLEGAL_STATE, "InitBufferStatus failed, null buffer", false);
        AUDIO_ERR_LOG("InitBufferStatus failed, null buffer.");
        return ERR_ILLEGAL_STATE;
    }

    uint32_t spanCount = audioServerBuffer_->GetSpanCount();
    AUDIO_INFO_LOG("InitBufferStatus: spanCount %{public}u", spanCount);
    for (uint32_t i = 0; i < spanCount; i++) {
        SpanInfo *spanInfo = audioServerBuffer_->GetSpanInfoByIndex(i);
        if (spanInfo == nullptr) {
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_CREATE_ILLEGAL_STATE, "InitBufferStatus failed, null spaninfo", false);
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
    if (ret != SUCCESS || stream_ == nullptr) {
        ReportServerWakeupEvent(processConfig_, STAGE_SERVER_CREATE_EXIT,
            WAKEUP_TRACK_SERVER_CREATE_STREAM_ERROR);
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_OPERATION_FAILED, "Construct CapturerInServer failed", false);
        return ERR_OPERATION_FAILED;
    }
    streamIndex_ = stream_->GetStreamIndex();
    capturerClock_ = CapturerClockManager::GetInstance().CreateCapturerClock(
        streamIndex_, processConfig_.streamInfo.samplingRate);
    ret = ConfigServerBuffer();
    if (ret != SUCCESS) {
        ReportServerWakeupEvent(processConfig_, STAGE_SERVER_CREATE_EXIT,
            WAKEUP_TRACK_SERVER_BUFFER_ERROR);
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_OPERATION_FAILED, "ConfigServerBuffer failed", false);
        return ERR_OPERATION_FAILED;
    }
    stream_->RegisterStatusCallback(shared_from_this());
    stream_->RegisterReadCallback(shared_from_this());

    traceTag_ = "[" + std::to_string(streamIndex_) + "]CapturerServerOut"; // [100001]CapturerServerOut
    // eg: /data/data/.pulse_dir/10000_100009_capturer_server_out_48000_2_1.pcm
    AudioStreamInfo tempInfo = processConfig_.streamInfo;
    dumpFileName_ = std::to_string(processConfig_.appInfo.appPid) + "_" + std::to_string(streamIndex_)
        + "_capturer_server_out_" + std::to_string(tempInfo.samplingRate) + "_"
        + std::to_string(tempInfo.channels) + "_" + std::to_string(tempInfo.format) + ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileName_, &dumpS2C_);
    recorderDfx_ = std::make_unique<RecorderDfxWriter>(processConfig_.appInfo, streamIndex_);
    AudioService::GetInstance()->RegisterMuteStateChangeCallback(streamIndex_, [this](bool flag) {
        AUDIO_INFO_LOG("recv mute state change flag %{public}d", flag ? 1 : 0);
        muteFlag_ = flag;
    });

    return SUCCESS;
}

// LCOV_EXCL_START
void CapturerInServer::OnStatusUpdate(IOperation operation)
{
    AUDIO_INFO_LOG("CapturerInServer::OnStatusUpdate operation: %{public}d", operation);
    operation_ = operation;
    if (status_ == I_STATUS_RELEASED) {
        AUDIO_WARNING_LOG("Stream already released");
        return;
    }
    std::shared_ptr<IStreamListener> stateListener = streamListener_.lock();
    CHECK_AND_CALL_FUNC_RETURN_LOG((stateListener != nullptr && recorderDfx_ != nullptr),
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "IStreamListener is nullptr", false),
        "IStreamListener is nullptr");
    switch (operation) {
        case OPERATION_UNDERFLOW:
            underflowCount += 1;
            AUDIO_INFO_LOG("Underflow!! underflow count %{public}d", underflowCount);
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
            HandleOperationStopped(CAPTURER_STAGE_PAUSE_OK);
            break;
        case OPERATION_STOPPED:
            status_ = I_STATUS_STOPPED;
            stateListener->OnOperationHandled(STOP_STREAM, 0);
            HandleOperationStopped(CAPTURER_STAGE_STOP_OK);
            break;
        case OPERATION_FLUSHED:
            HandleOperationFlushed();
            stateListener->OnOperationHandled(FLUSH_STREAM, 0);
            break;
        case OPERATION_UPDATE_SPANSIZE:
            RefreshServerBufferForSpanSizeUpdate(stateListener);
            break;
        default:
            AUDIO_INFO_LOG("Invalid operation %{public}u", operation);
            status_ = I_STATUS_INVALID;
    }
}
// LCOV_EXCL_STOP

int32_t CapturerInServer::GetCapturerFlagsByRoute(uint32_t routeFlag)
{
    if (routeFlag & AUDIO_INPUT_FLAG_FAST) {
        if (routeFlag & AUDIO_INPUT_FLAG_VOIP) {
            return AUDIO_FLAG_VOIP_FAST;
        } else {
            return AUDIO_FLAG_MMAP;
        }
    }
    return AUDIO_FLAG_NORMAL;
}

void CapturerInServer::RefreshServerBufferForSpanSizeUpdate(std::shared_ptr<IStreamListener> stateListener)
{
    CHECK_AND_RETURN_LOG(stream_ != nullptr, "RefreshServerBufferForSpanSizeUpdate: stream_ is nullptr");
    AUDIO_INFO_LOG("RefreshServerBufferForSpanSizeUpdate begin, sessionId:%{public}u oldSpan:%{public}zu",
        streamIndex_, spanSizeInFrame_);

    size_t newSpanSizeInFrame = 0;
    size_t newByteSizePerFrame = byteSizePerFrame_;
    stream_->GetSpanSizePerFrame(newSpanSizeInFrame);
    stream_->GetByteSizePerFrame(newByteSizePerFrame);
    AUDIO_INFO_LOG("New spanSize from stream: %{public}zu, byteSizePerFrame:%{public}zu",
        newSpanSizeInFrame, newByteSizePerFrame);

    CHECK_AND_RETURN_LOG(newSpanSizeInFrame != 0,
        "RefreshServerBufferForSpanSizeUpdate: newSpanSizeInFrame is zero");
    CHECK_AND_RETURN_LOG(newByteSizePerFrame != 0,
        "RefreshServerBufferForSpanSizeUpdate: byteSizePerFrame is zero");

    const size_t bufferNum = ((processConfig_.capturerInfo.sourceType == SOURCE_TYPE_WAKEUP) ?
        CAPTURER_BUFFER_WAKE_UP_NUM : CAPTURER_BUFFER_DEFAULT_NUM);
    size_t newTotalSizeInFrame = newSpanSizeInFrame * bufferNum;
    size_t newSpanSizeInBytes = newSpanSizeInFrame * newByteSizePerFrame;

    uint32_t newRouteFlag = stream_->GetRouteFlag();
    int32_t capturerFlags = GetCapturerFlagsByRoute(newRouteFlag);

    if (audioServerBuffer_ != nullptr) {
        AUDIO_INFO_LOG("SetPendingSpanSize: sessionId:%{public}u spanSize:%{public}zu totalSize:%{public}zu "
            "routeFlag:%{public}u", streamIndex_, newSpanSizeInFrame, newTotalSizeInFrame, newRouteFlag);
        audioServerBuffer_->SetPendingSpanSize(newSpanSizeInFrame, newTotalSizeInFrame, newRouteFlag);
        audioServerBuffer_->UpdateSpanBasicInfo(newSpanSizeInFrame, newTotalSizeInFrame, newByteSizePerFrame);
    }

    if (capturerFlags != processConfig_.capturerInfo.capturerFlags) {
        CHECK_AND_RETURN_LOG(stateListener != nullptr, "RefreshServerBufferForSpanSizeUpdate:stateListener is nullptr");
        AUDIO_INFO_LOG("Update processConfig_.capturerInfo.capturerFlags from %{public}u to %{public}u",
            processConfig_.capturerInfo.capturerFlags, capturerFlags);
        processConfig_.capturerInfo.capturerFlags = capturerFlags;
        stateListener->OnOperationHandled(UPDATE_SPANSIZE, capturerFlags == AUDIO_FLAG_NORMAL ?
            FASTSTATUS_NORMAL : FASTSTATUS_FAST);
    }

    spanSizeInFrame_ = newSpanSizeInFrame;
    totalSizeInFrame_ = newTotalSizeInFrame;
    byteSizePerFrame_ = newByteSizePerFrame;
    spanSizeInBytes_ = newSpanSizeInBytes;
    if (ringCache_ != nullptr) {
        int32_t ret = InitCacheBuffer(2 * spanSizeInBytes_);
        CHECK_AND_RETURN_LOG(ret == SUCCESS,
            "RefreshServerBufferForSpanSizeUpdate: InitCacheBuffer failed, ret:%{public}d", ret);
    }

    AUDIO_INFO_LOG("RefreshServerBufferForSpanSizeUpdate success, sessionId:%{public}u newSpan:%{public}zu "
        "totalSize:%{public}zu spanByte:%{public}zu", streamIndex_, spanSizeInFrame_, totalSizeInFrame_,
        spanSizeInBytes_);
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

void CapturerInServer::HandleOperationStopped(CapturerStage stage)
{
    CHECK_AND_CALL_FUNC_RETURN_LOG(recorderDfx_ != nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "recorderDfx nullptr", false),
        "nullptr");
    lastStopTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    recorderDfx_->WriteDfxStopMsg(streamIndex_, stage,
        GetLastAudioDuration(), processConfig_);
}

BufferDesc CapturerInServer::DequeueBuffer(size_t length)
{
    return stream_->DequeueBuffer(length);
}

int32_t CapturerInServer::SetInMainThreadState(bool isInMainThread)
{
    isInMainThread_ = isInMainThread;
    return SUCCESS;
}

int32_t CapturerInServer::OnMetaData(const std::vector<uint8_t> &metaData)
{
    CHECK_AND_RETURN_RET_LOG(!metaData.empty(), ERR_INVALID_PARAM, "metaData is empty");
    std::shared_ptr<IStreamListener> listener = streamListener_.lock();
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERR_OPERATION_FAILED, "IStreamListener is nullptr");
    return listener->OnMetaData(metaData);
}

bool CapturerInServer::IsAppStateBackground(int32_t appPid)
{
    AppExecFwk::AppMgrClient appManager;
    AppExecFwk::RunningProcessInfo infos;
    uint8_t state = 0;
    appManager.GetRunningProcessInfoByPid(appPid, infos);
    state = static_cast<uint8_t>(infos.state_);
    return state == static_cast<uint8_t>(AppExecFwk::AppProcessState::APP_STATE_BACKGROUND);
}

void CapturerInServer::ReportOverflowEvent()
{
    auto now = std::chrono::system_clock::now();
    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    int32_t reportCount = overFlowLogFlag_ - lastOverflowCount_;
    CHECK_AND_CALL_FUNC_RETURN((nowMs - lastOverflowTime_ >= OVERFLOW_REPORT_INTERVAL_MS && reportCount > 0),
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "overflow report interval not met or no overflow", false));
    lastOverflowCount_ = overFlowLogFlag_;
    lastOverflowTime_ = nowMs;
    bool isBackground = IsAppStateBackground(processConfig_.appInfo.appPid);
    AUDIO_INFO_LOG("isInMainThread_: %{public}d, isBackground: %{public}d, Underflow timestamp: %{public}" PRIu64,
        isInMainThread_, isBackground, static_cast<uint64_t>(nowMs));
    int32_t reason = REASON_RECORDING_DEFAULT |
        (isInMainThread_ ? REASON_RECORDING_IN_ARTTS_MAINTHREAD : 0) |
        (isBackground ? REASON_RECORDING_BACKGROUND : 0);

    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::RECORD_ERROR,
        Media::MediaMonitor::EventType::FAULT_EVENT);
    bean->Add("INCOMING_SOURCE", processConfig_.capturerInfo.sourceType);
    bean->Add("INCOMING_PID", processConfig_.appInfo.appPid);
    bean->Add("INCOMING_UID", processConfig_.appInfo.appUid);
    bean->Add("TYPE", RECORD_ERROR_TYPE_OVERFLOW);
    bean->Add("REASON", reason);
    bean->Add("COUNT", reportCount);
    bean->Add("START_TIME", static_cast<uint64_t>(nowMs));
    bean->Add("PIPE_TYPE", processConfig_.capturerInfo.pipeType);
    bean->Add("HDI_ADAPTER", static_cast<int32_t>(HDI_ADAPTER_TYPE_PRIMARY));
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

// LCOV_EXCL_START
bool CapturerInServer::IsReadDataOverFlow(size_t length, uint64_t currentWriteFrame,
    std::shared_ptr<IStreamListener> stateListener)
{
    if (audioServerBuffer_->GetWritableDataFrames() <= static_cast<int32_t>(spanSizeInFrame_)) {
        if (overFlowLogFlag_ == 0) {
            AUDIO_INFO_LOG("OverFlow!!!");
        } else if (overFlowLogFlag_ == OVERFLOW_LOG_LOOP_COUNT) {
            overFlowLogFlag_ = 0;
            lastOverflowCount_ = 0;
        }
        overFlowLogFlag_++;
        ReportOverflowEvent();
        int32_t engineFlag = GetEngineFlag();
        if (engineFlag != 1) {
            BufferDesc dstBuffer = stream_->DequeueBuffer(length);
            stream_->EnqueueBuffer(dstBuffer);
        }
        return true;
    }
    return false;
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
static uint32_t GetByteSizeByFormat(enum AudioSampleFormat format)
{
    uint32_t byteSize = 0;
    switch (format) {
        case AudioSampleFormat::SAMPLE_U8:
            byteSize = BYTE_SIZE_SAMPLE_U8;
            break;
        case AudioSampleFormat::SAMPLE_S16LE:
            byteSize = BYTE_SIZE_SAMPLE_S16;
            break;
        case AudioSampleFormat::SAMPLE_S24LE:
            byteSize = BYTE_SIZE_SAMPLE_S24;
            break;
        case AudioSampleFormat::SAMPLE_S32LE:
            byteSize = BYTE_SIZE_SAMPLE_S32;
            break;
        case AudioSampleFormat::SAMPLE_F32LE:
            byteSize = BYTE_SIZE_SAMPLE_S32;
            break;
        default:
            byteSize = BYTE_SIZE_SAMPLE_S16;
            break;
    }
    return byteSize;
}
// LCOV_EXCL_STOP

void CapturerInServer::UpdateBufferTimeStamp(size_t readLen)
{
    uint64_t timestamp = 0;
    uint32_t sizePerPos = static_cast<uint32_t>(GetByteSizeByFormat(processConfig_.streamInfo.format)) *
        processConfig_.streamInfo.channels;

    curProcessPos_ += lastPosInc_;
    CHECK_AND_CALL_FUNC_RETURN_LOG(readLen >= 0,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_SEND_DATA_INVALID_PARAM, "readLen is illegal", false),
        "readLen is illegal!");
    lastPosInc_ = static_cast<uint64_t>(readLen) / sizePerPos;

    if (capturerClock_ != nullptr) {
        capturerClock_->GetTimeStampByPosition(curProcessPos_, timestamp);
    }

    AUDIO_DEBUG_LOG("update buffer timestamp pos:%{public}" PRIu64 " ts:%{public}" PRIu64,
        curProcessPos_, timestamp);
    audioServerBuffer_->SetTimeStampInfo(curProcessPos_, timestamp);
}

// Handle buffer timestamp and frame information update
void CapturerInServer::HandleBufferTimeStampAndFrame(uint64_t currentWriteFrame,
    uint64_t nextWriteFrame, size_t bufLength)
{
    UpdateBufferTimeStamp(bufLength);
    audioServerBuffer_->SetHandleInfo(currentWriteFrame, ClockTime::GetCurNano());
    audioServerBuffer_->SetCurWriteFrame(nextWriteFrame);
}

void CapturerInServer::RecordOverflowStatus(bool currentStatus)
{
    if (currentStatus == true && currentStatus != lastOverflowStatus_.load()) {
        lastOverflowStatus_.store(currentStatus);
        audioStreamChecker_->RecordOverflowTime(currentStatus);
    }
    if (currentStatus == false && currentStatus != lastOverflowStatus_.load()) {
        lastOverflowStatus_.store(currentStatus);
        audioStreamChecker_->RecordOverflowTime(currentStatus);
    }
}

int32_t CapturerInServer::CheckForReadData(size_t length, uint64_t currentWriteFrame)
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(length >= spanSizeInBytes_, ERR_READ_FAILED,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_SEND_DATA_INVALID_PARAM, "Length is less than spanSizeInBytes", false),
        "Length %{public}zu is less than spanSizeInBytes %{public}zu", length, spanSizeInBytes_);
    std::shared_ptr<IStreamListener> stateListener = streamListener_.lock();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(stateListener != nullptr, ERR_READ_FAILED,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "IStreamListener is nullptr", false),
        "IStreamListener is nullptr");
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(stream_ != nullptr, ERR_READ_FAILED,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_SEND_DATA_ILLEGAL_STATE, "ReadData failed, stream_ is null", false),
        "ReadData failed, stream_ is null");
    if (IsReadDataOverFlow(length, currentWriteFrame, stateListener)) {
        UpdateBufferTimeStamp(length);
        RecordOverflowStatus(true);
        return ERR_READ_FAILED;
    }
    return SUCCESS;
}

// LCOV_EXCL_START
void CapturerInServer::ReadData(size_t length)
{
    // Loopback should not copy data to the buffer
    CHECK_AND_RETURN(!processConfig_.capturerInfo.isLoopback);
    if (audioServerBuffer_ == nullptr) {
        AUDIO_ERR_LOG("CapturerInServer::ReadData audioServerBuffer_ is nullptr");
        return;
    }
    uint64_t currentWriteFrame = audioServerBuffer_->GetCurWriteFrame();
    int32_t ret = CheckForReadData(length, currentWriteFrame);
    CHECK_AND_CALL_FUNC_RETURN(ret == SUCCESS,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_SEND_DATA_ILLEGAL_STATE, "CheckForReadData fail", false));
    RecordOverflowStatus(false);
    Trace trace(traceTag_ + "::ReadData:" + std::to_string(currentWriteFrame));
    OptResult result = ringCache_->GetWritableSize();
    CHECK_AND_CALL_FUNC_RETURN_LOG(result.ret == OPERATION_SUCCESS,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_SEND_DATA_OPERATION_FAILED, "RingCache write invalid size", false),
        "RingCache write invalid size %{public}zu", result.size);
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
    HandleBufferTimeStampAndFrame(currentWriteFrame, nextWriteFrame, dstBuffer.bufLength);
    stream_->EnqueueBuffer(srcBuffer);
}
// LCOV_EXCL_STOP

int32_t CapturerInServer::OnReadData(size_t length)
{
    Trace trace(traceTag_ + "::OnReadData:" + std::to_string(length));
    ReadData(length);
    return SUCCESS;
}

int32_t CapturerInServer::CheckForOnReadData(size_t requestDataLen, uint64_t currentWriteFrame)
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(status_.load() == I_STATUS_STARTED, ERR_READ_FAILED,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_SEND_DATA_ILLEGAL_STATE, "CapturerInServer is not started", false),
        "CapturerInServer is not started");
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(requestDataLen >= spanSizeInBytes_, ERR_READ_FAILED,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_SEND_DATA_INVALID_PARAM, "Length is less than spanSizeInBytes", false),
        "Length %{public}zu is less than spanSizeInBytes %{public}zu", requestDataLen, spanSizeInBytes_);
    std::shared_ptr<IStreamListener> stateListener = streamListener_.lock();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(stateListener != nullptr, ERR_READ_FAILED,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "IStreamListener is nullptr", false),
        "IStreamListener is nullptr");
    if (IsReadDataOverFlow(requestDataLen, currentWriteFrame, stateListener)) {
        UpdateBufferTimeStamp(requestDataLen);
        RecordOverflowStatus(true);
        return ERR_WRITE_FAILED;
    }
    return SUCCESS;
}

// LCOV_EXCL_START
int32_t CapturerInServer::OnReadData(int8_t *outputData, size_t requestDataLen)
{
    // Loopback should not copy data to the buffer
    CHECK_AND_RETURN_RET(!processConfig_.capturerInfo.isLoopback, SUCCESS);
    if (audioServerBuffer_ == nullptr) {
        AUDIO_ERR_LOG("CapturerInServer::OnReadData audioServerBuffer_ is nullptr");
        return ERR_READ_FAILED;
    }
    uint64_t currentWriteFrame = audioServerBuffer_->GetCurWriteFrame();
    int32_t ret = CheckForOnReadData(requestDataLen, currentWriteFrame);
    CHECK_AND_CALL_FUNC_RETURN_RET(ret == SUCCESS, ret == ERR_READ_FAILED ? ERR_READ_FAILED : ERR_WRITE_FAILED,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "CheckForOnReadData fail", false));
    RecordOverflowStatus(false);
    Trace trace("CapturerInServer::ReadData:" + std::to_string(currentWriteFrame));
    OptResult result = ringCache_->GetWritableSize();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERR_READ_FAILED,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_SEND_DATA_OPERATION_FAILED, "RingCache write invalid size", false),
        "RingCache write invalid size %{public}zu", result.size);

    BufferDesc srcBuffer = {reinterpret_cast<uint8_t *>(outputData), requestDataLen, 0};

    ringCache_->Enqueue({srcBuffer.buffer, srcBuffer.bufLength});
    result = ringCache_->GetReadableSize();
    if (result.ret != OPERATION_SUCCESS || result.size < spanSizeInBytes_) {
        return SUCCESS;
    }

    BufferDesc dstBuffer = {nullptr, 0, 0};
    uint64_t curWritePos = audioServerBuffer_->GetCurWriteFrame();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioServerBuffer_->GetWriteBuffer(curWritePos, dstBuffer) >= 0, ERR_READ_FAILED,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_SEND_DATA_OPERATION_FAILED, "GetWriteBuffer failed", false),
        "GetWriteBuffer failed");
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
    HandleBufferTimeStampAndFrame(currentWriteFrame, nextWriteFrame, dstBuffer.bufLength);
    audioStreamChecker_->RecordNormalFrame();
    return SUCCESS;
}
// LCOV_EXCL_STOP

int32_t CapturerInServer::UpdateReadIndex()
{
    AUDIO_DEBUG_LOG("audioServerBuffer_->GetWritableDataFrames(): %{public}d, needStart: %{public}d",
        audioServerBuffer_->GetWritableDataFrames(), needStart);
    return SUCCESS;
}

int32_t CapturerInServer::ResolveBuffer(std::shared_ptr<OHAudioBuffer> &buffer)
{
    buffer = audioServerBuffer_;
    return SUCCESS;
}

int32_t CapturerInServer::GetSessionId(uint32_t &sessionId)
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(stream_ != nullptr, ERR_OPERATION_FAILED,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "GetSessionId failed, stream_ is null", false),
        "GetSessionId failed, stream_ is null");
    sessionId = streamIndex_;
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(sessionId < INT32_MAX, ERR_OPERATION_FAILED,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "GetSessionId failed, sessionId invalid", false),
        "GetSessionId failed, sessionId:%{public}d", sessionId);

    return SUCCESS;
}

int32_t CapturerInServer::SetMuteHint(bool mute)
{
    uint32_t sessionId = 0;
    int32_t ret = GetSessionId(sessionId);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CONFIG_ILLEGAL_STATE, "SetMuteHint failed to get sessionId", false),
        "SetMuteHint failed to get sessionId");

    return CoreServiceHandler::GetInstance().SetCapturerMuteHint(sessionId, mute);
}

bool CapturerInServer::CheckBGCapture()
{
    uint32_t tokenId = processConfig_.appInfo.appTokenId;
    uint64_t fullTokenId = processConfig_.appInfo.appFullTokenId;

    if (PermissionUtil::VerifyBackgroundCapture(tokenId, fullTokenId)) {
        return true;
    }

    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(Util::IsBackgroundSourceType(processConfig_.capturerInfo.sourceType) &&
        AudioService::GetInstance()->InForegroundList(processConfig_.appInfo.appUid), false,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_START_ILLEGAL_STATE, "Check failed", false),
        "Check failed");

    AudioService::GetInstance()->UpdateForegroundState(tokenId, true);
    bool res = PermissionUtil::VerifyBackgroundCapture(tokenId, fullTokenId);
    AUDIO_INFO_LOG("Retry result:%{public}s", (res ? "success" : "fail"));
    AudioService::GetInstance()->UpdateForegroundState(tokenId, false);

    return res;
}

void CapturerInServer::ReportNoBgPermissionEvent()
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::RECORD_ERROR,
        Media::MediaMonitor::EventType::FAULT_EVENT);
    bean->Add("INCOMING_SOURCE", processConfig_.capturerInfo.sourceType);
    bean->Add("INCOMING_PID", processConfig_.appInfo.appPid);
    bean->Add("INCOMING_UID", processConfig_.appInfo.appUid);
    bean->Add("REASON", RECORD_ERROR_NO_BG_PERMISSION);
    bean->Add("TYPE", RECORD_ERROR_TYPE_START_FAIL);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

bool CapturerInServer::CheckBgRecordPermission(CapturerState capturerState)
{
    if (!CheckBGCapture()) {
        AUDIO_ERR_LOG("Verify failed, incoming: sourceType %{public}d pid %{public}d uid %{public}d",
            processConfig_.capturerInfo.sourceType, processConfig_.appInfo.appPid, processConfig_.appInfo.appUid);
        ReportNoBgPermissionEvent();
        return false;
    }
    return true;
}
 
void CapturerInServer::HandleMultiAncoCapturer(const std::vector<std::string> &bundleNameList)
{
    bool ret = AudioPrivacyManagerInServer::GetInstance().UpdateActiveAncoCapturer(bundleNameList);
    CHECK_AND_RETURN_LOG(ret, "UpdateActiveAncoCapturer failed, need mute anco capturer");
}

bool CapturerInServer::TurnOnMicIndicator()
{
    uint32_t tokenId = processConfig_.effectivePrivacyTokenId;
    if (isMicIndicatorOn_) {
        AUDIO_WARNING_LOG("MicIndicator:already on, Stream:%{public}u.", streamIndex_);
    } else {
        bool startMicResult = AudioPrivacyManagerInServer::GetInstance().GetStartMicResult();
        if (processConfig_.appInfo.appUid == UID_AUDIO) {
            CHECK_AND_RETURN_RET_LOG(startMicResult, false, "Anco NotifyPrivacyStart failed!");
        } else {
            CHECK_AND_CALL_FUNC_RETURN_RET_LOG(
                RecordPrivacyManager::GetInstance().NotifyPrivacyStart(tokenId, streamIndex_), false,
                StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                    RECORD_START_ILLEGAL_STATE, "NotifyPrivacyStart failed", false),
                "NotifyPrivacyStart failed!");
        }
        AUDIO_INFO_LOG("MicIndicator:turn on,Stream:%{public}u", streamIndex_);
        isMicIndicatorOn_ = true;
    }
    return true;
}

bool CapturerInServer::TurnOffMicIndicator(CapturerState capturerState)
{
    uint32_t tokenId = processConfig_.effectivePrivacyTokenId;
    if (isMicIndicatorOn_) {
        bool ret = RecordPrivacyManager::GetInstance().NotifyPrivacyStop(tokenId, streamIndex_);
        if (!ret) {
            AUDIO_ERR_LOG("NotifyPrivacyStop failed!");
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_STOP_ILLEGAL_STATE, "NotifyPrivacyStop failed", false);
        }
        AUDIO_INFO_LOG("MicIndicator:turn off, Stream:%{public}u", streamIndex_);
        isMicIndicatorOn_ = false;
        return ret;
    } else {
        AUDIO_WARNING_LOG("MicIndicator:already off, Stream:%{public}u", streamIndex_);
    }
    return true;
}

int32_t CapturerInServer::Start(bool hasNoCallback)
{
    if (processConfig_.capturerInfo.sourceType == SOURCE_TYPE_VOICE_DOWNLINK) {
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(CheckVoiceDownlinkPermission(), ERR_ILLEGAL_STATE,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_OPERATION_FAILED, "Voice downlink permission check failed", false),
            "Voice downlink permission check failed");
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(sensitiveRecordPermitted_.load(), ERR_ILLEGAL_STATE,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_OPERATION_FAILED, "Sensitive record not permitted", false),
            "Sensitive record not permitted");
    }
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(processConfig_.capturerInfo.sourceType != SOURCE_TYPE_PLAYBACK_CAPTURE ||
        processConfig_.isModernInnerCapturer == false || hasRequestUserPrivacyAuthority_ == true,
        ERR_INVALID_OPERATION,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_START_ILLEGAL_STATE, "New Innercapturer mode and not requestUserPrivacyAuthority", false),
        "New Innercapturer mode and not requestUserPrivacyAuthority");
    if (stream_ != nullptr) {
        stream_->TriggerAppsUidUpdate();
    }
    AudioXCollie audioXCollie(
        "CapturerInServer::Start", RELEASE_TIMEOUT_IN_SEC, nullptr, nullptr,
            AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    int32_t ret = StartInner(hasNoCallback);
    CapturerStage stage = ret == SUCCESS ? CAPTURER_STAGE_START_OK : CAPTURER_STAGE_START_FAIL;
    if (processConfig_.capturerInfo.sourceType == SOURCE_TYPE_VOICE_DOWNLINK) {
        auto audioGeneralManager = AudioGeneralManager::GetInstance();
        audioGeneralManager->SetAsrVoiceMuteMode(AsrVoiceMuteMode::CALL_MUTE, false);
        AUDIO_INFO_LOG("Start() set AsrVoiceMuteMode: CALL_MUTE");
    }
    if (recorderDfx_) {
        recorderDfx_->WriteDfxStartMsg(streamIndex_, stage, processConfig_);
    }
    if (ret == SUCCESS) {
        StreamDfxManager::GetInstance().CheckStreamOccupancy(streamIndex_, processConfig_, true);
        CaptureConcurrentCheck(streamIndex_);
    }
    audioStreamChecker_->MonitorOnAllCallback(AUDIO_STREAM_START, false);
    return ret;
}

// LCOV_EXCL_START
int32_t CapturerInServer::StartInner(bool hasNoCallback)
{
    needStart = 0;
    std::unique_lock<std::mutex> lock(statusLock_);

    if (status_ != I_STATUS_IDLE && status_ != I_STATUS_PAUSED && status_ != I_STATUS_STOPPED) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_START_ILLEGAL_STATE, "CapturerInServer::Start failed, Illegal state", false);
        AUDIO_ERR_LOG("CapturerInServer::Start failed, Illegal state: %{public}u", status_.load());
        return ERR_ILLEGAL_STATE;
    }

    // Loopback capturer should bypass background recording permission check (hardware passthrough)
    if (!needCheckBackground_ && !processConfig_.capturerInfo.isLoopback &&
        PermissionUtil::NeedVerifyBackgroundCapture(processConfig_.callerUid,
        processConfig_.capturerInfo.sourceType)) {
        AUDIO_INFO_LOG("set needCheckBackground_: true");
        needCheckBackground_ = true;
    }
    if (needCheckBackground_) {
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(CheckBgRecordPermission(CAPTURER_RUNNING), ERR_PERMISSION_DENIED,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_START_ILLEGAL_STATE, "Check backgroud capture failed", false),
            "Check backgroud capture failed for stream:%{public}d!", streamIndex_);
    }
    if (RecordPrivacyManager::GetInstance().NeedTurnOnMicIndicator(processConfig_.callerUid,
        processConfig_.capturerInfo.isLoopback, processConfig_.capturerInfo.sourceType)) {
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(TurnOnMicIndicator(), ERR_PERMISSION_DENIED,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_START_ILLEGAL_STATE, "Turn on micIndicator failed", false),
            "Turn on micIndicator failed for stream:%{public}d!", streamIndex_);
    }
    TurnOnRemoteMicIndicator();
    NotifyVoIPStart(processConfig_.capturerInfo.sourceType, processConfig_.appInfo.appUid);

    if (processConfig_.capturerInfo.sourceType != SOURCE_TYPE_PLAYBACK_CAPTURE) {
        CoreServiceHandler::GetInstance().UpdateSessionOperation(streamIndex_, SESSION_OPERATION_START,
            hasNoCallback ? SESSION_OP_MSG_FAST_CHANGE_TO_NORMAL : SESSION_OP_MSG_DEFAULT);
    }

    status_ = I_STATUS_STARTING;
    if (processConfig_.capturerInfo.sourceType == SOURCE_TYPE_PLAYBACK_CAPTURE) {
        PlaybackCapturerManager::GetInstance()->InitAllDupBuffer(innerCapId_);
    }
    return StartStream();
}

int32_t CapturerInServer::StartStream()
{
    int32_t ret = stream_->Start();
    if (ret != SUCCESS) {
        ReportServerWakeupEvent(processConfig_, STAGE_SERVER_START_EXIT, WAKEUP_TRACK_SERVER_START_ERROR);
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_START_OPERATION_FAILED, "Start stream failed", false);
        return ret;
    }
    resetTime_ = true;
    RebuildCaptureInjector();

    if (capturerClock_ != nullptr) {
        capturerClock_->Start();
    }

    return SUCCESS;
}
// LCOV_EXCL_STOP

void CapturerInServer::RebuildCaptureInjector()
{
    CHECK_AND_CALL_FUNC_RETURN_LOG(rebuildFlag_,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CONFIG_ILLEGAL_STATE, "no need to rebuild", false),
        "no need to rebuild");
    if (processConfig_.capturerInfo.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION) {
        CoreServiceHandler::GetInstance().RebuildCaptureInjector(streamIndex_);
    }
    rebuildFlag_ = false;
}

// LCOV_EXCL_START
int32_t CapturerInServer::Pause()
{
    AudioXCollie audioXCollie(
        "CapturerInServer::Pause", RELEASE_TIMEOUT_IN_SEC, nullptr, nullptr,
            AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    std::unique_lock<std::mutex> lock(statusLock_);

    if (status_ != I_STATUS_STARTED) {
        AUDIO_ERR_LOG("failed, Illegal state: %{public}u", status_.load());
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_PAUSE_ILLEGAL_STATE, "Pause failed, Illegal state", false);
        return ERR_ILLEGAL_STATE;
    }
    if (isMicIndicatorOn_) {
        TurnOffMicIndicator(CAPTURER_PAUSED);
    }
    TurnOffRemoteMicIndicator();
    status_ = I_STATUS_PAUSING;
    int ret = stream_->Pause();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_PAUSE_OPERATION_FAILED, "Pause stream failed", false),
        "Pause stream failed, reason: %{public}d", ret);
    CoreServiceHandler::GetInstance().UpdateSessionOperation(streamIndex_, SESSION_OPERATION_PAUSE);
    StreamDfxManager::GetInstance().CheckStreamOccupancy(streamIndex_, processConfig_, false);
    CaptureConcurrentCheck(streamIndex_);
    if (capturerClock_ != nullptr) {
        capturerClock_->Stop();
    }
    audioStreamChecker_->MonitorOnAllCallback(AUDIO_STREAM_PAUSE, false);
    return SUCCESS;
}
// LCOV_EXCL_STOP

void CapturerInServer::TurnOnRemoteMicIndicator()
{
    CHECK_AND_CALL_FUNC_RETURN(processConfig_.callerUid == UID_DISTRIBUTED_AUDIO_SA && !isRemoteMicIndicatorOn_,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_DEVICE_SWITCH_ILLEGAL_STATE, "callerUid not match or indicator already on", false));
    std::string deviceName = "";
    AudioService::GetInstance()->GetRemoteDeviceName(deviceId_, deviceName);
    PermissionUtil::StartRemoteUsingPermission(deviceId_, deviceName, MICROPHONE_PERMISSION);
    PermissionUtil::AddRemotePermissionUsedRecord(deviceId_, deviceName, MICROPHONE_PERMISSION, 1, 0);
    isRemoteMicIndicatorOn_ = true;
}

void CapturerInServer::TurnOffRemoteMicIndicator()
{
    CHECK_AND_CALL_FUNC_RETURN(processConfig_.callerUid == UID_DISTRIBUTED_AUDIO_SA && isRemoteMicIndicatorOn_,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_DEVICE_SWITCH_ILLEGAL_STATE, "callerUid not match or indicator not on", false));
    std::string deviceName = "";
    AudioService::GetInstance()->GetRemoteDeviceName(deviceId_, deviceName);
    PermissionUtil::StopRemoteUsingPermission(deviceId_, deviceName, MICROPHONE_PERMISSION);
    isRemoteMicIndicatorOn_ = false;
}

int32_t CapturerInServer::Flush()
{
    AudioXCollie audioXCollie(
        "CapturerInServer::Flush", RELEASE_TIMEOUT_IN_SEC, nullptr, nullptr,
            AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    std::unique_lock<std::mutex> lock(statusLock_);
    if (status_ == I_STATUS_STARTED) {
        status_ = I_STATUS_FLUSHING_WHEN_STARTED;
    } else if (status_ == I_STATUS_PAUSED) {
        status_ = I_STATUS_FLUSHING_WHEN_PAUSED;
    } else if (status_ == I_STATUS_STOPPED) {
        status_ = I_STATUS_FLUSHING_WHEN_STOPPED;
    } else {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_FLUSH_ILLEGAL_STATE, "Flush failed, Illegal state", false);
        AUDIO_ERR_LOG("failed, Illegal state: %{public}u", status_.load());
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
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_FLUSH_ILLEGAL_STATE, "Flush stream failed", false),
        "Flush stream failed, reason: %{public}d", ret);
    return SUCCESS;
}

int32_t CapturerInServer::DrainAudioBuffer()
{
    return SUCCESS;
}

// LCOV_EXCL_START
int32_t CapturerInServer::Stop()
{
    AudioXCollie audioXCollie(
        "CapturerInServer::Stop", RELEASE_TIMEOUT_IN_SEC, nullptr, nullptr,
            AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    std::unique_lock<std::mutex> lock(statusLock_);
    if (status_ != I_STATUS_STARTED && status_ != I_STATUS_PAUSED) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_STOP_ILLEGAL_STATE, "Stop failed, Illegal state", false);
        AUDIO_ERR_LOG("failed, Illegal state: %{public}u", status_.load());
        return ERR_ILLEGAL_STATE;
    }
    status_ = I_STATUS_STOPPING;

    if (capturerClock_ != nullptr) {
        capturerClock_->Stop();
    }
    if (isMicIndicatorOn_) {
        TurnOffMicIndicator(CAPTURER_STOPPED);
    }
    TurnOffRemoteMicIndicator();
    int ret = stream_->Stop();
    if (ret != SUCCESS) {
        ReportServerWakeupEvent(processConfig_, STAGE_SERVER_STOP_EXIT, WAKEUP_TRACK_SERVER_STOP_ERROR);
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_STOP_OPERATION_FAILED, "Stop stream failed", false);
        return ret;
    }
    CoreServiceHandler::GetInstance().UpdateSessionOperation(streamIndex_, SESSION_OPERATION_STOP);
    StreamDfxManager::GetInstance().CheckStreamOccupancy(streamIndex_, processConfig_, false);
    CaptureConcurrentCheck(streamIndex_);
    audioStreamChecker_->MonitorOnAllCallback(AUDIO_STREAM_STOP, false);
    return SUCCESS;
}
// LCOV_EXCL_STOP

int32_t CapturerInServer::Release(bool isSwitchStream)
{
    AudioXCollie audioXCollie("CapturerInServer::Release", RELEASE_TIMEOUT_IN_SEC,
        nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    AudioService::GetInstance()->RemoveCapturer(streamIndex_, isSwitchStream);
    ReleaseCallbackHandler();
    std::unique_lock<std::mutex> lock(statusLock_);
    if (status_ == I_STATUS_RELEASED) {
        AUDIO_INFO_LOG("Already released");
        return SUCCESS;
    }
    lock.unlock();
    AUDIO_INFO_LOG("Start release capturer");
    AudioStreamMonitor::GetInstance().DeleteCheckForMonitor(processConfig_.originalSessionId);

    StreamDfxManager::GetInstance().CheckStreamOccupancy(streamIndex_, processConfig_, false);
    int32_t ret = IStreamManager::GetRecorderManager().ReleaseCapturer(streamIndex_);
    if (ret < 0) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_RELEASE_ILLEGAL_STATE, "Release stream failed", false);
        AUDIO_ERR_LOG("Release stream failed, reason: %{public}d", ret);
        status_ = I_STATUS_INVALID;
        return ret;
    }

    bool needRemoveFromMap = !isSwitchStream;
    if (processConfig_.capturerInfo.sourceType != SOURCE_TYPE_PLAYBACK_CAPTURE) {
        int32_t result = CoreServiceHandler::GetInstance().UpdateSessionOperation(
            streamIndex_, SESSION_OPERATION_RELEASE, SESSION_OP_MSG_DEFAULT, needRemoveFromMap);
        CHECK_AND_CALL_FUNC_RETURN_RET_REPORT(result == SUCCESS, result,
            HILOG_COMM_ERROR("[Release]Policy remove client failed, reason: %{public}d", result),
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_RELEASE_ILLEGAL_STATE, "[Release]Policy remove client failed", true));
    }

    if (status_ != I_STATUS_STOPPING &&
        status_ != I_STATUS_STOPPED) {
        HandleOperationStopped(CAPTURER_STAGE_STOP_BY_RELEASE);
    }
    status_ = I_STATUS_RELEASED;

    if (processConfig_.capturerInfo.isLoopback) {
        AudioService::GetInstance()->SetDecMaxLoopbackStreamCnt(processConfig_.audioMode);
        AudioService::GetInstance()->DisableLoopback();
    }

    CapturerClockManager::GetInstance().DeleteCapturerClock(streamIndex_);
#ifdef HAS_FEATURE_INNERCAPTURER
    if (processConfig_.capturerInfo.sourceType == SOURCE_TYPE_PLAYBACK_CAPTURE) {
        AUDIO_INFO_LOG("Disable inner capturer for %{public}u, innerCapId :%{public}d, innerCapMode:%{public}d",
            streamIndex_, innerCapId_, processConfig_.innerCapMode);
        if (processConfig_.innerCapMode == MODERN_INNER_CAP) {
            PlaybackCapturerManager::GetInstance()->RemovePlaybackCapturerFilterInfo(streamIndex_, innerCapId_);
        } else {
            PlaybackCapturerManager::GetInstance()->SetInnerCapturerState(false);
        }
        if (PlaybackCapturerManager::GetInstance()->CheckReleaseUnloadModernInnerCapSink(innerCapId_)) {
            AudioService::GetInstance()->UnloadModernInnerCapSink(innerCapId_);
        }
        if (PlaybackCapturerManager::GetInstance()->CheckReleaseUnloadModernOffloadCapSource()) {
            AudioService::GetInstance()->UnloadModernOffloadCapSource();
        }
        innerCapId_ = 0;
    }
#endif
    if (isMicIndicatorOn_) {
        TurnOffMicIndicator(CAPTURER_RELEASED);
    }
    TurnOffRemoteMicIndicator();
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
    std::lock_guard<std::mutex> lock(filterConfigLock_);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(processConfig_.capturerInfo.sourceType == SOURCE_TYPE_PLAYBACK_CAPTURE,
        ERR_INVALID_OPERATION,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CONFIG_NOT_SUPPORTED, "This not a inner-cap source", false),
        "This not a inner-cap source!");

    AUDIO_INFO_LOG("Client using config: %{public}s", ProcessConfig::DumpInnerCapConfig(config).c_str());

    for (auto &usg : config.filterOptions.usages) {
        if (usg != STREAM_USAGE_VOICE_COMMUNICATION && usg != STREAM_USAGE_VIDEO_COMMUNICATION) {
            continue;
        }

        if (!PermissionUtil::VerifyPermission(CAPTURER_VOICE_DOWNLINK_PERMISSION, processConfig_.appInfo.appTokenId)) {
            AUDIO_ERR_LOG("downlink capturer permission check failed");
            return ERR_PERMISSION_DENIED;
        }
    }
    if (config.filterOptions.capturePrivatePrivacyStreams) {
        CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyCapturePrivatePlayback(processConfig_.appInfo.appTokenId),
            ERR_PERMISSION_DENIED,
            "capture private playback permission check failed");
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

// LCOV_EXCL_START
int32_t CapturerInServer::GetAudioTime(uint64_t &framePos, uint64_t &timestamp)
{
    if (status_ == I_STATUS_STOPPED) {
        AUDIO_WARNING_LOG("Current status is stopped");
        return ERR_ILLEGAL_STATE;
    }
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(stream_ != nullptr, ERR_OPERATION_FAILED,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "GetAudioTime failed, stream_ is null", false),
        "GetAudioTime failed, stream_ is null");
    stream_->GetStreamFramesRead(framePos);
    stream_->GetCurrentTimeStamp(timestamp);
    if (resetTime_) {
        resetTime_ = false;
        resetTimestamp_ = timestamp;
    }
    return SUCCESS;
}
// LCOV_EXCL_STOP

int32_t CapturerInServer::GetLatency(uint64_t &latency)
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(stream_ != nullptr, ERR_OPERATION_FAILED,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "GetLatency failed, stream_ is null", false),
        "GetLatency failed, stream_ is null");
    return stream_->GetLatency(latency);
}

int32_t CapturerInServer::RequestUserPrivacyAuthority(uint32_t sessionId)
{
    AUDIO_INFO_LOG("RequestUserPrivacyAuthority called");

    std::shared_ptr<IStreamListener> stateListener = streamListener_.lock();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(stateListener != nullptr, ERR_OPERATION_FAILED,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "IStreamListener is nullptr", false),
        "IStreamListener is nullptr");

    uint32_t tokenId = IPCSkeleton::GetCallingTokenID();
    if (PermissionUtil::VerifyPermission(CAPTURE_SERVER_PLAYBACK_PERMISSION, tokenId) ||
        PermissionUtil::VerifyPermission(CAPTURER_VOICE_DOWNLINK_PERMISSION, tokenId)) {
        hasRequestUserPrivacyAuthority_ = true;
        stateListener->OnOperationHandled(USER_PRIVACY_AUTHORITY, START_STATE_SUCCESS);
        AUDIO_INFO_LOG("Has static permission, granted");
        return SUCCESS;
    }

    int32_t ret = StartPrivacyConfirmDialog(sessionId);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Start privacy window failed, ret=%{public}d", ret);
        hasRequestUserPrivacyAuthority_ = false;
        stateListener->OnOperationHandled(USER_PRIVACY_AUTHORITY, START_STATE_NOT_AUTHORIZED);
        return ERR_OPERATION_FAILED;
    }

    AUDIO_INFO_LOG("Privacy window started, waiting for user choice");
    return SUCCESS;
}

int32_t CapturerInServer::InitCacheBuffer(size_t targetSize)
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(spanSizeInBytes_ != 0, ERR_OPERATION_FAILED,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_ILLEGAL_STATE, "spanSizeInByte_ invalid", false),
        "spanSizeInByte_ invalid");

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
        AUDIO_INFO_LOG("RestoreSession streamIndex:%{public}u uid:%{public}d tokenId:%{public}u "
            "Reason:NEED_RESTORE", streamIndex_, processConfig_.callerUid,
            processConfig_.appInfo.appTokenId);

        audioServerBuffer_->SetRestoreInfo(restoreInfo);
        audioServerBuffer_->WakeFutex(IS_PRE_EXIT);
    }
    return restoreStatus;
}

void  CapturerInServer::SetRebuildFlag()
{
    rebuildFlag_ = true;
}

int64_t CapturerInServer::GetLastAudioDuration()
{
    auto ret = lastStopTime_ - lastStartTime_;
    return ret < 0 ? -1 : ret;
}

int32_t CapturerInServer::StopSession()
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioServerBuffer_ != nullptr, ERR_INVALID_PARAM,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_STOP_ILLEGAL_STATE, "audioServerBuffer_ is nullptr", false),
        "audioServerBuffer_ is nullptr");
    audioServerBuffer_->SetStopFlag(true);
    return SUCCESS;
}

int32_t CapturerInServer::ResolveBufferBaseAndGetServerSpanSize(std::shared_ptr<OHAudioBufferBase> &buffer,
    uint32_t &spanSizeInFrame, uint64_t &engineTotalSizeInFrame)
{
    return ERR_NOT_SUPPORTED;
}

// LCOV_EXCL_START
inline void CapturerInServer::CaptureConcurrentCheck(uint32_t streamIndex)
{
    if (lastStatus_ == status_) {
        return;
    }
    std::atomic_store(&lastStatus_, status_);
    int32_t ret = CoreServiceHandler::GetInstance().CaptureConcurrentCheck(streamIndex);
    AUDIO_INFO_LOG("ret:%{public}d streamIndex_:%{public}d status_:%{public}u", ret, streamIndex, status_.load());
}

void CapturerInServer::InitCallbackHandler()
{
    CHECK_AND_CALL_FUNC_RETURN(status_ != I_STATUS_RELEASED,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "status already released", false));
    std::lock_guard<std::mutex> lock(runnerMutex_);
    CHECK_AND_CALL_FUNC_RETURN(callbackHandler_ == nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_NULL_POINTER, "callbackHandler already exists", false));
    callbackHandler_ = CallbackHandler::GetInstance(shared_from_this(),
        "OS_AudioCallbackHandler", false);
}

void CapturerInServer::ReleaseCallbackHandler()
{
    std::lock_guard<std::mutex> lock(runnerMutex_);
    CHECK_AND_CALL_FUNC_RETURN(callbackHandler_ != nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_NULL_POINTER, "callbackHandler is nullptr", false));
    callbackHandler_->ReleaseEventRunner();
    callbackHandler_ = nullptr;
}

void CapturerInServer::OnHandle(uint32_t code, int64_t data)
{
    AUDIO_DEBUG_LOG("On handle event, event code: %{public}d, data: %{public}" PRIu64 "", code, data);
    switch (code) {
        case NOTIFY_VOIP_START : {
#ifdef FEATURE_CALL_MANAGER
            std::shared_ptr<Telephony::CallManagerClient> callManager =
                DelayedSingleton<Telephony::CallManagerClient>::GetInstance();
            CHECK_AND_CALL_FUNC_RETURN_LOG(callManager != nullptr,
                StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                    RECORD_CALLBACK_ILLEGAL_STATE, "get callmanager failed", false),
                "get callmanager failed");
            callManager->Init(TELEPHONY_CALL_MANAGER_SYS_ABILITY_ID);
            int32_t ret = callManager->NotifyVoIPAudioStreamStart(data);
            CHECK_AND_CALL_FUNC_RETURN_LOG(ret == SUCCESS,
                StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                    RECORD_CALLBACK_OPERATION_FAILED, "NotifyVoIPAudioStreamStart failed", false),
                "NotifyVoIPAudioStreamStart failed, ret:%{public}d", ret);
#endif
            break;
        }
        default :
            break;
    }
}

void CapturerInServer::NotifyVoIPStart(SourceType sourceType, int32_t uid)
{
    CHECK_AND_CALL_FUNC_RETURN(sourceType == SOURCE_TYPE_VOICE_COMMUNICATION,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CONFIG_INVALID_PARAM, "sourceType not VOICE_COMMUNICATION", false));
    InitCallbackHandler();
    std::lock_guard<std::mutex> lock(runnerMutex_);
    CHECK_AND_CALL_FUNC_RETURN(callbackHandler_ != nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_NULL_POINTER, "callbackHandler is nullptr", false));
    callbackHandler_->SendCallbackEvent(NOTIFY_VOIP_START, uid);
}

int32_t CapturerInServer::StartPrivacyConfirmDialog(uint32_t sessionId)
{
    AAFwk::Want want;
    std::string bundleName =
        system::GetParameter("const.multimedia.screencapture.dialogconnectionbundlename", "default");
    std::string abilityName =
        system::GetParameter("const.multimedia.screencapture.dialogconnectionabilityname", "default");
    if (bundleName == "default" || abilityName == "default") {
        AUDIO_ERR_LOG("Failed to get want params");
        return ERR_NOT_SUPPORTED;
    }
    AUDIO_INFO_LOG("StartPrivacyConfirmDialog bundleName:%{public}s abilityName:%{public}s",
        bundleName.c_str(), abilityName.c_str());

    want.SetElementName(bundleName, abilityName);
    auto connectionHolder = std::make_shared<sptr<AudioPrivacyUIExtConnection>>();
    *connectionHolder = sptr<AudioPrivacyUIExtConnection>(
        new (std::nothrow) AudioPrivacyUIExtConnection(sessionId, IPCSkeleton::GetCallingUid())
    );

    if (*connectionHolder == nullptr) {
        AUDIO_ERR_LOG("Failed to create connection");
        return ERR_NO_MEMORY;
    }

    auto ret = OHOS::AAFwk::ExtensionManagerClient::GetInstance()
        .ConnectServiceExtensionAbility(want, *connectionHolder, nullptr, -1);
    if (ret != 0) {
        AUDIO_ERR_LOG("Connection failed");
        *connectionHolder = nullptr;
        return ret;
    }
    return ret;
}
 
void CapturerInServer::OnPrivacyAuthorityResult(bool isAllowed)
{
    std::shared_ptr<IStreamListener> stateListener = streamListener_.lock();
    CHECK_AND_CALL_FUNC_RETURN_LOG(stateListener != nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "IStreamListener is nullptr", false),
        "IStreamListener is nullptr");

    if (isAllowed) {
        hasRequestUserPrivacyAuthority_ = true;
        stateListener->OnOperationHandled(USER_PRIVACY_AUTHORITY, START_STATE_SUCCESS);
        AUDIO_INFO_LOG("User granted audio privacy authority");
    } else {
        hasRequestUserPrivacyAuthority_ = false;
        stateListener->OnOperationHandled(USER_PRIVACY_AUTHORITY, START_STATE_NOT_AUTHORIZED);
        AUDIO_WARNING_LOG("User denied audio privacy authority");
    }
}
 
int32_t CapturerInServer::ReportPlaybackCaptureUserChoice(uint32_t sessionId, bool choice)
{
    uint32_t curSessionId = 0;
    int32_t ret = GetSessionId(curSessionId);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "failed to get current sessionId", false),
        "failed to get current sessionId");
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(curSessionId == sessionId, ERR_INVALID_PARAM,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_INVALID_PARAM, "illegal sessionId", false),
        "illegal sessionId ");
    AUDIO_INFO_LOG("ReportPlaybackCaptureUserChoice choice:%{public}d", choice);
    OnPrivacyAuthorityResult(choice);
    return SUCCESS;
}

bool CapturerInServer::CheckVoiceDownlinkPermission()
{
    bool hasAclPermission = PermissionUtil::VerifyPermission(
        CAPTURE_SERVER_PLAYBACK_DOWNLINK_PERMISSION, IPCSkeleton::GetCallingTokenID());
    CHECK_AND_RETURN_RET_LOG(hasAclPermission, false,
        "CheckVoiceDownlinkPermission failed: no ACL permission");
    CHECK_AND_RETURN_RET_LOG(!processConfig_.capturerInfo.cellularRecordPhoneNum.empty() &&
        !processConfig_.capturerInfo.cellularRecordToken.empty(), false,
        "CheckCallRecordingPermission failed: phoneNum or token is empty");

#ifdef FEATURE_CALL_MANAGER
    std::shared_ptr<Telephony::CallManagerClient> callManager =
        DelayedSingleton<Telephony::CallManagerClient>::GetInstance();
    CHECK_AND_RETURN_RET_LOG(callManager != nullptr, false, "Get CallManagerClient failed");

    callManager->Init(TELEPHONY_CALL_MANAGER_SYS_ABILITY_ID);

    bool isValid = callManager->CheckCallRecordingPermission(
        processConfig_.capturerInfo.cellularRecordPhoneNum,
        processConfig_.capturerInfo.cellularRecordToken);

    CHECK_AND_RETURN_RET_LOG(isValid, false, "");
    return true;
#else
    AUDIO_INFO_LOG("CheckVoiceDownlinkPermission success!");
    return true;
#endif
}

void CapturerInServer::NotifySensitiveRecordPermit(const bool isPermitted)
{
    AUDIO_INFO_LOG("Set sensitive record isPermitted: %{public}d", isPermitted);
    sensitiveRecordPermitted_ = isPermitted;
    std::shared_ptr<IStreamListener> stateListener = streamListener_.lock();
    CHECK_AND_RETURN_LOG(stateListener != nullptr, "IStreamListener is nullptr");
    stateListener->OnOperationHandled(SENSITIVE_RECORD_PERMIT, isPermitted ? 1 : 0);
}

// LCOV_EXCL_STOP
} // namespace AudioStandard
} // namespace OHOS
