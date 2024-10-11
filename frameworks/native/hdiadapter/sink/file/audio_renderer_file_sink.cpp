/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioRendererFileSink"
#endif

#include "audio_renderer_file_sink.h"

#include <cerrno>
#include <cstring>
#include <dlfcn.h>
#include <iostream>
#include <string>
#include <unistd.h>

#include "audio_errors.h"
#include "audio_hdi_log.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
AudioRendererFileSink::AudioRendererFileSink()
{
}

AudioRendererFileSink::~AudioRendererFileSink()
{
    AudioRendererFileSink::DeInit();
}

AudioRendererFileSink *AudioRendererFileSink::GetInstance()
{
    static AudioRendererFileSink audioRenderer;

    return &audioRenderer;
}

bool AudioRendererFileSink::IsInited()
{
    return !filePath_.empty();
}

int32_t AudioRendererFileSink::GetVolume(float &left, float &right)
{
    return ERR_NOT_SUPPORTED;
}

int32_t AudioRendererFileSink::SetVoiceVolume(float volume)
{
    return ERR_NOT_SUPPORTED;
}

int32_t AudioRendererFileSink::SetAudioScene(AudioScene audioScene, std::vector<DeviceType> &activeDevices)
{
    return ERR_NOT_SUPPORTED;
}

int32_t AudioRendererFileSink::SetOutputRoutes(std::vector<DeviceType> &outputDevices)
{
    AUDIO_DEBUG_LOG("SetOutputRoutes not supported.");
    return ERR_NOT_SUPPORTED;
}

void AudioRendererFileSink::SetAudioParameter(const AudioParamKey key, const std::string &condition,
    const std::string &value)
{
    AUDIO_ERR_LOG("AudioRendererFileSink SetAudioParameter not supported.");
    return;
}

std::string AudioRendererFileSink::GetAudioParameter(const AudioParamKey key, const std::string &condition)
{
    AUDIO_ERR_LOG("AudioRendererFileSink GetAudioParameter not supported.");
    return "";
}

void AudioRendererFileSink::RegisterParameterCallback(IAudioSinkCallback* callback)
{
    AUDIO_ERR_LOG("AudioRendererFileSink RegisterParameterCallback not supported.");
}

void AudioRendererFileSink::SetAudioMonoState(bool audioMono)
{
    AUDIO_ERR_LOG("AudioRendererFileSink SetAudioMonoState not supported.");
    return;
}

void AudioRendererFileSink::SetAudioBalanceValue(float audioBalance)
{
    AUDIO_ERR_LOG("AudioRendererFileSink SetAudioBalanceValue not supported.");
    return;
}

int32_t AudioRendererFileSink::GetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec)
{
    return ERR_NOT_SUPPORTED;
}

void AudioRendererFileSink::DeInit()
{
    if (filePtr_ != nullptr) {
        fclose(filePtr_);
        filePtr_ = nullptr;
    }
}

int32_t AudioRendererFileSink::Init(const IAudioSinkAttr &attr)
{
    filePath_.assign(attr.filePath);

    return SUCCESS;
}

int32_t AudioRendererFileSink::RenderFrame(char &data, uint64_t len, uint64_t &writeLen)
{
    CHECK_AND_RETURN_RET_LOG(filePtr_ != nullptr, ERROR, "Invalid file ptr");

    size_t writeResult = fwrite(static_cast<void*>(&data), 1, len, filePtr_);
    if (writeResult != len) {
        AUDIO_WARNING_LOG("Failed to write the file.");
    }

    writeLen = writeResult;

    return SUCCESS;
}

int32_t AudioRendererFileSink::Start(void)
{
    char realPath[PATH_MAX + 1] = {0x00};
    std::string rootPath;
    std::string fileName;

    auto pos = filePath_.rfind("/");
    if (pos!= std::string::npos) {
        rootPath = filePath_.substr(0, pos);
        fileName = filePath_.substr(pos);
    }

    if (filePtr_ == nullptr) {
        CHECK_AND_RETURN_RET_LOG((filePath_.length() < PATH_MAX) && (realpath(rootPath.c_str(), realPath) != nullptr),
            ERROR, "AudioRendererFileSink:: Invalid path  errno = %{public}d", errno);

        std::string verifiedPath(realPath);
        filePtr_ = fopen(verifiedPath.append(fileName).c_str(), "wb+");
        CHECK_AND_RETURN_RET_LOG(filePtr_ != nullptr, ERROR, "Failed to open file, errno = %{public}d", errno);
    }

    return SUCCESS;
}

int32_t AudioRendererFileSink::Stop(void)
{
    if (filePtr_ != nullptr) {
        fclose(filePtr_);
        filePtr_ = nullptr;
    }

    return SUCCESS;
}

int32_t AudioRendererFileSink::Pause(void)
{
    return SUCCESS;
}

int32_t AudioRendererFileSink::Resume(void)
{
    return SUCCESS;
}

int32_t AudioRendererFileSink::SuspendRenderSink(void)
{
    return SUCCESS;
}

int32_t AudioRendererFileSink::RestoreRenderSink(void)
{
    return SUCCESS;
}

int32_t AudioRendererFileSink::Reset(void)
{
    return SUCCESS;
}

int32_t AudioRendererFileSink::Flush(void)
{
    return SUCCESS;
}

int32_t AudioRendererFileSink::SetVolume(float left, float right)
{
    return ERR_NOT_SUPPORTED;
}

int32_t AudioRendererFileSink::GetLatency(uint32_t *latency)
{
    return ERR_NOT_SUPPORTED;
}

int32_t AudioRendererFileSink::GetTransactionId(uint64_t *transactionId)
{
    AUDIO_ERR_LOG("AudioRendererFileSink %{public}s", __func__);
    return ERR_NOT_SUPPORTED;
}

void AudioRendererFileSink::ResetOutputRouteForDisconnect(DeviceType device)
{
    AUDIO_WARNING_LOG("not supported.");
}

float AudioRendererFileSink::GetMaxAmplitude()
{
    AUDIO_WARNING_LOG("getMaxAmplitude in render file sink not support");
    return 0;
}

int32_t AudioRendererFileSink::SetPaPower(int32_t flag)
{
    (void)flag;
    return ERR_NOT_SUPPORTED;
}

int32_t AudioRendererFileSink::SetPriPaPower()
{
    return ERR_NOT_SUPPORTED;
}

int32_t AudioRendererFileSink::UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size)
{
    return SUCCESS;
}

int32_t AudioRendererFileSink::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
    return SUCCESS;
}

} // namespace AudioStandard
} // namespace OHOS
