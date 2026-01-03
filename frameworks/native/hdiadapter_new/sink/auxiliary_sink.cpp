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
#define LOG_TAG "AuxiliarySink"
#endif

#include "sink/fast_audio_render_sink.h"
#include <climits>
#include "audio_hdi_log.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "audio_performance_monitor.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include "audio_stream_enum.h"

namespace OHOS {
namespace AudioStandard {
AuxiliarySink::~AuxiliarySink()
{
    AUDIO_INFO_LOG("in");
    if (sinkInited_) {
        DeInit();
    }
}

} // namespace AudioStandard
} // namespace OHOS
