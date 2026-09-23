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

#include "aiut_log.h"
#include "audio_errors.h"
#include "native_audio_manager.h"
#include "native_audio_stream_manager.h"
#include "gtest/gtest.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class OHAudioWorkgroupAiut : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp(void) {}
    void TearDown(void) {}
};

/**
 * @tc.name  : Test OH_AudioWorkgroup_RemoveThread with null group
 * @tc.number: OH_AudioWorkgroup_RemoveThread_NullGroup
 * @tc.desc  : Test OH_AudioWorkgroup_RemoveThread with null group returns INVALID_PARAM
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OHAudioWorkgroupAiut, OH_AudioWorkgroup_RemoveThread_NullGroup, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    int32_t tokenId = 1000;
    auto result = OH_AudioWorkgroup_RemoveThread(nullptr, tokenId);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "expected INVALID_PARAM, got %d", result);
}

/**
 * @tc.name  : Test OH_AudioWorkgroup_Start with null group
 * @tc.number: OH_AudioWorkgroup_Start_NullGroup
 * @tc.desc  : Test OH_AudioWorkgroup_Start with null group returns INVALID_PARAM
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OHAudioWorkgroupAiut, OH_AudioWorkgroup_Start_NullGroup, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    uint64_t startTime = 0;
    uint64_t deadlineTime = 100;
    auto result = OH_AudioWorkgroup_Start(nullptr, startTime, deadlineTime);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "expected INVALID_PARAM, got %d", result);
}

/**
 * @tc.name  : Test OH_AudioWorkgroup_Stop with null group
 * @tc.number: OH_AudioWorkgroup_Stop_NullGroup
 * @tc.desc  : Test OH_AudioWorkgroup_Stop with null group returns INVALID_PARAM
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OHAudioWorkgroupAiut, OH_AudioWorkgroup_Stop_NullGroup, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto result = OH_AudioWorkgroup_Stop(nullptr);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "expected INVALID_PARAM, got %d", result);
}

/**
 * @tc.name  : Test OH_AudioResourceManager_CreateWorkgroup with null name
 * @tc.number: OH_AudioResourceManager_CreateWorkgroup_NullName
 * @tc.desc  : Test CreateWorkgroup with null name returns INVALID_PARAM
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OHAudioWorkgroupAiut, OH_AudioResourceManager_CreateWorkgroup_NullName, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioResourceManager *audioResourceManager = nullptr;
    auto result = OH_AudioManager_GetAudioResourceManager(&audioResourceManager);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_SUCCESS, "GetAudioResourceManager failed, ret=%d", result);
    AIUT_ASSERT_NE_INFO_LOG(audioResourceManager, nullptr, "audioResourceManager is null");
    OH_AudioWorkgroup *group = nullptr;
    result = OH_AudioResourceManager_CreateWorkgroup(audioResourceManager, nullptr, &group);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "expected INVALID_PARAM, got %d", result);
}

/**
 * @tc.name  : Test OH_AudioResourceManager_CreateWorkgroup with null group output
 * @tc.number: OH_AudioResourceManager_CreateWorkgroup_NullGroup
 * @tc.desc  : Test CreateWorkgroup with null group output returns INVALID_PARAM
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OHAudioWorkgroupAiut, OH_AudioResourceManager_CreateWorkgroup_NullGroup, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioResourceManager *audioResourceManager = nullptr;
    auto result = OH_AudioManager_GetAudioResourceManager(&audioResourceManager);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_SUCCESS, "GetAudioResourceManager failed, ret=%d", result);
    AIUT_ASSERT_NE_INFO_LOG(audioResourceManager, nullptr, "audioResourceManager is null");
    result = OH_AudioResourceManager_CreateWorkgroup(audioResourceManager, "test", nullptr);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "expected INVALID_PARAM, got %d", result);
}

/**
 * @tc.name  : Test OH_AudioResourceManager_ReleaseWorkgroup with null group
 * @tc.number: OH_AudioResourceManager_ReleaseWorkgroup_NullGroup
 * @tc.desc  : Test ReleaseWorkgroup with null group returns INVALID_PARAM
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OHAudioWorkgroupAiut, OH_AudioResourceManager_ReleaseWorkgroup_NullGroup, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioResourceManager *audioResourceManager = nullptr;
    auto result = OH_AudioManager_GetAudioResourceManager(&audioResourceManager);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_SUCCESS, "GetAudioResourceManager failed, ret=%d", result);
    AIUT_ASSERT_NE_INFO_LOG(audioResourceManager, nullptr, "audioResourceManager is null");
    result = OH_AudioResourceManager_ReleaseWorkgroup(audioResourceManager, nullptr);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "expected INVALID_PARAM, got %d", result);
}

/**
 * @tc.name  : Test OH_AudioWorkgroup_AddCurrentThread with null tokenId
 * @tc.number: OH_AudioWorkgroup_AddCurrentThread_NullTokenId
 * @tc.desc  : Test AddCurrentThread with null tokenId returns INVALID_PARAM
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OHAudioWorkgroupAiut, OH_AudioWorkgroup_AddCurrentThread_NullTokenId, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioResourceManager *audioResourceManager = nullptr;
    auto result = OH_AudioManager_GetAudioResourceManager(&audioResourceManager);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_SUCCESS, "GetAudioResourceManager failed, ret=%d", result);
    AIUT_ASSERT_NE_INFO_LOG(audioResourceManager, nullptr, "audioResourceManager is null");
    OH_AudioWorkgroup *group = nullptr;
    result = OH_AudioResourceManager_CreateWorkgroup(audioResourceManager, "testGroup", &group);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_SUCCESS, "CreateWorkgroup failed, ret=%d", result);
    AIUT_ASSERT_NE_INFO_LOG(group, nullptr, "group is null");
    result = OH_AudioWorkgroup_AddCurrentThread(group, nullptr);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "expected INVALID_PARAM, got %d", result);
    result = OH_AudioResourceManager_ReleaseWorkgroup(audioResourceManager, group);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_SUCCESS, "ReleaseWorkgroup failed, ret=%d", result);
}

/**
 * @tc.name  : Test OH_AudioResourceManager full workflow
 * @tc.number: OH_AudioResourceManager_FullWorkflow
 * @tc.desc  : Test full workflow: create, add thread, start, stop, remove thread, release
 * @tc.owner : audio_framework_common
 */
HWTEST_F(OHAudioWorkgroupAiut, OH_AudioResourceManager_FullWorkflow, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    OH_AudioResourceManager *audioResourceManager = nullptr;
    auto result = OH_AudioManager_GetAudioResourceManager(&audioResourceManager);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_SUCCESS, "GetAudioResourceManager failed, ret=%d", result);
    AIUT_ASSERT_NE_INFO_LOG(audioResourceManager, nullptr, "audioResourceManager is null");

    OH_AudioWorkgroup *group = nullptr;
    result = OH_AudioResourceManager_CreateWorkgroup(audioResourceManager, "workflowTest", &group);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_SUCCESS, "CreateWorkgroup failed, ret=%d", result);
    AIUT_ASSERT_NE_INFO_LOG(group, nullptr, "group is null");

    int32_t tokenId = 5000;
    result = OH_AudioWorkgroup_AddCurrentThread(group, &tokenId);
    AIUT_EXPECT_TRUE_INFO_LOG(result == AUDIOCOMMON_RESULT_SUCCESS ||
        result == AUDIOCOMMON_RESULT_ERROR_SYSTEM, "AddCurrentThread unexpected result=%d", result);

    result = OH_AudioWorkgroup_Start(group, 0, 1000);
    AIUT_EXPECT_TRUE_INFO_LOG(result == AUDIOCOMMON_RESULT_SUCCESS ||
        result == AUDIOCOMMON_RESULT_ERROR_SYSTEM, "Start unexpected result=%d", result);

    result = OH_AudioWorkgroup_Stop(group);
    AIUT_EXPECT_TRUE_INFO_LOG(result == AUDIOCOMMON_RESULT_SUCCESS ||
        result == AUDIOCOMMON_RESULT_ERROR_SYSTEM, "Stop unexpected result=%d", result);

    result = OH_AudioWorkgroup_RemoveThread(group, tokenId);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_SUCCESS, "RemoveThread failed, ret=%d", result);

    result = OH_AudioResourceManager_ReleaseWorkgroup(audioResourceManager, group);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIOCOMMON_RESULT_SUCCESS, "ReleaseWorkgroup failed, ret=%d", result);
}

} // namespace AudioStandard
} // namespace OHOS