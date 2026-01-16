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

#ifndef AUDIO_TYPE_CONVERT_H
#define AUDIO_TYPE_CONVERT_H

#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {
class AudioTypeConvert {
public:
    /**
     * @brief Get Pin Value From Type
     *
     * @param deviceType deviceType
     * @param deviceRole deviceRole
     * @return Returns Enumerate AudioPin
     * @since 8
     */
    static AudioPin GetPinValueFromType(DeviceType deviceType, DeviceRole deviceRole);

    /**
     * @brief Get type Value From Pin
     *
     * @param pin AudioPin
     * @return Returns Enumerate DeviceType
     * @since 8
     */
    static DeviceType GetTypeValueFromPin(AudioPin pin);

    /**
     * @brief Get audio streamType.
     *
     * @param contentType Enumerates the audio content type.
     * @param streamUsage Enumerates the stream usage.
     * @return Returns Audio streamType.
     * @since 8
     */
    static AudioStreamType GetStreamType(ContentType contentType, StreamUsage streamUsage);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_TYPE_CONVERT_H
