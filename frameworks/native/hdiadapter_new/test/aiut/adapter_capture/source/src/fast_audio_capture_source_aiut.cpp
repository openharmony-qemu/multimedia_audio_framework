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

#include "fast_audio_capture_source_aiut.h"
#include "aiut_log.h"
#include "source/fast_audio_capture_source.h"
#include "source/i_audio_capture_source.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

namespace {
constexpr uint32_t TEST_SYNC_INFO_SIZE_ZERO = 0;
constexpr uint32_t TEST_SYNC_INFO_SIZE_ONE = 1;
} // namespace

// ==================== FastAudioCaptureSourceAiut ====================

void FastAudioCaptureSourceAiut::SetUpTestCase(void) {}
void FastAudioCaptureSourceAiut::TearDownTestCase(void) {}
void FastAudioCaptureSourceAiut::SetUp(void) {}
void FastAudioCaptureSourceAiut::TearDown(void) {}

// ==================== Construction / Destruction Tests ====================

/**
 * @tc.name  : Test FastAudioCaptureSource construction
 * @tc.number: FastCaptureSource_Constructor_001
 * @tc.desc  : Test FastAudioCaptureSource default constructor creates uninitialized object.
 */
HWTEST_F(FastAudioCaptureSourceAiut, FastCaptureSource_Constructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<FastAudioCaptureSource>();
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    AIUT_EXPECT_FALSE_INFO_LOG(source->IsInited(), "newly constructed source should not be inited");
}

/**
 * @tc.name  : Test FastAudioCaptureSource destruction
 * @tc.number: FastCaptureSource_Destructor_001
 * @tc.desc  : Test destructor on uninitialized source does not crash.
 */
HWTEST_F(FastAudioCaptureSourceAiut, FastCaptureSource_Destructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<FastAudioCaptureSource>();
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    source = nullptr;
    AIUT_EXPECT_EQ_INFO_LOG(source, nullptr, "source should be null after reset");
}

// ==================== EnableSyncInfo Tests ====================

/**
 * @tc.name  : Test EnableSyncInfo with zero size
 * @tc.number: FastCaptureSource_EnableSyncInfo_001
 * @tc.desc  : Test EnableSyncInfo sets syncInfoSize to 0 when input is 0.
 */
HWTEST_F(FastAudioCaptureSourceAiut, FastCaptureSource_EnableSyncInfo_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<FastAudioCaptureSource>();
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    source->EnableSyncInfo(TEST_SYNC_INFO_SIZE_ZERO);
    AIUT_EXPECT_EQ_INFO_LOG(source->syncInfoSize_, TEST_SYNC_INFO_SIZE_ZERO,
        "syncInfoSize should be 0, got %u", source->syncInfoSize_);
}

/**
 * @tc.name  : Test EnableSyncInfo with non-zero size
 * @tc.number: FastCaptureSource_EnableSyncInfo_002
 * @tc.desc  : Test EnableSyncInfo sets syncInfoSize to 1 when input is 1.
 */
HWTEST_F(FastAudioCaptureSourceAiut, FastCaptureSource_EnableSyncInfo_002, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<FastAudioCaptureSource>();
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    source->EnableSyncInfo(TEST_SYNC_INFO_SIZE_ONE);
    AIUT_EXPECT_EQ_INFO_LOG(source->syncInfoSize_, TEST_SYNC_INFO_SIZE_ONE,
        "syncInfoSize should be 1, got %u", source->syncInfoSize_);
}

// ==================== Lifecycle Tests ====================

/**
 * @tc.name  : Test FastAudioCaptureSource SetVolume without init
 * @tc.number: FastCaptureSource_SetVolume_001
 * @tc.desc  : Test SetVolume without init returns ERR_NOT_SUPPORTED.
 */
HWTEST_F(FastAudioCaptureSourceAiut, FastCaptureSource_SetVolume_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<FastAudioCaptureSource>();
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->SetVolume(1.0f, 1.0f);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED,
        "expected ERR_NOT_SUPPORTED for SetVolume without init, got %d", ret);
}

} // namespace AudioStandard
} // namespace OHOS
