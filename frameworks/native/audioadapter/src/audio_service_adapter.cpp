/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioServiceAdapter"
#endif

#include "pro_audio_service_adapter_impl.h"
#include <sstream>
#include <thread>

#include "audio_errors.h"
#include "audio_common_log.h"
#include "audio_info.h"
#include "audio_utils.h"
#include <set>
#include <unordered_map>
#ifdef SUPPORT_OLD_ENGINE
#include "pulse_audio_service_adapter_impl.h"
#endif // SUPPORT_OLD_ENGINE
using namespace std;

namespace OHOS {
namespace AudioStandard {

AudioServiceAdapter::~AudioServiceAdapter() = default;

// LCOV_EXCL_START
std::shared_ptr<AudioServiceAdapter> AudioServiceAdapter::CreateAudioAdapter(
    std::unique_ptr<AudioServiceAdapterCallback> cb, bool isAudioEngine)
{
    CHECK_AND_RETURN_RET_LOG(cb != nullptr, nullptr, "CreateAudioAdapter cb is nullptr!");
    AUDIO_INFO_LOG("CreateAudioAdapter");
#ifdef SUPPORT_OLD_ENGINE
    int32_t engineFlag = GetEngineFlag();
    if (engineFlag == 1 || isAudioEngine) {
        return make_shared<ProAudioServiceAdapterImpl>(cb);
    } else {
        return make_shared<PulseAudioServiceAdapterImpl>(cb);
    }
#else
    return make_shared<ProAudioServiceAdapterImpl>(cb);
#endif
}
// LCOV_EXCL_STOP
} // namespace AudioStandard
} // namespace OHOS