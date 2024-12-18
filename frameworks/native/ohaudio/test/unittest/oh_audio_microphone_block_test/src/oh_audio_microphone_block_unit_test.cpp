/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "oh_audio_microphone_block_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void OHAudioMicrophoneBlockUnitTest::SetUpTestCase(void) { }

void OHAudioMicrophoneBlockUnitTest::TearDownTestCase(void) { }

void OHAudioMicrophoneBlockUnitTest::SetUp(void) { }

void OHAudioMicrophoneBlockUnitTest::TearDown(void) { }

static void MicrophoneBlockedCallback(OH_AudioDeviceDescriptorArray *audioDeviceDescriptorArray,
    OH_AudioDevice_BlockStatus status, void* userData)
{
    AUDIO_DEBUG_LOG("MicrophoneBlockedCallback triggrred, blocked status: %d\n", status);
    int size = audioDeviceDescriptorArray->size;
    for (int index = 0; index < size; index++) {
        OH_AudioDeviceDescriptor *audioDeviceDescriptor = audioDeviceDescriptorArray->descriptors[index];
        if (audioDeviceDescriptor) {
            OH_AudioDevice_Role deviceRole = AUDIO_DEVICE_ROLE_OUTPUT;
            OH_AudioDeviceDescriptor_GetDeviceRole(audioDeviceDescriptor, &deviceRole);
            OH_AudioDevice_Type deviceType = AUDIO_DEVICE_TYPE_INVALID;
            OH_AudioDeviceDescriptor_GetDeviceType(audioDeviceDescriptor, &deviceType);
            AUDIO_DEBUG_LOG("Receive new block DeviceRole: %d, DeviceType: %d\n", deviceRole, deviceType);
        }
    }
}

/**
* @tc.name  : Test OH_AudioRoutingManager_SetMicBlockStatusCallback API via legal state.
* @tc.number: OH_AudioRoutingManager_SetMicBlockStatusCallback_001
* @tc.desc  : Test OH_AudioRoutingManager_SetMicBlockStatusCallback interface with callback.
* Returns true if result is successful.
*/
HWTEST(OHAudioMicrophoneBlockUnitTest, OH_AudioRoutingManager_SetMicBlockStatusCallback_001, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioRoutingManager_OnDeviceBlockStatusCallback callback = MicrophoneBlockedCallback;
    result = OH_AudioRoutingManager_SetMicBlockStatusCallback(audioRoutingManager, callback, nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioRoutingManager_SetMicBlockStatusCallback API via legal state.
* @tc.number: OH_AudioRoutingManager_SetMicBlockStatusCallback_002
* @tc.desc  : Test OH_AudioRoutingManager_SetMicBlockStatusCallback interface with nullptr callback.

*/
HWTEST(OHAudioMicrophoneBlockUnitTest, OH_AudioRoutingManager_SetMicBlockStatusCallback_002, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioRoutingManager_OnDeviceBlockStatusCallback callback = nullptr;
    result = OH_AudioRoutingManager_SetMicBlockStatusCallback(audioRoutingManager, callback, nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioRoutingManager_SetMicBlockStatusCallback API via legal state.
* @tc.number: OH_AudioRoutingManager_SetMicBlockStatusCallback_003
* @tc.desc  : Test OH_AudioRoutingManager_SetMicBlockStatusCallback interface with nullptr audioRoutingManager.
* Returns true if result is successful.
*/
HWTEST(OHAudioMicrophoneBlockUnitTest, OH_AudioRoutingManager_SetMicBlockStatusCallback_003, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioRoutingManager_OnDeviceBlockStatusCallback callback = MicrophoneBlockedCallback;
    OH_AudioCommon_Result result = OH_AudioRoutingManager_SetMicBlockStatusCallback(audioRoutingManager,
        callback, nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}
} // namespace AudioStandard
} // namespace OHOS