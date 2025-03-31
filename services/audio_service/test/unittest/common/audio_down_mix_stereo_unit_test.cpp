/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <functional>
#include "gtest/gtest.h"
#include "audio_down_mix_stereo.cpp"

using namespace std;
using namespace testing::ext;
using namespace testing;
namespace OHOS {
namespace AudioStandard {
const AudioChannelLayout MODE = CH_LAYOUT_MONO;
const int32_t CHANNELS = 2;
class Test : public ::testing::Test {
protected:
void SetUp() override {}
void TearDown() override {}
};

class AudioDownMixStereoTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

/**
 * @tc.name  : Test AudioDownMixStereo API
 * @tc.type  : FUNC
 * @tc.number: InitMixer_001
 * @tc.desc  : Test AudioDownMixStereo interface.
 */
HWTEST(AudioDownMixStereoTest, InitMixer_001, TestSize.Level0)
{
    std::unique_ptr<AudioDownMixStereo> audioDownMixStereo = std::make_unique<AudioDownMixStereo>();
    audioDownMixStereo->mixer_ = nullptr;
    int32_t ret = audioDownMixStereo->InitMixer(MODE, CHANNELS);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
}

/**
 * @tc.name  : Test AudioDownMixStereo API
 * @tc.type  : FUNC
 * @tc.number: InitMixer_002
 * @tc.desc  : Test AudioDownMixStereo interface.
 */
HWTEST(AudioDownMixStereoTest, InitMixer_002, TestSize.Level0)
{
    std::unique_ptr<AudioDownMixStereo> audioDownMixStereo = std::make_unique<AudioDownMixStereo>();
    int32_t ret = audioDownMixStereo->InitMixer(MODE, CHANNELS);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
}

/**
 * @tc.name  : Test AudioDownMixStereo API
 * @tc.type  : FUNC
 * @tc.number: Apply_001
 * @tc.desc  : Test AudioDownMixStereo interface.
 */
HWTEST(AudioDownMixStereoTest, Apply_001, TestSize.Level0)
{
    std::unique_ptr<AudioDownMixStereo> audioDownMixStereo = std::make_unique<AudioDownMixStereo>();
    audioDownMixStereo->mixer_ = nullptr;
    EXPECT_EQ(audioDownMixStereo->Apply(10, nullptr, nullptr), ERR_INVALID_HANDLE);
}

/**
 * @tc.name  : Test AudioDownMixStereo API
 * @tc.type  : FUNC
 * @tc.number: Apply_002
 * @tc.desc  : Test AudioDownMixStereo interface.
 */
HWTEST(AudioDownMixStereoTest, Apply_002, TestSize.Level0)
{
    std::unique_ptr<AudioDownMixStereo> audioDownMixStereo = std::make_unique<AudioDownMixStereo>();
    EXPECT_EQ(audioDownMixStereo->Apply(10, nullptr, nullptr), ERR_INVALID_HANDLE);
}
} // namespace AudioStandard
} // namespace OHOS
