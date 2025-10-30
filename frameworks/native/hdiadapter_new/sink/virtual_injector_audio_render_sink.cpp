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
#define LOG_TAG "VirtualInjectorAudioRenderSink"
#endif

#include "sink/virtual_injector_audio_render_sink.h"
#include "audio_hdi_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
VirtualInjectorAudioRenderSink::~VirtualInjectorAudioRenderSink()
{
    DeInit();
}

int32_t VirtualInjectorAudioRenderSink::Init(const IAudioSinkAttr &attr)
{
    return SUCCESS;
}

void VirtualInjectorAudioRenderSink::DeInit(void)
{
}

bool VirtualInjectorAudioRenderSink::IsInited(void)
{
    return true;
}

int32_t VirtualInjectorAudioRenderSink::Start(void)
{
    return SUCCESS;
}

int32_t VirtualInjectorAudioRenderSink::Stop(void)
{
    return SUCCESS;
}

int32_t VirtualInjectorAudioRenderSink::Resume(void)
{
    return SUCCESS;
}

int32_t VirtualInjectorAudioRenderSink::Pause(void)
{
    return SUCCESS;
}

int32_t VirtualInjectorAudioRenderSink::Flush(void)
{
    return SUCCESS;
}

int32_t VirtualInjectorAudioRenderSink::Reset(void)
{
    return SUCCESS;
}

int32_t VirtualInjectorAudioRenderSink::RenderFrame(char &data, uint64_t len, uint64_t &writeLen)
{
    return SUCCESS;
}

int64_t VirtualInjectorAudioRenderSink::GetVolumeDataCount()
{
    return 0;
}

int32_t VirtualInjectorAudioRenderSink::SuspendRenderSink(void)
{
    return SUCCESS;
}

int32_t VirtualInjectorAudioRenderSink::RestoreRenderSink(void)
{
    return SUCCESS;
}

void VirtualInjectorAudioRenderSink::SetAudioParameter(const AudioParamKey key, const std::string &condition,
    const std::string &value)
{
}

std::string VirtualInjectorAudioRenderSink::GetAudioParameter(const AudioParamKey key, const std::string &condition)
{
    return "";
}

int32_t VirtualInjectorAudioRenderSink::SetVolume(float left, float right)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t VirtualInjectorAudioRenderSink::GetVolume(float &left, float &right)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t VirtualInjectorAudioRenderSink::GetLatency(uint32_t &latency)
{
    AUDIO_INFO_LOG("not support");
    return SUCCESS;
}

int32_t VirtualInjectorAudioRenderSink::GetTransactionId(uint64_t &transactionId)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t VirtualInjectorAudioRenderSink::GetPresentationPosition(uint64_t &frames, int64_t &timeSec,
    int64_t &timeNanoSec)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

float VirtualInjectorAudioRenderSink::GetMaxAmplitude(void)
{
    AUDIO_INFO_LOG("not support");
    return 0;
}

void VirtualInjectorAudioRenderSink::SetAudioMonoState(bool audioMono)
{
    AUDIO_INFO_LOG("not support");
}

void VirtualInjectorAudioRenderSink::SetAudioBalanceValue(float audioBalance)
{
    AUDIO_INFO_LOG("not support");
}

int32_t VirtualInjectorAudioRenderSink::SetAudioScene(AudioScene audioScene, bool scoExcludeFlag)
{
    AUDIO_INFO_LOG("not support");
    return SUCCESS;
}

int32_t VirtualInjectorAudioRenderSink::GetAudioScene(void)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t VirtualInjectorAudioRenderSink::UpdateActiveDevice(std::vector<DeviceType> &outputDevices)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

void VirtualInjectorAudioRenderSink::ResetActiveDeviceForDisconnect(DeviceType device)
{
    AUDIO_INFO_LOG("not support");
}

int32_t VirtualInjectorAudioRenderSink::SetPaPower(int32_t flag)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t VirtualInjectorAudioRenderSink::SetPriPaPower(void)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t VirtualInjectorAudioRenderSink::UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t VirtualInjectorAudioRenderSink::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

void VirtualInjectorAudioRenderSink::DumpInfo(std::string &dumpString)
{
}
} // namespace AudioStandard
} // namespace OHOS