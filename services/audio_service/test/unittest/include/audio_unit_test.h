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


#ifndef AUDIO_UNIT_TEST_H
#define AUDIO_UNIT_TEST_H

#include "accesstoken_kit.h"
#include "token_setproc.h"

#define DEC 10

namespace OHOS::AudioStandard {
class MockNative {
public:
    MockNative() = delete;
    ~MockNative() = delete;
    MockNative(const MockNative &) = delete;
    MockNative &operator=(const MockNative &) = delete;

    static inline void Mock() { SetSelfTokenID(MockNative::native); }

    static inline void Resume() { SetSelfTokenID(MockNative::self); }

    static void GenerateNativeTokenID(const std::string processName = "audio_server")
    {
        std::string dumpInfo;
        Security::AccessToken::AtmToolsParamInfo info;
        info.processName = processName;
        Security::AccessToken::AccessTokenKit::DumpTokenInfo(info, dumpInfo);

        const std::string target = R"("tokenID": )";
        std::size_t pos = dumpInfo.find(target);
        if (pos == std::string::npos) {
            return;
        }
        pos += target.length();
        MockNative::native = strtoull(dumpInfo.c_str() + pos, nullptr, DEC);
    }

private:
    static inline Security::AccessToken::AccessTokenID self = GetSelfTokenID();

    static inline Security::AccessToken::AccessTokenID native = GetSelfTokenID();
};
} // namespace OHOS::AudioStandard

#endif // AUDIO_UNIT_TEST_H