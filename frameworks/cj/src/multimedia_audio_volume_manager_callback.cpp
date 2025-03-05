/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

#include "multimedia_audio_volume_manager_callback.h"
#include "multimedia_audio_common.h"

namespace OHOS {
namespace AudioStandard {

void CjVolumeKeyEventCallback::RegisterFunc(std::function<void(CVolumeEvent)> cjCallback)
{
    func_ = cjCallback;
}

void CjVolumeKeyEventCallback::OnVolumeKeyEvent(VolumeEvent volumeEvent)
{
    std::lock_guard<std::mutex> lock(cbMutex_);
    CVolumeEvent cVol{};
    cVol.volume = volumeEvent.volume;
    cVol.volumeType = static_cast<int32_t>(volumeEvent.volumeType);
    cVol.updateUi = volumeEvent.updateUi;
    func_(cVol);
}

} // namespace AudioStandard
} // namespace OHOS
