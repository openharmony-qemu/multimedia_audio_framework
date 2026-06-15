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

#include "va_capture_source_aiut.h"
#include "aiut_log.h"
#include "source/va_capture_source.h"
#include "source/i_audio_capture_source.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

// ==================== VACaptureSourceAiut ====================

void VACaptureSourceAiut::SetUpTestCase(void) {}
void VACaptureSourceAiut::TearDownTestCase(void) {}
void VACaptureSourceAiut::SetUp(void) {}
void VACaptureSourceAiut::TearDown(void) {}

// ==================== Construction / Destruction Tests ====================

/**
 * @tc.name  : Test VACaptureSource construction
 * @tc.number: VACaptureSource_Constructor_001
 * @tc.desc  : Test VACaptureSource constructor creates uninitialized object.
 */
HWTEST_F(VACaptureSourceAiut, VACaptureSource_Constructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<VACaptureSource>(1);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    AIUT_EXPECT_FALSE_INFO_LOG(source->IsInited(), "newly constructed source should not be inited");
}

/**
 * @tc.name  : Test VACaptureSource destruction
 * @tc.number: VACaptureSource_Destructor_001
 * @tc.desc  : Test destructor on uninitialized source does not crash.
 */
HWTEST_F(VACaptureSourceAiut, VACaptureSource_Destructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<VACaptureSource>(1);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    source = nullptr;
    AIUT_EXPECT_EQ_INFO_LOG(source, nullptr, "source should be null after reset");
}

// ==================== Lifecycle Tests ====================

/**
 * @tc.name  : Test VACaptureSource Resume without init
 * @tc.number: VACaptureSource_ResumeWithoutInit_001
 * @tc.desc  : Test Resume() without Init() returns SUCCESS.
 */
HWTEST_F(VACaptureSourceAiut, VACaptureSource_ResumeWithoutInit_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<VACaptureSource>(1);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->Resume();
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "expected SUCCESS for Resume without init, got %d", ret);
}

/**
 * @tc.name  : Test VACaptureSource Pause without init
 * @tc.number: VACaptureSource_PauseWithoutInit_001
 * @tc.desc  : Test Pause() without Init() returns SUCCESS.
 */
HWTEST_F(VACaptureSourceAiut, VACaptureSource_PauseWithoutInit_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<VACaptureSource>(1);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->Pause();
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "expected SUCCESS for Pause without init, got %d", ret);
}

/**
 * @tc.name  : Test VACaptureSource SetVolume without init
 * @tc.number: VACaptureSource_SetVolume_001
 * @tc.desc  : Test SetVolume without Init() returns SUCCESS.
 */
HWTEST_F(VACaptureSourceAiut, VACaptureSource_SetVolume_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<VACaptureSource>(1);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->SetVolume(0.0f, 0.0f);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "expected SUCCESS for SetVolume, got %d", ret);
}

/**
 * @tc.name  : Test VACaptureSource SetMute without init
 * @tc.number: VACaptureSource_SetMute_001
 * @tc.desc  : Test SetMute without Init() returns SUCCESS.
 */
HWTEST_F(VACaptureSourceAiut, VACaptureSource_SetMute_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<VACaptureSource>(1);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->SetMute(false);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "expected SUCCESS for SetMute, got %d", ret);
}

/**
 * @tc.name  : Test VACaptureSource SetInvalidState
 * @tc.number: VACaptureSource_SetInvalidState_001
 * @tc.desc  : Test SetInvalidState followed by Init/DeInit works correctly.
 */
HWTEST_F(VACaptureSourceAiut, VACaptureSource_SetInvalidState_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<VACaptureSource>(1);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    source->SetInvalidState();
    IAudioSourceAttr attr = {};
    (void)source->Init(attr);
    source->DeInit();
    AIUT_EXPECT_FALSE_INFO_LOG(source->IsInited(), "source should not be inited after DeInit");
}

} // namespace AudioStandard
} // namespace OHOS
