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
#ifndef ST_AUDIO_CORE_SERVICE_UTILS_H
#define ST_AUDIO_CORE_SERVICE_UTILS_H

#include <list>
#include <string>
#include <mutex>

#include "audio_log.h"
#include "audio_errors.h"
#include "audio_device_descriptor.h"
#include "audio_info.h"
#include "audio_stream_descriptor.h"
#include "audio_policy_utils.h"
#include "audio_module_info.h"

namespace OHOS {
namespace AudioStandard {

class AudioCoreServiceUtils {
public:
    static bool IsDualStreamWhenRingDual(AudioStreamType streamType);
    static bool IsOverRunPlayback(AudioMode &mode, RendererState rendererState);
    static bool IsRingDualToneOnPrimarySpeaker(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs,
        const int32_t sessionId);
    static bool NeedDualHalToneInStatus(AudioRingerMode mode, StreamUsage usage,
        bool isPcVolumeEnable, bool isMusicMute);
    static bool IsAlarmOnActive(StreamUsage usage, bool isAlarmActive);
};
}
}
#endif