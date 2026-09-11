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

#include "audio_highpass_filter.h"
#include "i_highpass_filter.h"

#include "aiut_log.h"

#include "audio_errors.h"

#include <gtest/gtest.h>
#include <vector>

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class MockHighPassFilter : public IHighPassFilter {
public:
    int32_t Init(const int32_t channels) override
    {
        if (channels <= 0) {
            return -1;
        }
        return 0;
    }
    int32_t Apply(const std::vector<float> &samples, std::vector<float> &result) override
    {
        if (samples.empty() || samples.size() != result.size()) {
            return -1;
        }
        return 0;
    }
};

class AudioHighPassFilterAiut : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name  : AudioHighPassFilter_Constructor_SoNotFound
 * @tc.number: AudioHighPassFilter_Constructor_SoNotFound
 * @tc.desc  : Test AudioHighPassFilter constructor when so file does not exist, filter_ should be nullptr.
 */
HWTEST_F(AudioHighPassFilterAiut, AudioHighPassFilter_Constructor_SoNotFound, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto filter = std::make_unique<AudioHighPassFilter>();
    AIUT_ASSERT_NE_INFO_LOG(filter, nullptr, "AudioHighPassFilter create failed, ptr=%p", filter.get());
    AIUT_EXPECT_EQ_INFO_LOG(filter->filter_, nullptr, "filter_ should be nullptr when so not found");
    AIUT_EXPECT_EQ_INFO_LOG(filter->handle_, nullptr, "handle_ should be nullptr when so not found");
}

/**
 * @tc.name  : AudioHighPassFilter_InitFilter_NullFilter
 * @tc.number: AudioHighPassFilter_InitFilter_NullFilter
 * @tc.desc  : Test InitFilter returns ERR_INVALID_HANDLE when filter_ is nullptr.
 */
HWTEST_F(AudioHighPassFilterAiut, AudioHighPassFilter_InitFilter_NullFilter, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioHighPassFilter filter;
    int32_t ret = filter.InitFilter(2);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_HANDLE, "InitFilter should return ERR_INVALID_HANDLE, ret=%d", ret);
}

/**
 * @tc.name  : AudioHighPassFilter_InitFilter_InvalidChannels
 * @tc.number: AudioHighPassFilter_InitFilter_InvalidChannels
 * @tc.desc  : Test InitFilter returns ERR_INVALID_PARAM when channels is invalid.
 */
HWTEST_F(AudioHighPassFilterAiut, AudioHighPassFilter_InitFilter_InvalidChannels, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioHighPassFilter filter;
    filter.filter_ = new MockHighPassFilter();
    int32_t ret = filter.InitFilter(0);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM, "InitFilter should return ERR_INVALID_PARAM, ret=%d", ret);
    ret = filter.InitFilter(-1);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM, "InitFilter should return ERR_INVALID_PARAM, ret=%d", ret);
}

/**
 * @tc.name  : AudioHighPassFilter_InitFilter_ValidChannels
 * @tc.number: AudioHighPassFilter_InitFilter_ValidChannels
 * @tc.desc  : Test InitFilter returns SUCCESS when channels is valid and filter init succeeds.
 */
HWTEST_F(AudioHighPassFilterAiut, AudioHighPassFilter_InitFilter_ValidChannels, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioHighPassFilter filter;
    filter.filter_ = new MockHighPassFilter();
    int32_t ret = filter.InitFilter(2);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "InitFilter should return SUCCESS, ret=%d", ret);
}

/**
 * @tc.name  : ApplyFilter_001
 * @tc.number: ApplyFilter_001
 * @tc.desc  : Test ApplyFilter returns ERR_INVALID_HANDLE when filter_ is nullptr.
 */
HWTEST_F(AudioHighPassFilterAiut, ApplyFilter_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioHighPassFilter filter;
    std::vector<float> samples(10, 0.5f);
    std::vector<float> result(10, 0.0f);
    int32_t ret = filter.ApplyFilter(samples, result);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_HANDLE, "ApplyFilter should return ERR_INVALID_HANDLE, ret=%d", ret);
}

/**
 * @tc.name  : AudioHighPassFilter_ApplyFilter_EmptySamples
 * @tc.number: AudioHighPassFilter_ApplyFilter_EmptySamples
 * @tc.desc  : Test ApplyFilter returns ERR_INVALID_PARAM when samples is empty.
 */
HWTEST_F(AudioHighPassFilterAiut, AudioHighPassFilter_ApplyFilter_EmptySamples, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioHighPassFilter filter;
    filter.filter_ = new MockHighPassFilter();
    std::vector<float> samples;
    std::vector<float> result(10, 0.0f);
    int32_t ret = filter.ApplyFilter(samples, result);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM, "ApplyFilter should return ERR_INVALID_PARAM, ret=%d", ret);
}

/**
 * @tc.name  : AudioHighPassFilter_ApplyFilter_SizeMismatch
 * @tc.number: AudioHighPassFilter_ApplyFilter_SizeMismatch
 * @tc.desc  : Test ApplyFilter returns ERR_INVALID_PARAM when samples size != result size.
 */
HWTEST_F(AudioHighPassFilterAiut, AudioHighPassFilter_ApplyFilter_SizeMismatch, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioHighPassFilter filter;
    filter.filter_ = new MockHighPassFilter();
    std::vector<float> samples(10, 0.5f);
    std::vector<float> result(5, 0.0f);
    int32_t ret = filter.ApplyFilter(samples, result);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM, "ApplyFilter should return ERR_INVALID_PARAM, ret=%d", ret);
}

/**
 * @tc.name  : AudioHighPassFilter_ApplyFilter_ValidInput
 * @tc.number: AudioHighPassFilter_ApplyFilter_ValidInput
 * @tc.desc  : Test ApplyFilter returns SUCCESS when all conditions are met.
 */
HWTEST_F(AudioHighPassFilterAiut, AudioHighPassFilter_ApplyFilter_ValidInput, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioHighPassFilter filter;
    filter.filter_ = new MockHighPassFilter();
    std::vector<float> samples(10, 0.5f);
    std::vector<float> result(10, 0.0f);
    int32_t ret = filter.ApplyFilter(samples, result);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "ApplyFilter should return SUCCESS, ret=%d", ret);
}

/**
 * @tc.name  : AudioHighPassFilter_Destructor_NullFilter
 * @tc.number: AudioHighPassFilter_Destructor_NullFilter
 * @tc.desc  : Test destructor when both filter_ and handle_ are nullptr.
 */
HWTEST_F(AudioHighPassFilterAiut, AudioHighPassFilter_Destructor_NullFilter, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto filter = std::make_unique<AudioHighPassFilter>();
    AIUT_ASSERT_NE_INFO_LOG(filter, nullptr, "AudioHighPassFilter create failed, ptr=%p", filter.get());
    AIUT_EXPECT_EQ_INFO_LOG(filter->filter_, nullptr, "filter_ should be nullptr before reset");
    filter.reset();
    AIUT_INFO_LOG("destructor with null filter_ and handle_ completed");
}

/**
 * @tc.name  : AudioHighPassFilter_Destructor_NonNullFilter
 * @tc.number: AudioHighPassFilter_Destructor_NonNullFilter
 * @tc.desc  : Test destructor when filter_ is not nullptr.
 */
HWTEST_F(AudioHighPassFilterAiut, AudioHighPassFilter_Destructor_NonNullFilter, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto filter = std::make_unique<AudioHighPassFilter>();
    AIUT_ASSERT_NE_INFO_LOG(filter, nullptr, "AudioHighPassFilter create failed, ptr=%p", filter.get());
    filter->filter_ = new MockHighPassFilter();
    AIUT_EXPECT_NE_INFO_LOG(filter->filter_, nullptr, "filter_ should not be null after set");
    filter.reset();
    AIUT_INFO_LOG("destructor with non-null filter_ completed");
}

} // namespace AudioStandard
} // namespace OHOS
