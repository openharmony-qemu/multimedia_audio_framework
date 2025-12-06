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
#include <cmath>
#include <memory>
#include <fstream>
#include <cstring>
#include "audio_suite_eq_algo_interface_impl.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;

namespace {

class AudioSuiteEqAlgoInterfaceImplTest : public testing::Test {
public:
    void SetUp(){};
    void TearDown(){};
};

HWTEST_F(AudioSuiteEqAlgoInterfaceImplTest, AudioSuiteEqAlgoInterfaceImplTest, TestSize.Level0)
{
    NodeCapability nc;
    nc.soName = "libimedia_sws.z.so";
    nc.soPath = "/system/lib64/";
    AudioSuiteEqAlgoInterfaceImpl eqAlgo(nc);
    EXPECT_EQ(eqAlgo.Init(), 0);
    EXPECT_NE(eqAlgo.Init(), 0);
    std::string a = "";
    std::string b = "";
    EXPECT_EQ(eqAlgo.GetParameter(a, b), 0);
}
}  // namespace