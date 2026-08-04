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

#ifndef OH_AUDIO_ACCESSORY_INPUT_STREAM_MANAGER_H
#define OH_AUDIO_ACCESSORY_INPUT_STREAM_MANAGER_H

#include "audio_accessory_input_stream.h"

namespace OHOS {
namespace AudioStandard {

class OHAudioAccessoryInputStreamManager {
public:
    static OH_AudioCommon_Result RegisterStartCallback(OH_AudioAccessoryInputStream *stream,
        OH_AudioAccessoryInputStream_StartCallback callback);
    static OH_AudioCommon_Result RegisterStopCallback(OH_AudioAccessoryInputStream *stream,
        OH_AudioAccessoryInputStream_StopCallback callback);
    static OH_AudioCommon_Result RegisterReleaseCallback(OH_AudioAccessoryInputStream *stream,
        OH_AudioAccessoryInputStream_ReleaseCallback callback);
    static OH_AudioCommon_Result RegisterLatencyCallback(OH_AudioAccessoryInputStream *stream,
        OH_AudioAccessoryInputStream_GetLatencyCallback callback);
    static OH_AudioCommon_Result RegisterFramePositionCallback(OH_AudioAccessoryInputStream *stream,
        OH_AudioAccessoryInputStream_GetFramePositionCallback callback);
    static OH_AudioCommon_Result Write(OH_AudioAccessoryInputStream *stream, const uint8_t *data, uint32_t dataSize);
    static OH_AudioCommon_Result GetWritableSize(OH_AudioAccessoryInputStream *stream, uint32_t *writableSize);
};

} // namespace AudioStandard
} // namespace OHOS

#endif // OH_AUDIO_ACCESSORY_INPUT_STREAM_MANAGER_H
