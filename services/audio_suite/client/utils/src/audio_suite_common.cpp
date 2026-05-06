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


#include "audio_suite_common.h"
#include "audio_suite_log.h"
#include "audio_errors.h"
#include <sstream>
#include <charconv>

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

static constexpr uint32_t MAX_CACHE = std::numeric_limits<uint32_t>::max() - 1; // Maximum allocation capacity.
static const char *const PREFIX = "/system/lib64/";

int32_t AudioSuiteRingBuffer::PushData(uint8_t* byteData, uint32_t size)
{
    if ((size == 0) || (size > GetRestSpace()) || (buffer_.data() == nullptr) || (byteData == nullptr)) {
        return ERR_INVALID_OPERATION;
    }

    size_t firstChunk = std::min(size, capacity_ - tail_);
    errno_t err = memcpy_s(buffer_.data() + tail_, capacity_ - tail_, byteData, firstChunk);
    if (err != 0) {
        AUDIO_ERR_LOG("AudioSuiteRingBuffer::PushData err:%{public}d, capacity_:%{public}u,"
            "tail_:%{public}u, size:%{public}u", err, capacity_, tail_, size);
        return ERR_INVALID_OPERATION;
    }

    if (size > firstChunk) {
        err = memcpy_s(buffer_.data(), head_, byteData + firstChunk, size - firstChunk);
        if (err != 0) {
            AUDIO_ERR_LOG("AudioSuiteRingBuffer::PushData err:%{public}d, head_:%{public}u,"
                "size:%{public}u, firstChunk:%{public}zu", err, head_, size, firstChunk);
            return ERR_INVALID_OPERATION;
        }
    }

    tail_ = (tail_ + size) % capacity_;
    size_ += size;
    return SUCCESS;
}

int32_t AudioSuiteRingBuffer::GetData(uint8_t* byteData, uint32_t size)
{
    if ((size == 0) || (size > size_) || (buffer_.data() == nullptr) || (byteData == nullptr)) {
        return ERR_INVALID_OPERATION;
    }
    size_t firstChunk = std::min(size, capacity_ - head_);
    errno_t err = memcpy_s(byteData, size, buffer_.data() + head_, firstChunk);
    if (err != 0) {
        AUDIO_ERR_LOG("AudioSuiteRingBuffer::GetData err:%{public}d, capacity_:%{public}u,"
            "head_:%{public}u, size:%{public}u", err, capacity_, head_, size);
        return ERR_INVALID_OPERATION;
    }

    if (size > firstChunk) {
        err = memcpy_s(byteData + firstChunk, size - firstChunk, buffer_.data(), size - firstChunk);
        if (err != 0) {
            AUDIO_ERR_LOG("AudioSuiteRingBuffer::GetData err:%{public}d, capacity_:%{public}u, head_:%{public}u, "
                "size:%{public}u, firstChunk:%{public}zu", err, capacity_, head_, size, firstChunk);
            return ERR_INVALID_OPERATION;
        }
    }

    head_ = (head_ + size) % capacity_;
    size_ -= size;
    return SUCCESS;
}

int32_t AudioSuiteRingBuffer::ResizeBuffer(uint32_t size)
{
    if (size <= 0 || size > MAX_CACHE) {
        return ERROR;
    }

    buffer_.resize(size);
    capacity_ = size;
    ClearBuffer();
    return 0;
}

int32_t AudioSuiteRingBuffer::ClearBuffer()
{
    head_ = 0;
    tail_ = 0;
    size_ = 0;
    return 0;
}

uint32_t AudioSuiteRingBuffer::GetRestSpace() const
{
    return capacity_ > size_ ? capacity_ - size_ : 0;
}

uint32_t AudioSuiteRingBuffer::GetSize() const
{
    return size_;
}

bool AudioSuiteLibraryManager::IsValidPath(const char *path)
{
    CHECK_AND_RETURN_RET_LOG(path != nullptr, false, "Path is null");
    CHECK_AND_RETURN_RET_LOG(
        strncmp(path, PREFIX, strlen(PREFIX)) == 0, false, "Path is invalid");
    return true;
}

void* AudioSuiteLibraryManager::LoadLibrary(std::string& libraryPath)
{
    char buffer[PATH_MAX] = {0};
    char *canonicalPath = realpath(libraryPath.c_str(), buffer);
    if (canonicalPath == nullptr) {
        return nullptr;
    }
    if (IsValidPath(canonicalPath) == false) {
        return nullptr;
    }
    
    libraryPath = buffer;
    void *handle = dlopen(libraryPath.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    CHECK_AND_RETURN_RET_LOG(handle != nullptr,
        nullptr,
        "Failed to load library: %{private}s. Error: %{public}s",
        libraryPath.c_str(),
        dlerror());
    return handle;
}

bool AudioSuiteUtil::IsEnumTypeOption(const std::string &name)
{
    static const std::set<std::string> enumTypes = {
        "EnvironmentType", "SoundFieldType",
        "VoiceBeautifierType", "GeneralVoiceChangeType"
    };
    return enumTypes.find(name) != enumTypes.end();
}

std::string AudioSuiteUtil::ConvertEnumToString(const std::string &name, const std::string &value)
{
    auto typeIt = ENUM_STRING_MAP.find(name);
    if (typeIt == ENUM_STRING_MAP.end()) {
        return value;
    }

    int enumValue = 0;
    auto result = std::from_chars(value.data(), value.data() + value.size(), enumValue);
    if (result.ec != std::errc()) {
        AUDIO_ERR_LOG("ConvertEnumToString from_chars failed");
        return value;
    }

    auto valueIt = typeIt->second.find(enumValue);
    if (valueIt == typeIt->second.end()) {
        return value;
    }
    return valueIt->second;
}

std::string AudioSuiteUtil::GetCurrentTimestamp()
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm tm = {};
#ifdef _WIN32
    errno_t err = localtime_s(&tm, &time);
    if (err != 0) {
        AUDIO_ERR_LOG("localtime_s failed, err = %{public}d", err);
        return "";
    }
#else
    std::tm *result = localtime_r(&time, &tm);
    if (result == nullptr) {
        AUDIO_ERR_LOG("localtime_r failed");
        return "";
    }
#endif

    char buffer[64];
    int ret = snprintf_s(buffer, sizeof(buffer), sizeof(buffer) - 1,
        "%04d-%02d-%02d %02d:%02d:%02d.%03d",
        tm.tm_year + 1900,
        tm.tm_mon + 1,
        tm.tm_mday,
        tm.tm_hour,
        tm.tm_min,
        tm.tm_sec,
        static_cast<int>(ms.count()));
    if (ret < 0) {
        AUDIO_ERR_LOG("snprintf_s failed");
        return "";
    }
    return std::string(buffer);
}

std::string AudioSuiteUtil::GetWorkModeString(PipelineWorkMode mode)
{
    switch (mode) {
        case PIPELINE_EDIT_MODE:
            return "EDIT_MODE";
        case PIPELINE_REALTIME_MODE:
            return "REALTIME_MODE";
        default:
            return "UNKNOWN";
    }
}

std::string AudioSuiteUtil::GetStateString(AudioSuitePipelineState state)
{
    switch (state) {
        case PIPELINE_STOPPED:
            return "STOPPED";
        case PIPELINE_RUNNING:
            return "RUNNING";
        default:
            return "UNKNOWN";
    }
}

std::string AudioSuiteUtil::GetSampleFormatString(AudioSampleFormat format)
{
    switch (format) {
        case SAMPLE_S16LE:
            return "PCM16";
        case SAMPLE_S24LE:
            return "PCM24";
        case SAMPLE_S32LE:
            return "PCM32";
        case SAMPLE_F32LE:
            return "FLOAT32";
        default:
            return "UNKNOWN";
    }
}

std::string AudioSuiteUtil::FormatOptionsValue(const std::string &name, const std::string &value)
{
    if (value.empty()) {
        return "";
    }

    if (IsEnumTypeOption(name)) {
        return ConvertEnumToString(name, value);
    }

    auto it = OPTIONS_FIELD_NAMES.find(name);
    if (it == OPTIONS_FIELD_NAMES.end()) {
        return value;
    }

    const std::vector<std::string> &fieldNames = it->second;

    char delimiter = (name == "AudioEqualizerFrequencyBandGains") ? ':' : ',';

    std::vector<std::string> values;
    std::istringstream iss(value);
    std::string token;
    while (std::getline(iss, token, delimiter)) {
        values.push_back(token);
    }

    auto fieldEnumIt = FIELD_ENUM_TYPE_MAP.find(name);
    const std::vector<std::string> &fieldEnumTypes =
        (fieldEnumIt != FIELD_ENUM_TYPE_MAP.end()) ? fieldEnumIt->second : std::vector<std::string>();

    std::string result;
    for (size_t i = 0; i < fieldNames.size() && i < values.size(); i++) {
        if (i > 0) {
            result += ", ";
        }
        std::string fieldValue = values[i];
        if (i < fieldEnumTypes.size() && !fieldEnumTypes[i].empty()) {
            fieldValue = ConvertEnumToString(fieldEnumTypes[i], values[i]);
        }
        result += fieldNames[i] + ":" + fieldValue;
    }

    return result;
}

}
}
}