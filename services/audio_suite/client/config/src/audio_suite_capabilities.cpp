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

#include <filesystem>
#include <audio_suite_capabilities.h>
#include "audio_suite_log.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

AudioSuiteCapabilities::AudioSuiteCapabilities()
{
    CHECK_AND_RETURN_LOG(audioSuiteCapabilitiesParser_.LoadConfiguration(audioSuiteCapabilities_),
        "audioSuiteCapabilitiesParser LoadConfiguration failed, path: %{public}s.",
        AUDIO_SUITE_CAPABILITIES_CONFIG_FILE);
}

template <typename T>
int32_t AudioSuiteCapabilities::LoadCapability(const std::string& functionName, std::string algoSoPath, T &specs)
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

template int32_t AudioSuiteCapabilities::LoadCapability(const std::string& functionName,
    std::string algoSoPath, AudioVoiceMorhpingSpec &specs);
template int32_t AudioSuiteCapabilities::LoadCapability(const std::string& functionName,
    std::string algoSoPath, iMedia_Support_SPECS &specs);
template int32_t AudioSuiteCapabilities::LoadCapability(const std::string& functionName,
    std::string algoSoPath, AudioVoiceMphTradSpec &specs);

template <typename T>
int32_t AudioSuiteCapabilities::SetAudioParameters(NodeParameter &np, T &specs)
{
    np.supportedOnThisDevice = specs.isSupport;
    if (specs.frameLen != 0) {
        np.frameLen = specs.frameLen;
    }
    np.inSampleRate = specs.inSampleRate;
    np.inChannels = specs.inChannels;
    np.inFormat = specs.inFormat;
    np.outSampleRate = specs.outSampleRate;
    np.outChannels = specs.outChannels;
    np.outFormat = specs.outFormat;
    AUDIO_INFO_LOG("inChannels:%{public}d, inFormat:%{public}d, inSampleRate:%{public}d  ",
        np.inChannels,
        np.inFormat,
        np.inSampleRate);
    AUDIO_INFO_LOG("outChannels:%{public}d, outFormat:%{public}d, outSampleRate:%{public}d, frameLen:%{public}d",
        np.outChannels,
        np.outFormat,
        np.outSampleRate,
        np.frameLen);
    return SUCCESS;
}

template int32_t AudioSuiteCapabilities::SetAudioParameters(NodeParameter &np, AudioVoiceMorhpingSpec &specs);
template int32_t AudioSuiteCapabilities::SetAudioParameters(NodeParameter &np, iMedia_Support_SPECS &specs);
template int32_t AudioSuiteCapabilities::SetAudioParameters(NodeParameter &np, SpaceRenderSpeces &specs);
template int32_t AudioSuiteCapabilities::SetAudioParameters(NodeParameter &np, AlgoSupportConfig &specs);
template int32_t AudioSuiteCapabilities::SetAudioParameters(NodeParameter &np, AudioVoiceMphTradSpec &specs);
template int32_t AudioSuiteCapabilities::SetAudioParameters(NodeParameter &np, AudioPVSpec &specs);

int32_t AudioSuiteCapabilities::LoadVbCapability(NodeParameter &np)
{
    AudioVoiceMorhpingSpec specs;
    CHECK_AND_RETURN_RET_LOG(
        LoadCapability("AudioVoiceMorphingGetSpec", np.soPath + np.soName, specs) == SUCCESS,
        ERROR, "LoadVbCapability failed.");
    CHECK_AND_RETURN_RET_LOG(SetAudioParameters(np, specs) == SUCCESS, ERROR, "SetAudioParameters failed.");
    return SUCCESS;
}

int32_t AudioSuiteCapabilities::LoadEqCapability(NodeParameter &np)
{
    iMedia_Support_SPECS specs;
    CHECK_AND_RETURN_RET_LOG(
        LoadCapability("iMedia_Eq_GetSPECS", np.soPath + np.soName, specs) == SUCCESS,
        ERROR, "LoadEqCapability failed.");
    CHECK_AND_RETURN_RET_LOG(SetAudioParameters(np, specs) == SUCCESS, ERROR, "SetAudioParameters failed.");
    return SUCCESS;
}

int32_t AudioSuiteCapabilities::LoadSfCapability(NodeParameter &np)
{
    iMedia_Support_SPECS specs;
    CHECK_AND_RETURN_RET_LOG(
        LoadCapability("iMedia_Surround_GetSPECS", np.soPath + np.soName, specs) == SUCCESS,
        ERROR, "LoadSfCapability failed.");
    CHECK_AND_RETURN_RET_LOG(SetAudioParameters(np, specs) == SUCCESS, ERROR, "SetAudioParameters failed.");
    return SUCCESS;
}

int32_t AudioSuiteCapabilities::LoadEnvCapability(NodeParameter &np)
{
    iMedia_Support_SPECS specs;
    CHECK_AND_RETURN_RET_LOG(
        LoadCapability("iMedia_Env_GetSPECS", np.soPath + np.soName, specs) == SUCCESS,
        ERROR, "LoadEnvCapability failed.");
    CHECK_AND_RETURN_RET_LOG(SetAudioParameters(np, specs) == SUCCESS, ERROR, "SetAudioParameters failed.");
    return SUCCESS;
}

int32_t AudioSuiteCapabilities::LoadSrCapability(NodeParameter &np)
{
    AUDIO_INFO_LOG("loadSrCapability start.");
    std::string algoSoPath = np.soPath + np.soName;
    void *libHandle = algoLibrary_.LoadLibrary(algoSoPath);
    CHECK_AND_RETURN_RET_LOG(
        libHandle != nullptr, ERROR, "LoadLibrary failed with path: %{private}s", algoSoPath.c_str());

    using FunSpaceRenderGetSpeces = SpaceRenderSpeces (*)();
    FunSpaceRenderGetSpeces getSpecsFunc =
        reinterpret_cast<FunSpaceRenderGetSpeces>(dlsym(libHandle, "SpaceRenderGetSpeces"));
    if (getSpecsFunc == nullptr) {
        dlclose(libHandle);
        libHandle = nullptr;
        AUDIO_ERR_LOG("dlsym algo: %{private}s so fail, function name: %{public}s",
            algoSoPath.c_str(), "SpaceRenderGetSpeces");
        return ERROR;
    }
    SpaceRenderSpeces specs = getSpecsFunc();
    CHECK_AND_RETURN_RET_LOG(SetAudioParameters(np, specs) == SUCCESS, ERROR, "SetAudioParameters failed.");
    dlclose(libHandle);
    libHandle = nullptr;
    AUDIO_INFO_LOG("loadCapability end.");
    return SUCCESS;
}

int32_t AudioSuiteCapabilities::LoadAinrCapability(NodeParameter &np)
{
    AUDIO_INFO_LOG("loadCapability start.");
    std::string algoSoPath = np.soPath + np.soName;
    void *libHandle = algoLibrary_.LoadLibrary(algoSoPath);
    CHECK_AND_RETURN_RET_LOG(
        libHandle != nullptr, ERROR, "LoadLibrary failed with path: %{private}s", algoSoPath.c_str());
    using GetFunc = AudioAinrSpecPointer (*)();
    GetFunc getSpecsFunc = reinterpret_cast<GetFunc>(dlsym(libHandle, "AudioAinrGetSpec"));
    CHECK_AND_RETURN_RET_LOG(getSpecsFunc != nullptr,
        ERROR, "dlsym algo: %{private}s so fail, function name: %{public}s",
        algoSoPath.c_str(), "AudioAinrGetSpec");
    AudioAinrSpecPointer specs = getSpecsFunc();
    CHECK_AND_RETURN_RET_LOG(specs != nullptr,
        ERROR, "function: %{public}s return a nullptr.", "AudioAinrGetSpec");
    np.supportedOnThisDevice = specs->isSupport;
    np.frameLen = specs->frameLen;
    np.inSampleRate = specs->inSampleRate;
    np.inChannels = specs->inChannels;
    np.inFormat = specs->inFormat;
    np.outSampleRate = specs->outSampleRate;
    np.outChannels = specs->outChannels;
    np.outFormat = specs->outFormat;
    dlclose(libHandle);
    libHandle = nullptr;
    AUDIO_INFO_LOG("loadCapability end.");
    return SUCCESS;
}

int32_t AudioSuiteCapabilities::LoadAissCapability(NodeParameter &np)
{
    AUDIO_INFO_LOG("LoadAissCapability start.");
    std::string algoSoPath = np.soPath + np.soName;
    void *libHandle = algoLibrary_.LoadLibrary(algoSoPath);
    CHECK_AND_RETURN_RET_LOG(
        libHandle != nullptr, ERROR, "LoadLibrary failed with path: %{private}s", algoSoPath.c_str());
    AudioEffectLibrary *audioEffectLibHandle =
        static_cast<AudioEffectLibrary *>(dlsym(libHandle, AISS_LIBRARY_INFO_SYM_AS_STR.c_str()));
    CHECK_AND_RETURN_RET_LOG(audioEffectLibHandle != nullptr,
        ERROR, "dlsym algo: %{private}s so fail, function name: %{public}s",
        algoSoPath.c_str(), AISS_LIBRARY_INFO_SYM_AS_STR.c_str());
    struct AlgoSupportConfig supportConfig = {};
    audioEffectLibHandle->supportEffect(&supportConfig);
    CHECK_AND_RETURN_RET_LOG(SetAudioParameters(np, supportConfig) == SUCCESS, ERROR, "SetAudioParameters failed.");

    dlclose(libHandle);
    libHandle = nullptr;
    AUDIO_INFO_LOG("LoadAissCapability end.");
    return SUCCESS;
}

int32_t AudioSuiteCapabilities::LoadGeneralCapability(NodeParameter &np)
{
    AudioVoiceMorhpingSpec specs;
    CHECK_AND_RETURN_RET_LOG(
        LoadCapability("AudioVoiceMorphingGetSpec", np.soPath + np.soName, specs) == SUCCESS,
        ERROR, "LoadGeneralCapability failed.");
    CHECK_AND_RETURN_RET_LOG(SetAudioParameters(np, specs) == SUCCESS, ERROR, "SetAudioParameters failed.");
    return SUCCESS;
}
 
int32_t AudioSuiteCapabilities::LoadPureCapability(NodeParameter &np)
{
    AudioVoiceMphTradSpec specs;
    CHECK_AND_RETURN_RET_LOG(
        LoadCapability("AudioVoiceMphTradGetSpec", np.soPath + np.soName, specs) == SUCCESS,
        ERROR, "LoadPureCapability failed.");
    CHECK_AND_RETURN_RET_LOG(SetAudioParameters(np, specs) == SUCCESS, ERROR, "SetAudioParameters failed.");
    return SUCCESS;
}

int32_t AudioSuiteCapabilities::LoadTempoPitchCapability(NodeParameter &np)
{
    AUDIO_INFO_LOG("LoadTempoPitchCapability start.");
    std::istringstream iss(np.soName);
    std::string tempoSoName = "";
    std::string pitchSoName = "";
    std::getline(iss, tempoSoName, ',');
    std::getline(iss, pitchSoName);
    CHECK_AND_RETURN_RET_LOG(!tempoSoName.empty() && !pitchSoName.empty(), ERROR,
        "LoadTempoPitchCapability parse so name fail");
    // tempo
    std::string tempoSoPath = np.soPath + tempoSoName;
    void *tempoSoHandle = algoLibrary_.LoadLibrary(tempoSoPath);
    CHECK_AND_RETURN_RET_LOG(
        tempoSoHandle != nullptr, ERROR, "LoadLibrary failed with path: %{private}s", tempoSoPath.c_str());
    using GET_SPEC_FUNC = AudioPVSpec(*)(void);
    GET_SPEC_FUNC pvGetSpecFunc = reinterpret_cast<GET_SPEC_FUNC>(dlsym(tempoSoHandle, "PVGetSpec"));
    if (pvGetSpecFunc == nullptr) {
        dlclose(tempoSoHandle);
        tempoSoHandle = nullptr;
        AUDIO_ERR_LOG("dlsym algo: %{private}s so fail, function name: %{public}s",
            tempoSoPath.c_str(), "PVGetSpec");
        return ERROR;
    }
    AudioPVSpec specs = pvGetSpecFunc();
    CHECK_AND_RETURN_RET_LOG(SetAudioParameters(np, specs) == SUCCESS, ERROR, "SetAudioParameters failed.");
    dlclose(tempoSoHandle);
    tempoSoHandle = nullptr;
    // pitch
    std::string pitchSoPath = np.soPath + pitchSoName;
    void *pitchSoHandle = algoLibrary_.LoadLibrary(pitchSoPath);
    CHECK_AND_RETURN_RET_LOG(pitchSoHandle != nullptr, ERROR,
        "LoadLibrary failed with path: %{private}s", pitchSoPath.c_str());
    AudioEffectLibrary *audioEffectLibHandle =
        static_cast<AudioEffectLibrary *>(dlsym(pitchSoHandle, PITCH_LIBRARY_INFO_SYM_AS_STR.c_str()));
    if (audioEffectLibHandle == nullptr) {
        dlclose(pitchSoHandle);
        pitchSoHandle = nullptr;
        AUDIO_ERR_LOG("dlsym algo: %{private}s so fail, function name: %{public}s",
            pitchSoName.c_str(), PITCH_LIBRARY_INFO_SYM_AS_STR.c_str());
        return ERROR;
    }
    struct AlgoSupportConfig supportConfig = {};
    audioEffectLibHandle->supportEffect(&supportConfig);
    np.supportedOnThisDevice &= supportConfig.isSupport;
    dlclose(pitchSoHandle);
    pitchSoHandle = nullptr;
    AUDIO_INFO_LOG("LoadTempoPitchCapability end.");
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
        NodeParameter &np = it->second;
        if (nodeType == NODE_TYPE_TEMPO_PITCH) {
            std::istringstream iss(np.soName);
            std::string tempoSoName = "";
            std::string pitchSoName = "";
            std::getline(iss, tempoSoName, ',');
            std::getline(iss, pitchSoName);
            if (!(std::filesystem::exists(np.soPath + tempoSoName)) ||
                !(std::filesystem::exists(np.soPath + pitchSoName))) {
                *isSupported = false;
                AUDIO_INFO_LOG("nodeType: %{public}d is not supported on this device, so does not exist.", nodeType);
                return SUCCESS;
            }
        } else if (!(std::filesystem::exists(np.soPath + np.soName))) {
            *isSupported = false;
            AUDIO_INFO_LOG("nodeType: %{public}d is not supported on this device, so does not exist.", nodeType);
            return SUCCESS;
        }
        if (np.general == "yes") {
            AUDIO_INFO_LOG("nodeType: %{public}d is general on all device.", nodeType);
            *isSupported = true;
            return SUCCESS;
        } else {
            if (GetNodeParameter(nodeType, np) == SUCCESS) {
                AUDIO_INFO_LOG("nodeType: %{public}d isSupported status is %{public}d on this device.",
                    nodeType, np.supportedOnThisDevice);
                *isSupported = np.supportedOnThisDevice;
                return SUCCESS;
            } else {
                AUDIO_ERR_LOG("GetNodeParameter failed for node type: %{public}d.", nodeType);
                return ERROR;
            }
        }
    } else {
        // For normal case, this nodeType must exist in audioSuiteCapabilities.
        AUDIO_ERR_LOG("unconfigured node type: %{public}d.", nodeType);
        return ERR_INVALID_PARAM;
    }
}

// This function is only provided for effect node without mixerNode.
int32_t AudioSuiteCapabilities::GetNodeParameter(AudioNodeType nodeType, NodeParameter &nodeParameter)
{
    CHECK_AND_RETURN_RET(nodeType != NODE_TYPE_AUDIO_MIXER, SUCCESS);
    auto it = audioSuiteCapabilities_.find(nodeType);
    CHECK_AND_RETURN_RET_LOG(
        it != audioSuiteCapabilities_.end(), ERROR, "no such nodeType: %{public}d configured.", nodeType);
    NodeParameter &np = it->second;
    if (!(np.isLoaded)) {
        switch (nodeType) {
            case NODE_TYPE_AUDIO_SEPARATION:
                CHECK_AND_RETURN_RET_LOG(LoadAissCapability(np) == SUCCESS, ERROR, "LoadAissCapability failed.");
                break;
            case NODE_TYPE_VOICE_BEAUTIFIER:
                CHECK_AND_RETURN_RET_LOG(LoadVbCapability(np) == SUCCESS, ERROR, "LoadVbCapability failed.");
                break;
            case NODE_TYPE_EQUALIZER:
                CHECK_AND_RETURN_RET_LOG(LoadEqCapability(np) == SUCCESS, ERROR, "LoadEqCapability failed.");
                break;
            case NODE_TYPE_SOUND_FIELD:
                CHECK_AND_RETURN_RET_LOG(LoadSfCapability(np) == SUCCESS, ERROR, "LoadSfCapability failed.");
                break;
            case NODE_TYPE_ENVIRONMENT_EFFECT:
                CHECK_AND_RETURN_RET_LOG(LoadEnvCapability(np) == SUCCESS, ERROR, "LoadEnvCapability failed.");
                break;
            case NODE_TYPE_NOISE_REDUCTION:
                CHECK_AND_RETURN_RET_LOG(LoadAinrCapability(np) == SUCCESS, ERROR, "LoadAinrCapability failed.");
                break;
            case NODE_TYPE_GENERAL_VOICE_CHANGE:
                CHECK_AND_RETURN_RET_LOG(LoadGeneralCapability(np) == SUCCESS, ERROR, "LoadGeneralCapability failed.");
                break;
            case NODE_TYPE_PURE_VOICE_CHANGE:
                CHECK_AND_RETURN_RET_LOG(LoadPureCapability(np) == SUCCESS, ERROR, "LoadPureCapability failed.");
                break;
            case NODE_TYPE_TEMPO_PITCH:
                CHECK_AND_RETURN_RET_LOG(LoadTempoPitchCapability(np) == SUCCESS, ERROR,
                    "LoadTempoPitchCapability failed.");
                break;
            case NODE_TYPE_SPACE_RENDER:
                CHECK_AND_RETURN_RET_LOG(LoadSrCapability(np) == SUCCESS, ERROR, "LoadSrCapability failed.");
                break;
            default:
                AUDIO_ERR_LOG("no such nodeType: %{public}d configured.", nodeType);
                return ERROR;
        }
        np.isLoaded = true;
    }
    nodeParameter = np;
    return SUCCESS;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
