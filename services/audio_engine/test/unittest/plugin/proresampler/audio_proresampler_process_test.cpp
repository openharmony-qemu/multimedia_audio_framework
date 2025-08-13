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
#include <vector>
#include <map>
#include <gtest/gtest.h>
#include <cstdlib>
#include <cmath>
#include <climits>
#include <cstdint>
#include "audio_proresampler_process.h"
#include "audio_engine_log.h"
#include "securec.h"

using namespace testing::ext;
using namespace testing;

class AudioProResamplerProcessTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void AudioProResamplerProcessTest::SetUp() {}

void AudioProResamplerProcessTest::TearDown() {}

/*
 * @tc.name  : Test SingleStagePolyphaseResamplerSetRate API
 * @tc.type  : FUNC
 * @tc.number: SingleStagePolyphaseResamplerSetRate_01
 * @tc.desc  : Test SingleStagePolyphaseResamplerSetRate, set decimateFactor is
 *             0 and interpolateFactor is 0.
 */
HWTEST_F(AudioProResamplerProcessTest, SingleStagePolyphaseResamplerSetRate_01, TestSize.Level0)
{
    SingleStagePolyphaseResamplerState state;
    uint32_t decimateFactor = 0;
    uint32_t interpolateFactor = 0;
    int32_t ret = SingleStagePolyphaseResamplerSetRate(&state, decimateFactor, interpolateFactor);
    EXPECT_EQ(ret, RESAMPLER_ERR_INVALID_ARG);
}

/*
 * @tc.name  : Test SingleStagePolyphaseResamplerSetRate API
 * @tc.type  : FUNC
 * @tc.number: SingleStagePolyphaseResamplerSetRate_02
 * @tc.desc  : Test SingleStagePolyphaseResamplerSetRate, set decimateFactor is
 *             0 and interpolateFactor is 2.
 */
HWTEST_F(AudioProResamplerProcessTest, SingleStagePolyphaseResamplerSetRate_02, TestSize.Level0)
{
    SingleStagePolyphaseResamplerState state;
    uint32_t decimateFactor = 0;
    uint32_t interpolateFactor = 2;
    int32_t ret = SingleStagePolyphaseResamplerSetRate(&state, decimateFactor, interpolateFactor);
    EXPECT_EQ(ret, RESAMPLER_ERR_INVALID_ARG);
}

/*
 * @tc.name  : Test SingleStagePolyphaseResamplerSetRate API
 * @tc.type  : FUNC
 * @tc.number: SingleStagePolyphaseResamplerSetRate_03
 * @tc.desc  : Test SingleStagePolyphaseResamplerSetRate, set decimateFactor is
 *             2 and interpolateFactor is 0.
 */
HWTEST_F(AudioProResamplerProcessTest, SingleStagePolyphaseResamplerSetRate_03, TestSize.Level0)
{
    SingleStagePolyphaseResamplerState state;
    uint32_t decimateFactor = 2;
    uint32_t interpolateFactor = 0;
    int32_t ret = SingleStagePolyphaseResamplerSetRate(&state, decimateFactor, interpolateFactor);
    EXPECT_EQ(ret, RESAMPLER_ERR_INVALID_ARG);
}