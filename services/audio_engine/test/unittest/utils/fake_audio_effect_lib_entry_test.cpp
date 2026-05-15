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

#include "gtest/gtest.h"
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

#include "fake_audio_effect_lib_entry.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class FakeAudioEffectLibEntryTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        fake_ = std::make_unique<FakeAudioEffectLibEntry>();
        library_ = fake_->GetLibrary();
    }

    void TearDown() override
    {
        fake_.reset();
    }

    std::unique_ptr<FakeAudioEffectLibEntry> fake_;
    AudioEffectLibrary library_;
};

// F-1: createEffect returns valid handle on success
HWTEST_F(FakeAudioEffectLibEntryTest, createEffect_success, TestSize.Level0)
{
    AudioEffectDescriptor desc;
    desc.libraryName = "FakeEffectLib";
    desc.effectName = "test_effect";

    AudioEffectHandle handle = nullptr;
    int32_t ret = library_.createEffect(desc, &handle);

    EXPECT_EQ(ret, 0);
    ASSERT_NE(handle, nullptr);
    EXPECT_EQ(fake_->GetCreateCount(), 1u);

    // Cleanup
    library_.releaseEffect(handle);
}

// F-2: createEffect fails when configured to fail
HWTEST_F(FakeAudioEffectLibEntryTest, createEffect_fail, TestSize.Level0)
{
    fake_->SetCreateFail(true);

    AudioEffectDescriptor desc;
    desc.libraryName = "FakeEffectLib";
    desc.effectName = "test_effect";

    AudioEffectHandle handle = nullptr;
    int32_t ret = library_.createEffect(desc, &handle);

    EXPECT_NE(ret, 0);
    EXPECT_EQ(handle, nullptr);
}

// F-3: process does passthrough copy of f32 data
HWTEST_F(FakeAudioEffectLibEntryTest, process_passthrough, TestSize.Level0)
{
    AudioEffectDescriptor desc;
    AudioEffectHandle handle = nullptr;
    ASSERT_EQ(library_.createEffect(desc, &handle), 0);
    ASSERT_NE(handle, nullptr);

    auto *iface = *handle;

    static constexpr size_t frameCount = 4;
    float inData[frameCount] = {1.0f, 2.0f, 3.0f, 4.0f};
    float outData[frameCount] = {0};

    AudioBuffer inBuf{};
    inBuf.frameLength = frameCount;
    inBuf.f32 = inData;

    AudioBuffer outBuf{};
    outBuf.frameLength = frameCount;
    outBuf.f32 = outData;

    int32_t ret = iface->process(handle, &inBuf, &outBuf);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(fake_->GetProcessCount(), 1u);

    for (size_t i = 0; i < frameCount; i++) {
        EXPECT_FLOAT_EQ(outBuf.f32[i], inData[i]) << "mismatch at index " << i;
    }

    library_.releaseEffect(handle);
}

// F-4: process fails when configured to fail
HWTEST_F(FakeAudioEffectLibEntryTest, process_fail, TestSize.Level0)
{
    AudioEffectDescriptor desc;
    AudioEffectHandle handle = nullptr;
    ASSERT_EQ(library_.createEffect(desc, &handle), 0);
    ASSERT_NE(handle, nullptr);

    fake_->SetProcessFail(true);

    auto *iface = *handle;

    float inData[4] = {};
    float outData[4] = {};
    AudioBuffer inBuf{};
    inBuf.frameLength = 4;
    inBuf.f32 = inData;
    AudioBuffer outBuf{};
    outBuf.frameLength = 4;
    outBuf.f32 = outData;

    int32_t ret = iface->process(handle, &inBuf, &outBuf);
    EXPECT_NE(ret, 0);
    EXPECT_EQ(fake_->GetProcessCount(), 1u);

    library_.releaseEffect(handle);
}

// F-5: flush command succeeds by default
HWTEST_F(FakeAudioEffectLibEntryTest, flush_support, TestSize.Level0)
{
    AudioEffectDescriptor desc;
    AudioEffectHandle handle = nullptr;
    ASSERT_EQ(library_.createEffect(desc, &handle), 0);
    ASSERT_NE(handle, nullptr);

    auto *iface = *handle;

    int32_t ret = iface->command(handle, EFFECT_CMD_FLUSH, nullptr, nullptr);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(fake_->GetFlushCount(), 1u);

    library_.releaseEffect(handle);
}

// F-6: flush command fails when configured to fail
HWTEST_F(FakeAudioEffectLibEntryTest, flush_not_supported, TestSize.Level0)
{
    AudioEffectDescriptor desc;
    AudioEffectHandle handle = nullptr;
    ASSERT_EQ(library_.createEffect(desc, &handle), 0);
    ASSERT_NE(handle, nullptr);

    fake_->SetFlushFail(true);
    auto *iface = *handle;

    int32_t ret = iface->command(handle, EFFECT_CMD_FLUSH, nullptr, nullptr);
    EXPECT_NE(ret, 0);
    EXPECT_EQ(fake_->GetFlushCount(), 1u);

    library_.releaseEffect(handle);
}

// F-7: GET_PARAM returns configured preheatFrames
HWTEST_F(FakeAudioEffectLibEntryTest, getParam_preheatFrames, TestSize.Level0)
{
    AudioEffectDescriptor desc;
    AudioEffectHandle handle = nullptr;
    ASSERT_EQ(library_.createEffect(desc, &handle), 0);
    ASSERT_NE(handle, nullptr);

    fake_->SetPreheatFrames(1024);
    auto *iface = *handle;

    uint32_t replyData = 0;
    AudioEffectTransInfo reply{};
    reply.data = &replyData;
    reply.size = sizeof(uint32_t);

    int32_t ret = iface->command(handle, EFFECT_CMD_GET_PARAM, nullptr, &reply);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(replyData, 1024u);
    EXPECT_EQ(fake_->GetParamLog().size(), 1u);

    library_.releaseEffect(handle);
}

TEST(FakeAudioEffectLibEntry, FeedInput_success)
{
    FakeAudioEffectLibEntry fake;
    auto lib = fake.GetLibrary();
    AudioEffectHandle handle = nullptr;
    lib.createEffect({"test", "test"}, &handle);
    ASSERT_NE(handle, nullptr);

    float data[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    AudioBuffer inBuf = {};
    inBuf.f32 = data;
    inBuf.frameLength = 4;
    int32_t ret = (*handle)->feedInput(handle, &inBuf);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(fake.GetFeedInputCount(), 1u);

    lib.releaseEffect(handle);
}

TEST(FakeAudioEffectLibEntry, GetOutput_has_data)
{
    FakeAudioEffectLibEntry fake;
    fake.SetProcessDelayMs(0);
    auto lib = fake.GetLibrary();
    AudioEffectHandle handle = nullptr;
    lib.createEffect({"test", "test"}, &handle);

    // Feed enough data to reach process threshold (192000 floats for stereo)
    std::vector<float> data(FAKE_PROCESS_THRESHOLD_FLOATS);
    for (size_t i = 0; i < data.size(); i += 2) {
        data[i] = 1.0f;     // L
        data[i + 1] = 2.0f; // R
    }
    AudioBuffer inBuf = {};
    inBuf.f32 = data.data();
    inBuf.frameLength = data.size();
    (*handle)->feedInput(handle, &inBuf);
    fake.WaitForProcessing();

    float out[4] = {};
    AudioBuffer outBuf = {};
    outBuf.f32 = out;
    outBuf.frameLength = 4;
    int32_t ret = (*handle)->getOutput(handle, &outBuf);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(outBuf.frameLength, 4u);
    // L/R swapped: original [1,2,1,2] -> [2,1,2,1]
    EXPECT_FLOAT_EQ(out[0], 2.0f);
    EXPECT_FLOAT_EQ(out[1], 1.0f);
    EXPECT_FLOAT_EQ(out[2], 2.0f);
    EXPECT_FLOAT_EQ(out[3], 1.0f);

    lib.releaseEffect(handle);
}

TEST(FakeAudioEffectLibEntry, GetOutput_empty)
{
    FakeAudioEffectLibEntry fake;
    auto lib = fake.GetLibrary();
    AudioEffectHandle handle = nullptr;
    lib.createEffect({"test", "test"}, &handle);

    float out[4] = {};
    AudioBuffer outBuf = {};
    outBuf.f32 = out;
    outBuf.frameLength = 4;
    int32_t ret = (*handle)->getOutput(handle, &outBuf);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(outBuf.frameLength, 0u);

    lib.releaseEffect(handle);
}

TEST(FakeAudioEffectLibEntry, FeedInput_fail)
{
    FakeAudioEffectLibEntry fake;
    fake.SetFeedInputFail(true);
    auto lib = fake.GetLibrary();
    AudioEffectHandle handle = nullptr;
    lib.createEffect({"test", "test"}, &handle);

    float data[4] = {};
    AudioBuffer inBuf = {};
    inBuf.f32 = data;
    inBuf.frameLength = 4;
    int32_t ret = (*handle)->feedInput(handle, &inBuf);
    EXPECT_NE(ret, 0);

    lib.releaseEffect(handle);
}

} // namespace AudioStandard
} // namespace OHOS
