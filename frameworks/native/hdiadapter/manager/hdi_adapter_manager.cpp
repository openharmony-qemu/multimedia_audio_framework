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

#ifndef LOG_TAG
#define LOG_TAG "HdiAdapterManager"
#endif

#include "hdi_adapter_manager.h"

#include "audio_hdi_log.h"
#include "audio_errors.h"

#include "include/hdi_adapter_manager_api.h"

using namespace OHOS::AudioStandard;

// Capture handle funcs impl
int32_t CaptureHandleInit(void *capture)
{
    IAudioCapturerSource *captureSource = static_cast<IAudioCapturerSource *>(capture);
    CHECK_AND_RETURN_RET_LOG(captureSource != nullptr, ERR_INVALID_HANDLE, "wrong capture");

    return captureSource->InitWithoutAttr();
}

int32_t CaptureHandleDeinit(void *capture)
{
    IAudioCapturerSource *captureSource = static_cast<IAudioCapturerSource *>(capture);
    CHECK_AND_RETURN_RET_LOG(captureSource != nullptr, ERR_INVALID_HANDLE, "wrong capture");

    captureSource->DeInit();

    return SUCCESS;
}

int32_t CaptureHandleStart(void *capture)
{
    IAudioCapturerSource *captureSource = static_cast<IAudioCapturerSource *>(capture);
    CHECK_AND_RETURN_RET_LOG(captureSource != nullptr, ERR_INVALID_HANDLE, "wrong capture");

    return captureSource->Start();
}

int32_t CaptureHandleStop(void *capture)
{
    IAudioCapturerSource *captureSource = static_cast<IAudioCapturerSource *>(capture);
    CHECK_AND_RETURN_RET_LOG(captureSource != nullptr, ERR_INVALID_HANDLE, "wrong capture");

    return captureSource->Stop();
}

int32_t CaptureHandleCaptureFrame(void *capture,
    char *frame, uint64_t requestBytes, uint64_t *replyBytes)
{
    IAudioCapturerSource *captureSource = static_cast<IAudioCapturerSource *>(capture);
    CHECK_AND_RETURN_RET_LOG(captureSource != nullptr, ERR_INVALID_HANDLE, "wrong capture");

    return captureSource->CaptureFrame(frame, requestBytes, *replyBytes);
}

int32_t CaptureHandleCaptureFrameWithEc(void *capture,
    FrameDesc *fdesc, uint64_t *replyBytes,
    FrameDesc *fdescEc, uint64_t *replyBytesEc)
{
    IAudioCapturerSource *captureSource = static_cast<IAudioCapturerSource *>(capture);
    CHECK_AND_RETURN_RET_LOG(captureSource != nullptr, ERR_INVALID_HANDLE, "wrong capture");

    return captureSource->CaptureFrameWithEc(
        fdesc, *replyBytes,
        fdescEc, *replyBytesEc);
}

// public api impl
int32_t CreateCaptureHandle(HdiCaptureHandle **handle, CaptureAttr *attr)
{
    OHOS::AudioStandard::HdiAdapterManager *manager = OHOS::AudioStandard::HdiAdapterManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(manager != nullptr, ERR_INVALID_HANDLE, "hdi adapter manager is null");

    struct HdiCaptureHandle *captureHandle =
        reinterpret_cast<struct HdiCaptureHandle *>(calloc(1, sizeof(*captureHandle)));
    if (captureHandle == nullptr) {
        AUDIO_ERR_LOG("allocate handle failed");
        return ERR_INVALID_HANDLE;
    }

    IAudioCapturerSource *capture = manager->CreateCapture(attr);
    captureHandle->capture = reinterpret_cast<void *>(capture);

    captureHandle->Init = CaptureHandleInit;
    captureHandle->Deinit = CaptureHandleDeinit;
    captureHandle->Start = CaptureHandleStart;
    captureHandle->Stop = CaptureHandleStop;
    captureHandle->CaptureFrame = CaptureHandleCaptureFrame;
    captureHandle->CaptureFrameWithEc = CaptureHandleCaptureFrameWithEc;

    *handle = captureHandle;

    return SUCCESS;
}

void ReleaseCaptureHandle(HdiCaptureHandle *handle)
{
    if (handle != nullptr) {
        OHOS::AudioStandard::HdiAdapterManager *manager = OHOS::AudioStandard::HdiAdapterManager::GetInstance();
        CHECK_AND_RETURN_LOG(manager != nullptr, "hdi adapter manager is null");

        // delete instance saved in handle
        IAudioCapturerSource *capture = reinterpret_cast<IAudioCapturerSource *>(handle->capture);
        manager->ReleaseCapture(capture);

        free(handle);
    }
}

namespace OHOS {
namespace AudioStandard {

HdiAdapterManager::HdiAdapterManager()
{
    // nothing to do now
}

HdiAdapterManager::~HdiAdapterManager()
{
    // nothing to do now
}

IAudioCapturerSource *HdiAdapterManager::CreateCapture(CaptureAttr *attr)
{
    IAudioCapturerSource *capture = IAudioCapturerSource::Create(attr);
    return capture;
}

void HdiAdapterManager::ReleaseCapture(IAudioCapturerSource *capture)
{
    if (capture != nullptr) {
        delete capture;
    }
}

HdiAdapterManager *HdiAdapterManager::GetInstance()
{
    static HdiAdapterManager manager;
    return &manager;
}

} // namespace AudioStandard
} // namespace OHOS