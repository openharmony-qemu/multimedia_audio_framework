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
#define LOG_TAG "CabinAudioRenderSink"
#endif


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
#include "sink/cabin_audio_render_sink.h"

namespace OHOS {
namespace AudioStandard {

namespace {
    static const int64_t SLEEP_MS = 20000;
}

CabinAudioRenderSink::~CabinAudioRenderSink()
{
    std::unique_lock<std::mutex> lock(sinkMutex_);
    if (started_) {
        started_ = false;
    }
}
 
int32_t CabinAudioRenderSink::Init(const IAudioSinkAttr &attr)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    Trace trace("CabinAudioRenderSink::Init");
    attr_ = attr;
    GetSysPara("persist.multimedia.3dadirecttest", direct3DATestFlag);
    if (direct3DATestFlag) {
        int32_t ret = CreateRender();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "create render fail");
    }
    sinkInited_ = true;
    return SUCCESS;
}

void CabinAudioRenderSink::DeInit(void)
{
    AUDIO_INFO_LOG("in");
    Trace trace("CabinAudioRenderSink::DeInit");
    std::lock_guard<std::mutex> lock(sinkMutex_);
    sinkInited_ = false;
    started_ = false;
    if (!direct3DATestFlag) {
        HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
        std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
        CHECK_AND_RETURN(deviceManager != nullptr);
        deviceManager->DestroyRender(attr_.adapterName, hdiRenderId_);
        audioRender_ = nullptr;
    }
    hdiCallback_ = {};
    DumpFileUtil::CloseDumpFile(&dumpFile_);
}

bool CabinAudioRenderSink::IsInited(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    return sinkInited_;
}

int32_t CabinAudioRenderSink::Start(void)
{
    std::unique_lock<std::mutex> lock(sinkMutex_);
    AUDIO_INFO_LOG("in");
    Trace trace("CabinAudioRenderSink::Start");
    if (started_) {
        return SUCCESS;
    }
    dumpFileName_ = EncodingTypeStr(attr_.encodingType) + "_" + GetTime() + "_" + std::to_string(attr_.sampleRate)
        + "_" + std::to_string(attr_.channel) + "_" + std::to_string(attr_.format) + ".not.pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileName_, &dumpFile_);
    if (direct3DATestFlag) {
        started_ = true;
        lock.unlock();
        return SUCCESS;
    }
    AudioXCollie audioXCollie("CabinAudioRenderSink::Start", TIMEOUT_SECONDS_10,
        nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    started_ = true;
    return SUCCESS;
}


int32_t CabinAudioRenderSink::Stop(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    AUDIO_INFO_LOG("in");
    Trace trace("CabinAudioRenderSink::Stop");
    if (!started_) {
        return SUCCESS;
    }
    if (!direct3DATestFlag) {
        AudioXCollie audioXCollie("CabinAudioRenderSink::Stop", TIMEOUT_SECONDS_10,
            nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    }
    started_ = false;
    return SUCCESS;
}

int32_t CabinAudioRenderSink::Resume(void)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t CabinAudioRenderSink::Pause(void)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    AudioXCollie audioXCollie("CabinAudioRenderSink::Pause", TIMEOUT_SECONDS_10, nullptr, nullptr,
        AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    started_ = false;
    return SUCCESS;
}

int32_t CabinAudioRenderSink::Flush(void)
{
    AUDIO_INFO_LOG("flush");
    std::lock_guard<std::mutex> lock(sinkMutex_);
    return SUCCESS;
}

int32_t CabinAudioRenderSink::Reset(void)
{
    return SUCCESS;
}

int32_t CabinAudioRenderSink::RenderFrame(char &data, uint64_t len, uint64_t &writeLen)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    if (direct3DATestFlag) {
        AUDIO_DEBUG_LOG("CabinAudioRenderSink::mockRenderFrame");
        int8_t *ptr = reinterpret_cast<int8_t *>(&data);
        DumpFileUtil::WriteDumpFile(dumpFile_, static_cast<void *>(ptr), len);
        return SUCCESS;
    } else {
        usleep(SLEEP_MS);
    }
    return SUCCESS;
}

int64_t CabinAudioRenderSink::GetVolumeDataCount()
{
    AUDIO_WARNING_LOG("not supported");
    return ERR_NOT_SUPPORTED;
}

void CabinAudioRenderSink::SetSpeed(float speed)
{
    std::lock_guard<std::mutex> lock(sinkMutex_);
    return;
}

int32_t CabinAudioRenderSink::SetVolume(float left, float right)
{
    return 0;
}

int32_t CabinAudioRenderSink::GetVolume(float &left, float &right)
{
    left = leftVolume_;
    right = rightVolume_;
    return SUCCESS;
}

int32_t CabinAudioRenderSink::GetLatency(uint32_t &latency)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t CabinAudioRenderSink::GetTransactionId(uint64_t &transactionId)
{
    transactionId = GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_AUDIO_VIVID_3DA_DIRECT);
    return SUCCESS;
}

int32_t CabinAudioRenderSink::GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec)
{
    return 0;
}

float CabinAudioRenderSink::GetMaxAmplitude(void)
{
    return 0.0f;
}

void CabinAudioRenderSink::SetAudioMonoState(bool audioMono)
{
}

void CabinAudioRenderSink::SetAudioBalanceValue(float audioBalance)
{
    return;
}

int32_t CabinAudioRenderSink::SetSinkMuteForSwitchDevice(bool mute)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t CabinAudioRenderSink::UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size)
{
#ifdef FEATURE_POWER_MANAGER
    CHECK_AND_RETURN_RET_LOG(runningLock_, ERR_INVALID_HANDLE, "running lock is nullptr");
    runningLock_->UpdateAppsUid(appsUid, appsUid + size);
#endif
    return SUCCESS;
}

int32_t CabinAudioRenderSink::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
#ifdef FEATURE_POWER_MANAGER
    CHECK_AND_RETURN_RET_LOG(runningLock_, ERR_INVALID_HANDLE, "running lock is nullptr");
    runningLock_->UpdateAppsUid(appsUid.cbegin(), appsUid.cend());
    runningLock_->UpdateAppsUidToPowerMgr();
#endif
    return SUCCESS;
}


void CabinAudioRenderSink::DumpInfo(std::string &dumpString)
{
    dumpString += "type: 3dadirectSink\tstarted: " + std::string(started_ ? "true" : "false") + "\n";
}

AudioFormat CabinAudioRenderSink::ConvertToHdiFormat(AudioSampleFormat format)
{
    AudioFormat hdiFormat;
    switch (format) {
        case SAMPLE_S16LE:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_16_BIT;
            break;
        case SAMPLE_S24LE:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_24_BIT;
            break;
        default:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_16_BIT;
            break;
    }
    return hdiFormat;
}

uint32_t CabinAudioRenderSink::PcmFormatToBit(AudioSampleFormat format)
{
    AudioFormat hdiFormat = ConvertToHdiFormat(format);
    switch (hdiFormat) {
        case AUDIO_FORMAT_TYPE_PCM_16_BIT:
            return PCM_16_BIT;
        case AUDIO_FORMAT_TYPE_PCM_24_BIT:
            return PCM_24_BIT;
        default:
            AUDIO_DEBUG_LOG("unknown format type, set it to default");
            return PCM_16_BIT;
    }
}

void CabinAudioRenderSink::InitAudioSampleAttr(struct AudioSampleAttributes &param)
{
    param.channelCount = attr_.channel;
    param.interleaved = true;
    param.streamId = static_cast<int32_t>(GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE,
        HDI_RENDER_OFFSET_AUDIO_VIVID_3DA_DIRECT));
    param.type = AUDIO_MULTI_CHANNEL;
    param.period = DEEP_BUFFER_RENDER_PERIOD_SIZE;
    param.isBigEndian = false;
    param.isSignedData = true;
    param.stopThreshold = INT_MAX;
    param.silenceThreshold = 0;
    param.sampleRate = attr_.sampleRate;
    param.channelLayout = attr_.channelLayout;
    param.format = ConvertToHdiFormat(attr_.format);
    param.frameSize = PcmFormatToBit(attr_.format) * param.channelCount / PCM_8_BIT;
    if (param.frameSize != 0) {
        param.startThreshold = DEEP_BUFFER_RENDER_PERIOD_SIZE / (param.frameSize);
    }
    AUDIO_INFO_LOG("encoding type:%{public}d, format:%{public}d, channelCount:%{public}d, sampleRate:%{public}d,"
        "channelLayout:%{public}" PRIu64,
        attr_.encodingType, param.format, param.channelCount, param.sampleRate, param.channelLayout);
}

void CabinAudioRenderSink::InitDeviceDesc(struct AudioDeviceDescriptor &deviceDesc)
{
    deviceDesc.pins = PIN_OUT_DP;
    deviceDesc.desc = const_cast<char *>("");
}

int32_t CabinAudioRenderSink::CreateRender(void)
{
    Trace trace("CabinAudioRenderSink::CreateRender");

    struct AudioSampleAttributes param;
    struct AudioDeviceDescriptor deviceDesc;
    InitAudioSampleAttr(param);
    InitDeviceDesc(deviceDesc);

    AUDIO_INFO_LOG("create render, rate: %{public}u, channel: %{public}u, format: %{public}u", param.sampleRate,
        param.channelCount, param.format);
    AudioXCollie audioXCollie("CabinAudioRenderSink::CreateRender", TIMEOUT_SECONDS_10,
         nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET_LOG(deviceManager != nullptr, ERR_INVALID_HANDLE, "DEVICE MANAGER NO");

    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
