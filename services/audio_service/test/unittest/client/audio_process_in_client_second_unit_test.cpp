/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "audio_service_log.h"
#include "audio_service.h"
#include "audio_errors.h"
#include "audio_process_in_client.h"
#include "audio_process_in_client.cpp"
#include "fast_audio_stream.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
class MockIAudioProcess : public IAudioProcess {
public:
    MOCK_METHOD(int32_t, ResolveBufferBaseAndGetServerSpanSize,
        (std::shared_ptr<OHAudioBufferBase> buffer, uint32_t &spanSizeInFrame), (override));

    MOCK_METHOD(int32_t, GetSessionId, (uint32_t &sessionId), (override));
    MOCK_METHOD(int32_t, Start, (), (override));
    MOCK_METHOD(int32_t, Pause, (bool isFlush), (override));
    MOCK_METHOD(int32_t, Resume, (), (override));

    MOCK_METHOD(int32_t, Stop, (int32_t stage), (override));

    MOCK_METHOD(int32_t, RequestHandleInfo, (), (override));

    MOCK_METHOD(int32_t, RequestHandleInfoAsync, (), (override));

    MOCK_METHOD(int32_t, Release, (bool isSwitchStream), (override));

    MOCK_METHOD(int32_t, RegisterProcessCb, (const &sptr<object>), (override));

    MOCK_METHOD(int32_t, RegisterThreadPriority,
        (int32_t tid, const std::string &bundleName, uint32_t method), (override));

    MOCK_METHOD(int32_t, SetDefaultOutputDevice, (int32_t defaultOutputDevice), (override));
    MOCK_METHOD(int32_t, SetSilentModeAndMixWithOthers, (bool on), (override));
    MOCK_METHOD(int32_t, SetSourceDuration, (int64_t duration), (override));
    MOCK_METHOD(int32_t, SetUnderrunCount, (uint32_t underrunCnt), (override));

    MOCK_METHOD(int32_t, SaveAdjustStreamVolumeInfo,
        (float volume, uint32_t sessionId, const std::string &adjustTime, uint32_t code), (override));

    MOCK_METHOD(int32_t, SetAudioHapticsSyncId, (int32_t audioHapticsSyncId), (override));

    MOCK_METHOD(sptr<IRemoteObject>, AsObject, (), (override));
};
}
}