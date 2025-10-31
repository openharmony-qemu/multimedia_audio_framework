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
#define LOG_TAG "BluetoothAudioCaptureSource"
#endif

#include "source/bluetooth_audio_capture_source.h"
#include <climits>
#include <future>
#include "parameters.h"
#include "audio_hdi_log.h"
#include "audio_errors.h"
#include "audio_dump_pcm.h"
#include "volume_tools.h"
#include "media_monitor_manager.h"
#include "audio_enhance_chain_manager.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include "adapter/i_device_manager.h"

using namespace OHOS::HDI::Audio_Bluetooth;

namespace OHOS {
namespace AudioStandard {
BluetoothAudioCaptureSource::BluetoothAudioCaptureSource(const uint32_t captureId)
    : captureId_(captureId)
{
    halName_ = "bt_hdap";
    audioSrcClock_ = std::make_shared<AudioSourceClock>();
    CapturerClockManager::GetInstance().RegisterAudioSourceClock(captureId, audioSrcClock_);
}

BluetoothAudioCaptureSource::~BluetoothAudioCaptureSource()
{
    if (sourceInited_) {
        DeInit();
    }

    DumpFileUtil::CloseDumpFile(&dumpFile_);
    AUDIO_INFO_LOG("[%{public}s] volumeDataCount: %{public}" PRId64, logUtilsTag_.c_str(), volumeDataCount_);
    CapturerClockManager::GetInstance().DeleteAudioSourceClock(captureId_);
}

int32_t BluetoothAudioCaptureSource::Init(const IAudioSourceAttr &attr)
{
    if (sourceInited_ && IsValidState()) {
        AUDIO_WARNING_LOG("source already inited");
        return SUCCESS;
    }

    std::lock_guard<std::mutex> lock(statusMutex_);
    attr_ = attr;
    logMode_ = system::GetIntParameter("persist.multimedia.audiolog.switch", 0);

    int32_t ret = CreateCapture();
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
    SetMute(muteState_);
    sourceInited_ = true;

    if (audioSrcClock_ != nullptr) {
        audioSrcClock_->Init(attr.sampleRate, attr.format, attr.channel);
    }
    return SUCCESS;
}

void BluetoothAudioCaptureSource::DeInit(void)
{
    std::lock_guard<std::mutex> lock(statusMutex_);
    Trace trace("BluetoothAudioCaptureSource::DeInit");

    sourceInited_ = false;
    started_ = false;
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_BLUETOOTH);
    CHECK_AND_RETURN(deviceManager != nullptr);
    if (IsValidState()) {
        deviceManager->DestroyCapture(adapterNameCase_, hdiCaptureId_);
    }
    audioCapture_ = nullptr;
    AUDIO_INFO_LOG("update validState:true");
    validState_ = true;
}

bool BluetoothAudioCaptureSource::IsInited(void)
{
    return sourceInited_;
}

int32_t BluetoothAudioCaptureSource::Start(void)
{
    std::lock_guard<std::mutex> lock(statusMutex_);
    Trace trace("BluetoothAudioCaptureSource::Start");
    AUDIO_INFO_LOG("in, halName: %{public}s", halName_.c_str());

    InitLatencyMeasurement();
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
    dumpFileName_ = halName_ + "_bluetooth_source_" + std::to_string(attr_.sourceType) + "_" + GetTime() + "_" +
        std::to_string(attr_.sampleRate) + "_" + std::to_string(attr_.channel) + "_" +
        std::to_string(attr_.format) + ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileName_, &dumpFile_);

    if (started_) {
        return SUCCESS;
    }
    callback_.OnCaptureState(true);
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    CHECK_AND_RETURN_RET(IsValidState(), ERR_INVALID_HANDLE);
    int32_t ret = audioCapture_->control.Start(reinterpret_cast<AudioHandle>(audioCapture_));
    if (ret < 0) {
        AUDIO_ERR_LOG("start fail");
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
    started_ = true;
    return SUCCESS;
}

int32_t BluetoothAudioCaptureSource::Stop(void)
{
    Trace trace("BluetoothAudioCaptureSource::Stop");
    std::promise<void> promiseEnsureLock;
    auto futurePromiseEnsureLock = promiseEnsureLock.get_future();
    std::thread stopThread([&promiseEnsureLock, this] {
        std::lock_guard<std::mutex> lock(statusMutex_);
        promiseEnsureLock.set_value();
        DoStop();
    });
    futurePromiseEnsureLock.get();
    stopThread.detach();
    return SUCCESS;
}

int32_t BluetoothAudioCaptureSource::Resume(void)
{
    std::lock_guard<std::mutex> lock(statusMutex_);
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    CHECK_AND_RETURN_RET(IsValidState(), ERR_INVALID_HANDLE);
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED, "not start, invalid state");

    Trace trace("BluetoothAudioCaptureSource::Resume");
    int32_t ret = audioCapture_->control.Resume(reinterpret_cast<AudioHandle>(audioCapture_));
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "resume fail");
    paused_ = false;
    return SUCCESS;
}

int32_t BluetoothAudioCaptureSource::Pause(void)
{
    std::lock_guard<std::mutex> lock(statusMutex_);
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    CHECK_AND_RETURN_RET(IsValidState(), ERR_INVALID_HANDLE);
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED, "not start, invalid state");

    Trace trace("BluetoothAudioCaptureSource::Pause");
    int32_t ret = audioCapture_->control.Pause(reinterpret_cast<AudioHandle>(audioCapture_));
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "pause fail");
    paused_ = true;
    return SUCCESS;
}

int32_t BluetoothAudioCaptureSource::Flush(void)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    CHECK_AND_RETURN_RET(IsValidState(), ERR_INVALID_HANDLE);
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED, "not start, invalid state");

    Trace trace("BluetoothAudioCaptureSource::Flush");
    int32_t ret = audioCapture_->control.Flush(reinterpret_cast<AudioHandle>(audioCapture_));
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "flush fail");
    return SUCCESS;
}

int32_t BluetoothAudioCaptureSource::Reset(void)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    CHECK_AND_RETURN_RET(IsValidState(), ERR_INVALID_HANDLE);
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED, "not start, invalid state");

    Trace trace("BluetoothAudioCaptureSource::Reset");
    int32_t ret = audioCapture_->control.Flush(reinterpret_cast<AudioHandle>(audioCapture_));
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "reset fail");
    return SUCCESS;
}

int32_t BluetoothAudioCaptureSource::CaptureFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    CHECK_AND_RETURN_RET(validState_, ERR_INVALID_HANDLE);
    Trace trace("BluetoothAudioCaptureSource::CaptureFrame");
    AudioCapturerSourceTsRecorder recorder(replyBytes, audioSrcClock_);

    int64_t stamp = ClockTime::GetCurNano();
    uint32_t frameLen = static_cast<uint32_t>(requestBytes);
    int32_t ret = audioCapture_->CaptureFrame(audioCapture_, reinterpret_cast<int8_t *>(frame), frameLen, &replyBytes);
    CHECK_AND_RETURN_RET_LOG(ret >= 0, ERR_READ_FAILED, "fail, ret: %{public}x", ret);
    CheckLatencySignal(reinterpret_cast<uint8_t *>(frame), replyBytes);

    BufferDesc buffer = { reinterpret_cast<uint8_t*>(frame), replyBytes, replyBytes };
    AudioStreamInfo streamInfo(static_cast<AudioSamplingRate>(attr_.sampleRate), AudioEncodingType::ENCODING_PCM,
        static_cast<AudioSampleFormat>(attr_.format), static_cast<AudioChannel>(attr_.channel));
    VolumeTools::DfxOperation(buffer, streamInfo, logUtilsTag_, volumeDataCount_);
    if (AudioDump::GetInstance().GetVersionType() == DumpFileUtil::BETA_VERSION) {
        DumpFileUtil::WriteDumpFile(dumpFile_, frame, replyBytes);
        AudioCacheMgr::GetInstance().CacheData(dumpFileName_, static_cast<void *>(frame), replyBytes);
    }
    CheckUpdateState(frame, requestBytes);
    stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
    if (logMode_) {
        AUDIO_WARNING_LOG("len: [%{public}" PRIu64 "], cost: [%{public}" PRId64 "]ms", requestBytes, stamp);
    }
    return SUCCESS;
}

int32_t BluetoothAudioCaptureSource::CaptureFrameWithEc(FrameDesc *fdesc, uint64_t &replyBytes, FrameDesc *fdescEc,
    uint64_t &replyBytesEc)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

std::string BluetoothAudioCaptureSource::GetAudioParameter(const AudioParamKey key, const std::string &condition)
{
    return "";
}

void BluetoothAudioCaptureSource::SetAudioParameter(
    const AudioParamKey key, const std::string &condition, const std::string &value)
{
    AUDIO_WARNING_LOG("not support");
    return;
}

int32_t BluetoothAudioCaptureSource::SetVolume(float left, float right)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
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

    audioCapture_->volume.SetVolume(reinterpret_cast<AudioHandle>(audioCapture_), volume);

    return SUCCESS;
}

int32_t BluetoothAudioCaptureSource::GetVolume(float &left, float &right)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    CHECK_AND_RETURN_RET(IsValidState(), ERR_INVALID_HANDLE);

    float val = 0.0;
    audioCapture_->volume.GetVolume(reinterpret_cast<AudioHandle>(audioCapture_), &val);
    left = val;
    right = val;
    return SUCCESS;
}

int32_t BluetoothAudioCaptureSource::SetMute(bool isMute)
{
    AUDIO_INFO_LOG("isMute: %{public}d", isMute);
    muteState_ = isMute;
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    CHECK_AND_RETURN_RET(IsValidState(), ERR_INVALID_HANDLE);

    if (sourceInited_) {
        int32_t ret = audioCapture_->volume.SetMute(reinterpret_cast<AudioHandle>(audioCapture_), isMute);
        if (ret != SUCCESS) {
            AUDIO_WARNING_LOG("set mute fail");
        } else {
            AUDIO_INFO_LOG("set mute succ");
        }
    }
    AUDIO_INFO_LOG("end");
    return SUCCESS;
}

int32_t BluetoothAudioCaptureSource::GetMute(bool &isMute)
{
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    CHECK_AND_RETURN_RET(IsValidState(), ERR_INVALID_HANDLE);
    bool hdiMuteState = false;
    int32_t ret = audioCapture_->volume.GetMute(reinterpret_cast<AudioHandle>(audioCapture_), &hdiMuteState);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("get mute fail");
    }
    AUDIO_DEBUG_LOG("hdiMuteState: %{public}d, muteState: %{public}d", hdiMuteState, muteState_);
    isMute = muteState_;
    return SUCCESS;
}

uint64_t BluetoothAudioCaptureSource::GetTransactionId(void)
{
    return reinterpret_cast<uint64_t>(audioCapture_);
}

int32_t BluetoothAudioCaptureSource::GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

float BluetoothAudioCaptureSource::GetMaxAmplitude(void)
{
    lastGetMaxAmplitudeTime_ = ClockTime::GetCurNano();
    startUpdate_ = true;
    return maxAmplitude_;
}

int32_t BluetoothAudioCaptureSource::SetAudioScene(AudioScene audioScene, bool scoExcludeFlag)
{
    AUDIO_INFO_LOG("update validState:%{public}s", (audioScene == AUDIO_SCENE_DEFAULT) ? "true" : "false");
    validState_ = (audioScene == AUDIO_SCENE_DEFAULT);
    return SUCCESS;
}

int32_t BluetoothAudioCaptureSource::UpdateActiveDevice(DeviceType inputDevice)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

void BluetoothAudioCaptureSource::RegistCallback(uint32_t type, std::shared_ptr<IAudioSourceCallback> callback)
{
    AUDIO_INFO_LOG("in");
    callback_.RegistCallback(type, callback);
}

int32_t BluetoothAudioCaptureSource::UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE], const size_t size)
{
#ifdef FEATURE_POWER_MANAGER
    CHECK_AND_RETURN_RET_LOG(runningLock_, ERR_INVALID_HANDLE, "running lock is nullptr");
    runningLock_->UpdateAppsUid(appsUid, appsUid + size);
    runningLock_->UpdateAppsUidToPowerMgr();
#endif
    return SUCCESS;
}

int32_t BluetoothAudioCaptureSource::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
#ifdef FEATURE_POWER_MANAGER
    CHECK_AND_RETURN_RET_LOG(runningLock_, ERR_INVALID_HANDLE, "running lock is nullptr");
    runningLock_->UpdateAppsUid(appsUid.cbegin(), appsUid.cend());
    runningLock_->UpdateAppsUidToPowerMgr();
#endif
    return SUCCESS;
}

void BluetoothAudioCaptureSource::SetInvalidState(void)
{
    AUDIO_INFO_LOG("update validState:false");
    std::lock_guard<std::mutex> lock(statusMutex_);
    validState_ = false;
    sourceInited_ = false;
    started_ = false;
}

void BluetoothAudioCaptureSource::DumpInfo(std::string &dumpString)
{
    dumpString += "type: BtSource\tstarted: " + std::string(started_ ? "true" : "false") + "\thalName: " + halName_ +
        "\n";
}

AudioFormat BluetoothAudioCaptureSource::ConvertToHdiFormat(AudioSampleFormat format)
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

void BluetoothAudioCaptureSource::InitAudioSampleAttr(struct AudioSampleAttributes &param)
{
    param.format = AUDIO_FORMAT_TYPE_PCM_16_BIT;
    param.channelCount = AUDIO_CHANNELCOUNT;
    param.sampleRate = AUDIO_SAMPLE_RATE_48K;
    param.interleaved = true;
    param.type = AUDIO_IN_MEDIA;
    param.period = DEEP_BUFFER_CAPTURE_PERIOD_SIZE;
    param.frameSize = PCM_16_BIT * param.channelCount / PCM_8_BIT;
    param.isBigEndian = false;
    param.isSignedData = true;
    if (param.frameSize != 0) {
        param.startThreshold = DEEP_BUFFER_CAPTURE_PERIOD_SIZE / (param.frameSize);
    }
    param.stopThreshold = INT_MAX;
    param.silenceThreshold = AUDIO_BUFFER_SIZE; // 16 * 1024

    param.sampleRate = attr_.sampleRate;
    param.format = ConvertToHdiFormat(attr_.format);
    param.isBigEndian = attr_.isBigEndian;
    param.channelCount = attr_.channel;
    param.silenceThreshold = attr_.bufferSize;
    param.frameSize = param.format * param.channelCount;
    if (param.frameSize != 0) {
        param.startThreshold = DEEP_BUFFER_CAPTURE_PERIOD_SIZE / (param.frameSize);
    }
}

void BluetoothAudioCaptureSource::InitDeviceDesc(struct AudioDeviceDescriptor &deviceDesc)
{
    deviceDesc.pins = PIN_IN_MIC;
    deviceDesc.desc = nullptr;
}

void BluetoothAudioCaptureSource::SetAudioRouteInfoForEnhanceChain(void)
{
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag != 1) {
        AudioEnhanceChainManager *audioEnhanceChainManager = AudioEnhanceChainManager::GetInstance();
        CHECK_AND_RETURN_LOG(audioEnhanceChainManager != nullptr, "audioEnhanceChainManager is nullptr");
        audioEnhanceChainManager->SetInputDevice(captureId_, currentActiveDevice_, "");
    }
}

int32_t BluetoothAudioCaptureSource::CreateCapture(void)
{
    Trace trace("BluetoothAudioCaptureSource::CreateCapture");

    struct AudioSampleAttributes param;
    struct AudioDeviceDescriptor deviceDesc;
    InitAudioSampleAttr(param);
    InitDeviceDesc(deviceDesc);

    AUDIO_INFO_LOG("create capture, halName: %{public}s, rate: %{public}u, channel: %{public}u, format: %{public}u, "
        "devicePin: %{public}u", halName_.c_str(), param.sampleRate, param.channelCount, param.format, deviceDesc.pins);
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_BLUETOOTH);
    CHECK_AND_RETURN_RET(deviceManager != nullptr, ERR_INVALID_HANDLE);
    void *capture = deviceManager->CreateCapture(adapterNameCase_, &param, &deviceDesc, hdiCaptureId_);
    audioCapture_ = static_cast<struct AudioCapture *>(capture);
    CHECK_AND_RETURN_RET(audioCapture_ != nullptr, ERR_NOT_STARTED);
    SetAudioRouteInfoForEnhanceChain();
    validState_ = true;

    return SUCCESS;
}

void BluetoothAudioCaptureSource::InitLatencyMeasurement(void)
{
    std::lock_guard<std::mutex> lock(signalDetectMutex_);
    if (!AudioLatencyMeasurement::CheckIfEnabled()) {
        return;
    }

    signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    CHECK_AND_RETURN_LOG(signalDetectAgent_ != nullptr, "signalDetectAgent is nullptr");
    signalDetectAgent_->sampleFormat_ = attr_.format;
    signalDetectAgent_->formatByteSize_ = GetFormatByteSize(attr_.format);
}

void BluetoothAudioCaptureSource::DeInitLatencyMeasurement(void)
{
    std::lock_guard<std::mutex> lock(signalDetectMutex_);

    signalDetected_ = false;
    signalDetectAgent_ = nullptr;
}

void BluetoothAudioCaptureSource::CheckLatencySignal(uint8_t *frame, size_t replyBytes)
{
    std::lock_guard<std::mutex> lock(signalDetectMutex_);
    CHECK_AND_RETURN(signalDetectAgent_ != nullptr);
    signalDetected_ = signalDetectAgent_->CheckAudioData(frame, replyBytes);
    if (signalDetected_) {
        AudioParamKey key = NONE;
        std::string condition = "debug_audio_latency_measurement";
        HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
        std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_BLUETOOTH);
        CHECK_AND_RETURN(deviceManager != nullptr);
        std::string value = deviceManager->GetAudioParameter(adapterNameCase_, key, condition);

        LatencyMonitor::GetInstance().UpdateDspTime(value.c_str());
        LatencyMonitor::GetInstance().UpdateSinkOrSourceTime(false, signalDetectAgent_->lastPeakBufferTime_);
        AUDIO_INFO_LOG("signal detected");
        signalDetected_ = false;
    }
}

void BluetoothAudioCaptureSource::CheckUpdateState(char *frame, size_t replyBytes)
{
    if (startUpdate_) {
        if (captureFrameNum_ == 0) {
            last10FrameStartTime_ = ClockTime::GetCurNano();
        }
        captureFrameNum_++;
        maxAmplitude_ = UpdateMaxAmplitude(static_cast<ConvertHdiFormat>(attr_.format), frame, replyBytes);
        if (captureFrameNum_ == GET_MAX_AMPLITUDE_FRAMES_THRESHOLD) {
            captureFrameNum_ = 0;
            if (last10FrameStartTime_ > lastGetMaxAmplitudeTime_) {
                startUpdate_ = false;
            }
        }
    }
}

int32_t BluetoothAudioCaptureSource::DoStop(void)
{
    AUDIO_INFO_LOG("halName: %{public}s", halName_.c_str());
    Trace trace("BluetoothAudioCaptureSource::DoStop");

    DeInitLatencyMeasurement();
#ifdef FEATURE_POWER_MANAGER
    if (runningLock_ != nullptr) {
        AUDIO_INFO_LOG("running lock unlock");
        runningLock_->UnLock();
    } else {
        AUDIO_WARNING_LOG("running lock is null, playback can not work well");
    }
#endif
    CHECK_AND_RETURN_RET_LOG(audioCapture_ != nullptr, ERR_INVALID_HANDLE, "capture is nullptr");
    CHECK_AND_RETURN_RET(IsValidState(), ERR_INVALID_HANDLE);
    if (!started_) {
        AUDIO_ERR_LOG("not start, invalid state");
        callback_.OnCaptureState(false);
        return ERR_OPERATION_FAILED;
    }
    int32_t ret = audioCapture_->control.Stop(reinterpret_cast<AudioHandle>(audioCapture_));
    CHECK_AND_RETURN_RET_LOG(ret >= 0, ERR_OPERATION_FAILED, "stop fail");
    started_ = false;
    paused_ = false;
    callback_.OnCaptureState(false);
    return SUCCESS;
}

bool BluetoothAudioCaptureSource::IsValidState(void)
{
    if (!validState_) {
        AUDIO_WARNING_LOG("disconnected, capture invalid");
    }
    return validState_;
}

void BluetoothAudioCaptureSource::SetDmDeviceType(uint16_t dmDeviceType, DeviceType deviceType)
{
    AUDIO_INFO_LOG("not support");
}

} // namespace AudioStandard
} // namespace OHOS
