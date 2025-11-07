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
#include "audio_suite_soundfield_algo_interface_impl.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;

namespace {
static std::string g_inputfile = "/data/audiosuite/soundfield_input_48000_2_S16LE.pcm";
static std::string g_targetfile = "/data/audiosuite/soundfield_target_48000_2_S16LE.pcm";
static std::string g_outputfile = "/data/audiosuite/soundfield_output_48000_2_S16LE.pcm";

class AudioSuiteSoundFieldAlgoInterfaceImplUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void){};
    static void TearDownTestCase(void){};
    void SetUp(void);
    void TearDown(void);

private:
    NodeCapability nc;
};

void AudioSuiteSoundFieldAlgoInterfaceImplUnitTest::SetUp(void)
{
    nc.soName = "libimedia_sws.z.so";
    nc.soPath = "/system/lib64/";
    std::filesystem::remove(g_outputfile);
}

void AudioSuiteSoundFieldAlgoInterfaceImplUnitTest::TearDown(void)
{}

HWTEST_F(AudioSuiteSoundFieldAlgoInterfaceImplUnitTest, TestInitAndDeinit_001, TestSize.Level0)
{
    AudioSuiteSoundFieldAlgoInterfaceImpl soundFieldAlgo(nc);
    ASSERT_EQ(soundFieldAlgo.Init(), 0);
    EXPECT_EQ(soundFieldAlgo.Deinit(), 0);

    EXPECT_EQ(soundFieldAlgo.Deinit(), 0);

    ASSERT_EQ(soundFieldAlgo.Init(), 0);
    EXPECT_EQ(soundFieldAlgo.Deinit(), 0);
}

HWTEST_F(AudioSuiteSoundFieldAlgoInterfaceImplUnitTest, TestApplyOK_001, TestSize.Level0)
{
    AudioSuiteSoundFieldAlgoInterfaceImpl soundFieldAlgo(nc);
    std::vector<uint8_t *> pcmInputs(1);
    std::vector<uint8_t *> pcmOutputs(1);
    std::vector<int16_t> dataIn(SOUNDFIELD_ALGO_FRAME_LEN, 0);
    std::vector<int16_t> dataOut(SOUNDFIELD_ALGO_FRAME_LEN, 0);

    ASSERT_EQ(soundFieldAlgo.Init(), 0);

    pcmInputs[0] = reinterpret_cast<uint8_t *>(dataIn.data());
    pcmOutputs[0] = reinterpret_cast<uint8_t *>(dataOut.data());
    EXPECT_EQ(soundFieldAlgo.Apply(pcmInputs, pcmOutputs), 0);

    EXPECT_EQ(soundFieldAlgo.Deinit(), 0);
}

HWTEST_F(AudioSuiteSoundFieldAlgoInterfaceImplUnitTest, TestWithoutInitDoApplyNOK_002, TestSize.Level0)
{
    AudioSuiteSoundFieldAlgoInterfaceImpl soundFieldAlgo(nc);
    std::vector<uint8_t *> pcmInputs(1);
    std::vector<uint8_t *> pcmOutputs(1);
    std::vector<int16_t> dataIn(SOUNDFIELD_ALGO_FRAME_LEN, 0);
    std::vector<int16_t> dataOut(SOUNDFIELD_ALGO_FRAME_LEN, 0);
    pcmInputs[0] = reinterpret_cast<uint8_t *>(dataIn.data());
    pcmOutputs[0] = reinterpret_cast<uint8_t *>(dataOut.data());

    EXPECT_EQ(soundFieldAlgo.Apply(pcmInputs, pcmOutputs), ERROR);
}

HWTEST_F(AudioSuiteSoundFieldAlgoInterfaceImplUnitTest, TestSetAndGetParameter_001, TestSize.Level0)
{
    AudioSuiteSoundFieldAlgoInterfaceImpl soundFieldAlgo(nc);
    std::vector<uint8_t *> pcmInputs(1);
    std::vector<uint8_t *> pcmOutputs(1);
    std::vector<int16_t> dataIn(SOUNDFIELD_ALGO_FRAME_LEN, 0);
    std::vector<int16_t> dataOut(SOUNDFIELD_ALGO_FRAME_LEN, 0);

    std::string paramType = "SoundFieldType";
    std::string paramValue;
    std::string newValue;

    // not init return error
    paramValue = std::to_string(static_cast<int32_t>(IMEDIA_SWS_SOUROUND_FRONT));
    EXPECT_EQ(soundFieldAlgo.SetParameter(paramType, paramValue), ERROR);
    EXPECT_EQ(soundFieldAlgo.GetParameter(paramType, newValue), ERROR);

    ASSERT_EQ(soundFieldAlgo.Init(), 0);

    paramValue = std::to_string(static_cast<int32_t>(IMEDIA_SWS_SOUROUND_FRONT));
    EXPECT_EQ(soundFieldAlgo.SetParameter(paramType, paramValue), 0);
    EXPECT_EQ(soundFieldAlgo.GetParameter(paramType, newValue), 0);
    EXPECT_EQ(newValue == paramValue, true);

    paramValue = std::to_string(static_cast<int32_t>(IMEDIA_SWS_SOUROUND_BROAD));
    EXPECT_EQ(soundFieldAlgo.SetParameter(paramType, paramValue), 0);
    EXPECT_EQ(soundFieldAlgo.GetParameter(paramType, newValue), 0);
    EXPECT_EQ(newValue == paramValue, true);

    paramValue = std::to_string(static_cast<int32_t>(IMEDIA_SWS_SOUROUND_DEFAULT));
    EXPECT_EQ(soundFieldAlgo.SetParameter(paramType, paramValue), 0);
    EXPECT_EQ(soundFieldAlgo.GetParameter(paramType, newValue), 0);
    EXPECT_EQ(newValue == paramValue, true);

    paramValue = std::to_string(static_cast<int32_t>(IMEDIA_SWS_SOUROUND_GRAND));
    EXPECT_EQ(soundFieldAlgo.SetParameter(paramType, paramValue), 0);
    EXPECT_EQ(soundFieldAlgo.GetParameter(paramType, newValue), 0);
    EXPECT_EQ(newValue == paramValue, true);

    EXPECT_EQ(soundFieldAlgo.Deinit(), 0);
}

HWTEST_F(AudioSuiteSoundFieldAlgoInterfaceImplUnitTest, TestSoundFieldAlgoApply_001, TestSize.Level0)
{
    AudioSuiteSoundFieldAlgoInterfaceImpl soundFieldAlgo(nc);
    std::vector<uint8_t *> pcmInputs(1);
    std::vector<uint8_t *> pcmOutputs(1);

    ASSERT_EQ(soundFieldAlgo.Init(), 0);

    std::string paramValue = std::to_string(static_cast<int32_t>(IMEDIA_SWS_SOUROUND_BROAD));
    EXPECT_EQ(soundFieldAlgo.SetParameter("SoundFieldType", paramValue), 0);

    // Read inputfile
    std::ifstream ifs(g_inputfile, std::ios::binary);
    ASSERT_EQ(ifs.is_open(), true);
    ifs.seekg(0, std::ios::end);
    size_t fileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    // Padding zero then send to algo
    size_t frameSize = SOUNDFIELD_ALGO_FRAME_SIZE;
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
        pcmInputs[0] = frameInputPtr;
        pcmOutputs[0] = frameOutputPtr;
        ASSERT_EQ(soundFieldAlgo.Apply(pcmInputs, pcmOutputs), 0);
        frameInputPtr += frameSize;
        frameOutputPtr += frameSize;
    }

    // write to outputfile
    ASSERT_EQ(CreateOutputPcmFile(g_outputfile), true);
    bool isWriteFileSucc = WritePcmFile(g_outputfile, outputfileBuffer.data(), fileSize);
    ASSERT_EQ(isWriteFileSucc, true);

    // compare result
    EXPECT_EQ(IsFilesEqual(g_outputfile, g_targetfile), true);

    EXPECT_EQ(soundFieldAlgo.Deinit(), 0);
}

}  // namespace
