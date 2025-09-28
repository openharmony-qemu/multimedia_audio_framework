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
#ifndef NAPI_DFX_UTILS_H
#define NAPI_DFX_UTILS_H

#include <string>
#include <cstdint>

namespace OHOS {
namespace AudioStandard {

class NapiDfxUtils {
public:
    static void SendVolumeApiInvokeEvent(int32_t uid, const std::string &functionName, int32_t paramValue);

    struct SteamDirection {
        static constexpr bool playback = false;
        static constexpr bool capture = true;
    };
 
    struct MainThreadCallFunc {
        static constexpr uint8_t write = 0;
        static constexpr uint8_t writeCb = 1;
        static constexpr uint8_t read = 0;
        static constexpr uint8_t readCb = 1;
    };
 
    static void ReportAudioMainThreadEvent(int32_t uid, bool direction,
        uint8_t usageOrSourceType, uint8_t functionType);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // NAPI_DFX_UTILS_H
