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

#include "oh_audio_workgroup_unit_test.h"
#include <pthread.h>

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void OHAudioWorkgroupUnitTest::SetUpTestCase(void) { }

void OHAudioWorkgroupUnitTest::TearDownTestCase(void) { }

void OHAudioWorkgroupUnitTest::SetUp(void) { }

void OHAudioWorkgroupUnitTest::TearDown(void) { }

/**
 * @tc.name  : Test OHAudioWorkgroup.
 * @tc.number: TestOHAudioWorkgroup_001
 * @tc.desc  : Test OHAudioWorkgroup.
 */

OH_AudioResourceManager *audioResourceManager = nullptr;
OH_AudioCommon_Result result;
OH_AudioWorkgroup *audioWorkgroup = nullptr;

void* TestFunc(void* arg)
{
    pthread_t tmpTid = pthread_self();
    pid_t testTid = static_cast<unsigned long>(tmpTid);
    const int32_t startTime = 20;
    const int32_t endTime = 40;

    result = OH_AudioWorkgroup_AddCurrentThread(audioWorkgroup, &testTid);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    result = OH_AudioWorkgroup_Start(audioWorkgroup, startTime, endTime);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    result = OH_AudioWorkgroup_Stop(audioWorkgroup);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    result = OH_AudioWorkgroup_RemoveThread(audioWorkgroup, testTid);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    result = OH_AudioResourceManager_ReleaseWorkgroup(audioResourceManager, audioWorkgroup);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    return nullptr;
}

HWTEST(OHAudioWorkgroupUnitTest, TestOHAudioWorkgroup_001, TestSize.Level0)
{
    result = OH_AudioManager_GetAudioResourceManager(&audioResourceManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioResourceManager, nullptr);
    result = OH_AudioResourceManager_CreateWorkgroup(audioResourceManager, "testAudioGroup", &audioWorkgroup);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioWorkgroup, nullptr);

    pthread_t tid;
    pthread_create(&tid, NULL, TestFunc, NULL);
    pthread_join(tid, NULL);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

HWTEST(OHAudioWorkgroupUnitTest, TestOHAudioWorkgroup_002, TestSize.Level0)
{
    OH_AudioResourceManager *audioResourceManager = nullptr;
    OH_AudioCommon_Result result;
    OH_AudioWorkgroup *group = nullptr;
    int32_t *tokenId = nullptr;
    char *name = nullptr;

    result = OH_AudioResourceManager_CreateWorkgroup(audioResourceManager, name, &group);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);

    result = OH_AudioResourceManager_ReleaseWorkgroup(audioResourceManager, group);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);

    result = OH_AudioWorkgroup_AddCurrentThread(group, tokenId);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

HWTEST(OHAudioWorkgroupUnitTest, TestOHAudioWorkgroup_003, TestSize.Level0)
{
    OH_AudioResourceManager **audioResourceManager = nullptr;
    OH_AudioCommon_Result result;

    result = OH_AudioManager_GetAudioResourceManager(audioResourceManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}
} // namespace AudioStandard
} // namespace OHOS