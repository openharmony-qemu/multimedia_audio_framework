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

#include "oh_audio_routing_manager_unit_test.h"


using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void OHAudioRoutingManagerUnitTest::SetUpTestCase(void) { }

void OHAudioRoutingManagerUnitTest::TearDownTestCase(void) { }

void OHAudioRoutingManagerUnitTest::SetUp(void) { }

void OHAudioRoutingManagerUnitTest::TearDown(void) { }

int32_t callbackRet = -1;

static int32_t DeviceChangeCallback(OH_AudioDevice_ChangeType type,
    OH_AudioDeviceDescriptorArray *audioDeviceDescriptorArray)
{
    int size = audioDeviceDescriptorArray->size;
    callbackRet = 0;
    if (type == AUDIO_DEVICE_CHANGE_TYPE_CONNECT) {
        for (int index = 0; index < size; index++) {
            OH_AudioDeviceDescriptor *audioDeviceDescriptor = audioDeviceDescriptorArray->descriptors[index];
            if (audioDeviceDescriptor) {
                OH_AudioDevice_Role deviceRole = AUDIO_DEVICE_ROLE_OUTPUT;
                OH_AudioDeviceDescriptor_GetDeviceRole(audioDeviceDescriptor, &deviceRole);
                OH_AudioDevice_Type deviceType = AUDIO_DEVICE_TYPE_INVALID;
                OH_AudioDeviceDescriptor_GetDeviceType(audioDeviceDescriptor, &deviceType);
            }
        }
    }
    return 0;
}

static void DeviceBlockStatusCallback(OH_AudioDeviceDescriptorArray *audioDeviceDescriptorArray,
    OH_AudioDevice_BlockStatus status, void *userData)
{
    int size = audioDeviceDescriptorArray->size;
    callbackRet = 0;
    if (status == AUDIO_DEVICE_BLOCKED) {
        for (int index = 0; index < size; index++) {
            OH_AudioDeviceDescriptor *audioDeviceDescriptor = audioDeviceDescriptorArray->descriptors[index];
            if (audioDeviceDescriptor) {
                OH_AudioDevice_Role deviceRole = AUDIO_DEVICE_ROLE_OUTPUT;
                OH_AudioDeviceDescriptor_GetDeviceRole(audioDeviceDescriptor, &deviceRole);
                OH_AudioDevice_Type deviceType = AUDIO_DEVICE_TYPE_INVALID;
                OH_AudioDeviceDescriptor_GetDeviceType(audioDeviceDescriptor, &deviceType);
            }
        }
    }
}

/**
 * @tc.name  : Test OH_AudioRoutingManager_GetAvailableDevices with null audioRoutingManager.
 * @tc.number: OH_AudioRoutingManager_GetAvailableDevices_001
 * @tc.desc  : Test case for null audioRoutingManager.
 */
HWTEST(OHAudioRoutingManagerUnitTest, OH_AudioRoutingManager_GetAvailableDevices_001, TestSize.Level0)
{
    OH_AudioRoutingManager* audioRoutingManager = nullptr;
    OH_AudioDevice_Usage deviceUsage = AUDIO_DEVICE_USAGE_CALL_ALL;
    OH_AudioDeviceDescriptorArray* audioDeviceDescriptorArray = nullptr;

    auto result = OH_AudioRoutingManager_GetAvailableDevices(
        audioRoutingManager, deviceUsage, &audioDeviceDescriptorArray);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}


/**
 * @tc.name  : Test OH_AudioRoutingManager_GetAvailableDevices with invalid device usage.
 * @tc.number: OH_AudioRoutingManager_GetAvailableDevices_002
 * @tc.desc  : Test case for invalid device usage.
 */
HWTEST(OHAudioRoutingManagerUnitTest, OH_AudioRoutingManager_GetAvailableDevices_002, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    auto result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    OH_AudioDevice_Usage deviceUsage = {};
    OH_AudioDeviceDescriptorArray* audioDeviceDescriptorArray = nullptr;

    result = OH_AudioRoutingManager_GetAvailableDevices(audioRoutingManager, deviceUsage, &audioDeviceDescriptorArray);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRoutingManager_GetAvailableDevices with null audioRoutingManager.
 * @tc.number: OH_AudioRoutingManager_GetAvailableDevices_003
 * @tc.desc  : Test case for null audioRoutingManager.
 */
HWTEST(OHAudioRoutingManagerUnitTest, OH_AudioRoutingManager_GetAvailableDevices_003, TestSize.Level0)
{
    OH_AudioDevice_Usage deviceUsage = AUDIO_DEVICE_USAGE_CALL_ALL;
    OH_AudioDeviceDescriptorArray* audioDeviceDescriptorArray = nullptr;
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    auto result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_INPUT;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &audioDeviceDescriptorArray);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    audioRoutingManager = nullptr;

    result = OH_AudioRoutingManager_GetAvailableDevices(audioRoutingManager, deviceUsage, &audioDeviceDescriptorArray);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRoutingManager_GetAvailableDevices with null audioRoutingManager.
 * @tc.number: OH_AudioRoutingManager_GetAvailableDevices_004
 * @tc.desc  : Test case for null audioRoutingManager.
 */
HWTEST(OHAudioRoutingManagerUnitTest, OH_AudioRoutingManager_GetAvailableDevices_004, TestSize.Level0)
{
    OH_AudioDevice_Usage deviceUsage = AUDIO_DEVICE_USAGE_CALL_ALL;
    OH_AudioDeviceDescriptorArray* audioDeviceDescriptorArray = nullptr;
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    auto result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_INPUT;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &audioDeviceDescriptorArray);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    result = OH_AudioRoutingManager_GetAvailableDevices(
        audioRoutingManager, deviceUsage, &audioDeviceDescriptorArray);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioRoutingManager_GetPreferredOutputDevice.
 * @tc.number: OH_AudioRoutingManager_GetPreferredOutputDevice_001
 * @tc.desc  : Test OH_AudioRoutingManager_GetPreferredOutputDevice.
 */
HWTEST(OHAudioRoutingManagerUnitTest, OH_AudioRoutingManager_GetPreferredOutputDevice_001, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioStream_Usage streamUsage = {};
    OH_AudioDeviceDescriptorArray* audioDeviceDescriptorArray = nullptr;

    auto result = OH_AudioRoutingManager_GetPreferredOutputDevice(
        audioRoutingManager, streamUsage, &audioDeviceDescriptorArray);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRoutingManager_GetPreferredOutputDevice.
 * @tc.number: OH_AudioRoutingManager_GetPreferredOutputDevice_002
 * @tc.desc  : Test OH_AudioRoutingManager_GetPreferredOutputDevice.
 */
HWTEST(OHAudioRoutingManagerUnitTest, OH_AudioRoutingManager_GetPreferredOutputDevice_002, TestSize.Level0)
{
    OH_AudioStream_Usage streamUsage = {};
    OH_AudioDeviceDescriptorArray* audioDeviceDescriptorArray = nullptr;
    OH_AudioRoutingManager *audioRoutingManager = nullptr;

    auto result = OH_AudioRoutingManager_GetPreferredOutputDevice(
        audioRoutingManager, streamUsage, &audioDeviceDescriptorArray);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRoutingManager_GetPreferredOutputDevice.
 * @tc.number: OH_AudioRoutingManager_GetPreferredOutputDevice_003
 * @tc.desc  : Test OH_AudioRoutingManager_GetPreferredOutputDevice.
 */
HWTEST(OHAudioRoutingManagerUnitTest, OH_AudioRoutingManager_GetPreferredOutputDevice_003, TestSize.Level0)
{
    OH_AudioStream_Usage streamUsage = AUDIOSTREAM_USAGE_AUDIOBOOK;
    OH_AudioDeviceDescriptorArray* audioDeviceDescriptorArray = nullptr;
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    auto result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_INPUT;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &audioDeviceDescriptorArray);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    audioRoutingManager = nullptr;

    result = OH_AudioRoutingManager_GetPreferredOutputDevice(
        audioRoutingManager, streamUsage, &audioDeviceDescriptorArray);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRoutingManager_GetPreferredOutputDevice.
 * @tc.number: OH_AudioRoutingManager_GetPreferredOutputDevice_004
 * @tc.desc  : Test OH_AudioRoutingManager_GetPreferredOutputDevice.
 */
HWTEST(OHAudioRoutingManagerUnitTest, OH_AudioRoutingManager_GetPreferredOutputDevice_004, TestSize.Level0)
{
    OH_AudioStream_Usage streamUsage = AUDIOSTREAM_USAGE_AUDIOBOOK;
    OH_AudioDeviceDescriptorArray* audioDeviceDescriptorArray = nullptr;
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    auto result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_INPUT;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &audioDeviceDescriptorArray);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    result = OH_AudioRoutingManager_GetPreferredOutputDevice(
        audioRoutingManager, streamUsage, &audioDeviceDescriptorArray);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioRoutingManager_GetPreferredInputDevice.
 * @tc.number: OH_AudioRoutingManager_GetPreferredInputDevice_001
 * @tc.desc  : Test OH_AudioRoutingManager_GetPreferredInputDevice.
 */
HWTEST(OHAudioRoutingManagerUnitTest, OH_AudioRoutingManager_GetPreferredInputDevice_001, TestSize.Level0)
{
    OH_AudioStream_SourceType sourceType = AUDIOSTREAM_SOURCE_TYPE_VOICE_CALL;
    OH_AudioDeviceDescriptorArray* audioDeviceDescriptorArray = nullptr;
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    auto result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_INPUT;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &audioDeviceDescriptorArray);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    result = OH_AudioRoutingManager_GetPreferredInputDevice(
        audioRoutingManager, sourceType, &audioDeviceDescriptorArray);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRoutingManager_GetPreferredInputDevice.
 * @tc.number: OH_AudioRoutingManager_GetPreferredInputDevice_002
 * @tc.desc  : Test OH_AudioRoutingManager_GetPreferredInputDevice.
 */
HWTEST(OHAudioRoutingManagerUnitTest, OH_AudioRoutingManager_GetPreferredInputDevice_002, TestSize.Level0)
{
    OH_AudioStream_SourceType sourceType = {};
    OH_AudioDeviceDescriptorArray* audioDeviceDescriptorArray = nullptr;
    OH_AudioRoutingManager *audioRoutingManager = nullptr;

    auto result = OH_AudioRoutingManager_GetPreferredInputDevice(
        audioRoutingManager, sourceType, &audioDeviceDescriptorArray);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRoutingManager_GetPreferredInputDevice.
 * @tc.number: OH_AudioRoutingManager_GetPreferredInputDevice_003
 * @tc.desc  : Test OH_AudioRoutingManager_GetPreferredInputDevice.
 */
HWTEST(OHAudioRoutingManagerUnitTest, OH_AudioRoutingManager_GetPreferredInputDevice_003, TestSize.Level0)
{
    OH_AudioStream_SourceType sourceType = AUDIOSTREAM_SOURCE_TYPE_VOICE_COMMUNICATION;
    OH_AudioDeviceDescriptorArray* audioDeviceDescriptorArray = nullptr;
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    auto result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_INPUT;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &audioDeviceDescriptorArray);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    audioRoutingManager = nullptr;

    result = OH_AudioRoutingManager_GetPreferredInputDevice(
        audioRoutingManager, sourceType, &audioDeviceDescriptorArray);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRoutingManager_GetPreferredInputDevice.
 * @tc.number: OH_AudioRoutingManager_GetPreferredInputDevice_004
 * @tc.desc  : Test OH_AudioRoutingManager_GetPreferredInputDevice.
 */
HWTEST(OHAudioRoutingManagerUnitTest, OH_AudioRoutingManager_GetPreferredInputDevice_004, TestSize.Level0)
{
    OH_AudioStream_SourceType sourceType = AUDIOSTREAM_SOURCE_TYPE_VOICE_COMMUNICATION;
    OH_AudioDeviceDescriptorArray* audioDeviceDescriptorArray = nullptr;
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    auto result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_OUTPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    result = OH_AudioRoutingManager_GetPreferredInputDevice(
        audioRoutingManager, sourceType, &audioDeviceDescriptorArray);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioRoutingManager_ReleaseDevices.
 * @tc.number: OH_AudioRoutingManager_ReleaseDevices_001
 * @tc.desc  : Test OH_AudioRoutingManager_ReleaseDevices.
 */
HWTEST(OHAudioRoutingManagerUnitTest, OH_AudioRoutingManager_ReleaseDevices_001, TestSize.Level0)
{
    OH_AudioDeviceDescriptorArray* audioDeviceDescriptorArray = nullptr;
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    auto result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_INPUT;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &audioDeviceDescriptorArray);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, audioDeviceDescriptorArray);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioRoutingManager_IsMicBlockDetectionSupported.
 * @tc.number: OH_AudioRoutingManager_IsMicBlockDetectionSupported_001
 * @tc.desc  : Test OH_AudioRoutingManager_IsMicBlockDetectionSupported.
 */
HWTEST(OHAudioRoutingManagerUnitTest, OH_AudioRoutingManager_IsMicBlockDetectionSupported_001, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    auto result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    bool supported = true;

    result = OH_AudioRoutingManager_IsMicBlockDetectionSupported(audioRoutingManager, &supported);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioRoutingManager_IsMicBlockDetectionSupported.
 * @tc.number: OH_AudioRoutingManager_IsMicBlockDetectionSupported_002
 * @tc.desc  : Test OH_AudioRoutingManager_IsMicBlockDetectionSupported.
 */
HWTEST(OHAudioRoutingManagerUnitTest, OH_AudioRoutingManager_IsMicBlockDetectionSupported_002, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    auto result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    bool supported = false;

    result = OH_AudioRoutingManager_IsMicBlockDetectionSupported(audioRoutingManager, &supported);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioRoutingManager_IsMicBlockDetectionSupported.
 * @tc.number: OH_AudioRoutingManager_IsMicBlockDetectionSupported_003
 * @tc.desc  : Test OH_AudioRoutingManager_IsMicBlockDetectionSupported.
 */
HWTEST(OHAudioRoutingManagerUnitTest, OH_AudioRoutingManager_IsMicBlockDetectionSupported_003, TestSize.Level0)
{
    bool supported = false;
    auto result = OH_AudioRoutingManager_IsMicBlockDetectionSupported(nullptr, &supported);
    EXPECT_NE(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioRoutingManager_IsMicBlockDetectionSupported.
 * @tc.number: OH_AudioRoutingManager_IsMicBlockDetectionSupported_004
 * @tc.desc  : Test OH_AudioRoutingManager_IsMicBlockDetectionSupported.
 */
HWTEST(OHAudioRoutingManagerUnitTest, OH_AudioRoutingManager_IsMicBlockDetectionSupported_004, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    auto result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);

    result = OH_AudioRoutingManager_IsMicBlockDetectionSupported(audioRoutingManager, nullptr);
    EXPECT_NE(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test SetDeviceChangeCallback.
 * @tc.number: SetDeviceChangeCallback_001
 * @tc.desc  : Test SetDeviceChangeCallback.
 */
HWTEST(OHAudioRoutingManagerUnitTest, SetDeviceChangeCallback_001, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    auto result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_ALL;
    OH_AudioRoutingManager_OnDeviceChangedCallback callback = DeviceChangeCallback;

    result = OH_AudioRoutingManager_RegisterDeviceChangeCallback(audioRoutingManager, deviceFlag, callback);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test SetDeviceChangeCallback.
 * @tc.number: SetDeviceChangeCallback_002
 * @tc.desc  : Test SetDeviceChangeCallback.
 */
HWTEST(OHAudioRoutingManagerUnitTest, SetDeviceChangeCallback_002, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    auto result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_INPUT;
    OH_AudioRoutingManager_OnDeviceChangedCallback callback = DeviceChangeCallback;

    result = OH_AudioRoutingManager_RegisterDeviceChangeCallback(audioRoutingManager, deviceFlag, callback);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test OnDeviceChange.
 * @tc.number: OnDeviceChange_001
 * @tc.desc  : Test OnDeviceChange.
 */
HWTEST(OHAudioRoutingManagerUnitTest, OnDeviceChange_001, TestSize.Level0)
{
    OH_AudioRoutingManager_OnDeviceChangedCallback callback = DeviceChangeCallback;
    std::shared_ptr<OHAudioDeviceChangedCallback> ohAudioOnDeviceChangedCallback =
        std::make_shared<OHAudioDeviceChangedCallback>(callback);
    DeviceChangeAction action;
    callbackRet = -1;
    ohAudioOnDeviceChangedCallback->OnDeviceChange(action);
    EXPECT_EQ(callbackRet, -1);

    callbackRet = -1;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    action.deviceDescriptors.push_back(audioDeviceDescriptor);
    ohAudioOnDeviceChangedCallback->OnDeviceChange(action);
    EXPECT_EQ(callbackRet, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioRoutingManager_UnregisterDeviceChangeCallback.
 * @tc.number: OH_AudioRoutingManager_UnregisterDeviceChangeCallback_001
 * @tc.desc  : Test OH_AudioRoutingManager_UnregisterDeviceChangeCallback.
 */
HWTEST(OHAudioRoutingManagerUnitTest, OH_AudioRoutingManager_UnregisterDeviceChangeCallback_001, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    auto result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioRoutingManager_OnDeviceChangedCallback callback = DeviceChangeCallback;

    result = OH_AudioRoutingManager_UnregisterDeviceChangeCallback(audioRoutingManager, callback);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioRoutingManager_UnregisterDeviceChangeCallback.
 * @tc.number: OH_AudioRoutingManager_UnregisterDeviceChangeCallback_002
 * @tc.desc  : Test OH_AudioRoutingManager_UnregisterDeviceChangeCallback.
 */
HWTEST(OHAudioRoutingManagerUnitTest, OH_AudioRoutingManager_UnregisterDeviceChangeCallback_002, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    auto result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioRoutingManager_OnDeviceChangedCallback callback = DeviceChangeCallback;

    result = OH_AudioRoutingManager_UnregisterDeviceChangeCallback(audioRoutingManager, callback);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test SetMicrophoneBlockedCallback.
 * @tc.number: SetMicrophoneBlockedCallback_001
 * @tc.desc  : Test SetMicrophoneBlockedCallback.
 */
HWTEST(OHAudioRoutingManagerUnitTest, SetMicrophoneBlockedCallback_001, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    auto result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioRoutingManager_OnDeviceBlockStatusCallback callback = nullptr;

    result = OH_AudioRoutingManager_SetMicBlockStatusCallback(audioRoutingManager, callback, nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
 * @tc.name  : Test OnMicrophoneBlocked.
 * @tc.number: OnMicrophoneBlocked_001
 * @tc.desc  : Test OnMicrophoneBlocked.
 */
HWTEST(OHAudioRoutingManagerUnitTest, OnMicrophoneBlocked_001, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    auto result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioRoutingManager_OnDeviceBlockStatusCallback callback = DeviceBlockStatusCallback;

    result = OH_AudioRoutingManager_SetMicBlockStatusCallback(audioRoutingManager, callback, nullptr);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);

    std::shared_ptr<OHMicrophoneBlockCallback> microphoneBlock =
        std::make_shared<OHMicrophoneBlockCallback>(callback, nullptr);
    MicrophoneBlockedInfo info;
    callbackRet = -1;
    microphoneBlock->OnMicrophoneBlocked(info);
    EXPECT_EQ(callbackRet, -1);

    callbackRet = -1;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    info.devices.push_back(audioDeviceDescriptor);
    microphoneBlock->OnMicrophoneBlocked(info);
    EXPECT_EQ(callbackRet, AUDIOCOMMON_RESULT_SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS