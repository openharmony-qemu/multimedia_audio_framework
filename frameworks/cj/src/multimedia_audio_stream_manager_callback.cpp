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
    func_ = cjCallback;
}

void CjAudioCapturerStateChangeCallback::OnCapturerStateChange(
    const std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    if (func_ == nullptr) {
        return;
    }
    CArrAudioCapturerChangeInfo arrInfo;
    arrInfo.size = static_cast<int64_t>(audioCapturerChangeInfos.size());
    auto head = static_cast<CAudioCapturerChangeInfo *>(
        malloc(sizeof(CAudioCapturerChangeInfo) * (arrInfo.size)));
    if (head == nullptr) {
        return;
    }
    int32_t errorCode = SUCCESS_CODE;
    arrInfo.head = head;
    if (memset_s(head, arrInfo.size, 0, arrInfo.size) != EOK) {
        FreeCArrAudioCapturerChangeInfo(arrInfo);
        return;
    }
    for (int32_t i = 0; i < static_cast<int32_t>(arrInfo.size); i++) {
        Convert2CAudioCapturerChangeInfo(head[i], *(audioCapturerChangeInfos[i]), &errorCode);
    }
    if (errorCode != SUCCESS_CODE) {
        FreeCArrAudioCapturerChangeInfo(arrInfo);
        return;
    }
    func_(arrInfo);
    FreeCArrAudioCapturerChangeInfo(arrInfo);
}

void CjAudioRendererStateChangeCallback::RegisterFunc(std::function<void(CArrAudioRendererChangeInfo)> cjCallback)
{
    func_ = cjCallback;
}

void CjAudioRendererStateChangeCallback::OnRendererStateChange(
    const std::vector<std::shared_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    CArrAudioRendererChangeInfo arrInfo;
    arrInfo.size = static_cast<int64_t>(audioRendererChangeInfos.size());
    auto head = static_cast<CAudioRendererChangeInfo *>(
        malloc(sizeof(CAudioRendererChangeInfo) * audioRendererChangeInfos.size()));
    if (head == nullptr) {
        return;
    }
    int32_t errorCode = SUCCESS_CODE;
    arrInfo.head = head;
    if (memset_s(head, arrInfo.size, 0, arrInfo.size) != EOK) {
        FreeCArrAudioRendererChangeInfo(arrInfo);
        errorCode = CJ_ERR_SYSTEM;
        return;
    }
    for (int32_t i = 0; i < static_cast<int32_t>(audioRendererChangeInfos.size()); i++) {
        Convert2CAudioRendererChangeInfo(head[i], *(audioRendererChangeInfos[i]), &errorCode);
    }
    if (errorCode != SUCCESS_CODE) {
        FreeCArrAudioRendererChangeInfo(arrInfo);
        errorCode = CJ_ERR_SYSTEM;
        return;
    }
    func_(arrInfo);
    FreeCArrAudioRendererChangeInfo(arrInfo);
}
} // namespace AudioStandard
} // namespace OHOS
