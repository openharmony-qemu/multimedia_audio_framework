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

#ifndef AUDIO_DEFINITIONS_UNIT_TEST_UTILS_H
#define AUDIO_DEFINITIONS_UNIT_TEST_UTILS_H

#include "audio_stream_descriptor.h"

namespace OHOS {
namespace AudioStandard {

const uint32_t TEST_RENDERER_SESSION_ID = 100000;
const uint32_t TEST_CAPTURER_SESSION_ID = 100001;
const AudioSamplingRate TEST_AUDIO_SAMPLE_RATE = SAMPLE_RATE_48000;
const AudioEncodingType TEST_AUDIO_ENCODING_TYPE = ENCODING_PCM;
const AudioSampleFormat TEST_AUDIO_SAMPLE_FORMAT = SAMPLE_S16LE;
const AudioChannel TEST_AUDIO_CHANNEL = STEREO;
const AudioChannelLayout TEST_AUDIO_CHANNEL_LAYOUT = CH_LAYOUT_STEREO;
const int32_t TEST_UID = 1000;
const uint32_t TEST_TOKEN_ID = 10000;
const int32_t TEST_PID = 2000;
const uint64_t TEST_FULL_TOKEN_ID = 20000;

class AudioDefinitionsUnitTestUtil {
public:
    static std::shared_ptr<AudioStreamDescriptor> GenerateCommonStream(AudioMode mode);
};

} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_DEFINITIONS_UNIT_TEST_UTILS_H
