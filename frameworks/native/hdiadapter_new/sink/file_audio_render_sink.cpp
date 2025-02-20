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
#define LOG_TAG "FileAudioRenderSink"
#endif

#include "sink/file_audio_render_sink.h"
#include "audio_hdi_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
FileAudioRenderSink::~FileAudioRenderSink()
{
    DeInit();
}

int32_t FileAudioRenderSink::Init(const IAudioSinkAttr &attr)
{
    filePath_.assign(attr.filePath);
    return SUCCESS;
}

void FileAudioRenderSink::DeInit(void)
{
    if (file_ != nullptr) {
        fclose(file_);
        file_ = nullptr;
    }
}

bool FileAudioRenderSink::IsInited(void)
{
    return !filePath_.empty();
}

int32_t FileAudioRenderSink::Start(void)
{
    std::string dirPath;
    std::string fileName;

    auto pos = filePath_.rfind("/");
    if (pos != std::string::npos) {
        dirPath = filePath_.substr(0, pos);
        fileName = filePath_.substr(pos);
    }

    if (file_ == nullptr) {
        char realPath[PATH_MAX + 1] = { 0x00 };
        CHECK_AND_RETURN_RET_LOG((filePath_.length() < PATH_MAX) && (realpath(dirPath.c_str(), realPath) != nullptr),
            ERR_INVALID_HANDLE, "invalid path, errno: %{public}d", errno);

        std::string realPathStr(realPath);
        file_ = fopen(realPathStr.append(fileName).c_str(), "wb+");
        CHECK_AND_RETURN_RET_LOG(file_ != nullptr, ERR_OPERATION_FAILED, "open file fail, errno: %{public}d", errno);
    }

    return SUCCESS;
}

int32_t FileAudioRenderSink::Stop(void)
{
    if (file_ != nullptr) {
        fclose(file_);
        file_ = nullptr;
    }

    return SUCCESS;
}

int32_t FileAudioRenderSink::Resume(void)
{
    return SUCCESS;
}

int32_t FileAudioRenderSink::Pause(void)
{
    return SUCCESS;
}

int32_t FileAudioRenderSink::Flush(void)
{
    return SUCCESS;
}

int32_t FileAudioRenderSink::Reset(void)
{
    return SUCCESS;
}

int32_t FileAudioRenderSink::RenderFrame(char &data, uint64_t len, uint64_t &writeLen)
{
    CHECK_AND_RETURN_RET_LOG(file_ != nullptr, ERR_INVALID_HANDLE, "file is nullptr");
    size_t realWriteLen = fwrite(static_cast<void *>(&data), 1, len, file_);
    if (realWriteLen != len) {
        AUDIO_WARNING_LOG("write file fail");
    }
    writeLen = realWriteLen;

    return SUCCESS;
}

int32_t FileAudioRenderSink::SuspendRenderSink(void)
{
    return SUCCESS;
}

int32_t FileAudioRenderSink::RestoreRenderSink(void)
{
    return SUCCESS;
}

void FileAudioRenderSink::SetAudioParameter(const AudioParamKey key, const std::string &condition,
    const std::string &value)
{
}

std::string FileAudioRenderSink::GetAudioParameter(const AudioParamKey key, const std::string &condition)
{
    return "";
}

int32_t FileAudioRenderSink::SetVolume(float left, float right)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t FileAudioRenderSink::GetVolume(float &left, float &right)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t FileAudioRenderSink::GetLatency(uint32_t &latency)
{
    AUDIO_INFO_LOG("not support");
    return SUCCESS;
}

int32_t FileAudioRenderSink::GetTransactionId(uint64_t &transactionId)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t FileAudioRenderSink::GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

float FileAudioRenderSink::GetMaxAmplitude(void)
{
    AUDIO_INFO_LOG("not support");
    return 0;
}

void FileAudioRenderSink::SetAudioMonoState(bool audioMono)
{
    AUDIO_INFO_LOG("not support");
}

void FileAudioRenderSink::SetAudioBalanceValue(float audioBalance)
{
    AUDIO_INFO_LOG("not support");
}

int32_t FileAudioRenderSink::SetAudioScene(AudioScene audioScene, std::vector<DeviceType> &activeDevices)
{
    AUDIO_INFO_LOG("not support");
    return SUCCESS;
}

int32_t FileAudioRenderSink::GetAudioScene(void)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t FileAudioRenderSink::UpdateActiveDevice(std::vector<DeviceType> &outputDevices)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

void FileAudioRenderSink::ResetActiveDeviceForDisconnect(DeviceType device)
{
    AUDIO_INFO_LOG("not support");
}

int32_t FileAudioRenderSink::SetPaPower(int32_t flag)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t FileAudioRenderSink::SetPriPaPower(void)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t FileAudioRenderSink::UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t FileAudioRenderSink::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

void FileAudioRenderSink::DumpInfo(std::string &dumpString)
{
    dumpString += "type: FileSink\tfilePath: " + filePath_ + "\n";
}

} // namespace AudioStandard
} // namespace OHOS
