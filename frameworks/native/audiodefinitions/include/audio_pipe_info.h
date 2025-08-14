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

#ifndef PIPE_INFO_H
#define PIPE_INFO_H
#define HDI_INVALID_ID 0xFFFFFFFF

#include <memory>
#include "parcel.h"

#include "audio_module_info.h"
#include "audio_stream_descriptor.h"

namespace OHOS {
namespace AudioStandard {

enum AudioPipeAction {
    PIPE_ACTION_DEFAULT = 0,
    PIPE_ACTION_NEW,
    PIPE_ACTION_UPDATE,
};

class AudioPipeInfo {
public:
    uint32_t id_ = HDI_INVALID_ID;

    uint32_t paIndex_ = 0;

    AudioPipeRole pipeRole_ = PIPE_ROLE_OUTPUT;

    std::string name_ = "undefine";

    uint32_t routeFlag_ = 0;

    std::string adapterName_ = "";

    AudioModuleInfo moduleInfo_ = {};
    AudioStreamInfo audioStreamInfo_ = {};
    void InitAudioStreamInfo();

    AudioPipeAction pipeAction_ = PIPE_ACTION_DEFAULT;

    bool softLinkFlag_ = false;

    std::vector<std::shared_ptr<AudioStreamDescriptor>> streamDescriptors_ = {};

    std::unordered_map<uint32_t, std::shared_ptr<AudioStreamDescriptor>> streamDescMap_ = {};

    AudioPipeInfo();

    virtual ~AudioPipeInfo();

    AudioPipeInfo(const std::shared_ptr<AudioPipeInfo> pipeInfo);

    void Dump(std::string &dumpString);

    std::string ToString();

private:
    bool IsOutput()
    {
        return pipeRole_ == PIPE_ROLE_OUTPUT;
    }

    void DumpCommonAttrs(std::string &dumpString);

    void DumpOutputAttrs(std::string &dumpString);

    void DumpInputAttrs(std::string &dumpString);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // PIPE_INFO_H
