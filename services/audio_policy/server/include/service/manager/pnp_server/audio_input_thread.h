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

#ifndef ST_AUDIO_INPUT_THREAD_H
#define ST_AUDIO_INPUT_THREAD_H

#include <linux/input.h>

#include "hdf_types.h"
#include "v3_0/audio_types.h"
#include "audio_pnp_param.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

class AudioInputThread {
public:
    static int32_t AudioPnpInputOpen();
    static int32_t AudioPnpInputPollAndRead();
    static AudioEvent audioInputEvent_;

private:
    static int32_t AudioAnalogHeadsetDeviceCheck(input_event evt);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_INPUT_THREAD_H