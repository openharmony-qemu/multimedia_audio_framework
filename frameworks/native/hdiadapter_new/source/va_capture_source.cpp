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
#define LOG_TAG "VACaptureSource"
#endif

#include "source/va_capture_source.h"

#include <climits>
#include <future>
#include "parameters.h"
#include "audio_hdi_log.h"
#include "audio_log.h"
#include "audio_dump_pcm.h"

#include "manager/hdi_adapter_manager.h"

#include "hisysevent.h"

#include "iservice_registry.h"
#include "iaudio_policy.h"

namespace OHOS {
namespace AudioStandard {

VACaptureSource::VACaptureSource(const uint32_t captureId) : captureId_(captureId)
{
    audioSrcClock_ = std::make_shared<AudioCapturerSourceClock>();
    CapturerClockManager::GetInstance().RegisterAudioSourceClock(captureId, audioSrcClock_);
}

VACaptureSource::~VACaptureSource()
{
    if (sourceInited_) {
        DeInit();
    }
    DumpFileUtil::CloseDumpFile(&dumpFile_);
    CapturerClockManager::GetInstance().DeleteAudioSourceClock(captureId_);
}

int32_t VACaptureSource::Init(const IAudioSourceAttr &attr)
{
    AUDIO_INFO_LOG("VACaptureSource::Init called");
    std::lock_guard<std::mutex> lock(statusMutex_);
    attr_ = attr;
    CHECK_AND_RETURN_RET_LOG(!sourceInited_.load(), SUCCESS, "va source already inited");
    logMode_ = system::GetIntParameter("persist.multimedia.audiolog.switch", 0);

    int ret = CreateCapture();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "va create capture failed");
    ret = InitOperator();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "va init operator failed");

    sourceInited_.store(true);

    if (audioSrcClock_ != nullptr) {
        audioSrcClock_->Init(attr.sampleRate, attr.format, attr.channel);
    }
    return SUCCESS;
}

int32_t VACaptureSource::CreateCapture()
{
    Trace trace("VACaptureSource::CreateCapture");
    std::string macAddress = attr_.macAddress;

    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_RET_LOG(samgr != nullptr, ERR_OPERATION_FAILED, "failed to obtain system ability manager");
    
    static int32_t systemAbilityId = 3009;
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(systemAbilityId);
    CHECK_AND_RETURN_RET_LOG(remoteObject != nullptr, ERR_OPERATION_FAILED, "policy service unavailable");
    
    sptr<IAudioPolicy> audioProxy = iface_cast<IAudioPolicy>(remoteObject);
    CHECK_AND_RETURN_RET_LOG(audioProxy != nullptr, ERR_OPERATION_FAILED, "audioProxy is null");
    
    sptr<IRemoteObject> controllerRemote;
    audioProxy->GetVADeviceController(macAddress, controllerRemote);
    CHECK_AND_RETURN_RET_LOG(controllerRemote != nullptr, ERR_OPERATION_FAILED, "get controller failed");
    
    deviceController_ = iface_cast<IVADeviceController>(controllerRemote);
    CHECK_AND_RETURN_RET_LOG(deviceController_ != nullptr, ERR_OPERATION_FAILED, "convert failed");
    
    std::shared_ptr<VAAudioStreamProperty> prop = MakeVAStreamPropertyFromIAudioSourceAttr();
    std::shared_ptr<VAInputStreamAttribute> attribute = MakeVAStreamAttributeFromIAudioSourceAttr();
    sptr<IRemoteObject> inputStreamRemote;
    int ret = deviceController_->OpenInputStream(*prop, *attribute, inputStreamRemote);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "openInputStream failed ret = %{public}d", ret);
    
    CHECK_AND_RETURN_RET_LOG(inputStreamRemote != nullptr, ERR_OPERATION_FAILED, "inputStreamRemote is nullptr");
    inputStream_ = iface_cast<IVAInputStream>(inputStreamRemote);
    CHECK_AND_RETURN_RET_LOG(inputStream_ != nullptr, ERR_OPERATION_FAILED, "inputStream_ is nullptr");
    
    return SUCCESS;
}

std::shared_ptr<VAAudioStreamProperty> VACaptureSource::MakeVAStreamPropertyFromIAudioSourceAttr()
{
    std::shared_ptr<VAAudioStreamProperty> streamProp = std::make_shared<VAAudioStreamProperty>();
    streamProp->sampleRate_ = attr_.sampleRate;
    streamProp->sampleFormat_ = attr_.format;
    streamProp->channelLayout_ = static_cast<AudioChannelLayout>(attr_.channelLayout);
    return streamProp;
}

std::shared_ptr<VAInputStreamAttribute> VACaptureSource::MakeVAStreamAttributeFromIAudioSourceAttr()
{
    std::shared_ptr<VAInputStreamAttribute> streamAttr = std::make_shared<VAInputStreamAttribute>();
    streamAttr->type = static_cast<SourceType>(attr_.sourceType);
    return streamAttr;
}

int32_t VACaptureSource::InitOperator()
{
    CHECK_AND_RETURN_RET_LOG(inputStream_ != nullptr, ERR_OPERATION_FAILED, "input stream is nullptr");
    uint32_t bufferCapacity = attr_.bufferSize * 2;
    std::shared_ptr<VASharedBuffer> vaBuffer = VASharedBuffer::CreateFromLocal(bufferCapacity);
    CHECK_AND_RETURN_RET_LOG(vaBuffer != nullptr, ERR_OPERATION_FAILED, "vaBuffer is null");
    bufferOperator_ = std::make_shared<VASharedBufferOperator>(*vaBuffer);
    CHECK_AND_RETURN_RET_LOG(bufferOperator_ != nullptr, ERR_OPERATION_FAILED, "buffer operator is null");
    bufferOperator_->SetMinReadSize(attr_.bufferSize);

    VASharedMemInfo memInfo;
    vaBuffer->GetVASharedMemInfo(memInfo);
    int ret = inputStream_->RequestSharedMem(memInfo);
    return ret;
}

void VACaptureSource::DeInit(void)
{
    AUDIO_INFO_LOG("VACaptureSource::DeInit called");

    Trace trace("VACaptureSource::DeInit");
    CHECK_AND_RETURN_LOG(sourceInited_.load(), "sourceInited_ is false");
    CHECK_AND_RETURN_LOG(inputStream_ != nullptr, "input stream is null");

    if (started_.load()) {
        AUDIO_WARNING_LOG("va capture source is still started.");
        Stop();
    }

    std::lock_guard<std::mutex> lock(statusMutex_);
    inputStream_->Close();

    sourceInited_.store(false);
    started_.store(false);
    inputStream_ = nullptr;
    bufferOperator_ = nullptr;
}

bool VACaptureSource::IsInited(void)
{
    return sourceInited_.load();
}

int32_t VACaptureSource::Start(void)
{
    AUDIO_INFO_LOG("VACaptureSource::Start called");
    CHECK_AND_RETURN_RET_LOG(bufferOperator_ != nullptr && inputStream_ != nullptr, ERR_OPERATION_FAILED, "BadStatus");
    std::lock_guard<std::mutex> lock(statusMutex_);

    dumpFileName_ = "_va_source_" + std::to_string(attr_.sourceType) + "_" + GetTime() + "_" +
                    std::to_string(attr_.sampleRate) + "_" + std::to_string(attr_.channel) + "_" +
                    std::to_string(attr_.format) + ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileName_, &dumpFile_);

    CHECK_AND_RETURN_RET_LOG(!started_.load(), ERR_OPERATION_FAILED, "va capture source already started.");

    bufferOperator_->Reset();

    if (audioSrcClock_ != nullptr) {
        audioSrcClock_->Reset();
    }
    callback_.OnCaptureState(true);
    int ret = inputStream_->Start();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "va start failed");
    started_.store(true);

    startTimestamp = ClockTime::GetCurNano();
    return SUCCESS;
}

int32_t VACaptureSource::Stop(void)
{
    Trace trace("VACaptureSource::Stop");
    AUDIO_INFO_LOG("VACaptureSource::Stop called");
    std::lock_guard<std::mutex> lock(statusMutex_);
    CHECK_AND_RETURN_RET_LOG(inputStream_ != nullptr, ERR_OPERATION_FAILED, "input stream is null");
    CHECK_AND_RETURN_RET_LOG(started_.load(), SUCCESS, "already stopped");
    
    int ret = inputStream_->Stop();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "stop fail");
    started_.store(false);
    callback_.OnCaptureState(false);
    int64_t stopTimestamp = ClockTime::GetCurNano();
    PrintUsageTimeDfx((stopTimestamp - startTimestamp) / AUDIO_NS_PER_SECOND);
    return SUCCESS;
}

int32_t VACaptureSource::Resume(void)
{
    return SUCCESS;
}

int32_t VACaptureSource::Pause(void)
{
    return SUCCESS;
}

int32_t VACaptureSource::Flush(void)
{
    return SUCCESS;
}

int32_t VACaptureSource::Reset(void)
{
    return SUCCESS;
}

int32_t VACaptureSource::CaptureFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes)
{
    CHECK_AND_RETURN_RET_LOG(bufferOperator_ != nullptr, ERR_OPERATION_FAILED, "buffer operator is null");
    CHECK_AND_RETURN_RET_LOG(inputStream_ != nullptr, ERR_OPERATION_FAILED, "input stream is null");

    int64_t stamp = ClockTime::GetCurNano();
    replyBytes = bufferOperator_->Read(reinterpret_cast<uint8_t *>(frame), requestBytes);
    if (AudioDump::GetInstance().GetVersionType() == DumpFileUtil::BETA_VERSION) {
        DumpFileUtil::WriteDumpFile(dumpFile_, frame, replyBytes);
        AudioCacheMgr::GetInstance().CacheData(dumpFileName_, static_cast<void *>(frame), replyBytes);
    }
    CheckUpdateState(frame, requestBytes);
    stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
    if (logMode_) {
        AUDIO_WARNING_LOG("len:[%{public}" PRIu64 "],cost:[%{public}" PRId64 "]ms", requestBytes, stamp);
    }
    return SUCCESS;
}

int32_t VACaptureSource::CaptureFrameWithEc(
    FrameDesc *fdesc, uint64_t &replyBytes, FrameDesc *fdescEc, uint64_t &replyBytesEc)
{
    AUDIO_INFO_LOG("not supported");
    return SUCCESS;
}

std::string VACaptureSource::GetAudioParameter(const AudioParamKey key, const std::string &condition)
{
    CHECK_AND_RETURN_RET_LOG(deviceController_ != nullptr, "", "device controller is null");
    std::string value;
    deviceController_->GetParameters(std::to_string(static_cast<int>(key)), value);
    AUDIO_INFO_LOG("get audio parameter from va device: {\"key\":\"%{public}d\", \"value\":\"%{public}s\"}",
        key, value.c_str());
    return value;
}

void VACaptureSource::SetAudioParameter(const AudioParamKey key, const std::string &condition, const std::string &value)
{
    AUDIO_INFO_LOG("set audio parameter for va device: {\"key\":\"%{public}d\", \"value\":\"%{public}s\"}",
        key, value.c_str());
    CHECK_AND_RETURN_LOG(deviceController_ != nullptr, "device controller is null");
    deviceController_->SetParameters(std::to_string(static_cast<int>(key)), value);
}

void VACaptureSource::PrintUsageTimeDfx(int64_t useTime)
{
    auto ret = HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::AUDIO, "AUDIO_DEVICE_UTILIZATION_STATS",
        HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "DEVICE_TYPE", DeviceType::DEVICE_TYPE_BT_SPP,
        "IS_PLAYBACK", false,
        "STREAM_TYPE", AudioStreamType::STREAM_RECORDING,
        "DURATION", (int32_t)useTime);
    if (ret) {
        AUDIO_ERR_LOG("write event fail: AUDIO_DEVICE_UTILIZATION_STATS, ret = %{public}d", ret);
    }
}

int32_t VACaptureSource::SetVolume(float left, float right)
{
    return SUCCESS;
}
int32_t VACaptureSource::GetVolume(float &left, float &right)
{
    return SUCCESS;
}
int32_t VACaptureSource::SetMute(bool isMute)
{
    return SUCCESS;
}
int32_t VACaptureSource::GetMute(bool &isMute)
{
    return SUCCESS;
}

uint64_t VACaptureSource::GetTransactionId(void)
{
    return SUCCESS;
}

int32_t VACaptureSource::GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec)
{
    return SUCCESS;
}

float VACaptureSource::GetMaxAmplitude(void)
{
    lastGetMaxAmplitudeTime_ = ClockTime::GetCurNano();
    startUpdate_ = true;
    return maxAmplitude_;
}

int32_t VACaptureSource::SetAudioScene(AudioScene audioScene, bool scoExcludeFlag)
{
    return SUCCESS;
}

int32_t VACaptureSource::UpdateActiveDevice(DeviceType inputDevice)
{
    return SUCCESS;
}

void VACaptureSource::CheckUpdateState(char *frame, size_t replyBytes)
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

void VACaptureSource::RegistCallback(uint32_t type, IAudioSourceCallback *callback)
{
    callback_.RegistCallback(type, callback);
}

void VACaptureSource::RegistCallback(uint32_t type, std::shared_ptr<IAudioSourceCallback> callback)
{
    callback_.RegistCallback(type, callback);
}

int32_t VACaptureSource::UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE], const size_t size)
{
    return SUCCESS;
}
int32_t VACaptureSource::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
    return SUCCESS;
}

void VACaptureSource::SetAddress(const std::string &address)
{}
void VACaptureSource::SetInvalidState(void)
{}

void VACaptureSource::DumpInfo(std::string &dumpString)
{
    dumpString += "type: VASource\tstarted: " + std::string(started_.load() ? "true" : "false") + "\n";
}

void VACaptureSource::SetDmDeviceType(uint16_t dmDeviceType, DeviceType deviceType)
{}

} // namespace AudioStandard
} // namespace OHOS
