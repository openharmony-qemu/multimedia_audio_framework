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

// WAV 文件头结构体
#pragma pack(push, 1)
struct WavHeader {
    // RIFF 块
    char riff[4];          // "RIFF"
    uint32_t chunkSize;    // 文件总大小 - 8 字节
    char wave[4];

    // fmt 子块
    char fmt[4];
    uint32_t subchunk1Size;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;

    // data 子块
    char data[4];
    uint32_t subchunk2Size;
};
#pragma pack(pop)

class WavFile {
public:
    // 构造函数
    WavFile() : currentFrame(0)
    {
        // 初始化头部为默认值
        std::fill(reinterpret_cast<char*>(&header), reinterpret_cast<char*>(&header) + sizeof(header), 0);
        memcpy_s(header.riff, sizeof(header.riff), "RIFF", NUM_FOUR);
        memcpy_s(header.wave, sizeof(header.wave), "WAVE", NUM_FOUR);
        memcpy_s(header.fmt, sizeof(header.fmt), "fmt ", NUM_FOUR);
        memcpy_s(header.data, sizeof(header.data), "data", NUM_FOUR);
        header.subchunk1Size = NUM_SIXTEEN; // PCM 格式
        header.audioFormat = 1;    // PCM
    }

    // 从文件读取 WAV
    bool ReadFromFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            printf("无法打开文件: %s\n", filename.c_str());
            return false;
        }

        // 读取文件头
        file.read(reinterpret_cast<char*>(&header), sizeof(WavHeader));

        // 检查是否是有效地 WAV 文件
        if (!IsValidWavHeader(header)) {
            file.close();
            printf("无效的 WAV 文件: %s\n", filename.c_str());
            return false;
        }

        // 计算数据大小并读取音频数据
        audioData.resize(header.subchunk2Size);
        file.read(reinterpret_cast<char*>(audioData.data()), header.subchunk2Size);

        file.close();
        currentFrame = 0;  // 重置帧位置
        return true;
    }

    // 写入 WAV 到文件
    bool WriteToFile(const std::string& filename)
    {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            printf("无法创建文件: %s\n", filename.c_str());
            return false;
        }

        // 更新文件头中的大小信息
        header.subchunk2Size = static_cast<uint32_t>(audioData.size());
        header.chunkSize = NUM_THIRTY_SIX + header.subchunk2Size;

        // 写入文件头
        file.write(reinterpret_cast<const char*>(&header), sizeof(WavHeader));

        // 写入音频数据
        file.write(reinterpret_cast<const char*>(audioData.data()), audioData.size());

        file.close();
        return true;
    }

    // 获取每帧的字节数(默认20ms)
    size_t GetFrameSize() const
    {
        // 20ms * sampleRate / 1000 * numChannels * (bitsPerSample / 8)
        size_t samplesPerFrame = static_cast<size_t>(std::round(header.sampleRate * 0.02));
        return samplesPerFrame * header.numChannels * (header.bitsPerSample / NUM_EIGHT);
    }

    // 获取总帧数
    size_t GetTotalFrames() const
    {
        size_t frameSize = GetFrameSize();
        if (frameSize == 0) {
            return 0;
        }
        return (audioData.size() + frameSize - 1) / frameSize; // 向上取整
    }

    // 读取下一帧数据(如果不足一帧,用0补齐)
    size_t ReadNextFrame(uint8_t *data)
    {
        size_t frameSize = GetFrameSize();
        if (frameSize == 0) {
            printf("无法计算帧大小,请先设置有效的WAV头信息\n");
            return 0;
        }
        std::vector<uint8_t> frame(frameSize, 0); // 初始化为0

        // 计算当前帧的起始位置
        size_t startPos = currentFrame * frameSize;

        // 如果无数据可读
        if (startPos >= audioData.size()) {
            return 0;
        }
        // 计算实际可读取的数据量
        size_t bytesToRead = std::min(frameSize, audioData.size() - startPos);
        memcpy_s(data, bytesToRead, audioData.data() + startPos, bytesToRead);
        currentFrame++;
        return bytesToRead;
    }

    // 检查是否还有更多帧可读
    bool HasMoreFrames() const
    {
        return currentFrame < GetTotalFrames();
    }

    // 重置帧读取位置
    void ResetFramePosition()
    {
        currentFrame = 0;
    }

    // 写入一帧数据
    void WriteFrame(const std::vector<uint8_t>& frame)
    {
        size_t frameSize = GetFrameSize();
        if (frameSize == 0) {
            printf("无法计算帧大小,请先设置有效的WAV头信息\n");
            return;
        }
        if (frame.size() != frameSize) {
            printf("帧大小不匹配，预期大小:%zu,实际打下:%lu\n", frameSize, frame.size());
            return;
        }

        // 将帧数据追加到音频数据中
        audioData.insert(audioData.end(), frame.begin(), frame.end());
    }

    // 获取 WAV 文件头
    const WavHeader& GetHeader() const
    {
        return header;
    }

    // 设置 WAV 文件头
    void SetHeader(const WavHeader& newHeader)
    {
        if (!IsValidWavHeader(newHeader)) {
            printf("无效的 WAV 文件头\n");
            return;
        }
        header = newHeader;
    }

    // 获取音频数据
    const std::vector<uint8_t>& GetAudioData() const
    {
        return audioData;
    }

    // 设置音频数据
    void SetAudioData(const std::vector<uint8_t>& newData)
    {
        audioData = newData;
        currentFrame = 0; // 重置帧位置
    }

    void SetChannels(uint16_t channel)
    {
        header.numChannels = channel;
    }

    // 打印文件头信息
    void PrintHeaderInfo() const
    {
        printf("RIFF: %s\n", std::string(header.riff, NUM_FOUR).c_str());
        printf("文件大小: %d字节\n", header.chunkSize + NUM_EIGHT);
        printf("WAVE: %s\n", std::string(header.wave, NUM_FOUR).c_str());
        printf("格式: %s\n", std::string(header.fmt, NUM_FOUR).c_str());
        printf("子块1大小: %d\n", header.subchunk1Size);
        printf("音频格式: %d (1 = PCM)\n", header.audioFormat);
        printf("声道数: %d\n", header.numChannels);
        printf("采样率: %d Hz\n", header.sampleRate);
        printf("字节率: %d 字节/秒\n", header.byteRate);
        printf("块对齐: %d 字节\n", header.blockAlign);
        printf("每样本位数: %d 位\n", header.bitsPerSample);
        printf("数据标识: %s\n", std::string(header.data, NUM_FOUR).c_str());
        printf("数据大小: %d 字节\n", header.subchunk2Size);
        printf("每帧大小: %zu 字节\n", GetFrameSize());
        printf("总帧数: %zu\n", GetTotalFrames());
    }

private:
    // 检查文件头是否是有效的 WAV 文件
    bool IsValidWavHeader(const WavHeader& hdr)
    {
        return (std::string(hdr.riff, NUM_FOUR) == "RIFF" &&
            std::string(hdr.wave, NUM_FOUR) == "WAVE" &&
            std::string(hdr.fmt, NUM_FOUR) == "fmt " &&
            std::string(hdr.data, NUM_FOUR) == "data");
    }
    WavHeader header;
    std::vector<uint8_t> audioData;
    size_t currentFrame; // 当前帧位置
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