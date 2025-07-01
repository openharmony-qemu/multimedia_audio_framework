/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "multimedia_audio_stream_manager_callback.h"

#include "multimedia_audio_common.h"
#include "multimedia_audio_error.h"

namespace OHOS {
namespace AudioStandard {
void CjAudioCapturerStateChangeCallback::RegisterFunc(std::function<void(CArrAudioCapturerChangeInfo)> cjCallback)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    func_ = cjCallback;
}

void CjAudioCapturerStateChangeCallback::OnCapturerStateChange(
    const std::vector<std::shared_ptr<AudioCapturerChangeInfo>>& audioCapturerChangeInfos)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    if (func_ == nullptr) {
        return;
    }
    CArrAudioCapturerChangeInfo arrInfo {};
    arrInfo.size = static_cast<int64_t>(audioCapturerChangeInfos.size());
    int32_t mallocSize = static_cast<int32_t>(sizeof(CAudioCapturerChangeInfo)) * static_cast<int32_t>(arrInfo.size);
    if (mallocSize <= 0 || mallocSize > static_cast<int32_t>(sizeof(CAudioCapturerChangeInfo) * MAX_MEM_MALLOC_SIZE)) {
        return;
    }
    auto head = static_cast<CAudioCapturerChangeInfo*>(malloc(mallocSize));
    if (head == nullptr) {
        return;
    }
    int32_t errorCode = SUCCESS_CODE;
    arrInfo.head = head;
    if (memset_s(head, mallocSize, 0, mallocSize) != EOK) {
        free(head);
        return;
    }
    for (int64_t i = 0; i < arrInfo.size; i++) {
        Convert2CAudioCapturerChangeInfo(head[i], *(audioCapturerChangeInfos[i]), &errorCode);
        if (errorCode != SUCCESS_CODE) {
            FreeCArrAudioCapturerChangeInfo(arrInfo);
            return;
        }
    }
    func_(arrInfo);
    FreeCArrAudioCapturerChangeInfo(arrInfo);
}

void CjAudioRendererStateChangeCallback::RegisterFunc(std::function<void(CArrAudioRendererChangeInfo)> cjCallback)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    func_ = cjCallback;
}

void CjAudioRendererStateChangeCallback::OnRendererStateChange(
    const std::vector<std::shared_ptr<AudioRendererChangeInfo>>& audioRendererChangeInfos)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    if (func_ == nullptr) {
        return;
    }
    CArrAudioRendererChangeInfo arrInfo {};
    arrInfo.size = static_cast<int64_t>(audioRendererChangeInfos.size());
    int32_t mallocSize = static_cast<int32_t>(sizeof(CAudioRendererChangeInfo)) * static_cast<int32_t>(arrInfo.size);
    if (mallocSize <= 0 || mallocSize > static_cast<int32_t>(sizeof(CAudioRendererChangeInfo) * MAX_MEM_MALLOC_SIZE)) {
        return;
    }
    auto head = static_cast<CAudioRendererChangeInfo*>(malloc(mallocSize));
    if (head == nullptr) {
        return;
    }
    int32_t errorCode = SUCCESS_CODE;
    arrInfo.head = head;
    if (memset_s(head, mallocSize, 0, mallocSize) != EOK) {
        free(head);
        return;
    }
    for (int64_t i = 0; i < arrInfo.size; i++) {
        Convert2CAudioRendererChangeInfo(head[i], *(audioRendererChangeInfos[i]), &errorCode);
        if (errorCode != SUCCESS_CODE) {
            FreeCArrAudioRendererChangeInfo(arrInfo);
            return;
        }
    }
    func_(arrInfo);
    FreeCArrAudioRendererChangeInfo(arrInfo);
}
} // namespace AudioStandard
} // namespace OHOS
