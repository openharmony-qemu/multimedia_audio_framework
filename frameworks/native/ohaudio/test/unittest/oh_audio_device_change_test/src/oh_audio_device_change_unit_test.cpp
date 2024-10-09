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

#include "oh_audio_device_change_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void OHAudioDeviceChangeUnitTest::SetUpTestCase(void) { }

void OHAudioDeviceChangeUnitTest::TearDownTestCase(void) { }

void OHAudioDeviceChangeUnitTest::SetUp(void) { }

void OHAudioDeviceChangeUnitTest::TearDown(void) { }

static int32_t DeviceChangeCallback(OH_AudioDevice_ChangeType type,
    OH_AudioDeviceDescriptorArray *audioDeviceDescriptorArray)
{
    AUDIO_DEBUG_LOG("DeviceChangeCallback triggrred, ChangeType: %d\n", type);
    int size = audioDeviceDescriptorArray->size;
    if (type == AUDIO_DEVICE_CHANGE_TYPE_CONNECT) {
        for (int index = 0; index < size; index++) {
            OH_AudioDeviceDescriptor *audioDeviceDescriptor = audioDeviceDescriptorArray->descriptors[index];
            if (audioDeviceDescriptor) {
                OH_AudioDevice_Role deviceRole = AUDIO_DEVICE_ROLE_OUTPUT;
                OH_AudioDeviceDescriptor_GetDeviceRole(audioDeviceDescriptor, &deviceRole);
                OH_AudioDevice_Type deviceType = AUDIO_DEVICE_TYPE_INVALID;
                OH_AudioDeviceDescriptor_GetDeviceType(audioDeviceDescriptor, &deviceType);
                AUDIO_DEBUG_LOG("Receive new device: DeviceRole: %d, DeviceType: %d\n", deviceRole, deviceType);
            }
        }
    }
    return 0;
}

/**
* @tc.name  : Test OH_AudioManager_GetAudioRoutingManager API via legal state.
* @tc.number: OH_AudioManager_GetAudioRoutingManager_001
* @tc.desc  : Test OH_AudioManager_GetAudioRoutingManager interface. Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioManager_GetAudioRoutingManager_001, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
}

/**
* @tc.name  : Test OH_AudioRoutingManager_GetDevices API via legal state.
* @tc.number: OH_AudioRoutingManager_GetDevices_001
* @tc.desc  : Test OH_AudioRoutingManager_GetDevices interface with NONE_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioRoutingManager_GetDevices_001, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_NONE;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_NO_MEMORY);
    EXPECT_EQ(array, nullptr);
}

/**
* @tc.name  : Test OH_AudioRoutingManager_GetDevices API via legal state.
* @tc.number: OH_AudioRoutingManager_GetDevices_002
* @tc.desc  : Test OH_AudioRoutingManager_GetDevices interface with OUTPUT_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioRoutingManager_GetDevices_002, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_OUTPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);
    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioRoutingManager_GetDevices API via legal state.
* @tc.number: OH_AudioRoutingManager_GetDevices_003
* @tc.desc  : Test OH_AudioRoutingManager_GetDevices interface with INPUT_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioRoutingManager_GetDevices_003, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_INPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);
    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioRoutingManager_GetDevices API via legal state.
* @tc.number: OH_AudioRoutingManager_GetDevices_004
* @tc.desc  : Test OH_AudioRoutingManager_GetDevices interface with ALL_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioRoutingManager_GetDevices_004, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_ALL;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);
    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioRoutingManager_GetDevices API via illegal state.
* @tc.number: OH_AudioRoutingManager_GetDevices_005
* @tc.desc  : Test OH_AudioRoutingManager_GetDevices interface with invalid deviceFlag.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioRoutingManager_GetDevices_005, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    int32_t flag = -1;
    OH_AudioDevice_Flag deviceFlag = (OH_AudioDevice_Flag)flag;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
* @tc.name  : Test OH_AudioRoutingManager_GetDevices API via illegal state.
* @tc.number: OH_AudioRoutingManager_GetDevices_006
* @tc.desc  : Test OH_AudioRoutingManager_GetDevices interface with nullptr audioRoutingManager.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioRoutingManager_GetDevices_006, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_OUTPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    OH_AudioCommon_Result result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
* @tc.name  : Test OH_AudioRoutingManager_RegisterDeviceChangeCallback API via legal state.
* @tc.number: OH_AudioRoutingManager_RegisterDeviceChangeCallback_001
* @tc.desc  : Test OH_AudioRoutingManager_RegisterDeviceChangeCallback interface with NONE_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioRoutingManager_RegisterDeviceChangeCallback_001, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_NONE;
    OH_AudioRoutingManager_OnDeviceChangedCallback callback = DeviceChangeCallback;
    result = OH_AudioRoutingManager_RegisterDeviceChangeCallback(audioRoutingManager, deviceFlag, callback);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioRoutingManager_RegisterDeviceChangeCallback API via legal state.
* @tc.number: OH_AudioRoutingManager_RegisterDeviceChangeCallback_002
* @tc.desc  : Test OH_AudioRoutingManager_RegisterDeviceChangeCallback interface with OUTPUT_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioRoutingManager_RegisterDeviceChangeCallback_002, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_OUTPUT;
    OH_AudioRoutingManager_OnDeviceChangedCallback callback = DeviceChangeCallback;
    result = OH_AudioRoutingManager_RegisterDeviceChangeCallback(audioRoutingManager, deviceFlag, callback);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioRoutingManager_RegisterDeviceChangeCallback API via legal state.
* @tc.number: OH_AudioRoutingManager_RegisterDeviceChangeCallback_003
* @tc.desc  : Test OH_AudioRoutingManager_RegisterDeviceChangeCallback interface with INPUT_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioRoutingManager_RegisterDeviceChangeCallback_003, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_INPUT;
    OH_AudioRoutingManager_OnDeviceChangedCallback callback = DeviceChangeCallback;
    result = OH_AudioRoutingManager_RegisterDeviceChangeCallback(audioRoutingManager, deviceFlag, callback);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioRoutingManager_RegisterDeviceChangeCallback API via legal state.
* @tc.number: OH_AudioRoutingManager_RegisterDeviceChangeCallback_004
* @tc.desc  : Test OH_AudioRoutingManager_RegisterDeviceChangeCallback interface with ALL_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioRoutingManager_RegisterDeviceChangeCallback_004, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_ALL;
    OH_AudioRoutingManager_OnDeviceChangedCallback callback = DeviceChangeCallback;
    result = OH_AudioRoutingManager_RegisterDeviceChangeCallback(audioRoutingManager, deviceFlag, callback);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioRoutingManager_RegisterDeviceChangeCallback API via illegal state.
* @tc.number: OH_AudioRoutingManager_RegisterDeviceChangeCallback_005
* @tc.desc  : Test OH_AudioRoutingManager_RegisterDeviceChangeCallback interface with nullptr audioRoutingManager.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioRoutingManager_RegisterDeviceChangeCallback_005, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_OUTPUT;
    OH_AudioRoutingManager_OnDeviceChangedCallback callback = DeviceChangeCallback;
    OH_AudioCommon_Result result =
        OH_AudioRoutingManager_RegisterDeviceChangeCallback(audioRoutingManager, deviceFlag, callback);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
* @tc.name  : Test OH_AudioRoutingManager_RegisterDeviceChangeCallback API via illegal state.
* @tc.number: OH_AudioRoutingManager_RegisterDeviceChangeCallback_006
* @tc.desc  : Test OH_AudioRoutingManager_RegisterDeviceChangeCallback interface with nullptr callback.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioRoutingManager_RegisterDeviceChangeCallback_006, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_ALL;
    OH_AudioRoutingManager_OnDeviceChangedCallback callback = nullptr;
    result = OH_AudioRoutingManager_RegisterDeviceChangeCallback(audioRoutingManager, deviceFlag, callback);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
* @tc.name  : Test OH_AudioRoutingManager_UnregisterDeviceChangeCallback API via legal state.
* @tc.number: OH_AudioRoutingManager_UnregisterDeviceChangeCallback_001
* @tc.desc  : Test OH_AudioRoutingManager_UnregisterDeviceChangeCallback interface.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioRoutingManager_UnregisterDeviceChangeCallback_001, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioRoutingManager_OnDeviceChangedCallback callback = DeviceChangeCallback;
    result = OH_AudioRoutingManager_UnregisterDeviceChangeCallback(audioRoutingManager, callback);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioRoutingManager_UnregisterDeviceChangeCallback API via illegal state.
* @tc.number: OH_AudioRoutingManager_UnregisterDeviceChangeCallback_002
* @tc.desc  : Test OH_AudioRoutingManager_UnregisterDeviceChangeCallback interface with nullptr audioRoutingManager.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioRoutingManager_UnregisterDeviceChangeCallback_002, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioRoutingManager_OnDeviceChangedCallback callback = DeviceChangeCallback;
    OH_AudioCommon_Result result = OH_AudioRoutingManager_UnregisterDeviceChangeCallback(
        audioRoutingManager, callback);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
* @tc.name  : Test OH_AudioRoutingManager_UnregisterDeviceChangeCallback API via illegal state.
* @tc.number: OH_AudioRoutingManager_UnregisterDeviceChangeCallback_003
* @tc.desc  : Test OH_AudioRoutingManager_UnregisterDeviceChangeCallback interface with nullptr callback.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioRoutingManager_UnregisterDeviceChangeCallback_003, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioRoutingManager_OnDeviceChangedCallback callback = nullptr;
    result = OH_AudioRoutingManager_UnregisterDeviceChangeCallback(audioRoutingManager, callback);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
* @tc.name  : Test OH_AudioRoutingManager_ReleaseDevices API via illegal state.
* @tc.number: OH_AudioRoutingManager_ReleaseDevices_001
* @tc.desc  : Test OH_AudioRoutingManager_ReleaseDevices interface with nullptr audioRoutingManager.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioRoutingManager_ReleaseDevices_001, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioDeviceDescriptorArray *audioDeviceDescriptorArray = nullptr;
    OH_AudioCommon_Result result =
        OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, audioDeviceDescriptorArray);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
* @tc.name  : Test OH_AudioRoutingManager_ReleaseDevices API via illegal state.
* @tc.number: OH_AudioRoutingManager_ReleaseDevices_002
* @tc.desc  : Test OH_AudioRoutingManager_ReleaseDevices interface with nullptr audioDeviceDescriptorArray.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioRoutingManager_ReleaseDevices_002, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    OH_AudioDeviceDescriptorArray *audioDeviceDescriptorArray = nullptr;
    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, audioDeviceDescriptorArray);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceType API via legal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceType_001
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceType interface with OUTPUT_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceType_001, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_OUTPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            OH_AudioDevice_Type deviceType = AUDIO_DEVICE_TYPE_INVALID;
            result = OH_AudioDeviceDescriptor_GetDeviceType(descriptor, &deviceType);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
            AUDIO_DEBUG_LOG("DeviceType: %d\n", deviceType);
        }
    }
    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceType API via legal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceType_002
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceType interface with INPUT_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceType_002, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_INPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            OH_AudioDevice_Type deviceType = AUDIO_DEVICE_TYPE_INVALID;
            result = OH_AudioDeviceDescriptor_GetDeviceType(descriptor, &deviceType);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
            AUDIO_DEBUG_LOG("DeviceType: %d\n", deviceType);
        }
    }
    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceType API via legal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceType_003
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceType interface with ALL_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceType_003, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_ALL;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            OH_AudioDevice_Type deviceType = AUDIO_DEVICE_TYPE_INVALID;
            result = OH_AudioDeviceDescriptor_GetDeviceType(descriptor, &deviceType);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
            AUDIO_DEBUG_LOG("DeviceType: %d\n", deviceType);
        }
    }
    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceType API via illegal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceType_004
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceType interface with nullptr deviceType.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceType_004, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_OUTPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            result = OH_AudioDeviceDescriptor_GetDeviceType(descriptor, nullptr);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
        }
    }
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceType API via illegal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceType_005
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceType interface with nullptr descriptor.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceType_005, TestSize.Level0)
{
    OH_AudioDeviceDescriptor *descriptor = nullptr;
    OH_AudioDevice_Type deviceType = AUDIO_DEVICE_TYPE_INVALID;
    OH_AudioCommon_Result result = OH_AudioDeviceDescriptor_GetDeviceType(descriptor, &deviceType);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceRole API via legal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceRole_001
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceRole interface with OUTPUT_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceRole_001, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_OUTPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            OH_AudioDevice_Role deviceRole = AUDIO_DEVICE_ROLE_OUTPUT;
            result = OH_AudioDeviceDescriptor_GetDeviceRole(descriptor, &deviceRole);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
            AUDIO_DEBUG_LOG("DeviceRole: %d\n", deviceRole);
        }
    }
    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceRole API via legal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceRole_002
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceRole interface with INPUT_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceRole_002, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_INPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            OH_AudioDevice_Role deviceRole = AUDIO_DEVICE_ROLE_OUTPUT;
            result = OH_AudioDeviceDescriptor_GetDeviceRole(descriptor, &deviceRole);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
            AUDIO_DEBUG_LOG("DeviceRole: %d\n", deviceRole);
        }
    }
    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceRole API via legal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceRole_003
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceRole interface with ALL_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceRole_003, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_ALL;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            OH_AudioDevice_Role deviceRole = AUDIO_DEVICE_ROLE_OUTPUT;
            result = OH_AudioDeviceDescriptor_GetDeviceRole(descriptor, &deviceRole);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
            AUDIO_DEBUG_LOG("DeviceRole: %d\n", deviceRole);
        }
    }
    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceRole API via illegal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceRole_004
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceRole interface with nullptr deviceRole.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceRole_004, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_OUTPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            result = OH_AudioDeviceDescriptor_GetDeviceRole(descriptor, nullptr);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
        }
    }
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceRole API via illegal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceRole_005
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceRole interface with nullptr descriptor.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceRole_005, TestSize.Level0)
{
    OH_AudioDeviceDescriptor *descriptor = nullptr;
    OH_AudioDevice_Role deviceRole = AUDIO_DEVICE_ROLE_OUTPUT;
    OH_AudioCommon_Result result = OH_AudioDeviceDescriptor_GetDeviceRole(descriptor, &deviceRole);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceId API via legal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceId_001
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceId interface with OUTPUT_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceId_001, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_OUTPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            uint32_t id = 0;
            result = OH_AudioDeviceDescriptor_GetDeviceId(descriptor, &id);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
            AUDIO_DEBUG_LOG("DeviceId: %d\n", id);
        }
    }
    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceId API via legal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceId_002
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceId interface with INPUT_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceId_002, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_INPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            uint32_t id = 0;
            result = OH_AudioDeviceDescriptor_GetDeviceId(descriptor, &id);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
            AUDIO_DEBUG_LOG("DeviceId: %d\n", id);
        }
    }
    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceId API via legal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceId_003
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceId interface with ALL_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceId_003, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_ALL;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            uint32_t id = 0;
            result = OH_AudioDeviceDescriptor_GetDeviceId(descriptor, &id);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
            AUDIO_DEBUG_LOG("DeviceId: %d\n", id);
        }
    }
    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceId API via illegal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceId_004
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceId interface with nullptr deviceID.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceId_004, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_OUTPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            result = OH_AudioDeviceDescriptor_GetDeviceId(descriptor, nullptr);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
        }
    }
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceId API via illegal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceId_005
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceId interface with nullptr descriptor.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceId_005, TestSize.Level0)
{
    OH_AudioDeviceDescriptor *descriptor = nullptr;
    uint32_t id = 0;
    OH_AudioCommon_Result result = OH_AudioDeviceDescriptor_GetDeviceId(descriptor, &id);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceName API via legal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceName_001
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceName interface with OUTPUT_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceName_001, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_OUTPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            char* deviceName;
            result = OH_AudioDeviceDescriptor_GetDeviceName(descriptor, &deviceName);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
            AUDIO_DEBUG_LOG("DeviceName: %s\n", deviceName);
        }
    }
    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceName API via legal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceName_002
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceName interface with INPUT_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceName_002, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_INPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            char* deviceName;
            result = OH_AudioDeviceDescriptor_GetDeviceName(descriptor, &deviceName);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
            AUDIO_DEBUG_LOG("DeviceName: %s\n", deviceName);
        }
    }
    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceName API via legal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceName_003
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceName interface with ALL_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceName_003, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_ALL;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            char* deviceName;
            result = OH_AudioDeviceDescriptor_GetDeviceName(descriptor, &deviceName);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
            AUDIO_DEBUG_LOG("DeviceName: %s\n", deviceName);
        }
    }
    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceName API via illegal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceName_004
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceName interface with nullptr deviceName.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceName_004, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_OUTPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            result = OH_AudioDeviceDescriptor_GetDeviceName(descriptor, nullptr);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
        }
    }
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceName API via illegal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceName_005
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceName interface with nullptr descriptor.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceName_005, TestSize.Level0)
{
    OH_AudioDeviceDescriptor *descriptor = nullptr;
    char* deviceName;
    OH_AudioCommon_Result result = OH_AudioDeviceDescriptor_GetDeviceName(descriptor, &deviceName);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceAddress API via legal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceAddress_001
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceAddress interface with OUTPUT_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceAddress_001, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_OUTPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            char *address;
            result = OH_AudioDeviceDescriptor_GetDeviceAddress(descriptor, &address);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
            AUDIO_DEBUG_LOG("DeviceAddress: %s\n", address);
        }
    }
    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceAddress API via legal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceAddress_002
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceAddress interface with INPUT_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceAddress_002, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_INPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            char *address;
            result = OH_AudioDeviceDescriptor_GetDeviceAddress(descriptor, &address);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
            AUDIO_DEBUG_LOG("DeviceAddress: %s\n", address);
        }
    }
    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceAddress API via legal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceAddress_003
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceAddress interface with ALL_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceAddress_003, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_ALL;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            char *address;
            result = OH_AudioDeviceDescriptor_GetDeviceAddress(descriptor, &address);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
            AUDIO_DEBUG_LOG("DeviceAddress: %s\n", address);
        }
    }
    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceAddress API via illegal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceAddress_004
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceAddress interface with nullptr deviceAddress.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceAddress_004, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_OUTPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            result = OH_AudioDeviceDescriptor_GetDeviceAddress(descriptor, nullptr);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
        }
    }
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceAddress API via illegal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceAddress_005
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceAddress interface with nullptr descriptor.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceAddress_005, TestSize.Level0)
{
    OH_AudioDeviceDescriptor *descriptor = nullptr;
    char *address;
    OH_AudioCommon_Result result = OH_AudioDeviceDescriptor_GetDeviceAddress(descriptor, &address);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceSampleRates API via legal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceSampleRates_001
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceSampleRates interface with OUTPUT_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceSampleRates_001, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_OUTPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            uint32_t *sampleRates;
            uint32_t size = 0;
            result = OH_AudioDeviceDescriptor_GetDeviceSampleRates(descriptor, &sampleRates, &size);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
            AUDIO_DEBUG_LOG("DeviceSampleRates: %d, size = %d\n", *sampleRates, size);
        }
    }
    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceSampleRates API via legal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceSampleRates_002
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceSampleRates interface with INPUT_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceSampleRates_002, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_INPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            uint32_t *sampleRates;
            uint32_t size = 0;
            result = OH_AudioDeviceDescriptor_GetDeviceSampleRates(descriptor, &sampleRates, &size);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
            AUDIO_DEBUG_LOG("DeviceSampleRates: %d, size = %d\n", *sampleRates, size);
        }
    }
    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceSampleRates API via legal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceSampleRates_003
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceSampleRates interface with ALL_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceSampleRates_003, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_ALL;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            uint32_t *sampleRates;
            uint32_t size = 0;
            result = OH_AudioDeviceDescriptor_GetDeviceSampleRates(descriptor, &sampleRates, &size);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
            AUDIO_DEBUG_LOG("DeviceSampleRates: %d, size = %d\n", *sampleRates, size);
        }
    }
    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceSampleRates API via illegal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceSampleRates_004
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceSampleRates interface with nullptr sampleRates.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceSampleRates_004, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_OUTPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            uint32_t size = 0;
            result = OH_AudioDeviceDescriptor_GetDeviceSampleRates(descriptor, nullptr, &size);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
        }
    }
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceSampleRates API via illegal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceSampleRates_005
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceSampleRates interface with nullptr size.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceSampleRates_005, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_OUTPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            uint32_t *sampleRates;
            result = OH_AudioDeviceDescriptor_GetDeviceSampleRates(descriptor, &sampleRates, nullptr);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
        }
    }
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceSampleRates API via illegal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceSampleRates_006
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceSampleRates interface with nullptr descriptor.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceSampleRates_006, TestSize.Level0)
{
    OH_AudioDeviceDescriptor *descriptor = nullptr;
    uint32_t *sampleRates;
    uint32_t size = 0;
    OH_AudioCommon_Result result = OH_AudioDeviceDescriptor_GetDeviceSampleRates(descriptor, &sampleRates, &size);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceChannelCounts API via legal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceChannelCounts_001
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceChannelCounts interface with OUTPUT_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceChannelCounts_001, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_OUTPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            uint32_t *channelCounts;
            uint32_t channelSize = 0;
            result = OH_AudioDeviceDescriptor_GetDeviceChannelCounts(descriptor, &channelCounts, &channelSize);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
            AUDIO_DEBUG_LOG("DeviceChannelCounts: %d, channelSize = %d\n", *channelCounts, channelSize);
        }
    }
    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceChannelCounts API via legal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceChannelCounts_002
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceChannelCounts interface with INPUT_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceChannelCounts_002, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_INPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            uint32_t *channelCounts;
            uint32_t channelSize = 0;
            result = OH_AudioDeviceDescriptor_GetDeviceChannelCounts(descriptor, &channelCounts, &channelSize);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
            AUDIO_DEBUG_LOG("DeviceChannelCounts: %d, channelSize = %d\n", *channelCounts, channelSize);
        }
    }
    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceChannelCounts API via legal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceChannelCounts_003
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceChannelCounts interface with ALL_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceChannelCounts_003, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_ALL;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            uint32_t *channelCounts;
            uint32_t channelSize = 0;
            result = OH_AudioDeviceDescriptor_GetDeviceChannelCounts(descriptor, &channelCounts, &channelSize);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
            AUDIO_DEBUG_LOG("DeviceChannelCounts: %d, channelSize = %d\n", *channelCounts, channelSize);
        }
    }
    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceChannelCounts API via illegal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceChannelCounts_004
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceChannelCounts interface with nullptr channelCounts.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceChannelCounts_004, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_OUTPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            uint32_t channelSize = 0;
            result = OH_AudioDeviceDescriptor_GetDeviceChannelCounts(descriptor, nullptr, &channelSize);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
        }
    }
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceChannelCounts API via illegal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceChannelCounts_005
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceChannelCounts interface with nullptr size.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceChannelCounts_005, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_OUTPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            uint32_t *channelCounts;
            result = OH_AudioDeviceDescriptor_GetDeviceChannelCounts(descriptor, &channelCounts, nullptr);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
        }
    }
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceChannelCounts API via illegal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceChannelCounts_006
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceChannelCounts interface with nullptr descriptor.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceChannelCounts_006, TestSize.Level0)
{
    OH_AudioDeviceDescriptor *descriptor = nullptr;
    uint32_t *channelCounts;
    uint32_t channelSize = 0;
    OH_AudioCommon_Result result = OH_AudioDeviceDescriptor_GetDeviceChannelCounts(
        descriptor, &channelCounts, &channelSize);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceDisplayName API via legal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceDisplayName_001
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceDisplayName interface with OUTPUT_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceDisplayName_001, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_OUTPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            char *displayName;
            result = OH_AudioDeviceDescriptor_GetDeviceDisplayName(descriptor, &displayName);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
            AUDIO_DEBUG_LOG("DeviceDisplayName: %s\n", displayName);
        }
    }
    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceDisplayName API via legal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceDisplayName_002
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceDisplayName interface with INPUT_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceDisplayName_002, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_INPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            char *displayName;
            result = OH_AudioDeviceDescriptor_GetDeviceDisplayName(descriptor, &displayName);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
            AUDIO_DEBUG_LOG("DeviceDisplayName: %s\n", displayName);
        }
    }
    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceDisplayName API via legal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceDisplayName_003
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceDisplayName interface with ALL_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceDisplayName_003, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_ALL;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            char *displayName;
            result = OH_AudioDeviceDescriptor_GetDeviceDisplayName(descriptor, &displayName);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
            AUDIO_DEBUG_LOG("DeviceDisplayName: %s\n", displayName);
        }
    }
    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceDisplayName API via illegal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceDisplayName_004
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceDisplayName interface with nullptr displayName.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceDisplayName_004, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_OUTPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            result = OH_AudioDeviceDescriptor_GetDeviceDisplayName(descriptor, nullptr);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
        }
    }
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceDisplayName API via illegal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceDisplayName_005
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceDisplayName interface with nullptr descriptor.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceDisplayName_005, TestSize.Level0)
{
    OH_AudioDeviceDescriptor *descriptor = nullptr;
    char *displayName;
    OH_AudioCommon_Result result = OH_AudioDeviceDescriptor_GetDeviceDisplayName(descriptor, &displayName);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceEncodingTypes API via legal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceEncodingTypes_001
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceEncodingTypes interface with OUTPUT_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceEncodingTypes_001, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_OUTPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            OH_AudioStream_EncodingType *encodingTypes;
            uint32_t encodingTypeSize = 0;
            result = OH_AudioDeviceDescriptor_GetDeviceEncodingTypes(descriptor, &encodingTypes, &encodingTypeSize);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
            AUDIO_DEBUG_LOG("DeviceEncodingTypes: %d, encodingTypeSize: %d\n", *encodingTypes, encodingTypeSize);
        }
    }
    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceEncodingTypes API via legal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceEncodingTypes_002
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceEncodingTypes interface with INPUT_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceEncodingTypes_002, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_INPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            OH_AudioStream_EncodingType *encodingTypes;
            uint32_t encodingTypeSize = 0;
            result = OH_AudioDeviceDescriptor_GetDeviceEncodingTypes(descriptor, &encodingTypes, &encodingTypeSize);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
            AUDIO_DEBUG_LOG("DeviceEncodingTypes: %d, encodingTypeSize: %d\n", *encodingTypes, encodingTypeSize);
        }
    }
    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceEncodingTypes API via legal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceEncodingTypes_003
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceEncodingTypes interface with ALL_DEVICES_FLAG.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceEncodingTypes_003, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_ALL;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            OH_AudioStream_EncodingType *encodingTypes;
            uint32_t encodingTypeSize = 0;
            result = OH_AudioDeviceDescriptor_GetDeviceEncodingTypes(descriptor, &encodingTypes, &encodingTypeSize);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
            AUDIO_DEBUG_LOG("DeviceEncodingTypes: %d, encodingTypeSize: %d\n", *encodingTypes, encodingTypeSize);
        }
    }
    result = OH_AudioRoutingManager_ReleaseDevices(audioRoutingManager, array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceEncodingTypes API via illegal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceEncodingTypes_004
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceEncodingTypes interface with nullptr encodingType.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceEncodingTypes_004, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_OUTPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            uint32_t encodingTypeSize = 0;
            result = OH_AudioDeviceDescriptor_GetDeviceEncodingTypes(descriptor, nullptr, &encodingTypeSize);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
        }
    }
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceEncodingTypes API via illegal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceEncodingTypes_005
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceEncodingTypes interface with nullptr size.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceEncodingTypes_005, TestSize.Level0)
{
    OH_AudioRoutingManager *audioRoutingManager = nullptr;
    OH_AudioCommon_Result result = OH_AudioManager_GetAudioRoutingManager(&audioRoutingManager);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(audioRoutingManager, nullptr);
    OH_AudioDevice_Flag deviceFlag = AUDIO_DEVICE_FLAG_OUTPUT;
    OH_AudioDeviceDescriptorArray *array = nullptr;
    result = OH_AudioRoutingManager_GetDevices(audioRoutingManager, deviceFlag, &array);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_SUCCESS);
    EXPECT_NE(array, nullptr);

    if (array) {
        int size = array->size;
        for (int i = 0; i < size; i++) {
            OH_AudioDeviceDescriptor *descriptor = array->descriptors[i];
            EXPECT_NE(descriptor, nullptr);
            OH_AudioStream_EncodingType *encodingTypes;
            result = OH_AudioDeviceDescriptor_GetDeviceEncodingTypes(descriptor, &encodingTypes, nullptr);
            EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
        }
    }
}

/**
* @tc.name  : Test OH_AudioDeviceDescriptor_GetDeviceEncodingTypes API via illegal state.
* @tc.number: OH_AudioDeviceDescriptor_GetDeviceEncodingTypes_006
* @tc.desc  : Test OH_AudioDeviceDescriptor_GetDeviceEncodingTypes interface with nullptr descriptor.
* Returns true if result is successful.
*/
HWTEST(OHAudioDeviceChangeUnitTest, OH_AudioDeviceDescriptor_GetDeviceEncodingTypes_006, TestSize.Level0)
{
    OH_AudioDeviceDescriptor *descriptor = nullptr;
    OH_AudioStream_EncodingType *encodingTypes;
    uint32_t encodingTypeSize = 0;
    OH_AudioCommon_Result result =
        OH_AudioDeviceDescriptor_GetDeviceEncodingTypes(descriptor, &encodingTypes, &encodingTypeSize);
    EXPECT_EQ(result, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
}

} // namespace AudioStandard
} // namespace OHOS