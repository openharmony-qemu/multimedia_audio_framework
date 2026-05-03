/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <getopt.h>
#include "audio_pcm_process.h"

namespace {

constexpr int MAX_FILE_SIZE = 100 * 1024 * 1024;
constexpr int CHUNK_DURATION_MS = 20;
constexpr int MS_PER_SECOND = 1000;

struct InputFileInfo {
    std::string path;
    AudioFormatConfig config;
    std::ifstream file;
    uint32_t sampleSize;
};

struct CommandLineArgs {
    std::vector<std::string> inputPaths;
    std::string outputPath;
    bool valid;
};

uint32_t GetSampleSize(AudioBitDepth bitDepth)
{
    switch (bitDepth) {
        case AudioBit16Int: return 2;
        case AudioBit24Int: return 3;
        case AudioBit32Int: return 4;
        case AudioBit32Float: return 4;
        default: return 0;
    }
}

void PrintUsage()
{
    std::cout << "Usage:" << std::endl;
    std::cout << "  audio_pcm_process_module_test -i <input.pcm> [-i <input2.pcm>...] -o <output.pcm>" << std::endl;
    std::cout << "Filename format: <sampleRate>_<channelCount>ch_<bitDepth>bit.pcm" << std::endl;
}

CommandLineArgs ParseCommandLine(int argc, char* argv[])
{
    CommandLineArgs args;
    args.valid = false;

    int opt;
    while ((opt = getopt(argc, argv, "i:o:h")) != -1) {
        switch (opt) {
            case 'i': args.inputPaths.push_back(optarg); break;
            case 'o': args.outputPath = optarg; break;
            case 'h': PrintUsage(); return args;
            default: PrintUsage(); return args;
        }
    }

    if (!args.inputPaths.empty() && !args.outputPath.empty()) {
        args.valid = true;
    } else {
        std::cerr << "Error: Missing required parameters" << std::endl;
        PrintUsage();
    }
    return args;
}

bool ParseFileName(const std::string& fileName, AudioFormatConfig& config)
{
    std::regex pattern("(\\d+)_(\\d+)ch_(\\d+)bit\\.pcm");
    std::smatch match;

    if (!std::regex_search(fileName, match, pattern)) {
        std::cerr << "Invalid filename format: " << fileName << std::endl;
        return false;
    }

    config.sampleRate = std::stoi(match[1].str());

    int channelCount = std::stoi(match[2].str());
    if (channelCount == 1) config.channel = AudioChMono;
    else if (channelCount == 2) config.channel = AudioChStereo;
    else { std::cerr << "Unsupported channel count" << std::endl; return false; }

    int bitDepth = std::stoi(match[3].str());
    if (bitDepth == 16) config.bitDepth = AudioBit16Int;
    else if (bitDepth == 24) config.bitDepth = AudioBit24Int;
    else if (bitDepth == 32) config.bitDepth = AudioBit32Int;
    else if (bitDepth == 33) config.bitDepth = AudioBit32Float;
    else { std::cerr << "Unsupported bit depth" << std::endl; return false; }

    return true;
}

std::string ExtractFileName(const std::string& path)
{
    size_t pos = path.find_last_of("/\\");
    return (pos == std::string::npos) ? path : path.substr(pos + 1);
}

bool OpenInputFile(InputFileInfo& info)
{
    info.file.open(info.path, std::ios::binary);
    if (!info.file.is_open()) {
        std::cerr << "File not found: " << info.path << std::endl;
        return false;
    }

    info.file.seekg(0, std::ios::end);
    size_t fileSize = info.file.tellg();
    info.file.seekg(0, std::ios::beg);

    if (fileSize > MAX_FILE_SIZE) {
        std::cerr << "File too large" << std::endl;
        return false;
    }
    info.sampleSize = GetSampleSize(info.config.bitDepth);
    return true;
}

uint32_t CalculateChunkBytes(const AudioFormatConfig& config)
{
    uint32_t sampleSize = GetSampleSize(config.bitDepth);
    uint32_t channelCount = (config.channel == AudioChMono) ? 1 : 2;
    return config.sampleRate * channelCount * sampleSize * CHUNK_DURATION_MS / MS_PER_SECOND;
}

bool ReadChunk(std::ifstream& file, std::vector<uint8_t>& buffer, uint32_t chunkBytes)
{
    buffer.resize(chunkBytes);
    file.read(reinterpret_cast<char*>(buffer.data()), chunkBytes);
    size_t bytesRead = file.gcount();

    if (bytesRead == 0) return false;
    if (bytesRead < chunkBytes) buffer.resize(bytesRead);
    return true;
}

bool WriteChunk(std::ofstream& outFile, const uint8_t* data, size_t size, bool isFirstChunk,
    const std::string& path)
{
    if (isFirstChunk) {
        outFile.open(path, std::ios::binary);
        if (!outFile.is_open()) {
            std::cerr << "Failed to create file: " << path << std::endl;
            return false;
        }
    }
    outFile.write(reinterpret_cast<const char*>(data), size);
    return outFile.good();
}

void PrintConfig(const AudioFormatConfig& config, const std::string& label)
{
    int channelCount = (config.channel == AudioChMono) ? 1 : 2;
    std::cout << label << ": " << config.sampleRate << "Hz, " << channelCount << "ch, "
              << static_cast<int>(config.bitDepth) << "bit" << std::endl;
}

bool AreConfigsEqual(const AudioFormatConfig& a, const AudioFormatConfig& b)
{
    return a.sampleRate == b.sampleRate && a.channel == b.channel && a.bitDepth == b.bitDepth;
}

int DoFormatConversion(InputFileInfo& input, const AudioFormatConfig& outputConfig,
    const std::string& outputPath)
{
    PrintConfig(input.config, "Input");
    PrintConfig(outputConfig, "Output");

    AudioConverterHandle handle = AudioConverterCreate(&input.config, &outputConfig);
    if (handle == nullptr) {
        std::cerr << "Error: Failed to create converter" << std::endl;
        return 1;
    }

    uint32_t chunkBytes = CalculateChunkBytes(input.config);
    std::vector<uint8_t> inBuffer;
    std::ofstream outFile;
    bool isFirstChunk = true;
    int totalSamples = 0;

    std::cout << "Processing with " << CHUNK_DURATION_MS << "ms chunks..." << std::endl;

    while (ReadChunk(input.file, inBuffer, chunkBytes)) {
        int inSampleCount = inBuffer.size() / input.sampleSize;
        CAudioConvertResult result = AudioConverterProcess(handle, inBuffer.data(), inSampleCount);

        if (result.errCode != 0) {
            std::cerr << "Error: Process failed, code=" << result.errCode << std::endl;
            AudioConverterDestroy(handle);
            return 1;
        }

        uint32_t outSampleSize = GetSampleSize(outputConfig.bitDepth);
        if (!WriteChunk(outFile, result.outData, result.outSampleCount * outSampleSize, isFirstChunk, outputPath)) {
            AudioConverterDestroy(handle);
            return 1;
        }
        isFirstChunk = false;
        totalSamples += result.outSampleCount;
    }

    AudioConverterDestroy(handle);
    std::cout << "Total output samples: " << totalSamples << std::endl;
    std::cout << "Format conversion completed!" << std::endl;
    return 0;
}

int DoAudioMixing(std::vector<InputFileInfo>& inputs, const AudioFormatConfig& outputConfig,
    const std::string& outputPath)
{
    for (size_t i = 0; i < inputs.size(); ++i) {
        PrintConfig(inputs[i].config, "Input " + std::to_string(i + 1));
    }
    PrintConfig(outputConfig, "Output");

    for (size_t i = 1; i < inputs.size(); ++i) {
        if (!AreConfigsEqual(inputs[i].config, inputs[0].config)) {
            std::cerr << "Error: Mix inputs format mismatch" << std::endl;
            return 1;
        }
    }
    if (!AreConfigsEqual(inputs[0].config, outputConfig)) {
        std::cerr << "Error: Output format must match input" << std::endl;
        return 1;
    }

    AudioMixHandle handle = AudioMixerCreate(&inputs[0].config);
    if (handle == nullptr) {
        std::cerr << "Error: Failed to create mixer" << std::endl;
        return 1;
    }

    uint32_t chunkBytes = CalculateChunkBytes(inputs[0].config);
    std::vector<std::vector<uint8_t>> inBuffers(inputs.size());
    std::ofstream outFile;
    bool isFirstChunk = true;
    int totalSamples = 0;

    std::cout << "Processing with " << CHUNK_DURATION_MS << "ms chunks..." << std::endl;

    while (true) {
        bool anyData = false;
        size_t minSize = chunkBytes;

        for (size_t i = 0; i < inputs.size(); ++i) {
            if (ReadChunk(inputs[i].file, inBuffers[i], chunkBytes)) {
                anyData = true;
                if (inBuffers[i].size() < minSize) minSize = inBuffers[i].size();
            } else {
                inBuffers[i].clear();
            }
        }
        if (!anyData) break;

        std::vector<CAudioMixStream> streams(inputs.size());
        int validCount = 0;
        for (size_t i = 0; i < inputs.size(); ++i) {
            if (inBuffers[i].size() >= minSize && inBuffers[i].size() > 0) {
                streams[validCount].data = inBuffers[i].data();
                streams[validCount].sampleCount = minSize / inputs[0].sampleSize;
                streams[validCount].volume = 1.0f / inputs.size();
                validCount++;
            }
        }
        if (validCount == 0) break;

        AudioMixResult result = AudioMixerProcess(handle, streams.data(), validCount);
        if (result.errCode != 0) {
            std::cerr << "Error: Mix failed, code=" << result.errCode << std::endl;
            AudioMixerDestroy(handle);
            return 1;
        }

        if (!WriteChunk(outFile, result.outData, result.outSamplerCount * inputs[0].sampleSize,
            isFirstChunk, outputPath)) {
            AudioMixerDestroy(handle);
            return 1;
        }
        isFirstChunk = false;
        totalSamples += result.outSamplerCount;
    }

    AudioMixerDestroy(handle);
    std::cout << "Total output samples: " << totalSamples << std::endl;
    std::cout << "Audio mixing completed!" << std::endl;
    return 0;
}

}

int main(int argc, char* argv[])
{
    CommandLineArgs args = ParseCommandLine(argc, argv);
    if (!args.valid) return 1;

    std::vector<InputFileInfo> inputs;
    for (const auto& path : args.inputPaths) {
        InputFileInfo info;
        info.path = path;
        if (!ParseFileName(ExtractFileName(path), info.config)) return 1;
        if (!OpenInputFile(info)) return 1;
        inputs.push_back(std::move(info));
    }

    AudioFormatConfig outputConfig;
    if (!ParseFileName(ExtractFileName(args.outputPath), outputConfig)) return 1;

    std::cout << "=== Audio PCM Process Module Test ===" << std::endl;

    int ret = 0;
    if (inputs.size() == 1) {
        std::cout << "Operation: Format Conversion" << std::endl;
        ret = DoFormatConversion(inputs[0], outputConfig, args.outputPath);
    } else {
        std::cout << "Operation: Audio Mixing (" << inputs.size() << " streams)" << std::endl;
        ret = DoAudioMixing(inputs, outputConfig, args.outputPath);
    }

    for (auto& info : inputs) {
        if (info.file.is_open()) info.file.close();
    }
    return ret;
}