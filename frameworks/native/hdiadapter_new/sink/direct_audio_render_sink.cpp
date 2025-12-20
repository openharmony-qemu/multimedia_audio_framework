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
#define LOG_TAG "DirectAudioRenderSink"
#endif

#include "sink/direct_audio_render_sink.h"
#include <climits>
#include <future>
#include "audio_hdi_log.h"
#include "audio_errors.h"
#include "audio_dump_pcm.h"
#include "volume_tools.h"
#include "parameters.h"
#include "media_monitor_manager.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"

namespace OHOS {
namespace AudioStandard {
DirectAudioRenderSink::~DirectAudioRenderSink()
{
    std::unique_lock<std::mutex> lock(sinkMutex_);
    if (started_) {
        started_ = false;
    }
    lock.unlock();
    if (testThread_.joinable()) {
        testThread_.join();
    }
}
 
int32_t DirectAudioRenderSink::Init(const IAudioSinkAttr &attr)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    Trace trace("DirectAudioRenderSink::Init");
    attr_ = attr;
    testFlag_ = system::GetIntParameter("persist.multimedia.eac3test", 0);
    if (!testFlag_) {
        int32_t ret = CreateRender();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "create render fail");
    }
    sinkInited_ = true;
    return SUCCESS;
}

void DirectAudioRenderSink::DeInit(void)
{
    AUDIO_INFO_LOG("in");
    Trace trace("DirectAudioRenderSink::DeInit");
    std::lock_guard<std::mutex> lock(sinkMutex_);
    sinkInited_ = false;
    started_ = false;
    if (!testFlag_) {
        HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
        std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
        CHECK_AND_RETURN(deviceManager != nullptr);
        deviceManager->DestroyRender(attr_.adapterName, hdiRenderId_);
        audioRender_ = nullptr;
    }
    hdiCallback_ = {};
    DumpFileUtil::CloseDumpFile(&dumpFile_);
}

bool DirectAudioRenderSink::IsInited(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    return sinkInited_;
}

int32_t DirectAudioRenderSink::Start(void)
{
    std::unique_lock<std::mutex> lock(sinkMutex_);
    AUDIO_INFO_LOG("in");
    Trace trace("DirectAudioRenderSink::Start");
    if (started_) {
        return SUCCESS;
    }
    dumpFileName_ = EncodingTypeStr(attr_.encodingType) + "_" + GetTime() + "_" + std::to_string(attr_.sampleRate)
        + "_" + std::to_string(attr_.channel) + "_" + std::to_string(attr_.format) + ".not.pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileName_, &dumpFile_);
    if (testFlag_) {
        started_ = true;
        lock.unlock();
        return SUCCESS;
    }
    AudioXCollie audioXCollie("DirectAudioRenderSink::Start", TIMEOUT_SECONDS_10,
        nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    int32_t ret = audioRender_->Start(audioRender_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "start fail, ret: %{public}d", ret);
    started_ = true;
    return SUCCESS;
}

void DirectAudioRenderSink::StartTestThread(void)
{
    testThread_ = std::thread([this]() {
        bool keepRunning = true;
        while (keepRunning) {
            std::unique_lock<std::mutex> lock(sinkMutex_);
            keepRunning = started_ && sinkInited_;
            hdiCallback_.serviceCallback_(CB_NONBLOCK_WRITE_COMPLETED);
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(TEST_CALLBACK_TIME));
        }
    });
}
 
int32_t DirectAudioRenderSink::Stop(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    AUDIO_INFO_LOG("in");
    Trace trace("DirectAudioRenderSink::Stop");
    if (!started_) {
        return SUCCESS;
    }
    if (!testFlag_) {
#ifdef FEATURE_POWER_MANAGER
        if (runningLock_ != nullptr) {
            std::thread runningLockThread([this] {
                runningLock_->UnLock();
            });
            runningLockThread.join();
        }
#endif
        AudioXCollie audioXCollie("DirectAudioRenderSink::Stop", TIMEOUT_SECONDS_10,
            nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
        CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
        int32_t ret = audioRender_->Stop(audioRender_);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "stop fail");
    }
    started_ = false;
    return SUCCESS;
}

int32_t DirectAudioRenderSink::Resume(void)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t DirectAudioRenderSink::Pause(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    AudioXCollie audioXCollie("DirectAudioRenderSink::Pause", TIMEOUT_SECONDS_10, nullptr, nullptr,
        AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    int32_t ret = audioRender_->Pause(audioRender_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "Pause fail:%{public}d", ret);
    started_ = false;
    return SUCCESS;
}

int32_t DirectAudioRenderSink::Flush(void)
{
    AUDIO_INFO_LOG("flush");
    std::lock_guard<std::mutex> lock(sinkMutex_);
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");

    int32_t ret = audioRender_->Flush(audioRender_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "flush fail:%{public}d", ret);
    return SUCCESS;
}

int32_t DirectAudioRenderSink::Reset(void)
{
    return SUCCESS;
}

int32_t DirectAudioRenderSink::RenderFrame(char &data, uint64_t len, uint64_t &writeLen)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    if (testFlag_) {
        Trace trace("DirectAudioRenderSink::MockRenderFrame");
        int8_t *ptr = reinterpret_cast<int8_t *>(&data) + sizeof(HWDecodingInfo);
        CHECK_AND_RETURN_RET_LOG(len > sizeof(HWDecodingInfo), ERR_OPERATION_FAILED, "mock write failed");
        size_t dataLen = len - sizeof(HWDecodingInfo);
        DumpFileUtil::WriteDumpFile(dumpFile_, static_cast<void *>(ptr), dataLen);
        uint64_t temp = *reinterpret_cast<uint64_t *>(&data);
        CHECK_AND_RETURN_RET_LOG(temp >= mockPts_, ERR_OPERATION_FAILED, "mock write failed");
        usleep(temp - mockPts_);
        mockPts_ = temp;
        return SUCCESS;
    } else {
        int64_t stamp = ClockTime::GetCurNano();
        CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
        CHECK_AND_RETURN_RET_LOG(started_, ERR_OPERATION_FAILED, "not start, invalid state");
    
        Trace trace("DirectAudioRenderSink::RenderFrame");
        int32_t ret = audioRender_->RenderFrame(audioRender_, reinterpret_cast<int8_t *>(&data),
            static_cast<uint32_t>(len), &writeLen);
#ifdef FEATURE_POWER_MANAGER
        if (runningLock_) {
            runningLock_->UpdateAppsUidToPowerMgr();
        }
#endif
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_WRITE_FAILED, "fail, ret: %{public}x", ret);
        if (writeLen != 0) {
            // EAC3 format is not supported to count volume, stream info in not needed.
            if (AudioDump::GetInstance().GetVersionType() == DumpFileUtil::BETA_VERSION) {
                DumpFileUtil::WriteDumpFile(dumpFile_, static_cast<void *>(&data), writeLen);
                AudioCacheMgr::GetInstance().CacheData(dumpFileName_, static_cast<void *>(&data), writeLen);
            }
        }
        stamp = (ClockTime::GetCurNano() - stamp) / AUDIO_US_PER_SECOND;
    }
    return SUCCESS;
}

int64_t DirectAudioRenderSink::GetVolumeDataCount()
{
    AUDIO_WARNING_LOG("not supported");
    return 0;
}

void DirectAudioRenderSink::SetSpeed(float speed)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    CHECK_AND_RETURN_LOG(audioRender_ != nullptr, "render is nullptr");
    int32_t ret = audioRender_->SetRenderSpeed(audioRender_, speed);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "failed:%{public}d", ret);
}

int32_t DirectAudioRenderSink::SetVolume(float left, float right)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "render is nullptr");
    float half = 2.0f;
    float volume = (left + right) / half;
    int32_t ret = audioRender_->SetVolume(audioRender_, volume);
    return ret;
}

int32_t DirectAudioRenderSink::GetVolume(float &left, float &right)
{
    left = leftVolume_;
    right = rightVolume_;
    return SUCCESS;
}

int32_t DirectAudioRenderSink::GetLatency(uint32_t &latency)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t DirectAudioRenderSink::GetTransactionId(uint64_t &transactionId)
{
    transactionId = GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_HWDECODING);
    return SUCCESS;
}

int32_t DirectAudioRenderSink::GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
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

float DirectAudioRenderSink::GetMaxAmplitude(void)
{
    return 0.0f;
}

void DirectAudioRenderSink::SetAudioMonoState(bool audioMono)
{
}

void DirectAudioRenderSink::SetAudioBalanceValue(float audioBalance)
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

int32_t DirectAudioRenderSink::SetSinkMuteForSwitchDevice(bool mute)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t DirectAudioRenderSink::UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size)
{
#ifdef FEATURE_POWER_MANAGER
    CHECK_AND_RETURN_RET_LOG(runningLock_, ERR_INVALID_HANDLE, "running lock is nullptr");
    runningLock_->UpdateAppsUid(appsUid, appsUid + size);
#endif
    return SUCCESS;
}

int32_t DirectAudioRenderSink::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
#ifdef FEATURE_POWER_MANAGER
    CHECK_AND_RETURN_RET_LOG(runningLock_, ERR_INVALID_HANDLE, "running lock is nullptr");
    runningLock_->UpdateAppsUid(appsUid.cbegin(), appsUid.cend());
    runningLock_->UpdateAppsUidToPowerMgr();
#endif
    return SUCCESS;
}

int32_t DirectAudioRenderSink::RegistDirectHdiCallback(std::function<void(const RenderCallbackType type)> callback)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    AUDIO_INFO_LOG("in");
    int32_t ret = SUCCESS;
    hdiCallback_ = {
        .callback_.RenderCallback = &DirectAudioRenderSink::DirectRenderCallback,
        .serviceCallback_ = callback,
        .sink_ = this,
    };
    if (!testFlag_) {
        ret = audioRender_->RegCallback(audioRender_, &hdiCallback_.callback_, (int8_t)0);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("fail, error code: %{public}d", ret);
        }
    }
    return ret;
}

void DirectAudioRenderSink::DumpInfo(std::string &dumpString)
{
    dumpString += "type: directSink\tstarted: " + std::string(started_ ? "true" : "false") + "\n";
}

int32_t DirectAudioRenderSink::DirectRenderCallback(struct IAudioCallback *self, enum AudioCallbackType type,
    int8_t *reserved, int8_t *cookie)
{
    (void)reserved;
    (void)cookie;
    auto *impl = reinterpret_cast<struct DirectHdiCallback *>(self);
    CHECK_AND_RETURN_RET_LOG(impl != nullptr, ERR_OPERATION_FAILED, "impl is nullptr");
    auto *sink = reinterpret_cast<DirectAudioRenderSink *>(impl->sink_);
    CHECK_AND_RETURN_RET_LOG(sink != nullptr, ERR_OPERATION_FAILED, "sink is nullptr");
    if (!sink->started_) {
        AUDIO_DEBUG_LOG("invalid call, started: %{public}d", sink->started_);
        return SUCCESS;
    }

    impl->serviceCallback_(static_cast<RenderCallbackType>(type));
    return SUCCESS;
}

void DirectAudioRenderSink::InitAudioSampleAttr(struct AudioSampleAttributes &param)
{
    param.format = AUDIO_FORMAT_TYPE_EAC3; // in plan: add type switcher
    param.channelCount = attr_.channel;
    param.sampleRate = AUDIO_SAMPLE_RATE_48K;
    param.interleaved = true;
    param.streamId = static_cast<int32_t>(GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_HWDECODING));
    param.type = AUDIO_DP;
    param.period = DEEP_BUFFER_RENDER_PERIOD_SIZE;
    param.isBigEndian = false;
    param.isSignedData = true;
    param.stopThreshold = INT_MAX;
    param.silenceThreshold = 0;
    param.sampleRate = attr_.sampleRate;
    param.channelLayout = attr_.channelLayout;
}

void DirectAudioRenderSink::InitDeviceDesc(struct AudioDeviceDescriptor &deviceDesc)
{
    deviceDesc.pins = PIN_OUT_DP;
    deviceDesc.desc = const_cast<char *>("");
}

int32_t DirectAudioRenderSink::CreateRender(void)
{
    Trace trace("DirectAudioRenderSink::CreateRender");

    struct AudioSampleAttributes param;
    struct AudioDeviceDescriptor deviceDesc;
    InitAudioSampleAttr(param);
    InitDeviceDesc(deviceDesc);

    AUDIO_INFO_LOG("create render, rate: %{public}u, channel: %{public}u, format: %{public}u", param.sampleRate,
        param.channelCount, param.format);
    AudioXCollie audioXCollie("DirectAudioRenderSink::CreateRender", TIMEOUT_SECONDS_10,
         nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET(deviceManager != nullptr, ERR_INVALID_HANDLE);
    void *render = deviceManager->CreateRender(attr_.adapterName, &param, &deviceDesc, hdiRenderId_);
    audioRender_ = static_cast<struct IAudioRender *>(render);
    CHECK_AND_RETURN_RET(audioRender_ != nullptr, ERR_NOT_STARTED);

    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
