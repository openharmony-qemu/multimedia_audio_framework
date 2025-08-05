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
#define LOG_TAG "AudioRenderSink"
#endif

#include "sink/audio_render_sink.h"
#include <thread>
#include <climits>
#include "parameters.h"
#include "audio_hdi_log.h"
#include "audio_errors.h"
#include "audio_dump_pcm.h"
#include "volume_tools.h"
#include "media_monitor_manager.h"
#include "audio_enhance_chain_manager.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include "manager/hdi_monitor.h"
#include "adapter/i_device_manager.h"

namespace OHOS {
namespace AudioStandard {
namespace {
const int64_t RENDER_FRAME_LIMIT = 50; // 50ms
const int64_t RENDER_FRAME_REPORT_LIMIT = 100000000; // 100ms
}
AudioRenderSink::AudioRenderSink(const uint32_t renderId, const std::string &halName)
    : renderId_(renderId), halName_(halName)
{
    if (halName_ == HDI_ID_INFO_DIRECT || halName_ == HDI_ID_INFO_VOIP) {
        sinkType_ = ADAPTER_TYPE_DIRECT;
    }
}

AudioRenderSink::~AudioRenderSink()
{
    AUDIO_WARNING_LOG("in");
    AUDIO_INFO_LOG("[%{public}s] volumeDataCount: %{public}" PRId64, logUtilsTag_.c_str(), volumeDataCount_);
#ifdef FEATURE_POWER_MANAGER
    if (runningLock_ != nullptr) {
        AUDIO_INFO_LOG("running lock unlock");
        runningLock_->UnLock();
    } else {
        AUDIO_WARNING_LOG("running lock is null, playback can not work well");
    }
#endif
    AudioPerformanceMonitor::GetInstance().DeleteOvertimeMonitor(sinkType_);
}

int32_t AudioRenderSink::Init(const IAudioSinkAttr &attr)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    attr_ = attr;
    adapterNameCase_ = attr_.adapterName;
    AUDIO_INFO_LOG("adapterNameCase_: %{public}s", adapterNameCase_.c_str());
    openSpeaker_ = attr_.openMicSpeaker;

    Trace trace("AudioRenderSink::Init " + adapterNameCase_);
    int32_t ret = InitRender();
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);

    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET(deviceManager != nullptr, ERR_INVALID_HANDLE);

    sinkInited_ = true;
    return SUCCESS;
}

void AudioRenderSink::DeInit(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    sinkInited_ = false;
    started_ = false;

    AUDIO_INFO_LOG("destroy render, hdiRenderId: %{public}u", hdiRenderId_);
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN(deviceManager != nullptr);
    renderInited_ = false;
    deviceManager->DestroyRender(adapterNameCase_, hdiRenderId_);
    audioRender_ = nullptr;
}

bool AudioRenderSink::IsInited(void)
{
    return sinkInited_;
}

int32_t AudioRenderSink::Start(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    AUDIO_INFO_LOG("halName: %{public}s", halName_.c_str());
    Trace trace("AudioRenderSink::Start");
#ifdef FEATURE_POWER_MANAGER
    AudioXCollie audioXCollie("AudioRenderSink::CreateRunningLock", TIMEOUT_SECONDS_10,
         nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    if (runningLock_ == nullptr) {
        WatchTimeout guard("create AudioRunningLock start");
        runningLock_ = std::make_shared<AudioRunningLock>(std::string(RUNNING_LOCK_NAME_BASE) + halName_);
        guard.CheckCurrTimeout();
    }
    if (runningLock_ != nullptr) {
        runningLock_->Lock(RUNNING_LOCK_TIMEOUTMS_LASTING);
    } else {
        AUDIO_ERR_LOG("running lock is null, playback can not work well");
    }
    audioXCollie.CancelXCollieTimer();
#endif
    dumpFileName_ = halName_ + "_sink_" + GetTime() + "_" + std::to_string(attr_.sampleRate) + "_" +
        std::to_string(attr_.channel) + "_" + std::to_string(attr_.format) + ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileName_, &dumpFile_);
    logUtilsTag_ = "AudioSink" + halName_;

    InitLatencyMeasurement();
    if (started_) {
        return SUCCESS;
    }
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    int32_t ret = audioRender_->Start(audioRender_);
    if (ret != SUCCESS) {
        HdiMonitor::ReportHdiException(HdiType::LOCAL, ErrorCase::CALL_HDI_FAILED, ret,
            "local start failed, halName_:" + halName_);
    }
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "start fail");
    UpdateSinkState(true);
    AudioPerformanceMonitor::GetInstance().RecordTimeStamp(sinkType_, INIT_LASTWRITTEN_TIME);
    started_ = true;
    isDataLinkConnected_ = false;
    return SUCCESS;
}

int32_t AudioRenderSink::Stop(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    AUDIO_INFO_LOG("halName: %{public}s", halName_.c_str());
    Trace trace("AudioRenderSink::Stop");
#ifdef FEATURE_POWER_MANAGER
    if (runningLock_ != nullptr) {
        std::thread runningLockThread([this] {
            runningLock_->UnLock();
        });
        runningLockThread.join();
    }
#endif

    DeInitLatencyMeasurement();
    if (!started_) {
        return SUCCESS;
    }
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    if (halName_ == "primary") {
        const char keyValueList[] = "primary=stop";
        if (audioRender_->SetExtraParams(audioRender_, keyValueList) == 0) {
            AUDIO_INFO_LOG("set primary stream stop info to hal");
        }
    }
    int32_t ret = audioRender_->Stop(audioRender_);
    UpdateSinkState(false);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "stop fail");
    started_ = false;

    DumpFileUtil::CloseDumpFile(&dumpFile_);
    return SUCCESS;
}

int32_t AudioRenderSink::Resume(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    AUDIO_INFO_LOG("halName: %{public}s", halName_.c_str());
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED, "not start, invalid state");

    if (!paused_) {
        return SUCCESS;
    }
    int32_t ret = audioRender_->Resume(audioRender_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "resume fail");
    AudioPerformanceMonitor::GetInstance().RecordTimeStamp(sinkType_, INIT_LASTWRITTEN_TIME);
    paused_ = false;
    return SUCCESS;
}

int32_t AudioRenderSink::Pause(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    AUDIO_INFO_LOG("halName: %{public}s", halName_.c_str());
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED, "not start, invalid state");

    if (paused_) {
        return SUCCESS;
    }
    int32_t ret = audioRender_->Pause(audioRender_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "pause fail");
    paused_ = true;
    return SUCCESS;
}

int32_t AudioRenderSink::Flush(void)
{
    AUDIO_INFO_LOG("halName: %{public}s", halName_.c_str());
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED, "not start, invalid state");

    int32_t ret = audioRender_->Flush(audioRender_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "flush fail");
    return SUCCESS;
}

int32_t AudioRenderSink::Reset(void)
{
    AUDIO_INFO_LOG("halName: %{public}s", halName_.c_str());
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED, "not start, invalid state");

    int32_t ret = audioRender_->Flush(audioRender_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "reset fail");
    return SUCCESS;
}

int32_t AudioRenderSink::RenderFrame(char &data, uint64_t len, uint64_t &writeLen)
{
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    if (!started_) {
        AUDIO_WARNING_LOG("not start, invalid state");
    }
    if (audioMonoState_) {
        AdjustStereoToMono(&data, len);
    }
    if (audioBalanceState_) {
        AdjustAudioBalance(&data, len);
    }
    CheckUpdateState(&data, len);
    if (switchDeviceMute_ || deviceConnectedFlag_) {
        Trace trace("AudioRenderSink::RenderFrame::renderEmpty");
        if (memset_s(reinterpret_cast<void *>(&data), static_cast<size_t>(len), 0, static_cast<size_t>(len)) != EOK) {
            AUDIO_WARNING_LOG("call memset_s fail");
        }
    }
    CheckLatencySignal(reinterpret_cast<uint8_t *>(&data), len);

    BufferDesc buffer = { reinterpret_cast<uint8_t *>(&data), len, len };
    AudioStreamInfo streamInfo(static_cast<AudioSamplingRate>(attr_.sampleRate), AudioEncodingType::ENCODING_PCM,
        static_cast<AudioSampleFormat>(attr_.format), static_cast<AudioChannel>(attr_.channel));
    VolumeTools::DfxOperation(buffer, streamInfo, logUtilsTag_, volumeDataCount_);
    if (AudioDump::GetInstance().GetVersionType() == DumpFileUtil::BETA_VERSION) {
        DumpFileUtil::WriteDumpFile(dumpFile_, static_cast<void *>(&data), len);
        AudioCacheMgr::GetInstance().CacheData(dumpFileName_, static_cast<void *>(&data), len);
    }
    Trace trace("AudioRenderSink::RenderFrame");
    int64_t stamp = ClockTime::GetCurNano();
    int32_t ret = audioRender_->RenderFrame(audioRender_, reinterpret_cast<int8_t *>(&data), static_cast<uint32_t>(len),
        &writeLen);
    stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
    AudioPerformanceMonitor::GetInstance().RecordTimeStamp(sinkType_, ClockTime::GetCurNano());
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_WRITE_FAILED, "fail, ret: %{public}x", ret);
    if (stamp >= RENDER_FRAME_LIMIT) {
        AUDIO_WARNING_LOG("len: [%{public}" PRIu64 "], cost: [%{public}" PRId64 "]ms", len, stamp);
    }
#ifdef FEATURE_POWER_MANAGER
    if (runningLock_) {
        runningLock_->UpdateAppsUidToPowerMgr();
    }
#endif
    if (stamp > RENDER_FRAME_REPORT_LIMIT) {
        HdiMonitor::ReportHdiException(HdiType::LOCAL, ErrorCase::CALL_HDI_TIMEOUT,
            static_cast<int32_t>(stamp), "call RenderFrame too long, " + halName_);
    }

    return SUCCESS;
}

int64_t AudioRenderSink::GetVolumeDataCount()
{
    return volumeDataCount_;
}

int32_t AudioRenderSink::SuspendRenderSink(void)
{
    return SUCCESS;
}

int32_t AudioRenderSink::RestoreRenderSink(void)
{
    return SUCCESS;
}

void AudioRenderSink::SetAudioParameter(const AudioParamKey key, const std::string &condition, const std::string &value)
{
    AUDIO_INFO_LOG("key: %{public}d, condition: %{public}s, value: %{public}s", key, condition.c_str(), value.c_str());
    CHECK_AND_RETURN_LOG(audioRender_ != nullptr, "render is nullptr");
    int32_t ret = audioRender_->SetExtraParams(audioRender_, value.c_str());
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("set parameter fail, error code: %{public}d", ret);
    }
}

std::string AudioRenderSink::GetAudioParameter(const AudioParamKey key, const std::string &condition)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    AUDIO_INFO_LOG("key: %{public}d, condition: %{public}s, halName: %{public}s", key, condition.c_str(),
        halName_.c_str());
    if (condition.starts_with("get_usb_info#C") && halName_ == HDI_ID_INFO_USB) {
        // init adapter to get parameter before load sink module (need fix)
        adapterNameCase_ = "usb";
        HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
        std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
        CHECK_AND_RETURN_RET_LOG(deviceManager != nullptr, "", "deviceManager is nullptr");
        return deviceManager->GetAudioParameter(adapterNameCase_, key, condition);
    }
    if (key == AudioParamKey::GET_DP_DEVICE_INFO && halName_ == HDI_ID_INFO_DP) {
        // init adapter and render to get parameter before load sink module (need fix)
        return GetDPDeviceInfo(condition);
    }
    return "";
}

int32_t AudioRenderSink::SetVolume(float left, float right)
{
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");

    leftVolume_ = left;
    rightVolume_ = right;
    CHECK_AND_RETURN_RET_LOG(!(halName_ == "voip" && switchDeviceMute_ && (abs(left) > FLOAT_EPS ||
        abs(right) > FLOAT_EPS)), ERR_ILLEGAL_STATE, "mute for switch device at voip scene, not support set volume");
    float volume;
    if ((abs(leftVolume_) < FLOAT_EPS) && (abs(rightVolume_) > FLOAT_EPS)) {
        volume = rightVolume_;
    } else if ((abs(leftVolume_) > FLOAT_EPS) && (abs(rightVolume_) < FLOAT_EPS)) {
        volume = leftVolume_;
    } else {
        volume = (leftVolume_ + rightVolume_) / HALF_FACTOR;
    }

    int32_t ret = audioRender_->SetVolume(audioRender_, volume);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("set volume fail");
    }

    return ret;
}

int32_t AudioRenderSink::GetVolume(float &left, float &right)
{
    left = leftVolume_;
    right = rightVolume_;
    return SUCCESS;
}

int32_t AudioRenderSink::GetLatency(uint32_t &latency)
{
    Trace trace("AudioRenderSink::GetLatency");
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");

    uint32_t hdiLatency;
    int32_t ret = audioRender_->GetLatency(audioRender_, &hdiLatency);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "get latency fail");
    latency = hdiLatency;
    return SUCCESS;
}

int32_t AudioRenderSink::GetTransactionId(uint64_t &transactionId)
{
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    transactionId = reinterpret_cast<uint64_t>(audioRender_);
    return SUCCESS;
}

int32_t AudioRenderSink::GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec)
{
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");

    struct AudioTimeStamp stamp = {};
    int32_t ret = audioRender_->GetRenderPosition(audioRender_, &frames, &stamp);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "get render position fail, ret: %{public}d", ret);
    int64_t maxSec = 9223372036; // (9223372036 + 1) * 10^9 > INT64_MAX, seconds should not bigger than it
    CHECK_AND_RETURN_RET_LOG(stamp.tvSec >= 0 && stamp.tvSec <= maxSec && stamp.tvNSec >= 0 &&
        stamp.tvNSec <= SECOND_TO_NANOSECOND, ERR_OPERATION_FAILED,
        "get invalid time, second: %{public}" PRId64 ", nanosecond: %{public}" PRId64, stamp.tvSec, stamp.tvNSec);
    timeSec = stamp.tvSec;
    timeNanoSec = stamp.tvNSec;
    return ret;
}

float AudioRenderSink::GetMaxAmplitude(void)
{
    lastGetMaxAmplitudeTime_ = ClockTime::GetCurNano();
    startUpdate_ = true;
    return maxAmplitude_;
}

void AudioRenderSink::SetAudioMonoState(bool audioMono)
{
    audioMonoState_ = audioMono;
}

void AudioRenderSink::SetAudioBalanceValue(float audioBalance)
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

int32_t AudioRenderSink::SetSinkMuteForSwitchDevice(bool mute)
{
    std::lock_guard<std::mutex> lock(switchDeviceMutex_);
    AUDIO_INFO_LOG("set %{public}s mute %{public}d", halName_.c_str(), mute);
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");

    if (mute) {
        muteCount_++;
        if (switchDeviceMute_) {
            AUDIO_INFO_LOG("%{public}s already muted", halName_.c_str());
            return SUCCESS;
        }
        switchDeviceMute_ = true;
        if (halName_ == HDI_ID_INFO_VOIP) {
            audioRender_->SetVolume(audioRender_, 0.0f);
        }
    } else {
        muteCount_--;
        if (muteCount_ > 0) {
            AUDIO_WARNING_LOG("%{public}s not all unmuted", halName_.c_str());
            return SUCCESS;
        }
        switchDeviceMute_ = false;
        muteCount_ = 0;
        if (halName_ == HDI_ID_INFO_VOIP) {
            SetVolume(leftVolume_, rightVolume_);
        }
    }

    return SUCCESS;
}

int32_t AudioRenderSink::SetDeviceConnectedFlag(bool flag)
{
    AUDIO_INFO_LOG("flag %{public}d", flag);
    deviceConnectedFlag_ = flag;
    return SUCCESS;
}

int32_t AudioRenderSink::SetAudioScene(AudioScene audioScene, bool scoExcludeFlag)
{
    CHECK_AND_RETURN_RET_LOG(audioScene >= AUDIO_SCENE_DEFAULT && audioScene < AUDIO_SCENE_MAX, ERR_INVALID_PARAM,
        "invalid scene");
    if (!openSpeaker_) {
        return SUCCESS;
    }

    if (audioScene != currentAudioScene_ && !scoExcludeFlag) {
        struct AudioSceneDescriptor sceneDesc;
        InitSceneDesc(sceneDesc, audioScene);

        CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
        int32_t ret = audioRender_->SelectScene(audioRender_, &sceneDesc);
        CHECK_AND_RETURN_RET_LOG(ret >= 0, ERR_OPERATION_FAILED, "select scene fail, ret: %{public}d", ret);
    }
    bool isRingingToDefaultScene = false;
    if (audioScene != currentAudioScene_) {
        if (audioScene == AUDIO_SCENE_PHONE_CALL || audioScene == AUDIO_SCENE_PHONE_CHAT) {
            forceSetRouteFlag_ = true;
        }
        if (audioScene == AUDIO_SCENE_DEFAULT &&
            (currentAudioScene_ == AUDIO_SCENE_RINGING || currentAudioScene_ == AUDIO_SCENE_VOICE_RINGING)) {
            isRingingToDefaultScene = true;
        }
        currentAudioScene_ = audioScene;
    }

    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET(deviceManager != nullptr, ERR_INVALID_HANDLE);
    deviceManager->SetAudioScene(currentAudioScene_);

    if (isRingingToDefaultScene) {
        AUDIO_INFO_LOG("ringing scene to default scene");
        return SUCCESS;
    }
    return SUCCESS;
}

int32_t AudioRenderSink::GetAudioScene(void)
{
    return currentAudioScene_;
}

int32_t AudioRenderSink::UpdateActiveDevice(std::vector<DeviceType> &outputDevices)
{
    CHECK_AND_RETURN_RET_LOG(!outputDevices.empty() && outputDevices.size() <= AUDIO_CONCURRENT_ACTIVE_DEVICES_LIMIT,
        ERR_INVALID_PARAM, "invalid device");
    AUDIO_INFO_LOG("device: %{public}d, currentActiveDevice: %{public}d", outputDevices[0], currentActiveDevice_);
    if (currentActiveDevice_ == outputDevices[0] && outputDevices.size() ==
        static_cast<uint32_t>(currentDevicesSize_) && !forceSetRouteFlag_) {
        AUDIO_INFO_LOG("output device not change, device: %{public}d", outputDevices[0]);
        return SUCCESS;
    }
    forceSetRouteFlag_ = false;
    currentActiveDevice_ = outputDevices[0];
    currentDevicesSize_ = static_cast<int32_t>(outputDevices.size());
    SetAudioRouteInfoForEnhanceChain();
    return DoSetOutputRoute(outputDevices);
}

void AudioRenderSink::RegistCallback(uint32_t type, IAudioSinkCallback *callback)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    callback_.RegistCallback(type, callback);
    AUDIO_INFO_LOG("regist succ");
}

void AudioRenderSink::ResetActiveDeviceForDisconnect(DeviceType device)
{
    if (currentActiveDevice_ == device) {
        currentActiveDevice_ = DEVICE_TYPE_NONE;
    }
}

int32_t AudioRenderSink::SetPaPower(int32_t flag)
{
    Trace trace("AudioRenderSink::SetPaPower flag: " + std::to_string(flag));
    std::string param;

    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    AUDIO_INFO_LOG("flag: %{public}d, paStatus: %{public}d", flag, paStatus_);
    if (flag == 0 && paStatus_ == 1) {
        param = "zero_volume=true;routing=0";
        AUDIO_INFO_LOG("param: %{public}s", param.c_str());
        int32_t ret = audioRender_->SetExtraParams(audioRender_, param.c_str());
        if (ret == SUCCESS) {
            paStatus_ = 0;
            WriteSmartPAStatusSysEvent(paStatus_);
        }
        return ret;
    } else if (flag == 1 && paStatus_ == 0) {
        param = "zero_volume=true;routing=" + GetRouting();
        AUDIO_INFO_LOG("param: %{public}s", param.c_str());
        int32_t ret = audioRender_->SetExtraParams(audioRender_, param.c_str());
        param = "zero_volume=false";
        ret += audioRender_->SetExtraParams(audioRender_, param.c_str());
        if (ret == SUCCESS) {
            paStatus_ = 1;
            WriteSmartPAStatusSysEvent(paStatus_);
        }
        return ret;
    } else if ((flag == 0 && paStatus_ == 0) || (flag == 1 && paStatus_ == 1)) {
        return SUCCESS;
    }

    AUDIO_WARNING_LOG("invalid flag");
    return ERR_INVALID_PARAM;
}

int32_t AudioRenderSink::SetPriPaPower(void)
{
    time_t currentTime = time(nullptr);
    double diff = difftime(currentTime, startTime_);
    if (diff > INTERVAL) {
        CHECK_AND_RETURN_RET(audioRender_ != nullptr, ERR_INVALID_HANDLE);
        int32_t ret = audioRender_->SetExtraParams(audioRender_, "primary=start");
        if (ret == SUCCESS) {
            AUDIO_INFO_LOG("set primary stream start info to hal");
        }
        time(&startTime_);
        return ret;
    }
    return ERR_OPERATION_FAILED;
}

int32_t AudioRenderSink::UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size)
{
#ifdef FEATURE_POWER_MANAGER
    CHECK_AND_RETURN_RET_LOG(runningLock_, ERR_INVALID_HANDLE, "running lock is nullptr");
    runningLock_->UpdateAppsUid(appsUid, appsUid + size);
#endif
    return SUCCESS;
}

int32_t AudioRenderSink::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
#ifdef FEATURE_POWER_MANAGER
    CHECK_AND_RETURN_RET_LOG(runningLock_, ERR_INVALID_HANDLE, "running lock is nullptr");
    runningLock_->UpdateAppsUid(appsUid.cbegin(), appsUid.cend());
    runningLock_->UpdateAppsUidToPowerMgr();
#endif
    return SUCCESS;
}

void AudioRenderSink::SetAddress(const std::string &address)
{
    address_ = address;
}

void AudioRenderSink::DumpInfo(std::string &dumpString)
{
    dumpString += "type: PrimarySink\tstarted: " + std::string(started_ ? "true" : "false") + "\thalName: " + halName_ +
        "\tcurrentActiveDevice: " + std::to_string(currentActiveDevice_) + "\n";
}

uint32_t AudioRenderSink::PcmFormatToBit(AudioSampleFormat format)
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

AudioFormat AudioRenderSink::ConvertToHdiFormat(AudioSampleFormat format)
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

int32_t AudioRenderSink::ConvertByteToAudioFormat(int32_t format)
{
    int32_t audioSampleFormat = 0;
    switch (format) {
        case FORMAT_1_BYTE:
            audioSampleFormat = SAMPLE_U8;
            break;
        case FORMAT_2_BYTE:
            audioSampleFormat = SAMPLE_S16LE;
            break;
        case FORMAT_3_BYTE:
            audioSampleFormat = SAMPLE_S24LE;
            break;
        case FORMAT_4_BYTE:
            audioSampleFormat = SAMPLE_S32LE;
            break;
        default:
            audioSampleFormat = SAMPLE_S16LE;
            break;
    }
    return audioSampleFormat;
}

std::string AudioRenderSink::ParseAudioFormatToStr(int32_t format)
{
    switch (format) {
        case FORMAT_1_BYTE:
            return "u8";
        case FORMAT_2_BYTE:
            return "s16";
        case FORMAT_3_BYTE:
            return "s24";
        case FORMAT_4_BYTE:
            return "s32";
        default:
            return "s16";
    }
    return "";
}

AudioSampleFormat AudioRenderSink::ParseAudioFormat(const std::string &format)
{
    if (format == "AUDIO_FORMAT_PCM_16_BIT") {
        return SAMPLE_S16LE;
    } else if (format == "AUDIO_FORMAT_PCM_24_BIT" || format == "AUDIO_FORMAT_PCM_24_BIT_PACKED") {
        return SAMPLE_S24LE;
    } else if (format == "AUDIO_FORMAT_PCM_32_BIT") {
        return SAMPLE_S32LE;
    } else {
        return SAMPLE_S16LE;
    }
}

AudioCategory AudioRenderSink::GetAudioCategory(AudioScene audioScene)
{
    AudioCategory audioCategory;
    switch (audioScene) {
        case AUDIO_SCENE_DEFAULT:
            audioCategory = AUDIO_IN_MEDIA;
            break;
        case AUDIO_SCENE_RINGING:
        case AUDIO_SCENE_VOICE_RINGING:
            audioCategory = AUDIO_IN_RINGTONE;
            break;
        case AUDIO_SCENE_PHONE_CALL:
            audioCategory = AUDIO_IN_CALL;
            break;
        case AUDIO_SCENE_PHONE_CHAT:
            audioCategory = AUDIO_IN_COMMUNICATION;
            break;
        default:
            audioCategory = AUDIO_IN_MEDIA;
            break;
    }
    AUDIO_DEBUG_LOG("audioCategory: %{public}d", audioCategory);

    return audioCategory;
}

AudioPortPin AudioRenderSink::GetAudioPortPin(void) const noexcept
{
    switch (attr_.deviceType) {
        case DEVICE_TYPE_EARPIECE:
            return PIN_OUT_EARPIECE;
        case DEVICE_TYPE_SPEAKER:
            return PIN_OUT_SPEAKER;
        case DEVICE_TYPE_WIRED_HEADSET:
            return PIN_OUT_HEADSET;
        case DEVICE_TYPE_WIRED_HEADPHONES:
            return PIN_OUT_HEADPHONE;
        case DEVICE_TYPE_BLUETOOTH_SCO:
            return PIN_OUT_BLUETOOTH_SCO;
        case DEVICE_TYPE_USB_HEADSET:
            return PIN_OUT_USB_EXT;
        case DEVICE_TYPE_HDMI:
             return PIN_OUT_HDMI;
        case DEVICE_TYPE_NONE:
            return PIN_NONE;
        default:
            return PIN_OUT_SPEAKER;
    }
}

uint32_t AudioRenderSink::GetUniqueId(void) const
{
    if (halName_ == HDI_ID_INFO_USB) {
        return GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_USB);
    } else if (halName_ == HDI_ID_INFO_DP) {
        return GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_DP);
    } else if (halName_ == HDI_ID_INFO_VOIP) {
        return GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_VOIP);
    } else if (halName_ == HDI_ID_INFO_DIRECT) {
        return GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_DIRECT);
    }
    return GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_PRIMARY);
}

void AudioRenderSink::InitAudioSampleAttr(struct AudioSampleAttributes &param)
{
    param.channelCount = AUDIO_CHANNELCOUNT;
    param.sampleRate = AUDIO_SAMPLE_RATE_48K;
    param.interleaved = true;
    param.streamId = static_cast<int32_t>(GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_PRIMARY));
    param.type = AUDIO_IN_MEDIA;
    param.period = DEEP_BUFFER_RENDER_PERIOD_SIZE;
    param.isBigEndian = false;
    param.isSignedData = true;
    param.stopThreshold = INT_MAX;
    param.silenceThreshold = 0;

    param.sampleRate = attr_.sampleRate;
    param.channelCount = attr_.channel;
    if (param.channelCount == MONO) {
        param.channelLayout = CH_LAYOUT_MONO;
    } else if (param.channelCount == STEREO) {
        param.channelLayout = CH_LAYOUT_STEREO;
    }
    if (halName_ == HDI_ID_INFO_DP) {
        param.type = AUDIO_DP;
    } else if (halName_ == HDI_ID_INFO_DIRECT) {
        param.type = AUDIO_DIRECT;
        param.streamId = static_cast<int32_t>(GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_DIRECT));
    } else if (halName_ == HDI_ID_INFO_VOIP) {
        param.type = AUDIO_IN_COMMUNICATION;
        param.streamId = static_cast<int32_t>(GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_VOIP));
    }
    param.format = ConvertToHdiFormat(attr_.format);
    param.frameSize = PcmFormatToBit(attr_.format) * param.channelCount / PCM_8_BIT;
    if (param.frameSize != 0) {
        param.startThreshold = DEEP_BUFFER_RENDER_PERIOD_SIZE / (param.frameSize);
    }
}

void AudioRenderSink::InitDeviceDesc(struct AudioDeviceDescriptor &deviceDesc)
{
    if (halName_ == HDI_ID_INFO_USB) {
        deviceDesc.desc = const_cast<char *>(address_.c_str());
    } else {
        deviceDesc.desc = const_cast<char *>(attr_.address.c_str());
    }
    deviceDesc.pins = GetAudioPortPin();
    if (halName_ == HDI_ID_INFO_USB) {
        deviceDesc.pins = PIN_OUT_USB_HEADSET;
    } else if (halName_ == HDI_ID_INFO_DP) {
        deviceDesc.pins = PIN_OUT_DP;
    }
}

void AudioRenderSink::InitSceneDesc(struct AudioSceneDescriptor &sceneDesc, AudioScene audioScene)
{
    sceneDesc.scene.id = GetAudioCategory(audioScene);
    if (halName_ == HDI_ID_INFO_DIRECT) {
        sceneDesc.scene.id = AUDIO_DIRECT;
    } else if (halName_ == HDI_ID_INFO_VOIP) {
        sceneDesc.scene.id = AUDIO_IN_COMMUNICATION;
    }

    AudioPortPin port = GetAudioPortPin();
    if (halName_ == HDI_ID_INFO_USB) {
        port = PIN_OUT_USB_HEADSET;
    } else if (halName_ == HDI_ID_INFO_DP) {
        port = PIN_OUT_DP;
    }
    AUDIO_DEBUG_LOG("port: %{public}d", port);
    sceneDesc.desc.pins = port;
    sceneDesc.desc.desc = const_cast<char *>("");
}

// LCOV_EXCL_START
void AudioRenderSink::SetAudioRouteInfoForEnhanceChain(void)
{
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag != 1) {
        AudioEnhanceChainManager *audioEnhanceChainManager = AudioEnhanceChainManager::GetInstance();
        CHECK_AND_RETURN_LOG(audioEnhanceChainManager != nullptr, "audioEnhanceChainManager is nullptr");
        if (halName_ == HDI_ID_INFO_USB) {
            audioEnhanceChainManager->SetOutputDevice(renderId_, DEVICE_TYPE_USB_ARM_HEADSET);
        } else if (halName_ == HDI_ID_INFO_DP) {
            audioEnhanceChainManager->SetOutputDevice(renderId_, DEVICE_TYPE_DP);
        } else {
            audioEnhanceChainManager->SetOutputDevice(renderId_, currentActiveDevice_);
        }
    }
}
// LCOV_EXCL_STOP

int32_t AudioRenderSink::CreateRender(void)
{
    Trace trace("AudioRenderSink::CreateRender");

    struct AudioSampleAttributes param;
    struct AudioDeviceDescriptor deviceDesc;
    InitAudioSampleAttr(param);
    InitDeviceDesc(deviceDesc);

    AUDIO_INFO_LOG("create render, halName: %{public}s, rate: %{public}u, channel: %{public}u, format: %{public}u, "
        "devicePin: %{public}u, desc: %{public}s", halName_.c_str(), param.sampleRate, param.channelCount, param.format,
        deviceDesc.pins, deviceDesc.desc);
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET(deviceManager != nullptr, ERR_INVALID_HANDLE);
    void *render = deviceManager->CreateRender(adapterNameCase_, &param, &deviceDesc, hdiRenderId_);
    audioRender_ = static_cast<struct IAudioRender *>(render);
    CHECK_AND_RETURN_RET(audioRender_ != nullptr, ERR_NOT_STARTED);
    SetAudioRouteInfoForEnhanceChain();

    AUDIO_INFO_LOG("create render success, hdiRenderId_: %{public}u, desc: %{public}s", hdiRenderId_, deviceDesc.desc);
    return SUCCESS;
}

int32_t AudioRenderSink::DoSetOutputRoute(std::vector<DeviceType> &outputDevices)
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET(deviceManager != nullptr, ERR_INVALID_HANDLE);
    int32_t ret = deviceManager->SetOutputRoute(adapterNameCase_, outputDevices,
        GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_PRIMARY));
    return ret;
}

int32_t AudioRenderSink::InitRender(void)
{
    AUDIO_INFO_LOG("start, halName: %{public}s", halName_.c_str());
    Trace trace("AudioRenderSink::InitRender");
    if (renderInited_) {
        AUDIO_INFO_LOG("render already inited");
        return SUCCESS;
    }

    int32_t ret = CreateRender();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "create render fail");
    if (openSpeaker_) {
        ret = SUCCESS;
        std::vector<DeviceType> outputDevices;
        if (halName_ == HDI_ID_INFO_USB) {
            outputDevices.push_back(DEVICE_TYPE_USB_ARM_HEADSET);
            ret = UpdateActiveDevice(outputDevices);
        } else if (halName_ == HDI_ID_INFO_DP) {
            outputDevices.push_back(DEVICE_TYPE_DP);
            ret = UpdateActiveDevice(outputDevices);
        } else if (halName_ == HDI_ID_INFO_VOIP) {
            // voip hal do not need to SetOutputRoute when create render, will SetOutputRoute when start stream
            AUDIO_INFO_LOG("voip hal do not need to SetOutputRoute when create render");
        } else {
            DeviceType type = static_cast<DeviceType>(attr_.deviceType);
            if (type == DEVICE_TYPE_INVALID) {
                type = DEVICE_TYPE_SPEAKER;
            }
            outputDevices.push_back(type);
            ret = UpdateActiveDevice(outputDevices);
        }
        if (ret != SUCCESS) {
            AUDIO_WARNING_LOG("update route fail, ret: %{public}d", ret);
        }
    }
    renderInited_ = true;
    return SUCCESS;
}

void AudioRenderSink::InitLatencyMeasurement(void)
{
    if (!AudioLatencyMeasurement::CheckIfEnabled()) {
        return;
    }

    AUDIO_INFO_LOG("in");
    signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    CHECK_AND_RETURN_LOG(signalDetectAgent_ != nullptr, "signalDetectAgent is nullptr");
    signalDetectAgent_->sampleFormat_ = attr_.format;
    signalDetectAgent_->formatByteSize_ = GetFormatByteSize(attr_.format);
    signalDetected_ = false;
}

void AudioRenderSink::DeInitLatencyMeasurement(void)
{
    signalDetectAgent_ = nullptr;
}

void AudioRenderSink::CheckLatencySignal(uint8_t *data, size_t len)
{
    CHECK_AND_RETURN(signalDetectAgent_ != nullptr);
    uint32_t byteSize = static_cast<uint32_t>(GetFormatByteSize(attr_.format));
    size_t newlyCheckedTime = len / (attr_.sampleRate / MILLISECOND_PER_SECOND) /
        (byteSize * sizeof(uint8_t) * attr_.channel);
    signalDetectedTime_ += newlyCheckedTime;
    if (signalDetectedTime_ >= MILLISECOND_PER_SECOND && signalDetectAgent_->signalDetected_ &&
        !signalDetectAgent_->dspTimestampGot_) {
        AudioParamKey key = NONE;
        std::string condition = "debug_audio_latency_measurement";
        HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
        std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
        CHECK_AND_RETURN(deviceManager != nullptr);
        std::string value = deviceManager->GetAudioParameter(adapterNameCase_, key, condition);

        LatencyMonitor::GetInstance().UpdateDspTime(value.c_str());
        LatencyMonitor::GetInstance().UpdateSinkOrSourceTime(true, signalDetectAgent_->lastPeakBufferTime_);
        LatencyMonitor::GetInstance().ShowTimestamp(true);
        signalDetectAgent_->dspTimestampGot_ = true;
        signalDetectAgent_->signalDetected_ = false;
    }
    signalDetected_ = signalDetectAgent_->CheckAudioData(data, len);
    if (signalDetected_) {
        AUDIO_INFO_LOG("signal detected");
        signalDetectedTime_ = 0;
    }
}

void AudioRenderSink::AdjustStereoToMono(char *data, uint64_t len)
{
    // only stereo is supported now (stereo channel count is 2)
    CHECK_AND_RETURN_LOG(attr_.channel == STEREO_CHANNEL_COUNT, "unsupport, channel: %{public}d", attr_.channel);

    switch (attr_.format) {
        case SAMPLE_U8:
            AdjustStereoToMonoForPCM8Bit(reinterpret_cast<int8_t *>(data), len);
            break;
        case SAMPLE_S16LE:
            AdjustStereoToMonoForPCM16Bit(reinterpret_cast<int16_t *>(data), len);
            break;
        case SAMPLE_S24LE:
            AdjustStereoToMonoForPCM24Bit(reinterpret_cast<uint8_t *>(data), len);
            break;
        case SAMPLE_S32LE:
            AdjustStereoToMonoForPCM32Bit(reinterpret_cast<int32_t *>(data), len);
            break;
        default:
            // if the audio format is unsupported, the audio data will not be changed
            AUDIO_ERR_LOG("unsupport, format: %{public}d", attr_.format);
            break;
    }
}

void AudioRenderSink::AdjustAudioBalance(char *data, uint64_t len)
{
    // only stereo is supported now (stereo channel count is 2)
    CHECK_AND_RETURN_LOG(attr_.channel == STEREO_CHANNEL_COUNT, "unsupport, channel: %{public}d", attr_.channel);

    switch (attr_.format) {
        case SAMPLE_U8:
            // this function needs further tested for usability
            AdjustAudioBalanceForPCM8Bit(reinterpret_cast<int8_t *>(data), len, leftBalanceCoef_, rightBalanceCoef_);
            break;
        case SAMPLE_S16LE:
            AdjustAudioBalanceForPCM16Bit(reinterpret_cast<int16_t *>(data), len, leftBalanceCoef_, rightBalanceCoef_);
            break;
        case SAMPLE_S24LE:
            // this function needs further tested for usability
            AdjustAudioBalanceForPCM24Bit(reinterpret_cast<uint8_t *>(data), len, leftBalanceCoef_, rightBalanceCoef_);
            break;
        case SAMPLE_S32LE:
            AdjustAudioBalanceForPCM32Bit(reinterpret_cast<int32_t *>(data), len, leftBalanceCoef_, rightBalanceCoef_);
            break;
        default:
            // if the audio format is unsupported, the audio data will not be changed
            AUDIO_ERR_LOG("unsupport, format: %{public}d", attr_.format);
            break;
    }
}

void AudioRenderSink::CheckUpdateState(char *data, uint64_t len)
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
                maxAmplitude_ = 0;
            }
        }
    }
}

std::string AudioRenderSink::GetAttrInfoStr(const struct AudioSampleAttributes &attrInfo)
{
    CHECK_AND_RETURN_RET_LOG(attrInfo.sampleRate > 0, "", "invalid rate: %{public}d", attrInfo.sampleRate);
    CHECK_AND_RETURN_RET_LOG(attrInfo.format > 0, "", "invalid format: %{public}d", attrInfo.format);
    CHECK_AND_RETURN_RET_LOG(attrInfo.channelCount > 0, "", "invalid channel: %{public}d", attrInfo.channelCount);

    uint32_t bufferSize = attrInfo.sampleRate * attrInfo.format * attrInfo.channelCount *
        BUFFER_CALC_20MS / BUFFER_CALC_1000MS;
    std::string attrInfoStr = "rate=" + std::to_string(attrInfo.sampleRate) + " format=" +
        ParseAudioFormatToStr(attrInfo.format) + " channels=" + std::to_string(attrInfo.channelCount) +
        " buffer_size=" + std::to_string(bufferSize);
    AUDIO_INFO_LOG("attrInfoStr: %{public}s", attrInfoStr.c_str());
    return attrInfoStr;
}

int32_t AudioRenderSink::UpdateDPAttr(const std::string &dpInfo)
{
    CHECK_AND_RETURN_RET_LOG(!dpInfo.empty(), ERR_INVALID_PARAM, "dp info is empty");

    auto sinkRate_begin = dpInfo.find("rate=");
    auto sinkRate_end = dpInfo.find_first_of(" ", sinkRate_begin);
    std::string sampleRateStr = dpInfo.substr(sinkRate_begin + std::strlen("rate="),
        sinkRate_end - sinkRate_begin - std::strlen("rate="));

    auto sinkBuffer_begin = dpInfo.find("buffer_size=");
    auto sinkBuffer_end = dpInfo.find_first_of(" ", sinkBuffer_begin);
    std::string bufferSizeStr = dpInfo.substr(sinkBuffer_begin + std::strlen("buffer_size="),
        sinkBuffer_end - sinkBuffer_begin - std::strlen("buffer_size="));

    auto sinkChannel_begin = dpInfo.find("channels=");
    auto sinkChannel_end = dpInfo.find_first_of(" ", sinkChannel_begin);
    std::string channelStr = dpInfo.substr(sinkChannel_begin + std::strlen("channels="),
        sinkChannel_end - sinkChannel_begin - std::strlen("channels="));

    auto address_begin = dpInfo.find("address=");
    auto address_end = dpInfo.find_first_of(" ", address_begin);
    std::string addressStr = dpInfo.substr(address_begin + std::strlen("address="),
        address_end - address_begin - std::strlen("address="));

    bool ret = StringConverter(sampleRateStr, attr_.sampleRate);
    CHECK_AND_RETURN_RET_LOG(ret, ERR_INVALID_PARAM, "convert fail, sampleRate: %{public}s", sampleRateStr.c_str());
    ret = StringConverter(channelStr, attr_.channel);
    CHECK_AND_RETURN_RET_LOG(ret, ERR_INVALID_PARAM, "convert fail, channel: %{public}s", channelStr.c_str());

    attr_.address = addressStr;
    uint32_t formatByte = 0;
    if (attr_.channel <= 0 || attr_.sampleRate <= 0 || bufferSizeStr.empty()) {
        AUDIO_ERR_LOG("check attr fail, channel: %{public}d, sampleRate: %{public}d", attr_.channel, attr_.sampleRate);
    } else {
        uint32_t bufferSize = 0;
        ret = StringConverter(bufferSizeStr, bufferSize);
        CHECK_AND_RETURN_RET_LOG(ret, ERR_INVALID_PARAM, "convert fail, bufferSize: %{public}s", bufferSizeStr.c_str());
        formatByte = bufferSize * BUFFER_CALC_1000MS / BUFFER_CALC_20MS / attr_.channel / attr_.sampleRate;
    }

    attr_.format = static_cast<AudioSampleFormat>(ConvertByteToAudioFormat(formatByte));

    AUDIO_DEBUG_LOG("sampleRate: %{public}d, format: %{public}d, channelCount: %{public}d, address: %{public}s",
        attr_.sampleRate, attr_.format, attr_.channel, addressStr.c_str());

    adapterNameCase_ = "dp";
    openSpeaker_ = 0;
    return SUCCESS;
}

std::string AudioRenderSink::GetDPDeviceInfo(const std::string &condition)
{
    int32_t ret = UpdateDPAttr(condition);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, "", "init attr fail");

    ret = InitRender();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS && audioRender_ != nullptr, "", "init render fail");

    struct AudioSampleAttributes attrInfo = {};
    ret = audioRender_->GetSampleAttributes(audioRender_, &attrInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, "", "get sample attr fail");

    AUDIO_DEBUG_LOG("sampleRate: %{public}d, format: %{public}d, channelCount: %{public}d, size: %{public}d",
        attrInfo.sampleRate, attrInfo.format, attrInfo.channelCount, attrInfo.frameSize);
    return GetAttrInfoStr(attrInfo);
}

std::string AudioRenderSink::GetRouting(void) const
{
    switch (currentActiveDevice_) {
        case DEVICE_TYPE_EARPIECE:
            return "1";
        case DEVICE_TYPE_SPEAKER:
            return "2";
        case DEVICE_TYPE_WIRED_HEADSET:
            return "4";
        case DEVICE_TYPE_USB_ARM_HEADSET:
            return "67108864";
        case DEVICE_TYPE_USB_HEADSET:
            return "545259520";
        case DEVICE_TYPE_BLUETOOTH_SCO:
            return "16";
        case DEVICE_TYPE_BLUETOOTH_A2DP:
            return "128";
        default:
            break;
    }
    return "0";
}

void AudioRenderSink::WriteSmartPAStatusSysEvent(int32_t status)
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::SMARTPA_STATUS,
        Media::MediaMonitor::BEHAVIOR_EVENT);
    bean->Add("STATUS", status);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

// must be called with sinkMutex_ held
void AudioRenderSink::UpdateSinkState(bool started)
{
    callback_.OnRenderSinkStateChange(GetUniqueId(), started);
}

int32_t AudioRenderSink::UpdatePrimaryConnectionState(uint32_t operation)
{
    if (operation == DATA_LINK_CONNECTING) {
        AUDIO_INFO_LOG("Primary sink is connecting");
        isDataLinkConnected_ = false;
    }
    if (operation == DATA_LINK_CONNECTED) {
        AUDIO_INFO_LOG("Primary sink is connected");
        isDataLinkConnected_ = true;
        dataConnectionCV_.notify_all();
    }
    return SUCCESS;
}

void AudioRenderSink::SetDmDeviceType(uint16_t dmDeviceType, DeviceType deviceType)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    const auto &it = dmDeviceTypeMap_.find(deviceType);
    bool isDmDeviceTypeUpdated = it == dmDeviceTypeMap_.end() || it->second != dmDeviceType;
    dmDeviceTypeMap_[deviceType] = dmDeviceType;
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_LOG(deviceManager != nullptr, "deviceManager is nullptr");
    deviceManager->SetDmDeviceType(dmDeviceType, deviceType);

    CHECK_AND_RETURN(isDmDeviceTypeUpdated);
    std::vector<DeviceType> outputDevices;
    outputDevices.push_back(currentActiveDevice_);
    AUDIO_INFO_LOG("dm deviceType update, need update output port pin");
    int32_t ret = DoSetOutputRoute(outputDevices);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "DoSetOutputRoute fails");
}

void AudioRenderSink::WaitForDataLinkConnected()
{
    std::unique_lock<std::mutex> dataConnectionWaitLock(dataConnectionMutex_);
    if (!isDataLinkConnected_ && (halName_ == "primary") && (sinkType_ == ADAPTER_TYPE_PRIMARY)) {
        AUDIO_INFO_LOG("data-connection blocking starts");
        bool stopWaiting = dataConnectionCV_.wait_for(
            dataConnectionWaitLock, std::chrono::milliseconds(DATA_CONNECTION_TIMEOUT_IN_MS), [this] {
                return isDataLinkConnected_;
            });
        if (stopWaiting) {
            AUDIO_INFO_LOG("data-connection blocking ends");
        } else {
            AUDIO_WARNING_LOG("data-connection time out, start RenderFrame anyway.");
        }
        isDataLinkConnected_ = true;
    }
    dataConnectionWaitLock.unlock();
}

} // namespace AudioStandard
} // namespace OHOS
