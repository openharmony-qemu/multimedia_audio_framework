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

#ifndef LOG_TAG
#define LOG_TAG "SafeVolumeManager"
#endif

#include "safe_volume_manager.h"

#include <dlfcn.h>
#include "audio_policy_log.h"
#include "audio_safe_volume_notification.h"
#include "audio_active_device.h"
#include "audio_policy_manager_factory.h"

namespace OHOS {
namespace AudioStandard {

SafeVolumeManager::SafeVolumeManager()
    : audioActiveDevice_(AudioActiveDevice::GetInstance()),
      audioPolicyManager_(AudioPolicyManagerFactory::GetAudioPolicyManager())
{
}

bool SafeVolumeManager::DeviceIsSupportSafeVolume()
{
    DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
    DeviceCategory curOutputDeviceCategory = audioPolicyManager_.GetCurrentOutputDeviceCategory();
    switch (curOutputDeviceType) {
        case DEVICE_TYPE_BLUETOOTH_A2DP:
        case DEVICE_TYPE_BLUETOOTH_SCO:
        case DEVICE_TYPE_NEARLINK:
            if (curOutputDeviceCategory != BT_SOUNDBOX &&
                curOutputDeviceCategory != BT_CAR) {
                return true;
            }
            [[fallthrough]];
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_WIRED_HEADPHONES:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
            return true;
        default:
            AUDIO_INFO_LOG("current device unsupport safe volume:%{public}d", curOutputDeviceType);
            return false;
    }
}

bool SafeVolumeManager::IsWiredHeadSet(const DeviceType &deviceType)
{
    switch (deviceType) {
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_WIRED_HEADPHONES:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
            return true;
        default:
            return false;
    }
}

bool SafeVolumeManager::IsBlueTooth(const DeviceType &deviceType)
{
    if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP || deviceType == DEVICE_TYPE_BLUETOOTH_SCO) {
        if (audioPolicyManager_.GetCurrentOutputDeviceCategory() != BT_CAR &&
            audioPolicyManager_.GetCurrentOutputDeviceCategory() != BT_SOUNDBOX) {
            return true;
        }
    }
    return false;
}

bool SafeVolumeManager::IsNearLink(const DeviceType &deviceType)
{
    switch (deviceType) {
        case DEVICE_TYPE_NEARLINK:
            return true;
        default:
            return false;
    }
}

void SafeVolumeManager::PublishSafeVolumeNotification(int32_t notificationId)
{
    void *libHandle = dlopen(LIB_NAME, RTLD_LAZY);
    if (libHandle == nullptr) {
        AUDIO_ERR_LOG("dlopen failed %{public}s", __func__);
        return;
    }
    CreateSafeVolumeNotification *createSafeVolumeNotificationImpl =
        reinterpret_cast<CreateSafeVolumeNotification*>(dlsym(libHandle, "CreateSafeVolumeNotificationImpl"));
    if (createSafeVolumeNotificationImpl == nullptr) {
        AUDIO_ERR_LOG("createSafeVolumeNotificationImpl failed %{public}s", __func__);
#ifndef TEST_COVERAGE
        dlclose(libHandle);
#endif
        return;
    }
    AudioSafeVolumeNotification *audioSafeVolumeNotificationImpl = createSafeVolumeNotificationImpl();
    if (audioSafeVolumeNotificationImpl == nullptr) {
        AUDIO_ERR_LOG("audioSafeVolumeNotificationImpl is nullptr %{public}s", __func__);
#ifndef TEST_COVERAGE
        dlclose(libHandle);
#endif
        return;
    }
    audioSafeVolumeNotificationImpl->PublishSafeVolumeNotification(notificationId);
    delete audioSafeVolumeNotificationImpl;
#ifndef TEST_COVERAGE
    dlclose(libHandle);
#endif
}

void SafeVolumeManager::CancelSafeVolumeNotification(int32_t notificationId)
{
    void *libHandle = dlopen(LIB_NAME, RTLD_LAZY);
    if (libHandle == nullptr) {
        AUDIO_ERR_LOG("dlopen failed %{public}s", __func__);
        return;
    }
    CreateSafeVolumeNotification *createSafeVolumeNotificationImpl =
        reinterpret_cast<CreateSafeVolumeNotification*>(dlsym(libHandle, "CreateSafeVolumeNotificationImpl"));
    if (createSafeVolumeNotificationImpl == nullptr) {
        AUDIO_ERR_LOG("createSafeVolumeNotificationImpl failed %{public}s", __func__);
#ifndef TEST_COVERAGE
        dlclose(libHandle);
#endif
        return;
    }
    AudioSafeVolumeNotification *audioSafeVolumeNotificationImpl = createSafeVolumeNotificationImpl();
    if (audioSafeVolumeNotificationImpl == nullptr) {
        AUDIO_ERR_LOG("audioSafeVolumeNotificationImpl is nullptr %{public}s", __func__);
#ifndef TEST_COVERAGE
        dlclose(libHandle);
#endif
        return;
    }
    audioSafeVolumeNotificationImpl->CancelSafeVolumeNotification(notificationId);
    delete audioSafeVolumeNotificationImpl;
#ifndef TEST_COVERAGE
    dlclose(libHandle);
#endif
}

void SafeVolumeManager::HandleLegacyHighVolumeNotification(LegacyHighVolumeAction action)
{
    void *libHandle = dlopen(LIB_NAME, RTLD_LAZY);
    if (libHandle == nullptr) {
        AUDIO_ERR_LOG("dlopen failed %{public}s", __func__);
        return;
    }
    CreateLegacyHighVolumeNotification *createLegacyHighVolumeNotificationImpl =
        reinterpret_cast<CreateLegacyHighVolumeNotification*>(dlsym(libHandle,
        "CreateLegacyHighVolumeNotificationImpl"));
    if (createLegacyHighVolumeNotificationImpl == nullptr) {
        AUDIO_ERR_LOG("createLegacyHighVolumeNotificationImpl failed %{public}s", __func__);
#ifndef TEST_COVERAGE
        dlclose(libHandle);
#endif
        return;
    }
    AudioLegacyHighVolumeNotification *audioLegacyHighVolumeNotificationImpl = createLegacyHighVolumeNotificationImpl();
    if (audioLegacyHighVolumeNotificationImpl == nullptr) {
        AUDIO_ERR_LOG("audioLegacyHighVolumeNotificationImpl is nullptr %{public}s", __func__);
#ifndef TEST_COVERAGE
        dlclose(libHandle);
#endif
        return;
    }

    if (action == LegacyHighVolumeAction::PUBLISH) {
        audioLegacyHighVolumeNotificationImpl->PublishLegacyHighVolumeNotification();
    } else {
        audioLegacyHighVolumeNotificationImpl->CancelLegacyHighVolumeNotification();
    }

    delete audioLegacyHighVolumeNotificationImpl;
#ifndef TEST_COVERAGE
    dlclose(libHandle);
#endif
}

void SafeVolumeManager::PublishLegacyHighVolumeNotification()
{
    HandleLegacyHighVolumeNotification(LegacyHighVolumeAction::PUBLISH);
}

void SafeVolumeManager::CancelLegacyHighVolumeNotification()
{
    HandleLegacyHighVolumeNotification(LegacyHighVolumeAction::CANCEL);
}

}  // namespace AudioStandard
}  // namespace OHOS