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

CabinAudioRenderSink::CabinAudioRenderSink()
#ifdef MULTI_BUS_ENABLE
    : playingAppManager_([this](const std::string &param) { return SetPlayingAppNameParam(param); })
#endif
{
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
    if (sinkInited_ && attr_.channel == attr.channel && attr_.sampleRate == attr.sampleRate
        && attr_.format == attr.format && attr_.encodingType == attr.encodingType
        && attr_.channelLayout == attr.channelLayout) {
        AUDIO_INFO_LOG("CabinAudioRenderSink already inited with same attr, skip");
        return SUCCESS;
    }
    // Re-init case: attr changed (e.g. port opened at boot with xml module args 6ch, while the
    // 3DA stream needs 16ch), or a previous render of this sink is still alive in HAL even after
    // DeInit (HAL index out of sync leaves the output stream leaked). hdiRenderId_ != 0 means this
    // sink object created a render before (not reset by DeInit). Destroy it defensively first —
    // harmless no-op if already closed — otherwise HAL rejects the new open with "bus already exist".
    if (hdiRenderId_ != 0) {
        AUDIO_INFO_LOG("re-init (ch %{public}u -> %{public}u, layout %{public}" PRIu64 " -> %{public}" PRIu64
            ", inited %{public}d), destroy previous render %{public}u first", attr_.channel, attr.channel,
            attr_.channelLayout, attr.channelLayout, sinkInited_, hdiRenderId_);
        HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
        std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
        if (deviceManager != nullptr) {
            deviceManager->DestroyRender(attr_.adapterName, hdiRenderId_);
        }
        audioRender_ = nullptr;
        hdiRenderId_ = 0;
        sinkInited_ = false;
    }
    Trace trace("CabinAudioRenderSink::Init");
    attr_ = attr;
    // The 3DA HDI render supports S16LE/F32LE only; reject other formats explicitly instead
    // of letting ConvertToHdiFormat silently downgrade them to 16-bit.
    CHECK_AND_RETURN_RET_LOG(attr.format == SAMPLE_S16LE || attr.format == SAMPLE_F32LE,
        ERR_NOT_SUPPORTED, "3DA sink unsupported format %{public}d", attr.format);
    int32_t ret = CreateRender();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_NOT_STARTED, "create render fail");
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
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    if (deviceManager != nullptr) {
        deviceManager->DestroyRender(attr_.adapterName, hdiRenderId_);
    }
    audioRender_ = nullptr;
    // reset the destroyed render id so a later re-init cannot destroy a stale id again
    hdiRenderId_ = 0;
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
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "audioRender_ is nullptr");
    AudioXCollie audioXCollie("CabinAudioRenderSink::Start", TIMEOUT_SECONDS_10,
        nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    int32_t ret = audioRender_->Start(audioRender_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "audioRender_ Start fail, ret: %{public}d", ret);
    started_ = true;
#ifdef MULTI_BUS_ENABLE
    playingAppManager_.Start();
#endif
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
    AudioXCollie audioXCollie("CabinAudioRenderSink::Stop", TIMEOUT_SECONDS_10,
        nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    if (audioRender_ != nullptr) {
        int32_t ret = audioRender_->Stop(audioRender_);
        if (ret != SUCCESS) {
            AUDIO_WARNING_LOG("audioRender_ Stop fail, ret: %{public}d", ret);
        }
    }
#ifdef MULTI_BUS_ENABLE
    playingAppManager_.Stop();
#endif
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
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_INVALID_HANDLE, "audioRender_ is nullptr");
    // The 3DA HDI frame (magic|size1|metadata|align|size2|pcm) is assembled by
    // ProRendererStreamImpl::WriteToSinkBuffer; pass the whole frame through to HDI RenderFrame.
    Trace trace("CabinAudioRenderSink::RenderFrame");
    DumpFileUtil::WriteDumpFile(dumpFile_, static_cast<void *>(&data), len);
    int32_t ret = audioRender_->RenderFrame(audioRender_, reinterpret_cast<int8_t *>(&data),
        static_cast<uint32_t>(len), &writeLen);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_WRITE_FAILED, "RenderFrame fail, ret: %{public}x", ret);
    return SUCCESS;
}

int32_t CabinAudioRenderSink::GetVolumeDataCount(int64_t &volumeData)
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

int32_t CabinAudioRenderSink::SetVolumeWithRamp(float left, float right, uint32_t durationMs)
{
    AUDIO_INFO_LOG("CabinAudioRenderSink::SetVolumeWithRamp in");
    return SUCCESS;
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

bool CabinAudioRenderSink::SetPlayingAppNameParam(const std::string &param)
{
    AUDIO_INFO_LOG("param: %{public}s", param.c_str());
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, false, "render is nullptr");
    int32_t ret = audioRender_->SetExtraParams(audioRender_, param.c_str());
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "set playing app name param fail, error code: %{public}d", ret);
    return true;
}

int32_t CabinAudioRenderSink::SetSinkMuteForSwitchDevice(bool mute)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t CabinAudioRenderSink::UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size)
{
#ifdef MULTI_BUS_ENABLE
    playingAppManager_.UpdateAppsUid(appsUid, appsUid + size);
    playingAppManager_.UpdatePlayingAppNameToHdi();
#endif
#ifdef FEATURE_POWER_MANAGER
    CHECK_AND_RETURN_RET_LOG(runningLock_, ERR_INVALID_HANDLE, "running lock is nullptr");
    runningLock_->UpdateAppsUid(appsUid, appsUid + size);
#endif
    return SUCCESS;
}

int32_t CabinAudioRenderSink::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
#ifdef MULTI_BUS_ENABLE
    playingAppManager_.UpdateAppsUid(appsUid.cbegin(), appsUid.cend());
    playingAppManager_.UpdatePlayingAppNameToHdi();
#endif
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
        case SAMPLE_F32LE:
            hdiFormat = AUDIO_FORMAT_TYPE_PCM_32_BIT;
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
        case AUDIO_FORMAT_TYPE_PCM_32_BIT:
            return PCM_32_BIT;
        default:
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
    deviceDesc.pins = PIN_OUT_SPEAKER;
    deviceDesc.desc = const_cast<char *>(attr_.address.c_str());
}

int32_t CabinAudioRenderSink::CreateRender(void)
{
    Trace trace("CabinAudioRenderSink::CreateRender");

    struct AudioSampleAttributes param;
    struct AudioDeviceDescriptor deviceDesc;
    InitAudioSampleAttr(param);
    InitDeviceDesc(deviceDesc);
    // type/format use generic multi-channel PCM values until the HDI AudioVivid render
    // contract lands; streamId/channelLayout are already wired for 3DA direct.

    AUDIO_INFO_LOG("create render, rate: %{public}u, channel: %{public}u, format: %{public}u", param.sampleRate,
        param.channelCount, param.format);
    AudioXCollie audioXCollie("CabinAudioRenderSink::CreateRender", TIMEOUT_SECONDS_10,
         nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_RET_LOG(deviceManager != nullptr, ERR_INVALID_HANDLE, "device manager is nullptr");
    void *render = deviceManager->CreateRender(attr_.adapterName, &param, &deviceDesc, hdiRenderId_);
    audioRender_ = static_cast<struct IAudioRender *>(render);
    CHECK_AND_RETURN_RET_LOG(audioRender_ != nullptr, ERR_NOT_STARTED, "create render fail, audioRender_ is null");

    AUDIO_INFO_LOG("create render success, hdiRenderId_: %{public}u", hdiRenderId_);
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
