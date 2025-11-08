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

#ifndef LOG_TAG
#define LOG_TAG "BluetoothAudioRenderSink"
#endif

#include "sink/bluetooth_audio_render_sink.h"
#include <climits>
#include "parameters.h"
#include "audio_hdi_log.h"
#include "audio_errors.h"
#include "volume_tools.h"
#include "media_monitor_manager.h"
#include "audio_dump_pcm.h"
#include "audio_performance_monitor.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include "manager/hdi_monitor.h"
#include "adapter/i_device_manager.h"

using namespace OHOS::HDI::Audio_Bluetooth;

namespace OHOS {
namespace AudioStandard {
namespace {
constexpr int64_t MONITOR_WRITE_COST = 1 * 1000 * 1000 * 1000; // 1s
}
BluetoothAudioRenderSink::BluetoothAudioRenderSink(bool isBluetoothLowLatency, const std::string &halName)
    : isBluetoothLowLatency_(isBluetoothLowLatency), halName_(halName)
{
    if (halName_ == HDI_ID_INFO_HEARING_AID) {
        sinkType_ = ADAPTER_TYPE_HEARING_AID;
    }

    logTypeTag_ = isBluetoothLowLatency_ ? "fast" : "normal";
}

BluetoothAudioRenderSink::~BluetoothAudioRenderSink()
{
    DeInit();
    DumpFileUtil::CloseDumpFile(&dumpFile_);
    AudioPerformanceMonitor::GetInstance().DeleteOvertimeMonitor(sinkType_);
    AUDIO_INFO_LOG("[%{public}s] volumeDataCount: %{public}" PRId64, logUtilsTag_.c_str(), volumeDataCount_);
}

int32_t BluetoothAudioRenderSink::Init(const IAudioSinkAttr &attr)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    if (sinkInited_ && IsValidState()) {
        AUDIO_WARNING_LOG("sink already inited");
        ++sinkInitCount_;
        return SUCCESS;
    }

    AUDIO_INFO_LOG("%{public}s in", logTypeTag_.c_str());
    logMode_ = system::GetIntParameter("persist.multimedia.audiolog.switch", 0);
    logUtilsTag_ = "A2dpSink";

    attr_ = attr;
    audioSampleFormat_ = attr_.format;
    if (isBluetoothLowLatency_ && attr.format == SAMPLE_S32LE) {
        AUDIO_WARNING_LOG("Format shoule not be s32 for bluetooth");
        return ERR_NOT_SUPPORTED;
    }
    int32_t ret = InitRender();
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
    if (!a2dpParam_.empty()) {
        SetAudioParameterInner(a2dpParam_.value);
        AUDIO_INFO_LOG("Set a2dpParam %{public}s SUCCESS", a2dpParam_.value.c_str());
        a2dpParam_ = {};
    }
    sinkInited_ = true;
    ++sinkInitCount_;
    started_ = false;
    return SUCCESS;
}

void BluetoothAudioRenderSink::DeInit(void)
{
    AUDIO_INFO_LOG("%{public}s in", logTypeTag_.c_str());
    std::lock_guard<std::mutex> lock(sinkMutex_);
    Trace trace("BluetoothAudioRenderSink::DeInit");
    if (sinkInitCount_ > 1) {
        --sinkInitCount_;
        AUDIO_WARNING_LOG("sink is still used, count: %{public}d", sinkInitCount_);
        return;
    }
    // sinkInitCount must be 1 or 0, if 0 sinkInited should be false
    sinkInitCount_ = 0;
    if (!sinkInited_) {
        AUDIO_WARNING_LOG("sink not inited");
        return;
    }

    sinkInited_ = false;
    started_ = false;

    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_BLUETOOTH);
    CHECK_AND_RETURN(deviceManager != nullptr);
    std::string adapterNameCase;
    if (halName_ == HDI_ID_INFO_HEARING_AID) {
        adapterNameCase = "bt_hearing_aid";
    } else {
        adapterNameCase = isBluetoothLowLatency_ ? "bt_a2dp_fast" : "bt_a2dp";
    }
    if (IsValidState()) {
        deviceManager->DestroyRender(adapterNameCase, hdiRenderId_);
    }
    audioRender_ = nullptr;
    AUDIO_INFO_LOG("%{public}s update validState:true", logTypeTag_.c_str());
    validState_ = true;
}

bool BluetoothAudioRenderSink::IsInited(void)
{
    return sinkInited_;
}

bool BluetoothAudioRenderSink::IsSinkInited(void)
{
    if (!sinkInited_) {
        AUDIO_ERR_LOG("sinkInited_ is false!");
        HdiMonitor::ReportHdiException(HdiType::A2DP, ErrorCase::CALL_HDI_FAILED, ERR_NOT_STARTED, "Hdi not inited"
            ":" + std::string(isBluetoothLowLatency_ ? "fast" : "normal"));
        return false;
    }
    return true;
}

int32_t BluetoothAudioRenderSink::Start(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    Trace trace("BluetoothAudioRenderSink::Start");
    AUDIO_INFO_LOG("%{public}s in", logTypeTag_.c_str());
    CHECK_AND_RETURN_RET(IsSinkInited(), ERR_NOT_STARTED);
#ifdef FEATURE_POWER_MANAGER
    if (runningLock_ == nullptr) {
        WatchTimeout guard("create AudioRunningLock start");
        runningLock_ = std::make_shared<AudioRunningLock>(std::string(RUNNING_LOCK_NAME));
        guard.CheckCurrTimeout();
    }
    if (runningLock_ != nullptr) {
        runningLock_->Lock(RUNNING_LOCK_TIMEOUTMS_LASTING);
    } else {
        AUDIO_ERR_LOG("running lock is null, playback can not work well");
    }
#endif
    dumpFileName_ = "bluetooth_sink_" + GetTime() + "_" + std::to_string(attr_.sampleRate) + "_" +
        std::to_string(attr_.channel) + "_" + std::to_string(attr_.format) + ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileName_, &dumpFile_);

    InitLatencyMeasurement();
    CHECK_AND_RETURN_RET(!started_, SUCCESS);
    int32_t tryCount = 3;
    while (tryCount-- > 0) {
        AUDIO_INFO_LOG("try to start bluetooth render");
        CHECK_AND_BREAK_LOG(audioRender_ != nullptr && IsValidState(), "Bluetooth renderer is nullptr");
        int32_t ret = audioRender_->control.Start(reinterpret_cast<AudioHandle>(audioRender_));
        if (ret) {
            AUDIO_ERR_LOG("start fail, remain %{public}d attempt(s)", tryCount);
            HdiMonitor::ReportHdiException(HdiType::A2DP, ErrorCase::CALL_HDI_FAILED, ret, "a2dp start "
                "failed:" + std::string(isBluetoothLowLatency_ ? "fast" : "normal"));
            usleep(WAIT_TIME_FOR_RETRY_IN_MICROSECOND);
            continue;
        }
        AudioPerformanceMonitor::GetInstance().RecordTimeStamp(sinkType_, INIT_LASTWRITTEN_TIME);
        started_ = true;
        return CheckBluetoothScenario();
    }
    AUDIO_ERR_LOG("start fail for three times, return");
#ifdef FEATURE_POWER_MANAGER
    if (runningLock_ != nullptr) {
        AUDIO_INFO_LOG("running lock unlock");
        runningLock_->UnLock();
    } else {
        AUDIO_WARNING_LOG("running lock is null, playback can not work well");
    }
#endif
    return ERR_NOT_STARTED;
}

int32_t BluetoothAudioRenderSink::Stop(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    Trace trace("BluetoothAudioRenderSink::Stop");
    AUDIO_INFO_LOG("%{public}s in", logTypeTag_.c_str());
    DeInitLatencyMeasurement();
#ifdef FEATURE_POWER_MANAGER
    if (runningLock_ != nullptr) {
        AUDIO_INFO_LOG("running lock unlock");
        runningLock_->UnLock();
    } else {
        AUDIO_WARNING_LOG("running lock is null, playback can not work well");
    }
#endif
    if (!started_) {
        return SUCCESS;
    }
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    CHECK_AND_RETURN_RET(IsValidState(), ERR_INVALID_HANDLE);
    Trace renderTrace("BluetoothAudioRenderSink::Stop inner stop");
    AUDIO_DEBUG_LOG("before render stop");
    int32_t ret = audioRender_->control.Stop(reinterpret_cast<AudioHandle>(audioRender_));
    UpdateSinkState(false);
    AUDIO_DEBUG_LOG("after render stop");
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "stop fail");
    started_ = false;
    paused_ = false;
    return SUCCESS;
}

int32_t BluetoothAudioRenderSink::Resume(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    AUDIO_INFO_LOG("%{public}s in", logTypeTag_.c_str());
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    CHECK_AND_RETURN_RET(IsValidState(), ERR_INVALID_HANDLE);
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED, "not start, invalid state");

    if (!paused_) {
        return SUCCESS;
    }
    int32_t ret = audioRender_->control.Resume(reinterpret_cast<AudioHandle>(audioRender_));
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "resume fail");
    AudioPerformanceMonitor::GetInstance().RecordTimeStamp(sinkType_, INIT_LASTWRITTEN_TIME);
    paused_ = false;
    return SUCCESS;
}

int32_t BluetoothAudioRenderSink::Pause(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    AUDIO_INFO_LOG("%{public}s in", logTypeTag_.c_str());
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    CHECK_AND_RETURN_RET(IsValidState(), ERR_INVALID_HANDLE);
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED, "not start, invalid state");

    if (paused_) {
        return SUCCESS;
    }
    int32_t ret = audioRender_->control.Pause(reinterpret_cast<AudioHandle>(audioRender_));
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "pause fail");
    paused_ = true;
    return SUCCESS;
}

int32_t BluetoothAudioRenderSink::Flush(void)
{
    AUDIO_INFO_LOG("%{public}s in", logTypeTag_.c_str());
    std::lock_guard<std::mutex> lock(sinkMutex_);
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    CHECK_AND_RETURN_RET(IsValidState(), ERR_INVALID_HANDLE);
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED, "not start, invalid state");

    int32_t ret = audioRender_->control.Flush(reinterpret_cast<AudioHandle>(audioRender_));
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "flush fail");
    return SUCCESS;
}

int32_t BluetoothAudioRenderSink::Reset(void)
{
    AUDIO_INFO_LOG("%{public}s in", logTypeTag_.c_str());
    std::lock_guard<std::mutex> lock(sinkMutex_);
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    CHECK_AND_RETURN_RET(IsValidState(), ERR_INVALID_HANDLE);
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED, "not start, invalid state");

    int32_t ret = audioRender_->control.Flush(reinterpret_cast<AudioHandle>(audioRender_));
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "reset fail");
    return SUCCESS;
}

int32_t BluetoothAudioRenderSink::RenderFrame(char &data, uint64_t len, uint64_t &writeLen)
{
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    CHECK_AND_RETURN_RET(validState_, ERR_INVALID_HANDLE);
    if (audioMonoState_) {
        AdjustStereoToMono(&data, len);
    }
    if (audioBalanceState_) {
        AdjustAudioBalance(&data, len);
    }
    CheckLatencySignal(reinterpret_cast<uint8_t *>(&data), len);
    CheckUpdateState(&data, len);
    if (suspend_) {
        return SUCCESS;
    }
    Trace trace("BluetoothAudioRenderSink::RenderFrame");
    if (switchDeviceMute_) {
        Trace renderTrace("BluetoothAudioRenderSink::RenderFrame::renderEmpty");
        if (memset_s(reinterpret_cast<void *>(&data), static_cast<size_t>(len), 0, static_cast<size_t>(len)) != EOK) {
            AUDIO_WARNING_LOG("call memset_s fail");
        }
    }

    BufferDesc buffer = { reinterpret_cast<uint8_t *>(&data), len, len };
    AudioStreamInfo streamInfo(static_cast<AudioSamplingRate>(attr_.sampleRate), AudioEncodingType::ENCODING_PCM,
        audioSampleFormat_, static_cast<AudioChannel>(attr_.channel));
    VolumeTools::DfxOperation(buffer, streamInfo, logUtilsTag_, volumeDataCount_);
    if (AudioDump::GetInstance().GetVersionType() == DumpFileUtil::BETA_VERSION) {
        DumpFileUtil::WriteDumpFile(dumpFile_, static_cast<void *>(&data), len);
        AudioCacheMgr::GetInstance().CacheData(dumpFileName_, static_cast<void *>(&data), len);
    }
    int32_t ret = DoRenderFrame(data, len, writeLen);
#ifdef FEATURE_POWER_MANAGER
    CHECK_AND_RETURN_RET_LOG(runningLock_, ERR_INVALID_HANDLE, "running lock is nullptr");
    runningLock_->UpdateAppsUidToPowerMgr();
#endif
    return ret;
}

int64_t BluetoothAudioRenderSink::GetVolumeDataCount()
{
    return volumeDataCount_;
}

int32_t BluetoothAudioRenderSink::SuspendRenderSink(void)
{
    AUDIO_INFO_LOG("%{public}s in", logTypeTag_.c_str());
    Trace trace("BluetoothAudioRenderSink::SuspendRenderSink");
    suspend_ = true;
    return SUCCESS;
}

int32_t BluetoothAudioRenderSink::RestoreRenderSink(void)
{
    AUDIO_INFO_LOG("%{public}s in", logTypeTag_.c_str());
    Trace trace("BluetoothAudioRenderSink::RestoreRenderSink");
    suspend_ = false;
    return SUCCESS;
}

void BluetoothAudioRenderSink::SetAudioParameter(const AudioParamKey key, const std::string &condition,
    const std::string &value)
{
    AUDIO_INFO_LOG("key: %{public}d, condition: %{public}s, value: %{public}s", key, condition.c_str(), value.c_str());
    std::lock_guard<std::mutex> lock(sinkMutex_);
    SetAudioParameterInner(value);

    int32_t ret = 0;
    if (started_ && isBluetoothLowLatency_ && !strcmp(value.c_str(), "A2dpSuspended=0;")) {
        int32_t tryCount = 3;
        while (tryCount-- > 0) {
            AUDIO_INFO_LOG("try to start bluetooth render");
            CHECK_AND_RETURN_LOG(audioRender_ != nullptr, "render is nullptr");
            ret = audioRender_->control.Start(reinterpret_cast<AudioHandle>(audioRender_));
            if (ret == SUCCESS) {
                AUDIO_INFO_LOG("start succ");
                started_ = true;
                CheckBluetoothScenario();
                return;
            } else {
                AUDIO_ERR_LOG("start fail, remain %{public}d attempt(s)", tryCount);
                usleep(WAIT_TIME_FOR_RETRY_IN_MICROSECOND);
            }
        }
    }
}

// need to hold sinkMutex when call this func.
void BluetoothAudioRenderSink::SetAudioParameterInner(const std::string &value)
{
    CHECK_AND_RETURN_LOG(audioRender_ != nullptr && IsValidState(), "render is nullptr");
    int32_t ret = audioRender_->attr.SetExtraParams(reinterpret_cast<AudioHandle>(audioRender_), value.c_str());
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("set parameter fail, error code: %{public}d", ret);
    }
}

std::string BluetoothAudioRenderSink::GetAudioParameter(const AudioParamKey key, const std::string &condition)
{
    AUDIO_INFO_LOG("not support");
    return "";
}

int32_t BluetoothAudioRenderSink::SetVolume(float left, float right)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    CHECK_AND_RETURN_RET(IsValidState(), ERR_INVALID_HANDLE);

    leftVolume_ = left;
    rightVolume_ = right;
    float volume;
    if ((leftVolume_ == 0) && (rightVolume_ != 0)) {
        volume = rightVolume_;
    } else if ((leftVolume_ != 0) && (rightVolume_ == 0)) {
        volume = leftVolume_;
    } else {
        volume = (leftVolume_ + rightVolume_) / HALF_FACTOR;
    }

    int32_t ret = audioRender_->volume.SetVolume(reinterpret_cast<AudioHandle>(audioRender_), volume);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("set volume fail");
    }

    return ret;
}

int32_t BluetoothAudioRenderSink::GetVolume(float &left, float &right)
{
    left = leftVolume_;
    right = rightVolume_;
    return SUCCESS;
}

int32_t BluetoothAudioRenderSink::GetLatency(uint32_t &latency)
{
    Trace trace("BluetoothAudioRenderSink::GetLatency");
    std::lock_guard<std::mutex> lock(sinkMutex_);
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    CHECK_AND_RETURN_RET(IsValidState(), ERR_INVALID_HANDLE);

    uint32_t hdiLatency;
    int32_t ret = audioRender_->GetLatency(audioRender_, &hdiLatency);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "get latency fail");
    latency = hdiLatency;
    return SUCCESS;
}

int32_t BluetoothAudioRenderSink::GetTransactionId(uint64_t &transactionId)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    CHECK_AND_RETURN_RET(IsValidState(), ERR_INVALID_HANDLE);
    transactionId = reinterpret_cast<uint64_t>(audioRender_);
    return SUCCESS;
}

int32_t BluetoothAudioRenderSink::GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

float BluetoothAudioRenderSink::GetMaxAmplitude(void)
{
    lastGetMaxAmplitudeTime_ = ClockTime::GetCurNano();
    startUpdate_ = true;
    return maxAmplitude_;
}

void BluetoothAudioRenderSink::SetAudioMonoState(bool audioMono)
{
    audioMonoState_ = audioMono;
}

void BluetoothAudioRenderSink::SetAudioBalanceValue(float audioBalance)
{
    // reset the balance coefficient value firstly
    leftBalanceCoef_ = 1.0f;
    rightBalanceCoef_ = 1.0f;

    if (std::abs(audioBalance - 0.0f) <= std::numeric_limits<float>::epsilon()) {
        // audioBalance is equal to 0.0f
        audioBalanceState_ = false;
    } else {
        // audioBalance is not equal to 0.0f
        audioBalanceState_ = true;
        // calculate the balance coefficient
        if (audioBalance > 0.0f) {
            leftBalanceCoef_ -= audioBalance;
        } else if (audioBalance < 0.0f) {
            rightBalanceCoef_ += audioBalance;
        }
    }
}

int32_t BluetoothAudioRenderSink::SetSinkMuteForSwitchDevice(bool mute)
{
    std::lock_guard<std::mutex> lock(switchDeviceMutex_);
    AUDIO_INFO_LOG("set a2dp mute %{public}d", mute);

    if (mute) {
        muteCount_++;
        if (switchDeviceMute_) {
            AUDIO_INFO_LOG("a2dp already muted");
            return SUCCESS;
        }
        switchDeviceMute_ = true;
    } else {
        muteCount_--;
        if (muteCount_ > 0) {
            AUDIO_WARNING_LOG("a2dp not all unmuted");
            return SUCCESS;
        }
        switchDeviceMute_ = false;
        muteCount_ = 0;
    }

    return SUCCESS;
}

int32_t BluetoothAudioRenderSink::SetAudioScene(AudioScene audioScene, bool scoExcludeFlag)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t BluetoothAudioRenderSink::GetAudioScene(void)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t BluetoothAudioRenderSink::UpdateActiveDevice(std::vector<DeviceType> &outputDevices)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

void BluetoothAudioRenderSink::RegistCallback(uint32_t type, IAudioSinkCallback *callback)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    callback_.RegistCallback(type, callback);
    AUDIO_INFO_LOG("regist succ");
}

void BluetoothAudioRenderSink::ResetActiveDeviceForDisconnect(DeviceType device)
{
    AUDIO_INFO_LOG("not support");
}

int32_t BluetoothAudioRenderSink::SetPaPower(int32_t flag)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t BluetoothAudioRenderSink::SetPriPaPower(void)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t BluetoothAudioRenderSink::UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size)
{
#ifdef FEATURE_POWER_MANAGER
    CHECK_AND_RETURN_RET_LOG(runningLock_, ERR_INVALID_HANDLE, "running lock is nullptr");
    runningLock_->UpdateAppsUid(appsUid, appsUid + size);
#endif
    return SUCCESS;
}

int32_t BluetoothAudioRenderSink::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
#ifdef FEATURE_POWER_MANAGER
    CHECK_AND_RETURN_RET_LOG(runningLock_, ERR_INVALID_HANDLE, "running lock is nullptr");
    runningLock_->UpdateAppsUid(appsUid.cbegin(), appsUid.cend());
    runningLock_->UpdateAppsUidToPowerMgr();
#endif
    return SUCCESS;
}

void BluetoothAudioRenderSink::SetInvalidState(void)
{
    AUDIO_INFO_LOG("%{public}s update validState:false", logTypeTag_.c_str());
    std::lock_guard<std::mutex> lock(sinkMutex_);
    validState_ = false;
    sinkInited_ = false;
    started_ = false;
}

void BluetoothAudioRenderSink::DumpInfo(std::string &dumpString)
{
    dumpString += "type: BtSink\tstarted: " + std::string(started_ ? "true" : "false") + "\tisLowLatency: " +
        std::string(isBluetoothLowLatency_ ? "true" : "false") + "\n";
}

void BluetoothAudioRenderSink::SetDmDeviceType(uint16_t dmDeviceType, DeviceType deviceType)
{
    AUDIO_INFO_LOG("not support");
}

int32_t BluetoothAudioRenderSink::GetMmapBufferInfo(int &fd, uint32_t &totalSizeInframe, uint32_t &spanSizeInframe,
    uint32_t &byteSizePerFrame, uint32_t &syncInfoSize)
{
    CHECK_AND_RETURN_RET_LOG(bufferFd_ != INVALID_FD, ERR_INVALID_HANDLE, "buffer fd has been released");
    fd = bufferFd_;
    totalSizeInframe = bufferTotalFrameSize_;
    spanSizeInframe = eachReadFrameSize_;
    byteSizePerFrame = PcmFormatToBit(attr_.format) * attr_.channel / PCM_8_BIT;
    syncInfoSize = syncInfoSize_;
    return SUCCESS;
}

int32_t BluetoothAudioRenderSink::GetMmapHandlePosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec)
{
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    CHECK_AND_RETURN_RET(IsValidState(), ERR_INVALID_HANDLE);

    struct AudioTimeStamp stamp = {};
    int32_t ret = audioRender_->attr.GetMmapPosition(audioRender_, &frames, &stamp);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "get mmap position fail, ret: %{public}d", ret);
    int64_t maxSec = 9223372036; // (9223372036 + 1) * 10^9 > INT64_MAX, seconds should not bigger than it
    CHECK_AND_RETURN_RET_LOG(stamp.tvSec >= 0 && stamp.tvSec <= maxSec && stamp.tvNSec >= 0 &&
        stamp.tvNSec <= SECOND_TO_NANOSECOND, ERR_OPERATION_FAILED,
        "get invalid time, second: %{public}" PRId64 ", nanosecond: %{public}" PRId64, stamp.tvSec, stamp.tvNSec);
    timeSec = stamp.tvSec;
    timeNanoSec = stamp.tvNSec;
    return ret;
}

uint32_t BluetoothAudioRenderSink::PcmFormatToBit(AudioSampleFormat format)
{
    AudioFormat hdiFormat = ConvertToHdiFormat(format);
    switch (hdiFormat) {
        case AUDIO_FORMAT_TYPE_PCM_8_BIT:
            return PCM_8_BIT;
        case AUDIO_FORMAT_TYPE_PCM_16_BIT:
            return PCM_16_BIT;
        case AUDIO_FORMAT_TYPE_PCM_24_BIT:
            return PCM_24_BIT;
        case AUDIO_FORMAT_TYPE_PCM_32_BIT:
            return PCM_32_BIT;
        default:
            AUDIO_DEBUG_LOG("unknown format type, set it to default");
            return PCM_24_BIT;
    }
}

AudioFormat BluetoothAudioRenderSink::ConvertToHdiFormat(AudioSampleFormat format)
{
    AudioFormat hdiFormat;
    switch (format) {
        case SAMPLE_U8:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_8_BIT;
            break;
        case SAMPLE_S16LE:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_16_BIT;
            break;
        case SAMPLE_S24LE:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_24_BIT;
            break;
        case SAMPLE_S32LE:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_32_BIT;
            break;
        default:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_16_BIT;
            break;
    }

    return hdiFormat;
}

void BluetoothAudioRenderSink::InitAudioSampleAttr(struct AudioSampleAttributes &param)
{
    // audio parameters for playback
    param.format = AUDIO_FORMAT_TYPE_PCM_16_BIT;
    param.channelCount = AUDIO_CHANNELCOUNT;
    param.frameSize = PCM_16_BIT * param.channelCount / PCM_8_BIT;
    param.sampleRate = AUDIO_SAMPLE_RATE_48K;
    param.interleaved = 0;
    // HDI use adapterNameCase to choose low latency / normal
    param.type = AUDIO_IN_MEDIA;
    param.period = DEEP_BUFFER_RENDER_PERIOD_SIZE;
    param.isBigEndian = false;
    param.isSignedData = true;
    if (param.frameSize != 0) {
        param.startThreshold = DEEP_BUFFER_RENDER_PERIOD_SIZE / (param.frameSize);
    }
    param.stopThreshold = INT_MAX;
    param.silenceThreshold = 0;

    param.sampleRate = attr_.sampleRate;
    param.channelCount = attr_.channel;
    param.format = ConvertToHdiFormat(attr_.format);
    param.frameSize = PcmFormatToBit(attr_.format) * param.channelCount / PCM_8_BIT;
    if (param.frameSize != 0) {
        param.startThreshold = DEEP_BUFFER_RENDER_PERIOD_SIZE / (param.frameSize);
    }
}

void BluetoothAudioRenderSink::InitDeviceDesc(struct AudioDeviceDescriptor &deviceDesc)
{
    deviceDesc.pins = PIN_OUT_SPEAKER;
    deviceDesc.desc = nullptr;
}

int32_t BluetoothAudioRenderSink::CreateRender(void)
{
    struct AudioSampleAttributes param;
    struct AudioDeviceDescriptor deviceDesc;
    InitAudioSampleAttr(param);
    InitDeviceDesc(deviceDesc);

    AUDIO_INFO_LOG("create render, rate: %{public}u, channel: %{public}u, format: %{public}u", param.sampleRate,
        param.channelCount, param.format);

    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_BLUETOOTH);
    CHECK_AND_RETURN_RET(deviceManager != nullptr, ERR_INVALID_HANDLE);
    std::string adapterNameCase = "";
    if (halName_ == HDI_ID_INFO_HEARING_AID) {
        adapterNameCase = "bt_hearing_aid";
    } else if (attr_.adapterName == "dp") {
        adapterNameCase = attr_.adapterName;
    } else {
        adapterNameCase = isBluetoothLowLatency_ ? "bt_a2dp_fast" : "bt_a2dp"; // set sound card infomation
    }
    void *render = deviceManager->CreateRender(adapterNameCase, &param, &deviceDesc, hdiRenderId_);
    audioRender_ = static_cast<struct AudioRender *>(render);
    CHECK_AND_RETURN_RET(audioRender_ != nullptr, ERR_NOT_STARTED);
    validState_ = true;

    return SUCCESS;
}

int32_t BluetoothAudioRenderSink::InitRender(void)
{
    int32_t ret = CreateRender();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "create render fail");
    if (isBluetoothLowLatency_) {
        ret = PrepareMmapBuffer();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "prepare mmap buffer fail");
    }
    return SUCCESS;
}

void BluetoothAudioRenderSink::InitLatencyMeasurement(void)
{
    if (!AudioLatencyMeasurement::CheckIfEnabled()) {
        return;
    }

    AUDIO_INFO_LOG("%{public}s in", logTypeTag_.c_str());
    signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    CHECK_AND_RETURN_LOG(signalDetectAgent_ != nullptr, "signalDetectAgent is nullptr");
    signalDetectAgent_->sampleFormat_ = attr_.format;
    signalDetectAgent_->formatByteSize_ = GetFormatByteSize(attr_.format);
    signalDetected_ = false;
}

void BluetoothAudioRenderSink::DeInitLatencyMeasurement(void)
{
    signalDetectAgent_ = nullptr;
}

void BluetoothAudioRenderSink::CheckLatencySignal(uint8_t *data, size_t len)
{
    CHECK_AND_RETURN(signalDetectAgent_ != nullptr);
    signalDetected_ = signalDetectAgent_->CheckAudioData(data, len);
    if (signalDetected_) {
        AUDIO_INFO_LOG("signal detected");
        LatencyMonitor::GetInstance().UpdateSinkOrSourceTime(true, signalDetectAgent_->lastPeakBufferTime_);
        LatencyMonitor::GetInstance().ShowBluetoothTimestamp();
    }
}

void BluetoothAudioRenderSink::AdjustStereoToMono(char *data, uint64_t len)
{
    // only stereo is supported now (stereo channel count is 2)
    CHECK_AND_RETURN_LOG(attr_.channel == STEREO_CHANNEL_COUNT, "unsupport, channel: %{public}d", attr_.channel);

    switch (ConvertToHdiFormat(attr_.format)) {
        case AUDIO_FORMAT_TYPE_PCM_8_BIT:
            // this function needs further tested for usability
            AdjustStereoToMonoForPCM8Bit(reinterpret_cast<int8_t *>(data), len);
            break;
        case AUDIO_FORMAT_TYPE_PCM_16_BIT:
            AdjustStereoToMonoForPCM16Bit(reinterpret_cast<int16_t *>(data), len);
            break;
        case AUDIO_FORMAT_TYPE_PCM_24_BIT:
            // this function needs further tested for usability
            AdjustStereoToMonoForPCM24Bit(reinterpret_cast<uint8_t *>(data), len);
            break;
        case AUDIO_FORMAT_TYPE_PCM_32_BIT:
            AdjustStereoToMonoForPCM32Bit(reinterpret_cast<int32_t *>(data), len);
            break;
        default:
            // if the audio format is unsupported, the audio data will not be changed
            AUDIO_ERR_LOG("unsupport, format: %{public}d", attr_.format);
            break;
    }
}

void BluetoothAudioRenderSink::AdjustAudioBalance(char *data, uint64_t len)
{
    // only stereo is supported now (stereo channel count is 2)
    CHECK_AND_RETURN_LOG(attr_.channel == STEREO_CHANNEL_COUNT, "unsupport, channel: %{public}d", attr_.channel);

    switch (ConvertToHdiFormat(attr_.format)) {
        case AUDIO_FORMAT_TYPE_PCM_8_BIT:
            // this function needs further tested for usability
            AdjustAudioBalanceForPCM8Bit(reinterpret_cast<int8_t *>(data), len, leftBalanceCoef_, rightBalanceCoef_);
            break;
        case AUDIO_FORMAT_TYPE_PCM_16_BIT:
            AdjustAudioBalanceForPCM16Bit(reinterpret_cast<int16_t *>(data), len, leftBalanceCoef_, rightBalanceCoef_);
            break;
        case AUDIO_FORMAT_TYPE_PCM_24_BIT:
            // this function needs further tested for usability
            AdjustAudioBalanceForPCM24Bit(reinterpret_cast<uint8_t *>(data), len, leftBalanceCoef_, rightBalanceCoef_);
            break;
        case AUDIO_FORMAT_TYPE_PCM_32_BIT:
            AdjustAudioBalanceForPCM32Bit(reinterpret_cast<int32_t *>(data), len, leftBalanceCoef_, rightBalanceCoef_);
            break;
        default:
            // if the audio format is unsupported, the audio data will not be changed
            AUDIO_ERR_LOG("unsupport, format: %{public}d", attr_.format);
            break;
    }
}

void BluetoothAudioRenderSink::CheckUpdateState(char *data, uint64_t len)
{
    if (startUpdate_) {
        if (renderFrameNum_ == 0) {
            last10FrameStartTime_ = ClockTime::GetCurNano();
        }
        renderFrameNum_++;
        maxAmplitude_ = UpdateMaxAmplitude(static_cast<ConvertHdiFormat>(attr_.format), data, len);
        if (renderFrameNum_ == GET_MAX_AMPLITUDE_FRAMES_THRESHOLD) {
            renderFrameNum_ = 0;
            if (last10FrameStartTime_ > lastGetMaxAmplitudeTime_) {
                startUpdate_ = false;
            }
        }
    }
}

int32_t BluetoothAudioRenderSink::DoRenderFrame(char &data, uint64_t len, uint64_t &writeLen)
{
    int32_t ret = SUCCESS;
    int64_t stamp = 0;
    while (true) {
        Trace trace("BluetoothAudioRenderSink::DoRenderFrame");
        stamp = ClockTime::GetCurNano();
        ret = audioRender_->RenderFrame(audioRender_, (void *)&data, len, &writeLen);
        AudioPerformanceMonitor::GetInstance().RecordTimeStamp(sinkType_, ClockTime::GetCurNano());
        stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
        if (logMode_ || stamp >= STAMP_THRESHOLD_MS) {
            AUDIO_PRERELEASE_LOGW("A2dp RenderFrame, len: [%{public}" PRIu64 "], cost: [%{public}" PRId64 "]ms, "
                "writeLen: [%{public}" PRIu64 "], ret: %{public}x", len, stamp, writeLen, ret);
        }
        if (ret == RENDER_FRAME_NUM) {
            AUDIO_ERR_LOG("retry render frame");
            usleep(RENDER_FRAME_INTERVAL_IN_MICROSECONDS);
            continue;
        }
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("A2dp RenderFrame fail, ret: %{public}x", ret);
            ret = ERR_WRITE_FAILED;
        }
        break;
    }

    if (stamp > MONITOR_WRITE_COST) {
        HdiMonitor::ReportHdiException(HdiType::A2DP, ErrorCase::CALL_HDI_TIMEOUT,
            static_cast<int32_t>(stamp), ("call RenderFrame too long!"));
    }

    return ret;
}

// must be called with sinkMutex_ held
void BluetoothAudioRenderSink::UpdateSinkState(bool started)
{
    if (halName_ == HDI_ID_INFO_HEARING_AID) {
        callback_.OnRenderSinkStateChange(GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_HEARING_AID),
            started);
    } else {
        callback_.OnRenderSinkStateChange(GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_BLUETOOTH),
            started);
    }
}

bool BluetoothAudioRenderSink::IsValidState(void)
{
    if (!validState_) {
        AUDIO_WARNING_LOG("disconnected, render invalid");
    }
    return validState_;
}

int32_t BluetoothAudioRenderSink::PrepareMmapBuffer(void)
{
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    uint32_t totalBufferInMs = 40; // 40: 5 * (6 + 2 * (1)) = 40ms, the buffer size, not latency
    uint32_t reqBufferFrameSize = totalBufferInMs * (attr_.sampleRate / SECOND_TO_MILLISECOND);
    struct AudioMmapBufferDescriptor desc = {0};

    // reqBufferFrameSize means frames in total, for example, 40ms * 48K = 1920
    // transferFrameSize means frames in one block, for example 5ms per block, 5ms * 48K = 240
    int32_t ret = audioRender_->attr.ReqMmapBuffer(audioRender_, reqBufferFrameSize, &desc);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "request mmap buffer fail, ret: %{public}d", ret);
    AUDIO_INFO_LOG("memoryAddress: [%{private}p], memoryFd: [%{public}d], totalBufferFrames: [%{public}d], "
        "transferFrameSize: [%{public}d], isShareable: [%{public}d], offset: [%{public}d]", desc.memoryAddress,
        desc.memoryFd, desc.totalBufferFrames, desc.transferFrameSize, desc.isShareable, desc.offset);

    bufferFd_ = desc.memoryFd; // fcntl(fd, 1030, 3) after dup?
    int32_t periodFrameMaxSize = 1920000; // 192khz * 10s
    CHECK_AND_RETURN_RET_LOG(desc.totalBufferFrames >= 0 && desc.transferFrameSize >= 0 &&
        desc.transferFrameSize <= periodFrameMaxSize, ERR_OPERATION_FAILED,
        "invalid value, totalBufferFrames: [%{public}d], transferFrameSize: [%{public}d]", desc.totalBufferFrames,
        desc.transferFrameSize);

    uint32_t frameSizeInByte = PcmFormatToBit(attr_.format) * attr_.channel / PCM_8_BIT;
    bufferTotalFrameSize_ = static_cast<uint32_t>(desc.totalBufferFrames); // 1440 ~ 3840
    eachReadFrameSize_ = static_cast<uint32_t>(desc.transferFrameSize); // 240
    CHECK_AND_RETURN_RET_LOG(frameSizeInByte <= ULLONG_MAX / bufferTotalFrameSize_, ERR_OPERATION_FAILED,
        "buffer size will overflow");
    return SUCCESS;
}

int32_t BluetoothAudioRenderSink::CheckPositionTime(void)
{
    int32_t tryCount = MAX_GET_POSITION_TRY_COUNT;
    uint64_t frames = 0;
    int64_t timeSec = 0;
    int64_t timeNanoSec = 0;
    while (tryCount-- > 0) {
        ClockTime::RelativeSleep(MAX_GET_POSITION_WAIT_TIME);
        int32_t ret = GetMmapHandlePosition(frames, timeSec, timeNanoSec);
        int64_t curTime = ClockTime::GetCurNano();
        int64_t curSec = curTime / AUDIO_NS_PER_SECOND;
        int64_t curNanoSec = curTime - curSec * AUDIO_NS_PER_SECOND;
        if (ret != SUCCESS || curSec != timeSec || curNanoSec - timeNanoSec > MAX_GET_POSITION_HANDLE_TIME) {
            AUDIO_WARNING_LOG("tryCount: %{public}d, ret: %{public}d", tryCount, ret);
            continue;
        } else {
            AUDIO_INFO_LOG("check succ");
            return SUCCESS;
        }
    }
    return ERR_OPERATION_FAILED;
}

int32_t BluetoothAudioRenderSink::CheckBluetoothScenario(void)
{
    UpdateSinkState(true);
    if (isBluetoothLowLatency_ && CheckPositionTime() != SUCCESS) {
        AUDIO_ERR_LOG("check position time fail");
#ifdef FEATURE_POWER_MANAGER
        if (runningLock_ != nullptr) {
            AUDIO_INFO_LOG("running lock unlock");
            runningLock_->UnLock();
        } else {
            AUDIO_WARNING_LOG("running lock is null, playback can not work well");
        }
#endif
        return ERR_NOT_STARTED;
    }
    return SUCCESS;
}

void BluetoothAudioRenderSink::SetBluetoothSinkParam(AudioParamKey key, std::string condition, std::string value)
{
    a2dpParam_.key = key;
    a2dpParam_.condition = condition;
    a2dpParam_.value = value;
    AUDIO_INFO_LOG("SetBluetoothSinkParam key %{public}u, condition %{public}s, value %{public}s",
        a2dpParam_.key, a2dpParam_.condition.c_str(), a2dpParam_.value.c_str());
}

} // namespace AudioStandard
} // namespace OHOS
