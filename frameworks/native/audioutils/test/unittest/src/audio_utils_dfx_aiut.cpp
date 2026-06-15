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

#include "audio_utils_dfx_aiut.h"
#include "aiut_log.h"
#include "dfx_collector.h"
#include "dfx_utils.h"
#include "dfx_stat.h"
#include "audio_format_utils.h"
#include "audio_utils.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

namespace {
constexpr uint8_t TEST_BYTE_1 = 0x01;
constexpr uint8_t TEST_BYTE_2 = 0x02;
constexpr uint8_t TEST_BYTE_3 = 0x03;
constexpr uint8_t TEST_BYTE_4 = 0x04;

constexpr int32_t TEST_DFX_INDEX_1 = 100;
constexpr int32_t TEST_DFX_INDEX_2 = 200;
}

// ==================== AudioUtilsDfxAiut ====================

void AudioUtilsDfxAiut::SetUpTestCase(void) {}
void AudioUtilsDfxAiut::TearDownTestCase(void) {}
void AudioUtilsDfxAiut::SetUp(void) {}
void AudioUtilsDfxAiut::TearDown(void) {}

// ==================== DfxCollector Tests ====================

class TestDfxCollector : public DfxCollector<DfxStatInt32> {
public:
    void FlushDfxMsg(uint32_t index, int32_t appUid) override
    {
        flushCallCount_++;
        lastIndex_ = index;
        lastAppUid_ = appUid;
    }

    int32_t flushCallCount_ = 0;
    uint32_t lastIndex_ = 0;
    int32_t lastAppUid_ = 0;
};

/**
 * @tc.name  : Test DfxCollector AddDfxMsg adds entry
 * @tc.number: DfxCollector_AddDfxMsg_001
 * @tc.desc  : Test DfxCollector::AddDfxMsg inserts a new index with the given info.
 */
HWTEST_F(AudioUtilsDfxAiut, DfxCollector_AddDfxMsg_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto collector = std::make_shared<TestDfxCollector>();
    AIUT_ASSERT_NE_INFO_LOG(collector, nullptr, "collector is null");

    DfxStatInt32 info(TEST_BYTE_1, TEST_BYTE_2, TEST_BYTE_3, TEST_BYTE_4);
    collector->AddDfxMsg(TEST_DFX_INDEX_1, info);

    AIUT_EXPECT_TRUE_INFO_LOG(collector->IsExist(TEST_DFX_INDEX_1),
        "index should exist after AddDfxMsg");
}

/**
 * @tc.name  : Test DfxCollector AddDfxMsg appends to existing index
 * @tc.number: DfxCollector_AddDfxMsg_002
 * @tc.desc  : Test DfxCollector::AddDfxMsg appends info when index already exists.
 */
HWTEST_F(AudioUtilsDfxAiut, DfxCollector_AddDfxMsg_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto collector = std::make_shared<TestDfxCollector>();
    AIUT_ASSERT_NE_INFO_LOG(collector, nullptr, "collector is null");

    DfxStatInt32 info1(TEST_BYTE_1, TEST_BYTE_2, TEST_BYTE_3, TEST_BYTE_4);
    DfxStatInt32 info2(TEST_BYTE_4, TEST_BYTE_3, TEST_BYTE_2, TEST_BYTE_1);
    collector->AddDfxMsg(TEST_DFX_INDEX_1, info1);
    collector->AddDfxMsg(TEST_DFX_INDEX_1, info2);

    // Should have 2 items in the list for this index
    AIUT_EXPECT_EQ_INFO_LOG(collector->dfxInfos_[TEST_DFX_INDEX_1].size(),
        static_cast<size_t>(2), "should have 2 items for the same index");
}

/**
 * @tc.name  : Test DfxCollector IsExist returns false for unknown index
 * @tc.number: DfxCollector_IsExist_UnknownIndex_001
 * @tc.desc  : Test DfxCollector::IsExist returns false for a non-existent index.
 */
HWTEST_F(AudioUtilsDfxAiut, DfxCollector_IsExist_UnknownIndex_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto collector = std::make_shared<TestDfxCollector>();
    AIUT_ASSERT_NE_INFO_LOG(collector, nullptr, "collector is null");

    AIUT_EXPECT_FALSE_INFO_LOG(collector->IsExist(TEST_DFX_INDEX_2),
        "IsExist should return false for unknown index");
}

/**
 * @tc.name  : Test DfxCollector multiple indices
 * @tc.number: DfxCollector_MultipleIndices_001
 * @tc.desc  : Test DfxCollector can store info under different indices.
 */
HWTEST_F(AudioUtilsDfxAiut, DfxCollector_MultipleIndices_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto collector = std::make_shared<TestDfxCollector>();
    AIUT_ASSERT_NE_INFO_LOG(collector, nullptr, "collector is null");

    DfxStatInt32 info(TEST_BYTE_1, TEST_BYTE_2, TEST_BYTE_3, TEST_BYTE_4);
    collector->AddDfxMsg(TEST_DFX_INDEX_1, info);
    collector->AddDfxMsg(TEST_DFX_INDEX_2, info);

    AIUT_EXPECT_TRUE_INFO_LOG(collector->IsExist(TEST_DFX_INDEX_1),
        "index 1 should exist");
    AIUT_EXPECT_TRUE_INFO_LOG(collector->IsExist(TEST_DFX_INDEX_2),
        "index 2 should exist");
    AIUT_EXPECT_EQ_INFO_LOG(collector->dfxInfos_.size(), static_cast<size_t>(2),
        "should have 2 separate indices");
}

// ==================== DfxUtils Tests ====================

/**
 * @tc.name  : Test DfxUtils SerializeToUint32
 * @tc.number: DfxUtils_SerializeToUint32_001
 * @tc.desc  : Test DfxUtils::SerializeToUint32 correctly packs four bytes into uint32.
 */
HWTEST_F(AudioUtilsDfxAiut, DfxUtils_SerializeToUint32_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    DfxStatInt32 data(TEST_BYTE_1, TEST_BYTE_2, TEST_BYTE_3, TEST_BYTE_4);
    uint32_t result = DfxUtils::SerializeToUint32(data);

    // Expected: firstByte=0x01 in MSB, fourthByte=0x04 in LSB
    // result = 0x01020304
    uint32_t expected = (static_cast<uint32_t>(TEST_BYTE_1) << 24) |
        (static_cast<uint32_t>(TEST_BYTE_2) << 16) |
        (static_cast<uint32_t>(TEST_BYTE_3) << 8) |
        static_cast<uint32_t>(TEST_BYTE_4);
    AIUT_EXPECT_EQ_INFO_LOG(result, expected,
        "SerializeToUint32 mismatch, result=0x%08X", result);
}

/**
 * @tc.name  : Test DfxUtils SerializeToUint32 with zeros
 * @tc.number: DfxUtils_SerializeToUint32_002
 * @tc.desc  : Test DfxUtils::SerializeToUint32 with all zero bytes.
 */
HWTEST_F(AudioUtilsDfxAiut, DfxUtils_SerializeToUint32_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    DfxStatInt32 data(0, 0, 0, 0);
    uint32_t result = DfxUtils::SerializeToUint32(data);
    AIUT_EXPECT_EQ_INFO_LOG(result, static_cast<uint32_t>(0),
        "all-zero bytes should serialize to 0, result=0x%08X", result);
}

/**
 * @tc.name  : Test DfxUtils SerializeToJSONString RendererStats
 * @tc.number: DfxUtils_SerializeRendererStats_001
 * @tc.desc  : Test DfxUtils::SerializeToJSONString produces valid JSON for RendererStats.
 */
HWTEST_F(AudioUtilsDfxAiut, DfxUtils_SerializeRendererStats_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    RendererStats stats;
    stats.samplingRate = 48000;
    stats.duration = 1000;
    stats.underrunCnt = 5;
    stats.originalFlag = 1;
    stats.zeroDataPercent = 10;
    stats.frameWritten = 96000;

    std::string json = DfxUtils::SerializeToJSONString(stats);
    AIUT_EXPECT_FALSE_INFO_LOG(json.empty(), "JSON string should not be empty");
    AIUT_EXPECT_NE_INFO_LOG(json.find("sampleRate"), std::string::npos,
        "JSON should contain 'sampleRate'");
    AIUT_EXPECT_NE_INFO_LOG(json.find("duration"), std::string::npos,
        "JSON should contain 'duration'");
}

/**
 * @tc.name  : Test DfxUtils SerializeToJSONString CapturerStats
 * @tc.number: DfxUtils_SerializeCapturerStats_001
 * @tc.desc  : Test DfxUtils::SerializeToJSONString produces valid JSON for CapturerStats.
 */
HWTEST_F(AudioUtilsDfxAiut, DfxUtils_SerializeCapturerStats_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    CapturerStats stats;
    stats.samplingRate = 16000;
    stats.duration = 500;

    std::string json = DfxUtils::SerializeToJSONString(stats);
    AIUT_EXPECT_FALSE_INFO_LOG(json.empty(), "JSON string should not be empty");
    AIUT_EXPECT_NE_INFO_LOG(json.find("sampleRate"), std::string::npos,
        "JSON should contain 'sampleRate'");
}

/**
 * @tc.name  : Test DfxUtils SerializeToJSONString InterruptEffect vector
 * @tc.number: DfxUtils_SerializeInterruptEffects_001
 * @tc.desc  : Test DfxUtils::SerializeToJSONString produces valid JSON for
 *             vector<InterruptEffect>.
 */
HWTEST_F(AudioUtilsDfxAiut, DfxUtils_SerializeInterruptEffects_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    std::vector<InterruptEffect> effects;
    InterruptEffect effect;
    effect.bundleName = "com.test.app";
    effect.streamUsage = 1;
    effect.appState = 2;
    effect.interruptEvent = 3;
    effects.push_back(effect);

    std::string json = DfxUtils::SerializeToJSONString(effects);
    AIUT_EXPECT_FALSE_INFO_LOG(json.empty(), "JSON string should not be empty");
    AIUT_EXPECT_NE_INFO_LOG(json.find("bundleName"), std::string::npos,
        "JSON should contain 'bundleName'");
}

// ==================== FormatUtils Tests ====================

/**
 * @tc.name  : Test FormatUtils StringToInt32 valid input
 * @tc.number: FormatUtils_StringToInt32_001
 * @tc.desc  : Test FormatUtils::StringToInt32 correctly parses a valid integer string.
 */
HWTEST_F(AudioUtilsDfxAiut, FormatUtils_StringToInt32_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int32_t result = FormatUtils::StringToInt32("42", 0);
    AIUT_EXPECT_EQ_INFO_LOG(result, 42, "expected 42, got=%d", result);
}

/**
 * @tc.name  : Test FormatUtils StringToInt32 empty string returns default
 * @tc.number: FormatUtils_StringToInt32_002
 * @tc.desc  : Test FormatUtils::StringToInt32 returns default value for empty string.
 */
HWTEST_F(AudioUtilsDfxAiut, FormatUtils_StringToInt32_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int32_t result = FormatUtils::StringToInt32("", -1);
    AIUT_EXPECT_EQ_INFO_LOG(result, -1, "expected default -1, got=%d", result);
}

/**
 * @tc.name  : Test FormatUtils StringToInt32 negative number
 * @tc.number: FormatUtils_StringToInt32_003
 * @tc.desc  : Test FormatUtils::StringToInt32 correctly parses a negative integer string.
 */
HWTEST_F(AudioUtilsDfxAiut, FormatUtils_StringToInt32_003, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int32_t result = FormatUtils::StringToInt32("-100", 0);
    AIUT_EXPECT_EQ_INFO_LOG(result, -100, "expected -100, got=%d", result);
}

/**
 * @tc.name  : Test FormatUtils StringToInt32 invalid string returns default
 * @tc.number: FormatUtils_StringToInt32_004
 * @tc.desc  : Test FormatUtils::StringToInt32 returns default value for non-numeric string.
 */
HWTEST_F(AudioUtilsDfxAiut, FormatUtils_StringToInt32_004, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int32_t result = FormatUtils::StringToInt32("abc", -99);
    AIUT_EXPECT_EQ_INFO_LOG(result, -99, "expected default -99 for invalid, got=%d", result);
}

/**
 * @tc.name  : Test FormatUtils StringToInt32 partial parse returns default
 * @tc.number: FormatUtils_StringToInt32_005
 * @tc.desc  : Test FormatUtils::StringToInt32 returns default for partially valid string
 *             (e.g., "12abc").
 */
HWTEST_F(AudioUtilsDfxAiut, FormatUtils_StringToInt32_005, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int32_t result = FormatUtils::StringToInt32("12abc", -99);
    AIUT_EXPECT_EQ_INFO_LOG(result, -99,
        "expected default for partial parse, got=%d", result);
}

} // namespace AudioStandard
} // namespace OHOS
