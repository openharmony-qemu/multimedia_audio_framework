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
#ifndef AUDIO_CAPTURER_TYPES_H
#define AUDIO_CAPTURER_TYPES_H

#ifdef __MUSL__
#include <stdint.h>
#endif // __MUSL__

#include <cmath>
#include <limits>
#include <string>
#include <vector>
#include <array>
#include <unistd.h>
#include <unordered_map>
#include <parcel.h>
#include <audio_source_type.h>
#include <audio_device_info.h>
#include "audio_pipe_types.h"
#include <audio_interrupt_info.h>
#include <audio_session_info.h>
#include <audio_stream_info.h>
#include <audio_asr.h>
#include "audio_shared_memory.h"

namespace OHOS {
namespace AudioStandard {
enum AudioWakeupStageCode {
    STAGE_FWK_CREATE_ENTER  = 0x0100,    /* fwk stage start */
    STAGE_FWK_CREATE_EXIT   = 0x015F,
    STAGE_FWK_START_ENTER   = 0x0160,
    STAGE_FWK_START_EXIT    = 0x01AF,    /* fwk stage end */
    STAGE_FWK_END           = 0x01FF,
};

enum AudioWakeupErrorCode {
    WAKEUP_TRACK_FWK_NO_ERROR = 0x1000,
    WAKEUP_TRACK_FWK_CREATE_ERROR,
    WAKEUP_TRACK_FWK_START_ERROR
};

enum AudioWakeupResult {
    WAKEUP_RESULT_DEFAULT,
    WAKEUP_RESULT_SUCC,
    WAKEUP_RESULT_FAIL
};

struct AudioWakeupTrackInfo : public Parcelable  {
    enum AudioWakeupStageCode stage;
    enum AudioWakeupResult result;
    enum AudioWakeupErrorCode errCode;

    bool Marshalling(Parcel &parcel) const override
    {
        return parcel.WriteInt32(stage) &&
               parcel.WriteInt32(result) &&
               parcel.WriteInt32(errCode);
    }

    static AudioWakeupTrackInfo *Unmarshalling(Parcel &parcel)
    {
        auto audioWakeupTrackInfo = new(std::nothrow) AudioWakeupTrackInfo();
        if (audioWakeupTrackInfo == nullptr) {
            return nullptr;
        }

        audioWakeupTrackInfo->stage = static_cast<AudioWakeupStageCode>(parcel.ReadInt32());
        audioWakeupTrackInfo->result = static_cast<AudioWakeupResult>(parcel.ReadInt32());
        audioWakeupTrackInfo->errCode = static_cast<AudioWakeupErrorCode>(parcel.ReadInt32());
        return audioWakeupTrackInfo;
    }
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_CAPTURER_TYPES_H