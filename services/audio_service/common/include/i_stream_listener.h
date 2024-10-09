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

#ifndef I_STREAM_LISTENER_H
#define I_STREAM_LISTENER_H

#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {
enum Operation : int32_t {
    START_STREAM = 0,
    PAUSE_STREAM,
    STOP_STREAM,
    RELEASE_STREAM,
    FLUSH_STREAM,
    DRAIN_STREAM,
    UPDATE_STREAM, // when server notify client index update
    BUFFER_UNDERRUN,
    BUFFER_OVERFLOW,
    SET_OFFLOAD_ENABLE,
    UNDERFLOW_COUNT_ADD, // notify client underflow count increment
    DATA_LINK_CONNECTING,  // a2dp offload connecting
    DATA_LINK_CONNECTED,
    MAX_OPERATION_CODE // in plan add underrun overflow
};
class IStreamListener {
public:
    virtual ~IStreamListener() = default;

    virtual int32_t OnOperationHandled(Operation operation, int64_t result) = 0;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // I_STREAM_LISTENER_H
