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
#define LOG_TAG "RendererInServer"
#endif

#include "renderer_in_server.h"
#include <chrono>
#include <cinttypes>
#include "securec.h"
#include <format>
#include "audio_errors.h"
#include "audio_renderer_log.h"
#include "audio_utils.h"
#include "audio_service.h"
#include "futex_tool.h"
#include "xperf_adapter.h"
#include "i_stream_manager.h"
#ifdef RESSCHE_ENABLE
#include "res_type.h"
#include "res_sched_client.h"
#endif
#include "volume_tools.h"
#include "policy_handler.h"
#include "audio_enhance_chain_manager.h"
#include "media_monitor_manager.h"
#include "audio_volume.h"
#include "audio_dump_pcm.h"
#include "audio_performance_monitor.h"
#include "audio_volume_c.h"
#include "core_service_handler.h"
#include "audio_service_enum.h"
#include "i_hpae_manager.h"
#include "stream_dfx_manager.h"
#include "audio_stream_enum.h"
#include "audio_stream_concurrency_detector.h"

namespace OHOS {
namespace AudioStandard {
namespace {
    static const int64_t MOCK_LATENCY = 45000000; // 45000000 -> 45ms
    static const int64_t START_MIN_COST = 80000000; // 80000000 -> 80ms
    static const int32_t NO_FADING = 0;
    static const int32_t DO_FADINGOUT = 1;
    static const int32_t FADING_OUT_DONE = 2;
    static const float FADINGOUT_BEGIN = 1.0f;
    static const float FADINGOUT_END = 0.0f;
    static constexpr int32_t ONE_MINUTE = 60;
    const int32_t MEDIA_UID = 1013;
    const float AUDIO_VOLOMUE_EPSILON = 0.0001;
    const int32_t OFFLOAD_INNER_CAP_PREBUF = 3;
    const size_t OFFLOAD_DUAL_RENDER_PREBUF = 3;
    constexpr int32_t RELEASE_TIMEOUT_IN_SEC = 10; // 10S
    constexpr int32_t DEFAULT_SPAN_SIZE = 2;
    constexpr size_t MSEC_PER_SEC = 1000;
    const int32_t DUP_OFFLOAD_LEN = 7000; // 7000 -> 7000ms
    const int32_t DUP_COMMON_LEN = 440; // 400 -> 440ms
    const int32_t DUP_DEFAULT_LEN = 20; // 20 -> 20ms
    const int32_t DUP_RECOVERY_AUTISHAKE_BUFFER_COUNT = 2; // 2 -> 2 frames -> 40ms
    // a2dp offload data connection max cost
    const int32_t DATA_CONNECTION_TIMEOUT_IN_MS = 800; // ms
}

RendererInServer::RendererInServer(AudioProcessConfig processConfig, std::weak_ptr<IStreamListener> streamListener)
    : processConfig_(processConfig)
{
    streamListener_ = streamListener;
    managerType_ = PLAYBACK;
    if (processConfig_.callerUid == MEDIA_UID) {
        isNeedFade_ = true;
        oldAppliedVolume_ = MIN_FLOAT_VOLUME;
    }
    audioStreamChecker_ = std::make_shared<AudioStreamChecker>(processConfig);
    AudioStreamMonitor::GetInstance().AddCheckForMonitor(processConfig.originalSessionId, audioStreamChecker_);
}

RendererInServer::~RendererInServer()
{
    if (status_ != I_STATUS_RELEASED) {
        Release();
    }
    DumpFileUtil::CloseDumpFile(&dumpC2S_);
}

void RendererInServer::UpdateStreamInfo()
{
    CHECK_AND_RETURN_LOG(checkCount_ <= audioCheckFreq_, "the stream had been already checked");

    if ((audioCheckFreq_ == checkCount_) || (checkCount_ == 0)) {
        AudioStreamConcurrencyDetector::GetInstance().UpdateWriteTime(processConfig_, streamIndex_);
    }
    checkCount_++;
}

void RendererInServer::RemoveStreamInfo()
{
    AudioStreamConcurrencyDetector::GetInstance().RemoveStream(processConfig_, streamIndex_);
    checkCount_ = 0;
}

int32_t RendererInServer::ConfigServerBuffer()
{
    if (audioServerBuffer_ != nullptr) {
        AUDIO_INFO_LOG("ConfigProcessBuffer: process buffer already configed!");
        return SUCCESS;
    }
    stream_->GetSpanSizePerFrame(spanSizeInFrame_);
    // default to 2, 40ms cache size for write mode
    engineTotalSizeInFrame_ = spanSizeInFrame_ * DEFAULT_SPAN_SIZE;

    stream_->GetByteSizePerFrame(byteSizePerFrame_);
    if (engineTotalSizeInFrame_ == 0 || spanSizeInFrame_ == 0 || engineTotalSizeInFrame_ % spanSizeInFrame_ != 0) {
        AUDIO_ERR_LOG("ConfigProcessBuffer: ERR_INVALID_PARAM");
        return ERR_INVALID_PARAM;
    }

    // 100 * 2 + 20 = 220ms, buffer total size.
    bufferTotalSizeInFrame_ = (MAX_CBBUF_IN_USEC * DEFAULT_SPAN_SIZE + MIN_CBBUF_IN_USEC) *
        (processConfig_.streamInfo.customSampleRate == 0 ? processConfig_.streamInfo.samplingRate :
        processConfig_.streamInfo.customSampleRate) / AUDIO_US_PER_S;

    spanSizeInByte_ = spanSizeInFrame_ * byteSizePerFrame_;
    CHECK_AND_RETURN_RET_LOG(spanSizeInByte_ != 0, ERR_OPERATION_FAILED, "Config oh audio buffer failed!");
    AUDIO_INFO_LOG("engineTotalSizeInFrame_: %{public}zu, spanSizeInFrame_: %{public}zu, byteSizePerFrame_:%{public}zu "
        "spanSizeInByte_: %{public}zu, bufferTotalSizeInFrame_: %{public}zu", engineTotalSizeInFrame_,
        spanSizeInFrame_, byteSizePerFrame_, spanSizeInByte_, bufferTotalSizeInFrame_);

    // create OHAudioBuffer in server
    audioServerBuffer_ = OHAudioBufferBase::CreateFromLocal(bufferTotalSizeInFrame_, byteSizePerFrame_);
    CHECK_AND_RETURN_RET_LOG(audioServerBuffer_ != nullptr, ERR_OPERATION_FAILED, "Create oh audio buffer failed");

    // we need to clear data buffer to avoid dirty data.
    memset_s(audioServerBuffer_->GetDataBase(), audioServerBuffer_->GetDataSize(), 0,
        audioServerBuffer_->GetDataSize());
    int32_t ret = InitBufferStatus();
    AUDIO_DEBUG_LOG("Clear data buffer, ret:%{public}d", ret);
    uint32_t spanTime = spanSizeInFrame_ * AUDIO_MS_PER_SECOND /
        (processConfig_.streamInfo.customSampleRate == 0 ? processConfig_.streamInfo.samplingRate :
        processConfig_.streamInfo.customSampleRate);
    audioCheckFreq_ = threshold * AUDIO_MS_PER_SECOND / spanTime;

    isBufferConfiged_ = true;
    isInited_ = true;
    return SUCCESS;
}

int32_t RendererInServer::InitBufferStatus()
{
    if (audioServerBuffer_ == nullptr) {
        AUDIO_ERR_LOG("InitBufferStatus failed, null buffer!");
        return ERR_ILLEGAL_STATE;
    }
    return SUCCESS;
}

void RendererInServer::GetEAC3ControlParam()
{
    int32_t eac3TestFlag = 0;
    GetSysPara("persist.multimedia.eac3test", eac3TestFlag);
    if (eac3TestFlag == 1) {
        managerType_ = EAC3_PLAYBACK;
    }
}

void RendererInServer::ProcessManagerType()
{
    if (processConfig_.rendererInfo.audioFlag == (AUDIO_OUTPUT_FLAG_HD|AUDIO_OUTPUT_FLAG_DIRECT)) {
        Trace trace("current stream marked as high resolution");
        managerType_ = DIRECT_PLAYBACK;
        AUDIO_INFO_LOG("current stream marked as high resolution");
    }
    if (processConfig_.streamInfo.encoding == ENCODING_EAC3) {
        managerType_ = EAC3_PLAYBACK;
        AUDIO_INFO_LOG("current stream marked as eac3 direct stream");
    }
    if (processConfig_.rendererInfo.rendererFlags == AUDIO_FLAG_VOIP_DIRECT) {
        if (IStreamManager::GetPlaybackManager(VOIP_PLAYBACK).GetStreamCount() <= 0) {
            AUDIO_INFO_LOG("current stream marked as VoIP direct stream");
            managerType_ = VOIP_PLAYBACK;
        } else {
            AUDIO_WARNING_LOG("One VoIP direct stream has been created! Use normal mode.");
        }
    }
}

int32_t RendererInServer::Init()
{
    ProcessManagerType();
    GetEAC3ControlParam();
    streamIndex_ = processConfig_.originalSessionId;
    AUDIO_INFO_LOG("Stream index: %{public}u", streamIndex_);

    int32_t ret = IStreamManager::GetPlaybackManager(managerType_).CreateRender(processConfig_, stream_);
    if (ret != SUCCESS && (managerType_ == DIRECT_PLAYBACK || managerType_ == VOIP_PLAYBACK)) {
        Trace trace("high resolution create failed use normal replace");
        managerType_ = PLAYBACK;
        ret = IStreamManager::GetPlaybackManager(managerType_).CreateRender(processConfig_, stream_);
        AUDIO_INFO_LOG("high resolution create failed use normal replace");
    }
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS && stream_ != nullptr, ERR_OPERATION_FAILED,
        "Construct rendererInServer failed: %{public}d", ret);
    bool isSystemApp = CheckoutSystemAppUtil::CheckoutSystemApp(processConfig_.appInfo.appUid);
    StreamVolumeParams streamVolumeParams = { streamIndex_, processConfig_.streamType,
        processConfig_.rendererInfo.streamUsage, processConfig_.appInfo.appUid, processConfig_.appInfo.appPid,
        isSystemApp, processConfig_.rendererInfo.volumeMode, processConfig_.rendererInfo.isVirtualKeyboard };
    AudioVolume::GetInstance()->AddStreamVolume(streamVolumeParams);
    traceTag_ = "[" + std::to_string(streamIndex_) + "]RendererInServer"; // [100001]RendererInServer:
    ret = ConfigServerBuffer();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED,
        "Construct rendererInServer failed: %{public}d", ret);
    stream_->RegisterStatusCallback(shared_from_this());
    stream_->RegisterWriteCallback(shared_from_this());

    // eg: /data/data/.pulse_dir/10000_100001_48000_2_1_server_in.pcm
    AudioStreamInfo tempInfo = processConfig_.streamInfo;
    dumpFileName_ = std::to_string(processConfig_.appInfo.appPid) + "_" + std::to_string(streamIndex_)
        + "_renderer_server_in_"
        + std::to_string(tempInfo.customSampleRate == 0 ? tempInfo.samplingRate : tempInfo.customSampleRate) + "_"
        + std::to_string(tempInfo.channels) + "_" + std::to_string(tempInfo.format) + ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileName_, &dumpC2S_);
    playerDfx_ = std::make_unique<PlayerDfxWriter>(processConfig_.appInfo, streamIndex_);

    return SUCCESS;
}

void RendererInServer::CheckAndWriterRenderStreamStandbySysEvent(bool standbyEnable)
{
    if (standbyEnable == lastWriteStandbyEnableStatus_) {
        return;
    }
    lastWriteStandbyEnableStatus_ = standbyEnable;
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::STREAM_STANDBY,
        Media::MediaMonitor::BEHAVIOR_EVENT);
    bean->Add("STREAMID", static_cast<int32_t>(streamIndex_));
    bean->Add("STANDBY", standbyEnable ? 1 : 0);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
    std::unordered_map<std::string, std::string> payload;
    payload["uid"] = std::to_string(processConfig_.appInfo.appUid);
    payload["sessionId"] = std::to_string(streamIndex_);
    payload["isStandby"] = std::to_string(standbyEnable ? 1 : 0);
    ReportDataToResSched(payload, ResourceSchedule::ResType::RES_TYPE_AUDIO_RENDERER_STANDBY);
    AudioService::GetInstance()->RenderersCheckForAudioWorkgroup(processConfig_.appInfo.appPid);
}

void RendererInServer::OnCheckActiveMusicTime(const std::string &reason)
{
    if (offloadEnable_ == true) {
        CoreServiceHandler::GetInstance().OnCheckActiveMusicTime(reason);
    }
}

void RendererInServer::OnStatusUpdate(IOperation operation)
{
    if (operation != OPERATION_UNDERFLOW) {
        HILOG_COMM_INFO("%{public}u recv operation:%{public}d standByEnable_:%{public}s", streamIndex_, operation,
            (standByEnable_ ? "true" : "false"));
    }
    Trace trace(traceTag_ + " OnStatusUpdate:" + std::to_string(operation));
    CHECK_AND_RETURN_LOG(operation != OPERATION_RELEASED, "Stream already released!");
    std::shared_ptr<IStreamListener> stateListener = streamListener_.lock();
    CHECK_AND_RETURN_LOG((stateListener != nullptr && playerDfx_ != nullptr), "nullptr");
    CHECK_AND_RETURN_LOG(audioServerBuffer_->GetStreamStatus() != nullptr,
        "stream status is nullptr");
    switch (operation) {
        case OPERATION_STARTED:
            HandleOperationStarted();
            stateListener->OnOperationHandled(START_STREAM, 0);
            OnCheckActiveMusicTime("Started");
            break;
        case OPERATION_PAUSED:
            if (standByEnable_) {
                AUDIO_INFO_LOG("%{public}u recv stand-by paused", streamIndex_);
                audioServerBuffer_->GetStreamStatus()->store(STREAM_STAND_BY);
                CheckAndWriterRenderStreamStandbySysEvent(true);
                return;
            }
            status_ = I_STATUS_PAUSED;
            stateListener->OnOperationHandled(PAUSE_STREAM, 0);
            playerDfx_->WriteDfxActionMsg(streamIndex_, RENDERER_STAGE_PAUSE_OK);
            OnCheckActiveMusicTime("Paused");
            break;
        case OPERATION_STOPPED:
            status_ = I_STATUS_STOPPED;
            stateListener->OnOperationHandled(STOP_STREAM, 0);
            HandleOperationStopped(RENDERER_STAGE_STOP_OK);
            OnCheckActiveMusicTime("Stopped");
            break;
        case OPERATION_FLUSHED:
            HandleOperationFlushed();
            stateListener->OnOperationHandled(FLUSH_STREAM, 0);
            break;
        case OPERATION_DRAINED:
            // Client's StopAudioStream will call Drain first and then Stop. If server's drain times out,
            // Stop will be completed first. After a period of time, when Drain's callback goes here,
            // state of server should not be changed to STARTED while the client state is Stopped.
            OnStatusUpdateExt(operation, stateListener);
            break;
        default:
            OnStatusUpdateSub(operation);
    }
    AudioService::GetInstance()->RenderersCheckForAudioWorkgroup(processConfig_.appInfo.appPid);
}

int64_t RendererInServer::GetLastAudioDuration()
{
    auto ret = lastStopTime_ - lastStartTime_;
    return ret < 0 ? -1 : ret;
}

void RendererInServer::OnStatusUpdateExt(IOperation operation, std::shared_ptr<IStreamListener> stateListener)
{
    if (status_ == I_STATUS_DRAINING) {
        status_ = I_STATUS_STARTED;
        stateListener->OnOperationHandled(DRAIN_STREAM, 0);
    }
    afterDrain = true;
    AudioPerformanceMonitor::GetInstance().StartSilenceMonitor(streamIndex_, processConfig_.appInfo.appTokenId);
}

void RendererInServer::HandleOperationStarted()
{
    CHECK_AND_RETURN_LOG(playerDfx_ != nullptr, "nullptr");
    CHECK_AND_RETURN_LOG(audioServerBuffer_->GetStreamStatus() != nullptr,
        "stream status is nullptr");
    if (standByEnable_) {
        standByEnable_ = false;
        AUDIO_INFO_LOG("%{public}u recv stand-by started", streamIndex_);
        audioServerBuffer_->GetStreamStatus()->store(STREAM_RUNNING);
        playerDfx_->WriteDfxActionMsg(streamIndex_, RENDERER_STAGE_STANDBY_END);
    }
    CheckAndWriterRenderStreamStandbySysEvent(false);
    status_ = I_STATUS_STARTED;
    startedTime_ = ClockTime::GetCurNano();
    
    lastStartTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    lastWriteFrame_ = static_cast<int64_t>(audioServerBuffer_->GetCurReadFrame());
    lastWriteMuteFrame_ = 0;
}

// LCOV_EXCL_START
void RendererInServer::OnStatusUpdateSub(IOperation operation)
{
    std::shared_ptr<IStreamListener> stateListener = streamListener_.lock();
    CHECK_AND_RETURN_LOG(stateListener != nullptr, "StreamListener is nullptr!");
    int32_t engineFlag = GetEngineFlag();
    switch (operation) {
        case OPERATION_RELEASED:
            stateListener->OnOperationHandled(RELEASE_STREAM, 0);
            status_ = I_STATUS_RELEASED;
            break;
        case OPERATION_UNDERRUN:
            AUDIO_INFO_LOG("Underrun: audioServerBuffer_->GetWritableDataFrames(): %{public}d",
                audioServerBuffer_->GetWritableDataFrames());
            if (audioServerBuffer_->GetWritableDataFrames() ==
                static_cast<int32_t>(DEFAULT_SPAN_SIZE * spanSizeInFrame_)) {
                AUDIO_INFO_LOG("Buffer is empty");
                needForceWrite_ = 0;
            } else {
                AUDIO_INFO_LOG("Buffer is not empty");
                std::unique_lock lock(writeLock_, std::try_to_lock);
                if (lock.owns_lock()) {
                    WriteData();
                }
            }
            break;
        case OPERATION_UNDERFLOW:
            if (ClockTime::GetCurNano() - startedTime_ > START_MIN_COST) {
                underrunCount_++;
                audioServerBuffer_->SetUnderrunCount(underrunCount_);
            }
            StandByCheck(); // if stand by is enbaled here, stream will be paused and not recv UNDERFLOW any more.
            break;
        case OPERATION_SET_OFFLOAD_ENABLE:
        case OPERATION_UNSET_OFFLOAD_ENABLE:
            offloadEnable_ = operation == OPERATION_SET_OFFLOAD_ENABLE ? true : false;
            if (engineFlag == 1) {
                ReConfigDupStreamCallback();
            }
            stateListener->OnOperationHandled(SET_OFFLOAD_ENABLE, operation == OPERATION_SET_OFFLOAD_ENABLE ? 1 : 0);
            break;
        default:
            AUDIO_INFO_LOG("Invalid operation %{public}u", operation);
            status_ = I_STATUS_INVALID;
    }
}
// LCOV_EXCL_STOP

void RendererInServer::ReConfigDupStreamCallback()
{
    size_t dupTotalSizeInFrameTemp_ = 0;

    if (offloadEnable_ == true) {
        dupTotalSizeInFrameTemp_ = dupSpanSizeInFrame_ * (DUP_OFFLOAD_LEN / DUP_DEFAULT_LEN);
    } else {
        dupTotalSizeInFrameTemp_ = dupSpanSizeInFrame_ * (DUP_COMMON_LEN / DUP_DEFAULT_LEN);
    }
    AUDIO_INFO_LOG("dupTotalSizeInFrameTemp_: %{public}zu, dupTotalSizeInFrame_: %{public}zu",
        dupTotalSizeInFrameTemp_, dupTotalSizeInFrame_);
    if (dupTotalSizeInFrameTemp_ == dupTotalSizeInFrame_) {
        return;
    }
    dupTotalSizeInFrame_ = dupTotalSizeInFrameTemp_;
    std::lock_guard<std::mutex> lock(dupMutex_);
    for (auto it = innerCapIdToDupStreamCallbackMap_.begin(); it != innerCapIdToDupStreamCallbackMap_.end(); ++it) {
        if (captureInfos_[(*it).first].dupStream != nullptr && (*it).second != nullptr &&
            (*it).second->GetDupRingBuffer() != nullptr) {
            (*it).second->GetDupRingBuffer()->ReConfig(dupTotalSizeInFrame_ * dupByteSizePerFrame_, false);
        }
    }
}

void RendererInServer::StandByCheck()
{
    Trace trace(traceTag_ + " StandByCheck:standByCounter_:" + std::to_string(standByCounter_.load()));

    // msdp wait for uncertain time when waiting for bt reply, which may case stream change into standby mode.
    // if client writes date when stream is changing into standby mode, it would cause drain fail problems.
    // msdp promises to call api correctly to avoid power problems
    if (processConfig_.rendererInfo.streamUsage == StreamUsage::STREAM_USAGE_ULTRASONIC) {
        return;
    }
    AUDIO_INFO_LOG("sessionId:%{public}u standByCounter_:%{public}u standByEnable_:%{public}s ", streamIndex_,
        standByCounter_.load(), (standByEnable_ ? "true" : "false"));

    // direct standBy need not in here
    if (managerType_ == DIRECT_PLAYBACK || managerType_ == VOIP_PLAYBACK) {
        return;
    }

    if (standByEnable_) {
        return;
    }
    standByCounter_++;
    audioStreamChecker_->RecordNodataFrame();
    if (!ShouldEnableStandBy()) {
        return;
    }

    // call enable stand by
    standByEnable_ = true;
    RecordStandbyTime(standByEnable_, true);
    enterStandbyTime_ = ClockTime::GetCurNano();
    // PaAdapterManager::PauseRender will hold mutex, may cause dead lock with pa_lock
    if (managerType_ == PLAYBACK) {
        stream_->Pause(true);
    }

    if (playerDfx_) {
        playerDfx_->WriteDfxActionMsg(streamIndex_, RENDERER_STAGE_STANDBY_BEGIN);
    }
}

bool RendererInServer::ShouldEnableStandBy()
{
    int64_t timeCost = ClockTime::GetCurNano() - lastWriteTime_;
    uint32_t maxStandByCounter = 50; // for 20ms, 50 * 20 = 1000ms
    int64_t timeLimit = 1000000000; // 1s
    if (offloadEnable_) {
        maxStandByCounter = 400; // for 20ms, 50 * 400 = 8000ms
        timeLimit = 8 * AUDIO_NS_PER_SECOND; // for 20ms 8s
    }
    if (standByCounter_ >= maxStandByCounter && timeCost >= timeLimit) {
        AUDIO_INFO_LOG("sessionId:%{public}u reach the limit of stand by: %{public}u time:%{public}" PRId64"ns",
            streamIndex_, standByCounter_.load(), timeCost);
        return true;
    }
    return false;
}

int32_t RendererInServer::GetStandbyStatus(bool &isStandby, int64_t &enterStandbyTime)
{
    Trace trace("RendererInServer::GetStandbyStatus:" + std::to_string(streamIndex_) + (standByEnable_ ? " Enabled" :
        "Disabled"));
    isStandby = standByEnable_;
    if (isStandby) {
        enterStandbyTime = enterStandbyTime_;
    } else {
        enterStandbyTime = 0;
    }
    return SUCCESS;
}

void RendererInServer::HandleOperationFlushed()
{
    switch (status_) {
        case I_STATUS_FLUSHING_WHEN_STARTED:
            status_ = I_STATUS_STARTED;
            break;
        case I_STATUS_FLUSHING_WHEN_PAUSED:
            status_ = I_STATUS_PAUSED;
            break;
        case I_STATUS_FLUSHING_WHEN_STOPPED:
            status_ = I_STATUS_STOPPED;
            break;
        default:
            AUDIO_WARNING_LOG("Invalid status before flusing");
    }
    AudioPerformanceMonitor::GetInstance().StartSilenceMonitor(streamIndex_, processConfig_.appInfo.appTokenId);
}

void RendererInServer::HandleOperationStopped(RendererStage stage)
{
    CHECK_AND_RETURN_LOG(audioServerBuffer_ != nullptr && playerDfx_ != nullptr, "nullptr");
    lastStopTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    lastWriteFrame_ = static_cast<int64_t>(audioServerBuffer_->GetCurReadFrame()) - lastWriteFrame_;
    playerDfx_->WriteDfxStopMsg(streamIndex_, stage,
        {lastWriteFrame_, lastWriteMuteFrame_, GetLastAudioDuration(), underrunCount_}, processConfig_);
}

BufferDesc RendererInServer::DequeueBuffer(size_t length)
{
    return stream_->DequeueBuffer(length);
}

void RendererInServer::DoFadingOut(RingBufferWrapper& bufferDesc)
{
    std::lock_guard<std::mutex> lock(fadeoutLock_);
    if (fadeoutFlag_ == DO_FADINGOUT) {
        AUDIO_INFO_LOG("format:%{public}u", processConfig_.streamInfo.format);
        AudioChannel channel = processConfig_.streamInfo.channels;
        ChannelVolumes mapVols = VolumeTools::GetChannelVolumes(channel, FADINGOUT_BEGIN, FADINGOUT_END);
        int32_t ret = VolumeTools::Process(bufferDesc, processConfig_.streamInfo.format, mapVols);
        if (ret != SUCCESS) {
            AUDIO_WARNING_LOG("VolumeTools::Process failed: %{public}d", ret);
        }
        fadeoutFlag_ = FADING_OUT_DONE;
        AUDIO_INFO_LOG("fadeoutFlag_ = FADING_OUT_DONE");
    }
}

bool RendererInServer::IsInvalidBuffer(uint8_t *buffer, size_t bufferSize)
{
    bool isInvalid = false;
    uint8_t ui8Data = 0;
    int16_t i16Data = 0;
    switch (processConfig_.streamInfo.format) {
        case SAMPLE_U8:
            CHECK_AND_RETURN_RET_LOG(bufferSize > 0, false, "buffer size is too small");
            ui8Data = *buffer;
            isInvalid = ui8Data == 0;
            break;
        case SAMPLE_S16LE:
            CHECK_AND_RETURN_RET_LOG(bufferSize > 1, false, "buffer size is too small");
            i16Data = *(reinterpret_cast<const int16_t*>(buffer));
            isInvalid = i16Data == 0;
            break;
        default:
            break;
    }
    return isInvalid;
}

void RendererInServer::WriteMuteDataSysEvent(BufferDesc &bufferDesc)
{
    int64_t muteFrameCnt = 0;
    VolumeTools::CalcMuteFrame(bufferDesc, processConfig_.streamInfo, traceTag_, volumeDataCount_, muteFrameCnt);
    lastWriteMuteFrame_ += muteFrameCnt;
    if (volumeDataCount_ < 0) {
        audioStreamChecker_->RecordMuteFrame();
    }
    if (silentModeAndMixWithOthers_) {
        return;
    }
    if (IsInvalidBuffer(bufferDesc.buffer, bufferDesc.bufLength)) {
        if (startMuteTime_ == 0) {
            startMuteTime_ = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        }
        std::time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        if ((currentTime - startMuteTime_ >= ONE_MINUTE) && !isInSilentState_) {
            isInSilentState_ = true;
            AUDIO_WARNING_LOG("write invalid data for some time in server");

            std::unordered_map<std::string, std::string> payload;
            payload["uid"] = std::to_string(processConfig_.appInfo.appUid);
            payload["sessionId"] = std::to_string(streamIndex_);
            payload["isSilent"] = std::to_string(true);
            ReportDataToResSched(payload, ResourceSchedule::ResType::RES_TYPE_AUDIO_RENDERER_SILENT_PLAYBACK);
        }
    } else {
        if (startMuteTime_ != 0) {
            startMuteTime_ = 0;
        }
        if (isInSilentState_) {
            AUDIO_WARNING_LOG("begin write valid data in server");
            isInSilentState_ = false;

            std::unordered_map<std::string, std::string> payload;
            payload["uid"] = std::to_string(processConfig_.appInfo.appUid);
            payload["sessionId"] = std::to_string(streamIndex_);
            payload["isSilent"] = std::to_string(false);
            ReportDataToResSched(payload, ResourceSchedule::ResType::RES_TYPE_AUDIO_RENDERER_SILENT_PLAYBACK);
        }
    }

    if ((!latestForWorkgroupInited_) || (latestForWorkgroup_.isInSilentState != isInSilentState_)) {
        AudioService::GetInstance()->RenderersCheckForAudioWorkgroup(processConfig_.appInfo.appPid);
    }
}

void RendererInServer::ReportDataToResSched(std::unordered_map<std::string, std::string> payload, uint32_t type)
{
#ifdef RESSCHE_ENABLE
    AUDIO_INFO_LOG("report event to ResSched ,event type : %{public}d", type);
    ResourceSchedule::ResSchedClient::GetInstance().ReportData(type, 0, payload);
#endif
}

void RendererInServer::VolumeHandle(BufferDesc &desc)
{
    // volume process in server
    if (audioServerBuffer_ == nullptr) {
        AUDIO_WARNING_LOG("buffer in not inited");
        return;
    }
    float applyVolume = 0.0f;
    if (muteFlag_) {
        applyVolume = 0.0f;
    } else {
        applyVolume = audioServerBuffer_->GetStreamVolume();
    }
    float duckVolume = audioServerBuffer_->GetDuckFactor();
    float muteVolume = audioServerBuffer_->GetMuteFactor();
    if (!IsVolumeSame(MAX_FLOAT_VOLUME, lowPowerVolume_, AUDIO_VOLOMUE_EPSILON)) {
        applyVolume *= lowPowerVolume_;
    }
    if (!IsVolumeSame(MAX_FLOAT_VOLUME, duckVolume, AUDIO_VOLOMUE_EPSILON)) {
        applyVolume *= duckVolume;
    }
    if (!IsVolumeSame(MAX_FLOAT_VOLUME, muteVolume, AUDIO_VOLOMUE_EPSILON)) {
        applyVolume *= muteVolume;
    }

    if (silentModeAndMixWithOthers_) {
        applyVolume = 0.0f;
    }

    //in plan: put system volume handle here
    if (!IsVolumeSame(MAX_FLOAT_VOLUME, applyVolume, AUDIO_VOLOMUE_EPSILON) ||
        !IsVolumeSame(oldAppliedVolume_, applyVolume, AUDIO_VOLOMUE_EPSILON)) {
        Trace traceVol("RendererInServer::VolumeTools::Process " + std::to_string(oldAppliedVolume_) + "~" +
            std::to_string(applyVolume));
        AudioChannel channel = processConfig_.streamInfo.channels;
        ChannelVolumes mapVols = VolumeTools::GetChannelVolumes(channel, oldAppliedVolume_, applyVolume);
        int32_t volRet = VolumeTools::Process(desc, processConfig_.streamInfo.format, mapVols);
        oldAppliedVolume_ = applyVolume;
        if (volRet != SUCCESS) {
            AUDIO_WARNING_LOG("VolumeTools::Process error: %{public}d", volRet);
        }
    }
}

BufferDesc RendererInServer::PrepareOutputBuffer(const RingBufferWrapper& ringBufferDesc)
{
    BufferDesc bufferDesc;
    if (ringBufferDesc.basicBufferDescs[0].bufLength >= ringBufferDesc.dataLength) {
        bufferDesc.buffer = ringBufferDesc.basicBufferDescs[0].buffer;
        bufferDesc.bufLength = ringBufferDesc.dataLength;
        bufferDesc.dataLength = ringBufferDesc.dataLength;
    } else {
        rendererTmpBuffer_.resize(ringBufferDesc.dataLength);
        RingBufferWrapper tmpWrapper;
        tmpWrapper.dataLength = ringBufferDesc.dataLength;
        tmpWrapper.basicBufferDescs[0].buffer = rendererTmpBuffer_.data();
        tmpWrapper.basicBufferDescs[0].bufLength = ringBufferDesc.dataLength;
        tmpWrapper.CopyInputBufferValueToCurBuffer(ringBufferDesc);

        bufferDesc.buffer = rendererTmpBuffer_.data();
        bufferDesc.bufLength = ringBufferDesc.dataLength;
        bufferDesc.dataLength = ringBufferDesc.dataLength;
    }
    return bufferDesc;
}

int32_t RendererInServer::WriteData()
{
    uint64_t currentReadFrame = audioServerBuffer_->GetCurReadFrame();
    uint64_t currentWriteFrame = audioServerBuffer_->GetCurWriteFrame();
    Trace trace1(traceTag_ + " WriteData"); // RendererInServer::sessionid:100001 WriteData
    if (currentReadFrame >= currentWriteFrame) {
        Trace trace2(traceTag_ + " near underrun"); // RendererInServer::sessionid:100001 near underrun
        if (!offloadEnable_) {
            CHECK_AND_RETURN_RET_LOG(currentWriteFrame >= currentReadFrame, ERR_OPERATION_FAILED,
                "invalid write and read position.");
            uint64_t dataSize = currentWriteFrame - currentReadFrame;
            AUDIO_INFO_LOG("sessionId: %{public}u OHAudioBuffer %{public}" PRIu64 "size is not enough",
                streamIndex_, dataSize);
        }
        return ERR_OPERATION_FAILED;
    }

    RingBufferWrapper ringBufferDesc; // will be changed in GetReadbuffer
    if (audioServerBuffer_->GetAllReadableBufferFromPosFrame(currentReadFrame, ringBufferDesc) == SUCCESS) {
        ringBufferDesc.dataLength = std::min(ringBufferDesc.dataLength, spanSizeInByte_);
        if (ringBufferDesc.dataLength == 0) {
            AUDIO_ERR_LOG("not enough data!");
            return ERR_INVALID_PARAM;
        }
        uint64_t durationMs = ((byteSizePerFrame_ * processConfig_.streamInfo.samplingRate) == 0) ? 0
            : ((MSEC_PER_SEC * processConfig_.rendererInfo.expectedPlaybackDurationBytes) /
            (byteSizePerFrame_ * processConfig_.streamInfo.samplingRate));
        if (processConfig_.streamType != STREAM_ULTRASONIC && (GetFadeStrategy(durationMs) == FADE_STRATEGY_DEFAULT)) {
            if (currentReadFrame + spanSizeInFrame_ >= currentWriteFrame) {
                DoFadingOut(ringBufferDesc);
            }
        }

        BufferDesc bufferDesc = PrepareOutputBuffer(ringBufferDesc);
        stream_->EnqueueBuffer(bufferDesc);
        if (AudioDump::GetInstance().GetVersionType() == DumpFileUtil::BETA_VERSION) {
            DumpFileUtil::WriteDumpFile(dumpC2S_, static_cast<void *>(bufferDesc.buffer), bufferDesc.bufLength);
            AudioCacheMgr::GetInstance().CacheData(dumpFileName_,
                static_cast<void *>(bufferDesc.buffer), bufferDesc.bufLength);
        }

        OtherStreamEnqueue(bufferDesc);

        WriteMuteDataSysEvent(bufferDesc);
        ringBufferDesc.SetBuffersValueWithSpecifyDataLen(0); // clear is needed for reuse.
        // Client may write the buffer immediately after SetCurReadFrame, so put memset_s before it!
        uint64_t nextReadFrame = currentReadFrame + (ringBufferDesc.dataLength / byteSizePerFrame_);
        audioServerBuffer_->SetCurReadFrame(nextReadFrame);
    }
    standByCounter_ = 0;
    lastWriteTime_ = ClockTime::GetCurNano();
    return SUCCESS;
}

int32_t RendererInServer::GetAvailableSize(size_t &length)
{
    uint64_t currentReadFrame = audioServerBuffer_->GetCurReadFrame();
    uint64_t currentWriteFrame = audioServerBuffer_->GetCurWriteFrame();
    if (currentWriteFrame < currentReadFrame) {
        return ERROR;
    }

    length = static_cast<size_t>((currentWriteFrame - currentReadFrame) * byteSizePerFrame_);
    return SUCCESS;
}

void RendererInServer::CopyDataToInputBuffer(int8_t* inputData, size_t requestDataLen,
    const RingBufferWrapper& ringBufferDesc)
{
    RingBufferWrapper wrapperInputData = {
        .basicBufferDescs = {{
            {reinterpret_cast<uint8_t*>(inputData), requestDataLen},
            {}
        }},
        .dataLength = requestDataLen
    };

    CHECK_AND_RETURN_LOG(wrapperInputData.CopyInputBufferValueToCurBuffer(ringBufferDesc) == 0,
        "memcpy error");
}

void RendererInServer::ProcessFadeOutIfNeeded(RingBufferWrapper& ringBufferDesc,
    uint64_t currentReadFrame, uint64_t currentWriteFrame,
    size_t requestDataInFrame)
{
    if (processConfig_.streamType != STREAM_ULTRASONIC &&
        currentReadFrame + requestDataInFrame == currentWriteFrame) {
        DoFadingOut(ringBufferDesc);
    }
}

void RendererInServer::OnWriteDataFinish()
{
    standByCounter_ = 0;
    lastWriteTime_ = ClockTime::GetCurNano();

    UpdateStreamInfo();
}

int32_t RendererInServer::WriteData(int8_t *inputData, size_t requestDataLen)
{
    size_t requestDataInFrame = requestDataLen / byteSizePerFrame_;

    std::lock_guard lock(writeLock_);
    uint64_t currentReadFrame = audioServerBuffer_->GetCurReadFrame();
    uint64_t currentWriteFrame = audioServerBuffer_->GetCurWriteFrame();
    CHECK_AND_RETURN_RET_LOG(spanSizeInFrame_ != 0, ERR_OPERATION_FAILED, "invalid span size");
    int64_t cacheCount = audioServerBuffer_->GetReadableDataFrames() / static_cast<int64_t>(spanSizeInFrame_);
    Trace trace1(traceTag_ + " OnWriteData cacheCount:" + std::to_string(cacheCount));
    if (requestDataLen == 0 || currentReadFrame + requestDataInFrame > currentWriteFrame) {
        Trace trace2(traceTag_ + " near underrun"); // RendererInServer::sessionid:100001 near underrun
        if (!offloadEnable_) {
            CHECK_AND_RETURN_RET_LOG(currentWriteFrame >= currentReadFrame, ERR_OPERATION_FAILED,
                "invalid write and read position.");
            uint64_t dataSize = currentWriteFrame - currentReadFrame;
            AUDIO_INFO_LOG("sessionId: %{public}u OHAudioBuffer %{public}" PRIu64 "size is not enough",
                streamIndex_, dataSize);
        }
        return ERR_OPERATION_FAILED;
    }

    RingBufferWrapper ringBufferDesc; // will be changed in GetReadbuffer
    int32_t ret = audioServerBuffer_->GetAllReadableBufferFromPosFrame(currentReadFrame, ringBufferDesc);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "WriteData GetReadbuffer failed");
    CHECK_AND_RETURN_RET_LOG(ringBufferDesc.dataLength >= requestDataLen, ERR_INVALID_PARAM, "data not enouth");

    ringBufferDesc.dataLength = requestDataLen;
    ProcessFadeOutIfNeeded(ringBufferDesc, currentReadFrame, currentWriteFrame, requestDataInFrame);
    CopyDataToInputBuffer(inputData, requestDataLen, ringBufferDesc);
    ringBufferDesc.SetBuffersValueWithSpecifyDataLen(0); // clear is needed for reuse.
    uint64_t nextReadFrame = currentReadFrame + requestDataInFrame;
    audioServerBuffer_->SetCurReadFrame(nextReadFrame);

    return SUCCESS;
}

void RendererInServer::OtherStreamEnqueue(const BufferDesc &bufferDesc)
{
    {
        // for inner capture
        std::lock_guard<std::mutex> captureLock(dupMutex_);
        for (auto &capInfo : captureInfos_) {
            InnerCaptureOtherStream(bufferDesc, capInfo.second, capInfo.first);
        }
    }
    // for dual tone
    if (isDualToneEnabled_) {
        Trace traceDup("RendererInServer::WriteData DualToneSteam write");
        std::lock_guard<std::mutex> lock(dualToneMutex_);
        if (dualToneStream_ != nullptr) {
            dualToneStream_->EnqueueBuffer(bufferDesc); // what if enqueue fail?
        }
    }
}

void RendererInServer::InnerCaptureEnqueueBuffer(const BufferDesc &bufferDesc, CaptureInfo &captureInfo,
    int32_t innerCapId)
{
    int32_t engineFlag = GetEngineFlag();
    if (renderEmptyCountForInnerCapToInnerCapIdMap_.find(innerCapId) !=
        renderEmptyCountForInnerCapToInnerCapIdMap_.end() &&
        renderEmptyCountForInnerCapToInnerCapIdMap_[innerCapId] > 0) {
        size_t emptyBufferSize = static_cast<size_t>
            (renderEmptyCountForInnerCapToInnerCapIdMap_[innerCapId]) * spanSizeInByte_;
        auto buffer = std::make_unique<uint8_t []>(emptyBufferSize);
        BufferDesc emptyBufferDesc = {buffer.get(), emptyBufferSize, emptyBufferSize};
        memset_s(emptyBufferDesc.buffer, emptyBufferDesc.bufLength, 0, emptyBufferDesc.bufLength);
        if (engineFlag == 1) {
            WriteDupBufferInner(emptyBufferDesc, innerCapId);
        } else {
            captureInfo.dupStream->EnqueueBuffer(emptyBufferDesc);
        }
        renderEmptyCountForInnerCapToInnerCapIdMap_[innerCapId] = 0;
    }
    if (engineFlag == 1) {
        AUDIO_DEBUG_LOG("OtherStreamEnqueue running");
        WriteDupBufferInner(bufferDesc, innerCapId);
    } else {
        captureInfo.dupStream->EnqueueBuffer(bufferDesc); // what if enqueue fail?
    }
}

void RendererInServer::InnerCaptureOtherStream(const BufferDesc &bufferDesc, CaptureInfo &captureInfo,
    int32_t innerCapId)
{
    if (captureInfo.isInnerCapEnabled) {
        Trace traceDup("RendererInServer::WriteData DupSteam write");
        if (captureInfo.dupStream != nullptr) {
            Trace trace("InnerCaptureOtherStream WriteData, sessionId: " +
                std::to_string(captureInfo.dupStream->GetStreamIndex()));
            InnerCaptureEnqueueBuffer(bufferDesc, captureInfo, innerCapId);
        }
    }
}

void RendererInServer::WriteEmptyData()
{
    Trace trace("RendererInServer::WriteEmptyData");
    AUDIO_WARNING_LOG("Underrun, write empty data");
    BufferDesc bufferDesc = stream_->DequeueBuffer(spanSizeInByte_);
    memset_s(bufferDesc.buffer, bufferDesc.bufLength, 0, bufferDesc.bufLength);
    stream_->EnqueueBuffer(bufferDesc);
    return;
}

int32_t RendererInServer::OnWriteData(size_t length)
{
    Trace trace("RendererInServer::OnWriteData length " + std::to_string(length));
    bool mayNeedForceWrite = false;
    std::unique_lock lock(writeLock_, std::defer_lock);
    if (lock.try_lock()) {
        // length unit is bytes, using spanSizeInByte_
        if (spanSizeInByte_ <= 0) {
            return ERR_WRITE_FAILED;
        }
        for (size_t i = 0; i < length / spanSizeInByte_; i++) {
            mayNeedForceWrite = WriteData() != SUCCESS || mayNeedForceWrite;
        }
        lock.unlock();
    } else {
        mayNeedForceWrite = true;
    }

    size_t maxEmptyCount = 1;
    size_t writableSize = stream_->GetWritableSize();
    if (mayNeedForceWrite && writableSize >= spanSizeInByte_ * maxEmptyCount) {
        AUDIO_DEBUG_LOG("Server need force write to recycle callback");
        needForceWrite_ =
            writableSize / spanSizeInByte_ > 3 ? 0 : 3 - writableSize / spanSizeInByte_; // 3 is maxlength - 1
    }

    uint64_t currentReadFrame = audioServerBuffer_->GetCurReadFrame();
    audioServerBuffer_->SetHandleInfo(currentReadFrame, ClockTime::GetCurNano() + MOCK_LATENCY);

    if (mayNeedForceWrite) {
        return ERR_RENDERER_IN_SERVER_UNDERRUN;
    }

    UpdateStreamInfo();

    return SUCCESS;
}

// Call WriteData will hold mainloop lock in EnqueueBuffer, we should not lock a mutex in WriteData while OnWriteData is
// called with mainloop locking.
int32_t RendererInServer::UpdateWriteIndex()
{
    Trace trace("RendererInServer::UpdateWriteIndex needForceWrite" + std::to_string(needForceWrite_));
    if (managerType_ != PLAYBACK) {
        IStreamManager::GetPlaybackManager(managerType_).TriggerStartIfNecessary();
    }
    std::unique_lock lock(writeLock_, std::defer_lock);
    if (needForceWrite_ < 3 && stream_->GetWritableSize() >= spanSizeInByte_) { // 3 is maxlength - 1
        if (lock.try_lock()) {
            AUDIO_DEBUG_LOG("Start force write data");
            int32_t ret = WriteData();
            if (ret == SUCCESS) {
                needForceWrite_++;
            }
            lock.unlock();
        }
    }

    int32_t engineFlag = GetEngineFlag();
    if (engineFlag != 1) {
        if (afterDrain == true) {
            if (lock.try_lock()) {
                afterDrain = false;
                AUDIO_DEBUG_LOG("After drain, start write data");
                WriteData();
                lock.unlock();
            }
        }
    }
    return SUCCESS;
}

int32_t RendererInServer::ResolveBuffer(std::shared_ptr<OHAudioBuffer> &buffer)
{
    AUDIO_ERR_LOG("Not support");
    return SUCCESS;
}

int32_t RendererInServer::ResolveBufferBaseAndGetServerSpanSize(std::shared_ptr<OHAudioBufferBase> &buffer,
    uint32_t &spanSizeInFrame, uint64_t &engineTotalSizeInFrame)
{
    buffer = audioServerBuffer_;
    spanSizeInFrame = spanSizeInFrame_;
    engineTotalSizeInFrame = engineTotalSizeInFrame_;
    return SUCCESS;
}

int32_t RendererInServer::GetSessionId(uint32_t &sessionId)
{
    CHECK_AND_RETURN_RET_LOG(stream_ != nullptr, ERR_OPERATION_FAILED, "GetSessionId failed, stream_ is null");
    sessionId = streamIndex_;
    CHECK_AND_RETURN_RET_LOG(sessionId < INT32_MAX, ERR_OPERATION_FAILED, "GetSessionId failed, sessionId:%{public}d",
        sessionId);

    return SUCCESS;
}

int32_t RendererInServer::Start()
{
    AudioXCollie audioXCollie(
        "RendererInServer::Start", RELEASE_TIMEOUT_IN_SEC, nullptr, nullptr,
            AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    int32_t ret = StartInner();
    RendererStage stage = ret == SUCCESS ? RENDERER_STAGE_START_OK : RENDERER_STAGE_START_FAIL;
    if (playerDfx_) {
        playerDfx_->WriteDfxStartMsg(streamIndex_, stage, sourceDuration_, processConfig_);
    }
    if (ret == SUCCESS) {
        StreamDfxManager::GetInstance().CheckStreamOccupancy(streamIndex_, processConfig_, true);
    }

    XperfAdapter::GetInstance().ReportStateChangeEventIfNeed(XPERF_EVENT_START,
        processConfig_.rendererInfo.streamUsage, streamIndex_, processConfig_.appInfo.appPid,
        processConfig_.appInfo.appUid);

    return ret;
}

int32_t RendererInServer::StartInnerDuringStandby()
{
    int32_t ret = 0;
    AUDIO_INFO_LOG("sessionId: %{public}u call to exit stand by!", streamIndex_);
    CHECK_AND_RETURN_RET_LOG(audioServerBuffer_->GetStreamStatus() != nullptr, ERR_OPERATION_FAILED, "null stream");
    standByCounter_ = 0;
    startedTime_ = ClockTime::GetCurNano();
    audioServerBuffer_->GetStreamStatus()->store(STREAM_STARTING);
    ret = CoreServiceHandler::GetInstance().UpdateSessionOperation(streamIndex_, SESSION_OPERATION_START);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Policy start client failed, reason: %{public}d", ret);
    ret = (managerType_ == DIRECT_PLAYBACK || managerType_ == VOIP_PLAYBACK) ?
        IStreamManager::GetPlaybackManager(managerType_).StartRender(streamIndex_) : stream_->Start();
    RecordStandbyTime(true, false);
    return ret;
}

int32_t RendererInServer::StartInner()
{
    AUDIO_INFO_LOG("sessionId: %{public}u", streamIndex_);
    int32_t ret = 0;
    if (lastTarget_ == INJECT_TO_VOICE_COMMUNICATION_CAPTURE) {
        ret = CoreServiceHandler::GetInstance().StartInjection(streamIndex_);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "StartInjection failed");
    }
    if (standByEnable_) {
        return StartInnerDuringStandby();
    } else {
        audioStreamChecker_->MonitorOnAllCallback(AUDIO_STREAM_START, false);
    }
    needForceWrite_ = 0;
    std::unique_lock<std::mutex> lock(statusLock_);
    if (status_ != I_STATUS_IDLE && status_ != I_STATUS_PAUSED && status_ != I_STATUS_STOPPED) {
        AUDIO_ERR_LOG("failed, Illegal state: %{public}u", status_.load());
        return ERR_ILLEGAL_STATE;
    }
    status_ = I_STATUS_STARTING;
    std::unique_lock<std::mutex> fadeLock(fadeoutLock_);
    AUDIO_INFO_LOG("fadeoutFlag_ = NO_FADING");
    fadeoutFlag_ = NO_FADING;
    fadeLock.unlock();
    ret = CoreServiceHandler::GetInstance().UpdateSessionOperation(streamIndex_, SESSION_OPERATION_START);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Policy start client failed, reason: %{public}d", ret);

    // Bluetooth connection may take a long time, which may cause the data before and after the connection
    // are not continuous, resulting in pop sounds. To avoid the problem,
    // WaitForDataConnection() needs to be called after UpdateSessionOperation(), where device is activated,
    // and also before stream_->Start(), where the stream is actually started.
    WaitForDataConnection();

    ret = (managerType_ == DIRECT_PLAYBACK || managerType_ == VOIP_PLAYBACK || managerType_ == EAC3_PLAYBACK) ?
        IStreamManager::GetPlaybackManager(managerType_).StartRender(streamIndex_) : stream_->Start();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Start stream failed, reason: %{public}d", ret);

    startedTime_ = ClockTime::GetCurNano();
    uint64_t currentReadFrame = audioServerBuffer_->GetCurReadFrame();
    int64_t tempTime = ClockTime::GetCurNano() + MOCK_LATENCY;
    audioServerBuffer_->SetHandleInfo(currentReadFrame, tempTime);
    AUDIO_INFO_LOG("Server update position %{public}" PRIu64" time%{public} " PRId64".", currentReadFrame, tempTime);
    resetTime_ = true;

    StartStreamByType();
    enterStandbyTime_ = 0;

    dualToneStreamInStart();
    AudioPerformanceMonitor::GetInstance().StartSilenceMonitor(streamIndex_, processConfig_.appInfo.appTokenId);
    return SUCCESS;
}

void RendererInServer::StartStreamByType()
{
    if (IsMovieOffloadStream()) {
        SetSoftLinkFunc([](auto &softLink) { softLink->Start(); });
    } else {
        std::lock_guard<std::mutex> dupLock(dupMutex_);
        for (auto &capInfo : captureInfos_) {
            if (IsEnabledAndValidDupStream(capInfo.second)) {
                capInfo.second.dupStream->Start();
            }
        }
    }
}

void RendererInServer::dualToneStreamInStart()
{
    if (isDualToneEnabled_ && dualToneStream_ != nullptr) {
        //Joint judgment ensures that there is a double ring and there is a stream to enter.
        stream_->GetAudioEffectMode(effectModeWhenDual_);
        stream_->SetAudioEffectMode(EFFECT_NONE);
        std::lock_guard<std::mutex> lock(dualToneMutex_);
        //Locking before SetAudioEffectMode/GetAudioEffectMode results in a deadlock.
        if (dualToneStream_ != nullptr) {
            //Since there was no lock protection before the last time it was awarded dualToneStream_ it was
            //modified elsewhere, it was decided again after the lock was awarded.
            dualToneStream_->SetAudioEffectMode(EFFECT_NONE);
            dualToneStream_->Start();
        }
    }
}

void RendererInServer::RecordStandbyTime(bool isStandby, bool isStandbyStart)
{
    if (!isStandby) {
        AUDIO_DEBUG_LOG("Not in standby, no need record time");
        return;
    }
    audioStreamChecker_->RecordStandbyTime(isStandbyStart);
}

void RendererInServer::PauseInner()
{
    AudioPerformanceMonitor::GetInstance().PauseSilenceMonitor(streamIndex_);
    XperfAdapter::GetInstance().ReportStateChangeEventIfNeed(XPERF_EVENT_STOP, processConfig_.rendererInfo.streamUsage,
        streamIndex_, processConfig_.appInfo.appPid, processConfig_.appInfo.appUid);

    RemoveStreamInfo();
}

int32_t RendererInServer::Pause()
{
    AUDIO_INFO_LOG("Pause.");
    AudioXCollie audioXCollie("RendererInServer::Pause", RELEASE_TIMEOUT_IN_SEC, nullptr, nullptr,
            AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    std::unique_lock<std::mutex> lock(statusLock_);
    CHECK_AND_RETURN_RET_LOG(status_ == I_STATUS_STARTED, ERR_ILLEGAL_STATE,
        "RendererInServer::Pause failed, Illegal state: %{public}u", status_.load());
    status_ = I_STATUS_PAUSING;
    bool isStandbyTmp = false;
    if (standByEnable_) {
        AUDIO_INFO_LOG("sessionId: %{public}u call Pause while stand by", streamIndex_);
        CHECK_AND_RETURN_RET_LOG(audioServerBuffer_->GetStreamStatus() != nullptr,
            ERR_OPERATION_FAILED, "stream status is nullptr");
        standByEnable_ = false;
        enterStandbyTime_ = 0;
        audioServerBuffer_->GetStreamStatus()->store(STREAM_PAUSED);
        if (playerDfx_) {
            playerDfx_->WriteDfxActionMsg(streamIndex_, RENDERER_STAGE_STANDBY_END);
        }
        isStandbyTmp = true;
    }
    standByCounter_ = 0;
    GetEAC3ControlParam();
    int32_t ret = (managerType_ == DIRECT_PLAYBACK || managerType_ == VOIP_PLAYBACK || managerType_ == EAC3_PLAYBACK) ?
        IStreamManager::GetPlaybackManager(managerType_).PauseRender(streamIndex_) : stream_->Pause();

    if (IsMovieOffloadStream()) {
        SetSoftLinkFunc([](auto &softLink) { softLink->Stop(); });
    } else {
        std::lock_guard<std::mutex> lock(dupMutex_);
        for (auto &capInfo : captureInfos_) {
            if (IsEnabledAndValidDupStream(capInfo.second)) {
                capInfo.second.dupStream->Pause();
            }
        }
    }

    pausedTime_ = ClockTime::GetCurNano();
    if (isDualToneEnabled_ && dualToneStream_ != nullptr) {
        //Joint judgment ensures that there is a double ring and there is a stream to enter.
        stream_->SetAudioEffectMode(effectModeWhenDual_);
        std::lock_guard<std::mutex> lock(dualToneMutex_);
        //Locking before SetAudioEffectMode/GetAudioEffectMode results in a deadlock.
        if (dualToneStream_ != nullptr) {
            //Since there was no lock protection before the last time it was awarded dualToneStream_ it was
            //modified elsewhere, it was decided again after the lock was awarded.
            dualToneStream_->Pause();
        }
    }
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Pause stream failed, reason: %{public}d", ret);
    CoreServiceHandler::GetInstance().UpdateSessionOperation(streamIndex_, SESSION_OPERATION_PAUSE);
    audioStreamChecker_->MonitorOnAllCallback(AUDIO_STREAM_PAUSE, isStandbyTmp);
    PauseInner();

    return SUCCESS;
}

int32_t RendererInServer::FlushOhAudioBuffer()
{
    std::lock_guard writeLock(writeLock_);
    // Flush buffer of audio server
    uint64_t writeFrame = audioServerBuffer_->GetCurWriteFrame();
    uint64_t readFrame = audioServerBuffer_->GetCurReadFrame();
    if (readFrame >= writeFrame) {
        AUDIO_ERR_LOG("readFrame: %{public}" PRIu64 " writeFrame: %{public}" PRIu64 "", readFrame, writeFrame);
        return ERR_ILLEGAL_STATE;
    }
    RingBufferWrapper buffer;
    int32_t readResult = audioServerBuffer_->GetAllReadableBuffer(buffer);
    if (readResult != 0) {
        return ERR_OPERATION_FAILED;
    }
    buffer.SetBuffersValueWithSpecifyDataLen(0);
    AUDIO_INFO_LOG("On flush, read frame: %{public}" PRIu64 ", nextReadFrame: %{public}zu,"
        "writeFrame: %{public}" PRIu64 "", readFrame, spanSizeInFrame_, writeFrame);
    audioServerBuffer_->SetCurReadFrame(writeFrame);

    return SUCCESS;
}

int32_t RendererInServer::Flush()
{
    AUDIO_PRERELEASE_LOGI("Flush.");
    AudioXCollie audioXCollie(
        "RendererInServer::Flush", RELEASE_TIMEOUT_IN_SEC, nullptr, nullptr,
            AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    Trace trace(traceTag_ + " Flush");
    std::unique_lock<std::mutex> lock(statusLock_);
    if (status_ == I_STATUS_STARTED) {
        status_ = I_STATUS_FLUSHING_WHEN_STARTED;
    } else if (status_ == I_STATUS_PAUSED) {
        status_ = I_STATUS_FLUSHING_WHEN_PAUSED;
    } else if (status_ == I_STATUS_STOPPED) {
        status_ = I_STATUS_FLUSHING_WHEN_STOPPED;
    } else {
        AUDIO_ERR_LOG("failed, Illegal state: %{public}u", status_.load());
        return ERR_ILLEGAL_STATE;
    }

    FlushOhAudioBuffer();

    flushedTime_ = ClockTime::GetCurNano();
    int ret = stream_->Flush();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Flush stream failed, reason: %{public}d", ret);
    {
        std::lock_guard<std::mutex> lock(dupMutex_);
        for (auto &capInfo : captureInfos_) {
            if (capInfo.second.isInnerCapEnabled && capInfo.second.dupStream != nullptr) {
                capInfo.second.dupStream->Flush();
                renderEmptyCountForInnerCapToInnerCapIdMap_[capInfo.first] = OFFLOAD_INNER_CAP_PREBUF;
                InitDupBufferInner(capInfo.first);
            }
        }
    }
    if (isDualToneEnabled_) {
        std::lock_guard<std::mutex> lock(dualToneMutex_);
        if (dualToneStream_ != nullptr) {
            dualToneStream_->Flush();
        }
    }
    return SUCCESS;
}

int32_t RendererInServer::DrainAudioBuffer()
{
    return SUCCESS;
}

int32_t RendererInServer::Drain(bool stopFlag)
{
    AudioXCollie audioXCollie(
        "RendererInServer::Drain", RELEASE_TIMEOUT_IN_SEC, nullptr, nullptr,
            AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    {
        std::unique_lock<std::mutex> lock(statusLock_);
        if (status_ != I_STATUS_STARTED) {
            AUDIO_ERR_LOG("failed, Illegal state: %{public}u", status_.load());
            return ERR_ILLEGAL_STATE;
        }
        status_ = I_STATUS_DRAINING;
    }
    AUDIO_INFO_LOG("stopFlag:%{public}d", stopFlag);
    if (stopFlag) {
        std::lock_guard<std::mutex> lock(fadeoutLock_);
        AUDIO_INFO_LOG("fadeoutFlag_ = DO_FADINGOUT");
        fadeoutFlag_ = DO_FADINGOUT;
    }
    DrainAudioBuffer();
    drainedTime_ = ClockTime::GetCurNano();
    AudioPerformanceMonitor::GetInstance().StartSilenceMonitor(streamIndex_, processConfig_.appInfo.appTokenId);
    int ret = stream_->Drain(stopFlag);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Drain stream failed, reason: %{public}d", ret);
    {
        std::lock_guard<std::mutex> lock(dupMutex_);
        for (auto &capInfo : captureInfos_) {
            if (capInfo.second.isInnerCapEnabled && capInfo.second.dupStream != nullptr) {
                capInfo.second.dupStream->Drain(stopFlag);
            }
        }
    }
    if (isDualToneEnabled_) {
        std::lock_guard<std::mutex> lock(dualToneMutex_);
        if (dualToneStream_ != nullptr) {
            dualToneStream_->Drain(stopFlag);
        }
    }
    return SUCCESS;
}

int32_t RendererInServer::Stop()
{
    AUDIO_INFO_LOG("Stop.");
    AudioXCollie audioXCollie(
        "RendererInServer::Stop", RELEASE_TIMEOUT_IN_SEC, nullptr, nullptr,
            AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    {
        std::unique_lock<std::mutex> lock(statusLock_);
        if (status_ != I_STATUS_STARTED && status_ != I_STATUS_PAUSED && status_ != I_STATUS_DRAINING &&
            status_ != I_STATUS_STARTING) {
            AUDIO_ERR_LOG("failed, Illegal state: %{public}u", status_.load());
            return ERR_ILLEGAL_STATE;
        }
        status_ = I_STATUS_STOPPING;
    }
    int32_t ret = StopInner();
    XperfAdapter::GetInstance().ReportStateChangeEventIfNeed(XPERF_EVENT_STOP,
        processConfig_.rendererInfo.streamUsage, streamIndex_, processConfig_.appInfo.appPid,
        processConfig_.appInfo.appUid);

    RemoveStreamInfo();

    return ret;
}

int32_t RendererInServer::StopInner()
{
    if (standByEnable_) {
        AUDIO_INFO_LOG("sessionId: %{public}u call Stop while stand by", streamIndex_);
        CHECK_AND_RETURN_RET_LOG(audioServerBuffer_->GetStreamStatus() != nullptr,
            ERR_OPERATION_FAILED, "stream status is nullptr");
        standByEnable_ = false;
        enterStandbyTime_ = 0;
        audioServerBuffer_->GetStreamStatus()->store(STREAM_STOPPED);
        if (playerDfx_) {
            playerDfx_->WriteDfxActionMsg(streamIndex_, RENDERER_STAGE_STANDBY_END);
        }
    }
    {
        std::lock_guard<std::mutex> lock(fadeoutLock_);
        AUDIO_INFO_LOG("fadeoutFlag_ = NO_FADING");
        fadeoutFlag_ = NO_FADING;
    }
    GetEAC3ControlParam();
    int32_t ret = (managerType_ == DIRECT_PLAYBACK || managerType_ == VOIP_PLAYBACK || managerType_ == EAC3_PLAYBACK) ?
        IStreamManager::GetPlaybackManager(managerType_).StopRender(streamIndex_) : stream_->Stop();

    if (IsMovieOffloadStream()) {
        SetSoftLinkFunc([](auto &softLink) { softLink->Stop(); });
    } else {
        std::lock_guard<std::mutex> lock(dupMutex_);
        for (auto &capInfo : captureInfos_) {
            if (IsEnabledAndValidDupStream(capInfo.second)) {
                capInfo.second.dupStream->Stop();
            }
        }
    }

    if (isDualToneEnabled_ && dualToneStream_ != nullptr) {
        //Joint judgment ensures that there is a double ring and there is a stream to enter.
        stream_->SetAudioEffectMode(effectModeWhenDual_);
        std::lock_guard<std::mutex> lock(dualToneMutex_);
        //Locking before SetAudioEffectMode/GetAudioEffectMode results in a deadlock.
        if (dualToneStream_ != nullptr) {
            //Since there was no lock protection before the last time it was awarded dualToneStream_ it was
            //modified elsewhere, it was decided again after the lock was awarded.
            dualToneStream_->Stop();
        }
    }
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Stop stream failed, reason: %{public}d", ret);
    CoreServiceHandler::GetInstance().UpdateSessionOperation(streamIndex_, SESSION_OPERATION_STOP);
    audioStreamChecker_->MonitorOnAllCallback(AUDIO_STREAM_STOP, false);
    StreamDfxManager::GetInstance().CheckStreamOccupancy(streamIndex_, processConfig_, false);
    AudioPerformanceMonitor::GetInstance().PauseSilenceMonitor(streamIndex_);
    return SUCCESS;
}

int32_t RendererInServer::Release(bool isSwitchStream)
{
    AUDIO_INFO_LOG("Start release");
    AudioXCollie audioXCollie(
        "RendererInServer::Release", RELEASE_TIMEOUT_IN_SEC, nullptr, nullptr,
            AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    {
        std::unique_lock<std::mutex> lock(statusLock_);
        if (status_ == I_STATUS_RELEASED) {
            AUDIO_INFO_LOG("Already released");
            return SUCCESS;
        }
    }

    AudioStreamMonitor::GetInstance().DeleteCheckForMonitor(processConfig_.originalSessionId);
    if (processConfig_.audioMode == AUDIO_MODE_PLAYBACK) {
        AudioService::GetInstance()->SetDecMaxRendererStreamCnt();
        AudioService::GetInstance()->CleanAppUseNumMap(processConfig_.appInfo.appUid);
    }

    int32_t ret = CoreServiceHandler::GetInstance().UpdateSessionOperation(streamIndex_, SESSION_OPERATION_RELEASE);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Policy remove client failed, reason: %{public}d", ret);
    StreamDfxManager::GetInstance().CheckStreamOccupancy(streamIndex_, processConfig_, false);
    ret = IStreamManager::GetPlaybackManager(managerType_).ReleaseRender(streamIndex_);

    AudioVolume::GetInstance()->RemoveStreamVolume(streamIndex_);
    AudioService::GetInstance()->RemoveRenderer(streamIndex_, isSwitchStream);
    if (ret < 0) {
        AUDIO_ERR_LOG("Release stream failed, reason: %{public}d", ret);
        status_ = I_STATUS_INVALID;
        return ret;
    }
    RemoveIdForInjector();
    if (status_ != I_STATUS_STOPPING &&
        status_ != I_STATUS_STOPPED) {
        HandleOperationStopped(RENDERER_STAGE_STOP_BY_RELEASE);
    }
    status_ = I_STATUS_RELEASED;
    DisableAllInnerCap();

    DisableDualTone();

    XperfAdapter::GetInstance().ReportStateChangeEventIfNeed(XPERF_EVENT_RELEASE,
        processConfig_.rendererInfo.streamUsage, streamIndex_, processConfig_.appInfo.appPid,
        processConfig_.appInfo.appUid);

    RemoveStreamInfo();

    return SUCCESS;
}

void RendererInServer::RemoveIdForInjector()
{
    if (lastTarget_ == INJECT_TO_VOICE_COMMUNICATION_CAPTURE) {
        CoreServiceHandler::GetInstance().RemoveIdForInjector(streamIndex_);
    }
}

int32_t RendererInServer::DisableAllInnerCap()
{
    AUDIO_INFO_LOG("Disable all inner capture");
    {
        std::lock_guard<std::mutex> lock(dupMutex_);
        for (auto &capInfo : captureInfos_) {
            if (capInfo.second.isInnerCapEnabled) {
                DisableInnerCapHandle(capInfo.first);
            }
        }
        captureInfos_.clear();
    }

    for (auto &softLinkInfo : softLinkInfos_) {
        if (softLinkInfo.second.isSoftLinkEnabled) {
            DestroySoftLink(softLinkInfo.first);
        }
    }
    softLinkInfos_.clear();

    return SUCCESS;
}

int32_t RendererInServer::GetAudioTime(uint64_t &framePos, uint64_t &timestamp)
{
    if (status_ == I_STATUS_STOPPED) {
        AUDIO_WARNING_LOG("Current status is stopped");
        return ERR_ILLEGAL_STATE;
    }
    stream_->GetStreamFramesWritten(framePos);
    stream_->GetCurrentTimeStamp(timestamp);
    if (resetTime_) {
        resetTime_ = false;
        resetTimestamp_ = timestamp;
    }
    return SUCCESS;
}

int32_t RendererInServer::GetAudioPosition(uint64_t &framePos, uint64_t &timestamp, uint64_t &latency, int32_t base)
{
    if (status_ == I_STATUS_STOPPED) {
        AUDIO_PRERELEASE_LOGW("Current status is stopped");
        return ERR_ILLEGAL_STATE;
    }
    stream_->GetCurrentPosition(framePos, timestamp, latency, base);
    return SUCCESS;
}

int32_t RendererInServer::GetSpeedPosition(uint64_t &framePos, uint64_t &timestamp, uint64_t &latency, int32_t base)
{
    CHECK_AND_RETURN_RET_LOG(status_ != I_STATUS_STOPPED, ERR_ILLEGAL_STATE, "Current status is stopped");
    return stream_->GetSpeedPosition(framePos, timestamp, latency, base);
}

int32_t RendererInServer::GetLatency(uint64_t &latency)
{
    std::unique_lock<std::mutex> lock(statusLock_);
    if (managerType_ == DIRECT_PLAYBACK) {
        latency = IStreamManager::GetPlaybackManager(managerType_).GetLatency();
        return SUCCESS;
    }
    return stream_->GetLatency(latency);
}

int32_t RendererInServer::SetRate(int32_t rate)
{
    return stream_->SetRate(rate);
}

int32_t RendererInServer::SetLowPowerVolume(float volume)
{
    if (volume < MIN_FLOAT_VOLUME || volume > MAX_FLOAT_VOLUME) {
        AUDIO_ERR_LOG("invalid volume:%{public}f", volume);
        return ERR_INVALID_PARAM;
    }
    std::string currentTime = GetTime();
    AUDIO_INFO_LOG("SetLowPowerVolumeInfo volume: %{public}f, sessionID: %{public}d, adjustTime: %{public}s",
        volume, streamIndex_, currentTime.c_str());
    AudioVolume::GetInstance()->SaveAdjustStreamVolumeInfo(volume, streamIndex_, currentTime,
        static_cast<uint32_t>(AdjustStreamVolume::LOW_POWER_VOLUME_INFO));

    lowPowerVolume_ = volume;
    AudioVolume::GetInstance()->SetStreamVolumeLowPowerFactor(streamIndex_, volume);
    {
        std::lock_guard<std::mutex> lock(dupMutex_);
        for (auto &capInfo : captureInfos_) {
            if (capInfo.second.isInnerCapEnabled && capInfo.second.dupStream != nullptr) {
                AudioVolume::GetInstance()->SetStreamVolumeLowPowerFactor(
                    capInfo.second.dupStream->GetStreamIndex(), volume);
            }
        }
    }
    SetSoftLinkFunc([volume](auto &softLink) { softLink->SetVolumeLowPowerFactor(volume); });
    if (isDualToneEnabled_) {
        AudioVolume::GetInstance()->SetStreamVolumeLowPowerFactor(dualToneStreamIndex_, volume);
    }
    if (offloadEnable_) {
        OffloadSetVolumeInner();
    }
    return SUCCESS;
}

int32_t RendererInServer::GetLowPowerVolume(float &volume)
{
    volume = lowPowerVolume_;
    return SUCCESS;
}

int32_t RendererInServer::SetAudioEffectMode(int32_t effectMode)
{
    if (isDualToneEnabled_) {
        effectModeWhenDual_ = effectMode;
        return SUCCESS;
    }
    return stream_->SetAudioEffectMode(effectMode);
}

int32_t RendererInServer::GetAudioEffectMode(int32_t &effectMode)
{
    return stream_->GetAudioEffectMode(effectMode);
}

int32_t RendererInServer::SetPrivacyType(int32_t privacyType)
{
    return stream_->SetPrivacyType(privacyType);
}

int32_t RendererInServer::GetPrivacyType(int32_t &privacyType)
{
    return stream_->GetPrivacyType(privacyType);
}

int32_t RendererInServer::EnableInnerCap(int32_t innerCapId)
{
    // in plan
    AUDIO_INFO_LOG("EnableInnerCap start:");
    if (IsMovieOffloadStream()) {
        int32_t ret = InitSoftLink(innerCapId);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "Init softLink failed");
    } else {
        int32_t ret = InitDupStream(innerCapId);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "Init dup stream failed");
    }

    return SUCCESS;
}

int32_t RendererInServer::DisableInnerCap(int32_t innerCapId)
{
    AUDIO_INFO_LOG("DisableInnerCap start:");
    if (IsMovieOffloadStream()) {
        std::lock_guard<std::mutex> lock(dupMutex_);
        int32_t ret = DestroySoftLink(innerCapId);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "Destory softLink failed");
    } else {
        std::lock_guard<std::mutex> lock(dupMutex_);
        int32_t ret = DisableInnerCapHandle(innerCapId);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "Destory dup stream failed");
    }
    return SUCCESS;
}

int32_t RendererInServer::DisableInnerCapHandle(int32_t innerCapId)
{
    if (!captureInfos_.count(innerCapId) || !captureInfos_[innerCapId].isInnerCapEnabled) {
        AUDIO_WARNING_LOG("InnerCap is already disabled.capId:%{public}d", innerCapId);
        return ERR_INVALID_OPERATION;
    }
    captureInfos_[innerCapId].isInnerCapEnabled = false;
    AUDIO_INFO_LOG("Disable dup renderer %{public}u with status: %{public}d", streamIndex_, status_.load());
    // in plan: call stop?
    if (captureInfos_[innerCapId].dupStream != nullptr) {
        uint32_t dupStreamIndex = captureInfos_[innerCapId].dupStream->GetStreamIndex();
        IStreamManager::GetDupPlaybackManager().ReleaseRender(dupStreamIndex);
        AudioVolume::GetInstance()->RemoveStreamVolume(dupStreamIndex);
        captureInfos_[innerCapId].dupStream = nullptr;
    }
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        DumpFileUtil::CloseDumpFile(&dumpDupIn_);
    }
    return SUCCESS;
}

int32_t RendererInServer::InitDupStream(int32_t innerCapId)
{
    AUDIO_INFO_LOG("InitDupStream for innerCapId:%{public}d", innerCapId);
    Trace trace(traceTag_ + "InitDupStream innerCapId:" + std::to_string(innerCapId));
    std::lock_guard<std::mutex> lock(dupMutex_);
    if (captureInfos_.count(innerCapId) && captureInfos_[innerCapId].isInnerCapEnabled) {
        AUDIO_INFO_LOG("InnerCap is already enabled,id:%{public}d", innerCapId);
        return SUCCESS;
    }
    auto &capInfo = captureInfos_[innerCapId];
    AudioProcessConfig dupConfig = processConfig_;
    dupConfig.innerCapId = innerCapId;
    int32_t ret = IStreamManager::GetDupPlaybackManager().CreateRender(dupConfig, capInfo.dupStream);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS && capInfo.dupStream != nullptr,
        ERR_OPERATION_FAILED, "Failed: %{public}d", ret);
    uint32_t dupStreamIndex = capInfo.dupStream->GetStreamIndex();
    bool isSystemApp = CheckoutSystemAppUtil::CheckoutSystemApp(processConfig_.appInfo.appUid);
    StreamVolumeParams streamVolumeParams = { dupStreamIndex, processConfig_.streamType,
        processConfig_.rendererInfo.streamUsage, processConfig_.appInfo.appUid, processConfig_.appInfo.appPid,
        isSystemApp, processConfig_.rendererInfo.volumeMode, processConfig_.rendererInfo.isVirtualKeyboard };
    AudioVolume::GetInstance()->AddStreamVolume(streamVolumeParams);
    innerCapIdToDupStreamCallbackMap_[innerCapId] = std::make_shared<StreamCallbacks>(dupStreamIndex);
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        ret = CreateDupBufferInner(innerCapId);
        dumpDupInFileName_ = std::to_string(dupStreamIndex) + "_dup_in_" + ".pcm";
        DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpDupInFileName_, &dumpDupIn_);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "Config dup buffer failed");
    }
    // todo check index
    capInfo.dupStream->RegisterStatusCallback(innerCapIdToDupStreamCallbackMap_[innerCapId]);
    capInfo.dupStream->RegisterWriteCallback(innerCapIdToDupStreamCallbackMap_[innerCapId]);

    AUDIO_INFO_LOG("Dup Renderer %{public}u with status: %{public}d", streamIndex_, status_.load());
    capInfo.isInnerCapEnabled = true;
    InitDupStreamVolume(dupStreamIndex);
    capInfo.dupStream->SetLoudnessGain(loudnessGain_);

    if (status_ == I_STATUS_STARTED) {
        AUDIO_INFO_LOG("Renderer %{public}u is already running, let's start the dup stream", streamIndex_);
        capInfo.dupStream->Start();

        if (offloadEnable_) {
            renderEmptyCountForInnerCapToInnerCapIdMap_[innerCapId] = OFFLOAD_INNER_CAP_PREBUF;
        }
    }
    return SUCCESS;
}

int32_t RendererInServer::InitDupStreamVolume(uint32_t dupStreamIndex)
{
    if (audioServerBuffer_ != nullptr) {
        float clientVolume = audioServerBuffer_->GetStreamVolume();
        float duckFactor = audioServerBuffer_->GetDuckFactor();
        bool isMuted = (isMuted_ || silentModeAndMixWithOthers_ || muteFlag_);
        // If some factors are not needed, remove them.
        AudioVolume::GetInstance()->SetStreamVolume(dupStreamIndex, clientVolume);
        AudioVolume::GetInstance()->SetStreamVolumeDuckFactor(dupStreamIndex, duckFactor);
        AudioVolume::GetInstance()->SetStreamVolumeMute(dupStreamIndex, isMuted);
        AudioVolume::GetInstance()->SetStreamVolumeLowPowerFactor(dupStreamIndex, lowPowerVolume_);
    }
    return SUCCESS;
}

int32_t RendererInServer::DisableDualToneInner()
{
    isDualToneEnabled_ = false;
    dupSinkName_ = std::nullopt;
    AUDIO_INFO_LOG("Disable dual tone renderer:[%{public}u] with status: %{public}d",
        dualToneStreamIndex_, status_.load());
    IStreamManager::GetDualPlaybackManager().ReleaseRender(dualToneStreamIndex_);
    AudioVolume::GetInstance()->RemoveStreamVolume(dualToneStreamIndex_);
    dualToneStream_ = nullptr;

    return ERROR;
}

int32_t RendererInServer::DisableDualTone()
{
    std::lock_guard<std::mutex> lock(dualToneMutex_);
    if (!isDualToneEnabled_) {
        AUDIO_WARNING_LOG("DualTone is already disabled.");
        return ERR_INVALID_OPERATION;
    }

    return DisableDualToneInner();
}

void RendererInServer::PreDualToneBufferSilenceForOffload()
{
    if (offloadEnable_) {
        size_t emptyBufferSize = OFFLOAD_DUAL_RENDER_PREBUF * spanSizeInByte_;
        auto buffer = std::make_unique<uint8_t []>(emptyBufferSize);
        BufferDesc emptyBufferDesc = {buffer.get(), emptyBufferSize, emptyBufferSize};
        memset_s(emptyBufferDesc.buffer, emptyBufferSize, 0, emptyBufferSize);
        dualToneStream_->EnqueueBuffer(emptyBufferDesc);
    }
}

int32_t RendererInServer::EnableDualTone(const std::string &dupSinkName)
{
    {
        std::lock_guard<std::mutex> lock(dualToneMutex_);
        bool isEnabled = isDualToneEnabled_.load();
        if (isEnabled && dupSinkName_ == dupSinkName) {
            AUDIO_INFO_LOG("DualTone is already enabled");
            return SUCCESS;
        }

        if (isEnabled) {
            DisableDualToneInner();
        }

        int32_t ret = IStreamManager::GetDualPlaybackManager().CreateRender(processConfig_, dualToneStream_,
            dupSinkName);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS && dualToneStream_ != nullptr,
            ERR_OPERATION_FAILED, "Failed: %{public}d", ret);
        dualToneStreamIndex_ = dualToneStream_->GetStreamIndex();
        AUDIO_INFO_LOG("init dual tone renderer:[%{public}u]", dualToneStreamIndex_);
        bool isSystemApp = CheckoutSystemAppUtil::CheckoutSystemApp(processConfig_.appInfo.appUid);
        StreamVolumeParams streamVolumeParams = { dualToneStreamIndex_, processConfig_.streamType,
            processConfig_.rendererInfo.streamUsage, processConfig_.appInfo.appUid, processConfig_.appInfo.appPid,
            isSystemApp, processConfig_.rendererInfo.volumeMode, processConfig_.rendererInfo.isVirtualKeyboard };
        AudioVolume::GetInstance()->AddStreamVolume(streamVolumeParams);

        PreDualToneBufferSilenceForOffload();

        isDualToneEnabled_ = true;
        dupSinkName_ = dupSinkName;
    }

    if (audioServerBuffer_ != nullptr) {
        float clientVolume = audioServerBuffer_->GetStreamVolume();
        float duckFactor = audioServerBuffer_->GetDuckFactor();
        bool isMuted = (isMuted_ || silentModeAndMixWithOthers_ || muteFlag_);
        // If some factors are not needed, remove them.
        AudioVolume::GetInstance()->SetStreamVolume(dualToneStreamIndex_, clientVolume);
        AudioVolume::GetInstance()->SetStreamVolumeDuckFactor(dualToneStreamIndex_, duckFactor);
        AudioVolume::GetInstance()->SetStreamVolumeMute(dualToneStreamIndex_, isMuted);
        AudioVolume::GetInstance()->SetStreamVolumeLowPowerFactor(dualToneStreamIndex_, lowPowerVolume_);
    }
    if (status_ == I_STATUS_STARTED) {
        AUDIO_INFO_LOG("Renderer %{public}u is already running, let's start the dual stream", dualToneStreamIndex_);
        stream_->GetAudioEffectMode(effectModeWhenDual_);
        stream_->SetAudioEffectMode(EFFECT_NONE);
        std::lock_guard<std::mutex> lock(dualToneMutex_);
        //Locking before SetAudioEffectMode/GetAudioEffectMode results in a deadlock.
        if (dualToneStream_ != nullptr) {
            //Since there was no lock protection before the last time it was awarded dualToneStream_ it was
            //modified elsewhere, it was decided again after the lock was awarded.
            dualToneStream_->SetAudioEffectMode(EFFECT_NONE);
            dualToneStream_->Start();
        }
    }
    return SUCCESS;
}

StreamCallbacks::StreamCallbacks(uint32_t streamIndex) : streamIndex_(streamIndex)
{
    AUDIO_INFO_LOG("DupStream %{public}u create StreamCallbacks", streamIndex_);
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        dumpDupOutFileName_ = std::to_string(streamIndex_) + "_dup_out_" + ".pcm";
        DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpDupOutFileName_, &dumpDupOut_);
    }
}
 
StreamCallbacks::~StreamCallbacks()
{
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        DumpFileUtil::CloseDumpFile(&dumpDupOut_);
    }
}

void StreamCallbacks::OnStatusUpdate(IOperation operation)
{
    AUDIO_INFO_LOG("DupStream %{public}u recv operation: %{public}d", streamIndex_, operation);
}

int32_t StreamCallbacks::OnWriteData(size_t length)
{
    Trace trace("DupStream::OnWriteData length " + std::to_string(length));
    return SUCCESS;
}

int32_t StreamCallbacks::OnWriteData(int8_t *inputData, size_t requestDataLen)
{
    Trace trace("DupStream::OnWriteData length " + std::to_string(requestDataLen));
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1 && dupRingBuffer_ != nullptr) {
        std::unique_ptr<AudioRingCache> &dupBuffer = dupRingBuffer_;
        // no need mutex
        // todo wait readable
        AUDIO_INFO_LOG("running");
        OptResult result = dupBuffer->GetReadableSize();
        CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERROR,
            "dupBuffer get readable size failed, size is:%{public}zu", result.size);
        if (result.size == 0 || result.size < requestDataLen) {
            recoveryAntiShakeBufferCount_ = DUP_RECOVERY_AUTISHAKE_BUFFER_COUNT;
            AUDIO_INFO_LOG("Readable size is invaild, result.size:%{public}zu, requstDataLen:%{public}zu",
                result.size, requestDataLen);
            return ERROR;
        }
        if (recoveryAntiShakeBufferCount_ > 0) {
            recoveryAntiShakeBufferCount_--;
            AUDIO_INFO_LOG("need recovery data anti-shake, no onWriteData, recoveryAntiShakeBufferCount_: %{public}d",
                recoveryAntiShakeBufferCount_);
            return ERROR;
        }
        AUDIO_DEBUG_LOG("requstDataLen is:%{public}zu readSize is:%{public}zu", requestDataLen, result.size);
        result = dupBuffer->Dequeue({reinterpret_cast<uint8_t *>(inputData), requestDataLen});
        CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERROR, "dupBuffer dequeue failed");
        DumpFileUtil::WriteDumpFile(dumpDupOut_, static_cast<void *>(inputData), requestDataLen);
    }
    return SUCCESS;
}

int32_t StreamCallbacks::GetAvailableSize(size_t &length)
{
    if (dupRingBuffer_ == nullptr) {
        AUDIO_ERR_LOG("nullptr");
        return ERROR;
    }

    OptResult result = dupRingBuffer_->GetReadableSize();
    CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERROR,
        "get readable size failed, size is:%{public}zu", result.size);

    length = result.size;
    return SUCCESS;
}

std::unique_ptr<AudioRingCache>& StreamCallbacks::GetDupRingBuffer()
{
    return dupRingBuffer_;
}

void RendererInServer::HandleOffloadStream(const int32_t captureId, const CaptureInfo& captureInfo)
{
    if (IsMovieStream()) {
        if (status_ == I_STATUS_STARTED) {
            captureInfo.dupStream->Stop();
        }
        captureInfo.dupStream->Flush();
        auto it = softLinkInfos_.find(captureId);
        if (it == softLinkInfos_.end() || !it->second.isSoftLinkEnabled) {
            InitSoftLink(captureId);
        } else if (status_ == I_STATUS_STARTED && it->second.softLink != nullptr) {
            it->second.softLink->Start();
        }
    } else {
        captureInfo.dupStream->UpdateMaxLength(350);  // 350 for cover offload
    }
}

int32_t RendererInServer::SetOffloadMode(int32_t state, bool isAppBack)
{
    AUDIO_INFO_LOG("SetOffloadMode, status: %{public}u", status_.load());
    int32_t ret = stream_->SetOffloadMode(state, isAppBack);
    {
        std::lock_guard<std::mutex> lock(dupMutex_);
        for (auto &capInfo : captureInfos_) {
            if (capInfo.second.isInnerCapEnabled && capInfo.second.dupStream != nullptr) {
                HandleOffloadStream(capInfo.first, capInfo.second);
            }
        }
    }
    if (isDualToneEnabled_) {
        std::lock_guard<std::mutex> lock(dualToneMutex_);
        if (dualToneStream_ != nullptr) {
            dualToneStream_->UpdateMaxLength(350); // 350 for cover offload
        }
    }
    return ret;
}

int32_t RendererInServer::UnsetOffloadMode()
{
    AUDIO_INFO_LOG("UnsetOffloadMode, status: %{public}u", status_.load());
    int32_t ret = stream_->UnsetOffloadMode();

    for (auto &softInfo : softLinkInfos_) {
        if (!IsEnabledAndValidSoftLink(softInfo.second)) {
            AUDIO_INFO_LOG("The soft link is not valid %{public}d", softInfo.first);
            continue;
        }
        if (status_ == I_STATUS_STARTED) {
            softInfo.second.softLink->Stop();
        }
        if (!captureInfos_.count(softInfo.first) || !captureInfos_[softInfo.first].isInnerCapEnabled) {
            InitDupStream(softInfo.first);
        }
    }

    {
        std::lock_guard<std::mutex> lock(dupMutex_);
        for (auto &capInfo : captureInfos_) {
            if (capInfo.second.isInnerCapEnabled && capInfo.second.dupStream != nullptr) {
                capInfo.second.dupStream->UpdateMaxLength(20);  // 20 for unset offload
                if (status_ == I_STATUS_STARTED) {
                    capInfo.second.dupStream->Start();
                }
            }
        }
    }

    if (isDualToneEnabled_) {
        std::lock_guard<std::mutex> lock(dualToneMutex_);
        if (dualToneStream_ != nullptr) {
            dualToneStream_->UpdateMaxLength(20); // 20 for cover offload
        }
    }
    return ret;
}

int32_t RendererInServer::GetOffloadApproximatelyCacheTime(uint64_t &timestamp, uint64_t &paWriteIndex,
    uint64_t &cacheTimeDsp, uint64_t &cacheTimePa)
{
    return stream_->GetOffloadApproximatelyCacheTime(timestamp, paWriteIndex, cacheTimeDsp, cacheTimePa);
}

int32_t RendererInServer::OffloadSetVolumeInner()
{
    return stream_->OffloadSetVolume();
}

int32_t RendererInServer::UpdateSpatializationState(bool spatializationEnabled, bool headTrackingEnabled)
{
    return stream_->UpdateSpatializationState(spatializationEnabled, headTrackingEnabled);
}

int32_t RendererInServer::GetStreamManagerType() const noexcept
{
    return managerType_ == DIRECT_PLAYBACK ? AUDIO_DIRECT_MANAGER_TYPE : AUDIO_NORMAL_MANAGER_TYPE;
}

bool RendererInServer::IsHighResolution() const noexcept
{
    Trace trace("CheckHighResolution");
    if (processConfig_.deviceType != DEVICE_TYPE_WIRED_HEADSET &&
        processConfig_.deviceType != DEVICE_TYPE_USB_HEADSET) {
        AUDIO_INFO_LOG("normal stream,device type:%{public}d", processConfig_.deviceType);
        return false;
    }
    if (processConfig_.streamType != STREAM_MUSIC || processConfig_.streamInfo.samplingRate < SAMPLE_RATE_48000 ||
        processConfig_.streamInfo.format < SAMPLE_S24LE) {
        AUDIO_INFO_LOG("normal stream because stream info");
        return false;
    }
    if (processConfig_.streamInfo.samplingRate > SAMPLE_RATE_192000) {
        AUDIO_INFO_LOG("sample rate over 192k");
        return false;
    }
    if (IStreamManager::GetPlaybackManager(DIRECT_PLAYBACK).GetStreamCount() > 0) {
        AUDIO_INFO_LOG("high resolution exist.");
        return false;
    }
    return true;
}

int32_t RendererInServer::SetSilentModeAndMixWithOthers(bool on)
{
    silentModeAndMixWithOthers_ = on;
    bool isMuted = (isMuted_ || on || muteFlag_);
    AudioVolume::GetInstance()->SetStreamVolumeMute(streamIndex_, isMuted);
    {
        std::lock_guard<std::mutex> lock(dupMutex_);
        for (auto &capInfo : captureInfos_) {
            if (capInfo.second.isInnerCapEnabled && capInfo.second.dupStream != nullptr) {
                AudioVolume::GetInstance()->SetStreamVolumeMute(
                    capInfo.second.dupStream->GetStreamIndex(), isMuted);
            }
        }
    }
    SetSoftLinkFunc([isMuted](auto &softLink) { softLink->SetVolumeMute(isMuted); });
    if (isDualToneEnabled_) {
        AudioVolume::GetInstance()->SetStreamVolumeMute(dualToneStreamIndex_, isMuted);
    }
    if (offloadEnable_) {
        OffloadSetVolumeInner();
    }
    AudioService::GetInstance()->RenderersCheckForAudioWorkgroup(processConfig_.appInfo.appPid);
    return SUCCESS;
}

int32_t RendererInServer::SetClientVolume()
{
    if (audioServerBuffer_ == nullptr || playerDfx_ == nullptr) {
        AUDIO_WARNING_LOG("buffer in not inited");
        return ERROR;
    }
    float clientVolume = audioServerBuffer_->GetStreamVolume();
    std::string currentTime = GetTime();
    AUDIO_INFO_LOG("SetVolumeInfo volume: %{public}f, sessionID: %{public}d, adjustTime: %{public}s",
        clientVolume, streamIndex_, currentTime.c_str());
    AudioVolume::GetInstance()->SaveAdjustStreamVolumeInfo(clientVolume, streamIndex_, currentTime,
        static_cast<uint32_t>(AdjustStreamVolume::STREAM_VOLUME_INFO));
    int32_t ret = stream_->SetClientVolume(clientVolume);
    SetStreamVolumeInfoForEnhanceChain();
    AudioVolume::GetInstance()->SetStreamVolume(streamIndex_, clientVolume);
    {
        std::lock_guard<std::mutex> lock(dupMutex_);
        for (auto &capInfo : captureInfos_) {
            if (capInfo.second.isInnerCapEnabled && capInfo.second.dupStream != nullptr) {
                AudioVolume::GetInstance()->SetStreamVolume(
                    capInfo.second.dupStream->GetStreamIndex(), clientVolume);
            }
        }
    }
    SetSoftLinkFunc([clientVolume](auto &softLink) { softLink->SetVolume(clientVolume); });
    if (isDualToneEnabled_) {
        AudioVolume::GetInstance()->SetStreamVolume(dualToneStreamIndex_, clientVolume);
    }
    if (offloadEnable_) {
        OffloadSetVolumeInner();
    }

    RendererStage stage = static_cast<size_t>(clientVolume) == 0 ?
        RENDERER_STAGE_SET_VOLUME_ZERO : RENDERER_STAGE_SET_VOLUME_NONZERO;
    playerDfx_->WriteDfxActionMsg(streamIndex_, stage);
    return ret;
}

int32_t RendererInServer::SetLoudnessGain(float loudnessGain)
{
    loudnessGain_ = loudnessGain;
    int32_t ret = stream_->SetLoudnessGain(loudnessGain);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "setloudnessGain failed");
    {
        std::lock_guard<std::mutex> lock(dupMutex_);
        for (auto &capInfo : captureInfos_) {
            if (capInfo.second.isInnerCapEnabled && capInfo.second.dupStream != nullptr) {
                ret += capInfo.second.dupStream->SetLoudnessGain(loudnessGain);
            }
        }
    }

    for (auto &softLinkInfo : softLinkInfos_) {
        if (softLinkInfo.second.isSoftLinkEnabled && softLinkInfo.second.softLink != nullptr) {
            ret += softLinkInfo.second.softLink->SetLoudnessGain(loudnessGain);
        }
    }

    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "setloudnessGain failed during capture, error: %{public}d", ret);
    return SUCCESS;
}

int32_t RendererInServer::SetMute(bool isMute)
{
    isMuted_ = isMute;
    AUDIO_INFO_LOG("SetStreamVolumeMute:%{public}d", isMute);
    bool isMuted = (isMute || silentModeAndMixWithOthers_ || muteFlag_);
    AudioVolume::GetInstance()->SetStreamVolumeMute(streamIndex_, isMuted);
    {
        std::lock_guard<std::mutex> lock(dupMutex_);
        for (auto &capInfo : captureInfos_) {
            if (capInfo.second.isInnerCapEnabled && capInfo.second.dupStream != nullptr) {
                AudioVolume::GetInstance()->SetStreamVolumeMute(
                    capInfo.second.dupStream->GetStreamIndex(), isMuted);
            }
        }
    }
    SetSoftLinkFunc([isMuted](auto &softLink) { softLink->SetVolumeMute(isMuted); });
    if (isDualToneEnabled_) {
        AudioVolume::GetInstance()->SetStreamVolumeMute(dualToneStreamIndex_, isMuted);
    }
    if (offloadEnable_) {
        OffloadSetVolumeInner();
    }
    return SUCCESS;
}

int32_t RendererInServer::SetDuckFactor(float duckFactor)
{
    if (duckFactor < MIN_FLOAT_VOLUME || duckFactor > MAX_FLOAT_VOLUME) {
        AUDIO_ERR_LOG("invalid duck volume:%{public}f", duckFactor);
        return ERR_INVALID_PARAM;
    }

    std::string currentTime = GetTime();
    AUDIO_INFO_LOG("SetDuckVolumeInfo volume: %{public}f, sessionID: %{public}d, adjustTime: %{public}s",
        duckFactor, streamIndex_, currentTime.c_str());
    AudioVolume::GetInstance()->SaveAdjustStreamVolumeInfo(duckFactor, streamIndex_, currentTime,
        static_cast<uint32_t>(AdjustStreamVolume::DUCK_VOLUME_INFO));

    AudioVolume::GetInstance()->SetStreamVolumeDuckFactor(streamIndex_, duckFactor);
    {
        std::lock_guard<std::mutex> lock(dupMutex_);
        for (auto &capInfo : captureInfos_) {
            if (capInfo.second.isInnerCapEnabled && capInfo.second.dupStream != nullptr) {
                AudioVolume::GetInstance()->SetStreamVolumeDuckFactor(
                    capInfo.second.dupStream->GetStreamIndex(), duckFactor);
            }
        }
    }
    SetSoftLinkFunc([duckFactor](auto &softLink) { softLink->SetVolumeDuckFactor(duckFactor); });
    if (isDualToneEnabled_) {
        AudioVolume::GetInstance()->SetStreamVolumeDuckFactor(dualToneStreamIndex_, duckFactor);
    }
    if (offloadEnable_) {
        OffloadSetVolumeInner();
    }
    return SUCCESS;
}

int32_t RendererInServer::SetStreamVolumeInfoForEnhanceChain()
{
    uint32_t sessionId = streamIndex_;
    float streamVolume = audioServerBuffer_->GetStreamVolume();
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        return HPAE::IHpaeManager::GetHpaeManager().SetStreamVolumeInfo(sessionId, streamVolume);
    } else {
        AudioEnhanceChainManager *audioEnhanceChainManager = AudioEnhanceChainManager::GetInstance();
        CHECK_AND_RETURN_RET_LOG(audioEnhanceChainManager != nullptr, ERROR, "audioEnhanceChainManager is nullptr");
        int32_t ret = audioEnhanceChainManager->SetStreamVolumeInfo(sessionId, streamVolume);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetStreamVolumeInfo failed");
        return ret;
    }
}

void RendererInServer::OnDataLinkConnectionUpdate(IOperation operation)
{
    std::shared_ptr<IStreamListener> stateListener = streamListener_.lock();
    CHECK_AND_RETURN_LOG(stateListener != nullptr, "StreamListener is nullptr");
    switch (operation) {
        case OPERATION_DATA_LINK_CONNECTING:
            {
                std::lock_guard<std::mutex> lock(dataConnectionMutex_);
                isDataLinkConnected_ = false;
            }
            AUDIO_DEBUG_LOG("OPERATION_DATA_LINK_CONNECTING received");
            break;
        case OPERATION_DATA_LINK_CONNECTED:
            {
                std::lock_guard<std::mutex> lock(dataConnectionMutex_);
                isDataLinkConnected_ = true;
                dataConnectionCV_.notify_all();
            }
            AUDIO_DEBUG_LOG("OPERATION_DATA_LINK_CONNECTED received");
            break;
        default:
            return;
    }
}

int32_t RendererInServer::GetActualStreamManagerType() const noexcept
{
    return managerType_;
}

static std::string GetStatusStr(IStatus status)
{
    switch (status) {
        case I_STATUS_INVALID:
            return "INVALID";
        case I_STATUS_IDLE:
            return "IDEL";
        case I_STATUS_STARTING:
            return "STARTING";
        case I_STATUS_STARTED:
            return "STARTED";
        case I_STATUS_PAUSING:
            return "PAUSING";
        case I_STATUS_PAUSED:
            return "PAUSED";
        case I_STATUS_FLUSHING_WHEN_STARTED:
            return "FLUSHING_WHEN_STARTED";
        case I_STATUS_FLUSHING_WHEN_PAUSED:
            return "FLUSHING_WHEN_PAUSED";
        case I_STATUS_FLUSHING_WHEN_STOPPED:
            return "FLUSHING_WHEN_STOPPED";
        case I_STATUS_DRAINING:
            return "DRAINING";
        case I_STATUS_DRAINED:
            return "DRAINED";
        case I_STATUS_STOPPING:
            return "STOPPING";
        case I_STATUS_STOPPED:
            return "STOPPED";
        case I_STATUS_RELEASING:
            return "RELEASING";
        case I_STATUS_RELEASED:
            return "RELEASED";
        default:
            break;
    }
    return "NO_SUCH_STATUS";
}

static std::string GetManagerTypeStr(ManagerType type)
{
    switch (type) {
        case PLAYBACK:
            return "Normal";
        case DUP_PLAYBACK:
            return "Dup Playback";
        case DUAL_PLAYBACK:
            return "DUAL Playback";
        case DIRECT_PLAYBACK:
            return "Direct";
        case VOIP_PLAYBACK:
            return "Voip";
        case RECORDER:
            return "Recorder";
        default:
            break;
    }
    return "NO_SUCH_TYPE";
}

bool RendererInServer::Dump(std::string &dumpString)
{
    bool ret = false;
    ret = DumpNormal(dumpString);
    CHECK_AND_RETURN_RET_LOG(ret == false, true, "DumpNormal");
    ret = DumpVoipAndDirect(dumpString);
    CHECK_AND_RETURN_RET_LOG(ret == false, true, "DumpVoipAndDirect");
    return ret;
}

bool RendererInServer::DumpNormal(std::string &dumpString)
{
    if (managerType_ != PLAYBACK) {
        return false;
    }
    DumpStreamInfo(dumpString);
    AppendFormat(dumpString, "  - stream type:%d\n", lastTarget_);
    DumpStatusInfo(dumpString);
    return true;
}

bool RendererInServer::DumpVoipAndDirect(std::string &dumpString)
{
    if (managerType_ != DIRECT_PLAYBACK && managerType_ != VOIP_PLAYBACK) {
        return false;
    }
    DumpStreamInfo(dumpString);
    DumpStatusInfo(dumpString);
    return true;
}
void RendererInServer::DumpStreamInfo(std::string &dumpString)
{
    // dump audio stream info
    dumpString += "audio stream info:\n";
    AppendFormat(dumpString, "  - session id:%u\n", streamIndex_);
    AppendFormat(dumpString, "  - appid:%d\n", processConfig_.appInfo.appPid);
    AppendFormat(dumpString, "  - stream type:%d\n", processConfig_.streamType);

    AppendFormat(dumpString, "  - samplingRate: %d\n", processConfig_.streamInfo.samplingRate);
    AppendFormat(dumpString, "  - channels: %u\n", processConfig_.streamInfo.channels);
    AppendFormat(dumpString, "  - format: %u\n", processConfig_.streamInfo.format);
    AppendFormat(dumpString, "  - device type: %u\n", processConfig_.deviceType);
    AppendFormat(dumpString, "  - sink type: %s\n", GetManagerTypeStr(managerType_).c_str());
}
    
void RendererInServer::DumpStatusInfo(std::string &dumpString)
{
    // dump status info
    AppendFormat(dumpString, "  - Current stream status: %s\n", GetStatusStr(status_.load()).c_str());
    if (audioServerBuffer_ != nullptr) {
        AppendFormat(dumpString, "  - Current read position: %u\n", audioServerBuffer_->GetCurReadFrame());
        AppendFormat(dumpString, "  - Current write position: %u\n", audioServerBuffer_->GetCurWriteFrame());
    }

    dumpString += "\n";
}

void RendererInServer::SetNonInterruptMute(const bool muteFlag)
{
    AUDIO_INFO_LOG("mute flag %{public}d", muteFlag);
    muteFlag_ = muteFlag;
    AudioService::GetInstance()->UpdateMuteControlSet(streamIndex_, muteFlag);

    bool isMuted = (isMuted_ || silentModeAndMixWithOthers_ || muteFlag);
    AudioVolume::GetInstance()->SetStreamVolumeMute(streamIndex_, isMuted);
    {
        std::lock_guard<std::mutex> lock(dupMutex_);
        for (auto &captureInfo : captureInfos_) {
            if (captureInfo.second.isInnerCapEnabled && captureInfo.second.dupStream != nullptr) {
                AudioVolume::GetInstance()->SetStreamVolumeMute(
                    captureInfo.second.dupStream->GetStreamIndex(), isMuted);
            }
        }
    }
    SetSoftLinkFunc([isMuted](auto &softLink) { softLink->SetVolumeMute(isMuted); });
    if (isDualToneEnabled_) {
        AudioVolume::GetInstance()->SetStreamVolumeMute(dualToneStreamIndex_, isMuted);
    }
    if (offloadEnable_) {
        OffloadSetVolumeInner();
    }
}

RestoreStatus RendererInServer::RestoreSession(RestoreInfo restoreInfo)
{
    RestoreStatus restoreStatus = audioServerBuffer_->SetRestoreStatus(NEED_RESTORE);
    if (restoreStatus == NEED_RESTORE) {
        audioServerBuffer_->SetRestoreInfo(restoreInfo);
    }
    audioServerBuffer_->WakeFutex();
    return restoreStatus;
}

int32_t RendererInServer::SetDefaultOutputDevice(const DeviceType defaultOutputDevice, bool skipForce)
{
    return CoreServiceHandler::GetInstance().SetDefaultOutputDevice(defaultOutputDevice, streamIndex_,
        processConfig_.rendererInfo.streamUsage, status_ == I_STATUS_STARTED, skipForce);
}

int32_t RendererInServer::SetSourceDuration(int64_t duration)
{
    sourceDuration_ = duration;
    return SUCCESS;
}

std::unique_ptr<AudioRingCache>& RendererInServer::GetDupRingBuffer()
{
    return dupRingBuffer_;
}
 
int32_t RendererInServer::CreateDupBufferInner(int32_t innerCapId)
{
    // todo dynamic
    CHECK_AND_RETURN_RET_LOG(innerCapIdToDupStreamCallbackMap_.find(innerCapId) !=
        innerCapIdToDupStreamCallbackMap_.end(), SUCCESS,
        "innerCapIdToDupStreamCallbackMap_ is no find innerCapId: %{public}d", innerCapId);
    CHECK_AND_RETURN_RET_LOG(innerCapIdToDupStreamCallbackMap_[innerCapId] != nullptr,
        SUCCESS, "innerCapIdToDupStreamCallbackMap_ is null, innerCapId: %{public}d", innerCapId);
    CHECK_AND_RETURN_RET_LOG(innerCapIdToDupStreamCallbackMap_[innerCapId]->GetDupRingBuffer() == nullptr,
        SUCCESS, "DupRingBuffer not null, no need CreateDupBuffer, innerCapId: %{public}d", innerCapId);

    auto &capInfo = captureInfos_[innerCapId];
    capInfo.dupStream->GetSpanSizePerFrame(dupSpanSizeInFrame_);
    if (offloadEnable_ == true) {
        dupTotalSizeInFrame_ = dupSpanSizeInFrame_ * (DUP_OFFLOAD_LEN / DUP_DEFAULT_LEN);
    } else {
        dupTotalSizeInFrame_ = dupSpanSizeInFrame_ * (DUP_COMMON_LEN/DUP_DEFAULT_LEN);
    }

    capInfo.dupStream->GetByteSizePerFrame(dupByteSizePerFrame_);
    if (dupSpanSizeInFrame_ == 0 || dupByteSizePerFrame_ == 0) {
        AUDIO_ERR_LOG("ERR_INVALID_PARAM");
        return ERR_INVALID_PARAM;
    }
    dupSpanSizeInByte_ = dupSpanSizeInFrame_ * dupByteSizePerFrame_;
    CHECK_AND_RETURN_RET_LOG(dupSpanSizeInByte_ != 0, ERR_OPERATION_FAILED, "Config dup buffer failed");
    AUDIO_INFO_LOG("dupTotalSizeInFrame_: %{public}zu, dupSpanSizeInFrame_: %{public}zu,"
        "dupByteSizePerFrame_:%{public}zu dupSpanSizeInByte_: %{public}zu,",
        dupTotalSizeInFrame_, dupSpanSizeInFrame_, dupByteSizePerFrame_, dupSpanSizeInByte_);
 
    // create dupBuffer in server
    innerCapIdToDupStreamCallbackMap_[innerCapId]->GetDupRingBuffer() =
        AudioRingCache::Create(dupTotalSizeInFrame_ * dupByteSizePerFrame_);
    CHECK_AND_RETURN_RET_LOG(innerCapIdToDupStreamCallbackMap_[innerCapId]->GetDupRingBuffer() != nullptr,
        ERR_OPERATION_FAILED, "Create dup buffer failed");
    return SUCCESS;
}
 
int32_t RendererInServer::WriteDupBufferInner(const BufferDesc &bufferDesc, int32_t innerCapId)
{
    size_t targetSize = bufferDesc.bufLength;
    CHECK_AND_RETURN_RET_LOG(innerCapIdToDupStreamCallbackMap_.find(innerCapId) !=
        innerCapIdToDupStreamCallbackMap_.end(), ERROR,
        "innerCapIdToDupStreamCallbackMap_ is no find innerCapId: %{public}d", innerCapId);
    CHECK_AND_RETURN_RET_LOG(innerCapIdToDupStreamCallbackMap_[innerCapId] != nullptr,
        ERROR, "innerCapIdToDupStreamCallbackMap_ is null, innerCapId: %{public}d", innerCapId);
    CHECK_AND_RETURN_RET_LOG(innerCapIdToDupStreamCallbackMap_[innerCapId]->GetDupRingBuffer() != nullptr,
        ERROR, "DupRingBuffe is null, innerCapId: %{public}d", innerCapId);
    OptResult result = innerCapIdToDupStreamCallbackMap_[innerCapId]->GetDupRingBuffer()->GetWritableSize();
    // todo get writeable size failed
    CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERROR,
        "DupRingBuffer write invalid size is:%{public}zu", result.size);
    size_t writableSize = result.size;
    AUDIO_DEBUG_LOG("targetSize: %{public}zu, writableSize: %{public}zu", targetSize, writableSize);
    size_t writeSize = std::min(writableSize, targetSize);
    BufferWrap bufferWrap = {bufferDesc.buffer, writeSize};
    if (lastTarget_ == INJECT_TO_VOICE_COMMUNICATION_CAPTURE) {
        WriteSilenceDupBuffer(bufferDesc, bufferWrap, innerCapId);
    } else if (writeSize > 0) {
        result = innerCapIdToDupStreamCallbackMap_[innerCapId]->GetDupRingBuffer()->Enqueue(bufferWrap);
        if (result.ret != OPERATION_SUCCESS) {
            AUDIO_ERR_LOG("RingCache Enqueue failed ret:%{public}d size:%{public}zu", result.ret, result.size);
        }
        DumpFileUtil::WriteDumpFile(dumpDupIn_, static_cast<void *>(bufferDesc.buffer), writeSize);
    }
    return SUCCESS;
}

void RendererInServer::WriteSilenceDupBuffer(const BufferDesc &bufferDesc, BufferWrap &bufferWrap, int32_t innerCapId)
{
    CHECK_AND_RETURN(bufferWrap.dataSize > 0);
    auto buffer = std::make_unique<uint8_t []>(bufferWrap.dataSize);
    bufferWrap.dataPtr = buffer.get();
    memset_s(bufferWrap.dataPtr, bufferWrap.dataSize, 0, bufferWrap.dataSize);
    OptResult result = innerCapIdToDupStreamCallbackMap_[innerCapId]->GetDupRingBuffer()->Enqueue(bufferWrap);
    if (result.ret != OPERATION_SUCCESS) {
        AUDIO_ERR_LOG("RingCache Enqueue failed ret:%{public}d size:%{public}zu", result.ret, result.size);
    }
    DumpFileUtil::WriteDumpFile(dumpDupIn_, static_cast<void *>(bufferDesc.buffer), bufferWrap.dataSize);
}

int32_t RendererInServer::SetSpeed(float speed)
{
    CHECK_AND_RETURN_RET_LOG(stream_ != nullptr, ERR_OPERATION_FAILED, "stream_ is null");
    return stream_->SetSpeed(speed);
}

int32_t RendererInServer::SetOffloadDataCallbackState(int32_t state)
{
    return stream_->SetOffloadDataCallbackState(state);
}

int32_t RendererInServer::StopSession()
{
    CHECK_AND_RETURN_RET_LOG(audioServerBuffer_ != nullptr, ERR_INVALID_PARAM, "audioServerBuffer_ is nullptr");
    audioServerBuffer_->SetStopFlag(true);
    return SUCCESS;
}

int32_t RendererInServer::SetAudioHapticsSyncId(const int32_t &audioHapticsSyncId)
{
    audioHapticsSyncId_.store(audioHapticsSyncId);
    return SUCCESS;
}

void RendererInServer::UpdateLatestForWorkgroup(float systemVolume)
{
    latestForWorkgroup_.status = status_;
    latestForWorkgroup_.isInSilentState = isInSilentState_;
    latestForWorkgroup_.silentModeAndMixWithOthers = silentModeAndMixWithOthers_.load();
    latestForWorkgroup_.lastWriteStandbyEnableStatus = lastWriteStandbyEnableStatus_;
    latestForWorkgroup_.streamVolume = audioServerBuffer_->GetStreamVolume();
    latestForWorkgroup_.systemVolume = systemVolume;
    AUDIO_INFO_LOG("[WorkgroupInServer] pid = %{public}d, status_ = %{public}d, "
        "isInSilentState_ = %{public}d, "
        "silentModeAndMixWithOthers_ = %{public}d, "
        "lastWriteStandbyEnableStatus_ = %{public}d, "
        "streamVolume = %{public}f, "
        "systemVolume = %{public}f",
        processConfig_.appInfo.appPid, latestForWorkgroup_.status, latestForWorkgroup_.isInSilentState,
        latestForWorkgroup_.silentModeAndMixWithOthers, latestForWorkgroup_.lastWriteStandbyEnableStatus,
        latestForWorkgroup_.streamVolume, latestForWorkgroup_.systemVolume);
}

bool RendererInServer::CollectInfosForWorkgroup(float systemVolume)
{
    bool running = (status_ == I_STATUS_STARTED) ? true : false;
    float streamVolume = audioServerBuffer_->GetStreamVolume();
    bool haveStreamSound = (fabs(streamVolume) > AUDIO_VOLOMUE_EPSILON) ? true : false;
    bool haveSystemSound = (fabs(systemVolume) > AUDIO_VOLOMUE_EPSILON) ? true : false;

    if (!latestForWorkgroupInited_) {
        UpdateLatestForWorkgroup(systemVolume);
        latestForWorkgroupInited_ = true;
    }
    UpdateLatestForWorkgroup(systemVolume);

    return running && haveStreamSound && haveSystemSound &&
        !isInSilentState_ && !silentModeAndMixWithOthers_ && !lastWriteStandbyEnableStatus_;
}

void RendererInServer::InitDupBuffer(int32_t innerCapId)
{
    std::lock_guard<std::mutex> lock(dupMutex_);
    InitDupBufferInner(innerCapId);
}

void RendererInServer::InitDupBufferInner(int32_t innerCapId)
{
    CHECK_AND_RETURN_LOG(innerCapIdToDupStreamCallbackMap_.find(innerCapId) != innerCapIdToDupStreamCallbackMap_.end(),
        "innerCapIdToDupStreamCallbackMap_ is no find innerCapId: %{public}d", innerCapId);
    CHECK_AND_RETURN_LOG(innerCapIdToDupStreamCallbackMap_[innerCapId] != nullptr,
        "innerCapIdToDupStreamCallbackMap_ is null, innerCapId: %{public}d", innerCapId);
    CHECK_AND_RETURN_LOG(innerCapIdToDupStreamCallbackMap_[innerCapId]->GetDupRingBuffer() != nullptr,
        "DupRingBuffe is null, innerCapId: %{public}d", innerCapId);
    innerCapIdToDupStreamCallbackMap_[innerCapId]->GetDupRingBuffer()->
        ReConfig(dupTotalSizeInFrame_ * dupByteSizePerFrame_, false);
    AUDIO_INFO_LOG("InitDupBuffer success, innerCapId: %{public}d, stream sessionId: %{public}u",
        innerCapId, streamIndex_);
}


int32_t RendererInServer::InitSoftLink(int32_t innerCapId)
{
    std::lock_guard<std::mutex> lock(softLinkMutex_);
    if (softLinkInfos_.count(innerCapId) && softLinkInfos_[innerCapId].isSoftLinkEnabled) {
        AUDIO_INFO_LOG("softLink is already enabled,id:%{public}d", innerCapId);
        return SUCCESS;
    }
    std::string sinkName = INNER_CAPTURER_SINK + std::to_string(innerCapId);
    std::string sourceName = OFFLOAD_CAPTURER_SOURCE;

    uint32_t renderIdx = CoreServiceHandler::GetInstance().GetPaIndexByPortName(sinkName);
    uint32_t captureIdx = CoreServiceHandler::GetInstance().GetPaIndexByPortName(sourceName);
    CHECK_AND_RETURN_RET_LOG(renderIdx != OPEN_PORT_FAILURE, ERR_OPERATION_FAILED, "OpenRender Failed");
    CHECK_AND_RETURN_RET_LOG(captureIdx != OPEN_PORT_FAILURE, ERR_OPERATION_FAILED, "OpenCapture Failed");
    AUDIO_INFO_LOG("InitSoftLink:renderIdx%{public}u captureIdx%{public}u", renderIdx, captureIdx);

    std::shared_ptr<HPAE::IHpaeSoftLink> ptrSoftLink =
        HPAE::IHpaeSoftLink::CreateSoftLink(renderIdx, captureIdx, HPAE::SoftLinkMode::OFFLOADINNERCAP_AID);
    CHECK_AND_RETURN_RET_LOG(ptrSoftLink != nullptr, ERR_OPERATION_FAILED, "CreateSoftLink Failed");
    auto &softLinkInfo = softLinkInfos_[innerCapId];
    softLinkInfo.softLink = ptrSoftLink;
    softLinkInfo.isSoftLinkEnabled = true;
    InitSoftLinkVolume(ptrSoftLink);

    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        std::string dumpSoftLinkFileName_ = std::to_string(innerCapId) + "_softLink_" + ".pcm";
        DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpSoftLinkFileName_, &dumpSoftLink);
    }

    if (status_ == I_STATUS_STARTED) {
        AUDIO_INFO_LOG("Renderer %{public}u is already running, let's start the softlink", streamIndex_);
        softLinkInfo.softLink->Start();
    }
    return SUCCESS;
}

int32_t RendererInServer::DestroySoftLink(int32_t innerCapId)
{
    if (!softLinkInfos_.count(innerCapId) || !softLinkInfos_[innerCapId].isSoftLinkEnabled) {
        AUDIO_WARNING_LOG("soft Link is already Destroy.capId:%{public}d", innerCapId);
        return ERR_INVALID_OPERATION;
    }

    softLinkInfos_[innerCapId].isSoftLinkEnabled = false;
    AUDIO_INFO_LOG("Disable soft link %{public}u", innerCapId);

    if (softLinkInfos_[innerCapId].softLink != nullptr) {
        int32_t ret = softLinkInfos_[innerCapId].softLink->Release();
        if (ret < 0) {
            AUDIO_WARNING_LOG("Release softLink failed!");
            return ERR_OPERATION_FAILED;
        }
        softLinkInfos_[innerCapId].softLink = nullptr;
    }

    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1) {
        DumpFileUtil::CloseDumpFile(&dumpSoftLink);
    }
    return SUCCESS;
}

int32_t RendererInServer::InitSoftLinkVolume(std::shared_ptr<HPAE::IHpaeSoftLink> softLinkPtr)
{
    if (audioServerBuffer_ != nullptr) {
        float clientVolume = audioServerBuffer_->GetStreamVolume();
        float duckFactor = audioServerBuffer_->GetDuckFactor();
        bool isMuted = (isMuted_ || silentModeAndMixWithOthers_ || muteFlag_);
        // If some factors are not needed, remove them.
        softLinkPtr->SetVolume(clientVolume);
        softLinkPtr->SetVolumeDuckFactor(duckFactor);
        softLinkPtr->SetVolumeMute(isMuted);
        softLinkPtr->SetVolumeLowPowerFactor(lowPowerVolume_);
    }
    return SUCCESS;
}

template <typename T>
void RendererInServer::SetSoftLinkFunc(T&& softLinkFunc)
{
    for (auto& [id, softInfo] : softLinkInfos_) {
        if (softInfo.isSoftLinkEnabled && softInfo.softLink != nullptr) {
            softLinkFunc(softInfo.softLink);
        }
    }
}

bool RendererInServer::IsEnabledAndValidSoftLink(SoftLinkInfo& softLinkInfo)
{
    return softLinkInfo.isSoftLinkEnabled && softLinkInfo.softLink != nullptr;
}

bool RendererInServer::IsEnabledAndValidDupStream(CaptureInfo& captureInfo)
{
    return captureInfo.isInnerCapEnabled && captureInfo.dupStream != nullptr;
}

bool RendererInServer::IsMovieOffloadStream()
{
    return processConfig_.streamType == STREAM_MOVIE &&
    processConfig_.rendererInfo.originalFlag == AUDIO_FLAG_PCM_OFFLOAD && offloadEnable_;
}

bool RendererInServer::IsMovieStream()
{
    return processConfig_.streamType == STREAM_MOVIE &&
    processConfig_.rendererInfo.originalFlag == AUDIO_FLAG_PCM_OFFLOAD;
}

int32_t RendererInServer::SetTarget(RenderTarget target, int32_t &ret)
{
    CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifySystemPermission(),
        ERR_SYSTEM_PERMISSION_DENIED, "verify system permission failed");
    if (target == lastTarget_) {
        ret = SUCCESS;
        return ret;
    }
    if (target == INJECT_TO_VOICE_COMMUNICATION_CAPTURE) {
        auto tokenId = IPCSkeleton::GetCallingTokenID();
        CHECK_AND_RETURN_RET_LOG(PermissionUtil::VerifyPermission(INJECT_PLAYBACK_TO_AUDIO_CAPTURE_PERMISSION, tokenId),
            ERR_PERMISSION_DENIED, "verify permission failed");
    }
    if (status_ == I_STATUS_IDLE || status_ == I_STATUS_PAUSED || status_ == I_STATUS_STOPPED) {
        ret = CoreServiceHandler::GetInstance().SetRendererTarget(target, lastTarget_, streamIndex_);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "Injector::SetRendererTarget failed");
        lastTarget_ = target;
        ClearInnerCapBufferForInject();
        return ret;
    }
    ret = ERR_ILLEGAL_STATE;
    return ret;
}

void RendererInServer::ClearInnerCapBufferForInject()
{
    CHECK_AND_RETURN(lastTarget_ == INJECT_TO_VOICE_COMMUNICATION_CAPTURE);
    for (auto &capInfo : captureInfos_) {
        CHECK_AND_CONTINUE(innerCapIdToDupStreamCallbackMap_.find(capInfo.first) !=
            innerCapIdToDupStreamCallbackMap_.end());
        innerCapIdToDupStreamCallbackMap_[capInfo.first]->GetDupRingBuffer()->ResetBuffer();
    }
}

void RendererInServer::WaitForDataConnection()
{
    Trace trace("WaitForA2dpDataConnection");
    std::unique_lock<std::mutex> dataConnectionWaitLock(dataConnectionMutex_);
    if (!isDataLinkConnected_) {
        AUDIO_INFO_LOG("data-connection blocking starts.");
        bool stopWaiting = dataConnectionCV_.wait_for(
            dataConnectionWaitLock, std::chrono::milliseconds(DATA_CONNECTION_TIMEOUT_IN_MS), [this] {
                return isDataLinkConnected_;
            });
        AUDIO_INFO_LOG("data-connection blocking ends, reason %{public}s.", stopWaiting ? "connected" : "timeout");
    }
}

int32_t RendererInServer::OnWriteData(int8_t *inputData, size_t requestDataLen)
{
    int32_t ret = WriteData(inputData, requestDataLen);
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);

    BufferDesc bufferDesc = {
        .buffer = reinterpret_cast<uint8_t*>(inputData),
        .bufLength = requestDataLen,
        .dataLength = requestDataLen
    };
    if (AudioDump::GetInstance().GetVersionType() == DumpFileUtil::BETA_VERSION) {
        DumpFileUtil::WriteDumpFile(dumpC2S_, static_cast<void *>(bufferDesc.buffer), bufferDesc.bufLength);
        AudioCacheMgr::GetInstance().CacheData(dumpFileName_,
            static_cast<void *>(bufferDesc.buffer), bufferDesc.bufLength);
    }
    OtherStreamEnqueue(bufferDesc);
    audioStreamChecker_->RecordNormalFrame();
    WriteMuteDataSysEvent(bufferDesc);

    OnWriteDataFinish();
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
