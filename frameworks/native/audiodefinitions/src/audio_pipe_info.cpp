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

#include "audio_pipe_info.h"

#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {

bool AudioPipeInfo::HasSameStreams(std::shared_ptr<AudioPipeInfo> &pipeInfo1, std::shared_ptr<AudioPipeInfo> &pipeInfo2)
{
    if (pipeInfo1->streamDescriptors_.size() != pipeInfo2->streamDescriptors_.size()) {
        return false;
    }
    for (auto &streamDesc1 : pipeInfo1->streamDescriptors_) {
        auto iter = std::find_if(pipeInfo2->streamDescriptors_.begin(), pipeInfo2->streamDescriptors_.end(),
            [&](const std::shared_ptr<AudioStreamDescriptor> &streamDesc2) {
                return streamDesc1->sessionId_ == streamDesc2->sessionId_;
            });
        if (iter == pipeInfo2->streamDescriptors_.end()) {
            return false;
        }
    }
    return true;
}

AudioPipeInfo::AudioPipeInfo()
{
}

AudioPipeInfo::~AudioPipeInfo()
{
}

AudioPipeInfo::AudioPipeInfo(const std::shared_ptr<AudioPipeInfo> pipeInfo)
{
    id_ = pipeInfo->id_;
    paIndex_ = pipeInfo->paIndex_;
    name_ = pipeInfo->name_;
    pipeRole_ = pipeInfo->pipeRole_;
    routeFlag_ = pipeInfo->routeFlag_;
    adapterName_ = pipeInfo->adapterName_;
    moduleInfo_ = pipeInfo->moduleInfo_;
    pipeAction_ = pipeInfo->pipeAction_;
    streamDescriptors_ = pipeInfo->streamDescriptors_;
    streamDescMap_ = pipeInfo->streamDescMap_;
}

void AudioPipeInfo::Dump(std::string &dumpString)
{
    AppendFormat(dumpString, "Pipe %u, role %s, adapter %s, name %s:\n",
        id_, IsOutput() ? "Output" : "Input", adapterName_.c_str(), name_.c_str());

    DumpCommonAttrs(dumpString);

    if (IsOutput()) {
        DumpOutputAttrs(dumpString);
    } else {
        DumpInputAttrs(dumpString);
    }

    // dump each stream in current pipe
    for (auto &streamDesc : streamDescriptors_) {
        if (streamDesc != nullptr) {
            streamDesc->Dump(dumpString);
        }
    }

    AppendFormat(dumpString, "\n");
}

void AudioPipeInfo::DumpCommonAttrs(std::string &dumpString)
{
    AppendFormat(dumpString, "  - SampleRate: %s\n", moduleInfo_.rate.c_str());
    AppendFormat(dumpString, "  - ChannelCount: %s\n", moduleInfo_.channels.c_str());
    AppendFormat(dumpString, "  - Format: %s\n", moduleInfo_.format.c_str());
    AppendFormat(dumpString, "  - BufferSize: %s\n", moduleInfo_.bufferSize.c_str());
}

void AudioPipeInfo::DumpOutputAttrs(std::string &dumpString)
{
    AppendFormat(dumpString, "  - RenderInIdleState: %s\n", moduleInfo_.renderInIdleState.c_str());
}

void AudioPipeInfo::DumpInputAttrs(std::string &dumpString)
{
    AppendFormat(dumpString, "  - SourceType: %s\n", moduleInfo_.sourceType.c_str());
}

std::string AudioPipeInfo::ToString()
{
    std::string out = "";
    AppendFormat(out, "id %u, adapter %s, name %s",
        id_, adapterName_.c_str(), name_.c_str());
    return out;
}

} // AudioStandard
} // namespace OHOS