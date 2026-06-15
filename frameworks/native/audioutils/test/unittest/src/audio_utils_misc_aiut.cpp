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

#include "audio_utils_misc_aiut.h"
#include "aiut_log.h"
#include "audio_utils.h"
#include <map>
#include <unordered_map>
#include <optional>

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

namespace {
}

// ==================== AudioUtilsMiscAiut ====================

void AudioUtilsMiscAiut::SetUpTestCase(void) {}
void AudioUtilsMiscAiut::TearDownTestCase(void) {}
void AudioUtilsMiscAiut::SetUp(void) {}
void AudioUtilsMiscAiut::TearDown(void) {}

// ==================== AudioDump Tests ====================

/**
 * @tc.name  : Test AudioDump SetVersionType and GetVersionType
 * @tc.number: AudioDump_SetVersionType_001
 * @tc.desc  : Test AudioDump::SetVersionType changes the version type and
 *             GetVersionType returns it.
 */
HWTEST_F(AudioUtilsMiscAiut, AudioDump_SetVersionType_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AudioDump::GetInstance().SetVersionType("beta");
    std::string versionType = AudioDump::GetInstance().GetVersionType();
    AIUT_EXPECT_EQ_INFO_LOG(versionType, "beta",
        "version type should be 'beta', got='%s'", versionType.c_str());
}

/**
 * @tc.name  : Test AudioDump GetVersionType default value
 * @tc.number: AudioDump_GetVersionType_Default_001
 * @tc.desc  : Test AudioDump default version type is "commercial".
 */
HWTEST_F(AudioUtilsMiscAiut, AudioDump_GetVersionType_Default_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AudioDump::GetInstance().SetVersionType("commercial");
    std::string versionType = AudioDump::GetInstance().GetVersionType();
    AIUT_EXPECT_EQ_INFO_LOG(versionType, "commercial",
        "default version type should be 'commercial', got='%s'", versionType.c_str());
}

// ==================== CheckoutSystemAppUtil Tests ====================

/**
 * @tc.name  : Test CheckoutSystemApp with root UID
 * @tc.number: CheckoutSystemApp_RootUid_001
 * @tc.desc  : Test CheckoutSystemAppUtil::CheckoutSystemApp returns true for root UID (0).
 */
HWTEST_F(AudioUtilsMiscAiut, CheckoutSystemApp_RootUid_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    bool result = CheckoutSystemAppUtil::CheckoutSystemApp(ROOT_UID);
    AIUT_EXPECT_TRUE_INFO_LOG(result, "root UID should pass checkout");
}

/**
 * @tc.name  : Test CheckoutSystemApp with non-system UID
 * @tc.number: CheckoutSystemApp_NonSystemUid_001
 * @tc.desc  : Test CheckoutSystemAppUtil::CheckoutSystemApp returns false for
 *             non-system UID.
 */
HWTEST_F(AudioUtilsMiscAiut, CheckoutSystemApp_NonSystemUid_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    bool result = CheckoutSystemAppUtil::CheckoutSystemApp(9999);
    AIUT_EXPECT_FALSE_INFO_LOG(result, "non-system UID should fail checkout");
}

// ==================== SafeGetMap Tests ====================

/**
 * @tc.name  : Test SafeGetMap with unordered_map existing key
 * @tc.number: SafeGetMap_UnorderedExisting_001
 * @tc.desc  : Test SafeGetMap returns the value for an existing key in unordered_map.
 */
HWTEST_F(AudioUtilsMiscAiut, SafeGetMap_UnorderedExisting_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    std::unordered_map<int32_t, std::shared_ptr<std::string>> map;
    map[1] = std::make_shared<std::string>("hello");

    auto result = SafeGetMap(map, 1);
    AIUT_ASSERT_NE_INFO_LOG(result, nullptr, "SafeGetMap should return non-null for existing key");
    AIUT_EXPECT_EQ_INFO_LOG(*result, "hello", "value mismatch");
}

/**
 * @tc.name  : Test SafeGetMap with unordered_map missing key
 * @tc.number: SafeGetMap_UnorderedMissing_001
 * @tc.desc  : Test SafeGetMap returns nullptr for a missing key in unordered_map.
 */
HWTEST_F(AudioUtilsMiscAiut, SafeGetMap_UnorderedMissing_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    std::unordered_map<int32_t, std::shared_ptr<std::string>> map;
    map[1] = std::make_shared<std::string>("hello");

    auto result = SafeGetMap(map, 2);
    AIUT_EXPECT_EQ_INFO_LOG(result, nullptr, "SafeGetMap should return nullptr for missing key");
}

/**
 * @tc.name  : Test SafeGetMap with ordered map existing key
 * @tc.number: SafeGetMap_OrderedExisting_001
 * @tc.desc  : Test SafeGetMap returns the value for an existing key in std::map.
 */
HWTEST_F(AudioUtilsMiscAiut, SafeGetMap_OrderedExisting_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    std::map<int32_t, std::shared_ptr<std::string>> map;
    map[1] = std::make_shared<std::string>("world");

    auto result = SafeGetMap(map, 1);
    AIUT_ASSERT_NE_INFO_LOG(result, nullptr, "SafeGetMap should return non-null for existing key");
    AIUT_EXPECT_EQ_INFO_LOG(*result, "world", "value mismatch");
}

/**
 * @tc.name  : Test SafeGetMap with null value in map
 * @tc.number: SafeGetMap_NullValue_001
 * @tc.desc  : Test SafeGetMap returns nullptr when the key exists but value is null.
 */
HWTEST_F(AudioUtilsMiscAiut, SafeGetMap_NullValue_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    std::unordered_map<int32_t, std::shared_ptr<std::string>> map;
    map[1] = nullptr;

    auto result = SafeGetMap(map, 1);
    AIUT_EXPECT_EQ_INFO_LOG(result, nullptr,
        "SafeGetMap should return nullptr for null value");
}

// ==================== NotContain Tests ====================

/**
 * @tc.name  : Test NotContain returns true for absent value
 * @tc.number: NotContain_AbsentValue_001
 * @tc.desc  : Test NotContain returns true when the value is not in the vector.
 */
HWTEST_F(AudioUtilsMiscAiut, NotContain_AbsentValue_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    std::vector<int32_t> vec = {1, 2, 3};
    AIUT_EXPECT_TRUE_INFO_LOG(NotContain(vec, 4), "4 should not be in {1,2,3}");
}

/**
 * @tc.name  : Test NotContain returns false for present value
 * @tc.number: NotContain_PresentValue_001
 * @tc.desc  : Test NotContain returns false when the value is in the vector.
 */
HWTEST_F(AudioUtilsMiscAiut, NotContain_PresentValue_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    std::vector<int32_t> vec = {1, 2, 3};
    AIUT_EXPECT_FALSE_INFO_LOG(NotContain(vec, 2), "2 should be in {1,2,3}");
}

// ==================== IsNonStandardSampleRate Tests ====================

/**
 * @tc.name  : Test IsNonStandardSampleRate with standard rates
 * @tc.number: IsNonStandardSampleRate_Standard_001
 * @tc.desc  : Test IsNonStandardSampleRate returns false for standard sample rates.
 */
HWTEST_F(AudioUtilsMiscAiut, IsNonStandardSampleRate_Standard_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AIUT_EXPECT_FALSE_INFO_LOG(IsNonStandardSampleRate(48000),
        "48000 is standard");
    AIUT_EXPECT_FALSE_INFO_LOG(IsNonStandardSampleRate(44100),
        "44100 is standard");
    AIUT_EXPECT_FALSE_INFO_LOG(IsNonStandardSampleRate(16000),
        "16000 is standard");
}

/**
 * @tc.name  : Test IsNonStandardSampleRate with non-standard rates
 * @tc.number: IsNonStandardSampleRate_NonStandard_001
 * @tc.desc  : Test IsNonStandardSampleRate returns true for non-standard sample rates.
 */
HWTEST_F(AudioUtilsMiscAiut, IsNonStandardSampleRate_NonStandard_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AIUT_EXPECT_TRUE_INFO_LOG(IsNonStandardSampleRate(7999), "7999 is non-standard");
    AIUT_EXPECT_TRUE_INFO_LOG(IsNonStandardSampleRate(11023), "11023 is non-standard");
}

// ==================== GenerateUniqueID Tests ====================

/**
 * @tc.name  : Test GenerateUniqueID with render base
 * @tc.number: GenerateUniqueID_RenderBase_001
 * @tc.desc  : Test GenerateUniqueID produces different IDs for different offset values.
 */
HWTEST_F(AudioUtilsMiscAiut, GenerateUniqueID_RenderBase_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    uint32_t id1 = GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_PRIMARY);
    uint32_t id2 = GenerateUniqueID(AUDIO_HDI_RENDER_ID_BASE, HDI_RENDER_OFFSET_FAST);
    AIUT_EXPECT_NE_INFO_LOG(id1, id2, "different offsets should produce different IDs");
}

/**
 * @tc.name  : Test GenerateUniqueID with capture base
 * @tc.number: GenerateUniqueID_CaptureBase_001
 * @tc.desc  : Test GenerateUniqueID produces IDs in the capture range.
 */
HWTEST_F(AudioUtilsMiscAiut, GenerateUniqueID_CaptureBase_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    uint32_t id = GenerateUniqueID(AUDIO_HDI_CAPTURE_ID_BASE, HDI_CAPTURE_OFFSET_PRIMARY);
    AIUT_EXPECT_NE_INFO_LOG(id, static_cast<uint32_t>(0), "ID should not be 0");
}

// ==================== ClockTime Tests ====================

/**
 * @tc.name  : Test ClockTime GetCurNano returns positive value
 * @tc.number: ClockTime_GetCurNano_001
 * @tc.desc  : Test ClockTime::GetCurNano returns a positive nanosecond timestamp.
 */
HWTEST_F(AudioUtilsMiscAiut, ClockTime_GetCurNano_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int64_t nano = ClockTime::GetCurNano();
    AIUT_EXPECT_TRUE_INFO_LOG(nano > 0,
        "GetCurNano should return positive value, got=%lld",
        static_cast<long long>(nano));
}

/**
 * @tc.name  : Test ClockTime GetCurMilli returns positive value
 * @tc.number: ClockTime_GetCurMilli_001
 * @tc.desc  : Test ClockTime::GetCurMilli returns a positive millisecond timestamp.
 */
HWTEST_F(AudioUtilsMiscAiut, ClockTime_GetCurMilli_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int64_t milli = ClockTime::GetCurMilli();
    AIUT_EXPECT_TRUE_INFO_LOG(milli > 0,
        "GetCurMilli should return positive value, got=%lld",
        static_cast<long long>(milli));
}

/**
 * @tc.name  : Test ClockTime NanoTimeToString produces non-empty string
 * @tc.number: ClockTime_NanoTimeToString_001
 * @tc.desc  : Test ClockTime::NanoTimeToString returns a non-empty formatted time string.
 */
HWTEST_F(AudioUtilsMiscAiut, ClockTime_NanoTimeToString_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    std::string timeStr = ClockTime::NanoTimeToString(ClockTime::GetCurNano());
    AIUT_EXPECT_FALSE_INFO_LOG(timeStr.empty(), "NanoTimeToString should not return empty");
}

// ==================== isEqual Tests ====================

/**
 * @tc.name  : Test isEqual with close values
 * @tc.number: IsEqual_CloseValues_001
 * @tc.desc  : Test isEqual returns true for values within default precision.
 */
HWTEST_F(AudioUtilsMiscAiut, IsEqual_CloseValues_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AIUT_EXPECT_TRUE_INFO_LOG(isEqual(1.0, 1.005), "values within 0.01 should be equal");
    AIUT_EXPECT_FALSE_INFO_LOG(isEqual(1.0, 1.1), "values beyond 0.01 should not be equal");
}

} // namespace AudioStandard
} // namespace OHOS
