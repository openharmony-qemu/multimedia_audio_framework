/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "audio_log.h"
#include "audio_usr_select_manager.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
static int32_t NUM_2 = 2;
static int32_t NUM_5 = 5;
static int32_t NUM_10 = 10;
typedef void (*TestPtr)();


template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (g_dataSize < g_pos) {
        return object;
    }
    if (RAW_DATA == nullptr || objectSize > g_dataSize - g_pos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, RAW_DATA + g_pos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_pos += objectSize;
    return object;
}

void AudioUsrSelectManagerSelectInputDeviceByUidFuzzTest()
{
    int32_t uid = GetData<int32_t>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    deviceDescriptor->deviceId_ = GetData<int32_t>() % NUM_2;

    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceId_ = GetData<int32_t>() % NUM_2;
    AudioDeviceManager::GetAudioDeviceManager().connectedDevices_.push_back(desc);

    AudioUsrSelectManager::GetAudioUsrSelectManager().SelectInputDeviceByUid(deviceDescriptor, uid);
}

void AudioUsrSelectManagerGetSelectedInputDeviceByUidFuzzTest()
{
    int32_t uid = GetData<int32_t>();
    AudioUsrSelectManager::GetAudioUsrSelectManager().GetSelectedInputDeviceByUid(uid);
}

void AudioUsrSelectManagerGetPreferBluetoothAndNearlinkRecordByUidFuzzTest()
{
    int32_t uid = GetData<int32_t>();
    RecordDeviceInfo recordDeviceInfo;
    recordDeviceInfo.uid_ = uid;
    AudioUsrSelectManager::GetAudioUsrSelectManager().recordDeviceInfoList_.push_back(recordDeviceInfo);
    AudioUsrSelectManager::GetAudioUsrSelectManager().GetPreferBluetoothAndNearlinkRecordByUid(uid);
}

void AudioUsrSelectManagerGetCapturerDeviceFuzzTest()
{
    int32_t uid = GetData<int32_t>();
    SourceType sourceType = GetData<SourceType>();
    RecordDeviceInfo recordDeviceInfo;
    recordDeviceInfo.uid_ = GetData<int32_t>();
    AudioUsrSelectManager::GetAudioUsrSelectManager().recordDeviceInfoList_.push_back(recordDeviceInfo);
    AudioUsrSelectManager::GetAudioUsrSelectManager().GetCapturerDevice(uid, sourceType);
}

void AudioUsrSelectManagerJudgeFinalSelectDeviceFuzzTest()
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    SourceType sourceType = GetData<SourceType>();
    BluetoothAndNearlinkPreferredRecordCategory category = GetData<BluetoothAndNearlinkPreferredRecordCategory>();
    AudioUsrSelectManager::GetAudioUsrSelectManager().JudgeFinalSelectDevice(desc, sourceType, category);
}

void AudioUsrSelectManagerGetPreferDeviceFuzzTest()
{
    int32_t index = GetData<int32_t>() % NUM_5;
    RecordDeviceInfo recordDeviceInfo;
    recordDeviceInfo.appPreferredCategory_ = BluetoothAndNearlinkPreferredRecordCategory::PREFERRED_DEFAULT;
    AudioUsrSelectManager::GetAudioUsrSelectManager().recordDeviceInfoList_.resize(NUM_10, recordDeviceInfo);
    AudioUsrSelectManager::GetAudioUsrSelectManager().GetPreferDevice(index);
}

void AudioUsrSelectManagerUpdateRecordDeviceInfoForStartInnerFuzzTest()
{
    int32_t index = GetData<int32_t>() % NUM_5;
    RecordDeviceInfo info;
    RecordDeviceInfo recordDeviceInfo;
    AudioUsrSelectManager::GetAudioUsrSelectManager().recordDeviceInfoList_.resize(NUM_10, recordDeviceInfo);
    AudioUsrSelectManager::GetAudioUsrSelectManager().mcSelectedFlag_ = GetData<bool>();
    AudioUsrSelectManager::GetAudioUsrSelectManager().UpdateRecordDeviceInfoForStartInner(index, info);
}

void AudioUsrSelectManagerUpdateRecordDeviceInfoForStopInnerFuzzTest()
{
    int32_t index = GetData<int32_t>() % NUM_5;
    RecordDeviceInfo recordDeviceInfo;
    recordDeviceInfo.appPreferredCategory_ = GetData<BluetoothAndNearlinkPreferredRecordCategory>();
    recordDeviceInfo.selectedDevice_->deviceType_ = GetData<DeviceType>();
        AudioUsrSelectManager::GetAudioUsrSelectManager().recordDeviceInfoList_.resize(NUM_10, recordDeviceInfo);
    AudioUsrSelectManager::GetAudioUsrSelectManager().mcSelectedFlag_ = true;
    AudioUsrSelectManager::GetAudioUsrSelectManager().UpdateRecordDeviceInfoForStopInner(index);
}

void AudioUsrSelectManagerUpdateAppIsBackStateFuzzTest()
{
    int32_t uid = GetData<int32_t>() % NUM_5;
    AppIsBackState appState = GetData<AppIsBackState>();
    AudioUsrSelectManager::GetAudioUsrSelectManager().UpdateAppIsBackState(uid, appState);
}

TestPtr g_testPtrs[] = {
    AudioUsrSelectManagerSelectInputDeviceByUidFuzzTest,
    AudioUsrSelectManagerGetSelectedInputDeviceByUidFuzzTest,
    AudioUsrSelectManagerGetPreferBluetoothAndNearlinkRecordByUidFuzzTest,
    AudioUsrSelectManagerGetCapturerDeviceFuzzTest,
    AudioUsrSelectManagerJudgeFinalSelectDeviceFuzzTest,
    AudioUsrSelectManagerGetPreferDeviceFuzzTest,
    AudioUsrSelectManagerUpdateRecordDeviceInfoForStartInnerFuzzTest,
    AudioUsrSelectManagerUpdateRecordDeviceInfoForStopInnerFuzzTest,
    AudioUsrSelectManagerUpdateAppIsBackStateFuzzTest,
};

void FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return;
    }

    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;

    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testPtrs);
    if (len > 0) {
        g_testPtrs[code % len]();
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }
    return;
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
