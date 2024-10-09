/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#ifndef AUDIO_CONVERTER_3DA
#define AUDIO_CONVERTER_3DA

#include <cstdint>
#include <cstring>
#include <string>
#include <dlfcn.h>
#include <unistd.h>
#include "audio_info.h"
#include "audio_effect.h"
#include "audio_service_log.h"

namespace OHOS {
namespace AudioStandard {

class LibLoader {
public:
    LibLoader() = default;
    ~LibLoader();
    bool Init();
    uint32_t GetLatency();
    bool AddAlgoHandle(Library lib);
    void SetIOBufferConfig(bool isInput, uint32_t sampleRate, uint8_t format, uint32_t channels,
        uint64_t channelLayout);
    int32_t ApplyAlgo(AudioBuffer &inputBuffer, AudioBuffer &outputBuffer);
    bool FlushAlgo();

private:
    bool LoadLibrary(const std::string &relativePath) noexcept;
    std::unique_ptr<AudioEffectLibEntry> libEntry_;
    uint32_t latency_;
    AudioEffectHandle handle_;
    AudioEffectConfig ioBufferConfig_;
    void *libHandle_;
};

class AudioSpatialChannelConverter {
public:
    AudioSpatialChannelConverter() = default;
    bool Init(const AudioStreamParams info, const ConverterConfig cfg);
    bool GetInputBufferSize(size_t &bufferSize);
    size_t GetMetaSize();
    bool CheckInputValid(const BufferDesc bufDesc);
    bool AllocateMem();
    bool Flush();
    uint32_t GetLatency();
    void Process(const BufferDesc bufDesc);
    void ConverterChannels(uint8_t &channel, uint64_t &channelLayout);
    void GetOutputBufferStream(uint8_t *&buffer, uint32_t &bufferLen);

private:
    size_t GetPcmLength(int32_t channels, int8_t bps);

    std::unique_ptr<uint8_t[]> outPcmBuf_;

    int32_t inChannel_;
    int32_t outChannel_;
    int32_t sampleRate_;

    uint8_t bps_;
    uint8_t encoding_;

    uint64_t outChannelLayout_;

    bool loadSuccess_;

    LibLoader externalLoader_;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_CONVERTER_3DA