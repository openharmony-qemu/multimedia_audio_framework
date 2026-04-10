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
#define LOG_TAG "SafeVolumeNotificationLoader"
#endif

#include "safe_volume_notification_loader.h"

#include <dlfcn.h>
#include "audio_policy_log.h"
#include "audio_safe_volume_notification.h"

namespace OHOS {
namespace AudioStandard {

void SafeVolumeNotificationLoader::PublishSafeVolumeNotification(int32_t notificationId)
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

void SafeVolumeNotificationLoader::CancelSafeVolumeNotification(int32_t notificationId)
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

void SafeVolumeNotificationLoader::PublishLegacyHighVolumeNotification()
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
    audioLegacyHighVolumeNotificationImpl->PublishLegacyHighVolumeNotification();
    delete audioLegacyHighVolumeNotificationImpl;
#ifndef TEST_COVERAGE
    dlclose(libHandle);
#endif
}

void SafeVolumeNotificationLoader::CancelLegacyHighVolumeNotification()
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
    audioLegacyHighVolumeNotificationImpl->CancelLegacyHighVolumeNotification();
    delete audioLegacyHighVolumeNotificationImpl;
#ifndef TEST_COVERAGE
    dlclose(libHandle);
#endif
}

}  // namespace AudioStandard
}  // namespace OHOS