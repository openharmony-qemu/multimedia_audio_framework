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
#include <memory>
#include <vector>
#include "audio_highpass_filter.h"
#include "audio_errors.h"
#include "i_highpass_filter.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

class MockHighPassFilter : public IHighPassFilter {
public:
    MockHighPassFilter() : initialized_(false) {}
    virtual ~MockHighPassFilter() = default;

    int32_t Init(const int32_t channels) override
    {
        if (channels <= 0) {
            return -1;
        }
        initialized_ = true;
        return 0;
    }

    int32_t Apply(const std::vector<float> &samples, std::vector<float> &result) override
    {
        if (!initialized_) {
            return -1;
        }
        if (samples.empty()) {
            return -1;
        }
        if (samples.size() != result.size()) {
            return -1;
        }
        for (size_t i = 0; i < samples.size(); ++i) {
            result[i] = samples[i];
        }
        return 0;
    }

    void Reset()
    {
        initialized_ = false;
    }

private:
    bool initialized_;
};

class AudioHighPassFilterUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::unique_ptr<MockHighPassFilter> mockFilter_;
};

void AudioHighPassFilterUnitTest::SetUpTestCase(void)
{
}

void AudioHighPassFilterUnitTest::TearDownTestCase(void)
{
}

void AudioHighPassFilterUnitTest::SetUp()
{
    mockFilter_ = std::make_unique<MockHighPassFilter>();
}

void AudioHighPassFilterUnitTest::TearDown()
{
    mockFilter_.reset();
}

class TestHighPassFilter : public IHighPassFilter {
public:
    explicit TestHighPassFilter(bool shouldSucceed = true) : shouldSucceed_(shouldSucceed) {}
    ~TestHighPassFilter() override = default;

    int32_t Init(const int32_t channels) override
    {
        if (channels <= 0) {
            shouldSucceed_ = false;
            return -1;
        }
        initCalled_ = true;
        initChannels_ = channels;
        return shouldSucceed_ ? 0 : -1;
    }

    int32_t Apply(const std::vector<float> &samples, std::vector<float> &result) override
    {
        applyCalled_ = true;
        if (!shouldSucceed_) {
            return -1;
        }
        for (size_t i = 0; i < samples.size(); ++i) {
            result[i] = samples[i] * 0.95f;
        }
        return 0;
    }

    static bool initCalled_;
    static int32_t initChannels_;
    static bool applyCalled_;
    bool shouldSucceed_;

    static void Reset()
    {
        initCalled_ = false;
        initChannels_ = 0;
        applyCalled_ = false;
    }
};

bool TestHighPassFilter::initCalled_ = false;
int32_t TestHighPassFilter::initChannels_ = 0;
bool TestHighPassFilter::applyCalled_ = false;

int32_t TestCreateFunc(IHighPassFilter **filter)
{
    if (filter == nullptr) {
        return -1;
    }
    *filter = new TestHighPassFilter();
    return 0;
}

int32_t TestCreateFuncFail(IHighPassFilter **filter)
{
    if (filter == nullptr) {
        return -1;
    }
    *filter = new TestHighPassFilter(false);
    return 0;
}

int32_t TestCreateFuncNullFilter(IHighPassFilter **filter)
{
    (void)filter;
    return -1;
}

HWTEST_F(AudioHighPassFilterUnitTest, MockHighPassFilter_Init_001, TestSize.Level1)
{
    auto filter = std::make_unique<MockHighPassFilter>();
    ASSERT_NE(filter, nullptr);

    int32_t ret = filter->Init(2);
    EXPECT_EQ(ret, 0);

    filter->Reset();
}

HWTEST_F(AudioHighPassFilterUnitTest, MockHighPassFilter_Init_002, TestSize.Level1)
{
    auto filter = std::make_unique<MockHighPassFilter>();
    ASSERT_NE(filter, nullptr);

    int32_t ret = filter->Init(0);
    EXPECT_NE(ret, 0);
}

HWTEST_F(AudioHighPassFilterUnitTest, MockHighPassFilter_Init_003, TestSize.Level1)
{
    auto filter = std::make_unique<MockHighPassFilter>();
    ASSERT_NE(filter, nullptr);

    int32_t ret = filter->Init(-1);
    EXPECT_NE(ret, 0);
}

HWTEST_F(AudioHighPassFilterUnitTest, MockHighPassFilter_Apply_001, TestSize.Level1)
{
    auto filter = std::make_unique<MockHighPassFilter>();
    ASSERT_NE(filter, nullptr);
    EXPECT_EQ(filter->Init(2), 0);

    std::vector<float> input(100, 1.0f);
    std::vector<float> output(100);

    int32_t ret = filter->Apply(input, output);
    EXPECT_EQ(ret, 0);

    for (size_t i = 0; i < 100; ++i) {
        EXPECT_EQ(output[i], 1.0f);
    }
}

HWTEST_F(AudioHighPassFilterUnitTest, MockHighPassFilter_Apply_002, TestSize.Level1)
{
    auto filter = std::make_unique<MockHighPassFilter>();
    ASSERT_NE(filter, nullptr);

    std::vector<float> input(100, 1.0f);
    std::vector<float> output(100);

    int32_t ret = filter->Apply(input, output);
    EXPECT_NE(ret, 0);
}

HWTEST_F(AudioHighPassFilterUnitTest, MockHighPassFilter_Apply_003, TestSize.Level1)
{
    auto filter = std::make_unique<MockHighPassFilter>();
    ASSERT_NE(filter, nullptr);
    EXPECT_EQ(filter->Init(2), 0);

    std::vector<float> input;
    std::vector<float> output;

    int32_t ret = filter->Apply(input, output);
    EXPECT_NE(ret, 0);
}

HWTEST_F(AudioHighPassFilterUnitTest, MockHighPassFilter_Apply_004, TestSize.Level1)
{
    auto filter = std::make_unique<MockHighPassFilter>();
    ASSERT_NE(filter, nullptr);
    EXPECT_EQ(filter->Init(2), 0);

    std::vector<float> input(100, 1.0f);
    std::vector<float> output(50, 0.0f);

    int32_t ret = filter->Apply(input, output);
    EXPECT_NE(ret, 0);
}

HWTEST_F(AudioHighPassFilterUnitTest, MockHighPassFilter_MultiChannel_001, TestSize.Level1)
{
    auto filter = std::make_unique<MockHighPassFilter>();
    ASSERT_NE(filter, nullptr);

    std::vector<int32_t> channels = {1, 2, 4, 8};
    for (auto ch : channels) {
        int32_t ret = filter->Init(ch);
        EXPECT_EQ(ret, 0);
        filter->Reset();
    }
}

HWTEST_F(AudioHighPassFilterUnitTest, MockHighPassFilter_LargeData_001, TestSize.Level1)
{
    auto filter = std::make_unique<MockHighPassFilter>();
    ASSERT_NE(filter, nullptr);
    EXPECT_EQ(filter->Init(2), 0);

    size_t largeSize = 48000;
    std::vector<float> input(largeSize, 1.0f);
    std::vector<float> output(largeSize);

    int32_t ret = filter->Apply(input, output);
    EXPECT_EQ(ret, 0);
    
    for (size_t i = 0; i < largeSize; ++i) {
        EXPECT_FLOAT_EQ(output[i], 1.0f);
    }
}

HWTEST_F(AudioHighPassFilterUnitTest, TestHighPassFilter_InitChannels_001, TestSize.Level1)
{
    TestHighPassFilter::Reset();
    TestHighPassFilter filter;

    filter.Init(2);
    EXPECT_TRUE(TestHighPassFilter::initCalled_);
    EXPECT_EQ(TestHighPassFilter::initChannels_, 2);
}

HWTEST_F(AudioHighPassFilterUnitTest, TestHighPassFilter_InitChannels_002, TestSize.Level1)
{
    TestHighPassFilter::Reset();
    TestHighPassFilter filter;

    filter.Init(1);
    EXPECT_TRUE(TestHighPassFilter::initCalled_);
    EXPECT_EQ(TestHighPassFilter::initChannels_, 1);
}

HWTEST_F(AudioHighPassFilterUnitTest, TestHighPassFilter_InitChannels_003, TestSize.Level1)
{
    TestHighPassFilter::Reset();
    TestHighPassFilter filter;

    int32_t ret = filter.Init(8);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(TestHighPassFilter::initChannels_, 8);
}

HWTEST_F(AudioHighPassFilterUnitTest, TestHighPassFilter_Apply_001, TestSize.Level1)
{
    TestHighPassFilter::Reset();
    TestHighPassFilter filter;
    filter.Init(2);

    std::vector<float> input(10, 1.0f);
    std::vector<float> output(10);

    filter.Apply(input, output);
    EXPECT_TRUE(TestHighPassFilter::applyCalled_);
    
    for (size_t i = 0; i < 10; ++i) {
        EXPECT_FLOAT_EQ(output[i], 0.95f);
    }
}

HWTEST_F(AudioHighPassFilterUnitTest, TestHighPassFilter_ApplyFail_001, TestSize.Level1)
{
    TestHighPassFilter::Reset();
    TestHighPassFilter filter(false);
    filter.Init(2);

    std::vector<float> input(10, 1.0f);
    std::vector<float> output(10);

    int32_t ret = filter.Apply(input, output);
    EXPECT_NE(ret, 0);
}

HWTEST_F(AudioHighPassFilterUnitTest, TestCreateFunc_001, TestSize.Level1)
{
    IHighPassFilter *filter = nullptr;
    int32_t ret = TestCreateFunc(&filter);
    EXPECT_EQ(ret, 0);
    EXPECT_NE(filter, nullptr);

    delete static_cast<TestHighPassFilter*>(filter);
}

HWTEST_F(AudioHighPassFilterUnitTest, TestCreateFunc_002, TestSize.Level1)
{
    IHighPassFilter *filter = nullptr;
    int32_t ret = TestCreateFuncNullFilter(&filter);
    EXPECT_NE(ret, 0);
    EXPECT_EQ(filter, nullptr);
}

HWTEST_F(AudioHighPassFilterUnitTest, TestCreateFunc_003, TestSize.Level1)
{
    IHighPassFilter *filter = nullptr;
    int32_t ret = TestCreateFunc(nullptr);
    EXPECT_NE(ret, 0);
    EXPECT_EQ(filter, nullptr);
}

HWTEST_F(AudioHighPassFilterUnitTest, TestCreateFuncFail_001, TestSize.Level1)
{
    IHighPassFilter *filter = nullptr;
    int32_t ret = TestCreateFuncFail(&filter);
    EXPECT_EQ(ret, 0);
    EXPECT_NE(filter, nullptr);

    int32_t initRet = filter->Init(2);
    EXPECT_NE(initRet, 0);

    delete static_cast<TestHighPassFilter*>(filter);
}

HWTEST_F(AudioHighPassFilterUnitTest, FilterIntegration_001, TestSize.Level1)
{
    TestHighPassFilter::Reset();
    IHighPassFilter *filterPtr = nullptr;
    ASSERT_EQ(TestCreateFunc(&filterPtr), 0);
    ASSERT_NE(filterPtr, nullptr);
    std::shared_ptr<IHighPassFilter> filter(filterPtr);

    EXPECT_EQ(filter->Init(2), 0);

    std::vector<float> input(1920, 0.5f);
    std::vector<float> output(1920);

    EXPECT_EQ(filter->Apply(input, output), 0);

    for (size_t i = 0; i < output.size(); ++i) {
        EXPECT_FLOAT_EQ(output[i], 0.475f);
    }
}

HWTEST_F(AudioHighPassFilterUnitTest, FilterIntegration_002, TestSize.Level1)
{
    TestHighPassFilter::Reset();
    IHighPassFilter *filterPtr = nullptr;
    ASSERT_EQ(TestCreateFuncFail(&filterPtr), 0);
    ASSERT_NE(filterPtr, nullptr);
    std::shared_ptr<IHighPassFilter> filter(filterPtr);

    EXPECT_NE(filter->Init(2), 0);
    
    std::vector<float> input(1920, 0.5f);
    std::vector<float> output(1920);

    EXPECT_NE(filter->Apply(input, output), 0);
}

HWTEST_F(AudioHighPassFilterUnitTest, MultipleFilterInstances_001, TestSize.Level1)
{
    std::vector<std::shared_ptr<IHighPassFilter>> filters;

    for (int i = 0; i < 5; ++i) {
        IHighPassFilter *filterPtr = nullptr;
        EXPECT_EQ(TestCreateFunc(&filterPtr), 0);
        EXPECT_NE(filterPtr, nullptr);
        filters.emplace_back(filterPtr);
    }

    for (auto &filter : filters) {
        EXPECT_EQ(filter->Init(2), 0);
        
        std::vector<float> input(100, 1.0f);
        std::vector<float> output(100);
        
        EXPECT_EQ(filter->Apply(input, output), 0);
    }
}

HWTEST_F(AudioHighPassFilterUnitTest, EdgeCases_001, TestSize.Level1)
{
    TestHighPassFilter filter;

    EXPECT_NE(filter.Init(-100), 0);
    EXPECT_NE(filter.Init(0), 0);
}

HWTEST_F(AudioHighPassFilterUnitTest, EdgeCases_002, TestSize.Level1)
{
    TestHighPassFilter filter;
    EXPECT_EQ(filter.Init(2), 0);

    std::vector<float> input(1, 1.0f);
    std::vector<float> output(1);

    EXPECT_EQ(filter.Apply(input, output), 0);
    EXPECT_FLOAT_EQ(output[0], 0.95f);
}

HWTEST_F(AudioHighPassFilterUnitTest, EdgeCases_003, TestSize.Level1)
{
    TestHighPassFilter filter;
    EXPECT_EQ(filter.Init(1), 0);

    std::vector<float> input(960, 0.0f);
    std::vector<float> output(960);

    EXPECT_EQ(filter.Apply(input, output), 0);
    
    for (size_t i = 0; i < output.size(); ++i) {
        EXPECT_FLOAT_EQ(output[i], 0.0f);
    }
}

HWTEST_F(AudioHighPassFilterUnitTest, EdgeCases_004, TestSize.Level1)
{
    TestHighPassFilter filter;
    EXPECT_EQ(filter.Init(2), 0);

    float minVal = -1.0f;
    float maxVal = 1.0f;
    std::vector<float> input = {minVal, maxVal, 0.0f, 0.5f, -0.5f};
    std::vector<float> output(input.size());

    EXPECT_EQ(filter.Apply(input, output), 0);

    EXPECT_FLOAT_EQ(output[0], minVal * 0.95f);
    EXPECT_FLOAT_EQ(output[1], maxVal * 0.95f);
    EXPECT_FLOAT_EQ(output[2], 0.0f);
    EXPECT_FLOAT_EQ(output[3], 0.5f * 0.95f);
    EXPECT_FLOAT_EQ(output[4], -0.5f * 0.95f);
}
} // namespace AudioStandard
} // namespace OHOS
