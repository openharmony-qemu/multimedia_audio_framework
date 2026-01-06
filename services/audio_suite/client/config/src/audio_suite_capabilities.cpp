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

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

AudioSuiteCapabilities::AudioSuiteCapabilities()
{
    CHECK_AND_RETURN_LOG(audioSuiteCapabilitiesParser_.LoadConfiguration(audioSuiteCapabilities_),
        "audioSuiteCapabilitiesParser LoadConfiguration failed, path: %{public}s.",
        AUDIO_SUITE_CAPABILITIES_CONFIG_FILE);
}

int32_t AudioSuiteCapabilities::LoadVbCapability(NodeParameter &nc)
{
    AudioVoiceMorhpingSpec specs;
    CHECK_AND_RETURN_RET_LOG(
        LoadCapability("AudioVoiceMorphingGetSpec", nc.soPath + nc.soName, specs) == SUCCESS,
        ERROR, "LoadVbCapability failed.");
    CHECK_AND_RETURN_RET_LOG(SetAudioParameters(nc, specs) == SUCCESS, ERROR, "SetAudioParameters failed.");
    return SUCCESS;
}

int32_t AudioSuiteCapabilities::LoadEqCapability(NodeParameter &nc)
{
    iMedia_Support_SPECS specs;
    CHECK_AND_RETURN_RET_LOG(
        LoadCapability("iMedia_Eq_GetSPECS", nc.soPath + nc.soName, specs) == SUCCESS,
        ERROR, "LoadEqCapability failed.");
    CHECK_AND_RETURN_RET_LOG(SetAudioParameters(nc, specs) == SUCCESS, ERROR, "SetAudioParameters failed.");
    return SUCCESS;
}

int32_t AudioSuiteCapabilities::LoadSfCapability(NodeParameter &nc)
{
    iMedia_Support_SPECS specs;
    CHECK_AND_RETURN_RET_LOG(
        LoadCapability("iMedia_Surround_GetSPECS", nc.soPath + nc.soName, specs) == SUCCESS,
        ERROR, "LoadSfCapability failed.");
    CHECK_AND_RETURN_RET_LOG(SetAudioParameters(nc, specs) == SUCCESS, ERROR, "SetAudioParameters failed.");
    return SUCCESS;
}

int32_t AudioSuiteCapabilities::LoadEnvCapability(NodeParameter &nc)
{
    iMedia_Support_SPECS specs;
    CHECK_AND_RETURN_RET_LOG(
        LoadCapability("iMedia_Env_GetSPECS", nc.soPath + nc.soName, specs) == SUCCESS,
        ERROR, "LoadEnvCapability failed.");
    CHECK_AND_RETURN_RET_LOG(SetAudioParameters(nc, specs) == SUCCESS, ERROR, "SetAudioParameters failed.");
    return SUCCESS;
}

int32_t AudioSuiteCapabilities::LoadSrCapability(NodeParameter &nc)
{
    AUDIO_INFO_LOG("loadSrCapability start.");
    std::string algoSoPath = nc.soPath + nc.soName;
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
    CHECK_AND_RETURN_RET_LOG(SetAudioParameters(nc, specs) == SUCCESS, ERROR, "SetAudioParameters failed.");
    dlclose(libHandle);
    libHandle = nullptr;
    AUDIO_INFO_LOG("loadCapability end.");
    return SUCCESS;
}

int32_t AudioSuiteCapabilities::LoadAinrCapability(NodeParameter &nc)
{
    AUDIO_INFO_LOG("loadCapability start.");
    std::string algoSoPath = nc.soPath + nc.soName;
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
    nc.supportedOnThisDevice = specs->isSupport;
    nc.frameLen = specs->frameLen;
    nc.inSampleRate = specs->inSampleRate;
    nc.inChannels = specs->inChannels;
    nc.inFormat = specs->inFormat;
    nc.outSampleRate = specs->outSampleRate;
    nc.outChannels = specs->outChannels;
    nc.outFormat = specs->outFormat;
    dlclose(libHandle);
    libHandle = nullptr;
    AUDIO_INFO_LOG("loadCapability end.");
    return SUCCESS;
}

int32_t AudioSuiteCapabilities::LoadAissCapability(NodeParameter &nc)
{
    AUDIO_INFO_LOG("LoadAissCapability start.");
    std::string algoSoPath = nc.soPath + nc.soName;
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
    CHECK_AND_RETURN_RET_LOG(SetAudioParameters(nc, supportConfig) == SUCCESS, ERROR, "SetAudioParameters failed.");

    dlclose(libHandle);
    libHandle = nullptr;
    AUDIO_INFO_LOG("LoadAissCapability end.");
    return SUCCESS;
}

int32_t AudioSuiteCapabilities::LoadGeneralCapability(NodeParameter &nc)
{
    AudioVoiceMorhpingSpec specs;
    CHECK_AND_RETURN_RET_LOG(
        LoadCapability("AudioVoiceMorphingGetSpec", nc.soPath + nc.soName, specs) == SUCCESS,
        ERROR, "LoadGeneralCapability failed.");
    CHECK_AND_RETURN_RET_LOG(SetAudioParameters(nc, specs) == SUCCESS, ERROR, "SetAudioParameters failed.");
    return SUCCESS;
}
 
int32_t AudioSuiteCapabilities::LoadPureCapability(NodeParameter &nc)
{
    AudioVoiceMphTradSpec specs;
    CHECK_AND_RETURN_RET_LOG(
        LoadCapability("AudioVoiceMphTradGetSpec", nc.soPath + nc.soName, specs) == SUCCESS,
        ERROR, "LoadPureCapability failed.");
    CHECK_AND_RETURN_RET_LOG(SetAudioParameters(nc, specs) == SUCCESS, ERROR, "SetAudioParameters failed.");
    return SUCCESS;
}

int32_t AudioSuiteCapabilities::LoadTempoPitchCapability(NodeParameter &nc)
{
    AUDIO_INFO_LOG("LoadTempoPitchCapability start.");
    std::istringstream iss(nc.soName);
    std::string tempoSoName = "";
    std::string pitchSoName = "";
    std::getline(iss, tempoSoName, ',');
    std::getline(iss, pitchSoName);
    CHECK_AND_RETURN_RET_LOG(!tempoSoName.empty() && !pitchSoName.empty(), ERROR,
        "LoadTempoPitchCapability parse so name fail");
    // tempo
    std::string tempoSoPath = nc.soPath + tempoSoName;
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
    CHECK_AND_RETURN_RET_LOG(SetAudioParameters(nc, specs) == SUCCESS, ERROR, "SetAudioParameters failed.");
    dlclose(tempoSoHandle);
    tempoSoHandle = nullptr;
    // pitch
    std::string pitchSoPath = nc.soPath + pitchSoName;
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
    nc.supportedOnThisDevice &= supportConfig.isSupport;
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
        NodeParameter &nc = it->second;
        if (nodeType == NODE_TYPE_TEMPO_PITCH) {
            std::istringstream iss(nc.soName);
            std::string tempoSoName = "";
            std::string pitchSoName = "";
            std::getline(iss, tempoSoName, ',');
            std::getline(iss, pitchSoName);
            if (!(std::filesystem::exists(nc.soPath + tempoSoName)) ||
                !(std::filesystem::exists(nc.soPath + pitchSoName))) {
                *isSupported = false;
                AUDIO_INFO_LOG("nodeType: %{public}d is not supported on this device, so does not exist.", nodeType);
                return SUCCESS;
            }
        } else if (!(std::filesystem::exists(nc.soPath + nc.soName))) {
            *isSupported = false;
            AUDIO_INFO_LOG("nodeType: %{public}d is not supported on this device, so does not exist.", nodeType);
            return SUCCESS;
        }
        if (nc.general == "yes") {
            AUDIO_INFO_LOG("nodeType: %{public}d is general on all device.", nodeType);
            *isSupported = true;
            return SUCCESS;
        } else {
            if (GetNodeParameter(nodeType, nc) == SUCCESS) {
                AUDIO_INFO_LOG("nodeType: %{public}d isSupported status is %{public}d on this device.",
                    nodeType, nc.supportedOnThisDevice);
                *isSupported = nc.supportedOnThisDevice;
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
int32_t AudioSuiteCapabilities::GetNodeParameter(AudioNodeType nodeType, NodeParameter &nodeCapability)
{
    CHECK_AND_RETURN_RET(nodeType != NODE_TYPE_AUDIO_MIXER, SUCCESS);
    auto it = audioSuiteCapabilities_.find(nodeType);
    CHECK_AND_RETURN_RET_LOG(
        it != audioSuiteCapabilities_.end(), ERROR, "no such nodeType: %{public}d configured.", nodeType);
    NodeParameter &nc = it->second;
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
            case NODE_TYPE_GENERAL_VOICE_CHANGE:
                CHECK_AND_RETURN_RET_LOG(LoadGeneralCapability(nc) == SUCCESS, ERROR, "LoadGeneralCapability failed.");
                break;
            case NODE_TYPE_PURE_VOICE_CHANGE:
                CHECK_AND_RETURN_RET_LOG(LoadPureCapability(nc) == SUCCESS, ERROR, "LoadPureCapability failed.");
                break;
            case NODE_TYPE_TEMPO_PITCH:
                CHECK_AND_RETURN_RET_LOG(LoadTempoPitchCapability(nc) == SUCCESS, ERROR,
                    "LoadTempoPitchCapability failed.");
                break;
            case NODE_TYPE_SPACE_RENDER:
                CHECK_AND_RETURN_RET_LOG(LoadSrCapability(nc) == SUCCESS, ERROR, "LoadSrCapability failed.");
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
