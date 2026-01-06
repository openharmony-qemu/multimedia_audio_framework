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
#include "audio_suite_tempo_pitch_api.h"
#include "audio_effect.h"
#include "audio_hms_space_render_api.h"
#include "audio_suite_common.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

const std::string AISS_LIBRARY_INFO_SYM_AS_STR = "AISSLIB";
const std::string PITCH_LIBRARY_INFO_SYM_AS_STR = "PITCHLIB";

class AudioSuiteCapabilities {
public:
    AudioSuiteCapabilities(const AudioSuiteCapabilities&) = delete;
    AudioSuiteCapabilities& operator=(const AudioSuiteCapabilities&) = delete;

    static AudioSuiteCapabilities& GetInstance()
    {
        static AudioSuiteCapabilities instance;
        return instance;
    }

    int32_t IsNodeTypeSupported(AudioNodeType nodeType, bool* isSupported);
    int32_t GetNodeParameter(AudioNodeType nodeType, NodeParameter &nodeCapability);

private:
    AudioSuiteLibraryManager algoLibrary_;

    AudioSuiteCapabilities();
    ~AudioSuiteCapabilities() = default;

    template <typename T>
    int32_t LoadCapability(std::string functionName, std::string algoSoPath, T &specs)
    {
        AUDIO_INFO_LOG("loadCapability start.");
        void *libHandle = algoLibrary_.LoadLibrary(algoSoPath);
        CHECK_AND_RETURN_RET_LOG(
            libHandle != nullptr, ERROR, "LoadLibrary failed with path: %{private}s", algoSoPath.c_str());
        using GetFunc = T (*)();
        GetFunc getSpecsFunc = reinterpret_cast<GetFunc>(dlsym(libHandle, functionName.c_str()));
        if (getSpecsFunc == nullptr) {
            dlclose(libHandle);
            AUDIO_ERR_LOG("dlsym failed");
            return ERROR;
        }

        specs = getSpecsFunc();
        dlclose(libHandle);
        libHandle = nullptr;
        AUDIO_INFO_LOG("loadCapability end.");
        return SUCCESS;
    }

    template <typename T>
    int32_t SetAudioParameters(NodeParameter &nc, T &specs)
    {
        nc.supportedOnThisDevice = specs.isSupport;
        if (specs.frameLen != 0) {
            nc.frameLen = specs.frameLen;
        }
        nc.inSampleRate = specs.inSampleRate;
        nc.inChannels = specs.inChannels;
        nc.inFormat = specs.inFormat;
        nc.outSampleRate = specs.outSampleRate;
        nc.outChannels = specs.outChannels;
        nc.outFormat = specs.outFormat;
        AUDIO_INFO_LOG("inChannels:%{public}d, inFormat:%{public}d, inSampleRate:%{public}d  ",
            nc.inChannels,
            nc.inFormat,
            nc.inSampleRate);
        AUDIO_INFO_LOG("outChannels:%{public}d, outFormat:%{public}d, outSampleRate:%{public}d, frameLen:%{public}d",
            nc.outChannels,
            nc.outFormat,
            nc.outSampleRate,
            nc.frameLen);
        return SUCCESS;
    }
    int32_t LoadVbCapability(NodeParameter &nc);
    int32_t LoadEqCapability(NodeParameter &nc);
    int32_t LoadAinrCapability(NodeParameter &nc);
    int32_t LoadSfCapability(NodeParameter &nc);
    int32_t LoadEnvCapability(NodeParameter &nc);
    int32_t LoadSrCapability(NodeParameter &nc);
    int32_t LoadAissCapability(NodeParameter &nc);
    int32_t LoadGeneralCapability(NodeParameter &nc);
    int32_t LoadPureCapability(NodeParameter &nc);
    int32_t LoadTempoPitchCapability(NodeParameter &nc);
    std::unordered_map<AudioNodeType, NodeParameter> audioSuiteCapabilities_;
    AudioSuiteCapabilitiesParser audioSuiteCapabilitiesParser_;
};
}
}
}

#endif // AUDIO_SUITE_CAPABILITIES_H