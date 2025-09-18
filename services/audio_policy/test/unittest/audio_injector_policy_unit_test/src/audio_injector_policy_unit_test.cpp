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
#include "audio_injector_policy_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioInjectorPolicyUnitTest::SetUpTestCase(void) {}
void AudioInjectorPolicyUnitTest::TearDownTestCase(void) {}
void AudioInjectorPolicyUnitTest::SetUp(void) {}
void AudioInjectorPolicyUnitTest::TearDown(void) {}

/**
 * @tc.name: Init_001
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, Init_001, TestSize.Level1)
{
    AudioInjectorPolicy::GetInstance().isOpened_ = false;
    AudioInjectorPolicy::GetInstance().Init();
    AudioInjectorPolicy::GetInstance().isOpened_ = true;
    int32_t ret = AudioInjectorPolicy::GetInstance().Init();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: DeInit_001
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, DeInit_001, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.isOpened_ = false;
    audioInjectorPolicy.rendererStreamMap_.clear();
    audioInjectorPolicy.DeInit();
    audioInjectorPolicy.isOpened_ = false;
    std::shared_ptr<AudioStreamDescriptor> streamDesc1 = std::make_shared<AudioStreamDescriptor>();
    audioInjectorPolicy.rendererStreamMap_.clear();
    audioInjectorPolicy.rendererStreamMap_[1111] = streamDesc1;
    int32_t ret = audioInjectorPolicy.DeInit();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: DeInit_002
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, DeInit_002, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.isOpened_ = true;
    audioInjectorPolicy.rendererStreamMap_.clear();
    audioInjectorPolicy.DeInit();
    audioInjectorPolicy.isOpened_ = true;
    std::shared_ptr<AudioStreamDescriptor> streamDesc1 = std::make_shared<AudioStreamDescriptor>();
    audioInjectorPolicy.rendererStreamMap_.clear();
    audioInjectorPolicy.rendererStreamMap_[1111] = streamDesc1;
    int32_t ret = audioInjectorPolicy.DeInit();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: IsContainStream_001
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, IsContainStream_001, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.rendererStreamMap_.clear();
    bool ret = audioInjectorPolicy.IsContainStream(1111);
    EXPECT_EQ(false, ret);
    std::shared_ptr<AudioStreamDescriptor> streamDesc1 = std::make_shared<AudioStreamDescriptor>();
    audioInjectorPolicy.rendererStreamMap_[1111] = streamDesc1;
    ret = audioInjectorPolicy.IsContainStream(1111);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: AddCaptureInjector_001
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, AddCaptureInjector_001, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.rendererStreamMap_.clear();
    audioInjectorPolicy.isConnected_ = false;
    audioInjectorPolicy.AddCaptureInjector();

    audioInjectorPolicy.isConnected_ = true;
    int32_t ret = audioInjectorPolicy.AddCaptureInjector();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: RemoveCaptureInjector_001
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, RemoveCaptureInjector_001, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.isConnected_ = false;
    audioInjectorPolicy.rendererStreamMap_.clear();
    audioInjectorPolicy.RemoveCaptureInjector();
    audioInjectorPolicy.isConnected_ = false;
    std::shared_ptr<AudioStreamDescriptor> streamDesc1 = std::make_shared<AudioStreamDescriptor>();
    audioInjectorPolicy.rendererStreamMap_.clear();
    audioInjectorPolicy.rendererStreamMap_[1111] = streamDesc1;
    int32_t ret = audioInjectorPolicy.RemoveCaptureInjector();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: RemoveCaptureInjector_002
 * @tc.desc: wzwzwz
 * @tc.type: FUNC
 * @tc.require: #I5Y4MZ
 */
HWTEST_F(AudioInjectorPolicyUnitTest, RemoveCaptureInjector_002, TestSize.Level1)
{
    auto &audioInjectorPolicy = AudioInjectorPolicy::GetInstance();
    audioInjectorPolicy.isConnected_ = true;
    audioInjectorPolicy.rendererStreamMap_.clear();
    audioInjectorPolicy.RemoveCaptureInjector();
    audioInjectorPolicy.isConnected_ = true;
    std::shared_ptr<AudioStreamDescriptor> streamDesc1 = std::make_shared<AudioStreamDescriptor>();
    audioInjectorPolicy.rendererStreamMap_.clear();
    audioInjectorPolicy.rendererStreamMap_[1111] = streamDesc1;
    int32_t ret = audioInjectorPolicy.RemoveCaptureInjector();
    EXPECT_EQ(SUCCESS, ret);
}
} // namespace AudioStandard
} // namespace OHOS