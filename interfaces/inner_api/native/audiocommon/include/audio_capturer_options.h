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

#ifndef AUDIO_CAPTURER_OPTIONS_H
#define AUDIO_CAPTURER_OPTIONS_H

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
#include "audio_source_type.h"
#include "audio_device_info.h"
#include "audio_interrupt_info.h"
#include "audio_session_info.h"
#include "audio_stream_info.h"
#include "audio_asr.h"
#include "audio_device_descriptor.h"
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {
struct AudioCapturerOptions {
    AudioStreamInfo streamInfo;
    AudioCapturerInfo capturerInfo;
    AudioPlaybackCaptureConfig playbackCaptureConfig;
    AudioSessionStrategy strategy = { AudioConcurrencyMode::INVALID };
    AudioDeviceDescriptor preferredInputDevice;
};
}  //namespace AudioStandard
}  //namespace OHOS
#endif // AUDIO_CAPTURER_OPTIONS_H
