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

#include <gtest/gtest.h>
#include <vector>
#include <memory>
#include <cstring>
#include "audio_suite_hoa_render_algo_interface_impl.h"
#include "audio_suite_unittest_tools.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;

class AudioSuiteHoaRenderAlgoInterfaceImplTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void AudioSuiteHoaRenderAlgoInterfaceImplTest::SetUp()
{
    if (!AllNodeTypesSupported()) {
        GTEST_SKIP() << "not support all node types, skip this test";
    }
}

void AudioSuiteHoaRenderAlgoInterfaceImplTest::TearDown() {}

HWTEST_F(AudioSuiteHoaRenderAlgoInterfaceImplTest, AudioSuiteHoaRenderAlgoInterfaceImplInitTest, TestSize.Level0)
{
    NodeParameter np;
    np.soName = "libhvs_rendering.z.so";
    np.soPath = "/system/lib64/";
    AudioSuiteHoaRenderAlgoInterfaceImpl hoaAlgo(np);

    int32_t ret = hoaAlgo.Init();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(hoaAlgo.isHoaAlgoInit_);

    ret = hoaAlgo.Deinit();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_FALSE(hoaAlgo.isHoaAlgoInit_);
}

HWTEST_F(AudioSuiteHoaRenderAlgoInterfaceImplTest, AudioSuiteHoaRenderAlgoInterfaceImplDestructorTest, TestSize.Level0)
{
    NodeParameter np;
    np.soName = "libhvs_rendering.z.so";
    np.soPath = "/system/lib64/";

    auto hoaAlgo = std::make_shared<AudioSuiteHoaRenderAlgoInterfaceImpl>(np);
    hoaAlgo->Init();
    EXPECT_TRUE(hoaAlgo->isHoaAlgoInit_);
    hoaAlgo.reset();

    auto hoaAlgoNotInit = std::make_shared<AudioSuiteHoaRenderAlgoInterfaceImpl>(np);
    EXPECT_FALSE(hoaAlgoNotInit->isHoaAlgoInit_);
    hoaAlgoNotInit.reset();
}