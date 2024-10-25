/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioCapturerFileSource"
#endif

#include "audio_capturer_file_source.h"

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <string>

#include "audio_errors.h"
#include "audio_hdi_log.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
AudioCapturerFileSource::AudioCapturerFileSource()
{
}

AudioCapturerFileSource::~AudioCapturerFileSource()
{
    AudioCapturerFileSource::DeInit();
}

int32_t AudioCapturerFileSource::SetVolume(float left, float right)
{
    return SUCCESS;
}

int32_t AudioCapturerFileSource::GetVolume(float &left, float &right)
{
    return SUCCESS;
}

int32_t AudioCapturerFileSource::SetMute(bool isMute)
{
    return SUCCESS;
}

int32_t AudioCapturerFileSource::GetMute(bool &isMute)
{
    return SUCCESS;
}

int32_t AudioCapturerFileSource::SetInputRoute(DeviceType inputDevice, const std::string &deviceName)
{
    return SUCCESS;
}

void AudioCapturerFileSource::RegisterWakeupCloseCallback(IAudioSourceCallback *callback)
{
    AUDIO_WARNING_LOG("RegisterWakeupCloseCallback FAILED");
}

void AudioCapturerFileSource::RegisterAudioCapturerSourceCallback(std::unique_ptr<ICapturerStateCallback> callback)
{
    AUDIO_WARNING_LOG("RegisterAudioCapturerSourceCallback FAILED");
}

void AudioCapturerFileSource::RegisterParameterCallback(IAudioSourceCallback *callback)
{
    AUDIO_WARNING_LOG("RegisterParameterCallback in file mode is not supported!");
}

int32_t AudioCapturerFileSource::SetAudioScene(AudioScene audioScene, DeviceType activeDevice,
    const std::string &deviceName)
{
    return SUCCESS;
}

uint64_t AudioCapturerFileSource::GetTransactionId(void)
{
    uint64_t res = -1L;
    return res;
}

int32_t AudioCapturerFileSource::GetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec)
{
    return SUCCESS;
}

int32_t AudioCapturerFileSource::Pause(void)
{
    return SUCCESS;
}

int32_t AudioCapturerFileSource::Resume(void)
{
    return SUCCESS;
}

int32_t AudioCapturerFileSource::Reset(void)
{
    return SUCCESS;
}

int32_t AudioCapturerFileSource::Flush(void)
{
    return SUCCESS;
}

bool AudioCapturerFileSource::IsInited(void)
{
    return capturerInited_;
}

void AudioCapturerFileSource::DeInit()
{
    if (filePtr != nullptr) {
        fclose(filePtr);
        filePtr = nullptr;
    }
    capturerInited_ = false;
}

int32_t AudioCapturerFileSource::Init(const IAudioSourceAttr &attr)
{
    const char *filePath = attr.filePath;
    char realPath[PATH_MAX + 1] = {0x00};
    std::string sourceFilePath(filePath);
    std::string rootPath;
    std::string fileName;

    auto pos = sourceFilePath.rfind("/");
    if (pos!= std::string::npos) {
        rootPath = sourceFilePath.substr(0, pos);
        fileName = sourceFilePath.substr(pos);
    }

    bool tmp = strlen(sourceFilePath.c_str()) >= PATH_MAX || realpath(rootPath.c_str(), realPath) == nullptr;
    CHECK_AND_RETURN_RET_LOG(!tmp, ERROR, "AudioCapturerFileSource:: Invalid path errno = %{public}d", errno);

    std::string verifiedPath(realPath);
    filePtr = fopen(verifiedPath.append(fileName).c_str(), "rb");
    CHECK_AND_RETURN_RET_LOG(filePtr != nullptr, ERROR, "Error opening pcm test file!");

    capturerInited_ = true;
    return SUCCESS;
}

int32_t AudioCapturerFileSource::CaptureFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes)
{
    CHECK_AND_RETURN_RET_LOG(filePtr != nullptr, ERROR, "Invalid filePtr!");

    if (feof(filePtr)) {
        AUDIO_INFO_LOG("End of the file reached, start reading from beginning");
        rewind(filePtr);
    }

    replyBytes = fread(frame, 1, requestBytes, filePtr);

    return SUCCESS;
}

int32_t AudioCapturerFileSource::CaptureFrameWithEc(
    FrameDesc *fdesc, uint64_t &replyBytes,
    FrameDesc *fdescEc, uint64_t &replyBytesEc)
{
    AUDIO_ERR_LOG("not supported!");
    return ERR_DEVICE_NOT_SUPPORTED;
}

int32_t AudioCapturerFileSource::Start(void)
{
    return SUCCESS;
}

int32_t AudioCapturerFileSource::Stop(void)
{
    if (filePtr != nullptr) {
        fclose(filePtr);
        filePtr = nullptr;
    }
    return SUCCESS;
}

std::string AudioCapturerFileSource::GetAudioParameter(const AudioParamKey key, const std::string &condition)
{
    AUDIO_WARNING_LOG("not supported yet");
    return "";
}

float AudioCapturerFileSource::GetMaxAmplitude()
{
    AUDIO_WARNING_LOG("getMaxAmplitude in audio cap file not support");
    return 0;
}

int32_t AudioCapturerFileSource::UpdateAppsUid(const int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE], const size_t size)
{
    AUDIO_WARNING_LOG("not supported.");
    return ERR_NOT_SUPPORTED;
}

int32_t AudioCapturerFileSource::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
    AUDIO_WARNING_LOG("not supported.");
    return ERR_NOT_SUPPORTED;
}

int32_t AudioCapturerFileSource::GetCaptureId(uint32_t &captureId) const
{
    AUDIO_WARNING_LOG("not supported.");
    return ERR_NOT_SUPPORTED;
}
} // namespace AudioStandard
} // namespace OHOS
