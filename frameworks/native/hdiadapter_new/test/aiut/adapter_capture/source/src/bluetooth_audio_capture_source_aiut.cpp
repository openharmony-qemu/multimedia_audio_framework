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

#include "bluetooth_audio_capture_source_aiut.h"
#include "aiut_log.h"
#include "source/bluetooth_audio_capture_source.h"
#include "source/i_audio_capture_source.h"
#include "audio_errors.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

namespace {
constexpr uint32_t TEST_CAPTURE_ID = 1;
} // namespace

// ==================== BluetoothAudioCaptureSourceAiut ====================

void BluetoothAudioCaptureSourceAiut::SetUpTestCase(void) {}
void BluetoothAudioCaptureSourceAiut::TearDownTestCase(void) {}
void BluetoothAudioCaptureSourceAiut::SetUp(void) {}
void BluetoothAudioCaptureSourceAiut::TearDown(void) {}

// ==================== Construction / Destruction Tests ====================

/**
 * @tc.name  : Test BluetoothAudioCaptureSource construction
 * @tc.number: BtCaptureSource_Constructor_001
 * @tc.desc  : Test BluetoothAudioCaptureSource constructor creates uninitialized object.
 */
HWTEST_F(BluetoothAudioCaptureSourceAiut, BtCaptureSource_Constructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<BluetoothAudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    AIUT_EXPECT_FALSE_INFO_LOG(source->IsInited(), "newly constructed source should not be inited");
}

/**
 * @tc.name  : Test BluetoothAudioCaptureSource destruction
 * @tc.number: BtCaptureSource_Destructor_001
 * @tc.desc  : Test destructor on uninitialized source does not crash.
 */
HWTEST_F(BluetoothAudioCaptureSourceAiut, BtCaptureSource_Destructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<BluetoothAudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    source = nullptr;
    AIUT_EXPECT_EQ_INFO_LOG(source, nullptr, "source should be null after reset");
}

// ==================== Lifecycle Tests ====================

/**
 * @tc.name  : Test BluetoothAudioCaptureSource Start without init
 * @tc.number: BtCaptureSource_StartWithoutInit_001
 * @tc.desc  : Test Start() before Init() returns ERR_INVALID_HANDLE.
 */
HWTEST_F(BluetoothAudioCaptureSourceAiut, BtCaptureSource_StartWithoutInit_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<BluetoothAudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->Start();
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_HANDLE,
        "expected ERR_INVALID_HANDLE for Start without init, got %d", ret);
}

/**
 * @tc.name  : Test BluetoothAudioCaptureSource Stop without init
 * @tc.number: BtCaptureSource_StopWithoutInit_001
 * @tc.desc  : Test Stop() before Init() returns SUCCESS.
 */
HWTEST_F(BluetoothAudioCaptureSourceAiut, BtCaptureSource_StopWithoutInit_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<BluetoothAudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->Stop();
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "expected SUCCESS for Stop without init, got %d", ret);
}

/**
 * @tc.name  : Test BluetoothAudioCaptureSource Resume without init
 * @tc.number: BtCaptureSource_ResumeWithoutInit_001
 * @tc.desc  : Test Resume() before Init() returns ERR_INVALID_HANDLE.
 */
HWTEST_F(BluetoothAudioCaptureSourceAiut, BtCaptureSource_ResumeWithoutInit_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<BluetoothAudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->Resume();
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_HANDLE,
        "expected ERR_INVALID_HANDLE for Resume without init, got %d", ret);
}

/**
 * @tc.name  : Test BluetoothAudioCaptureSource Pause without init
 * @tc.number: BtCaptureSource_PauseWithoutInit_001
 * @tc.desc  : Test Pause() before Init() returns ERR_INVALID_HANDLE.
 */
HWTEST_F(BluetoothAudioCaptureSourceAiut, BtCaptureSource_PauseWithoutInit_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<BluetoothAudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->Pause();
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_HANDLE,
        "expected ERR_INVALID_HANDLE for Pause without init, got %d", ret);
}

// ==================== SetAudioScene Tests ====================

/**
 * @tc.name  : Test BluetoothAudioCaptureSource SetAudioScene
 * @tc.number: BtCaptureSource_SetAudioScene_001
 * @tc.desc  : Test SetAudioScene returns SUCCESS without init.
 */
HWTEST_F(BluetoothAudioCaptureSourceAiut, BtCaptureSource_SetAudioScene_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<BluetoothAudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->SetAudioScene(AUDIO_SCENE_DEFAULT);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "expected SUCCESS for SetAudioScene, got %d", ret);
}

// ==================== UpdateActiveDevice Tests ====================

/**
 * @tc.name  : Test BluetoothAudioCaptureSource UpdateActiveDevice
 * @tc.number: BtCaptureSource_UpdateActiveDevice_001
 * @tc.desc  : Test UpdateActiveDevice returns ERR_NOT_SUPPORTED without init.
 */
HWTEST_F(BluetoothAudioCaptureSourceAiut, BtCaptureSource_UpdateActiveDevice_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<BluetoothAudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    int32_t ret = source->UpdateActiveDevice(DEVICE_TYPE_SPEAKER);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED,
        "expected ERR_NOT_SUPPORTED for UpdateActiveDevice, got %d", ret);
}

// ==================== SetInvalidState Tests ====================

/**
 * @tc.name  : Test BluetoothAudioCaptureSource SetInvalidState
 * @tc.number: BtCaptureSource_SetInvalidState_001
 * @tc.desc  : Test SetInvalidState followed by Init/DeInit works correctly.
 */
HWTEST_F(BluetoothAudioCaptureSourceAiut, BtCaptureSource_SetInvalidState_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto source = std::make_shared<BluetoothAudioCaptureSource>(TEST_CAPTURE_ID);
    AIUT_ASSERT_NE_INFO_LOG(source, nullptr, "source is null");
    source->SetInvalidState();
    IAudioSourceAttr attr = {};
    (void)source->Init(attr);
    source->DeInit();
    AIUT_EXPECT_FALSE_INFO_LOG(source->IsInited(), "source should not be inited after DeInit");
}

} // namespace AudioStandard
} // namespace OHOS
