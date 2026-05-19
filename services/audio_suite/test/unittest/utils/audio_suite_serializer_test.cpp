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
#include "audio_suite_serializer.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace AudioSuite;
using namespace testing::ext;
using namespace testing;

namespace {
class AudioSuiteSerializerTest : public testing::Test {
public:
    void SetUp() {};
    void TearDown() {};
};

HWTEST_F(AudioSuiteSerializerTest, SplitString_001, TestSize.Level0)
{
    std::string input = "1,2,3,4,5";
    std::vector<std::string> result = SplitString(input, ',');
    EXPECT_EQ(result.size(), 5U);
    EXPECT_EQ(result[0], "1");
    EXPECT_EQ(result[1], "2");
    EXPECT_EQ(result[2], "3");
    EXPECT_EQ(result[3], "4");
    EXPECT_EQ(result[4], "5");
}

HWTEST_F(AudioSuiteSerializerTest, SplitString_002, TestSize.Level0)
{
    std::string input = "1.5,2.5,3.5";
    std::vector<std::string> result = SplitString(input, ',');
    EXPECT_EQ(result.size(), 3U);
    EXPECT_EQ(result[0], "1.5");
    EXPECT_EQ(result[1], "2.5");
    EXPECT_EQ(result[2], "3.5");
}

HWTEST_F(AudioSuiteSerializerTest, SplitString_003, TestSize.Level0)
{
    std::string input = "";
    std::vector<std::string> result = SplitString(input, ',');
    EXPECT_EQ(result.size(), 0U);
}

HWTEST_F(AudioSuiteSerializerTest, StringConverter_001, TestSize.Level0)
{
    std::string input = "123";
    int32_t value = 0;
    bool result = StringConverter(input, value);
    EXPECT_TRUE(result);
    EXPECT_EQ(value, 123);
}

HWTEST_F(AudioSuiteSerializerTest, StringConverter_002, TestSize.Level0)
{
    std::string input = "-456";
    int32_t value = 0;
    bool result = StringConverter(input, value);
    EXPECT_TRUE(result);
    EXPECT_EQ(value, -456);
}

HWTEST_F(AudioSuiteSerializerTest, StringConverter_003, TestSize.Level0)
{
    std::string input = "0";
    int32_t value = 0;
    bool result = StringConverter(input, value);
    EXPECT_TRUE(result);
    EXPECT_EQ(value, 0);
}

HWTEST_F(AudioSuiteSerializerTest, StringConverter_004, TestSize.Level0)
{
    std::string input = "";
    int32_t value = 0;
    bool result = StringConverter(input, value);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, StringConverter_005, TestSize.Level0)
{
    std::string input = "abc";
    int32_t value = 0;
    bool result = StringConverter(input, value);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, StringConverter_006, TestSize.Level0)
{
    std::string input = "123abc";
    int32_t value = 0;
    bool result = StringConverter(input, value);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, StringConverterFloat_001, TestSize.Level0)
{
    std::string input = "1.5";
    float value = 0.0f;
    bool result = StringConverterFloat(input, value);
    EXPECT_TRUE(result);
    EXPECT_FLOAT_EQ(value, 1.5f);
}

HWTEST_F(AudioSuiteSerializerTest, StringConverterFloat_002, TestSize.Level0)
{
    std::string input = "-2.5";
    float value = 0.0f;
    bool result = StringConverterFloat(input, value);
    EXPECT_TRUE(result);
    EXPECT_FLOAT_EQ(value, -2.5f);
}

HWTEST_F(AudioSuiteSerializerTest, StringConverterFloat_003, TestSize.Level0)
{
    std::string input = "0.0";
    float value = 0.0f;
    bool result = StringConverterFloat(input, value);
    EXPECT_TRUE(result);
    EXPECT_FLOAT_EQ(value, 0.0f);
}

HWTEST_F(AudioSuiteSerializerTest, StringConverterFloat_004, TestSize.Level0)
{
    std::string input = "";
    float value = 0.0f;
    bool result = StringConverterFloat(input, value);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, StringConverterFloat_005, TestSize.Level0)
{
    std::string input = "abc";
    float value = 0.0f;
    bool result = StringConverterFloat(input, value);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, StringConverterFloat_006, TestSize.Level0)
{
    std::string input = "1.5abc";
    float value = 0.0f;
    bool result = StringConverterFloat(input, value);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, AudioSpaceRenderPositionParams_Serialize_001, TestSize.Level0)
{
    AudioSpaceRenderPositionParams params;
    params.x = 1.0f;
    params.y = 2.0f;
    params.z = 3.0f;
    
    std::string result = Serializer<AudioSpaceRenderPositionParams>::Serialize(params);
    EXPECT_FALSE(result.empty());
    
    std::vector<std::string> tokens = SplitString(result, ',');
    EXPECT_EQ(tokens.size(), 3U);
}

HWTEST_F(AudioSuiteSerializerTest, AudioSpaceRenderPositionParams_Deserialize_001, TestSize.Level0)
{
    std::string input = "1.0,2.0,3.0";
    AudioSpaceRenderPositionParams params;
    bool result = Serializer<AudioSpaceRenderPositionParams>::Deserialize(input, params);
    EXPECT_TRUE(result);
    EXPECT_FLOAT_EQ(params.x, 1.0f);
    EXPECT_FLOAT_EQ(params.y, 2.0f);
    EXPECT_FLOAT_EQ(params.z, 3.0f);
}

HWTEST_F(AudioSuiteSerializerTest, AudioSpaceRenderPositionParams_Deserialize_002, TestSize.Level0)
{
    std::string input = "-1.5,-2.5,-3.5";
    AudioSpaceRenderPositionParams params;
    bool result = Serializer<AudioSpaceRenderPositionParams>::Deserialize(input, params);
    EXPECT_TRUE(result);
    EXPECT_FLOAT_EQ(params.x, -1.5f);
    EXPECT_FLOAT_EQ(params.y, -2.5f);
    EXPECT_FLOAT_EQ(params.z, -3.5f);
}

HWTEST_F(AudioSuiteSerializerTest, AudioSpaceRenderPositionParams_Deserialize_003, TestSize.Level0)
{
    std::string input = "0.0,0.0,0.0";
    AudioSpaceRenderPositionParams params;
    bool result = Serializer<AudioSpaceRenderPositionParams>::Deserialize(input, params);
    EXPECT_TRUE(result);
    EXPECT_FLOAT_EQ(params.x, 0.0f);
    EXPECT_FLOAT_EQ(params.y, 0.0f);
    EXPECT_FLOAT_EQ(params.z, 0.0f);
}

HWTEST_F(AudioSuiteSerializerTest, AudioSpaceRenderPositionParams_Deserialize_004, TestSize.Level0)
{
    std::string input = "1.0,2.0";
    AudioSpaceRenderPositionParams params;
    bool result = Serializer<AudioSpaceRenderPositionParams>::Deserialize(input, params);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, AudioSpaceRenderPositionParams_Deserialize_005, TestSize.Level0)
{
    std::string input = "";
    AudioSpaceRenderPositionParams params;
    bool result = Serializer<AudioSpaceRenderPositionParams>::Deserialize(input, params);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, AudioSpaceRenderPositionParams_Deserialize_006, TestSize.Level0)
{
    std::string input = "abc,def,ghi";
    AudioSpaceRenderPositionParams params;
    bool result = Serializer<AudioSpaceRenderPositionParams>::Deserialize(input, params);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, AudioSpaceRenderRotationParams_Serialize_001, TestSize.Level0)
{
    AudioSpaceRenderRotationParams params;
    params.x = 1.0f;
    params.y = 2.0f;
    params.z = 3.0f;
    params.surroundTime = 100;
    params.surroundDirection = AudioSurroundDirection::SPACE_RENDER_CCW;
    
    std::string result = Serializer<AudioSpaceRenderRotationParams>::Serialize(params);
    EXPECT_FALSE(result.empty());
    
    std::vector<std::string> tokens = SplitString(result, ',');
    EXPECT_EQ(tokens.size(), 5U);
}

HWTEST_F(AudioSuiteSerializerTest, AudioSpaceRenderRotationParams_Deserialize_001, TestSize.Level0)
{
    std::string input = "1.0,2.0,3.0,100,1";
    AudioSpaceRenderRotationParams params;
    bool result = Serializer<AudioSpaceRenderRotationParams>::Deserialize(input, params);
    EXPECT_TRUE(result);
    EXPECT_FLOAT_EQ(params.x, 1.0f);
    EXPECT_FLOAT_EQ(params.y, 2.0f);
    EXPECT_FLOAT_EQ(params.z, 3.0f);
    EXPECT_EQ(params.surroundTime, 100);
    EXPECT_EQ(params.surroundDirection, AudioSurroundDirection::SPACE_RENDER_CW);
}

HWTEST_F(AudioSuiteSerializerTest, AudioSpaceRenderRotationParams_Deserialize_002, TestSize.Level0)
{
    std::string input = "1.0,2.0,3.0";
    AudioSpaceRenderRotationParams params;
    bool result = Serializer<AudioSpaceRenderRotationParams>::Deserialize(input, params);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, AudioSpaceRenderRotationParams_Deserialize_003, TestSize.Level0)
{
    std::string input = "1.0,2.0,3.0,100";
    AudioSpaceRenderRotationParams params;
    bool result = Serializer<AudioSpaceRenderRotationParams>::Deserialize(input, params);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, AudioSpaceRenderRotationParams_Deserialize_004, TestSize.Level0)
{
    std::string input = "";
    AudioSpaceRenderRotationParams params;
    bool result = Serializer<AudioSpaceRenderRotationParams>::Deserialize(input, params);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, AudioSpaceRenderExtensionParams_Serialize_001, TestSize.Level0)
{
    AudioSpaceRenderExtensionParams params;
    params.extRadius = 1.5f;
    params.extAngle = 45;
    
    std::string result = Serializer<AudioSpaceRenderExtensionParams>::Serialize(params);
    EXPECT_FALSE(result.empty());
    
    std::vector<std::string> tokens = SplitString(result, ',');
    EXPECT_EQ(tokens.size(), 2U);
}

HWTEST_F(AudioSuiteSerializerTest, AudioSpaceRenderExtensionParams_Deserialize_001, TestSize.Level0)
{
    std::string input = "1.5,45";
    AudioSpaceRenderExtensionParams params;
    bool result = Serializer<AudioSpaceRenderExtensionParams>::Deserialize(input, params);
    EXPECT_TRUE(result);
    EXPECT_FLOAT_EQ(params.extRadius, 1.5f);
    EXPECT_EQ(params.extAngle, 45);
}

HWTEST_F(AudioSuiteSerializerTest, AudioSpaceRenderExtensionParams_Deserialize_002, TestSize.Level0)
{
    std::string input = "-2.5,-30";
    AudioSpaceRenderExtensionParams params;
    bool result = Serializer<AudioSpaceRenderExtensionParams>::Deserialize(input, params);
    EXPECT_TRUE(result);
    EXPECT_FLOAT_EQ(params.extRadius, -2.5f);
    EXPECT_EQ(params.extAngle, -30);
}

HWTEST_F(AudioSuiteSerializerTest, AudioSpaceRenderExtensionParams_Deserialize_003, TestSize.Level0)
{
    std::string input = "1.5";
    AudioSpaceRenderExtensionParams params;
    bool result = Serializer<AudioSpaceRenderExtensionParams>::Deserialize(input, params);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, AudioSpaceRenderExtensionParams_Deserialize_004, TestSize.Level0)
{
    std::string input = "";
    AudioSpaceRenderExtensionParams params;
    bool result = Serializer<AudioSpaceRenderExtensionParams>::Deserialize(input, params);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, TempoPitchParams_Serialize_001, TestSize.Level0)
{
    TempoPitchParams params;
    params.speed = 1.5f;
    params.pitch = 2.0f;
    
    std::string result = Serializer<TempoPitchParams>::Serialize(params);
    EXPECT_FALSE(result.empty());
    
    std::vector<std::string> tokens = SplitString(result, ',');
    EXPECT_EQ(tokens.size(), 2U);
}

HWTEST_F(AudioSuiteSerializerTest, TempoPitchParams_Deserialize_001, TestSize.Level0)
{
    std::string input = "1.5,2.0";
    TempoPitchParams params;
    bool result = Serializer<TempoPitchParams>::Deserialize(input, params);
    EXPECT_TRUE(result);
    EXPECT_FLOAT_EQ(params.speed, 1.5f);
    EXPECT_FLOAT_EQ(params.pitch, 2.0f);
}

HWTEST_F(AudioSuiteSerializerTest, TempoPitchParams_Deserialize_002, TestSize.Level0)
{
    std::string input = "-0.5,-1.0";
    TempoPitchParams params;
    bool result = Serializer<TempoPitchParams>::Deserialize(input, params);
    EXPECT_TRUE(result);
    EXPECT_FLOAT_EQ(params.speed, -0.5f);
    EXPECT_FLOAT_EQ(params.pitch, -1.0f);
}

HWTEST_F(AudioSuiteSerializerTest, TempoPitchParams_Deserialize_003, TestSize.Level0)
{
    std::string input = "0.0,0.0";
    TempoPitchParams params;
    bool result = Serializer<TempoPitchParams>::Deserialize(input, params);
    EXPECT_TRUE(result);
    EXPECT_FLOAT_EQ(params.speed, 0.0f);
    EXPECT_FLOAT_EQ(params.pitch, 0.0f);
}

HWTEST_F(AudioSuiteSerializerTest, TempoPitchParams_Deserialize_004, TestSize.Level0)
{
    std::string input = "1.5";
    TempoPitchParams params;
    bool result = Serializer<TempoPitchParams>::Deserialize(input, params);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, TempoPitchParams_Deserialize_005, TestSize.Level0)
{
    std::string input = "";
    TempoPitchParams params;
    bool result = Serializer<TempoPitchParams>::Deserialize(input, params);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, AudioPureVoiceChangeOption_Serialize_001, TestSize.Level0)
{
    AudioPureVoiceChangeOption option;
    option.optionGender = AudioPureVoiceChangeGenderOption::PURE_VOICE_CHANGE_FEMALE;
    option.optionType = AudioPureVoiceChangeType::PURE_VOICE_CHANGE_TYPE_CUTE;
    option.pitch = 1.5f;
    
    std::string result = Serializer<AudioPureVoiceChangeOption>::Serialize(option);
    EXPECT_FALSE(result.empty());
    
    std::vector<std::string> tokens = SplitString(result, ',');
    EXPECT_GE(tokens.size(), 2U);
}

HWTEST_F(AudioSuiteSerializerTest, AudioPureVoiceChangeOption_Deserialize_001, TestSize.Level0)
{
    std::string input = "1,2,1.5";
    AudioPureVoiceChangeOption option;
    bool result = Serializer<AudioPureVoiceChangeOption>::Deserialize(input, option);
    EXPECT_TRUE(result);
    EXPECT_EQ(option.optionGender, AudioPureVoiceChangeGenderOption::PURE_VOICE_CHANGE_FEMALE);
    EXPECT_EQ(option.optionType, AudioPureVoiceChangeType::PURE_VOICE_CHANGE_TYPE_CUTE);
    EXPECT_FLOAT_EQ(option.pitch, 1.5f);
}

HWTEST_F(AudioSuiteSerializerTest, AudioPureVoiceChangeOption_Deserialize_002, TestSize.Level0)
{
    std::string input = "1,1";
    AudioPureVoiceChangeOption option;
    bool result = Serializer<AudioPureVoiceChangeOption>::Deserialize(input, option);
    EXPECT_TRUE(result);
    EXPECT_EQ(option.optionGender, AudioPureVoiceChangeGenderOption::PURE_VOICE_CHANGE_FEMALE);
    EXPECT_EQ(option.optionType, AudioPureVoiceChangeType::PURE_VOICE_CHANGE_TYPE_CARTOON);
}

HWTEST_F(AudioSuiteSerializerTest, AudioPureVoiceChangeOption_Deserialize_003, TestSize.Level0)
{
    std::string input = "1";
    AudioPureVoiceChangeOption option;
    bool result = Serializer<AudioPureVoiceChangeOption>::Deserialize(input, option);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, AudioPureVoiceChangeOption_Deserialize_004, TestSize.Level0)
{
    std::string input = "";
    AudioPureVoiceChangeOption option;
    bool result = Serializer<AudioPureVoiceChangeOption>::Deserialize(input, option);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, AudioGeneralVoiceChangeType_Serialize_001, TestSize.Level0)
{
    AudioGeneralVoiceChangeType type = AudioGeneralVoiceChangeType::GENERAL_VOICE_CHANGE_TYPE_CUTE;
    std::string result = Serializer<AudioGeneralVoiceChangeType>::Serialize(type);
    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result, "1");
}

HWTEST_F(AudioSuiteSerializerTest, AudioGeneralVoiceChangeType_Serialize_002, TestSize.Level0)
{
    AudioGeneralVoiceChangeType type = AudioGeneralVoiceChangeType::GENERAL_VOICE_CHANGE_TYPE_CYBERPUNK;
    std::string result = Serializer<AudioGeneralVoiceChangeType>::Serialize(type);
    EXPECT_FALSE(result.empty());
    EXPECT_EQ(result, "2");
}

HWTEST_F(AudioSuiteSerializerTest, AudioGeneralVoiceChangeType_Deserialize_001, TestSize.Level0)
{
    std::string input = "1";
    AudioGeneralVoiceChangeType type;
    bool result = Serializer<AudioGeneralVoiceChangeType>::Deserialize(input, type);
    EXPECT_TRUE(result);
    EXPECT_EQ(type, AudioGeneralVoiceChangeType::GENERAL_VOICE_CHANGE_TYPE_CUTE);
}

HWTEST_F(AudioSuiteSerializerTest, AudioGeneralVoiceChangeType_Deserialize_002, TestSize.Level0)
{
    std::string input = "2";
    AudioGeneralVoiceChangeType type;
    bool result = Serializer<AudioGeneralVoiceChangeType>::Deserialize(input, type);
    EXPECT_TRUE(result);
    EXPECT_EQ(type, AudioGeneralVoiceChangeType::GENERAL_VOICE_CHANGE_TYPE_CYBERPUNK);
}

HWTEST_F(AudioSuiteSerializerTest, AudioGeneralVoiceChangeType_Deserialize_003, TestSize.Level0)
{
    std::string input = "";
    AudioGeneralVoiceChangeType type;
    bool result = Serializer<AudioGeneralVoiceChangeType>::Deserialize(input, type);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, AudioGeneralVoiceChangeType_Deserialize_004, TestSize.Level0)
{
    std::string input = "abc";
    AudioGeneralVoiceChangeType type;
    bool result = Serializer<AudioGeneralVoiceChangeType>::Deserialize(input, type);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, AudioGeneralVoiceChangeType_Deserialize_005, TestSize.Level0)
{
    std::string input = "123abc";
    AudioGeneralVoiceChangeType type;
    bool result = Serializer<AudioGeneralVoiceChangeType>::Deserialize(input, type);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, RoundTrip_AudioSpaceRenderPositionParams, TestSize.Level0)
{
    AudioSpaceRenderPositionParams original;
    original.x = 1.5f;
    original.y = -2.5f;
    original.z = 3.7f;
    
    std::string serialized = Serializer<AudioSpaceRenderPositionParams>::Serialize(original);
    AudioSpaceRenderPositionParams deserialized;
    bool result = Serializer<AudioSpaceRenderPositionParams>::Deserialize(serialized, deserialized);
    
    EXPECT_TRUE(result);
    EXPECT_FLOAT_EQ(deserialized.x, original.x);
    EXPECT_FLOAT_EQ(deserialized.y, original.y);
    EXPECT_FLOAT_EQ(deserialized.z, original.z);
}

HWTEST_F(AudioSuiteSerializerTest, RoundTrip_AudioSpaceRenderRotationParams, TestSize.Level0)
{
    AudioSpaceRenderRotationParams original;
    original.x = 1.5f;
    original.y = -2.5f;
    original.z = 3.7f;
    original.surroundTime = 200;
    original.surroundDirection = AudioSurroundDirection::SPACE_RENDER_CW;
    
    std::string serialized = Serializer<AudioSpaceRenderRotationParams>::Serialize(original);
    AudioSpaceRenderRotationParams deserialized;
    bool result = Serializer<AudioSpaceRenderRotationParams>::Deserialize(serialized, deserialized);
    
    EXPECT_TRUE(result);
    EXPECT_FLOAT_EQ(deserialized.x, original.x);
    EXPECT_FLOAT_EQ(deserialized.y, original.y);
    EXPECT_FLOAT_EQ(deserialized.z, original.z);
    EXPECT_EQ(deserialized.surroundTime, original.surroundTime);
    EXPECT_EQ(deserialized.surroundDirection, original.surroundDirection);
}

HWTEST_F(AudioSuiteSerializerTest, RoundTrip_AudioSpaceRenderExtensionParams, TestSize.Level0)
{
    AudioSpaceRenderExtensionParams original;
    original.extRadius = 5.5f;
    original.extAngle = 90;
    
    std::string serialized = Serializer<AudioSpaceRenderExtensionParams>::Serialize(original);
    AudioSpaceRenderExtensionParams deserialized;
    bool result = Serializer<AudioSpaceRenderExtensionParams>::Deserialize(serialized, deserialized);
    
    EXPECT_TRUE(result);
    EXPECT_FLOAT_EQ(deserialized.extRadius, original.extRadius);
    EXPECT_EQ(deserialized.extAngle, original.extAngle);
}

HWTEST_F(AudioSuiteSerializerTest, RoundTrip_TempoPitchParams, TestSize.Level0)
{
    TempoPitchParams original;
    original.speed = 2.5f;
    original.pitch = -1.5f;
    
    std::string serialized = Serializer<TempoPitchParams>::Serialize(original);
    TempoPitchParams deserialized;
    bool result = Serializer<TempoPitchParams>::Deserialize(serialized, deserialized);
    
    EXPECT_TRUE(result);
    EXPECT_FLOAT_EQ(deserialized.speed, original.speed);
    EXPECT_FLOAT_EQ(deserialized.pitch, original.pitch);
}

HWTEST_F(AudioSuiteSerializerTest, RoundTrip_AudioPureVoiceChangeOption, TestSize.Level0)
{
    AudioPureVoiceChangeOption original;
    original.optionGender = AudioPureVoiceChangeGenderOption::PURE_VOICE_CHANGE_MALE;
    original.optionType = AudioPureVoiceChangeType::PURE_VOICE_CHANGE_TYPE_SEASONED;
    original.pitch = 2.5f;
    
    std::string serialized = Serializer<AudioPureVoiceChangeOption>::Serialize(original);
    AudioPureVoiceChangeOption deserialized;
    bool result = Serializer<AudioPureVoiceChangeOption>::Deserialize(serialized, deserialized);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(deserialized.optionGender, original.optionGender);
    EXPECT_EQ(deserialized.optionType, original.optionType);
    EXPECT_FLOAT_EQ(deserialized.pitch, original.pitch);
}

HWTEST_F(AudioSuiteSerializerTest, RoundTrip_AudioGeneralVoiceChangeType, TestSize.Level0)
{
    AudioGeneralVoiceChangeType original = AudioGeneralVoiceChangeType::GENERAL_VOICE_CHANGE_TYPE_CUTE;
    
    std::string serialized = Serializer<AudioGeneralVoiceChangeType>::Serialize(original);
    AudioGeneralVoiceChangeType deserialized;
    bool result = Serializer<AudioGeneralVoiceChangeType>::Deserialize(serialized, deserialized);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(deserialized, original);
}

HWTEST_F(AudioSuiteSerializerTest, StringConverter_007, TestSize.Level0)
{
    std::string input = "2147483647";
    int32_t value = 0;
    bool result = StringConverter(input, value);
    EXPECT_TRUE(result);
    EXPECT_EQ(value, INT32_MAX);
}

HWTEST_F(AudioSuiteSerializerTest, StringConverter_008, TestSize.Level0)
{
    std::string input = "-2147483648";
    int32_t value = 0;
    bool result = StringConverter(input, value);
    EXPECT_TRUE(result);
    EXPECT_EQ(value, INT32_MIN);
}

HWTEST_F(AudioSuiteSerializerTest, StringConverter_009, TestSize.Level0)
{
    std::string input = "2147483648";
    int32_t value = 0;
    bool result = StringConverter(input, value);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, StringConverter_010, TestSize.Level0)
{
    std::string input = "-2147483649";
    int32_t value = 0;
    bool result = StringConverter(input, value);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, StringConverter_011, TestSize.Level0)
{
    std::string input = "  123";
    int32_t value = 0;
    bool result = StringConverter(input, value);
    EXPECT_TRUE(result);
    EXPECT_EQ(value, 123);
}

HWTEST_F(AudioSuiteSerializerTest, StringConverter_012, TestSize.Level0)
{
    std::string input = "123  ";
    int32_t value = 0;
    bool result = StringConverter(input, value);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, AudioSpaceRenderPositionParams_Deserialize_007, TestSize.Level0)
{
    std::string input = "1.0,abc,3.0";
    AudioSpaceRenderPositionParams params;
    bool result = Serializer<AudioSpaceRenderPositionParams>::Deserialize(input, params);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, AudioSpaceRenderPositionParams_Deserialize_008, TestSize.Level0)
{
    std::string input = "1.0,2.0,3.0,4.0";
    AudioSpaceRenderPositionParams params;
    bool result = Serializer<AudioSpaceRenderPositionParams>::Deserialize(input, params);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, AudioSpaceRenderRotationParams_Deserialize_005, TestSize.Level0)
{
    std::string input = "abc,2.0,3.0,100,1";
    AudioSpaceRenderRotationParams params;
    bool result = Serializer<AudioSpaceRenderRotationParams>::Deserialize(input, params);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, AudioSpaceRenderRotationParams_Deserialize_006, TestSize.Level0)
{
    std::string input = "1.0,2.0,abc,100,1";
    AudioSpaceRenderRotationParams params;
    bool result = Serializer<AudioSpaceRenderRotationParams>::Deserialize(input, params);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, AudioSpaceRenderRotationParams_Deserialize_007, TestSize.Level0)
{
    std::string input = "1.0,2.0,3.0,abc,1";
    AudioSpaceRenderRotationParams params;
    bool result = Serializer<AudioSpaceRenderRotationParams>::Deserialize(input, params);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, AudioSpaceRenderRotationParams_Deserialize_008, TestSize.Level0)
{
    std::string input = "1.0,2.0,3.0,100,abc";
    AudioSpaceRenderRotationParams params;
    bool result = Serializer<AudioSpaceRenderRotationParams>::Deserialize(input, params);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, AudioSpaceRenderExtensionParams_Deserialize_005, TestSize.Level0)
{
    std::string input = "abc,45";
    AudioSpaceRenderExtensionParams params;
    bool result = Serializer<AudioSpaceRenderExtensionParams>::Deserialize(input, params);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, AudioSpaceRenderExtensionParams_Deserialize_006, TestSize.Level0)
{
    std::string input = "1.5,abc";
    AudioSpaceRenderExtensionParams params;
    bool result = Serializer<AudioSpaceRenderExtensionParams>::Deserialize(input, params);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, AudioSpaceRenderExtensionParams_Deserialize_007, TestSize.Level0)
{
    std::string input = "1.5,45,60";
    AudioSpaceRenderExtensionParams params;
    bool result = Serializer<AudioSpaceRenderExtensionParams>::Deserialize(input, params);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, TempoPitchParams_Deserialize_006, TestSize.Level0)
{
    std::string input = "abc,2.0";
    TempoPitchParams params;
    bool result = Serializer<TempoPitchParams>::Deserialize(input, params);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, TempoPitchParams_Deserialize_007, TestSize.Level0)
{
    std::string input = "1.5,abc";
    TempoPitchParams params;
    bool result = Serializer<TempoPitchParams>::Deserialize(input, params);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, AudioPureVoiceChangeOption_Deserialize_005, TestSize.Level0)
{
    std::string input = "abc,2";
    AudioPureVoiceChangeOption option;
    bool result = Serializer<AudioPureVoiceChangeOption>::Deserialize(input, option);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, AudioPureVoiceChangeOption_Deserialize_006, TestSize.Level0)
{
    std::string input = "1,abc";
    AudioPureVoiceChangeOption option;
    bool result = Serializer<AudioPureVoiceChangeOption>::Deserialize(input, option);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, AudioPureVoiceChangeOption_Deserialize_007, TestSize.Level0)
{
    std::string input = "1,2,abc";
    AudioPureVoiceChangeOption option;
    bool result = Serializer<AudioPureVoiceChangeOption>::Deserialize(input, option);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, AudioPureVoiceChangeOption_Deserialize_008, TestSize.Level0)
{
    std::string input = "1,2,1.5,extra";
    AudioPureVoiceChangeOption option;
    bool result = Serializer<AudioPureVoiceChangeOption>::Deserialize(input, option);
    EXPECT_TRUE(result);
    EXPECT_EQ(option.optionGender, AudioPureVoiceChangeGenderOption::PURE_VOICE_CHANGE_FEMALE);
    EXPECT_EQ(option.optionType, AudioPureVoiceChangeType::PURE_VOICE_CHANGE_TYPE_CUTE);
    EXPECT_FLOAT_EQ(option.pitch, 1.5f);
}

HWTEST_F(AudioSuiteSerializerTest, AudioSpaceRenderPositionParams_Deserialize_009, TestSize.Level0)
{
    std::string input = "1.0,2.0,3.0,4.0,5.0";
    AudioSpaceRenderPositionParams params;
    bool result = Serializer<AudioSpaceRenderPositionParams>::Deserialize(input, params);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, AudioSpaceRenderRotationParams_Deserialize_009, TestSize.Level0)
{
    std::string input = "1.0,2.0,3.0,100,1,extra";
    AudioSpaceRenderRotationParams params;
    bool result = Serializer<AudioSpaceRenderRotationParams>::Deserialize(input, params);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, AudioSpaceRenderExtensionParams_Deserialize_008, TestSize.Level0)
{
    std::string input = "1.5,45,60,extra";
    AudioSpaceRenderExtensionParams params;
    bool result = Serializer<AudioSpaceRenderExtensionParams>::Deserialize(input, params);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, TempoPitchParams_Deserialize_008, TestSize.Level0)
{
    std::string input = "1.5,2.0,extra";
    TempoPitchParams params;
    bool result = Serializer<TempoPitchParams>::Deserialize(input, params);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, AudioPureVoiceChangeOption_Deserialize_009, TestSize.Level0)
{
    std::string input = "2,1";
    AudioPureVoiceChangeOption option;
    bool result = Serializer<AudioPureVoiceChangeOption>::Deserialize(input, option);
    EXPECT_TRUE(result);
    EXPECT_EQ(option.optionGender, AudioPureVoiceChangeGenderOption::PURE_VOICE_CHANGE_MALE);
    EXPECT_EQ(option.optionType, AudioPureVoiceChangeType::PURE_VOICE_CHANGE_TYPE_CARTOON);
    EXPECT_FLOAT_EQ(option.pitch, 0.0f);
}

HWTEST_F(AudioSuiteSerializerTest, StringConverter_Overflow, TestSize.Level0)
{
    std::string input = "999999999999999999999999999999";
    int32_t value = 0;
    bool result = StringConverter(input, value);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioSuiteSerializerTest, StringConverterFloat_Overflow, TestSize.Level0)
{
    std::string input = "1e999";
    float value = 0.0f;
    bool result = StringConverterFloat(input, value);
    EXPECT_FALSE(result);
}
}