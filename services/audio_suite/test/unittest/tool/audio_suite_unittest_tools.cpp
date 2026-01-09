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

#include "audio_suite_unittest_tools.h"
#include "audio_suite_nr_node.h"
#include "audio_suite_env_node.h"
#include "audio_suite_eq_node.h"
#include "audio_suite_soundfield_node.h"
#include "audio_suite_voice_beautifier_node.h"
#include "audio_suite_log.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

template <typename T>
int32_t TestEffectNodeSignalProcess(std::shared_ptr<T> node,
    const std::vector<AudioSuitePcmBuffer *> &inputs, const std::string &inputFile, const std::string &outputFile,
    const std::string &targetFile)
{
    size_t frameSizeInput = inputs[0]->GetDataSize();
    // Preset condition: Algorithm input and output formats must be identical;
    // otherwise, output length needs to be obtained based on node information.
    size_t frameSizeOutput = frameSizeInput;
    uint8_t *inputData = inputs[0]->GetPcmData();

    // Read input file
    std::ifstream ifs(inputFile, std::ios::binary);
    CHECK_AND_RETURN_RET_LOG(ifs.is_open(), ERROR, "Failed to open input file: %{public}s", inputFile.c_str());
    ifs.seekg(0, std::ios::end);
    size_t inputFileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    // Padding zero then send to apply
    CHECK_AND_RETURN_RET_LOG(frameSizeInput != 0, ERROR, "frameSizeInput Division by zero error");
    size_t zeroPaddingSize =
        (inputFileSize % frameSizeInput == 0) ? 0 : (frameSizeInput - inputFileSize % frameSizeInput);
    size_t inputFileBufferSize = inputFileSize + zeroPaddingSize;
    std::vector<uint8_t> inputfileBuffer(inputFileBufferSize, 0);  // PCM data padding 0
    ifs.read(reinterpret_cast<char *>(inputfileBuffer.data()), inputFileSize);
    ifs.close();

    // apply data
    CHECK_AND_RETURN_RET_LOG(frameSizeInput != 0, ERROR, "frameSizeInput Division by zero error");
    size_t outputFileBufferSize = inputFileBufferSize * frameSizeOutput / frameSizeInput;
    std::vector<uint8_t> outputfileBuffer(outputFileBufferSize);
    uint8_t *readPtr = inputfileBuffer.data();
    uint8_t *writePtr = outputfileBuffer.data();
    CHECK_AND_RETURN_RET(frameSizeInput != 0, ERROR);
    int32_t frames = inputFileBufferSize / frameSizeInput;
    for (int32_t i = 0; i < frames; i++) {
        memcpy_s(inputData, frameSizeInput, readPtr, frameSizeInput);
        AudioSuitePcmBuffer *out = node->SignalProcess(inputs);
        memcpy_s(writePtr, frameSizeOutput, out->GetPcmData(), frameSizeOutput);

        readPtr += frameSizeInput;
        writePtr += frameSizeOutput;
    }

    // write to output file
    bool isCreateFileSucc = CreateOutputPcmFile(outputFile);
    CHECK_AND_RETURN_RET_LOG(isCreateFileSucc, ERROR, "Failed to create output file: %{public}s", outputFile.c_str());
    bool isWriteFileSucc = WritePcmFile(outputFile, outputfileBuffer.data(), outputFileBufferSize);
    CHECK_AND_RETURN_RET_LOG(isWriteFileSucc, ERROR, "Failed to write data to file: %{public}s", outputFile.c_str());

    // compare the output file with target file
    bool isFileEqual = IsFilesEqual(outputFile, targetFile);
    CHECK_AND_RETURN_RET_LOG(isFileEqual, ERROR, "Compare outputFile and targetFile not equal");

    return SUCCESS;
}

template int32_t TestEffectNodeSignalProcess(std::shared_ptr<AudioSuiteNrNode> node,
    const std::vector<AudioSuitePcmBuffer *> &inputs, const std::string &inputFile, const std::string &outputFile,
    const std::string &targetFile);
template int32_t TestEffectNodeSignalProcess(std::shared_ptr<AudioSuiteEnvNode> node,
    const std::vector<AudioSuitePcmBuffer *> &inputs, const std::string &inputFile, const std::string &outputFile,
    const std::string &targetFile);
template int32_t TestEffectNodeSignalProcess(std::shared_ptr<AudioSuiteEqNode> node,
    const std::vector<AudioSuitePcmBuffer *> &inputs, const std::string &inputFile, const std::string &outputFile,
    const std::string &targetFile);
template int32_t TestEffectNodeSignalProcess(std::shared_ptr<AudioSuiteSoundFieldNode> node,
    const std::vector<AudioSuitePcmBuffer *> &inputs, const std::string &inputFile, const std::string &outputFile,
    const std::string &targetFile);
template int32_t TestEffectNodeSignalProcess(std::shared_ptr<AudioSuiteVoiceBeautifierNode> node,
    const std::vector<AudioSuitePcmBuffer *> &inputs, const std::string &inputFile, const std::string &outputFile,
    const std::string &targetFile);

bool CreateOutputPcmFile(const std::string &filename)
{
    if (std::filesystem::exists(filename)) {
        std::filesystem::remove(filename);
    }

    std::ofstream ofs;
    ofs.open(filename, std::ios::out | std::ios::trunc);
    CHECK_AND_RETURN_RET_LOG(ofs.is_open(), false, "Failed to open output file: %{public}s", filename.c_str());
    ofs.close();
    return true;
}

bool WritePcmFile(const std::string &filename, const uint8_t *data, size_t dataSize)
{
    std::ofstream file(filename, std::ios::binary | std::ios::app);
    if (!file.is_open()) {
        return false;
    }

    if (!file.write(reinterpret_cast<const char *>(data), dataSize)) {
        file.close();
        return false;
    }

    file.close();
    return true;
}

bool IsFilesEqual(const std::string &filename1, const std::string &filename2)
{
    std::ifstream file1(filename1, std::ios::binary);
    std::ifstream file2(filename2, std::ios::binary);

    if (!file1.is_open() || !file2.is_open()) {
        return false;
    }

    file1.seekg(0, std::ios::end);
    file2.seekg(0, std::ios::end);
    if (file1.tellg() != file2.tellg()) {
        return false;
    }

    file1.seekg(0, std::ios::beg);
    file2.seekg(0, std::ios::beg);

    char buffer1[4096];
    char buffer2[4096];
    size_t bytesRead;

    do {
        file1.read(buffer1, sizeof(buffer1));
        file2.read(buffer2, sizeof(buffer2));
        bytesRead = file1.gcount();
        if (bytesRead != file2.gcount()) {
            return false;
        }
        if (std::memcmp(buffer1, buffer2, bytesRead) != 0) {
            return false;
        }
    } while (bytesRead > 0);

    return true;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS