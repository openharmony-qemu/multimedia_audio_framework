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

#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <gtest/gtest.h>
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_unittest_tools.h"
#include "audio_suite_nr_algo_interface_impl.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;

namespace {
static std::string g_inputPcmFilePath = "/data/audiosuite/nr/ainr_input_16000_1_S16LE.pcm";
static std::string g_targetPcmFilePath = "/data/audiosuite/nr/ainr_target_16000_1_S16LE.pcm";
static std::string g_outputPcmFilePath = "/data/audiosuite/nr/ainr_output_16000_1_S16LE.pcm";

class AudioSuiteNrAlgoInterfaceImplUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void){};
    static void TearDownTestCase(void){};
    void SetUp(void);
    void TearDown(void);
private:
    NodeCapability nc;
};

void AudioSuiteNrAlgoInterfaceImplUnitTest::SetUp(void)
{
    nc.soName = "libimedia_vqe_ainr.z.so";
    nc.soPath = "/system/lib64/";
    std::filesystem::remove(g_outputPcmFilePath);
}

void AudioSuiteNrAlgoInterfaceImplUnitTest::TearDown(void)
{}

HWTEST_F(AudioSuiteNrAlgoInterfaceImplUnitTest, TestNrAlgoInitAndDeinit_001, TestSize.Level0)
{
    AudioSuiteNrAlgoInterfaceImpl nrAlgo(nc);
    EXPECT_EQ(nrAlgo.Init(), 0);
    EXPECT_EQ(nrAlgo.Deinit(), 0);

    EXPECT_EQ(nrAlgo.Deinit(), 0);

    EXPECT_EQ(nrAlgo.Init(), 0);
    EXPECT_EQ(nrAlgo.Deinit(), 0);
}

HWTEST_F(AudioSuiteNrAlgoInterfaceImplUnitTest, TestNrAlgoApplyOK_001, TestSize.Level0)
{
    AudioSuiteNrAlgoInterfaceImpl nrAlgo(nc);
    std::vector<uint8_t *> audioInputs(1);
    std::vector<uint8_t *> audioOutputs(1);
    std::vector<int16_t> dataIn(AUDIO_AINR_PCM_16K_FRAME_LEN, 0);
    std::vector<int16_t> dataOut(AUDIO_AINR_PCM_16K_FRAME_LEN, 0);

    EXPECT_EQ(nrAlgo.Init(), 0);

    audioInputs[0] = reinterpret_cast<uint8_t *>(dataIn.data());
    audioOutputs[0] = reinterpret_cast<uint8_t *>(dataOut.data());
    EXPECT_EQ(nrAlgo.Apply(audioInputs, audioOutputs), 0);

    EXPECT_EQ(nrAlgo.Deinit(), 0);
}

HWTEST_F(AudioSuiteNrAlgoInterfaceImplUnitTest, TestNrAlgoApplyNOK_002, TestSize.Level0)
{
    AudioSuiteNrAlgoInterfaceImpl nrAlgo(nc);
    std::vector<uint8_t *> audioInputs(1);
    std::vector<uint8_t *> audioOutputs(1);
    std::vector<int16_t> dataIn(AUDIO_AINR_PCM_16K_FRAME_LEN, 0);
    std::vector<int16_t> dataOut(AUDIO_AINR_PCM_16K_FRAME_LEN, 0);
    audioInputs[0] = reinterpret_cast<uint8_t *>(dataIn.data());
    audioOutputs[0] = reinterpret_cast<uint8_t *>(dataOut.data());

    EXPECT_EQ(nrAlgo.Apply(audioInputs, audioOutputs), ERROR);
}

HWTEST_F(AudioSuiteNrAlgoInterfaceImplUnitTest, TestNrAlgoApplyPcmFile_003, TestSize.Level0)
{
    AudioSuiteNrAlgoInterfaceImpl nrAlgo(nc);
    std::vector<uint8_t *> audioInputs(1);
    std::vector<uint8_t *> audioOutputs(1);
    size_t frameSize = AUDIO_AINR_PCM_16K_FRAME_LEN * sizeof(int16_t);

    EXPECT_EQ(nrAlgo.Init(), 0);

    // Read inputfile
    std::ifstream ifs(g_inputPcmFilePath, std::ios::binary);
    ifs.seekg(0, std::ios::end);
    size_t fileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    // Padding zero then send to algo
    size_t zeroPaddingSize = (fileSize % frameSize == 0) ? 0 : (frameSize - fileSize % frameSize);
    size_t fileBufferSize = fileSize + zeroPaddingSize;
    std::vector<int16_t> inputfileBuffer(fileBufferSize / sizeof(int16_t), 0);  // 16-bit PCM file
    ifs.read(reinterpret_cast<char *>(inputfileBuffer.data()), fileSize);
    ifs.close();

    // Apply algorithm
    std::vector<uint8_t> outputfileBuffer(fileBufferSize);
    uint8_t *frameInputPtr = reinterpret_cast<uint8_t *>(inputfileBuffer.data());
    uint8_t *frameOutputPtr = outputfileBuffer.data();
    for (int32_t i = 0; i + frameSize <= fileBufferSize; i += frameSize) {
        audioInputs[0] = frameInputPtr;
        audioOutputs[0] = frameOutputPtr;
        ASSERT_EQ(nrAlgo.Apply(audioInputs, audioOutputs), 0);
        frameInputPtr += frameSize;
        frameOutputPtr += frameSize;
    }

    // write to outputfile
    ASSERT_EQ(CreateOutputPcmFile(g_outputPcmFilePath), true);
    bool isWriteFileSucc = WritePcmFile(g_outputPcmFilePath, outputfileBuffer.data(), fileSize);
    ASSERT_EQ(isWriteFileSucc, true);

    // compare result
    EXPECT_EQ(IsFilesEqual(g_outputPcmFilePath, g_targetPcmFilePath), true);

    EXPECT_EQ(nrAlgo.Deinit(), 0);
}

}  // namespace
