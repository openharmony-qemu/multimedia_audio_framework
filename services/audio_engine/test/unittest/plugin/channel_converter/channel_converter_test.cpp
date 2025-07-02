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
#include "audio_engine_log.h"
#include "down_mixer.h"
#include "channel_converter.h"
#include <vector>

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
constexpr uint32_t TEST_BUFFER_LEN = 10;
constexpr bool MIX_FLE = true;
class ChannelConverterTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void ChannelConverterTest::SetUp() {}

void ChannelConverterTest::TearDown() {}

HWTEST_F(ChannelConverterTest, ChannelConverterTestProcessTest, TestSize.Level0)
{
    // test upmix
    AudioChannelInfo inChannelInfo;
    AudioChannelInfo outChannelInfo;
    inChannelInfo.numChannels = MONO;
    inChannelInfo.channelLayout = CH_LAYOUT_MONO;
    outChannelInfo.numChannels = STEREO;
    outChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    ChannelConverter channelConverter;
    std::vector<float> in(TEST_BUFFER_LEN * MONO, 0.0f);
    std::vector<float> out(TEST_BUFFER_LEN * STEREO, 0.0f);
    EXPECT_EQ(channelConverter.SetParam(inChannelInfo, outChannelInfo, SAMPLE_F32LE, MIX_FLE), DMIX_ERR_SUCCESS);
    EXPECT_EQ(channelConverter.Process(TEST_BUFFER_LEN, in.data(), in.size() * sizeof(float), out.data(),
        out.size() * sizeof(float)), DMIX_ERR_SUCCESS);
    
    // test downmix
    inChannelInfo.numChannels = CHANNEL_6;
    inChannelInfo.channelLayout = CH_LAYOUT_5POINT1;
    in.resize(TEST_BUFFER_LEN * CHANNEL_6, 0.0f);
    EXPECT_EQ(channelConverter.SetParam(inChannelInfo, outChannelInfo, SAMPLE_F32LE, MIX_FLE), DMIX_ERR_SUCCESS);
    EXPECT_EQ(channelConverter.Process(TEST_BUFFER_LEN, in.data(), in.size() * sizeof(float), out.data(),
        out.size() * sizeof(float)), DMIX_ERR_SUCCESS);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS