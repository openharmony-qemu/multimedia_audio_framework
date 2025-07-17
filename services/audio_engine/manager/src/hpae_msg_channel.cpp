/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "HpaeMsgChannel"
#endif

#include "hpae_msg_channel.h"
#include "audio_engine_log.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
template <typename... Args>
void CallbackSender::TriggerCallback(HpaeMsgCode cmdID, Args &&...args)
{
    if (auto callback = weakCallback_.lock()) {
        // pack the arguments into a tuple
        auto packed = std::make_tuple(std::forward<Args>(args)...);
        callback->Invoke(cmdID, packed);
    } else {
        AUDIO_ERR_LOG("Hpae TriggerCallback callback is null");
    }
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS