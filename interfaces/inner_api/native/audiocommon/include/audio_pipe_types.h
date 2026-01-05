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
#ifndef AUDIO_PIPE_TYPES_H
#define AUDIO_PIPE_TYPES_H

namespace OHOS {
namespace AudioStandard {

constexpr uint32_t PIPE_ID_INVALID = 0;

enum AudioPipeRole : int32_t {
    PIPE_ROLE_OUTPUT = 0,
    PIPE_ROLE_INPUT,
    PIPE_ROLE_NONE,
};

enum AudioPipeStatus : int32_t {
    PIPE_STATUS_OPEN = 0,
    PIPE_STATUS_CLOSE,
    PIPE_STATUS_RUNNING,
    PIPE_STATUS_STANDBY, // stop status
};

enum HdiAdapterType : uint32_t {
    HDI_ADAPTER_TYPE_UNKNOWN = 0,
    HDI_ADAPTER_TYPE_PRIMARY,
    HDI_ADAPTER_TYPE_A2DP,
    HDI_ADAPTER_TYPE_USB,
    HDI_ADAPTER_TYPE_DP,
    HDI_ADAPTER_TYPE_REMOTE,
    HDI_ADAPTER_TYPE_HEARING_AID,
    HDI_ADAPTER_TYPE_ACCESSORY,
    HDI_ADAPTER_TYPE_SLE,
    HDI_ADAPTER_TYPE_VA,
};

} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_PIPE_TYPES_H
