/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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
#ifndef AUDIO_GROUP_HANDLE_H
#define AUDIO_GROUP_HANDLE_H

#include <iostream>
#include <string>

namespace OHOS {
namespace AudioStandard {
static const int32_t MAX_ID = 10000;
static const int32_t GROUP_ID_NONE = -1;
static const int32_t NO_REMOTE_ID = -2;
static const char* GROUP_NAME_NONE = "NO_GROUP";
static const char* GROUP_NAME_DEFAULT = "DEFULT_GROUP";

enum GroupType {
    VOLUME_TYPE = 1,
    INTERRUPT_TYPE = 2
};

class AudioGroupHandle {
public:
    static AudioGroupHandle& GetInstance()
    {
        static AudioGroupHandle audioGroupHandle;
        return audioGroupHandle;
    }

    int32_t GetNextId(GroupType type);

private:
    AudioGroupHandle()
    {
        currentVolumeId_ = 0;
        currentInterruptId_ = 0;
    }

    ~AudioGroupHandle();

    void CheckId(GroupType type);

    int32_t currentVolumeId_;
    int32_t currentInterruptId_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_GROUP_HANDLE_H