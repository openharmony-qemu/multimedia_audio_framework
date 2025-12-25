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

#ifndef LOG_TAG
#define LOG_TAG "AudioSuiteSpaceRenderAlgoInterface"
#endif

#include <dlfcn.h>
#include <vector>
#include <sstream>
#include <cstring>
#include <iostream>
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_space_render_algo_interface_impl.h"
#include "audio_suite_algo_interface.h"
#include "audio_hms_space_render_api.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
constexpr uint32_t SPACE_RENDER_POSITIONS_PARAMS_NUM = 3;
constexpr uint32_t SPACE_RENDER_ROTATION_PARAMS_NUM = 5;
constexpr uint32_t SPACE_RENDER_EXTENSION_PARAMS_NUM = 2;

constexpr uint32_t PARAMS_NUM_ZERO = 0;
constexpr uint32_t PARAMS_NUM_ONE = 1;
constexpr uint32_t PARAMS_NUM_TWO = 2;
constexpr uint32_t PARAMS_NUM_THREE = 3;
constexpr uint32_t PARAMS_NUM_FOUR = 4;
constexpr uint32_t INPUT_TIME = 20;
constexpr uint32_t INPUT_RATE = 48000;
constexpr uint32_t INPUT_CHANNEL = 2;
constexpr uint32_t INPUT_DATA_LENGTH = INPUT_TIME * INPUT_RATE * INPUT_CHANNEL / 1000;

const std::string SPACE_RENDER_POSITIONS_MOD = "AudioSpaceRenderPositionParams";
const std::string SPACE_RENDER_ROTATION_MOD = "AudioSpaceRenderRotationParams";
const std::string SPACE_RENDER_EXTENSION_MOD = "AudioSpaceRenderExtensionParams";

AudioSuiteSpaceRenderAlgoInterfaceImpl::AudioSuiteSpaceRenderAlgoInterfaceImpl(NodeCapability &nc)
{
    nodeCapability = nc;
    AUDIO_INFO_LOG("AudioSuiteSpaceRenderAlgoInterfaceImpl::AudioSuiteSpaceRenderAlgoInterfaceImpl()");
}

AudioSuiteSpaceRenderAlgoInterfaceImpl::~AudioSuiteSpaceRenderAlgoInterfaceImpl()
{
    AUDIO_INFO_LOG("AudioSuiteSpaceRenderAlgoInterfaceImpl::~AudioSuiteSpaceRenderAlgoInterfaceImpl()");
    if (isSpaceRenderAlgoInit_) {
        Deinit();
    }
}

int32_t AudioSuiteSpaceRenderAlgoInterfaceImpl::Init()
{
    std::string soPath = nodeCapability.soPath + nodeCapability.soName;
    libHandle_ = algoLibrary_.LoadLibrary(soPath);
    CHECK_AND_RETURN_RET_LOG(libHandle_ != nullptr, ERROR,
        "LoadLibrary failed with path: %{private}s", soPath.c_str());
 
    algoApi_.getSpeces = reinterpret_cast<FunSpaceRenderGetSpeces>(dlsym(libHandle_, "SpaceRenderGetSpeces"));
    CHECK_AND_RETURN_RET_LOG(algoApi_.getSpeces != nullptr, ERROR, "Failed to get symbol SpaceRenderGetSpeces");
    algoApi_.getSize = reinterpret_cast<FunSpaceRenderGetSize>(dlsym(libHandle_, "SpaceRenderGetSize"));
    CHECK_AND_RETURN_RET_LOG(algoApi_.getSize != nullptr, ERROR, "Failed to get symbol SpaceRenderGetSize");
    algoApi_.getLateSamples = reinterpret_cast<FunSpaceRenderGetLateSamples>(dlsym(libHandle_,
        "SpaceRenderGetLateSamples"));
    CHECK_AND_RETURN_RET_LOG(algoApi_.getLateSamples != nullptr, ERROR,
        "Failed to get symbol SpaceRenderGetLateSamples");
    algoApi_.initAlgo = reinterpret_cast<FunSpaceRenderInit>(dlsym(libHandle_, "SpaceRenderInit"));
    CHECK_AND_RETURN_RET_LOG(algoApi_.initAlgo != nullptr, ERROR, "Failed to get symbol SpaceRenderInit");
    algoApi_.applyAlgo = reinterpret_cast<FunSpaceRenderApply>(dlsym(libHandle_, "SpaceRenderApply"));
    CHECK_AND_RETURN_RET_LOG(algoApi_.applyAlgo != nullptr, ERROR, "Failed to get symbol SpaceRenderApply");
    algoApi_.releaseAlgo = reinterpret_cast<FunSpaceRenderRelease>(dlsym(libHandle_, "SpaceRenderRelease"));
    CHECK_AND_RETURN_RET_LOG(algoApi_.releaseAlgo != nullptr, ERROR, "Failed to get symbol SpaceRenderRelease");
 
    isSpaceRenderAlgoInit_ = true;
    return SUCCESS;
}

int32_t AudioSuiteSpaceRenderAlgoInterfaceImpl::Deinit()
{
    isSpaceRenderAlgoInit_ = false;
    algoApi_.releaseAlgo(spaceRenderHandle_.data());
    if (libHandle_ != nullptr) {
        int32_t ret = dlclose(libHandle_);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ret, "dlclose failed: %{public}s", dlerror());
        libHandle_ = nullptr;
    }
    static_cast<void>(memset_s(&algoApi_, sizeof(algoApi_), 0, sizeof(algoApi_)));
    AUDIO_INFO_LOG("AudioSuiteSpaceRenderAlgoInterfaceImpl::Deinit end");
    return SUCCESS;
}

std::vector<std::string> SplitString(const std::string &str, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream iss(str);
 
    while (std::getline(iss, token, delimiter)) {
        tokens.push_back(token);
    }
 
    return tokens;
}

int AudioSuiteSpaceRenderAlgoInterfaceImpl::SetPositionParameter(const std::string &paramValue)
{
    spaceRenderParam_.mode = SPACE_RENDER_MODE_STATIC;
    std::vector<std::string> tokens = SplitString(paramValue, ',');
 
    CHECK_AND_RETURN_RET_LOG(tokens.size() == SPACE_RENDER_POSITIONS_PARAMS_NUM,
        ERR_INVALID_PARAM, "Invalid Position parameter format.");
 
    spaceRenderParam_.cartPoint[PARAMS_NUM_ZERO] = std::stof(tokens[PARAMS_NUM_ZERO]);
    spaceRenderParam_.cartPoint[PARAMS_NUM_ONE] = std::stof(tokens[PARAMS_NUM_ONE]);
    spaceRenderParam_.cartPoint[PARAMS_NUM_TWO] = std::stof(tokens[PARAMS_NUM_TWO]);
    AUDIO_INFO_LOG("SpaceRender set Position x :%{public}f y :%{public}f z :%{public}f",
        spaceRenderParam_.cartPoint[PARAMS_NUM_ZERO],
        spaceRenderParam_.cartPoint[PARAMS_NUM_ONE], spaceRenderParam_.cartPoint[PARAMS_NUM_TWO]);
    return SUCCESS;
}

int AudioSuiteSpaceRenderAlgoInterfaceImpl::SetRotationParameter(const std::string &paramValue)
{
    spaceRenderParam_.mode = SPACE_RENDER_MODE_ROTATION;
    std::vector<std::string> tokens = SplitString(paramValue, ',');
 
    CHECK_AND_RETURN_RET_LOG(tokens.size() == SPACE_RENDER_ROTATION_PARAMS_NUM,
        ERR_INVALID_PARAM, "Invalid Rotation parameter format.");
 
    spaceRenderParam_.cartPoint[PARAMS_NUM_ZERO] = std::stof(tokens[PARAMS_NUM_ZERO]);
    spaceRenderParam_.cartPoint[PARAMS_NUM_ONE] = std::stof(tokens[PARAMS_NUM_ONE]);
    spaceRenderParam_.cartPoint[PARAMS_NUM_TWO] = std::stof(tokens[PARAMS_NUM_TWO]);
    spaceRenderParam_.rotationTime = std::stoi(tokens[PARAMS_NUM_THREE]);
    spaceRenderParam_.rotationDirection = static_cast<SpaceRenderRotationMode>(std::stoi(tokens[PARAMS_NUM_FOUR]));
    AUDIO_INFO_LOG(
        "SpaceRender set Rotation x :%{public}f y :%{public}f z :%{public}f time :%{public}f Direction :%{public}d",
        spaceRenderParam_.cartPoint[PARAMS_NUM_ZERO],
        spaceRenderParam_.cartPoint[PARAMS_NUM_ONE], spaceRenderParam_.cartPoint[PARAMS_NUM_TWO],
        spaceRenderParam_.rotationTime, spaceRenderParam_.rotationDirection);
    return SUCCESS;
}

int AudioSuiteSpaceRenderAlgoInterfaceImpl::SetExtensionParameter(const std::string &paramValue)
{
    spaceRenderParam_.mode = SPACE_RENDER_MODE_EXPAND;
    std::vector<std::string> tokens = SplitString(paramValue, ',');
 
    CHECK_AND_RETURN_RET_LOG(tokens.size() == SPACE_RENDER_EXTENSION_PARAMS_NUM,
        ERR_INVALID_PARAM, "Invalid Extension parameter format.");
 
    spaceRenderParam_.expandRadius = std::stof(tokens[PARAMS_NUM_ZERO]);
    spaceRenderParam_.expandAngle = std::stoi(tokens[PARAMS_NUM_ONE]);
    AUDIO_INFO_LOG(
        "SpaceRender set Rotation Radius :%{public}f Angle :%{public}d",
        spaceRenderParam_.expandRadius, spaceRenderParam_.expandAngle);
    return SUCCESS;
}

int32_t AudioSuiteSpaceRenderAlgoInterfaceImpl::SetParameter(const std::string &paramType,
    const std::string &paramValue)
{
    int32_t ret;
    if (spaceRenderHandle_.data()) {
        algoApi_.releaseAlgo(spaceRenderHandle_.data());
    }

    if (paramType == SPACE_RENDER_POSITIONS_MOD) {
        ret = SetPositionParameter(paramValue);
    } else if (paramType == SPACE_RENDER_ROTATION_MOD) {
        ret = SetRotationParameter(paramValue);
    } else if (paramType == SPACE_RENDER_EXTENSION_MOD) {
        ret = SetExtensionParameter(paramValue);
    } else {
        AUDIO_ERR_LOG("Invalid space render mod format");
        return ERR_INVALID_PARAM;
    }
 
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Failed to set parameter.");
 
    int32_t handleSize = algoApi_.getSize(&spaceRenderParam_);
    CHECK_AND_RETURN_RET_LOG(handleSize >= 0, handleSize, "getSize error.");
 
    spaceRenderHandle_.resize(handleSize);
 
    ret = algoApi_.initAlgo(spaceRenderHandle_.data(), &spaceRenderParam_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "space render init failed.");

    return SUCCESS;
}

int AudioSuiteSpaceRenderAlgoInterfaceImpl::GetPositionParameter(std::string &paramValue)
{
    CHECK_AND_RETURN_RET_LOG(spaceRenderParam_.mode == SPACE_RENDER_MODE_STATIC,
        ERR_INVALID_PARAM, "Mode is not Position.");
 
    paramValue = std::to_string(spaceRenderParam_.cartPoint[PARAMS_NUM_ZERO]) + "," +
        std::to_string(spaceRenderParam_.cartPoint[PARAMS_NUM_ONE]) + "," +
        std::to_string(spaceRenderParam_.cartPoint[PARAMS_NUM_TWO]);

    return SUCCESS;
}

int AudioSuiteSpaceRenderAlgoInterfaceImpl::GetRotationParameter(std::string &paramValue)
{
    CHECK_AND_RETURN_RET_LOG(spaceRenderParam_.mode == SPACE_RENDER_MODE_ROTATION,
        ERR_INVALID_PARAM, "Mode is not rotation.");

    int32_t rotationTime = static_cast<int32_t>(spaceRenderParam_.rotationTime);

    paramValue = std::to_string(spaceRenderParam_.cartPoint[PARAMS_NUM_ZERO]) + "," +
        std::to_string(spaceRenderParam_.cartPoint[PARAMS_NUM_ONE]) + "," +
        std::to_string(spaceRenderParam_.cartPoint[PARAMS_NUM_TWO]) + "," +
        std::to_string(rotationTime) + "," +
        std::to_string(spaceRenderParam_.rotationDirection);

    return SUCCESS;
}

int AudioSuiteSpaceRenderAlgoInterfaceImpl::GetExtensionParameter(std::string &paramValue)
{
    CHECK_AND_RETURN_RET_LOG(spaceRenderParam_.mode == SPACE_RENDER_MODE_EXPAND,
        ERR_INVALID_PARAM, "Mode is not extension.");
 
    paramValue = std::to_string(spaceRenderParam_.expandRadius) + "," +
        std::to_string(spaceRenderParam_.expandAngle);

    return SUCCESS;
}

int32_t AudioSuiteSpaceRenderAlgoInterfaceImpl::GetParameter(const std::string &paramType, std::string &paramValue)
{
    int32_t ret;
    if (paramType == SPACE_RENDER_POSITIONS_MOD) {
        ret = GetPositionParameter(paramValue);
    } else if (paramType == SPACE_RENDER_ROTATION_MOD) {
        ret = GetRotationParameter(paramValue);
    } else if (paramType == SPACE_RENDER_EXTENSION_MOD) {
        ret = GetExtensionParameter(paramValue);
    } else {
        AUDIO_ERR_LOG("Invalid space render mod format");
        return ERR_INVALID_PARAM;
    }
 
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Space render mode is not support.");

    AUDIO_INFO_LOG("SpaceRender get %{public}s : %{public}s", paramType.c_str(), paramValue.c_str());
 
    return SUCCESS;
}

int32_t AudioSuiteSpaceRenderAlgoInterfaceImpl::Apply(std::vector<uint8_t *> &pcmInBuf,
    std::vector<uint8_t *> &pcmOutBuf)
{
    CHECK_AND_RETURN_RET_LOG(!pcmInBuf.empty(), ERROR, "pcmInBuf is empty");
    CHECK_AND_RETURN_RET_LOG(pcmInBuf[0] != nullptr, ERROR, "pcmInBuf[0] is empty");
    CHECK_AND_RETURN_RET_LOG(!pcmOutBuf.empty(), ERROR, "pcmOutBuf is empty");
    CHECK_AND_RETURN_RET_LOG(pcmOutBuf[0] != nullptr, ERROR, "pcmOutBuf[0] is empty");

    const short *bufIn = reinterpret_cast<const short *>(pcmInBuf[0]);
    short *pcmOut = reinterpret_cast<short *>(pcmOutBuf[0]);
    int32_t ret = algoApi_.applyAlgo(spaceRenderHandle_.data(), bufIn, INPUT_DATA_LENGTH, pcmOut);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "space render apply failed %{public}d ", ret);
 
    return SUCCESS;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS