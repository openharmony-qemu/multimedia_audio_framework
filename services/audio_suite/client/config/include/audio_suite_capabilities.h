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

#ifndef AUDIO_SUITE_CAPABILITIES_H
#define AUDIO_SUITE_CAPABILITIES_H

#include <dlfcn.h>
#include "audio_suite_base.h"
#include "audio_suite_capabilities_parser.h"
#include "imedia_api.h"
#include "audio_hms_ainr_api.h"
#include "audio_voicemorphing_api.h"
#include "audio_effect.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

const std::string AISS_LIBRARY_INFO_SYM_AS_STR = "AISSLIB";

class AudioSuiteCapabilities {
public:
    AudioSuiteCapabilities(const AudioSuiteCapabilities&) = delete;
    AudioSuiteCapabilities& operator=(const AudioSuiteCapabilities&) = delete;

    static AudioSuiteCapabilities& getInstance()
    {
        static AudioSuiteCapabilities instance;
        return instance;
    }

    int32_t IsNodeTypeSupported(AudioNodeType nodeType, bool* isSupported);
    int32_t GetNodeCapability(AudioNodeType nodeType, NodeCapability &nodeCapability);

private:

    AudioSuiteCapabilities();
    ~AudioSuiteCapabilities() = default;

    template <typename T>
    int32_t LoadCapability(std::string functionName, std::string algoSoPath, T &specs)
    {
        AUDIO_INFO_LOG("loadCapability start.");
        void *libHandle = dlopen(algoSoPath.c_str(), RTLD_LAZY | RTLD_GLOBAL);
        CHECK_AND_RETURN_RET_LOG(libHandle != nullptr,
            ERROR, "dlopen algo: %{private}s so fail, error: %{public}s",
            algoSoPath.c_str(), dlerror());
        using GetFunc = T (*)();
        GetFunc getSpecsFunc = reinterpret_cast<GetFunc>(dlsym(libHandle, functionName.c_str()));
        CHECK_AND_RETURN_RET_LOG(getSpecsFunc != nullptr,
            ERROR, "dlsym algo: %{private}s so fail, function name: %{public}s",
            algoSoPath.c_str(), functionName.c_str());
        specs = getSpecsFunc();
        dlclose(libHandle);
        libHandle = nullptr;
        AUDIO_INFO_LOG("loadCapability end.");
        return SUCCESS;
    }

    int32_t LoadVbCapability(NodeCapability &nc);
    int32_t LoadEqCapability(NodeCapability &nc);
    int32_t LoadAinrCapability(NodeCapability &nc);
    int32_t LoadSfCapability(NodeCapability &nc);
    int32_t LoadEnvCapability(NodeCapability &nc);
    int32_t LoadAissCapability(NodeCapability &nc);
    std::unordered_map<AudioNodeType, NodeCapability> audioSuiteCapabilities_;
    AudioSuiteCapabilitiesParser audioSuiteCapabilitiesParser_;
};
}
}
}

#endif // AUDIO_SUITE_CAPABILITIES_H