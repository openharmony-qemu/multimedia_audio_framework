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
#ifndef LOG_TAG
#define LOG_TAG "AudioSuiteCapabilities"
#endif

#include <audio_suite_capabilities.h>

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

AudioSuiteCapabilities::AudioSuiteCapabilities()
{
    CHECK_AND_RETURN_LOG(audioSuiteCapabilitiesParser_.LoadConfiguration(audioSuiteCapabilities_),
        "audioSuiteCapabilitiesParser LoadConfiguration failed, path: %{public}s.",
        AUDIO_SUITE_CAPABILITIES_CONFIG_FILE);
}

int32_t AudioSuiteCapabilities::LoadVbCapability(NodeCapability &nc)
{
    AudioVoiceMorhpingSpec specs;
    CHECK_AND_RETURN_RET_LOG(
        LoadCapability("AudioVoiceMorphingGetSpec", nc.soPath + nc.soName, specs) == SUCCESS,
        ERROR, "LoadVbCapability failed.");
    nc.supportedOnThisDevice = specs.currentDeviceSupport;
    nc.isSupportRealtime = specs.realTimeSupport;
    return SUCCESS;
}

int32_t AudioSuiteCapabilities::LoadEqCapability(NodeCapability &nc)
{
    iMedia_Support_SPECS specs;
    CHECK_AND_RETURN_RET_LOG(
        LoadCapability("iMedia_Eq_GetSPECS", nc.soPath + nc.soName, specs) == SUCCESS,
        ERROR, "LoadEqCapability failed.");
    nc.supportedOnThisDevice = specs.currentDeviceSupport;
    nc.isSupportRealtime = specs.realTimeSupport;
    return SUCCESS;
}

int32_t AudioSuiteCapabilities::LoadSfCapability(NodeCapability &nc)
{
    iMedia_Support_SPECS specs;
    CHECK_AND_RETURN_RET_LOG(
        LoadCapability("iMedia_Surround_GetSPECS", nc.soPath + nc.soName, specs) == SUCCESS,
        ERROR, "LoadSfCapability failed.");
    nc.supportedOnThisDevice = specs.currentDeviceSupport;
    nc.isSupportRealtime = specs.realTimeSupport;
    return SUCCESS;
}

int32_t AudioSuiteCapabilities::LoadEnvCapability(NodeCapability &nc)
{
    iMedia_Support_SPECS specs;
    CHECK_AND_RETURN_RET_LOG(
        LoadCapability("iMedia_Env_GetSPECS", nc.soPath + nc.soName, specs) == SUCCESS,
        ERROR, "LoadEnvCapability failed.");
    nc.supportedOnThisDevice = specs.currentDeviceSupport;
    nc.isSupportRealtime = specs.realTimeSupport;
    return SUCCESS;
}

int32_t AudioSuiteCapabilities::LoadAinrCapability(NodeCapability &nc)
{
    AUDIO_INFO_LOG("loadCapability start.");
    std::string algoSoPath = nc.soPath + nc.soName;
    void *libHandle = dlopen(algoSoPath.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    CHECK_AND_RETURN_RET_LOG(libHandle != nullptr,
        ERROR, "dlopen algo: %{private}s so fail, error: %{public}s",
        algoSoPath.c_str(), dlerror());
    using GetFunc = AudioAinrSpecPointer (*)();
    GetFunc getSpecsFunc = reinterpret_cast<GetFunc>(dlsym(libHandle, "AudioAinrGetSpec"));
    CHECK_AND_RETURN_RET_LOG(getSpecsFunc != nullptr,
        ERROR, "dlsym algo: %{private}s so fail, function name: %{public}s",
        algoSoPath.c_str(), "AudioAinrGetSpec");
    AudioAinrSpecPointer specs = getSpecsFunc();
    CHECK_AND_RETURN_RET_LOG(specs != nullptr,
        ERROR, "function: %{public}s return a nullptr.", "AudioAinrGetSpec");
    nc.supportedOnThisDevice = specs->supportCurrdevice;
    nc.isSupportRealtime = specs->supportRealtimeProc;
    dlclose(libHandle);
    libHandle = nullptr;
    AUDIO_INFO_LOG("loadCapability end.");
    return SUCCESS;
}

int32_t AudioSuiteCapabilities::LoadAissCapability(NodeCapability &nc)
{
    AUDIO_INFO_LOG("LoadAissCapability start.");
    std::string algoSoPath = nc.soPath + nc.soName;
    void *libHandle = dlopen(algoSoPath.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    CHECK_AND_RETURN_RET_LOG(libHandle != nullptr,
        ERROR, "dlopen algo: %{private}s so fail, error: %{public}s",
        algoSoPath.c_str(), dlerror());
    AudioEffectLibrary *audioEffectLibHandle =
        static_cast<AudioEffectLibrary *>(dlsym(libHandle, AISS_LIBRARY_INFO_SYM_AS_STR.c_str()));
    CHECK_AND_RETURN_RET_LOG(audioEffectLibHandle != nullptr,
        ERROR, "dlsym algo: %{private}s so fail, function name: %{public}s",
        algoSoPath.c_str(), AISS_LIBRARY_INFO_SYM_AS_STR.c_str());
    struct AlgoSupportConfig supportConfig = {};
    audioEffectLibHandle->supportEffect(&supportConfig);
    nc.supportedOnThisDevice = supportConfig.isSupport;
    nc.isSupportRealtime = supportConfig.isRealTime;

    dlclose(libHandle);
    libHandle = nullptr;
    AUDIO_INFO_LOG("LoadAissCapability end.");
    return SUCCESS;
}

int32_t AudioSuiteCapabilities::IsNodeTypeSupported(AudioNodeType nodeType, bool* isSupported)
{
    CHECK_AND_RETURN_RET_LOG(isSupported != nullptr,
        ERR_INVALID_PARAM, "IsNodeTypeSupported isSupported is nullptr, nodeType: %{public}d.", nodeType);
    if (nodeType == NODE_TYPE_INPUT || nodeType == NODE_TYPE_OUTPUT || nodeType == NODE_TYPE_AUDIO_MIXER) {
        *isSupported = true;
        return SUCCESS;
    }
    auto it = audioSuiteCapabilities_.find(nodeType);
    if (it != audioSuiteCapabilities_.end()) {
        NodeCapability &nc = it->second;
        if (nc.general == "yes") {
            AUDIO_INFO_LOG("nodeType: %{public}d is general on all device.", nodeType);
            *isSupported = true;
            return SUCCESS;
        } else {
            if (GetNodeCapability(nodeType, nc) == SUCCESS) {
                AUDIO_INFO_LOG("nodeType: %{public}d isSupported status is %{public}d on this device.",
                    nodeType, nc.supportedOnThisDevice);
                *isSupported = nc.supportedOnThisDevice;
                return SUCCESS;
            } else {
                AUDIO_ERR_LOG("GetNodeCapability failed for node type: %{public}d.", nodeType);
                return ERR_INVALID_PARAM;
            }
        }
    } else {
        // For normal case, this nodeType must exist in audioSuiteCapabilities.
        AUDIO_ERR_LOG("unconfigured node type: %{public}d.", nodeType);
        return ERR_INVALID_PARAM;
    }
}

// This function is only provided for effect node without mixerNode.
int32_t AudioSuiteCapabilities::GetNodeCapability(AudioNodeType nodeType, NodeCapability &nodeCapability)
{
    CHECK_AND_RETURN_RET(nodeType != NODE_TYPE_AUDIO_MIXER, SUCCESS);
    auto it = audioSuiteCapabilities_.find(nodeType);
    CHECK_AND_RETURN_RET_LOG(
        it != audioSuiteCapabilities_.end(), ERROR, "no such nodeType: %{public}d configured.", nodeType);
    NodeCapability &nc = it->second;
    if (!(nc.isLoaded)) {
        switch (nodeType) {
            case NODE_TYPE_AUDIO_SEPARATION:
                CHECK_AND_RETURN_RET_LOG(LoadAissCapability(nc) == SUCCESS, ERROR, "LoadAissCapability failed.");
                break;
            case NODE_TYPE_VOICE_BEAUTIFIER:
                CHECK_AND_RETURN_RET_LOG(LoadVbCapability(nc) == SUCCESS, ERROR, "LoadVbCapability failed.");
                break;
            case NODE_TYPE_EQUALIZER:
                CHECK_AND_RETURN_RET_LOG(LoadEqCapability(nc) == SUCCESS, ERROR, "LoadEqCapability failed.");
                break;
            case NODE_TYPE_SOUND_FIELD:
                CHECK_AND_RETURN_RET_LOG(LoadSfCapability(nc) == SUCCESS, ERROR, "LoadSfCapability failed.");
                break;
            case NODE_TYPE_ENVIRONMENT_EFFECT:
                CHECK_AND_RETURN_RET_LOG(LoadEnvCapability(nc) == SUCCESS, ERROR, "LoadEnvCapability failed.");
                break;
            case NODE_TYPE_NOISE_REDUCTION:
                CHECK_AND_RETURN_RET_LOG(LoadAinrCapability(nc) == SUCCESS, ERROR, "LoadAinrCapability failed.");
                break;
            default:
                AUDIO_ERR_LOG("no such nodeType: %{public}d configured.", nodeType);
                return ERROR;
        }
        nc.isLoaded = true;
    }
    nodeCapability = nc;
    return SUCCESS;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
