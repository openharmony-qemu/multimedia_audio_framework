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

#ifndef AUDIO_SUITE_COMMON_H
#define AUDIO_SUITE_COMMON_H

#include <dlfcn.h>
#include <stdlib.h>
#include "audio_stream_info.h"
#include "audio_suite_base.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

static constexpr uint32_t SAMPLE_SIZE_1_BYTE = 1;
static constexpr uint32_t SAMPLE_SIZE_2_BYTE = 2;
static constexpr uint32_t SAMPLE_SIZE_3_BYTE = 3;
static constexpr uint32_t SAMPLE_SIZE_4_BYTE = 4;

static const std::unordered_map<std::string, std::vector<std::string>> OPTIONS_FIELD_NAMES = {
    {"AudioEqualizerFrequencyBandGains",
        {"band0", "band1", "band2", "band3", "band4", "band5", "band6", "band7", "band8", "band9"}},
    {"AudioSpaceRenderPositionParams", {"x", "y", "z"}},
    {"AudioSpaceRenderRotationParams", {"x", "y", "z", "surroundTime", "surroundDirection"}},
    {"AudioSpaceRenderExtensionParams", {"extRadius", "extAngle"}},
    {"speedAndPitch", {"speed", "pitch"}},
    {"AudioPureVoiceChangeOption", {"optionGender", "optionType", "pitch"}},
};

static const std::unordered_map<std::string, std::unordered_map<int, std::string>> ENUM_STRING_MAP = {
    {"EnvironmentType", {
        {0, "Close"}, {1, "Broadcast"}, {2, "Earpiece"},
        {3, "Underwater"}, {4, "Gramophone"}
    }},
    {"SoundFieldType", {
        {0, "Close"}, {1, "FrontFacing"}, {2, "Grand"},
        {3, "Near"}, {4, "Wide"}
    }},
    {"VoiceBeautifierType", {
        {1, "Clear"}, {2, "Theatre"}, {3, "CD"}, {4, "Studio"}
    }},
    {"GeneralVoiceChangeType", {
        {1, "Cute"}, {2, "Cyberpunk"}, {3, "Female"}, {4, "Male"},
        {5, "Mix"}, {6, "Monster"}, {7, "Seasoned"}, {8, "Synth"},
        {9, "Trill"}, {10, "War"}
    }},
    {"AudioPureVoiceChangeGenderOption", {
        {1, "FEMALE"}, {2, "MALE"}
    }},
    {"AudioPureVoiceChangeType", {
        {1, "CARTOON"}, {2, "CUTE"}, {3, "FEMALE"}, {4, "MALE"},
        {5, "MONSTER"}, {6, "ROBOTS"}, {7, "SEASONED"}
    }}
};

static const std::unordered_map<std::string, std::vector<std::string>> FIELD_ENUM_TYPE_MAP = {
    {"AudioPureVoiceChangeOption", {"AudioPureVoiceChangeGenderOption", "AudioPureVoiceChangeType", ""}}
};

class AudioSuiteRingBuffer {
public:
    AudioSuiteRingBuffer() = default;
    AudioSuiteRingBuffer(uint32_t capacity) : capacity_(capacity), head_(0), tail_(0), size_(0)
    {
        ResizeBuffer(capacity);
    }

    ~AudioSuiteRingBuffer()
    {
    }

    AudioSuiteRingBuffer(const AudioSuiteRingBuffer &other)
        : capacity_(other.capacity_),
          head_(other.head_),
          tail_(other.tail_),
          size_(other.size_)
    {
        buffer_.clear();
        buffer_.resize(other.buffer_.size());
        std::copy(other.buffer_.begin(), other.buffer_.end(), buffer_.begin());
    }
    AudioSuiteRingBuffer& operator=(const AudioSuiteRingBuffer&) = delete;

    int32_t PushData(uint8_t* byteData, uint32_t size);
    int32_t GetData(uint8_t* byteData, uint32_t size);
    int32_t ResizeBuffer(uint32_t size);
    int32_t ClearBuffer();
    uint32_t GetRestSpace() const;
    uint32_t GetSize() const;
    
private:
    std::vector<uint8_t> buffer_;
    uint32_t capacity_ = 0;
    uint32_t head_ = 0;
    uint32_t tail_ = 0;
    uint32_t size_ = 0;
};

class AudioSuiteUtil {
public:
    static uint32_t GetSampleSize(AudioSampleFormat type)
    {
        uint32_t sampleSize = SAMPLE_SIZE_4_BYTE;
        switch (type) {
            case AudioSampleFormat::SAMPLE_U8:
                sampleSize = SAMPLE_SIZE_1_BYTE;
                break;
            case AudioSampleFormat::SAMPLE_S16LE:
                sampleSize = SAMPLE_SIZE_2_BYTE;
                break;
            case AudioSampleFormat::SAMPLE_S24LE:
                sampleSize = SAMPLE_SIZE_3_BYTE;
                break;
            case AudioSampleFormat::SAMPLE_S32LE:
            case AudioSampleFormat::SAMPLE_F32LE:
                sampleSize = SAMPLE_SIZE_4_BYTE;
                break;
            default:
                break;
        }
        return sampleSize;
    }

    static bool IsEnumTypeOption(const std::string &name);
    static std::string ConvertEnumToString(const std::string &name, const std::string &value);
    static std::string GetCurrentTimestamp();
    static std::string GetWorkModeString(PipelineWorkMode mode);
    static std::string GetStateString(AudioSuitePipelineState state);
    static std::string GetSampleFormatString(AudioSampleFormat format);
    static std::string FormatOptionsValue(const std::string &name, const std::string &value);
};

class AudioSuiteLibraryManager {
    public:
        bool IsValidPath(const char *path);
        void* LoadLibrary(std::string& libraryPath);
};

}
}
}
#endif