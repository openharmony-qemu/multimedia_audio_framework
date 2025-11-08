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
#include <vector>
#include <memory>
#include <cstring>
#include <algorithm>
#include "audio_suite_env_algo_interface_impl.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;

class AudioSuiteEnvAlgoInterfaceImplTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void AudioSuiteEnvAlgoInterfaceImplTest::SetUp()
{}

void AudioSuiteEnvAlgoInterfaceImplTest::TearDown()
{}

const size_t FRAME_LEN = 1920;   // 每帧采样点数

HWTEST_F(AudioSuiteEnvAlgoInterfaceImplTest, AudioSuiteEnvAlgoInterfaceImplTest, TestSize.Level0)
{
    NodeCapability nc;
    nc.soName = "libimedia_sws.z.so";
    nc.soPath = "/system/lib64/";
    AudioSuiteEnvAlgoInterfaceImpl envAlgo(nc);
    EXPECT_EQ(envAlgo.Init(), 0);
    EXPECT_NE(envAlgo.Init(), 0);
    std::string a = "";
    std::string b = "";
    EXPECT_EQ(envAlgo.GetParameter(a, b), 0);

    std::vector<uint8_t*> pcmInBuf;
    std::vector<uint8_t*> pcmOutBuf;

    IMEDIA_INT16* tempBuffer = new IMEDIA_INT16[FRAME_LEN];
    std::fill(tempBuffer, tempBuffer + FRAME_LEN, 0);
    pcmInBuf.push_back(reinterpret_cast<uint8_t*>(tempBuffer));
    pcmOutBuf.push_back(reinterpret_cast<uint8_t*>(tempBuffer));
    envAlgo.Apply(pcmInBuf, pcmOutBuf);
    EXPECT_EQ(envAlgo.Deinit(), 0);
    delete[] tempBuffer;
}