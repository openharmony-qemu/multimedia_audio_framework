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
#define LOG_TAG "FileAudioCaptureSource"
#endif

#include "source/file_audio_capture_source.h"
#include "audio_hdi_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
FileAudioCaptureSource::~FileAudioCaptureSource()
{
    DeInit();
}

int32_t FileAudioCaptureSource::Init(const IAudioSourceAttr &attr)
{
    std::string filePath(attr.filePath);
    std::string dirPath;
    std::string fileName;

    auto pos = filePath.rfind("/");
    if (pos != std::string::npos) {
        dirPath = filePath.substr(0, pos);
        fileName = filePath.substr(pos);
    }

    char realPath[PATH_MAX + 1] = { 0x00 };
    CHECK_AND_RETURN_RET_LOG((filePath.length() < PATH_MAX) && (realpath(dirPath.c_str(), realPath) != nullptr),
        ERR_INVALID_HANDLE, "invalid path, errno: %{public}d", errno);

    std::string realPathStr(realPath);
    file_ = fopen(realPathStr.append(fileName).c_str(), "rb");
    CHECK_AND_RETURN_RET_LOG(file_ != nullptr, ERROR, "open file fail, errno: %{public}d", errno);

    sourceInited_ = true;
    return SUCCESS;
}

void FileAudioCaptureSource::DeInit(void)
{
    if (file_ != nullptr) {
        fclose(file_);
        file_ = nullptr;
    }

    sourceInited_ = false;
}

bool FileAudioCaptureSource::IsInited(void)
{
    return sourceInited_;
}

int32_t FileAudioCaptureSource::Start(void)
{
    return SUCCESS;
}

int32_t FileAudioCaptureSource::Stop(void)
{
    if (file_ != nullptr) {
        fclose(file_);
        file_ = nullptr;
    }

    return SUCCESS;
}

int32_t FileAudioCaptureSource::Resume(void)
{
    return SUCCESS;
}

int32_t FileAudioCaptureSource::Pause(void)
{
    return SUCCESS;
}

int32_t FileAudioCaptureSource::Flush(void)
{
    return SUCCESS;
}

int32_t FileAudioCaptureSource::Reset(void)
{
    return SUCCESS;
}

int32_t FileAudioCaptureSource::CaptureFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes)
{
    CHECK_AND_RETURN_RET_LOG(file_ != nullptr, ERROR, "file is nullptr");
    if (feof(file_)) {
        AUDIO_INFO_LOG("reach end of the file, start reading from beginning");
        rewind(file_);
    }
    replyBytes = fread(frame, 1, requestBytes, file_);

    return SUCCESS;
}

int32_t FileAudioCaptureSource::CaptureFrameWithEc(FrameDesc *fdesc, uint64_t &replyBytes, FrameDesc *fdescEc,
    uint64_t &replyBytesEc)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

std::string FileAudioCaptureSource::GetAudioParameter(const AudioParamKey key, const std::string &condition)
{
    return "";
}

int32_t FileAudioCaptureSource::SetVolume(float left, float right)
{
    return SUCCESS;
}

int32_t FileAudioCaptureSource::GetVolume(float &left, float &right)
{
    return SUCCESS;
}

int32_t FileAudioCaptureSource::SetMute(bool isMute)
{
    return SUCCESS;
}

int32_t FileAudioCaptureSource::GetMute(bool &isMute)
{
    return SUCCESS;
}

uint64_t FileAudioCaptureSource::GetTransactionId(void)
{
    uint64_t res = -1L;
    return res;
}

int32_t FileAudioCaptureSource::GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec)
{
    return SUCCESS;
}

float FileAudioCaptureSource::GetMaxAmplitude(void)
{
    AUDIO_INFO_LOG("not support");
    return 0;
}

int32_t FileAudioCaptureSource::SetAudioScene(AudioScene audioScene, DeviceType activeDevice)
{
    return SUCCESS;
}

int32_t FileAudioCaptureSource::UpdateActiveDevice(DeviceType inputDevice)
{
    return SUCCESS;
}

int32_t FileAudioCaptureSource::UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE], const size_t size)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t FileAudioCaptureSource::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
    AUDIO_INFO_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

void FileAudioCaptureSource::DumpInfo(std::string &dumpString)
{
    dumpString += "type: FileSource\n";
}

} // namespace AudioStandard
} // namespace OHOS
