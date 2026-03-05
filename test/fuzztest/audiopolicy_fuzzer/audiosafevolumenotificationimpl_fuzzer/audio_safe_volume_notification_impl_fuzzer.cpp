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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include "audio_log.h"
#include "audio_safe_volume_notification_impl.h"
#include "../../fuzz_utils.h"
#include <fuzzer/FuzzedDataProvider.h>

using namespace std;

namespace OHOS {
namespace AudioStandard {

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
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

void AudioSafeVolumeNotificationImplGetSystemStringByNameFuzzTest(FuzzedDataProvider& fdp)
{
    AudioSafeVolumeNotificationImpl impl;
    std::string name = "test";
    impl.GetSystemStringByName(name);
}

void AudioSafeVolumeNotificationImplSetTitleAndTextFuzzTest(FuzzedDataProvider& fdp)
{
    vector<int32_t> uidList = {
        RESTORE_VOLUME_NOTIFICATION_ID,
        INCREASE_VOLUME_NOTIFICATION_ID,
        SAVE_VOLUME_SYS_ABILITY_ID,
    };
    uint32_t index = GetData<uint32_t>() % uidList.size();
    int32_t notificationId = uidList[index];

    auto normal = std::make_shared<Notification::NotificationNormalContent>();
    vector<std::shared_ptr<Notification::NotificationNormalContent>> contentList = {
        normal,
        nullptr,
 	};
    uint32_t index1 = GetData<uint32_t>() % contentList.size();
    std::shared_ptr<Notification::NotificationNormalContent> content = contentList[index1];

    AudioSafeVolumeNotificationImpl impl;
    impl.SetTitleAndText(notificationId, content);
}

void AudioSafeVolumeNotificationImplGetPixelMapFuzzTest(FuzzedDataProvider& fdp)
{
    AudioSafeVolumeNotificationImpl impl;
    auto iconPixelMap = std::shared_ptr<Media::PixelMap>(reinterpret_cast<Media::PixelMap*>(0x1), [](auto) {});
    vector<std::shared_ptr<Media::PixelMap>> pixeList = {
        iconPixelMap,
        nullptr,
    };
    uint32_t index = GetData<uint32_t>() % pixeList.size();
    impl.iconPixelMap_ = pixeList[index];
    impl.GetPixelMap();
}

void AudioSafeVolumeNotificationImplGetMediaDataByNameFuzzTest(FuzzedDataProvider& fdp)
{
    AudioSafeVolumeNotificationImpl impl;
    std::string name = "test";
    size_t len = GetData<size_t>();
    std::unique_ptr<uint8_t[]> outValue = nullptr;
    uint32_t density = GetData<uint32_t>();
    impl.GetMediaDataByName(name, len, outValue, density);
}

void AudioLoudVolumeNotificationImplGetSystemStringByNameFuzzTest(FuzzedDataProvider& fdp)
{
    AudioLoudVolumeNotificationImpl impl;
    int32_t notificationId = GetData<int32_t>();
    Notification::NotificationCapsule capsule;
    impl.SetTitleAndText(notificationId, capsule);
}

void AudioLoudVolumeNotificationImplGetPixelMapFuzzTest(FuzzedDataProvider& fdp)
{
    AudioLoudVolumeNotificationImpl impl;
    auto iconPixelMap = std::shared_ptr<Media::PixelMap>(reinterpret_cast<Media::PixelMap*>(0x1), [](auto) {});
    vector<std::shared_ptr<Media::PixelMap>> pixeList = {
        iconPixelMap,
        nullptr,
    };
    uint32_t index = GetData<uint32_t>() % pixeList.size();
    impl.iconPixelMap_ = pixeList[index];
    impl.GetPixelMap();
}

void  AudioLoudVolumeNotificationImplPublishLoudVolumeNotificationFuzzTest(FuzzedDataProvider& fdp)
{
    AudioLoudVolumeNotificationImpl impl;
    int32_t notificationId = GetData<int32_t>();
    impl.PublishLoudVolumeNotification(notificationId);
}

void AudioLoudVolumeNotificationImplGetMediaDataByNameFuzzTest(FuzzedDataProvider& fdp)
{
    AudioLoudVolumeNotificationImpl impl;
    std::string name = "test";
    size_t len = GetData<size_t>();
    std::unique_ptr<uint8_t[]> outValue = nullptr;
    uint32_t density = GetData<uint32_t>();
    impl.GetMediaDataByName(name, len, outValue, density);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    AudioSafeVolumeNotificationImplGetSystemStringByNameFuzzTest,
    AudioSafeVolumeNotificationImplSetTitleAndTextFuzzTest,
    AudioSafeVolumeNotificationImplGetPixelMapFuzzTest,
    AudioSafeVolumeNotificationImplGetMediaDataByNameFuzzTest,
    AudioLoudVolumeNotificationImplGetSystemStringByNameFuzzTest,
    AudioLoudVolumeNotificationImplGetPixelMapFuzzTest,
    AudioLoudVolumeNotificationImplPublishLoudVolumeNotificationFuzzTest,
    AudioLoudVolumeNotificationImplGetMediaDataByNameFuzzTest,
    });
    func(fdp);
};
void Init()
{
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
extern "C" int LLVMFuzzerInitialize(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::Init();
    return 0;
}