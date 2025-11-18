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
#include <iostream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <chrono>
#include <thread>
#include <dlfcn.h>

#include "securec.h"
#include "audio_suite_aiss_node.h"
#include "audio_suite_aiss_algo_interface_impl.h"
#include "audio_suite_algo_interface.h"

namespace {
    const std::string INPUT_PATH = "/data/aiss_48000_2_S32LE.wav";
    const std::string OUT_PATH = "/data/aiss_output.wav";
    const std::string HUMAN_PATH = "/data/aiss_humanSound.wav";
    const std::string BKG_PATH = "/data/aiss_bkgSound.wav";
    constexpr uint32_t FRAME_LEN_MS = 20;
    constexpr uint32_t DEFAULT_SAMPLING_RATE = 48000;
    constexpr uint32_t DEFAULT_CHANNELS_IN = 2;
    constexpr uint32_t DEFAULT_CHANNELS_OUT = 4;
    constexpr uint32_t BYTES_PER_SAMPLE = 4;
    constexpr uint8_t NUM_EIGHT = 8;
    constexpr uint32_t NUM_THIRTY_SIX = 36;
    constexpr uint32_t NUM_SIXTEEN = 16;
    constexpr size_t NUM_FOUR = 4;
}

using namespace OHOS;
using namespace OHOS::AudioStandard;
using namespace OHOS::AudioStandard::AudioSuite;
using namespace std;

// WAV file header structure
#pragma pack(push, 1)
struct WavHeader {
    // RIFF chunk
    char riff[4];          // "RIFF"
    uint32_t chunkSize;    // Total file size - 8 bytes
    char wave[4];

    // fmt subchunk
    char fmt[4];
    uint32_t subchunk1Size;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;

    // data subchunk
    char data[4];
    uint32_t subchunk2Size;
};
#pragma pack(pop)

class WavFile {
public:
    // Constructor
    WavFile() : currentFrame(0)
    {
        // Initialize header with default values
        std::fill(reinterpret_cast<char*>(&header), reinterpret_cast<char*>(&header) + sizeof(header), 0);
        memcpy_s(header.riff, sizeof(header.riff), "RIFF", NUM_FOUR);
        memcpy_s(header.wave, sizeof(header.wave), "WAVE", NUM_FOUR);
        memcpy_s(header.fmt, sizeof(header.fmt), "fmt ", NUM_FOUR);
        memcpy_s(header.data, sizeof(header.data), "data", NUM_FOUR);
        header.subchunk1Size = NUM_SIXTEEN; // PCM format
        header.audioFormat = 1;    // PCM
    }

    // Read WAV from file
    bool ReadFromFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            printf("Failed to open file: %s\n", filename.c_str());
            return false;
        }

        // Read file header
        file.read(reinterpret_cast<char*>(&header), sizeof(WavHeader));

        // Check if valid WAV file
        if (!IsValidWavHeader(header)) {
            file.close();
            printf("Invalid WAV file: %s\n", filename.c_str());
            return false;
        }

        // Calculate data size and read audio data
        audioData.resize(header.subchunk2Size);
        file.read(reinterpret_cast<char*>(audioData.data()), header.subchunk2Size);

        file.close();
        currentFrame = 0;  // Reset frame position
        return true;
    }

    // Write WAV to file
    bool WriteToFile(const std::string& filename)
    {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            printf("Failed to create file: %s\n", filename.c_str());
            return false;
        }

        // Update size information in file header
        header.subchunk2Size = static_cast<uint32_t>(audioData.size());
        header.chunkSize = NUM_THIRTY_SIX + header.subchunk2Size;

        // Write file header
        file.write(reinterpret_cast<const char*>(&header), sizeof(WavHeader));

        // Write audio data
        file.write(reinterpret_cast<const char*>(audioData.data()), audioData.size());

        file.close();
        return true;
    }

    // Get bytes per frame (default 20ms)
    size_t GetFrameSize() const
    {
        // 20ms * sampleRate / 1000 * numChannels * (bitsPerSample / 8)
        size_t samplesPerFrame = static_cast<size_t>(std::round(header.sampleRate * 0.02));
        return samplesPerFrame * header.numChannels * (header.bitsPerSample / NUM_EIGHT);
    }

    // Get total number of frames
    size_t GetTotalFrames() const
    {
        size_t frameSize = GetFrameSize();
        if (frameSize == 0) {
            return 0;
        }
        return (audioData.size() + frameSize - 1) / frameSize; // Round up
    }

    // Read next frame data (pad with zeros if less than full frame)
    size_t ReadNextFrame(uint8_t *data)
    {
        size_t frameSize = GetFrameSize();
        if (frameSize == 0) {
            printf("Cannot calculate frame size, please set valid WAV header first\n");
            return 0;
        }
        std::vector<uint8_t> frame(frameSize, 0); // Initialize with zeros

        // Calculate starting position of current frame
        size_t startPos = currentFrame * frameSize;

        // If no data to read
        if (startPos >= audioData.size()) {
            return 0;
        }
        // Calculate actual readable data amount
        size_t bytesToRead = std::min(frameSize, audioData.size() - startPos);
        memcpy_s(data, bytesToRead, audioData.data() + startPos, bytesToRead);
        currentFrame++;
        return bytesToRead;
    }

    // Check if more frames available to read
    bool HasMoreFrames() const
    {
        return currentFrame < GetTotalFrames();
    }

    // Reset frame read position
    void ResetFramePosition()
    {
        currentFrame = 0;
    }

    // Write one frame of data
    void WriteFrame(const std::vector<uint8_t>& frame)
    {
        size_t frameSize = GetFrameSize();
        if (frameSize == 0) {
            printf("Cannot calculate frame size, please set valid WAV header first\n");
            return;
        }
        if (frame.size() != frameSize) {
            printf("Frame size mismatch, expected:%zu, actual:%lu\n", frameSize, frame.size());
            return;
        }

        // Append frame data to audio data
        audioData.insert(audioData.end(), frame.begin(), frame.end());
    }

    // Get WAV file header
    const WavHeader& GetHeader() const
    {
        return header;
    }

    // Set WAV file header
    void SetHeader(const WavHeader& newHeader)
    {
        if (!IsValidWavHeader(newHeader)) {
            printf("Invalid WAV file header\n");
            return;
        }
        header = newHeader;
    }

    // Get audio data
    const std::vector<uint8_t>& GetAudioData() const
    {
        return audioData;
    }

    // Set audio data
    void SetAudioData(const std::vector<uint8_t>& newData)
    {
        audioData = newData;
        currentFrame = 0; // Reset frame position
    }

    void SetChannels(uint16_t channel)
    {
        header.numChannels = channel;
    }

    // Print header information
    void PrintHeaderInfo() const
    {
        printf("RIFF: %s\n", std::string(header.riff, NUM_FOUR).c_str());
        printf("File size: %d bytes\n", header.chunkSize + NUM_EIGHT);
        printf("WAVE: %s\n", std::string(header.wave, NUM_FOUR).c_str());
        printf("Format: %s\n", std::string(header.fmt, NUM_FOUR).c_str());
        printf("Subchunk1 size: %d\n", header.subchunk1Size);
        printf("Audio format: %d (1 = PCM)\n", header.audioFormat);
        printf("Number of channels: %d\n", header.numChannels);
        printf("Sample rate: %d Hz\n", header.sampleRate);
        printf("Byte rate: %d bytes/second\n", header.byteRate);
        printf("Block align: %d bytes\n", header.blockAlign);
        printf("Bits per sample: %d bits\n", header.bitsPerSample);
        printf("Data identifier: %s\n", std::string(header.data, NUM_FOUR).c_str());
        printf("Data size: %d bytes\n", header.subchunk2Size);
        printf("Frame size: %zu bytes\n", GetFrameSize());
        printf("Total frames: %zu\n", GetTotalFrames());
    }

private:
    // Check if header is valid WAV file
    bool IsValidWavHeader(const WavHeader& hdr)
    {
        return (std::string(hdr.riff, NUM_FOUR) == "RIFF" &&
            std::string(hdr.wave, NUM_FOUR) == "WAVE" &&
            std::string(hdr.fmt, NUM_FOUR) == "fmt " &&
            std::string(hdr.data, NUM_FOUR) == "data");
    }
    WavHeader header;
    std::vector<uint8_t> audioData;
    size_t currentFrame; // Current frame position
};

int main()
{
    NodeCapability nc;
    nc.soName = "libaudio_aiss_intergration.z.so";
    nc.soPath = "/system/lib64/";
    std::shared_ptr<AudioSuiteAlgoInterface> aissAlgoImpl =
        AudioSuiteAlgoInterface::CreateAlgoInterface(AlgoType::AUDIO_NODE_TYPE_AUDIO_SEPARATION, nc);
    int32_t retValue = aissAlgoImpl->Init();
    if (retValue != SUCCESS) {
        printf("InitAlgorithm failed, retValue: %d", retValue);
        return -1;
    }
    const uint32_t byteSizePerFrameIn = DEFAULT_SAMPLING_RATE * FRAME_LEN_MS /
        1000 * DEFAULT_CHANNELS_IN * BYTES_PER_SAMPLE;
    const uint32_t byteSizePerFrameOut = DEFAULT_SAMPLING_RATE * FRAME_LEN_MS /
        1000 * DEFAULT_CHANNELS_OUT * BYTES_PER_SAMPLE;
    std::vector<uint8_t> inBuffer(byteSizePerFrameIn);
    std::vector<uint8_t> outBuffer(byteSizePerFrameOut);
    std::vector<uint8_t> humanSoundBuffer(byteSizePerFrameIn);
    std::vector<uint8_t> bkgSoundBuffer(byteSizePerFrameIn);
    WavFile wavIn;
    WavFile wavOut;
    wavIn.ReadFromFile(INPUT_PATH);
    wavIn.PrintHeaderInfo();
    wavOut.SetHeader(wavIn.GetHeader());
    wavOut.SetChannels(DEFAULT_CHANNELS_OUT);
    std::vector<uint8_t *> tmpin;
    std::vector<uint8_t *> tmpout;
    while (wavIn.HasMoreFrames()) {
        size_t bytesRead = wavIn.ReadNextFrame(inBuffer.data());
        if (bytesRead == 0) {
            break;
        }
        if (bytesRead < byteSizePerFrameIn) {
            memset_s(inBuffer.data() + bytesRead, byteSizePerFrameIn - bytesRead, 0, byteSizePerFrameIn - bytesRead);
        }
        tmpin.clear();
        tmpout.clear();
        tmpin.emplace_back(reinterpret_cast<uint8_t *>(inBuffer.data()));
        tmpout.emplace_back(reinterpret_cast<uint8_t *>(outBuffer.data()));
        tmpout.emplace_back(reinterpret_cast<uint8_t *>(humanSoundBuffer.data()));
        tmpout.emplace_back(reinterpret_cast<uint8_t *>(bkgSoundBuffer.data()));
        aissAlgoImpl->Apply(tmpin, tmpout);
        wavOut.WriteFrame(outBuffer);
    }
    wavOut.WriteToFile(OUT_PATH);
    wavOut.PrintHeaderInfo();
    aissAlgoImpl->Deinit();
    return 0;
}