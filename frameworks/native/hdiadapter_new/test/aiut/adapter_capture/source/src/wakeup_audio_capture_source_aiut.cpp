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

#include "wakeup_audio_capture_source_aiut.h"
#include "aiut_log.h"
#include "source/wakeup_audio_capture_source.h"
#include "source/i_audio_capture_source.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

// ==================== WakeupAudioCaptureSourceAiut ====================

void WakeupAudioCaptureSourceAiut::SetUpTestCase(void) {}
void WakeupAudioCaptureSourceAiut::TearDownTestCase(void) {}
void WakeupAudioCaptureSourceAiut::SetUp(void) {}
void WakeupAudioCaptureSourceAiut::TearDown(void) {}

// ==================== Construction / Destruction Tests ====================

/**
 * @tc.name  : Test WakeupAudioCaptureSource construction
 * @tc.number: WakeupCaptureSource_Constructor_001
 * @tc.desc  : Test WakeupAudioCaptureSource constructor creates uninitialized object.
 */
HWTEST_F(WakeupAudioCaptureSourceAiut, WakeupCaptureSource_Constructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<WakeupAudioCaptureSource>(1);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    AIUT_EXPECT_FALSE_INFO_LOG(source->IsInited(), "newly constructed source should not be inited");
}

/**
 * @tc.name  : Test WakeupAudioCaptureSource destruction
 * @tc.number: WakeupCaptureSource_Destructor_001
 * @tc.desc  : Test destructor on uninitialized source does not crash.
 */
HWTEST_F(WakeupAudioCaptureSourceAiut, WakeupCaptureSource_Destructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<WakeupAudioCaptureSource>(1);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    source = nullptr;
    AIUT_EXPECT_EQ_INFO_LOG(source, nullptr, "source should be null after reset");
}

// ==================== Lifecycle Tests ====================

/**
 * @tc.name  : Test WakeupAudioCaptureSource Start without init
 * @tc.number: WakeupCaptureSource_StartWithoutInit_001
 * @tc.desc  : Test Start() before Init() returns error.
 */
HWTEST_F(WakeupAudioCaptureSourceAiut, WakeupCaptureSource_StartWithoutInit_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<WakeupAudioCaptureSource>(1);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->Start();
    AIUT_EXPECT_NE_INFO_LOG(ret, SUCCESS,
        "Start without init should not return SUCCESS, got %d", ret);
}

/**
 * @tc.name  : Test WakeupAudioCaptureSource Stop without init
 * @tc.number: WakeupCaptureSource_StopWithoutInit_001
 * @tc.desc  : Test Stop() without Init() returns SUCCESS.
 */
HWTEST_F(WakeupAudioCaptureSourceAiut, WakeupCaptureSource_StopWithoutInit_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<WakeupAudioCaptureSource>(1);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->Stop();
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "expected SUCCESS for Stop without init, got %d", ret);
}

/**
 * @tc.name  : Test WakeupAudioCaptureSource SetAudioScene without init
 * @tc.number: WakeupCaptureSource_SetAudioScene_001
 * @tc.desc  : Test SetAudioScene without init returns SUCCESS.
 */
HWTEST_F(WakeupAudioCaptureSourceAiut, WakeupCaptureSource_SetAudioScene_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<WakeupAudioCaptureSource>(1);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->SetAudioScene(AUDIO_SCENE_DEFAULT);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "expected SUCCESS for SetAudioScene, got %d", ret);
}

/**
 * @tc.name  : Test WakeupAudioCaptureSource UpdateSourceType without init
 * @tc.number: WakeupCaptureSource_UpdateSourceType_001
 * @tc.desc  : Test UpdateSourceType returns SUCCESS without init.
 */
HWTEST_F(WakeupAudioCaptureSourceAiut, WakeupCaptureSource_UpdateSourceType_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<WakeupAudioCaptureSource>(1);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->UpdateSourceType(SOURCE_TYPE_MIC);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "expected SUCCESS for UpdateSourceType, got %d", ret);
}

/**
 * @tc.name  : Test WakeupAudioCaptureSource UpdateAppsUid without init
 * @tc.number: WakeupCaptureSource_UpdateAppsUid_001
 * @tc.desc  : Test UpdateAppsUid without init returns SUCCESS.
 */
HWTEST_F(WakeupAudioCaptureSourceAiut, WakeupCaptureSource_UpdateAppsUid_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<WakeupAudioCaptureSource>(1);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    std::vector<int32_t> appsUid;
    int32_t ret = source->UpdateAppsUid(appsUid);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "expected SUCCESS for UpdateAppsUid, got %d", ret);
}

} // namespace AudioStandard
} // namespace OHOS
