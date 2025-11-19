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

#include <gtest/gtest.h>
#include <iostream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <chrono>
#include <thread>
#include <dlfcn.h>
#include "securec.h"
#include "audio_suite_aiss_algo_interface_impl.h"
#include "audio_suite_algo_interface.h"
#include "audio_suite_pcm_buffer.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;

class AudioSuiteAissAlgoInterfaceImplTest : public testing::Test {
public:
    void SetUp()
    {
        nc.soName = "libaudio_aiss_intergration.z.so";
        nc.soPath = "/system/lib64/";
    };
    void TearDown()
    {
    };
private:
    NodeCapability nc;
};
namespace {
    const std::string INPUT_PATH = "/data/aiss_48000_2_S32LE.pcm";
    const std::string OUTPUT_PATH = "/data/aiss_output.pcm";
    const std::string HUMAN_PATH = "/data/humanSound.pcm";
    const std::string BKG_PATH = "/data/bkgSound.pcm";

    HWTEST_F(AudioSuiteAissAlgoInterfaceImplTest, CheckFilePathTest, TestSize.Level0)
    {
        std::string path = INPUT_PATH;
        AudioSuiteAissAlgoInterfaceImpl impl(nc);
        ASSERT_EQ(impl.CheckFilePath(path), SUCCESS);
        path = "./errorPath";
        ASSERT_EQ(impl.CheckFilePath(path), ERROR);
    }

    HWTEST_F(AudioSuiteAissAlgoInterfaceImplTest, SeparateChannelsTest, TestSize.Level0)
    {
        const int frameLength = 2;
        float input[8] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8};
        float humanOut[4];
        float bkgOut[4];
        AudioSuiteAissAlgoInterfaceImpl impl(nc);
        impl.SeparateChannels(frameLength, input, humanOut, bkgOut);
        EXPECT_FLOAT_EQ(humanOut[0], 0.1);
        EXPECT_FLOAT_EQ(humanOut[1], 0.2);
        EXPECT_FLOAT_EQ(humanOut[2], 0.5);
        EXPECT_FLOAT_EQ(humanOut[3], 0.6);
        EXPECT_FLOAT_EQ(bkgOut[0], 0.3);
        EXPECT_FLOAT_EQ(bkgOut[1], 0.4);
        EXPECT_FLOAT_EQ(bkgOut[2], 0.7);
        EXPECT_FLOAT_EQ(bkgOut[3], 0.8);
    }

    HWTEST_F(AudioSuiteAissAlgoInterfaceImplTest, InitDeinitTest, TestSize.Level0)
    {
        AudioSuiteAissAlgoInterfaceImpl impl(nc);
        ASSERT_EQ(impl.Init(), SUCCESS);
        ASSERT_EQ(impl.Deinit(), SUCCESS);
    }

    HWTEST_F(AudioSuiteAissAlgoInterfaceImplTest, ParameterTest, TestSize.Level0)
    {
        AudioSuiteAissAlgoInterfaceImpl impl(nc);
        std::string paramType = "Property";
        std::string paramValue = "AISSVX";
        ASSERT_EQ(impl.SetParameter(paramType, paramValue), SUCCESS);
        ASSERT_EQ(impl.GetParameter(paramType, paramValue), SUCCESS);
        ASSERT_EQ("AISSVX", paramValue);
    }

}