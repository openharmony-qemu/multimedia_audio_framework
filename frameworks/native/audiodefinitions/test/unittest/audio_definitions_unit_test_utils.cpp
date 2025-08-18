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

#include "audio_definitions_unit_test_utils.h"

namespace OHOS {
namespace AudioStandard {

std::shared_ptr<AudioStreamDescriptor> AudioDefinitionsUnitTestUtil::GenerateCommonStream(AudioMode mode)
{
    AudioStreamInfo streamInfo = {
        TEST_AUDIO_SAMPLE_RATE,
        TEST_AUDIO_ENCODING_TYPE,
        TEST_AUDIO_SAMPLE_FORMAT,
        TEST_AUDIO_CHANNEL,
        TEST_AUDIO_CHANNEL_LAYOUT
    };

    AppInfo appInfo = {
        TEST_UID,
        TEST_TOKEN_ID,
        TEST_PID,
        TEST_FULL_TOKEN_ID
    };

    std::shared_ptr<AudioStreamDescriptor> streamDesc;
    if (mode == AUDIO_MODE_PLAYBACK) {
        AudioRendererInfo rendererInfo(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_MOVIE);
        streamDesc = std::make_shared<AudioStreamDescriptor>(
            streamInfo, rendererInfo, appInfo);
        streamDesc->sessionId_ = TEST_RENDERER_SESSION_ID;
    } else {
        AudioCapturerInfo capturerInfo(SOURCE_TYPE_MIC, 0);
        streamDesc = std::make_shared<AudioStreamDescriptor>(
            streamInfo, capturerInfo, appInfo);
        streamDesc->sessionId_ = TEST_CAPTURER_SESSION_ID;
    }

    return streamDesc;
}

} // namespace AudioStandard
} // namespace OHOS