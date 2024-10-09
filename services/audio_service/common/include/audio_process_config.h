/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef AUDIO_PROCESS_CONFIG_H
#define AUDIO_PROCESS_CONFIG_H

#include <string>

#include "message_parcel.h"

#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {
class ProcessConfig {
public:
    static int32_t WriteInnerCapConfigToParcel(const AudioPlaybackCaptureConfig &config, MessageParcel &parcel);

    static int32_t ReadInnerCapConfigFromParcel(AudioPlaybackCaptureConfig &config, MessageParcel &parcel);

    static std::string DumpInnerCapConfig(const AudioPlaybackCaptureConfig &config);

    static int32_t WriteConfigToParcel(const AudioProcessConfig &config, MessageParcel &parcel);

    static int32_t ReadConfigFromParcel(AudioProcessConfig &config, MessageParcel &parcel);

    static std::string DumpProcessConfig(const AudioProcessConfig &config);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_PROCESS_CONFIG_H
