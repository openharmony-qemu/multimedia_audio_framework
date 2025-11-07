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
#include "audio_suite_voice_beautifier_algo_interface_impl.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;

namespace {
class AudioSuiteVbAlgoInterfaceImplUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void){};
    static void TearDownTestCase(void){};
    void SetUp(void);
    void TearDown(void);

private:
    NodeCapability nc;
};

void AudioSuiteVbAlgoInterfaceImplUnitTest::SetUp(void)
{
    std::filesystem::remove("/data/audiosuite/vb/vb_output_48000_2_S16LE_out.pcm");
    nc.soName = "libaudio_voice_morph_bgm.z.so";
    nc.soPath = "/system/lib64/";
}

void AudioSuiteVbAlgoInterfaceImplUnitTest::TearDown(void)
{}

HWTEST_F(AudioSuiteVbAlgoInterfaceImplUnitTest, TestVbAlgoInitAndDeinit_001, TestSize.Level0)
{
    AudioSuiteVoiceBeautifierAlgoInterfaceImpl vbAlgo(nc);
    EXPECT_EQ(vbAlgo.Init(), 0);
    std::string value = std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_CD));
    std::string name = "VoiceBeautifierType";
    EXPECT_EQ(vbAlgo.SetParameter(name, value), 0);
    EXPECT_EQ(vbAlgo.Deinit(), 0);
}

HWTEST_F(AudioSuiteVbAlgoInterfaceImplUnitTest, TestVbAlgoApply_001, TestSize.Level0)
{
    AudioSuiteVoiceBeautifierAlgoInterfaceImpl vbAlgo(nc);
    std::vector<uint8_t *> audioInputs(1);
    std::vector<uint8_t *> audioOutputs(1);
    std::vector<int16_t> dataIn(960 * 2, 0);
    std::vector<int16_t> dataOut(960 * 2, 0);

    EXPECT_EQ(vbAlgo.Init(), 0);
    std::string value = std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_CD));
    std::string name = "VoiceBeautifierType";
    EXPECT_EQ(vbAlgo.SetParameter(name, value), 0);
    audioInputs[0] = reinterpret_cast<uint8_t *>(dataIn.data());
    audioOutputs[0] = reinterpret_cast<uint8_t *>(dataOut.data());
    EXPECT_EQ(vbAlgo.Apply(audioInputs, audioOutputs), 0);

    EXPECT_EQ(vbAlgo.Deinit(), 0);
}

HWTEST_F(AudioSuiteVbAlgoInterfaceImplUnitTest, TestVbAlgoApply_002, TestSize.Level0)
{
    AudioSuiteVoiceBeautifierAlgoInterfaceImpl vbAlgo(nc);
    std::vector<uint8_t *> audioInputs(1);
    std::vector<uint8_t *> audioOutputs(1);
    size_t frameSize = 960 * 2 * 2;

    EXPECT_EQ(vbAlgo.Init(), 0);
    std::string value = std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_CD));
    std::string name = "VoiceBeautifierType";
    EXPECT_EQ(vbAlgo.SetParameter(name, value), 0);
    // 处理输入文件
    std::ifstream ifs("/data/audiosuite/vb/voice_morph_input.pcm", std::ios::binary);
    ifs.seekg(0, std::ios::end);
    size_t fileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    // pcm文件长度可能不是帧长的整数倍，补0后再传给算法处理
    size_t zeroPaddingSize = (fileSize % frameSize == 0) ? 0 : (frameSize - fileSize % frameSize);
    size_t fileBufferSize = fileSize + zeroPaddingSize;
    std::vector<int16_t> inputfileBuffer(fileBufferSize / sizeof(int16_t), 0);  // 16-bit PCM file
    ifs.read(reinterpret_cast<char *>(inputfileBuffer.data()), fileSize);
    ifs.close();

    std::vector<uint8_t> outputfileBuffer(fileBufferSize);
    uint8_t *frameInputPtr = reinterpret_cast<uint8_t *>(inputfileBuffer.data());
    uint8_t *frameOutputPtr = outputfileBuffer.data();
    for (int32_t i = 0; i + frameSize <= fileBufferSize; i += frameSize) {
        audioInputs[0] = frameInputPtr;
        audioOutputs[0] = frameOutputPtr;
        ASSERT_EQ(vbAlgo.Apply(audioInputs, audioOutputs), 0);
        frameInputPtr += frameSize;
        frameOutputPtr += frameSize;
    }

    // 输出pcm数据写入文件
    ASSERT_EQ(CreateOutputPcmFile("/data/audiosuite/vb/vb_output_48000_2_S16LE_out.pcm"), true);
    bool isWriteFileSucc =
        WritePcmFile("/data/audiosuite/vb/vb_output_48000_2_S16LE_out.pcm", outputfileBuffer.data(), fileSize);
    ASSERT_EQ(isWriteFileSucc, true);

    // 和归档结果比对
    EXPECT_EQ(IsFilesEqual("/data/audiosuite/vb/vb_output_48000_2_S16LE_out.pcm",
                  "/data/audiosuite/vb/voice_morph_pc_output_cd.pcm"),
        true);

    EXPECT_EQ(vbAlgo.Deinit(), 0);
}

HWTEST_F(AudioSuiteVbAlgoInterfaceImplUnitTest, TestVbAlgoApply_003, TestSize.Level0)
{
    AudioSuiteVoiceBeautifierAlgoInterfaceImpl vbAlgo(nc);
    std::vector<uint8_t *> audioInputs(1);
    std::vector<uint8_t *> audioOutputs(1);
    std::vector<int16_t> dataIn(960 * 2, 0);
    std::vector<int16_t> dataOut(960 * 2, 0);

    EXPECT_EQ(vbAlgo.Init(), 0);
    std::string value = std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_CD));
    std::string name = "VoiceBeautifierType";
    EXPECT_EQ(vbAlgo.SetParameter(name, value), 0);
    audioInputs[0] = nullptr;
    audioOutputs[0] = nullptr;
    EXPECT_EQ(vbAlgo.Apply(audioInputs, audioOutputs), ERROR);

    EXPECT_EQ(vbAlgo.Deinit(), 0);
}

HWTEST_F(AudioSuiteVbAlgoInterfaceImplUnitTest, TestVbAlgoApply_004, TestSize.Level0)
{
    AudioSuiteVoiceBeautifierAlgoInterfaceImpl vbAlgo(nc);
    std::vector<uint8_t *> audioInputs;
    std::vector<uint8_t *> audioOutputs;
    std::vector<int16_t> dataIn(960 * 2, 0);
    std::vector<int16_t> dataOut(960 * 2, 0);

    EXPECT_EQ(vbAlgo.Init(), 0);
    std::string value = std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_CD));
    std::string name = "VoiceBeautifierType";
    EXPECT_EQ(vbAlgo.SetParameter(name, value), 0);
    EXPECT_EQ(vbAlgo.Apply(audioInputs, audioOutputs), ERROR);

    EXPECT_EQ(vbAlgo.Deinit(), 0);
}

HWTEST_F(AudioSuiteVbAlgoInterfaceImplUnitTest, TestVbAlgoApply_005, TestSize.Level0)
{
    AudioSuiteVoiceBeautifierAlgoInterfaceImpl vbAlgo(nc);
    std::vector<uint8_t *> audioInputs(1);
    std::vector<uint8_t *> audioOutputs(1);
    std::vector<int16_t> dataIn(960 * 2, 0);
    std::vector<int16_t> dataOut(960 * 2, 0);

    EXPECT_EQ(vbAlgo.Init(), 0);
    std::string value = std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_CD));
    std::string name = "VoiceBeautifierType";
    EXPECT_EQ(vbAlgo.SetParameter(name, value), 0);
    audioInputs[0] = reinterpret_cast<uint8_t *>(dataIn.data());
    audioOutputs[0] = reinterpret_cast<uint8_t *>(dataOut.data());
    vbAlgo.inBuf_ = nullptr;
    EXPECT_EQ(vbAlgo.Apply(audioInputs, audioOutputs), ERROR);

    EXPECT_EQ(vbAlgo.Deinit(), 0);
}

HWTEST_F(AudioSuiteVbAlgoInterfaceImplUnitTest, TestVbAlgoApply_006, TestSize.Level0)
{
    AudioSuiteVoiceBeautifierAlgoInterfaceImpl vbAlgo(nc);
    std::vector<uint8_t *> audioInputs(1);
    std::vector<uint8_t *> audioOutputs(1);
    std::vector<int16_t> dataIn(960 * 2, 0);
    std::vector<int16_t> dataOut(960 * 2, 0);

    EXPECT_EQ(vbAlgo.Init(), 0);
    std::string value = std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_CD));
    std::string name = "VoiceBeautifierType";
    EXPECT_EQ(vbAlgo.SetParameter(name, value), 0);
    audioInputs[0] = reinterpret_cast<uint8_t *>(dataIn.data());
    audioOutputs[0] = reinterpret_cast<uint8_t *>(dataOut.data());
    vbAlgo.outBuf_ = nullptr;
    EXPECT_EQ(vbAlgo.Apply(audioInputs, audioOutputs), ERROR);

    EXPECT_EQ(vbAlgo.Deinit(), 0);
}

HWTEST_F(AudioSuiteVbAlgoInterfaceImplUnitTest, TestVbAlgoApply_007, TestSize.Level0)
{
    AudioSuiteVoiceBeautifierAlgoInterfaceImpl vbAlgo(nc);

    EXPECT_EQ(vbAlgo.Init(), 0);
    std::string value = std::to_string(10);
    std::string name = "VoiceBeautifierType";
    EXPECT_EQ(vbAlgo.SetParameter(name, value), ERROR);

    EXPECT_EQ(vbAlgo.Deinit(), 0);
}

}  // namespace
